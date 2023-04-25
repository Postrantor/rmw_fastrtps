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

#include <algorithm>
#include <array>
#include <cassert>
#include <condition_variable>
#include <limits>
#include <list>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "fastdds/dds/domain/DomainParticipant.hpp"
#include "fastdds/dds/publisher/Publisher.hpp"
#include "fastdds/dds/publisher/qos/DataWriterQos.hpp"
#include "fastdds/dds/subscriber/Subscriber.hpp"
#include "fastdds/dds/subscriber/qos/DataReaderQos.hpp"
#include "fastdds/dds/topic/Topic.hpp"
#include "fastdds/dds/topic/TypeSupport.hpp"
#include "fastdds/dds/topic/qos/TopicQos.hpp"
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
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_service_info.hpp"
#include "rmw_fastrtps_shared_cpp/names.hpp"
#include "rmw_fastrtps_shared_cpp/namespace_prefix.hpp"
#include "rmw_fastrtps_shared_cpp/qos.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"
#include "rmw_fastrtps_shared_cpp/utils.hpp"
#include "type_support_common.hpp"

extern "C" {

/**
 * @brief 创建一个 ROS 服务 (Create a ROS service)
 *
 * @param[in] node 指向要创建服务的节点的指针 (Pointer to the node where the service will be
 * created)
 * @param[in] type_supports 服务类型支持结构体的指针 (Pointer to the service type support structure)
 * @param[in] service_name 要创建的服务的名称 (Name of the service to be created)
 * @param[in] qos_policies 服务的 QoS 策略 (QoS policies for the service)
 * @return 返回一个指向新创建的 rmw_service_t 结构体的指针，如果失败则返回 nullptr (Returns a
 * pointer to the newly created rmw_service_t structure, or nullptr if failed)
 */
rmw_service_t *rmw_create_service(
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
  if (0 == strlen(service_name)) {
    RMW_SET_ERROR_MSG("service_name argument is an empty string");
    return nullptr;
  }
  RMW_CHECK_ARGUMENT_FOR_NULL(qos_policies, nullptr);

  // 如果避免了 ROS 命名空间约定，则验证服务名称 (Validate the service name if ROS namespace
  // conventions are avoided)
  if (!qos_policies->avoid_ros_namespace_conventions) {
    int validation_result = RMW_TOPIC_VALID;
    rmw_ret_t ret = rmw_validate_full_topic_name(service_name, &validation_result, nullptr);
    if (RMW_RET_OK != ret) {
      return nullptr;
    }
    if (RMW_TOPIC_VALID != validation_result) {
      const char *reason = rmw_full_topic_name_validation_result_string(validation_result);
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("service_name argument is invalid: %s", reason);
      return nullptr;
    }
  }

  // 更新服务的 QoS 策略，以便使用最佳可用策略 (Update the QoS policies for the service to use the
  // best available policies)
  rmw_qos_profile_t adapted_qos_policies =
      rmw_dds_common::qos_profile_update_best_available_for_services(*qos_policies);

  /////
  // 检查 RMW QoS (Check RMW QoS)
  if (!is_valid_qos(adapted_qos_policies)) {
    // 如果 QoS 无效，设置错误消息并返回空指针 (If QoS is invalid, set error message and return
    // nullptr)
    RMW_SET_ERROR_MSG("create_service() called with invalid QoS");
    return nullptr;
  }

  /////
  // 获取参与者和子实体 (Get Participant and SubEntities)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);
  auto participant_info =
      static_cast<CustomParticipantInfo *>(node->context->impl->participant_info);

  // 获取 DDS 参与者、发布者和订阅者 (Get DDS Participant, Publisher, and Subscriber)
  eprosima::fastdds::dds::DomainParticipant *dds_participant = participant_info->participant_;
  eprosima::fastdds::dds::Publisher *publisher = participant_info->publisher_;
  eprosima::fastdds::dds::Subscriber *subscriber = participant_info->subscriber_;

  /////
  // 获取 RMW 类型支持 (Get RMW Type Support)
  const rosidl_service_type_support_t *type_support =
      get_service_typesupport_handle(type_supports, RMW_FASTRTPS_CPP_TYPESUPPORT_C);
  if (!type_support) {
    // 如果 C 类型支持不可用，尝试获取 CPP 类型支持 (If C type support is not available, try getting
    // CPP type support)
    rcutils_error_string_t prev_error_string = rcutils_get_error_string();
    rcutils_reset_error();
    type_support = get_service_typesupport_handle(type_supports, RMW_FASTRTPS_CPP_TYPESUPPORT_CPP);
    if (!type_support) {
      // 如果类型支持不是来自此实现，设置错误消息并返回空指针 (If type support is not from this
      // implementation, set error message and return nullptr)
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

  // 锁定实体创建互斥锁以确保线程安全 (Lock the entity creation mutex to ensure thread safety)
  std::lock_guard<std::mutex> lck(participant_info->entity_creation_mutex_);

  // 查找并检查现有的主题和类型 (Find and check existing topics and types)

  // 创建主题和类型名称 (Create Topic and Type names)
  auto service_members = static_cast<const service_type_support_callbacks_t *>(type_support->data);
  auto request_members = static_cast<const message_type_support_callbacks_t *>(
      service_members->request_members_->data);
  auto response_members = static_cast<const message_type_support_callbacks_t *>(
      service_members->response_members_->data);

  std::string request_type_name = _create_type_name(request_members);
  std::string response_type_name = _create_type_name(response_members);

  std::string request_topic_name =
      _create_topic_name(
          &adapted_qos_policies, ros_service_requester_prefix, service_name, "Request")
          .to_string();
  std::string response_topic_name =
      _create_topic_name(&adapted_qos_policies, ros_service_response_prefix, service_name, "Reply")
          .to_string();

  // 获取请求主题和类型 (Get request topic and type)
  eprosima::fastdds::dds::TypeSupport request_fastdds_type;
  eprosima::fastdds::dds::TopicDescription *request_topic_desc = nullptr;
  if (!rmw_fastrtps_shared_cpp::find_and_check_topic_and_type(
          participant_info, request_topic_name, request_type_name, &request_topic_desc,
          &request_fastdds_type)) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "create_service() called for existing request topic name %s with incompatible type %s",
        request_topic_name.c_str(), request_type_name.c_str());
    return nullptr;
  }

  // 获取响应主题和类型 (Get response topic and type)
  eprosima::fastdds::dds::TypeSupport response_fastdds_type;
  eprosima::fastdds::dds::TopicDescription *response_topic_desc = nullptr;
  if (!rmw_fastrtps_shared_cpp::find_and_check_topic_and_type(
          participant_info, response_topic_name, response_type_name, &response_topic_desc,
          &response_fastdds_type)) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "create_service() called for existing response topic name %s with incompatible type %s",
        response_topic_name.c_str(), response_type_name.c_str());
    return nullptr;
  }

  /**
   * @brief 创建自定义服务结构体 (info)
   * @details 为 CustomServiceInfo 分配内存并初始化
   *
   * Create the custom Service struct (info)
   * Allocate memory for CustomServiceInfo and initialize it
   */
  CustomServiceInfo *info = new (std::nothrow) CustomServiceInfo();
  if (!info) {
    // 如果分配失败，设置错误信息
    // If allocation fails, set error message
    RMW_SET_ERROR_MSG("create_service() failed to allocate custom info");
    return nullptr;
  }

  // 创建清理函数，在作用域结束时调用以释放资源
  // Create a cleanup function that will be called when the scope ends to release resources
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
  // Set the typesupport identifier
  info->typesupport_identifier_ = type_support->typesupport_identifier;

  /**
   * @brief 创建 Type Support 结构体
   * @details 初始化请求和响应的类型支持实现
   *
   * Create the Type Support structs
   * Initialize request and response typesupport implementations
   */
  info->request_type_support_impl_ = request_members;
  info->response_type_support_impl_ = response_members;

  if (!request_fastdds_type) {
    auto tsupport = new (std::nothrow) RequestTypeSupport_cpp(service_members);
    if (!tsupport) {
      // 如果分配失败，设置错误信息
      // If allocation fails, set error message
      RMW_SET_ERROR_MSG("create_service() failed to allocate request typesupport");
      return nullptr;
    }

    request_fastdds_type.reset(tsupport);
  }
  if (!response_fastdds_type) {
    auto tsupport = new (std::nothrow) ResponseTypeSupport_cpp(service_members);
    if (!tsupport) {
      // 如果分配失败，设置错误信息
      // If allocation fails, set error message
      RMW_SET_ERROR_MSG("create_service() failed to allocate response typesupport");
      return nullptr;
    }

    response_fastdds_type.reset(tsupport);
  }

  // 注册请求类型
  // Register the request type
  if (ReturnCode_t::RETCODE_OK != request_fastdds_type.register_type(dds_participant)) {
    // 如果注册失败，设置错误信息
    // If registration fails, set error message
    RMW_SET_ERROR_MSG("create_service() failed to register request type");
    return nullptr;
  }
  info->request_type_support_ = request_fastdds_type;

  // 注册响应类型
  // Register the response type
  if (ReturnCode_t::RETCODE_OK != response_fastdds_type.register_type(dds_participant)) {
    // 如果注册失败，设置错误信息
    // If registration fails, set error message
    RMW_SET_ERROR_MSG("create_service() failed to register response type");
    return nullptr;
  }
  info->response_type_support_ = response_fastdds_type;

  // 创建监听器（Create Listeners）
  info->listener_ = new (std::nothrow) ServiceListener(info);
  if (!info->listener_) {
    RMW_SET_ERROR_MSG("create_service() failed to create request subscriber listener");
    return nullptr;
  }

  info->pub_listener_ = new (std::nothrow) ServicePubListener(info);
  if (!info->pub_listener_) {
    RMW_SET_ERROR_MSG("create_service() failed to create response publisher listener");
    return nullptr;
  }

  // 创建并注册主题（Create and register Topics）
  // 为两个主题使用相同的默认主题 QoS（Same default topic QoS for both topics）
  eprosima::fastdds::dds::TopicQos topic_qos = dds_participant->get_default_topic_qos();
  if (!get_topic_qos(adapted_qos_policies, topic_qos)) {
    RMW_SET_ERROR_MSG("create_service() failed setting topic QoS");
    return nullptr;
  }

  // 创建请求主题（Create request topic）
  info->request_topic_ = participant_info->find_or_create_topic(
      request_topic_name, request_type_name, topic_qos, nullptr);
  if (!info->request_topic_) {
    RMW_SET_ERROR_MSG("create_service() failed to create request topic");
    return nullptr;
  }

  request_topic_desc = info->request_topic_;

  // 创建响应主题（Create response topic）
  info->response_topic_ = participant_info->find_or_create_topic(
      response_topic_name, response_type_name, topic_qos, nullptr);
  if (!info->response_topic_) {
    RMW_SET_ERROR_MSG("create_service() failed to create response topic");
    return nullptr;
  }

  // 查找 DataWriter 和 DataReader QoS 的关键字（Keyword to find DataWriter and DataReader QoS）
  const std::string topic_name_fallback = "service";

  /////
  // Create request DataReader

  // 创建请求 DataReader (Create request DataReader)
  // 如果定义了 FASTRTPS_DEFAULT_PROFILES_FILE，则使用基于 _create_topic_name()
  // 定义的主题名称定位的订阅者配置文件填充 DataReader QoS。 如果找不到配置文件，尝试使用
  // profile_name "service" 进行搜索。否则，使用默认的 Fast DDS QoS。 (If
  // FASTRTPS_DEFAULT_PROFILES_FILE defined, fill DataReader QoS with a subscriber profile located
  // based on topic name defined by _create_topic_name(). If no profile is found, a search with
  // profile_name "service" is attempted. Else, use the default Fast DDS QoS.)
  eprosima::fastdds::dds::DataReaderQos reader_qos = subscriber->get_default_datareader_qos();

  // 尝试加载名为 "service" 的配置文件，
  // 如果不存在，则尝试使用请求主题名称
  // 不需要检查返回代码，因为如果任何配置文件都不存在，
  // QoS 已经正确设置：
  // 如果都不存在，则为默认值；如果只存在一个，则选择该配置文件；
  // 如果两个都存在，则选择主题名称
  // (Try to load the profile named "service",
  // if it does not exist it tries with the request topic name
  // It does not need to check the return code, as if any of the profile does not exist,
  // the QoS is already set correctly:
  // If none exist is default, if only one exists is the one chosen,
  // if both exist topic name is chosen)
  subscriber->get_datareader_qos_from_profile(topic_name_fallback, reader_qos);
  subscriber->get_datareader_qos_from_profile(request_topic_name, reader_qos);

  if (!participant_info->leave_middleware_default_qos) {
    reader_qos.endpoint().history_memory_policy =
        eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    reader_qos.data_sharing().off();
  }

  // 获取 DataReader QoS (Get DataReader QoS)
  if (!get_datareader_qos(
          adapted_qos_policies,
          *type_supports->request_typesupport->get_type_hash_func(
              type_supports->request_typesupport),
          reader_qos)) {
    RMW_SET_ERROR_MSG("create_service() failed setting request DataReader QoS");
    return nullptr;
  }

  // 创建 DataReader (Creates DataReader)
  info->request_reader_ = subscriber->create_datareader(
      request_topic_desc, reader_qos, info->listener_,
      eprosima::fastdds::dds::StatusMask::subscription_matched());

  if (!info->request_reader_) {
    RMW_SET_ERROR_MSG("create_service() failed to create request DataReader");
    return nullptr;
  }

  // 设置状态条件 (Set status condition)
  info->request_reader_->get_statuscondition().set_enabled_statuses(
      eprosima::fastdds::dds::StatusMask::data_available());

  // 删除 datareader 的 lambda 函数 (Lambda to delete datareader)
  auto cleanup_datareader = rcpputils::make_scope_exit(
      [subscriber, info]() { subscriber->delete_datareader(info->request_reader_); });

  /////
  // Create response DataWriter

  // 如果定义了 FASTRTPS_DEFAULT_PROFILES_FILE，则根据 _create_topic_name() 定义的主题名称填充
  // DataWriter QoS 的发布者配置文件。 如果找不到配置文件，尝试使用 profile_name "service"
  // 进行搜索。否则，使用 Fast DDS 默认 QoS。 If FASTRTPS_DEFAULT_PROFILES_FILE is defined, fill
  // DataWriter QoS with a publisher profile located based on topic name defined by
  // _create_topic_name(). If no profile is found, a search with profile_name "service" is
  // attempted. Else, use the default Fast DDS QoS.
  eprosima::fastdds::dds::DataWriterQos writer_qos = publisher->get_default_datawriter_qos();

  // 尝试加载名为 "service" 的配置文件，
  // 如果不存在，则尝试使用请求主题名称。
  // 无需检查返回代码，因为如果任何配置文件都不存在，
  // QoS 已经正确设置：
  // 如果都不存在，则为默认值；如果只存在一个，则为所选值；
  // 如果两个都存在，则选择主题名称。
  // Try to load the profile named "service",
  // if it does not exist it tries with the request topic name
  // It does not need to check the return code, as if any of the profile does not exist,
  // the QoS is already set correctly:
  // If none exist is default, if only one exists is the one chosen,
  // if both exist topic name is chosen
  publisher->get_datawriter_qos_from_profile(topic_name_fallback, writer_qos);
  publisher->get_datawriter_qos_from_profile(response_topic_name, writer_qos);

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

  // 获取 DataWriter QoS
  // Get DataWriter QoS
  if (!get_datawriter_qos(
          adapted_qos_policies,
          *type_supports->response_typesupport->get_type_hash_func(
              type_supports->response_typesupport),
          writer_qos)) {
    RMW_SET_ERROR_MSG("create_service() failed setting response DataWriter QoS");
    return nullptr;
  }

  // 使用启用监听器的 publication_matched 调用的掩码创建 DataWriter
  // Creates DataWriter with a mask enabling publication_matched calls for the listener
  info->response_writer_ = publisher->create_datawriter(
      info->response_topic_, writer_qos, info->pub_listener_,
      eprosima::fastdds::dds::StatusMask::publication_matched());

  // 创建响应 DataWriter 失败
  // Failed to create response DataWriter
  if (!info->response_writer_) {
    RMW_SET_ERROR_MSG("create_service() failed to create response DataWriter");
    return nullptr;
  }

  // 将 StatusCondition 设置为 none，以防止通过 WaitSets 触发
  // Set the StatusCondition to none to prevent triggering via WaitSets
  info->response_writer_->get_statuscondition().set_enabled_statuses(
      eprosima::fastdds::dds::StatusMask::none());

  // lambda 删除 datawriter
  // lambda to delete datawriter
  auto cleanup_datawriter = rcpputils::make_scope_exit(
      [publisher, info]() { publisher->delete_datawriter(info->response_writer_); });

  /////
  // Create Service

  /*!
   * \brief
   * 为以下代码段添加参数列表的说明，并以双语（中文、英文）的形式对代码块中的每一行都尽可能的添加详细的注释。
   * \param[in] request_topic_name 请求主题名称 (Request topic name)
   * \param[in] response_topic_name 响应主题名称 (Response topic name)
   * \param[in] service_name 服务名称 (Service name)
   * \param[in] node 节点指针 (Node pointer)
   * \param[in] common_context 共享上下文指针 (Shared context pointer)
   * \return rmw_service 返回创建的服务对象指针 (Return the created service object pointer)
   */
  // Debug info
  RCUTILS_LOG_DEBUG_NAMED("rmw_fastrtps_cpp", "************ Service Details *********");
  // 打印服务详情标题 (Print service details title)
  RCUTILS_LOG_DEBUG_NAMED("rmw_fastrtps_cpp", "Sub Topic %s", request_topic_name.c_str());
  // 打印请求主题名称 (Print request topic name)
  RCUTILS_LOG_DEBUG_NAMED("rmw_fastrtps_cpp", "Pub Topic %s", response_topic_name.c_str());
  // 打印响应主题名称 (Print response topic name)
  RCUTILS_LOG_DEBUG_NAMED("rmw_fastrtps_cpp", "***********");
  // 打印分隔线 (Print separator line)

  rmw_service_t *rmw_service = rmw_service_allocate();
  // 分配内存给 rmw_service 对象 (Allocate memory for rmw_service object)
  if (!rmw_service) {
    RMW_SET_ERROR_MSG("create_service() failed to allocate memory for rmw_service");
    // 设置错误消息 (Set error message)
    return nullptr;
  }
  auto cleanup_rmw_service = rcpputils::make_scope_exit([rmw_service]() {
    rmw_free(const_cast<char *>(rmw_service->service_name));
    rmw_free(rmw_service);
  });
  // 创建清理作用域，确保在退出时释放资源 (Create cleanup scope to ensure resources are released on
  // exit)

  rmw_service->implementation_identifier = eprosima_fastrtps_identifier;
  // 设置实现标识符 (Set implementation identifier)
  rmw_service->data = info;
  // 设置服务数据 (Set service data)
  rmw_service->service_name =
      reinterpret_cast<const char *>(rmw_allocate(strlen(service_name) + 1));
  // 分配内存给服务名称 (Allocate memory for service name)
  if (!rmw_service->service_name) {
    RMW_SET_ERROR_MSG("create_service() failed to allocate memory for service name");
    // 设置错误消息 (Set error message)
    return nullptr;
  }
  memcpy(const_cast<char *>(rmw_service->service_name), service_name, strlen(service_name) + 1);
  // 复制服务名称到 rmw_service 对象中 (Copy service name into rmw_service object)

  {
    // Update graph
    std::lock_guard<std::mutex> guard(common_context->node_update_mutex);
    // 加锁以更新图 (Lock to update graph)
    rmw_gid_t request_subscriber_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        eprosima_fastrtps_identifier, info->request_reader_->guid());
    // 创建请求订阅者的 GID (Create GID for request subscriber)
    common_context->graph_cache.associate_reader(
        request_subscriber_gid, common_context->gid, node->name, node->namespace_);
    // 关联请求订阅者到图缓存 (Associate request subscriber to graph cache)
    rmw_gid_t response_publisher_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        eprosima_fastrtps_identifier, info->response_writer_->guid());
    // 创建响应发布者的 GID (Create GID for response publisher)
    rmw_dds_common::msg::ParticipantEntitiesInfo msg = common_context->graph_cache.associate_writer(
        response_publisher_gid, common_context->gid, node->name, node->namespace_);
    // 关联响应发布者到图缓存 (Associate response publisher to graph cache)
    rmw_ret_t ret = rmw_fastrtps_shared_cpp::__rmw_publish(
        eprosima_fastrtps_identifier, common_context->pub, static_cast<void *>(&msg), nullptr);
    // 发布参与者实体信息消息 (Publish ParticipantEntitiesInfo message)
    if (RMW_RET_OK != ret) {
      common_context->graph_cache.dissociate_writer(
          response_publisher_gid, common_context->gid, node->name, node->namespace_);
      // 解除关联响应发布者 (Dissociate response publisher)
      common_context->graph_cache.dissociate_reader(
          request_subscriber_gid, common_context->gid, node->name, node->namespace_);
      // 解除关联请求订阅者 (Dissociate request subscriber)
      return nullptr;
    }
  }

  // 取消清理作用域 (Cancel cleanup scope)
  cleanup_rmw_service.cancel();
  // 取消数据写入器清理作用域 (Cancel data writer cleanup scope)
  cleanup_datawriter.cancel();
  // 取消数据读取器清理作用域 (Cancel data reader cleanup scope)
  cleanup_datareader.cancel();
  // 取消信息清理作用域 (Cancel info cleanup scope)
  cleanup_info.cancel();
  // 返回创建的服务对象指针 (Return the created service object pointer)
  return rmw_service;
}

