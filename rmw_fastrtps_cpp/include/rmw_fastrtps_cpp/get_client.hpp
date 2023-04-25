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

#ifndef RMW_FASTRTPS_CPP__GET_CLIENT_HPP_
#define RMW_FASTRTPS_CPP__GET_CLIENT_HPP_

#include "fastdds/dds/publisher/DataWriter.hpp"
#include "fastdds/dds/subscriber/DataReader.hpp"
#include "rmw/rmw.h"
#include "rmw_fastrtps_cpp/visibility_control.h"

namespace rmw_fastrtps_cpp {

/// 返回一个用于请求的原生 Fast DDS DataWriter 句柄。 (Return a native Fast DDS DataWriter handle
/// for the request.)
/**
 * 当客户端句柄为 `NULL` 或者客户端句柄来自不同的 rmw 实现时，函数返回 `NULL`。
 * (The function returns `NULL` when either the client handle is `NULL` or
 * when the client handle is from a different rmw implementation.)
 *
 * \return 如果成功，则返回原生 Fast DDS DataWriter 句柄，否则返回 `NULL`
 *         (native Fast DDS DataWriter handle if successful, otherwise `NULL`)
 */
RMW_FASTRTPS_CPP_PUBLIC
eprosima::fastdds::dds::DataWriter* get_request_datawriter(rmw_client_t* client);

/// 返回一个用于响应的原生 Fast DDS DataReader 句柄。 (Return a native Fast DDS DataReader handle
/// for the response.)
/**
 * 当客户端句柄为 `NULL` 或者客户端句柄来自不同的 rmw 实现时，函数返回 `NULL`。
 * (The function returns `NULL` when either the client handle is `NULL` or
 * when the client handle is from a different rmw implementation.)
 *
 * \return 如果成功，则返回原生 Fast DDS DataReader 句柄，否则返回 `NULL`
 *         (native Fast DDS DataReader handle if successful, otherwise `NULL`)
 */
RMW_FASTRTPS_CPP_PUBLIC
eprosima::fastdds::dds::DataReader* get_response_datareader(rmw_client_t* client);

}  // namespace rmw_fastrtps_cpp

#endif  // RMW_FASTRTPS_CPP__GET_CLIENT_HPP_
