// Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <algorithm>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "demangle.hpp"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw/topic_endpoint_info.h"
#include "rmw/topic_endpoint_info_array.h"
#include "rmw/types.h"
#include "rmw_dds_common/graph_cache.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/names.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 验证参数的有效性 (Validate the arguments)
 *
 * @param[in] identifier 实现标识符 (Implementation identifier)
 * @param[in] node ROS2节点指针 (Pointer to the ROS2 node)
 * @param[in,out] allocator 内存分配器 (Memory allocator)
 * @param[in] topic_name 话题名称 (Topic name)
 * @param[out] participants_info 参与者信息数组 (Array of participant information)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
static rmw_ret_t __validate_arguments(
    const char *identifier,
    const rmw_node_t *node,
    rcutils_allocator_t *allocator,
    const char *topic_name,
    rmw_topic_endpoint_info_array_t *participants_info) {
  // 检查节点是否为空 (Check if the node is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  // 检查类型标识符是否匹配 (Check if the type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查内存分配器是否有效 (Check if the allocator is valid)
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
      allocator, "allocator argument is invalid", return RMW_RET_INVALID_ARGUMENT);
  // 检查话题名称是否为空 (Check if the topic name is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(topic_name, RMW_RET_INVALID_ARGUMENT);
  // 检查参与者信息数组是否为零 (Check if the participant info array is zero)
  if (RMW_RET_OK != rmw_topic_endpoint_info_array_check_zero(participants_info)) {
    return RMW_RET_INVALID_ARGUMENT;
  }
  return RMW_RET_OK;
}

/**
 * @brief 获取指定话题的发布者信息 (Get publisher information for the specified topic)
 *
 * @param[in] identifier 实现标识符 (Implementation identifier)
 * @param[in] node ROS2节点指针 (Pointer to the ROS2 node)
 * @param[in,out] allocator 内存分配器 (Memory allocator)
 * @param[in] topic_name 话题名称 (Topic name)
 * @param[in] no_mangle 是否禁用话题名称修饰 (Whether to disable topic name mangling)
 * @param[out] publishers_info 发布者信息数组 (Array of publisher information)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_get_publishers_info_by_topic(
    const char *identifier,
    const rmw_node_t *node,
    rcutils_allocator_t *allocator,
    const char *topic_name,
    bool no_mangle,
    rmw_topic_endpoint_info_array_t *publishers_info) {
  // 验证参数有效性 (Validate the arguments)
  rmw_ret_t ret = __validate_arguments(identifier, node, allocator, topic_name, publishers_info);
  if (ret != RMW_RET_OK) {
    return ret;
  }
  // 获取公共上下文 (Get the common context)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);
  // 初始化修饰后的话题名称 (Initialize the mangled topic name)
  std::string mangled_topic_name = topic_name;
  // 设置反修饰函数 (Set the demangle function)
  DemangleFunction demangle_type = _identity_demangle;
  // 如果不禁用修饰 (If mangling is not disabled)
  if (!no_mangle) {
    // 修饰话题名称 (Mangle the topic name)
    mangled_topic_name = _mangle_topic_name(ros_topic_prefix, topic_name).to_string();
    // 设置反修饰函数为 ROS 类型 (Set the demangle function to ROS type)
    demangle_type = _demangle_if_ros_type;
  }

  // 获取指定话题的发布者信息 (Get publisher information for the specified topic)
  return common_context->graph_cache.get_writers_info_by_topic(
      mangled_topic_name, demangle_type, allocator, publishers_info);
}

/**
 * @brief 获取订阅者信息列表，根据给定的主题名称 (Get the list of subscription information by the
 * given topic name)
 *
 * @param[in] identifier 节点标识符 (Node identifier)
 * @param[in] node ROS2节点指针 (Pointer to the ROS2 node)
 * @param[in] allocator 分配器用于分配内存 (Allocator for memory allocation)
 * @param[in] topic_name 主题名称 (Topic name)
 * @param[in] no_mangle 是否不对主题名称进行处理 (Whether to not mangle the topic name)
 * @param[out] subscriptions_info 订阅者信息数组 (Array of subscription information)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_get_subscriptions_info_by_topic(
    const char *identifier,
    const rmw_node_t *node,
    rcutils_allocator_t *allocator,
    const char *topic_name,
    bool no_mangle,
    rmw_topic_endpoint_info_array_t *subscriptions_info) {
  // 验证输入参数是否有效 (Validate if the input arguments are valid)
  rmw_ret_t ret = __validate_arguments(identifier, node, allocator, topic_name, subscriptions_info);
  // 如果验证失败，返回错误代码 (If validation fails, return the error code)
  if (ret != RMW_RET_OK) {
    return ret;
  }
  // 获取通用上下文 (Get the common context)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);
  // 初始化处理后的主题名称 (Initialize the mangled topic name)
  std::string mangled_topic_name = topic_name;
  // 设置反混淆函数 (Set the demangle function)
  DemangleFunction demangle_type = _identity_demangle;
  // 如果需要处理主题名称 (If mangling the topic name is required)
  if (!no_mangle) {
    // 处理主题名称 (Mangle the topic name)
    mangled_topic_name = _mangle_topic_name(ros_topic_prefix, topic_name).to_string();
    // 设置反混淆函数为 ROS 类型 (Set the demangle function for ROS type)
    demangle_type = _demangle_if_ros_type;
  }

  // 获取订阅者信息列表 (Get the list of subscription information)
  return common_context->graph_cache.get_readers_info_by_topic(
      mangled_topic_name, demangle_type, allocator, subscriptions_info);
}

}  // namespace rmw_fastrtps_shared_cpp
