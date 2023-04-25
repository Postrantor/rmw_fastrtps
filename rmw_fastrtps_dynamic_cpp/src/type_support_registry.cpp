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

#include "type_support_registry.hpp"

#include "rcutils/logging_macros.h"
#include "rmw/error_handling.h"
#include "type_support_common.hpp"

/**
 * @brief 获取类型支持对象 (Get the type support object)
 *
 * @tparam key_type ROS 类型支持的键类型 (Key type for ROS type support)
 * @tparam map_type 映射类型，用于存储类型支持信息 (Map type for storing type support information)
 * @tparam creator 创建类型支持对象的函数类型 (Function type for creating type support objects)
 * @param ros_type_support ROS 类型支持的键值 (Key value for ROS type support)
 * @param map 存储类型支持信息的映射 (Map for storing type support information)
 * @param fun 创建类型支持对象的函数 (Function for creating type support objects)
 * @return 返回类型支持指针 (Return the type support pointer)
 */
template <typename key_type, typename map_type, typename creator>
type_support_ptr get_type_support(const key_type &ros_type_support, map_type &map, creator fun) {
  // 对映射加锁，防止多线程冲突 (Lock the map to prevent multi-threading conflicts)
  std::lock_guard<std::mutex> guard(map.getMutex());
  // 获取或创建类型支持项 (Get or create the type support item)
  RefCountedTypeSupport &item = map()[ros_type_support];
  // 如果引用计数为 0，则创建新的类型支持对象 (If the reference count is 0, create a new type
  // support object)
  if (0 == item.ref_count++) {
    item.type_support = fun();
    // 如果创建失败，则从映射中删除该项并返回空指针 (If creation fails, remove the item from the map
    // and return a nullptr)
    if (!item.type_support) {
      map().erase(ros_type_support);
      return nullptr;
    }
  }
  // 返回类型支持指针 (Return the type support pointer)
  return item.type_support;
}

/**
 * @brief 归还类型支持对象 (Return the type support object)
 *
 * @tparam key_type ROS 类型支持的键类型 (Key type for ROS type support)
 * @tparam map_type 映射类型，用于存储类型支持信息 (Map type for storing type support information)
 * @param ros_type_support ROS 类型支持的键值 (Key value for ROS type support)
 * @param map 存储类型支持信息的映射 (Map for storing type support information)
 */
template <typename key_type, typename map_type>
void return_type_support(const key_type &ros_type_support, map_type &map) {
  // 对映射加锁，防止多线程冲突 (Lock the map to prevent multi-threading conflicts)
  std::lock_guard<std::mutex> guard(map.getMutex());
  // 查找类型支持项 (Find the type support item)
  auto it = map().find(ros_type_support);
  // 确保找到了该项 (Make sure the item is found)
  assert(it != map().end());
  // 减少引用计数，如果为 0，则删除类型支持对象并从映射中移除该项 (Decrease the reference count, if
  // it's 0, delete the type support object and remove the item from the map)
  if (0 == --it->second.ref_count) {
    delete it->second.type_support;
    map().erase(it);
  }
}

/**
 * @brief 清理类型支持注册表 (Clean up the type support registry)
 *
 * @tparam map_type 映射类型，用于存储类型支持信息 (Map type for storing type support information)
 * @param map 存储类型支持信息的映射 (Map for storing type support information)
 * @param msg 日志消息 (Log message)
 */
template <typename map_type>
void cleanup(map_type &map, const char *msg) {
  // 对映射加锁，防止多线程冲突 (Lock the map to prevent multi-threading conflicts)
  std::lock_guard<std::mutex> guard(map.getMutex());
  // 如果映射不为空，则清理类型支持对象并清空映射 (If the map is not empty, clean up the type
  // support objects and clear the map)
  if (!map().empty()) {
    RCUTILS_LOG_DEBUG_NAMED(
        "rmw_fastrtps_dynamic_cpp", "TypeSupportRegistry %s is not empty. Cleaning it up...", msg);
    for (auto it : map()) {
      delete it.second.type_support;
    }
    map().clear();
  }
}

/**
 * @brief 析构函数，清理消息类型、请求类型和响应类型的资源 (Destructor, clean up resources for
 * message types, request types and response types)
 */
TypeSupportRegistry::~TypeSupportRegistry() {
  cleanup(message_types_, "message_types_");  // 清理消息类型资源 (Clean up message type resources)
  cleanup(request_types_, "request_types_");  // 清理请求类型资源 (Clean up request type resources)
  cleanup(
      response_types_, "response_types_");  // 清理响应类型资源 (Clean up response type resources)
}

/**
 * @brief 获取 TypeSupportRegistry 的单例实例 (Get the singleton instance of TypeSupportRegistry)
 * @return TypeSupportRegistry 实例的引用 (Reference to the TypeSupportRegistry instance)
 */
TypeSupportRegistry &TypeSupportRegistry::get_instance() {
  static TypeSupportRegistry type_registry_instance;  // 静态实例 (Static instance)
  return type_registry_instance;
}

/**
 * @brief 获取消息类型支持 (Get message type support)
 * @param ros_type_support ROS 消息类型支持结构体指针 (Pointer to ROS message type support
 * structure)
 * @return 类型支持智能指针 (Smart pointer to type support)
 */
