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

#ifndef RMW_FASTRTPS_SHARED_CPP__CUSTOM_EVENT_INFO_HPP_
#define RMW_FASTRTPS_SHARED_CPP__CUSTOM_EVENT_INFO_HPP_

#include <mutex>

#include "fastcdr/FastBuffer.h"
#include "fastdds/dds/core/condition/GuardCondition.hpp"
#include "fastdds/dds/core/condition/StatusCondition.hpp"
#include "fastdds/dds/core/status/BaseStatus.hpp"
#include "rcpputils/thread_safety_annotations.hpp"
#include "rmw/event.h"
#include "rmw/event_callback_type.h"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"

/**
 * @class EventListenerInterface
 * @brief 事件监听器接口类 (Event Listener Interface class)
 */
class EventListenerInterface {
public:
  /**
   * @brief 获取状态条件引用
   * @return eprosima::fastdds::dds::StatusCondition& 状态条件引用
   */
  virtual eprosima::fastdds::dds::StatusCondition& get_statuscondition() const = 0;

  /// 为某个事件类型获取准备好的数据 (Take ready data for an event type)
  /**
   * @param[in] event_type 要获取数据的事件类型 (The event type to get data for)
   * @param[out] event_info 预分配的事件信息（来自rmw/types.h），用于填充数据 (A preallocated event
   * information (from rmw/types.h) to fill with data)
   * @return `true` 如果成功获取数据 (if data was successfully taken)
   * @return `false` 如果数据不可用，此时event_info中没有写入任何内容 (if data was not available, in
   * this case nothing was written to event_info)
   */
  virtual bool take_event(rmw_event_type_t event_type, void* event_info) = 0;

  /// 当此监听器的新事件发生时，提供处理程序执行操作
  /**
   * @param[in] event_type 要为其设置新回调的事件类型
   * @param[in] user_data 与事件关联的用户数据
   * @param[in] callback 当事件发生时调用的回调
   */
  virtual void set_on_new_event_callback(
      rmw_event_type_t event_type, const void* user_data, rmw_event_callback_t callback) = 0;

  /**
   * @brief 获取事件保护条件引用
   * @param[in] event_type 要获取保护条件的事件类型
   * @return eprosima::fastdds::dds::GuardCondition& 事件保护条件引用
   */
  eprosima::fastdds::dds::GuardCondition& get_event_guard(rmw_event_type_t event_type) {
    return event_guard[event_type];
  }

  /// 更新内部不一致主题数据的回调
  /**
   * @param[in] total_count 所有时间的不一致主题事件总数
   * @param[in] total_count_change 正在报告的不一致主题事件数量
   */
  virtual void update_inconsistent_topic(uint32_t total_count, uint32_t total_count_change) = 0;

protected:
  // 事件保护条件数组 (Array of event GuardConditions)
  eprosima::fastdds::dds::GuardCondition event_guard[RMW_EVENT_INVALID];

  // 新事件回调数组 (Array of new event callbacks)
  rmw_event_callback_t on_new_event_cb_[RMW_EVENT_INVALID] = {nullptr};

  // 用户数据数组 (Array of user data)
  const void* user_data_[RMW_EVENT_INVALID] = {nullptr};

  // 新事件互斥锁 (Mutex for new event)
  std::mutex on_new_event_m_;

  // 不一致主题更改标志 (Inconsistent topic changed flag)
  bool inconsistent_topic_changed_{false} RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);

  // 不一致主题状态 (Inconsistent topic status)
  eprosima::fastdds::dds::InconsistentTopicStatus inconsistent_topic_status_
      RCPPUTILS_TSA_GUARDED_BY(on_new_event_m_);
};

/**
 * @struct CustomEventInfo
 * @brief 自定义事件信息结构体 (Custom Event Info structure)
 */
struct CustomEventInfo {
  /**
   * @brief 获取监听器指针 (Get the pointer to the listener)
   * @return EventListenerInterface* 监听器指针 (Pointer to the listener)
   */
  virtual EventListenerInterface* get_listener() const = 0;
};

#endif  // RMW_FASTRTPS_SHARED_CPP__CUSTOM_EVENT_INFO_HPP_
