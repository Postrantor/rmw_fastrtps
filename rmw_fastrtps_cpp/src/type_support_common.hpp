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

#ifndef TYPE_SUPPORT_COMMON_HPP_
#define TYPE_SUPPORT_COMMON_HPP_

#include <sstream>
#include <string>

#include "rmw/error_handling.h"
#include "rmw_fastrtps_cpp/MessageTypeSupport.hpp"
#include "rmw_fastrtps_cpp/ServiceTypeSupport.hpp"
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"
#include "rosidl_typesupport_fastrtps_c/identifier.h"
#include "rosidl_typesupport_fastrtps_cpp/identifier.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"
#include "rosidl_typesupport_fastrtps_cpp/service_type_support.h"
#define RMW_FASTRTPS_CPP_TYPESUPPORT_C rosidl_typesupport_fastrtps_c__identifier
#define RMW_FASTRTPS_CPP_TYPESUPPORT_CPP rosidl_typesupport_fastrtps_cpp::typesupport_identifier

using MessageTypeSupport_cpp = rmw_fastrtps_cpp::MessageTypeSupport;
using TypeSupport_cpp = rmw_fastrtps_cpp::TypeSupport;
using RequestTypeSupport_cpp = rmw_fastrtps_cpp::RequestTypeSupport;
using ResponseTypeSupport_cpp = rmw_fastrtps_cpp::ResponseTypeSupport;

/**
 * @brief 创建类型名称
 * @param message_namespace 消息命名空间
 * @param message_name 消息名称
 * @return 返回创建的类型名称字符串
 *
 * @brief Create type name
 * @param message_namespace Message namespace
 * @param message_name Message name
 * @return Returns the created type name string
 */
inline std::string _create_type_name(std::string message_namespace, std::string message_name) {
  // 创建一个输出字符串流对象
  // Create an output string stream object
  std::ostringstream ss;

  // 如果消息命名空间不为空，则在输出流中添加命名空间和分隔符"::"
  // If the message namespace is not empty, add the namespace and separator "::" to the output
  // stream
  if (!message_namespace.empty()) {
    ss << message_namespace << "::";
  }

  // 在输出流中添加 "dds_::" 和消息名称，并在消息名称后添加下划线"_"
  // Add "dds_::" and the message name to the output stream, and append an underscore "_" after the
  // message name
  ss << "dds_::" << message_name << "_";

  // 返回创建的类型名称字符串
  // Return the created type name string
  return ss.str();
}

/**
 * @brief 创建类型名称
 * @param members 消息类型支持回调结构体指针
 * @return 返回创建的类型名称字符串
 *
 * @brief Create type name
 * @param members Pointer to the message type support callbacks structure
 * @return Returns the created type name string
 */
inline std::string _create_type_name(const message_type_support_callbacks_t* members) {
  // 如果 members 为空，设置错误消息并返回空字符串
  // If members is null, set the error message and return an empty string
  if (!members) {
    RMW_SET_ERROR_MSG("members handle is null");
    return "";
  }

  // 从 members 中获取消息命名空间和消息名称
  // Get the message namespace and message name from members
  std::string message_namespace(members->message_namespace_);
  std::string message_name(members->message_name_);

  // 调用之前定义的 _create_type_name 函数并返回创建的类型名称字符串
  // Call the previously defined _create_type_name function and return the created type name string
  return _create_type_name(message_namespace, message_name);
}

#endif  // TYPE_SUPPORT_COMMON_HPP_
