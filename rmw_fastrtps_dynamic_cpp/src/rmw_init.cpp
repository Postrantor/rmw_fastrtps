// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#include "rmw_fastrtps_shared_cpp/rmw_init.hpp"

#include <cassert>
#include <cstring>
#include <memory>

#include "rcpputils/scope_exit.hpp"
#include "rcutils/strdup.h"
#include "rcutils/types.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/init.h"
#include "rmw/init_options.h"
#include "rmw/publisher_options.h"
#include "rmw/rmw.h"
#include "rmw_dds_common/context.hpp"
#include "rmw_dds_common/msg/participant_entities_info.hpp"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_dynamic_cpp/publisher.hpp"
#include "rmw_fastrtps_dynamic_cpp/subscription.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/participant.hpp"
#include "rmw_fastrtps_shared_cpp/publisher.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"
#include "rmw_fastrtps_shared_cpp/subscription.hpp"
#include "rosidl_typesupport_cpp/message_type_support.hpp"

extern "C" {

/**
 * @brief 初始化 rmw_init_options_t 结构体
 * Initialize the rmw_init_options_t structure
 *
 * @param[in] init_options 指向要初始化的 rmw_init_options_t 结构体的指针
 * Pointer to the rmw_init_options_t structure to be initialized
 * @param[in] allocator 分配器用于分配内存
 * Allocator used for memory allocation
 * @return 返回一个 rmw_ret_t 类型的状态值，表示操作成功或失败
 * Returns an rmw_ret_t type status value indicating success or failure of the operation
 */
rmw_ret_t rmw_init_options_init(rmw_init_options_t* init_options, rcutils_allocator_t allocator) {
  return rmw_fastrtps_shared_cpp::rmw_init_options_init(
      eprosima_fastrtps_identifier, init_options, allocator);
}

/**
 * @brief 复制 rmw_init_options_t 结构体
 * Copy the rmw_init_options_t structure
 *
 * @param[in] src 指向源 rmw_init_options_t 结构体的指针
 * Pointer to the source rmw_init_options_t structure
 * @param[out] dst 指向目标 rmw_init_options_t 结构体的指针
 * Pointer to the destination rmw_init_options_t structure
 * @return 返回一个 rmw_ret_t 类型的状态值，表示操作成功或失败
 * Returns an rmw_ret_t type status value indicating success or failure of the operation
 */
rmw_ret_t rmw_init_options_copy(const rmw_init_options_t* src, rmw_init_options_t* dst) {
  return rmw_fastrtps_shared_cpp::rmw_init_options_copy(eprosima_fastrtps_identifier, src, dst);
}

/**
 * @brief 清理 rmw_init_options_t 结构体
 * Clean up the rmw_init_options_t structure
 *
 * @param[in] init_options 指向要清理的 rmw_init_options_t 结构体的指针
 * Pointer to the rmw_init_options_t structure to be cleaned up
 * @return 返回一个 rmw_ret_t 类型的状态值，表示操作成功或失败
 * Returns an rmw_ret_t type status value indicating success or failure of the operation
 */
rmw_ret_t rmw_init_options_fini(rmw_init_options_t* init_options) {
  return rmw_fastrtps_shared_cpp::rmw_init_options_fini(eprosima_fastrtps_identifier, init_options);
}

using rmw_dds_common::msg::ParticipantEntitiesInfo;

/**
 * @brief 初始化 rmw_context_t 结构体
 * Initialize the rmw_context_t structure
 *
 * @param[in] options 指向用于初始化的 rmw_init_options_t 结构体的指针
 * Pointer to the rmw_init_options_t structure used for initialization
 * @param[out] context 指向要初始化的 rmw_context_t 结构体的指针
 * Pointer to the rmw_context_t structure to be initialized
 * @return 返回一个 rmw_ret_t 类型的状态值，表示操作成功或失败
 * Returns an rmw_ret_t type status value indicating success or failure of the operation
 */
rmw_ret_t rmw_init(const rmw_init_options_t* options, rmw_context_t* context) {
  // 检查输入参数是否为空
  // Check if input arguments are null
  RMW_CHECK_ARGUMENT_FOR_NULL(options, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(context, RMW_RET_INVALID_ARGUMENT);

  // 检查 options 的实现标识符是否已初始化
  // Check if the implementation identifier of options is initialized
  RMW_CHECK_FOR_NULL_WITH_MSG(
      options->implementation_identifier, "expected initialized init options",
      return RMW_RET_INVALID_ARGUMENT);

  // 检查类型标识符是否匹配
  // Check if type identifiers match
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      options, options->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查 enclave 是否为空
  // Check if enclave is null
  RMW_CHECK_FOR_NULL_WITH_MSG(
      options->enclave, "expected non-null enclave", return RMW_RET_INVALID_ARGUMENT);

  // 检查 context 的实现标识符是否为零初始化
  // Check if the implementation identifier of context is zero-initialized
  if (NULL != context->implementation_identifier) {
    RMW_SET_ERROR_MSG("expected a zero-initialized context");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 创建一个恢复上下文的作用域退出对象
  // Create a scope exit object to restore context
  auto restore_context =
      rcpputils::make_scope_exit([context]() { *context = rmw_get_zero_initialized_context(); });

  // 设置 context 的属性
  // Set properties of context
  context->instance_id = options->instance_id;
  context->implementation_identifier = eprosima_fastrtps_identifier;
  context->actual_domain_id =
      RMW_DEFAULT_DOMAIN_ID == options->domain_id ? 0uL : options->domain_id;

  // 分配 context 的实现结构体内存
  // Allocate memory for the implementation structure of context
  context->impl = new (std::nothrow) rmw_context_impl_t();
  if (nullptr == context->impl) {
    RMW_SET_ERROR_MSG("failed to allocate context impl");
    return RMW_RET_BAD_ALLOC;
  }

  // 创建一个清理实现结构体的作用域退出对象
  // Create a scope exit object to clean up the implementation structure
  auto cleanup_impl = rcpputils::make_scope_exit([context]() { delete context->impl; });

  // 设置 context 的实现属性
  // Set properties of the implementation of context
  context->impl->is_shutdown = false;

  // 初始化 context 的选项
  // Initialize options of context
  context->options = rmw_get_zero_initialized_init_options();
  rmw_ret_t ret = rmw_init_options_copy(options, &context->options);
  if (RMW_RET_OK != ret) {
    return ret;
  }

  // 取消清理实现结构体和恢复上下文的作用域退出操作
  // Cancel the scope exit operations for cleaning up the implementation structure and restoring
  // context
  cleanup_impl.cancel();
  restore_context.cancel();

  // 返回成功状态值
  // Return success status value
  return RMW_RET_OK;
}

/**
 * @brief 关闭 ROS2 上下文 (Shutdown the ROS2 context)
 *
 * @param[in,out] context 指向要关闭的上下文的指针 (Pointer to the context to be shutdown)
 * @return rmw_ret_t 返回操作状态 (Return the operation status)
 */
rmw_ret_t rmw_shutdown(rmw_context_t* context) {
  // 检查 context 是否为空，如果为空返回 RMW_RET_INVALID_ARGUMENT 错误
  // Check if context is NULL, if it is, return RMW_RET_INVALID_ARGUMENT error
  RMW_CHECK_ARGUMENT_FOR_NULL(context, RMW_RET_INVALID_ARGUMENT);

  // 检查 context->impl 是否为空，如果为空返回 RMW_RET_INVALID_ARGUMENT 错误
  // Check if context->impl is NULL, if it is, return RMW_RET_INVALID_ARGUMENT error
  RMW_CHECK_FOR_NULL_WITH_MSG(
      context->impl, "expected initialized context", return RMW_RET_INVALID_ARGUMENT);

  // 检查 context 的实现标识符是否与 eprosima_fastrtps_identifier 匹配，如果不匹配返回
  // RMW_RET_INCORRECT_RMW_IMPLEMENTATION 错误 Check if the implementation identifier of context
  // matches with eprosima_fastrtps_identifier, if not, return RMW_RET_INCORRECT_RMW_IMPLEMENTATION
  // error
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      context, context->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 设置上下文为关闭状态
  // Set the context to shutdown state
  context->impl->is_shutdown = true;

  // 返回操作成功
  // Return operation success
  return RMW_RET_OK;
}

/**
 * @brief 终止 ROS2 上下文 (Finalize the ROS2 context)
 *
 * @param[in,out] context 指向要终止的上下文的指针 (Pointer to the context to be finalized)
 * @return rmw_ret_t 返回操作状态 (Return the operation status)
 */
rmw_ret_t rmw_context_fini(rmw_context_t* context) {
  // 检查 context 是否为空，如果为空返回 RMW_RET_INVALID_ARGUMENT 错误
  // Check if context is NULL, if it is, return RMW_RET_INVALID_ARGUMENT error
  RMW_CHECK_ARGUMENT_FOR_NULL(context, RMW_RET_INVALID_ARGUMENT);

  // 检查 context->impl 是否为空，如果为空返回 RMW_RET_INVALID_ARGUMENT 错误
  // Check if context->impl is NULL, if it is, return RMW_RET_INVALID_ARGUMENT error
  RMW_CHECK_FOR_NULL_WITH_MSG(
      context->impl, "expected initialized context", return RMW_RET_INVALID_ARGUMENT);

  // 检查 context 的实现标识符是否与 eprosima_fastrtps_identifier 匹配，如果不匹配返回
  // RMW_RET_INCORRECT_RMW_IMPLEMENTATION 错误 Check if the implementation identifier of context
  // matches with eprosima_fastrtps_identifier, if not, return RMW_RET_INCORRECT_RMW_IMPLEMENTATION
  // error
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      context, context->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查上下文是否已关闭，如果没有关闭则返回 RMW_RET_INVALID_ARGUMENT 错误
  // Check if the context has been shutdown, if not, return RMW_RET_INVALID_ARGUMENT error
  if (!context->impl->is_shutdown) {
    RCUTILS_SET_ERROR_MSG("context has not been shutdown");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 检查上下文中是否有活动节点，如果有则返回 RMW_RET_ERROR 错误
  // Check if there are active nodes in the context, if there are, return RMW_RET_ERROR error
  if (context->impl->count > 0) {
    RMW_SET_ERROR_MSG("Finalizing a context with active nodes");
    return RMW_RET_ERROR;
  }

  // 终止上下文的初始化选项
  // Finalize the init options of the context
  rmw_ret_t ret = rmw_init_options_fini(&context->options);

  // 删除上下文的实现
  // Delete the implementation of the context
  delete context->impl;

  // 将上下文设置为零初始化状态
  // Set the context to zero-initialized state
  *context = rmw_get_zero_initialized_context();

  // 返回操作结果
  // Return the operation result
  return ret;
}

}  // extern "C"
