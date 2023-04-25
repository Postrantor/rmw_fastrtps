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

#ifndef RMW_FASTRTPS_DYNAMIC_CPP__SERVICETYPESUPPORT_IMPL_HPP_
#define RMW_FASTRTPS_DYNAMIC_CPP__SERVICETYPESUPPORT_IMPL_HPP_

#include <cassert>
#include <sstream>
#include <string>

#include "fastcdr/Cdr.h"
#include "fastcdr/FastBuffer.h"
#include "rcpputils/find_and_replace.hpp"
#include "rmw_fastrtps_dynamic_cpp/ServiceTypeSupport.hpp"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"

namespace rmw_fastrtps_dynamic_cpp {

/**
 * @brief RequestTypeSupport 类模板 (RequestTypeSupport class template)
 *
 * @tparam ServiceMembersType 服务成员类型 (Service members type)
 * @tparam MessageMembersType 消息成员类型 (Message members type)
 */
template <typename ServiceMembersType, typename MessageMembersType>
RequestTypeSupport<ServiceMembersType, MessageMembersType>::RequestTypeSupport(
    const ServiceMembersType* members, const void* ros_type_support)
    : TypeSupport<MessageMembersType>(ros_type_support) {
  // 确保 members 不为空 (Ensure members is not null)
  assert(members);
  // 设置请求成员 (Set request members)
  this->members_ = members->request_members_;

  std::ostringstream ss;
  // 获取服务命名空间 (Get service namespace)
  std::string service_namespace(members->service_namespace_);
  // 获取服务名称 (Get service name)
  std::string service_name(members->service_name_);
  if (!service_namespace.empty()) {
    // 查找并替换 C 命名空间分隔符为 C++，以防使用 C 类型支持 (Find and replace C namespace
    // separator with C++, in case this is using C typesupport)
    service_namespace = rcpputils::find_and_replace(service_namespace, "__", "::");
    ss << service_namespace << "::";
  }
  ss << "dds_::" << service service_name << "_Request_";
  // 设置类型名称 (Set type name)
  this->setName(ss.str().c_str());

  // 默认情况下完全绑定和简单 (Fully bound and plain by default)
  this->max_size_bound_ = true;
  this->is_plain_ = true;
  // 封装大小 (Encapsulation size)
  this->m_typeSize = 4;
  if (this->members_->member_count_ != 0) {
    // 计算最大序列化大小 (Calculate max serialized size)
    this->m_typeSize += static_cast<uint32_t>(this->calculateMaxSerializedSize(this->members_, 0));
  } else {
    this->m_typeSize++;
  }
  // 考虑 RTPS 子消息对齐 (Account for RTPS submessage alignment)
  this->m_typeSize = (this->m_typeSize + 3) & ~3;
}

/**
 * @brief ResponseTypeSupport 类模板，用于支持 ROS2 服务的响应类型。
 *        The ResponseTypeSupport class template, used to support response types in ROS2 services.
 *
 * @tparam ServiceMembersType 服务成员类型。The service members type.
 * @tparam MessageMembersType 消息成员类型。The message members type.
 */
template <typename ServiceMembersType, typename MessageMembersType>
ResponseTypeSupport<ServiceMembersType, MessageMembersType>::ResponseTypeSupport(
    const ServiceMembersType* members, const void* ros_type_support)
    : TypeSupport<MessageMembersType>(ros_type_support) {
  // 确保 members 不为空。Ensure that members is not null.
  assert(members);
  // 设置响应成员。Set the response members.
  this->members_ = members->response_members_;

  std::ostringstream ss;
  std::string service_namespace(members->service_namespace_);
  std::string service_name(members->service_name_);
  if (!service_namespace.empty()) {
    // 如果使用 C 类型支持，请查找并替换 C 命名空间分隔符为 C++。Find and replace C namespace
    // separator with C++, in case this is using C typesupport.
    service_namespace = rcpputils::find_and_replace(service_namespace, "__", "::");
    ss << service_namespace << "::";
  }
  // 添加 DDS 命名空间和服务名称。Add DDS namespace and service name.
  ss << "dds_::" << service_name << "_Response_";
  // 设置类型名称。Set the type name.
  this->setName(ss.str().c_str());

  // 默认情况下完全绑定和简单。Fully bound and plain by default.
  this->max_size_bound_ = true;
  this->is_plain_ = true;
  // 封装大小。Encapsulation size.
  this->m_typeSize = 4;
  if (this->members_->member_count_ != 0) {
    // 计算最大序列化大小。Calculate the maximum serialized size.
    this->m_typeSize += static_cast<uint32_t>(this->calculateMaxSerializedSize(this->members_, 0));
  } else {
    this->m_typeSize++;
  }
  // 考虑 RTPS 子消息对齐。Account for RTPS submessage alignment.
  this->m_typeSize = (this->m_typeSize + 3) & ~3;
}

}  // namespace rmw_fastrtps_dynamic_cpp

#endif  // RMW_FASTRTPS_DYNAMIC_CPP__SERVICETYPESUPPORT_IMPL_HPP_
