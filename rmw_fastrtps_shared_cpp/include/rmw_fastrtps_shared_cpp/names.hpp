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

#ifndef RMW_FASTRTPS_SHARED_CPP__NAMES_HPP_
#define RMW_FASTRTPS_SHARED_CPP__NAMES_HPP_

#include <string>

#include "fastrtps/utils/fixed_size_string.hpp"
#include "namespace_prefix.hpp"
#include "rmw/types.h"

/// 构造一个主题名称。 (Construct a topic name.)
/**
 * \param[in] prefix 主题名称所需的前缀。 (Required prefix for topic name.)
 * \param[in] base 主题的必需名称。 (Required name of the topic.)
 * \param[in] suffix 主题名称的可选后缀。 (Optional suffix for topic name.)
 */
inline eprosima::fastrtps::string_255 _mangle_topic_name(
    const char* prefix, const char* base, const char* suffix = nullptr) {
  std::ostringstream topicName;  // 创建一个字符串流对象来构建主题名称。 (Create a string stream
                                 // object to build the topic name.)
  if (prefix) {  // 如果有前缀，将其添加到主题名称中。 (If there is a prefix, add it to the topic
                 // name.)
    topicName << prefix;
  }
  topicName
      << base;  // 将基本主题名称添加到主题名称中。 (Add the base topic name to the topic name.)
  if (suffix) {  // 如果有后缀，将其添加到主题名称中。 (If there is a suffix, add it to the topic
                 // name.)
    topicName << suffix;
  }
  return topicName.str();  // 返回构建好的主题名称。 (Return the constructed topic name.)
}

/// 根据适当的约定构造主题名称。 (Construct a topic name according to proper conventions.)
/**
 * \param[in] qos_profile 主题的QoS配置文件。 (The QoS profile for the topic.)
 * \param[in] prefix 主题名称所需的前缀。 (Required prefix for topic name.)
 * \param[in] base 主题的必需名称。 (Required name of the topic.)
 * \param[in] suffix 主题名称的可选后缀。 (Optional suffix for topic name.)
 */
inline eprosima::fastrtps::string_255 _create_topic_name(
    const rmw_qos_profile_t* qos_profile,
    const char* prefix,
    const char* base,
    const char* suffix = nullptr) {
  assert(qos_profile);  // 确保 qos_profile 不为空。 (Ensure qos_profile is not null.)
  assert(base);         // 确保 base 不为空。 (Ensure base is not null.)
  /*
    > [!NOTE]:
    > `rmw/qos_profile_*_.md`
    > `avoid_ros_namespace_conventions` 主要是用于与非 ROS 进行通信时候对命名空间的问题
    > 因为 ROS 中对于 `/` 是有划分命名空间的意义，如果启用这个功能，表示 `/`
    将被识别为一个一般的字符
  */
  if (qos_profile
          ->avoid_ros_namespace_conventions) {  // 如果 QoS 配置文件要求避免 ROS
                                                // 命名空间约定，则将前缀设置为 nullptr。 (If the
                                                // QoS profile requires avoiding ROS namespace
                                                // conventions, set the prefix to nullptr.)
    prefix = nullptr;
  }
  return _mangle_topic_name(
      prefix, base,
      suffix);  // 调用 _mangle_topic_name 函数构建并返回主题名称。 (Call the _mangle_topic_name
                // function to construct and return the topic name.)
}

#endif  // RMW_FASTRTPS_SHARED_CPP__NAMES_HPP_
