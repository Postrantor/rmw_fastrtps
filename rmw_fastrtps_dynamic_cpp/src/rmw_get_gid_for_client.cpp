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
#include "rmw/rmw.h"
#include "rmw/types.h"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 获取客户端的全局唯一标识符 (Get the Global Unique Identifier (GID) for a client)
 *
 * @param[in] client 指向客户端的指针 (Pointer to the client)
 * @param[out] gid 用于存储 GID 的指针 (Pointer to store the GID)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_get_gid_for_client(const rmw_client_t* client, rmw_gid_t* gid) {
  // 调用 __rmw_get_gid_for_client 函数，传入 eprosima_fastrtps_identifier、client 和 gid 参数
  // Call the __rmw_get_gid_for_client function with eprosima_fastrtps_identifier, client, and gid
  // parameters
  return rmw_fastrtps_shared_cpp::__rmw_get_gid_for_client(
      eprosima_fastrtps_identifier, client, gid);
}

}  // extern "C"
