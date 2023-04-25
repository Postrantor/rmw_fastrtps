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

/**
 * @brief 检查 rmw_fastrtps_shared_cpp 是否支持指定的特性 (Check if the specified feature is
 * supported by rmw_fastrtps_shared_cpp)
 *
 * @param[in] feature 要检查的特性枚举值 (The enumeration value of the feature to check)
 * @return bool 如果特性被支持则返回 true，否则返回 false (Returns true if the feature is supported,
 * otherwise returns false)
 */
bool rmw_fastrtps_shared_cpp::__rmw_feature_supported(rmw_feature_t feature) {
  switch (feature) {
    // 支持消息信息发布序列号特性 (Supports message info publication sequence number feature)
    case RMW_FEATURE_MESSAGE_INFO_PUBLICATION_SEQUENCE_NUMBER:
      return true;

    // 注意：动态类型推迟案例 !! 不受支持 !! (NOTE: The dynamic type deferred case is !! NOT
    // SUPPORTED !!) 原因是在没有类型的情况下创建订阅很困难。需要在其他地方进行太多重构以支持延迟...
    // (This is because it's difficult as-is to create a subscription without already having the
    // type. Too much restructuring is needed elsewhere to support deferral...)
    //
    // 需要注意的是，FastRTPS 支持类型发现，但是 create_subscription
    // 实现的结构目前还不适合集成该类型发现支持。 (This is noting that type discovery IS a thing
    // that FastRTPS supports, but the structure of the create_subscription implementations don't
    // lend themselves currently to integrating that type discovery support yet.)
    case RMW_MIDDLEWARE_SUPPORTS_TYPE_DISCOVERY:
      return false;

    // 支持接收动态消息特性 (Supports taking dynamic message feature)
    case RMW_MIDDLEWARE_CAN_TAKE_DYNAMIC_MESSAGE:
      return true;

    // 其他情况下，特性不受支持 (In other cases, the feature is not supported)
    default:
      return false;
  }
}
