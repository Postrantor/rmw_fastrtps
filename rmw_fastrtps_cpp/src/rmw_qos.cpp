// Copyright 2021 Open Source Robotics Foundation, Inc.
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

#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 检查发布者和订阅者的 QoS 配置文件是否兼容 (Check if the QoS profiles of publisher and
 * subscriber are compatible)
 *
 * @param[in] publisher_profile 发布者的 QoS 配置文件 (QoS profile of the publisher)
 * @param[in] subscription_profile 订阅者的 QoS 配置文件 (QoS profile of the subscriber)
 * @param[out] compatibility 返回兼容性类型 (Returns the compatibility type)
 * @param[out] reason 如果不兼容，返回原因 (If not compatible, returns the reason)
 * @param[in] reason_size 原因字符串的大小 (Size of the reason string)
 * @return rmw_ret_t 返回操作结果 (Returns the operation result)
 */
rmw_ret_t rmw_qos_profile_check_compatible(
    const rmw_qos_profile_t publisher_profile,
    const rmw_qos_profile_t subscription_profile,
    rmw_qos_compatibility_type_t* compatibility,
    char* reason,
    size_t reason_size) {
  // 调用 __rmw_qos_profile_check_compatible 函数来检查发布者和订阅者的 QoS 配置文件是否兼容
  // (Call the __rmw_qos_profile_check_compatible function to check if the QoS profiles of publisher
  // and subscriber are compatible)
  return rmw_fastrtps_shared_cpp::__rmw_qos_profile_check_compatible(
      publisher_profile, subscription_profile, compatibility, reason, reason_size);
}

}  // extern "C"
