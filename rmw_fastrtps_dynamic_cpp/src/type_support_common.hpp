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

#include "rcpputils/find_and_replace.hpp"
#include "rmw/error_handling.h"
#include "rmw_fastrtps_dynamic_cpp/MessageTypeSupport.hpp"
#include "rmw_fastrtps_dynamic_cpp/ServiceTypeSupport.hpp"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"
#include "rosidl_typesupport_introspection_c/visibility_control.h"

// 使用 MessageTypeSupport_c 类型定义 rmw_fastrtps_dynamic_cpp::MessageTypeSupport，
// 其中 rosidl_typesupport_introspection_c__MessageMembers 用于 C 语言类型支持。
// Define MessageTypeSupport_c as a type alias for rmw_fastrtps_dynamic_cpp::MessageTypeSupport,
// where rosidl_typesupport_introspection_c__MessageMembers is used for C language typesupport.
using MessageTypeSupport_c = rmw_fastrtps_dynamic_cpp::MessageTypeSupport<
    rosidl_typesupport_introspection_c__MessageMembers>;

// 使用 MessageTypeSupport_cpp 类型定义 rmw_fastrtps_dynamic_cpp::MessageTypeSupport，
// 其中 rosidl_typesupport_introspection_cpp::MessageMembers 用于 C++ 语言类型支持。
// Define MessageTypeSupport_cpp as a type alias for rmw_fastrtps_dynamic_cpp::MessageTypeSupport,
// where rosidl_typesupport_introspection_cpp::MessageMembers is used for C++ language typesupport.
using MessageTypeSupport_cpp = rmw_fastrtps_dynamic_cpp::MessageTypeSupport<
    rosidl_typesupport_introspection_cpp::MessageMembers>;

// 使用 TypeSupport_c 类型定义 rmw_fastrtps_dynamic_cpp::TypeSupport，
// 其中 rosidl_typesupport_introspection_c__MessageMembers 用于 C 语言类型支持。
// Define TypeSupport_c as a type alias for rmw_fastrtps_dynamic_cpp::TypeSupport,
// where rosidl_typesupport_introspection_c__MessageMembers is used for C language typesupport.
using TypeSupport_c =
    rmw_fastrtps_dynamic_cpp::TypeSupport<rosidl_typesupport_introspection_c__MessageMembers>;

// 使用 TypeSupport_cpp 类型定义 rmw_fastrtps_dynamic_cpp::TypeSupport，
// 其中 rosidl_typesupport_introspection_cpp::MessageMembers 用于 C++ 语言类型支持。
// Define TypeSupport_cpp as a type alias for rmw_fastrtps_dynamic_cpp::TypeSupport,
// where rosidl_typesupport_introspection_cpp::MessageMembers is used for C++ language typesupport.
using TypeSupport_cpp =
    rmw_fastrtps_dynamic_cpp::TypeSupport<rosidl_typesupport_introspection_cpp::MessageMembers>;

// 使用 RequestTypeSupport_c 类型定义 rmw_fastrtps_dynamic_cpp::RequestTypeSupport，
// 其中 rosidl_typesupport_introspection_c__ServiceMembers 和
// rosidl_typesupport_introspection_c__MessageMembers 用于 C 语言类型支持。
// Define RequestTypeSupport_c as a type alias for rmw_fastrtps_dynamic_cpp::RequestTypeSupport,
// where rosidl_typesupport_introspection_c__ServiceMembers and
// rosidl_typesupport_introspection_c__MessageMembers are used for C language typesupport.
using RequestTypeSupport_c = rmw_fastrtps_dynamic_cpp::RequestTypeSupport<
    rosidl_typesupport_introspection_c__ServiceMembers,
    rosidl_typesupport_introspection_c__MessageMembers>;

// 使用 RequestTypeSupport_cpp 类型定义 rmw_fastrtps_dynamic_cpp::RequestTypeSupport，
// 其中 rosidl_typesupport_introspection_cpp::ServiceMembers 和
// rosidl_typesupport_introspection_cpp::MessageMembers 用于 C++ 语言类型支持。
// Define RequestTypeSupport_cpp as a type alias for rmw_fastrtps_dynamic_cpp::RequestTypeSupport,
// where rosidl_typesupport_introspection_cpp::ServiceMembers and
// rosidl_typesupport_introspection_cpp::MessageMembers are used for C++ language typesupport.
using RequestTypeSupport_cpp = rmw_fastrtps_dynamic_cpp::RequestTypeSupport<
    rosidl_typesupport_introspection_cpp::ServiceMembers,
    rosidl_typesupport_introspection_cpp::MessageMembers>;

