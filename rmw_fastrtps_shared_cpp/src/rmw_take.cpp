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

#include <memory>
#include <utility>

#include "fastcdr/Cdr.h"
#include "fastcdr/FastBuffer.h"
#include "fastdds/dds/core/StackAllocatedSequence.hpp"
#include "fastdds/dds/subscriber/SampleInfo.hpp"
#include "fastrtps/utils/collections/ResourceLimitedVector.hpp"
#include "rcpputils/scope_exit.hpp"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/serialized_message.h"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"
#include "rmw_fastrtps_shared_cpp/custom_subscriber_info.hpp"
#include "rmw_fastrtps_shared_cpp/guid_utils.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/subscription.hpp"
#include "rmw_fastrtps_shared_cpp/utils.hpp"
#include "rosidl_dynamic_typesupport/types.h"
#include "tracetools/tracetools.h"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 为消息信息分配并设置相关参数 (Assign and set relevant parameters for message information)
 *
 * @param[in] identifier 指示实现的字符串 (A string indicating the implementation)
 * @param[out] message_info 指向要填充的rmw_message_info_t结构的指针 (Pointer to the
 * rmw_message_info_t structure to be filled)
 * @param[in] sinfo 指向eprosima::fastdds::dds::SampleInfo结构的指针，包含源时间戳和接收时间戳等信息
 * (Pointer to the eprosima::fastdds::dds::SampleInfo structure containing information such as
 * source timestamp and reception timestamp)
 */
void _assign_message_info(
    const char *identifier,
    rmw_message_info_t *message_info,
    const eprosima::fastdds::dds::SampleInfo *sinfo) {
  // 将源时间戳从sinfo中复制到message_info中 (Copy the source timestamp from sinfo to message_info)
  message_info->source_timestamp = sinfo->source_timestamp.to_ns();
  // 将接收时间戳从sinfo中复制到message_info中 (Copy the reception timestamp from sinfo to
  // message_info)
  message_info->received_timestamp = sinfo->reception_timestamp.to_ns();
  // 获取Fast DDS序列号 (Get Fast DDS sequence number)
  auto fastdds_sn = sinfo->sample_identity.sequence_number();
  // 将Fast DDS序列号转换为发布序列号 (Convert Fast DDS sequence number to publication sequence
  // number)
  message_info->publication_sequence_number =
      (static_cast<uint64_t>(fastdds_sn.high) << 32) | static_cast<uint64_t>(fastdds_sn.low);
  // 设置接收序列号为不支持 (Set reception sequence number to unsupported)
  message_info->reception_sequence_number = RMW_MESSAGE_INFO_SEQUENCE_NUMBER_UNSUPPORTED;
  // 获取发布者GID (Get publisher GID)
  rmw_gid_t *sender_gid = &message_info->publisher_gid;
  // 设置实现标识符 (Set implementation identifier)
  sender_gid->implementation_identifier = identifier;
  // 将GID数据设置为0 (Set GID data to 0)
  memset(sender_gid->data, 0, RMW_GID_STORAGE_SIZE);

  // 从Fast DDS GUID复制到字节数组 (Copy from Fast DDS GUID to byte array)
  rmw_fastrtps_shared_cpp::copy_from_fastrtps_guid_to_byte_array(
      sinfo->sample_identity.writer_guid(), sender_gid->data);
}

/**
 * @brief 从订阅者中获取消息
 * @param[in] identifier RMW实现的标识符
 * @param[in] subscription 订阅者对象指针
 * @param[out] ros_message 存储接收到的ROS消息的指针
 * @param[out] taken 是否成功获取到消息的布尔值
 * @param[out] message_info 消息相关信息的指针（可选）
 * @param[in] allocation 订阅者分配器指针（未使用）
 * @return rmw_ret_t RMW操作结果状态码
 *
 * @brief Take a message from the subscriber
 * @param[in] identifier Identifier of the RMW implementation
 * @param[in] subscription Pointer to the subscription object
 * @param[out] ros_message Pointer to store the received ROS message
 * @param[out] taken Boolean value indicating whether a message was successfully taken
 * @param[out] message_info Pointer to message related information (optional)
 * @param[in] allocation Pointer to the subscription allocator (unused)
 * @return rmw_ret_t RMW operation result status code
 */
