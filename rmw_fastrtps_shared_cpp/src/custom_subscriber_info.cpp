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

#include "rmw_fastrtps_shared_cpp/custom_subscriber_info.hpp"

#include "event_helpers.hpp"
#include "fastdds/dds/core/status/DeadlineMissedStatus.hpp"
#include "fastdds/dds/core/status/LivelinessChangedStatus.hpp"
#include "types/event_types.hpp"

/**
 * @brief 获取订阅事件监听器 (Get the subscription event listener)
 *
 * @return EventListenerInterface* 返回订阅事件监听器指针 (Return a pointer to the subscription
 * event listener)
 */
EventListenerInterface *CustomSubscriberInfo::get_listener() const { return subscription_event_; }

/**
 * @brief CustomDataReaderListener 构造函数 (Constructor for CustomDataReaderListener)
 *
 * @param sub_event 订阅事件指针 (Pointer to the subscription event)
 */
CustomDataReaderListener::CustomDataReaderListener(RMWSubscriptionEvent *sub_event)
    : subscription_event_(sub_event) {}

/**
 * @brief 当订阅匹配时的回调函数 (Callback function when a subscription is matched)
 *
 * @param reader 数据读取器指针 (Pointer to the data reader)
 * @param info 匹配状态信息 (Matching status information)
 */
void CustomDataReaderListener::on_subscription_matched(
    eprosima::fastdds::dds::DataReader *reader,
    const eprosima::fastdds::dds::SubscriptionMatchedStatus &info) {
  (void)reader;  // 忽略未使用的参数 (Ignore unused parameter)

  if (info.current_count_change == 1) {
    // 跟踪唯一发布者 (Track unique publisher)
    subscription_event_->track_unique_publisher(
        eprosima::fastrtps::rtps::iHandle2GUID(info.last_publication_handle));
  } else if (info.current_count_change == -1) {
    // 取消跟踪唯一发布者 (Untrack unique publisher)
    subscription_event_->untrack_unique_publisher(
        eprosima::fastrtps::rtps::iHandle2GUID(info.last_publication_handle));
  } else {
    return;
  }

  // 更新匹配信息 (Update matched information)
  subscription_event_->update_matched(
      info.total_count, info.total_count_change, info.current_count, info.current_count_change);
}

/**
 * @brief 当有数据可用时的回调函数 (Callback function when data is available)
 *
 * @param reader 数据读取器指针 (Pointer to the data reader)
 */
void CustomDataReaderListener::on_data_available(eprosima::fastdds::dds::DataReader *reader) {
  (void)reader;  // 忽略未使用的参数 (Ignore unused parameter)

  // 更新数据可用状态 (Update data available status)
  subscription_event_->update_data_available();
}

/**
 * @brief 当请求的截止日期被错过时的回调函数 (Callback function when a requested deadline is missed)
 *
 * @param reader 数据读取器指针 (Pointer to the data reader)
 * @param status 请求截止日期错过状态信息 (Requested deadline missed status information)
 */
void CustomDataReaderListener::on_requested_deadline_missed(
    eprosima::fastdds::dds::DataReader *reader,
    const eprosima::fastdds::dds::RequestedDeadlineMissedStatus &status) {
  (void)reader;  // 忽略未使用的参数 (Ignore unused parameter)

  // 更新请求截止日期错过状态 (Update requested deadline missed status)
  subscription_event_->update_requested_deadline_missed(
      status.total_count, status.total_count_change);
}

/**
 * @brief 当数据读取器的生命周期发生变化时，此方法将被调用。
 *        This method is called when the liveliness of a data reader changes.
 *
 * @param[in] reader 数据读取器指针。Pointer to the data reader.
 * @param[in] status 生命周期状态信息。Liveliness status information.
 */
