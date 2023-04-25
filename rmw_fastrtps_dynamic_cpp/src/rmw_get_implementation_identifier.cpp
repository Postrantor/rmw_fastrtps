// Copyright 2016-2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"

extern "C" {

/**
 * @brief 获取 RMW 实现的标识符 (Get the RMW implementation identifier)
 *
 * @return 返回 eprosima_fastrtps_identifier，它是一个表示 FastRTPS 动态实现的字符串 (Returns
 * eprosima_fastrtps_identifier, which is a string representing the FastRTPS dynamic implementation)
 */
const char* rmw_get_implementation_identifier() {
  // 返回 eprosima_fastrtps_identifier，这是一个用于表示 FastRTPS 动态实现的字符串
  // (Return eprosima_fastrtps_identifier, which is a string used to represent the FastRTPS dynamic
  // implementation)
  return eprosima_fastrtps_identifier;
}

}  // extern "C"
