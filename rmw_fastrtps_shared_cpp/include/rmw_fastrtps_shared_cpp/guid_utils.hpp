// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#ifndef RMW_FASTRTPS_SHARED_CPP__GUID_UTILS_HPP_
#define RMW_FASTRTPS_SHARED_CPP__GUID_UTILS_HPP_

#include <cassert>
#include <cstddef>
#include <cstring>
#include <type_traits>

#include "fastdds/rtps/common/Guid.h"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 将字节数组复制到 FastRTPS GUID 中 (Copy byte array to FastRTPS GUID)
 *
 * @tparam ByteT 字节类型，应为 int8_t 或 uint8_t (Byte type, should be either int8_t or uint8_t)
 * @param[in] guid_byte_array 指向 GUID 字节数组的指针 (Pointer to the GUID byte array)
 * @param[out] guid 指向 eprosima::fastrtps::rtps::GUID_t 结构的指针 (Pointer to the
 * eprosima::fastrtps::rtps::GUID_t structure)
 */
template <typename ByteT>
void copy_from_byte_array_to_fastrtps_guid(
    const ByteT* guid_byte_array, eprosima::fastrtps::rtps::GUID_t* guid) {
  // 静态断言，确保 ByteT 类型是 int8_t 或 uint8_t
  // Static assertion to ensure that ByteT type is either int8_t or uint8_t
  static_assert(
      std::is_same<uint8_t, ByteT>::value || std::is_same<int8_t, ByteT>::value,
      "ByteT should be either int8_t or uint8_t");

  // 断言，确保 guid_byte_array 和 guid 非空
  // Assert to ensure that guid_byte_array and guid are not null
  assert(guid_byte_array);
  assert(guid);

  // 计算 GUID 前缀大小
  // Calculate the prefix size of the GUID
  constexpr auto prefix_size = sizeof(guid->guidPrefix.value);

  // 将 guid_byte_array 的内容复制到 guid 的 guidPrefix 中
  // Copy the content of guid_byte_array to the guidPrefix of guid
  memcpy(guid->guidPrefix.value, guid_byte_array, prefix_size);

  // 将 guid_byte_array 的剩余内容复制到 guid 的 entityId 中
  // Copy the remaining content of guid_byte_array to the entityId of guid
  memcpy(guid->entityId.value, &guid_byte_array[prefix_size], guid->entityId.size);
}

/**
 * @brief 将 FastRTPS GUID 复制到字节数组中 (Copy FastRTPS GUID to byte array)
 *
 * @tparam ByteT 字节类型，应为 int8_t 或 uint8_t (Byte type, should be either int8_t or uint8_t)
 * @param[in] guid eprosima::fastrtps::rtps::GUID_t 结构的引用 (Reference to the
 * eprosima::fastrtps::rtps::GUID_t structure)
 * @param[out] guid_byte_array 指向 GUID 字节数组的指针 (Pointer to the GUID byte array)
 */
template <typename ByteT>
void copy_from_fastrtps_guid_to_byte_array(
    const eprosima::fastrtps::rtps::GUID_t& guid, ByteT* guid_byte_array) {
  // 静态断言，确保 ByteT 类型是 int8_t 或 uint8_t
  // Static assertion to ensure that ByteT type is either int8_t or uint8_t
  static_assert(
      std::is_same<uint8_t, ByteT>::value || std::is_same<int8_t, ByteT>::value,
      "ByteT should be either int8_t or uint8_t");

  // 断言，确保 guid_byte_array 非空
  // Assert to ensure that guid_byte_array is not null
  assert(guid_byte_array);

  // 计算 GUID 前缀大小
  // Calculate the prefix size of the GUID
  constexpr auto prefix_size = sizeof(guid.guidPrefix.value);

  // 将 guid 的 guidPrefix 复制到 guid_byte_array 中
  // Copy the guidPrefix of guid to guid_byte_array
  memcpy(guid_byte_array, &guid.guidPrefix, prefix_size);

  // 将 guid 的 entityId 复制到 guid_byte_array 的剩余部分中
  // Copy the entityId of guid to the remaining part of guid_byte_array
  memcpy(&guid_byte_array[prefix_size], &guid.entityId, guid.entityId.size);
}

/**
 * @brief FastRTPS GUID 的哈希结构 (Hash structure for FastRTPS GUID)
 */
struct hash_fastrtps_guid {
  /**
   * @brief 计算 FastRTPS GUID 的哈希值 (Calculate the hash value of FastRTPS GUID)
   *
   * @param[in] guid eprosima::fastrtps::rtps::GUID_t 结构的引用 (Reference to the
   * eprosima::fastrtps::rtps::GUID_t structure)
   * @return 哈希值 (Hash value)
   */
  std::size_t operator()(const eprosima::fastrtps::rtps::GUID_t& guid) const {
    // 联合体，用于转换 GUID 到 uint32_t 数组
    // Union for converting GUID to uint32_t array
    union u_convert {
      uint8_t plain_value[sizeof(guid)];
      uint32_t plain_ints[sizeof(guid) / sizeof(uint32_t)];
    } u{};

    // 静态断言，确保 GUID 可以轻松地转换为 uint32_t[4]
    // Static assertion to ensure that GUID can be easily converted to uint32_t[4]
    static_assert(
        sizeof(guid) == 16 && sizeof(u.plain_value) == sizeof(u.plain_ints) &&
            offsetof(u_convert, plain_value) == offsetof(u_convert, plain_ints),
        "Plain guid should be easily convertible to uint32_t[4]");

    // 将 FastRTPS GUID 复制到字节数组中
    // Copy FastRTPS GUID to byte array
    copy_from_fastrtps_guid_to_byte_array(guid, u.plain_value);

    // 定义质数用于哈希计算
    // Define primes for hash calculation
    constexpr std::size_t prime_1 = 7;
    constexpr std::size_t prime_2 = 31;
    constexpr std::size_t prime_3 = 59;

    // 计算哈希值
    // Calculate the hash value
    size_t ret_val = prime_1 * u.plain_ints[0];
    ret_val = prime_2 * (u.plain_ints[1] + ret_val);
    ret_val = prime_3 * (u.plain_ints[2] + ret_val);
    ret_val = u.plain_ints[3] + ret_val;

    return ret_val;
  }
};

}  // namespace rmw_fastrtps_shared_cpp

#endif  // RMW_FASTRTPS_SHARED_CPP__GUID_UTILS_HPP_
