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

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/serialized_message.h"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 从订阅者中获取消息 (Take a message from the subscription)
 *
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[out] ros_message 存储接收到的消息的指针 (Pointer to store the received message)
 * @param[out] taken 是否成功获取到消息的标志 (Flag indicating if a message was successfully taken)
 * @param[in] allocation 订阅分配指针 (Pointer to the subscription allocation)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_take(
    const rmw_subscription_t* subscription,
    void* ros_message,
    bool* taken,
    rmw_subscription_allocation_t* allocation) {
  // 调用共享函数 __rmw_take 进行消息获取 (Call the shared function __rmw_take to take the message)
  return rmw_fastrtps_shared_cpp::__rmw_take(
      eprosima_fastrtps_identifier, subscription, ros_message, taken, allocation);
}

/**
 * @brief 从订阅者中获取消息及其相关信息 (Take a message and its related info from the subscription)
 *
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[out] ros_message 存储接收到的消息的指针 (Pointer to store the received message)
 * @param[out] taken 是否成功获取到消息的标志 (Flag indicating if a message was successfully taken)
 * @param[out] message_info 存储接收到的消息相关信息的指针 (Pointer to store the received message's
 * info)
 * @param[in] allocation 订阅分配指针 (Pointer to the subscription allocation)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_take_with_info(
    const rmw_subscription_t* subscription,
    void* ros_message,
    bool* taken,
    rmw_message_info_t* message_info,
    rmw_subscription_allocation_t* allocation) {
  // 调用共享函数 __rmw_take_with_info 进行消息及其相关信息的获取 (Call the shared function
  // __rmw_take_with_info to take the message and its related info)
  return rmw_fastrtps_shared_cpp::__rmw_take_with_info(
      eprosima_fastrtps_identifier, subscription, ros_message, taken, message_info, allocation);
}

/**
 * @brief 从订阅者中获取一系列消息 (Take a sequence of messages from the subscription)
 *
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[in] count 要获取的消息数量 (Number of messages to take)
 * @param[out] message_sequence 存储接收到的消息序列的指针 (Pointer to store the received message
 * sequence)
 * @param[out] message_info_sequence 存储接收到的消息相关信息序列的指针 (Pointer to store the
 * received message info sequence)
 * @param[out] taken 实际获取到的消息数量 (Actual number of messages taken)
 * @param[in] allocation 订阅分配指针 (Pointer to the subscription allocation)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_take_sequence(
    const rmw_subscription_t* subscription,
    size_t count,
    rmw_message_sequence_t* message_sequence,
    rmw_message_info_sequence_t* message_info_sequence,
    size_t* taken,
    rmw_subscription_allocation_t* allocation) {
  // 调用共享函数 __rmw_take_sequence 进行一系列消息的获取 (Call the shared function
  // __rmw_take_sequence to take a sequence of messages)
  return rmw_fastrtps_shared_cpp::__rmw_take_sequence(
      eprosima_fastrtps_identifier, subscription, count, message_sequence, message_info_sequence,
      taken, allocation);
}

/**
 * @brief 从订阅者中获取序列化的消息 (Take a serialized message from the subscription)
 *
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[out] serialized_message 存储接收到的序列化消息的指针 (Pointer to store the received
 * serialized message)
 * @param[out] taken 是否成功获取到序列化消息的标志 (Flag indicating if a serialized message was
 * successfully taken)
 * @param[in] allocation 订阅分配指针 (Pointer to the subscription allocation)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_take_serialized_message(
    const rmw_subscription_t* subscription,
    rmw_serialized_message_t* serialized_message,
    bool* taken,
    rmw_subscription_allocation_t* allocation) {
  // 调用共享函数 __rmw_take_serialized_message 进行序列化消息的获取 (Call the shared function
  // __rmw_take_serialized_message to take the serialized message)
  return rmw_fastrtps_shared_cpp::__rmw_take_serialized_message(
      eprosima_fastrtps_identifier, subscription, serialized_message, taken, allocation);
}

/**
 * @brief 从订阅者中获取序列化的消息及其相关信息 (Take a serialized message and its related info
 * from the subscription)
 *
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[out] serialized_message 存储接收到的序列化消息的指针 (Pointer to store the received
 * serialized message)
 * @param[out] taken 是否成功获取到序列化消息的标志 (Flag indicating if a serialized message was
 * successfully taken)
 * @param[out] message_info 存储接收到的序列化消息相关信息的指针 (Pointer to store the received
 * serialized message's info)
 * @param[in] allocation 订阅分配指针 (Pointer to the subscription allocation)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_take_serialized_message_with_info(
    const rmw_subscription_t* subscription,
    rmw_serialized_message_t* serialized_message,
    bool* taken,
    rmw_message_info_t* message_info,
    rmw_subscription_allocation_t* allocation) {
  // 调用共享函数 __rmw_take_serialized_message_with_info 进行序列化消息及其相关信息的获取 (Call the
  // shared function __rmw_take_serialized_message_with_info to take the serialized message and its
  // related info)
  return rmw_fastrtps_shared_cpp::__rmw_take_serialized_message_with_info(
      eprosima_fastrtps_identifier, subscription, serialized_message, taken, message_info,
      allocation);
}

/**
 * @brief 从订阅中获取借用的消息 (Take a loaned message from a subscription)
 *
 * @param[in] subscription 订阅指针 (Pointer to the subscription)
 * @param[out] loaned_message 借用的消息指针 (Pointer to the loaned message)
 * @param[out] taken 是否成功获取到消息的标志 (Flag indicating if a message was successfully taken)
 * @param[in] allocation 分配器指针 (Pointer to the allocator)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_take_loaned_message(
    const rmw_subscription_t* subscription,
    void** loaned_message,
    bool* taken,
    rmw_subscription_allocation_t* allocation) {
  // 忽略分配器参数 (Ignore the allocator argument)
  static_cast<void>(allocation);
  // 调用内部函数来处理借用消息的获取 (Call the internal function to handle taking the loaned
  // message)
  return rmw_fastrtps_shared_cpp::__rmw_take_loaned_message_internal(
      eprosima_fastrtps_identifier, subscription, loaned_message, taken, nullptr);
}

/**
 * @brief 从订阅中获取带有信息的借用消息 (Take a loaned message with info from a subscription)
 *
 * @param[in] subscription 订阅指针 (Pointer to the subscription)
 * @param[out] loaned_message 借用的消息指针 (Pointer to the loaned message)
 * @param[out] taken 是否成功获取到消息的标志 (Flag indicating if a message was successfully taken)
 * @param[out] message_info 消息信息指针 (Pointer to the message info)
 * @param[in] allocation 分配器指针 (Pointer to the allocator)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_take_loaned_message_with_info(
    const rmw_subscription_t* subscription,
    void** loaned_message,
    bool* taken,
    rmw_message_info_t* message_info,
    rmw_subscription_allocation_t* allocation) {
  // 忽略分配器参数 (Ignore the allocator argument)
  static_cast<void>(allocation);
  // 检查 message_info 参数是否为空 (Check if the message_info argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(message_info, RMW_RET_INVALID_ARGUMENT);
  // 调用内部函数来处理带有信息的借用消息的获取 (Call the internal function to handle taking the
  // loaned message with info)
  return rmw_fastrtps_shared_cpp::__rmw_take_loaned_message_internal(
      eprosima_fastrtps_identifier, subscription, loaned_message, taken, message_info);
}

/**
 * @brief 归还从订阅中借用的消息 (Return a loaned message from a subscription)
 *
 * @param[in] subscription 订阅指针 (Pointer to the subscription)
 * @param[in] loaned_message 借用的消息指针 (Pointer to the loaned message)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_return_loaned_message_from_subscription(
    const rmw_subscription_t* subscription, void* loaned_message) {
  // 调用内部函数来处理归还借用的消息 (Call the internal function to handle returning the loaned
  // message)
  return rmw_fastrtps_shared_cpp::__rmw_return_loaned_message_from_subscription(
      eprosima_fastrtps_identifier, subscription, loaned_message);
}

/**
 * @brief 从事件句柄中获取事件 (Take an event from an event handle)
 *
 * @param[in] event_handle 事件句柄指针 (Pointer to the event handle)
 * @param[out] event_info 事件信息指针 (Pointer to the event info)
 * @param[out] taken 是否成功获取到事件的标志 (Flag indicating if an event was successfully taken)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_take_event(const rmw_event_t* event_handle, void* event_info, bool* taken) {
  // 调用内部函数来处理获取事件 (Call the internal function to handle taking the event)
  return rmw_fastrtps_shared_cpp::__rmw_take_event(
      eprosima_fastrtps_identifier, event_handle, event_info, taken);
}

}  // extern "C"
