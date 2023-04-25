// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef TYPE_SUPPORT_REGISTRY_HPP_
#define TYPE_SUPPORT_REGISTRY_HPP_

#include <unordered_map>

#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"
#include "rmw_fastrtps_shared_cpp/locked_object.hpp"
#include "type_support_common.hpp"

using type_support_ptr = rmw_fastrtps_dynamic_cpp::BaseTypeSupport *;

/**
 * 用作类型注册表的值类型的数据结构。
 * A data structure to use as value type for the type registry.
 */
struct RefCountedTypeSupport {
  // 类型支持指针，初始值为 nullptr
  // Type support pointer, initialized to nullptr
  type_support_ptr type_support = nullptr;

  // 引用计数，初始值为 0
  // Reference count, initialized to 0
  uint32_t ref_count = 0;
};

// 消息类型映射
// Message type mapping
using msg_map_t = std::unordered_map<const rosidl_message_type_support_t *, RefCountedTypeSupport>;

// 服务类型映射
// Service type mapping
using srv_map_t = std::unordered_map<const rosidl_service_type_support_t *, RefCountedTypeSupport>;

class TypeSupportRegistry {
private:
  // 消息类型对象，带有锁保护
  // Locked message types object
  LockedObject<msg_map_t> message_types_;

  // 请求类型对象，带有锁保护
  // Locked request types object
  LockedObject<srv_map_t> request_types_;

  // 响应类型对象，带有锁保护
  // Locked response types object
  LockedObject<srv_map_t> response_types_;

  // 默认构造函数
  // Default constructor
  TypeSupportRegistry() = default;

public:
  // 析构函数
  // Destructor
  ~TypeSupportRegistry();

  // 获取实例的静态方法
  // Static method to get instance
  static TypeSupportRegistry &get_instance();

  // 获取消息类型支持的方法
  // Method to get message type support
  type_support_ptr get_message_type_support(const rosidl_message_type_support_t *ros_type_support);

  // 获取请求类型支持的方法
  // Method to get request type support
  type_support_ptr get_request_type_support(const rosidl_service_type_support_t *ros_type_support);

  // 获取响应类型支持的方法
  // Method to get response type support
  type_support_ptr get_response_type_support(const rosidl_service_type_support_t *ros_type_support);

  // 返回消息类型支持的方法
  // Method to return message type support
  void return_message_type_support(const rosidl_message_type_support_t *ros_type_support);

  // 返回请求类型支持的方法
  // Method to return request type support
  void return_request_type_support(const rosidl_service_type_support_t *ros_type_support);

  // 返回响应类型支持的方法
  // Method to return response type support
  void return_response_type_support(const rosidl_service_type_support_t *ros_type_support);
};

#endif  // TYPE_SUPPORT_REGISTRY_HPP_