/**
 * @brief 销毁一个服务 (Destroy a service)
 *
 * @param[in] node 指向要销毁的服务所属节点的指针 (Pointer to the node that owns the service to be
 * destroyed)
 * @param[in] service 要销毁的服务的指针 (Pointer to the service to be destroyed)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_destroy_service(rmw_node_t *node, rmw_service_t *service) {
  // 检查 node 参数是否为空 (Check if the node argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  // 检查 node 的实现标识符是否与 eprosima_fastrtps_identifier 匹配 (Check if the node's
  // implementation identifier matches eprosima_fastrtps_identifier)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查 service 参数是否为空 (Check if the service argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(service, RMW_RET_INVALID_ARGUMENT);
  // 检查 service 的实现标识符是否与 eprosima_fastrtps_identifier 匹配 (Check if the service's
  // implementation identifier matches eprosima_fastrtps_identifier)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      service, service->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 调用共享函数来销毁服务 (Call the shared function to destroy the service)
  return rmw_fastrtps_shared_cpp::__rmw_destroy_service(
      eprosima_fastrtps_identifier, node, service);
}

/**
 * @brief 获取服务响应发布者的实际 QoS (Get the actual QoS of the service response publisher)
 *
 * @param[in] service 指向要查询的服务的指针 (Pointer to the service to query)
 * @param[out] qos 存储查询到的 QoS 的指针 (Pointer to store the queried QoS)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_service_response_publisher_get_actual_qos(
    const rmw_service_t *service, rmw_qos_profile_t *qos) {
  // 检查 service 参数是否为空 (Check if the service argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(service, RMW_RET_INVALID_ARGUMENT);
  // 检查 service 的实现标识符是否与 eprosima_fastrtps_identifier 匹配 (Check if the service's
  // implementation identifier matches eprosima_fastrtps_identifier)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      service, service->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查 qos 参数是否为空 (Check if the qos argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);

  // 调用共享函数来获取服务响应发布者的实际 QoS (Call the shared function to get the actual QoS of
  // the service response publisher)
  return rmw_fastrtps_shared_cpp::__rmw_service_response_publisher_get_actual_qos(service, qos);
}

/**
 * @brief 获取服务请求订阅者的实际 QoS (Get the actual QoS of the service request subscriber)
 *
 * @param[in] service 指向要查询的服务的指针 (Pointer to the service to query)
 * @param[out] qos 存储查询到的 QoS 的指针 (Pointer to store the queried QoS)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_service_request_subscription_get_actual_qos(
    const rmw_service_t *service, rmw_qos_profile_t *qos) {
  // 检查 service 参数是否为空 (Check if the service argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(service, RMW_RET_INVALID_ARGUMENT);
  // 检查 service 的实现标识符是否与 eprosima_fastrtps_identifier 匹配 (Check if the service's
  // implementation identifier matches eprosima_fastrtps_identifier)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      service, service->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查 qos 参数是否为空 (Check if the qos argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);

  // 调用共享函数来获取服务请求订阅者的实际 QoS (Call the shared function to get the actual QoS of
  // the service request subscriber)
  return rmw_fastrtps_shared_cpp::__rmw_service_request_subscription_get_actual_qos(service, qos);
}

/**
 * @brief 设置新请求回调函数 (Set the new request callback function)
 *
 * @param[in] rmw_service 指向要设置回调的服务的指针 (Pointer to the service to set the callback)
 * @param[in] callback 要设置的回调函数 (The callback function to set)
 * @param[in] user_data 用户数据，将传递给回调函数 (User data that will be passed to the callback
 * function)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_service_set_on_new_request_callback(
    rmw_service_t *rmw_service, rmw_event_callback_t callback, const void *user_data) {
  // 检查 rmw_service 参数是否为空 (Check if the rmw_service argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(rmw_service, RMW_RET_INVALID_ARGUMENT);

  // 调用共享函数来设置新请求回调函数 (Call the shared function to set the new request callback
  // function)
  return rmw_fastrtps_shared_cpp::__rmw_service_set_on_new_request_callback(
      rmw_service, callback, user_data);
}

}  // extern "C"
