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

#include "client_service_common.hpp"
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
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_service_info.hpp"
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
 * @brief 创建一个 ROS2 服务 (Create a ROS2 service)
 *
 * @param[in] node 指向要创建服务的节点的指针 (Pointer to the node where the service will be
 * created)
 * @param[in] type_supports 服务类型支持结构体，包含了服务的请求和响应类型 (Service type support
 * structure, containing the request and response types of the service)
 * @param[in] service_name 要创建的服务的名称 (Name of the service to be created)
 * @param[in] qos_policies 服务的质量保证策略 (Quality of Service policies for the service)
 *
 * @return rmw_service_t* 成功时返回指向新创建的服务的指针，失败时返回 nullptr (Pointer to the newly
 * created service on success, nullptr on failure)
 */
rmw_service_t *rmw_create_service(
    const rmw_node_t *node,
    const rosidl_service_type_support_t *type_supports,
    const char *service_name,
    const rmw_qos_profile_t *qos_policies) {
  /////
  // 检查输入参数 (Check input parameters)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, nullptr);
  // 检查节点的实现标识符是否与 eprosima_fastrtps_identifier 匹配 (Check if the node's
  // implementation identifier matches eprosima_fastrtps_identifier)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, eprosima_fastrtps_identifier, return nullptr);
  // 检查 type_supports 参数是否为空 (Check if the type_supports argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(type_supports, nullptr);
  // 检查 service_name 参数是否为空 (Check if the service_name argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(service_name, nullptr);
  // 如果 service_name 为空字符串，则设置错误消息并返回空指针 (If service_name is an empty string,
  // set error message and return nullptr)
  if (0 == strlen(service_name)) {
    RMW_SET_ERROR_MSG("service_name argument is an empty string");
    return nullptr;
  }
  // 检查 qos_policies 参数是否为空 (Check if the qos_policies argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos_policies, nullptr);
  // 如果避免 ROS 命名空间约定为 false (If avoid_ros_namespace_conventions is false)
  if (!qos_policies->avoid_ros_namespace_conventions) {
    int validation_result = RMW_TOPIC_VALID;
    // 验证服务名称是否有效 (Validate if the service_name is valid)
    rmw_ret_t ret = rmw_validate_full_topic_name(service_name, &validation_result, nullptr);
    // 如果验证结果不是 RMW_RET_OK，则返回空指针 (If the validation result is not RMW_RET_OK, return
    // nullptr)
    if (RMW_RET_OK != ret) {
      return nullptr;
    }
    // 如果验证结果不是 RMW_TOPIC_VALID，则设置错误消息并返回空指针 (If the validation result is not
    // RMW_TOPIC_VALID, set error message and return nullptr)
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
   * @brief 检查 RMW QoS (Check RMW QoS)
   *
   * @param[in] adapted_qos_policies 要检查的 QoS 策略 (The QoS policies to be checked)
   *
   * @return 如果 QoS 策略有效，则返回 true；否则返回 false (Returns true if the QoS policies are
   * valid; otherwise, returns false)
   */
  if (!is_valid_qos(adapted_qos_policies)) {
    // 如果 QoS 不合法，则设置错误消息并返回空指针
    // (If the QoS is invalid, set the error message and return a nullptr)
    RMW_SET_ERROR_MSG("create_service() called with invalid QoS");
    return nullptr;
  }

  /////
  /**
   * @brief 获取参与者和子实体 (Get Participant and SubEntities)
   *
   * @param[in] node 当前节点的指针，包含了 ROS2 节点的上下文信息 (Pointer to the current node,
   * which contains the context information of the ROS2 node)
   */
  // 获取通用上下文 (Get common context)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);
  // 从节点上下文中提取自定义参与者信息 (Extract custom participant information from
  // the node context)
  auto participant_info =
      static_cast<CustomParticipantInfo *>(node->context->impl->participant_info);

  // 获取 Fast DDS 域参与者 (Get Fast DDS DomainParticipant)
  eprosima::fastdds::dds::DomainParticipant *dds_participant = participant_info->participant_;
  eprosima::fastdds::dds::Publisher *publisher = participant_info->publisher_;
  eprosima::fastdds::dds::Subscriber *subscriber = participant_info->subscriber_;

  /////
  /**
   * @brief 获取 RMW 类型支持 (Get RMW Type Support)
   *
   * @param[in] type_supports 服务类型支持的列表 (List of service type supports)
   * @param[in] rosidl_typesupport_introspection_c__identifier C 语言类型支持标识符 (C language type
   * support identifier)
   * @param[out] type_support 返回找到的类型支持 (Return the found type support)
   *
   * @return 如果找到类型支持，返回指向它的指针；否则返回 nullptr (If the type support is found,
   * return a pointer to it; otherwise return nullptr)
   */

  // 获取 RMW 类型支持 (Get RMW Type Support)
  const rosidl_service_type_support_t *type_support =
      get_service_typesupport_handle(type_supports, rosidl_typesupport_introspection_c__identifier);

  // 如果没有找到类型支持 (If the type support is not found)
  if (!type_support) {
    // 获取之前的错误字符串 (Get the previous error string)
    rcutils_error_string_t prev_error_string = rcutils_get_error_string();
    // 重置错误 (Reset the error)
    rcutils_reset_error();

    // 再次尝试获取类型支持，这次使用 C++ 语言类型支持标识符 (Try to get the type support again,
    // this time using the C++ language type support identifier)
    type_support = get_service_typesupport_handle(
        type_supports, rosidl_typesupport_introspection_cpp::typesupport_identifier);

    // 如果仍然没有找到类型支持 (If the type support is still not found)
    if (!type_support) {
      // 获取错误字符串 (Get the error string)
      rcutils_error_string_t error_string = rcutils_get_error_string();
      // 重置错误 (Reset the error)
      rcutils_reset_error();

      // 设置错误消息，包含之前的错误字符串和当前的错误字符串 (Set the error message, including the
      // previous error string and the current error string)
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "Type support not from this implementation. Got:\n"
          "    %s\n"
          "    %s\n"
          "while fetching it",
          prev_error_string.str, error_string.str);
      // 返回 nullptr，表示没有找到类型支持 (Return nullptr, indicating that the type support was
      // not found)
      return nullptr;
    }
  }

  // 对实体创建互斥锁进行加锁，确保线程安全 (Lock the entity creation mutex to ensure thread safety)
  std::lock_guard<std::mutex> lck(participant_info->entity_creation_mutex_);

  /////
  // 找到并检查现有的主题和类型 (Find and check existing topics and types)

  // 创建主题和类型名称 (Create Topic and Type names)
  const void *untyped_request_members =
      get_request_ptr(type_support->data, type_support->typesupport_identifier);
  // 获取请求成员指针 (Get the pointer to request members)
  const void *untyped_response_members =
      get_response_ptr(type_support->data, type_support->typesupport_identifier);
  // 获取响应成员指针 (Get the pointer to response members)

  std::string request_type_name =
      _create_type_name(untyped_request_members, type_support->typesupport_identifier);
  // 创建请求类型名称 (Create request type name)
  std::string response_type_name =
      _create_type_name(untyped_response_members, type_support->typesupport_identifier);
  // 创建响应类型名称 (Create response type name)

  std::string response_topic_name =
      _create_topic_name(&adapted_qos_policies, ros_service_response_prefix, service_name, "Reply")
          .to_string();
  // 创建响应主题名称 (Create response topic name)
  std::string request_topic_name =
      _create_topic_name(
          &adapted_qos_policies, ros_service_requester_prefix, service_name, "Request")
          .to_string();
  // 创建请求主题名称 (Create request topic name)

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

  /////
  // 创建自定义服务结构 (info)
  // Create the custom Service struct (info)
  CustomServiceInfo *info = new (std::nothrow) CustomServiceInfo();
  if (!info) {
    // 如果分配自定义信息失败，设置错误消息
    // If allocation of custom info fails, set error message
    RMW_SET_ERROR_MSG("create_service() failed to allocate custom info");
    return nullptr;
  }

  // 创建一个作用域退出对象，以便在退出当前作用域时执行清理操作
  // Create a scope exit object to perform cleanup operations when exiting the current scope
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
    // 删除自定义服务信息对象
    // Delete custom service info object
    delete info;
  });

  // 设置类型支持标识符
  // Set the typesupport identifier
  info->typesupport_identifier_ = type_support->typesupport_identifier;

  /////
  // 创建类型支持结构体 (Create the Type Support structs)
  TypeSupportRegistry &type_registry = TypeSupportRegistry::get_instance();

  // 获取请求类型支持实现 (Get the request type support implementation)
  auto request_type_impl = type_registry.get_request_type_support(type_support);
  if (!request_type_impl) {
    // 设置错误信息 (Set error message)
    RMW_SET_ERROR_MSG("create_service() failed to get request_type_support");
    return nullptr;
  }

  // 返回请求类型支持并在作用域结束时执行 (Return request type support and execute when scope ends)
  auto return_request_type_support = rcpputils::make_scope_exit([&type_registry, type_support]() {
    type_registry.return_request_type_support(type_support);
  });

  // 获取响应类型支持实现 (Get the response type support implementation)
  auto response_type_impl = type_registry.get_response_type_support(type_support);
  if (!response_type_impl) {
    // 设置错误信息 (Set error message)
    RMW_SET_ERROR_MSG("create_service() failed to get response_type_support");
    return nullptr;
  }

  // 返回响应类型支持并在作用域结束时执行 (Return response type support and execute when scope ends)
  auto return_response_type_support = rcpputils::make_scope_exit([&type_registry, type_support]() {
    type_registry.return_response_type_support(type_support);
  });

  // 设置请求和响应类型支持实现 (Set request and response type support implementations)
  info->request_type_support_impl_ = request_type_impl;
  info->response_type_support_impl_ = response_type_impl;

  // 如果请求的 Fast-DDS 类型不存在，则创建一个新的类型支持代理 (If request Fast-DDS type doesn't
  // exist, create a new TypeSupportProxy)
  if (!request_fastdds_type) {
    auto tsupport =
        new (std::nothrow) rmw_fastrtps_dynamic_cpp::TypeSupportProxy(request_type_impl);
    if (!tsupport) {
      // 设置错误信息 (Set error message)
      RMW_SET_ERROR_MSG("create_service() failed to allocate request TypeSupportProxy");
      return nullptr;
    }

    // 重置请求的 Fast-DDS 类型 (Reset the request Fast-DDS type)
    request_fastdds_type.reset(tsupport);
  }

  // 如果响应的 Fast-DDS 类型不存在，则创建一个新的类型支持代理 (If response Fast-DDS type doesn't
  // exist, create a new TypeSupportProxy)
  if (!response_fastdds_type) {
    auto tsupport =
        new (std::nothrow) rmw_fastrtps_dynamic_cpp::TypeSupportProxy(response_type_impl);
    if (!tsupport) {
      // 设置错误信息 (Set error message)
      RMW_SET_ERROR_MSG("create_service() failed to allocate response TypeSupportProxy");
      return nullptr;
    }

    // 重置响应的 Fast-DDS 类型 (Reset the response Fast-DDS type)
    response_fastdds_type.reset(tsupport);
  }

  // 注册请求类型，如果失败则设置错误信息并返回空指针 (Register request type, if fails set error
  // message and return nullptr)
  if (ReturnCode_t::RETCODE_OK != request_fastdds_type.register_type(dds_participant)) {
    RMW_SET_ERROR_MSG("create_service() failed to register request type");
    return nullptr;
  }
  info->request_type_support_ = request_fastdds_type;

  // 注册响应类型，如果失败则设置错误信息并返回空指针 (Register response type, if fails set error
  // message and return nullptr)
  if (ReturnCode_t::RETCODE_OK != response_fastdds_type.register_type(dds_participant)) {
    RMW_SET_ERROR_MSG("create_service() failed to register response type");
    return nullptr;
  }
  info->response_type_support_ = response_fastdds_type;

  /////
  /**
   * @brief 创建服务监听器和发布器监听器 (Create service listeners)
   *
   * @param[in] info 服务信息结构体指针，用于存储监听器实例 (Pointer to the service information
   * structure for storing listener instances)
   *
   * @return 如果成功创建监听器，则返回nullptr；否则，返回错误消息 (Returns nullptr if the listeners
   * are successfully created; otherwise, returns an error message)
   */
  // 创建监听器 (Create Listeners)
  info->listener_ = new (std::nothrow) ServiceListener(info);
  if (!info->listener_) {
    // 创建请求订阅者监听器失败时设置错误消息 (Set error message when failed to create request
    // subscriber listener)
    RMW_SET_ERROR_MSG("create_service() failed to create request subscriber listener");
    return nullptr;
  }

  // 创建发布器监听器 (Create Publisher Listener)
  info->pub_listener_ = new (std::nothrow) ServicePubListener(info);
  if (!info->pub_listener_) {
    // 创建响应发布者监听器失败时设置错误消息 (Set error message when failed to create response
    // publisher listener)
    RMW_SET_ERROR_MSG("create_service() failed to create response publisher listener");
    return nullptr;
  }

  /////
  // 创建并注册主题 (Create and register Topics)
  // 为两个主题设置相同的默认主题 QoS (Same default topic QoS for both topics)
  eprosima::fastdds::dds::TopicQos topic_qos = dds_participant->get_default_topic_qos();
  if (!get_topic_qos(adapted_qos_policies, topic_qos)) {
    // 设置主题 QoS 失败时返回错误信息 (Set error message when setting topic QoS fails)
    RMW_SET_ERROR_MSG("create_service() failed setting topic QoS");
    return nullptr;
  }

  // 创建请求主题 (Create request topic)
  info->request_topic_ = participant_info->find_or_create_topic(
      request_topic_name, request_type_name, topic_qos, nullptr);
  if (!info->request_topic_) {
    // 创建请求主题失败时返回错误信息 (Set error message when creating request topic fails)
    RMW_SET_ERROR_MSG("create_service() failed to create request topic");
    return nullptr;
  }

  // 将请求主题描述符赋值给 request_topic_desc (Assign the request topic descriptor to
  // request_topic_desc)
  request_topic_desc = info->request_topic_;

  // 创建响应主题 (Create response topic)
  info->response_topic_ = participant_info->find_or_create_topic(
      response_topic_name, response_type_name, topic_qos, nullptr);
  if (!info->response_topic_) {
    // 创建响应主题失败时返回错误信息 (Set error message when creating response topic fails)
    RMW_SET_ERROR_MSG("create_service() failed to create response topic");
    return nullptr;
  }

  // 查找 DataWriter 和 DataReader QoS 的关键字 (Keyword to find DataWriter and DataReader QoS)
  const std::string topic_name_fallback = "service";

  /////
  // 创建请求 DataReader
  // Create request DataReader

  // 如果定义了 FASTRTPS_DEFAULT_PROFILES_FILE，则使用 _create_topic_name()
  // 定义的主题名称来定位订阅者配置文件， 以填充 DataReader QoS。如果找不到配置文件，尝试使用
  // profile_name "service" 进行搜索。否则，使用 Fast DDS 默认 QoS。 If
  // FASTRTPS_DEFAULT_PROFILES_FILE is defined, fill DataReader QoS with a subscriber profile
  // located based on the topic name defined by _create_topic_name(). If no profile is found, a
  // search with profile_name "service" is attempted. Else, use the default Fast DDS QoS.
  eprosima::fastdds::dds::DataReaderQos reader_qos = subscriber->get_default_datareader_qos();

  // 尝试加载名为 "service" 的配置文件，
  // 如果不存在，则尝试使用请求主题名称
  // 不需要检查返回代码，因为如果任何配置文件都不存在，
  // QoS 已经正确设置：
  // 如果都不存在，则为默认值，如果只存在一个，则为所选配置文件，
  // 如果两个都存在，则选择主题名称
  // Try to load the profile named "service",
  // if it does not exist it tries with the request topic name
  // It does not need to check the return code, as if any of the profile does not exist,
  // the QoS is already set correctly:
  // If none exist is default, if only one exists is the one chosen,
  // if both exist topic name is chosen
  subscriber->get_datareader_qos_from_profile(topic_name_fallback, reader_qos);
  subscriber->get_datareader_qos_from_profile(request_topic_name, reader_qos);

  // 如果不保留中间件默认 QoS，则设置历史内存策略和关闭数据共享
  // If not leaving middleware default QoS, set history memory policy and turn off data sharing
  if (!participant_info->leave_middleware_default_qos) {
    reader_qos.endpoint().history_memory_policy =
        eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    reader_qos.data_sharing().off();
  }

  // 获取 DataReader QoS，如果失败则设置错误消息并返回空指针
  // Get DataReader QoS, if failed, set error message and return nullptr
  if (!get_datareader_qos(
          adapted_qos_policies,
          *type_supports->request_typesupport->get_type_hash_func(
              type_supports->request_typesupport),
          reader_qos)) {
    RMW_SET_ERROR_MSG("create_service() failed setting request DataReader QoS");
    return nullptr;
  }

  // 创建 DataReader
  // Creates DataReader
  info->request_reader_ = subscriber->create_datareader(
      request_topic_desc, reader_qos, info->listener_,
      eprosima::fastdds::dds::StatusMask::subscription_matched());

  // 如果创建请求 DataReader 失败，则设置错误消息并返回空指针
  // If failed to create request DataReader, set error message and return nullptr
  if (!info->request_reader_) {
    RMW_SET_ERROR_MSG("create_service() failed to create request DataReader");
    return nullptr;
  }

  // 设置状态条件的启用状态
  // Set enabled statuses for the status condition
  info->request_reader_->get_statuscondition().set_enabled_statuses(
      eprosima::fastdds::dds::StatusMask::data_available());

  // lambda 函数用于删除 datareader
  // Lambda function to delete datareader
  auto cleanup_datareader = rcpputils::make_scope_exit(
      [subscriber, info]() { subscriber->delete_datareader(info->request_reader_); });

  /////
  // Create response DataWriter

  // 如果定义了 FASTRTPS_DEFAULT_PROFILES_FILE，则根据 _create_topic_name() 定义的主题名称，
  // 在位于基础上填充 DataWriter QoS 的发布者配置文件。如果找不到配置文件，将尝试使用 profile_name
  // "service" 进行搜索。 否则，使用 Fast DDS 默认 QoS。 If FASTRTPS_DEFAULT_PROFILES_FILE is
  // defined, fill DataWriter QoS with a publisher profile located based on the topic name defined
  // by _create_topic_name(). If no profile is found, a search with profile_name "service" is
  // attempted. Else, use the default Fast DDS QoS.
  eprosima::fastdds::dds::DataWriterQos writer_qos = publisher->get_default_datawriter_qos();

  // 尝试加载名为 "service" 的配置文件，
  // 如果不存在，则尝试使用请求主题名称
  // 无需检查返回代码，因为如果任何配置文件都不存在，
  // QoS 已经正确设置：
  // 如果没有存在，默认值，如果只有一个存在，则选择该配置文件，
  // 如果两个都存在，则选择主题名称
  // Try to load the profile named "service",
  // if it does not exist it tries with the request topic name
  // It does not need to check the return code, as if any of the profile does not exist,
  // the QoS is already set correctly:
  // If none exist is default, if only one exists is the one chosen,
  // if both exist topic name is chosen
  publisher->get_datawriter_qos_from_profile(topic_name_fallback, writer_qos);
  publisher->get_datawriter_qos_from_profile(response_topic_name, writer_qos);

  // 修改特定 DataWriter Qos
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

  // 创建 DataWriter
  // Creates DataWriter
  info->response_writer_ = publisher->create_datawriter(
      info->response_topic_, writer_qos, info->pub_listener_,
      eprosima::fastdds::dds::StatusMask::publication_matched());

  // 如果无法创建 DataWriter，则返回错误信息
  // If the creation of the DataWriter fails, return an error message
  if (!info->response_writer_) {
    RMW_SET_ERROR_MSG("create_service() failed to create response DataWriter");
    return nullptr;
  }

  // 设置状态条件
  // Set status condition
  info->response_writer_->get_statuscondition().set_enabled_statuses(
      eprosima::fastdds::dds::StatusMask::none());

  // lambda 用于删除 datawriter
  // Lambda to delete DataWriter
  auto cleanup_datawriter = rcpputils::make_scope_exit(
      [publisher, info]() { publisher->delete_datawriter(info->response_writer_); });

  /////
  // Create Service

  // 打印服务详细信息的调试日志标题
  // Print the debug log title for service details
  RCUTILS_LOG_DEBUG_NAMED("rmw_fastrtps_dynamic_cpp", "************ Service Details *********");

  // 打印请求主题名称的调试日志
  // Print the debug log for request topic name
  RCUTILS_LOG_DEBUG_NAMED("rmw_fastrtps_dynamic_cpp", "Sub Topic %s", request_topic_name.c_str());

  // 打印响应主题名称的调试日志
  // Print the debug log for response topic name
  RCUTILS_LOG_DEBUG_NAMED("rmw_fastrtps_dynamic_cpp", "Pub Topic %s", response_topic_name.c_str());

  // 打印服务详细信息的调试日志结束符
  // Print the debug log end mark for service details
  RCUTILS_LOG_DEBUG_NAMED("rmw_fastrtps_dynamic_cpp", "***********");

  // 为 rmw_service 分配内存
  // Allocate memory for rmw_service
  rmw_service_t *rmw_service = rmw_service_allocate();
  if (!rmw_service) {
    RMW_SET_ERROR_MSG("create_service() failed to allocate memory for rmw_service");
    return nullptr;
  }

  // 创建一个作用域退出对象，用于在函数返回时自动清理 rmw_service 资源
  // Create a scope exit object to automatically clean up rmw_service resources when the function
  // returns
  auto cleanup_rmw_service = rcpputils::make_scope_exit([rmw_service]() {
    rmw_free(const_cast<char *>(rmw_service->service_name));
    rmw_free(rmw_service);
  });

  // 设置 rmw_service 的实现标识符和数据
  // Set the implementation identifier and data for rmw_service
  rmw_service->implementation_identifier = eprosima_fastrtps_identifier;
  rmw_service->data = info;

  // 为服务名称分配内存并设置 rmw_service 的服务名称
  // Allocate memory for the service name and set the service name for rmw_service
  rmw_service->service_name =
      reinterpret_cast<const char *>(rmw_allocate(strlen(service_name) + 1));
  if (!rmw_service->service_name) {
    RMW_SET_ERROR_MSG("create_service() failed to allocate memory for service name");
    return nullptr;
  }

  // 复制服务名称到 rmw_service 的服务名称中
  // Copy the service name to the service name of rmw_service
  memcpy(const_cast<char *>(rmw_service->service_name), service_name, strlen(service_name) + 1);

  {
    // 更新图形信息
    // Update graph information
    std::lock_guard<std::mutex> guard(common_context->node_update_mutex);

    // 创建请求订阅者的 GID 并将其与节点关联
    // Create the GID for the request subscriber and associate it with the node
    rmw_gid_t request_subscriber_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        eprosima_fastrtps_identifier, info->request_reader_->guid());
    common_context->graph_cache.associate_reader(
        request_subscriber_gid, common_context->gid, node->name, node->namespace_);

    // 创建响应发布者的 GID 并将其与节点关联
    // Create the GID for the response publisher and associate it with the node
    rmw_gid_t response_publisher_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
        eprosima_fastrtps_identifier, info->response_writer_->guid());
    rmw_dds_common::msg::ParticipantEntitiesInfo msg = common_context->graph_cache.associate_writer(
        response_publisher_gid, common_context->gid, node->name, node->namespace_);

    // 发布更新后的图形信息
    // Publish the updated graph information
    rmw_ret_t rmw_ret = rmw_fastrtps_shared_cpp::__rmw_publish(
        eprosima_fastrtps_identifier, common_context->pub, static_cast<void *>(&msg), nullptr);
    if (RMW_RET_OK != rmw_ret) {
      // 如果发布失败，取消关联并返回空指针
      // If publishing fails, dissociate and return a null pointer
      common_context->graph_cache.dissociate_writer(
          response_publisher_gid, common_context->gid, node->name, node->namespace_);
      common_context->graph_cache.dissociate_reader(
          request_subscriber_gid, common_context->gid, node->name, node->namespace_);
      return nullptr;
    }
  }

  // 取消清理操作
  // Cancel cleanup operations
  cleanup_rmw_service.cancel();
  cleanup_datawriter.cancel();
  cleanup_datareader.cancel();
  return_response_type_support.cancel();
  return_request_type_support.cancel();
  cleanup_info.cancel();

  // 返回创建的 rmw_service 对象
  // Return the created rmw_service object
  return rmw_service;
}

