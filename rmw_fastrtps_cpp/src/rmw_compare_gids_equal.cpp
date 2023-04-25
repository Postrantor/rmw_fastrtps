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
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 比较两个 rmw_gid_t 结构体是否相等 (Compare two rmw_gid_t structures for equality)
 *
 * @param[in] gid1 第一个 rmw_gid_t 结构体指针 (Pointer to the first rmw_gid_t structure)
 * @param[in] gid2 第二个 rmw_gid_t 结构体指针 (Pointer to the second rmw_gid_t structure)
 * @param[out] result 存储比较结果的布尔值指针，如果相等则为 true，否则为 false (Pointer to a
 * boolean value that stores the comparison result, true if equal, false otherwise)
 * @return rmw_ret_t 返回比较操作状态 (Return the status of the comparison operation)
 */
rmw_ret_t rmw_compare_gids_equal(const rmw_gid_t* gid1, const rmw_gid_t* gid2, bool* result) {
  // 调用 __rmw_compare_gids_equal 函数，传递 eprosima_fastrtps_identifier、gid1、gid2 和 result
  // 参数 (Call the __rmw_compare_gids_equal function, passing the eprosima_fastrtps_identifier,
  // gid1, gid2, and result parameters)
  return rmw_fastrtps_shared_cpp::__rmw_compare_gids_equal(
      eprosima_fastrtps_identifier, gid1, gid2, result);
}

}  // extern "C"