type_support_ptr TypeSupportRegistry::get_message_type_support(
    const rosidl_message_type_support_t *ros_type_support) {
  auto creator_fun = [&ros_type_support]() -> type_support_ptr {
    if (using_introspection_c_typesupport(ros_type_support->typesupport_identifier)) {
      auto members = static_cast<const rosidl_typesupport_introspection_c__MessageMembers *>(
          ros_type_support->data);
      return new MessageTypeSupport_c(
          members, ros_type_support);  // 使用 C 语言类型支持 (Using C language type support)
    } else if (using_introspection_cpp_typesupport(ros_type_support->typesupport_identifier)) {
      auto members = static_cast<const rosidl_typesupport_introspection_cpp::MessageMembers *>(
          ros_type_support->data);
      return new MessageTypeSupport_cpp(
          members, ros_type_support);  // 使用 C++ 语言类型支持 (Using C++ language type support)
    }
    RMW_SET_ERROR_MSG("Unknown typesupport identifier");  // 设置错误消息 (Set error message)
    return nullptr;
  };

  return get_type_support(ros_type_support, message_types_, creator_fun);
}

/**
 * @brief 获取服务请求类型支持 (Get service request type support)
 * @param ros_type_support ROS 服务类型支持结构体指针 (Pointer to ROS service type support
 * structure)
 * @return 类型支持智能指针 (Smart pointer to type support)
 */
type_support_ptr TypeSupportRegistry::get_request_type_support(
    const rosidl_service_type_support_t *ros_type_support) {
  auto creator_fun = [&ros_type_support]() -> type_support_ptr {
    if (using_introspection_c_typesupport(ros_type_support->typesupport_identifier)) {
      auto members = static_cast<const rosidl_typesupport_introspection_c__ServiceMembers *>(
          ros_type_support->data);
      return new RequestTypeSupport_c(
          members, ros_type_support);  // 使用 C 语言类型支持 (Using C language type support)
    } else if (using_introspection_cpp_typesupport(ros_type_support->typesupport_identifier)) {
      auto members = static_cast<const rosidl_typesupport_introspection_cpp::ServiceMembers *>(
          ros_type_support->data);
      return new RequestTypeSupport_cpp(
          members, ros_type_support);  // 使用 C++ 语言类型支持 (Using C++ language type support)
    }
    RMW_SET_ERROR_MSG("Unknown typesupport identifier");  // 设置错误消息 (Set error message)
    return nullptr;
  };

  return get_type_support(ros_type_support, request_types_, creator_fun);
}

/**
 * @brief 获取响应类型支持 (Get response type support)
 *
 * @param[in] ros_type_support ROS 服务类型支持指针 (Pointer to ROS service type support)
 * @return 类型支持指针 (Type support pointer)
 */
type_support_ptr TypeSupportRegistry::get_response_type_support(
    const rosidl_service_type_support_t *ros_type_support) {
  // 使用 lambda 函数创建类型支持对象 (Use a lambda function to create the type support object)
  auto creator_fun = [&ros_type_support]() -> type_support_ptr {
    // 如果使用 C 语言的内省类型支持 (If using introspection C type support)
    if (using_introspection_c_typesupport(ros_type_support->typesupport_identifier)) {
      // 将 data 转换为 C 语言内省服务成员指针 (Cast data to C introspection service members
      // pointer)
      auto members = static_cast<const rosidl_typesupport_introspection_c__ServiceMembers *>(
          ros_type_support->data);
      // 创建并返回 C 语言响应类型支持对象 (Create and return C response type support object)
      return new ResponseTypeSupport_c(members, ros_type_support);
    } else if (using_introspection_cpp_typesupport(ros_type_support->typesupport_identifier)) {
      // 将 data 转换为 C++ 语言内省服务成员指针 (Cast data to C++ introspection service members
      // pointer)
      auto members = static_cast<const rosidl_typesupport_introspection_cpp::ServiceMembers *>(
          ros_type_support->data);
      // 创建并返回 C++ 语言响应类型支持对象 (Create and return C++ response type support object)
      return new ResponseTypeSupport_cpp(members, ros_type_support);
    }
    // 设置未知类型支持标识符错误消息 (Set unknown type support identifier error message)
    RMW_SET_ERROR_MSG("Unknown typesupport identifier");
    return nullptr;
  };

  // 获取并返回响应类型支持 (Get and return response type support)
  return get_type_support(ros_type_support, response_types_, creator_fun);
}

/**
 * @brief 归还消息类型支持 (Return message type support)
 *
 * @param[in] ros_type_support ROS 消息类型支持指针 (Pointer to ROS message type support)
 */
void TypeSupportRegistry::return_message_type_support(
    const rosidl_message_type_support_t *ros_type_support) {
  return_type_support(ros_type_support, message_types_);
}

/**
 * @brief 归还请求类型支持 (Return request type support)
 *
 * @param[in] ros_type_support ROS 服务类型支持指针 (Pointer to ROS service type support)
 */
void TypeSupportRegistry::return_request_type_support(
    const rosidl_service_type_support_t *ros_type_support) {
  return_type_support(ros_type_support, request_types_);
}

/**
 * @brief 归还响应类型支持 (Return response type support)
 *
 * @param[in] ros_type_support ROS 服务类型支持指针 (Pointer to ROS service type support)
 */
void TypeSupportRegistry::return_response_type_support(
    const rosidl_service_type_support_t *ros_type_support) {
  return_type_support(ros_type_support, response_types_);
}
