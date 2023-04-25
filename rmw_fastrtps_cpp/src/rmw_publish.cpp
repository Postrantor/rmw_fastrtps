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
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 发布一个 ROS 消息 (Publish a ROS message)
 *
 * @param[in] publisher 一个有效的 rmw_publisher_t 结构体指针 (A valid pointer to an rmw_publisher_t
 * structure)
 * @param[in] ros_message 要发布的 ROS 消息 (The ROS message to be published)
 * @param[in,out] allocation 预先分配内存的结构体指针，可为 nullptr (A pointer to a preallocated
 * memory structure, can be nullptr)
 *
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_publish(
    const rmw_publisher_t* publisher,
    const void* ros_message,
    rmw_publisher_allocation_t* allocation) {
  // 调用 __rmw_publish 函数，并传入 eprosima_fastrtps_identifier 作为实现标识符
  // Call the __rmw_publish function and pass in eprosima_fastrtps_identifier as the implementation
  // identifier
  return rmw_fastrtps_shared_cpp::__rmw_publish(
      eprosima_fastrtps_identifier, publisher, ros_message, allocation);
}

/**
 * @brief 发布一个序列化的 ROS 消息 (Publish a serialized ROS message)
 *
 * @param[in] publisher 一个有效的 rmw_publisher_t 结构体指针 (A valid pointer to an rmw_publisher_t
 * structure)
 * @param[in] serialized_message 要发布的序列化 ROS 消息 (The serialized ROS message to be
 * published)
 * @param[in,out] allocation 预先分配内存的结构体指针，可为 nullptr (A pointer to a preallocated
 * memory structure, can be nullptr)
 *
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_publish_serialized_message(
    const rmw_publisher_t* publisher,
    const rmw_serialized_message_t* serialized_message,
    rmw_publisher_allocation_t* allocation) {
  // 调用 __rmw_publish_serialized_message 函数，并传入 eprosima_fastrtps_identifier 作为实现标识符
  // Call the __rmw_publish_serialized_message function and pass in eprosima_fastrtps_identifier as
  // the implementation identifier
  return rmw_fastrtps_shared_cpp::__rmw_publish_serialized_message(
      eprosima_fastrtps_identifier, publisher, serialized_message, allocation);
}

/**
 * @brief 发布一个借用的 ROS 消息 (Publish a loaned ROS message)
 *
 * @param[in] publisher 一个有效的 rmw_publisher_t 结构体指针 (A valid pointer to an rmw_publisher_t
 * structure)
 * @param[in] ros_message 要发布的借用 ROS 消息 (The loaned ROS message to be published)
 * @param[in,out] allocation 预先分配内存的结构体指针，可为 nullptr (A pointer to a preallocated
 * memory structure, can be nullptr)
 *
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_publish_loaned_message(
    const rmw_publisher_t* publisher, void* ros_message, rmw_publisher_allocation_t* allocation) {
  // 调用 __rmw_publish_loaned_message 函数，并传入 eprosima_fastrtps_identifier 作为实现标识符
  // Call the __rmw_publish_loaned_message function and pass in eprosima_fastrtps_identifier as the
  // implementation identifier
  return rmw_fastrtps_shared_cpp::__rmw_publish_loaned_message(
      eprosima_fastrtps_identifier, publisher, ros_message, allocation);
}

}  // extern "C"