// 使用 ResponseTypeSupport_c 类型定义 rmw_fastrtps_dynamic_cpp::ResponseTypeSupport，
// 其中 rosidl_typesupport_introspection_c__ServiceMembers 和
// rosidl_typesupport_introspection_c__MessageMembers 用于 C 语言类型支持。
// Define ResponseTypeSupport_c as a type alias for rmw_fastrtps_dynamic_cpp::ResponseTypeSupport,
// where rosidl_typesupport_introspection_c__ServiceMembers and
// rosidl_typesupport_introspection_c__MessageMembers are used for C language typesupport.
using ResponseTypeSupport_c = rmw_fastrtps_dynamic_cpp::ResponseTypeSupport<
    rosidl_typesupport_introspection_c__ServiceMembers,
    rosidl_typesupport_introspection_c__MessageMembers>;

// 使用 ResponseTypeSupport_cpp 类型定义 rmw_fastrtps_dynamic_cpp::ResponseTypeSupport，
// 其中 rosidl_typesupport_introspection_cpp::ServiceMembers 和
// rosidl_typesupport_introspection_cpp::MessageMembers 用于 C++ 语言类型支持。
// Define ResponseTypeSupport_cpp as a type alias for rmw_fastrtps_dynamic_cpp::ResponseTypeSupport,
// where rosidl_typesupport_introspection_cpp::ServiceMembers and
// rosidl_typesupport_introspection_cpp::MessageMembers are used for C++ language typesupport.
using ResponseTypeSupport_cpp = rmw_fastrtps_dynamic_cpp::ResponseTypeSupport<
    rosidl_typesupport_introspection_cpp::ServiceMembers,
    rosidl_typesupport_introspection_cpp::MessageMembers>;

// 判断是否使用 C 语言的内省类型支持。
bool using_introspection_c_typesupport(const char *typesupport_identifier);

// 判断是否使用 C++ 语言的内省
bool using_introspection_cpp_typesupport(const char *typesupport_identifier);

/**
 * @brief 创建类型名称的辅助函数（Create a type name helper function）
 *
 * @tparam MembersType 成员类型（Member type）
 * @param[in] untyped_members 未类型化的成员指针（Pointer to untyped members）
 * @return std::string 类型名称（Type name）
 */
template <typename MembersType>
ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_LOCAL inline std::string _create_type_name(
    const void *untyped_members) {
  // 将未类型化的成员指针转换为指定的成员类型指针（Cast the untyped member pointer to the specified
  // member type pointer）
  auto members = static_cast<const MembersType *>(untyped_members);
  if (!members) {
    // 如果成员指针为空，则设置错误消息（If the member pointer is null, set an error message）
    RMW_SET_ERROR_MSG("members handle is null");
    return "";
  }

  std::ostringstream ss;
  // 获取消息命名空间（Get the message namespace）
  std::string message_namespace(members->message_namespace_);
  // 查找并替换C命名空间分隔符为C++，以防使用C类型支持（Find and replace C namespace separator with
  // C++, in case this is using C typesupport）
  message_namespace = rcpputils::find_and_replace(message_namespace, "__", "::");
  // 获取消息名称（Get the message name）
  std::string message_name(members->message_name_);
  if (!message_namespace.empty()) {
    // 如果消息命名空间不为空，则将其添加到输出流中（If the message namespace is not empty, add it
    // to the output stream）
    ss << message_namespace << "::";
  }
  // 将 "dds_::" 和消息名称添加到输出流中（Add "dds_::" and the message name to the output stream）
  ss << "dds_::" << message_name << "_";
  return ss.str();
}

/**
 * @brief 创建类型名称的函数（Create a type name function）
 *
 * @param[in] untyped_members 未类型化的成员指针（Pointer to untyped members）
 * @param[in] typesupport 类型支持标识符（Typesupport identifier）
 * @return std::string 类型名称（Type name）
 */
ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_LOCAL
inline std::string _create_type_name(const void *untyped_members, const char *typesupport) {
  // 根据类型支持标识符选择适当的类型名称创建辅助函数（Select the appropriate type name creation
  // helper function based on the typesupport identifier）
  if (using_introspection_c_typesupport(typesupport)) {
    return _create_type_name<rosidl_typesupport_introspection_c__MessageMembers>(untyped_members);
  } else if (using_introspection_cpp_typesupport(typesupport)) {
    return _create_type_name<rosidl_typesupport_introspection_cpp::MessageMembers>(untyped_members);
  }
  // 如果类型支持标识符未知，则设置错误消息（If the typesupport identifier is unknown, set an error
  // message）
  RMW_SET_ERROR_MSG("Unknown typesupport identifier");
  return "";
}

#endif  // TYPE_SUPPORT_COMMON_HPP_
