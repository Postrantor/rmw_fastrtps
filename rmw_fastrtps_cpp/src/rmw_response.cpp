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
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 接收一个服务的响应 (Take a response from a service)
 *
 * @param[in] client 一个指向已创建的 rmw_client_t 结构体的指针 (A pointer to an already created
 * rmw_client_t structure)
 * @param[out] request_header 一个指向 rmw_service_info_t 结构体的指针，用于存储请求头信息 (A
 * pointer to an rmw_service_info_t structure to store the request header information)
 * @param[out] ros_response 一个指向存储响应数据的内存区域的指针 (A pointer to a memory area to
 * store the response data)
 * @param[out] taken 一个布尔值指针，如果成功接收到响应，则设置为 true (A boolean pointer, set to
 * true if a response is successfully taken)
 * @return 返回 rmw_ret_t 类型的结果代码 (Returns a result code of type rmw_ret_t)
 */
rmw_ret_t rmw_take_response(
    const rmw_client_t* client,
    rmw_service_info_t* request_header,
    void* ros_response,
    bool* taken) {
  // 调用共享实现中的 __rmw_take_response 函数，并传入 eprosima_fastrtps_identifier 作为参数
  // Call the __rmw_take_response function in the shared implementation and pass the
  // eprosima_fastrtps_identifier as a parameter
  return rmw_fastrtps_shared_cpp::__rmw_take_response(
      eprosima_fastrtps_identifier, client, request_header, ros_response, taken);
}

/**
 * @brief 发送一个服务的响应 (Send a response for a service)
 *
 * @param[in] service 一个指向已创建的 rmw_service_t 结构体的指针 (A pointer to an already created
 * rmw_service_t structure)
 * @param[in] request_header 一个指向 rmw_request_id_t 结构体的指针，包含请求头信息 (A pointer to an
 * rmw_request_id_t structure containing the request header information)
 * @param[in] ros_response 一个指向存储响应数据的内存区域的指针 (A pointer to a memory area storing
 * the response data)
 * @return 返回 rmw_ret_t 类型的结果代码 (Returns a result code of type rmw_ret_t)
 */
rmw_ret_t rmw_send_response(
    const rmw_service_t* service, rmw_request_id_t* request_header, void* ros_response) {
  // 调用共享实现中的 __rmw_send_response 函数，并传入 eprosima_fastrtps_identifier 作为参数
  // Call the __rmw_send_response function in the shared implementation and pass the
  // eprosima_fastrtps_identifier as a parameter
  return rmw_fastrtps_shared_cpp::__rmw_send_response(
      eprosima_fastrtps_identifier, service, request_header, ros_response);
}

}  // extern "C"
