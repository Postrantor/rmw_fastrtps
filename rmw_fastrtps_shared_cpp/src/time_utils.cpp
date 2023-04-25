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

#include "rmw_dds_common/time_utils.hpp"

#include "time_utils.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 将 rmw_time_t 类型的时间转换为 eprosima::fastrtps::Duration_t 类型的时间 (Converts
 * rmw_time_t type time to eprosima::fastrtps::Duration_t type time)
 *
 * @param time 一个 rmw_time_t 类型的时间变量 (A rmw_time_t type time variable)
 * @return eprosima::fastrtps::Duration_t 转换后的 eprosima::fastrtps::Duration_t 类型的时间 (The
 * converted eprosima::fastrtps::Duration_t type time)
 */
eprosima::fastrtps::Duration_t rmw_time_to_fastrtps(const rmw_time_t& time) {
  // 判断传入的时间是否为无穷大 (Check if the input time is infinite)
  // 如果是无穷大，则返回 FastRTPS 中表示无穷大的 Duration_t 类型的时间 (If it is infinite, return
  // the Duration_t type time representing infinity in FastRTPS)
  if (rmw_time_equal(time, RMW_DURATION_INFINITE)) {
    return eprosima::fastrtps::rtps::c_RTPSTimeInfinite.to_duration_t();
  }

  // 将 rmw_time_t 类型的时间限制在 DDS 支持的范围内 (Clamp the rmw_time_t type time within the
  // range supported by DDS)
  rmw_time_t clamped_time = rmw_dds_common::clamp_rmw_time_to_dds_time(time);

  // 将限制后的 rmw_time_t 类型的时间转换为 eprosima::fastrtps::Duration_t 类型的时间并返回 (Convert
  // the clamped rmw_time_t type time to eprosima::fastrtps::Duration_t type time and return)
  return eprosima::fastrtps::Duration_t(
      static_cast<int32_t>(clamped_time.sec), static_cast<uint32_t>(clamped_time.nsec));
}

}  // namespace rmw_fastrtps_shared_cpp
