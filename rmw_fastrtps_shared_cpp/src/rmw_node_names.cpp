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

#include <string>

#include "rcutils/allocator.h"
#include "rcutils/logging_macros.h"
#include "rcutils/strdup.h"
#include "rcutils/types.h"
#include "rmw/allocators.h"
#include "rmw/convert_rcutils_ret_to_rmw_ret.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw/sanity_checks.h"
#include "rmw_dds_common/context.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 获取节点名称和命名空间
 *        Get node names and namespaces.
 *
 * @param[in] identifier 指向实现的标识符字符串的指针
 *              Pointer to the identifier string of the implementation.
 * @param[in] node 指向要查询的节点的指针
 *              Pointer to the node to query.
 * @param[out] node_names 用于存储节点名称的字符串数组
 *               String array to store the node names.
 * @param[out] node_namespaces 用于存储节点命名空间的字符串数组
 *               String array to store the node namespaces.
 * @return rmw_ret_t 返回操作结果
 *                   Return the operation result.
 */
rmw_ret_t __rmw_get_node_names(
    const char *identifier,
    const rmw_node_t *node,
    rcutils_string_array_t *node_names,
    rcutils_string_array_t *node_namespaces) {
  // 检查节点是否为空，如果为空则返回无效参数错误
  // Check if the node is null, return invalid argument error if it is.
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);

  // 检查节点类型标识符是否匹配，如果不匹配则返回错误的 RMW 实现错误
  // Check if the node type identifiers match, return incorrect RMW implementation error if not.
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查节点名称字符串数组是否为空，如果为空则返回无效参数错误
  // Check if the node names string array is empty, return invalid argument error if it is.
  if (RMW_RET_OK != rmw_check_zero_rmw_string_array(node_names)) {
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 检查节点命名空间字符串数组是否为空，如果为空则返回无效参数错误
  // Check if the node namespaces string array is empty, return invalid argument error if it is.
  if (RMW_RET_OK != rmw_check_zero_rmw_string_array(node_namespaces)) {
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 获取公共上下文并转换为 rmw_dds_common::Context 类型
  // Get the common context and cast it to rmw_dds_common::Context type.
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);

  // 获取默认分配器
  // Get the default allocator.
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  // 从图缓存中获取节点名称、命名空间和分配器
  // Get node names, namespaces, and allocator from the graph cache.
  return common_context->graph_cache.get_node_names(
      node_names, node_namespaces, nullptr, &allocator);
}

/**
 * @brief 获取节点名称、命名空间和围地（enclaves）
 *        Get node names, namespaces, and enclaves.
 *
 * @param[in] identifier 指向实现的标识符字符串的指针
 *              Pointer to the identifier string of the implementation.
 * @param[in] node 指向要查询的节点的指针
 *              Pointer to the node to query.
 * @param[out] node_names 用于存储节点名称的字符串数组
 *               String array to store the node names.
 * @param[out] node_namespaces 用于存储节点命名空间的字符串数组
 *               String array to store the node namespaces.
 * @param[out] enclaves 用于存储围地（enclaves）的字符串数组
 *               String array to store the enclaves.
 * @return rmw_ret_t 返回操作结果
 *                   Return the operation result.
 */
rmw_ret_t __rmw_get_node_names_with_enclaves(
    const char *identifier,
    const rmw_node_t *node,
    rcutils_string_array_t *node_names,
    rcutils_string_array_t *node_namespaces,
    rcutils_string_array_t *enclaves) {
  // 检查节点是否为空，如果为空则返回无效参数错误
  // Check if the node is null, return invalid argument error if it is.
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);

  // 检查节点类型标识符是否匹配，如果不匹配则返回错误的 RMW 实现错误
  // Check if the node type identifiers match, return incorrect RMW implementation error if not.
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查节点名称字符串数组是否为空，如果为空则返回无效参数错误
  // Check if the node names string array is empty, return invalid argument error if it is.
  if (RMW_RET_OK != rmw_check_zero_rmw_string_array(node_names)) {
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 检查节点命名空间字符串数组是否为空，如果为空则返回无效参数错误
  // Check if the node namespaces string array is empty, return invalid argument error if it is.
  if (RMW_RET_OK != rmw_check_zero_rmw_string_array(node_namespaces)) {
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 检查围地（enclaves）字符串数组是否为空，如果为空则返回无效参数错误
  // Check if the enclaves string array is empty, return invalid argument error if it is.
  if (RMW_RET_OK != rmw_check_zero_rmw_string_array(enclaves)) {
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 获取公共上下文并转换为 rmw_dds_common::Context 类型
  // Get the common context and cast it to rmw_dds_common::Context type.
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);

  // 获取默认分配器
  // Get the default allocator.
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  // 从图缓存中获取节点名称、命名空间、围地（enclaves）和分配器
  // Get node names, namespaces, enclaves, and allocator from the graph cache.
  return common_context->graph_cache.get_node_names(
      node_names, node_namespaces, enclaves, &allocator);
}

}  // namespace rmw_fastrtps_shared_cpp
