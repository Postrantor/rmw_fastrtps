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

#include "type_support_common.hpp"

#include <fastcdr/exceptions/Exception.h>

#include <string>

#include "rmw/error_handling.h"

namespace rmw_fastrtps_cpp {

/**
 * @brief 构造函数，初始化 TypeSupport 类的成员变量 (Constructor, initialize the member variables of
 * the TypeSupport class)
 */
TypeSupport::TypeSupport() {
  // 设置 m_isGetKeyDefined 为 false (Set m_isGetKeyDefined to false)
  m_isGetKeyDefined = false;
  // 设置 max_size_bound_ 为 false (Set max_size_bound_ to false)
  max_size_bound_ = false;
  // 设置 is_plain_ 为 false (Set is_plain_ to false)
  is_plain_ = false;
}

/**
 * @brief 设置 TypeSupport 的成员变量 (Set the member variables of the TypeSupport)
 * @param[in] members 指向 message_type_support_callbacks_t 结构体的指针 (Pointer to a
 * message_type_support_callbacks_t struct)
 */
void TypeSupport::set_members(const message_type_support_callbacks_t *members) {
  // 将传入的 members 赋值给成员变量 members_ (Assign the input members to the member variable
  // members_)
  members_ = members;

#ifdef ROSIDL_TYPESUPPORT_FASTRTPS_HAS_PLAIN_TYPES
  char bounds_info;
  auto data_size = static_cast<uint32_t>(members->max_serialized_size(bounds_info));
  max_size_bound_ = 0 != (bounds_info & ROSIDL_TYPESUPPORT_FASTRTPS_BOUNDED_TYPE);
  is_plain_ = bounds_info == ROSIDL_TYPESUPPORT_FASTRTPS_PLAIN_TYPE;
#else
  is_plain_ = true;
  auto data_size = static_cast<uint32_t>(members->max_serialized_size(is_plain_));
  max_size_bound_ = is_plain_;
#endif

  // 如果是简单类型且数据大小为 0，则表示为空消息 (If it's a plain type and data size is 0, it's an
  // empty message)
  if (is_plain_ && (data_size == 0)) {
    has_data_ = false;
    ++data_size;  // 增加一个虚拟字节 (Add a dummy byte)
  } else {
    has_data_ = true;
  }

  // 总大小为封装大小 + 数据大小 (Total size is encapsulation size + data size)
  m_typeSize = 4 + data_size;
  // 考虑到 RTPS 子消息对齐 (Account for RTPS submessage alignment)
  m_typeSize = (m_typeSize + 3) & ~3;
}

/**
 * @brief 获取预计的序列化大小 (Get the estimated serialized size)
 * @param[in] ros_message 指向 ROS 消息的指针 (Pointer to the ROS message)
 * @param[in] impl 指向实现细节的指针 (Pointer to the implementation details)
 * @return 预计的序列化大小 (Estimated serialized size)
 */
size_t TypeSupport::getEstimatedSerializedSize(const void *ros_message, const void *impl) const {
  // 如果是简单类型，则直接返回 m_typeSize (If it's a plain type, return m_typeSize directly)
  if (is_plain_) {
    return m_typeSize;
  }

  // 断言 ros_message 和 impl 不为空 (Assert that ros_message and impl are not null)
  assert(ros_message);
  assert(impl);

  auto callbacks = static_cast<const message_type_support_callbacks_t *>(impl);

  // 封装大小 + 消息大小 (Encapsulation size + message size)
  return 4 + callbacks->get_serialized_size(ros_message);
}

/**
 * @brief 序列化 ROS 消息为 Fast-CDR 格式
 *        Serialize a ROS message to Fast-CDR format
 *
 * @param[in] ros_message 输入的 ROS 消息
 *                  The input ROS message
 * @param[out] ser 输出的序列化后的 Fast-CDR 对象
 *                  The output serialized Fast-CDR object
 * @param[in] impl 指向消息类型支持回调的指针
 *                 Pointer to the message type support callbacks
 * @return 是否成功序列化
 *         Whether the serialization was successful
 */
bool TypeSupport::serializeROSmessage(
    const void *ros_message, eprosima::fastcdr::Cdr &ser, const void *impl) const {
  // 检查输入参数是否有效
  // Check if input arguments are valid
  assert(ros_message);
  assert(impl);

  // 序列化封装格式
  // Serialize encapsulation format
  ser.serialize_encapsulation();

  // 如果类型非空，则序列化消息
  // If the type is not empty, serialize the message
  if (has_data_) {
    auto callbacks = static_cast<const message_type_support_callbacks_t *>(impl);
    return callbacks->cdr_serialize(ros_message, ser);
  }

  // 否则，添加一个虚拟字节
  // Otherwise, add a dummy byte
  ser << (uint8_t)0;
  return true;
}

/**
 * @brief 反序列化 Fast-CDR 格式为 ROS 消息
 *        Deserialize a Fast-CDR format to ROS message
 *
 * @param[in] deser 输入的 Fast-CDR 对象
 *                  The input Fast-CDR object
 * @param[out] ros_message 输出的反序列化后的 ROS 消息
 *                  The output deserialized ROS message
 * @param[in] impl 指向消息类型支持回调的指针
 *                 Pointer to the message type support callbacks
 * @return 是否成功反序列化
 *         Whether the deserialization was successful
 */
bool TypeSupport::deserializeROSmessage(
    eprosima::fastcdr::Cdr &deser, void *ros_message, const void *impl) const {
  // 检查输入参数是否有效
  // Check if input arguments are valid
  assert(ros_message);
  assert(impl);

  try {
    // 反序列化封装格式
    // Deserialize encapsulation format
    deser.read_encapsulation();

    // 如果类型非空，则反序列化消息
    // If the type is not empty, deserialize the message
    if (has_data_) {
      auto callbacks = static_cast<const message_type_support_callbacks_t *>(impl);
      return callbacks->cdr_deserialize(deser, ros_message);
    }

    // 否则，消耗虚拟字节
    // Otherwise, consume dummy byte
    uint8_t dump = 0;
    deser >> dump;
    (void)dump;
  } catch (const eprosima::fastcdr::exception::Exception &) {
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "Fast CDR exception deserializing message of type %s.", getName());
    return false;
  }

