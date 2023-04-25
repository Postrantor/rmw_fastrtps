// Copyright 2020 Canonical Ltd.
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

#include "rmw_fastrtps_shared_cpp/rmw_security_logging.hpp"

#include <map>
#include <sstream>
#include <string>
#include <utility>

#include "fastdds/rtps/attributes/PropertyPolicy.h"
#include "fastdds/rtps/common/Property.h"
#include "fastrtps/config.h"
#include "rcutils/env.h"
#include "rcutils/filesystem.h"
#include "rmw/error_handling.h"
#include "rmw/qos_profiles.h"
#include "rmw/types.h"

#if HAVE_SECURITY

namespace {

// 环境变量名 (Environment variable names)
// TODO(security-wg): 这些是临时的，需要重构为适当的抽象。
// (These are intended to be temporary, and need to be refactored into a proper abstraction.)
const char log_file_variable_name[] = "ROS_SECURITY_LOG_FILE";
const char log_publish_variable_name[] = "ROS_SECURITY_LOG_PUBLISH";
const char log_verbosity_variable_name[] = "ROS_SECURITY_LOG_VERBOSITY";

// 日志属性 (Logging properties)
const char logging_plugin_property_name[] = "dds.sec.log.plugin";
const char log_file_property_name[] = "dds.sec.log.builtin.DDS_LogTopic.log_file";
const char verbosity_property_name[] = "dds.sec.log.builtin.DDS_LogTopic.logging_level";
const char distribute_enable_property_name[] = "dds.sec.log.builtin.DDS_LogTopic.distribute";

// Fast DDS 支持以下详细程度 (Fast DDS supports the following verbosities):
//   - EMERGENCY_LEVEL
//   - ALERT_LEVEL
//   - CRITICAL_LEVEL
//   - ERROR_LEVEL
//   - WARNING_LEVEL
//   - NOTICE_LEVEL
//   - INFORMATIONAL_LEVEL
//   - DEBUG_LEVEL
//
// ROS 的日志级别较少，但在此处使用它们以保持一致性，因此我们有以下映射。
// (ROS has less logging levels, but it makes sense to use them here for consistency, so we have the
// following mapping.)
const std::map<RCUTILS_LOG_SEVERITY, std::string> verbosity_mapping{
    {RCUTILS_LOG_SEVERITY_FATAL, "EMERGENCY_LEVEL"},
    {RCUTILS_LOG_SEVERITY_ERROR, "ERROR_LEVEL"},
    {RCUTILS_LOG_SEVERITY_WARN, "WARNING_LEVEL"},
    {RCUTILS_LOG_SEVERITY_INFO, "INFORMATIONAL_LEVEL"},
    {RCUTILS_LOG_SEVERITY_DEBUG, "DEBUG_LEVEL"},
};

// 将严重性名称转换为字符串 (Convert severity names to string)
void severity_names_str(std::string &str) {
  std::stringstream stream;
  // 获取倒数第二个元素的迭代器 (Get iterator of the penultimate element)
  auto penultimate = --verbosity_mapping.crend();
  // 遍历映射，将严重性名称添加到流中 (Iterate through the mapping, adding severity names to the
  // stream)
  for (auto it = verbosity_mapping.crbegin(); it != penultimate; ++it) {
    stream << g_rcutils_log_severity_names[it->first] << ", ";
  }

  // 添加最后一个严重性名称 (Add the last severity name)
  stream << "or " << g_rcutils_log_severity_names[penultimate->first];
  // 将流转换为字符串 (Convert the stream to a string)
  str = stream.str();
}

/**
 * @brief 将字符串转换为日志级别 (Convert string to logging verbosity level)
 *
 * @param[in] str 输入的字符串 (Input string)
 * @param[out] verbosity 输出的日志级别 (Output logging verbosity level)
 * @return 转换成功返回 true，否则返回 false (Return true if conversion is successful, otherwise
 * return false)
 */
bool string_to_verbosity(const std::string &str, std::string &verbosity) {
  int ros_severity;
  // 尝试将输入字符串转换为 ROS 日志严重性级别 (Try to convert input string to ROS logging severity
  // level)
  if (rcutils_logging_severity_level_from_string(
          str.c_str(), rcutils_get_default_allocator(), &ros_severity) == RCUTILS_RET_OK) {
    try {
      // 使用映射表将 ROS 日志严重性级别转换为对应的日志级别 (Use mapping table to convert ROS
      // logging severity level to corresponding verbosity level)
      verbosity = verbosity_mapping.at(static_cast<RCUTILS_LOG_SEVERITY>(ros_severity));
      return true;
    } catch (std::out_of_range &) {
      // 如果映射表中找不到对应的日志级别，则跳到下面的返回语句 (If the corresponding verbosity
      // level cannot be found in the mapping table, jump to the return statement below)
    }
  }

  return false;
}

/**
 * @brief 验证字符串是否表示布尔值 (Validate if the string represents a boolean value)
 *
 * @param[in] str 输入的字符串 (Input string)
 * @return 如果字符串表示布尔值则返回 true，否则返回 false (Return true if the string represents a
 * boolean value, otherwise return false)
 */
bool validate_boolean(const std::string &str) { return str == "true" || str == "false"; }

/**
 * @brief 向属性序列中添加或更新属性 (Add or update property in the property sequence)
 *
 * @param[in, out] properties 属性序列 (Property sequence)
 * @param[in] property 要添加或更新的属性 (Property to add or update)
 */
void add_property(
    eprosima::fastrtps::rtps::PropertySeq &properties,
    eprosima::fastrtps::rtps::Property &&property) {
  // 将属性添加到向量中。如果属性已存在，则覆盖它 (Add property to vector. If property already
  // exists, overwrite it)
  std::string property_name = property.name();
  for (auto &existing_property : properties) {
    if (existing_property.name() == property_name) {
      existing_property = property;
      return;
    }
  }

  properties.push_back(property);
}

/**
 * @brief 获取环境变量的值 (Get the value of an environment variable)
 *
 * @param[in] variable_name 环境变量名称 (Environment variable name)
 * @param[out] variable_value 输出的环境变量值 (Output environment variable value)
 * @return 如果获取成功则返回 true，否则返回 false (Return true if getting the value is successful,
 * otherwise return false)
 */
bool get_env(const std::string &variable_name, std::string &variable_value) {
  const char *value;
  // 获取环境变量值 (Get the value of the environment variable)
  const char *error_message = rcutils_get_env(variable_name.c_str(), &value);
  if (error_message != NULL) {
    // 如果获取失败，设置错误消息 (If getting the value fails, set an error message)
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "unable to get %s environment variable: %s", variable_name.c_str(), error_message);
    return false;
  }