void CustomDataReaderListener::on_liveliness_changed(
    eprosima::fastdds::dds::DataReader *reader,
    const eprosima::fastdds::dds::LivelinessChangedStatus &status) {
  (void)reader;  // 忽略未使用的参数。Ignore unused parameter.

  // 更新订阅事件的生命周期状态。
  // Update the subscription event's liveliness status.
  subscription_event_->update_liveliness_changed(
      status.alive_count, status.not_alive_count, status.alive_count_change,
      status.not_alive_count_change);
}

/**
 * @brief 当数据读取器丢失样本时，此方法将被调用。
 *        This method is called when a sample is lost by the data reader.
 *
 * @param[in] reader 数据读取器指针。Pointer to the data reader.
 * @param[in] status 样本丢失状态信息。Sample lost status information.
 */
void CustomDataReaderListener::on_sample_lost(
    eprosima::fastdds::dds::DataReader *reader,
    const eprosima::fastdds::dds::SampleLostStatus &status) {
  (void)reader;  // 忽略未使用的参数。Ignore unused parameter.

  // 更新订阅事件的样本丢失状态。
  // Update the subscription event's sample lost status.
  subscription_event_->update_sample_lost(status.total_count, status.total_count_change);
}

/**
 * @brief 当数据读取器请求的 QoS 不兼容时，此方法将被调用。
 *        This method is called when the requested QoS of a data reader is incompatible.
 *
 * @param[in] reader 数据读取器指针。Pointer to the data reader.
 * @param[in] status 请求不兼容 QoS 的状态信息。Requested incompatible QoS status information.
 */
void CustomDataReaderListener::on_requested_incompatible_qos(
    eprosima::fastdds::dds::DataReader *reader,
    const eprosima::fastdds::dds::RequestedIncompatibleQosStatus &status) {
  (void)reader;  // 忽略未使用的参数。Ignore unused parameter.

  // 更新订阅事件的请求不兼容 QoS 状态。
  // Update the subscription event's requested incompatible QoS status.
  subscription_event_->update_requested_incompatible_qos(
      status.last_policy_id, status.total_count, status.total_count_change);
}

/**
 * @brief RMWSubscriptionEvent 构造函数。
 *        Constructor for RMWSubscriptionEvent.
 *
 * @param[in] info 自定义订阅者信息。Custom subscriber information.
 */
RMWSubscriptionEvent::RMWSubscriptionEvent(CustomSubscriberInfo *info)
    : subscriber_info_(info),
      deadline_changed_(false),
      liveliness_changed_(false),
      sample_lost_changed_(false),
      incompatible_qos_changed_(false),
      matched_changes_(false) {}

/**
 * @brief 获取订阅者的状态条件。
 *        Get the status condition of the subscriber.
 *
 * @return 订阅者的状态条件引用。Reference to the subscriber's status condition.
 */
eprosima::fastdds::dds::StatusCondition &RMWSubscriptionEvent::get_statuscondition() const {
  return subscriber_info_->data_reader_->get_statuscondition();
}

/**
 * @brief 从订阅者事件中获取事件信息
 * @param event_type 事件类型，例如 RMW_EVENT_LIVELINESS_CHANGED 等
 * @param event_info 存储事件信息的指针
 * @return 如果成功提取事件信息，则返回 true，否则返回 false
 *
 * @brief Takes event information from a subscription event
 * @param event_type The type of the event, e.g., RMW_EVENT_LIVELINESS_CHANGED, etc.
 * @param event_info Pointer to store the event information
 * @return Returns true if the event information is successfully extracted, otherwise returns false
 */
