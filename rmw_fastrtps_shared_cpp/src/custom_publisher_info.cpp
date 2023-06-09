// Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "rmw_fastrtps_shared_cpp/custom_publisher_info.hpp"

#include "event_helpers.hpp"
#include "fastdds/dds/core/policy/QosPolicies.hpp"
#include "fastdds/dds/core/status/BaseStatus.hpp"
#include "fastdds/dds/core/status/DeadlineMissedStatus.hpp"
#include "types/event_types.hpp"

/**
 * @brief 获取监听器对象 (Get the listener object)
 *
 * @return EventListenerInterface* 监听器指针 (Pointer to the listener)
 */
EventListenerInterface *CustomPublisherInfo::get_listener() const { return publisher_event_; }

/**
 * @brief CustomDataWriterListener 构造函数 (Constructor for CustomDataWriterListener)
 *
 * @param pub_event 发布者事件对象指针 (Pointer to the RMWPublisherEvent object)
 */
CustomDataWriterListener::CustomDataWriterListener(RMWPublisherEvent *pub_event)
    : publisher_event_(pub_event) {}

/**
 * @brief 当出版物匹配时调用 (Called when a publication is matched)
 *
 * @param writer 数据写入器指针 (Pointer to the DataWriter)
 * @param status 匹配状态信息 (Information about the matching status)
 */
void CustomDataWriterListener::on_publication_matched(
    eprosima::fastdds::dds::DataWriter *writer,
    const eprosima::fastdds::dds::PublicationMatchedStatus &status) {
  (void)writer;  // 忽略未使用的参数 (Ignore unused parameter)

  if (status.current_count_change == 1) {
    // 跟踪唯一订阅 (Track unique subscription)
    publisher_event_->track_unique_subscription(
        eprosima::fastrtps::rtps::iHandle2GUID(status.last_subscription_handle));
  } else if (status.current_count_change == -1) {
    // 取消跟踪唯一订阅 (Untrack unique subscription)
    publisher_event_->untrack_unique_subscription(
        eprosima::fastrtps::rtps::iHandle2GUID(status.last_subscription_handle));
  } else {
    return;
  }

  // 更新匹配状态 (Update matched status)
  publisher_event_->update_matched(
      status.total_count, status.total_count_change, status.current_count,
      status.current_count_change);
}

/**
 * @brief 当提供的截止日期被错过时调用 (Called when the offered deadline is missed)
 *
 * @param writer 数据写入器指针 (Pointer to the DataWriter)
 * @param status 截止日期丢失状态信息 (Information about the missed deadline status)
 */
void CustomDataWriterListener::on_offered_deadline_missed(
    eprosima::fastdds::dds::DataWriter *writer,
    const eprosima::fastdds::dds::OfferedDeadlineMissedStatus &status) {
  (void)writer;  // 忽略未使用的参数 (Ignore unused parameter)

  // 更新截止日期状态 (Update deadline status)
  publisher_event_->update_deadline(status.total_count, status.total_count_change);
}

/**
 * @brief 当生命周期丢失时调用 (Called when liveliness is lost)
 *
 * @param writer 数据写入器指针 (Pointer to the DataWriter)
 * @param status 生命周期丢失状态信息 (Information about the liveliness lost status)
 */
void CustomDataWriterListener::on_liveliness_lost(
    eprosima::fastdds::dds::DataWriter *writer,
    const eprosima::fastdds::dds::LivelinessLostStatus &status) {
  (void)writer;  // 忽略未使用的参数 (Ignore unused parameter)

  // 更新生命周期丢失状态 (Update liveliness lost status)
  publisher_event_->update_liveliness_lost(status.total_count, status.total_count_change);
}

/**
 * @brief 当提供的 QoS 不兼容时调用 (Called when the offered QoS is incompatible)
 *
 * @param writer 数据写入器指针 (Pointer to the DataWriter)
 * @param status 不兼容 QoS 状态信息 (Information about the incompatible QoS status)
 */
