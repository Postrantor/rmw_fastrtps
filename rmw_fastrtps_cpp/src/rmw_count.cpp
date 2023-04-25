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
#include <string>
#include <vector>

#include "rcutils/logging_macros.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/types.h"
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 计算给定主题上的发布者数量 (Counts the number of publishers on a given topic)
 *
 * @param[in] node 指向当前 ROS2 节点的指针 (Pointer to the current ROS2 node)
 * @param[in] topic_name 需要计算发布者数量的主题名称 (The topic name for which to count the
 * publishers)
 * @param[out] count 存储发布者数量的变量指针 (Pointer to the variable that will store the publisher
 * count)
 * @return rmw_ret_t 返回操作结果，成功或错误 (Returns the operation result, success or error)
 */
rmw_ret_t rmw_count_publishers(const rmw_node_t* node, const char* topic_name, size_t* count) {
  // 调用 FastRTPS 实现的共享函数来计算发布者数量
  // Call the shared FastRTPS implementation function to count the publishers
  return rmw_fastrtps_shared_cpp::__rmw_count_publishers(
      eprosima_fastrtps_identifier, node, topic_name, count);
}

/**
 * @brief 计算给定主题上的订阅者数量 (Counts the number of subscribers on a given topic)
 *
 * @param[in] node 指向当前 ROS2 节点的指针 (Pointer to the current ROS2 node)
 * @param[in] topic_name 需要计算订阅者数量的主题名称 (The topic name for which to count the
 * subscribers)
 * @param[out] count 存储订阅者数量的变量指针 (Pointer to the variable that will store the
 * subscriber count)
 * @return rmw_ret_t 返回操作结果，成功或错误 (Returns the operation result, success or error)
 */
rmw_ret_t rmw_count_subscribers(const rmw_node_t* node, const char* topic_name, size_t* count) {
  // 调用 FastRTPS 实现的共享函数来计算订阅者数量
  // Call the shared FastRTPS implementation function to count the subscribers
  return rmw_fastrtps_shared_cpp::__rmw_count_subscribers(
      eprosima_fastrtps_identifier, node, topic_name, count);
}

}  // extern "C"