/**
 * @brief 销毁一个 ROS 服务 (Destroy a ROS service)
 *
 * @param[in] node 指向要销毁服务的节点的指针 (Pointer to the node where the service is to be
 * destroyed)
 * @param[in] service 要销毁的服务的指针 (Pointer to the service to be destroyed)
 * @return rmw_ret_t 返回操作结果 (Return the result of the operation)
 */
rmw_ret_t rmw_destroy_service(rmw_node_t *node, rmw_service_t *service) {
  // 检查 node 参数是否为空，如果为空返回错误 (Check if the node argument is null, return error if
  // it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);

  // 检查 node 的实现标识符是否与 eprosima_fastrtps_identifier 匹配，如果不匹配返回错误 (Check if
  // the node's implementation identifier matches eprosima_fastrtps_identifier, return error if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查 service 参数是否为空，如果为空返回错误 (Check if the service argument is null, return
  // error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(service, RMW_RET_INVALID_ARGUMENT);

  // 检查 service 的实现标识符是否与 eprosima_fastrtps_identifier 匹配，如果不匹配返回错误 (Check if
  // the service's implementation identifier matches eprosima_fastrtps_identifier, return error if
  // not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      service, service->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 获取服务的自定义信息 (Get the custom information of the service)
  auto info = static_cast<CustomServiceInfo *>(service->data);

  // 获取类型支持注册表实例 (Get the instance of the type support registry)
  TypeSupportRegistry &type_registry = TypeSupportRegistry::get_instance();

  // 获取请求类型支持的实现 (Get the implementation of the request type support)
  auto impl = static_cast<rmw_fastrtps_dynamic_cpp::BaseTypeSupport *>(
      const_cast<void *>(info->request_type_support_impl_));

  // 获取 ROS 类型支持 (Get the ROS type support)
  auto ros_type_support =
      static_cast<const rosidl_service_type_support_t *>(impl->ros_type_support());

  // 返回请求类型支持 (Return the request type support)
  type_registry.return_request_type_support(ros_type_support);

  // 获取响应类型支持的实现 (Get the implementation of the response type support)
  impl = static_cast<rmw_fastrtps_dynamic_cpp::BaseTypeSupport *>(
      const_cast<void *>(info->response_type_support_impl_));

  // 获取 ROS 类型支持 (Get the ROS type support)
  ros_type_support = static_cast<const rosidl_service_type_support_t *>(impl->ros_type_support());

  // 返回响应类型支持 (Return the response type support)
  type_registry.return_response_type_support(ros_type_support);

  // 销毁服务 (Destroy the service)
  return rmw_fastrtps_shared_cpp::__rmw_destroy_service(
      eprosima_fastrtps_identifier, node, service);
}

