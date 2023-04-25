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

#include <map>
#include <mutex>
#include <numeric>
#include <string>
#include <vector>

#include "rcutils/logging_macros.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw/types.h"
#include "rmw/validate_full_topic_name.h"
#include "rmw_dds_common/context.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/names.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 计算发布者的数量 (Count the number of publishers)
 *
 * @param[in] identifier 指定的实现标识符 (The specified implementation identifier)
 * @param[in] node 要查询的 ROS2 节点 (The ROS2 node to query)
 * @param[in] topic_name 要查询的主题名称 (The topic name to query)
 * @param[out] count 存储发布者数量的指针 (Pointer to store the number of publishers)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_count_publishers(
    const char *identifier, const rmw_node_t *node, const char *topic_name, size_t *count) {
  // 检查节点是否为空 (Check if the node is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  // 检查类型标识符是否匹配 (Check if the type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查主题名称是否为空 (Check if the topic name is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(topic_name, RMW_RET_INVALID_ARGUMENT);
  int validation_result = RMW_TOPIC_VALID;
  // 验证完整的主题名称 (Validate the full topic name)
  rmw_ret_t ret = rmw_validate_full_topic_name(topic_name, &validation_result, nullptr);
  if (RMW_RET_OK != ret) {
    return ret;
  }
  if (RMW_TOPIC_VALID != validation_result) {
    const char *reason = rmw_full_topic_name_validation_result_string(validation_result);
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("topic_name argument is invalid: %s", reason);
    return RMW_RET_INVALID_ARGUMENT;
  }
  // 检查计数指针是否为空 (Check if the count pointer is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(count, RMW_RET_INVALID_ARGUMENT);
  // 获取公共上下文 (Get the common context)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);
  // 转换主题名称 (Mangle the topic name)
  const std::string mangled_topic_name =
      _mangle_topic_name(ros_topic_prefix, topic_name).to_string();
  // 获取发布者数量 (Get the number of publishers)
  return common_context->graph_cache.get_writer_count(mangled_topic_name, count);
}

/**
 * @brief 计算订阅者的数量 (Count the number of subscribers)
 *
 * @param[in] identifier 指定的实现标识符 (The specified implementation identifier)
 * @param[in] node 要查询的 ROS2 节点 (The ROS2 node to query)
 * @param[in] topic_name 要查询的主题名称 (The topic name to query)
 * @param[out] count 存储订阅者数量的指针 (Pointer to store the number of subscribers)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_count_subscribers(
    const char *identifier, const rmw_node_t *node, const char *topic_name, size_t *count) {
  // 检查节点是否为空 (Check if the node is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  // 检查类型标识符是否匹配 (Check if the type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查主题名称是否为空 (Check if the topic name is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(topic_name, RMW_RET_INVALID_ARGUMENT);
  int validation_result = RMW_TOPIC_VALID;
  // 验证完整的主题名称 (Validate the full topic name)
  rmw_ret_t ret = rmw_validate_full_topic_name(topic_name, &validation_result, nullptr);
  if (RMW_RET_OK != ret) {
    return ret;
  }
  if (RMW_TOPIC_VALID != validation_result) {
    const char *reason = rmw_full_topic_name_validation_result_string(validation_result);
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("topic_name argument is invalid: %s", reason);
    return RMW_RET_INVALID_ARGUMENT;
  }
  // 检查计数指针是否为空 (Check if the count pointer is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(count, RMW_RET_INVALID_ARGUMENT);
  // 获取公共上下文 (Get the common context)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);
  // 转换主题名称 (Mangle the topic name)
  const std::string mangled_topic_name =
      _mangle_topic_name(ros_topic_prefix, topic_name).to_string();
  // 获取订阅者数量 (Get the number of subscribers)
  return common_context->graph_cache.get_reader_count(mangled_topic_name, count);
}

}  // namespace rmw_fastrtps_shared_cpp
