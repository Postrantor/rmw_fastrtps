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

#include <cassert>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "fastcdr/Cdr.h"
#include "fastcdr/FastBuffer.h"
#include "fastdds/rtps/common/SerializedPayload.h"
#include "fastrtps/rtps/common/SerializedPayload.h"
#include "fastrtps/types/AnnotationParameterValue.h"
#include "fastrtps/types/DynamicData.h"
#include "fastrtps/types/DynamicPubSubType.h"
#include "fastrtps/types/TypeNamesGenerator.h"
#include "fastrtps/types/TypeObjectFactory.h"
#include "fastrtps/types/TypesBase.h"
#include "fastrtps/utils/md5.h"
#include "rmw/error_handling.h"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 构造函数 (Constructor)
 *
 * 初始化 TypeSupport 对象的成员变量。 (Initializes the member variables of the TypeSupport object.)
 */
TypeSupport::TypeSupport() {
  // 设置 m_isGetKeyDefined 为 false (Set m_isGetKeyDefined to false)
  m_isGetKeyDefined = false;

  // 设置 max_size_bound_ 为 false (Set max_size_bound_ to false)
  max_size_bound_ = false;

  // 设置 is_plain_ 为 false (Set is_plain_ to false)
  is_plain_ = false;

  // 自动填充类型对象，参数为 false (Auto-fill type object with parameter set to false)
  auto_fill_type_object(false);

  // 自动填充类型信息，参数为 false (Auto-fill type information with parameter set to false)
  auto_fill_type_information(false);
}

/**
 * @brief 删除数据 (Delete data)
 *
 * @param[in] data 要删除的数据指针 (Pointer to the data to be deleted)
 */
void TypeSupport::deleteData(void *data) {
  // 断言数据指针不为空 (Assert that the data pointer is not null)
  assert(data);

  // 删除 eprosima::fastcdr::FastBuffer 类型的数据 (Delete data of type
  // eprosima::fastcdr::FastBuffer)
  delete static_cast<eprosima::fastcdr::FastBuffer *>(data);
}

/**
 * @brief 创建数据 (Create data)
 *
 * @return 返回一个新创建的 eprosima::fastcdr::FastBuffer 类型的数据指针 (Returns a pointer to newly
 * created data of type eprosima::fastcdr::FastBuffer)
 */
void *TypeSupport::createData() {
  // 创建一个新的 eprosima::fastcdr::FastBuffer 类型的数据并返回其指针 (Create a new
  // eprosima::fastcdr::FastBuffer type data and return its pointer)
  return new eprosima::fastcdr::FastBuffer();
}

/**
 * @brief 序列化函数，将数据序列化为 SerializedPayload_t 类型
 * @param data 需要序列化的数据
 * @param payload 用于存储序列化后的数据的 SerializedPayload_t 对象指针
 * @return 成功序列化返回 true，否则返回 false
 *
 * @brief Serialize function, serializes data into SerializedPayload_t type
 * @param data Data to be serialized
 * @param payload Pointer to SerializedPayload_t object for storing serialized data
 * @return Returns true if serialization is successful, otherwise returns false
 */
