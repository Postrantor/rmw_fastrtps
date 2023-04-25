// Copyright 2016-2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef RMW_FASTRTPS_DYNAMIC_CPP__MESSAGETYPESUPPORT_IMPL_HPP_
#define RMW_FASTRTPS_DYNAMIC_CPP__MESSAGETYPESUPPORT_IMPL_HPP_

#include <cassert>
#include <memory>
#include <sstream>
#include <string>

#include "fastcdr/Cdr.h"
#include "fastcdr/FastBuffer.h"
#include "rcpputils/find_and_replace.hpp"
#include "rmw_fastrtps_dynamic_cpp/MessageTypeSupport.hpp"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"

namespace rmw_fastrtps_dynamic_cpp {

/**
 * @brief 消息类型支持类 (Message type support class)
 *
 * @tparam MembersType 成员类型 (Member type)
 */
template <typename MembersType>
MessageTypeSupport<MembersType>::MessageTypeSupport(
    const MembersType* members, const void* ros_type_support)
    : TypeSupport<MembersType>(ros_type_support) {
  // 确保成员不为空 (Ensure members is not null)
  assert(members);
  this->members_ = members;

  // 创建一个字符串流 (Create a string stream)
  std::ostringstream ss;
  // 获取消息命名空间和消息名称 (Get message namespace and message name)
  std::string message_namespace(this->members_->message_namespace_);
  std::string message_name(this->members_->message_name_);
  if (!message_namespace.empty()) {
    // 查找并替换 C 命名空间分隔符为 C++，以防使用 C 类型支持 (Find and replace C namespace
    // separator with C++, in case this is using C typesupport)
    message_namespace = rcpputils::find_and_replace(message_namespace, "__", "::");
    ss << message_namespace << "::";
  }
  // 添加 DDS 前缀和消息名称 (Add DDS prefix and message name)
  ss << "dds_::" << message_name << "_";
  this->setName(ss.str().c_str());

  // 默认情况下完全绑定和简单 (Fully bound and plain by default)
  this->max_size_bound_ = true;
  this->is_plain_ = true;
  // 封装大小 (Encapsulation size)
  this->m_typeSize = 4;
  if (this->members_->member_count_ != 0) {
    // 计算最大序列化大小 (Calculate the maximum serialized size)
    this->m_typeSize += static_cast<uint32_t>(this->calculateMaxSerializedSize(members, 0));
  } else {
    this->m_typeSize++;
  }
  // 考虑 RTPS 子消息对齐 (Account for RTPS submessage alignment)
  this->m_typeSize = (this->m_typeSize + 3) & ~3;
}

}  // namespace rmw_fastrtps_dynamic_cpp

#endif  // RMW_FASTRTPS_DYNAMIC_CPP__MESSAGETYPESUPPORT_IMPL_HPP_
