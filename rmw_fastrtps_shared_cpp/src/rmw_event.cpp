// Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <unordered_set>

#include "event_helpers.hpp"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw_fastrtps_shared_cpp/custom_event_info.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "types/event_types.hpp"

/**
 * @brief 静态常量，包含了所有支持的 rmw 事件类型 (Static constant containing all supported rmw
 * event types)
 *
 * 这个集合包含了所有在 rmw_fastrtps_cpp 层支持的事件类型。
 * (This set contains all the event types supported in the rmw_fastrtps_cpp layer.)
 */
static const std::unordered_set<rmw_event_type_t> g_rmw_event_type_set{
    RMW_EVENT_LIVELINESS_CHANGED,  ///< 活跃度状态发生改变 (Liveliness status changed)
    RMW_EVENT_REQUESTED_DEADLINE_MISSED,  ///< 请求的截止时间未满足 (Requested deadline missed)
    RMW_EVENT_LIVELINESS_LOST,            ///< 丢失活跃度 (Liveliness lost)
    RMW_EVENT_OFFERED_DEADLINE_MISSED,  ///< 提供的截止时间未满足 (Offered deadline missed)
    RMW_EVENT_MESSAGE_LOST,             ///< 丢失消息 (Message lost)
    RMW_EVENT_OFFERED_QOS_INCOMPATIBLE,    ///< 提供的 QoS 不兼容 (Offered QoS incompatible)
    RMW_EVENT_REQUESTED_QOS_INCOMPATIBLE,  ///< 请求的 QoS 不兼容 (Requested QoS incompatible)
    RMW_EVENT_SUBSCRIPTION_INCOMPATIBLE_TYPE,  ///< 订阅的类型不兼容 (Subscription incompatible
                                               ///< type)
    RMW_EVENT_PUBLISHER_INCOMPATIBLE_TYPE,  ///< 发布者的类型不兼容 (Publisher incompatible type)
    RMW_EVENT_SUBSCRIPTION_MATCHED,         ///< 订阅匹配成功 (Subscription matched)
    RMW_EVENT_PUBLICATION_MATCHED           ///< 发布匹配成功 (Publication matched)
};

