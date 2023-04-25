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
 * @brief 设置日志的严重级别 (Set the log severity level)
 *
 * @param[in] severity 日志严重级别枚举值 (Enumeration value of log severity level)
 * @return 返回设置日志严重级别操作的结果 (Return the result of setting the log severity level
 * operation)
 */
rmw_ret_t rmw_set_log_severity(rmw_log_severity_t severity) {
  // 调用 rmw_fastrtps_shared_cpp 命名空间中的 __rmw_set_log_severity 函数，将参数 severity 传递给它
  // (Call the __rmw_set_log_severity function in the rmw_fastrtps_shared_cpp namespace and pass the
  // parameter severity to it)
  return rmw_fastrtps_shared_cpp::__rmw_set_log_severity(severity);
}

}  // extern "C"
