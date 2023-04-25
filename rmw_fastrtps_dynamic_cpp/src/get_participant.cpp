// Copyright 2017 Open Source Robotics Foundation, Inc.
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

#include "rmw_fastrtps_dynamic_cpp/get_participant.hpp"

#include "fastdds/dds/domain/DomainParticipant.hpp"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"

namespace rmw_fastrtps_dynamic_cpp {

/**
 * @brief 获取域参与者 (Get the domain participant)
 *
 * @param[in] node rmw_node_t类型的指针，表示ROS2节点 (Pointer to an rmw_node_t, representing a ROS2
 * node)
 * @return eprosima::fastdds::dds::DomainParticipant*
 * 返回域参与者指针，如果输入节点为空或实现标识符不匹配，则返回nullptr (Returns a pointer to the
 * domain participant, or nullptr if the input node is null or the implementation identifier does
 * not match)
 */
eprosima::fastdds::dds::DomainParticipant *get_domain_participant(rmw_node_t *node) {
  // 如果节点为空，返回nullptr (If the node is null, return nullptr)
  if (!node) {
    return nullptr;
  }
  // 如果实现标识符与期望的eprosima_fastrtps_identifier不匹配，返回nullptr (If the implementation
  // identifier does not match the expected eprosima_fastrtps_identifier, return nullptr)
  if (node->implementation_identifier != eprosima_fastrtps_identifier) {
    return nullptr;
  }
  // 将节点上下文中的参与者信息强制转换为CustomParticipantInfo类型 (Cast the participant information
  // in the node context to CustomParticipantInfo type)
  auto impl = static_cast<CustomParticipantInfo *>(node->context->impl->participant_info);
  // 返回参与者指针 (Return the participant pointer)
  return impl->participant_;
}

}  // namespace rmw_fastrtps_dynamic_cpp