bool TypeSupport::serialize(void *data, eprosima::fastrtps::rtps::SerializedPayload_t *payload) {
  // 检查输入参数是否有效
  // Check if input parameters are valid
  assert(data);
  assert(payload);

  // 将 void* 类型的 data 转换为 SerializedData* 类型
  // Cast void* type data to SerializedData* type
  auto ser_data = static_cast<SerializedData *>(data);

  // 根据 ser_data 的类型进行相应的序列化操作
  // Perform corresponding serialization operations according to the type of ser_data
  switch (ser_data->type) {
    case FASTRTPS_SERIALIZED_DATA_TYPE_ROS_MESSAGE: {
      // 创建 FastBuffer 对象，管理原始缓冲区
      // Create FastBuffer object that manages the raw buffer
      eprosima::fastcdr::FastBuffer fastbuffer(
          reinterpret_cast<char *>(payload->data), payload->max_size);
      // 创建 Cdr 对象，用于序列化数据
      // Create Cdr object for serializing data
      eprosima::fastcdr::Cdr ser(
          fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
      // 序列化 ROS 消息
      // Serialize ROS message
      if (this->serializeROSmessage(ser_data->data, ser, ser_data->impl)) {
        // 设置序列化后的数据的字节序
        // Set the byte order of the serialized data
        payload->encapsulation =
            ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
        // 设置序列化后的数据长度
        // Set the length of the serialized data
        payload->length = (uint32_t)ser.getSerializedDataLength();
        return true;
      }
      break;
    }

    case FASTRTPS_SERIALIZED_DATA_TYPE_CDR_BUFFER: {
      // 将 ser_data->data 转换为 eprosima::fastcdr::Cdr 类型
      // Cast ser_data->data to eprosima::fastcdr::Cdr type
      auto ser = static_cast<eprosima::fastcdr::Cdr *>(ser_data->data);
      // 检查序列化后的数据是否会超过最大容量
      // Check if the serialized data will exceed the maximum capacity
      if (payload->max_size >= ser->getSerializedDataLength()) {
        // 设置序列化后的数据长度
        // Set the length of the serialized data
        payload->length = static_cast<uint32_t>(ser->getSerializedDataLength());
        // 设置序列化后的数据的字节序
        // Set the byte order of the serialized data
        payload->encapsulation =
            ser->endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
        // 将序列化后的数据复制到 payload 的 data 成员中
        // Copy the serialized data to the data member of payload
        memcpy(payload->data, ser->getBufferPointer(), ser->getSerializedDataLength());
        return true;
      }
      break;
    }

    case FASTRTPS_SERIALIZED_DATA_TYPE_DYNAMIC_MESSAGE: {
      // 创建 DynamicPubSubType 对象
      // Create a DynamicPubSubType object
      auto m_type = std::make_shared<eprosima::fastrtps::types::DynamicPubSubType>();

      // 将 payload 序列化为存储在 data->data 中的动态数据
      // Serialize payload into dynamic data stored in data->data
      return m_type->serialize(
          static_cast<eprosima::fastrtps::types::DynamicData *>(ser_data->data), payload);
    }

    default:
      return false;
  }
  return false;
}

/**
 * @brief 反序列化函数，将 SerializedPayload_t 转换为相应的数据类型 (Deserialize function, converts
 * SerializedPayload_t to the corresponding data type)
 *
 * @param[in] payload 序列化后的负载数据 (Serialized payload data)
 * @param[out] data 反序列化后的数据 (Deserialized data)
 * @return bool 反序列化是否成功 (Whether deserialization is successful)
 */
bool TypeSupport::deserialize(eprosima::fastrtps::rtps::SerializedPayload_t *payload, void *data) {
  // 检查 data 是否为空指针 (Check if data is a null pointer)
  assert(data);
  // 检查 payload 是否为空指针 (Check if payload is a null pointer)
  assert(payload);

  // 将 data 强制转换为 SerializedData 类型 (Cast data to SerializedData type)
  auto ser_data = static_cast<SerializedData *>(data);

  // 根据 ser_data 的类型进行不同的反序列化操作 (Perform different deserialization operations based
  // on the type of ser_data)
  switch (ser_data->type) {
    // 如果是 ROS 消息类型 (If it's a ROS message type)
    case FASTRTPS_SERIALIZED_DATA_TYPE_ROS_MESSAGE: {
      // 创建 FastBuffer 对象，用于存储 payload 数据 (Create a FastBuffer object to store payload
      // data)
      eprosima::fastcdr::FastBuffer fastbuffer(
          reinterpret_cast<char *>(payload->data), payload->length);
      // 创建 Cdr 对象，用于反序列化操作 (Create a Cdr object for deserialization operation)
      eprosima::fastcdr::Cdr deser(
          fastbuffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);
      // 调用 deserializeROSmessage 函数进行反序列化 (Call the deserializeROSmessage function for
      // deserialization)
      return deserializeROSmessage(deser, ser_data->data, ser_data->impl);
    }

    // 如果是 CDR 缓冲区类型 (If it's a CDR buffer type)
    case FASTRTPS_SERIALIZED_DATA_TYPE_CDR_BUFFER: {
      // 将 ser_data->data 强制转换为 FastBuffer 类型 (Cast ser_data->data to FastBuffer type)
      auto buffer = static_cast<eprosima::fastcdr::FastBuffer *>(ser_data->data);
      // 检查是否能够分配足够的空间 (Check if enough space can be allocated)
      if (!buffer->reserve(payload->length)) {
        return false;
      }
      // 将 payload 数据复制到缓冲区中 (Copy payload data into the buffer)
      memcpy(buffer->getBuffer(), payload->data, payload->length);
      return true;
    }

    // 如果是动态消息类型 (If it's a dynamic message type)
    case FASTRTPS_SERIALIZED_DATA_TYPE_DYNAMIC_MESSAGE: {
      // 创建 DynamicPubSubType 对象 (Create a DynamicPubSubType object)
      auto m_type = std::make_shared<eprosima::fastrtps::types::DynamicPubSubType>();

      // 反序列化 payload 到 ser_data->data 中存储的动态数据中 (copies!) (Deserialize payload into
      // dynamic data stored in ser_data->data (copies!))
      return m_type->deserialize(
          payload, static_cast<eprosima::fastrtps::types::DynamicData *>(ser_data->data));
    }

    // 其他未知类型 (Other unknown types)
    default:
      return false;
  }
  return false;
}

/**
 * @brief 获取序列化大小提供者函数 (Get the serialized size provider function)
 *
 * @param data 传入的数据指针，不能为空 (Pointer to the input data, must not be null)
 * @return std::function<uint32_t()> 返回一个可调用的函数对象，该对象返回序列化数据的大小 (Returns a
 * callable function object that returns the size of the serialized data)
 */
std::function<uint32_t()> TypeSupport::getSerializedSizeProvider(void *data) {
  // 断言确保输入的数据指针不为空 (Assert to ensure the input data pointer is not null)
  assert(data);

  // 将输入数据转换为 SerializedData 类型的指针 (Cast the input data to a pointer of type
  // SerializedData)
  auto ser_data = static_cast<SerializedData *>(data);

  // 创建一个 lambda 函数，用于计算序列化数据的大小 (Create a lambda function to calculate the size
  // of the serialized data)
  auto ser_size = [this, ser_data]() -> uint32_t {
    // 判断序列化数据的类型是否为 FASTRTPS_SERIALIZED_DATA_TYPE_CDR_BUFFER (Check if the
    // serialization data type is FASTRTPS_SERIALIZED_DATA_TYPE_CDR_BUFFER)
    if (ser_data->type == FASTRTPS_SERIALIZED_DATA_TYPE_CDR_BUFFER) {
      // 将 ser_data->data 转换为 eprosima::fastcdr::Cdr 类型的指针 (Cast ser_data->data to a
      // pointer of type eprosima::fastcdr::Cdr)
      auto ser = static_cast<eprosima::fastcdr::Cdr *>(ser_data->data);

      // 返回序列化数据长度 (Return the length of the serialized data)
      return static_cast<uint32_t>(ser->getSerializedDataLength());
    }

    // 如果不是 FASTRTPS_SERIALIZED_DATA_TYPE_CDR_BUFFER 类型，则使用 getEstimatedSerializedSize
    // 计算序列化大小 (If it's not of type FASTRTPS_SERIALIZED_DATA_TYPE_CDR_BUFFER, use
    // getEstimatedSerializedSize to calculate the serialization size)
    return static_cast<uint32_t>(this->getEstimatedSerializedSize(ser_data->data, ser_data->impl));
  };

  // 返回可调用的 lambda 函数 (Return the callable lambda function)
  return ser_size;
}

// TODO(iuhilnehc-ynos): add the following content into new files named TypeObject?
using CompleteStructType = eprosima::fastrtps::types::CompleteStructType;
using CompleteStructMember = eprosima::fastrtps::types::CompleteStructMember;
using MinimalStructType = eprosima::fastrtps::types::MinimalStructType;
using MinimalStructMember = eprosima::fastrtps::types::MinimalStructMember;
using SerializedPayload_t = eprosima::fastrtps::rtps::SerializedPayload_t;
using TypeNamesGenerator = eprosima::fastrtps::types::TypeNamesGenerator;
using TypeIdentifier = eprosima::fastrtps::types::TypeIdentifier;
using TypeObject = eprosima::fastrtps::types::TypeObject;
using TypeObjectFactory = eprosima::fastrtps::types::TypeObjectFactory;

/**
 * @brief 获取类型支持的内省信息 (Get the introspection information of the type support)
 *
 * @param[in] type_supports 指向 rosidl_message_type_support_t 结构体的指针 (Pointer to the
 * rosidl_message_type_support_t structure)
 * @return 返回一个指向 rosidl_message_type_support_t 结构体的指针，如果找不到，则返回 nullptr
 * (Returns a pointer to the rosidl_message_type_support_t structure, or nullptr if not found)
 */
const rosidl_message_type_support_t *get_type_support_introspection(
    const rosidl_message_type_support_t *type_supports) {
  // 尝试获取 C 语言实现的类型支持句柄 (Try to get the type support handle implemented in C
  // language)
  const rosidl_message_type_support_t *type_support =
      get_message_typesupport_handle(type_supports, rosidl_typesupport_introspection_c__identifier);

  // 如果类型支持句柄为空，则尝试获取 C++ 语言实现的类型支持句柄 (If the type support handle is
  // null, try to get the type support handle implemented in C++ language)
  if (nullptr == type_support) {
    // 获取先前的错误字符串 (Get the previous error string)
    rcutils_error_string_t prev_error_string = rcutils_get_error_string();
    // 重置错误 (Reset the error)
    rcutils_reset_error();

    // 尝试获取 C++ 语言实现的类型支持句柄 (Try to get the type support handle implemented in C++
    // language)
    type_support = get_message_typesupport_handle(
        type_supports, rosidl_typesupport_introspection_cpp::typesupport_identifier);

    // 如果类型支持句柄仍然为空，则设置错误信息 (If the type support handle is still null, set the
    // error message)
    if (nullptr == type_support) {
      // 获取错误字符串 (Get the error string)
      rcutils_error_string_t error_string = rcutils_get_error_string();
      // 重置错误 (Reset the error)
      rcutils_reset_error();

      // 设置错误信息，包括先前的错误字符串和当前的错误字符串 (Set the error message, including the
      // previous error string and the current error string)
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "Type support not from this implementation. Got:\n"
          "    %s\n"
          "    %s\n"
          "while fetching it",
          prev_error_string.str, error_string.str);
      return nullptr;
    }
  }

  // 返回找到的类型支持句柄 (Return the found type support handle)
  return type_support;
}

