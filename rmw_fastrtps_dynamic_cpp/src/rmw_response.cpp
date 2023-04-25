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
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 从客户端接收服务响应 (Take a response from the client)
 *
 * @param[in] client 指向一个有效的rmw_client_t结构体的指针 (Pointer to a valid rmw_client_t
 * structure)
 * @param[out] request_header 请求头信息，包含请求ID和时间戳 (Request header information, including
 * request ID and timestamp)
 * @param[out] ros_response 存储接收到的响应数据的缓冲区 (Buffer to store the received response
 * data)
 * @param[out] taken 是否成功接收到响应的标志 (Flag indicating whether a response was successfully
 * taken)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
rmw_ret_t rmw_take_response(
    const rmw_client_t* client,
    rmw_service_info_t* request_header,
    void* ros_response,
    bool* taken) {
  // 调用共享实现中的__rmw_take_response函数，并传入FastRTPS的标识符
  // Call the __rmw_take_response function in the shared implementation and pass the FastRTPS
  // identifier
  return rmw_fastrtps_shared_cpp::__rmw_take_response(
      eprosima_fastrtps_identifier, client, request_header, ros_response, taken);
}

/**
 * @brief 发送服务响应给客户端 (Send a service response to the client)
 *
 * @param[in] service 指向一个有效的rmw_service_t结构体的指针 (Pointer to a valid rmw_service_t
 * structure)
 * @param[in] request_header 包含请求ID的请求头信息 (Request header information containing the
 * request ID)
 * @param[in] ros_response 要发送的响应数据 (Response data to be sent)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
rmw_ret_t rmw_send_response(
    const rmw_service_t* service, rmw_request_id_t* request_header, void* ros_response) {
  // 调用共享实现中的__rmw_send_response函数，并传入FastRTPS的标识符
  // Call the __rmw_send_response function in the shared implementation and pass the FastRTPS
  // identifier
  return rmw_fastrtps_shared_cpp::__rmw_send_response(
      eprosima_fastrtps_identifier, service, request_header, ros_response);
}

}  // extern "C"
