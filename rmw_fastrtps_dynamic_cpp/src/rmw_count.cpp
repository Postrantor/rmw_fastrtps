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
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 计算发布者的数量 (Count the number of publishers)
 *
 * @param[in] node 指向 ROS2 节点的指针 (Pointer to the ROS2 node)
 * @param[in] topic_name 主题名称 (Topic name)
 * @param[out] count 存储发布者数量的指针 (Pointer to store the publisher count)
 *
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_count_publishers(const rmw_node_t* node, const char* topic_name, size_t* count) {
  // 调用共享函数，计算发布者数量 (Call the shared function to count the publishers)
  return rmw_fastrtps_shared_cpp::__rmw_count_publishers(
      eprosima_fastrtps_identifier, node, topic_name, count);
}

/**
 * @brief 计算订阅者的数量 (Count the number of subscribers)
 *
 * @param[in] node 指向 ROS2 节点的指针 (Pointer to the ROS2 node)
 * @param[in] topic_name 主题名称 (Topic name)
 * @param[out] count 存储订阅者数量的指针 (Pointer to store the subscriber count)
 *
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_count_subscribers(const rmw_node_t* node, const char* topic_name, size_t* count) {
  // 调用共享函数，计算订阅者数量 (Call the shared function to count the subscribers)
  return rmw_fastrtps_shared_cpp::__rmw_count_subscribers(
      eprosima_fastrtps_identifier, node, topic_name, count);
}

}  // extern "C"
