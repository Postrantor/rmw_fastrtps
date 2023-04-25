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

#ifndef RMW_FASTRTPS_DYNAMIC_CPP__TYPESUPPORT_HPP_
#define RMW_FASTRTPS_DYNAMIC_CPP__TYPESUPPORT_HPP_

#include <cassert>
#include <string>

#include "fastcdr/Cdr.h"
#include "fastcdr/FastBuffer.h"
#include "rcutils/logging_macros.h"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"
#include "rosidl_runtime_c/string.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "rosidl_typesupport_introspection_c/service_introspection.h"
#include "rosidl_typesupport_introspection_c/visibility_control.h"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/service_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/visibility_control.h"

namespace rmw_fastrtps_dynamic_cpp {

// 辅助类，使用模板特化来读取/写入字符串类型到 eprosima::fastcdr::Cdr
// Helper class that uses template specialization to read/write string types to/from a
// eprosima::fastcdr::Cdr
template <typename MembersType>
struct StringHelper;

// 对于 C 语言内省类型支持，我们创建 std::string 的中间实例，以便
// eprosima::fastcdr::Cdr 可以正确处理字符串。
// For C introspection typesupport we create intermediate instances of std::string so that
// eprosima::fastcdr::Cdr can handle the string properly.
template <>
struct StringHelper<rosidl_typesupport_introspection_c__MessageMembers> {
  // 使用 rosidl_runtime_c__String 类型
  // Using type rosidl_runtime_c__String
  using type = rosidl_runtime_c__String;

  // 计算下一个字段的对齐大小
  // Calculate the alignment size for the next field
  static size_t next_field_align(void *data, size_t current_alignment) {
    auto c_string = static_cast<rosidl_runtime_c__String *>(data);
    if (!c_string) {
      RCUTILS_LOG_ERROR_NAMED(
          "rmw_fastrtps_dynamic_cpp", "Failed to cast data as rosidl_runtime_c__String");
      return current_alignment;
    }
    if (!c_string->data) {
      RCUTILS_LOG_ERROR_NAMED(
          "rmw_fastrtps_dynamic_cpp", "rosidl_generator_c_String had invalid data");
      return current_alignment;
    }

    current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
    current_alignment += 4;
    return current_alignment + strlen(c_string->data) + 1;
  }

  // 将 void * 类型的数据转换为 std::string
  // Convert void * type data to std::string
  static std::string convert_to_std_string(void *data) {
    auto c_string = static_cast<rosidl_runtime_c__String *>(data);
    if (!c_string) {
      RCUTILS_LOG_ERROR_NAMED(
          "rmw_fastrtps_dynamic_cpp", "Failed to cast data as rosidl_runtime_c__String");
      return "";
    }
    if (!c_string->data) {
      RCUTILS_LOG_ERROR_NAMED(
          "rmw_fastrtps_dynamic_cpp", "rosidl_generator_c_String had invalid data");
      return "";
    }
    return std::string(c_string->data);
  }

  // 将 rosidl_runtime_c__String 类型的数据转换为 std::string
  // Convert rosidl_runtime_c__String type data to std::string
  static std::string convert_to_std_string(rosidl_runtime_c__String &data) {
    return std::string(data.data);
  }

  // 将 eprosima::fastcdr::Cdr 类型的数据分配给 field
  // Assign eprosima::fastcdr::Cdr type data to field
  static void assign(eprosima::fastcdr::Cdr &deser, void *field) {
    std::string str;
    deser >> str;
    rosidl_runtime_c__String *c_str = static_cast<rosidl_runtime_c__String *>(field);
    rosidl_runtime_c__String__assign(c_str, str.c_str());
  }
};

// 对于 C++ 内省类型支持，我们只需透明地重用相同的 std::string。
// For C++ introspection typesupport, we just reuse the same std::string transparently.
template <>
struct StringHelper<rosidl_typesupport_introspection_cpp::MessageMembers> {
  // 使用 std::string 类型
  // Using type as std::string
  using type = std::string;

  // 将 void *data 转换为 std::string，并返回引用
  // Convert void *data to std::string and return its reference
  static std::string &convert_to_std_string(void *data) {
    return *(static_cast<std::string *>(data));
  }

  // 将 eprosima::fastcdr::Cdr 类型的 deser 分配给 field
  // Assign eprosima::fastcdr::Cdr type deser to field
  static void assign(eprosima::fastcdr::Cdr &deser, void *field) {
    std::string &str = *(std::string *)field;
    deser >> str;
  }
};

// TypeSupportProxy 类继承自 rmw_fastrtps_shared_cpp::TypeSupport
// TypeSupportProxy class inherits from rmw_fastrtps_shared_cpp::TypeSupport
class TypeSupportProxy : public rmw_fastrtps_shared_cpp::TypeSupport {
public:
  // 显式构造函数，接受一个 rmw_fastrtps_shared_cpp::TypeSupport 类型的指针
  // Explicit constructor, accepts a pointer of type rmw_fastrtps_shared_cpp::TypeSupport
  explicit TypeSupportProxy(rmw_fastrtps_shared_cpp::TypeSupport *inner_type);

