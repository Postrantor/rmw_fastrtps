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

#include <map>
#include <set>
#include <string>

#include "rcutils/allocator.h"
#include "rcutils/error_handling.h"
#include "rcutils/logging_macros.h"
#include "rcutils/strdup.h"
#include "rcutils/types.h"
#include "rmw/allocators.h"
#include "rmw/convert_rcutils_ret_to_rmw_ret.h"
#include "rmw/error_handling.h"
#include "rmw/get_topic_names_and_types.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/names_and_types.h"
#include "rmw/rmw.h"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 获取 ROS2 主题名称和类型 (Get topic names and types in ROS2)
 *
 * @param[in] node 指向 ROS2 节点的指针 (Pointer to the ROS2 node)
 * @param[in] allocator 用于分配内存的分配器 (Allocator for memory allocation)
 * @param[in] no_demangle 是否对主题名称进行解扰处理 (Whether to demangle the topic names or not)
 * @param[out] topic_names_and_types 存储获取到的主题名称和类型的结构体 (Structure to store the
 * obtained topic names and types)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_get_topic_names_and_types(
    const rmw_node_t* node,
    rcutils_allocator_t* allocator,
    bool no_demangle,
    rmw_names_and_types_t* topic_names_and_types) {
  // 调用 rmw_fastrtps_shared_cpp 中的 __rmw_get_topic_names_and_types 函数，并传入
  // eprosima_fastrtps_identifier 作为 FastRTPS 的实现标识符 Call the
  // __rmw_get_topic_names_and_types function in rmw_fastrtps_shared_cpp, and pass
  // eprosima_fastrtps_identifier as the implementation identifier for FastRTPS
  return rmw_fastrtps_shared_cpp::__rmw_get_topic_names_and_types(
      eprosima_fastrtps_identifier, node, allocator, no_demangle, topic_names_and_types);
}

}  // extern "C"
