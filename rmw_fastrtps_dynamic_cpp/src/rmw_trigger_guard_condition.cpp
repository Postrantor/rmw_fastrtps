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
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 触发保护条件 (Trigger a guard condition)
 *
 * @param[in] guard_condition_handle 保护条件句柄 (Guard condition handle)
 * @return rmw_ret_t 返回触发结果 (Return the trigger result)
 */
rmw_ret_t rmw_trigger_guard_condition(const rmw_guard_condition_t* guard_condition_handle) {
  // 调用 __rmw_trigger_guard_condition 函数，传入 eprosima_fastrtps_identifier 和
  // guard_condition_handle 参数 Call the __rmw_trigger_guard_condition function with
  // eprosima_fastrtps_identifier and guard_condition_handle parameters
  return rmw_fastrtps_shared_cpp::__rmw_trigger_guard_condition(
      eprosima_fastrtps_identifier, guard_condition_handle);
}

}  // extern "C"