void CustomDataWriterListener::on_offered_incompatible_qos(
    eprosima::fastdds::dds::DataWriter *writer,
    const eprosima::fastdds::dds::OfferedIncompatibleQosStatus &status) {
  (void)writer;  // 忽略未使用的参数 (Ignore unused parameter)

  // 更新不兼容 QoS 状态 (Update incompatible QoS status)
  publisher_event_->update_offered_incompatible_qos(
      status.last_policy_id, status.total_count, status.total_count_change);
}

/**
 * @brief 构造函数，初始化 RMWPublisherEvent 对象。
 * @param info 自定义发布者信息指针。
 */
RMWPublisherEvent::RMWPublisherEvent(CustomPublisherInfo *info)
    : publisher_info_(info),
      deadline_changed_(false),
      liveliness_changed_(false),
      incompatible_qos_changed_(false),
      matched_changes_(false) {}

/**
 * @brief 获取状态条件。
 * @return 状态条件的引用。
 */
eprosima::fastdds::dds::StatusCondition &RMWPublisherEvent::get_statuscondition() const {
  return publisher_info_->data_writer_->get_statuscondition();
}

/**
 * @brief 处理事件。
 * @param event_type 事件类型。
 * @param event_info 事件信息指针。
 * @return 如果成功处理事件，则返回 true，否则返回 false。
 */
