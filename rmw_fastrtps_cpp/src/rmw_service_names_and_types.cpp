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
#include "rmw/convert_rcutils_ret_to_rmw_ret.h"
#include "rmw/error_handling.h"
#include "rmw/get_service_names_and_types.h"
#include "rmw/names_and_types.h"
#include "rmw/rmw.h"
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 获取服务名称和类型 (Get service names and types)
 *
 * @param[in] node 指向 ROS2 节点的指针 (Pointer to the ROS2 node)
 * @param[in] allocator 分配器，用于分配内存 (Allocator for memory allocation)
 * @param[out] service_names_and_types 存储获取到的服务名称和类型的结构体 (Structure to store the
 * retrieved service names and types)
 *
 * @return 返回 rmw_ret_t 类型的结果，表示操作是否成功 (Returns an rmw_ret_t type result indicating
 * whether the operation was successful or not)
 */
rmw_ret_t rmw_get_service_names_and_types(
    const rmw_node_t* node,
    rcutils_allocator_t* allocator,
    rmw_names_and_types_t* service_names_and_types) {
  // 调用 rmw_fastrtps_shared_cpp 中的 __rmw_get_service_names_and_types 函数实现，
  // 并传入 eprosima_fastrtps_identifier 作为 FastRTPS 的标识符
  // (Call the __rmw_get_service_names_and_types function in rmw_fastrtps_shared_cpp,
  // passing eprosima_fastrtps_identifier as the identifier for FastRTPS)
  return rmw_fastrtps_shared_cpp::__rmw_get_service_names_and_types(
      eprosima_fastrtps_identifier, node, allocator, service_names_and_types);
}

}  // extern "C"
