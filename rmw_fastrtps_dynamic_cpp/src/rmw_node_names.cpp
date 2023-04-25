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

#include <string>

#include "rcutils/allocator.h"
#include "rcutils/strdup.h"
#include "rcutils/types.h"
#include "rmw/allocators.h"
#include "rmw/convert_rcutils_ret_to_rmw_ret.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/sanity_checks.h"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 获取节点名称和命名空间 (Get node names and namespaces)
 *
 * @param[in] node 指向当前节点的指针 (Pointer to the current node)
 * @param[out] node_names 存储节点名称的字符串数组 (String array to store node names)
 * @param[out] node_namespaces 存储节点命名空间的字符串数组 (String array to store node namespaces)
 * @return 返回 rmw_ret_t 类型的结果，表示操作成功或失败 (Return an rmw_ret_t type result,
 * indicating success or failure of the operation)
 */
rmw_ret_t rmw_get_node_names(
    const rmw_node_t* node,
    rcutils_string_array_t* node_names,
    rcutils_string_array_t* node_namespaces) {
  // 调用共享函数 __rmw_get_node_names 并传递 eprosima_fastrtps_identifier 作为参数
  // Call the shared function __rmw_get_node_names and pass eprosima_fastrtps_identifier as a
  // parameter
  return rmw_fastrtps_shared_cpp::__rmw_get_node_names(
      eprosima_fastrtps_identifier, node, node_names, node_namespaces);
}

/**
 * @brief 获取节点名称、命名空间和 enclave (Get node names, namespaces, and enclaves)
 *
 * @param[in] node 指向当前节点的指针 (Pointer to the current node)
 * @param[out] node_names 存储节点名称的字符串数组 (String array to store node names)
 * @param[out] node_namespaces 存储节点命名空间的字符串数组 (String array to store node namespaces)
 * @param[out] enclaves 存储 enclave 的字符串数组 (String array to store enclaves)
 * @return 返回 rmw_ret_t 类型的结果，表示操作成功或失败 (Return an rmw_ret_t type result,
 * indicating success or failure of the operation)
 */
rmw_ret_t rmw_get_node_names_with_enclaves(
    const rmw_node_t* node,
    rcutils_string_array_t* node_names,
    rcutils_string_array_t* node_namespaces,
    rcutils_string_array_t* enclaves) {
  // 调用共享函数 __rmw_get_node_names_with_enclaves 并传递 eprosima_fastrtps_identifier 作为参数
  // Call the shared function __rmw_get_node_names_with_enclaves and pass
  // eprosima_fastrtps_identifier as a parameter
  return rmw_fastrtps_shared_cpp::__rmw_get_node_names_with_enclaves(
      eprosima_fastrtps_identifier, node, node_names, node_namespaces, enclaves);
}

}  // extern "C"
