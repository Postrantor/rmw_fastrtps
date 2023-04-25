// Copyright 2022 Open Source Robotics Foundation, Inc.
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

#include <fastcdr/Cdr.h>
#include <rcutils/allocator.h>
#include <rcutils/logging_macros.h>
#include <rosidl_dynamic_typesupport_fastrtps/serialization_support.h>

#include "rmw/allocators.h"
#include "rmw/convert_rcutils_ret_to_rmw_ret.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 从订阅中获取动态消息 (Take a dynamic message from the subscription)
 *
 * @param[in] subscription 订阅指针 (Pointer to the subscription)
 * @param[out] dynamic_message 动态消息数据结构指针 (Pointer to the dynamic message data structure)
 * @param[out] taken 是否成功获取到消息的标志 (Flag indicating whether a message was successfully
 * taken)
 * @param[in] allocation 订阅分配指针 (Pointer to the subscription allocation)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_take_dynamic_message(
    const rmw_subscription_t* subscription,
    rosidl_dynamic_typesupport_dynamic_data_t* dynamic_message,
    bool* taken,
    rmw_subscription_allocation_t* allocation) {
  // 将输入参数转换为void类型，避免编译器警告 (Cast input parameters to void type to avoid compiler
  // warnings)
  static_cast<void>(subscription);
  static_cast<void>(dynamic_message);
  static_cast<void>(taken);
  static_cast<void>(allocation);

  // 设置错误信息并返回不支持的状态 (Set error message and return unsupported status)
  RMW_SET_ERROR_MSG("rmw_take_dynamic_message: unimplemented");
  return RMW_RET_UNSUPPORTED;
}

/**
 * @brief 从订阅中获取动态消息及其相关信息 (Take a dynamic message and its related info from the
 * subscription)
 *
 * @param[in] subscription 订阅指针 (Pointer to the subscription)
 * @param[out] dynamic_message 动态消息数据结构指针 (Pointer to the dynamic message data structure)
 * @param[out] taken 是否成功获取到消息的标志 (Flag indicating whether a message was successfully
 * taken)
 * @param[out] message_info 消息相关信息指针 (Pointer to the message related info)
 * @param[in] allocation 订阅分配指针 (Pointer to the subscription allocation)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_take_dynamic_message_with_info(
    const rmw_subscription_t* subscription,
    rosidl_dynamic_typesupport_dynamic_data_t* dynamic_message,
    bool* taken,
    rmw_message_info_t* message_info,
    rmw_subscription_allocation_t* allocation) {
  // 将输入参数转换为void类型，避免编译器警告 (Cast input parameters to void type to avoid compiler
  // warnings)
  static_cast<void>(subscription);
  static_cast<void>(dynamic_message);
  static_cast<void>(taken);
  static_cast<void>(message_info);
  static_cast<void>(allocation);

  // 设置错误信息并返回不支持的状态 (Set error message and return unsupported status)
  RMW_SET_ERROR_MSG("rmw_take_dynamic_message_with_info: unimplemented");
  return RMW_RET_UNSUPPORTED;
}

/**
 * @brief 初始化序列化支持 (Initialize serialization support)
 *
 * @param[in] serialization_lib_name 序列化库名称 (Serialization library name)
 * @param[in] allocator 分配器指针 (Pointer to the allocator)
 * @param[out] serialization_support 序列化支持数据结构指针 (Pointer to the serialization support
 * data structure)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_serialization_support_init(
    const char* serialization_lib_name,
    rcutils_allocator_t* allocator,
    rosidl_dynamic_typesupport_serialization_support_t* serialization_support) {
  // 将输入参数转换为void类型，避免编译器警告 (Cast input parameters to void type to avoid compiler
  // warnings)
  static_cast<void>(serialization_lib_name);
  static_cast<void>(allocator);
  static_cast<void>(serialization_support);

  // 设置错误信息并返回不支持的状态 (Set error message and return unsupported status)
  RMW_SET_ERROR_MSG("rmw_serialization_support_init: unimplemented");
  return RMW_RET_UNSUPPORTED;
}

}  // extern "C"
