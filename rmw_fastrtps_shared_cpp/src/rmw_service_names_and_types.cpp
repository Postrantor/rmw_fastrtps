// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#include "demangle.hpp"
#include "rcutils/allocator.h"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/get_service_names_and_types.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/names_and_types.h"
#include "rmw/types.h"
#include "rmw_dds_common/context.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 获取服务名称和类型 (Get service names and types)
 *
 * @param[in] identifier 指定的实现标识符 (The specified implementation identifier)
 * @param[in] node 当前 ROS2 节点指针 (Pointer to the current ROS2 node)
 * @param[in] allocator 分配器，用于分配内存 (Allocator for allocating memory)
 * @param[out] service_names_and_types 服务名称和类型的结构体指针 (Pointer to the structure of
 * service names and types)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
rmw_ret_t __rmw_get_service_names_and_types(
    const char *identifier,
    const rmw_node_t *node,
    rcutils_allocator_t *allocator,
    rmw_names_and_types_t *service_names_and_types) {
  // 检查节点是否为空，如果为空返回无效参数错误 (Check if the node is null, return invalid argument
  // error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);

  // 检查节点的实现标识符与传入的标识符是否匹配，如果不匹配返回错误的 RMW 实现 (Check if the node's
  // implementation identifier matches the passed identifier, return incorrect RMW implementation if
  // not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查分配器是否有效，如果无效返回无效参数错误 (Check if the allocator is valid, return invalid
  // argument error if it is not)
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
      allocator, "allocator argument is invalid", return RMW_RET_INVALID_ARGUMENT);

  // 检查服务名称和类型是否已初始化，如果已初始化返回无效参数错误 (Check if the service names and
  // types are initialized, return invalid argument error if they are)
  if (RMW_RET_OK != rmw_names_and_types_check_zero(service_names_and_types)) {
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 获取节点上下文中的公共上下文 (Get the common context from the node context)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);

  // 从图缓存中获取服务名称和类型，并返回操作状态 (Get the service names and types from the graph
  // cache and return the operation status)
  return common_context->graph_cache.get_names_and_types(
      _demangle_service_from_topic, _demangle_service_type_only, allocator,
      service_names_and_types);
}

}  // namespace rmw_fastrtps_shared_cpp
