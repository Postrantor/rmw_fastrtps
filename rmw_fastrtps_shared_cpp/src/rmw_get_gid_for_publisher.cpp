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
#include "rmw/types.h"
#include "rmw_fastrtps_shared_cpp/custom_publisher_info.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 获取发布者的全局唯一标识符 (Get the Global Unique Identifier (GID) for a publisher)
 *
 * @param[in] identifier 指向当前 rmw 实现的字符串指针 (A string pointer pointing to the current rmw
 * implementation)
 * @param[in] publisher 指向要获取 GID 的发布者的指针 (A pointer to the publisher whose GID is to be
 * obtained)
 * @param[out] gid 用于存储获取到的 GID 的指针 (A pointer to store the obtained GID)
 *
 * @return 返回一个 rmw_ret_t 类型的结果，表示函数执行成功或失败 (Returns an rmw_ret_t type result,
 * indicating success or failure of the function execution)
 */
rmw_ret_t __rmw_get_gid_for_publisher(
    const char *identifier, const rmw_publisher_t *publisher, rmw_gid_t *gid) {
  // 检查 publisher 参数是否为空，如果为空则返回 RMW_RET_INVALID_ARGUMENT 错误 (Check if the
  // publisher argument is NULL, and return RMW_RET_INVALID_ARGUMENT error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);

  // 检查 publisher 的实现标识符与传入的 identifier 是否匹配，如果不匹配则返回
  // RMW_RET_INCORRECT_RMW_IMPLEMENTATION 错误 (Check if the publisher's implementation identifier
  // matches the passed identifier, and return RMW_RET_INCORRECT_RMW_IMPLEMENTATION error if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      publisher, publisher->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查 gid 参数是否为空，如果为空则返回 RMW_RET_INVALID_ARGUMENT 错误 (Check if the gid argument
  // is NULL, and return RMW_RET_INVALID_ARGUMENT error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(gid, RMW_RET_INVALID_ARGUMENT);

  // 将 publisher->data 转换为 CustomPublisherInfo 类型的指针，并将其存储在 info 变量中 (Cast
  // publisher->data to a pointer of type CustomPublisherInfo and store it in the info variable)
  auto info = static_cast<const CustomPublisherInfo *>(publisher->data);

  // 将 info 中的 publisher_gid 赋值给 gid 指针所指向的变量 (Assign the publisher_gid from info to
  // the variable pointed to by the gid pointer)
  *gid = info->publisher_gid;

  // 返回 RMW_RET_OK，表示函数执行成功 (Return RMW_RET_OK, indicating successful function execution)
  return RMW_RET_OK;
}

}  // namespace rmw_fastrtps_shared_cpp
