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

#include "rmw_fastrtps_shared_cpp/create_rmw_gid.hpp"

#include "fastdds/rtps/common/Guid.h"
#include "rmw/types.h"
#include "rmw_fastrtps_shared_cpp/guid_utils.hpp"

/**
 * @brief 创建一个 rmw_gid_t 类型的对象，用于存储 Cyclone DDS 项目中的 GUID (Globally Unique
 * IDentifier)。 Create an rmw_gid_t object to store the GUID (Globally Unique IDentifier) in the
 * Cyclone DDS project.
 *
 * @param[in] identifier 指向一个表示实现标识符的字符串的指针。A pointer to a string representing
 * the implementation identifier.
 * @param[in] guid eprosima::fastrtps::rtps::GUID_t 类型的引用，表示要转换为 rmw_gid_t 的 GUID。A
 * reference to an eprosima::fastrtps::rtps::GUID_t, representing the GUID to be converted to
 * rmw_gid_t.
 * @return 返回一个包含转换后的 GUID 的 rmw_gid_t 对象。Returns an rmw_gid_t object containing the
 * converted GUID.
 */
rmw_gid_t rmw_fastrtps_shared_cpp::create_rmw_gid(
    const char* identifier, const eprosima::fastrtps::rtps::GUID_t& guid) {
  // 初始化一个空的 rmw_gid_t 结构体。Initialize an empty rmw_gid_t struct.
  rmw_gid_t rmw_gid = {};

  // 将传入的实现标识符赋值给 rmw_gid 的 implementation_identifier 成员。Assign the incoming
  // implementation identifier to the implementation_identifier member of rmw_gid.
  rmw_gid.implementation_identifier = identifier;

  // 静态断言，检查 eprosima::fastrtps::rtps::GUID_t 是否能够存储在 RMW_GID_STORAGE_SIZE 中。Static
  // assertion to check if eprosima::fastrtps::rtps::GUID_t can be stored in RMW_GID_STORAGE_SIZE.
  static_assert(
      sizeof(eprosima::fastrtps::rtps::GUID_t) <= RMW_GID_STORAGE_SIZE,
      "RMW_GID_STORAGE_SIZE insufficient to store the fastrtps GUID_t.");

  // 将传入的 guid 转换为字节数组，并将结果存储在 rmw_gid 的 data 成员中。Convert the incoming guid
  // to a byte array and store the result in the data member of rmw_gid.
  rmw_fastrtps_shared_cpp::copy_from_fastrtps_guid_to_byte_array(guid, rmw_gid.data);

  // 返回包含转换后的 GUID 的 rmw_gid_t 对象。Return the rmw_gid_t object containing the converted
  // GUID.
  return rmw_gid;
}
