// Copyright 2022 Open Source Robotics Foundation, Inc.
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
#include "rmw_fastrtps_shared_cpp/custom_client_info.hpp"
#include "rmw_fastrtps_shared_cpp/guid_utils.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 获取客户端的全局唯一标识符 (Get the Global Unique Identifier (GID) for a client)
 *
 * @param[in] identifier 指向实现标识符的指针 (Pointer to the implementation identifier)
 * @param[in] client 指向要获取其 GID 的 rmw_client_t 结构体的指针 (Pointer to the rmw_client_t
 * structure whose GID is to be obtained)
 * @param[out] gid 用于存储 GID 的 rmw_gid_t 结构体的指针 (Pointer to the rmw_gid_t structure where
 * the GID will be stored)
 * @return 返回操作结果，成功返回 RMW_RET_OK，否则返回相应的错误代码 (Returns the operation result,
 * RMW_RET_OK if successful, otherwise returns the corresponding error code)
 */
rmw_ret_t __rmw_get_gid_for_client(
    const char *identifier, const rmw_client_t *client, rmw_gid_t *gid) {
  // 检查 client 参数是否为空，如果为空则返回无效参数错误 (Check if the client parameter is NULL,
  // return an invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);

  // 检查 client 的实现标识符与传入的 identifier 是否匹配，如果不匹配则返回不正确的 RMW 实现错误
  // (Check if the client's implementation identifier matches the passed identifier, return an
  // incorrect RMW implementation error if they don't match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      client, client->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查 gid 参数是否为空，如果为空则返回无效参数错误 (Check if the gid parameter is NULL, return
  // an invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(gid, RMW_RET_INVALID_ARGUMENT);

  // 获取 client 的数据指针，并将其转换为 CustomClientInfo 类型 (Get the data pointer of the client
  // and cast it to the CustomClientInfo type)
  const auto *info = static_cast<const CustomClientInfo *>(client->data);

  // 将 FastRTPS GUID 复制到 gid 结构体的 data 成员中 (Copy the FastRTPS GUID to the data member of
  // the gid structure)
  copy_from_fastrtps_guid_to_byte_array(info->writer_guid_, gid->data);

  // 设置 gid 结构体的实现标识符为传入的 identifier (Set the implementation identifier of the gid
  // structure to the passed identifier)
  gid->implementation_identifier = identifier;

  // 返回操作成功 (Return operation successful)
  return RMW_RET_OK;
}

}  // namespace rmw_fastrtps_shared_cpp