namespace rmw_fastrtps_shared_cpp {
namespace internal {

/**
 * @brief 将 rmw_event 转换为 DDS StatusMask (Convert rmw_event to DDS StatusMask)
 *
 * @param[in] event_type rmw_event 类型 (rmw_event type)
 * @return eprosima::fastdds::dds::StatusMask 对应的 DDS StatusMask (Corresponding DDS StatusMask)
 */
eprosima::fastdds::dds::StatusMask rmw_event_to_dds_statusmask(const rmw_event_type_t event_type) {
  // 初始化返回值为 none 状态 (Initialize the return value as none status)
  eprosima::fastdds::dds::StatusMask ret_statusmask = eprosima::fastdds::dds::StatusMask::none();

  // 根据 event_type 设置对应的 StatusMask (Set the corresponding StatusMask based on event_type)
  switch (event_type) {
    case RMW_EVENT_LIVELINESS_CHANGED:
      // 活跃度改变事件 (Liveliness changed event)
      ret_statusmask = eprosima::fastdds::dds::StatusMask::liveliness_changed();
      break;
    case RMW_EVENT_REQUESTED_DEADLINE_MISSED:
      // 请求截止日期错过事件 (Requested deadline missed event)
      ret_statusmask = eprosima::fastdds::dds::StatusMask::requested_deadline_missed();
      break;
    case RMW_EVENT_LIVELINESS_LOST:
      // 活跃度丢失事件 (Liveliness lost event)
      ret_statusmask = eprosima::fastdds::dds::StatusMask::liveliness_lost();
      break;
    case RMW_EVENT_OFFERED_DEADLINE_MISSED:
      // 提供的截止日期错过事件 (Offered deadline missed event)
      ret_statusmask = eprosima::fastdds::dds::StatusMask::offered_deadline_missed();
      break;
    case RMW_EVENT_MESSAGE_LOST:
      // 消息丢失事件 (Message lost event)
      ret_statusmask = eprosima::fastdds::dds::StatusMask::sample_lost();
      break;
    case RMW_EVENT_OFFERED_QOS_INCOMPATIBLE:
      // 提供的 QoS 不兼容事件 (Offered QoS incompatible event)
      ret_statusmask = eprosima::fastdds::dds::StatusMask::offered_incompatible_qos();
      break;
    case RMW_EVENT_REQUESTED_QOS_INCOMPATIBLE:
      // 请求的 QoS 不兼容事件 (Requested QoS incompatible event)
      ret_statusmask = eprosima::fastdds::dds::StatusMask::requested_incompatible_qos();
      break;
    case RMW_EVENT_SUBSCRIPTION_INCOMPATIBLE_TYPE:
      // 订阅类型不兼容事件 (Subscription incompatible type event)
      ret_statusmask = eprosima::fastdds::dds::StatusMask::inconsistent_topic();
      break;
    case RMW_EVENT_PUBLISHER_INCOMPATIBLE_TYPE:
      // 发布者类型不兼容事件 (Publisher incompatible type event)
      ret_statusmask = eprosima::fastdds::dds::StatusMask::inconsistent_topic();
      break;
    case RMW_EVENT_SUBSCRIPTION_MATCHED:
      // 订阅匹配事件 (Subscription matched event)
      ret_statusmask = eprosima::fastdds::dds::StatusMask::subscription_matched();
      break;
    case RMW_EVENT_PUBLICATION_MATCHED:
      // 发布匹配事件 (Publication matched event)
      ret_statusmask = eprosima::fastdds::dds::StatusMask::publication_matched();
      break;
    default:
      // 未知事件类型，保持 none 状态 (Unknown event type, keep none status)
      break;
  }

  return ret_statusmask;
}

/**
 * @brief 检查事件类型是否受支持 (Check if the event type is supported)
 *
 * @param[in] event_type rmw_event 类型 (rmw_event type)
 * @return bool 支持返回 true，不支持返回 false (Return true if supported, false otherwise)
 */
bool is_event_supported(rmw_event_type_t event_type) {
  // 如果 g_rmw_event_type_set 中包含 event_type，则返回 true，否则返回 false
  // (If g_rmw_event_type_set contains event_type, return true, otherwise return false)
  return g_rmw_event_type_set.count(event_type) == 1;
}

/**
 * @brief 将 DDS QoS 策略转换为 RMW QoS 策略 (Convert DDS QoS policy to RMW QoS policy)
 *
 * @param[in] policy_id DDS QoS 策略 ID (DDS QoS policy ID)
 * @return rmw_qos_policy_kind_t 对应的 RMW QoS 策略 (Corresponding RMW QoS policy)
 */
rmw_qos_policy_kind_t dds_qos_policy_to_rmw_qos_policy(
    eprosima::fastdds::dds::QosPolicyId_t policy_id) {
  // 使用 eprosima::fastdds::dds::QosPolicyId_t 命名空间
  // Use the eprosima::fastdds::dds::QosPolicyId_t namespace
  using eprosima::fastdds::dds::QosPolicyId_t;

  // 根据传入的 policy_id 进行 switch-case 判断
  // Perform a switch-case based on the input policy_id
  switch (policy_id) {
    // 如果是 DURABILITY_QOS_POLICY_ID
    case QosPolicyId_t::DURABILITY_QOS_POLICY_ID:
      // 返回对应的 RMW_QOS_POLICY_DURABILITY
      return RMW_QOS_POLICY_DURABILITY;
    // 如果是 DEADLINE_QOS_POLICY_ID
    case QosPolicyId_t::DEADLINE_QOS_POLICY_ID:
      // 返回对应的 RMW_QOS_POLICY_DEADLINE
      return RMW_QOS_POLICY_DEADLINE;
    // 如果是 LIVELINESS_QOS_POLICY_ID
    case QosPolicyId_t::LIVELINESS_QOS_POLICY_ID:
      // 返回对应的 RMW_QOS_POLICY_LIVELINESS
      return RMW_QOS_POLICY_LIVELINESS;
    // 如果是 RELIABILITY_QOS_POLICY_ID
    case QosPolicyId_t::RELIABILITY_QOS_POLICY_ID:
      // 返回对应的 RMW_QOS_POLICY_RELIABILITY
      return RMW_QOS_POLICY_RELIABILITY;
    // 如果是 HISTORY_QOS_POLICY_ID
    case QosPolicyId_t::HISTORY_QOS_POLICY_ID:
      // 返回对应的 RMW_QOS_POLICY_HISTORY
      return RMW_QOS_POLICY_HISTORY;
    // 如果是 LIFESPAN_QOS_POLICY_ID
    case QosPolicyId_t::LIFESPAN_QOS_POLICY_ID:
      // 返回对应的 RMW_QOS_POLICY_LIFESPAN
      return RMW_QOS_POLICY_LIFESPAN;
    // 其他情况
    default:
      // 返回无效的 RMW_QOS_POLICY_INVALID
      return RMW_QOS_POLICY_INVALID;
  }
}

}  // namespace internal

/**
 * @brief 初始化事件 (Initialize the event)
 *
 * @param[in] identifier 标识符，用于检查 RMW 实现是否匹配 (Identifier for checking if the RMW
 * implementation matches)
 * @param[out] rmw_event 用于存储初始化后的事件信息的结构体指针 (Pointer to the structure for
 * storing the initialized event information)
 * @param[in] topic_endpoint_impl_identifier 主题端点实现标识符 (Topic endpoint implementation
 * identifier)
 * @param[in] data 事件相关数据 (Event-related data)
 * @param[in] event_type 要初始化的事件类型 (The event type to initialize)
 * @return 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_init_event(
    const char *identifier,
    rmw_event_t *rmw_event,
    const char *topic_endpoint_impl_identifier,
    void *data,
    rmw_event_type_t event_type) {
  // 检查输入参数是否为空 (Check if input arguments are null)
  RMW_CHECK_ARGUMENT_FOR_NULL(identifier, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(rmw_event, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(topic_endpoint_impl_identifier, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(data, RMW_RET_INVALID_ARGUMENT);

  // 检查类型标识符是否匹配 (Check if type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      topic endpoint, topic_endpoint_impl_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查事件类型是否受支持 (Check if the event type is supported)
  if (!internal::is_event_supported(event_type)) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("provided event_type is not supported by %s", identifier);
    return RMW_RET_UNSUPPORTED;
  }

  // 设置事件信息 (Set the event information)
  rmw_event->implementation_identifier = topic_endpoint_impl_identifier;
  rmw_event->data = data;
  rmw_event->event_type = event_type;

  // 获取自定义事件信息 (Get custom event information)
  CustomEventInfo *event = static_cast<CustomEventInfo *>(rmw_event->data);

  // 获取并设置状态掩码 (Get and set the status mask)
  eprosima::fastdds::dds::StatusMask status_mask =
      event->get_listener()->get_statuscondition().get_enabled_statuses();
  status_mask |= rmw_fastrtps_shared_cpp::internal::rmw_event_to_dds_statusmask(event_type);
  event->get_listener()->get_statuscondition().set_enabled_statuses(status_mask);

  return RMW_RET_OK;
}

/**
 * @brief 设置事件回调函数 (Set the event callback function)
 *
 * @param[in,out] rmw_event 事件结构体指针，包含事件相关信息 (Pointer to the event structure,
 * containing event-related information)
 * @param[in] callback 要设置的回调函数 (The callback function to set)
 * @param[in] user_data 用户数据，将传递给回调函数 (User data that will be passed to the callback
 * function)
 * @return 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_event_set_callback(
    rmw_event_t *rmw_event, rmw_event_callback_t callback, const void *user_data) {
  // 获取自定义事件信息 (Get custom event information)
  auto custom_event_info = static_cast<CustomEventInfo *>(rmw_event->data);

  // 设置新事件回调 (Set the new event callback)
  custom_event_info->get_listener()->set_on_new_event_callback(
      rmw_event->event_type, user_data, callback);

  return RMW_RET_OK;
}

}  // namespace rmw_fastrtps_shared_cpp
