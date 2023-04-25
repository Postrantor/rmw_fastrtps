// Copyright 2019 Open Source Robotics Foundation, Inc.
// Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <functional>
#include <map>
#include <set>
#include <string>
#include <utility>

#include "demangle.hpp"
#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/logging_macros.h"
#include "rcutils/strdup.h"
#include "rcutils/types.h"
#include "rmw/allocators.h"
#include "rmw/convert_rcutils_ret_to_rmw_ret.h"
#include "rmw/error_handling.h"
#include "rmw/get_topic_names_and_types.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/names_and_types.h"
#include "rmw/rmw.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"
#include "rmw_dds_common/context.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/names.hpp"
#include "rmw_fastrtps_shared_cpp/namespace_prefix.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief GetNamesAndTypesByNodeFunction 类型定义，用于获取节点的主题名称和类型。
 *
 * @param[in] rmw_dds_common::Context * 上下文指针
 * @param[in] const std::string & 节点名称
 * @param[in] const std::string & 节点命名空间
 * @param[in] DemangleFunction 反混淆主题函数
 * @param[in] DemangleFunction 反混淆类型函数
 * @param[in] rcutils_allocator_t * 分配器指针
 * @param[out] rmw_names_and_types_t * 主题名称和类型结构体指针
 * @return rmw_ret_t 返回操作状态
 */
using GetNamesAndTypesByNodeFunction = rmw_ret_t (*)(
    rmw_dds_common::Context *,
    const std::string &,
    const std::string &,
    DemangleFunction,
    DemangleFunction,
    rcutils_allocator_t *,
    rmw_names_and_types_t *);

/**
 * @brief 获取指定节点的主题名称和类型。
 *
 * @param[in] identifier 实现标识符
 * @param[in] node 节点指针
 * @param[in] allocator 分配器指针
 * @param[in] node_name 节点名称
 * @param[in] node_namespace 节点命名空间
 * @param[in] demangle_topic 反混淆主题函数
 * @param[in] demangle_type 反混淆类型函数
 * @param[in] no_demangle 是否禁用反混淆
 * @param[in] get_names_and_types_by_node 获取节点主题名称和类型的函数指针
 * @param[out] topic_names_and_types 主题名称和类型结构体指针
 * @return rmw_ret_t 返回操作状态
 */
rmw_ret_t __rmw_get_topic_names_and_types_by_node(
    const char *identifier,
    const rmw_node_t *node,
    rcutils_allocator_t *allocator,
    const char *node_name,
    const char *node_namespace,
    DemangleFunction demangle_topic,
    DemangleFunction demangle_type,
    bool no_demangle,
    GetNamesAndTypesByNodeFunction get_names_and_types_by_node,
    rmw_names_and_types_t *topic_names_and_types) {
  // 检查 node 参数是否为空
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  // 检查 node 的实现标识符是否匹配
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查分配器参数是否有效
  RCUTILS_CHECK_ALLOCATOR_WITH_MSG(
      allocator, "allocator argument is invalid", return RMW_RET_INVALID_ARGUMENT);
  // 验证节点名称
  int validation_result = RMW_NODE_NAME_VALID;
  rmw_ret_t ret = rmw_validate_node_name(node_name, &validation_result, nullptr);
  if (RMW_RET_OK != ret) {
    return ret;
  }
  if (RMW_NODE_NAME_VALID != validation_result) {
    const char *reason = rmw_node_name_validation_result_string(validation_result);
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("node_name argument is invalid: %s", reason);
    return RMW_RET_INVALID_ARGUMENT;
  }
  // 验证节点命名空间
  validation_result = RMW_NAMESPACE_VALID;
  ret = rmw_validate_namespace(node_namespace, &validation_result, nullptr);
  if (RMW_RET_OK != ret) {
    return ret;
  }
  if (RMW_NAMESPACE_VALID != validation_result) {
    const char *reason = rmw_namespace_validation_result_string(validation_result);
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("node_namespace argument is invalid: %s", reason);
    return RMW_RET_INVALID_ARGUMENT;
  }
  // 检查主题名称和类型是否为空
  ret = rmw_names_and_types_check_zero(topic_names_and_types);
  if (RMW_RET_OK != ret) {
    return ret;
  }
  // 获取通用上下文
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);

  // 如果禁用反混淆，则设置为 _identity_demangle 函数
  if (no_demangle) {
    demangle_topic = _identity_demangle;
    demangle_type = _identity_demangle;
  }

  // 调用 get_names_and_types_by_node 函数获取节点的主题名称和类型
  return get_names_and_types_by_node(
      common_context, node_name, node_namespace, demangle_topic, demangle_type, allocator,
      topic_names_and_types);
}