bool RMWPublisherEvent::take_event(rmw_event_type_t event_type, void *event_info) {
  assert(rmw_fastrtps_shared_cpp::internal::is_event_supported(event_type));

  std::unique_lock<std::mutex> lock_mutex(on_new_event_m_);

  /**
   * @brief 处理不同类型的事件并更新相应的状态信息
   *
   * @param[in] event_type 事件类型 (Event type)
   * @param[out] event_info 用于存储事件状态信息的指针 (Pointer to store event status information)
   */
  switch (event_type) {
    // 处理活跃度丢失事件 (Handle Liveliness Lost event)
    case RMW_EVENT_LIVELINESS_LOST: {
      // 将 event_info 转换为 rmw_liveliness_lost_status_t 类型指针
      auto rmw_data = static_cast<rmw_liveliness_lost_status_t *>(event_info);

      // 检查活跃度是否已经改变 (Check if liveliness has already changed)
      if (liveliness_changed_) {
        liveliness_changed_ = false;
      } else {
        // 获取活跃度丢失状态 (Get Liveliness Lost status)
        publisher_info_->data_writer_->get_liveliness_lost_status(liveliness_lost_status_);
      }

      // 更新活跃度丢失状态信息 (Update Liveliness Lost status information)
      rmw_data->total_count = liveliness_lost_status_.total_count;
      rmw_data->total_count_change = liveliness_lost_status_.total_count_change;
      liveliness_lost_status_.total_count_change = 0;
    } break;

    // 处理提供的截止时间错过事件 (Handle Offered Deadline Missed event)
    case RMW_EVENT_OFFERED_DEADLINE_MISSED: {
      // 将 event_info 转换为 rmw_offered_deadline_missed_status_t 类型指针
      auto rmw_data = static_cast<rmw_offered_deadline_missed_status_t *>(event_info);

      // 检查截止时间是否已经改变 (Check if deadline has already changed)
      if (deadline_changed_) {
        deadline_changed_ = false;
      } else {
        // 获取提供的截止时间错过状态 (Get Offered Deadline Missed status)
        publisher_info_->data_writer_->get_offered_deadline_missed_status(
            offered_deadline_missed_status_);
      }

      // 更新提供的截止时间错过状态信息 (Update Offered Deadline Missed status information)
      rmw_data->total_count = offered_deadline_missed_status_.total_count;
      rmw_data->total_count_change = offered_deadline_missed_status_.total_count_change;
      offered_deadline_missed_status_.total_count_change = 0;
    } break;

    // 处理提供的 QoS 不兼容事件 (Handle Offered QoS Incompatible event)
    case RMW_EVENT_OFFERED_QOS_INCOMPATIBLE: {
      // 将 event_info 转换为 rmw_offered_qos_incompatible_event_status_t 类型指针
      auto rmw_data = static_cast<rmw_offered_qos_incompatible_event_status_t *>(event_info);

      // 检查 QoS 是否已经改变 (Check if QoS has already changed)
      if (incompatible_qos_changed_) {
        incompatible_qos_changed_ = false;
      } else {
        // 获取提供的 QoS 不兼容状态 (Get Offered QoS Incompatible status)
        publisher_info_->data_writer_->get_offered_incompatible_qos_status(
            incompatible_qos_status_);
      }

      // 更新提供的 QoS 不兼容状态信息 (Update Offered QoS Incompatible status information)
      rmw_data->total_count = incompatible_qos_status_.total_count;
      rmw_data->total_count_change = incompatible_qos_status_.total_count_change;
      rmw_data->last_policy_kind =
          rmw_fastrtps_shared_cpp::internal::dds_qos_policy_to_rmw_qos_policy(
              incompatible_qos_status_.last_policy_id);
      incompatible_qos_status_.total_count_change = 0;
    } break;
    // 当事件类型为 RMW_EVENT_PUBLISHER_INCOMPATIBLE_TYPE 时的处理逻辑。
    case RMW_EVENT_PUBLISHER_INCOMPATIBLE_TYPE: {
      // 将 event_info 转换为 rmw_incompatible_type_status_t 类型。
      auto rmw_data = static_cast<rmw_incompatible_type_status_t *>(event_info);

      // 检查 inconsistent_topic_changed_ 是否已更改。
      if (inconsistent_topic_changed_) {
        inconsistent_topic_changed_ = false;
      } else {
        // 获取不一致主题状态。
        publisher_info_->data_writer_->get_topic()->get_inconsistent_topic_status(
            inconsistent_topic_status_);
      }

      // 更新 rmw_data 的 total_count 和 total_count_change。
      rmw_data->total_count = inconsistent_topic_status_.total_count;
      rmw_data->total_count_change = inconsistent_topic_status_.total_count_change;

      // 重置 total_count_change。
      inconsistent_topic_status_.total_count_change = 0;
    } break;

    // 当事件类型为 RMW_EVENT_PUBLICATION_MATCHED 时的处理逻辑。
    case RMW_EVENT_PUBLICATION_MATCHED: {
      // 将 event_info 转换为 rmw_matched_status_t 类型。
      auto rmw_data = static_cast<rmw_matched_status_t *>(event_info);

      // 定义一个 PublicationMatchedStatus 类型的变量 matched_status。
      eprosima::fastdds::dds::PublicationMatchedStatus matched_status;

      // 获取 publication_matched_status。
      publisher_info_->data_writer_->get_publication_matched_status(matched_status);

      // 更新 rmw_data 的 total_count、current_count、total_count_change 和 current_count_change。
      rmw_data->total_count = static_cast<size_t>(matched_status.total_count);
      rmw_data->current_count = static_cast<size_t>(matched_status.current_count);
      rmw_data->total_count_change = static_cast<size_t>(matched_status.total_count_change);
      rmw_data->current_count_change = matched_status.current_count_change;

      // 如果存在匹配的更改，则更新 rmw_data 的 total_count_change 和 current_count_change。
      if (matched_changes_) {
        rmw_data->total_count_change += static_cast<size_t>(matched_status_.total_count_change);
        rmw_data->current_count_change += matched_status_.current_count_change;
        matched_changes_ = false;
      }

      // 重置 total_count_change 和 current_count_change。
      matched_status_.total_count_change = 0;
      matched_status_.current_count_change = 0;
    } break;

    // 对于未知事件类型，返回 false。
    default:
      return false;
  }

  // 设置 event_guard[event_type] 的触发值为 false。
  event_guard[event_type].set_trigger_value(false);

  return true;
}

/**
 * @brief 设置新事件回调函数
 * @param event_type 事件类型 (Event type)
 * @param user_data 用户数据指针 (Pointer to user data)
 * @param callback 回调函数 (Callback function)
 *
 * 当事件发生时，此函数将设置一个回调函数来处理这些事件。
 */
