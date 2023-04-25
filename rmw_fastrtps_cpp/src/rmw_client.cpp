// Copyright 2016-2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>

#include "./type_support_common.hpp"
#include "fastdds/dds/core/policy/QosPolicies.hpp"
#include "fastdds/dds/domain/DomainParticipant.hpp"
#include "fastdds/dds/publisher/Publisher.hpp"
#include "fastdds/dds/publisher/qos/DataWriterQos.hpp"
#include "fastdds/dds/subscriber/Subscriber.hpp"
#include "fastdds/dds/subscriber/qos/DataReaderQos.hpp"
#include "fastdds/dds/topic/Topic.hpp"
#include "fastdds/dds/topic/TopicDescription.hpp"
#include "fastdds/dds/topic/TypeSupport.hpp"
#include "fastdds/dds/topic/qos/TopicQos.hpp"
#include "fastdds/rtps/resources/ResourceManagement.h"
#include "rcpputils/scope_exit.hpp"
#include "rcutils/error_handling.h"
#include "rcutils/logging_macros.h"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw/validate_full_topic_name.h"
#include "rmw_dds_common/qos.hpp"
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/custom_client_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/names.hpp"
#include "rmw_fastrtps_shared_cpp/namespace_prefix.hpp"
#include "rmw_fastrtps_shared_cpp/qos.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"
#include "rmw_fastrtps_shared_cpp/utils.hpp"

