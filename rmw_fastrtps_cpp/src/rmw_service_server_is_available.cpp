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
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw/types.h"
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 检查服务端是否可用 (Check if the service server is available)
 *
 * @param[in] node ROS2节点指针 (Pointer to the ROS2 node)
 * @param[in] client 服务客户端指针 (Pointer to the service client)
 * @param[out] is_available 用于存储服务端是否可用的布尔值指针 (Pointer to a boolean value to store
 * whether the service server is available)
 * @return 返回rmw_ret_t类型的结果，表示成功或失败 (Return an rmw_ret_t type result, indicating
 * success or failure)
 */
rmw_ret_t rmw_service_server_is_available(
    const rmw_node_t* node, const rmw_client_t* client, bool* is_available) {
  // 调用FastRTPS共享CPP实现的__rmw_service_server_is_available函数，并传入eprosima_fastrtps_identifier、node、client和is_available参数
  // (Call the __rmw_service_server_is_available function of FastRTPS shared CPP implementation, and
  // pass in eprosima_fastrtps_identifier, node, client, and is_available parameters)
  return rmw_fastrtps_shared_cpp::__rmw_service_server_is_available(
      eprosima_fastrtps_identifier, node, client, is_available);
}

}  // extern "C"