/**
 * @brief 获取节点的读取器名称和类型 (Get reader names and types for a node)
 *
 * @param common_context 共享上下文指针 (Pointer to the shared context)
 * @param node_name 节点名称 (Node name)
 * @param node_namespace 节点命名空间 (Node namespace)
 * @param demangle_topic 解析主题函数 (Function to demangle topic)
 * @param demangle_type 解析类型函数 (Function to demangle type)
 * @param allocator 分配器指针 (Pointer to the allocator)
 * @param topic_names_and_types 存储获取到的主题名称和类型的结构体指针 (Pointer to the structure
 * that stores the obtained topic names and types)
 * @return rmw_ret_t 返回值状态 (Return value status)
 */
rmw_ret_t __get_reader_names_and_types_by_node(
    rmw_dds_common::Context *common_context,
    const std::string &node_name,
    const std::string &node_namespace,
    DemangleFunction demangle_topic,
    DemangleFunction demangle_type,
    rcutils_allocator_t *allocator,
    rmw_names_and_types_t *topic_names_and_types) {
  // 调用 graph_cache 的 get_reader_names_and_types_by_node 方法获取读取器名称和类型 (Call the
  // get_reader_names_and_types_by_node method of graph_cache to get the reader names and types)
  return common_context->graph_cache.get_reader_names_and_types_by_node(
      node_name, node_namespace, demangle_topic, demangle_type, allocator, topic_names_and_types);
}

/**
 * @brief 获取节点的写入器名称和类型 (Get writer names and types for a node)
 *
 * @param common_context 共享上下文指针 (Pointer to the shared context)
 * @param node_name 节点名称 (Node name)
 * @param node_namespace 节点命名空间 (Node namespace)
 * @param demangle_topic 解析主题函数 (Function to demangle topic)
 * @param demangle_type 解析类型函数 (Function to demangle type)
 * @param allocator 分配器指针 (Pointer to the allocator)
 * @param topic_names_and_types 存储获取到的主题名称和类型的结构体指针 (Pointer to the structure
 * that stores the obtained topic names and types)
 * @return rmw_ret_t 返回值状态 (Return value status)
 */
rmw_ret_t __get_writer_names_and_types_by_node(
    rmw_dds_common::Context *common_context,
    const std::string &node_name,
    const std::string &node_namespace,
    DemangleFunction demangle_topic,
    DemangleFunction demangle_type,
    rcutils_allocator_t *allocator,
    rmw_names_and_types_t *topic_names_and_types) {
  // 调用 graph_cache 的 get_writer_names_and_types_by_node 方法获取写入器名称和类型 (Call the
  // get_writer_names_and_types_by_node method of graph_cache to get the writer names and types)
  return common_context->graph_cache.get_writer_names_and_types_by_node(
      node_name, node_namespace, demangle_topic, demangle_type, allocator, topic_names_and_types);
}

/**
 * @brief 获取节点的订阅者名称和类型 (Get subscriber names and types for a node)
 *
 * @param identifier 标识符 (Identifier)
 * @param node 节点指针 (Pointer to the node)
 * @param allocator 分配器指针 (Pointer to the allocator)
 * @param node_name 节点名称 (Node name)
 * @param node_namespace 节点命名空间 (Node namespace)
 * @param no_demangle 是否不解析 (Whether to not demangle)
 * @param topic_names_and_types 存储获取到的主题名称和类型的结构体指针 (Pointer to the structure
 * that stores the obtained topic names and types)
 * @return rmw_ret_t 返回值状态 (Return value status)
 */
rmw_ret_t __rmw_get_subscriber_names_and_types_by_node(
    const char *identifier,
    const rmw_node_t *node,
    rcutils_allocator_t *allocator,
    const char *node_name,
    const char *node_namespace,
    bool no_demangle,
    rmw_names_and_types_t *topic_names_and_types) {
  // 调用 __rmw_get_topic_names_and_types_by_node 方法获取订阅者名称和类型 (Call the
  // __rmw_get_topic_names_and_types_by_node method to get subscriber names and types)
  return __rmw_get_topic_names_and_types_by_node(
      identifier, node, allocator, node_name, node_namespace, _demangle_ros_topic_from_topic,
      _demangle_if_ros_type, no_demangle, __get_reader_names_and_types_by_node,
      topic_names_and_types);
}

