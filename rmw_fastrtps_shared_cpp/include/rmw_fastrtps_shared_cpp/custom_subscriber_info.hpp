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

#ifndef RMW_FASTRTPS_SHARED_CPP__CUSTOM_SUBSCRIBER_INFO_HPP_
#define RMW_FASTRTPS_SHARED_CPP__CUSTOM_SUBSCRIBER_INFO_HPP_

#include <algorithm>
#include <limits>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>

#include "fastdds/dds/core/status/DeadlineMissedStatus.hpp"
#include "fastdds/dds/core/status/LivelinessChangedStatus.hpp"
#include "fastdds/dds/core/status/SubscriptionMatchedStatus.hpp"
#include "fastdds/dds/subscriber/DataReader.hpp"
#include "fastdds/dds/subscriber/DataReaderListener.hpp"
#include "fastdds/dds/subscriber/qos/DataReaderQos.hpp"
#include "fastdds/dds/topic/ContentFilteredTopic.hpp"
#include "fastdds/dds/topic/TypeSupport.hpp"
#include "fastdds/rtps/common/Guid.h"
#include "fastdds/rtps/common/InstanceHandle.h"
#include "rcpputils/thread_safety_annotations.hpp"
#include "rmw/event_callback_type.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw_dds_common/context.hpp"
#include "rmw_fastrtps_shared_cpp/custom_event_info.hpp"

class RMWSubscriptionEvent;

/**
 * @class CustomDataReaderListener
 * @brief 自定义数据读取监听器类，继承自 eprosima::fastdds::dds::DataReaderListener
 *        Custom data reader listener class, inheriting from
 * eprosima::fastdds::dds::DataReaderListener
 */
class CustomDataReaderListener final : public eprosima::fastdds::dds::DataReaderListener {
public:
  /**
   * @brief 构造函数
   *        Constructor
   * @param sub_event 订阅事件指针
   *                  Pointer to the subscription event
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  explicit CustomDataReaderListener(RMWSubscriptionEvent* sub_event);

  /**
   * @brief 当订阅匹配时调用
   *        Called when a subscription is matched
   * @param reader 数据读取器指针
   *               Pointer to the data reader
   * @param info 匹配状态信息
   *             Subscription matched status information
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void on_subscription_matched(
      eprosima::fastdds::dds::DataReader* reader,
      const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;

  /**
   * @brief 当有数据可用时调用
   *        Called when data is available
   * @param reader 数据读取器指针
   *               Pointer to the data reader
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void on_data_available(eprosima::fastdds::dds::DataReader* reader) override;

  /**
   * @brief 当请求的截止时间被错过时调用
   *        Called when the requested deadline is missed
   * @param reader 数据读取器指针
   *               Pointer to the data reader
   * @param status 请求截止时间错过状态信息
   *               Requested deadline missed status information
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void on_requested_deadline_missed(
      eprosima::fastdds::dds::DataReader* reader,
      const eprosima::fastrtps::RequestedDeadlineMissedStatus& status) override;

  /**
   * @brief 当生命周期改变时调用
   *        Called when liveliness changes
   * @param reader 数据读取器指针
   *               Pointer to the data reader
   * @param status 生命周期改变状态信息
   *               Liveliness changed status information
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void on_liveliness_changed(
      eprosima::fastdds::dds::DataReader* reader,
      const eprosima::fastrtps::LivelinessChangedStatus& status) override;

  /**
   * @brief 当样本丢失时调用
   *        Called when a sample is lost
   * @param reader 数据读取器指针
   *               Pointer to the data reader
   * @param status 样本丢失状态信息
   *               Sample lost status information
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void on_sample_lost(
      eprosima::fastdds::dds::DataReader* reader,
      const eprosima::fastdds::dds::SampleLostStatus& status) override;

  /**
   * @brief 当请求的 QoS 不兼容时调用
   *        Called when requested QoS is incompatible
   * @param reader 数据读取器指针
   *               Pointer to the data reader
   * @param status 请求的不兼容 QoS 状态信息
   *               Requested incompatible QoS status information
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void on_requested_incompatible_qos(
      eprosima::fastdds::dds::DataReader* reader,
      const eprosima::fastdds::dds::RequestedIncompatibleQosStatus& status) override;

private:
  /**
   * @brief 订阅事件指针
   *        Pointer to the subscription event
   */
  RMWSubscriptionEvent* subscription_event_;
};

