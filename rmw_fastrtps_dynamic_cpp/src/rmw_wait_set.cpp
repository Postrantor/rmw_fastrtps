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
#include "rmw/rmw.h"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 创建一个等待集 (Create a wait set)
 *
 * @param[in] context ROS2 上下文 (ROS2 context)
 * @param[in] max_conditions 等待集中的最大条件数 (Maximum number of conditions in the wait set)
 * @return 返回创建的等待集指针 (Return a pointer to the created wait set)
 */
rmw_wait_set_t* rmw_create_wait_set(rmw_context_t* context, size_t max_conditions) {
  // 调用共享函数 __rmw_create_wait_set，传入 FastRTPS 标识符、上下文和最大条件数
  // Call the shared function __rmw_create_wait_set with the FastRTPS identifier, context, and
  // maximum number of conditions
  return rmw_fastrtps_shared_cpp::__rmw_create_wait_set(
      eprosima_fastrtps_identifier, context, max_conditions);
}

/**
 * @brief 销毁一个等待集 (Destroy a wait set)
 *
 * @param[in] wait_set 要销毁的等待集指针 (Pointer to the wait set to be destroyed)
 * @return 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_destroy_wait_set(rmw_wait_set_t* wait_set) {
  // 调用共享函数 __rmw_destroy_wait_set，传入 FastRTPS 标识符和等待集指针
  // Call the shared function __rmw_destroy_wait_set with the FastRTPS identifier and wait set
  // pointer
  return rmw_fastrtps_shared_cpp::__rmw_destroy_wait_set(eprosima_fastrtps_identifier, wait_set);
}

}  // extern "C"