extern "C" {

/**
 * @brief 创建一个服务客户端 (Create a service client)
 *
 * @param[in] node 指向 ROS2 节点的指针 (Pointer to the ROS2 node)
 * @param[in] type_supports 服务类型支持结构体 (Service type support structure)
 * @param[in] service_name 服务名称 (Service name)
 * @param[in] qos_policies 服务质量配置 (Quality of Service policies for the client)
 * @return 返回创建的服务客户端，如果失败则返回 nullptr (Returns the created service client, or
 * nullptr if failed)
 */
rmw_client_t *rmw_create_client(
    const rmw_node_t *node,
    const rosidl_service_type_support_t *type_supports,
    const char *service_name,
    const rmw_qos_profile_t *qos_policies) {
  // 检查输入参数 (Check input parameters)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, nullptr);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, eprosima_fastrtps_identifier, return nullptr);
  RMW_CHECK_ARGUMENT_FOR_NULL(type_supports, nullptr);
  RMW_CHECK_ARGUMENT_FOR_NULL(service_name, nullptr);

  // 检查服务名称是否为空字符串 (Check if the service_name is an empty string)
  if (0 == strlen(service_name)) {
    RMW_SET_ERROR_MSG("service_name argument is an empty string");
    return nullptr;
  }

  RMW_CHECK_ARGUMENT_FOR_NULL(qos_policies, nullptr);

  // 如果避免 ROS 命名空间约定，则验证 service_name (If avoid_ros_namespace_conventions is false,
  // validate the service_name)
  if (!qos_policies->avoid_ros_namespace_conventions) {
    int validation_result = RMW_TOPIC_VALID;
    rmw_ret_t ret = rmw_validate_full_topic_name(service_name, &validation_result, nullptr);

    // 验证失败，返回 nullptr (Validation failed, return nullptr)
    if (RMW_RET_OK != ret) {
      return nullptr;
    }

    // 如果验证结果无效，则设置错误消息并返回 nullptr (If validation result is invalid, set error
    // message and return nullptr)
    if (RMW_TOPIC_VALID != validation_result) {
      const char *reason = rmw_full_topic_name_validation_result_string(validation_result);
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("service_name argument is invalid: %s", reason);
      return nullptr;
    }
  }

  /**
   * @brief 创建客户端并设置QoS策略、参与者和子实体，以及获取RMW类型支持。
   * @param[in] qos_policies QoS策略指针
   * @param[in] node 节点指针
   * @param[in] type_supports 服务类型支持指针
   * @return nullptr 如果创建失败，否则返回创建的客户端指针
   *
   * @brief Create a client with the given QoS policies, participant and sub-entities, and get RMW
   * type support.
   * @param[in] qos_policies Pointer to QoS policies
   * @param[in] node Pointer to the node
   * @param[in] type_supports Pointer to service type supports
   * @return nullptr if creation fails, otherwise returns the pointer to the created client
   */
  rmw_qos_profile_t adapted_qos_policies =
      rmw_dds_common::qos_profile_update_best_available_for_services(*qos_policies);

  // 检查RMW QoS是否有效
  // Check if RMW QoS is valid
  if (!is_valid_qos(adapted_qos_policies)) {
    RMW_SET_ERROR_MSG("create_client() called with invalid QoS");
    return nullptr;
  }

  // 获取参与者和子实体
  // Get Participant and SubEntities
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);
  auto participant_info =
      static_cast<CustomParticipantInfo *>(node->context->impl->participant_info);

  eprosima::fastdds::dds::DomainParticipant *dds_participant = participant_info->participant_;
  eprosima::fastdds::dds::Publisher *publisher = participant_info->publisher_;
  eprosima::fastdds::dds::Subscriber *subscriber = participant_info->subscriber_;

  // 获取RMW类型支持
  // Get RMW Type Support
  const rosidl_service_type_support_t *type_support =
      get_service_typesupport_handle(type_supports, RMW_FASTRTPS_CPP_TYPESUPPORT_C);
  if (!type_support) {
    rcutils_error_string_t prev_error_string = rcutils_get_error_string();
    rcutils_reset_error();
    type_support = get_service_typesupport_handle(type_supports, RMW_FASTRTPS_CPP_TYPESUPPORT_CPP);
    if (!type_support) {
      rcutils_error_string_t error_string = rcutils_get_error_string();
      rcutils_reset_error();
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "Type support not from this implementation. Got:\n"
          "    %s\n"
          "    %s\n"
          "while fetching it",
          prev_error_string.str, error_string.str);
      return nullptr;
    }
  }

  // 对参与者信息的实体创建互斥锁进行加锁，以确保线程安全
  // Lock the entity creation mutex of the participant info to ensure thread safety
  std::lock_guard<std::mutex> lck(participant_info->entity_creation_mutex_);

  // 找到并检查现有的主题和类型
  // Find and check existing topics and types

  // 创建 Topic 和 Type 名称
  // Create Topic and Type names
  auto service_members = static_cast<const service_type_support_callbacks_t *>(type_support->data);
  auto request_members = static_cast<const message_type_support_callbacks_t *>(
      service_members->request_members_->data);
  auto response_members = static_cast<const message_type_support_callbacks_t *>(
      service_members->response_members_->data);

  // 根据请求成员创建类型名称
  // Create type name based on request members
  std::string request_type_name = _create_type_name(request_members);
  // 根据响应成员创建类型名称
  // Create type name based on response members
  std::string response_type_name = _create_type_name(response_members);

  // 根据给定参数创建请求主题名称
  // Create request topic name based on given parameters
  std::string request_topic_name =
      _create_topic_name(
          &adapted_qos_policies, ros_service_requester_prefix, service_name, "Request")
          .to_string();
  // 根据给定参数创建响应主题名称
  // Create response topic name based on given parameters
  std::string response_topic_name =
      _create_topic_name(&adapted_qos_policies, ros_service_response_prefix, service_name, "Reply")
          .to_string();

  // 获取请求主题和类型
  // Get request topic and type
  eprosima::fastdds::dds::TypeSupport request_fastdds_type;
  eprosima::fastdds::dds::TopicDescription *request_topic_desc = nullptr;
  if (!rmw_fastrtps_shared_cpp::find_and_check_topic_and_type(
          participant_info, request_topic_name, request_type_name, &request_topic_desc,
          &request_fastdds_type)) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "create_client() called for existing request topic name %s with incompatible type %s",
        request_topic_name.c_str(), request_type_name.c_str());
    return nullptr;
  }

  // 获取响应主题和类型
  // Get response topic and type
  eprosima::fastdds::dds::TypeSupport response_fastdds_type;
  eprosima::fastdds::dds::TopicDescription *response_topic_desc = nullptr;
  if (!rmw_fastrtps_shared_cpp::find_and_check_topic_and_type(
          participant_info, response_topic_name, response_type_name, &response_topic_desc,
          &response_fastdds_type)) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "create_client() called for existing response topic name %s with incompatible type %s",
        response_topic_name.c_str(), response_type_name.c_str());
    return nullptr;
  }

  // 创建自定义客户端结构体（info）
  // Create the custom Client struct (info)
  CustomClientInfo *info = new (std::nothrow) CustomClientInfo();
  if (!info) {
    // 如果分配内存失败，设置错误信息
    // If memory allocation fails, set the error message
    RMW_SET_ERROR_MSG("create_client() failed to allocate custom info");
    return nullptr;
  }

  // 创建一个作用域退出时清理info和participant_info的lambda函数
  // Create a lambda function to clean up info and participant_info when scope exits
  auto cleanup_info = rcpputils::make_scope_exit([info, participant_info]() {
    rmw_fastrtps_shared_cpp::remove_topic_and_type(
        participant_info, nullptr, info->response_topic_, info->response_type_support_);
    rmw_fastrtps_shared_cpp::remove_topic_and_type(
        participant_info, nullptr, info->request_topic_, info->request_type_support_);
    delete info->pub_listener_;
    delete info->listener_;
    delete info;
  });

  // 设置类型支持标识符
  // Set the type support identifier
  info->typesupport_identifier_ = type_support->typesupport_identifier;
  // 初始化请求发布者匹配计数为0
  // Initialize request publisher matched count to 0
  info->request_publisher_matched_count_ = 0;
  // 初始化响应订阅者匹配计数为0
  // Initialize response subscriber matched count to 0
  info->response_subscriber_matched_count_ = 0;

  // 创建类型支持结构体 (Create the Type Support structs)
  info->request_type_support_impl_ =
      request_members;  // 将请求成员赋值给请求类型支持实现 (Assign request members to request type
                        // support implementation)
  info->response_type_support_impl_ =
      response_members;  // 将响应成员赋值给响应类型支持实现 (Assign response members to response
                         // type support implementation)

  if (!request_fastdds_type) {
    // 使用 service_members 创建 RequestTypeSupport_cpp 对象 (Create a RequestTypeSupport_cpp object
    // using service_members)
    auto tsupport = new (std::nothrow) RequestTypeSupport_cpp(service_members);
    if (!tsupport) {
      RMW_SET_ERROR_MSG("create_client() failed to allocate request typesupport");
      return nullptr;
    }

    request_fastdds_type.reset(
        tsupport);  // 设置 request_fastdds_type 指针 (Set the request_fastdds_type pointer)
  }
  if (!response_fastdds_type) {
    // 使用 service_members 创建 ResponseTypeSupport_cpp 对象 (Create a ResponseTypeSupport_cpp
    // object using service_members)
    auto tsupport = new (std::nothrow) ResponseTypeSupport_cpp(service_members);
    if (!tsupport) {
      RMW_SET_ERROR_MSG("create_client() failed to allocate response typesupport");
      return nullptr;
    }

    response_fastdds_type.reset(
        tsupport);  // 设置 response_fastdds_type 指针 (Set the response_fastdds_type pointer)
  }

  // 注册请求类型 (Register the request type)
  if (ReturnCode_t::RETCODE_OK != request_fastdds_type.register_type(dds_participant)) {
    RMW_SET_ERROR_MSG("create_client() failed to register request type");
    return nullptr;
  }
  info->request_type_support_ =
      request_fastdds_type;  // 设置请求类型支持 (Set the request type support)

  // 注册响应类型 (Register the response type)
  if (ReturnCode_t::RETCODE_OK != response_fastdds_type.register_type(dds_participant)) {
    RMW_SET_ERROR_MSG("create_client() failed to register response type");
    return nullptr;
  }
  info->response_type_support_ =
      response_fastdds_type;  // 设置响应类型支持 (Set the response type support)

  // 创建监听器 (Create Listeners)
  info->listener_ = new (std::nothrow)
      ClientListener(info);  // 创建响应订阅者监听器 (Create response subscriber listener)
  if (!info->listener_) {
    RMW_SET_ERROR_MSG("create_client() failed to create response subscriber listener");
    return nullptr;
  }

  info->pub_listener_ = new (std::nothrow)
      ClientPubListener(info);  // 创建请求发布者监听器 (Create request publisher listener)
  if (!info->pub_listener_) {
    RMW_SET_ERROR_MSG("create_client() failed to create request publisher listener");
    return nullptr;
  }

  // 创建并注册 Topics (Create and register Topics)
  // 为两个主题使用相同的默认主题 QoS (Same default topic QoS for both topics)
  eprosima::fastdds::dds::TopicQos topic_qos = dds_participant->get_default_topic_qos();
  if (!get_topic_qos(adapted_qos_policies, topic_qos)) {
    // 设置主题 QoS 失败时返回错误信息 (Set error message when setting topic QoS fails)
    RMW_SET_ERROR_MSG("create_client() failed setting topic QoS");
    return nullptr;
  }

  // 创建响应主题 (Create response topic)
  info->response_topic_ = participant_info->find_or_create_topic(
      response_topic_name, response_type_name, topic_qos, nullptr);
  if (!info->response_topic_) {
    // 创建响应主题失败时返回错误信息 (Set error message when creating response topic fails)
    RMW_SET_ERROR_MSG("create_client() failed to create response topic");
    return nullptr;
  }

  response_topic_desc = info->response_topic_;

  // 创建请求主题 (Create request topic)
  info->request_topic_ = participant_info->find_or_create_topic(
      request_topic_name, request_type_name, topic_qos, nullptr);
  if (!info->request_topic_) {
    // 创建请求主题失败时返回错误信息 (Set error message when creating request topic fails)
    RMW_SET_ERROR_MSG("create_client() failed to create request topic");
    return nullptr;
  }

  info->request_topic_name_ = request_topic_name;
  info->response_topic_name_ = response_topic_name;

  // 查找 DataWriter 和 DataReader QoS 的关键字 (Keyword to find DataWriter and DataReader QoS)
  const std::string topic_name_fallback = "client";

  // 创建响应 DataReader
  // Create response DataReader

  // 如果定义了 FASTRTPS_DEFAULT_PROFILES_FILE，则使用 _create_topic_name()
  // 定义的主题名称来定位订阅者配置文件， 填充 DataReader
  // QoS。如果找不到配置文件，尝试使用"profile_name"为 "client" 的搜索。否则，使用默认的 Fast DDS
  // QoS。 If FASTRTPS_DEFAULT_PROFILES_FILE defined, fill DataReader QoS with a subscriber profile
  // located based on topic name defined by _create_topic_name(). If no profile is found, a search
  // with profile_name "client" is attempted. Else, use the default Fast DDS QoS.
  eprosima::fastdds::dds::DataReaderQos reader_qos = subscriber->get_default_datareader_qos();

  // 尝试加载名为 "client" 的配置文件，
  // 如果不存在，则尝试使用响应主题名称
  // 不需要检查返回代码，因为如果任何配置文件都不存在，
  // QoS已经正确设置：如果都不存在则为默认值，如果只存在一个则为所选值，
  // 如果两个都存在，则选择主题名称
  // Try to load the profile named "client",
  // if it does not exist it tries with the response topic name
  // It does not need to check the return code, as if any of the profile does not exist,
  // the QoS is already set correctly:
  // If none exist is default, if only one exists is the one chosen,
  // if both exist topic name is chosen
  subscriber->get_datareader_qos_from_profile(topic_name_fallback, reader_qos);
  subscriber->get_datareader_qos_from_profile(response_topic_name, reader_qos);

  if (!participant_info->leave_middleware_default_qos) {
    reader_qos.endpoint().history_memory_policy =
        eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    reader_qos.data_sharing().off();
  }

  if (!get_datareader_qos(
          adapted_qos_policies,
          *type_supports->response_typesupport->get_type_hash_func(
              type_supports->response_typesupport),
          reader_qos)) {
    RMW_SET_ERROR_MSG("create_client() failed setting response DataReader QoS");
    return nullptr;
  }

  // 创建 DataReader
  // Creates DataReader
  info->response_reader_ = subscriber->create_datareader(
      response_topic_desc, reader_qos, info->listener_,
      eprosima::fastdds::dds::StatusMask::subscription_matched());

  if (!info->response_reader_) {
    RMW_SET_ERROR_MSG("create_client() failed to create response DataReader");
    return nullptr;
  }

  info->response_reader_->get_statuscondition().set_enabled_statuses(
      eprosima::fastdds::dds::StatusMask::data_available());

  // lambda 函数用于删除 datareader
  // lambda to delete datareader
  auto cleanup_datareader = rcpputils::make_scope_exit(
      [subscriber, info]() { subscriber->delete_datareader(info->response_reader_); });

  // 如果定义了 FASTRTPS_DEFAULT_PROFILES_FILE，则使用 _create_topic_name()
  // 定义的主题名称来定位发布者配置文件， 填充 DataWriter
  // QoS。如果找不到配置文件，尝试使用"profile_name"为 "client" 的搜索。否则，使用默认的 Fast DDS
  // QoS。 If FASTRTPS_DEFAULT_PROFILES_FILE defined, fill DataWriter QoS with a publisher profile
  // located based on topic name defined by _create_topic_name(). If no profile is found, a search
  // with profile_name "client" is attempted. Else, use the default Fast DDS QoS.
  eprosima::fastdds::dds::DataWriterQos writer_qos = publisher->get_default_datawriter_qos();

  /*
  topic_name_fallback: 字符串类型，表示回退主题名称。(String type, representing the fallback topic
  name.) writer_qos: DataWriterQos 类型，表示要设置的 DataWriter QoS。(DataWriterQos type,
  representing the DataWriter QoS to be set.) request_topic_name:
  字符串类型，表示请求主题名称。(String type, representing the request topic name.)
  participant_info: 存储参与者信息的结构体。(Structure that stores participant information.)
  publishing_mode_t: 枚举类型，表示发布模式（异步或同步）。(Enum type, representing publishing mode
  (asynchronous or synchronous).) adapted_qos_policies: 自适应 QoS 策略集合。(Adaptive QoS policies
  collection.) type_supports: 类型支持结构体，包含请求和响应类型支持。(Type supports structure,
  containing request and response type supports.) info: 存储客户端信息的结构体。(Structure that
  stores client information.) publisher: 发布者实例。(Publisher instance.)
  */
  // 尝试加载名为 "client" 的配置文件，
  // 如果不存在，则尝试使用请求主题名称。
  // 不需要检查返回码，因为如果任何配置文件都不存在，
  // QoS 已经设置正确：
  // 如果都不存在则为默认值，如果只存在一个则为所选值，
  // 如果两者都存在，则选择主题名称
  // Try to load the profile named "client",
  // if it does not exist it tries with the request topic name.
  // It does not need to check the return code, as if any of the profile does not exist,
  // the QoS is already set correctly:
  // If none exist is default, if only one exists is the one chosen,
  // if both exist topic name is chosen
  publisher->get_datawriter_qos_from_profile(topic_name_fallback, writer_qos);
  publisher->get_datawriter_qos_from_profile(request_topic_name, writer_qos);

  // 修改特定的 DataWriter Qos
  // Modify specific DataWriter Qos
  if (!participant_info->leave_middleware_default_qos) {
    if (participant_info->publishing_mode == publishing_mode_t::ASYNCHRONOUS) {
      writer_qos.publish_mode().kind = eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE;
    } else if (participant_info->publishing_mode == publishing_mode_t::SYNCHRONOUS) {
      writer_qos.publish_mode().kind = eprosima::fastrtps::SYNCHRONOUS_PUBLISH_MODE;
    }

    writer_qos.endpoint().history_memory_policy =
        eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    writer_qos.data_sharing().off();
  }

  // 获取 DataWriter QoS，如果失败则返回 nullptr
  // Get DataWriter QoS, return nullptr if failed
  if (!get_datawriter_qos(
          adapted_qos_policies,
          *type_supports->request_typesupport->get_type_hash_func(
              type_supports->request_typesupport),
          writer_qos)) {
    RMW_SET_ERROR_MSG("create_client() failed setting request DataWriter QoS");
    return nullptr;
  }

  // 使用使侦听器的 publication_matched 调用启用的掩码创建 DataWriter
  // Creates DataWriter with a mask enabling publication_matched calls for the listener
  info->request_writer_ = publisher->create_datawriter(
      info->request_topic_, writer_qos, info->pub_listener_,
      eprosima::fastdds::dds::StatusMask::publication_matched());

  // 如果创建请求 DataWriter 失败，返回 nullptr
  // Return nullptr if failed to create request DataWriter
  if (!info->request_writer_) {
    RMW_SET_ERROR_MSG("create_client() failed to create request DataWriter");
    return nullptr;
  }

  // 将 StatusCondition 设置为 none，以防止通过 WaitSets 触发
  // Set the StatusCondition to none to prevent triggering via WaitSets
  info->request_writer_->get_statuscondition().set_enabled_statuses(
      eprosima::fastdds::dds::StatusMask::none());

  // lambda 函数用于删除 datawriter
  // lambda to delete datawriter
  auto cleanup_datawriter = rcpputils::make_scope_exit(
      [publisher, info]() { publisher->delete_datawriter(info->request_writer_); });

  // 创建客户端 (Create client)

  // 调试信息 (Debug info)
  RCUTILS_LOG_DEBUG_NAMED("rmw_fastrtps_cpp", "************ Client Details *********");
  RCUTILS_LOG_DEBUG_NAMED("rmw_fastrtps_cpp", "Sub Topic %s", response_topic_name.c_str());
  RCUTILS_LOG_DEBUG_NAMED("rmw_fastrtps_cpp", "Pub Topic %s", request_topic_name.c_str());
  RCUTILS_LOG_DEBUG_NAMED("rmw_fastrtps_cpp", "***********");

  // 获取 GUID (Get the GUID)
  info->writer_guid_ = info->request_writer_->guid();
  info->reader_guid_ = info->response_reader_->guid();

  // 分配内存给 rmw_client_t 结构体 (Allocate memory for rmw_client_t structure)
  rmw_client_t *rmw_client = rmw_client_allocate();
  if (!rmw_client) {
    RMW_SET_ERROR_MSG("create_client() failed to allocate memory for rmw_client");
    return nullptr;
  }
  // 当出现异常时，清理 rmw_client (Clean up rmw_client in case of an exception)
  auto cleanup_rmw_client = rcpputils::make_scope_exit([rmw_client]() {
    rmw_free(const_cast<char *>(rmw_client->service_name));
    rmw_free(rmw_client);
  });

  // 初始化 rmw_client 结构体的成员变量 (Initialize the members of the rmw_client structure)
  rmw_client->implementation_identifier = eprosima_fastrtps_identifier;
  rmw_client->data = info;
  rmw_client->service_name = reinterpret_cast<const char *>(rmw_allocate(strlen(service_name) + 1));
  if (!rmw_client->service_name) {
    RMW_SET_ERROR_MSG("create_client() failed to allocate memory for service name");
    return nullptr;
  }
  memcpy(const_cast<char *>(rmw_client->service_name), service_name, strlen(service_name) + 1);

  // 更新图 (Update graph)
  {
    std::lock_guard<std::mutex> guard(common_context->node_update_mutex);
    rmw_gid_t request_publisher_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        eprosima_fastrtps_identifier, info->request_writer_->guid());
    common_context->graph_cache.associate_writer(
        request_publisher_gid, common_context->gid, node->name, node->namespace_);

    rmw_gid_t response_subscriber_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        eprosima_fastrtps_identifier, info->response_reader_->guid());
    rmw_dds_common::msg::ParticipantEntitiesInfo msg = common_context->graph_cache.associate_reader(
        response_subscriber_gid, common_context->gid, node->name, node->namespace_);
    rmw_ret_t ret = rmw_fastrtps_shared_cpp::__rmw_publish(
        eprosima_fastrtps_identifier, common_context->pub, static_cast<void *>(&msg), nullptr);
    if (RMW_RET_OK != ret) {
      common_context->graph_cache.dissociate_reader(
          response_subscriber_gid, common_context->gid, node->name, node->namespace_);
      common_context->graph_cache.dissociate_writer(
          request_publisher_gid, common_context->gid, node->name, node->namespace_);
      return nullptr;
    }
  }

  // 取消清理操作 (Cancel cleanup actions)
  cleanup_rmw_client.cancel();
  cleanup_datawriter.cancel();
  cleanup_datareader.cancel();
  cleanup_info.cancel();

  // 返回创建的客户端 (Return the created client)
  return rmw_client;
}

