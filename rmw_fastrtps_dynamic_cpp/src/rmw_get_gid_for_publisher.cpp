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
 * @brief 获取发布者的全局唯一标识符 (Get the Global Unique Identifier for a publisher)
 *
 * @param[in] publisher 指向要获取其 GID 的发布者指针 (Pointer to the publisher whose GID is to be
 * obtained)
 * @param[out] gid 用于存储发布者 GID 的指针 (Pointer to store the GID of the publisher)
 * @return rmw_ret_t 返回操作结果，成功或失败 (Return the operation result, success or failure)
 */
rmw_ret_t rmw_get_gid_for_publisher(const rmw_publisher_t* publisher, rmw_gid_t* gid) {
  // 调用共享 FastRTPS 实现的函数来获取发布者的 GID，并传递 eprosima_fastrtps_identifier
  // 作为实现标识符 (Call the shared FastRTPS implementation function to get the GID of the
  // publisher, and pass eprosima_fastrtps_identifier as the implementation identifier)
  return rmw_fastrtps_shared_cpp::__rmw_get_gid_for_publisher(
      eprosima_fastrtps_identifier, publisher, gid);
}

}  // extern "C"
