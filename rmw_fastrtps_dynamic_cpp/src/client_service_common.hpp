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

#ifndef CLIENT_SERVICE_COMMON_HPP_
#define CLIENT_SERVICE_COMMON_HPP_

#include "rmw/error_handling.h"

/**
 * @brief 获取请求指针 (Get request pointer)
 *
 * @tparam ServiceType 服务类型 (Service type)
 * @param[in] untyped_service_members 未类型化的服务成员指针 (Pointer to untyped service members)
 * @return 返回请求成员指针，如果出错则返回 nullptr (Returns a pointer to the request members, or
 * nullptr if an error occurs)
 */
template <typename ServiceType>
const void *get_request_ptr(const void *untyped_service_members) {
  // 将未类型化的服务成员指针强制转换为 ServiceType 类型 (Cast the untyped service members pointer
  // to the ServiceType type)
  auto service_members = static_cast<const ServiceType *>(untyped_service_members);

  // 检查转换后的服务成员指针是否为空 (Check if the casted service members pointer is null)
  if (!service_members) {
    // 设置错误消息 (Set error message)
    RMW_SET_ERROR_MSG("service members handle is null");
    // 返回空指针 (Return nullptr)
    return nullptr;
  }

  // 返回请求成员指针 (Return the request members pointer)
  return service_members->request_members_;
}

// 声明 get_request_ptr 函数的另一个版本，接受类型支持参数 (Declare another version of the
// get_request_ptr function that takes a typesupport argument)
const void *get_request_ptr(const void *untyped_service_members, const char *typesupport);

/**
 * @brief 获取响应指针 (Get response pointer)
 *
 * @tparam ServiceType 服务类型 (Service type)
 * @param[in] untyped_service_members 未类型化的服务成员指针 (Pointer to untyped service members)
 * @return 返回响应成员指针，如果出错则返回 nullptr (Returns a pointer to the response members, or
 * nullptr if an error occurs)
 */
template <typename ServiceType>
const void *get_response_ptr(const void *untyped_service_members) {
  // 将未类型化的服务成员指针强制转换为 ServiceType 类型 (Cast the untyped service members pointer
  // to the ServiceType type)
  auto service_members = static_cast<const ServiceType *>(untyped_service_members);

  // 检查转换后的服务成员指针是否为空 (Check if the casted service members pointer is null)
  if (!service_members) {
    // 设置错误消息 (Set error message)
    RMW_SET_ERROR_MSG("service members handle is null");
    // 返回空指针 (Return nullptr)
    return nullptr;
  }

  // 返回响应成员指针 (Return the response members pointer)
  return service_members->response_members_;
}

// 声明 get_response_ptr 函数的另一个版本，接受类型支持参数 (Declare another version of the
// get_response_ptr function that takes a typesupport argument)
const void *get_response_ptr(const void *untyped_service_members, const char *typesupport);

#endif  // CLIENT_SERVICE_COMMON_HPP_
