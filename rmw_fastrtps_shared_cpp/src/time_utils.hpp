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

#ifndef TIME_UTILS_HPP_
#define TIME_UTILS_HPP_

#include "fastdds/rtps/common/Time_t.h"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 将 rmw_time_t 类型转换为 eprosima::fastrtps::Duration_t 类型 (Convert rmw_time_t to
 * eprosima::fastrtps::Duration_t)
 *
 * @param time 一个 rmw_time_t 类型的时间变量 (A time variable of type rmw_time_t)
 * @return eprosima::fastrtps::Duration_t 转换后的 eprosima::fastrtps::Duration_t 类型的时间变量
 * (The converted time variable of type eprosima::fastrtps::Duration_t)
 */
eprosima::fastrtps::Duration_t rmw_time_to_fastrtps(const rmw_time_t& time);

}  // namespace rmw_fastrtps_shared_cpp

#endif  // TIME_UTILS_HPP_
