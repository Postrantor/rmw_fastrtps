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

#include "rmw/rmw.h"
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 初始化发布者事件 (Initialize publisher event)
 *
 * @param[out] rmw_event 用于存储初始化后的事件指针 (Pointer to store the initialized event)
 * @param[in] publisher 与要初始化的事件关联的发布者 (Publisher associated with the event to be
 * initialized)
 * @param[in] event_type 要初始化的事件类型 (Type of the event to be initialized)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_publisher_event_init(
    rmw_event_t* rmw_event, const rmw_publisher_t* publisher, rmw_event_type_t event_type) {
  // 调用共享实现进行事件初始化 (Call shared implementation for event initialization)
  return rmw_fastrtps_shared_cpp::__rmw_init_event(
      eprosima_fastrtps_identifier, rmw_event, publisher->implementation_identifier,
      publisher->data, event_type);
}

/**
 * @brief 初始化订阅者事件 (Initialize subscription event)
 *
 * @param[out] rmw_event 用于存储初始化后的事件指针 (Pointer to store the initialized event)
 * @param[in] subscription 与要初始化的事件关联的订阅者 (Subscription associated with the event to
 * be initialized)
 * @param[in] event_type 要初始化的事件类型 (Type of the event to be initialized)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_subscription_event_init(
    rmw_event_t* rmw_event, const rmw_subscription_t* subscription, rmw_event_type_t event_type) {
  // 调用共享实现进行事件初始化 (Call shared implementation for event initialization)
  return rmw_fastrtps_shared_cpp::__rmw_init_event(
      eprosima_fastrtps_identifier, rmw_event, subscription->implementation_identifier,
      subscription->data, event_type);
}

/**
 * @brief 设置事件回调函数 (Set event callback function)
 *
 * @param[in,out] rmw_event 要设置回调的事件对象 (Event object to set the callback for)
 * @param[in] callback 回调函数指针 (Pointer to the callback function)
 * @param[in] user_data 用户提供的数据，将传递给回调函数 (User provided data that will be passed to
 * the callback function)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_event_set_callback(
    rmw_event_t* rmw_event, rmw_event_callback_t callback, const void* user_data) {
  // 检查输入参数是否为空 (Check if input argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(rmw_event, RMW_RET_INVALID_ARGUMENT);

  // 调用共享实现进行设置回调 (Call shared implementation for setting callback)
  return rmw_fastrtps_shared_cpp::__rmw_event_set_callback(rmw_event, callback, user_data);
}

}  // extern "C"