bool RMWSubscriptionEvent::take_event(rmw_event_type_t event_type, void *event_info) {
  // 检查事件类型是否受支持
  // Check if the event type is supported
  assert(rmw_fastrtps_shared_cpp::internal::is_event_supported(event_type));

  // 创建互斥锁，确保线程安全
  // Create a mutex lock to ensure thread safety
  std::unique_lock<std::mutex> lock_mutex(on_new_event_m_);

  // 根据事件类型处理不同的事件
  // Handle different events based on the event type
  switch (event_type) {
    case RMW_EVENT_LIVELINESS_CHANGED: {
      // 将 event_info 转换为 rmw_liveliness_changed_status_t 类型的指针
      // Cast event_info to a pointer of type rmw_liveliness_changed_status_t
      auto rmw_data = static_cast<rmw_liveliness_changed_status_t *>(event_info);
      if (liveliness_changed_) {
        liveliness_changed_ = false;
      } else {
        subscriber_info_->data_reader_->get_liveliness_changed_status(liveliness_changed_status_);
      }
      // 更新 rmw_data 的值
      // Update the values of rmw_data
      rmw_data->alive_count = liveliness_changed_status_.alive_count;
      rmw_data->not_alive_count = liveliness_changed_status_.not_alive_count;
      rmw_data->alive_count_change = liveliness_changed_status_.alive_count_change;
      rmw_data->not_alive_count_change = liveliness_changed_status_.not_alive_count_change;
      // 重置活跃状态计数器的变化值
      // Reset the change values of the liveliness status counters
      liveliness_changed_status_.alive_count_change = 0;
      liveliness_changed_status_.not_alive_count_change = 0;
    } break;
    case RMW_EVENT_REQUESTED_DEADLINE_MISSED: {
      // 将 event_info 转换为 rmw_requested_deadline_missed_status_t 类型的指针
      // Cast event_info to a pointer of type rmw_requested_deadline_missed_status_t
      auto rmw_data = static_cast<rmw_requested_deadline_missed_status_t *>(event_info);
      if (deadline_changed_) {
        deadline_changed_ = false;
      } else {
        subscriber_info_->data_reader_->get_requested_deadline_missed_status(
            requested_deadline_missed_status_);
      }
      // 更新 rmw_data 的值
      // Update the values of rmw_data
      rmw_data->total_count = requested_deadline_missed_status_.total_count;
      rmw_data->total_count_change = requested_deadline_missed_status_.total_count_change;
      // 重置请求截止日期未满足状态的总计数器变化值
      // Reset the total count change value of the requested deadline missed status
      requested_deadline_missed_status_.total_count_change = 0;
    } break;
    case RMW_EVENT_MESSAGE_LOST: {
      // 将 event_info 转换为 rmw_message_lost_status_t 类型的指针
      // Cast event_info to a pointer of type rmw_message_lost_status_t
      auto rmw_data = static_cast<rmw_message_lost_status_t *>(event_info);
      if (sample_lost_changed_) {
        sample_lost_changed_ = false;
      } else {
        subscriber_info_->data_reader_->get_sample_lost_status(sample_lost_status_);
      }
      // 更新 rmw_data 的值
      // Update the values of rmw_data
      rmw_data->total_count = sample_lost_status_.total_count;
      rmw_data->total_count_change = sample_lost_status_.total_count_change;
      // 重置样本丢失状态的总计数器变化值
      // Reset the total count change value of the sample lost status
      sample_lost_status_.total_count_change = 0;
    } break;
    case RMW_EVENT_REQUESTED_QOS_INCOMPATIBLE: {
      // 将 event_info 转换为 rmw_requested_qos_incompatible_event_status_t 类型的指针
      // Cast event_info to a pointer of type rmw_requested_qos_incompatible_event_status_t
      auto rmw_data = static_cast<rmw_requested_qos_incompatible_event_status_t *>(event_info);
      if (incompatible_qos_changed_) {
        incompatible_qos_changed_ = false;
      } else {
        subscriber_info_->data_reader_->get_requested_incompatible_qos_status(
            incompatible_qos_status_);
      }
      // 更新 rmw_data 的值
      // Update the values of rmw_data
      rmw_data->total_count = incompatible_qos_status_.total_count;
      rmw_data->total_count_change = incompatible_qos_status_.total_count_change;
      rmw_data->last_policy_kind =
          rmw_fastrtps_shared_cpp::internal::dds_qos_policy_to_rmw_qos_policy(
              incompatible_qos_status_.last_policy_id);
      // 重置请求的 QoS 不兼容状态的总计数器变化值
      // Reset the total count change value of the requested QoS incompatible status
      incompatible_qos_status_.total_count_change = 0;
    } break;
    case RMW_EVENT_SUBSCRIPTION_INCOMPATIBLE_TYPE: {
      // 将 event_info 转换为 rmw_incompatible_type_status_t 类型的指针
      // Cast event_info to a pointer of type rmw_incompatible_type_status_t
      auto rmw_data = static_cast<rmw_incompatible_type_status_t *>(event_info);
      if (inconsistent_topic_changed_) {
        inconsistent_topic_changed_ = false;
      } else {
        subscriber_info_->topic_->get_inconsistent_topic_status(inconsistent_topic_status_);
      }
      // 更新 rmw_data 的值
      // Update the values of rmw_data
      rmw_data->total_count = inconsistent_topic_status_.total_count;
      rmw_data->total_count_change = inconsistent_topic_status_.total_count_change;
      // 重置订阅不兼容类型状态的总计数器变化值
      // Reset the total count change value of the subscription incompatible type status
      inconsistent_topic_status_.total_count_change = 0;
    } break;
    case RMW_EVENT_SUBSCRIPTION_MATCHED: {
      // 将 event_info 转换为 rmw_matched_status_t 类型的指针
      auto rmw_data = static_cast<rmw_matched_status_t *>(event_info);

      eprosima::fastdds::dds::SubscriptionMatchedStatus matched_status;
      subscriber_info_->data_reader_->get_subscription_matched_status(matched_status);

      rmw_data->total_count = static_cast<size_t>(matched_status.total_count);
      rmw_data->total_count_change = static_cast<size_t>(matched_status.total_count_change);
      rmw_data->current_count = static_cast<size_t>(matched_status.current_count);
      rmw_data->current_count_change = matched_status.current_count_change;

      if (matched_changes_) {
        rmw_data->total_count_change += static_cast<size_t>(matched_status_.total_count_change);
        rmw_data->current_count_change += matched_status_.current_count_change;
        matched_changes_ = false;
      }

      matched_status_.total_count_change = 0;
      matched_status_.current_count_change = 0;
    } break;
    default:
      return false;
  }

  event_guard[event_type].set_trigger_value(false);
  return true;
}

