// Copyright 2017 Open Source Robotics Foundation, Inc.
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

#ifndef RMW_FASTRTPS_CPP__GET_SUBSCRIBER_HPP_
#define RMW_FASTRTPS_CPP__GET_SUBSCRIBER_HPP_

#include "fastdds/dds/subscriber/DataReader.hpp"
#include "rmw/rmw.h"
#include "rmw_fastrtps_cpp/visibility_control.h"

namespace rmw_fastrtps_cpp {

/// 返回一个原生的 Fast DDS DataReader 句柄。
/**
 * 当订阅句柄为 `NULL` 或来自不同的 rmw 实现时，该函数返回 `NULL`。
 *
 * \return 如果成功，则返回原生的 Fast DDS DataReader 句柄；否则返回 `NULL`
 */
RMW_FASTRTPS_CPP_PUBLIC
eprosima::fastdds::dds::DataReader* get_datareader(rmw_subscription_t* subscription);

}  // namespace rmw_fastrtps_cpp

#endif  // RMW_FASTRTPS_CPP__GET_SUBSCRIBER_HPP_
