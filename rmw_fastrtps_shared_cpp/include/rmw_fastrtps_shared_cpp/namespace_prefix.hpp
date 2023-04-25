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

#ifndef RMW_FASTRTPS_SHARED_CPP__NAMESPACE_PREFIX_HPP_
#define RMW_FASTRTPS_SHARED_CPP__NAMESPACE_PREFIX_HPP_

#include <string>
#include <vector>

#include "rmw_fastrtps_shared_cpp/visibility_control.h"

extern "C" {
// 声明 ROS 主题前缀常量 (Declare the ROS topic prefix constant)
RMW_FASTRTPS_SHARED_CPP_PUBLIC extern const char* const ros_topic_prefix;
// 声明 ROS 服务请求者前缀常量 (Declare the ROS service requester prefix constant)
RMW_FASTRTPS_SHARED_CPP_PUBLIC extern const char* const ros_service_requester_prefix;
// 声明 ROS 服务响应前缀常量 (Declare the ROS service response prefix constant)
RMW_FASTRTPS_SHARED_CPP_PUBLIC extern const char* const ros_service_response_prefix;

// 声明一个包含所有 ROS 前缀的字符串向量 (Declare a vector of strings containing all ROS prefixes)
RMW_FASTRTPS_SHARED_CPP_PUBLIC extern const std::vector<std::string> _ros_prefixes;
}  // extern "C"

/// 返回去除 `prefix` 的 `name`，如果不存在，则返回 ""。
/**
 * \param[in] name 将从中删除前缀的字符串 (The string from which the prefix will be removed)
 * \param[in] prefix 要删除的前缀 (The prefix to be removed)
 * \return 去除前缀的名称，或 (The name stripped of the prefix, or)
 * \return 如果名称不以前缀开头，则返回 "" (Return "" if the name doesn't start with the prefix)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
std::string _resolve_prefix(const std::string& name, const std::string& prefix);

/// 如果存在，则返回 ROS 特定前缀，否则返回 ""。
RMW_FASTRTPS_SHARED_CPP_PUBLIC
std::string _get_ros_prefix_if_exists(const std::string& topic_name);

/// 返回去除 ROS 特定前缀（如果存在）的主题名称。
RMW_FASTRTPS_SHARED_CPP_PUBLIC
std::string _strip_ros_prefix_if_exists(const std::string& topic_name);

/// 返回 ros 前缀列表
RMW_FASTRTPS_SHARED_CPP_PUBLIC
const std::vector<std::string>& _get_all_ros_prefixes();

#endif  // RMW_FASTRTPS_SHARED_CPP__NAMESPACE_PREFIX_HPP_
