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

#include "rmw/rmw.h"
#include "rmw_fastrtps_dynamic_cpp/serialization_format.hpp"

extern "C" {

/**
 * @brief 获取序列化格式 (Get the serialization format)
 *
 * @return 返回 eprosima_fastrtps_dynamic_serialization_format 的指针 (Return a pointer to
 * eprosima_fastrtps_dynamic_serialization_format)
 */
const char* rmw_get_serialization_format() {
  // 返回 eprosima_fastrtps_dynamic_serialization_format 的指针
  // (Return a pointer to eprosima_fastrtps_dynamic_serialization_format)
  return eprosima_fastrtps_dynamic_serialization_format;
}

}  // extern "C"
