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
#include "fastcdr/FastBuffer.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/serialized_message.h"

extern "C" {

/**
 * @brief 序列化ROS2消息 (Serialize a ROS2 message)
 *
 * @param[in] ros_message 输入的ROS2消息 (Input ROS2 message)
 * @param[in] type_support 消息类型支持 (Message type support)
 * @param[out] serialized_message 序列化后的消息 (Serialized message after serialization)
 * @return rmw_ret_t 序列化操作结果 (Serialization operation result)
 */
rmw_ret_t rmw_serialize(
    const void *ros_message,
    const rosidl_message_type_support_t *type_support,
    rmw_serialized_message_t *serialized_message) {
  // 获取C类型支持的消息类型支持句柄 (Get the message type support handle for C type support)
  const rosidl_message_type_support_t *ts =
      get_message_typesupport_handle(type_support, RMW_FASTRTPS_CPP_TYPESUPPORT_C);
  if (!ts) {
    // 如果C类型支持不可用，尝试获取C++类型支持的消息类型支持句柄 (If C type support is not
    // available, try to get the message type support handle for C++ type support)
    ts = get_message_typesupport_handle(type_support, RMW_FASTRTPS_CPP_TYPESUPPORT_CPP);
    if (!ts) {
      // 如果都不可用，设置错误消息并返回错误 (If neither is available, set error message and return
      // error)
      RMW_SET_ERROR_MSG("type support not from this implementation");
      return RMW_RET_ERROR;
    }
  }

  // 获取回调函数指针 (Get callback function pointers)
  auto callbacks = static_cast<const message_type_support_callbacks_t *>(ts->data);
  // 创建MessageTypeSupport_cpp对象 (Create MessageTypeSupport_cpp object)
  auto tss = MessageTypeSupport_cpp(callbacks);
  // 获取序列化后数据的预估大小 (Get the estimated size of the serialized data)
  auto data_length = tss.getEstimatedSerializedSize(ros_message, callbacks);
  // 检查序列化消息的缓冲区容量是否足够 (Check if the buffer capacity of the serialized message is
  // sufficient)
  if (serialized_message->buffer_capacity < data_length) {
    // 如果不够，尝试调整序列化消息的大小 (If not, try to resize the serialized message)
    if (rmw_serialized_message_resize(serialized_message, data_length) != RMW_RET_OK) {
      // 如果调整失败，设置错误消息并返回错误 (If resizing fails, set error message and return
      // error)
      RMW_SET_ERROR_MSG("unable to dynamically resize serialized message");
      return RMW_RET_ERROR;
    }
  }

  // 创建FastBuffer对象，用于存储序列化后的数据 (Create FastBuffer object for storing serialized
  // data)
  eprosima::fastcdr::FastBuffer buffer(
      reinterpret_cast<char *>(serialized_message->buffer), data_length);
  // 创建Cdr对象，用于序列化操作 (Create Cdr object for serialization operation)
  eprosima::fastcdr::Cdr ser(
      buffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

  // 执行序列化操作 (Perform serialization operation)
  auto ret = tss.serializeROSmessage(ros_message, ser, callbacks);
  // 设置序列化消息的长度和容量 (Set length and capacity of the serialized message)
  serialized_message->buffer_length = data_length;
  serialized_message->buffer_capacity = data_length;
  // 返回序列化结果 (Return serialization result)
  return ret == true ? RMW_RET_OK : RMW_RET_ERROR;
}

/**
 * @brief 反序列化一个 ROS2 消息 (Deserialize a ROS2 message)
 *
 * @param[in] serialized_message 序列化后的消息 (Serialized message)
 * @param[in] type_support 消息类型支持 (Message type support)
 * @param[out] ros_message 反序列化后的 ROS 消息 (Deserialized ROS message)
 * @return rmw_ret_t 反序列化操作结果 (Deserialization operation result)
 */
rmw_ret_t rmw_deserialize(
    const rmw_serialized_message_t *serialized_message,
    const rosidl_message_type_support_t *type_support,
    void *ros_message) {
  // 获取 C 类型支持句柄 (Get the C typesupport handle)
  const rosidl_message_type_support_t *ts =
      get_message_typesupport_handle(type_support, RMW_FASTRTPS_CPP_TYPESUPPORT_C);
  if (!ts) {
    // 如果没有找到 C 类型支持，尝试获取 C++ 类型支持句柄 (If no C typesupport is found, try getting
    // the C++ typesupport handle)
    ts = get_message_typesupport_handle(type_support, RMW_FASTRTPS_CPP_TYPESUPPORT_CPP);
    if (!ts) {
      RMW_SET_ERROR_MSG("type support not from this implementation");
      return RMW_RET_ERROR;
    }
  }

  // 获取回调函数 (Get the callbacks)
  auto callbacks = static_cast<const message_type_support_callbacks_t *>(ts->data);
  // 创建 MessageTypeSupport_cpp 对象 (Create a MessageTypeSupport_cpp object)
  auto tss = MessageTypeSupport_cpp(callbacks);
  // 初始化 FastBuffer (Initialize FastBuffer)
  eprosima::fastcdr::FastBuffer buffer(
      reinterpret_cast<char *>(serialized_message->buffer), serialized_message->buffer_length);
  // 初始化 CDR 反序列化对象 (Initialize the CDR deserialization object)
  eprosima::fastcdr::Cdr deser(
      buffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

  // 反序列化 ROS 消息 (Deserialize the ROS message)
  auto ret = tss.deserializeROSmessage(deser, ros_message, callbacks);
  // 返回反序列化结果 (Return the deserialization result)
  return ret == true ? RMW_RET_OK : RMW_RET_ERROR;
}

/**
 * @brief 获取序列化消息的大小 (Get the size of a serialized message)
 *
 * @param[in] type_support 消息类型支持 (Message type support)
 * @param[in] message_bounds 消息边界 (Message bounds)
 * @param[out] size 序列化消息的大小 (Size of the serialized message)
 * @return rmw_ret_t 获取操作结果 (Get operation result)
 */
rmw_ret_t rmw_get_serialized_message_size(
    const rosidl_message_type_support_t * /*type_support*/,
    const rosidl_runtime_c__Sequence__bound * /*message_bounds*/,
    size_t * /*size*/) {
  // 设置错误消息并返回不支持的状态 (Set error message and return unsupported status)
  RMW_SET_ERROR_MSG("unimplemented");
  return RMW_RET_UNSUPPORTED;
}

}  // extern "C"