/**
 * @brief 销毁客户端 (Destroy a client)
 *
 * @param[in] node 节点指针 (Pointer to the node)
 * @param[in] client 客户端指针 (Pointer to the client)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_destroy_client(rmw_node_t *node, rmw_client_t *client) {
  // 检查节点是否为空，如果为空返回无效参数错误 (Check if the node is null, return an invalid
  // argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  // 检查节点的实现标识符是否与预期相符，如果不符则返回错误的 RMW 实现 (Check if the node's
  // implementation identifier matches the expected one, return incorrect RMW implementation if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查客户端是否为空，如果为空返回无效参数错误 (Check if the client is null, return an invalid
  // argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  // 检查客户端的实现标识符是否与预期相符，如果不符则返回错误的 RMW 实现 (Check if the client's
  // implementation identifier matches the expected one, return incorrect RMW implementation if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      client, client->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 调用底层共享实现销毁客户端 (Call the underlying shared implementation to destroy the client)
  return rmw_fastrtps_shared_cpp::__rmw_destroy_client(eprosima_fastrtps_identifier, node, client);
}

/**
 * @brief 获取客户端请求发布者的实际 QoS 配置 (Get the actual QoS configuration of the client
 * request publisher)
 *
 * @param[in] client 客户端指针 (Pointer to the client)
 * @param[out] qos QoS 配置指针 (Pointer to the QoS configuration)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_client_request_publisher_get_actual_qos(
    const rmw_client_t *client, rmw_qos_profile_t *qos) {
  // 检查客户端是否为空，如果为空返回无效参数错误 (Check if the client is null, return an invalid
  // argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  // 检查客户端的实现标识符是否与预期相符，如果不符则返回错误的 RMW 实现 (Check if the client's
  // implementation identifier matches the expected one, return incorrect RMW implementation if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      client, client->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查 QoS 配置是否为空，如果为空返回无效参数错误 (Check if the QoS configuration is null, return
  // an invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);

  // 调用底层共享实现获取客户端请求发布者的实际 QoS 配置 (Call the underlying shared implementation
  // to get the actual QoS configuration of the client request publisher)
  return rmw_fastrtps_shared_cpp::__rmw_client_request_publisher_get_actual_qos(client, qos);
}

/**
 * @brief 获取客户端响应订阅者的实际 QoS 配置 (Get the actual QoS configuration of the client
 * response subscriber)
 *
 * @param[in] client 客户端指针 (Pointer to the client)
 * @param[out] qos QoS 配置指针 (Pointer to the QoS configuration)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_client_response_subscription_get_actual_qos(
    const rmw_client_t *client, rmw_qos_profile_t *qos) {
  // 检查客户端是否为空，如果为空返回无效参数错误 (Check if the client is null, return an invalid
  // argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  // 检查客户端的实现标识符是否与预期相符，如果不符则返回错误的 RMW 实现 (Check if the client's
  // implementation identifier matches the expected one, return incorrect RMW implementation if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      client, client->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查 QoS 配置是否为空，如果为空返回无效参数错误 (Check if the QoS configuration is null, return
  // an invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);

  // 调用底层共享实现获取客户端响应订阅者的实际 QoS 配置 (Call the underlying shared implementation
  // to get the actual QoS configuration of the client response subscriber)
  return rmw_fastrtps_shared_cpp::__rmw_client_response_subscription_get_actual_qos(client, qos);
}

/**
 * @brief 设置客户端新响应回调函数 (Set the client's new response callback function)
 *
 * @param[in] rmw_client 客户端指针 (Pointer to the client)
 * @param[in] callback 回调函数 (Callback function)
 * @param[in] user_data 用户数据指针 (Pointer to user data)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_client_set_on_new_response_callback(
    rmw_client_t *rmw_client, rmw_event_callback_t callback, const void *user_data) {
  // 检查客户端是否为空，如果为空返回无效参数错误 (Check if the client is null, return an invalid
  // argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(rmw_client, RMW_RET_INVALID_ARGUMENT);

  // 调用底层共享实现设置客户端新响应回调函数 (Call the underlying shared implementation to set the
  // client's new response callback function)
  return rmw_fastrtps_shared_cpp::__rmw_client_set_on_new_response_callback(
      rmw_client, callback, user_data);
}

}  // extern "C"