void RMWPublisherEvent::set_on_new_event_callback(
    rmw_event_type_t event_type, const void *user_data, rmw_event_callback_t callback) {
  // 对互斥锁进行上锁 (Lock the mutex)
  std::unique_lock<std::mutex> lock_mutex(on_new_event_m_);

  // 获取当前状态掩码 (Get the current status mask)
  eprosima::fastdds::dds::StatusMask status_mask = publisher_info_->data_writer_->get_status_mask();

  // 如果提供了回调函数 (If a callback is provided)
  if (callback) {
    // 根据事件类型执行相应操作 (Perform actions based on the event type)
    switch (event_type) {
      case RMW_EVENT_LIVELINESS_LOST:
        // 获取活跃度丢失状态 (Get the liveliness lost status)
        publisher_info_->data_writer_->get_liveliness_lost_status(liveliness_lost_status_);

        // 如果活跃度丢失计数有变化，则调用回调函数
        if (liveliness_lost_status_.total_count_change > 0) {
          callback(user_data, liveliness_lost_status_.total_count_change);
          liveliness_lost_status_.total_count_change = 0;
        }
        break;
      case RMW_EVENT_OFFERED_DEADLINE_MISSED:
        // 获取提供的截止日期未满足状态 (Get the offered deadline missed status)
        publisher_info_->data_writer_->get_offered_deadline_missed_status(
            offered_deadline_missed_status_);

        // 如果截止日期未满足计数有变化，则调用回调函数
        if (offered_deadline_missed_status_.total_count_change > 0) {
          callback(user_data, offered_deadline_missed_status_.total_count_change);
          offered_deadline_missed_status_.total_count_change = 0;
        }
        break;
      case RMW_EVENT_OFFERED_QOS_INCOMPATIBLE:
        // 获取提供的 QoS 不兼容状态 (Get the offered incompatible QoS status)
        publisher_info_->data_writer_->get_offered_incompatible_qos_status(
            incompatible_qos_status_);

        // 如果 QoS 不兼容计数有变化，则调用回调函数
        if (incompatible_qos_status_.total_count_change > 0) {
          callback(user_data, incompatible_qos_status_.total_count_change);
          incompatible_qos_status_.total_count_change = 0;
        }
        break;
      case RMW_EVENT_PUBLISHER_INCOMPATIBLE_TYPE:
        // 获取发布者不兼容类型状态 (Get the publisher incompatible type status)
        publisher_info_->data_writer_->get_topic()->get_inconsistent_topic_status(
            inconsistent_topic_status_);
        // 如果不兼容类型计数有变化，则调用回调函数
        if (inconsistent_topic_status_.total_count_change > 0) {
          callback(user_data, inconsistent_topic_status_.total_count_change);
          inconsistent_topic_status_.total_count_change = 0;
        }
        break;
      case RMW_EVENT_PUBLICATION_MATCHED: {
        // 如果匹配状态计数有变化，则调用回调函数
        if (matched_status_.total_count_change > 0) {
          callback(user_data, matched_status_.total_count_change);
          publisher_info_->data_writer_->get_publication_matched_status(matched_status_);
          matched_status_.total_count_change = 0;
          matched_status_.current_count_change = 0;
        }
      } break;
      default:
        break;
    }

    // 存储用户数据和回调函数
    user_data_[event_type] = user_data;
    on_new_event_cb_[event_type] = callback;

    // 更新状态掩码 (Update the status mask)
    status_mask |= rmw_fastrtps_shared_cpp::internal::rmw_event_to_dds_statusmask(event_type);
  } else {
    // 清除用户数据和回调函数 (Clear the user data and callback function)
    user_data_[event_type] = nullptr;
    on_new_event_cb_[event_type] = nullptr;

    // publication_matched 状态应保持启用，因为我们需要继续跟踪匹配的订阅
    if (RMW_EVENT_PUBLICATION_MATCHED != event_type) {
      status_mask &= ~rmw_fastrtps_shared_cpp::internal::rmw_event_to_dds_statusmask(event_type);
    }
  }

  // 设置监听器和状态掩码 (Set the listener and status mask)
  publisher_info_->data_writer_->set_listener(publisher_info_->data_writer_listener_, status_mask);
}

/**
 * @brief 跟踪唯一订阅 (Track unique subscription)
 *
 * @param guid 订阅的全局唯一标识符 (Global Unique Identifier of the subscription)
 */
