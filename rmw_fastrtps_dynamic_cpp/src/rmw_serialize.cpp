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

#include "./type_support_common.hpp"
#include "./type_support_registry.hpp"
#include "fastcdr/FastBuffer.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/serialized_message.h"

extern "C" {

/**
 * @brief 将 ROS 消息序列化为二进制数据 (Serialize a ROS message into binary data)
 *
 * @param[in] ros_message 输入的 ROS 消息 (The input ROS message)
 * @param[in] type_support 消息类型支持 (Message type support)
 * @param[out] serialized_message 序列化后的消息 (The serialized message)
 * @return rmw_ret_t 序列化结果 (Serialization result)
 */
rmw_ret_t rmw_serialize(
    const void *ros_message,
    const rosidl_message_type_support_t *type_support,
    rmw_serialized_message_t *serialized_message) {
  // 获取 C 类型支持 (Get the C typesupport)
  const rosidl_message_type_support_t *ts =
      get_message_typesupport_handle(type_support, rosidl_typesupport_introspection_c__identifier);
  if (!ts) {
    // 如果没有找到 C 类型支持，尝试获取 C++ 类型支持 (If no C typesupport found, try to get the C++
    // typesupport)
    ts = get_message_typesupport_handle(
        type_support, rosidl_typesupport_introspection_cpp::typesupport_identifier);
    if (!ts) {
      RMW_SET_ERROR_MSG("type support not from this implementation");
      return RMW_RET_ERROR;
    }
  }

  // 获取类型支持注册表实例 (Get the TypeSupportRegistry instance)
  TypeSupportRegistry &type_registry = TypeSupportRegistry::get_instance();
  // 获取消息类型支持 (Get the message type support)
  auto tss = type_registry.get_message_type_support(ts);
  // 计算序列化后的数据长度 (Calculate the length of the serialized data)
  auto data_length = tss->getEstimatedSerializedSize(ros_message, ts->data);
  if (serialized_message->buffer_capacity < data_length) {
    // 如果缓冲区容量不足，尝试调整序列化消息的大小 (If the buffer capacity is insufficient, try to
    // resize the serialized message)
    if (rmw_serialized_message_resize(serialized_message, data_length) != RMW_RET_OK) {
      RMW_SET_ERROR_MSG("unable to dynamically resize serialized message");
      type_registry.return_message_type_support(ts);
      return RMW_RET_ERROR;
    }
  }

  // 创建 FastBuffer 对象 (Create a FastBuffer object)
  eprosima::fastcdr::FastBuffer buffer(
      reinterpret_cast<char *>(serialized_message->buffer), data_length);
  // 创建 Cdr 序列化对象 (Create a Cdr serialization object)
  eprosima::fastcdr::Cdr ser(
      buffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

  // 将 ROS 消息序列化为二进制数据 (Serialize the ROS message into binary data)
  auto ret = tss->serializeROSmessage(ros_message, ser, ts->data);
  // 设置序列化消息的长度和容量 (Set the length and capacity of the serialized message)
  serialized_message->buffer_length = data_length;
  serialized_message->buffer_capacity = data_length;
  // 返回消息类型支持 (Return the message type support)
  type_registry.return_message_type_support(ts);
  // 返回序列化结果 (Return the serialization result)
  return ret == true ? RMW_RET_OK : RMW_RET_ERROR;
}

/**
 * @brief 反序列化一个ROS2消息 (Deserialize a ROS2 message)
 *
 * @param[in] serialized_message 序列化后的消息 (Serialized message)
 * @param[in] type_support 消息类型支持 (Message type support)
 * @param[out] ros_message 反序列化后的ROS2消息 (Deserialized ROS2 message)
 * @return rmw_ret_t 反序列化结果 (Deserialization result)
 */
rmw_ret_t rmw_deserialize(
    const rmw_serialized_message_t *serialized_message,
    const rosidl_message_type_support_t *type_support,
    void *ros_message) {
  // 获取C语言的消息类型支持句柄 (Get the message typesupport handle for C language)
  const rosidl_message_type_support_t *ts =
      get_message_typesupport_handle(type_support, rosidl_typesupport_introspection_c__identifier);
  if (!ts) {
    // 如果C语言的句柄不存在，尝试获取C++的消息类型支持句柄 (If the C language handle does not
    // exist, try to get the message typesupport handle for C++)
    ts = get_message_typesupport_handle(
        type_support, rosidl_typesupport_introspection_cpp::typesupport_identifier);
    if (!ts) {
      // 如果都不存在，则设置错误信息并返回错误 (If neither exists, set the error message and return
      // an error)
      RMW_SET_ERROR_MSG("type support not from this implementation");
      return RMW_RET_ERROR;
    }
  }

  // 获取类型支持注册表实例 (Get the instance of the TypeSupportRegistry)
  TypeSupportRegistry &type_registry = TypeSupportRegistry::get_instance();
  // 从注册表中获取消息类型支持 (Get the message type support from the registry)
  auto tss = type_registry.get_message_type_support(ts);
  // 创建一个FastBuffer，用于存储序列化后的消息 (Create a FastBuffer to store the serialized
  // message)
  eprosima::fastcdr::FastBuffer buffer(
      reinterpret_cast<char *>(serialized_message->buffer), serialized_message->buffer_length);
  // 创建一个Cdr对象，用于反序列化 (Create a Cdr object for deserialization)
  eprosima::fastcdr::Cdr deser(
      buffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

  // 反序列化ROS消息 (Deserialize the ROS message)
  auto ret = tss->deserializeROSmessage(deser, ros_message, ts->data);
  // 返回消息类型支持到注册表 (Return the message type support to the registry)
  type_registry.return_message_type_support(ts);
  // 根据反序列化结果返回相应的值 (Return the corresponding value based on the deserialization
  // result)
  return ret == true ? RMW_RET_OK : RMW_RET_ERROR;
}

/**
 * @brief 获取序列化消息的大小 (Get the size of a serialized message)
 *
 * @param[in] type_support 消息类型支持 (Message type support)
 * @param[in] message_bounds 消息边界 (Message bounds)
 * @param[out] size 序列化消息的大小 (Size of the serialized message)
 * @return rmw_ret_t 结果 (Result)
 */
rmw_ret_t rmw_get_serialized_message_size(
    const rosidl_message_type_support_t * /*type_support*/,
    const rosidl_runtime_c__Sequence__bound * /*message_bounds*/,
    size_t * /*size*/) {
  // 设置错误信息并返回不支持的结果 (Set the error message and return an unsupported result)
  RMW_SET_ERROR_MSG("unimplemented");
  return RMW_RET_UNSUPPORTED;
}

}  // extern "C"
