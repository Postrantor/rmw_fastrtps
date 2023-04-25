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

#ifndef RMW_FASTRTPS_SHARED_CPP__RMW_COMMON_HPP_
#define RMW_FASTRTPS_SHARED_CPP__RMW_COMMON_HPP_

#include "./visibility_control.h"
#include "rmw/error_handling.h"
#include "rmw/event.h"
#include "rmw/features.h"
#include "rmw/names_and_types.h"
#include "rmw/network_flow_endpoint_array.h"
#include "rmw/rmw.h"
#include "rmw/topic_endpoint_info_array.h"
#include "rmw/types.h"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 销毁客户端 (Destroy a client)
 *
 * @param[in] identifier 指定的标识符 (The specified identifier)
 * @param[in,out] node 要销毁的节点 (The node to be destroyed)
 * @param[in,out] client 要销毁的客户端 (The client to be destroyed)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_destroy_client(const char* identifier, rmw_node_t* node, rmw_client_t* client);

/**
 * @brief 比较两个 GID 是否相等 (Compare whether two GIDs are equal)
 *
 * @param[in] identifier 指定的标识符 (The specified identifier)
 * @param[in] gid1 第一个 GID (The first GID)
 * @param[in] gid2 第二个 GID (The second GID)
 * @param[out] result 比较结果，如果相等则为 true，否则为 false (Comparison result, true if equal,
 * otherwise false)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_compare_gids_equal(
    const char* identifier, const rmw_gid_t* gid1, const rmw_gid_t* gid2, bool* result);

/**
 * @brief 计算给定主题的发布者数量 (Count the number of publishers for a given topic)
 *
 * @param[in] identifier 指定的标识符 (The specified identifier)
 * @param[in] node 当前节点 (The current node)
 * @param[in] topic_name 主题名称 (Topic name)
 * @param[out] count 发布者数量 (Number of publishers)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_count_publishers(
    const char* identifier, const rmw_node_t* node, const char* topic_name, size_t* count);

/**
 * @brief 计算给定主题的订阅者数量 (Count the number of subscribers for a given topic)
 *
 * @param[in] identifier 指定的标识符 (The specified identifier)
 * @param[in] node 当前节点 (The current node)
 * @param[in] topic_name 主题名称 (Topic name)
 * @param[out] count 订阅者数量 (Number of subscribers)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_count_subscribers(
    const char* identifier, const rmw_node_t* node, const char* topic_name, size_t* count);

/**
 * @brief 获取发布者的 GID (Get the GID for a publisher)
 *
 * @param[in] identifier 指定的标识符 (The specified identifier)
 * @param[in] publisher 要获取 GID 的发布者 (The publisher to get the GID for)
 * @param[out] gid 发布者的 GID (The GID of the publisher)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_get_gid_for_publisher(
    const char* identifier, const rmw_publisher_t* publisher, rmw_gid_t* gid);

/**
 * @brief 获取客户端的 GID (Get the GID for a client)
 *
 * @param[in] identifier 指定的标识符 (The specified identifier)
 * @param[in] client 要获取 GID 的客户端 (The client to get the GID for)
 * @param[out] gid 客户端的 GID (The GID of the client)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_get_gid_for_client(
    const char* identifier, const rmw_client_t* client, rmw_gid_t* gid);

/**
 * @brief 创建一个守护条件 (Create a guard condition)
 *
 * @param[in] identifier 指定的标识符 (The specified identifier)
 * @return rmw_guard_condition_t* 返回创建的守护条件 (Return the created guard condition)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_guard_condition_t* __rmw_create_guard_condition(const char* identifier);

/**
 * @brief 销毁守护条件 (Destroy a guard condition)
 *
 * @param[in,out] guard_condition 要销毁的守护条件 (The guard condition to be destroyed)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_destroy_guard_condition(rmw_guard_condition_t* guard_condition);

/**
 * @brief 触发守护条件 (Trigger a guard condition)
 *
 * @param[in] identifier 指定的标识符 (The specified identifier)
 * @param[in] guard_condition_handle 要触发的守护条件句柄 (The guard condition handle to be
 * triggered)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_trigger_guard_condition(
    const char* identifier, const rmw_guard_condition_t* guard_condition_handle);

/**
 * @brief 设置日志严重程度 (Set log severity)
 *
 * @param[in] severity 日志严重程度 (Log severity)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_set_log_severity(rmw_log_severity_t severity);

/**
 * @brief 创建一个节点 (Create a node)
 *
 * @param[in,out] context 当前上下文 (Current context)
 * @param[in] identifier 指定的标识符 (The specified identifier)
 * @param[in] name 节点名称 (Node name)
 * @param[in] namespace_ 命名空间 (Namespace)
 * @return rmw_node_t* 返回创建的节点 (Return the created node)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_node_t* __rmw_create_node(
    rmw_context_t* context, const char* identifier, const char* name, const char* namespace_);

/**
 * @brief 销毁节点 (Destroy a node)
 *
 * @param[in] identifier 节点标识符 (Node identifier)
 * @param[in,out] node 要销毁的节点指针 (Pointer to the node to be destroyed)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_destroy_node(const char* identifier, rmw_node_t* node);

/**
 * @brief 获取节点的图形保护条件 (Get the graph guard condition of a node)
 *
 * @param[in] node 节点指针 (Pointer to the node)
 * @return const rmw_guard_condition_t* 返回节点的图形保护条件 (Return the graph guard condition of
 * the node)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
const rmw_guard_condition_t* __rmw_node_get_graph_guard_condition(const rmw_node_t* node);

/**
 * @brief 获取节点名称 (Get node names)
 *
 * @param[in] identifier 节点标识符 (Node identifier)
 * @param[in] node 节点指针 (Pointer to the node)
 * @param[out] node_names 节点名称数组 (Array of node names)
 * @param[out] node_namespaces 节点命名空间数组 (Array of node namespaces)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_get_node_names(
    const char* identifier,
    const rmw_node_t* node,
    rcutils_string_array_t* node_names,
    rcutils_string_array_t* node_namespaces);

/**
 * @brief 初始化事件 (Initialize an event)
 *
 * @param[in] identifier 事件标识符 (Event identifier)
 * @param[out] rmw_event 初始化后的事件指针 (Pointer to the initialized event)
 * @param[in] topic_endpoint_impl_identifier 主题端点实现标识符 (Topic endpoint implementation
 * identifier)
 * @param[in] data 事件相关数据 (Event related data)
 * @param[in] event_type 事件类型 (Event type)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_init_event(
    const char* identifier,
    rmw_event_t* rmw_event,
    const char* topic_endpoint_impl_identifier,
    void* data,
    rmw_event_type_t event_type);

/**
 * @brief 获取节点名称和隔离区 (Get node names and enclaves)
 *
 * @param[in] identifier 节点标识符 (Node identifier)
 * @param[in] node 节点指针 (Pointer to the node)
 * @param[out] node_names 节点名称数组 (Array of node names)
 * @param[out] node_namespaces 节点命名空间数组 (Array of node namespaces)
 * @param[out] enclaves 隔离区数组 (Array of enclaves)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_get_node_names_with_enclaves(
    const char* identifier,
    const rmw_node_t* node,
    rcutils_string_array_t* node_names,
    rcutils_string_array_t* node_namespaces,
    rcutils_string_array_t* enclaves);

/**
 * @brief 发布消息 (Publish a message)
 *
 * @param[in] identifier 发布者标识符 (Publisher identifier)
 * @param[in] publisher 发布者指针 (Pointer to the publisher)
 * @param[in] ros_message ROS 消息 (ROS message)
 * @param[in] allocation 发布者分配 (Publisher allocation)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_publish(
    const char* identifier,
    const rmw_publisher_t* publisher,
    const void* ros_message,
    rmw_publisher_allocation_t* allocation);

/**
 * @brief 发布序列化消息 (Publish a serialized message)
 *
 * @param[in] identifier 发布者标识符 (Publisher identifier)
 * @param[in] publisher 发布者指针 (Pointer to the publisher)
 * @param[in] serialized_message 序列化消息 (Serialized message)
 * @param[in] allocation 发布者分配 (Publisher allocation)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_publish_serialized_message(
    const char* identifier,
    const rmw_publisher_t* publisher,
    const rmw_serialized_message_t* serialized_message,
    rmw_publisher_allocation_t* allocation);

/**
 * @brief 借用一条消息 (Borrow a loaned message)
 *
 * @param[in] identifier 指示符 (Identifier)
 * @param[in] publisher 发布者对象指针 (Pointer to the publisher object)
 * @param[in] type_support 消息类型支持 (Message type support)
 * @param[out] ros_message 借用的 ROS 消息指针 (Pointer to the borrowed ROS message)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_borrow_loaned_message(
    const char* identifier,
    const rmw_publisher_t* publisher,
    const rosidl_message_type_support_t* type_support,
    void** ros_message);

/**
 * @brief 归还借用的消息 (Return a loaned message from publisher)
 *
 * @param[in] identifier 指示符 (Identifier)
 * @param[in] publisher 发布者对象指针 (Pointer to the publisher object)
 * @param[in] loaned_message 借用的消息指针 (Pointer to the loaned message)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_return_loaned_message_from_publisher(
    const char* identifier, const rmw_publisher_t* publisher, void* loaned_message);

/**
 * @brief 发布借用的消息 (Publish a loaned message)
 *
 * @param[in] identifier 指示符 (Identifier)
 * @param[in] publisher 发布者对象指针 (Pointer to the publisher object)
 * @param[in] ros_message ROS 消息指针 (Pointer to the ROS message)
 * @param[in] allocation 发布者分配对象 (Publisher allocation object)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_publish_loaned_message(
    const char* identifier,
    const rmw_publisher_t* publisher,
    const void* ros_message,
    rmw_publisher_allocation_t* allocation);

/**
 * @brief 断言发布者的活跃状态 (Assert liveliness of a publisher)
 *
 * @param[in] identifier 指示符 (Identifier)
 * @param[in] publisher 发布者对象指针 (Pointer to the publisher object)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_publisher_assert_liveliness(
    const char* identifier, const rmw_publisher_t* publisher);

/**
 * @brief 等待所有已发布的消息被确认 (Wait for all published messages to be acknowledged)
 *
 * @param[in] identifier 指示符 (Identifier)
 * @param[in] publisher 发布者对象指针 (Pointer to the publisher object)
 * @param[in] wait_timeout 等待超时时间 (Wait timeout duration)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_publisher_wait_for_all_acked(
    const char* identifier, const rmw_publisher_t* publisher, rmw_time_t wait_timeout);

/**
 * @brief 销毁发布者 (Destroy a publisher)
 *
 * @param[in] identifier 指示符 (Identifier)
 * @param[in] node 节点对象指针 (Pointer to the node object)
 * @param[in] publisher 发布者对象指针 (Pointer to the publisher object)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_destroy_publisher(
    const char* identifier, const rmw_node_t* node, rmw_publisher_t* publisher);

/**
 * @brief 计算匹配的订阅数量 (Count matched subscriptions for a publisher)
 *
 * @param[in] publisher 发布者对象指针 (Pointer to the publisher object)
 * @param[out] subscription_count 匹配的订阅数量 (Matched subscription count)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_publisher_count_matched_subscriptions(
    const rmw_publisher_t* publisher, size_t* subscription_count);

/**
 * @brief 获取发布者的实际 QoS 配置 (Get actual QoS settings of a publisher)
 *
 * @param[in] publisher 发布者对象指针 (Pointer to the publisher object)
 * @param[out] qos QoS 配置对象 (QoS configuration object)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_publisher_get_actual_qos(const rmw_publisher_t* publisher, rmw_qos_profile_t* qos);

/**
 * @brief 发送请求 (Send a request)
 *
 * @param[in] identifier 指示符 (Identifier)
 * @param[in] client 客户端对象指针 (Pointer to the client object)
 * @param[in] ros_request ROS 请求消息指针 (Pointer to the ROS request message)
 * @param[out] sequence_id 请求序列号 (Request sequence number)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_send_request(
    const char* identifier,
    const rmw_client_t* client,
    const void* ros_request,
    int64_t* sequence_id);

/**
 * @brief 接收请求 (Take a request)
 *
 * @param[in] identifier 指示符 (Identifier)
 * @param[in] service 服务对象指针 (Pointer to the service object)
 * @param[out] request_header 请求头信息 (Request header information)
 * @param[out] ros_request ROS 请求消息指针 (Pointer to the ROS request message)
 * @param[out] taken 是否成功接收到请求 (Whether a request was successfully taken)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_take_request(
    const char* identifier,
    const rmw_service_t* service,
    rmw_service_info_t* request_header,
    void* ros_request,
    bool* taken);

/**
 * @brief 接收服务端的响应 (Take a response from the server)
 *
 * @param[in] identifier 用于标识 rmw 实现的唯一字符串 (Unique string to identify the rmw
 * implementation)
 * @param[in] client 指向客户端实例的指针 (Pointer to the client instance)
 * @param[out] request_header 包含请求头信息的结构体 (Structure containing request header
 * information)
 * @param[out] ros_response 存储接收到的 ROS 响应的缓冲区 (Buffer to store the received ROS
 * response)
 * @param[out] taken 是否成功接收到响应的标志 (Flag indicating whether a response was successfully
 * taken)
 * @return rmw_ret_t RMW 返回值 (RMW return value)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_take_response(
    const char* identifier,
    const rmw_client_t* client,
    rmw_service_info_t* request_header,
    void* ros_response,
    bool* taken);

/**
 * @brief 发送服务端的响应 (Send a response from the server)
 *
 * @param[in] identifier 用于标识 rmw 实现的唯一字符串 (Unique string to identify the rmw
 * implementation)
 * @param[in] service 指向服务实例的指针 (Pointer to the service instance)
 * @param[in] request_header 包含请求头信息的结构体 (Structure containing request header
 * information)
 * @param[in] ros_response 要发送的 ROS 响应 (The ROS response to send)
 * @return rmw_ret_t RMW 返回值 (RMW return value)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_send_response(
    const char* identifier,
    const rmw_service_t* service,
    rmw_request_id_t* request_header,
    void* ros_response);

/**
 * @brief 销毁服务实例 (Destroy a service instance)
 *
 * @param[in] identifier 用于标识 rmw 实现的唯一字符串 (Unique string to identify the rmw
 * implementation)
 * @param[in] node 指向节点实例的指针 (Pointer to the node instance)
 * @param[in] service 要销毁的服务实例指针 (Pointer to the service instance to destroy)
 * @return rmw_ret_t RMW 返回值 (RMW return value)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_destroy_service(const char* identifier, rmw_node_t* node, rmw_service_t* service);

/**
 * @brief 获取服务名称和类型 (Get service names and types)
 *
 * @param[in] identifier 用于标识 rmw 实现的唯一字符串 (Unique string to identify the rmw
 * implementation)
 * @param[in] node 指向节点实例的指针 (Pointer to the node instance)
 * @param[in] allocator 分配器实例 (Allocator instance)
 * @param[out] service_names_and_types 存储服务名称和类型的结构体 (Structure to store service names
 * and types)
 * @return rmw_ret_t RMW 返回值 (RMW return value)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_get_service_names_and_types(
    const char* identifier,
    const rmw_node_t* node,
    rcutils_allocator_t* allocator,
    rmw_names_and_types_t* service_names_and_types);

/**
 * @brief 根据节点获取发布者名称和类型 (Get publisher names and types by node)
 *
 * @param[in] identifier 用于标识 rmw 实现的唯一字符串 (Unique string to identify the rmw
 * implementation)
 * @param[in] node 指向节点实例的指针 (Pointer to the node instance)
 * @param[in] allocator 分配器实例 (Allocator instance)
 * @param[in] node_name 节点名称 (Node name)
 * @param[in] node_namespace 节点命名空间 (Node namespace)
 * @param[in] no_demangle 是否取消混淆 (Whether to demangle or not)
 * @param[out] topic_names_and_types 存储主题名称和类型的结构体 (Structure to store topic names and
 * types)
 * @return rmw_ret_t RMW 返回值 (RMW return value)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_get_publisher_names_and_types_by_node(
    const char* identifier,
    const rmw_node_t* node,
    rcutils_allocator_t* allocator,
    const char* node_name,
    const char* node_namespace,
    bool no_demangle,
    rmw_names_and_types_t* topic_names_and_types);

/**
 * @brief 根据节点获取服务名称和类型 (Get service names and types by node)
 *
 * @param[in] identifier 用于标识 rmw 实现的唯一字符串 (Unique string to identify the rmw
 * implementation)
 * @param[in] node 指向节点实例的指针 (Pointer to the node instance)
 * @param[in] allocator 分配器实例 (Allocator instance)
 * @param[in] node_name 节点名称 (Node name)
 * @param[in] node_namespace 节点命名空间 (Node namespace)
 * @param[out] service_names_and_types 存储服务名称和类型的结构体 (Structure to store service names
 * and types)
 * @return rmw_ret_t RMW 返回值 (RMW return value)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_get_service_names_and_types_by_node(
    const char* identifier,
    const rmw_node_t* node,
    rcutils_allocator_t* allocator,
    const char* node_name,
    const char* node_namespace,
    rmw_names_and_types_t* service_names_and_types);

/**
 * @brief 获取指定节点上的客户端名称和类型 (Get client names and types by node)
 *
 * @param[in] identifier 用于标识 ROS2 实现的唯一字符串 (Unique string to identify the ROS2
 * implementation)
 * @param[in] node 指向要查询的节点的指针 (Pointer to the node to query)
 * @param[in,out] allocator 用于分配结果数据结构的内存分配器 (Allocator for allocating memory for
 * the result data structures)
 * @param[in] node_name 要查询的节点的名称 (Name of the node to query)
 * @param[in] node_namespace 要查询的节点的命名空间 (Namespace of the node to query)
 * @param[out] service_names_and_types 存储查询结果的结构 (Structure to store the query results)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_get_client_names_and_types_by_node(
    const char* identifier,
    const rmw_node_t* node,
    rcutils_allocator_t* allocator,
    const char* node_name,
    const char* node_namespace,
    rmw_names_and_types_t* service_names_and_types);

/**
 * @brief 获取指定节点上的订阅者名称和类型 (Get subscriber names and types by node)
 *
 * @param[in] identifier 用于标识 ROS2 实现的唯一字符串 (Unique string to identify the ROS2
 * implementation)
 * @param[in] node 指向要查询的节点的指针 (Pointer to the node to query)
 * @param[in,out] allocator 用于分配结果数据结构的内存分配器 (Allocator for allocating memory for
 * the result data structures)
 * @param[in] node_name 要查询的节点的名称 (Name of the node to query)
 * @param[in] node_namespace 要查询的节点的命名空间 (Namespace of the node to query)
 * @param[in] no_demangle 如果为 true，则不对查询结果进行解析 (If true, do not demangle the query
 * results)
 * @param[out] topic_names_and_types 存储查询结果的结构 (Structure to store the query results)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_get_subscriber_names_and_types_by_node(
    const char* identifier,
    const rmw_node_t* node,
    rcutils_allocator_t* allocator,
    const char* node_name,
    const char* node_namespace,
    bool no_demangle,
    rmw_names_and_types_t* topic_names_and_types);

/**
 * @brief 检查服务服务器是否可用 (Check if service server is available)
 *
 * @param[in] identifier 用于标识 ROS2 实现的唯一字符串 (Unique string to identify the ROS2
 * implementation)
 * @param[in] node 指向要查询的节点的指针 (Pointer to the node to query)
 * @param[in] client 指向要检查的客户端的指针 (Pointer to the client to check)
 * @param[out] is_available 存储服务服务器是否可用的布尔值 (Boolean value to store whether the
 * service server is available or not)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_service_server_is_available(
    const char* identifier, const rmw_node_t* node, const rmw_client_t* client, bool* is_available);

/**
 * @brief 销毁订阅 (Destroy subscription)
 *
 * @param[in] identifier 用于标识 ROS2 实现的唯一字符串 (Unique string to identify the ROS2
 * implementation)
 * @param[in] node 指向要销毁订阅的节点的指针 (Pointer to the node where the subscription will be
 * destroyed)
 * @param[in,out] subscription 要销毁的订阅指针 (Pointer to the subscription to destroy)
 * @param[in] reset_cft 是否重置内容过滤器主题 (Whether to reset the content filter topic)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_destroy_subscription(
    const char* identifier,
    const rmw_node_t* node,
    rmw_subscription_t* subscription,
    bool reset_cft = false);

/**
 * @brief 计算匹配发布者的数量 (Count matched publishers)
 *
 * @param[in] subscription 指向要查询的订阅的指针 (Pointer to the subscription to query)
 * @param[out] publisher_count 存储匹配发布者数量的变量 (Variable to store the count of matched
 * publishers)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_subscription_count_matched_publishers(
    const rmw_subscription_t* subscription, size_t* publisher_count);

/**
 * @brief 获取订阅的实际 QoS (Get actual QoS of the subscription)
 *
 * @param[in] subscription 指向要查询的订阅的指针 (Pointer to the subscription to query)
 * @param[out] qos 存储订阅的实际 QoS 的变量 (Variable to store the actual QoS of the subscription)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_subscription_get_actual_qos(
    const rmw_subscription_t* subscription, rmw_qos_profile_t* qos);

/**
 * @brief 设置订阅的内容过滤器 (Set content filter for the subscription)
 *
 * @param[in,out] subscription 要设置内容过滤器的订阅指针 (Pointer to the subscription to set the
 * content filter)
 * @param[in] options 内容过滤器选项 (Content filter options)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_subscription_set_content_filter(
    rmw_subscription_t* subscription, const rmw_subscription_content_filter_options_t* options);

/**
 * @brief 获取订阅的内容过滤器 (Get content filter of the subscription)
 *
 * @param[in] subscription 指向要查询的订阅的指针 (Pointer to the subscription to query)
 * @param[in,out] allocator 用于分配结果数据结构的内存分配器 (Allocator for allocating memory for
 * the result data structures)
 * @param[out] options 存储内容过滤器选项的变量 (Variable to store the content filter options)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_subscription_get_content_filter(
    const rmw_subscription_t* subscription,
    rcutils_allocator_t* allocator,
    rmw_subscription_content_filter_options_t* options);

/**
 * @brief 获取服务响应发布者的实际QoS配置 (Get the actual QoS settings of the service response
 * publisher)
 *
 * @param[in] service 服务对象指针 (Pointer to the service object)
 * @param[out] qos 返回的QoS配置 (Returned QoS settings)
 * @return rmw_ret_t 操作结果 (Operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_service_response_publisher_get_actual_qos(
    const rmw_service_t* service, rmw_qos_profile_t* qos);

/**
 * @brief 获取服务请求订阅者的实际QoS配置 (Get the actual QoS settings of the service request
 * subscriber)
 *
 * @param[in] service 服务对象指针 (Pointer to the service object)
 * @param[out] qos 返回的QoS配置 (Returned QoS settings)
 * @return rmw_ret_t 操作结果 (Operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_service_request_subscription_get_actual_qos(
    const rmw_service_t* service, rmw_qos_profile_t* qos);

/**
 * @brief 获取客户端请求发布者的实际QoS配置 (Get the actual QoS settings of the client request
 * publisher)
 *
 * @param[in] client 客户端对象指针 (Pointer to the client object)
 * @param[out] qos 返回的QoS配置 (Returned QoS settings)
 * @return rmw_ret_t 操作结果 (Operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_client_request_publisher_get_actual_qos(
    const rmw_client_t* client, rmw_qos_profile_t* qos);

/**
 * @brief 获取客户端响应订阅者的实际QoS配置 (Get the actual QoS settings of the client response
 * subscriber)
 *
 * @param[in] client 客户端对象指针 (Pointer to the client object)
 * @param[out] qos 返回的QoS配置 (Returned QoS settings)
 * @return rmw_ret_t 操作结果 (Operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_client_response_subscription_get_actual_qos(
    const rmw_client_t* client, rmw_qos_profile_t* qos);

/**
 * @brief 从订阅者中获取一条消息 (Take a message from the subscriber)
 *
 * @param[in] identifier 标识符 (Identifier)
 * @param[in] subscription 订阅者对象指针 (Pointer to the subscription object)
 * @param[out] ros_message 存储返回消息的指针 (Pointer to store the returned message)
 * @param[out] taken 是否成功获取到消息 (Whether the message was successfully taken)
 * @param[in] allocation 分配器 (Allocator)
 * @return rmw_ret_t 操作结果 (Operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_take(
    const char* identifier,
    const rmw_subscription_t* subscription,
    void* ros_message,
    bool* taken,
    rmw_subscription_allocation_t* allocation);

/**
 * @brief 从订阅者中获取一系列消息 (Take a sequence of messages from the subscriber)
 *
 * @param[in] identifier 标识符 (Identifier)
 * @param[in] subscription 订阅者对象指针 (Pointer to the subscription object)
 * @param[in] count 要获取的消息数量 (Number of messages to take)
 * @param[out] message_sequence 存储返回消息序列的指针 (Pointer to store the returned message
 * sequence)
 * @param[out] message_info_sequence 存储返回消息信息序列的指针 (Pointer to store the returned
 * message info sequence)
 * @param[out] taken 实际获取到的消息数量 (Actual number of messages taken)
 * @param[in] allocation 分配器 (Allocator)
 * @return rmw_ret_t 操作结果 (Operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_take_sequence(
    const char* identifier,
    const rmw_subscription_t* subscription,
    size_t count,
    rmw_message_sequence_t* message_sequence,
    rmw_message_info_sequence_t* message_info_sequence,
    size_t* taken,
    rmw_subscription_allocation_t* allocation);

/**
 * @brief 从订阅者中获取一个借用的消息 (Take a loaned message from the subscriber)
 *
 * @param[in] identifier 标识符 (Identifier)
 * @param[in] subscription 订阅者对象指针 (Pointer to the subscription object)
 * @param[out] loaned_message 存储返回借用消息的指针 (Pointer to store the returned loaned message)
 * @param[out] taken 是否成功获取到借用消息 (Whether the loaned message was successfully taken)
 * @param[out] message_info 存储返回消息信息的指针 (Pointer to store the returned message info)
 * @return rmw_ret_t 操作结果 (Operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_take_loaned_message_internal(
    const char* identifier,
    const rmw_subscription_t* subscription,
    void** loaned_message,
    bool* taken,
    rmw_message_info_t* message_info);

/**
 * @brief 将借用的消息返回给订阅者 (Return the loaned message to the subscriber)
 *
 * @param[in] identifier 标识符 (Identifier)
 * @param[in] subscription 订阅者对象指针 (Pointer to the subscription object)
 * @param[in] loaned_message 要返回的借用消息指针 (Pointer to the loaned message to return)
 * @return rmw_ret_t 操作结果 (Operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_return_loaned_message_from_subscription(
    const char* identifier, const rmw_subscription_t* subscription, void* loaned_message);

/**
 * @brief 从事件句柄中获取一个事件 (Take an event from the event handle)
 *
 * @param[in] identifier 标识符 (Identifier)
 * @param[in] event_handle 事件句柄 (Event handle)
 * @param[out] event_info 存储返回事件信息的指针 (Pointer to store the returned event info)
 * @param[out] taken 是否成功获取到事件 (Whether the event was successfully taken)
 * @return rmw_ret_t 操作结果 (Operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_take_event(
    const char* identifier, const rmw_event_t* event_handle, void* event_info, bool* taken);

/**
 * @brief 从订阅中获取消息并附带信息 (Take a message with info from the subscription)
 *
 * @param[in] identifier 标识符，用于区分不同的 rmw 实现 (Identifier to distinguish different rmw
 * implementations)
 * @param[in] subscription 订阅对象指针 (Pointer to the subscription object)
 * @param[out] ros_message 存储接收到的 ROS 消息的指针 (Pointer to store the received ROS message)
 * @param[out] taken 是否成功接收到消息的标志 (Flag indicating if a message was successfully taken)
 * @param[out] message_info 接收到的消息的附加信息 (Additional information about the received
 * message)
 * @param[in] allocation 预留的订阅分配参数 (Reserved subscription allocation parameter)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_take_with_info(
    const char* identifier,
    const rmw_subscription_t* subscription,
    void* ros_message,
    bool* taken,
    rmw_message_info_t* message_info,
    rmw_subscription_allocation_t* allocation);

/**
 * @brief 从订阅中获取序列化消息 (Take a serialized message from the subscription)
 *
 * @param[in] identifier 标识符，用于区分不同的 rmw 实现 (Identifier to distinguish different rmw
 * implementations)
 * @param[in] subscription 订阅对象指针 (Pointer to the subscription object)
 * @param[out] serialized_message 存储接收到的序列化消息的指针 (Pointer to store the received
 * serialized message)
 * @param[out] taken 是否成功接收到消息的标志 (Flag indicating if a message was successfully taken)
 * @param[in] allocation 预留的订阅分配参数 (Reserved subscription allocation parameter)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_take_serialized_message(
    const char* identifier,
    const rmw_subscription_t* subscription,
    rmw_serialized_message_t* serialized_message,
    bool* taken,
    rmw_subscription_allocation_t* allocation);

/**
 * @brief 从订阅中获取序列化消息并附带信息 (Take a serialized message with info from the
 * subscription)
 *
 * @param[in] identifier 标识符，用于区分不同的 rmw 实现 (Identifier to distinguish different rmw
 * implementations)
 * @param[in] subscription 订阅对象指针 (Pointer to the subscription object)
 * @param[out] serialized_message 存储接收到的序列化消息的指针 (Pointer to store the received
 * serialized message)
 * @param[out] taken 是否成功接收到消息的标志 (Flag indicating if a message was successfully taken)
 * @param[out] message_info 接收到的消息的附加信息 (Additional information about the received
 * message)
 * @param[in] allocation 预留的订阅分配参数 (Reserved subscription allocation parameter)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_take_serialized_message_with_info(
    const char* identifier,
    const rmw_subscription_t* subscription,
    rmw_serialized_message_t* serialized_message,
    bool* taken,
    rmw_message_info_t* message_info,
    rmw_subscription_allocation_t* allocation);

/**
 * @brief 从订阅中获取动态消息 (Take a dynamic message from the subscription)
 *
 * @param[in] identifier 标识符，用于区分不同的 rmw 实现 (Identifier to distinguish different rmw
 * implementations)
 * @param[in] subscription 订阅对象指针 (Pointer to the subscription object)
 * @param[out] dynamic_data 存储接收到的动态消息数据的指针 (Pointer to store the received dynamic
 * message data)
 * @param[out] taken 是否成功接收到消息的标志 (Flag indicating if a message was successfully taken)
 * @param[in] allocation 预留的订阅分配参数 (Reserved subscription allocation parameter)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_take_dynamic_message(
    const char* identifier,
    const rmw_subscription_t* subscription,
    rosidl_dynamic_typesupport_dynamic_data_t* dynamic_data,
    bool* taken,
    rmw_subscription_allocation_t* allocation);

/**
 * @brief 从订阅中获取动态消息并附带信息 (Take a dynamic message with info from the subscription)
 *
 * @param[in] identifier 标识符，用于区分不同的 rmw 实现 (Identifier to distinguish different rmw
 * implementations)
 * @param[in] subscription 订阅对象指针 (Pointer to the subscription object)
 * @param[out] dynamic_data 存储接收到的动态消息数据的指针 (Pointer to store the received dynamic
 * message data)
 * @param[out] taken 是否成功接收到消息的标志 (Flag indicating if a message was successfully taken)
 * @param[out] message_info 接收到的消息的附加信息 (Additional information about the received
 * message)
 * @param[in] allocation 预留的订阅分配参数 (Reserved subscription allocation parameter)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_take_dynamic_message_with_info(
    const char* identifier,
    const rmw_subscription_t* subscription,
    rosidl_dynamic_typesupport_dynamic_data_t* dynamic_data,
    bool* taken,
    rmw_message_info_t* message_info,
    rmw_subscription_allocation_t* allocation);

/**
 * @brief 获取节点上的主题名称和类型 (Get topic names and types on the node)
 *
 * @param[in] identifier 标识符，用于区分不同的 rmw 实现 (Identifier to distinguish different rmw
 * implementations)
 * @param[in] node 节点对象指针 (Pointer to the node object)
 * @param[in] allocator 分配器对象指针 (Pointer to the allocator object)
 * @param[in] no_demangle 是否禁用反混淆 (Whether to disable demangling)
 * @param[out] topic_names_and_types 存储获取到的主题名称和类型的指针 (Pointer to store the
 * retrieved topic names and types)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_get_topic_names_and_types(
    const char* identifier,
    const rmw_node_t* node,
    rcutils_allocator_t* allocator,
    bool no_demangle,
    rmw_names_and_types_t* topic_names_and_types);

/**
 * @brief 等待订阅、守卫条件、服务、客户端和事件的变化（Waits for changes in subscriptions, guard
 * conditions, services, clients, and events）
 *
 * @param[in] identifier 用于标识 rmw 实现的字符串（A string to identify the rmw implementation）
 * @param[in,out] subscriptions 订阅列表（List of subscriptions）
 * @param[in,out] guard_conditions 守卫条件列表（List of guard conditions）
 * @param[in,out] services 服务列表（List of services）
 * @param[in,out] clients 客户端列表（List of clients）
 * @param[in,out] events 事件列表（List of events）
 * @param[in,out] wait_set 等待集合（Wait set）
 * @param[in] wait_timeout 等待超时时间（Wait timeout duration）
 * @return rmw_ret_t 返回操作结果（Returns the operation result）
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_wait(
    const char* identifier,
    rmw_subscriptions_t* subscriptions,
    rmw_guard_conditions_t* guard_conditions,
    rmw_services_t* services,
    rmw_clients_t* clients,
    rmw_events_t* events,
    rmw_wait_set_t* wait_set,
    const rmw_time_t* wait_timeout);

/**
 * @brief 创建等待集合（Creates a wait set）
 *
 * @param[in] identifier 用于标识 rmw 实现的字符串（A string to identify the rmw implementation）
 * @param[in] context ROS2 上下文（ROS2 context）
 * @param[in] max_conditions 最大条件数量（Maximum number of conditions）
 * @return rmw_wait_set_t* 返回创建的等待集合指针（Returns a pointer to the created wait set）
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_wait_set_t* __rmw_create_wait_set(
    const char* identifier, rmw_context_t* context, size_t max_conditions);

/**
 * @brief 销毁等待集合（Destroys a wait set）
 *
 * @param[in] identifier 用于标识 rmw 实现的字符串（A string to identify the rmw implementation）
 * @param[in,out] wait_set 要销毁的等待集合（The wait set to be destroyed）
 * @return rmw_ret_t 返回操作结果（Returns the operation result）
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_destroy_wait_set(const char* identifier, rmw_wait_set_t* wait_set);

/**
 * @brief 获取给定主题上的发布者信息（Gets the publisher information for a given topic）
 *
 * @param[in] identifier 用于标识 rmw 实现的字符串（A string to identify the rmw implementation）
 * @param[in] node ROS2 节点（ROS2 node）
 * @param[in] allocator 分配器（Allocator）
 * @param[in] topic_name 主题名称（Topic name）
 * @param[in] no_mangle 是否取消混淆（Whether to demangle or not）
 * @param[out] publishers_info 发布者信息数组（Array of publisher information）
 * @return rmw_ret_t 返回操作结果（Returns the operation result）
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_get_publishers_info_by_topic(
    const char* identifier,
    const rmw_node_t* node,
    rcutils_allocator_t* allocator,
    const char* topic_name,
    bool no_mangle,
    rmw_topic_endpoint_info_array_t* publishers_info);

/**
 * @brief 获取给定主题上的订阅者信息（Gets the subscription information for a given topic）
 *
 * @param[in] identifier 用于标识 rmw 实现的字符串（A string to identify the rmw implementation）
 * @param[in] node ROS2 节点（ROS2 node）
 * @param[in] allocator 分配器（Allocator）
 * @param[in] topic_name 主题名称（Topic name）
 * @param[in] no_mangle 是否取消混淆（Whether to demangle or not）
 * @param[out] subscriptions_info 订阅者信息数组（Array of subscription information）
 * @return rmw_ret_t 返回操作结果（Returns the operation result）
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_get_subscriptions_info_by_topic(
    const char* identifier,
    const rmw_node_t* node,
    rcutils_allocator_t* allocator,
    const char* topic_name,
    bool no_mangle,
    rmw_topic_endpoint_info_array_t* subscriptions_info);

/**
 * @brief 检查发布者和订阅者的 QoS 配置是否兼容（Checks if the QoS profiles of the publisher and
 * subscriber are compatible）
 *
 * @param[in] publisher_profile 发布者 QoS 配置（Publisher QoS profile）
 * @param[in] subscription_profile 订阅者 QoS 配置（Subscriber QoS profile）
 * @param[out] compatibility 兼容性类型（Compatibility type）
 * @param[out] reason 不兼容原因（Incompatibility reason）
 * @param[in] reason_size 原因缓冲区大小（Size of the reason buffer）
 * @return rmw_ret_t 返回操作结果（Returns the operation result）
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_qos_profile_check_compatible(
    const rmw_qos_profile_t publisher_profile,
    const rmw_qos_profile_t subscription_profile,
    rmw_qos_compatibility_type_t* compatibility,
    char* reason,
    size_t reason_size);

/**
 * @brief 获取发布者的网络流端点信息（Gets the network flow endpoint information for a publisher）
 *
 * @param[in] publisher 发布者（Publisher）
 * @param[in] allocator 分配器（Allocator）
 * @param[out] network_flow_endpoint_array 网络流端点信息数组（Array of network flow endpoint
 * information）
 * @return rmw_ret_t 返回操作结果（Returns the operation result）
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_publisher_get_network_flow_endpoints(
    const rmw_publisher_t* publisher,
    rcutils_allocator_t* allocator,
    rmw_network_flow_endpoint_array_t* network_flow_endpoint_array);

/**
 * @brief 获取订阅者的网络流端点 (Get the network flow endpoints of a subscription)
 *
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[in] allocator 分配器指针，用于分配内存 (Pointer to the allocator for memory allocation)
 * @param[out] network_flow_endpoint_array 存储网络流端点信息的数组 (Array to store the network flow
 * endpoint information)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_subscription_get_network_flow_endpoints(
    const rmw_subscription_t* subscription,
    rcutils_allocator_t* allocator,
    rmw_network_flow_endpoint_array_t* network_flow_endpoint_array);

/**
 * @brief 设置订阅者的新消息回调函数 (Set the new message callback function for a subscription)
 *
 * @param[in,out] rmw_subscription 订阅者指针 (Pointer to the subscription)
 * @param[in] callback 新消息回调函数 (New message callback function)
 * @param[in] user_data 用户数据指针，传递给回调函数 (Pointer to user data, passed to the callback
 * function)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_subscription_set_on_new_message_callback(
    rmw_subscription_t* rmw_subscription, rmw_event_callback_t callback, const void* user_data);

/**
 * @brief 设置服务端的新请求回调函数 (Set the new request callback function for a service)
 *
 * @param[in,out] rmw_service 服务端指针 (Pointer to the service)
 * @param[in] callback 新请求回调函数 (New request callback function)
 * @param[in] user_data 用户数据指针，传递给回调函数 (Pointer to user data, passed to the callback
 * function)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_service_set_on_new_request_callback(
    rmw_service_t* rmw_service, rmw_event_callback_t callback, const void* user_data);

/**
 * @brief 设置客户端的新响应回调函数 (Set the new response callback function for a client)
 *
 * @param[in,out] rmw_client 客户端指针 (Pointer to the client)
 * @param[in] callback 新响应回调函数 (New response callback function)
 * @param[in] user_data 用户数据指针，传递给回调函数 (Pointer to user data, passed to the callback
 * function)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_client_set_on_new_response_callback(
    rmw_client_t* rmw_client, rmw_event_callback_t callback, const void* user_data);

/**
 * @brief 设置事件的回调函数 (Set the callback function for an event)
 *
 * @param[in,out] rmw_event 事件指针 (Pointer to the event)
 * @param[in] callback 事件回调函数 (Event callback function)
 * @param[in] user_data 用户数据指针，传递给回调函数 (Pointer to user data, passed to the callback
 * function)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t __rmw_event_set_callback(
    rmw_event_t* rmw_event, rmw_event_callback_t callback, const void* user_data);

/**
 * @brief 检查特定功能是否受支持 (Check if a specific feature is supported)
 *
 * @param[in] feature 要检查的功能 (The feature to check)
 * @return bool 返回该功能是否受支持 (Return whether the feature is supported or not)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
bool __rmw_feature_supported(rmw_feature_t feature);

}  // namespace rmw_fastrtps_shared_cpp

#endif  // RMW_FASTRTPS_SHARED_CPP__RMW_COMMON_HPP_
