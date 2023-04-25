// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#include "fastdds/dds/publisher/Publisher.hpp"

#include <string>

#include "fastdds/dds/core/policy/QosPolicies.hpp"
#include "fastdds/dds/domain/DomainParticipant.hpp"
#include "fastdds/dds/publisher/qos/DataWriterQos.hpp"
#include "fastdds/dds/topic/Topic.hpp"
#include "fastdds/dds/topic/TopicDescription.hpp"
#include "fastdds/dds/topic/TypeSupport.hpp"
#include "fastdds/dds/topic/qos/TopicQos.hpp"
#include "fastdds/rtps/resources/ResourceManagement.h"
#include "rcpputils/scope_exit.hpp"
#include "rcutils/error_handling.h"
#include "rcutils/macros.h"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/validate_full_topic_name.h"
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_cpp/publisher.hpp"
#include "rmw_fastrtps_shared_cpp/create_rmw_gid.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_publisher_info.hpp"
#include "rmw_fastrtps_shared_cpp/names.hpp"
#include "rmw_fastrtps_shared_cpp/namespace_prefix.hpp"
#include "rmw_fastrtps_shared_cpp/qos.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/utils.hpp"
#include "tracetools/tracetools.h"
#include "type_support_common.hpp"

/**
 * @brief 创建一个 FastRTPS 发布者 (Create a FastRTPS publisher)
 *
 * @param[in] participant_info 参与者信息，包含 FastRTPS 参与者实例 (Participant information,
 * containing the FastRTPS participant instance)
 * @param[in] type_supports 消息类型支持，用于序列化和反序列化消息 (Message type support for
 * serialization and deserialization of messages)
 * @param[in] topic_name 要发布的主题名称 (The name of the topic to be published)
 * @param[in] qos_policies 质量服务策略，定义发布者的行为 (Quality of Service policies defining the
 * behavior of the publisher)
 * @param[in] publisher_options 发布者选项，包括用户数据等 (Publisher options, including user data,
 * etc.)
 *
 * @return 返回创建的 FastRTPS 发布者指针或 nullptr (if failed) (Returns the created FastRTPS
 * publisher pointer or nullptr if failed)
 */