/**
 * @brief 设置新事件回调函数
 * @param event_type 事件类型
 * @param user_data 用户数据，将传递给回调函数
 * @param callback 回调函数指针
 *
 * @brief Set the new event callback function
 * @param event_type The type of the event
 * @param user_data User data that will be passed to the callback function
 * @param callback Pointer to the callback function
 */
void RMWSubscriptionEvent::set_on_new_event_callback(
    rmw_event_type_t event_type, const void *user_data, rmw_event_callback_t callback) {
  // 加锁以确保线程安全
  // Lock to ensure thread safety
  std::unique_lock<std::mutex> lock_mutex(on_new_event_m_);

  // 获取当前 DataReader 的状态掩码
  // Get the current status mask of the DataReader
  eprosima::fastdds::dds::StatusMask status_mask =
      subscriber_info_->data_reader_->get_status_mask();

  if (callback) {
    switch (event_type) {
      case RMW_EVENT_LIVELINESS_CHANGED: {
        // 获取活跃度改变状态
        // Get the liveliness changed status
        subscriber_info_->data_reader_->get_liveliness_changed_status(liveliness_changed_status_);

        // 如果有活跃度改变，则触发回调函数
        // Trigger the callback function if there is a change in liveliness
        if ((liveliness_changed_status_.alive_count_change > 0) ||
            (liveliness_changed_status_.not_alive_count_change > 0)) {
          callback(
              user_data, liveliness_changed_status_.alive_count_change +
                             liveliness_changed_status_.not_alive_count_change);

          // 重置活跃度改变计数
          // Reset the liveliness change counts
          liveliness_changed_status_.alive_count_change = 0;
          liveliness_changed_status_.not_alive_count_change = 0;
        }
      } break;
      case RMW_EVENT_REQUESTED_DEADLINE_MISSED: {
        // 获取请求的截止时间未满足状态
        // Get the requested deadline missed status
        subscriber_info_->data_reader_->get_requested_deadline_missed_status(
            requested_deadline_missed_status_);

        // 如果有截止时间未满足，则触发回调函数
        // Trigger the callback function if there is a deadline missed
        if (requested_deadline_missed_status_.total_count_change > 0) {
          callback(user_data, requested_deadline_missed_status_.total_count_change);
          // 重置截止时间未满足计数
          // Reset the deadline missed count
          requested_deadline_missed_status_.total_count_change = 0;
        }
      } break;
      case RMW_EVENT_MESSAGE_LOST: {
        // 获取丢失的消息状态
        // Get the sample lost status
        subscriber_info_->data_reader_->get_sample_lost_status(sample_lost_status_);

        // 如果有消息丢失，则触发回调函数
        // Trigger the callback function if there is a message lost
        if (sample_lost_status_.total_count_change > 0) {
          callback(user_data, sample_lost_status_.total_count_change);
          // 重置消息丢失计数
          // Reset the message lost count
          sample_lost_status_.total_count_change = 0;
        }
      } break;
      case RMW_EVENT_REQUESTED_QOS_INCOMPATIBLE: {
        // 获取请求的 QoS 不兼容状态
        // Get the requested incompatible QoS status
        subscriber_info_->data_reader_->get_requested_incompatible_qos_status(
            incompatible_qos_status_);

        // 如果有 QoS 不兼容，则触发回调函数
        // Trigger the callback function if there is a QoS incompatibility
        if (incompatible_qos_status_.total_count_change > 0) {
          callback(user_data, incompatible_qos_status_.total_count_change);
          // 重置 QoS 不兼容计数
          // Reset the QoS incompatibility count
          incompatible_qos_status_.total_count_change = 0;
        }
      } break;
      case RMW_EVENT_SUBSCRIPTION_INCOMPATIBLE_TYPE: {
        // 获取订阅不兼容类型状态
        // Get the subscription incompatible type status
        subscriber_info_->topic_->get_inconsistent_topic_status(inconsistent_topic_status_);
        if (inconsistent_topic_status_.total_count_change > 0) {
          callback(user_data, inconsistent_topic_status_.total_count_change);
          // 重置订阅不兼容类型计数
          // Reset the subscription incompatible type count
          inconsistent_topic_status_.total_count_change = 0;
        }
      } break;
      case RMW_EVENT_SUBSCRIPTION_MATCHED: {
        if (matched_status_.total_count_change > 0) {
          callback(user_data, matched_status_.total_count_change);
          // 获取订阅匹配状态
          // Get the subscription matched status
          subscriber_info_->data_reader_->get_subscription_matched_status(matched_status_);
          // 重置订阅匹配计数
          // Reset the subscription matched counts
          matched_status_.total_count_change = 0;
          matched_status_.current_count_change = 0;
        }
      }
      default:
        break;
    }

    // 保存用户数据和事件回调
    user_data_[event_type] = user_data;
    on_new_event_cb_[event_type] = callback;

    status_mask |= rmw_fastrtps_shared_cpp::internal::rmw_event_to_dds_statusmask(event_type);
  } else {
    user_data_[event_type] = nullptr;
    on_new_event_cb_[event_type] = nullptr;

    // subscription_matched status should be kept enabled, since we need to
    // keep tracking matched publications
    if (RMW_EVENT_SUBSCRIPTION_MATCHED != event_type) {
      status_mask &= ~rmw_fastrtps_shared_cpp::internal::rmw_event_to_dds_statusmask(event_type);
    }
  }

  subscriber_info_->data_reader_->set_listener(
      subscriber_info_->data_reader_listener_, status_mask);
}