/**
 * @brief 创建类型名称字符串 (Create a type name string)
 *
 * @tparam MembersType 成员类型 (Member type)
 * @param[in] members 成员指针 (Pointer to the members)
 * @return std::string 类型名称字符串 (Type name string)
 */
template <typename MembersType>
inline std::string _create_type_name(const MembersType *members) {
  // 如果成员指针为空，返回空字符串 (If the member pointer is null, return an empty string)
  if (!members) {
    return std::string();
  }

  // 创建一个字符串输出流 (Create a string output stream)
  std::ostringstream ss;
  // 获取消息命名空间 (Get the message namespace)
  std::string message_namespace(members->message_namespace_);
  // 获取消息名称 (Get the message name)
  std::string message_name(members->message_name_);
  // 如果消息命名空间不为空，在输出流中添加命名空间和双冒号分隔符 (If the message namespace is not
  // empty, add the namespace and double colon separator to the output stream)
  if (!message_namespace.empty()) {
    ss << message_namespace << "::";
  }
  // 在输出流中添加 "dds_::" 前缀和消息名称后缀 (Add the "dds_::" prefix and message name suffix to
  // the output stream)
  ss << "dds_::" << message_name << "_";
  // 返回类型名称字符串 (Return the type name string)
  return ss.str();
}

