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

#ifndef RMW_FASTRTPS_SHARED_CPP__PARTICIPANT_HPP_
#define RMW_FASTRTPS_SHARED_CPP__PARTICIPANT_HPP_

#include "rmw/types.h"
#include "rmw_dds_common/context.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/visibility_control.h"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 创建一个 DDS DomainParticipant，以及一个 DDS Publisher 和一个 DDS Subscriber。
 *        若要创建 DDS DataWriter，请参阅 create_publisher 方法。
 *        若要创建 DDS DataReader，请参阅 create_subscription 方法。
 *        请注意，ROS 2 Publishers 和 Subscriptions 分别对应于 DDS DataWriters 和 DataReaders，
 *        而不是 DDS Publishers 和 Subscribers。
 *
 * @brief This method will create a DDS DomainParticipant along with
 *        a DDS Publisher and a DDS Subscriber.
 *        For the creation of DDS DataWriter see method create_publisher.
 *        For the creation of DDS DataReader see method create_subscription.
 *        Note that ROS 2 Publishers and Subscriptions correspond with DDS DataWriters
 *        and DataReaders respectively and not with DDS Publishers and Subscribers.
 *
 * @param identifier 指定要创建的参与者的标识符。Specifies the identifier for the participant to be
 * created.
 * @param domain_id 指定要创建的参与者的域 ID。Specifies the domain ID for the participant to be
 * created.
 * @param security_options 指向 rmw_security_options_t 结构的指针，用于配置安全选项。A pointer to an
 * rmw_security_options_t structure for configuring security options.
 * @param discovery_options 指向 rmw_discovery_options_t 结构的指针，用于配置发现选项。A pointer to
 * an rmw_discovery_options_t structure for configuring discovery options.
 * @param enclave 指定参与者的围场。Specifies the enclave for the participant.
 * @param common_context 指向 rmw_dds_common::Context 结构的指针，用于存储公共上下文。A pointer to
 * an rmw_dds_common::Context structure for storing the common context.
 * @return 返回一个指向 CustomParticipantInfo 结构的指针，表示创建的参与者。Returns a pointer to a
 * CustomParticipantInfo structure representing the created participant.
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
CustomParticipantInfo* create_participant(
    const char* identifier,
    size_t domain_id,
    const rmw_security_options_t* security_options,
    const rmw_discovery_options_t* discovery_options,
    const char* enclave,
    rmw_dds_common::Context* common_context);

/**
 * @brief 销毁给定的 CustomParticipantInfo 结构，并释放其相关资源。
 *
 * @brief Destroy the given CustomParticipantInfo structure and release its associated resources.
 *
 * @param info 指向要销毁的 CustomParticipantInfo 结构的指针。A pointer to the CustomParticipantInfo
 * structure to be destroyed.
 * @return 返回一个 rmw_ret_t 枚举值，表示操作的结果。Returns an rmw_ret_t enumeration value
 * representing the result of the operation.
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t destroy_participant(CustomParticipantInfo* info);

}  // namespace rmw_fastrtps_shared_cpp

#endif  // RMW_FASTRTPS_SHARED_CPP__PARTICIPANT_HPP_