rmw_ret_t _take(
    const char *identifier,
    const rmw_subscription_t *subscription,
    void *ros_message,
    bool *taken,
    rmw_message_info_t *message_info,
    rmw_subscription_allocation_t *allocation) {
  // 忽略未使用的参数
  // Ignore unused parameter
  (void)allocation;
  *taken = false;

  // 检查订阅者的RMW实现是否匹配
  // Check if the RMW implementation of the subscription matches
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      subscription handle, subscription->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 获取订阅者的自定义信息
  // Get the custom information of the subscriber
  auto info = static_cast<CustomSubscriberInfo *>(subscription->data);
  // 检查自定义信息是否为空
  // Check if custom information is null
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(info, "custom subscriber info is null", return RMW_RET_ERROR);

  // 初始化序列化数据结构
  // Initialize serialized data structure
  rmw_fastrtps_shared_cpp::SerializedData data;
  data.type = FASTRTPS_SERIALIZED_DATA_TYPE_ROS_MESSAGE;
  data.data = ros_message;
  data.impl = info->type_support_impl_;

  eprosima::fastdds::dds::StackAllocatedSequence<void *, 1> data_values;
  const_cast<void **>(data_values.buffer())[0] = &data;
  eprosima::fastdds::dds::SampleInfoSeq info_seq{1};

  // 循环尝试从数据读取器中获取消息
  // Loop to try taking messages from the data reader
  while (ReturnCode_t::RETCODE_OK == info->data_reader_->take(data_values, info_seq, 1)) {
    // 调用info->data_reader_->take()已经修改了ros_message参数
    // See rmw_fastrtps_shared_cpp/src/TypeSupport_impl.cpp
    // The info->data_reader_->take() call already modified the ros_message arg

    // 创建作用域退出时重置长度的lambda函数
    // Create a lambda function to reset lengths on scope exit
    auto reset = rcpputils::make_scope_exit([&]() {
      data_values.length(0);
      info_seq.length(0);
    });

    // 检查是否忽略本地发布的消息
    // Check if local publications should be ignored
    if (subscription->options.ignore_local_publications) {
      auto sample_writer_guid =
          eprosima::fastrtps::rtps::iHandle2GUID(info_seq[0].publication_handle);

      if (sample_writer_guid.guidPrefix == info->data_reader_->guid().guidPrefix) {
        // 这是一个本地发布，忽略它
        // This is a local publication. Ignore it
        continue;
      }
    }

    // 检查获取到的消息是否有效
    // Check if the taken message is valid
    if (info_seq[0].valid_data) {
      if (message_info) {
        _assign_message_info(identifier, message_info, &info_seq[0]);
      }
      *taken = true;
      break;
    }
  }

  // 添加跟踪点
  // Add tracepoint
  TRACEPOINT(
      rmw_take, static_cast<const void *>(subscription), static_cast<const void *>(ros_message),
      (message_info ? message_info->source_timestamp : 0LL), *taken);
  return RMW_RET_OK;
}

/**
 * @brief 从订阅者中获取一系列消息 (Take a sequence of messages from the subscriber)
 *
 * @param[in] identifier RMW实现的标识符 (Identifier for the RMW implementation)
 * @param[in] subscription 订阅者对象指针 (Pointer to the subscription object)
 * @param[in] count 要获取的消息数量 (Number of messages to take)
 * @param[out] message_sequence 存储获取到的消息序列的结构体 (Structure to store the taken message
 * sequence)
 * @param[out] message_info_sequence 存储获取到的消息信息序列的结构体 (Structure to store the taken
 * message info sequence)
 * @param[out] taken 实际获取到的消息数量 (Actual number of messages taken)
 * @param[in] allocation 预留的订阅分配参数，可以为NULL (Reserved subscription allocation parameter,
 * can be NULL)
 * @return rmw_ret_t 操作结果状态码 (Operation result status code)
 */
rmw_ret_t _take_sequence(
    const char *identifier,
    const rmw_subscription_t *subscription,
    size_t count,
    rmw_message_sequence_t *message_sequence,
    rmw_message_info_sequence_t *message_info_sequence,
    size_t *taken,
    rmw_subscription_allocation_t *allocation) {
  // 初始化已获取消息数量为0 (Initialize the number of messages taken to 0)
  *taken = 0;
  bool taken_flag = false;
  rmw_ret_t ret = RMW_RET_OK;

  // 检查订阅者的RMW实现是否匹配 (Check if the RMW implementation of the subscriber matches)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      subscription handle, subscription->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 获取订阅者的自定义信息 (Get the custom information of the subscriber)
  auto info = static_cast<CustomSubscriberInfo *>(subscription->data);
  // 检查自定义信息是否为空 (Check if the custom information is null)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(info, "custom subscriber info is null", return RMW_RET_ERROR);

  // 遍历并尝试获取指定数量的消息 (Iterate and try to take the specified number of messages)
  for (size_t ii = 0; ii < count; ++ii) {
    taken_flag = false;
    // 调用_take函数尝试获取单个消息 (Call the _take function to try to take a single message)
    ret = _take(
        identifier, subscription, message_sequence->data[*taken], &taken_flag,
        &message_info_sequence->data[*taken], allocation);

    // 如果获取过程中出现错误，跳出循环 (If an error occurs during the taking process, break the
    // loop)
    if (ret != RMW_RET_OK) {
      break;
    }

    // 如果成功获取到消息，更新已获取消息数量 (If a message is successfully taken, update the number
    // of messages taken)
    if (taken_flag) {
      (*taken)++;
    }
  }

  // 更新消息序列和消息信息序列的大小 (Update the size of the message sequence and message info
  // sequence)
  message_sequence->size = *taken;
  message_info_sequence->size = *taken;

  // 返回操作结果状态码 (Return the operation result status code)
  return ret;
}

/**
 * @brief 从事件中获取信息 (Take information from an event)
 *
 * @param[in] identifier 指定的实现标识符 (The specified implementation identifier)
 * @param[in] event_handle 事件句柄，包含事件信息 (Event handle containing the event information)
 * @param[out] event_info 存储取得的事件信息的指针 (Pointer to store the taken event information)
 * @param[out] taken 如果成功获取到事件信息，则为 true，否则为 false (True if event information is
 * successfully taken, otherwise false)
 * @return rmw_ret_t RMW_RET_OK 表示成功，其他值表示失败 (RMW_RET_OK indicates success, other values
 * indicate failure)
 */
