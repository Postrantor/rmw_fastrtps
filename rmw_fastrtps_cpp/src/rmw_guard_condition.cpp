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
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 创建一个守护条件 (Create a guard condition)
 *
 * @param[in] context rmw上下文指针 (Pointer to the rmw context)
 * @return 返回创建的守护条件指针，如果失败则返回NULL (Return pointer to the created guard
 * condition, or NULL if failed)
 */
rmw_guard_condition_t* rmw_create_guard_condition(rmw_context_t* context) {
  // 检查传入的context参数是否为空，如果为空则返回NULL
  // (Check if the input 'context' parameter is NULL, and return NULL if it is)
  RCUTILS_CHECK_ARGUMENT_FOR_NULL(context, NULL);

  // 检查类型标识符是否匹配，如果不匹配则返回NULL
  // (Check if type identifiers match, and return NULL if they don't)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      init context, context->implementation_identifier, eprosima_fastrtps_identifier,
      // TODO(wjwwood): replace this with RMW_RET_INCORRECT_RMW_IMPLEMENTATION when refactored
      return NULL);

  // 调用__rmw_create_guard_condition函数并返回结果
  // (Call the __rmw_create_guard_condition function and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_create_guard_condition(eprosima_fastrtps_identifier);
}

/**
 * @brief 销毁守护条件 (Destroy a guard condition)
 *
 * @param[in] guard_condition 守护条件指针 (Pointer to the guard condition)
 * @return 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_destroy_guard_condition(rmw_guard_condition_t* guard_condition) {
  return rmw_fastrtps_shared_cpp::__rmw_destroy_guard_condition(guard_condition);
}

}  // extern "C"
