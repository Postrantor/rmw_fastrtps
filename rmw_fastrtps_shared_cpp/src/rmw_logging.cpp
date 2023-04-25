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

#include "fastdds/dds/log/Log.hpp"
#include "rcutils/logging_macros.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 设置日志严重程度 (Set the log severity)
 *
 * @param[in] severity 日志严重程度枚举值 (The enumeration value of log severity)
 * @return rmw_ret_t 返回设置结果，成功或错误 (Return the setting result, success or error)
 */
rmw_ret_t __rmw_set_log_severity(rmw_log_severity_t severity) {
  // 使用 eprosima::fastdds::dds::Log 命名空间 (Use the eprosima::fastdds::dds::Log namespace)
  using eprosima::fastdds::dds::Log;

  // 定义 log_kind 变量 (Define the log_kind variable)
  Log::Kind log_kind;

  // 根据 severity 参数设置 log_kind 的值 (Set the value of log_kind based on the severity
  // parameter)
  switch (severity) {
    case RMW_LOG_SEVERITY_DEBUG:
      // 向下穿透 (fall through)
    case RMW_LOG_SEVERITY_INFO:
      // 设置 log_kind 为 Info 类型 (Set log_kind to Info type)
      log_kind = Log::Kind::Info;
      break;
    case RMW_LOG_SEVERITY_WARN:
      // 设置 log_kind 为 Warning 类型 (Set log_kind to Warning type)
      log_kind = Log::Kind::Warning;
      break;
    case RMW_LOG_SEVERITY_ERROR:
      // 向下穿透 (fall through)
    case RMW_LOG_SEVERITY_FATAL:
      // 设置 log_kind 为 Error 类型 (Set log_kind to Error type)
      log_kind = Log::Kind::Error;
      break;
    default:
      // 如果遇到未知的严重程度类型，记录错误并返回 RMW_RET_ERROR (If an unknown severity type is
      // encountered, log the error and return RMW_RET_ERROR)
      RCUTILS_LOG_ERROR("Unknown logging severity type %d", severity);
      return RMW_RET_ERROR;
  }

  // 设置日志系统的详细程度 (Set the verbosity of the logging system)
  Log::SetVerbosity(log_kind);

  // 返回设置成功 (Return setting success)
  return RMW_RET_OK;
}
}  // namespace rmw_fastrtps_shared_cpp
