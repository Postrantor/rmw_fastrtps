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
#include "rmw/rmw.h"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 发布 ROS 消息 (Publish a ROS message)
 *
 * @param[in] publisher 指向要发布消息的 rmw_publisher_t 结构体的指针 (Pointer to the
 * rmw_publisher_t structure for publishing the message)
 * @param[in] ros_message 要发布的 ROS 消息的指针 (Pointer to the ROS message to be published)
 * @param[in] allocation 指向分配给发布器的 rmw_publisher_allocation_t 结构体的指针 (Pointer to the
 * rmw_publisher_allocation_t structure allocated for the publisher)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_publish(
    const rmw_publisher_t* publisher,
    const void* ros_message,
    rmw_publisher_allocation_t* allocation) {
  // 调用共享函数 __rmw_publish 并传入 FastRTPS 标识符，发布器，ROS 消息和分配信息 (Call the shared
  // function __rmw_publish with FastRTPS identifier, publisher, ROS message and allocation info)
  return rmw_fastrtps_shared_cpp::__rmw_publish(
      eprosima_fastrtps_identifier, publisher, ros_message, allocation);
}

/**
 * @brief 发布借用的 ROS 消息 (Publish a loaned ROS message)
 *
 * @param[in] publisher 指向要发布消息的 rmw_publisher_t 结构体的指针 (Pointer to the
 * rmw_publisher_t structure for publishing the message)
 * @param[in] ros_message 要发布的借用的 ROS 消息的指针 (Pointer to the loaned ROS message to be
 * published)
 * @param[in] allocation 指向分配给发布器的 rmw_publisher_allocation_t 结构体的指针 (Pointer to the
 * rmw_publisher_allocation_t structure allocated for the publisher)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_publish_loaned_message(
    const rmw_publisher_t* publisher, void* ros_message, rmw_publisher_allocation_t* allocation) {
  // 调用共享函数 __rmw_publish_loaned_message 并传入 FastRTPS 标识符，发布器，借用的 ROS
  // 消息和分配信息 (Call the shared function __rmw_publish_loaned_message with FastRTPS identifier,
  // publisher, loaned ROS message and allocation info)
  return rmw_fastrtps_shared_cpp::__rmw_publish_loaned_message(
      eprosima_fastrtps_identifier, publisher, ros_message, allocation);
}

/**
 * @brief 发布序列化的 ROS 消息 (Publish a serialized ROS message)
 *
 * @param[in] publisher 指向要发布消息的 rmw_publisher_t 结构体的指针 (Pointer to the
 * rmw_publisher_t structure for publishing the message)
 * @param[in] serialized_message 要发布的序列化的 ROS 消息的指针 (Pointer to the serialized ROS
 * message to be published)
 * @param[in] allocation 指向分配给发布器的 rmw_publisher_allocation_t 结构体的指针 (Pointer to the
 * rmw_publisher_allocation_t structure allocated for the publisher)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_publish_serialized_message(
    const rmw_publisher_t* publisher,
    const rmw_serialized_message_t* serialized_message,
    rmw_publisher_allocation_t* allocation) {
  // 调用共享函数 __rmw_publish_serialized_message 并传入 FastRTPS 标识符，发布器，序列化的 ROS
  // 消息和分配信息 (Call the shared function __rmw_publish_serialized_message with FastRTPS
  // identifier, publisher, serialized ROS message and allocation info)
  return rmw_fastrtps_shared_cpp::__rmw_publish_serialized_message(
      eprosima_fastrtps_identifier, publisher, serialized_message, allocation);
}

}  // extern "C"
