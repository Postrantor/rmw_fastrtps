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
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 从订阅中获取一条消息 (Take a single message from the subscription)
 *
 * @param[in] subscription 订阅指针 (Pointer to the subscription)
 * @param[out] ros_message 存储接收到的消息的指针 (Pointer to store the received message)
 * @param[out] taken 是否成功接收到消息的标志 (Flag indicating whether a message was successfully
 * taken)
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
 * @brief 从订阅中获取一条带有信息的消息 (Take a single message with info from the subscription)
 *
 * @param[in] subscription 订阅指针 (Pointer to the subscription)
 * @param[out] ros_message 存储接收到的消息的指针 (Pointer to store the received message)
 * @param[out] taken 是否成功接收到消息的标志 (Flag indicating whether a message was successfully
 * taken)
 * @param[out] message_info 存储消息相关信息的指针 (Pointer to store the message related info)
 * @param[in] allocation 订阅分配指针 (Pointer to the subscription allocation)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_take_with_info(
    const rmw_subscription_t* subscription,
    void* ros_message,
    bool* taken,
    rmw_message_info_t* message_info,
    rmw_subscription_allocation_t* allocation) {
  // 调用共享函数 __rmw_take_with_info 进行带有信息的消息获取 (Call the shared function
  // __rmw_take_with_info to take the message with info)
  return rmw_fastrtps_shared_cpp::__rmw_take_with_info(
      eprosima_fastrtps_identifier, subscription, ros_message, taken, message_info, allocation);
}

/**
 * @brief 从订阅中获取一系列消息 (Take a sequence of messages from the subscription)
 *
 * @param[in] subscription 订阅指针 (Pointer to the subscription)
 * @param[in] count 要获取的消息数量 (Number of messages to take)
 * @param[out] message_sequence 存储接收到的消息序列的指针 (Pointer to store the received message
 * sequence)
 * @param[out] message_info_sequence 存储消息相关信息序列的指针 (Pointer to store the message
 * related info sequence)
 * @param[out] taken 实际成功接收到的消息数量 (Actual number of messages successfully taken)
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
  // 调用共享函数 __rmw_take_sequence 进行消息序列获取 (Call the shared function __rmw_take_sequence
  // to take the message sequence)
  return rmw_fastrtps_shared_cpp::__rmw_take_sequence(
      eprosima_fastrtps_identifier, subscription, count, message_sequence, message_info_sequence,
      taken, allocation);
}

/**
 * @brief 从订阅者中获取序列化的消息 (Take a serialized message from the subscription)
 *
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[out] serialized_message 序列化消息指针 (Pointer to the serialized message)
 * @param[out] taken 是否成功获取到消息的标志 (Flag indicating if a message was successfully taken)
 * @param[in] allocation 订阅者分配指针 (Pointer to the subscription allocation)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_take_serialized_message(
    const rmw_subscription_t* subscription,
    rmw_serialized_message_t* serialized_message,
    bool* taken,
    rmw_subscription_allocation_t* allocation) {
  // 调用共享函数并返回结果 (Call the shared function and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_take_serialized_message(
      eprosima_fastrtps_identifier, subscription, serialized_message, taken, allocation);
}

/**
 * @brief 从订阅者中获取序列化的消息和相关信息 (Take a serialized message and its info from the
 * subscription)
 *
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[out] serialized_message 序列化消息指针 (Pointer to the serialized message)
 * @param[out] taken 是否成功获取到消息的标志 (Flag indicating if a message was successfully taken)
 * @param[out] message_info 消息信息指针 (Pointer to the message info)
 * @param[in] allocation 订阅者分配指针 (Pointer to the subscription allocation)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_take_serialized_message_with_info(
    const rmw_subscription_t* subscription,
    rmw_serialized_message_t* serialized_message,
    bool* taken,
    rmw_message_info_t* message_info,
    rmw_subscription_allocation_t* allocation) {
  // 调用共享函数并返回结果 (Call the shared function and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_take_serialized_message_with_info(
      eprosima_fastrtps_identifier, subscription, serialized_message, taken, message_info,
      allocation);
}

/**
 * @brief 从订阅者中获取借用的消息 (Take a loaned message from the subscription)
 *
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[out] loaned_message 借用消息指针 (Pointer to the loaned message)
 * @param[out] taken 是否成功获取到消息的标志 (Flag indicating if a message was successfully taken)
 * @param[in] allocation 订阅者分配指针 (Pointer to the subscription allocation)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_take_loaned_message(
    const rmw_subscription_t* subscription,
    void** loaned_message,
    bool* taken,
    rmw_subscription_allocation_t* allocation) {
  // 忽略分配参数 (Ignore the allocation parameter)
  static_cast<void>(allocation);
  // 调用内部共享函数并返回结果 (Call the internal shared function and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_take_loaned_message_internal(
      eprosima_fastrtps_identifier, subscription, loaned_message, taken, nullptr);
}

/**
 * @brief 从订阅者中获取借用的消息和相关信息 (Take a loaned message and its info from the
 * subscription)
 *
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[out] loaned_message 借用消息指针 (Pointer to the loaned message)
 * @param[out] taken 是否成功获取到消息的标志 (Flag indicating if a message was successfully taken)
 * @param[out] message_info 消息信息指针 (Pointer to the message info)
 * @param[in] allocation 订阅者分配指针 (Pointer to the subscription allocation)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_take_loaned_message_with_info(
    const rmw_subscription_t* subscription,
    void** loaned_message,
    bool* taken,
    rmw_message_info_t* message_info,
    rmw_subscription_allocation_t* allocation) {
  // 忽略分配参数 (Ignore the allocation parameter)
  static_cast<void>(allocation);
  // 检查消息信息参数是否为空 (Check if the message info argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(message_info, RMW_RET_INVALID_ARGUMENT);
  // 调用内部共享函数并返回结果 (Call the internal shared function and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_take_loaned_message_internal(
      eprosima_fastrtps_identifier, subscription, loaned_message, taken, message_info);
}

/**
 * @brief 从订阅中返回借用的消息 (Return a loaned message from a subscription)
 *
 * @param[in] subscription 订阅指针 (Pointer to the subscription)
 * @param[out] loaned_message 借用的消息指针 (Pointer to the loaned message)
 * @return rmw_ret_t 返回操作状态 (Return the operation status)
 */
rmw_ret_t rmw_return_loaned_message_from_subscription(
    const rmw_subscription_t* subscription, void* loaned_message) {
  // 调用共享函数，传入 FastRTPS 标识符、订阅和借用的消息
  // (Call the shared function with FastRTPS identifier, subscription and loaned message)
  return rmw_fastrtps_shared_cpp::__rmw_return_loaned_message_from_subscription(
      eprosima_fastrtps_identifier, subscription, loaned_message);
}

/**
 * @brief 获取事件 (Take an event)
 *
 * @param[in] event_handle 事件句柄指针 (Pointer to the event handle)
 * @param[out] event_info 事件信息指针 (Pointer to the event information)
 * @param[out] taken 是否成功获取事件的标志 (Flag indicating whether the event was successfully
 * taken or not)
 * @return rmw_ret_t 返回操作状态 (Return the operation status)
 */
rmw_ret_t rmw_take_event(const rmw_event_t* event_handle, void* event_info, bool* taken) {
  // 调用共享函数，传入 FastRTPS 标识符、事件句柄、事件信息和取走标志
  // (Call the shared function with FastRTPS identifier, event handle, event information and taken
  // flag)
  return rmw_fastrtps_shared_cpp::__rmw_take_event(
      eprosima_fastrtps_identifier, event_handle, event_info, taken);
}

}  // extern "C"
