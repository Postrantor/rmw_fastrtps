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

#include <rosidl_dynamic_typesupport/identifier.h>

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
#include "fastrtps/types/DynamicType.h"
#include "fastrtps/types/DynamicTypePtr.h"
#include "rcpputils/scope_exit.hpp"
#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/macros.h"
#include "rcutils/strdup.h"
#include "rmw/allocators.h"
#include "rmw/dynamic_message_type_support.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/validate_full_topic_name.h"
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_cpp/subscription.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_subscriber_info.hpp"
#include "rmw_fastrtps_shared_cpp/names.hpp"
#include "rmw_fastrtps_shared_cpp/namespace_prefix.hpp"
#include "rmw_fastrtps_shared_cpp/qos.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/utils.hpp"
#include "rosidl_dynamic_typesupport/dynamic_message_type_support_struct.h"
#include "tracetools/tracetools.h"
#include "type_support_common.hpp"

using PropertyPolicyHelper = eprosima::fastrtps::rtps::PropertyPolicyHelper;

namespace rmw_fastrtps_cpp {

/**
 * @brief 创建一个动态订阅 (Create a dynamic subscription)
 *
 * @param[in] participant_info 自定义参与者信息 (Custom participant information)
 * @param[in] type_support 消息类型支持 (Message type support)
 * @param[in] topic_name 订阅的主题名称 (Topic name to subscribe)
 * @param[in] qos_policies QoS策略 (QoS policies)
 * @param[in] subscription_options 订阅选项 (Subscription options)
 * @param[in] keyed 是否为键控消息 (Whether the message is keyed)
 * @return rmw_subscription_t* 动态订阅指针 (Pointer to the dynamic subscription)
 */
rmw_subscription_t *__create_dynamic_subscription(
    CustomParticipantInfo *participant_info,
    const rosidl_message_type_support_t *type_support,
    const char *topic_name,
    const rmw_qos_profile_t *qos_policies,
    const rmw_subscription_options_t *subscription_options,
    bool keyed);

/**
 * @brief 创建一个订阅 (Create a subscription)
 *
 * @param[in] participant_info 自定义参与者信息 (Custom participant information)
 * @param[in] type_supports 消息类型支持 (Message type supports)
 * @param[in] topic_name 订阅的主题名称 (Topic name to subscribe)
 * @param[in] qos_policies QoS策略 (QoS policies)
 * @param[in] subscription_options 订阅选项 (Subscription options)
 * @param[in] keyed 是否为键控消息 (Whether the message is keyed)
 * @return rmw_subscription_t* 订阅指针 (Pointer to the subscription)
 */
rmw_subscription_t *__create_subscription(
    CustomParticipantInfo *participant_info,
    const rosidl_message_type_support_t *type_supports,
    const char *topic_name,
    const rmw_qos_profile_t *qos_policies,
    const rmw_subscription_options_t *subscription_options,
    bool keyed);

/**
 * @brief 创建一个订阅者 (Create a subscription)
 *
 * @param[in] participant_info 自定义参与者信息 (Custom participant info)
 * @param[in] type_supports 消息类型支持 (Message type supports)
 * @param[in] topic_name 主题名称 (Topic name)
 * @param[in] qos_policies QoS策略 (QoS policies)
 * @param[in] subscription_options 订阅选项 (Subscription options)
 * @param[in] keyed 是否为键控 (Whether it is keyed or not)
 * @return rmw_subscription_t* 创建的订阅指针 (Pointer to the created subscription)
 */
rmw_subscription_t *create_subscription(
    CustomParticipantInfo *participant_info,
    const rosidl_message_type_support_t *type_supports,
    const char *topic_name,
    const rmw_qos_profile_t *qos_policies,
    const rmw_subscription_options_t *subscription_options,
    bool keyed) {
  // 检查输入参数 (Check input parameters)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(nullptr);

  // 验证传入的参数是否为空 (Verify that the passed arguments are not null)
  RMW_CHECK_ARGUMENT_FOR_NULL(participant_info, nullptr);
  RMW_CHECK_ARGUMENT_FOR_NULL(type_supports, nullptr);
  RMW_CHECK_ARGUMENT_FOR_NULL(topic_name, nullptr);
  // 检查主题名是否为空字符串 (Check if the topic name is an empty string)
  if (0 == strlen(topic_name)) {
    RMW_SET_ERROR_MSG("create_subscription() called with an empty topic_name argument");
    return nullptr;
  }
  RMW_CHECK_ARGUMENT_FOR_NULL(qos_policies, nullptr);
  // 如果不遵循ROS命名空间约定，验证主题名 (Validate the topic name if not following ROS namespace
  // conventions)
  if (!qos_policies->avoid_ros_namespace_conventions) {
    int validation_result = RMW_TOPIC_VALID;
    rmw_ret_t ret = rmw_validate_full_topic_name(topic_name, &validation_result, nullptr);
    if (RMW_RET_OK != ret) {
      return nullptr;
    }
    if (RMW_TOPIC_VALID != validation_result) {
      const char *reason = rmw_full_topic_name_validation_result_string(validation_result);
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "create_subscription() called with invalid topic name: %s", reason);
      return nullptr;
    }
  }
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription_options, nullptr);

  // 检查RMW QoS是否有效 (Check if the RMW QoS is valid)
  if (!is_valid_qos(*qos_policies)) {
    RMW_SET_ERROR_MSG("create_subscription() called with invalid QoS");
    return nullptr;
  }

  // 获取RMW类型支持 (Get RMW Type Support)
  const rosidl_message_type_support_t *type_support =
      get_message_typesupport_handle(type_supports, rosidl_get_dynamic_typesupport_identifier());
  // 如果找到动态类型支持，创建动态订阅 (If dynamic type support is found, create a dynamic
  // subscription)
  if (type_support) {
    return __create_dynamic_subscription(
        participant_info, type_support, topic_name, qos_policies, subscription_options, keyed);
  }
  // 如果未找到类型支持，rosidl_typesupport会发出错误消息 (In case it fails to find,
  // rosidl_typesupport emits an error message)
  rcutils_reset_error();

  // 创建一个订阅 (Create a subscription)
  return __create_subscription(
      participant_info, type_supports, topic_name, qos_policies, subscription_options, keyed);
}