/**
 * @brief 设置新消息回调函数
 * @param user_data 用户数据，将传递给回调函数
 * @param callback 回调函数，当有新消息时触发
 *
 * @details 为订阅者设置一个回调函数，当有新消息到达时触发。
 */
// Set the new message callback function
// user_data: User data that will be passed to the callback function
// callback: The callback function that will be triggered when a new message arrives
void RMWSubscriptionEvent::set_on_new_message_callback(
    const void *user_data, rmw_event_callback_t callback) {
  if (callback) {
    // 获取未读消息数量
    // Get the number of unread messages
    auto unread_messages = subscriber_info_->data_reader_->get_unread_count(true);

    std::lock_guard<std::mutex> lock_mutex(on_new_message_m_);

    // 如果有未读消息，立即触发回调函数
    // If there are unread messages, trigger the callback function immediately
    if (0 < unread_messages) {
      callback(user_data, unread_messages);
    }

    // 设置用户数据和回调函数
    // Set user data and callback function
    new_message_user_data_ = user_data;
    on_new_message_cb_ = callback;

    // 更新状态掩码以包含数据可用状态
    // Update the status mask to include the data available status
    eprosima::fastdds::dds::StatusMask status_mask =
        subscriber_info_->data_reader_->get_status_mask();
    status_mask |= eprosima::fastdds::dds::StatusMask::data_available();
    subscriber_info_->data_reader_->set_listener(
        subscriber_info_->data_reader_listener_, status_mask);
  } else {
    std::lock_guard<std::mutex> lock_mutex(on_new_message_m_);

    // 更新状态掩码以移除数据可用状态
    // Update the status mask to remove the data available status
    eprosima::fastdds::dds::StatusMask status_mask =
        subscriber_info_->data_reader_->get_status_mask();
    status_mask &= ~eprosima::fastdds::dds::StatusMask::data_available();
    subscriber_info_->data_reader_->set_listener(
        subscriber_info_->data_reader_listener_, status_mask);

    // 清除用户数据和回调函数
    // Clear user data and callback function
    new_message_user_data_ = nullptr;
    on_new_message_cb_ = nullptr;
  }
}