void RMWPublisherEvent::track_unique_subscription(eprosima::fastrtps::rtps::GUID_t guid) {
  // 对 subscriptions_mutex_ 上锁，以保护共享资源 subscriptions_
  std::lock_guard<std::mutex> lock(subscriptions_mutex_);
  // 将 guid 插入到 subscriptions_ 集合中
  subscriptions_.insert(guid);
}

/**
 * @brief 取消跟踪唯一订阅 (Untrack unique subscription)
 *
 * @param guid 订阅的全局唯一标识符 (Global Unique Identifier of the subscription)
 */
void RMWPublisherEvent::untrack_unique_subscription(eprosima::fastrtps::rtps::GUID_t guid) {
  // 对 subscriptions_mutex_ 上锁，以保护共享资源 subscriptions_
  std::lock_guard<std::mutex> lock(subscriptions_mutex_);
  // 从 subscriptions_ 集合中删除 guid (Erase guid from the subscriptions_ set)
  subscriptions_.erase(guid);
}

/**
 * @brief 获取订阅数量 (Get subscription count)
 *
 * @return size_t 订阅数量 (Number of subscriptions)
 */
size_t RMWPublisherEvent::subscription_count() const {
  // 对 subscriptions_mutex_ 上锁，以保护共享资源 subscriptions_ (Lock subscriptions_mutex_ to
  // protect shared resource subscriptions_)
  std::lock_guard<std::mutex> lock(subscriptions_mutex_);
  // 返回 subscriptions_ 集合的大小 (Return the size of the subscriptions_ set)
  return subscriptions_.size();
}

/**
 * @brief 更新截止日期状态 (Update deadline status)
 *
 * @param total_count 截止日期未满足的总次数 (Total number of times the deadline was not met)
 * @param total_count_change 自上次调用以来未满足截止日期的次数 (Number of times the deadline was
 * not met since the last call)
 */
void RMWPublisherEvent::update_deadline(uint32_t total_count, uint32_t total_count_change) {
  // 对 on_new_event_m_ 上锁，以保护共享资源 offered_deadline_missed_status_ (Lock on_new_event_m_
  // to protect shared resource offered_deadline_missed_status_)
  std::unique_lock<std::mutex> lock_mutex(on_new_event_m_);

  // 分配绝对值 (Assign absolute values)
  offered_deadline_missed_status_.total_count = total_count;
  // 累积增量 (Accumulate deltas)
  offered_deadline_missed_status_.total_count_change += total_count_change;

  // 标记截止日期已更改 (Mark deadline as changed)
  deadline_changed_ = true;

  // 触发 RMW_EVENT_OFFERED_DEADLINE_MISSED 事件 (Trigger RMW_EVENT_OFFERED_DEADLINE_MISSED event)
  trigger_event(RMW_EVENT_OFFERED_DEADLINE_MISSED);
}

/**
 * @brief 更新活跃度丢失状态 (Update liveliness lost status)
 *
 * @param total_count 活跃度丢失的总次数 (Total number of times liveliness was lost)
 * @param total_count_change 自上次调用以来活跃度丢失的次数 (Number of times liveliness was lost
 * since the last call)
 */
void RMWPublisherEvent::update_liveliness_lost(uint32_t total_count, uint32_t total_count_change) {
  // 对 on_new_event_m_ 上锁，以保护共享资源 liveliness_lost_status_ (Lock on_new_event_m_ to
  // protect shared resource liveliness_lost_status_)
  std::unique_lock<std::mutex> lock_mutex(on_new_event_m_);

  // 分配绝对值 (Assign absolute values)
  liveliness_lost_status_.total_count = total_count;
  // 累积增量 (Accumulate deltas)
  liveliness_lost_status_.total_count_change += total_count_change;

  // 标记活跃度已更改 (Mark liveliness as changed)
  liveliness_changed_ = true;

  // 触发 RMW_EVENT_LIVELINESS_LOST 事件 (Trigger RMW_EVENT_LIVELINESS_LOST event)
  trigger_event(RMW_EVENT_LIVELINESS_LOST);
}

