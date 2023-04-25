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
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 初始化发布者事件 (Initialize publisher event)
 *
 * @param[out] rmw_event 指向要初始化的事件结构体的指针 (Pointer to the event structure to be
 * initialized)
 * @param[in] publisher 指向发布者结构体的指针 (Pointer to the publisher structure)
 * @param[in] event_type 要初始化的事件类型 (Event type to initialize)
 * @return 返回操作结果，成功返回 RMW_RET_OK，失败返回相应错误代码 (Returns the operation result,
 * success returns RMW_RET_OK, failure returns the corresponding error code)
 */
rmw_ret_t rmw_publisher_event_init(
    rmw_event_t* rmw_event, const rmw_publisher_t* publisher, rmw_event_type_t event_type) {
  // 调用共享实现中的 __rmw_init_event 函数进行事件初始化 (Call the __rmw_init_event function in the
  // shared implementation for event initialization)
  return rmw_fastrtps_shared_cpp::__rmw_init_event(
      eprosima_fastrtps_identifier, rmw_event, publisher->implementation_identifier,
      publisher->data, event_type);
}

/**
 * @brief 初始化订阅者事件 (Initialize subscription event)
 *
 * @param[out] rmw_event 指向要初始化的事件结构体的指针 (Pointer to the event structure to be
 * initialized)
 * @param[in] subscription 指向订阅者结构体的指针 (Pointer to the subscription structure)
 * @param[in] event_type 要初始化的事件类型 (Event type to initialize)
 * @return 返回操作结果，成功返回 RMW_RET_OK，失败返回相应错误代码 (Returns the operation result,
 * success returns RMW_RET_OK, failure returns the corresponding error code)
 */
rmw_ret_t rmw_subscription_event_init(
    rmw_event_t* rmw_event, const rmw_subscription_t* subscription, rmw_event_type_t event_type) {
  // 调用共享实现中的 __rmw_init_event 函数进行事件初始化 (Call the __rmw_init_event function in the
  // shared implementation for event initialization)
  return rmw_fastrtps_shared_cpp::__rmw_init_event(
      eprosima_fastrtps_identifier, rmw_event, subscription->implementation_identifier,
      subscription->data, event_type);
}

/**
 * @brief 设置事件回调函数 (Set event callback function)
 *
 * @param[in,out] rmw_event 指向要设置回调函数的事件结构体的指针 (Pointer to the event structure to
 * set the callback function)
 * @param[in] callback 要设置的回调函数 (Callback function to set)
 * @param[in] user_data 用户数据，将传递给回调函数 (User data that will be passed to the callback
 * function)
 * @return 返回操作结果，成功返回 RMW_RET_OK，失败返回相应错误代码 (Returns the operation result,
 * success returns RMW_RET_OK, failure returns the corresponding error code)
 */
rmw_ret_t rmw_event_set_callback(
    rmw_event_t* rmw_event, rmw_event_callback_t callback, const void* user_data) {
  // 检查 rmw_event 参数是否为空，为空则返回 RMW_RET_INVALID_ARGUMENT 错误代码 (Check if the
  // rmw_event parameter is NULL, if it is, return the RMW_RET_INVALID_ARGUMENT error code)
  RMW_CHECK_ARGUMENT_FOR_NULL(rmw_event, RMW_RET_INVALID_ARGUMENT);

  // 调用共享实现中的 __rmw_event_set_callback 函数设置回调函数 (Call the __rmw_event_set_callback
  // function in the shared implementation to set the callback function)
  return rmw_fastrtps_shared_cpp::__rmw_event_set_callback(rmw_event, callback, user_data);
}

}  // extern "C"
