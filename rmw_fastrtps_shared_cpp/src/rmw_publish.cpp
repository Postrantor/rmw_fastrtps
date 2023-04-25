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

#include "fastcdr/Cdr.h"
#include "fastcdr/FastBuffer.h"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"
#include "rmw_fastrtps_shared_cpp/custom_publisher_info.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "tracetools/tracetools.h"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 发布 ROS2 消息 (Publish a ROS2 message)
 *
 * @param[in] identifier RMW 实现的标识符 (Identifier of the RMW implementation)
 * @param[in] publisher 有效的 rmw_publisher_t 结构指针 (Pointer to a valid rmw_publisher_t
 * structure)
 * @param[in] ros_message 要发布的 ROS 消息 (The ROS message to be published)
 * @param[in] allocation 预留给未来实现的内存分配器 (Memory allocator reserved for future
 * implementation)
 *
 * @return 成功时返回 RMW_RET_OK，否则返回相应的错误代码 (Returns RMW_RET_OK on success, otherwise
 * returns the corresponding error code)
 */
rmw_ret_t __rmw_publish(
    const char *identifier,
    const rmw_publisher_t *publisher,
    const void *ros_message,
    rmw_publisher_allocation_t *allocation) {
  // 设置可能的错误返回值 (Set possible error return values)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_ERROR);

  // 忽略未使用的参数 (Ignore unused parameter)
  (void)allocation;

  // 检查 publisher 是否为空 (Check if publisher is null)
  RMW_CHECK_FOR_NULL_WITH_MSG(
      publisher, "publisher handle is null", return RMW_RET_INVALID_ARGUMENT);

  // 检查 RMW 实现是否匹配 (Check if RMW implementation matches)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      publisher, publisher->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查 ros_message 是否为空 (Check if ros_message is null)
  RMW_CHECK_FOR_NULL_WITH_MSG(
      ros_message, "ros message handle is null", return RMW_RET_INVALID_ARGUMENT);

  // 获取 publisher 的自定义信息 (Get custom information of the publisher)
  auto info = static_cast<CustomPublisherInfo *>(publisher->data);

  // 检查 info 是否为空 (Check if info is null)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(info, "publisher info pointer is null", return RMW_RET_ERROR);

  // 初始化序列化数据结构 (Initialize serialized data structure)
  rmw_fastrtps_shared_cpp::SerializedData data;
  data.type = FASTRTPS_SERIALIZED_DATA_TYPE_ROS_MESSAGE;
  data.data = const_cast<void *>(ros_message);
  data.impl = info->type_support_impl_;

  // 跟踪发布事件 (Trace publish event)
  TRACEPOINT(rmw_publish, ros_message);

  // 将数据写入 DataWriter (Write data to DataWriter)
  if (!info->data_writer_->write(&data)) {
    RMW_SET_ERROR_MSG("cannot publish data");
    return RMW_RET_ERROR;
  }

  // 返回成功 (Return success)
  return RMW_RET_OK;
}

/**
 * @brief 发布序列化消息 (Publish a serialized message)
 *
 * @param[in] identifier 标识符，用于检查 RMW 实现是否匹配 (Identifier for checking if the RMW
 * implementation matches)
 * @param[in] publisher 发布者指针 (Pointer to the publisher)
 * @param[in] serialized_message 序列化消息指针 (Pointer to the serialized message)
 * @param[in] allocation 发布者分配指针（未使用）(Publisher allocation pointer (not used))
 *
 * @return RMW_RET_OK 成功发布序列化消息 (Successfully published the serialized message)
 * @return RMW_RET_INVALID_ARGUMENT 无效参数错误 (Invalid argument error)
 * @return RMW_RET_INCORRECT_RMW_IMPLEMENTATION RMW 实现不正确错误 (Incorrect RMW implementation
 * error)
 * @return RMW_RET_ERROR 其他错误 (Other errors)
 */
