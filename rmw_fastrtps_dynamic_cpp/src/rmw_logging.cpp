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

#include "rcutils/logging_macros.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 设置日志严重性级别 (Set the log severity level)
 *
 * @param[in] severity 日志严重性级别枚举值 (The enumeration value of the log severity level)
 * @return 返回 rmw_ret_t 类型的结果，表示设置操作是否成功 (Returns a result of type rmw_ret_t,
 * indicating whether the setting operation was successful)
 */
rmw_ret_t rmw_set_log_severity(rmw_log_severity_t severity) {
  // 调用 rmw_fastrtps_shared_cpp 命名空间中的 __rmw_set_log_severity 函数，并将 severity
  // 参数传递给它 (Call the __rmw_set_log_severity function in the rmw_fastrtps_shared_cpp namespace
  // and pass the severity parameter to it)
  return rmw_fastrtps_shared_cpp::__rmw_set_log_severity(severity);
}

}  // extern "C"
