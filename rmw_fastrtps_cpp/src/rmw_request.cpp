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
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 发送请求 (Send a request)
 *
 * @param[in] client 客户端指针 (Pointer to the client)
 * @param[in] ros_request ROS 请求数据 (ROS request data)
 * @param[out] sequence_id 请求序列号 (Request sequence number)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_send_request(
    const rmw_client_t* client, const void* ros_request, int64_t* sequence_id) {
  // 调用共享的 Fast RTPS 实现发送请求 (Call shared Fast RTPS implementation to send request)
  return rmw_fastrtps_shared_cpp::__rmw_send_request(
      eprosima_fastrtps_identifier, client, ros_request, sequence_id);
}

/**
 * @brief 接收请求 (Take a request)
 *
 * @param[in] service 服务指针 (Pointer to the service)
 * @param[out] request_header 请求头信息 (Request header information)
 * @param[out] ros_request ROS 请求数据 (ROS request data)
 * @param[out] taken 是否成功接收请求 (Whether the request was successfully taken)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_take_request(
    const rmw_service_t* service,
    rmw_service_info_t* request_header,
    void* ros_request,
    bool* taken) {
  // 调用共享的 Fast RTPS 实现接收请求 (Call shared Fast RTPS implementation to take request)
  return rmw_fastrtps_shared_cpp::__rmw_take_request(
      eprosima_fastrtps_identifier, service, request_header, ros_request, taken);
}

}  // extern "C"
