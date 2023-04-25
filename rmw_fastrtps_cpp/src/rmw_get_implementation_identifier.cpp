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
#include "rmw_fastrtps_cpp/identifier.hpp"

extern "C" {

/**
 * @brief 获取 RMW 实现的标识符 (Get the identifier of the RMW implementation)
 *
 * 这个函数返回一个指向 eprosima_fastrtps_identifier 的常量字符指针，用于标识当前使用的 RMW 实现。
 * (This function returns a constant character pointer pointing to eprosima_fastrtps_identifier,
 * used to identify the current RMW implementation being used.)
 *
 * @return 指向 eprosima_fastrtps_identifier 的常量字符指针 (A constant character pointer pointing
 * to eprosima_fastrtps_identifier)
 */
const char* rmw_get_implementation_identifier() {
  // 返回 eprosima_fastrtps_identifier，它是一个表示当前 RMW 实现的字符串
  // (Return eprosima_fastrtps_identifier, which is a string representing the current RMW
  // implementation)
  return eprosima_fastrtps_identifier;
}

}  // extern "C"