/**
 * @brief 更新提供的不兼容QoS状态（Update the offered incompatible QoS status）
 *
 * @param[in] last_policy_id 最后一个不兼容的QoS策略ID（The last incompatible QoS policy ID）
 * @param[in] total_count 不兼容QoS策略的总数（Total count of incompatible QoS policies）
 * @param[in] total_count_change 自上次调用以来不兼容QoS策略的变化数量（Change in the number of
 * incompatible QoS policies since the last call）
 */
void RMWPublisherEvent::update_offered_incompatible_qos(
    eprosima::fastdds::dds::QosPolicyId_t last_policy_id,
    uint32_t total_count,
    uint32_t total_count_change) {
  // 对互斥量进行加锁（Lock the mutex）
  std::unique_lock<std::mutex> lock_mutex(on_new_event_m_);

  // 分配绝对值（Assign absolute values）
  incompatible_qos_status_.last_policy_id = last_policy_id;
  incompatible_qos_status_.total_count = total_count;
  // 累积增量（Accumulate deltas）
  incompatible_qos_status_.total_count_change += total_count_change;

  // 设置不兼容QoS已更改标志（Set the incompatible QoS changed flag）
  incompatible_qos_changed_ = true;

  // 触发事件（Trigger the event）
  trigger_event(RMW_EVENT_OFFERED_QOS_INCOMPATIBLE);
}

/**
 * @brief 更新不一致主题状态（Update the inconsistent topic status）
 *
 * @param[in] total_count 不一致主题的总数（Total count of inconsistent topics）
 * @param[in] total_count_change 自上次调用以来不一致主题的变化数量（Change in the number of
 * inconsistent topics since the last call）
 */
void RMWPublisherEvent::update_inconsistent_topic(
    uint32_t total_count, uint32_t total_count_change) {
  // 对互斥量进行加锁（Lock the mutex）
  std::unique_lock<std::mutex> lock_mutex(on_new_event_m_);

  // 分配绝对值（Assign absolute values）
  inconsistent_topic_status_.total_count = total_count;
  // 累积增量（Accumulate deltas）
  inconsistent_topic_status_.total_count_change += total_count_change;

  // 设置不一致主题已更改标志（Set the inconsistent topic changed flag）
  inconsistent_topic_changed_ = true;

  // 触发事件（Trigger the event）
  trigger_event(RMW_EVENT_PUBLISHER_INCOMPATIBLE_TYPE);
}

/**
 * @brief 更新匹配状态（Update the matched status）
 *
 * @param[in] total_count 匹配的总数（Total count of matches）
 * @param[in] total_count_change 自上次调用以来匹配的变化数量（Change in the number of matches since
 * the last call）
 * @param[in] current_count 当前匹配的数量（Current count of matches）
 * @param[in] current_count_change 自上次调用以来当前匹配的变化数量（Change in the current count of
 * matches since the last call）
 */
void RMWPublisherEvent::update_matched(
    int32_t total_count,
    int32_t total_count_change,
    int32_t current_count,
    int32_t current_count_change) {
  // 对互斥量进行加锁（Lock the mutex）
  std::lock_guard<std::mutex> lock(on_new_event_m_);

  // 更新匹配状态（Update the matched status）
  matched_status_.total_count = total_count;
  matched_status_.total_count_change += total_count_change;
  matched_status_.current_count = current_count;
  matched_status_.current_count_change += current_count_change;
  // 设置匹配更改标志（Set the matched changes flag）
  matched_changes_ = true;

  // 触发事件（Trigger the event）
  trigger_event(RMW_EVENT_PUBLICATION_MATCHED);
}

/**
 * @brief 触发指定类型的事件（Trigger the specified type of event）
 *
 * @param[in] event_type 要触发的事件类型（The type of event to trigger）
 */
void RMWPublisherEvent::trigger_event(rmw_event_type_t event_type) {
  // 如果存在回调函数，执行回调（If a callback exists, execute the callback）
  if (on_new_event_cb_[event_type]) {
    on_new_event_cb_[event_type](user_data_[event_type], 1);
  }

  // 设置事件触发值（Set the event trigger value）
  event_guard[event_type].set_trigger_value(true);
}
