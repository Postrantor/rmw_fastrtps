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
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 获取服务名称和类型 (Get service names and types)
 *
 * @param[in] node 指向一个有效的rmw_node_t结构体的指针 (Pointer to a valid rmw_node_t structure)
 * @param[in] allocator 用于分配内存的rcutils_allocator_t结构体指针 (Pointer to an
 * rcutils_allocator_t structure for memory allocation)
 * @param[out] service_names_and_types 存储获取到的服务名称和类型的rmw_names_and_types_t结构体指针
 * (Pointer to an rmw_names_and_types_t structure to store the retrieved service names and types)
 * @return rmw_ret_t 返回操作结果，成功返回RMW_RET_OK，失败返回相应的错误代码 (Return operation
 * result, success returns RMW_RET_OK, failure returns corresponding error code)
 */
rmw_ret_t rmw_get_service_names_and_types(
    const rmw_node_t* node,
    rcutils_allocator_t* allocator,
    rmw_names_and_types_t* service_names_and_types) {
  // 调用rmw_fastrtps_shared_cpp中的__rmw_get_service_names_and_types函数，并传入eprosima_fastrtps_identifier作为参数
  // (Call the __rmw_get_service_names_and_types function in rmw_fastrtps_shared_cpp and pass
  // eprosima_fastrtps_identifier as a parameter)
  return rmw_fastrtps_shared_cpp::__rmw_get_service_names_and_types(
      eprosima_fastrtps_identifier, node, allocator, service_names_and_types);
}

}  // extern "C"