rmw_ret_t __rmw_take_event(
    const char *identifier, const rmw_event_t *event_handle, void *event_info, bool *taken) {
  // 检查 event_handle 是否为空，如果为空则返回 RMW_RET_INVALID_ARGUMENT 错误
  // Check if event_handle is null, return RMW_RET_INVALID_ARGUMENT error if it is
  RMW_CHECK_ARGUMENT_FOR_NULL(event_handle, RMW_RET_INVALID_ARGUMENT);

  // 检查 event_info 是否为空，如果为空则返回 RMW_RET_INVALID_ARGUMENT 错误
  // Check if event_info is null, return RMW_RET_INVALID_ARGUMENT error if it is
  RMW_CHECK_ARGUMENT_FOR_NULL(event_info, RMW_RET_INVALID_ARGUMENT);

  // 检查 taken 是否为空，如果为空则返回 RMW_RET_INVALID_ARGUMENT 错误
  // Check if taken is null, return RMW_RET_INVALID_ARGUMENT error if it is
  RMW_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);

  // 初始化 taken 为 false
  // Initialize taken to false
  *taken = false;

  // 检查事件句柄的实现标识符是否与指定的实现标识符匹配，如果不匹配则返回 RMW_RET_ERROR 错误
  // Check if the event handle's implementation identifier matches the specified one, return
  // RMW_RET_ERROR error if it doesn't
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      event handle, event_handle->implementation_identifier, identifier, return RMW_RET_ERROR);

  // 将 event_handle 的 data 转换为 CustomEventInfo 类型的指针
  // Cast the data of event_handle to a pointer of type CustomEventInfo
  auto event = static_cast<CustomEventInfo *>(event_handle->data);

  // 如果成功获取到事件信息，则将 taken 设置为 true 并返回 RMW_RET_OK
  // If event information is successfully taken, set taken to true and return RMW_RET_OK
  if (event->get_listener()->take_event(event_handle->event_type, event_info)) {
    *taken = true;
    return RMW_RET_OK;
  }

  // 如果未能成功获取事件信息，则返回 RMW_RET_ERROR
  // If event information is not successfully taken, return RMW_RET_ERROR
  return RMW_RET_ERROR;
}

/**
 * @brief 从订阅中获取消息 (Take a message from a subscription)
 *
 * @param[in] identifier 指定的实现标识符 (The specified implementation identifier)
 * @param[in] subscription 订阅句柄，包含订阅信息 (Subscription handle containing the subscription
 * information)
 * @param[out] ros_message 存储取得的 ROS 消息的指针 (Pointer to store the taken ROS message)
 * @param[out] taken 如果成功获取到消息，则为 true，否则为 false (True if message is successfully
 * taken, otherwise false)
 * @param[in] allocation 订阅分配信息 (Subscription allocation information)
 * @return rmw_ret_t RMW_RET_OK 表示成功，其他值表示失败 (RMW_RET_OK indicates success, other values
 * indicate failure)
 */
rmw_ret_t __rmw_take(
    const char *identifier,
    const rmw_subscription_t *subscription,
    void *ros_message,
    bool *taken,
    rmw_subscription_allocation_t *allocation) {
  // 检查 subscription 是否为空，如果为空则返回 RMW_RET_INVALID_ARGUMENT 错误
  // Check if subscription is null, return RMW_RET_INVALID_ARGUMENT error if it is
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);

  // 检查 ros_message 是否为空，如果为空则返回 RMW_RET_INVALID_ARGUMENT 错误
  // Check if ros_message is null, return RMW_RET_INVALID_ARGUMENT error if it is
  RMW_CHECK_ARGUMENT_FOR_NULL(ros_message, RMW_RET_INVALID_ARGUMENT);

  // 检查 taken 是否为空，如果为空则返回 RMW_RET_INVALID_ARGUMENT 错误
  // Check if taken is null, return RMW_RET_INVALID_ARGUMENT error if it is
  RMW_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);

  // 调用 _take 函数并返回结果
  // Call the _take function and return the result
  return _take(identifier, subscription, ros_message, taken, nullptr, allocation);
}

/**
 * @brief 从订阅者中获取消息序列（Take a sequence of messages from a subscription）
 *
 * @param[in] identifier 指定的rmw实现的标识符（Identifier for the specified rmw implementation）
 * @param[in] subscription 订阅者指针（Pointer to the subscription）
 * @param[in] count 要获取的消息数量（Number of messages to take）
 * @param[out] message_sequence 存储获取到的消息序列的结构体指针（Pointer to the structure storing
 * the taken message sequence）
 * @param[out] message_info_sequence 存储获取到的消息信息序列的结构体指针（Pointer to the structure
 * storing the taken message info sequence）
 * @param[out] taken 实际获取到的消息数量（Actual number of messages taken）
 * @param[in] allocation 预留给特定rmw实现的分配器（Allocator reserved for a specific rmw
 * implementation）
 * @return rmw_ret_t 返回操作结果（Return the result of the operation）
 */