  return true;
}

/**
 * @brief 构造函数，用于创建 MessageTypeSupport 对象 (Constructor for creating a MessageTypeSupport
 * object)
 *
 * @param[in] members 指向 message_type_support_callbacks_t 结构体的指针 (Pointer to the
 * message_type_support_callbacks_t structure)
 */
MessageTypeSupport::MessageTypeSupport(const message_type_support_callbacks_t *members) {
  // 确保传入的 members 参数非空 (Ensure that the input parameter 'members' is not null)
  assert(members);

  // 调用 _create_type_name 函数根据 members 创建类型名称 (Call the _create_type_name function to
  // create a type name based on 'members')
  std::string name = _create_type_name(members);
  // 设置 MessageTypeSupport 对象的名称 (Set the name of the MessageTypeSupport object)
  this->setName(name.c_str());

  // 设置 MessageTypeSupport 对象的成员 (Set the members of the MessageTypeSupport object)
  set_members(members);
}

/**
 * @brief ServiceTypeSupport 的默认构造函数 (Default constructor for ServiceTypeSupport)
 */
ServiceTypeSupport::ServiceTypeSupport() {}

/**
 * @brief 构造函数，用于创建 RequestTypeSupport 对象 (Constructor for creating a RequestTypeSupport
 * object)
 *
 * @param[in] members 指向 service_type_support_callbacks_t 结构体的指针 (Pointer to the
 * service_type_support_callbacks_t structure)
 */
RequestTypeSupport::RequestTypeSupport(const service_type_support_callbacks_t *members) {
  // 确保传入的 members 参数非空 (Ensure that the input parameter 'members' is not null)
  assert(members);

  // 获取 request_members_ 结构体中的数据，并将其转换为 message_type_support_callbacks_t 类型 (Get
  // the data in the 'request_members_' structure and cast it to type
  // message_type_support_callbacks_t)
  auto msg = static_cast<const message_type_support_callbacks_t *>(members->request_members_->data);
  // 调用 _create_type_name 函数根据 msg 创建类型名称 (Call the _create_type_name function to create
  // a type name based on 'msg')
  std::string name = _create_type_name(msg);  // + "Request_";
  // 设置 RequestTypeSupport 对象的名称 (Set the name of the RequestTypeSupport object)
  this->setName(name.c_str());

  // 设置 RequestTypeSupport 对象的成员 (Set the members of the RequestTypeSupport object)
  set_members(msg);
}

/**
 * @brief 构造函数，用于创建 ResponseTypeSupport 对象 (Constructor for creating a
 * ResponseTypeSupport object)
 *
 * @param[in] members 指向 service_type_support_callbacks_t 结构体的指针 (Pointer to the
 * service_type_support_callbacks_t structure)
 */
ResponseTypeSupport::ResponseTypeSupport(const service_type_support_callbacks_t *members) {
  // 确保传入的 members 参数非空 (Ensure that the input parameter 'members' is not null)
  assert(members);

  // 获取 response_members_ 结构体中的数据，并将其转换为 message_type_support_callbacks_t 类型 (Get
  // the data in the 'response_members_' structure and cast it to type
  // message_type_support_callbacks_t)
  auto msg =
      static_cast<const message_type_support_callbacks_t *>(members->response_members_->data);
  // 调用 _create_type_name 函数根据 msg 创建类型名称 (Call the _create_type_name function to create
  // a type name based on 'msg')
  std::string name = _create_type_name(msg);  // + "Response_";
  // 设置 ResponseTypeSupport 对象的名称 (Set the name of the ResponseTypeSupport object)
  this->setName(name.c_str());

  // 设置 ResponseTypeSupport 对象的成员 (Set the members of the ResponseTypeSupport object)
  set_members(msg);
}

}  // namespace rmw_fastrtps_cpp
