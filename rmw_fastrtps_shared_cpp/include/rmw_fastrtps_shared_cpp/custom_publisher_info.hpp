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

#ifndef RMW_FASTRTPS_SHARED_CPP__CUSTOM_PUBLISHER_INFO_HPP_
#define RMW_FASTRTPS_SHARED_CPP__CUSTOM_PUBLISHER_INFO_HPP_

#include <mutex>
#include <set>

#include "fastdds/dds/core/policy/QosPolicies.hpp"
#include "fastdds/dds/core/status/BaseStatus.hpp"
#include "fastdds/dds/core/status/DeadlineMissedStatus.hpp"
#include "fastdds/dds/core/status/IncompatibleQosStatus.hpp"
#include "fastdds/dds/core/status/PublicationMatchedStatus.hpp"
#include "fastdds/dds/publisher/DataWriter.hpp"
#include "fastdds/dds/publisher/DataWriterListener.hpp"
#include "fastdds/dds/topic/Topic.hpp"
#include "fastdds/dds/topic/TypeSupport.hpp"
#include "fastdds/rtps/common/Guid.h"
#include "fastdds/rtps/common/InstanceHandle.h"
#include "rcpputils/thread_safety_annotations.hpp"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/custom_event_info.hpp"

class RMWPublisherEvent;

/**
 * @class CustomDataWriterListener
 * @brief 自定义数据写入监听器类，继承自 eprosima::fastdds::dds::DataWriterListener
 *        Custom data writer listener class, inheriting from
 * eprosima::fastdds::dds::DataWriterListener
 */
class CustomDataWriterListener final : public eprosima::fastdds::dds::DataWriterListener {
public:
  /**
   * @brief 构造函数
   *        Constructor
   * @param pub_event RMWPublisherEvent类型的指针
   *                  Pointer of type RMWPublisherEvent
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  explicit CustomDataWriterListener(RMWPublisherEvent *pub_event);

  /**
   * @brief 当发布匹配时触发的回调函数
   *        Callback function triggered when publication matched
   * @param writer DataWriter类型的指针
   *               Pointer of type DataWriter
   * @param status PublicationMatchedStatus类型的引用
   *               Reference of type PublicationMatchedStatus
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void on_publication_matched(
      eprosima::fastdds::dds::DataWriter *writer,
      const eprosima::fastdds::dds::PublicationMatchedStatus &status) override;

  /**
   * @brief 当提供的截止时间错过时触发的回调函数
   *        Callback function triggered when offered deadline missed
   * @param writer DataWriter类型的指针
   *               Pointer of type DataWriter
   * @param status OfferedDeadlineMissedStatus类型的引用
   *               Reference of type OfferedDeadlineMissedStatus
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void on_offered_deadline_missed(
      eprosima::fastdds::dds::DataWriter *writer,
      const eprosima::fastdds::dds::OfferedDeadlineMissedStatus &status) override;

  /**
   * @brief 当生命周期丢失时触发的回调函数
   *        Callback function triggered when liveliness lost
   * @param writer DataWriter类型的指针
   *               Pointer of type DataWriter
   * @param status LivelinessLostStatus类型的引用
   *               Reference of type LivelinessLostStatus
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void on_liveliness_lost(
      eprosima::fastdds::dds::DataWriter *writer,
      const eprosima::fastdds::dds::LivelinessLostStatus &status) override;

  /**
   * @brief 当提供的QoS不兼容时触发的回调函数
   *        Callback function triggered when offered incompatible QoS
   * @param writer DataWriter类型的指针
   *               Pointer of type DataWriter
   * @param status OfferedIncompatibleQosStatus类型的引用
   *               Reference of type OfferedIncompatibleQosStatus
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void on_offered_incompatible_qos(
      eprosima::fastdds::dds::DataWriter *,
      const eprosima::fastdds::dds::OfferedIncompatibleQosStatus &status) override;

private:
  /**
   * @brief 发布者事件指针
   *        Publisher event pointer
   */
  RMWPublisherEvent *publisher_event_;
};

