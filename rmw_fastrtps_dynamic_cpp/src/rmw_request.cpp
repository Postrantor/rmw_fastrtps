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

#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/types.h"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 发送请求 (Send a request)
 *
 * @param[in] client 指向一个有效的rmw_client_t结构体的指针，用于发送请求 (A pointer to a valid
 * rmw_client_t structure, used for sending the request)
 * @param[in] ros_request 指向ROS请求消息的指针 (A pointer to the ROS request message)
 * @param[out] sequence_id 请求的序列号，用于跟踪请求和响应之间的对应关系 (The sequence number of
 * the request, used to track the correspondence between requests and responses)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_send_request(
    const rmw_client_t* client, const void* ros_request, int64_t* sequence_id) {
  // 调用共享实现中的__rmw_send_request函数，并传递eprosima_fastrtps_identifier作为标识符
  // (Call the __rmw_send_request function in the shared implementation and pass
  // eprosima_fastrtps_identifier as the identifier)
  return rmw_fastrtps_shared_cpp::__rmw_send_request(
      eprosima_fastrtps_identifier, client, ros_request, sequence_id);
}

/**
 * @brief 接收请求 (Take a request)
 *
 * @param[in] service 指向一个有效的rmw_service_t结构体的指针，用于接收请求 (A pointer to a valid
 * rmw_service_t structure, used for receiving the request)
 * @param[out] request_header 包含服务请求信息的结构体 (A structure containing service request
 * information)
 * @param[out] ros_request 存储接收到的ROS请求消息的指针 (A pointer to store the received ROS
 * request message)
 * @param[out] taken 一个布尔值，表示是否成功接收到请求 (A boolean value indicating whether the
 * request was successfully received or not)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_take_request(
    const rmw_service_t* service,
    rmw_service_info_t* request_header,
    void* ros_request,
    bool* taken) {
  // 调用共享实现中的__rmw_take_request函数，并传递eprosima_fastrtps_identifier作为标识符
  // (Call the __rmw_take_request function in the shared implementation and pass
  // eprosima_fastrtps_identifier as the identifier)
  return rmw_fastrtps_shared_cpp::__rmw_take_request(
      eprosima_fastrtps_identifier, service, request_header, ros_request, taken);
}

}  // extern "C"
