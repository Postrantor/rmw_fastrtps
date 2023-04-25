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

#ifndef RMW_FASTRTPS_DYNAMIC_CPP__TYPESUPPORT_IMPL_HPP_
#define RMW_FASTRTPS_DYNAMIC_CPP__TYPESUPPORT_IMPL_HPP_

#include <cassert>
#include <string>
#include <vector>

#include "fastcdr/Cdr.h"
#include "fastcdr/FastBuffer.h"
#include "fastcdr/exceptions/Exception.h"
#include "rmw/error_handling.h"
#include "rmw_fastrtps_dynamic_cpp/TypeSupport.hpp"
#include "rmw_fastrtps_dynamic_cpp/macros.hpp"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/u16string_functions.h"
#include "rosidl_typesupport_fastrtps_c/wstring_conversion.hpp"
#include "rosidl_typesupport_fastrtps_cpp/wstring_conversion.hpp"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "rosidl_typesupport_introspection_c/service_introspection.h"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/service_introspection.hpp"

namespace rmw_fastrtps_dynamic_cpp {

// 定义一个通用的C序列结构体模板
// Define a generic C sequence struct template
template <typename T>
struct GenericCSequence;

// 为不同的基本类型定义多个特化版本的GenericCSequence
// Define multiple specializations of GenericCSequence for various primitive types
SPECIALIZE_GENERIC_C_SEQUENCE(bool, bool)
SPECIALIZE_GENERIC_C_SEQUENCE(byte, uint8_t)
SPECIALIZE_GENERIC_C_SEQUENCE(char, char)
SPECIALIZE_GENERIC_C_SEQUENCE(float32, float)
SPECIALIZE_GENERIC_C_SEQUENCE(float64, double)
SPECIALIZE_GENERIC_C_SEQUENCE(int8, int8_t)
SPECIALIZE_GENERIC_C_SEQUENCE(int16, int16_t)
SPECIALIZE_GENERIC_C_SEQUENCE(uint16, uint16_t)
SPECIALIZE_GENERIC_C_SEQUENCE(int32, int32_t)
SPECIALIZE_GENERIC_C_SEQUENCE(uint32, uint32_t)
SPECIALIZE_GENERIC_C_SEQUENCE(int64, int64_t)
SPECIALIZE_GENERIC_C_SEQUENCE(uint64, uint64_t)

/**
 * @brief 构造函数，初始化TypeSupport对象
 * @param ros_type_support 指向ROS类型支持的指针
 *
 * @brief Constructor, initializes TypeSupport object
 * @param ros_type_support Pointer to ROS type support
 */
template <typename MembersType>
TypeSupport<MembersType>::TypeSupport(const void *ros_type_support)
    : BaseTypeSupport(ros_type_support) {
  m_isGetKeyDefined = false;
  max_size_bound_ = false;
  is_plain_ = false;
}

/**
 * @brief 序列化字段的C++特化版本
 * @param member 指向rosidl_typesupport_introspection_cpp::MessageMember的指针
 * @param field 需要序列化的字段的指针
 * @param ser 用于序列化的eprosima::fastcdr::Cdr对象引用
 *
 * @brief C++ specialization for serializing fields
 * @param member Pointer to rosidl_typesupport_introspection_cpp::MessageMember
 * @param field Pointer to the field to be serialized
 * @param ser Reference to eprosima::fastcdr::Cdr object used for serialization
 */
template <typename T>
void serialize_field(
    const rosidl_typesupport_introspection_cpp::MessageMember *member,
    void *field,
    eprosima::fastcdr::Cdr &ser) {
  // 如果字段不是数组，则直接序列化
  // If the field is not an array, serialize it directly
  if (!member->is_array_) {
    ser << *static_cast<T *>(field);
  } else if (member->array_size_ && !member->is_upper_bound_) {
    // 如果字段是固定大小的数组，则序列化整个数组
    // If the field is a fixed-size array, serialize the entire array
    ser.serializeArray(static_cast<T *>(field), member->array_size_);
  } else {
    // 如果字段是可变大小的数组，则使用std::vector进行序列化
    // If the field is a variable-size array, use std::vector for serialization
    std::vector<T> &data = *reinterpret_cast<std::vector<T> *>(field);
    ser << data;
  }
}

// 以下代码是在 ROS2 项目中 rmw_fastrtps_cpp 相关的代码，用于序列化 std::wstring 类型的字段。
// The following code is related to the rmw_fastrtps_cpp in the ROS2 project, used for serializing
// std::wstring type fields.
template <>
inline void serialize_field<std::wstring>(
    const rosidl_typesupport_introspection_cpp::MessageMember *member,
    void *field,
    eprosima::fastcdr::Cdr &ser) {
  // 定义一个 std::wstring 类型的变量 wstr
  // Define a variable of type std::wstring named wstr
  std::wstring wstr;

  // 如果成员不是数组类型
  // If the member is not an array type
  if (!member->is_array_) {
    // 将 field 转换为 std::u16string 类型的指针
    // Cast field to a pointer of type std::u16string
    auto u16str = static_cast<std::u16string *>(field);

    // 将 u16str 转换为宽字符串 wstr
    // Convert u16str to wide string wstr
    rosidl_typesupport_fastrtps_cpp::u16string_to_wstring(*u16str, wstr);

    // 将 wstr 序列化到 ser 中
    // Serialize wstr into ser
    ser << wstr;
  } else {
    // 定义一个 size_t 类型的变量 size
    // Define a variable of type size_t named size
    size_t size;

    // 如果成员具有固定大小且没有上限
    // If the member has a fixed size and no upper bound
    if (member->array_size_ && !member->is_upper_bound_) {
      size = member->array_size_;
    } else {
      // 获取数组大小并序列化数组大小
      // Get the array size and serialize the array size
      size = member->size_function(field);
      ser << static_cast<uint32_t>(size);
    }

    // 遍历数组中的每个元素
    // Iterate through each element in the array
    for (size_t i = 0; i < size; ++i) {
      // 获取数组中第 i 个元素的指针
      // Get the pointer to the i-th element in the array
      const void *element = member->get_const_function(field, i);

      // 将 element 转换为 std::u16string 类型的指针
      // Cast element to a pointer of type std::u16string
      auto u16str = static_cast<const std::u16string *>(element);

      // 将 u16str 转换为宽字符串 wstr
      // Convert u16str to wide string wstr
      rosidl_typesupport_fastrtps_cpp::u16string_to_wstring(*u16str, wstr);

      // 将 wstr 序列化到 ser 中
      // Serialize wstr into ser
      ser << wstr;
    }
  }
}

// C specialization
template <typename T>
void serialize_field(
    const rosidl_typesupport_introspection_c__MessageMember *member,
    void *field,
    eprosima::fastcdr::Cdr &ser) {
  // 如果成员不是数组类型
  // If the member is not an array type
  if (!member->is_array_) {
    // 将 field 转换为 T 类型的指针并序列化
    // Cast field to a pointer of type T and serialize it
    ser << *static_cast<T *>(field);
  } else if (member->array_size_ && !member->is_upper_bound_) {
    // 如果成员具有固定大小且没有上限，序列化数组
    // If the member has a fixed size and no upper bound, serialize the array
    ser.serializeArray(static_cast<T *>(field), member->array_size_);
  } else {
    // 获取数据并序列化序列
    // Get the data and serialize the sequence
    auto &data = *reinterpret_cast<typename GenericCSequence<T>::type *>(field);
    ser.serializeSequence(reinterpret_cast<T *>(data.data), data.size);
  }
}

/**
 * @brief 序列化 std::string 类型的字段 (Serialize a field of type std::string)
 *
 * @param[in] member 指向 rosidl_typesupport_introspection_c__MessageMember
 * 结构体的指针，包含了字段的元数据信息 (Pointer to the
 * rosidl_typesupport_introspection_c__MessageMember struct, which contains metadata about the
 * field)
 * @param[in] field 要序列化的字段的指针 (Pointer to the field to be serialized)
 * @param[out] ser 用于序列化的 eprosima::fastcdr::Cdr 对象 (eprosima::fastcdr::Cdr object used for
 * serialization)
 */
template <>
inline void serialize_field<std::string>(
    const rosidl_typesupport_introspection_c__MessageMember *member,
    void *field,
    eprosima::fastcdr::Cdr &ser) {
  // 使用 StringHelper 将 rosidl_runtime_c__String 转换为 std::string (Use StringHelper to convert
  // rosidl_runtime_c__String to std::string)
  using CStringHelper = StringHelper<rosidl_typesupport_introspection_c__MessageMembers>;

  // 如果字段不是数组类型 (If the field is not an array type)
  if (!member->is_array_) {
    // 将字段转换为 std::string 类型 (Convert the field to std::string type)
    auto &&str = CStringHelper::convert_to_std_string(field);

    // 控制字符串的最大长度 (Control the maximum length of the string)
    if (member->string_upper_bound_ && str.length() > member->string_upper_bound_ + 1) {
      throw std::runtime_error("string overcomes the maximum length");
    }

    // 将字符串序列化 (Serialize the string)
    ser << str;
  } else {
    // 如果字段是数组类型 (If the field is an array type)

    // 首先，将字段强制转换为 rosidl_generator_c 类型 (First, cast the field to rosidl_generator_c
    // type) 然后使用 StringHelper 将其转换为 std::string 并进行序列化 (Then convert it to a
    // std::string using StringHelper and serialize it)
    if (member->array_size_ && !member->is_upper_bound_) {
      // 在此处定义 tmpstring 而不是在下面，以避免在 for 循环的每次迭代中分配内存 (Define tmpstring
      // here instead of below to avoid memory allocation in every iteration of the for loop)
      std::string tmpstring;
      auto string_field = static_cast<rosidl_runtime_c__String *>(field);

      // 遍历数组并序列化每个元素 (Iterate through the array and serialize each element)
      for (size_t i = 0; i < member->array_size_; ++i) {
        tmpstring = string_field[i].data;
        ser.serialize(tmpstring);
      }
    } else {
      // 如果是无界数组 (If it's an unbounded array)
      auto &string_sequence_field = *reinterpret_cast<rosidl_runtime_c__String__Sequence *>(field);
      std::vector<std::string> cpp_string_vector;

      // 遍历数组并将每个元素添加到 std::vector 中 (Iterate through the array and add each element
      // to the std::vector)
      for (size_t i = 0; i < string_sequence_field.size; ++i) {
        cpp_string_vector.push_back(
            CStringHelper::convert_to_std_string(string_sequence_field.data[i]));
      }

      // 序列化整个 std::vector (Serialize the entire std::vector)
      ser << cpp_string_vector;
    }
  }
}

/**
 * @brief 序列化 std::wstring 类型的字段 (Serialize a field of type std::wstring)
 *
 * @param[in] member 指向 rosidl_typesupport_introspection_c__MessageMember
 * 结构体的指针，包含了字段的元信息 (Pointer to the
 * rosidl_typesupport_introspection_c__MessageMember struct, containing metadata about the field)
 * @param[in] field 要序列化的字段的指针 (Pointer to the field to be serialized)
 * @param[out] ser 用于序列化数据的 eprosima::fastcdr::Cdr 对象 (eprosima::fastcdr::Cdr object used
 * for serializing data)
 */
template <>
inline void serialize_field<std::wstring>(
    const rosidl_typesupport_introspection_c__MessageMember *member,
    void *field,
    eprosima::fastcdr::Cdr &ser) {
  // 定义一个 std::wstring 类型的变量 wstr (Define a variable of type std::wstring named wstr)
  std::wstring wstr;

  // 如果字段不是数组类型 (If the field is not an array type)
  if (!member->is_array_) {
    // 将 field 强制转换为 rosidl_runtime_c__U16String 类型的指针 (Cast field to a pointer of type
    // rosidl_runtime_c__U16String)
    auto u16str = static_cast<rosidl_runtime_c__U16String *>(field);
    // 将 u16str 转换为 wstr (Convert u16str to wstr)
    rosidl_typesupport_fastrtps_c::u16string_to_wstring(*u16str, wstr);
    // 将 wstr 序列化到 ser 中 (Serialize wstr into ser)
    ser << wstr;
  }
  // 如果字段是固定大小的数组类型 (If the field is a fixed-size array type)
  else if (member->array_size_ && !member->is_upper_bound_) {
    // 将 field 强制转换为 rosidl_runtime_c__U16String 类型的指针 (Cast field to a pointer of type
    // rosidl_runtime_c__U16String)
    auto array = static_cast<rosidl_runtime_c__U16String *>(field);
    // 遍历数组中的每个元素 (Iterate through each element in the array)
    for (size_t i = 0; i < member->array_size_; ++i) {
      // 将当前元素转换为 wstr (Convert the current element to wstr)
      rosidl_typesupport_fastrtps_c::u16string_to_wstring(array[i], wstr);
      // 将 wstr 序列化到 ser 中 (Serialize wstr into ser)
      ser << wstr;
    }
  }
  // 如果字段是可变大小的数组类型 (If the field is a variable-size array type)
  else {
    // 将 field 强制转换为 rosidl_runtime_c__U16String__Sequence 类型的指针 (Cast field to a pointer
    // of type rosidl_runtime_c__U16String__Sequence)
    auto sequence = static_cast<rosidl_runtime_c__U16String__Sequence *>(field);
    // 将序列的大小序列化到 ser 中 (Serialize the size of the sequence into ser)
    ser << static_cast<uint32_t>(sequence->size);
    // 遍历序列中的每个元素 (Iterate through each element in the sequence)
    for (size_t i = 0; i < sequence->size; ++i) {
      // 将当前元素转换为 wstr (Convert the current element to wstr)
      rosidl_typesupport_fastrtps_c::u16string_to_wstring(sequence->data[i], wstr);
      // 将 wstr 序列化到 ser 中 (Serialize wstr into ser)
      ser << wstr;
    }
  }
}

/**
 * @brief 序列化 ROS 消息
 * @tparam MembersType 成员类型
 * @param ser Fast CDR 序列化对象
 * @param members 成员类型指针
 * @param ros_message ROS 消息指针
 * @return 是否序列化成功
 *
 * @brief Serialize ROS message
 * @tparam MembersType Member type
 * @param ser Fast CDR serialization object
 * @param members Pointer to member type
 * @param ros_message Pointer to ROS message
 * @return Whether the serialization is successful
 */
template <typename MembersType>
bool TypeSupport<MembersType>::serializeROSmessage(
    eprosima::fastcdr::Cdr &ser, const MembersType *members, const void *ros_message) const {
  // 确保成员和消息不为空
  // Ensure members and message are not null
  assert(members);
  assert(ros_message);

  // 遍历所有成员
  // Iterate through all members
  for (uint32_t i = 0; i < members->member_count_; ++i) {
    // 获取当前成员
    // Get the current member
    const auto member = members->members_ + i;
    // 获取字段指针
    // Get field pointer
    void *field = const_cast<char *>(static_cast<const char *>(ros_message)) + member->offset_;
    // 根据类型 ID 进行处理
    // Process based on type ID
    switch (member->type_id_) {
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL:
        if (!member->is_array_) {
          // 不要在这里转换为 bool，因为如果 bool 未初始化，随机值无法反序列化
          // Don't cast to bool here because if the bool is uninitialized, the random value can't be
          // deserialized
          ser << (*static_cast<uint8_t *>(field) ? true : false);
        } else {
          serialize_field<bool>(member, field, ser);
        }
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
        serialize_field<uint8_t>(member, field, ser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
        serialize_field<char>(member, field, ser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT32:
        serialize_field<float>(member, field, ser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT64:
        serialize_field<double>(member, field, ser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
        serialize_field<int16_t>(member, field, ser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
        serialize_field<uint16_t>(member, field, ser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
        serialize_field<int32_t>(member, field, ser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
        serialize_field<uint32_t>(member, field, ser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
        serialize_field<int64_t>(member, field, ser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
        serialize_field<uint64_t>(member, field, ser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
        serialize_field<std::string>(member, field, ser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_WSTRING:
        serialize_field<std::wstring>(member, field, ser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE: {
        auto sub_members = static_cast<const MembersType *>(member->members_->data);
        if (!member->is_array_) {
          serializeROSmessage(ser, sub_members, field);
        } else {
          size_t array_size = 0;

          if (member->array_size_ && !member->is_upper_bound_) {
            array_size = member->array_size_;
          } else {
            if (!member->size_function) {
              RMW_SET_ERROR_MSG("unexpected error: size function is null");
              return false;
            }
            array_size = member->size_function(field);

            // 序列化长度
            // Serialize length
            ser << (uint32_t)array_size;
          }

          if (array_size != 0 && !member->get_function) {
            RMW_SET_ERROR_MSG("unexpected error: get_function function is null");
            return false;
          }
          for (size_t index = 0; index < array_size; ++index) {
            serializeROSmessage(ser, sub_members, member->get_function(field, index));
          }
        }
      } break;
      default:
        throw std::runtime_error("unknown type");
    }
  }

  return true;
}

/**
 * @brief 计算下一个字段的对齐大小 (Calculate the alignment size of the next field)
 *
 * @tparam T 字段类型 (Field type)
 * @param[in] member 指向消息成员的指针 (Pointer to the message member)
 * @param[in] field 指向字段数据的指针 (Pointer to the field data)
 * @param[in] current_alignment 当前对齐大小 (Current alignment size)
 * @return size_t 返回新的对齐大小 (Return the new alignment size)
 */
template <typename T>
size_t next_field_align(
    const rosidl_typesupport_introspection_cpp::MessageMember *member,
    void *field,
    size_t current_alignment) {
  // 定义填充大小为4 (Define padding size as 4)
  const size_t padding = 4;
  // 获取字段类型的大小 (Get the size of the field type)
  size_t item_size = sizeof(T);
  // 如果不是数组类型 (If it is not an array type)
  if (!member->is_array_) {
    // 计算当前对齐和字段大小之间的对齐 (Calculate the alignment between current alignment and field
    // size)
    current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
    // 更新当前对齐大小 (Update the current alignment size)
    current_alignment += item_size;
  } else if (member->array_size_ && !member->is_upper_bound_) {
    // 如果是固定大小的数组 (If it is a fixed-size array)
    // 计算当前对齐和字段大小之间的对齐 (Calculate the alignment between current alignment and field
    // size)
    current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
    // 更新当前对齐大小 (Update the current alignment size)
    current_alignment += item_size * member->array_size_;
  } else {
    // 如果是可变大小的数组 (If it is a variable-size array)
    // 强制转换字段数据为 std::vector<T> 类型 (Force cast the field data to std::vector<T> type)
    std::vector<T> &data = *reinterpret_cast<std::vector<T> *>(field);
    // 计算当前对齐和填充大小之间的对齐 (Calculate the alignment between current alignment and
    // padding size)
    current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    // 更新当前对齐大小 (Update the current alignment size)
    current_alignment += padding;
    // 如果数组不为空 (If the array is not empty)
    if (!data.empty()) {
      // 计算当前对齐和字段大小之间的对齐 (Calculate the alignment between current alignment and
      // field size)
      current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
      // 更新当前对齐大小 (Update the current alignment size)
      current_alignment += item_size * data.size();
    }
  }
  // 返回新的对齐大小 (Return the new alignment size)
  return current_alignment;
}

/**
 * @brief 计算下一个字符串字段的对齐大小 (Calculate the alignment size of the next string field)
 *
 * @tparam T 字符串类型 (String type)
 * @param[in] member 消息成员指针 (Pointer to the message member)
 * @param[in] field 字段指针 (Pointer to the field)
 * @param[in] current_alignment 当前对齐大小 (Current alignment size)
 * @return size_t 返回新的对齐大小 (Return the new alignment size)
 */
template <typename T>
size_t next_field_align_string(
    const rosidl_typesupport_introspection_cpp::MessageMember *member,
    void *field,
    size_t current_alignment) {
  // 定义填充大小为4 (Define padding size as 4)
  const size_t padding = 4;
  // 根据类型确定字符大小 (Determine character size based on type)
  size_t character_size =
      (member->type_id_ == rosidl_typesupport_introspection_cpp::ROS_TYPE_WSTRING) ? 4 : 1;
  // 如果不是数组类型 (If it's not an array type)
  if (!member->is_array_) {
    // 对齐当前对齐大小 (Align the current alignment size)
    current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    // 增加填充大小 (Add padding size)
    current_alignment += padding;
    // 获取字符串引用 (Get the reference to the string)
    auto &str = *static_cast<T *>(field);
    // 增加字符串长度和结束符的字符大小 (Add character size of the string length and the null
    // terminator)
    current_alignment += character_size * (str.size() + 1);
    // 如果是固定大小数组类型 (If it's a fixed-size array type)
  } else if (member->array_size_ && !member->is_upper_bound_) {
    // 获取字符串数组指针 (Get the pointer to the string array)
    auto str_arr = static_cast<T *>(field);
    // 遍历数组中的每个字符串 (Iterate through each string in the array)
    for (size_t index = 0; index < member->array_size_; ++index) {
      // 对齐当前对齐大小 (Align the current alignment size)
      current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
      // 增加填充大小 (Add padding size)
      current_alignment += padding;
      // 增加字符串长度和结束符的字符大小 (Add character size of the string length and the null
      // terminator)
      current_alignment += character_size * (str_arr[index].size() + 1);
    }
    // 如果是可变大小数组类型 (If it's a variable-size array type)
  } else {
    // 获取数据向量引用 (Get the reference to the data vector)
    auto &data = *reinterpret_cast<std::vector<T> *>(field);
    // 对齐当前对齐大小 (Align the current alignment size)
    current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    // 增加填充大小 (Add padding size)
    current_alignment += padding;
    // 遍历向量中的每个字符串 (Iterate through each string in the vector)
    for (auto &it : data) {
      // 对齐当前对齐大小 (Align the current alignment size)
      current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
      // 增加填充大小 (Add padding size)
      current_alignment += padding;
      // 增加字符串长度和结束符的字符大小 (Add character size of the string length and the null
      // terminator)
      current_alignment += character_size * (it.size() + 1);
    }
  }
  // 返回新的对齐大小 (Return the new alignment size)
  return current_alignment;
}

/**
 * @brief 计算下一个字段的对齐大小 (Calculate the alignment size of the next field)
 *
 * @tparam T 字段类型 (Field type)
 * @param[in] member 指向消息成员结构体的指针 (Pointer to the message member structure)
 * @param[in] field 指向字段数据的指针 (Pointer to the field data)
 * @param[in] current_alignment 当前对齐大小 (Current alignment size)
 * @return size_t 返回计算后的对齐大小 (Return the calculated alignment size)
 */
template <typename T>
size_t next_field_align(
    const rosidl_typesupport_introspection_c__MessageMember *member,
    void *field,
    size_t current_alignment) {
  // 定义填充大小为4 (Define padding size as 4)
  const size_t padding = 4;
  // 获取字段类型的大小 (Get the size of the field type)
  size_t item_size = sizeof(T);
  // 如果不是数组类型 (If it's not an array type)
  if (!member->is_array_) {
    // 计算当前对齐和字段大小之间的对齐值 (Calculate the alignment value between the current
    // alignment and the field size)
    current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
    // 更新当前对齐大小 (Update the current alignment size)
    current_alignment += item_size;
    // 如果是固定大小数组 (If it's a fixed-size array)
  } else if (member->array_size_ && !member->is_upper_bound_) {
    // 计算当前对齐和字段大小之间的对齐值 (Calculate the alignment value between the current
    // alignment and the field size)
    current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
    // 更新当前对齐大小 (Update the current alignment size)
    current_alignment += item_size * member->array_size_;
    // 如果是不定大小数组 (If it's a variable-size array)
  } else {
    // 计算当前对齐和填充大小之间的对齐值 (Calculate the alignment value between the current
    // alignment and the padding size)
    current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    // 更新当前对齐大小 (Update the current alignment size)
    current_alignment += padding;

    // 强制转换字段数据为 GenericCSequence 类型 (Force cast the field data to GenericCSequence type)
    auto &data = *reinterpret_cast<typename GenericCSequence<T>::type *>(field);
    // 计算当前对齐和字段大小之间的对齐值 (Calculate the alignment value between the current
    // alignment and the field size)
    current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
    // 更新当前对齐大小 (Update the current alignment size)
    current_alignment += item_size * data.size;
  }
  // 返回计算后的对齐大小 (Return the calculated alignment size)
  return current_alignment;
}

/**
 * @brief 计算下一个字符串字段的对齐大小 (Calculate the alignment size of the next string field)
 *
 * @tparam T 类型参数 (Type parameter)
 * @param[in] member 指向消息成员结构体的指针 (Pointer to the message member structure)
 * @param[in] field 字段数据的指针 (Pointer to the field data)
 * @param[in] current_alignment 当前对齐值 (Current alignment value)
 * @return size_t 返回新的对齐值 (Return the new alignment value)
 */
template <typename T>
size_t next_field_align_string(
    const rosidl_typesupport_introspection_c__MessageMember *member,
    void *field,
    size_t current_alignment);

// 显式实例化 next_field_align_string 函数模板，用于 std::string 类型 (Explicit instantiation of the
// next_field_align_string function template for std::string type)
template <>
inline size_t next_field_align_string<std::string>(
    const rosidl_typesupport_introspection_c__MessageMember *member,
    void *field,
    size_t current_alignment) {
  // 定义填充大小为 4 (Define padding size as 4)
  const size_t padding = 4;
  // 使用 StringHelper 类型别名，方便后续调用 (Use the StringHelper type alias for easier subsequent
  // calls)
  using CStringHelper = StringHelper<rosidl_typesupport_introspection_c__MessageMembers>;

  // 如果成员不是数组类型 (If the member is not an array type)
  if (!member->is_array_) {
    // 计算当前字段的对齐值 (Calculate the alignment value of the current field)
    current_alignment = CStringHelper::next_field_align(field, current_alignment);
  } else {
    // 如果成员是固定大小数组 (If the member is a fixed-size array)
    if (member->array_size_ && !member->is_upper_bound_) {
      // 将 field 转换为 rosidl_runtime_c__String 类型的指针 (Cast field to a pointer of type
      // rosidl_runtime_c__String)
      auto string_field = static_cast<rosidl_runtime_c__String *>(field);

      // 遍历数组中的每个元素 (Iterate through each element in the array)
      for (size_t i = 0; i < member->array_size_; ++i) {
        // 计算当前对齐值和填充大小之间的对齐关系 (Calculate the alignment relationship between the
        // current alignment value and padding size)
        current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
        // 增加填充大小 (Increase padding size)
        current_alignment += padding;
        // 增加字符串长度加 1 (Increase the string length by 1)
        current_alignment += strlen(string_field[i].data) + 1;
      }
    } else {
      // 计算当前对齐值和填充大小之间的对齐关系 (Calculate the alignment relationship between the
      // current alignment value and padding size)
      current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
      // 增加填充大小 (Increase padding size)
      current_alignment += padding;
      // 将 field 转换为 rosidl_runtime_c__String__Sequence 类型的引用 (Cast field to a reference of
      // type rosidl_runtime_c__String__Sequence)
      auto &string_sequence_field = *reinterpret_cast<rosidl_runtime_c__String__Sequence *>(field);

      // 遍历序列中的每个元素 (Iterate through each element in the sequence)
      for (size_t i = 0; i < string_sequence_field.size; ++i) {
        // 计算当前字段的对齐值 (Calculate the alignment value of the current field)
        current_alignment =
            CStringHelper::next_field_align(&(string_sequence_field.data[i]), current_alignment);
      }
    }
  }

  // 返回新的对齐值 (Return the new alignment value)
  return current_alignment;
}

/**
 * @brief 计算下一个字符串字段的对齐大小 (Calculate the alignment size of the next string field)
 *
 * @tparam std::wstring 字符串类型 (String type)
 * @param[in] member 消息成员指针 (Pointer to the message member)
 * @param[in] field 字段指针 (Pointer to the field)
 * @param[in] current_alignment 当前对齐大小 (Current alignment size)
 * @return size_t 返回新的对齐大小 (Return the new alignment size)
 */
template <>
inline size_t next_field_align_string<std::wstring>(
    const rosidl_typesupport_introspection_c__MessageMember *member,
    void *field,
    size_t current_alignment) {
  // 定义填充大小为4 (Define padding size as 4)
  const size_t padding = 4;

  // 如果不是数组类型 (If it's not an array type)
  if (!member->is_array_) {
    // 将字段转换为U16String类型 (Convert the field to U16String type)
    auto u16str = static_cast<rosidl_runtime_c__U16String *>(field);

    // 计算当前对齐和填充大小之间的对齐 (Calculate the alignment between current alignment and
    // padding size)
    current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

    // 增加填充大小 (Increase the padding size)
    current_alignment += padding;

    // 增加字符串大小乘以4，再加1 (Increase the string size multiplied by 4, plus 1)
    current_alignment += 4 * (u16str->size + 1);
  } else {
    // 如果是固定大小数组 (If it's a fixed-size array)
    if (member->array_size_ && !member->is_upper_bound_) {
      // 将字段转换为U16String类型 (Convert the field to U16String type)
      auto string_field = static_cast<rosidl_runtime_c__U16String *>(field);

      // 遍历数组中的每个元素 (Iterate through each element in the array)
      for (size_t i = 0; i < member->array_size_; ++i) {
        // 计算当前对齐和填充大小之间的对齐 (Calculate the alignment between current alignment and
        // padding size)
        current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

        // 增加填充大小 (Increase the padding size)
        current_alignment += padding;

        // 增加字符串大小乘以4，再加1 (Increase the string size multiplied by 4, plus 1)
        current_alignment += 4 * (string_field[i].size + 1);
      }
    } else {
      // 计算当前对齐和填充大小之间的对齐 (Calculate the alignment between current alignment and
      // padding size)
      current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

      // 增加填充大小 (Increase the padding size)
      current_alignment += padding;

      // 引用字符串序列字段 (Reference the string sequence field)
      auto &string_sequence_field =
          *reinterpret_cast<rosidl_runtime_c__U16String__Sequence *>(field);

      // 遍历序列中的每个元素 (Iterate through each element in the sequence)
      for (size_t i = 0; i < string_sequence_field.size; ++i) {
        // 计算当前对齐和填充大小之间的对齐 (Calculate the alignment between current alignment and
        // padding size)
        current_alignment += eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

        // 增加填充大小 (Increase the padding size)
        current_alignment += padding;

        // 增加字符串大小乘以4，再加1 (Increase the string size multiplied by 4, plus 1)
        current_alignment += 4 * (string_sequence_field.data[i].size + 1);
      }
    }
  }

  // 返回新的对齐大小 (Return the new alignment size)
  return current_alignment;
}

/**
 * @brief 计算序列化后的大小估计值 (Calculate the estimated serialized size)
 *
 * @tparam MembersType 成员类型 (Member type)
 * @param[in] members 成员指针 (Pointer to members)
 * @param[in] ros_message ROS 消息指针 (Pointer to ROS message)
 * @param[in] current_alignment 当前对齐大小 (Current alignment size)
 * @return size_t 序列化后的大小估计值 (Estimated serialized size after serialization)
 */
template <typename MembersType>
size_t TypeSupport<MembersType>::getEstimatedSerializedSize(
    const MembersType *members, const void *ros_message, size_t current_alignment) const {
  // 确保成员和 ROS 消息非空 (Ensure members and ROS message are not null)
  assert(members);
  assert(ros_message);

  // 初始化对齐值 (Initialize alignment value)
  size_t initial_alignment = current_alignment;

  // 遍历成员 (Iterate through members)
  for (uint32_t i = 0; i < members->member_count_; ++i) {
    // 获取当前成员 (Get the current member)
    const auto member = members->members_ + i;
    // 获取字段指针 (Get the field pointer)
    void *field = const_cast<char *>(static_cast<const char *>(ros_message)) + member->offset_;
    // 根据成员类型进行处理 (Process according to member type)
    switch (member->type_id_) {
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL:
        current_alignment = next_field_align<bool>(member, field, current_alignment);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
        current_alignment = next_field_align<uint8_t>(member, field, current_alignment);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
        current_alignment = next_field_align<char>(member, field, current_alignment);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT32:
        current_alignment = next_field_align<float>(member, field, current_alignment);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT64:
        current_alignment = next_field_align<double>(member, field, current_alignment);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
        current_alignment = next_field_align<int16_t>(member, field, current_alignment);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
        current_alignment = next_field_align<uint16_t>(member, field, current_alignment);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
        current_alignment = next_field_align<int32_t>(member, field, current_alignment);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
        current_alignment = next_field_align<uint32_t>(member, field, current_alignment);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
        current_alignment = next_field_align<int64_t>(member, field, current_alignment);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
        current_alignment = next_field_align<uint64_t>(member, field, current_alignment);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
        current_alignment = next_field_align_string<std::string>(member, field, current_alignment);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_WSTRING:
        current_alignment = next_field_align_string<std::wstring>(member, field, current_alignment);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE: {
        // 获取子成员 (Get sub-members)
        auto sub_members = static_cast<const MembersType *>(member->members_->data);
        if (!member->is_array_) {
          // 非数组类型 (Non-array type)
          current_alignment += getEstimatedSerializedSize(sub_members, field, current_alignment);
        } else {
          // 数组类型 (Array type)
          size_t array_size = 0;

          if (member->array_size_ && !member->is_upper_bound_) {
            array_size = member->array_size_;
          } else {
            if (!member->size_function) {
              RMW_SET_ERROR_MSG("unexpected error: size function is null");
              return false;
            }
            array_size = member->size_function(field);

            // Length serialization
            current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);
          }

          if (array_size != 0 && !member->get_function) {
            RMW_SET_ERROR_MSG("unexpected error: get_function function is null");
            return false;
          }
          // 遍历数组元素 (Iterate through array elements)
          for (size_t index = 0; index < array_size; ++index) {
            current_alignment += getEstimatedSerializedSize(
                sub_members, member->get_function(field, index), current_alignment);
          }
        }
      } break;
      default:
        throw std::runtime_error("unknown type");
    }
  }

  // 返回序列化后的大小估计值 (Return the estimated serialized size)
  return current_alignment - initial_alignment;
}

/**
 * @brief 反序列化字段 (Deserialize a field)
 *
 * @tparam T 字段类型 (Field type)
 * @param[in] member 指向消息成员的指针 (Pointer to the message member)
 * @param[out] field 要反序列化的字段 (Field to be deserialized)
 * @param[in,out] deser 反序列化对象 (Deserializer object)
 */
template <typename T>
void deserialize_field(
    const rosidl_typesupport_introspection_cpp::MessageMember *member,
    void *field,
    eprosima::fastcdr::Cdr &deser) {
  // 如果不是数组类型 (If not an array type)
  if (!member->is_array_) {
    // 反序列化单个值 (Deserialize single value)
    deser >> *static_cast<T *>(field);
  } else if (member->array_size_ && !member->is_upper_bound_) {  // 如果是固定大小的数组 (If it's a
                                                                 // fixed-size array)
    // 反序列化固定大小的数组 (Deserialize fixed-size array)
    deser.deserializeArray(static_cast<T *>(field), member->array_size_);
  } else {  // 如果是动态大小的数组 (If it's a dynamic-size array)
    // 反序列化动态大小的数组 (Deserialize dynamic-size array)
    auto &vector = *reinterpret_cast<std::vector<T> *>(field);
    deser >> vector;
  }
}

/**
 * @brief 反序列化 std::string 类型的字段 (Deserialize a field of type std::string)
 *
 * @param[in] member 指向消息成员的指针 (Pointer to the message member)
 * @param[out] field 要反序列化的字段 (Field to be deserialized)
 * @param[in,out] deser 反序列化对象 (Deserializer object)
 */
template <>
inline void deserialize_field<std::string>(
    const rosidl_typesupport_introspection_cpp::MessageMember *member,
    void *field,
    eprosima::fastcdr::Cdr &deser) {
  // 如果不是数组类型 (If not an array type)
  if (!member->is_array_) {
    // 反序列化单个字符串值 (Deserialize single string value)
    deser >> *static_cast<std::string *>(field);
  } else if (member->array_size_ && !member->is_upper_bound_) {  // 如果是固定大小的数组 (If it's a
                                                                 // fixed-size array)
    // 反序列化固定大小的字符串数组 (Deserialize fixed-size string array)
    std::string *array = static_cast<std::string *>(field);
    deser.deserializeArray(array, member->array_size_);
  } else {  // 如果是动态大小的数组 (If it's a dynamic-size array)
    // 反序列化动态大小的字符串数组 (Deserialize dynamic-size string array)
    auto &vector = *reinterpret_cast<std::vector<std::string> *>(field);
    deser >> vector;
  }
}

/**
 * @brief 反序列化 std::wstring 类型的字段 (Deserialize a field of type std::wstring)
 *
 * @param[in] member 指向消息成员的指针 (Pointer to the message member)
 * @param[out] field 存储反序列化数据的字段 (Field to store the deserialized data)
 * @param[in,out] deser 反序列化器对象 (Deserializer object)
 */
template <>
inline void deserialize_field<std::wstring>(
    const rosidl_typesupport_introspection_cpp::MessageMember *member,
    void *field,
    eprosima::fastcdr::Cdr &deser) {
  // 定义一个宽字符串变量 (Define a wide string variable)
  std::wstring wstr;
  // 如果不是数组类型 (If it's not an array type)
  if (!member->is_array_) {
    // 从反序列化器中读取宽字符串 (Read the wide string from the deserializer)
    deser >> wstr;
    // 将宽字符串转换为 UTF-16 字符串并存储到字段中 (Convert the wide string to a UTF-16 string and
    // store it in the field)
    rosidl_typesupport_fastrtps_cpp::wstring_to_u16string(
        wstr, *static_cast<std::u16string *>(field));
  } else {
    // 定义数组大小变量 (Define the array size variable)
    uint32_t size;
    // 如果有固定数组大小且没有上界限制 (If there is a fixed array size and no upper bound limit)
    if (member->array_size_ && !member->is_upper_bound_) {
      size = static_cast<uint32_t>(member->array_size_);
    } else {
      // 从反序列化器中读取数组大小 (Read the array size from the deserializer)
      deser >> size;
      // 调整字段的大小以容纳新的数据 (Resize the field to accommodate the new data)
      member->resize_function(field, size);
    }
    // 遍历数组元素 (Iterate through the array elements)
    for (size_t i = 0; i < size; ++i) {
      // 获取当前元素的指针 (Get the pointer to the current element)
      void *element = member->get_function(field, i);
      // 将元素转换为 UTF-16 字符串类型 (Cast the element to a UTF-16 string type)
      auto u16str = static_cast<std::u16string *>(element);
      // 从反序列化器中读取宽字符串 (Read the wide string from the deserializer)
      deser >> wstr;
      // 将宽字符串转换为 UTF-16 字符串并存储到当前元素中 (Convert the wide string to a UTF-16
      // string and store it in the current element)
      rosidl_typesupport_fastrtps_cpp::wstring_to_u16string(wstr, *u16str);
    }
  }
}

/**
 * @brief 反序列化给定类型 T 的字段 (Deserialize a field of given type T)
 *
 * @tparam T 字段的数据类型 (Data type of the field)
 * @param[in] member 指向消息成员的指针 (Pointer to the message member)
 * @param[out] field 存储反序列化数据的字段 (Field to store the deserialized data)
 * @param[in,out] deser 反序列化器对象 (Deserializer object)
 */
template <typename T>
void deserialize_field(
    const rosidl_typesupport_introspection_c__MessageMember *member,
    void *field,
    eprosima::fastcdr::Cdr &deser) {
  // 如果不是数组类型 (If it's not an array type)
  if (!member->is_array_) {
    // 从反序列化器中读取数据并存储到字段中 (Read the data from the deserializer and store it in the
    // field)
    deser >> *static_cast<T *>(field);
  } else if (member->array_size_ && !member->is_upper_bound_) {
    // 从反序列化器中读取固定大小的数组并存储到字段中 (Read the fixed-size array from the
    // deserializer and store it in the field)
    deser.deserializeArray(static_cast<T *>(field), member->array_size_);
  } else {
    // 获取字段的泛型 C 序列类型引用 (Get a reference to the generic C sequence type of the field)
    auto &data = *reinterpret_cast<typename GenericCSequence<T>::type *>(field);
    // 定义数组大小变量 (Define the array size variable)
    int32_t dsize = 0;
    // 从反序列化器中读取数组大小 (Read the array size from the deserializer)
    deser >> dsize;
    // 初始化泛型 C 序列并设置大小 (Initialize the generic C sequence and set its size)
    GenericCSequence<T>::init(&data, dsize);
    // 从反序列化器中读取数组数据并存储到泛型 C 序列中 (Read the array data from the deserializer
    // and store it in the generic C sequence)
    deser.deserializeArray(reinterpret_cast<T *>(data.data), dsize);
  }
}

/**
 * @brief 反序列化 std::string 类型的字段 (Deserialize a field of type std::string)
 *
 * @param[in] member 指向消息成员结构体的指针 (Pointer to the message member structure)
 * @param[out] field 指向要反序列化的字段的指针 (Pointer to the field to be deserialized)
 * @param[in,out] deser 反序列化对象 (Deserialization object)
 */
template <>
inline void deserialize_field<std::string>(
    const rosidl_typesupport_introspection_c__MessageMember *member,
    void *field,
    eprosima::fastcdr::Cdr &deser) {
  // 判断字段是否为数组 (Check if the field is an array)
  if (!member->is_array_) {
    // 使用 CStringHelper 类来分配字符串 (Use CStringHelper class to assign string)
    using CStringHelper = StringHelper<rosidl_typesupport_introspection_c__MessageMembers>;
    CStringHelper::assign(deser, field);
  } else {
    // 处理定长数组情况 (Handle fixed-size array case)
    if (member->array_size_ && !member->is_upper_bound_) {
      auto deser_field = static_cast<rosidl_runtime_c__String *>(field);
      // 在此处定义 tmpstring 而不是在下面，以避免在 for 循环的每次迭代中进行内存分配
      // (Define tmpstring here and not below to avoid memory allocation in every iteration of the
      // for loop)
      std::string tmpstring;
      for (size_t i = 0; i < member->array_size_; ++i) {
        deser.deserialize(tmpstring);
        if (!rosidl_runtime_c__String__assign(&deser_field[i], tmpstring.c_str())) {
          throw std::runtime_error("unable to assign rosidl_runtime_c__String");
        }
      }
    } else {
      // 处理不定长数组情况 (Handle variable-length array case)
      std::vector<std::string> cpp_string_vector;
      deser >> cpp_string_vector;

      auto &string_sequence_field = *reinterpret_cast<rosidl_runtime_c__String__Sequence *>(field);
      if (!rosidl_runtime_c__String__Sequence__init(
              &string_sequence_field, cpp_string_vector.size())) {
        throw std::runtime_error("unable to initialize rosidl_runtime_c__String array");
      }

      for (size_t i = 0; i < cpp_string_vector.size(); ++i) {
        if (!rosidl_runtime_c__String__assign(
                &string_sequence_field.data[i], cpp_string_vector[i].c_str())) {
          throw std::runtime_error("unable to assign rosidl_runtime_c__String");
        }
      }
    }
  }
}

/**
 * @brief 反序列化 std::wstring 类型的字段 (Deserialize a field of type std::wstring)
 *
 * @param[in] member 指向 rosidl_typesupport_introspection_c__MessageMember
 * 结构体的指针，包含了字段的元数据信息 (Pointer to the
 * rosidl_typesupport_introspection_c__MessageMember struct, containing metadata about the field)
 * @param[out] field 指向要反序列化的字段的内存空间 (Pointer to the memory space of the field to be
 * deserialized)
 * @param[in,out] deser 一个 eprosima::fastcdr::Cdr 对象，用于执行反序列化操作 (An
 * eprosima::fastcdr::Cdr object used for performing deserialization operations)
 */
template <>
inline void deserialize_field<std::wstring>(
    const rosidl_typesupport_introspection_c__MessageMember *member,
    void *field,
    eprosima::fastcdr::Cdr &deser) {
  // 定义一个 std::wstring 类型的变量 wstr (Define a variable of type std::wstring named wstr)
  std::wstring wstr;

  // 如果字段不是数组类型 (If the field is not an array type)
  if (!member->is_array_) {
    // 从 deser 中反序列化 wstr (Deserialize wstr from deser)
    deser >> wstr;
    // 将 wstr 转换为 U16String 类型，并将结果存储在 field 指向的内存空间中 (Convert wstr to
    // U16String type and store the result in the memory space pointed by field)
    rosidl_typesupport_fastrtps_c::wstring_to_u16string(
        wstr, *static_cast<rosidl_runtime_c__U16String *>(field));
  } else if (member->array_size_ && !member->is_upper_bound_) {
    // 如果字段是固定大小的数组类型 (If the field is a fixed-size array type)
    auto array = static_cast<rosidl_runtime_c__U16String *>(field);
    for (size_t i = 0; i < member->array_size_; ++i) {
      // 对于数组中的每个元素，从 deser 中反序列化 wstr (For each element in the array, deserialize
      // wstr from deser)
      deser >> wstr;
      // 将 wstr 转换为 U16String 类型，并将结果存储在 array[i] 中 (Convert wstr to U16String type
      // and store the result in array[i])
      rosidl_typesupport_fastrtps_c::wstring_to_u16string(wstr, array[i]);
    }
  } else {
    // 如果字段是动态大小的数组类型 (If the field is a dynamic-size array type)
    uint32_t size;
    // 从 deser 中反序列化数组的大小 (Deserialize the size of the array from deser)
    deser >> size;
    auto sequence = static_cast<rosidl_runtime_c__U16String__Sequence *>(field);
    // 初始化 sequence 的大小为 size (Initialize the size of sequence to be size)
    if (!rosidl_runtime_c__U16String__Sequence__init(sequence, size)) {
      // 如果无法初始化 sequence，则抛出运行时错误 (Throw a runtime error if unable to initialize
      // sequence)
      throw std::runtime_error("unable to initialize rosidl_runtime_c__U16String sequence");
    }
    for (size_t i = 0; i < sequence->size; ++i) {
      // 对于数组中的每个元素，从 deser 中反序列化 wstr (For each element in the array, deserialize
      // wstr from deser)
      deser >> wstr;
      // 将 wstr 转换为 U16String 类型，并将结果存储在 sequence->data[i] 中 (Convert wstr to
      // U16String type and store the result in sequence->data[i])
      rosidl_typesupport_fastrtps_c::wstring_to_u16string(wstr, sequence->data[i]);
    }
  }
}

/**
 * @brief 反序列化 ROS 消息
 * @tparam MembersType 成员类型
 * @param deser 反序列化对象
 * @param members 成员指针
 * @param ros_message ROS 消息指针
 * @return 反序列化是否成功
 *
 * @brief Deserialize ROS message
 * @tparam MembersType Member type
 * @param deser Deserialization object
 * @param members Pointer to members
 * @param ros_message Pointer to ROS message
 * @return Whether deserialization is successful
 */
template <typename MembersType>
bool TypeSupport<MembersType>::deserializeROSmessage(
    eprosima::fastcdr::Cdr &deser, const MembersType *members, void *ros_message) const {
  // 确保成员和 ROS 消息不为空
  // Ensure members and ROS message are not null
  assert(members);
  assert(ros_message);

  // 遍历成员并反序列化
  // Iterate through members and deserialize
  for (uint32_t i = 0; i < members->member_count_; ++i) {
    const auto *member = members->members_ + i;
    void *field = static_cast<char *>(ros_message) + member->offset_;
    switch (member->type_id_) {
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL:
        deserialize_field<bool>(member, field, deser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
        deserialize_field<uint8_t>(member, field, deser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
        deserialize_field<char>(member, field, deser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT32:
        deserialize_field<float>(member, field, deser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT64:
        deserialize_field<double>(member, field, deser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
        deserialize_field<int16_t>(member, field, deser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
        deserialize_field<uint16_t>(member, field, deser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
        deserialize_field<int32_t>(member, field, deser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
        deserialize_field<uint32_t>(member, field, deser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
        deserialize_field<int64_t>(member, field, deser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
        deserialize_field<uint64_t>(member, field, deser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
        deserialize_field<std::string>(member, field, deser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_WSTRING:
        deserialize_field<std::wstring>(member, field, deser);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE: {
        auto sub_members = static_cast<const MembersType *>(member->members_->data);
        if (!member->is_array_) {
          if (!deserializeROSmessage(deser, sub_members, field)) {
            return false;
          }
        } else {
          size_t array_size = 0;

          if (member->array_size_ && !member->is_upper_bound_) {
            array_size = member->array_size_;
          } else {
            uint32_t num_elems = 0;
            deser >> num_elems;
            array_size = static_cast<size_t>(num_elems);

            if (!member->resize_function) {
              RMW_SET_ERROR_MSG("unexpected error: resize function is null");
              return false;
            }
            member->resize_function(field, array_size);
          }

          if (array_size != 0 && !member->get_function) {
            RMW_SET_ERROR_MSG("unexpected error: get_function function is null");
            return false;
          }
          for (size_t index = 0; index < array_size; ++index) {
            if (!deserializeROSmessage(deser, sub_members, member->get_function(field, index))) {
              return false;
            }
          }
        }
      } break;
      default:
        throw std::runtime_error("unknown type");
    }
  }

  return true;
}

/**
 * @brief 计算序列化后的最大大小 (Calculate the maximum serialized size)
 *
 * @tparam MembersType 成员类型 (Member type)
 * @param[in] members 成员指针 (Pointer to the members)
 * @param[in] current_alignment 当前对齐值 (Current alignment value)
 * @return size_t 序列化后的最大大小 (Maximum serialized size)
 */
template <typename MembersType>
size_t TypeSupport<MembersType>::calculateMaxSerializedSize(
    const MembersType *members, size_t current_alignment) {
  // 确保成员不为空 (Ensure members are not null)
  assert(members);

  // 初始化对齐值 (Initialize alignment value)
  size_t initial_alignment = current_alignment;

  // 定义填充大小 (Define padding size)
  const size_t padding = 4;

  // 遍历所有成员 (Iterate through all members)
  for (uint32_t i = 0; i < members->member_count_; ++i) {
    // 获取当前成员 (Get the current member)
    const auto *member = members->members_ + i;

    // 初始化数组大小为1 (Initialize array size to 1)
    size_t array_size = 1;
    if (member->is_array_) {
      // 如果是数组，更新数组大小 (If it's an array, update the array size)
      array_size = member->array_size_;

      // 判断是否无界 (Check if it's unbounded)
      if (0u == array_size) {
        this->max_size_bound_ = false;
      }

      // 判断是否是序列 (Check if it's a sequence)
      if (0 == array_size || member->is_upper_bound_) {
        this->is_plain_ = false;
        current_alignment +=
            padding + eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
      }
    }

    // 根据成员类型进行处理 (Handle according to member type)
    switch (member->type_id_) {
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOL:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BYTE:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8:
        current_alignment += array_size * sizeof(int8_t);
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16:
        current_alignment += array_size * sizeof(uint16_t) +
                             eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint16_t));
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT32:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32:
        current_alignment += array_size * sizeof(uint32_t) +
                             eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint32_t));
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT64:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64:
        current_alignment += array_size * sizeof(uint64_t) +
                             eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint64_t));
        break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_WSTRING: {
        this->max_size_bound_ = false;
        this->is_plain_ = false;
        size_t character_size =
            (member->type_id_ == rosidl_typesupport_introspection_cpp::ROS_TYPE_WSTRING) ? 4 : 1;
        for (size_t index = 0; index < array_size; ++index) {
          current_alignment += padding +
                               eprosima::fastcdr::Cdr::alignment(current_alignment, padding) +
                               character_size * (member->string_upper_bound_ + 1);
        }
      } break;
      case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE: {
        auto sub_members = static_cast<const MembersType *>(member->members_->data);
        for (size_t index = 0; index < array_size; ++index) {
          current_alignment += calculateMaxSerializedSize(sub_members, current_alignment);
        }
      } break;
      default:
        throw std::runtime_error("unknown type");
    }
  }

  // 返回序列化后的最大大小 (Return the maximum serialized size)
  return current_alignment - initial_alignment;
}

/**
 * @brief 获取序列化后的消息大小估计值
 *        Get the estimated serialized size of the message.
 *
 * @tparam MembersType 成员类型
 *                     The type of the members.
 * @param ros_message  ROS 消息指针
 *                     Pointer to the ROS message.
 * @param impl         实现细节指针（未使用）
 *                     Pointer to implementation details (unused).
 * @return size_t      序列化后的消息大小估计值
 *                     Estimated serialized size of the message.
 */
template <typename MembersType>
size_t TypeSupport<MembersType>::getEstimatedSerializedSize(
    const void *ros_message, const void *impl) const {
  // 判断是否为简单类型
  // Check if it's a plain type.
  if (is_plain_) {
    return m_typeSize;
  }

  // 断言：确保 ros_message 和 members_ 不为空
  // Assert: Ensure ros_message and members_ are not null.
  assert(ros_message);
  assert(members_);

  // 初始化返回值为封装大小
  // Initialize return value with encapsulation size.
  size_t ret_val = 4;

  // 忽略实现细节参数
  // Ignore the implementation details parameter.
  (void)impl;
  // 如果成员数量不为0，调用 getEstimatedSerializedSize 计算序列化大小
  // If member count is not 0, call getEstimatedSerializedSize to calculate serialized size.
  if (members_->member_count_ != 0) {
    ret_val += TypeSupport::getEstimatedSerializedSize(members_, ros_message, 0);
  } else {
    ret_val += 1;
  }

  // 返回序列化大小
  // Return the serialized size.
  return ret_val;
}

/**
 * @brief 序列化 ROS 消息
 *        Serialize the ROS message.
 *
 * @tparam MembersType 成员类型
 *                     The type of the members.
 * @param ros_message  ROS 消息指针
 *                     Pointer to the ROS message.
 * @param ser          Fast CDR 序列化对象
 *                     Fast CDR serialization object.
 * @param impl         实现细节指针（未使用）
 *                     Pointer to implementation details (unused).
 * @return bool        序列化操作是否成功
 *                     Whether the serialization operation was successful.
 */
template <typename MembersType>
bool TypeSupport<MembersType>::serializeROSmessage(
    const void *ros_message, eprosima::fastcdr::Cdr &ser, const void *impl) const {
  // 断言：确保 ros_message 和 members_ 不为空
  // Assert: Ensure ros_message and members_ are not null.
  assert(ros_message);
  assert(members_);

  // 序列化封装
  // Serialize encapsulation.
  ser.serialize_encapsulation();

  // 忽略实现细节参数
  // Ignore the implementation details parameter.
  (void)impl;
  // 如果成员数量不为0，调用 serializeROSmessage 进行序列化
  // If member count is not 0, call serializeROSmessage to perform serialization.
  if (members_->member_count_ != 0) {
    TypeSupport::serializeROSmessage(ser, members_, ros_message);
  } else {
    ser << (uint8_t)0;
  }

  // 返回序列化操作成功
  // Return serialization operation success.
  return true;
}

/**
 * @brief 反序列化 ROS 消息 (Deserialize a ROS message)
 *
 * @tparam MembersType 成员类型 (Member types)
 * @param deser Fast CDR 反序列化对象 (Fast CDR deserialization object)
 * @param ros_message 存储反序列化数据的 ROS 消息指针 (Pointer to the ROS message where the
 * deserialized data will be stored)
 * @param impl 类型支持实现的指针 (Pointer to the type support implementation)
 * @return 反序列化是否成功 (Whether the deserialization was successful or not)
 */
template <typename MembersType>
bool TypeSupport<MembersType>::deserializeROSmessage(
    eprosima::fastcdr::Cdr &deser, void *ros_message, const void *impl) const {
  // 确保 ros_message 不为空 (Ensure that ros_message is not null)
  assert(ros_message);
  // 确保 members_ 不为空 (Ensure that members_ is not null)
  assert(members_);

  try {
    // 反序列化封装 (Deserialize encapsulation)
    deser.read_encapsulation();

    // 忽略 impl 参数 (Ignore the impl parameter)
    (void)impl;
    // 如果成员数量不为零，则继续反序列化 ROS 消息 (If the member count is not zero, continue
    // deserializing the ROS message)
    if (members_->member_count_ != 0) {
      return TypeSupport::deserializeROSmessage(deser, members_, ros_message);
    }

    // 读取并忽略一个字节 (Read and ignore a byte)
    uint8_t dump = 0;
    deser >> dump;
    (void)dump;
  } catch (const eprosima::fastcdr::exception::Exception &) {
    // 设置错误消息，表示 Fast CDR 反序列化异常 (Set error message indicating a Fast CDR
    // deserialization exception)
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Fast CDR exception deserializing message of type %s.", getName());
    return false;
  }

  // 反序列化成功 (Deserialization was successful)
  return true;
}

}  // namespace rmw_fastrtps_dynamic_cpp

#endif  // RMW_FASTRTPS_DYNAMIC_CPP__TYPESUPPORT_IMPL_HPP_