// 定义一个结构体 CustomPublisherInfo，继承自 CustomEventInfo 类型。
// Define a struct named CustomPublisherInfo, which inherits from the CustomEventInfo type.
typedef struct CustomPublisherInfo : public CustomEventInfo {
  // 虚析构函数，用于在派生类中正确地释放资源。
  // Virtual destructor, used to correctly release resources in derived classes.
  virtual ~CustomPublisherInfo() = default;

  // DataWriter 指针，用于发布数据。
  // DataWriter pointer, used for publishing data.
  eprosima::fastdds::dds::DataWriter *data_writer_{nullptr};

  // RMWPublisherEvent 指针，用于处理与发布者相关的事件。
  // RMWPublisherEvent pointer, used for handling events related to the publisher.
  RMWPublisherEvent *publisher_event_{nullptr};

  // CustomDataWriterListener 指针，用于监听数据写入事件。
  // CustomDataWriterListener pointer, used for listening to data writing events.
  CustomDataWriterListener *data_writer_listener_{nullptr};

  // 类型支持对象，用于序列化和反序列化数据。
  // Type support object, used for serialization and deserialization of data.
  eprosima::fastdds::dds::TypeSupport type_support_;

  // 类型支持实现的指针，用于存储类型支持的具体实现。
  // Pointer to the type support implementation, used for storing the specific implementation of
  // type support.
  const void *type_support_impl_{nullptr};

  // 发布者的全局唯一标识符。
  // Global unique identifier for the publisher.
  rmw_gid_t publisher_gid{};

  // 类型支持标识符，用于区分不同的类型支持。
  // Type support identifier, used to distinguish different type supports.
  const char *typesupport_identifier_{nullptr};

  // Topic 指针，表示发布者所使用的主题。
  // Topic pointer, representing the topic used by the publisher.
  eprosima::fastdds::dds::Topic *topic_{nullptr};

  // RMW_FASTRTPS_SHARED_CPP_PUBLIC 宏定义，用于设置符号可见性。
  // RMW_FASTRTPS_SHARED_CPP_PUBLIC macro definition, used for setting symbol visibility.
  RMW_FASTRTPS_SHARED_CPP_PUBLIC

  // 最终修饰符，表示在派生类中不能再被覆盖。返回 EventListenerInterface 指针，用于获取监听器对象。
  // Final specifier, indicating that it cannot be overridden in derived classes. Returns an
  // EventListenerInterface pointer, used for getting the listener object.
  EventListenerInterface *get_listener() const final;
} CustomPublisherInfo;

/**
 * @class RMWPublisherEvent
 * @brief RMWPublisherEvent 类是一个继承自 EventListenerInterface 的类，用于处理 ROS2
 * rmw_fastrtps_cpp 层的发布者事件. The RMWPublisherEvent class is a class that inherits from
 * EventListenerInterface, used to handle publisher events in the ROS2 rmw_fastrtps_cpp layer.
 */
class RMWPublisherEvent final : public EventListenerInterface {
public:
  /**
   * @brief 构造函数，初始化 RMWPublisherEvent 对象.
   * Constructor, initializes an RMWPublisherEvent object.
   *
   * @param info 指向 CustomPublisherInfo 结构体的指针.
   * @param info A pointer to the CustomPublisherInfo structure.
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  explicit RMWPublisherEvent(CustomPublisherInfo *info);

  /**
   * @brief 获取 StatusCondition 引用.
   * Get the reference to the StatusCondition.
   *
   * @return StatusCondition 的引用.
   * @return A reference to the StatusCondition.
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  eprosima::fastdds::dds::StatusCondition &get_statuscondition() const override;

  /**
   * @brief 获取特定类型的事件信息.
   * Take the event information of a specific type.
   *
   * @param event_type 要获取的事件类型.
   * @param event_info 存储事件信息的指针.
   * @return 是否成功获取事件信息.
   * @param event_type The type of event to take.
   * @param event_info Pointer to store the event information.
   * @return Whether the event information was successfully taken.
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  bool take_event(rmw_event_type_t event_type, void *event_info) override;

  /**
   * @brief 设置新事件回调函数.
   * Set the callback function for new events.
   *
   * @param event_type 要设置回调的事件类型.
   * @param user_data 用户数据指针.
   * @param callback 回调函数.
   * @param event_type The type of event to set the callback for.
   * @param user_data Pointer to user data.
   * @param callback The callback function.
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void set_on_new_event_callback(
      rmw_event_type_t event_type, const void *user_data, rmw_event_callback_t callback) override;

  /**
   * @brief 更新不一致主题计数.
   * Update the inconsistent topic count.
   *
   * @param total_count 总计数.
   * @param total_count_change 计数变化值.
   * @param total_count The total count.
   * @param total_count_change The count change value.
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void update_inconsistent_topic(uint32_t total_count, uint32_t total_count_change) override;

  /// 添加一个 GUID 到与此发布者匹配的唯一订阅集合中。
  /// Add a GUID to the internal set of unique subscriptions matched to this publisher.
  /**
   * 这样我们可以在用户调用 rmw_count_subscribers() 时为 RMW 层提供准确的匹配订阅计数。
   * This is so we can provide the RMW layer with an accurate count of matched subscriptions if the
   * user calls rmw_count_subscribers().
   *
   * \param[in] guid 要跟踪的新匹配订阅的 GUID。
   * \param[in] guid The GUID of the newly-matched subscription to track.
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void track_unique_subscription(eprosima::fastrtps::rtps::GUID_t guid);

  /// 从与此发布者匹配的唯一订阅集合中删除一个 GUID。
  /// Remove a GUID from the internal set of unique subscriptions matched to this publisher.
  /**
   * 这样我们可以在用户调用 rmw_count_subscribers() 时为 RMW 层提供准确的匹配订阅计数。
   * This is so we can provide the RMW layer with an accurate count of matched subscriptions if the
   * user calls rmw_count_subscribers().
   *
   * \param[in] guid 要跟踪的新取消匹配订阅的 GUID。
   * \param[in] guid The GUID of the newly-unmatched subscription to track.
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void untrack_unique_subscription(eprosima::fastrtps::rtps::GUID_t guid);

  /// 返回与此发布者匹配的唯一订阅数量。
  /// Return the number of unique subscriptions matched to this publisher.
  /**
   * \return 与此发布者匹配的唯一订阅数量。
   * \return Number of unique subscriptions matched to this publisher.
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  size_t subscription_count() const;

  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void update_deadline(uint32_t total_count, uint32_t total_count_change);

  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void update_liveliness_lost(uint32_t total_count, uint32_t total_count_change);

  /// 更新提供的不兼容 QoS 策略。
  /// Update offered incompatible QoS policy.
  /**
   * \param[in] last_policy_id 最后一个不兼容 QoS 策略的 ID。
   * \param[in] last_policy_id The ID of the last incompatible QoS policy.
   * \param[in] total_count 不兼容 QoS 策略的总计数。
   * \param[in] total_count The total count of incompatible QoS policies.
   * \param[in] total_count_change 不兼容 QoS 策略的总计数变化。
   * \param[in] total_count_change The change in total count of incompatible QoS policies.
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void update_offered_incompatible_qos(
      eprosima::fastdds::dds::QosPolicyId_t last_policy_id,
      uint32_t total_count,
      uint32_t total_count_change);

  /// 更新匹配的订阅数量。
  /// Update the number of matched subscriptions.
  /**
   * \param[in] total_count 匹配订阅的总计数。
   * \param[in] total_count The total count of matched subscriptions.
   * \param[in] total_count_change 匹配订阅的总计数变化。
   * \param[in] total_count_change The change in total count of matched subscriptions.
   * \param[in] current_count 当前匹配订阅的计数。
   * \param[in] current_count The current count of matched subscriptions.
   * \param[in] current_count_change 当前匹配订阅的计数变化。
   * \param[in] current_count_change The change in current count of matched subscriptions.
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void update_matched(
      int32_t total_count,
      int32_t total_count_change,
      int32_t current_count,
      int32_t current_count_change);

private:
  /**
   * @brief CustomPublisherInfo 类，用于存储与发布者相关的信息。
   * (CustomPublisherInfo class, used to store information related to the publisher.)
   */
  class CustomPublisherInfo {
  public:
    // 发布者信息指针 (Publisher information pointer)
    CustomPublisherInfo *publisher_info_ = nullptr;

