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
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 创建一个守护条件对象 (Create a guard condition object)
 *
 * @param[in] context ROS2 上下文对象指针 (Pointer to the ROS2 context object)
 * @return rmw_guard_condition_t* 创建的守护条件对象指针，如果失败则返回 NULL (Pointer to the
 * created guard condition object, or NULL if failed)
 */
rmw_guard_condition_t* rmw_create_guard_condition(rmw_context_t* context) {
  // 检查传入的 context 参数是否为空，如果为空则返回 NULL
  // (Check if the input context parameter is NULL, and return NULL if it is)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, NULL);

  // 检查 context 的实现标识符是否与 eprosima_fastrtps_identifier 匹配，如果不匹配则返回 NULL
  // (Check if the implementation identifier of the context matches eprosima_fastrtps_identifier,
  // and return NULL if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      init context, context->implementation_identifier, eprosima_fastrtps_identifier,
      // TODO(wjwwood): replace this with RMW_RET_INCORRECT_RMW_IMPLEMENTATION when refactored
      return NULL);

  // 调用 rmw_fastrtps_shared_cpp 中的 __rmw_create_guard_condition 函数创建守护条件对象，并返回结果
  // (Call the __rmw_create_guard_condition function in rmw_fastrtps_shared_cpp to create a guard
  // condition object, and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_create_guard_condition(eprosima_fastrtps_identifier);
}

/**
 * @brief 销毁一个守护条件对象 (Destroy a guard condition object)
 *
 * @param[in] guard_condition 要销毁的守护条件对象指针 (Pointer to the guard condition object to be
 * destroyed)
 * @return rmw_ret_t 成功返回 RMW_RET_OK，失败返回相应的错误代码 (Return RMW_RET_OK if successful,
 * or the corresponding error code if failed)
 */
rmw_ret_t rmw_destroy_guard_condition(rmw_guard_condition_t* guard_condition) {
  // 调用 rmw_fastrtps_shared_cpp 中的 __rmw_destroy_guard_condition
  // 函数销毁守护条件对象，并返回结果 (Call the __rmw_destroy_guard_condition function in
  // rmw_fastrtps_shared_cpp to destroy the guard condition object, and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_destroy_guard_condition(guard_condition);
}

}  // extern "C"
