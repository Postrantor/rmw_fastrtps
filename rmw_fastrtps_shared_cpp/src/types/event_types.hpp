// Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef TYPES__EVENT_TYPES_HPP_
#define TYPES__EVENT_TYPES_HPP_

#include "fastdds/dds/core/status/StatusMask.hpp"
#include "rmw/event.h"

namespace rmw_fastrtps_shared_cpp {
namespace internal {

/**
 * @brief 判断事件类型是否受支持 (Check if the event type is supported)
 *
 * @param[in] event_type 事件类型 (Event type)
 * @return bool 如果事件类型受支持，则返回 true，否则返回 false (Returns true if the event type is
 * supported, otherwise returns false)
 */
bool is_event_supported(rmw_event_type_t event_type);

/**
 * @brief 将 rmw 事件类型转换为 Fast DDS 状态掩码 (Convert rmw event type to Fast DDS status mask)
 *
 * @param[in] event_type rmw 事件类型 (rmw event type)
 * @return eprosima::fastdds::dds::StatusMask 返回对应的 Fast DDS 状态掩码 (Returns the
 * corresponding Fast DDS status mask)
 */
eprosima::fastdds::dds::StatusMask rmw_event_to_dds_statusmask(const rmw_event_type_t event_type);

}  // namespace internal
}  // namespace rmw_fastrtps_shared_cpp

#endif  // TYPES__EVENT_TYPES_HPP_