// 定义成员标识符名称类型 (Define the MemberIdentifierName type)
typedef std::pair<const TypeIdentifier *, std::string> MemberIdentifierName;

/**
 * @brief 获取类型标识符 (Get the type identifier)
 *
 * @tparam MembersType 成员类型 (Member type)
 * @param[in] member 成员指针 (Pointer to the member)
 * @param[in] index 索引值 (Index value)
 * @param[in] complete 是否完整 (Whether it is complete)
 * @return MemberIdentifierName 类型标识符和名称 (Type identifier and name)
 */
template <typename MembersType>
MemberIdentifierName GetTypeIdentifier(const MembersType *member, uint32_t index, bool complete);

/**
 * @brief 获取完整的 TypeObject
 * @tparam MembersType 成员类型
 * @param type_name 类型名称
 * @param members 成员指针
 * @return 完整的 TypeObject 指针
 *
 * @brief Get the complete TypeObject
 * @tparam MembersType Member type
 * @param type_name Type name
 * @param members Pointer to the members
 * @return Pointer to the complete TypeObject
 */
template <typename MembersType>
const TypeObject *GetCompleteObject(const std::string &type_name, const MembersType *members) {
  // 尝试从 TypeObjectFactory 中获取已存在的完整 TypeObject
  // Try to get the existing complete TypeObject from the TypeObjectFactory
  const TypeObject *c_type_object =
      TypeObjectFactory::get_instance()->get_type_object(type_name, true);
  // 如果已存在并且为完整类型，则直接返回
  // If it exists and is a complete type, return it directly
  if (c_type_object != nullptr && c_type_object->_d() == eprosima::fastrtps::types::EK_COMPLETE) {
    return c_type_object;
  }

  // 创建一个新的 TypeObject
  // Create a new TypeObject
  TypeObject *type_object = new TypeObject();

  // 设置 TypeObject 为完整类型
  // Set the TypeObject as a complete type
  type_object->_d(eprosima::fastrtps::types::EK_COMPLETE);
  // 设置结构体类型
  // Set the structure type
  type_object->complete()._d(eprosima::fastrtps::types::TK_STRUCTURE);
  // 设置结构体标志位
  // Set the structure flags
  type_object->complete().struct_type().struct_flags().IS_FINAL(false);
  type_object->complete().struct_type().struct_flags().IS_APPENDABLE(false);
  type_object->complete().struct_type().struct_flags().IS_MUTABLE(false);
  // 不确定当前类型是否为嵌套类型，将所有类型设置为嵌套类型
  // Not sure whether the current type is nested or not, make all types nested
  type_object->complete().struct_type().struct_flags().IS_NESTED(true);
  // 设置不支持的标志位
  // Set unsupported flags
  type_object->complete().struct_type().struct_flags().IS_AUTOID_HASH(false);  // Unsupported

  // 遍历成员并添加到 TypeObject 中
  // Iterate through members and add them to the TypeObject
  for (uint32_t i = 0; i < members->member_count_; ++i) {
    CompleteStructMember cst_field;
    // 设置成员 ID
    // Set member ID
    cst_field.common().member_id(i);
    // 设置不支持的成员标志位
    // Set unsupported member flags
    cst_field.common().member_flags().TRY_CONSTRUCT1(false);  // Unsupported
    cst_field.common().member_flags().TRY_CONSTRUCT2(false);  // Unsupported
    cst_field.common().member_flags().IS_EXTERNAL(false);     // Unsupported
    // 设置其他成员标志位
    // Set other member flags
    cst_field.common().member_flags().IS_OPTIONAL(false);
    cst_field.common().member_flags().IS_MUST_UNDERSTAND(false);
    cst_field.common().member_flags().IS_KEY(false);
    cst_field.common().member_flags().IS_DEFAULT(false);  // Doesn't apply

    // 获取类型标识符和名称
    // Get the type identifier and name
    MemberIdentifierName pair = GetTypeIdentifier(members, i, true);
    if (!pair.first) {
      continue;
    }
    cst_field.common().member_type_id(*pair.first);
    cst_field.detail().name(pair.second);
    // 将成员添加到 TypeObject 的结构体中
    // Add the member to the TypeObject's structure
    type_object->complete().struct_type().member_seq().emplace_back(cst_field);
  }

  // 设置头部信息
  // Set header information
  type_object->complete().struct_type().header().detail().type_name(type_name);

  // 创建类型标识符
  // Create a type identifier
  TypeIdentifier identifier;
  identifier._d(eprosima::fastrtps::types::EK_COMPLETE);

  // 序列化 TypeObject
  // Serialize the TypeObject
  SerializedPayload_t payload(static_cast<uint32_t>(
      CompleteStructType::getCdrSerializedSize(type_object->complete().struct_type()) + 4));

  eprosima::fastcdr::FastBuffer fastbuffer(
      reinterpret_cast<char *>(payload.data), payload.max_size);

  // 使用固定的字节序 (参考文档：Extensible and Dynamic Topic Types for DDS)
  // Use fixed endianness (Reference: Extensible and Dynamic Topic Types for DDS)
  eprosima::fastcdr::Cdr ser(
      fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
      eprosima::fastcdr::Cdr::DDS_CDR);  // Object that serializes the data.
  payload.encapsulation = CDR_LE;

  // 序列化 TypeObject
  // Serialize the TypeObject
  type_object->serialize(ser);
  // 获取序列化后的长度
  // Get the serialized length
  payload.length = static_cast<uint32_t>(ser.getSerializedDataLength());

  // 计算 MD5 哈希值
  // Calculate the MD5 hash
  MD5 objectHash;
  objectHash.update(reinterpret_cast<char *>(payload.data), payload.length);
  objectHash.finalize();
  for (int i = 0; i < 14; ++i) {
    identifier.equivalence_hash()[i] = objectHash.digest[i];
  }

  // 将新创建的 TypeObject 添加到 TypeObjectFactory 中
  // Add the newly created TypeObject to the TypeObjectFactory
  TypeObjectFactory::get_instance()->add_type_object(type_name, &identifier, type_object);
  delete type_object;

  return TypeObjectFactory::get_instance()->get_type_object(type_name, true);
}

