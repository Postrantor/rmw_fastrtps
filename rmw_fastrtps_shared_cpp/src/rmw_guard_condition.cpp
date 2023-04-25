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

#include "fastdds/dds/core/condition/GuardCondition.hpp"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 创建一个守护条件对象 (Create a guard condition object)
 *
 * @param[in] identifier 守护条件的标识符 (Identifier for the guard condition)
 * @return 返回创建的守护条件对象指针，如果出错则返回 nullptr (Returns a pointer to the created
 * guard condition object, or nullptr if an error occurs)
 */
rmw_guard_condition_t *__rmw_create_guard_condition(const char *identifier) {
  // 检查是否可以返回 nullptr 错误 (Check if it can return with a nullptr error)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(nullptr);

  // 分配一个新的 rmw_guard_condition_t 对象 (Allocate a new rmw_guard_condition_t object)
  rmw_guard_condition_t *guard_condition_handle = new rmw_guard_condition_t;
  // 设置守护条件对象的实现标识符 (Set the implementation identifier for the guard condition object)
  guard_condition_handle->implementation_identifier = identifier;
  // 为守护条件对象分配一个新的 eprosima::fastdds::dds::GuardCondition 对象 (Allocate a new
  // eprosima::fastdds::dds::GuardCondition object for the guard condition object)
  guard_condition_handle->data = new eprosima::fastdds::dds::GuardCondition();
  // 返回创建的守护条件对象指针 (Return the pointer to the created guard condition object)
  return guard_condition_handle;
}

/**
 * @brief 销毁一个守护条件对象 (Destroy a guard condition object)
 *
 * @param[in] guard_condition 要销毁的守护条件对象指针 (Pointer to the guard condition object to be
 * destroyed)
 * @return 返回 RMW_RET_OK 如果成功销毁，否则返回 RMW_RET_ERROR (Returns RMW_RET_OK if successfully
 * destroyed, otherwise returns RMW_RET_ERROR)
 */
rmw_ret_t __rmw_destroy_guard_condition(rmw_guard_condition_t *guard_condition) {
  // 初始化返回值为错误 (Initialize the return value as an error)
  rmw_ret_t ret = RMW_RET_ERROR;

  // 检查守护条件对象是否存在 (Check if the guard condition object exists)
  if (guard_condition) {
    // 删除 eprosima::fastdds::dds::GuardCondition 对象 (Delete the
    // eprosima::fastdds::dds::GuardCondition object)
    delete static_cast<eprosima::fastdds::dds::GuardCondition *>(guard_condition->data);
    // 删除守护条件对象 (Delete the guard condition object)
    delete guard_condition;
    // 设置返回值为成功 (Set the return value to success)
    ret = RMW_RET_OK;
  }

  // 在完成时检查是否可以返回 RMW_RET_ERROR 错误 (Check if it can return with an RMW_RET_ERROR error
  // upon completion)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_ERROR);
  // 返回结果 (Return the result)
  return ret;
}

}  // namespace rmw_fastrtps_shared_cpp