/**
 * @brief 获取发布者数量
 * @return 发布者数量
 *
 * @details 返回当前跟踪的发布者数量。
 */
// Get the number of publishers
// return: The number of publishers
size_t RMWSubscriptionEvent::publisher_count() const {
  std::lock_guard<std::mutex> lock(publishers_mutex_);
  return publishers_.size();
}

/**
 * @brief 跟踪唯一发布者
 * @param guid 发布者的全局唯一标识符
 *
 * @details 将指定的发布者添加到跟踪列表中，确保每个发布者只被跟踪一次。
 */
// Track a unique publisher
// guid: The globally unique identifier of the publisher
// Add the specified publisher to the tracking list, ensuring that each publisher is tracked only
// once
void RMWSubscriptionEvent::track_unique_publisher(eprosima::fastrtps::rtps::GUID_t guid) {
  std::lock_guard<std::mutex> lock(publishers_mutex_);
  publishers_.insert(guid);
}

/**
 * @brief 删除唯一发布者 (Remove unique publisher)
 *
 * @param[in] guid 发布者的GUID (Publisher's GUID)
 */
void RMWSubscriptionEvent::untrack_unique_publisher(eprosima::fastrtps::rtps::GUID_t guid) {
  // 对publishers_mutex_进行加锁，防止多线程冲突 (Lock publishers_mutex_ to prevent multi-threading
  // conflicts)
  std::lock_guard<std::mutex> lock(publishers_mutex_);
  // 从publishers_中删除指定的guid (Remove the specified guid from publishers_)
  publishers_.erase(guid);
}

/**
 * @brief 更新数据可用性 (Update data availability)
 */
