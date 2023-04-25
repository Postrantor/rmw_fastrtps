// Copyright 2018 Open Source Robotics Foundation, Inc.
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

#include "rmw_fastrtps_dynamic_cpp/serialization_format.hpp"

/**
 * @brief eprosima_fastrtps_dynamic_serialization_format 是一个常量字符串，表示 Fast RTPS
 * 动态类型序列化格式。
 * @details 该字符串用于指定 ROS2 项目中 rmw_fastrtps_dynamic_cpp 的序列化格式。Fast RTPS
 * 是一种实时发布-订阅通信中间件。
 *
 * eprosima_fastrtps_dynamic_serialization_format is a constant string representing the Fast RTPS
 * dynamic type serialization format.
 * @details This string is used to specify the serialization format in the ROS2 project's
 * rmw_fastrtps_dynamic_cpp. Fast RTPS is a real-time publish-subscribe communication middleware.
 */
const char* const eprosima_fastrtps_dynamic_serialization_format = "cdr";
