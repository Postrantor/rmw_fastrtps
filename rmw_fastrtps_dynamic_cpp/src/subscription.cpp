// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#include "rmw_fastrtps_shared_cpp/subscription.hpp"

#include <string>
#include <utility>

#include "fastdds/dds/domain/DomainParticipant.hpp"
#include "fastdds/dds/subscriber/Subscriber.hpp"
#include "fastdds/dds/subscriber/qos/DataReaderQos.hpp"
#include "fastdds/dds/topic/Topic.hpp"
#include "fastdds/dds/topic/TopicDescription.hpp"
#include "fastdds/dds/topic/TypeSupport.hpp"
#include "fastdds/dds/topic/qos/TopicQos.hpp"
#include "fastdds/rtps/resources/ResourceManagement.h"
#include "fastrtps/participant/Participant.h"
#include "fastrtps/subscriber/Subscriber.h"
#include "fastrtps/xmlparser/XMLProfileManager.h"
#include "rcpputils/scope_exit.hpp"
#include "rcutils/error_handling.h"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/validate_full_topic_name.h"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_dynamic_cpp/subscription.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_subscriber_info.hpp"
#include "rmw_fastrtps_shared_cpp/names.hpp"
#include "rmw_fastrtps_shared_cpp/namespace_prefix.hpp"
#include "rmw_fastrtps_shared_cpp/qos.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/utils.hpp"
#include "rosidl_runtime_c/type_hash.h"
#include "type_support_common.hpp"
#include "type_support_registry.hpp"

using PropertyPolicyHelper = eprosima::fastrtps::rtps::PropertyPolicyHelper;

