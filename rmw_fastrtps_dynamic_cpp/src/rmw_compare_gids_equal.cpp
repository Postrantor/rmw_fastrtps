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
#include "rmw/types.h"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 比较两个 rmw_gid_t 是否相等 (Compare two rmw_gid_t for equality)
 *
 * @param[in] gid1 第一个 rmw_gid_t 结构体指针 (Pointer to the first rmw_gid_t structure)
 * @param[in] gid2 第二个 rmw_gid_t 结构体指针 (Pointer to the second rmw_gid_t structure)
 * @param[out] result 布尔值指针，用于存储比较结果 (Pointer to a boolean value to store the
 * comparison result)
 *
 * @return rmw_ret_t 返回比较操作的状态 (Return the status of the comparison operation)
 */
rmw_ret_t rmw_compare_gids_equal(const rmw_gid_t* gid1, const rmw_gid_t* gid2, bool* result) {
  // 调用 rmw_fastrtps_shared_cpp 中的 __rmw_compare_gids_equal 函数进行实际的比较操作
  // (Call the __rmw_compare_gids_equal function in rmw_fastrtps_shared_cpp for the actual
  // comparison operation)
  return rmw_fastrtps_shared_cpp::__rmw_compare_gids_equal(
      eprosima_fastrtps_identifier,  // 使用 eprosima_fastrtps_identifier 作为 FastRTPS 的标识符
                                     // (Use eprosima_fastrtps_identifier as the identifier for
                                     // FastRTPS)
      gid1,  // 传递第一个 rmw_gid_t 结构体指针 (Pass the pointer to the first rmw_gid_t structure)
      gid2,  // 传递第二个 rmw_gid_t 结构体指针 (Pass the pointer to the second rmw_gid_t structure)
      result  // 传递布尔值指针以存储比较结果 (Pass the pointer to a boolean value to store the
              // comparison result)
  );
}

}  // extern "C"
