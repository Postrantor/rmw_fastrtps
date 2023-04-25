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

#ifndef RMW_FASTRTPS_DYNAMIC_CPP__MESSAGETYPESUPPORT_HPP_
#define RMW_FASTRTPS_DYNAMIC_CPP__MESSAGETYPESUPPORT_HPP_

#include <cassert>
#include <memory>

#include "TypeSupport.hpp"
#include "fastcdr/Cdr.h"
#include "fastcdr/FastBuffer.h"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"

namespace rmw_fastrtps_dynamic_cpp {

/**
 * @brief 模板类 MessageTypeSupport，继承自 TypeSupport 类。
 *        Template class MessageTypeSupport, inheriting from TypeSupport class.
 *
 * @tparam MembersType 成员类型。Member type.
 */
template <typename MembersType>
class MessageTypeSupport : public TypeSupport<MembersType> {
public:
  /**
   * @brief MessageTypeSupport 构造函数。
   *        Constructor for MessageTypeSupport.
   *
   * @param members 成员类型的指针，用于初始化 TypeSupport 基类。
   *                Pointer to the member type, used for initializing the TypeSupport base class.
   * @param ros_type_support ROS 类型支持的指针，用于传递给 TypeSupport 基类。
   *                         Pointer to the ROS type support, passed to the TypeSupport base class.
   */
  MessageTypeSupport(const MembersType* members, const void* ros_type_support);
};

}  // namespace rmw_fastrtps_dynamic_cpp

#include "MessageTypeSupport_impl.hpp"

#endif  // RMW_FASTRTPS_DYNAMIC_CPP__MESSAGETYPESUPPORT_HPP_
