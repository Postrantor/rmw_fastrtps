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

#ifndef RMW_FASTRTPS_DYNAMIC_CPP__PUBLISHER_HPP_
#define RMW_FASTRTPS_DYNAMIC_CPP__PUBLISHER_HPP_

#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"

namespace rmw_fastrtps_dynamic_cpp {

/**
 * @brief 创建一个发布者 (Create a publisher)
 *
 * @param[in] participant_info 自定义参与者信息 (Custom participant information)
 * @param[in] type_supports 消息类型支持 (Message type supports)
 * @param[in] topic_name 主题名称 (Topic name)
 * @param[in] qos_policies 服务质量策略 (Quality of Service policies)
 * @param[in] publisher_options 发布者选项 (Publisher options)
 * @return rmw_publisher_t* 创建的发布者指针 (Pointer to the created publisher)
 */
rmw_publisher_t* create_publisher(
    // 自定义参与者信息，用于存储 ROS2 节点和 FastRTPS 参与者之间的映射关系 (Custom participant
    // information for storing the mapping between ROS2 nodes and FastRTPS participants)
    CustomParticipantInfo* participant_info,
    // 消息类型支持，用于确定所使用的消息类型 (Message type supports for determining the message
    // type being used)
    const rosidl_message_type_support_t* type_supports,
    // 主题名称，发布者将在此主题上发布消息 (Topic name on which the publisher will publish
    // messages)
    const char* topic_name,
    // 服务质量策略，用于配置发布者的服务质量设置 (Quality of Service policies for configuring the
    // publisher's QoS settings)
    const rmw_qos_profile_t* qos_policies,
    // 发布者选项，包括分配器等其他选项 (Publisher options including allocator and other options)
    const rmw_publisher_options_t* publisher_options);

}  // namespace rmw_fastrtps_dynamic_cpp

#endif  // RMW_FASTRTPS_DYNAMIC_CPP__PUBLISHER_HPP_
