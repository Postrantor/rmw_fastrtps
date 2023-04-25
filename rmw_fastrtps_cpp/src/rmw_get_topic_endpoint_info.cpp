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

#include "rmw/get_topic_endpoint_info.h"
#include "rmw/topic_endpoint_info_array.h"
#include "rmw/types.h"
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 获取指定主题的发布者信息 (Get the publisher information for a specific topic)
 *
 * @param[in] node 指向 ROS2 节点的指针 (Pointer to the ROS2 node)
 * @param[in] allocator 用于分配内存的分配器 (Allocator used for memory allocation)
 * @param[in] topic_name 需要获取发布者信息的主题名称 (The topic name for which publisher
 * information is required)
 * @param[in] no_mangle 是否不修改主题名称 (Whether to not modify the topic name)
 * @param[out] publishers_info 存储发布者信息的数组结构 (Array structure to store the publisher's
 * information)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_get_publishers_info_by_topic(
    const rmw_node_t* node,
    rcutils_allocator_t* allocator,
    const char* topic_name,
    bool no_mangle,
    rmw_topic_endpoint_info_array_t* publishers_info) {
  // 调用共享实现并返回结果 (Call the shared implementation and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_get_publishers_info_by_topic(
      eprosima_fastrtps_identifier, node, allocator, topic_name, no_mangle, publishers_info);
}

/**
 * @brief 获取指定主题的订阅者信息 (Get the subscriber information for a specific topic)
 *
 * @param[in] node 指向 ROS2 节点的指针 (Pointer to the ROS2 node)
 * @param[in] allocator 用于分配内存的分配器 (Allocator used for memory allocation)
 * @param[in] topic_name 需要获取订阅者信息的主题名称 (The topic name for which subscriber
 * information is required)
 * @param[in] no_mangle 是否不修改主题名称 (Whether to not modify the topic name)
 * @param[out] subscriptions_info 存储订阅者信息的数组结构 (Array structure to store the
 * subscriber's information)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_get_subscriptions_info_by_topic(
    const rmw_node_t* node,
    rcutils_allocator_t* allocator,
    const char* topic_name,
    bool no_mangle,
    rmw_topic_endpoint_info_array_t* subscriptions_info) {
  // 调用共享实现并返回结果 (Call the shared implementation and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_get_subscriptions_info_by_topic(
      eprosima_fastrtps_identifier, node, allocator, topic_name, no_mangle, subscriptions_info);
}

}  // extern "C"