/**
 * @brief 获取服务响应发布者的实际 QoS 配置 (Get the actual QoS configuration of the service
 * response publisher)
 *
 * @param[in] service 指向要查询的服务的指针 (Pointer to the service to query)
 * @param[out] qos 存储实际 QoS 配置的指针 (Pointer to store the actual QoS configuration)
 * @return rmw_ret_t 返回操作结果 (Return the result of the operation)
 */
rmw_ret_t rmw_service_response_publisher_get_actual_qos(
    const rmw_service_t *service, rmw_qos_profile_t *qos) {
  // 检查 service 参数是否为空，如果为空返回错误 (Check if the service argument is null, return
  // error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(service, RMW_RET_INVALID_ARGUMENT);

  // 检查 service 的实现标识符是否与 eprosima_fastrtps_identifier 匹配，如果不匹配返回错误 (Check if
  // the service's implementation identifier matches eprosima_fastrtps_identifier, return error if
  // not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      service, service->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查 qos 参数是否为空，如果为空返回错误 (Check if the qos argument is null, return error if it
  // is)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);

  // 获取服务响应发布者的实际 QoS 配置 (Get the actual QoS configuration of the service response
  // publisher)
  return rmw_fastrtps_shared_cpp::__rmw_service_response_publisher_get_actual_qos(service, qos);
}

