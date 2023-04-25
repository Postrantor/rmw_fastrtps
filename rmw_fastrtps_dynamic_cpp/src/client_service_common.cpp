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

#include "client_service_common.hpp"

#include <string>

#include "type_support_common.hpp"

/**
 * @brief 获取请求指针 (Get the request pointer)
 *
 * @param[in] untyped_service_members 未类型化的服务成员指针 (Pointer to untyped service members)
 * @param[in] typesupport 类型支持标识符 (Typesupport identifier)
 * @return 返回请求指针，如果类型支持未知，则返回 nullptr (Returns the request pointer, or nullptr
 * if typesupport is unknown)
 */
const void* get_request_ptr(const void* untyped_service_members, const char* typesupport) {
  // 判断是否使用 C 语言内省类型支持 (Check if using introspection C typesupport)
  if (using_introspection_c_typesupport(typesupport)) {
    // 使用 C 语言内省类型支持获取请求指针 (Get the request pointer using introspection C
    // typesupport)
    return get_request_ptr<rosidl_typesupport_introspection_c__ServiceMembers>(
        untyped_service_members);
  } else if (using_introspection_cpp_typesupport(typesupport)) {
    // 使用 C++ 语言内省类型支持获取请求指针 (Get the request pointer using introspection C++
    // typesupport)
    return get_request_ptr<rosidl_typesupport_introspection_cpp::ServiceMembers>(
        untyped_service_members);
  }
  // 设置错误消息为 "Unknown typesupport identifier" (Set error message to "Unknown typesupport
  // identifier")
  RMW_SET_ERROR_MSG("Unknown typesupport identifier");
  // 返回空指针 (Return nullptr)
  return nullptr;
}

/**
 * @brief 获取响应指针 (Get the response pointer)
 *
 * @param[in] untyped_service_members 未类型化的服务成员指针 (Pointer to untyped service members)
 * @param[in] typesupport 类型支持标识符 (Typesupport identifier)
 * @return 返回响应指针，如果类型支持未知，则返回 nullptr (Returns the response pointer, or nullptr
 * if typesupport is unknown)
 */
const void* get_response_ptr(const void* untyped_service_members, const char* typesupport) {
  // 判断是否使用 C 语言内省类型支持 (Check if using introspection C typesupport)
  if (using_introspection_c_typesupport(typesupport)) {
    // 使用 C 语言内省类型支持获取响应指针 (Get the response pointer using introspection C
    // typesupport)
    return get_response_ptr<rosidl_typesupport_introspection_c__ServiceMembers>(
        untyped_service_members);
  } else if (using_introspection_cpp_typesupport(typesupport)) {
    // 使用 C++ 语言内省类型支持获取响应指针 (Get the response pointer using introspection C++
    // typesupport)
    return get_response_ptr<rosidl_typesupport_introspection_cpp::ServiceMembers>(
        untyped_service_members);
  }
  // 设置错误消息为 "Unknown typesupport identifier" (Set error message to "Unknown typesupport
  // identifier")
  RMW_SET_ERROR_MSG("Unknown typesupport identifier");
  // 返回空指针 (Return nullptr)
  return nullptr;
}
