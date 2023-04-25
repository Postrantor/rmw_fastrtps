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

#include "fastdds/rtps/common/Guid.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw/types.h"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 比较两个全局唯一标识符（GID）是否相等 (Compare two Global Unique Identifiers (GIDs) for
 * equality)
 *
 * @param[in] identifier 用于检查实现的兼容性的字符串 (A string used to check the compatibility of
 * the implementation)
 * @param[in] gid1 第一个要比较的 GID (The first GID to be compared)
 * @param[in] gid2 第二个要比较的 GID (The second GID to be compared)
 * @param[out] result 存储比较结果的布尔值指针，如果 GID 相等则为 true，否则为 false (A pointer to a
 * boolean value that stores the comparison result; true if the GIDs are equal, false otherwise)
 * @return RMW_RET_OK 如果成功，或者适当的错误代码 (RMW_RET_OK if successful, or an appropriate
 * error code)
 */
rmw_ret_t __rmw_compare_gids_equal(
    const char* identifier, const rmw_gid_t* gid1, const rmw_gid_t* gid2, bool* result) {
  // 检查 gid1 是否为空 (Check if gid1 is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(gid1, RMW_RET_INVALID_ARGUMENT);

  // 检查 gid1 的实现标识符与提供的标识符是否匹配 (Check if the implementation identifier of gid1
  // matches the provided identifier)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      gid1, gid1->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查 gid2 是否为空 (Check if gid2 is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(gid2, RMW_RET_INVALID_ARGUMENT);

  // 检查 gid2 的实现标识符与提供的标识符是否匹配 (Check if the implementation identifier of gid2
  // matches the provided identifier)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      gid2, gid2->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查结果指针是否为空 (Check if the result pointer is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(result, RMW_RET_INVALID_ARGUMENT);

  // 使用 memcmp 函数比较两个 GID 的数据，如果相等则返回 0，将结果存储在布尔值中 (Compare the data
  // of the two GIDs using the memcmp function; returns 0 if equal, store the result in a boolean
  // value)
  *result = memcmp(gid1->data, gid2->data, sizeof(eprosima::fastrtps::rtps::GUID_t)) == 0;

  // 返回成功代码 (Return the success code)
  return RMW_RET_OK;
}

}  // namespace rmw_fastrtps_shared_cpp