/**
 * @brief 获取指定类型的最小 TypeObject
 * @tparam MembersType 成员类型
 * @param type_name 类型名称
 * @param members 成员指针
 * @return 指向 TypeObject 的指针
 *
 * @brief Get the minimal TypeObject for a specific type
 * @tparam MembersType The member type
 * @param type_name The name of the type
 * @param members Pointer to the members
 * @return Pointer to the TypeObject
 */
template <typename MembersType>
const TypeObject *GetMinimalObject(const std::string &type_name, const MembersType *members) {
  // 尝试从 TypeObjectFactory 获取已存在的 TypeObject
  // Try to get an existing TypeObject from the TypeObjectFactory
  const TypeObject *c_type_object =
      TypeObjectFactory::get_instance()->get_type_object(type_name, false);
  if (c_type_object != nullptr) {
    return c_type_object;
  }

  // 创建一个新的 TypeObject
  // Create a new TypeObject
  TypeObject *type_object = new TypeObject();
  type_object->_d(eprosima::fastrtps::types::EK_MINIMAL);
  type_object->minimal()._d(eprosima::fastrtps::types::TK_STRUCTURE);
  type_object->minimal().struct_type().struct_flags().IS_FINAL(false);
  type_object->minimal().struct_type().struct_flags().IS_APPENDABLE(false);
  type_object->minimal().struct_type().struct_flags().IS_MUTABLE(false);
  type_object->minimal().struct_type().struct_flags().IS_NESTED(true);
  type_object->minimal().struct_type().struct_flags().IS_AUTOID_HASH(false);  // Unsupported

  // 遍历成员并设置相应的字段
  // Iterate through the members and set the corresponding fields
  for (uint32_t i = 0; i < members->member_count_; ++i) {
    MinimalStructMember mst_field;
    mst_field.common().member_id(i);
    mst_field.common().member_flags().TRY_CONSTRUCT1(false);  // Unsupported
    mst_field.common().member_flags().TRY_CONSTRUCT2(false);  // Unsupported
    mst_field.common().member_flags().IS_EXTERNAL(false);     // Unsupported
    mst_field.common().member_flags().IS_OPTIONAL(false);
    mst_field.common().member_flags().IS_MUST_UNDERSTAND(false);
    mst_field.common().member_flags().IS_KEY(false);
    mst_field.common().member_flags().IS_DEFAULT(false);  // Doesn't apply

    MemberIdentifierName pair = GetTypeIdentifier(members, i, false);
    if (!pair.first) {
      continue;
    }
    mst_field.common().member_type_id(*pair.first);
    MD5 field_hash(pair.second);
    for (int i = 0; i < 4; ++i) {
      mst_field.detail().name_hash()[i] = field_hash.digest[i];
    }
    type_object->minimal().struct_type().member_seq().emplace_back(mst_field);
  }

  TypeIdentifier identifier;
  identifier._d(eprosima::fastrtps::types::EK_MINIMAL);

  SerializedPayload_t payload(static_cast<uint32_t>(
      MinimalStructType::getCdrSerializedSize(type_object->minimal().struct_type()) + 4));

  eprosima::fastcdr::FastBuffer fastbuffer(
      reinterpret_cast<char *>(payload.data), payload.max_size);

  // 固定字节序（参考 Extensible and Dynamic Topic Types for DDS 文档的第 221 页，EquivalenceHash
  // 定义） Fixed endian (Page 221, EquivalenceHash definition of Extensible and Dynamic Topic Types
  // for DDS document)
  eprosima::fastcdr::Cdr ser(
      fastbuffer, eprosima::fastcdr::Cdr::LITTLE_ENDIANNESS,
      eprosima::fastcdr::Cdr::DDS_CDR);  // Object that serializes the data.
  payload.encapsulation = CDR_LE;

  type_object->serialize(ser);
  payload.length =
      static_cast<uint32_t>(ser.getSerializedDataLength());  // Get the serialized length
  MD5 objectHash;
  objectHash.update(reinterpret_cast<char *>(payload.data), payload.length);
  objectHash.finalize();
  for (int i = 0; i < 14; ++i) {
    identifier.equivalence_hash()[i] = objectHash.digest[i];
  }

  // 将新创建的 TypeObject 添加到 TypeObjectFactory
  // Add the newly created TypeObject to the TypeObjectFactory
  TypeObjectFactory::get_instance()->add_type_object(type_name, &identifier, type_object);
  delete type_object;
  return TypeObjectFactory::get_instance()->get_type_object(type_name, false);
}

