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

#include "rmw/allocators.h"
#include "rmw/convert_rcutils_ret_to_rmw_ret.h"
#include "rmw/error_handling.h"
#include "rmw/get_node_info_and_types.h"
#include "rmw/names_and_types.h"
#include "rmw/rmw.h"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

// The extern "C" here enforces that overloading is not used.
extern "C" {

/**
 * @brief 获取指定节点的订阅者名称和类型 (Get the subscriber names and types of a specified node)
 *
 * @param[in] node 指向要查询的节点的指针 (Pointer to the node to query)
 * @param[in] allocator 用于分配内存的分配器 (Allocator for memory allocation)
 * @param[in] node_name 要查询的节点名称 (Name of the node to query)
 * @param[in] node_namespace 要查询的节点命名空间 (Namespace of the node to query)
 * @param[in] no_demangle 是否取消修饰 (Whether to demangle or not)
 * @param[out] topic_names_and_types 存储获取到的订阅者名称和类型的结构体 (Structure to store the
 * obtained subscriber names and types)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_get_subscriber_names_and_types_by_node(
    const rmw_node_t* node,
    rcutils_allocator_t* allocator,
    const char* node_name,
    const char* node_namespace,
    bool no_demangle,
    rmw_names_and_types_t* topic_names_and_types) {
  // 调用共享实现并传递 FastRTPS 标识符 (Call shared implementation with FastRTPS identifier)
  return rmw_fastrtps_shared_cpp::__rmw_get_subscriber_names_and_types_by_node(
      eprosima_fastrtps_identifier, node, allocator, node_name, node_namespace, no_demangle,
      topic_names_and_types);
}

/**
 * @brief 获取指定节点的发布者名称和类型 (Get the publisher names and types of a specified node)
 *
 * @param[in] node 指向要查询的节点的指针 (Pointer to the node to query)
 * @param[in] allocator 用于分配内存的分配器 (Allocator for memory allocation)
 * @param[in] node_name 要查询的节点名称 (Name of the node to query)
 * @param[in] node_namespace 要查询的节点命名空间 (Namespace of the node to query)
 * @param[in] no_demangle 是否取消修饰 (Whether to demangle or not)
 * @param[out] topic_names_and_types 存储获取到的发布者名称和类型的结构体 (Structure to store the
 * obtained publisher names and types)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_get_publisher_names_and_types_by_node(
    const rmw_node_t* node,
    rcutils_allocator_t* allocator,
    const char* node_name,
    const char* node_namespace,
    bool no_demangle,
    rmw_names_and_types_t* topic_names_and_types) {
  // 调用共享实现并传递 FastRTPS 标识符 (Call shared implementation with FastRTPS identifier)
  return rmw_fastrtps_shared_cpp::__rmw_get_publisher_names_and_types_by_node(
      eprosima_fastrtps_identifier, node, allocator, node_name, node_namespace, no_demangle,
      topic_names_and_types);
}

/**
 * @brief 获取指定节点的服务名称和类型 (Get the service names and types of a specified node)
 *
 * @param[in] node 指向要查询的节点的指针 (Pointer to the node to query)
 * @param[in] allocator 用于分配内存的分配器 (Allocator for memory allocation)
 * @param[in] node_name 要查询的节点名称 (Name of the node to query)
 * @param[in] node_namespace 要查询的节点命名空间 (Namespace of the node to query)
 * @param[out] service_names_and_types 存储获取到的服务名称和类型的结构体 (Structure to store the
 * obtained service names and types)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_get_service_names_and_types_by_node(
    const rmw_node_t* node,
    rcutils_allocator_t* allocator,
    const char* node_name,
    const char* node_namespace,
    rmw_names_and_types_t* service_names_and_types) {
  // 调用共享实现并传递 FastRTPS 标识符 (Call shared implementation with FastRTPS identifier)
  return rmw_fastrtps_shared_cpp::__rmw_get_service_names_and_types_by_node(
      eprosima_fastrtps_identifier, node, allocator, node_name, node_namespace,
      service_names_and_types);
}

/**
 * @brief 获取指定节点的客户端名称和类型 (Get the client names and types of a specified node)
 *
 * @param[in] node 指向要查询的节点的指针 (Pointer to the node to query)
 * @param[in] allocator 用于分配内存的分配器 (Allocator for memory allocation)
 * @param[in] node_name 要查询的节点名称 (Name of the node to query)
 * @param[in] node_namespace 要查询的节点命名空间 (Namespace of the node to query)
 * @param[out] service_names_and_types 存储获取到的客户端名称和类型的结构体 (Structure to store the
 * obtained client names and types)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_get_client_names_and_types_by_node(
    const rmw_node_t* node,
    rcutils_allocator_t* allocator,
    const char* node_name,
    const char* node_namespace,
    rmw_names_and_types_t* service_names_and_types) {
  // 调用共享实现并传递 FastRTPS 标识符 (Call shared implementation with FastRTPS identifier)
  return rmw_fastrtps_shared_cpp::__rmw_get_client_names_and_types_by_node(
      eprosima_fastrtps_identifier, node, allocator, node_name, node_namespace,
      service_names_and_types);
}

}  // extern "C"