    /**
     * @brief 订阅者集合，存储订阅者的 GUID。
     * (Set of subscribers, storing the GUIDs of the subscribers.)
     */
    std::set<eprosima::fastrtps::rtps::GUID_t> subscriptions_
        RCPPUTILS_TSA_GUARDED_BY(subscriptions_mutex_);

    // 用于保护订阅者集合的互斥锁 (Mutex for protecting the set of subscribers)
    mutable std::mutex subscriptions_mutex_;

    // 表示 deadline 是否发生变化的标志 (Flag indicating whether the deadline has changed)
    bool deadline_changed_ RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

    // 存储 OfferedDeadlineMissedStatus 的状态信息 (Status information for
    // OfferedDeadlineMissedStatus)
    eprosima::fastdds::dds::OfferedDeadlineMissedStatus offered_deadline_missed_status_
        RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

    // 表示 liveliness 是否发生变化的标志 (Flag indicating whether the liveliness has changed)
    bool liveliness_changed_ RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

    // 存储 LivelinessLostStatus 的状态信息 (Status information for LivelinessLostStatus)
    eprosima::fastdds::dds::LivelinessLostStatus liveliness_lost_status_
        RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

    // 表示 QoS 是否发生变化的标志 (Flag indicating whether the QoS has changed)
    bool incompatible_qos_changed_ RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

    // 存储 OfferedIncompatibleQosStatus 的状态信息 (Status information for
    // OfferedIncompatibleQosStatus)
    eprosima::fastdds::dds::OfferedIncompatibleQosStatus incompatible_qos_status_
        RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

    // 表示匹配是否发生变化的标志 (Flag indicating whether the matching has changed)
    bool matched_changes_ RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

    // 存储 PublicationMatchedStatus 的状态信息 (Status information for PublicationMatchedStatus)
    eprosima::fastdds::dds::PublicationMatchedStatus matched_status_
        RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

    /**
     * @brief 触发事件的函数，用于处理不同类型的事件。
     * (Function to trigger events, used to handle different types of events.)
     *
     * @param event_type 事件类型 (Event type)
     */
    void trigger_event(rmw_event_type_t event_type);
  };
};

#endif  // RMW_FASTRTPS_SHARED_CPP__CUSTOM_PUBLISHER_INFO_HPP_
