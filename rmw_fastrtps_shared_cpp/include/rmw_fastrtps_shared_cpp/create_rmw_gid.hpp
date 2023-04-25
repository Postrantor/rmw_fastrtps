// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#ifndef RMW_FASTRTPS_SHARED_CPP__CREATE_RMW_GID_HPP_
#define RMW_FASTRTPS_SHARED_CPP__CREATE_RMW_GID_HPP_

#include "fastdds/rtps/common/Guid.h"
#include "rmw/types.h"
#include "rmw_fastrtps_shared_cpp/visibility_control.h"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 创建 ROS2 全局唯一标识符（GID） / Create a ROS2 Global Unique Identifier (GID)
 *
 * @param identifier 用于指定 RMW 实现的字符串 / A string specifying the RMW implementation
 * @param guid Fast RTPS 全局唯一标识符 / The Fast RTPS Global Unique Identifier
 * @return rmw_gid_t 类型的 ROS2 GID 对象 / An rmw_gid_t type ROS2 GID object
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_gid_t create_rmw_gid(const char* identifier, const eprosima::fastrtps::rtps::GUID_t& guid) {
  // 创建一个 rmw_gid_t 类型的对象 / Create an rmw_gid_t type object
  rmw_gid_t gid;

  // 将传入的 identifier 字符串复制到 gid 的成员变量中 / Copy the incoming identifier string to the
  // member variable of gid
  memcpy(gid.data, identifier, RMW_GID_STORAGE_SIZE);

  // 将传入的 guid 复制到 gid 的成员变量中 / Copy the incoming guid to the member variable of gid
  memcpy(gid.data + RMW_GID_STORAGE_SIZE, &guid, sizeof(guid));

  // 返回创建的 ROS2 GID 对象 / Return the created ROS2 GID object
  return gid;
}

}  // namespace rmw_fastrtps_shared_cpp

#endif  // RMW_FASTRTPS_SHARED_CPP__CREATE_RMW_GID_HPP_