namespace rmw_fastrtps_shared_cpp {
struct LoanManager;
}  // namespace rmw_fastrtps_shared_cpp

// 自定义订阅者信息结构体，继承自自定义事件信息类
// Custom subscriber information structure, inheriting from the custom event information class
struct CustomSubscriberInfo : public CustomEventInfo {
  // 虚析构函数
  // Virtual destructor
  virtual ~CustomSubscriberInfo() = default;

  // 数据读取器指针
  // Data reader pointer
  eprosima::fastdds::dds::DataReader* data_reader_{nullptr};
  // 订阅事件指针
  // Subscription event pointer
  RMWSubscriptionEvent* subscription_event_{nullptr};
  // 数据读取器监听器指针
  // Data reader listener pointer
  CustomDataReaderListener* data_reader_listener_{nullptr};
  // 类型支持实例
  // Type support instance
  eprosima::fastdds::dds::TypeSupport type_support_;
  // 类型支持实现指针
  // Type support implementation pointer
  const void* type_support_impl_{nullptr};
  // 订阅者全局唯一标识符
  // Subscriber global unique identifier
  rmw_gid_t subscription_gid_{};
  // 类型支持标识符
  // Type support identifier
  const char* typesupport_identifier_{nullptr};
  // 贷款管理器共享指针
  // Loan manager shared pointer
  std::shared_ptr<rmw_fastrtps_shared_cpp::LoanManager> loan_manager_;

  // 用于重新创建或删除内容过滤主题
  // For re-creating or deleting content filtered topics
  const rmw_node_t* node_{nullptr};
  // 公共上下文指针
  // Common context pointer
  rmw_dds_common::Context* common_context_{nullptr};
  // DDS 域参与者指针
  // DDS domain participant pointer
  eprosima::fastdds::dds::DomainParticipant* dds_participant_{nullptr};
  // 订阅者指针
  // Subscriber pointer
  eprosima::fastdds::dds::Subscriber* subscriber_{nullptr};
  // 主题名称（经过处理）
  // Mangled topic name
  std::string topic_name_mangled_;
  // 主题指针
  // Topic pointer
  eprosima::fastdds::dds::Topic* topic_{nullptr};
  // 内容过滤主题指针
  // Content filtered topic pointer
  eprosima::fastdds::dds::ContentFilteredTopic* filtered_topic_{nullptr};
  // 数据读取器 QoS 实例
  // Data reader QoS instance
  eprosima::fastdds::dds::DataReaderQos datareader_qos_;

  // 获取监听器接口的公共方法
  // Public method to get the listener interface
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  EventListenerInterface* get_listener() const final;
};

/**
 * @class RMWSubscriptionEvent
 * @brief RMWSubscriptionEvent 是一个继承自 EventListenerInterface 的类，用于处理订阅者事件。
 *        (RMWSubscriptionEvent is a class that inherits from EventListenerInterface, used for
 * handling subscriber events.)
 */
