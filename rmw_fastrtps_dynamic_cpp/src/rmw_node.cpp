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

#include <array>
#include <set>
#include <string>
#include <utility>

#include "rcutils/filesystem.h"
#include "rcutils/logging_macros.h"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_dynamic_cpp/init_rmw_context_impl.hpp"
#include "rmw_fastrtps_shared_cpp/init_rmw_context_impl.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"

extern "C" {

/**
 * @brief 创建一个 ROS2 节点 (Create a ROS2 node)
 *
 * @param[in] context 指向 rmw_context_t 结构体的指针，用于存储 ROS2 上下文信息 (Pointer to an
 * rmw_context_t structure, which stores ROS2 context information)
 * @param[in] name 节点名称 (Node name)
 * @param[in] namespace_ 节点所在的命名空间 (Namespace the node belongs to)
 * @return rmw_node_t* 创建成功时返回指向新节点的指针，否则返回 nullptr (Returns a pointer to the
 * new node if successful, otherwise returns nullptr)
 */
rmw_node_t* rmw_create_node(rmw_context_t* context, const char* name, const char* namespace_) {
  // 检查 context 参数是否为空，如果为空则返回 nullptr
  // Check if the context argument is null, return nullptr if it is
  RMW_CHECK_ARGUMENT_FOR_NULL(context, nullptr);

  // 检查 context 的实现标识符是否与 eprosima_fastrtps_identifier 匹配，如果不匹配则返回 nullptr
  // Check if the implementation identifier of the context matches eprosima_fastrtps_identifier,
  // return nullptr if not
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      init context, context->implementation_identifier, eprosima_fastrtps_identifier,
      // TODO(wjwwood): replace this with RMW_RET_INCORRECT_RMW_IMPLEMENTATION when refactored
      return nullptr);

  // 检查 context 的 impl 是否为空，如果为空则返回 nullptr
  // Check if the impl of the context is null, return nullptr if it is
  RMW_CHECK_FOR_NULL_WITH_MSG(context->impl, "expected initialized context", return nullptr);

  // 检查 context 是否已经关闭，如果已经关闭则返回 nullptr
  // Check if the context has been shut down, return nullptr if it has
  if (context->impl->is_shutdown) {
    RCUTILS_SET_ERROR_MSG("context has been shutdown");
    return nullptr;
  }

  // 增加 context 的引用计数，如果失败则返回 nullptr
  // Increment the reference count of the context, return nullptr if failed
  if (RMW_RET_OK != rmw_fastrtps_dynamic_cpp::increment_context_impl_ref_count(context)) {
    return nullptr;
  }

  // 使用给定的参数创建一个新节点
  // Create a new node with the given parameters
  rmw_node_t* node = rmw_fastrtps_shared_cpp::__rmw_create_node(
      context, eprosima_fastrtps_identifier, name, namespace_);

  // 如果节点创建失败，则尝试减少 context 的引用计数
  // If the node creation fails, try to decrement the reference count of the context
  if (nullptr == node) {
    if (RMW_RET_OK != rmw_fastrtps_shared_cpp::decrement_context_impl_ref_count(context)) {
      RCUTILS_SAFE_FWRITE_TO_STDERR(
          "'decrement_context_impl_ref_count' failed while being executed due to "
          "'" RCUTILS_STRINGIFY(__function__) "' failing");
    }
  }

  // 返回新创建的节点指针
  // Return the pointer to the newly created node
  return node;
}

/**
 * @brief 销毁一个 ROS2 节点 (Destroy a ROS2 node)
 *
 * @param[in] node 要销毁的节点指针 (Pointer to the node to be destroyed)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_destroy_node(rmw_node_t* node) {
  // 检查输入参数是否为空，如果为空则返回无效参数错误 (Check if the input argument is null, return
  // invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);

  // 检查节点实现标识符是否匹配，如果不匹配则返回错误的 RMW 实现错误 (Check if the node
  // implementation identifier matches, return incorrect RMW implementation error if it doesn't)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 获取节点上下文 (Get the node context)
  rmw_context_t* context = node->context;

  // 初始化返回值为成功 (Initialize the return value as successful)
  rmw_ret_t ret = RMW_RET_OK;
  // 初始化错误状态 (Initialize the error state)
  rmw_error_state_t error_state;

  // 调用内部函数销毁节点，并获取返回值 (Call the internal function to destroy the node and get the
  // return value)
  rmw_ret_t inner_ret =
      rmw_fastrtps_shared_cpp::__rmw_destroy_node(eprosima_fastrtps_identifier, node);

  // 如果内部返回值不是成功，则更新错误状态和返回值 (If the internal return value is not successful,
  // update the error state and return value)
  if (RMW_RET_OK != ret) {
    error_state = *rmw_get_error_state();
    ret = inner_ret;
    rmw_reset_error();
  }

  // 减少上下文实现引用计数，并获取返回值 (Decrease the context implementation reference count and
  // get the return value)
  inner_ret = rmw_fastrtps_shared_cpp::decrement_context_impl_ref_count(context);

  // 如果内部返回值不是成功，则更新错误状态和返回值 (If the internal return value is not successful,
  // update the error state and return value)
  if (RMW_RET_OK != inner_ret) {
    if (RMW_RET_OK != ret) {
      RMW_SAFE_FWRITE_TO_STDERR(rmw_get_error_string().str);
      RMW_SAFE_FWRITE_TO_STDERR(" during '" RCUTILS_STRINGIFY(__function__) "'\n");
    } else {
      error_state = *rmw_get_error_state();
      ret = inner_ret;
    }
    rmw_reset_error();
  }

  // 如果返回值不是成功，则设置错误状态 (If the return value is not successful, set the error state)
  if (RMW_RET_OK != ret) {
    rmw_set_error_state(error_state.message, error_state.file, error_state.line_number);
  }

  // 返回操作结果 (Return the operation result)
  return ret;
}

/**
 * @brief 获取节点的图形保护条件 (Get the graph guard condition of the node)
 *
 * @param[in] node 要查询的节点指针 (Pointer to the node to be queried)
 * @return const rmw_guard_condition_t* 返回节点的图形保护条件指针 (Return the pointer to the graph
 * guard condition of the node)
 */
const rmw_guard_condition_t* rmw_node_get_graph_guard_condition(const rmw_node_t* node) {
  // 调用内部函数获取节点的图形保护条件，并返回结果 (Call the internal function to get the graph
  // guard condition of the node and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_node_get_graph_guard_condition(node);
}

}  // extern "C"