rmw_publisher_t *rmw_fastrtps_cpp::create_publisher(
    CustomParticipantInfo *participant_info,
    const rosidl_message_type_support_t *type_supports,
    const char *topic_name,
    const rmw_qos_profile_t *qos_policies,
    const rmw_publisher_options_t *publisher_options) {
  // 检查输入参数是否有效 (Check if input parameters are valid)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(nullptr);
  // 检查 participant_info 参数是否为空 (Check if participant_info argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(participant_info, nullptr);
  // 检查 type_supports 参数是否为空 (Check if type_supports argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(type_supports, nullptr);
  // 检查 topic_name 参数是否为空 (Check if topic_name argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(topic_name, nullptr);
  // 判断 topic_name 是否为空字符串 (Check if topic_name is an empty string)
  if (0 == strlen(topic_name)) {
    // 设置错误信息 (Set error message)
    RMW_SET_ERROR_MSG("create_publisher() called with an empty topic_name argument");
    return nullptr;
  }
  // 检查 qos_policies 参数是否为空 (Check if qos_policies argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos_policies, nullptr);
  // 如果不遵循 ROS 命名空间约定 (If not following ROS namespace conventions)
  if (!qos_policies->avoid_ros_namespace_conventions) {
    int validation_result = RMW_TOPIC_VALID;
    // 验证完整的主题名称 (Validate full topic name)
    rmw_ret_t ret = rmw_validate_full_topic_name(topic_name, &validation_result, nullptr);
    // 如果验证失败 (If validation fails)
    if (RMW_RET_OK != ret) {
      return nullptr;
    }
    // 如果主题名称无效 (If topic name is invalid)
    if (RMW_TOPIC_VALID != validation_result) {
      // 获取原因 (Get the reason)
      const char *reason = rmw_full_topic_name_validation_result_string(validation_result);
      // 设置错误信息 (Set error message)
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "create_publisher() called with invalid topic name: %s", reason);
      return nullptr;
    }
  }
  // 检查 publisher_options 参数是否为空 (Check if publisher_options argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(publisher_options, nullptr);

  // 如果要求严格的唯一网络流终端 (If unique network flow endpoints are strictly required)
  if (RMW_UNIQUE_NETWORK_FLOW_ENDPOINTS_STRICTLY_REQUIRED ==
      publisher_options->require_unique_network_flow_endpoints) {
    // 设置错误信息 (Set error message)
    RMW_SET_ERROR_MSG("Unique network flow endpoints not supported on publishers");
    return nullptr;
  }

  // 检查 RMW QoS（检查 RMW 服务质量）
  // Check RMW QoS (Check the Quality of Service for RMW)
  if (!is_valid_qos(*qos_policies)) {
    // 设置错误信息（如果服务质量无效）
    // Set error message (if the Quality of Service is invalid)
    RMW_SET_ERROR_MSG("create_publisher() called with invalid QoS");
    return nullptr;
  }

  // 获取 RMW 类型支持
  // Get RMW Type Support
  const rosidl_message_type_support_t *type_support =
      get_message_typesupport_handle(type_supports, RMW_FASTRTPS_CPP_TYPESUPPORT_C);
  if (!type_support) {
    rcutils_error_string_t prev_error_string = rcutils_get_error_string();
    rcutils_reset_error();
    type_support = get_message_typesupport_handle(type_supports, RMW_FASTRTPS_CPP_TYPESUPPORT_CPP);
    if (!type_support) {
      rcutils_error_string_t error_string = rcutils_get_error_string();
      rcutils_reset_error();
      // 设置错误信息，类型支持不是此实现的
      // Set error message, type support is not from this implementation
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "Type support not from this implementation. Got:\n"
          "    %s\n"
          "    %s\n"
          "while fetching it",
          prev_error_string.str, error_string.str);
      return nullptr;
    }
  }

  std::lock_guard<std::mutex> lck(participant_info->entity_creation_mutex_);

  // 查找并检查现有主题和类型
  // Find and check existing topic and type

  // 创建主题和类型名称
  // Create Topic and Type names
  auto callbacks = static_cast<const message_type_support_callbacks_t *>(type_support->data);
  std::string type_name = _create_type_name(callbacks);
  auto topic_name_mangled =
      _create_topic_name(qos_policies, ros_topic_prefix, topic_name).to_string();

  eprosima::fastdds::dds::TypeSupport fastdds_type;
  eprosima::fastdds::dds::TopicDescription *des_topic;
  // 查找并检查与参与者信息、主题名称和类型名称匹配的主题和类型
  // Find and check the topic and type that match with participant info, topic name and type name
  if (!rmw_fastrtps_shared_cpp::find_and_check_topic_and_type(
          participant_info, topic_name_mangled, type_name, &des_topic, &fastdds_type)) {
    // 设置错误信息，调用 create_publisher() 时使用了现有主题名称和不兼容的类型
    // Set error message, create_publisher() called with existing topic name and incompatible type
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "create_publisher() called for existing topic name %s with incompatible type %s",
        topic_name_mangled.c_str(), type_name.c_str());
    return nullptr;
  }

  // 获取参与者（DomainParticipant）和发布者（Publisher）
  // Get Participant (DomainParticipant) and Publisher
  eprosima::fastdds::dds::DomainParticipant *dds_participant = participant_info->participant_;
  eprosima::fastdds::dds::Publisher *publisher = participant_info->publisher_;

  // 创建自定义发布者结构体（info）
  // Create the custom Publisher struct (info)
  auto info = new (std::nothrow) CustomPublisherInfo();
  if (!info) {
    // 如果分配失败，设置错误信息
    // Set error message if allocation fails
    RMW_SET_ERROR_MSG("create_publisher() failed to allocate CustomPublisherInfo");
    return nullptr;
  }

  // 定义清理 info 的 lambda 函数
  // Define the lambda function to clean up info
  auto cleanup_info = rcpputils::make_scope_exit([info, participant_info]() {
    rmw_fastrtps_shared_cpp::remove_topic_and_type(
        participant_info, info->publisher_event_, info->topic_, info->type_support_);
    delete info->data_writer_listener_;
    delete info->publisher_event_;
    delete info;
  });

  // 设置类型支持标识符和回调实现
  // Set the type support identifier and callback implementation
  info->typesupport_identifier_ = type_support->typesupport_identifier;
  info->type_support_impl_ = callbacks;

  // 创建类型支持结构体
  // Create the Type Support struct
  if (!fastdds_type) {
    auto tsupport = new (std::nothrow) MessageTypeSupport_cpp(callbacks);
    if (!tsupport) {
      // 如果分配失败，设置错误信息
      // Set error message if allocation fails
      RMW_SET_ERROR_MSG("create_publisher() failed to allocate MessageTypeSupport");
      return nullptr;
    }

    // 将所有权转移到 fastdds_type
    // Transfer ownership to fastdds_type
    fastdds_type.reset(tsupport);
  }

  // 注册类型到参与者（DomainParticipant）
  // Register the type with the Participant (DomainParticipant)
  if (ReturnCode_t::RETCODE_OK != fastdds_type.register_type(dds_participant)) {
    // 如果注册失败，设置错误信息
    // Set error message if registration fails
    RMW_SET_ERROR_MSG("create_publisher() failed to register type");
    return nullptr;
  }
  info->type_support_ = fastdds_type;

  // 注册类型对象
  // Register the type object
  if (!rmw_fastrtps_shared_cpp::register_type_object(type_supports, type_name)) {
    // 如果注册失败，设置错误信息
    // Set error message if registration fails
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "failed to register type object with incompatible type %s", type_name.c_str());
    return nullptr;
  }

  // 创建监听器 (Create Listener)
  info->publisher_event_ = new (std::nothrow) RMWPublisherEvent(info);
  // 如果无法创建发布者事件，则设置错误消息并返回空指针 (If the publisher event cannot be created,
  // set the error message and return a nullptr)
  if (!info->publisher_event_) {
    RMW_SET_ERROR_MSG("create_publisher() could not create publisher event");
    return nullptr;
  }

  info->data_writer_listener_ = new (std::nothrow) CustomDataWriterListener(info->publisher_event_);
  // 如果无法创建发布者数据写入监听器，则设置错误消息并返回空指针 (If the publisher data writer
  // listener cannot be created, set the error message and return a nullptr)
  if (!info->data_writer_listener_) {
    RMW_SET_ERROR_MSG("create_publisher() could not create publisher data writer listener");
    return nullptr;
  }

  // 创建并注册主题 (Create and register Topic)
  eprosima::fastdds::dds::TopicQos topic_qos = dds_participant->get_default_topic_qos();
  // 如果无法获取主题 QoS，则设置错误消息并返回空指针 (If the topic QoS cannot be obtained, set the
  // error message and return a nullptr)
  if (!get_topic_qos(*qos_policies, topic_qos)) {
    RMW_SET_ERROR_MSG("create_publisher() failed setting topic QoS");
    return nullptr;
  }

  // 查找或创建主题 (Find or create topic)
  info->topic_ = participant_info->find_or_create_topic(
      topic_name_mangled, type_name, topic_qos, info->publisher_event_);
  // 如果无法创建主题，则设置错误消息并返回空指针 (If the topic cannot be created, set the error
  // message and return a nullptr)
  if (!info->topic_) {
    RMW_SET_ERROR_MSG("create_publisher() failed to create topic");
    return nullptr;
  }

  // 创建 DataWriter (Create DataWriter)

  // 如果用户通过环境变量 "FASTRTPS_DEFAULT_PROFILES_FILE" 定义了一个 XML
  // 文件，请尝试加载与主题名称匹配的数据写入器配置文件。 如果不存在这样的配置文件，则使用 Fast DDS
  // 默认的 QoS 设置。 (If the user defined an XML file via env "FASTRTPS_DEFAULT_PROFILES_FILE",
  // try to load datawriter which profile name matches with topic_name. If such profile does not
  // exist, then use the default Fast DDS QoS.)
  eprosima::fastdds::dds::DataWriterQos writer_qos = publisher->get_default_datawriter_qos();

  // 尝试加载具有主题名称的配置文件 (Try to load the profile with the topic name)
  // 不需要检查返回代码，因为如果配置文件不存在，QoS 已经是默认值了 (It does not need to check the
  // return code, as if the profile does not exist, the QoS is already the default)
  publisher->get_datawriter_qos_from_profile(topic_name, writer_qos);

  // 修改特定的 DataWriter Qos (Modify specific DataWriter Qos)
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

  // 从 RMW 获取 QoS (Get QoS from RMW)
  if (!get_datawriter_qos(
          *qos_policies, *type_supports->get_type_hash_func(type_supports), writer_qos)) {
    // 如果无法设置数据写入器的 QoS，则设置错误消息并返回空指针 (If the data writer QoS cannot be
    // set, set the error message and return a nullptr)
    RMW_SET_ERROR_MSG("create_publisher() failed setting data writer QoS");
    return nullptr;
  }

  // 创建一个 DataWriter，其遮罩用于启用监听器的 publication_matched 调用
  // Creates a DataWriter with a mask enabling publication_matched calls for the listener
  info->data_writer_ = publisher->create_datawriter(
      info->topic_, writer_qos, info->data_writer_listener_,
      eprosima::fastdds::dds::StatusMask::publication_matched());

  // 如果 DataWriter 创建失败，则设置错误信息并返回空指针
  // If the creation of DataWriter fails, set an error message and return nullptr
  if (!info->data_writer_) {
    RMW_SET_ERROR_MSG("create_publisher() could not create data writer");
    return nullptr;
  }

  // 将 StatusCondition 设置为 none，以防止通过 WaitSets 触发
  // Set the StatusCondition to none to prevent triggering via WaitSets
  info->data_writer_->get_statuscondition().set_enabled_statuses(
      eprosima::fastdds::dds::StatusMask::none());

  // 定义一个 lambda 函数，用于在作用域结束时删除 datawriter
  // Define a lambda function to delete datawriter when the scope ends
  auto cleanup_datawriter = rcpputils::make_scope_exit(
      [publisher, info]() { publisher->delete_datawriter(info->data_writer_); });

  // 创建 RMW GID
  // Create RMW GID
  info->publisher_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
      eprosima_fastrtps_identifier, info->data_writer_->guid());

  // 分配 publisher
  // Allocate publisher
  rmw_publisher_t *rmw_publisher = rmw_publisher_allocate();
  if (!rmw_publisher) {
    RMW_SET_ERROR_MSG("create_publisher() failed to allocate rmw_publisher");
    return nullptr;
  }
  // 定义一个 lambda 函数，用于在作用域结束时清理 rmw_publisher
  // Define a lambda function to cleanup rmw_publisher when the scope ends
  auto cleanup_rmw_publisher = rcpputils::make_scope_exit([rmw_publisher]() {
    rmw_free(const_cast<char *>(rmw_publisher->topic_name));
    rmw_publisher_free(rmw_publisher);
  });

  rmw_publisher->can_loan_messages = info->type_support_->is_plain();
  rmw_publisher->implementation_identifier = eprosima_fastrtps_identifier;
  rmw_publisher->data = info;

  // 为 rmw_publisher 的 topic_name 分配内存
  // Allocate memory for rmw_publisher's topic_name
  rmw_publisher->topic_name = static_cast<char *>(rmw_allocate(strlen(topic_name) + 1));
  if (!rmw_publisher->topic_name) {
    RMW_SET_ERROR_MSG("create_publisher() failed to allocate memory for rmw_publisher topic name");
    return nullptr;
  }
  // 将 topic_name 复制到 rmw_publisher 的 topic_name 中
  // Copy the topic_name into rmw_publisher's topic_name
  memcpy(const_cast<char *>(rmw_publisher->topic_name), topic_name, strlen(topic_name) + 1);

  rmw_publisher->options = *publisher_options;

  // 取消清理操作
  // Cancel cleanup operations
  cleanup_rmw_publisher.cancel();
  cleanup_datawriter.cancel();
  cleanup_info.cancel();

  // 记录追踪点
  // Record tracepoint
  TRACEPOINT(
      rmw_publisher_init, static_cast<const void *>(rmw_publisher), info->publisher_gid.data);

  return rmw_publisher;
}
