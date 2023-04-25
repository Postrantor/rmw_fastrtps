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
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/ret_types.h"
#include "rmw/rmw.h"
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_cpp/init_rmw_context_impl.hpp"
#include "rmw_fastrtps_shared_cpp/init_rmw_context_impl.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"

extern "C" {

/**
 * @brief 创建一个 ROS2 节点 (Create a ROS2 node)
 *
 * @param[in] context 指向 ROS2 上下文的指针 (Pointer to the ROS2 context)
 * @param[in] name 节点的名称 (Name of the node)
 * @param[in] namespace_ 节点的命名空间 (Namespace of the node)
 * @return 返回创建的节点，如果失败则返回 nullptr (Returns the created node, or nullptr if failed)
 */
rmw_node_t* rmw_create_node(rmw_context_t* context, const char* name, const char* namespace_) {
  // 检查 context 是否为空 (Check if context is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(context, nullptr);
  // 检查类型标识符是否匹配 (Check if type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      init context, context->implementation_identifier, eprosima_fastrtps_identifier,
      // TODO(wjwwood): replace this with RMW_RET_INCORRECT_RMW_IMPLEMENTATION when refactored
      return nullptr);
  // 检查 context 是否已初始化 (Check if context is initialized)
  RMW_CHECK_FOR_NULL_WITH_MSG(context->impl, "expected initialized context", return nullptr);
  // 检查 context 是否已关闭 (Check if context has been shutdown)
  if (context->impl->is_shutdown) {
    RCUTILS_SET_ERROR_MSG("context has been shutdown");
    return nullptr;
  }

  // 增加上下文引用计数 (Increment context reference count)
  if (RMW_RET_OK != rmw_fastrtps_cpp::increment_context_impl_ref_count(context)) {
    return nullptr;
  }

  // 创建节点 (Create node)
  rmw_node_t* node = rmw_fastrtps_shared_cpp::__rmw_create_node(
      context, eprosima_fastrtps_identifier, name, namespace_);

  // 如果创建失败，减少上下文引用计数 (If creation failed, decrement context reference count)
  if (nullptr == node) {
    if (RMW_RET_OK != rmw_fastrtps_shared_cpp::decrement_context_impl_ref_count(context)) {
      RCUTILS_SAFE_FWRITE_TO_STDERR(
          "'decrement_context_impl_ref_count' failed while being executed due to "
          "'" RCUTILS_STRINGIFY(__function__) "' failing");
    }
  }
  return node;
}

/**
 * @brief 销毁一个 ROS2 节点 (Destroy a ROS2 node)
 *
 * @param[in] node 要销毁的节点指针 (Pointer to the node to be destroyed)
 * @return 返回 rmw_ret_t 类型的结果代码 (Returns a result code of type rmw_ret_t)
 */
rmw_ret_t rmw_destroy_node(rmw_node_t* node) {
  // 检查节点是否为空 (Check if node is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  // 检查类型标识符是否匹配 (Check if type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 获取节点的上下文 (Get the node's context)
  rmw_context_t* context = node->context;

  // 初始化返回值和错误状态 (Initialize return value and error state)
  rmw_ret_t ret = RMW_RET_OK;
  rmw_error_state_t error_state;
  // 销毁节点 (Destroy the node)
  rmw_ret_t inner_ret =
      rmw_fastrtps_shared_cpp::__rmw_destroy_node(eprosima_fastrtps_identifier, node);
  // 更新返回值和错误状态 (Update return value and error state)
  if (RMW_RET_OK != ret) {
    error_state = *rmw_get_error_state();
    ret = inner_ret;
    rmw_reset_error();
  }

  // 减少上下文引用计数 (Decrement context reference count)
  inner_ret = rmw_fastrtps_shared_cpp::decrement_context_impl_ref_count(context);
  // 更新返回值和错误状态 (Update return value and error state)
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

  // 如果返回值不是 RMW_RET_OK，则设置错误状态 (If return value is not RMW_RET_OK, set error state)
  if (RMW_RET_OK != ret) {
    rmw_set_error_state(error_state.message, error_state.file, error_state.line_number);
  }

  return ret;
}

/**
 * @brief 获取 ROS2 节点的图形保护条件 (Get the graph guard condition of a ROS2 node)
 *
 * @param[in] node 指向 ROS2 节点的指针 (Pointer to the ROS2 node)
 * @return 返回指向 rmw_guard_condition_t 类型的保护条件的指针，如果失败则返回 nullptr (Returns a
 * pointer to the guard condition of type rmw_guard_condition_t, or nullptr if failed)
 */
const rmw_guard_condition_t* rmw_node_get_graph_guard_condition(const rmw_node_t* node) {
  return rmw_fastrtps_shared_cpp::__rmw_node_get_graph_guard_condition(node);
}

}  // extern "C"
