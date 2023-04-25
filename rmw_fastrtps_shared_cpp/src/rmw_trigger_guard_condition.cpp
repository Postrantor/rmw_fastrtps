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

#include <cassert>

#include "fastdds/dds/core/condition/GuardCondition.hpp"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 触发保护条件 (Trigger a guard condition)
 *
 * @param[in] identifier 实现标识符，用于检查保护条件句柄是否与此实现匹配 (Implementation
 * identifier, used to check if the guard condition handle matches this implementation)
 * @param[in] guard_condition_handle 保护条件句柄，用于触发保护条件 (Guard condition handle, used to
 * trigger the guard condition)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_trigger_guard_condition(
    const char *identifier, const rmw_guard_condition_t *guard_condition_handle) {
  // 断言：保护条件句柄不为空 (Assert: Guard condition handle is not null)
  assert(guard_condition_handle);

  // 检查保护条件句柄的实现标识符是否与传入的标识符匹配
  // (Check if the implementation identifier of the guard condition handle matches the passed
  // identifier)
  if (guard_condition_handle->implementation_identifier != identifier) {
    // 设置错误消息：保护条件句柄不是来自此实现
    // (Set error message: Guard condition handle not from this implementation)
    RMW_SET_ERROR_MSG("guard condition handle not from this implementation");
    // 返回错误状态 (Return error status)
    return RMW_RET_ERROR;
  }

  // 将保护条件句柄的数据转换为 eprosima::fastdds::dds::GuardCondition 类型
  // (Cast the data of the guard condition handle to eprosima::fastdds::dds::GuardCondition type)
  auto guard_condition =
      static_cast<eprosima::fastdds::dds::GuardCondition *>(guard_condition_handle->data);
  // 设置触发值为 true，触发保护条件 (Set the trigger value to true, triggering the guard condition)
  guard_condition->set_trigger_value(true);
  // 返回操作成功状态 (Return operation success status)
  return RMW_RET_OK;
}

}  // namespace rmw_fastrtps_shared_cpp