rmw_ret_t __rmw_take_sequence(
    const char *identifier,
    const rmw_subscription_t *subscription,
    size_t count,
    rmw_message_sequence_t *message_sequence,
    rmw_message_info_sequence_t *message_info_sequence,
    size_t *taken,
    rmw_subscription_allocation_t *allocation) {
  // 检查订阅者参数是否为空（Check if the subscription argument is NULL）
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);

  // 检查消息序列参数是否为空（Check if the message_sequence argument is NULL）
  RMW_CHECK_ARGUMENT_FOR_NULL(message_sequence, RMW_RET_INVALID_ARGUMENT);

  // 检查消息信息序列参数是否为空（Check if the message_info_sequence argument is NULL）
  RMW_CHECK_ARGUMENT_FOR_NULL(message_info_sequence, RMW_RET_INVALID_ARGUMENT);

  // 检查实际获取到的消息数量参数是否为空（Check if the taken argument is NULL）
  RMW_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);

  // 如果要获取的消息数量为0，则返回无效参数错误（If the number of messages to take is 0, return an
  // invalid argument error）
  if (0u == count) {
    RMW_SET_ERROR_MSG("count cannot be 0");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 如果消息序列的容量不足，则返回无效参数错误（If the capacity of the message_sequence is
  // insufficient, return an invalid argument error）
  if (count > message_sequence->capacity) {
    RMW_SET_ERROR_MSG("Insufficient capacity in message_sequence");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 如果消息信息序列的容量不足，则返回无效参数错误（If the capacity of the message_info_sequence is
  // insufficient, return an invalid argument error）
  if (count > message_info_sequence->capacity) {
    RMW_SET_ERROR_MSG("Insufficient capacity in message_info_sequence");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 调用内部函数_take_sequence来实际执行获取操作（Call the internal function _take_sequence to
  // actually perform the take operation）
  return _take_sequence(
      identifier, subscription, count, message_sequence, message_info_sequence, taken, allocation);
}

/**
 * @brief 从订阅器中获取消息，并提供相关信息 (Take a message from the subscription, providing its
 * related information)
 *
 * @param[in] identifier 指定的 rmw 实现的标识符 (Identifier of the specified rmw implementation)
 * @param[in] subscription 订阅器指针，用于接收消息 (Pointer to the subscription to receive messages
 * from)
 * @param[out] ros_message 存储接收到的 ROS 消息的指针 (Pointer to store the received ROS message)
 * @param[out] taken 一个布尔值，表示是否成功获取到消息 (A boolean value indicating whether a
 * message was successfully taken or not)
 * @param[out] message_info 存储接收到的消息的相关信息的指针 (Pointer to store the related
 * information of the received message)
 * @param[in] allocation 预分配的内存空间，用于存储接收到的消息 (Preallocated memory space for
 * storing the received message)
 *
 * @return 返回 rmw_ret_t 类型的结果，表示操作是否成功 (Returns an rmw_ret_t type result indicating
 * the success of the operation)
 */
rmw_ret_t __rmw_take_with_info(
    const char *identifier,
    const rmw_subscription_t *subscription,
    void *ros_message,
    bool *taken,
    rmw_message_info_t *message_info,
    rmw_subscription_allocation_t *allocation) {
  // 检查 message_info 参数是否为空，如果为空则返回 RMW_RET_INVALID_ARGUMENT 错误 (Check if the
  // message_info argument is NULL, and return RMW_RET_INVALID_ARGUMENT error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(message_info, RMW_RET_INVALID_ARGUMENT);

  // 检查 taken 参数是否为空，如果为空则返回 RMW_RET_INVALID_ARGUMENT 错误 (Check if the taken
  // argument is NULL, and return RMW_RET_INVALID_ARGUMENT error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);

  // 检查 ros_message 参数是否为空，如果为空则返回 RMW_RET_INVALID_ARGUMENT 错误 (Check if the
  // ros_message argument is NULL, and return RMW_RET_INVALID_ARGUMENT error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(ros_message, RMW_RET_INVALID_ARGUMENT);

  // 检查 subscription 参数是否为空，如果为空则返回 RMW_RET_INVALID_ARGUMENT 错误 (Check if the
  // subscription argument is NULL, and return RMW_RET_INVALID_ARGUMENT error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);

  // 调用 _take 函数并返回结果 (Call the _take function and return the result)
  return _take(identifier, subscription, ros_message, taken, message_info, allocation);
}

/**
 * @brief 从订阅者中获取序列化的消息 (Take a serialized message from the subscription)
 *
 * @param[in] identifier RMW实现的标识符 (Identifier of the RMW implementation)
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[out] serialized_message 序列化消息的输出 (Output for the serialized message)
 * @param[out] taken 是否成功获取到序列化消息的标志 (Flag indicating if a serialized message was
 * successfully taken)
 * @param[out] message_info 消息相关信息的输出 (Output for the message related information)
 * @param[in] allocation 分配器指针 (Pointer to the allocator)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t _take_serialized_message(
    const char *identifier,
    const rmw_subscription_t *subscription,
    rmw_serialized_message_t *serialized_message,
    bool *taken,
    rmw_message_info_t *message_info,
    rmw_subscription_allocation_t *allocation) {
  (void)allocation;  // 忽略分配器参数 (Ignore the allocator parameter)
  *taken = false;

  // 检查订阅者类型是否匹配 (Check if the subscription type matches)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      subscription handle, subscription->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 获取自定义订阅者信息 (Get the custom subscriber info)
  auto info = static_cast<CustomSubscriberInfo *>(subscription->data);
  // 检查自定义订阅者信息是否为空 (Check if the custom subscriber info is null)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(info, "custom subscriber info is null", return RMW_RET_ERROR);

  // 创建FastBuffer对象 (Create a FastBuffer object)
  eprosima::fastcdr::FastBuffer buffer;

  // 初始化序列化数据结构 (Initialize the serialized data structure)
  rmw_fastrtps_shared_cpp::SerializedData data;
  data.type = FASTRTPS_SERIALIZED_DATA_TYPE_CDR_BUFFER;
  data.data = &buffer;
  data.impl = nullptr;  // not used when type is FASTRTPS_SERIALIZED_DATA_TYPE_CDR_BUFFER

  // 创建数据值序列 (Create a data values sequence)
  eprosima::fastdds::dds::StackAllocatedSequence<void *, 1> data_values;
  const_cast<void **>(data_values.buffer())[0] = &data;
  // 创建样本信息序列 (Create a sample info sequence)
  eprosima::fastdds::dds::SampleInfoSeq info_seq{1};

  // 循环尝试获取序列化消息 (Loop to try taking the serialized message)
  while (ReturnCode_t::RETCODE_OK == info->data_reader_->take(data_values, info_seq, 1)) {
    // 设置作用域退出时重置数据值和信息序列的长度 (Set scope exit to reset data values and info
    // sequence length)
    auto reset = rcpputils::make_scope_exit([&]() {
      data_values.length(0);
      info_seq.length(0);
    });

    // 检查是否有有效数据 (Check if there is valid data)
    if (info_seq[0].valid_data) {
      // 获取缓冲区大小 (Get the buffer size)
      auto buffer_size = static_cast<size_t>(buffer.getBufferSize());
      // 检查序列化消息的缓冲区容量是否足够 (Check if the serialized message buffer capacity is
      // enough)
      if (serialized_message->buffer_capacity < buffer_size) {
        // 调整序列化消息的大小 (Resize the serialized message)
        auto ret = rmw_serialized_message_resize(serialized_message, buffer_size);
        if (ret != RMW_RET_OK) {
          return ret;  // Error message already set
        }
      }
      // 设置序列化消息的缓冲区长度 (Set the serialized message buffer length)
      serialized_message->buffer_length = buffer_size;
      // 复制数据到序列化消息的缓冲区 (Copy data to the serialized message buffer)
      memcpy(serialized_message->buffer, buffer.getBuffer(), serialized_message->buffer_length);

      // 如果提供了message_info参数，将信息分配给它 (If message_info parameter is provided, assign
      // the information to it)
      if (message_info) {
        _assign_message_info(identifier, message_info, &info_seq[0]);
      }
      // 设置已成功获取序列化消息的标志 (Set the flag indicating a serialized message was
      // successfully taken)
      *taken = true;
      break;
    }
  }

  return RMW_RET_OK;
}

/**
 * @brief 从订阅中获取序列化消息 (Take a serialized message from a subscription)
 *
 * @param[in] identifier 指定的RMW实现的标识符 (Identifier of the specified RMW implementation)
 * @param[in] subscription 订阅对象指针 (Pointer to the subscription object)
 * @param[out] serialized_message 存储序列化消息的结构体指针 (Pointer to the structure storing the
 * serialized message)
 * @param[out] taken 是否成功获取到序列化消息的布尔值指针 (Pointer to a boolean value indicating
 * whether a serialized message was successfully taken)
 * @param[in] allocation 订阅分配对象指针 (Pointer to the subscription allocation object)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_take_serialized_message(
    const char *identifier,
    const rmw_subscription_t *subscription,
    rmw_serialized_message_t *serialized_message,
    bool *taken,
    rmw_subscription_allocation_t *allocation) {
  // 检查订阅参数是否为空 (Check if the subscription argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);

  // 检查序列化消息参数是否为空 (Check if the serialized_message argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(serialized_message, RMW_RET_INVALID_ARGUMENT);

  // 检查taken参数是否为空 (Check if the taken argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);

  // 调用_take_serialized_message函数并返回结果 (Call the _take_serialized_message function and
  // return the result)
  return _take_serialized_message(
      identifier, subscription, serialized_message, taken, nullptr, allocation);
}

/**
 * @brief 从订阅中获取序列化消息及其相关信息 (Take a serialized message and its related info from a
 * subscription)
 *
 * @param[in] identifier 指定的RMW实现的标识符 (Identifier of the specified RMW implementation)
 * @param[in] subscription 订阅对象指针 (Pointer to the subscription object)
 * @param[out] serialized_message 存储序列化消息的结构体指针 (Pointer to the structure storing the
 * serialized message)
 * @param[out] taken 是否成功获取到序列化消息的布尔值指针 (Pointer to a boolean value indicating
 * whether a serialized message was successfully taken)
 * @param[out] message_info 存储消息相关信息的结构体指针 (Pointer to the structure storing the
 * message-related info)
 * @param[in] allocation 订阅分配对象指针 (Pointer to the subscription allocation object)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_take_serialized_message_with_info(
    const char *identifier,
    const rmw_subscription_t *subscription,
    rmw_serialized_message_t *serialized_message,
    bool *taken,
    rmw_message_info_t *message_info,
    rmw_subscription_allocation_t *allocation) {
  // 检查订阅参数是否为空 (Check if the subscription argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);

  // 检查序列化消息参数是否为空 (Check if the serialized_message argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(serialized_message, RMW_RET_INVALID_ARGUMENT);

  // 检查taken参数是否为空 (Check if the taken argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);

  // 检查消息信息参数是否为空 (Check if the message_info argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(message_info, RMW_RET_INVALID_ARGUMENT);

  // 调用_take_serialized_message函数并返回结果 (Call the _take_serialized_message function and
  // return the result)
  return _take_serialized_message(
      identifier, subscription, serialized_message, taken, message_info, allocation);
}

/**
 * @brief 从订阅者中获取动态消息 (Take a dynamic message from the subscriber)
 *
 * @param[in] identifier RMW实现的标识符 (Identifier of the RMW implementation)
 * @param[in] subscription 订阅者对象指针 (Pointer to the subscription object)
 * @param[out] dynamic_data 动态数据类型支持的数据结构 (Data structure for dynamic data type
 * support)
 * @param[out] taken 是否成功获取到消息的标志 (Flag indicating whether a message was successfully
 * taken)
 * @param[out] message_info 消息相关信息 (Message-related information)
 * @param[in] allocation 订阅者分配对象 (Subscription allocation object)
 * @return rmw_ret_t RMW操作结果 (Result of the RMW operation)
 */
rmw_ret_t _take_dynamic_message(
    const char *identifier,
    const rmw_subscription_t *subscription,
    rosidl_dynamic_typesupport_dynamic_data_t *dynamic_data,
    bool *taken,
    rmw_message_info_t *message_info,
    rmw_subscription_allocation_t *allocation) {
  // 忽略分配参数 (Ignore the allocation parameter)
  (void)allocation;
  // 初始化为未获取到消息 (Initialize as no message taken)
  *taken = false;

  // 检查订阅者和RMW实现的标识符是否匹配 (Check if the subscription and RMW implementation
  // identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      subscription handle, subscription->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 获取订阅者信息 (Get the subscriber information)
  auto info = static_cast<CustomSubscriberInfo *>(subscription->data);
  // 检查订阅者信息是否为空 (Check if the subscriber information is null)
  RCUTILS_CHECK_FOR_NULL_WITH_MSG(info, "custom subscriber info is null", return RMW_RET_ERROR);

  // 创建缓冲区 (Create a buffer)
  eprosima::fastcdr::FastBuffer buffer;

  // 初始化序列化数据结构 (Initialize the serialized data structure)
  rmw_fastrtps_shared_cpp::SerializedData data;
  data.type = FASTRTPS_SERIALIZED_DATA_TYPE_DYNAMIC_MESSAGE;
  data.data = dynamic_data->impl.handle;
  data.impl = nullptr;  // not used when type is FASTRTPS_SERIALIZED_DATA_TYPE_DYNAMIC_MESSAGE

  // 创建数据值序列 (Create a data values sequence)
  eprosima::fastdds::dds::StackAllocatedSequence<void *, 1> data_values;
  const_cast<void **>(data_values.buffer())[0] = &data;
  // 创建样本信息序列 (Create a sample info sequence)
  eprosima::fastdds::dds::SampleInfoSeq info_seq{1};

  // 循环尝试获取消息 (Loop to try taking messages)
  while (ReturnCode_t::RETCODE_OK == info->data_reader_->take(data_values, info_seq, 1)) {
    // info->data_reader_->take() 已经修改了dynamic_data参数 (info->data_reader_->take() has already
    // modified the dynamic_data argument) 参见rmw_fastrtps_shared_cpp/src/TypeSupport_impl.cpp (See
    // rmw_fastrtps_shared_cpp/src/TypeSupport_impl.cpp)

    // 使用作用域退出重置数据值和信息序列长度 (Reset the data values and info sequence length using
    // scope exit)
    auto reset = rcpputils::make_scope_exit([&]() {
      data_values.length(0);
      info_seq.length(0);
    });

    // 检查是否获取到有效数据 (Check if valid data is taken)
    if (info_seq[0].valid_data) {
      // 如果提供了message_info参数，分配消息信息 (If the message_info parameter is provided, assign
      // the message information)
      if (message_info) {
        _assign_message_info(identifier, message_info, &info_seq[0]);
      }
      // 设置已成功获取到消息的标志 (Set the flag indicating a message was successfully taken)
      *taken = true;
      break;
    }
  }

  // 返回操作结果 (Return the operation result)
  return RMW_RET_OK;
}

/**
 * @brief 接收动态消息 (Take a dynamic message)
 *
 * @param[in] identifier 用于标识 rmw 实现的字符串 (A string to identify the rmw implementation)
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[out] dynamic_data 动态数据类型支持的输出数据 (Output data with dynamic data type support)
 * @param[out] taken 是否成功接收到消息的标志 (Flag indicating whether a message was successfully
 * taken)
 * @param[in] allocation 分配给订阅者的内存 (Memory allocated for the subscription)
 * @return rmw_ret_t 操作结果状态码 (Operation result status code)
 */
rmw_ret_t __rmw_take_dynamic_message(
    const char *identifier,
    const rmw_subscription_t *subscription,
    rosidl_dynamic_typesupport_dynamic_data_t *dynamic_data,
    bool *taken,
    rmw_subscription_allocation_t *allocation) {
  // 检查订阅者参数是否为空 (Check if the subscription argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);

  // 检查动态数据参数是否为空 (Check if the dynamic_data argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(dynamic_data, RMW_RET_INVALID_ARGUMENT);

  // 检查 taken 参数是否为空 (Check if the taken argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);

  // 调用 _take_dynamic_message 函数并返回结果 (Call the _take_dynamic_message function and return
  // the result)
  return _take_dynamic_message(identifier, subscription, dynamic_data, taken, nullptr, allocation);
}

/**
 * @brief 接收动态消息并获取相关信息 (Take a dynamic message with additional info)
 *
 * @param[in] identifier 用于标识 rmw 实现的字符串 (A string to identify the rmw implementation)
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[out] dynamic_data 动态数据类型支持的输出数据 (Output data with dynamic data type support)
 * @param[out] taken 是否成功接收到消息的标志 (Flag indicating whether a message was successfully
 * taken)
 * @param[out] message_info 消息相关信息 (Message related information)
 * @param[in] allocation 分配给订阅者的内存 (Memory allocated for the subscription)
 * @return rmw_ret_t 操作结果状态码 (Operation result status code)
 */
rmw_ret_t __rmw_take_dynamic_message_with_info(
    const char *identifier,
    const rmw_subscription_t *subscription,
    rosidl_dynamic_typesupport_dynamic_data_t *dynamic_data,
    bool *taken,
    rmw_message_info_t *message_info,
    rmw_subscription_allocation_t *allocation) {
  // 检查订阅者参数是否为空 (Check if the subscription argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);

  // 检查动态数据参数是否为空 (Check if the dynamic_data argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(dynamic_data, RMW_RET_INVALID_ARGUMENT);

  // 检查 taken 参数是否为空 (Check if the taken argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);

  // 检查 message_info 参数是否为空 (Check if the message_info argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(message_info, RMW_RET_INVALID_ARGUMENT);

  // 调用 _take_dynamic_message 函数并返回结果 (Call the _take_dynamic_message function and return
  // the result)
  return _take_dynamic_message(
      identifier, subscription, dynamic_data, taken, message_info, allocation);
}

// ----------------- Loans related code ------------------------- //

// 结构体 GenericSequence，继承自 eprosima::fastdds::dds::LoanableCollection 类
// (Struct GenericSequence, inherits from eprosima::fastdds::dds::LoanableCollection class)
struct GenericSequence : public eprosima::fastdds::dds::LoanableCollection {
  // 默认构造函数
  // (Default constructor)
  GenericSequence() = default;

  // 重写 resize 函数，接收一个 size_type 类型的参数 new_length
  // (Override the resize function, accepts a size_type parameter named new_length)
  void resize(size_type /*new_length*/) override {
    // 这种集合只应与贷款一起使用
    // (This kind of collection should only be used with loans)
    throw std::bad_alloc();
  }
};

// 结构体 LoanManager
// (Struct LoanManager)
struct LoanManager {
  // 内部结构体 Item
  // (Inner struct Item)
  struct Item {
    // GenericSequence 类型的成员变量 data_seq
    // (Member variable of type GenericSequence named data_seq)
    GenericSequence data_seq{};
    // eprosima::fastdds::dds::SampleInfoSeq 类型的成员变量 info_seq
    // (Member variable of type eprosima::fastdds::dds::SampleInfoSeq named info_seq)
    eprosima::fastdds::dds::SampleInfoSeq info_seq{};
  };

  // 显式构造函数，接收一个 eprosima::fastrtps::ResourceLimitedContainerConfig 类型的参数 items_cfg
  // (Explicit constructor, accepts a parameter of type
  // eprosima::fastrtps::ResourceLimitedContainerConfig named items_cfg)
  explicit LoanManager(const eprosima::fastrtps::ResourceLimitedContainerConfig &items_cfg)
      : items(items_cfg) {}

  // 添加 item 的函数，接收一个 std::unique_ptr<Item> 类型的参数 item
  // (Function to add an item, accepts a parameter of type std::unique_ptr<Item> named item)
  void add_item(std::unique_ptr<Item> item) {
    // 使用互斥锁保护线程安全
    // (Use mutex lock to protect thread safety)
    std::lock_guard<std::mutex> guard(mtx);
    // 将 item 添加到 items 向量中
    // (Add the item to the items vector)
    items.push_back(std::move(item));
  }

  // 删除 item 的函数，接收一个 void * 类型的参数 loaned_message
  // (Function to erase an item, accepts a parameter of type void* named loaned_message)
  std::unique_ptr<Item> erase_item(void *loaned_message) {
    // 初始化一个空的 std::unique_ptr<Item> 类型的变量 ret
    // (Initialize an empty variable of type std::unique_ptr<Item> named ret)
    std::unique_ptr<Item> ret{nullptr};

    // 使用互斥锁保护线程安全
    // (Use mutex lock to protect thread safety)
    std::lock_guard<std::mutex> guard(mtx);
    // 遍历 items 向量
    // (Iterate through the items vector)
    for (auto it = items.begin(); it != items.end(); ++it) {
      // 如果找到与 loaned_message 匹配的 item
      // (If an item matching loaned_message is found)
      if (loaned_message == (*it)->data_seq.buffer()[0]) {
        // 移动找到的 item 到 ret
        // (Move the found item to ret)
        ret = std::move(*it);
        // 从 items 向量中删除找到的 item
        // (Erase the found item from the items vector)
        items.erase(it);
        break;
      }
    }

    // 返回 ret
    // (Return ret)
    return ret;
  }

private:
  // 定义一个互斥锁变量 mtx
  // (Define a mutex variable named mtx)
  std::mutex mtx;
  // 使用 eprosima::fastrtps::ResourceLimitedVector 类型定义一个 ItemVector
  // (Define an ItemVector using eprosima::fastrtps::ResourceLimitedVector type)
  using ItemVector = eprosima::fastrtps::ResourceLimitedVector<std::unique_ptr<Item>>;
  // 定义一个 ItemVector 类型的成员变量 items，使用 RCPPUTILS_TSA_GUARDED_BY 宏保护线程安全
  // (Define a member variable of type ItemVector named items, use RCPPUTILS_TSA_GUARDED_BY macro to
  // protect thread safety)
  ItemVector items RCPPUTILS_TSA_GUARDED_BY(mtx);
};

/**
 * @brief 初始化订阅器的消息借用功能（Initialize the message loaning feature for a subscription）
 *
 * @param[in] subscription 要初始化的订阅器指针（Pointer to the subscription to be initialized）
 */
void __init_subscription_for_loans(rmw_subscription_t *subscription) {
  // 将订阅器的数据转换为 CustomSubscriberInfo 类型（Cast the subscription data to
  // CustomSubscriberInfo type） Convert the subscription data to CustomSubscriberInfo type
  auto info = static_cast<CustomSubscriberInfo *>(subscription->data);

  // 获取数据阅读器的 QoS 设置（Get the QoS settings of the data reader）
  // Get the QoS settings of the data reader
  const auto &qos = info->data_reader_->get_qos();

  // 检查类型支持是否为简单类型，如果是，则设置 can_loan_messages 为 true（Check if the type support
  // is plain, and set can_loan_messages to true if it is） Check if the type support is plain, and
  // set can_loan_messages to true if it is
  subscription->can_loan_messages = info->type_support_->is_plain();

  // 如果可以借用消息（If messages can be loaned）
  if (subscription->can_loan_messages) {
    // 获取 QoS 中的资源限制设置（Get the resource limits settings from QoS）
    // Get the resource limits settings from QoS
    const auto &allocation_qos = qos.reader_resource_limits().outstanding_reads_allocation;

    // 使用资源限制设置创建 LoanManager 实例，并将其存储在 info 中（Create a LoanManager instance
    // with the resource limits settings and store it in info） Create a LoanManager instance with
    // the resource limits settings and store it in info
    info->loan_manager_ = std::make_shared<LoanManager>(allocation_qos);
  }
}

/**
 * @brief 从订阅者中获取一条借用的消息（Take a loaned message from the subscription）
 *
 * @param[in] identifier RMW 实现的标识符（Identifier of the RMW implementation）
 * @param[in] subscription 订阅者指针（Pointer to the subscription）
 * @param[out] loaned_message 存储借用消息的指针（Pointer to store the loaned message）
 * @param[out] taken 是否成功获取到消息的标志（Flag indicating if a message was successfully taken）
 * @param[out] message_info 消息相关信息（Message related information）
 * @return rmw_ret_t RMW 返回状态码（RMW return status code）
 */
rmw_ret_t __rmw_take_loaned_message_internal(
    const char *identifier,
    const rmw_subscription_t *subscription,
    void **loaned_message,
    bool *taken,
    rmw_message_info_t *message_info) {
  // 检查订阅者是否为空（Check if the subscription is null）
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);

  // 检查订阅者类型与实现标识符是否匹配（Check if the subscription type matches the implementation
  // identifier）
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      subscription, subscription->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查订阅者是否支持消息借用（Check if the subscription supports message loaning）
  if (!subscription->can_loan_messages) {
    RMW_SET_ERROR_MSG("Loaning is not supported");
    return RMW_RET_UNSUPPORTED;
  }

  // 检查借用消息的指针是否为空（Check if the loaned message pointer is null）
  RMW_CHECK_ARGUMENT_FOR_NULL(loaned_message, RMW_RET_INVALID_ARGUMENT);

  // 检查 taken 参数是否为空（Check if the taken parameter is null）
  RMW_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);

  // 获取订阅者信息（Get the subscriber information）
  auto info = static_cast<CustomSubscriberInfo *>(subscription->data);

  // 创建一个 LoanManager::Item 对象（Create a LoanManager::Item object）
  auto item = std::make_unique<rmw_fastrtps_shared_cpp::LoanManager::Item>();

  // 循环尝试获取一条借用的消息（Loop to try taking a loaned message）
  while (ReturnCode_t::RETCODE_OK == info->data_reader_->take(item->data_seq, item->info_seq, 1)) {
    // 检查获取到的消息是否有效（Check if the taken message is valid）
    if (item->info_seq[0].valid_data) {
      // 如果提供了 message_info 参数，将消息信息赋值给它（If the message_info parameter is
      // provided, assign the message information to it）
      if (nullptr != message_info) {
        _assign_message_info(identifier, message_info, &item->info_seq[0]);
      }

      // 将借用的消息存储到 loaned_message 中（Store the loaned message in loaned_message）
      *loaned_message = item->data_seq.buffer()[0];

      // 设置 taken 标志为 true（Set the taken flag to true）
      *taken = true;

      // 将 item 添加到 LoanManager 中（Add the item to the LoanManager）
      info->loan_manager_->add_item(std::move(item));

      // 返回成功状态码（Return the success status code）
      return RMW_RET_OK;
    }

    // 在再次尝试获取消息之前，应该归还借用的消息（Return the loaned message before trying to take
    // again）
    info->data_reader_->return_loan(item->data_seq, item->info_seq);
  }

  // 如果没有可用数据，设置 taken 标志为 false 并返回成功状态码（If no data is available, set the
  // taken flag to false and return the success status code）
  *taken = false;
  return RMW_RET_OK;
}