class RMWSubscriptionEvent final : public EventListenerInterface {
public:
  /**
   * @brief 构造函数，初始化 RMWSubscriptionEvent 对象。
   *        (Constructor, initializes the RMWSubscriptionEvent object.)
   * @param info CustomSubscriberInfo 指针，包含订阅者信息。
   *             (CustomSubscriberInfo pointer, contains subscriber information.)
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  explicit RMWSubscriptionEvent(CustomSubscriberInfo* info);

  /**
   * @brief 设置新消息回调函数。
   *        (Set the callback function for new messages.)
   * @param user_data 用户数据指针。
   *                  (User data pointer.)
   * @param callback 回调函数。
   *                 (Callback function.)
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void set_on_new_message_callback(const void* user_data, rmw_event_callback_t callback);

  /**
   * @brief 获取状态条件。
   *        (Get the status condition.)
   * @return StatusCondition 引用。
   *         (StatusCondition reference.)
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  eprosima::fastdds::dds::StatusCondition& get_statuscondition() const override;

  /**
   * @brief 获取事件。
   *        (Take the event.)
   * @param event_type 事件类型。
   *                   (Event type.)
   * @param event_info 事件信息指针。
   *                   (Event info pointer.)
   * @return 是否成功获取事件。
   *         (Whether the event was successfully taken.)
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  bool take_event(rmw_event_type_t event_type, void* event_info) override;

  /**
   * @brief 设置新事件回调函数。
   *        (Set the callback function for new events.)
   * @param event_type 事件类型。
   *                   (Event type.)
   * @param user_data 用户数据指针。
   *                  (User data pointer.)
   * @param callback 回调函数。
   *                 (Callback function.)
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void set_on_new_event_callback(
      rmw_event_type_t event_type, const void* user_data, rmw_event_callback_t callback) override;

  // void
  // on_type_discovery(
  //   DomainParticipant *,
  //   const eprosima::fastrtps::rtps::SampleIdentity &,
  //   const eprosima::fastrtps::string_255 & topic_name,
  //   const eprosima::fastrtps::types::TypeIdentifier *,
  //   const eprosima::fastrtps::types::TypeObject *,
  //   eprosima::fastrtps::types::DynamicType_ptr dyn_type)) final
  // {
  // NOTE(methylDragon): The dynamic type deferred case is !! NOT SUPPORTED !!
  //                     This is because currently subscriptions are required to have the type at
  //                     construction to create the listener. Deferring it means that the listener
  //                     construction will have to be deferred, and that would require logic changes
  //                     elsewhere (e.g. to check for listener initialization status), which is
  // }

  /**
   * @brief 更新不一致主题。
   *        (Update inconsistent topic.)
   * @param total_count 总计数。
   *                    (Total count.)
   * @param total_count_change 总计数变化。
   *                           (Total count change.)
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void update_inconsistent_topic(uint32_t total_count, uint32_t total_count_change) override;

  /// 添加一个 GUID 到内部唯一发布者集合，该集合与此订阅匹配。
  /// Add a GUID to the internal set of unique publishers matched to this subscription.
  /**
   * 这样我们就可以在用户调用 rmw_count_publishers() 时为 RMW 层提供准确的匹配发布者计数。
   * This is so we can provide the RMW layer with an accurate count of matched publishers if the
   * user calls rmw_count_publishers().
   *
   * \param[in] guid 要跟踪的新匹配发布者的 GUID。
   * \param[in] guid The GUID of the newly-matched publisher to track.
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void track_unique_publisher(eprosima::fastrtps::rtps::GUID_t guid);

  /// 从与此订阅匹配的内部唯一发布者集合中删除一个 GUID。
  /// Remove a GUID from the internal set of unique publishers matched to this subscription.
  /**
   * 这样我们就可以在用户调用 rmw_count_publishers() 时为 RMW 层提供准确的匹配发布者计数。
   * This is so we can provide the RMW layer with an accurate count of matched publishers if the
   * user calls rmw_count_publishers().
   *
   * \param[in] guid 要跟踪的新取消匹配发布者的 GUID。
   * \param[in] guid The GUID of the newly-unmatched publisher to track.
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void untrack_unique_publisher(eprosima::fastrtps::rtps::GUID_t guid);

  /// 返回与此订阅匹配的唯一发布者数量。
  /// Return the number of unique publishers matched to this subscription.
  /**
   * \return 与此订阅匹配的唯一发布者数量。
   * \return Number of unique publishers matched to this subscription.
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  size_t publisher_count() const;

  // 更新数据可用性。
  // Update data availability.
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void update_data_available();

  // 更新请求的截止日期未满足的次数。
  // Update requested deadline missed count.
  /**
   * \param[in] total_count 总的截止日期未满足次数。
   * \param[in] total_count Total count of deadline missed.
   * \param[in] total_count_change 截止日期未满足次数的变化。
   * \param[in] total_count_change Change in count of deadline missed.
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void update_requested_deadline_missed(uint32_t total_count, uint32_t total_count_change);

  /**
   * @brief 更新生命周期改变状态 (Update liveliness changed status)
   *
   * @param[in] alive_count 存活实体的数量 (Number of alive entities)
   * @param[in] not_alive_count 非存活实体的数量 (Number of not alive entities)
   * @param[in] alive_count_change 存活实体数量的变化 (Change in the number of alive entities)
   * @param[in] not_alive_count_change 非存活实体数量的变化 (Change in the number of not alive
   * entities)
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void update_liveliness_changed(
      uint32_t alive_count,
      uint32_t not_alive_count,
      uint32_t alive_count_change,
      uint32_t not_alive_count_change);

  /**
   * @brief 更新样本丢失状态 (Update sample lost status)
   *
   * @param[in] total_count 总丢失样本数 (Total number of lost samples)
   * @param[in] total_count_change 丢失样本数的变化 (Change in the number of lost samples)
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void update_sample_lost(uint32_t total_count, uint32_t total_count_change);

  /**
   * @brief 更新请求的不兼容 QoS 状态 (Update requested incompatible QoS status)
   *
   * @param[in] last_policy_id 最后一个不兼容的 QoS 策略 ID (Last incompatible QoS policy ID)
   * @param[in] total_count 总不兼容次数 (Total number of incompatible occurrences)
   * @param[in] total_count_change 不兼容次数的变化 (Change in the number of incompatible
   * occurrences)
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void update_requested_incompatible_qos(
      eprosima::fastdds::dds::QosPolicyId_t last_policy_id,
      uint32_t total_count,
      uint32_t total_count_change);

  /**
   * @brief 更新匹配状态 (Update matched status)
   *
   * @param[in] total_count 总匹配次数 (Total number of matches)
   * @param[in] total_count_change 匹配次数的变化 (Change in the number of matches)
   * @param[in] current_count 当前匹配次数 (Current number of matches)
   * @param[in] current_count_change 当前匹配次数的变化 (Change in the current number of matches)
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void update_matched(
      int32_t total_count,
      int32_t total_count_change,
      int32_t current_count,
      int32_t current_count_change);

private:
  /**
   * @brief CustomSubscriberInfo 类的实例指针 (Pointer to an instance of CustomSubscriberInfo class)
   */
  CustomSubscriberInfo* subscriber_info_ = nullptr;