// =================================================================================================
// CREATE RUNTIME SUBSCRIPTION
// =================================================================================================
/**
 * @brief 创建一个动态订阅 (Create a dynamic subscription)
 *
 * @param[in] participant_info 自定义参与者信息 (Custom participant information)
 * @param[in] type_support 消息类型支持 (Message type support)
 * @param[in] topic_name 主题名称 (Topic name)
 * @param[in] qos_policies QoS策略 (QoS policies)
 * @param[in] subscription_options 订阅选项 (Subscription options)
 * @param[in] keyed 是否为键值类型 (Whether it is a keyed type)
 * @return rmw_subscription_t* 动态订阅指针 (Pointer to the dynamic subscription)
 */
rmw_subscription_t *__create_dynamic_subscription(
    CustomParticipantInfo *participant_info,
    const rosidl_message_type_support_t *type_support,
    const char *topic_name,
    const rmw_qos_profile_t *qos_policies,
    const rmw_subscription_options_t *subscription_options,
    bool keyed) {
  // 注意(methylDragon): 动态类型延迟情况是 !! 不支持的 !!
  //                    这是因为在没有类型的情况下创建订阅很困难。
  //                    需要在其他地方进行太多重构以支持延迟...
  // NOTE(methylDragon): The dynamic type deferred case is !! NOT SUPPORTED !!
  //                     This is because it's difficult as-is to create a subscription without
  //                     already having the type. Too much restructuring is needed elsewhere to
  //                     support deferral...

  /**
   * @brief 检查类型支持是否来自于此实现并创建订阅 (Check if the type support is from this
   * implementation and create a subscription)
   *
   * @param[in] type_support 消息类型支持 (Message type support)
   * @param[in] participant_info 自定义参与者信息 (Custom participant information)
   * @return rmw_subscription_t* 动态订阅指针或nullptr (Pointer to the dynamic subscription or
   * nullptr)
   */
  // 检查类型支持是否来自于此实现
  // Check if the type support is from this implementation
  if (type_support->typesupport_identifier != rosidl_get_dynamic_typesupport_identifier()) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Type support not from this implementation. Got:\n"
        "    %s, but expected\n"
        "    %s\n"
        "while fetching it",
        type_support->typesupport_identifier, rosidl_get_dynamic_typesupport_identifier());
    return nullptr;
  }

  // 注意(methylDragon): 内部是非const的，所以从技术上讲这不是const正确的
  // NOTE(methylDragon): The internals are non-const, so this is technically not const correct
  auto ts_impl =
      static_cast<const rosidl_dynamic_message_type_support_impl_t *>(type_support->data);

  std::lock_guard<std::mutex> lck(participant_info->entity_creation_mutex_);

  /////
  // 查找并检查现有主题和类型
  // Find and check existing topic and type

  // 创建主题和类型名称
  // Create Topic and Type names
  auto dyn_type_ptr = eprosima::fastrtps::types::DynamicType_ptr(
      *static_cast<eprosima::fastrtps::types::DynamicType_ptr *>(
          ts_impl->dynamic_message_type->impl.handle));

  // 检查我们是否需要将名称分为命名空间和类型名称
  // Check if we need to split the name into namespace and type name
  std::string type_name = dyn_type_ptr->get_name();

  int occurrences = 0;
  std::string::size_type pos = 0;
  std::string::size_type last_pos = 0;
  while ((pos = type_name.find("::", pos)) != std::string::npos) {
    ++occurrences;
    pos += 2;  // Length of "::"
    last_pos = pos;
  }

  // 名称不是带有命名空间的: <package_name>::<msg/srv>::<type_name>
  // Name is not namespaced: <package_name>::<msg/srv>::<type_name>
  if (occurrences != 2) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "create_subscription() called for runtime subscription for invalid message type name %s. "
        "Type name should be <package_name>::<msg/srv>::<type_name>",
        type_name.c_str());
    return nullptr;
  }

  // 创建类型名称
  // Create the type name
  type_name = _create_type_name(
      type_name.substr(0, last_pos - 2),                           // package
      type_name.substr(last_pos, type_name.length() - last_pos));  // name

  // 创建主题名称
  // Create the topic name
  auto topic_name_mangled =
      _create_topic_name(qos_policies, ros_topic_prefix, topic_name).to_string();

  // 定义 Fast-DDS 类型支持和主题描述变量
  // Define Fast-DDS TypeSupport and TopicDescription variables
  eprosima::fastdds::dds::TypeSupport fastdds_type;
  eprosima::fastdds::dds::TopicDescription *des_topic = nullptr;

  // 查找并检查主题和类型是否存在和兼容
  // Find and check if topic and type exist and are compatible
  if (!rmw_fastrtps_shared_cpp::find_and_check_topic_and_type(
          participant_info, topic_name_mangled,
          type_name,  // Should be <msg_package>::msg::dds_::<msg_name>_
          &des_topic, &fastdds_type)) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "create_subscription() called for existing topic name %s with incompatible type %s",
        topic_name_mangled.c_str(), type_name.c_str());
    return nullptr;
  }

  // 获取参与者和订阅者
  // Get Participant and Subscriber
  eprosima::fastdds::dds::DomainParticipant *dds_participant = participant_info->participant_;
  eprosima::fastdds::dds::Subscriber *subscriber = participant_info->subscriber_;

  // 创建自定义订阅者结构（info）
  // Create the custom Subscriber struct (info)
  auto info = new (std::nothrow) CustomSubscriberInfo();
  if (!info) {
    RMW_SET_ERROR_MSG("create_subscription() failed to allocate CustomSubscriberInfo");
    return nullptr;
  }

  // 设置清理函数，用于在出错时释放资源
  // Set up cleanup function for releasing resources in case of error
  auto cleanup_info = rcpputils::make_scope_exit([info, participant_info]() {
    rmw_fastrtps_shared_cpp::remove_topic_and_type(
        participant_info, info->subscription_event_, info->topic_, info->type_support_);
    delete info->subscription_event_;
    delete info->data_reader_listener_;
    delete info;
  });

  // 为自定义订阅者结构设置类型支持信息
  // Set type support information for the custom subscriber struct
  info->typesupport_identifier_ = type_support->typesupport_identifier;
  info->type_support_impl_ = ts_impl;

  /// \brief 创建 Type Support 结构体 (Create the Type Support struct)
  ///
  if (!fastdds_type) {
    // 将所有权转移到 fastdds_type (Transfer ownership to fastdds_type)
    //
    // 注意（methylDragon）：我们不能仅使用 DynamicType_ptr（转换为 DynamicPubSubType）！
    //                     原因是 DynamicPubSubType 仅支持序列化和反序列化到 DynamicData。
    //                     但我们希望能够首先直接获取序列化消息，
    //                     然后将其转换。否则，我们无法让非动态类型订阅处理动态类型。
    //
    //                     不过，稍后我们仍然需要一个 DynamicPubSubType（从 DynamicType_ptr 构造）
    //                     以将 CDR 缓冲区转换为 DynamicData...
    //
    // NOTE(methylDragon): We cannot just use a DynamicType_ptr (which converts to a
    //                     DynamicPubSubType)!!
    //
    //                     The reason being, the DynamicPubSubType only supports serialization and
    //                     deserialization to and from DynamicData.
    //                     But we want to be able to first get a serialized message directly,
    //                     THEN convert it. Otherwise we can't let a non-dynamic type subscription
    //                     handle dynamic types.
    //
    //                     We will still need a DynamicPubSubType later on (constructed from a
    //                     DynamicType_ptr) to convert the CDR buffer to a DynamicData, however...
    // fastdds_type.reset(dyn_type_ptr);

    // 创建 TypeSupport_cpp 对象，注意这里不是 MessageTypeSupport_cpp (Create a TypeSupport_cpp
    // object, not MessageTypeSupport_cpp)
    auto tsupport = new (std::nothrow) TypeSupport_cpp();  // NOT MessageTypeSupport_cpp!!!
    if (!tsupport) {
      // 设置错误消息：创建订阅时分配 TypeSupport 失败 (Set error message: Failed to allocate
      // TypeSupport when creating subscription)
      RMW_SET_ERROR_MSG("create_subscription() failed to allocate TypeSupport");
      return nullptr;
    }

    // 因为我们使用的是 TypeSupport_cpp，所以需要执行此操作 (Since we're using TypeSupport_cpp, we
    // need to do this)
    tsupport->setName(type_name.c_str());
    fastdds_type.reset(tsupport);
  }

  // 如果请求了带有非键控类型的键控主题，则设置错误消息并返回空指针 (If a keyed topic is requested
  // with a non-keyed type, set error message and return null pointer)
  if (keyed && !fastdds_type->m_isGetKeyDefined) {
    RMW_SET_ERROR_MSG("create_subscription() requested a keyed topic with a non-keyed type");
    return nullptr;
  }

  // 如果注册类型失败，则设置错误消息并返回空指针 (If registering the type fails, set error message
  // and return null pointer)
  if (ReturnCode_t::RETCODE_OK != fastdds_type.register_type(dds_participant)) {
    RMW_SET_ERROR_MSG("create_subscription() failed to register type");
    return nullptr;
  }
  info->type_support_ = fastdds_type;

  // 注意（methylDragon）：我不确定这是否必要...
  //                     不过 FastDDS 的动态类型示例中似乎没有出现
  //
  // NOTE(methylDragon): I'm not sure if this is essential or not...
  //                     It doesn't appear in the dynamic type example for FastDDS though
  // if (!rmw_fastrtps_shared_cpp::register_type_object(type_support, type_name)) {
  //   RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
  //     "failed to register type object with incompatible type %s",
  //     type_name.c_str());
  //   return nullptr;
  // }

  // 创建监听器 (Create Listener)
  // 使用nothrow关键字为 RMWSubscriptionEvent 类型的对象分配内存，确保在内存不足时返回 nullptr
  // 而不是抛出异常 (Allocate memory for an object of type RMWSubscriptionEvent using the nothrow
  // keyword, ensuring that nullptr is returned instead of throwing an exception when there is not
  // enough memory)
  info->subscription_event_ = new (std::nothrow) RMWSubscriptionEvent(info);
  if (!info->subscription_event_) {
    // 设置错误消息 (Set error message)
    RMW_SET_ERROR_MSG("create_subscription() could not create subscription event");
    return nullptr;
  }

  // 使用nothrow关键字为 CustomDataReaderListener 类型的对象分配内存，确保在内存不足时返回 nullptr
  // 而不是抛出异常 (Allocate memory for an object of type CustomDataReaderListener using the
  // nothrow keyword, ensuring that nullptr is returned instead of throwing an exception when there
  // is not enough memory)
  info->data_reader_listener_ =
      new (std::nothrow) CustomDataReaderListener(info->subscription_event_);
  if (!info->data_reader_listener_) {
    // 设置错误消息 (Set error message)
    RMW_SET_ERROR_MSG("create_subscription() could not create subscription data reader listener");
    return nullptr;
  }

  // 创建并注册主题 (Create and register Topic)
  // 获取默认的主题 QoS (Get default topic QoS)
  eprosima::fastdds::dds::TopicQos topic_qos = dds_participant->get_default_topic_qos();
  // 设置主题 QoS (Set topic QoS)
  if (!get_topic_qos(*qos_policies, topic_qos)) {
    RMW_SET_ERROR_MSG("create_publisher() failed setting topic QoS");
    return nullptr;
  }

  // 查找或创建主题 (Find or create topic)
  info->topic_ = participant_info->find_or_create_topic(
      topic_name_mangled, type_name, topic_qos, info->subscription_event_);
  if (!info->topic_) {
    // 设置错误消息 (Set error message)
    RMW_SET_ERROR_MSG("create_subscription() failed to create topic");
    return nullptr;
  }

  // 设置成员变量 (Set member variables)
  info->dds_participant_ = dds_participant;
  info->subscriber_ = subscriber;
  info->topic_name_mangled_ = topic_name_mangled;
  des_topic = info->topic_;

  // 创建内容过滤主题 (Create ContentFilteredTopic)
  if (subscription_options->content_filter_options) {
    rmw_subscription_content_filter_options_t *options =
        subscription_options->content_filter_options;
    if (nullptr != options->filter_expression) {
      eprosima::fastdds::dds::ContentFilteredTopic *filtered_topic = nullptr;
      // 创建内容过滤主题 (Create content filtered topic)
      if (!rmw_fastrtps_shared_cpp::create_content_filtered_topic(
              dds_participant, des_topic, topic_name_mangled, options, &filtered_topic)) {
        // 设置错误消息 (Set error message)
        RMW_SET_ERROR_MSG("create_contentfilteredtopic() failed to create contentfilteredtopic");
        return nullptr;
      }
      // 设置成员变量 (Set member variable)
      info->filtered_topic_ = filtered_topic;
      des_topic = filtered_topic;
    }
  }

  // 创建 DataReader (Create DataReader)
  // 如果用户通过环境变量 "FASTRTPS_DEFAULT_PROFILES_FILE" 定义了一个 XML 文件，尝试加载与
  // topic_name 匹配的 profile 名称的 datareader。 如果此类配置文件不存在，则使用默认的 Fast DDS
  // QoS。(If the user defined an XML file via env "FASTRTPS_DEFAULT_PROFILES_FILE", try to load
  // datareader which profile name matches with topic_name. If such profile does not exist, then use
  // the default Fast DDS QoS.)
  eprosima::fastdds::dds::DataReaderQos reader_qos = subscriber->get_default_datareader_qos();

  // 尝试加载带有主题名称的配置文件（Try to load the profile with the topic name）
  // 不需要检查返回代码，因为如果配置文件不存在，QoS 已经是默认值了（It does not need to check the
  // return code, as if the profile does not exist, the QoS is already the default）
  subscriber->get_datareader_qos_from_profile(topic_name, reader_qos);

  if (!participant_info->leave_middleware_default_qos) {
    // 设置历史记录内存策略为 PREALLOCATED_WITH_REALLOC_MEMORY_MODE（Set history memory policy to
    // PREALLOCATED_WITH_REALLOC_MEMORY_MODE）
    reader_qos.endpoint().history_memory_policy =
        eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    // 关闭数据共享（Turn off data sharing）
    reader_qos.data_sharing().off();
  }

  // 获取 DataReader 的 QoS（Get DataReader QoS）
  if (!get_datareader_qos(
          *qos_policies, *type_support->get_type_hash_func(type_support), reader_qos)) {
    RMW_SET_ERROR_MSG("create_subscription() failed setting data reader QoS");
    return nullptr;
  }

  // 保存 DataReader 的 QoS（Store DataReader QoS）
  info->datareader_qos_ = reader_qos;

  // 创建 DataReader（Create DataReader）
  if (!rmw_fastrtps_shared_cpp::create_datareader(
          info->datareader_qos_, subscription_options, subscriber, des_topic,
          info->data_reader_listener_, &info->data_reader_)) {
    RMW_SET_ERROR_MSG("create_datareader() could not create data reader");
    return nullptr;
  }

  // 初始化 DataReader 的 StatusCondition，以便在有新数据可用时得到通知（Initialize DataReader's
  // StatusCondition to be notified when new data is available）
  info->data_reader_->get_statuscondition().set_enabled_statuses(
      eprosima::fastdds::dds::StatusMask::data_available());

  // 定义一个 lambda 函数来删除 DataReader（Define a lambda function to delete DataReader）
  auto cleanup_datareader = rcpputils::make_scope_exit(
      [subscriber, info]() { subscriber->delete_datareader(info->data_reader_); });

  // 创建 RMW GID（Create RMW GID）
  info->subscription_gid_ = rmw_fastrtps_shared_cpp::create_rmw_gid(
      eprosima_fastrtps_identifier, info->data_reader_->guid());

  // 分配订阅 (Allocate subscription)
  rmw_subscription_t *rmw_subscription = rmw_subscription_allocate();

  // 如果分配失败，设置错误信息并返回 nullptr (If allocation fails, set error message and return
  // nullptr)
  if (!rmw_subscription) {
    RMW_SET_ERROR_MSG("create_subscription() failed to allocate subscription");
    return nullptr;
  }

  // 创建清理 rmw_subscription 的作用域退出函数 (Create scope exit function for cleaning up
  // rmw_subscription)
  auto cleanup_rmw_subscription = rcpputils::make_scope_exit([rmw_subscription]() {
    rmw_free(const_cast<char *>(rmw_subscription->topic_name));
    rmw_subscription_free(rmw_subscription);
  });

  // 设置实现标识符 (Set implementation identifier)
  rmw_subscription->implementation_identifier = eprosima_fastrtps_identifier;

  // 设置订阅数据 (Set subscription data)
  rmw_subscription->data = info;

  // 分配并设置订阅的主题名称 (Allocate and set the topic name of the subscription)
  rmw_subscription->topic_name = rcutils_strdup(topic_name, rcutils_get_default_allocator());
  if (!rmw_subscription->topic_name) {
    RMW_SET_ERROR_MSG(
        "create_subscription() failed to allocate memory for subscription topic name");
    return nullptr;
  }

  // 设置订阅选项 (Set subscription options)
  rmw_subscription->options = *subscription_options;

  // 初始化用于 loans 的订阅 (Initialize subscription for loans)
  rmw_fastrtps_shared_cpp::__init_subscription_for_loans(rmw_subscription);

  // 设置是否启用了内容过滤主题 (Set whether content filtered topic is enabled)
  rmw_subscription->is_cft_enabled = info->filtered_topic_ != nullptr;

  // 取消清理函数 (Cancel cleanup functions)
  cleanup_rmw_subscription.cancel();
  cleanup_datareader.cancel();
  cleanup_info.cancel();

  // 添加追踪点 (Add tracepoint)
  TRACEPOINT(
      rmw_subscription_init, static_cast<const void *>(rmw_subscription),
      info->subscription_gid_.data);

  // 返回创建的订阅 (Return the created subscription)
  return rmw_subscription;
}