/**
 * @brief 从订阅者返回借用的消息 (Return a loaned message from the subscription)
 *
 * @param[in] identifier 指示符，用于检查实现是否匹配 (Identifier used to check if the
 * implementation matches)
 * @param[in] subscription 订阅者指针，不能为空 (Pointer to the subscription, must not be null)
 * @param[in] loaned_message 借用的消息指针，不能为空 (Pointer to the loaned message, must not be
 * null)
 * @return rmw_ret_t 返回操作状态 (Return the operation status)
 */
rmw_ret_t __rmw_return_loaned_message_from_subscription(
    const char *identifier, const rmw_subscription_t *subscription, void *loaned_message) {
  // 检查订阅者参数是否为空 (Check if the subscription argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);
  // 检查类型标识符是否匹配 (Check if the type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      subscription, subscription->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查订阅者是否支持借用消息 (Check if the subscription supports loaning messages)
  if (!subscription->can_loan_messages) {
    RMW_SET_ERROR_MSG("Loaning is not supported");
    return RMW_RET_UNSUPPORTED;
  }
  // 检查借用的消息参数是否为空 (Check if the loaned message argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(loaned_message, RMW_RET_INVALID_ARGUMENT);

  // 获取订阅者信息 (Get the subscription info)
  auto info = static_cast<CustomSubscriberInfo *>(subscription->data);
  // 创建一个独特的指针，用于管理借用的消息 (Create a unique pointer for managing the loaned
  // message)
  std::unique_ptr<rmw_fastrtps_shared_cpp::LoanManager::Item> item;
  // 从订阅者信息中删除借用的消息 (Erase the loaned message from the subscription info)
  item = info->loan_manager_->erase_item(loaned_message);
  // 检查是否成功删除借用的消息 (Check if the loaned message was successfully erased)
  if (item != nullptr) {
    // 尝试返回借用的消息 (Try to return the loaned message)
    if (!info->data_reader_->return_loan(item->data_seq, item->info_seq)) {
      RMW_SET_ERROR_MSG("Error returning loan");
      return RMW_RET_ERROR;
    }

    // 返回操作成功状态 (Return the operation success status)
    return RMW_RET_OK;
  }

  // 设置错误消息，表示尝试返回未由此订阅者借用的消息 (Set error message indicating an attempt to
  // return a message not loaned by this subscription)
  RMW_SET_ERROR_MSG("Trying to return message not loaned by this subscription");
  return RMW_RET_ERROR;
}

}  // namespace rmw_fastrtps_shared_cpp
