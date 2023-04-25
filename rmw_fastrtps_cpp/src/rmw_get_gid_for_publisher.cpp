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
 * @brief 获取发布者的全局唯一ID（Get the Global Unique IDentifier (GID) for a publisher）
 *
 * @param[in] publisher 输入参数，表示一个发布者对象（Input parameter, represents a publisher
 * object）
 * @param[out] gid 输出参数，用于存储发布者的GID（Output parameter, used to store the GID of the
 * publisher）
 * @return 返回rmw_ret_t类型的结果，表示操作是否成功（Returns a result of type rmw_ret_t, indicating
 * whether the operation was successful）
 */
rmw_ret_t rmw_get_gid_for_publisher(const rmw_publisher_t* publisher, rmw_gid_t* gid) {
  // 调用rmw_fastrtps_shared_cpp命名空间中的__rmw_get_gid_for_publisher函数，传入eprosima_fastrtps_identifier、publisher和gid参数
  // Call the __rmw_get_gid_for_publisher function in the rmw_fastrtps_shared_cpp namespace, passing
  // in the eprosima_fastrtps_identifier, publisher, and gid parameters
  return rmw_fastrtps_shared_cpp::__rmw_get_gid_for_publisher(
      eprosima_fastrtps_identifier, publisher, gid);
}

}  // extern "C"