rmw_ret_t __rmw_publish_serialized_message(
    const char *identifier,
    const rmw_publisher_t *publisher,
    const rmw_serialized_message_t *serialized_message,
    rmw_publisher_allocation_t *allocation) {
  // 检查可能的错误返回值 (Check for possible error return values)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_ERROR);

  // 忽略未使用的 allocation 参数 (Ignore unused allocation parameter)
  (void)allocation;

  // 检查 publisher 是否为空，如果为空则返回无效参数错误 (Check if publisher is null, return invalid
  // argument error if it is)
  RMW_CHECK_FOR_NULL_WITH_MSG(
      publisher, "publisher handle is null", return RMW_RET_INVALID_ARGUMENT);

  // 检查 publisher 的实现标识符是否与传入的标识符匹配 (Check if the publisher's implementation
  // identifier matches the given identifier)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      publisher, publisher->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查序列化消息是否为空，如果为空则返回无效参数错误 (Check if serialized message is null, return
  // invalid argument error if it is)
  RMW_CHECK_FOR_NULL_WITH_MSG(
      serialized_message, "serialized message handle is null", return RMW_RET_INVALID_ARGUMENT);

  // 获取 publisher 的自定义信息 (Get the custom information of the publisher)
  auto info = static_cast<CustomPublisherInfo *>(publisher->data);

  // 检查自定义信息是否为空，如果为空则返回错误 (Check if custom information is null, return error
  // if it is)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(info, "publisher info pointer is null", return RMW_RET_ERROR);

  // 创建 FastBuffer 对象，用于存储序列化消息 (Create a FastBuffer object to store the serialized
  // message)
  eprosima::fastcdr::FastBuffer buffer(
      reinterpret_cast<char *>(serialized_message->buffer), serialized_message->buffer_length);

  // 创建 Cdr 对象，用于序列化和反序列化操作 (Create a Cdr object for serialization and
  // deserialization operations)
  eprosima::fastcdr::Cdr ser(
      buffer, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR);

  // 跳过序列化消息的缓冲区长度 (Skip the buffer length of the serialized message)
  if (!ser.jump(serialized_message->buffer_length)) {
    RMW_SET_ERROR_MSG("cannot correctly set serialized buffer");
    return RMW_RET_ERROR;
  }

  // 创建 SerializedData 对象，用于存储序列化数据 (Create a SerializedData object to store the
  // serialized data)
  rmw_fastrtps_shared_cpp::SerializedData data;
  data.type = FASTRTPS_SERIALIZED_DATA_TYPE_CDR_BUFFER;
  data.data = &ser;
  data.impl = nullptr;  // not used when type is FASTRTPS_SERIALIZED_DATA_TYPE_CDR_BUFFER

  // 将序列化数据写入 data_writer_，如果失败则返回错误 (Write the serialized data to data_writer_,
  // return error if it fails)
  if (!info->data_writer_->write(&data)) {
    RMW_SET_ERROR_MSG("cannot publish data");
    return RMW_RET_ERROR;
  }

  // 成功发布序列化消息，返回 RMW_RET_OK (Successfully published the serialized message, return
  // RMW_RET_OK)
  return RMW_RET_OK;
}

/**
 * @brief 发布一个借用的消息 (Publish a loaned message)
 *
 * @param[in] identifier 实现标识符 (Implementation identifier)
 * @param[in] publisher 发布者指针 (Pointer to the publisher)
 * @param[in] ros_message ROS 消息指针 (Pointer to the ROS message)
 * @param[in] allocation 发布者分配指针 (Pointer to the publisher allocation)
 * @return rmw_ret_t RMW 返回值 (RMW return value)
 */
rmw_ret_t __rmw_publish_loaned_message(
    const char *identifier,
    const rmw_publisher_t *publisher,
    const void *ros_message,
    rmw_publisher_allocation_t *allocation) {
  // 忽略分配参数 (Ignore the allocation parameter)
  static_cast<void>(allocation);

  // 定义可能的错误返回值 (Define possible error return values)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_INVALID_ARGUMENT);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_ERROR);

  // 检查发布者是否为空 (Check if the publisher is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);

  // 检查类型标识符是否匹配 (Check if type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      publisher, publisher->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查是否支持借用消息 (Check if loaning messages is supported)
  if (!publisher->can_loan_messages) {
    RMW_SET_ERROR_MSG("Loaning is not supported");
    return RMW_RET_UNSUPPORTED;
  }

  // 检查 ROS 消息是否为空 (Check if the ROS message is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(ros_message, RMW_RET_INVALID_ARGUMENT);

  // 获取自定义发布者信息 (Get the custom publisher info)
  auto info = static_cast<CustomPublisherInfo *>(publisher->data);

  // 发布数据，如果失败则设置错误消息 (Publish data, set error message if it fails)
  if (!info->data_writer_->write(const_cast<void *>(ros_message))) {
    RMW_SET_ERROR_MSG("cannot publish data");
    return RMW_RET_ERROR;
  }

  // 返回成功 (Return success)
  return RMW_RET_OK;
}

}  // namespace rmw_fastrtps_shared_cpp
