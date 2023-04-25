// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#include "demangle.hpp"

#include <algorithm>
#include <string>
#include <vector>

#include "rcpputils/find_and_replace.hpp"
#include "rcutils/logging_macros.h"
#include "rcutils/types.h"
#include "rmw_fastrtps_shared_cpp/namespace_prefix.hpp"

/// 返回解析后的 ROS 主题名称，如果不是 ROS 主题，则返回原始名称。
/// Return the demangled ROS topic name, or the original if not a ROS topic.
std::string _demangle_if_ros_topic(const std::string& topic_name) {
  // 调用 _strip_ros_prefix_if_exists 函数处理主题名称
  // Call the _strip_ros_prefix_if_exists function to process the topic name
  return _strip_ros_prefix_if_exists(topic_name);
}

/// 返回解析后的 ROS 类型名称，如果不是 ROS 类型，则返回原始名称。
/// Return the demangled ROS type name, or the original if not a ROS type.
std::string _demangle_if_ros_type(const std::string& dds_type_string) {
  // 检查字符串末尾是否有下划线，没有则认为不是 ROS 类型
  // Check if the string ends with an underscore, if not, it's not a ROS type
  if (dds_type_string[dds_type_string.size() - 1] != '_') {
    // 不是 ROS 类型
    // Not a ROS type
    return dds_type_string;
  }

  // 定义子字符串 "dds_::"
  // Define the substring "dds_::"
  std::string substring = "dds_::";
  // 查找子字符串在 dds_type_string 中的位置
  // Find the position of the substring in dds_type_string
  size_t substring_position = dds_type_string.find(substring);
  // 如果子字符串不存在，则认为不是 ROS 类型
  // If the substring is not found, it's not a ROS type
  if (substring_position == std::string::npos) {
    // 不是 ROS 类型
    // Not a ROS type
    return dds_type_string;
  }

  // 获取类型命名空间
  // Get the type namespace
  std::string type_namespace = dds_type_string.substr(0, substring_position);
  // 将 "::" 替换为 "/"
  // Replace "::" with "/"
  type_namespace = rcpputils::find_and_replace(type_namespace, "::", "/");
  // 计算类型名称的起始位置
  // Calculate the starting position of the type name
  size_t start = substring_position + substring.size();
  // 提取类型名称
  // Extract the type name
  std::string type_name = dds_type_string.substr(start, dds_type_string.length() - 1 - start);
  // 返回解析后的 ROS 类型名称
  // Return the demangled ROS type name
  return type_namespace + type_name;
}

/// 如果给定主题是某个主题的一部分，则返回该主题的名称，否则返回空字符串。
/// Return the topic name for a given topic if it is part of one, else an empty string.
std::string _demangle_ros_topic_from_topic(const std::string& topic_name) {
  // 调用 _resolve_prefix 函数处理主题名称，并使用 ros_topic_prefix
  // Call the _resolve_prefix function to process the topic name and use ros_topic_prefix
  return _resolve_prefix(topic_name, ros_topic_prefix);
}

/// 返回给定主题的服务名称（如果它是其中的一部分），否则返回 ""。
/// Return the service name for a given topic if it is part of one, else "".
/**
 * @param[in] prefix 前缀字符串
 * @param[in] topic_name 主题名称字符串
 * @param[in] suffix 后缀字符串
 * @return 如果主题名称是服务的一部分，则返回服务名称，否则返回空字符串
 */
std::string _demangle_service_from_topic(
    const std::string& prefix, const std::string& topic_name, std::string suffix) {
  // 解析主题名称中的前缀
  // Resolve the prefix in the topic name
  std::string service_name = _resolve_prefix(topic_name, prefix);

  // 如果服务名称为空，则返回空字符串
  // If the service name is empty, return an empty string
  if ("" == service_name) {
    return "";
  }

  // 查找后缀在服务名称中的位置
  // Find the position of the suffix in the service name
  size_t suffix_position = service_name.rfind(suffix);
  if (suffix_position != std::string::npos) {
    // 如果后缀不在服务名称的末尾，则发出警告并返回空字符串
    // If the suffix is not at the end of the service name, issue a warning and return an empty
    // string
    if (service_name.length() - suffix_position - suffix.length() != 0) {
      RCUTILS_LOG_WARN_NAMED(
          "rmw_fastrtps_shared_cpp",
          "service topic has service prefix and a suffix, but not at the end"
          ", report this: '%s'",
          topic_name.c_str());
      return "";
    }
  } else {
    // 如果服务主题有前缀但没有后缀，则发出警告并返回空字符串
    // If the service topic has a prefix but no suffix, issue a warning and return an empty string
    RCUTILS_LOG_WARN_NAMED(
        "rmw_fastrtps_shared_cpp",
        "service topic has prefix but no suffix"
        ", report this: '%s'",
        topic_name.c_str());
    return "";
  }

  // 返回去掉后缀的服务名称
  // Return the service name without the suffix
  return service_name.substr(0, suffix_position);
}