// =================================================================================================
// CREATE SUBSCRIPTION
// =================================================================================================
/**
 * @brief 创建一个订阅者 (Create a subscription)
 *
 * @param[in] participant_info 自定义参与者信息 (Custom participant information)
 * @param[in] type_supports 消息类型支持的句柄 (Handle to message type supports)
 * @param[in] topic_name 主题名称 (Topic name)
 * @param[in] qos_policies 服务质量策略 (Quality of Service policies)
 * @param[in] subscription_options 订阅选项 (Subscription options)
 * @param[in] keyed 是否为键控 (Whether it is keyed or not)
 * @return rmw_subscription_t* 创建的订阅者指针 (Pointer to the created subscription)
 */
rmw_subscription_t *__create_subscription(
    CustomParticipantInfo *participant_info,
    const rosidl_message_type_support_t *type_supports,
    const char *topic_name,
    const rmw_qos_profile_t *qos_policies,
    const rmw_subscription_options_t *subscription_options,
    bool keyed) {
  // 获取消息类型支持句柄，以 C 类型支持为优先 (Get the message type support handle, with priority
  // on C type support)
  const rosidl_message_type_support_t *type_support =
      get_message_typesupport_handle(type_supports, RMW_FASTRTPS_CPP_TYPESUPPORT_C);

  // 如果未找到 C 类型支持，则尝试获取 C++ 类型支持 (If C type support is not found, try to get C++
  // type support)
  if (!type_support) {
    rcutils_error_string_t prev_error_string = rcutils_get_error_string();
    rcutils_reset_error();
    type_support = get_message_typesupport_handle(type_supports, RMW_FASTRTPS_CPP_TYPESUPPORT_CPP);

    // 如果未找到 C++ 类型支持，设置错误消息并返回 nullptr (If C++ type support is not found, set
    // error message and return nullptr)
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

  // 使用 std::lock_guard 对象来保护实体创建互斥体
  // Use a std::lock_guard object to protect the entity creation mutex
  std::lock_guard<std::mutex> lck(participant_info->entity_creation_mutex_);

  /////
  // 查找并检查现有的主题和类型
  // Find and check existing topic and type

  // 创建主题和类型名称
  // Create Topic and Type names
  auto callbacks = static_cast<const message_type_support_callbacks_t *>(type_support->data);
  std::string type_name = _create_type_name(callbacks);
  auto topic_name_mangled =
      _create_topic_name(qos_policies, ros_topic_prefix, topic_name).to_string();

  eprosima::fastdds::dds::TypeSupport fastdds_type;
  eprosima::fastdds::dds::TopicDescription *des_topic = nullptr;

  // 查找并检查与参与者信息、主题名称和类型名称匹配的主题和类型
  // Find and check topic and type that match participant info, topic name and type name
  if (!rmw_fastrtps_shared_cpp::find_and_check_topic_and_type(
          participant_info, topic_name_mangled, type_name, &des_topic, &fastdds_type)) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "create_subscription() called for existing topic name %s with incompatible type %s",
        topic_name_mangled.c_str(), type_name.c_str());
    return nullptr;
  }

  /////
  // 获取参与者和订阅者
  // Get Participant and Subscriber
  eprosima::fastdds::dds::DomainParticipant *dds_participant = participant_info->participant_;
  eprosima::fastdds::dds::Subscriber *subscriber = participant_info->subscriber_;

  /////
  // 创建自定义订阅者结构（info）
  // Create the custom Subscriber struct (info)
  auto info = new (std::nothrow) CustomSubscriberInfo();
  if (!info) {
    RMW_SET_ERROR_MSG("create_subscription() failed to allocate CustomSubscriberInfo");
    return nullptr;
  }

  // 在退出作用域时清理相关信息
  // Clean up related information when exiting the scope
  auto cleanup_info = rcpputils::make_scope_exit([info, participant_info]() {
    rmw_fastrtps_shared_cpp::remove_topic_and_type(
        participant_info, info->subscription_event_, info->topic_, info->type_support_);
    delete info->subscription_event_;
    delete info->data_reader_listener_;
    delete info;
  });

  // 设置类型支持标识符和实现回调
  // Set the type support identifier and implementation callbacks
  info->typesupport_identifier_ = type_support->typesupport_identifier;
  info->type_support_impl_ = callbacks;

  // 创建 Type Support 结构体 (Create the Type Support struct)
  if (!fastdds_type) {
    // 使用回调函数创建 MessageTypeSupport_cpp 对象 (Create a MessageTypeSupport_cpp object using
    // callbacks)
    auto tsupport = new (std::nothrow) MessageTypeSupport_cpp(callbacks);
    if (!tsupport) {
      // 如果分配失败，设置错误消息 (Set error message if allocation fails)
      RMW_SET_ERROR_MSG("create_subscription() failed to allocate MessageTypeSupport");
      return nullptr;
    }

    // 将所有权转移给 fastdds_type (Transfer ownership to fastdds_type)
    fastdds_type.reset(tsupport);
  }

  // 检查是否请求了带键的主题和非键类型 (Check if a keyed topic with a non-keyed type is requested)
  if (keyed && !fastdds_type->m_isGetKeyDefined) {
    RMW_SET_ERROR_MSG("create_subscription() requested a keyed topic with a non-keyed type");
    return nullptr;
  }

  // 注册类型 (Register the type)
  if (ReturnCode_t::RETCODE_OK != fastdds_type.register_type(dds_participant)) {
    RMW_SET_ERROR_MSG("create_subscription() failed to register type");
    return nullptr;
  }
  // 设置 info 的 type_support_ 属性 (Set the type_support_ attribute of info)
  info->type_support_ = fastdds_type;

  // 注册类型对象 (Register the type object)
  if (!rmw_fastrtps_shared_cpp::register_type_object(type_supports, type_name)) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "failed to register type object with incompatible type %s", type_name.c_str());
    return nullptr;
  }

  // 创建监听器 (Create Listener)
  // 创建订阅事件 (Create subscription event)
  info->subscription_event_ = new (std::nothrow) RMWSubscriptionEvent(info);
  if (!info->subscription_event_) {
    RMW_SET_ERROR_MSG("create_subscription() could not create subscription event");
    return nullptr;
  }

  // 创建订阅数据读取器监听器 (Create subscription data reader listener)
  info->data_reader_listener_ =
      new (std::nothrow) CustomDataReaderListener(info->subscription_event_);
  if (!info->data_reader_listener_) {
    RMW_SET_ERROR_MSG("create_subscription() could not create subscription data reader listener");
    return nullptr;
  }

  /// 创建并注册 Topic (Create and register Topic)
  eprosima::fastdds::dds::TopicQos topic_qos = dds_participant->get_default_topic_qos();
  // 获取 Topic 的 QoS 设置，如果失败则设置错误信息并返回空指针 (Get the Topic QoS settings, if
  // failed, set error message and return nullptr)
  if (!get_topic_qos(*qos_policies, topic_qos)) {
    RMW_SET_ERROR_MSG("create_publisher() failed setting topic QoS");
    return nullptr;
  }

  // 在参与者信息中查找或创建主题，并将其与订阅事件关联 (Find or create a topic in participant_info
  // and associate it with the subscription event)
  info->topic_ = participant_info->find_or_create_topic(
      topic_name_mangled, type_name, topic_qos, info->subscription_event_);
  // 如果主题创建失败，则设置错误信息并返回空指针 (If the topic creation fails, set the error
  // message and return nullptr)
  if (!info->topic_) {
    RMW_SET_ERROR_MSG("create_subscription() failed to create topic");
    return nullptr;
  }

  // 将 DDS 参与者、订阅者和混淆的主题名称分别存储在 info 中 (Store the DDS participant, subscriber,
  // and mangled topic name in info)
  info->dds_participant_ = dds_participant;
  info->subscriber_ = subscriber;
  info->topic_name_mangled_ = topic_name_mangled;

  // 设置 des_topic 为 info 中的 topic (Set des_topic as the topic in info)
  des_topic = info->topic_;

  // 创建 ContentFilteredTopic (Create ContentFilteredTopic)
  if (subscription_options->content_filter_options) {
    rmw_subscription_content_filter_options_t *options =
        subscription_options->content_filter_options;
    // 如果过滤表达式不为空，则创建一个 ContentFilteredTopic (If the filter expression is not
    // nullptr, create a ContentFilteredTopic)
    if (nullptr != options->filter_expression) {
      eprosima::fastdds::dds::ContentFilteredTopic *filtered_topic = nullptr;
      // 创建内容过滤主题，如果失败则设置错误信息并返回空指针 (Create content filtered topic, if
      // failed, set error message and return nullptr)
      if (!rmw_fastrtps_shared_cpp::create_content_filtered_topic(
              dds_participant, des_topic, topic_name_mangled, options, &filtered_topic)) {
        RMW_SET_ERROR_MSG("create_contentfilteredtopic() failed to create contentfilteredtopic");
        return nullptr;
      }
      // 将创建的过滤主题存储在 info 中 (Store the created filtered topic in info)
      info->filtered_topic_ = filtered_topic;
      // 设置 des_topic 为过滤主题 (Set des_topic as the filtered topic)
      des_topic = filtered_topic;
    }
  }

  // 创建 DataReader
  // Create DataReader

  // 如果用户通过环境变量 "FASTRTPS_DEFAULT_PROFILES_FILE" 定义了一个 XML 文件，尝试加载与
  // topic_name 匹配的 profile 名称的 datareader。 如果不存在这样的配置文件，则使用默认的 Fast DDS
  // QoS。 If the user defined an XML file via env "FASTRTPS_DEFAULT_PROFILES_FILE", try to load
  // datareader which profile name matches with topic_name. If such profile does not exist,
  // then use the default Fast DDS QoS.
  eprosima::fastdds::dds::DataReaderQos reader_qos = subscriber->get_default_datareader_qos();

  // 尝试加载带有 topic_name 的配置文件
  // 不需要检查返回代码，因为如果配置文件不存在，QoS 已经是默认值
  // Try to load the profile with the topic name
  // It does not need to check the return code, as if the profile does not exist,
  // the QoS is already the default
  subscriber->get_datareader_qos_from_profile(topic_name, reader_qos);

  if (!participant_info->leave_middleware_default_qos) {
    reader_qos.endpoint().history_memory_policy =
        eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    reader_qos.data_sharing().off();
  }

  // 获取 datareader 的 QoS 设置
  // Get the datareader QoS settings
  if (!get_datareader_qos(
          *qos_policies, *type_supports->get_type_hash_func(type_supports), reader_qos)) {
    RMW_SET_ERROR_MSG("create_subscription() failed setting data reader QoS");
    return nullptr;
  }

  info->datareader_qos_ = reader_qos;

  // 创建 datareader
  // Create the datareader
  if (!rmw_fastrtps_shared_cpp::create_datareader(
          info->datareader_qos_, subscription_options, subscriber, des_topic,
          info->data_reader_listener_, &info->data_reader_)) {
    RMW_SET_ERROR_MSG("create_datareader() could not create data reader");
    return nullptr;
  }

  // 初始化 DataReader 的 StatusCondition，以便在有新数据可用时得到通知
  // Initialize DataReader's StatusCondition to be notified when new data is available
  info->data_reader_->get_statuscondition().set_enabled_statuses(
      eprosima::fastdds::dds::StatusMask::data_available());

  // 删除 datareader 的 lambda 函数
  // Lambda function to delete the datareader
  auto cleanup_datareader = rcpputils::make_scope_exit(
      [subscriber, info]() { subscriber->delete_datareader(info->data_reader_); });

  /// 创建 RMW GID (Create RMW GID)
  // 使用 eprosima_fastrtps_identifier 和 info->data_reader_->guid() 创建一个 rmw_gid_t 类型的对象
  // (Create an rmw_gid_t object using eprosima_fastrtps_identifier and info->data_reader_->guid())
  info->subscription_gid_ = rmw_fastrtps_shared_cpp::create_rmw_gid(
      eprosima_fastrtps_identifier, info->data_reader_->guid());

  // 分配订阅 (Allocate subscription)
  // 为 rmw_subscription_t 类型的指针分配内存空间
  // (Allocate memory space for a rmw_subscription_t pointer)
  rmw_subscription_t *rmw_subscription = rmw_subscription_allocate();
  if (!rmw_subscription) {
    RMW_SET_ERROR_MSG("create_subscription() failed to allocate subscription");
    return nullptr;
  }
  // 创建一个清理 rmw_subscription 的作用域退出对象
  // (Create a scope exit object for cleaning up rmw_subscription)
  auto cleanup_rmw_subscription = rcpputils::make_scope_exit([rmw_subscription]() {
    rmw_free(const_cast<char *>(rmw_subscription->topic_name));
    rmw_subscription_free(rmw_subscription);
  });

  // 设置实现标识符 (Set implementation identifier)
  rmw_subscription->implementation_identifier = eprosima_fastrtps_identifier;
  // 设置数据指针 (Set data pointer)
  rmw_subscription->data = info;

  /// 设置主题名称 (Set topic name)
  // 使用 rcutils_strdup 函数为主题名称分配内存并复制给定的主题名称
  // (Allocate memory for the topic name and copy the given topic name using rcutils_strdup
  // function)
  rmw_subscription->topic_name = rcutils_strdup(topic_name, rcutils_get_default_allocator());
  if (!rmw_subscription->topic_name) {
    RMW_SET_ERROR_MSG(
        "create_subscription() failed to allocate memory for subscription topic name");
    return nullptr;
  }
  // 设置订阅选项 (Set subscription options)
  rmw_subscription->options = *subscription_options;
  // 初始化 loans (Initialize loans)
  rmw_fastrtps_shared_cpp::__init_subscription_for_loans(rmw_subscription);
  // 设置是否启用内容过滤主题 (Set whether ContentFilteredTopic is enabled)
  rmw_subscription->is_cft_enabled = info->filtered_topic_ != nullptr;

  // 取消清理操作 (Cancel cleanup operations)
  cleanup_rmw_subscription.cancel();
  cleanup_datareader.cancel();
  cleanup_info.cancel();

  // 添加跟踪点 (Add tracepoint)
  TRACEPOINT(
      rmw_subscription_init, static_cast<const void *>(rmw_subscription),
      info->subscription_gid_.data);
  // 返回创建的订阅 (Return the created subscription)
  return rmw_subscription;
}

}  // namespace rmw_fastrtps_cpp
