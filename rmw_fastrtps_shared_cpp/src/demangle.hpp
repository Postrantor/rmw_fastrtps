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

#ifndef DEMANGLE_HPP_
#define DEMANGLE_HPP_

#include <string>

/// 返回解析后的 ROS 主题名称，如果不是 ROS 主题，则返回原始名称。
/// Return the demangled ROS topic name or the original if not a ROS topic.
std::string _demangle_if_ros_topic(const std::string &topic_name);

/// 返回解析后的 ROS 类型名称，如果不是 ROS 类型，则返回原始名称。
/// Return the demangled ROS type name or the original if not a ROS type.
std::string _demangle_if_ros_type(const std::string &dds_type_string);

/// 如果给定主题是某个主题的一部分，则返回该主题名称，否则返回空字符串。
/// Return the topic name for a given topic if it is part of one, else "".
std::string _demangle_ros_topic_from_topic(const std::string &topic_name);

/// 如果给定主题是服务的一部分，则返回该服务名称，否则返回空字符串。
/// Return the service name for a given topic if it is part of a service, else "".
std::string _demangle_service_from_topic(const std::string &topic_name);

/// 如果给定主题是服务请求的一部分，则返回该服务名称，否则返回空字符串。
/// Return the service name for a given topic if it is part of a service request, else "".
std::string _demangle_service_request_from_topic(const std::string &topic_name);

/// 如果给定主题是服务回复的一部分，则返回该服务名称，否则返回空字符串。
/// Return the service name for a given topic if it is part of a service reply, else "".
std::string _demangle_service_reply_from_topic(const std::string &topic_name);

/// 如果给定类型是 ROS 服务类型，则返回解析后的服务类型名称，否则返回空字符串。
/// Return the demangled service type name if it is a ROS srv type, else "".
std::string _demangle_service_type_only(const std::string &dds_type_name);

/// 当 ROS 名称未经处理时使用。
/// Used when ros names are not mangled.
std::string _identity_demangle(const std::string &name);

// 定义解析函数和组合函数类型
// Define DemangleFunction and MangleFunction types
using DemangleFunction = std::string (*)(const std::string &);
using MangleFunction = DemangleFunction;

#endif  // DEMANGLE_HPP_