/**
 * @param[in] topic_name 主题名称字符串
 * @return 如果主题名称是服务的一部分，则返回服务名称，否则返回空字符串
 */
std::string _demangle_service_from_topic(const std::string& topic_name) {
  // 尝试从主题名称中解析服务回复
  // Attempt to demangle the service reply from the topic name
  const std::string demangled_topic = _demangle_service_reply_from_topic(topic_name);

  // 如果解析到服务回复，则返回解析结果
  // If a service reply is demangled, return the result
  if ("" != demangled_topic) {
    return demangled_topic;
  }

  // 否则，尝试从主题名称中解析服务请求
  // Otherwise, attempt to demangle the service request from the topic name
  return _demangle_service_request_from_topic(topic_name);
}

/**
 * @param[in] topic_name 主题名称字符串
 * @return 如果主题名称是服务请求的一部分，则返回服务名称，否则返回空字符串
 */
std::string _demangle_service_request_from_topic(const std::string& topic_name) {
  return _demangle_service_from_topic(ros_service_requester_prefix, topic_name, "Request");
}

/**
 * @param[in] topic_name 主题名称字符串
 * @return 如果主题名称是服务回复的一部分，则返回服务名称，否则返回空字符串
 */
std::string _demangle_service_reply_from_topic(const std::string& topic_name) {
  return _demangle_service_from_topic(ros_service_response_prefix, topic_name, "Reply");
}

/// 返回解析后的服务类型，如果是 ROS 服务类型，则返回解析后的类型，否则返回空字符串。
/// Return the demangled service type if it is a ROS srv type, else "".
std::string _demangle_service_type_only(const std::string& dds_type_name) {
  // 定义一个子字符串 "dds_::"
  // Define a substring "dds_::"
  std::string ns_substring = "dds_::";

  // 查找子字符串在 dds_type_name 中的位置
  // Find the position of the substring in dds_type_name
  size_t ns_substring_position = dds_type_name.find(ns_substring);

  // 如果子字符串未找到，则不是 ROS 服务类型
  // If the substring is not found, it's not a ROS service type
  if (std::string::npos == ns_substring_position) {
    return "";
  }

  // 定义后缀列表
  // Define a list of suffixes
  auto suffixes = {
      std::string("_Response_"),
      std::string("_Request_"),
  };

  std::string found_suffix = "";
  size_t suffix_position = 0;

  // 遍历后缀列表
  // Iterate through the list of suffixes
  for (auto suffix : suffixes) {
    // 查找后缀在 dds_type_name 中的位置
    // Find the position of the suffix in dds_type_name
    suffix_position = dds_type_name.rfind(suffix);

    // 如果找到了后缀
    // If the suffix is found
    if (suffix_position != std::string::npos) {
      // 检查后缀是否在字符串的末尾
      // Check if the suffix is at the end of the string
      if (dds_type_name.length() - suffix_position - suffix.length() != 0) {
        RCUTILS_LOG_WARN_NAMED(
            "rmw_fastrtps_shared_cpp",
            "service type contains 'dds_::' and a suffix, but not at the end"
            ", report this: '%s'",
            dds_type_name.c_str());
        continue;
      }
      found_suffix = suffix;
      break;
    }
  }

  // 如果没有找到后缀
  // If the suffix is not found
  if (std::string::npos == suffix_position) {
    RCUTILS_LOG_WARN_NAMED(
        "rmw_fastrtps_shared_cpp",
        "service type contains 'dds_::' but does not have a suffix"
        ", report this: '%s'",
        dds_type_name.c_str());
    return "";
  }

  // 一切正常，将其从 '[type_namespace::]dds_::<type><suffix>' 格式转换为 '[type_namespace/]<type>'
  // 格式 Everything checks out, reformat it from '[type_namespace::]dds_::<type><suffix>' to
  // '[type_namespace/]<type>'
  std::string type_namespace = dds_type_name.substr(0, ns_substring_position);
  type_namespace = rcpputils::find_and_replace(type_namespace, "::", "/");
  size_t start = ns_substring_position + ns_substring.length();
  std::string type_name = dds_type_name.substr(start, suffix_position - start);
  return type_namespace + type_name;
}

// 返回未经修改的名称
// Return the unmodified name
std::string _identity_demangle(const std::string& name) { return name; }
