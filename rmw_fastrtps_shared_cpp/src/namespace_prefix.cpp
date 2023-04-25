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

#include "rmw_fastrtps_shared_cpp/namespace_prefix.hpp"

#include <string>
#include <vector>

extern "C" {
// 定义 ROS 主题前缀 (Define the ROS topic prefix)
const char* const ros_topic_prefix = "rt";
// 定义 ROS 服务请求者前缀 (Define the ROS service requester prefix)
const char* const ros_service_requester_prefix = "rq";
// 定义 ROS 服务响应前缀 (Define the ROS service response prefix)
const char* const ros_service_response_prefix = "rr";

// 创建一个包含所有 ROS 前缀的向量 (Create a vector containing all ROS prefixes)
const std::vector<std::string> _ros_prefixes = {
    ros_topic_prefix, ros_service_requester_prefix, ros_service_response_prefix};
}  // extern "C"

/**
 * @brief 返回去除 `prefix` 的 `name`.
 * @param name 要处理的字符串
 * @param prefix 要移除的前缀
 * @return 去除前缀后的字符串
 *
 * @brief Returns `name` stripped of `prefix`.
 * @param name The string to process
 * @param prefix The prefix to remove
 * @return The string with the prefix removed
 */
std::string _resolve_prefix(const std::string& name, const std::string& prefix) {
  if (name.rfind(prefix + "/", 0) == 0) {
    return name.substr(prefix.length());
  }
  return "";
}

/**
 * @brief 如果存在，返回 ROS 特定前缀；否则返回 "".
 * @param topic_name 主题名称
 * @return 如果存在 ROS 前缀，则返回前缀；否则返回空字符串
 *
 * @brief Return the ROS specific prefix if it exists, otherwise "".
 * @param topic_name The topic name
 * @return The ROS prefix if it exists, otherwise an empty string
 */
std::string _get_ros_prefix_if_exists(const std::string& topic_name) {
  for (const auto& prefix : _ros_prefixes) {
    if (topic_name.rfind(prefix + "/", 0) == 0) {
      return prefix;
    }
  }
  return "";
}

/**
 * @brief 如果存在，从主题名称中去除 ROS 特定前缀.
 * @param topic_name 主题名称
 * @return 去除 ROS 前缀后的主题名称
 *
 * @brief Strip the ROS specific prefix if it exists from the topic name.
 * @param topic_name The topic name
 * @return The topic name with the ROS prefix removed if it exists
 */
std::string _strip_ros_prefix_if_exists(const std::string& topic_name) {
  for (const auto& prefix : _ros_prefixes) {
    if (topic_name.rfind(prefix + "/", 0) == 0) {
      return topic_name.substr(prefix.length());
    }
  }
  return topic_name;
}

/// Returns the list of ros prefixes
const std::vector<std::string>& _get_all_ros_prefixes() { return _ros_prefixes; }