/**
 * @brief 获取类型标识符 (Get type identifier)
 *
 * @tparam MembersType 成员类型 (Member type)
 * @param[in] members 指向成员类型的指针 (Pointer to the member type)
 * @param[in] index 成员索引 (Member index)
 * @param[in] complete 是否为完整类型 (Whether it is a complete type)
 * @return MemberIdentifierName 类型标识符名称 (Type identifier name)
 */
template <typename MembersType>
MemberIdentifierName GetTypeIdentifier(const MembersType *members, uint32_t index, bool complete) {
  // 获取成员指针 (Get member pointer)
  const auto member = members->members_ + index;

  // 初始化类型标识符指针 (Initialize type identifier pointer)
  const TypeIdentifier *type_identifier = nullptr;

  // 获取成员名称 (Get member name)
  std::string name = member->name_;

  // 初始化类型名称和完整类型标志 (Initialize type name and complete type flag)
  std::string type_name;
  bool complete_type = false;

  // 根据成员类型 ID 确定类型名称 (Determine type name based on member type ID)
  switch (member->type_id_) {
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT: {
      type_name = "float";
      break;
    }
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE: {
      type_name = "double";
      break;
    }
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_LONG_DOUBLE: {
      type_name = "longdouble";
      break;
    }
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_CHAR: {
      type_name = "char";
      break;
    }
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_WCHAR: {
      type_name = "wchar";
      break;
    }
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_BOOLEAN: {
      type_name = "bool";
      break;
    }
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_OCTET:
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8: {
      type_name = "uint8_t";
      break;
    }
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT8: {
      type_name = "int8_t";
      break;
    }
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT16: {
      type_name = "uint16_t";
      break;
    }
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT16: {
      type_name = "int16_t";
      break;
    }
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT32: {
      type_name = "uint32_t";
      break;
    }
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32: {
      type_name = "int32_t";
      break;
    }
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT64: {
      type_name = "uint64_t";
      break;
    }
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT64: {
      type_name = "int64_t";
      break;
    }
    // 根据不同的 ROS 类型处理数据 (Handle data according to different ROS types)
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING:
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_WSTRING: {
      // 获取字符串的上界，如果没有定义，则默认为 255 (Get the upper bound of the string, if not
      // defined, default to 255)
      uint32_t bound =
          member->string_upper_bound_ ? static_cast<uint32_t>(member->string_upper_bound_) : 255;
      // 判断是否为宽字符串类型 (Determine if it is a wide string type)
      bool wide = (member->type_id_ == ::rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING)
                      ? false
                      : true;
      // 获取字符串类型的标识符 (Get the identifier of the string type)
      TypeObjectFactory::get_instance()->get_string_identifier(bound, wide);
      // 获取字符串类型的名称 (Get the name of the string type)
      type_name = TypeNamesGenerator::get_string_type_name(bound, wide);
      break;
    }
    // 处理 ROS 消息类型 (Handle ROS message type)
    case ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE: {
      // 获取消息类型支持的内省信息 (Get introspection information for message type support)
      const rosidl_message_type_support_t *type_support_intro =
          get_type_support_introspection(member->members_);
      // 转换为 MembersType 类型 (Cast to MembersType type)
      const MembersType *sub_members = static_cast<const MembersType *>(type_support_intro->data);
      // 创建子类型名称 (Create sub-type name)
      std::string sub_type_name = _create_type_name(sub_members);
      // 判断是否为完整类型 (Determine if it is a complete type)
      if (complete) {
        GetCompleteObject(sub_type_name, sub_members);
      } else {
        GetMinimalObject(sub_type_name, sub_members);
      }
      // 设置类型名称 (Set the type name)
      type_name = sub_type_name;
      // 设置完整类型标志 (Set the complete type flag)
      complete_type = complete;
    } break;
    // 其他情况不处理 (Do not handle other cases)
    default:
      break;
  }

  // 如果类型名称不为空。If the type name is not empty.
  if (!type_name.empty()) {
    // 如果成员不是数组。If the member is not an array.
    if (!member->is_array_) {
      // 获取对应类型的 TypeIdentifier。Get the TypeIdentifier for the corresponding type.
      type_identifier =
          TypeObjectFactory::get_instance()->get_type_identifier(type_name, complete_type);
    }
    // 如果成员是固定大小的数组。If the member is a fixed-size array.
    else if (member->array_size_ && !member->is_upper_bound_) {
      // 获取数组类型的 TypeIdentifier。Get the TypeIdentifier for the array type.
      type_identifier = TypeObjectFactory::get_instance()->get_array_identifier(
          type_name, {static_cast<uint32_t>(member->array_size_)}, complete_type);
    }
    // 如果成员是不定大小的数组。If the member is a variable-size array.
    else {
      // 获取序列类型的 TypeIdentifier。Get the TypeIdentifier for the sequence type.
      type_identifier =
          TypeObjectFactory::get_instance()->get_sequence_identifier(type_name, 0, complete_type);
    }
  }

  // 返回 TypeIdentifier 和成员名称。Return the TypeIdentifier and member name.
  return {type_identifier, member->name_};
}