  variable_value = std::string(value);

  return true;
}

}  // namespace

#endif

/**
 * @brief 应用安全日志配置 (Apply security logging configuration)
 *
 * @param[in,out] policy eProsima Fast RTPS 属性策略对象 (eProsima Fast RTPS PropertyPolicy object)
 * @return 成功返回 true，失败返回 false (Return true if successful, false otherwise)
 */
bool apply_security_logging_configuration(eprosima::fastrtps::rtps::PropertyPolicy &policy) {
#if HAVE_SECURITY
  // 创建一个属性序列 (Create a property sequence)
  eprosima::fastrtps::rtps::PropertySeq properties;
  std::string env_value;

  // 处理写入文件的日志 (Handle logging to file)
  if (!get_env(log_file_variable_name, env_value)) {
    return false;
  }
  if (!env_value.empty()) {
    // 将属性添加到属性序列中 (Add the property to the property sequence)
    add_property(
        properties, eprosima::fastrtps::rtps::Property(log_file_property_name, env_value.c_str()));
  }

  // 处理通过 DDS 分发的日志 (Handle log distribution over DDS)
  if (!get_env(log_publish_variable_name, env_value)) {
    return false;
  }
  if (!env_value.empty()) {
    if (!validate_boolean(env_value)) {
      // 设置错误消息 (Set error message)
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "%s is not valid: '%s' is not a supported value (use 'true' or 'false')",
          log_publish_variable_name, env_value.c_str());
      return false;
    }

    // 将属性添加到属性序列中 (Add the property to the property sequence)
    add_property(
        properties,
        eprosima::fastrtps::rtps::Property(distribute_enable_property_name, env_value.c_str()));
  }

  // 处理日志详细程度 (Handle log verbosity)
  if (!get_env(log_verbosity_variable_name, env_value)) {
    return false;
  }
  if (!env_value.empty()) {
    std::string verbosity;
    if (!string_to_verbosity(env_value, verbosity)) {
      std::string humanized_severity_list;
      severity_names_str(humanized_severity_list);

      // 设置错误消息 (Set error message)
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "%s is not valid: %s is not a supported verbosity (use %s)", log_verbosity_variable_name,
          env_value.c_str(), humanized_severity_list.c_str());
      return false;
    }

    // 将属性添加到属性序列中 (Add the property to the property sequence)
    add_property(
        properties, eprosima::fastrtps::rtps::Property(verbosity_property_name, verbosity.c_str()));
  }

  if (!properties.empty()) {
    // 将属性添加到属性序列中 (Add the property to the property sequence)
    add_property(
        properties,
        eprosima::fastrtps::rtps::Property(logging_plugin_property_name, "builtin.DDS_LogTopic"));
  }

  // 解析完成后，实际更新属性 (Now that we're done parsing, actually update the properties)
  for (auto &item : properties) {
    add_property(policy.properties(), std::move(item));
  }

  return true;
#else
  (void)policy;
  // 设置错误消息 (Set error message)
  RMW_SET_ERROR_MSG(
      "This Fast DDS version doesn't have the security libraries\n"
      "Please compile Fast DDS using the -DSECURITY=ON CMake option");
  return false;
#endif
}