  // 获取序列化后的大小估计
  // Get the estimated serialized size
  size_t getEstimatedSerializedSize(const void *ros_message, const void *impl) const override;

  // 序列化 ROS 消息
  // Serialize the ROS message
  bool serializeROSmessage(
      const void *ros_message, eprosima::fastcdr::Cdr &ser, const void *impl) const override;

  // 反序列化 ROS 消息
  // Deserialize the ROS message
  bool deserializeROSmessage(
      eprosima::fastcdr::Cdr &deser, void *ros_message, const void *impl) const override;
};

// BaseTypeSupport 类继承自 rmw_fastrtps_shared_cpp::TypeSupport
// BaseTypeSupport class inherits from rmw_fastrtps_shared_cpp::TypeSupport
class BaseTypeSupport : public rmw_fastrtps_shared_cpp::TypeSupport {
public:
  // 返回 ros_type_support_ 的指针
  // Return pointer of ros_type_support_
  const void *ros_type_support() const { return ros_type_support_; }

protected:
  // 显式构造函数，接受一个 void 类型的指针
  // Explicit constructor, accepts a pointer of type void
  explicit BaseTypeSupport(const void *ros_type_support) { ros_type_support_ = ros_type_support; }

private:
  // 私有成员变量，存储 ros_type_support 的指针
  // Private member variable, stores pointer of ros_type_support
  const void *ros_type_support_;
};

/**
 * @brief 类型支持模板类，用于序列化和反序列化 ROS 消息
 *        Type support template class for serializing and deserializing ROS messages
 *
 * @tparam MembersType 成员类型，通常是从 rosidl_typesupport_introspection_cpp::MessageMembers
 * 派生的类型 Member type, usually derived from rosidl_typesupport_introspection_cpp::MessageMembers
 */
template <typename MembersType>
class TypeSupport : public BaseTypeSupport {
public:
  /**
   * @brief 获取估计的序列化大小
   *        Get the estimated serialized size
   *
   * @param[in] ros_message ROS 消息指针
   *                        Pointer to the ROS message
   * @param[in] impl 实现细节指针
   *                 Pointer to implementation details
   * @return 序列化大小
   *         Serialized size
   */
  size_t getEstimatedSerializedSize(const void *ros_message, const void *impl) const override;

  /**
   * @brief 序列化 ROS 消息
   *        Serialize the ROS message
   *
   * @param[in] ros_message ROS 消息指针
   *                        Pointer to the ROS message
   * @param[out] ser Fast CDR 序列化对象
   *                 Fast CDR serialization object
   * @param[in] impl 实现细节指针
   *                 Pointer to implementation details
   * @return 是否成功序列化
   *         Whether the serialization was successful
   */
  bool serializeROSmessage(
      const void *ros_message, eprosima::fastcdr::Cdr &ser, const void *impl) const override;

  /**
   * @brief 反序列化 ROS 消息
   *        Deserialize the ROS message
   *
   * @param[in] deser Fast CDR 反序列化对象
   *                  Fast CDR deserialization object
   * @param[out] ros_message ROS 消息指针
   *                         Pointer to the ROS message
   * @param[in] impl 实现细节指针
   *                 Pointer to implementation details
   * @return 是否成功反序列化
   *         Whether the deserialization was successful
   */
  bool deserializeROSmessage(
      eprosima::fastcdr::Cdr &deser, void *ros_message, const void *impl) const override;

protected:
  /**
   * @brief 构造函数
   *        Constructor
   *
   * @param[in] ros_type_support ROS 类型支持指针
   *                             Pointer to ROS type support
   */
  explicit TypeSupport(const void *ros_type_support);

  /**
   * @brief 计算最大序列化大小
   *        Calculate the maximum serialized size
   *
   * @param[in] members 成员类型指针
   *                    Pointer to member type
   * @param[in] current_alignment 当前对齐值
   *                              Current alignment value
   * @return 最大序列化大小
   *         Maximum serialized size
   */
  size_t calculateMaxSerializedSize(const MembersType *members, size_t current_alignment);

  const MembersType *members_;  ///< 成员类型指针 Member type pointer

private:
  size_t getEstimatedSerializedSize(
      const MembersType *members, const void *ros_message, size_t current_alignment) const;

  bool serializeROSmessage(
      eprosima::fastcdr::Cdr &ser, const MembersType *members, const void *ros_message) const;

  bool deserializeROSmessage(
      eprosima::fastcdr::Cdr &deser, const MembersType *members, void *ros_message) const;
};

}  // namespace rmw_fastrtps_dynamic_cpp

#include "TypeSupport_impl.hpp"

#endif  // RMW_FASTRTPS_DYNAMIC_CPP__TYPESUPPORT_HPP_