/**
 * @brief 获取类型对象 (Get the TypeObject)
 *
 * @tparam MembersType 成员类型 (Member type)
 * @param type_name 类型名称 (Type name)
 * @param complete 是否获取完整的类型对象 (Whether to get the complete TypeObject)
 * @param members 类型成员指针 (Pointer to the type members)
 * @return 返回类型对象指针 (Return a pointer to the TypeObject)
 */
template <typename MembersType>
const TypeObject *GetTypeObject(
    const std::string &type_name, bool complete, const MembersType *members) {
  // 获取类型对象实例 (Get the TypeObject instance)
  const TypeObject *c_type_object =
      TypeObjectFactory::get_instance()->get_type_object(type_name, complete);

  // 如果类型对象不为空，则返回该类型对象 (If the TypeObject is not null, return it)
  if (c_type_object != nullptr) {
    return c_type_object;
  } else if (complete) {  // 如果需要获取完整的类型对象 (If a complete TypeObject is required)
    return GetCompleteObject(type_name, members);
  }

  // 否则，获取最小类型对象 (Otherwise, get the minimal TypeObject)
  return GetMinimalObject(type_name, members);
}

/**
 * @brief 获取类型标识符 (Get the TypeIdentifier)
 *
 * @tparam MembersType 成员类型 (Member type)
 * @param type_name 类型名称 (Type name)
 * @param complete 是否获取完整的类型标识符 (Whether to get the complete TypeIdentifier)
 * @param members 类型成员指针 (Pointer to the type members)
 * @return 返回类型标识符指针 (Return a pointer to the TypeIdentifier)
 */
