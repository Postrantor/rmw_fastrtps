// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#ifndef RMW_FASTRTPS_CPP__SUBSCRIPTION_HPP_
#define RMW_FASTRTPS_CPP__SUBSCRIPTION_HPP_

#include "rmw/rmw.h"
#include "rmw/subscription_options.h"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"

namespace rmw_fastrtps_cpp {

/**
 * @brief 创建一个订阅对象 (Create a subscription object)
 *
 * @param[in] participant_info 参与者信息，包含了 ROS2 通信所需的各种信息和配置 (Participant
 * information, contains various information and configurations required for ROS2 communication)
 * @param[in] type_supports 消息类型支持，用于序列化和反序列化消息 (Message type support, used for
 * serialization and deserialization of messages)
 * @param[in] topic_name 要订阅的主题名称 (The topic name to subscribe to)
 * @param[in] qos_policies QoS 策略，用于控制订阅的质量 (QoS policies, used to control the quality
 * of the subscription)
 * @param[in] subscription_options 订阅选项，包括一些额外的订阅配置 (Subscription options, including
 * some additional subscription configurations)
 * @param[in] keyed 是否使用键值对进行订阅 (Whether to use key-value pairs for subscription)
 *
 * @return rmw_subscription_t* 成功时返回一个指向新创建的订阅对象的指针，失败时返回 nullptr (Returns
 * a pointer to the newly created subscription object on success, nullptr on failure)
 */
rmw_subscription_t* create_subscription(
    CustomParticipantInfo* participant_info,
    const rosidl_message_type_support_t* type_supports,
    const char* topic_name,
    const rmw_qos_profile_t* qos_policies,
    const rmw_subscription_options_t* subscription_options,
    bool keyed);

}  // namespace rmw_fastrtps_cpp

#endif  // RMW_FASTRTPS_CPP__SUBSCRIPTION_HPP_
