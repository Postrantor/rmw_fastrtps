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

#include "rmw_fastrtps_cpp/serialization_format.hpp"

/**
 * @brief eprosima_fastrtps_serialization_format 是一个常量字符串，用于指定 Fast RTPS 的序列化格式。
 * @details 该字符串被设置为 "cdr"，表示使用 Common Data Representation (CDR)
 * 格式进行序列化。这是一种跨平台的二进制数据表示方法，广泛应用于 ROS2 和 DDS 系统中。
 *
 * eprosima_fastrtps_serialization_format is a constant string used to specify the serialization
 * format for Fast RTPS. The string is set to "cdr", which stands for Common Data Representation
 * (CDR), a cross-platform binary data representation widely used in ROS2 and DDS systems.
 */
const char* const eprosima_fastrtps_serialization_format = "cdr";
