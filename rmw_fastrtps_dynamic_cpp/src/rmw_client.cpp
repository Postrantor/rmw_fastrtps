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

#include "client_service_common.hpp"
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
#include "rcutils/logging_macros.h"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw/validate_full_topic_name.h"
#include "rmw_dds_common/qos.hpp"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/custom_client_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/names.hpp"
#include "rmw_fastrtps_shared_cpp/namespace_prefix.hpp"
#include "rmw_fastrtps_shared_cpp/qos.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"
#include "rmw_fastrtps_shared_cpp/utils.hpp"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "type_support_common.hpp"
#include "type_support_registry.hpp"

extern "C" {

/**
 * @brief 创建一个 ROS2 服务客户端 (Create a ROS2 service client)
 *
 * @param[in] node 指向要创建客户端的节点的指针 (Pointer to the node for which the client is to be
 * created)
 * @param[in] type_supports 服务类型支持结构体的指针 (Pointer to the service type support structure)
 * @param[in] service_name 要连接的服务的名称 (Name of the service to connect to)
 * @param[in] qos_policies 指向 QoS 策略的指针 (Pointer to the QoS policies)
 * @return rmw_client_t* 成功时返回指向新创建的客户端的指针，失败时返回 nullptr (On success, returns
 * a pointer to the newly created client, otherwise returns nullptr)
 */
rmw_client_t *rmw_create_client(
    const rmw_node_t *node,
    const rosidl_service_type_support_t *type_supports,
    const char *service_name,
    const rmw_qos_profile_t *qos_policies) {
  /////
  // 检查输入参数 (Check input parameters)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, nullptr);
  // 检查节点的实现标识符是否与预期相匹配 (Check if the node's implementation identifier matches the
  // expected one)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, eprosima_fastrtps_identifier, return nullptr);
  // 检查类型支持参数是否为空 (Check if the type_supports argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(type_supports, nullptr);
  // 检查服务名称参数是否为空 (Check if the service_name argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(service_name, nullptr);
  // 如果服务名称长度为0，则设置错误消息并返回空指针 (If the service_name length is 0, set an error
  // message and return a null pointer)
  if (0 == strlen(service_name)) {
    RMW_SET_ERROR_MSG("service_name argument is an empty string");
    return nullptr;
  }
  // 检查 qos_policies 参数是否为空 (Check if the qos_policies argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos_policies, nullptr);
  // 如果避免 ROS 命名空间约定为 false，则验证完整主题名称 (If avoid_ros_namespace_conventions is
  // false, validate the full topic name)
  if (!qos_policies->avoid_ros_namespace_conventions) {
    int validation_result = RMW_TOPIC_VALID;
    rmw_ret_t ret = rmw_validate_full_topic_name(service_name, &validation_result, nullptr);
    // 如果验证结果不是 RMW_RET_OK，则返回空指针 (If the validation result is not RMW_RET_OK, return
    // a null pointer)
    if (RMW_RET_OK != ret) {
      return nullptr;
    }
    // 如果主题验证结果不是 RMW_TOPIC_VALID，则设置错误消息并返回空指针 (If the topic validation
    // result is not RMW_TOPIC_VALID, set an error message and return a null pointer)
    if (RMW_TOPIC_VALID != validation_result) {
      const char *reason = rmw_full_topic_name_validation_result_string(validation_result);
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("service_name argument is invalid: %s", reason);
      return nullptr;
    }
  }

  // 更新服务的 QoS 策略为最佳可用策略 (Update the QoS policies for services to the best available
  // policies)
  rmw_qos_profile_t adapted_qos_policies =
      rmw_dds_common::qos_profile_update_best_available_for_services(*qos_policies);

  /////
  /**
   * @brief 检查 RMW QoS 是否有效，并在无效时设置错误信息。
   * @brief Check if the RMW QoS is valid, and set an error message if it's not.
   *
   * @param[in] adapted_qos_policies 经过适配的 QoS 策略。
   * @param[in] adapted_qos_policies The adapted QoS policies.
   *
   * @return 如果 QoS 有效，则返回 true；否则返回 false。
   * @return Returns true if the QoS is valid; otherwise, returns false.
   */
  if (!is_valid_qos(adapted_qos_policies)) {
    // 如果 QoS 不合法，设置错误信息并返回空指针
    // If the QoS is invalid, set the error message and return a null pointer
    RMW_SET_ERROR_MSG("create_client() called with invalid QoS");
    return nullptr;
  }

  /////
  /**
   * @brief 获取参与者和子实体 (Get Participant and SubEntities)
   *
   * @param[in] node 节点指针，用于获取上下文信息 (Pointer to the node, used to get context
   * information)
   */
  // 获取通用上下文 (Get common context)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);
  // 获取参与者信息 (Get participant information)
  auto participant_info =
      static_cast<CustomParticipantInfo *>(node->context->impl->participant_info);

  // 获取DDS参与者、发布者、订阅者 (Get DDS participant\publisher\subscriber)
  eprosima::fastdds::dds::DomainParticipant *dds_participant = participant_info->participant_;
  eprosima::fastdds::dds::Publisher *publisher = participant_info->publisher_;
  eprosima::fastdds::dds::Subscriber *subscriber = participant_info->subscriber_;

  /////
  // 获取 RMW 类型支持 (Get RMW Type Support)
  const rosidl_service_type_support_t *type_support =
      // 从类型支持列表中获取服务类型支持句柄，使用 rosidl_typesupport_introspection_c__identifier
      // 作为标识符 (Get the service type support handle from the type supports list, using
      // rosidl_typesupport_introspection_c__identifier as the identifier)
      get_service_typesupport_handle(type_supports, rosidl_typesupport_introspection_c__identifier);
  if (!type_support) {
    // 获取之前的错误字符串 (Get the previous error string)
    rcutils_error_string_t prev_error_string = rcutils_get_error_string();
    // 重置错误 (Reset the error)
    rcutils_reset_error();
    // 使用 rosidl_typesupport_introspection_cpp::typesupport_identifier
    // 作为标识符重新尝试获取服务类型支持句柄 (Retry getting the service type support handle with
    // rosidl_typesupport_introspection_cpp::typesupport_identifier as the identifier)
    type_support = get_service_typesupport_handle(
        type_supports, rosidl_typesupport_introspection_cpp::typesupport_identifier);
    if (!type_support) {
      // 获取当前错误字符串 (Get the current error string)
      rcutils_error_string_t error_string = rcutils_get_error_string();
      // 重置错误 (Reset the error)
      rcutils_reset_error();
      // 设置错误消息，包括之前和当前的错误字符串 (Set the error message, including the previous and
      // current error strings)
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "Type support not from this implementation. Got:\n"
          "    %s\n"
          "    %s\n"
          "while fetching it",
          prev_error_string.str, error_string.str);
      // 返回空指针 (Return nullptr)
      return nullptr;
    }
  }

  // 使用 std::lock_guard 对象锁定 participant_info 的实体创建互斥锁，以确保线程安全
  // (Lock the entity_creation_mutex_ of participant_info using a std::lock_guard object to ensure
  // thread safety)
  std::lock_guard<std::mutex> lck(participant_info->entity_creation_mutex_);

  /////
  /**
   * @brief 查找并检查现有的主题和类型 (Find and check existing topics and types)
   */

  // 创建主题和类型名称 (Create Topic and Type names)
  const void *untyped_request_members;
  const void *untyped_response_members;

  // 获取请求成员指针 (Get the pointer to request members)
  untyped_request_members =
      get_request_ptr(type_support->data, type_support->typesupport_identifier);
  // 获取响应成员指针 (Get the pointer to response members)
  untyped_response_members =
      get_response_ptr(type_support->data, type_support->typesupport_identifier);

  // 创建请求类型名称 (Create request type name)
  std::string request_type_name =
      _create_type_name(untyped_request_members, type_support->typesupport_identifier);
  // 创建响应类型名称 (Create response type name)
  std::string response_type_name =
      _create_type_name(untyped_response_members, type_support->typesupport_identifier);

  // 创建响应主题名称 (Create response topic name)
  std::string response_topic_name =
      _create_topic_name(&adapted_qos_policies, ros_service_response_prefix, service_name, "Reply")
          .to_string();
  // 创建请求主题名称 (Create request topic name)
  std::string request_topic_name =
      _create_topic_name(
          &adapted_qos_policies, ros_service_requester_prefix, service_name, "Request")
          .to_string();

  // 获取请求主题和类型 (Get request topic and type)
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

  // 获取响应主题和类型 (Get response topic and type)
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

  /////
  // 创建自定义客户端结构体 (info)
  // Create the custom Client struct (info)
  CustomClientInfo *info = new (std::nothrow) CustomClientInfo();
  if (!info) {
    // 如果分配失败，设置错误信息
    // If allocation fails, set error message
    RMW_SET_ERROR_MSG("create_client() failed to allocate custom info");
    return nullptr;
  }

  // 创建一个作用域退出对象，用于在函数返回时自动清理资源
  // Create a scope exit object to automatically clean up resources when the function returns
  auto cleanup_info = rcpputils::make_scope_exit([info, participant_info]() {
    // 移除响应主题和类型
    // Remove response topic and type
    rmw_fastrtps_shared_cpp::remove_topic_and_type(
        participant_info, nullptr, info->response_topic_, info->response_type_support_);
    // 移除请求主题和类型
    // Remove request topic and type
    rmw_fastrtps_shared_cpp::remove_topic_and_type(
        participant_info, nullptr, info->request_topic_, info->request_type_support_);
    // 删除发布者监听器
    // Delete publisher listener
    delete info->pub_listener_;
    // 删除订阅者监听器
    // Delete subscriber listener
    delete info->listener_;
    // 删除 info 对象
    // Delete info object
    delete info;
  });

  // 设置类型支持标识符
  // Set typesupport identifier
  info->typesupport_identifier_ = type_support->typesupport_identifier;
  // 初始化请求发布者匹配计数为0
  // Initialize request publisher matched count to 0
  info->request_publisher_matched_count_ = 0;
  // 初始化响应订阅者匹配计数为0
  // Initialize response subscriber matched count to 0
  info->response_subscriber_matched_count_ = 0;

  /////
  // 创建类型支持结构 (Create the Type Support structs)
  TypeSupportRegistry &type_registry = TypeSupportRegistry::get_instance();

  // 获取请求类型支持实现 (Get the request type support implementation)
  auto request_type_impl = type_registry.get_request_type_support(type_support);
  if (!request_type_impl) {
    // 设置错误消息 (Set error message)
    RMW_SET_ERROR_MSG("create_client() failed to get request_type_support");
    return nullptr;
  }

  // 返回请求类型支持并在作用域结束时执行 (Return request type support and execute when scope ends)
  auto return_request_type_support = rcpputils::make_scope_exit([&type_registry, type_support]() {
    type_registry.return_request_type_support(type_support);
  });

  // 获取响应类型支持实现 (Get the response type support implementation)
  auto response_type_impl = type_registry.get_response_type_support(type_support);
  if (!response_type_impl) {
    // 设置错误消息 (Set error message)
    RMW_SET_ERROR_MSG("create_client() failed to allocate response type support");
    return nullptr;
  }

  // 返回响应类型支持并在作用域结束时执行 (Return response type support and execute when scope ends)
  auto return_response_type_support = rcpputils::make_scope_exit([&type_registry, type_support]() {
    type_registry.return_response_type_support(type_support);
  });

  // 设置请求和响应类型支持实现 (Set request and response type support implementations)
  info->request_type_support_impl_ = request_type_impl;
  info->response_type_support_impl_ = response_type_impl;

  // 如果没有请求的 Fast-DDS 类型 (If there is no request Fast-DDS type)
  if (!request_fastdds_type) {
    // 创建一个新的类型支持代理 (Create a new Type Support Proxy)
    auto tsupport =
        new (std::nothrow) rmw_fastrtps_dynamic_cpp::TypeSupportProxy(request_type_impl);
    if (!tsupport) {
      // 设置错误消息 (Set error message)
      RMW_SET_ERROR_MSG("create_client() failed to allocate request TypeSupportProxy");
      return nullptr;
    }

    // 重置请求的 Fast-DDS 类型 (Reset the request Fast-DDS type)
    request_fastdds_type.reset(tsupport);
  }

  // 如果没有响应的 Fast-DDS 类型 (If there is no response Fast-DDS type)
  if (!response_fastdds_type) {
    // 创建一个新的类型支持代理 (Create a new Type Support Proxy)
    auto tsupport =
        new (std::nothrow) rmw_fastrtps_dynamic_cpp::TypeSupportProxy(response_type_impl);
    if (!tsupport) {
      // 设置错误消息 (Set error message)
      RMW_SET_ERROR_MSG("create_client() failed to allocate response TypeSupportProxy");
      return nullptr;
    }

    // 重置响应的 Fast-DDS 类型 (Reset the response Fast-DDS type)
    response_fastdds_type.reset(tsupport);
  }

  // 注册请求类型 (Register the request type)
  if (ReturnCode_t::RETCODE_OK != request_fastdds_type.register_type(dds_participant)) {
    // 设置错误消息 (Set error message)
    RMW_SET_ERROR_MSG("create_client() failed to register request type");
    return nullptr;
  }
  info->request_type_support_ = request_fastdds_type;

  // 注册响应类型 (Register the response type)
  if (ReturnCode_t::RETCODE_OK != response_fastdds_type.register_type(dds_participant)) {
    // 设置错误消息 (Set error message)
    RMW_SET_ERROR_MSG("create_client() failed to register response type");
    return nullptr;
  }
  info->response_type_support_ = response_fastdds_type;

  /////
  /**
   * @brief 创建监听器 (Create Listeners)
   * @param info 传入的客户端信息结构体指针 (Pointer to the client information structure)
   * @return nullptr 如果创建监听器失败，否则返回创建成功的监听器指针 (nullptr if the creation of
   * listeners fails, otherwise returns the successfully created listener pointer)
   */

  // 创建 ClientListener 对象，并将其分配给 info->listener_ 指针
  // Create a ClientListener object and assign it to the info->listener_ pointer
  info->listener_ = new (std::nothrow) ClientListener(info);
  if (!info->listener_) {
    // 如果创建 ClientListener 失败，设置错误消息并返回 nullptr
    // If the creation of ClientListener fails, set the error message and return nullptr
    RMW_SET_ERROR_MSG("create_client() failed to create response subscriber listener");
    return nullptr;
  }

  // 创建 ClientPubListener 对象，并将其分配给 info->pub_listener_ 指针
  // Create a ClientPubListener object and assign it to the info->pub_listener_ pointer
  info->pub_listener_ = new (std::nothrow) ClientPubListener(info);
  if (!info->pub_listener_) {
    // 如果创建 ClientPubListener 失败，设置错误消息并返回 nullptr
    // If the creation of ClientPubListener fails, set the error message and return nullptr
    RMW_SET_ERROR_MSG("create_client() failed to create request publisher listener");
    return nullptr;
  }

  /////
  // 创建并注册主题 (Create and register Topics)
  // 为两个主题使用相同的默认主题 QoS (Same default topic QoS for both topics)
  eprosima::fastdds::dds::TopicQos topic_qos = dds_participant->get_default_topic_qos();
  // 获取主题 QoS，如果失败则设置错误消息并返回空指针 (Get topic QoS, if failed set error message
  // and return nullptr)
  if (!get_topic_qos(adapted_qos_policies, topic_qos)) {
    RMW_SET_ERROR_MSG("create_client() failed setting topic QoS");
    return nullptr;
  }

  // 创建响应主题 (Create response topic)
  info->response_topic_ = participant_info->find_or_create_topic(
      response_topic_name, response_type_name, topic_qos, nullptr);
  // 如果响应主题创建失败，则设置错误消息并返回空指针 (If creating response topic fails, set error
  // message and return nullptr)
  if (!info->response_topic_) {
    RMW_SET_ERROR_MSG("create_client() failed to create response topic");
    return nullptr;
  }

  // 设置响应主题描述 (Set response topic description)
  response_topic_desc = info->response_topic_;

  // 创建请求主题 (Create request topic)
  info->request_topic_ = participant_info->find_or_create_topic(
      request_topic_name, request_type_name, topic_qos, nullptr);
  // 如果请求主题创建失败，则设置错误消息并返回空指针 (If creating request topic fails, set error
  // message and return nullptr)
  if (!info->request_topic_) {
    RMW_SET_ERROR_MSG("create_client() failed to create request topic");
    return nullptr;
  }

  // 设置请求和响应主题名称 (Set request and response topic names)
  info->request_topic_name_ = request_topic_name;
  info->response_topic_name_ = response_topic_name;

  // 查找 DataWriter 和 DataReader QoS 的关键字 (Keyword to find DataWriter and DataReader QoS)
  const std::string topic_name_fallback = "client";

  /////
  // 创建响应 DataReader
  // Create response DataReader

  // 如果定义了 FASTRTPS_DEFAULT_PROFILES_FILE，则使用 _create_topic_name() 定义的主题名称
  // 填充带有订阅者配置文件的 DataReader QoS。如果找不到配置文件，尝试使用 profile_name "client"
  // 进行搜索。 否则，使用 Fast DDS 默认 QoS。 If FASTRTPS_DEFAULT_PROFILES_FILE defined, fill
  // DataReader QoS with a subscriber profile located based on topic name defined by
  // _create_topic_name(). If no profile is found, a search with profile_name "client" is attempted.
  // Else, use the default Fast DDS QoS.
  eprosima::fastdds::dds::DataReaderQos reader_qos = subscriber->get_default_datareader_qos();

  // 尝试加载名为 "client" 的配置文件，
  // 如果不存在，则尝试使用响应主题名称
  // 无需检查返回代码，因为如果任何配置文件都不存在，
  // QoS 已经正确设置：
  // 如果都不存在，则为默认值；如果只存在一个，则为所选值；
  // 如果两个都存在，则选择主题名称
  // Try to load the profile named "client",
  // if it does not exist it tries with the response topic name
  // It does not need to check the return code, as if any of the profile does not exist,
  // the QoS is already set correctly:
  // If none exist is default, if only one exists is the one chosen,
  // if both exist topic name is chosen
  subscriber->get_datareader_qos_from_profile(topic_name_fallback, reader_qos);
  subscriber->get_datareader_qos_from_profile(response_topic_name, reader_qos);

  // 如果不使用中间件默认 QoS
  // If not using middleware default QoS
  if (!participant_info->leave_middleware_default_qos) {
    // 设置历史记录内存策略为预分配并在需要时重新分配内存模式
    // Set history memory policy to preallocated with realloc memory mode
    reader_qos.endpoint().history_memory_policy =
        eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    // 关闭数据共享
    // Turn off data sharing
    reader_qos.data_sharing().off();
  }

  // 获取 DataReader QoS，如果失败，则设置错误消息并返回 nullptr
  // Get DataReader QoS, if failed, set error message and return nullptr
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

  // 如果创建响应 DataReader 失败，则设置错误消息并返回 nullptr
  // If failed to create response DataReader, set error message and return nullptr
  if (!info->response_reader_) {
    RMW_SET_ERROR_MSG("create_client() failed to create response DataReader");
    return nullptr;
  }

  // 设置状态条件的启用状态为 data_available
  // Set enabled statuses of the status condition to data_available
  info->response_reader_->get_statuscondition().set_enabled_statuses(
      eprosima::fastdds::dds::StatusMask::data_available());

  // lambda 函数用于删除 datareader
  // Lambda function to delete datareader
  auto cleanup_datareader = rcpputils::make_scope_exit(
      [subscriber, info]() { subscriber->delete_datareader(info->response_reader_); });

  /**
   * @brief 如果定义了 FASTRTPS_DEFAULT_PROFILES_FILE，则根据 _create_topic_name()
   * 定义的主题名称填充 DataWriter QoS 的发布者配置文件。 如果找不到配置文件，尝试使用 profile_name
   * "client" 进行搜索。否则，使用 Fast DDS 默认 QoS。 (If FASTRTPS_DEFAULT_PROFILES_FILE is
   * defined, fill DataWriter QoS with a publisher profile located based on topic name defined by
   * _create_topic_name(). If no profile is found, a search with profile_name "client" is attempted.
   * Else, use the default Fast DDS QoS.)
   */
  eprosima::fastdds::dds::DataWriterQos writer_qos = publisher->get_default_datawriter_qos();

  /**
   * @brief 尝试加载名为 "client" 的配置文件，
   *        如果不存在，则尝试使用请求主题名称。
   *        无需检查返回代码，因为如果任何配置文件都不存在，
   *        QoS 已经正确设置：如果都不存在则为默认值，如果只存在一个则为所选值，
   *        如果两个都存在，则选择主题名称。
   *        (Try to load the profile named "client",
   *        if it does not exist it tries with the request topic name.
   *        It does not need to check the return code, as if any of the profile does not exist,
   *        the QoS is already set correctly:
   *        If none exist is default, if only one exists is the one chosen,
   *        if both exist topic name is chosen.)
   */
  publisher->get_datawriter_qos_from_profile(topic_name_fallback, writer_qos);
  publisher->get_datawriter_qos_from_profile(request_topic_name, writer_qos);

  /**
   * @brief 修改特定的 DataWriter Qos
   *        (Modify specific DataWriter Qos)
   */
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

  /**
   * @brief 如果无法获取 DataWriter QoS，则设置错误消息并返回 nullptr。
   *        (If unable to get DataWriter QoS, set error message and return nullptr.)
   */
  if (!get_datawriter_qos(
          adapted_qos_policies,
          *type_supports->request_typesupport->get_type_hash_func(
              type_supports->request_typesupport),
          writer_qos)) {
    RMW_SET_ERROR_MSG("create_client() failed setting request DataWriter QoS");
    return nullptr;
  }

  // 创建 DataWriter（Creates DataWriter）
  info->request_writer_ = publisher->create_datawriter(
      // 请求主题（Request topic）
      info->request_topic_,
      // 写入者质量参数（Writer QoS settings）
      writer_qos,
      // 发布者监听器（Publisher listener）
      info->pub_listener_,
      // 状态掩码，用于匹配发布者（Status mask for publication matching）
      eprosima::fastdds::dds::StatusMask::publication_matched());

  // 如果 DataWriter 创建失败（If the DataWriter creation fails）
  if (!info->request_writer_) {
    // 设置错误信息（Set error message）
    RMW_SET_ERROR_MSG("create_client() failed to create request DataWriter");
    // 返回空指针（Return nullptr）
    return nullptr;
  }

  // 获取状态条件并设置启用的状态为无（Get status condition and set enabled statuses to none）
  info->request_writer_->get_statuscondition().set_enabled_statuses(
      eprosima::fastdds::dds::StatusMask::none());

  // 使用 lambda 函数删除 DataWriter（Lambda function to delete DataWriter）
  auto cleanup_datawriter = rcpputils::make_scope_exit(
      // 删除 DataWriter 的操作（Operation to delete the DataWriter）
      [publisher, info]() { publisher->delete_datawriter(info->request_writer_); });

  /////
  /**
   * @brief 创建一个客户端 (Create a client)
   *
   * @param[in] response_topic_name 响应主题名称 (Response topic name)
   * @param[in] request_topic_name 请求主题名称 (Request topic name)
   * @param[in] service_name 服务名称 (Service name)
   * @param[in] node 节点指针 (Pointer to the node)
   * @param[in] common_context 共享上下文 (Shared context)
   * @return rmw_client_t* 创建的客户端指针 (Pointer to the created client)
   */
  // 创建客户端 (Create client)
  RCUTILS_LOG_DEBUG_NAMED("rmw_fastrtps_dynamic_cpp", "************ Client Details *********");
  RCUTILS_LOG_DEBUG_NAMED("rmw_fastrtps_dynamic_cpp", "Sub Topic %s", response_topic_name.c_str());
  RCUTILS_LOG_DEBUG_NAMED("rmw_fastrtps_dynamic_cpp", "Pub Topic %s", request_topic_name.c_str());
  RCUTILS_LOG_DEBUG_NAMED("rmw_fastrtps_dynamic_cpp", "***********");

  // 获取写入器和读取器的 GUID (Get the GUID of the writer and reader)
  info->writer_guid_ = info->request_writer_->guid();
  info->reader_guid_ = info->response_reader_->guid();

  // 为 rmw_client 分配内存 (Allocate memory for rmw_client)
  rmw_client_t *rmw_client = rmw_client_allocate();
  if (!rmw_client) {
    RMW_SET_ERROR_MSG("create_client() failed to allocate memory for rmw_client");
    return nullptr;
  }
  auto cleanup_rmw_client = rcpputils::make_scope_exit([rmw_client]() {
    rmw_free(const_cast<char *>(rmw_client->service_name));
    rmw_free(rmw_client);
  });

  // 设置客户端实现标识符和数据 (Set the client implementation identifier and data)
  rmw_client->implementation_identifier = eprosima_fastrtps_identifier;
  rmw_client->data = info;

  // 为服务名称分配内存并复制 (Allocate memory for service name and copy)
  rmw_client->service_name = reinterpret_cast<const char *>(rmw_allocate(strlen(service_name) + 1));
  if (!rmw_client->service_name) {
    RMW_SET_ERROR_MSG("create_client() failed to allocate memory for service name");
    return nullptr;
  }
  memcpy(const_cast<char *>(rmw_client->service_name), service_name, strlen(service_name) + 1);

  {
    // 更新图 (Update graph)
    std::lock_guard<std::mutex> guard(common_context->node_update_mutex);
    rmw_gid_t request_publisher_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        eprosima_fastrtps_identifier, info->request_writer_->guid());
    common_context->graph_cache.associate_writer(
        request_publisher_gid, common_context->gid, node->name, node->namespace_);

    rmw_gid_t response_subscriber_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        eprosima_fastrtps_identifier, info->response_reader_->guid());
    rmw_dds_common::msg::ParticipantEntitiesInfo msg = common_context->graph_cache.associate_reader(
        response_subscriber_gid, common_context->gid, node->name, node->namespace_);
    rmw_ret_t rmw_ret = rmw_fastrtps_shared_cpp::__rmw_publish(
        eprosima_fastrtps_identifier, common_context->pub, static_cast<void *>(&msg), nullptr);
    if (RMW_RET_OK != rmw_ret) {
      common_context->graph_cache.dissociate_reader(
          response_subscriber_gid, common_context->gid, node->name, node->namespace_);
      common_context->graph_cache.dissociate_writer(
          request_publisher_gid, common_context->gid, node->name, node->namespace_);
      return nullptr;
    }
  }

  // 取消清理操作 (Cancel cleanup operations)
  cleanup_rmw_client.cancel();
  cleanup_datawriter.cancel();
  cleanup_datareader.cancel();
  return_response_type_support.cancel();
  return_request_type_support.cancel();
  cleanup_info.cancel();

  // 返回创建的客户端 (Return the created client)
  return rmw_client;
}

