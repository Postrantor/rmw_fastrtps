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

#ifndef RMW_FASTRTPS_CPP__TYPESUPPORT_HPP_
#define RMW_FASTRTPS_CPP__TYPESUPPORT_HPP_

#include <cassert>
#include <string>

#include "fastcdr/Cdr.h"
#include "fastcdr/FastBuffer.h"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"
#include "rosidl_runtime_c/string.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"

namespace rmw_fastrtps_cpp {

/**
 * @class TypeSupport
 * @brief 类 TypeSupport 是 rmw_fastrtps_shared_cpp::TypeSupport 的子类，用于实现序列化和反序列化
 * ROS 消息。 Class TypeSupport is a subclass of rmw_fastrtps_shared_cpp::TypeSupport, used for
 * implementing serialization and deserialization of ROS messages.
 */
class TypeSupport : public rmw_fastrtps_shared_cpp::TypeSupport {
public:
  /**
   * @brief 获取估计的序列化大小
   *        Get the estimated serialized size
   *
   * @param[in] ros_message 输入的 ROS 消息
   *                        Input ROS message
   * @param[in] impl 实现细节
   *                 Implementation details
   * @return 返回序列化大小
   *         Return the serialized size
   */
  size_t getEstimatedSerializedSize(const void* ros_message, const void* impl) const override;

  /**
   * @brief 序列化 ROS 消息
   *        Serialize ROS message
   *
   * @param[in] ros_message 输入的 ROS 消息
   *                        Input ROS message
   * @param[out] ser 序列化后的数据
   *                 Serialized data
   * @param[in] impl 实现细节
   *                 Implementation details
   * @return 如果成功返回 true，否则返回 false
   *         Return true if successful, false otherwise
   */
  bool serializeROSmessage(
      const void* ros_message, eprosima::fastcdr::Cdr& ser, const void* impl) const override;

  /**
   * @brief 反序列化 ROS 消息
   *        Deserialize ROS message
   *
   * @param[in] deser 反序列化后的数据
   *                  Deserialized data
   * @param[out] ros_message 输出的 ROS 消息
   *                         Output ROS message
   * @param[in] impl 实现细节
   *                 Implementation details
   * @return 如果成功返回 true，否则返回 false
   *         Return true if successful, false otherwise
   */
  bool deserializeROSmessage(
      eprosima::fastcdr::Cdr& deser, void* ros_message, const void* impl) const override;

  /**
   * @brief 默认构造函数
   *        Default constructor
   */
  TypeSupport();

protected:
  /**
   * @brief 设置成员变量
   *        Set member variables
   *
   * @param[in] members 成员变量指针
   *                    Pointer to member variables
   */
  void set_members(const message_type_support_callbacks_t* members);

private:
  const message_type_support_callbacks_t* members_;  ///< 成员变量指针 Pointer to member variables
  bool has_data_;  ///< 是否有数据的标志 Flag for whether there is data
};

}  // namespace rmw_fastrtps_cpp

#endif  // RMW_FASTRTPS_CPP__TYPESUPPORT_HPP_
