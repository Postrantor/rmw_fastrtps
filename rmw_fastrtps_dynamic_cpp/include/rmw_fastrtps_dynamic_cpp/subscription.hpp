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

#ifndef RMW_FASTRTPS_DYNAMIC_CPP__SUBSCRIPTION_HPP_
#define RMW_FASTRTPS_DYNAMIC_CPP__SUBSCRIPTION_HPP_

#include "rmw/rmw.h"
#include "rmw/subscription_options.h"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"

namespace rmw_fastrtps_dynamic_cpp {

/**
 * @brief 创建一个订阅者 (Create a subscription)
 *
 * @param[in] participant_info 参与者信息，包含了与 ROS2 通信所需的 FastRTPS 实体 (Participant
 * information, containing the FastRTPS entities needed for communication with ROS2)
 * @param[in] type_supports 消息类型支持，用于识别和处理特定消息类型 (Message type support, used to
 * identify and handle specific message types)
 * @param[in] topic_name 要订阅的主题名称 (The name of the topic to subscribe to)
 * @param[in] qos_policies 质量服务策略，用于配置订阅者的行为 (Quality of Service policies, used to
 * configure the behavior of the subscriber)
 * @param[in] subscription_options 订阅选项，包括忽略本地发布者等设置 (Subscription options,
 * including settings like ignoring local publishers)
 * @param[in] keyed 是否使用键值对模式 (Whether to use keyed mode)
 *
 * @return rmw_subscription_t* 指向创建的订阅者的指针 (Pointer to the created subscription)
 */
rmw_subscription_t* create_subscription(
    CustomParticipantInfo* participant_info,  // 自定义参与者信息 (Custom participant information)
    const rosidl_message_type_support_t* type_supports,  // 消息类型支持 (Message type supports)
    const char* topic_name,                              // 主题名称 (Topic name)
    const rmw_qos_profile_t* qos_policies,  // 质量服务策略 (Quality of Service policies)
    const rmw_subscription_options_t* subscription_options,  // 订阅选项 (Subscription options)
    bool keyed  // 是否使用键值对模式 (Whether to use keyed mode)
);

}  // namespace rmw_fastrtps_dynamic_cpp

#endif  // RMW_FASTRTPS_DYNAMIC_CPP__SUBSCRIPTION_HPP_