/**
 * @brief 销毁一个 ROS2 客户端 (Destroy a ROS2 client)
 *
 * @param[in] node 指向要销毁的客户端所属节点的指针 (Pointer to the node that the client belongs to)
 * @param[in] client 指向要销毁的客户端的指针 (Pointer to the client to be destroyed)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_destroy_client(rmw_node_t *node, rmw_client_t *client) {
  // 检查 node 参数是否为空，如果为空返回无效参数错误 (Check if the node argument is null, return
  // invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  // 检查 node 的实现标识符是否与 eprosima_fastrtps_identifier 匹配，如果不匹配返回错误的 RMW
  // 实现错误 (Check if the node's implementation identifier matches eprosima_fastrtps_identifier,
  // return incorrect RMW implementation error if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查 client 参数是否为空，如果为空返回无效参数错误 (Check if the client argument is null,
  // return invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  // 检查 client 的实现标识符是否与 eprosima_fastrtps_identifier 匹配，如果不匹配返回错误的 RMW
  // 实现错误 (Check if the client's implementation identifier matches eprosima_fastrtps_identifier,
  // return incorrect RMW implementation error if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      client, client->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 获取客户端的自定义信息 (Get the custom information of the client)
  auto info = static_cast<CustomClientInfo *>(client->data);

  // 获取类型支持注册表实例 (Get the type support registry instance)
  TypeSupportRegistry &type_registry = TypeSupportRegistry::get_instance();

  // 获取请求类型支持实现并返回给类型注册表 (Get the request type support implementation and return
  // it to the type registry)
  auto impl = static_cast<rmw_fastrtps_dynamic_cpp::BaseTypeSupport *>(
      const_cast<void *>(info->request_type_support_impl_));
  auto ros_type_support =
      static_cast<const rosidl_service_type_support_t *>(impl->ros_type_support());
  type_registry.return_request_type_support(ros_type_support);

  // 获取响应类型支持实现并返回给类型注册表 (Get the response type support implementation and return
  // it to the type registry)
  impl = static_cast<rmw_fastrtps_dynamic_cpp::BaseTypeSupport *>(
      const_cast<void *>(info->response_type_support_impl_));
  ros_type_support = static_cast<const rosidl_service_type_support_t *>(impl->ros_type_support());
  type_registry.return_response_type_support(ros_type_support);

  // 销毁客户端 (Destroy the client)
  return rmw_fastrtps_shared_cpp::__rmw_destroy_client(eprosima_fastrtps_identifier, node, client);
}

/**
 * @brief 获取客户端请求发布者的实际 QoS 配置 (Get the actual QoS configuration of the client
 * request publisher)
 *
 * @param[in] client 指向要查询的客户端的指针 (Pointer to the client to be queried)
 * @param[out] qos 存储实际 QoS 配置的指针 (Pointer to store the actual QoS configuration)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_client_request_publisher_get_actual_qos(
    const rmw_client_t *client, rmw_qos_profile_t *qos) {
  // 检查 client 参数是否为空，如果为空返回无效参数错误 (Check if the client argument is null,
  // return invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  // 检查 client 的实现标识符是否与 eprosima_fastrtps_identifier 匹配，如果不匹配返回错误的 RMW
  // 实现错误 (Check if the client's implementation identifier matches eprosima_fastrtps_identifier,
  // return incorrect RMW implementation error if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      client, client->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查 qos 参数是否为空，如果为空返回无效参数错误 (Check if the qos argument is null, return
  // invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);

  // 获取客户端请求发布者的实际 QoS 配置 (Get the actual QoS configuration of the client request
  // publisher)
  return rmw_fastrtps_shared_cpp::__rmw_client_request_publisher_get_actual_qos(client, qos);
}

/**
 * @brief 获取客户端响应订阅的实际QoS配置 (Get the actual QoS configuration of the client response
 * subscription)
 *
 * @param[in] client 指向rmw_client_t结构体的指针 (Pointer to the rmw_client_t structure)
 * @param[out] qos 用于存储获取到的QoS配置的指针 (Pointer to store the obtained QoS configuration)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_client_response_subscription_get_actual_qos(
    const rmw_client_t *client, rmw_qos_profile_t *qos) {
  // 检查client参数是否为空，如果为空则返回无效参数错误 (Check if the client parameter is null,
  // return invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  // 检查client的实现标识符与期望的eprosima_fastrtps_identifier是否匹配，不匹配则返回错误 (Check if
  // the implementation identifier of the client matches the expected eprosima_fastrtps_identifier,
  // return an error if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      client, client->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查qos参数是否为空，如果为空则返回无效参数错误 (Check if the qos parameter is null, return
  // invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);

  // 调用共享函数__rmw_client_response_subscription_get_actual_qos并返回结果 (Call the shared
  // function __rmw_client_response_subscription_get_actual_qos and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_client_response_subscription_get_actual_qos(client, qos);
}

/**
 * @brief 设置客户端新响应回调函数 (Set the client's new response callback function)
 *
 * @param[in,out] rmw_client 指向rmw_client_t结构体的指针 (Pointer to the rmw_client_t structure)
 * @param[in] callback 新响应回调函数 (New response callback function)
 * @param[in] user_data 用户数据，将传递给回调函数 (User data that will be passed to the callback
 * function)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_client_set_on_new_response_callback(
    rmw_client_t *rmw_client, rmw_event_callback_t callback, const void *user_data) {
  // 检查rmw_client参数是否为空，如果为空则返回无效参数错误 (Check if the rmw_client parameter is
  // null, return invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(rmw_client, RMW_RET_INVALID_ARGUMENT);

  // 调用共享函数__rmw_client_set_on_new_response_callback并返回结果 (Call the shared function
  // __rmw_client_set_on_new_response_callback and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_client_set_on_new_response_callback(
      rmw_client, callback, user_data);
}

}  // extern "C"
