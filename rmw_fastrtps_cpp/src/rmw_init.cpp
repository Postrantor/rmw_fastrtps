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
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_cpp/publisher.hpp"
#include "rmw_fastrtps_cpp/subscription.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/participant.hpp"
#include "rmw_fastrtps_shared_cpp/publisher.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"
#include "rmw_fastrtps_shared_cpp/subscription.hpp"
#include "rosidl_typesupport_cpp/message_type_support.hpp"

extern "C" {

/**
 * @brief 初始化 rmw_init_options_t 结构体
 *        Initialize the rmw_init_options_t structure
 *
 * @param init_options 指向要初始化的 rmw_init_options_t 结构体的指针
 *                     Pointer to the rmw_init_options_t structure to be initialized
 * @param allocator 分配器，用于分配内存
 *                  Allocator for memory allocation
 * @return 成功时返回 RMW_RET_OK，失败时返回相应的错误代码
 *         On success, return RMW_RET_OK; on failure, return the corresponding error code
 */
rmw_ret_t rmw_init_options_init(rmw_init_options_t* init_options, rcutils_allocator_t allocator) {
  return rmw_fastrtps_shared_cpp::rmw_init_options_init(
      eprosima_fastrtps_identifier, init_options, allocator);
}

/**
 * @brief 复制 rmw_init_options_t 结构体
 *        Copy the rmw_init_options_t structure
 *
 * @param src 指向源 rmw_init_options_t 结构体的指针
 *            Pointer to the source rmw_init_options_t structure
 * @param dst 指向目标 rmw_init_options_t 结构体的指针
 *            Pointer to the destination rmw_init_options_t structure
 * @return 成功时返回 RMW_RET_OK，失败时返回相应的错误代码
 *         On success, return RMW_RET_OK; on failure, return the corresponding error code
 */
rmw_ret_t rmw_init_options_copy(const rmw_init_options_t* src, rmw_init_options_t* dst) {
  return rmw_fastrtps_shared_cpp::rmw_init_options_copy(eprosima_fastrtps_identifier, src, dst);
}

/**
 * @brief 清理 rmw_init_options_t 结构体
 *        Clean up the rmw_init_options_t structure
 *
 * @param init_options 指向要清理的 rmw_init_options_t 结构体的指针
 *                     Pointer to the rmw_init_options_t structure to be cleaned up
 * @return 成功时返回 RMW_RET_OK，失败时返回相应的错误代码
 *         On success, return RMW_RET_OK; on failure, return the corresponding error code
 */
rmw_ret_t rmw_init_options_fini(rmw_init_options_t* init_options) {
  return rmw_fastrtps_shared_cpp::rmw_init_options_fini(eprosima_fastrtps_identifier, init_options);
}

using rmw_dds_common::msg::ParticipantEntitiesInfo;

/**
 * @brief 初始化 rmw_context_t 结构体
 *        Initialize the rmw_context_t structure
 *
 * @param options 指向用于初始化 rmw_context_t 结构体的 rmw_init_options_t 结构体的指针
 *                Pointer to the rmw_init_options_t structure used for initializing rmw_context_t
 * structure
 * @param context 指向要初始化的 rmw_context_t 结构体的指针
 *                Pointer to the rmw_context_t structure to be initialized
 * @return 成功时返回 RMW_RET_OK，失败时返回相应的错误代码
 *         On success, return RMW_RET_OK; on failure, return the corresponding error code
 */
rmw_ret_t rmw_init(const rmw_init_options_t* options, rmw_context_t* context) {
  // 检查输入参数是否为空，为空则返回 RMW_RET_INVALID_ARGUMENT 错误
  // Check if input arguments are null, if so return RMW_RET_INVALID_ARGUMENT error
  RMW_CHECK_ARGUMENT_FOR_NULL(options, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(context, RMW_RET_INVALID_ARGUMENT);

  // 检查 options 的实现标识符是否为空，为空则返回 RMW_RET_INVALID_ARGUMENT 错误
  // Check if the implementation identifier in options is null, if so return
  // RMW_RET_INVALID_ARGUMENT error
  RMW_CHECK_FOR_NULL_WITH_MSG(
      options->implementation_identifier, "expected initialized init options",
      return RMW_RET_INVALID_ARGUMENT);

  // 检查类型标识符是否匹配，不匹配则返回 RMW_RET_INCORRECT_RMW_IMPLEMENTATION 错误
  // Check if type identifiers match, if not return RMW_RET_INCORRECT_RMW_IMPLEMENTATION error
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      options, options->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查 options 的 enclave 是否为空，为空则返回 RMW_RET_INVALID_ARGUMENT 错误
  // Check if the enclave in options is null, if so return RMW_RET_INVALID_ARGUMENT error
  RMW_CHECK_FOR_NULL_WITH_MSG(
      options->enclave, "expected non-null enclave", return RMW_RET_INVALID_ARGUMENT);

  // 检查 context 的实现标识符是否为空，不为空则设置错误消息并返回 RMW_RET_INVALID_ARGUMENT 错误
  // Check if the implementation identifier in context is null, if not set error message and return
  // RMW_RET_INVALID_ARGUMENT error
  if (NULL != context->implementation_identifier) {
    RMW_SET_ERROR_MSG("expected a zero-initialized context");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 创建一个清理 context 的作用域
  // Create a scope for cleaning up context
  auto restore_context =
      rcpputils::make_scope_exit([context]() { *context = rmw_get_zero_initialized_context(); });

  // 初始化 context 的各个成员变量
  // Initialize the various member variables of context
  context->instance_id = options->instance_id;
  context->implementation_identifier = eprosima_fastrtps_identifier;
  context->actual_domain_id =
      RMW_DEFAULT_DOMAIN_ID == options->domain_id ? 0uL : options->domain_id;

  // 为 context 的实现分配内存
  // Allocate memory for context implementation
  context->impl = new (std::nothrow) rmw_context_impl_t();
  if (nullptr == context->impl) {
    RMW_SET_ERROR_MSG("failed to allocate context impl");
    return RMW_RET_BAD_ALLOC;
  }
  // 创建一个清理 context 实现的作用域
  // Create a scope for cleaning up context implementation
  auto cleanup_impl = rcpputils::make_scope_exit([context]() { delete context->impl; });

  // 设置 context 的实现状态为未关闭
  // Set the implementation state of context to not closed
  context->impl->is_shutdown = false;

  // 初始化 context 的选项为零值
  // Initialize the options of context to zero value
  context->options = rmw_get_zero_initialized_init_options();

  // 复制 options 到 context 的选项中
  // Copy options into the options of context
  rmw_ret_t ret = rmw_init_options_copy(options, &context->options);
  if (RMW_RET_OK != ret) {
    return ret;
  }

  // 取消清理 context 实现的作用域
  // Cancel the scope for cleaning up context implementation
  cleanup_impl.cancel();

  // 取消清理 context 的作用域
  // Cancel the scope for cleaning up context
  restore_context.cancel();

  // 返回成功
  // Return success
  return RMW_RET_OK;
}

/**
 * @brief 关闭ROS2上下文 (Shutdown the ROS2 context)
 *
 * @param[in] context 指向要关闭的rmw_context_t结构的指针 (Pointer to the rmw_context_t structure to
 * be shut down)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_shutdown(rmw_context_t* context) {
  // 检查context参数是否为空，如果为空则返回无效参数错误 (Check if the context parameter is NULL,
  // and return an invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(context, RMW_RET_INVALID_ARGUMENT);

  // 检查context->impl是否为空，如果为空则返回无效参数错误 (Check if context->impl is NULL, and
  // return an invalid argument error if it is)
  RMW_CHECK_FOR_NULL_WITH_MSG(
      context->impl, "expected initialized context", return RMW_RET_INVALID_ARGUMENT);

  // 检查类型标识符是否匹配，如果不匹配则返回不正确的RMW实现错误 (Check if the type identifiers
  // match, and return an incorrect RMW implementation error if they don't)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      context, context->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 设置上下文为关闭状态 (Set the context to shutdown state)
  context->impl->is_shutdown = true;

  // 返回操作成功 (Return operation success)
  return RMW_RET_OK;
}

/**
 * @brief 终止ROS2上下文 (Finalize the ROS2 context)
 *
 * @param[in] context 指向要终止的rmw_context_t结构的指针 (Pointer to the rmw_context_t structure to
 * be finalized)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_context_fini(rmw_context_t* context) {
  // 检查context参数是否为空，如果为空则返回无效参数错误 (Check if the context parameter is NULL,
  // and return an invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(context, RMW_RET_INVALID_ARGUMENT);

  // 检查context->impl是否为空，如果为空则返回无效参数错误 (Check if context->impl is NULL, and
  // return an invalid argument error if it is)
  RMW_CHECK_FOR_NULL_WITH_MSG(
      context->impl, "expected initialized context", return RMW_RET_INVALID_ARGUMENT);

  // 检查类型标识符是否匹配，如果不匹配则返回不正确的RMW实现错误 (Check if the type identifiers
  // match, and return an incorrect RMW implementation error if they don't)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      context, context->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 如果上下文没有关闭，则设置错误信息并返回无效参数错误 (If the context has not been shut down,
  // set the error message and return an invalid argument error)
  if (!context->impl->is_shutdown) {
    RCUTILS_SET_ERROR_MSG("context has not been shutdown");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 如果上下文中还有活动节点，则设置错误信息并返回错误 (If there are active nodes in the context,
  // set the error message and return an error)
  if (context->impl->count > 0) {
    RMW_SET_ERROR_MSG("Finalizing a context with active nodes");
    return RMW_RET_ERROR;
  }

  // 终止初始化选项 (Finalize the initialization options)
  rmw_ret_t ret = rmw_init_options_fini(&context->options);

  // 删除上下文实现 (Delete the context implementation)
  delete context->impl;

  // 将上下文设置为零初始化状态 (Set the context to zero-initialized state)
  *context = rmw_get_zero_initialized_context();

  // 返回操作结果 (Return the operation result)
  return ret;
}

}  // extern "C"