  /**
   * @brief deadline_changed_ 标志，表示 deadline 是否发生变化 (Flag indicating if the deadline has
   * changed)
   */
  bool deadline_changed_ RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

  /**
   * @brief 请求的 Deadline Missed 状态 (Requested Deadline Missed status)
   */
  eprosima::fastdds::dds::RequestedDeadlineMissedStatus requested_deadline_missed_status_
      RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

  /**
   * @brief liveliness_changed_ 标志，表示 liveliness 是否发生变化 (Flag indicating if the
   * liveliness has changed)
   */
  bool liveliness_changed_ RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

  /**
   * @brief Liveliness Changed 状态 (Liveliness Changed status)
   */
  eprosima::fastdds::dds::LivelinessChangedStatus liveliness_changed_status_
      RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

  /**
   * @brief sample_lost_changed_ 标志，表示 sample lost 是否发生变化 (Flag indicating if the sample
   * lost has changed)
   */
  bool sample_lost_changed_ RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

  /**
   * @brief Sample Lost 状态 (Sample Lost status)
   */
  eprosima::fastdds::dds::SampleLostStatus sample_lost_status_
      RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

  /**
   * @brief incompatible_qos_changed_ 标志，表示 incompatible QoS 是否发生变化 (Flag indicating if
   * the incompatible QoS has changed)
   */
  bool incompatible_qos_changed_ RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

  /**
   * @brief 请求的 Incompatible QoS 状态 (Requested Incompatible QoS status)
   */
  eprosima::fastdds::dds::RequestedIncompatibleQosStatus incompatible_qos_status_
      RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

  /**
   * @brief matched_changes_ 标志，表示匹配状态是否发生变化 (Flag indicating if the matched status
   * has changed)
   */
  bool matched_changes_ RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

  /**
   * @brief Subscription Matched 状态 (Subscription Matched status)
   */
  eprosima::fastdds::dds::SubscriptionMatchedStatus matched_status_
      RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

  /**
   * @brief 与当前订阅者匹配的发布者集合 (Set of publishers matched with the current subscriber)
   */
  std::set<eprosima::fastrtps::rtps::GUID_t> publishers_
      RCPPUTILS_TSA_GUARDED_BY(publishers_mutex_);

  /**
   * @brief 新消息事件回调函数 (Callback function for new message events)
   */
  rmw_event_callback_t on_new_message_cb_{nullptr};

  /**
   * @brief 新消息事件的用户数据指针 (Pointer to user data for new message events)
   */
  const void* new_message_user_data_{nullptr};

  /**
   * @brief 保护新消息事件的互斥锁 (Mutex protecting new message events)
   */
  std::mutex on_new_message_m_;

  /**
   * @brief 保护发布者集合的互斥锁 (Mutex protecting the set of publishers)
   */
  mutable std::mutex publishers_mutex_;

  /**
   * @brief 触发特定类型的事件 (Trigger an event of a specific type)
   *
   * @param event_type 要触发的事件类型 (The type of event to trigger)
   */
  void trigger_event(rmw_event_type_t event_type);
};

#endif  // RMW_FASTRTPS_SHARED_CPP__CUSTOM_SUBSCRIBER_INFO_HPP_