void RMWSubscriptionEvent::update_data_available() {
  // 对on_new_message_m_进行加锁 (Lock on_new_message_m_)
  std::unique_lock<std::mutex> lock_mutex(on_new_message_m_);

  // 检查是否有新消息回调 (Check if there is a new message callback)
  if (on_new_message_cb_) {
    // 获取未读消息数量 (Get the number of unread messages)
    auto unread_messages = subscriber_info_->data_reader_->get_unread_count(true);

    // 如果有未读消息，则触发回调 (If there are unread messages, trigger the callback)
    if (0 < unread_messages) {
      on_new_message_cb_(new_message_user_data_, unread_messages);
    }
  }
}

/**
 * @brief 更新请求的截止时间丢失事件 (Update requested deadline missed event)
 *
 * @param[in] total_count 总计数 (Total count)
 * @param[in] total_count_change 总计数变化 (Total count change)
 */
void RMWSubscriptionEvent::update_requested_deadline_missed(
    uint32_t total_count, uint32_t total_count_change) {
  // 对on_new_event_m_进行加锁 (Lock on_new_event_m_)
  std::unique_lock<std::mutex> lock_mutex(on_new_event_m_);

  // 分配绝对值 (Assign absolute values)
  requested_deadline_missed_status_.total_count = total_count;
  // 累计增量 (Accumulate deltas)
  requested_deadline_missed_status_.total_count_change += total_count_change;

  deadline_changed_ = true;

  // 触发事件 (Trigger event)
  trigger_event(RMW_EVENT_REQUESTED_DEADLINE_MISSED);
}

/**
 * @brief 更新生命周期改变事件 (Update liveliness changed event)
 *
 * @param[in] alive_count 存活计数 (Alive count)
 * @param[in] not_alive_count 非存活计数 (Not alive count)
 * @param[in] alive_count_change 存活计数变化 (Alive count change)
 * @param[in] not_alive_count_change 非存活计数变化 (Not alive count change)
 */
void RMWSubscriptionEvent::update_liveliness_changed(
    uint32_t alive_count,
    uint32_t not_alive_count,
    uint32_t alive_count_change,
    uint32_t not_alive_count_change) {
  // 对on_new_event_m_进行加锁 (Lock on_new_event_m_)
  std::unique_lock<std::mutex> lock_mutex(on_new_event_m_);

  // 分配绝对值 (Assign absolute values)
  liveliness_changed_status_.alive_count = alive_count;
  liveliness_changed_status_.not_alive_count = not_alive_count;
  // 累计增量 (Accumulate deltas)
  liveliness_changed_status_.alive_count_change += alive_count_change;
  liveliness_changed_status_.not_alive_count_change += not_alive_count_change;

  liveliness_changed_ = true;

  // 触发事件 (Trigger event)
  trigger_event(RMW_EVENT_LIVELINESS_CHANGED);
}

/**
 * @brief 更新丢失的样本数量 (Update the number of lost samples)
 *
 * @param total_count 总丢失样本数 (Total number of lost samples)
 * @param total_count_change 丢失样本数变化量 (Change in the number of lost samples)
 */
void RMWSubscriptionEvent::update_sample_lost(uint32_t total_count, uint32_t total_count_change) {
  // 对互斥锁进行加锁，确保线程安全 (Lock the mutex to ensure thread safety)
  std::lock_guard<std::mutex> lock_mutex(on_new_event_m_);

  // 分配绝对值 (Assign absolute values)
  sample_lost_status_.total_count = total_count;
  // 累积增量 (Accumulate deltas)
  sample_lost_status_.total_count_change += total_count_change;

  // 设置样本丢失状态已更改 (Set the sample lost status as changed)
  sample_lost_changed_ = true;

  // 触发事件 (Trigger the event)
  trigger_event(RMW_EVENT_MESSAGE_LOST);
}

/**
 * @brief 更新请求的不兼容 QoS (Update requested incompatible QoS)
 *
 * @param last_policy_id 最后一个不兼容的 QoS 策略 ID (Last incompatible QoS policy ID)
 * @param total_count 总不兼容 QoS 数 (Total number of incompatible QoS)
 * @param total_count_change 不兼容 QoS 数变化量 (Change in the number of incompatible QoS)
 */
