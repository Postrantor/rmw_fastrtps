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
#include "rmw/get_topic_names_and_types.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/names_and_types.h"
#include "rmw/types.h"
#include "rmw_dds_common/context.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 获取话题名称和类型 (Get topic names and types)
 *
 * @param[in] identifier 实现标识符 (Implementation identifier)
 * @param[in] node ROS2节点指针 (Pointer to the ROS2 node)
 * @param[in,out] allocator 用于分配内存的分配器 (Allocator for memory allocation)
 * @param[in] no_demangle 是否禁用反混淆 (Whether to disable demangling)
 * @param[out] topic_names_and_types 存储获取到的话题名称和类型的结构体指针 (Pointer to the
 * structure that stores the obtained topic names and types)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
rmw_ret_t __rmw_get_topic_names_and_types(
    const char *identifier,
    const rmw_node_t *node,
    rcutils_allocator_t *allocator,
    bool no_demangle,
    rmw_names_and_types_t *topic_names_and_types) {
  // 检查节点是否为空，如果为空返回无效参数错误 (Check if the node is null, return invalid argument
  // error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  // 检查节点实现标识符与传入标识符是否匹配，如果不匹配返回错误的RMW实现错误 (Check if the node
  // implementation identifier matches the incoming identifier, return incorrect RMW implementation
  // error if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查分配器是否有效，如果无效返回无效参数错误 (Check if the allocator is valid, return invalid
  // argument error if it is not)
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
      allocator, "allocator argument is invalid", return RMW_RET_INVALID_ARGUMENT);
  // 检查话题名称和类型是否为零，如果不为零返回无效参数错误 (Check if the topic names and types are
  // zero, return invalid argument error if not)
  if (RMW_RET_OK != rmw_names_and_types_check_zero(topic_names_and_types)) {
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 定义反混淆函数指针 (Define demangle function pointers)
  DemangleFunction demangle_topic = _demangle_ros_topic_from_topic;
  DemangleFunction demangle_type = _demangle_if_ros_type;

  // 如果禁用反混淆，则使用标识反混淆函数 (If demangling is disabled, use identity demangle
  // functions)
  if (no_demangle) {
    demangle_topic = _identity_demangle;
    demangle_type = _identity_demangle;
  }
  // 获取节点的公共上下文 (Get the common context of the node)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);

  // 使用图缓存获取话题名称和类型 (Get topic names and types using graph cache)
  return common_context->graph_cache.get_names_and_types(
      demangle_topic, demangle_type, allocator, topic_names_and_types);
}

}  // namespace rmw_fastrtps_shared_cpp
