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

#include "rmw_fastrtps_cpp/get_participant.hpp"

#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"

namespace rmw_fastrtps_cpp {

/**
 * @brief 获取域参与者 (Get the domain participant)
 *
 * @param[in] node RMW节点指针 (Pointer to the RMW node)
 * @return eprosima::fastdds::dds::DomainParticipant* 域参与者指针，如果失败则返回nullptr (Pointer
 * to the domain participant, nullptr if failed)
 */
eprosima::fastdds::dds::DomainParticipant *get_domain_participant(rmw_node_t *node) {
  // 检查传入的节点是否为空 (Check if the input node is nullptr)
  if (!node) {
    return nullptr;  // 如果节点为空，则返回nullptr (Return nullptr if the node is nullptr)
  }

  // 检查节点的实现标识符是否为 eprosima_fastrtps_identifier (Check if the node's
  // implementation_identifier is eprosima_fastrtps_identifier)
  if (node->implementation_identifier != eprosima_fastrtps_identifier) {
    return nullptr;  // 如果实现标识符不匹配，则返回nullptr (Return nullptr if the
                     // implementation_identifier does not match)
  }

  // 将节点的参与者信息转换为 CustomParticipantInfo 类型 (Cast the node's participant_info to
  // CustomParticipantInfo type)
  auto impl = static_cast<CustomParticipantInfo *>(node->context->impl->participant_info);

  // 返回域参与者指针 (Return the domain participant pointer)
  return impl->participant_;
}

}  // namespace rmw_fastrtps_cpp
