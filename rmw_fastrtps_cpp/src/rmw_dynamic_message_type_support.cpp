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
#include <rosidl_dynamic_typesupport/api/serialization_support_interface.h>
#include <rosidl_dynamic_typesupport/types.h>
#include <rosidl_dynamic_typesupport_fastrtps/serialization_support.h>

#include "rmw/allocators.h"
#include "rmw/convert_rcutils_ret_to_rmw_ret.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 从订阅中获取动态消息 (Take a dynamic message from the subscription)
 *
 * @param[in] subscription 订阅指针 (Pointer to the subscription)
 * @param[out] dynamic_data 动态数据类型支持的数据指针 (Pointer to data with dynamic typesupport)
 * @param[out] taken 是否成功获取到消息的标志 (Flag indicating if a message was successfully taken)
 * @param[in] allocation 消息分配器 (Message allocator)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_take_dynamic_message(
    const rmw_subscription_t* subscription,
    rosidl_dynamic_typesupport_dynamic_data_t* dynamic_data,
    bool* taken,
    rmw_subscription_allocation_t* allocation) {
  // 调用共享的实现函数 (Call the shared implementation function)
  return rmw_fastrtps_shared_cpp::__rmw_take_dynamic_message(
      eprosima_fastrtps_identifier, subscription, dynamic_data, taken, allocation);
}

/**
 * @brief 从订阅中获取动态消息及相关信息 (Take a dynamic message and its related info from the
 * subscription)
 *
 * @param[in] subscription 订阅指针 (Pointer to the subscription)
 * @param[out] dynamic_data 动态数据类型支持的数据指针 (Pointer to data with dynamic typesupport)
 * @param[out] taken 是否成功获取到消息的标志 (Flag indicating if a message was successfully taken)
 * @param[out] message_info 消息相关信息 (Message related info)
 * @param[in] allocation 消息分配器 (Message allocator)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_take_dynamic_message_with_info(
    const rmw_subscription_t* subscription,
    rosidl_dynamic_typesupport_dynamic_data_t* dynamic_data,
    bool* taken,
    rmw_message_info_t* message_info,
    rmw_subscription_allocation_t* allocation) {
  // 调用共享的实现函数 (Call the shared implementation function)
  return rmw_fastrtps_shared_cpp::__rmw_take_dynamic_message_with_info(
      eprosima_fastrtps_identifier, subscription, dynamic_data, taken, message_info, allocation);
}

/**
 * @brief 初始化序列化支持 (Initialize serialization support)
 *
 * @param[in] serialization_lib_name 序列化库名称 (Serialization library name)
 * @param[in] allocator 分配器指针 (Pointer to the allocator)
 * @param[out] serialization_support 序列化支持结构体指针 (Pointer to the serialization support
 * structure)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_serialization_support_init(
    const char* /*serialization_lib_name*/,
    rcutils_allocator_t* allocator,
    rosidl_dynamic_typesupport_serialization_support_t* serialization_support) {
  // 检查分配器和序列化支持参数是否为空 (Check if allocator and serialization_support arguments are
  // null)
  RMW_CHECK_ARGUMENT_FOR_NULL(allocator, RMW_RET_INVALID_ARGUMENT);
  if (!rcutils_allocator_is_valid(allocator)) {
    RMW_SET_ERROR_MSG("allocator is invalid");
    return RMW_RET_INVALID_ARGUMENT;
  }
  RMW_CHECK_ARGUMENT_FOR_NULL(serialization_support, RMW_RET_INVALID_ARGUMENT);

  rcutils_ret_t ret = RCUTILS_RET_ERROR;

  // 初始化序列化支持实现结构体 (Initialize the serialization support implementation structure)
  rosidl_dynamic_typesupport_serialization_support_impl_t impl =
      rosidl_dynamic_typesupport_get_zero_initialized_serialization_support_impl();

  // 初始化序列化支持接口结构体 (Initialize the serialization support interface structure)
  rosidl_dynamic_typesupport_serialization_support_interface_t methods =
      rosidl_dynamic_typesupport_get_zero_initialized_serialization_support_interface();

  // 初始化 FastRTPS 序列化支持实现 (Initialize FastRTPS serialization support implementation)
  ret = rosidl_dynamic_typesupport_fastrtps_init_serialization_support_impl(allocator, &impl);
  if (ret != RCUTILS_RET_OK) {
    RMW_SET_ERROR_MSG_AND_APPEND_PREV_ERROR("Could not initialize serialization support impl");
    goto fail;
  }

  // 初始化 FastRTPS 序列化支持接口 (Initialize FastRTPS serialization support interface)
  ret =
      rosidl_dynamic_typesupport_fastrtps_init_serialization_support_interface(allocator, &methods);
  if (ret != RCUTILS_RET_OK) {
    RMW_SET_ERROR_MSG_AND_APPEND_PREV_ERROR("could not initialize serialization support interface");
    goto fail;
  }

  // 转换 rcutils_ret_t 结果为 rmw_ret_t 结果并初始化序列化支持 (Convert rcutils_ret_t result to
  // rmw_ret_t result and initialize serialization support)
  return rmw_convert_rcutils_ret_to_rmw_ret(rosidl_dynamic_typesupport_serialization_support_init(
      &impl, &methods, allocator, serialization_support));

fail:
  // 如果发生错误，尝试终止序列化支持 (If an error occurs, try to terminate serialization support)
  if (rosidl_dynamic_typesupport_serialization_support_fini(serialization_support) !=
      RCUTILS_RET_ERROR) {
    RCUTILS_SAFE_FWRITE_TO_STDERR_AND_APPEND_PREV_ERROR(
        "While handling another error, could not finalize serialization support");
  }
  // 转换 rcutils_ret_t 结果为 rmw_ret_t 结果 (Convert rcutils_ret_t result to rmw_ret_t result)
  return rmw_convert_rcutils_ret_to_rmw_ret(ret);
}

}  // extern "C"