template <typename MembersType>
const TypeIdentifier *GetTypeIdentifier(
    const std::string &type_name, bool complete, const MembersType *members) {
  // 获取类型标识符实例 (Get the TypeIdentifier instance)
  const TypeIdentifier *c_identifier =
      TypeObjectFactory::get_instance()->get_type_identifier(type_name, complete);

  // 如果类型标识符不为空且满足完整性要求，则返回该类型标识符 (If the TypeIdentifier is not null and
  // meets the completeness requirements, return it)
  if (c_identifier != nullptr &&
      (!complete || c_identifier->_d() == eprosima::fastrtps::types::EK_COMPLETE)) {
    return c_identifier;
  }

  // 生成类型对象 (Generate the TypeObject)
  GetTypeObject(type_name, complete, members);

  // 返回类型标识符实例 (Return the TypeIdentifier instance)
  return TypeObjectFactory::get_instance()->get_type_identifier(type_name, complete);
}

/**
 * @brief 添加类型对象到TypeObjectFactory中 (Add type object to TypeObjectFactory)
 *
 * @tparam MembersType 成员类型 (Member type)
 * @param untype_members 未类型化的成员指针 (Untyped member pointer)
 * @param type_name 类型名称 (Type name)
 * @return true 如果成功添加类型对象 (If successfully added type object)
 * @return false 如果添加类型对象失败 (If failed to add type object)
 */
template <typename MembersType>
inline bool add_type_object(const void *untype_members, const std::string &type_name) {
  // 将未类型化的成员指针转换为MembersType类型的指针 (Cast the untyped member pointer to a pointer
  // of type MembersType)
  const MembersType *members = static_cast<const MembersType *>(untype_members);
  // 如果成员指针为空，则返回false (If the member pointer is null, return false)
  if (!members) {
    return false;
  }

  // 获取TypeObjectFactory实例 (Get the instance of TypeObjectFactory)
  TypeObjectFactory *factory = TypeObjectFactory::get_instance();
  // 如果工厂实例为空，则返回false (If the factory instance is null, return false)
  if (!factory) {
    return false;
  }

  // 声明类型标识符和类型对象指针 (Declare type identifier and type object pointers)
  const TypeIdentifier *identifier = nullptr;
  const TypeObject *type_object = nullptr;
  // 获取类型标识符 (Get the type identifier)
  identifier = GetTypeIdentifier(type_name, true, members);
  // 如果类型标识符为空，则返回false (If the type identifier is null, return false)
  if (!identifier) {
    return false;
  }
  // 获取类型对象 (Get the type object)
  type_object = GetTypeObject(type_name, true, members);
  // 如果类型对象为空，则返回false (If the type object is null, return false)
  if (!type_object) {
    return false;
  }

  // 将类型标识符和类型对象添加到工厂中 (Add the type identifier and type object to the factory)
  factory->add_type_object(type_name, identifier, type_object);

  // 获取另一个类型标识符 (Get another type identifier)
  identifier = GetTypeIdentifier(type_name, false, members);
  // 如果类型标识符为空，则返回false (If the type identifier is null, return false)
  if (!identifier) {
    return false;
  }
  // 获取另一个类型对象 (Get another type object)
  type_object = GetTypeObject(type_name, false, members);
  // 如果类型对象为空，则返回false (If the type object is null, return false)
  if (!type_object) {
    return false;
  }
  // 将另一个类型标识符和类型对象添加到工厂中 (Add another type identifier and type object to the
  // factory)
  factory->add_type_object(type_name, identifier, type_object);

  // 返回true表示成功添加类型对象 (Return true to indicate successful addition of type objects)
  return true;
}

/**
 * @brief 注册类型对象 (Register type object)
 *
 * @param[in] type_supports 消息类型支持的指针 (Pointer to message type supports)
 * @param[in] type_name 类型名称 (Type name)
 * @return 成功时返回 true，失败时返回 false (Returns true on success, false on failure)
 */
bool register_type_object(
    const rosidl_message_type_support_t *type_supports, const std::string &type_name) {
  // 获取类型支持内省的指针 (Get the pointer to type support introspection)
  const rosidl_message_type_support_t *type_support_intro =
      get_type_support_introspection(type_supports);
  // 如果类型支持内省为空，则返回 false (If type support introspection is null, return false)
  if (!type_support_intro) {
    return false;
  }

  bool ret = false;
  // 判断类型支持标识符是否为 rosidl_typesupport_introspection_c__identifier
  // (Check if the type support identifier is rosidl_typesupport_introspection_c__identifier)
  if (type_support_intro->typesupport_identifier ==
      rosidl_typesupport_introspection_c__identifier) {
    // 添加类型对象（C语言）(Add type object (C language))
    ret = add_type_object<rosidl_typesupport_introspection_c__MessageMembers>(
        type_support_intro->data, type_name);
  } else {
    // 添加类型对象（C++语言）(Add type object (C++ language))
    ret = add_type_object<rosidl_typesupport_introspection_cpp::MessageMembers>(
        type_support_intro->data, type_name);
  }

  // 返回结果 (Return the result)
  return ret;
}

}  // namespace rmw_fastrtps_shared_cpp
