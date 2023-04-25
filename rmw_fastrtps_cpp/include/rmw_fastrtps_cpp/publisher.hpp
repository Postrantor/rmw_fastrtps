// Copyright 2017-2019 Open Source Robotics Foundation, Inc.
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

#ifndef RMW_FASTRTPS_CPP__PUBLISHER_HPP_
#define RMW_FASTRTPS_CPP__PUBLISHER_HPP_

#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"

namespace rmw_fastrtps_cpp {

/**
 * @brief 创建一个发布者 (Create a publisher)
 *
 * @param[in] participant_info 参与者信息，包括域ID和参与者实例 (Participant information, including
 * domain ID and participant instance)
 * @param[in] type_supports 消息类型支持，用于序列化和反序列化消息 (Message type support for
 * serialization and deserialization of messages)
 * @param[in] topic_name 要发布的主题名称 (The name of the topic to be published)
 * @param[in] qos_policies 发布者的QoS策略 (Quality of Service policies for the publisher)
 * @param[in] publisher_options 发布者选项，包括分配器等 (Publisher options, including allocator,
 * etc.)
 *
 * @return 返回创建的rmw_publisher_t实例 (Returns the created rmw_publisher_t instance)
 */
rmw_publisher_t* create_publisher(
    CustomParticipantInfo* participant_info,
    const rosidl_message_type_support_t* type_supports,
    const char* topic_name,
    const rmw_qos_profile_t* qos_policies,
    const rmw_publisher_options_t* publisher_options);

}  // namespace rmw_fastrtps_cpp

#endif  // RMW_FASTRTPS_CPP__PUBLISHER_HPP_