void RMWSubscriptionEvent::update_requested_incompatible_qos(
    eprosima::fastdds::dds::QosPolicyId_t last_policy_id,
    uint32_t total_count,
    uint32_t total_count_change) {
  // 对互斥锁进行加锁，确保线程安全 (Lock the mutex to ensure thread safety)
  std::lock_guard<std::mutex> lock_mutex(on_new_event_m_);

  // 分配绝对值 (Assign absolute values)
  incompatible_qos_status_.last_policy_id = last_policy_id;
  incompatible_qos_status_.total_count = total_count;
  // 累积增量 (Accumulate deltas)
  incompatible_qos_status_.total_count_change += total_count_change;

  // 设置不兼容 QoS 状态已更改 (Set the incompatible QoS status as changed)
  incompatible_qos_changed_ = true;

  // 触发事件 (Trigger the event)
  trigger_event(RMW_EVENT_REQUESTED_QOS_INCOMPATIBLE);
}

/**
 * @brief 更新不一致主题 (Update inconsistent topic)
 *
 * @param total_count 总不一致主题数 (Total number of inconsistent topics)
 * @param total_count_change 不一致主题数变化量 (Change in the number of inconsistent topics)
 */
void RMWSubscriptionEvent::update_inconsistent_topic(
    uint32_t total_count, uint32_t total_count_change) {
  // 对互斥锁进行加锁，确保线程安全 (Lock the mutex to ensure thread safety)
  std::lock_guard<std::mutex> lock_mutex(on_new_event_m_);

  // 分配绝对值 (Assign absolute values)
  inconsistent_topic_status_.total_count = total_count;
  // 累积增量 (Accumulate deltas)
  inconsistent_topic_status_.total_count_change += total_count_change;

  // 设置不一致主题状态已更改 (Set the inconsistent topic status as changed)
  inconsistent_topic_changed_ = true;

  // 触发事件 (Trigger the event)
  trigger_event(RMW_EVENT_SUBSCRIPTION_INCOMPATIBLE_TYPE);
}

/**
 * @brief 更新匹配状态 (Update the matched status)
 *
 * @param total_count 总匹配次数 (Total number of matches)
 * @param total_count_change 总匹配次数变化 (Change in total number of matches)
 * @param current_count 当前匹配次数 (Current number of matches)
 * @param current_count_change 当前匹配次数变化 (Change in current number of matches)
 */
void RMWSubscriptionEvent::update_matched(
    int32_t total_count,
    int32_t total_count_change,
    int32_t current_count,
    int32_t current_count_change) {
  // 锁定互斥量，防止多线程问题 (Lock the mutex to prevent multi-threading issues)
  std::lock_guard<std::mutex> lock(on_new_event_m_);

  // 更新总匹配次数 (Update the total count of matches)
  matched_status_.total_count = total_count;
  // 更新总匹配次数变化 (Update the change in total count of matches)
  matched_status_.total_count_change += total_count_change;
  // 更新当前匹配次数 (Update the current count of matches)
  matched_status_.current_count = current_count;
  // 更新当前匹配次数变化 (Update the change in current count of matches)
  matched_status_.current_count_change += current_count_change;
  // 设置匹配状态更改标志 (Set the matched changes flag)
  matched_changes_ = true;

  // 触发订阅匹配事件 (Trigger the subscription matched event)
  trigger_event(RMW_EVENT_SUBSCRIPTION_MATCHED);
}

/**
 * @brief 触发指定类型的事件 (Trigger the specified type of event)
 *
 * @param event_type 事件类型 (Event type)
 */
void RMWSubscriptionEvent::trigger_event(rmw_event_type_t event_type) {
  // 如果事件回调存在，则调用它 (If the event callback exists, call it)
  if (on_new_event_cb_[event_type]) {
    on_new_event_cb_[event_type](user_data_[event_type], 1);
  }

  // 设置触发器值为 true (Set the trigger value to true)
  event_guard[event_type].set_trigger_value(true);
}
