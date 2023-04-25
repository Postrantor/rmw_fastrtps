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

#include "fastdds/dds/core/condition/WaitSet.hpp"
#include "rcutils/macros.h"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 创建一个 rmw_wait_set_t 对象 (Create an rmw_wait_set_t object)
 *
 * @param[in] identifier 用于检查 RMW 实现是否匹配的字符串 (String used to check if the RMW
 * implementation matches)
 * @param[in] context 指向 rmw_context_t 的指针，用于存储 ROS2 上下文信息 (Pointer to an
 * rmw_context_t, which stores ROS2 context information)
 * @param[in] max_conditions 最大条件数，当前未使用 (Maximum number of conditions, currently unused)
 * @return 成功时返回指向新创建的 rmw_wait_set_t 的指针，失败时返回 nullptr (On success, returns a
 * pointer to the newly created rmw_wait_set_t; on failure, returns nullptr)
 */
rmw_wait_set_t *__rmw_create_wait_set(
    const char *identifier, rmw_context_t *context, size_t max_conditions) {
  // 检查是否可以返回 nullptr 错误 (Check if it can return with a nullptr error)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(nullptr);
  // 检查 context 参数是否为空 (Check if the context argument is null)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, NULL);
  // 检查类型标识符是否匹配 (Check if type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      init context, context->implementation_identifier, identifier,
      // TODO(wjwwood): replace this with RMW_RET_INCORRECT_RMW_IMPLEMENTATION when refactored
      return nullptr);

  // 忽略 max_conditions 参数 (Ignore the max_conditions parameter)
  (void)max_conditions;

  // 从这里开始，错误将在 goto fail 块中回滚 (From here onward, errors result in unrolling in the
  // goto fail block)
  eprosima::fastdds::dds::WaitSet *fastdds_wait_set = nullptr;
  // 为 rmw_wait_set_t 分配内存 (Allocate memory for rmw_wait_set_t)
  rmw_wait_set_t *wait_set = rmw_wait_set_allocate();
  if (!wait_set) {
    RMW_SET_ERROR_MSG("failed to allocate wait set");
    goto fail;
  }
  wait_set->implementation_identifier = identifier;
  // 为 eprosima::fastdds::dds::WaitSet 分配内存 (Allocate memory for
  // eprosima::fastdds::dds::WaitSet)
  wait_set->data = rmw_allocate(sizeof(eprosima::fastdds::dds::WaitSet));
  if (!wait_set->data) {
    RMW_SET_ERROR_MSG("failed to allocate wait set info");
    goto fail;
  }
  // 默认构造 CustomWaitsetInfo 的字段 (Default-construct the fields of CustomWaitsetInfo)
  RMW_TRY_PLACEMENT_NEW(
      fastdds_wait_set, wait_set->data, goto fail,
      // cppcheck-suppress syntaxError
      eprosima::fastdds::dds::WaitSet, );
  // 忽略 fastdds_wait_set 变量 (Ignore the fastdds_wait_set variable)
  (void)fastdds_wait_set;

  // 返回创建的 wait_set 对象 (Return the created wait_set object)
  return wait_set;

// 失败时，释放已分配的资源 (On failure, release allocated resources)
fail:
  if (wait_set) {
    if (wait_set->data) {
      rmw_free(wait_set->data);
    }
    rmw_wait_set_free(wait_set);
  }
  // 返回 nullptr 表示失败 (Return nullptr to indicate failure)
  return nullptr;
}

/**
 * @brief 销毁一个等待集 (Destroy a wait set)
 *
 * @param[in] identifier 指定的实现标识符 (The specified implementation identifier)
 * @param[in,out] wait_set 要销毁的等待集指针 (Pointer to the wait set to be destroyed)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_destroy_wait_set(const char *identifier, rmw_wait_set_t *wait_set) {
  // 检查 wait_set 是否为空，如果为空则返回错误 (Check if wait_set is NULL, return error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(wait_set, RMW_RET_ERROR);

  // 检查类型标识符是否匹配，如果不匹配则返回错误 (Check if type identifiers match, return error if
  // they don't)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      wait set handle, wait_set->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  auto result = RMW_RET_OK;

  // 如果 wait_set_info 为 nullptr，则可能有以下三种情况：
  // - 等待集无效。调用者未遵循前提条件。
  // - 实现逻辑上有问题。这绝对不是我们想要处理的正常错误。
  // - 堆已损坏。
  // 在所有这三种情况下，最好尽早使其崩溃。
  // (If wait_set_info is ever nullptr, it can only mean one of three things:
  // - Wait set is invalid. Caller did not respect preconditions.
  // - Implementation is logically broken. Definitely not something we want to treat as a normal
  // error.
  // - Heap is corrupt.
  // In all three cases, it's better if this crashes soon enough.)
  auto fastdds_wait_set = static_cast<eprosima::fastdds::dds::WaitSet *>(wait_set->data);

  if (wait_set->data) {
    if (fastdds_wait_set) {
      // 尝试析构 fastdds_wait_set，如果失败则返回错误 (Attempt to destruct fastdds_wait_set, return
      // error if failed)
      RMW_TRY_DESTRUCTOR(
          fastdds_wait_set->eprosima::fastdds::dds::WaitSet::~WaitSet(), fastdds_wait_set,
          result = RMW_RET_ERROR)
    }
    // 释放 wait_set->data 的内存 (Free the memory of wait_set->data)
    rmw_free(wait_set->data);
  }
  // 释放 wait_set 的内存 (Free the memory of wait_set)
  rmw_wait_set_free(wait_set);

  // 在完成时返回错误 (Return error on completion)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_ERROR);
  return result;
}

}  // namespace rmw_fastrtps_shared_cpp