/**
 * @brief 获取服务请求订阅的实际QoS配置 (Get the actual QoS configuration of the service request
 * subscription)
 *
 * @param[in] service 服务指针，不能为空 (Pointer to the service, cannot be null)
 * @param[out] qos 返回QoS配置的指针，不能为空 (Pointer to return the QoS configuration, cannot be
 * null)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_service_request_subscription_get_actual_qos(
    const rmw_service_t *service, rmw_qos_profile_t *qos) {
  // 检查 service 参数是否为空，为空则返回无效参数错误 (Check if the service parameter is null,
  // return invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(service, RMW_RET_INVALID_ARGUMENT);
  // 检查 service 的实现标识符是否与 eprosima_fastrtps_identifier 匹配，不匹配则返回错误的 RMW 实现
  // (Check if the implementation identifier of the service matches eprosima_fastrtps_identifier,
  // return incorrect RMW implementation if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      service, service->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查 qos 参数是否为空，为空则返回无效参数错误 (Check if the qos parameter is null, return
  // invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);

  // 调用共享实现获取实际QoS配置，并返回结果 (Call the shared implementation to get the actual QoS
  // configuration and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_service_request_subscription_get_actual_qos(service, qos);
}

/**
 * @brief 设置服务的新请求回调 (Set the new request callback for the service)
 *
 * @param[in] rmw_service 服务指针，不能为空 (Pointer to the service, cannot be null)
 * @param[in] callback 回调函数 (Callback function)
 * @param[in] user_data 用户数据，可以为空 (User data, can be null)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_service_set_on_new_request_callback(
    rmw_service_t *rmw_service, rmw_event_callback_t callback, const void *user_data) {
  // 检查 rmw_service 参数是否为空，为空则返回无效参数错误 (Check if the rmw_service parameter is
  // null, return invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(rmw_service, RMW_RET_INVALID_ARGUMENT);

  // 调用共享实现设置新请求回调，并返回结果 (Call the shared implementation to set the new request
  // callback and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_service_set_on_new_request_callback(
      rmw_service, callback, user_data);
}

}  // extern "C"