namespace rmw_fastrtps_dynamic_cpp {

/**
 * @brief 创建一个订阅者 (Create a subscription)
 *
 * @param[in] participant_info 参与者信息，包含了与 ROS2 通信所需的数据结构 (Participant
 * information, contains data structures required for communication with ROS2)
 * @param[in] type_supports 消息类型支持，用于序列化和反序列化消息 (Message type support, used for
 * serialization and deserialization of messages)
 * @param[in] topic_name 要订阅的主题名称 (The name of the topic to subscribe to)
 * @param[in] qos_policies QoS 策略，用于指定订阅者的服务质量 (QoS policies, used to specify the
 * quality of service for the subscriber)
 * @param[in] subscription_options 订阅选项，包括订阅回调和其他设置 (Subscription options, includes
 * subscription callback and other settings)
 * @param[in] keyed 是否为键控消息，这将影响序列化和反序列化过程 (Whether the message is keyed,
 * which will affect the serialization and deserialization process)
 *
 * @return rmw_subscription_t* 创建的订阅者指针，如果创建失败则返回 nullptr (Pointer to the created
 * subscription, returns nullptr if creation fails)
 */
rmw_subscription_t *create_subscription(
    CustomParticipantInfo *participant_info,
    const rosidl_message_type_support_t *type_supports,
    const char *topic_name,
    const rmw_qos_profile_t *qos_policies,
    const rmw_subscription_options_t *subscription_options,
    bool keyed) {
  // 检查是否可以返回空指针错误，这是一种错误处理机制 (Check if it can return with a nullptr error,
  // this is an error handling mechanism)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(nullptr);

  // 检查输入参数 (Check input parameters)
  // 确保 participant_info 不为空指针 (Ensure participant_info is not a null pointer)
  RMW_CHECK_ARGUMENT_FOR_NULL(participant_info, nullptr);
  // 确保 type_supports 不为空指针 (Ensure type_supports is not a null pointer)
  RMW_CHECK_ARGUMENT_FOR_NULL(type_supports, nullptr);
  // 确保 topic_name 不为空指针 (Ensure topic_name is not a null pointer)
  RMW_CHECK_ARGUMENT_FOR_NULL(topic_name, nullptr);
  // 如果 topic_name 长度为0，设置错误消息并返回空指针 (If the length of topic_name is 0, set an
  // error message and return a null pointer)
  if (0 == strlen(topic_name)) {
    RMW_SET_ERROR_MSG("create_subscription() called with an empty topic_name argument");
    return nullptr;
  }
  // 确保 qos_policies 不为空指针 (Ensure qos_policies is not a null pointer)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos_policies, nullptr);
  // 如果避免 ROS 命名空间约定为 false，则验证主题名称 (If avoid_ros_namespace_conventions is false,
  // validate the topic name)
  if (!qos_policies->avoid_ros_namespace_conventions) {
    int validation_result = RMW_TOPIC_VALID;
    rmw_ret_t ret = rmw_validate_full_topic_name(topic_name, &validation_result, nullptr);
    // 如果验证结果不是 RMW_RET_OK，则返回空指针 (If the validation result is not RMW_RET_OK, return
    // a null pointer)
    if (RMW_RET_OK != ret) {
      return nullptr;
    }
    // 如果主题验证结果不是 RMW_TOPIC_VALID，则设置错误消息并返回空指针 (If the topic validation
    // result is not RMW_TOPIC_VALID, set an error message and return a null pointer)
    if (RMW_TOPIC_VALID != validation_result) {
      const char *reason = rmw_full_topic_name_validation_result_string(validation_result);
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "create_subscription() called with invalid topic name: %s", reason);
      return nullptr;
    }
  }
  // 确保 subscription_options 不为空指针 (Ensure subscription_options is not a null pointer)
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription_options, nullptr);

  // 检查 RMW QoS (Check RMW QoS)
  // 如果 QoS 策略无效，设置错误消息并返回空指针 (If the QoS policies are invalid, set an error
  // message and return a null pointer)
  if (!is_valid_qos(*qos_policies)) {
    RMW_SET_ERROR_MSG("create_subscription() called with invalid QoS");
    return nullptr;
  }

  /////
  /// 获取 RMW 类型支持 (Get RMW Type Support)
  const rosidl_message_type_support_t *type_support =
      // 从类型支持列表中获取指定标识符的消息类型支持句柄
      // (Get the message type support handle with the specified identifier from the type supports
      // list)
      get_message_typesupport_handle(type_supports, rosidl_typesupport_introspection_c__identifier);
  if (!type_support) {
    // 获取先前的错误字符串 (Get the previous error string)
    rcutils_error_string_t prev_error_string = rcutils_get_error_string();
    // 重置错误 (Reset the error)
    rcutils_reset_error();
    // 以 C++ 标识符重新尝试获取消息类型支持句柄
    // (Retry getting the message type support handle with the C++ identifier)
    type_support = get_message_typesupport_handle(
        type_supports, rosidl_typesupport_introspection_cpp::typesupport_identifier);
    if (!type_support) {
      // 获取当前错误字符串 (Get the current error string)
      rcutils_error_string_t error_string = rcutils_get_error_string();
      // 重置错误 (Reset the error)
      rcutils_reset_error();
      // 设置错误消息，包括先前和当前的错误字符串
      // (Set the error message including both the previous and current error strings)
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "Type support not from this implementation. Got:\n"
          "    %s\n"
          "    %s\n"
          "while fetching it",
          prev_error_string.str, error_string.str);
      // 返回空指针 (Return a nullptr)
      return nullptr;
    }
  }

  // 创建互斥锁，保护参与者信息中的实体创建过程
  // (Create a lock_guard to protect the entity creation process in participant_info)
  std::lock_guard<std::mutex> lck(participant_info->entity_creation_mutex_);

  /////
  // 创建主题和类型名称
  // Create Topic and Type names
  std::string type_name =
      _create_type_name(type_support->data, type_support->typesupport_identifier);
  auto topic_name_mangled =
      _create_topic_name(qos_policies, ros_topic_prefix, topic_name).to_string();

  // 定义 eprosima::fastdds::dds::TypeSupport 变量 fastdds_type
  // Define eprosima::fastdds::dds::TypeSupport variable fastdds_type
  eprosima::fastdds::dds::TypeSupport fastdds_type;

  // 定义 eprosima::fastdds::dds::TopicDescription 指针 des_topic
  // Define eprosima::fastdds::dds::TopicDescription pointer des_topic
  eprosima::fastdds::dds::TopicDescription *des_topic;

  // 调用 find_and_check_topic_and_type 函数，检查现有的主题和类型是否存在并且兼容
  // Call the find_and_check_topic_and_type function to check if the existing topic and type are
  // present and compatible
  if (!rmw_fastrtps_shared_cpp::find_and_check_topic_and_type(
          participant_info, topic_name_mangled, type_name, &des_topic, &fastdds_type)) {
    // 如果主题和类型不兼容，则设置错误消息并返回 nullptr
    // If the topic and type are not compatible, set the error message and return nullptr
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "create_subscription() called with existing topic name %s with incompatible type %s",
        topic_name_mangled.c_str(), type_name.c_str());
    return nullptr;
  }

  /////
  // 获取 Fast DDS 域参与者和订阅者指针
  // Get the Fast DDS DomainParticipant/Subscriber pointer
  eprosima::fastdds::dds::DomainParticipant *dds_participant = participant_info->participant_;
  eprosima::fastdds::dds::Subscriber *subscriber = participant_info->subscriber_;

  /////
  /**
   * @brief 创建自定义订阅者结构体 (Create the custom Subscriber struct)
   *
   * @param[out] info 自定义订阅者信息指针 (Pointer to CustomSubscriberInfo)
   * @param[in] participant_info 包含参与者和订阅者信息的结构体指针 (Pointer to a structure
   * containing participant and subscriber information)
   * @return nullptr 如果分配失败 (nullptr if allocation fails)
   */

  // 使用 new (std::nothrow) 为 CustomSubscriberInfo 分配内存
  // Allocate memory for CustomSubscriberInfo using new (std::nothrow)
  auto info = new (std::nothrow) CustomSubscriberInfo();

  // 检查内存分配是否成功
  // Check if memory allocation was successful
  if (!info) {
    // 设置错误消息
    // Set error message
    RMW_SET_ERROR_MSG("create_subscription() failed to allocate CustomSubscriberInfo");
    // 返回 nullptr 表示分配失败
    // Return nullptr to indicate allocation failure
    return nullptr;
  }

  // 创建一个清理函数，用于在作用域结束时自动执行
  // Create a cleanup function that will be executed automatically when the scope ends
  auto cleanup_info = rcpputils::make_scope_exit([info, participant_info]() {
    // 移除主题和类型
    // Remove topic and type
    rmw_fastrtps_shared_cpp::remove_topic_and_type(
        participant_info, info->subscription_event_, info->topic_, info->type_support_);
    // 删除订阅事件
    // Delete subscription event
    delete info->subscription_event_;
    // 删除数据读取监听器
    // Delete data reader listener
    delete info->data_reader_listener_;
    // 删除 info 结构体
    // Delete info structure
    delete info;
  });

  /////
  // 创建 Type Support 结构体 (Create the Type Support struct)
  TypeSupportRegistry &type_registry = TypeSupportRegistry::get_instance();

  // 获取消息类型支持实现 (Get the message type support implementation)
  auto type_support_impl = type_registry.get_message_type_support(type_support);
  if (!type_support_impl) {
    // 设置错误信息 (Set error message)
    RMW_SET_ERROR_MSG("create_subscription() failed to get message_type_support");
    return nullptr;
  }

  // 返回类型支持并在作用域结束时释放资源 (Return type support and release resources when scope
  // ends)
  auto return_type_support = rcpputils::make_scope_exit([&type_registry, type_support]() {
    type_registry.return_message_type_support(type_support);
  });

  // 设置类型支持标识符 (Set the type support identifier)
  info->typesupport_identifier_ = type_support->typesupport_identifier;

  // 设置类型支持实现 (Set the type support implementation)
  info->type_support_impl_ = type_support_impl;

  // 如果 fastdds_type 不存在 (If fastdds_type does not exist)
  if (!fastdds_type) {
    // 创建一个新的 TypeSupportProxy (Create a new TypeSupportProxy)
    auto tsupport = new (std::nothrow) TypeSupportProxy(type_support_impl);
    if (!tsupport) {
      // 设置错误信息 (Set error message)
      RMW_SET_ERROR_MSG("create_subscription() failed to allocate TypeSupportProxy");
      return nullptr;
    }

    // 将所有权转移给 fastdds_type (Transfer ownership to fastdds_type)
    fastdds_type.reset(tsupport);
  }

  // 检查是否请求了带键值的主题和非键值类型 (Check if keyed topic with non-keyed type is requested)
  if (keyed && !fastdds_type->m_isGetKeyDefined) {
    // 设置错误信息 (Set error message)
    RMW_SET_ERROR_MSG("create_subscription() requested a keyed topic with a non-keyed type");
    return nullptr;
  }

  // 注册类型 (Register the type)
  if (ReturnCode_t::RETCODE_OK != fastdds_type.register_type(dds_participant)) {
    // 设置错误信息 (Set error message)
    RMW_SET_ERROR_MSG("create_subscription() failed to register type");
    return nullptr;
  }

  // 设置类型支持 (Set the type support)
  info->type_support_ = fastdds_type;

  /////
  /**
   * @brief 创建监听器 (Create Listener)
   *
   * @param[in] info 传入的订阅信息结构体指针，用于存储订阅相关的数据 (Pointer to the subscription
   * information structure for storing subscription-related data)
   */
  // 创建 RMWSubscriptionEvent 实例 (Create an instance of RMWSubscriptionEvent)
  info->subscription_event_ = new (std::nothrow) RMWSubscriptionEvent(info);
  if (!info->subscription_event_) {
    // 如果创建失败，设置错误信息并返回空指针 (If creation fails, set error message and return
    // nullptr)
    RMW_SET_ERROR_MSG("create_subscription() could not create subscription event");
    return nullptr;
  }

  // 创建 CustomDataReaderListener 实例 (Create an instance of CustomDataReaderListener)
  info->data_reader_listener_ =
      new (std::nothrow) CustomDataReaderListener(info->subscription_event_);
  if (!info->data_reader_listener_) {
    // 如果创建失败，设置错误信息并返回空指针 (If creation fails, set error message and return
    // nullptr)
    RMW_SET_ERROR_MSG("create_subscription() could not create subscription data reader listener");
    return nullptr;
  }

  /////
  /**
   * @brief 创建并注册 Topic，并为代码段添加双语（中文、英文）详细注释
   *
   * @param[in] qos_policies QoS 策略指针，用于获取和设置 Topic 的 QoS
   * @param[out] des_topic 输出参数，存储创建的主题
   * @param[in] dds_participant DDS 参与者指针，用于获取默认的 Topic QoS
   * @param[in] participant_info 参与者信息指针，用于查找或创建主题
   * @param[in] topic_name_mangled 主题名称（经过处理）
   * @param[in] type_name 类型名称
   * @param[out] info->topic_ 存储创建的主题
   * @param[out] info->subscription_event_ 存储订阅事件
   *
   * @return 如果成功创建并注册 Topic，则返回 true；否则返回 false
   */
  // 创建并注册 Topic (Create and register Topic)
  eprosima::fastdds::dds::TopicQos topic_qos = dds_participant->get_default_topic_qos();

  // 获取 Topic 的 QoS 设置，如果失败则设置错误信息并返回空指针
  // Get the Topic QoS settings, if failed, set error message and return nullptr
  if (!get_topic_qos(*qos_policies, topic_qos)) {
    RMW_SET_ERROR_MSG("create_publisher() failed setting topic QoS");
    return nullptr;
  }

  // 在参与者信息中查找或创建主题，并将其与订阅事件关联
  // Find or create a topic in participant info and associate it with subscription event
  info->topic_ = participant_info->find_or_create_topic(
      topic_name_mangled, type_name, topic_qos, info->subscription_event_);

  // 如果主题创建失败，则设置错误信息并返回空指针
  // If topic creation failed, set error message and return nullptr
  if (!info->topic_) {
    RMW_SET_ERROR_MSG("create_subscription() failed to create topic");
    return nullptr;
  }

  // 将创建的主题赋值给 des_topic 变量
  // Assign the created topic to the des_topic variable
  des_topic = info->topic_;

  /////
  // 创建 DataReader (Create DataReader)
  // 如果用户通过环境变量 "FASTRTPS_DEFAULT_PROFILES_FILE" 定义了一个 XML 文件，尝试加载与
  // topic_name 匹配的 profile 名称的 datareader。 如果此类配置文件不存在，则使用默认的 Fast DDS
  // QoS。 (If the user defined an XML file via env "FASTRTPS_DEFAULT_PROFILES_FILE", try to load
  // datareader which profile name matches with topic_name. If such profile does not exist,
  // then use the default Fast DDS QoS.)
  eprosima::fastdds::dds::DataReaderQos reader_qos = subscriber->get_default_datareader_qos();

  // 尝试加载具有 topic 名称的配置文件 (Try to load the profile with the topic name)
  // 无需检查返回代码，因为如果配置文件不存在，QoS 已经是默认值
  // (It does not need to check the return code, as if the profile does not exist,
  // the QoS is already the default)
  subscriber->get_datareader_qos_from_profile(topic_name, reader_qos);

  // 如果不保留中间件默认的 QoS (if not leaving middleware default qos)
  if (!participant_info->leave_middleware_default_qos) {
    // 设置历史记录内存策略 (Set history memory policy)
    reader_qos.endpoint().history_memory_policy =
        eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    // 关闭数据共享 (Turn off data sharing)
    reader_qos.data_sharing().off();
  }

  // 获取 datareader 的 QoS (Get datareader QoS)
  if (!get_datareader_qos(
          *qos_policies, *type_supports->get_type_hash_func(type_supports), reader_qos)) {
    // 设置错误消息 (Set error message)
    RMW_SET_ERROR_MSG("create_subscription() failed setting data reader QoS");
    // 返回空指针 (Return nullptr)
    return nullptr;
  }

  // 存储原始 QoS (Store the original QoS)
  eprosima::fastdds::dds::DataReaderQos original_qos = reader_qos;

  // 根据订阅选项要求唯一的网络流端点 (Switch based on subscription options requiring unique network
  // flow endpoints)
  switch (subscription_options->require_unique_network_flow_endpoints) {
    default:
    case RMW_UNIQUE_NETWORK_FLOW_ENDPOINTS_SYSTEM_DEFAULT:
    case RMW_UNIQUE_NETWORK_FLOW_ENDPOINTS_NOT_REQUIRED:
      // 不需要唯一的网络流端点。我们将决策留给 XML 配置文件。
      // (Unique network flow endpoints not required. We leave the decision to the XML profile.)
      break;

    case RMW_UNIQUE_NETWORK_FLOW_ENDPOINTS_OPTIONALLY_REQUIRED:
    case RMW_UNIQUE_NETWORK_FLOW_ENDPOINTS_STRICTLY_REQUIRED:
      // 确保我们请求唯一的网络流端点
      // (Ensure we request unique network flow endpoints)
      if (nullptr == PropertyPolicyHelper::find_property(
                         reader_qos.properties(), "fastdds.unique_network_flows")) {
        reader_qos.properties().properties().emplace_back("fastdds.unique_network_flows", "");
      }
      break;
  }

  // 使用订阅者名称创建 DataReader（以免更改名称策略）
  // (Creates DataReader (with subscriber name to not change name policy))
  info->data_reader_ = subscriber->create_datareader(
      des_topic, reader_qos, info->data_reader_listener_,
      eprosima::fastdds::dds::StatusMask::subscription_matched());

  // 如果 DataReader 未创建且需要唯一的网络流端点（可选）
  // (If DataReader not created and unique network flow endpoints are optionally required)
  if (!info->data_reader_ && (RMW_UNIQUE_NETWORK_FLOW_ENDPOINTS_OPTIONALLY_REQUIRED ==
                              subscription_options->require_unique_network_flow_endpoints)) {
    // 使用原始 QoS 创建 DataReader
    // (Create DataReader with original QoS)
    info->data_reader_ = subscriber->create_datareader(
        des_topic, original_qos, info->data_reader_listener_,
        eprosima::fastdds::dds::StatusMask::subscription_matched());
  }

  // 如果未创建 DataReader (If DataReader not created)
  if (!info->data_reader_) {
    // 设置错误消息 (Set error message)
    RMW_SET_ERROR_MSG("create_subscription() could not create data reader");
    // 返回空指针 (Return nullptr)
    return nullptr;
  }

  // 初始化 DataReader 的 StatusCondition，以便在有新数据可用时得到通知
  // (Initialize DataReader's StatusCondition to be notified when new data is available)
  info->data_reader_->get_statuscondition().set_enabled_statuses(
      eprosima::fastdds::dds::StatusMask::data_available());

  // lambda 函数删除 datareader (lambda to delete datareader)
  auto cleanup_datareader = rcpputils::make_scope_exit(
      [subscriber, info]() { subscriber->delete_datareader(info->data_reader_); });

  /////
  // 创建 RMW GID (Create RMW GID)
  info->subscription_gid_ = rmw_fastrtps_shared_cpp::create_rmw_gid(
      eprosima_fastrtps_identifier, info->data_reader_->guid());

  // 为 rmw_subscription_t 分配内存 (Allocate memory for rmw_subscription_t)
  rmw_subscription_t *rmw_subscription = rmw_subscription_allocate();
  if (!rmw_subscription) {
    // 设置错误消息，表示分配订阅失败 (Set error message indicating subscription allocation failed)
    RMW_SET_ERROR_MSG("create_subscription() failed to allocate subscription");
    return nullptr;
  }

  // 创建清理 rmw_subscription 的作用域退出对象 (Create a scope exit object for cleaning up
  // rmw_subscription)
  auto cleanup_rmw_subscription = rcpputils::make_scope_exit([rmw_subscription]() {
    rmw_free(const_cast<char *>(rmw_subscription->topic_name));
    rmw_subscription_free(rmw_subscription);
  });

  // 设置实现标识符 (Set implementation identifier)
  rmw_subscription->implementation_identifier = eprosima_fastrtps_identifier;

  // 将 info 数据赋值给 rmw_subscription (Assign info data to rmw_subscription)
  rmw_subscription->data = info;

  // 为订阅主题名称分配内存 (Allocate memory for subscription topic name)
  rmw_subscription->topic_name =
      reinterpret_cast<const char *>(rmw_allocate(strlen(topic_name) + 1));
  if (!rmw_subscription->topic_name) {
    // 设置错误消息，表示分配订阅主题名称内存失败 (Set error message indicating subscription topic
    // name memory allocation failed)
    RMW_SET_ERROR_MSG(
        "create_subscription() failed to allocate memory for subscription topic name");
    return nullptr;
  }

  // 复制主题名称到 rmw_subscription (Copy topic name to rmw_subscription)
  memcpy(const_cast<char *>(rmw_subscription->topic_name), topic_name, strlen(topic_name) + 1);

  // 将订阅选项赋值给 rmw_subscription (Assign subscription options to rmw_subscription)
  rmw_subscription->options = *subscription_options;

  // 初始化 loans 的订阅 (Initialize subscription for loans)
  rmw_fastrtps_shared_cpp::__init_subscription_for_loans(rmw_subscription);

  // TODO(iuhilnehc-ynos): 在确认 rmw_fastrtps_cpp 后进行更新 (Update after rmw_fastrtps_cpp is
  // confirmed)
  rmw_subscription->is_cft_enabled = false;

  // 取消清理操作 (Cancel cleanup operations)
  cleanup_rmw_subscription.cancel();
  cleanup_datareader.cancel();
  return_type_support.cancel();
  cleanup_info.cancel();

  // 返回创建的 rmw_subscription (Return the created rmw_subscription)
  return rmw_subscription;
}

}  // namespace rmw_fastrtps_dynamic_cpp
