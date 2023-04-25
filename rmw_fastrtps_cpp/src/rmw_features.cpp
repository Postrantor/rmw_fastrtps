// Copyright 2022 Open Source Robotics Foundation, Inc.
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

#include "rmw/features.h"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 检查 ROS2 Fast RTPS 实现是否支持指定的特性 (Check if the specified feature is supported by
 * ROS2 Fast RTPS implementation)
 *
 * @param[in] feature 要检查的特性枚举值 (The enumerated value of the feature to check)
 * @return 如果特性被支持，返回 true；否则，返回 false (Returns true if the feature is supported;
 * otherwise, returns false)
 */
bool rmw_feature_supported(rmw_feature_t feature) {
  // 调用 rmw_fastrtps_shared_cpp 中的 __rmw_feature_supported 函数来检查特性是否被支持
  // (Call the __rmw_feature_supported function in rmw_fastrtps_shared_cpp to check if the feature
  // is supported)
  return rmw_fastrtps_shared_cpp::__rmw_feature_supported(feature);
}

}  // extern "C"