/**
 * @brief 获取节点的发布者名称和类型 (Get publisher names and types for a node)
 *
 * @param identifier 标识符 (Identifier)
 * @param node 节点指针 (Pointer to the node)
 * @param allocator 分配器指针 (Pointer to the allocator)
 * @param node_name 节点名称 (Node name)
 * @param node_namespace 节点命名空间 (Node namespace)
 * @param no_demangle 是否不解析 (Whether to not demangle)
 * @param topic_names_and_types 存储获取到的主题名称和类型的结构体指针 (Pointer to the structure
 * that stores the obtained topic names and types)
 * @return rmw_ret_t 返回值状态 (Return value status)
 */
rmw_ret_t __rmw_get_publisher_names_and_types_by_node(
    const char *identifier,
    const rmw_node_t *node,
    rcutils_allocator_t *allocator,
    const char *node_name,
    const char *node_namespace,
    bool no_demangle,
    rmw_names_and_types_t *topic_names_and_types) {
  // 调用 __rmw_get_topic_names_and_types_by_node 方法获取发布者名称和类型 (Call the
  // __rmw_get_topic_names_and_types_by_node method to get publisher names and types)
  return __rmw_get_topic_names_and_types_by_node(
      identifier, node, allocator, node_name, node_namespace, _demangle_ros_topic_from_topic,
      _demangle_if_ros_type, no_demangle, __get_writer_names_and_types_by_node,
      topic_names_and_types);
}

/**
 * @brief 获取指定节点的服务名称和类型 (Get service names and types for a specific node)
 *
 * @param[in] identifier 节点标识符 (Node identifier)
 * @param[in] node 指向 rmw_node_t 结构体的指针 (Pointer to the rmw_node_t structure)
 * @param[in] allocator 分配器，用于分配内存空间 (Allocator for allocating memory space)
 * @param[in] node_name 节点名称 (Node name)
 * @param[in] node_namespace 节点命名空间 (Node namespace)
 * @param[out] service_names_and_types 服务名称和类型的结构体指针 (Pointer to the structure of
 * service names and types)
 * @return rmw_ret_t 返回状态 (Return status)
 */
rmw_ret_t __rmw_get_service_names_and_types_by_node(
    const char *identifier,
    const rmw_node_t *node,
    rcutils_allocator_t *allocator,
    const char *node_name,
    const char *node_namespace,
    rmw_names_and_types_t *service_names_and_types) {
  // 调用通用函数获取主题名称和类型 (Call the generic function to get topic names and types)
  return __rmw_get_topic_names_and_types_by_node(
      identifier, node, allocator, node_name, node_namespace, _demangle_service_request_from_topic,
      _demangle_service_type_only, false, __get_reader_names_and_types_by_node,
      service_names_and_types);
}

/**
 * @brief 获取指定节点的客户端名称和类型 (Get client names and types for a specific node)
 *
 * @param[in] identifier 节点标识符 (Node identifier)
 * @param[in] node 指向 rmw_node_t 结构体的指针 (Pointer to the rmw_node_t structure)
 * @param[in] allocator 分配器，用于分配内存空间 (Allocator for allocating memory space)
 * @param[in] node_name 节点名称 (Node name)
 * @param[in] node_namespace 节点命名空间 (Node namespace)
 * @param[out] service_names_and_types 服务名称和类型的结构体指针 (Pointer to the structure of
 * service names and types)
 * @return rmw_ret_t 返回状态 (Return status)
 */
rmw_ret_t __rmw_get_client_names_and_types_by_node(
    const char *identifier,
    const rmw_node_t *node,
    rcutils_allocator_t *allocator,
    const char *node_name,
    const char *node_namespace,
    rmw_names_and_types_t *service_names_and_types) {
  // 调用通用函数获取主题名称和类型 (Call the generic function to get topic names and types)
  return __rmw_get_topic_names_and_types_by_node(
      identifier, node, allocator, node_name, node_namespace, _demangle_service_reply_from_topic,
      _demangle_service_type_only, false, __get_reader_names_and_types_by_node,
      service_names_and_types);
}

}  // namespace rmw_fastrtps_shared_cpp
