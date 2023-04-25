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

#ifndef RMW_FASTRTPS_SHARED_CPP__TYPESUPPORT_HPP_
#define RMW_FASTRTPS_SHARED_CPP__TYPESUPPORT_HPP_

#include <cassert>
#include <string>

#include "./visibility_control.h"
#include "fastcdr/Cdr.h"
#include "fastcdr/FastBuffer.h"
#include "fastdds/dds/topic/TopicDataType.hpp"
#include "fastdds/rtps/common/InstanceHandle.h"
#include "fastdds/rtps/common/SerializedPayload.h"
#include "rcutils/logging_macros.h"
#include "rosidl_runtime_c/message_type_support_struct.h"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 序列化数据类型枚举 (Serialized data type enumeration)
 *
 * 用于指示序列化数据的类型，以便在发布和订阅时进行正确的处理。
 * (Indicates the type of serialized data for proper handling during publishing and subscribing.)
 */
enum SerializedDataType {
  FASTRTPS_SERIALIZED_DATA_TYPE_CDR_BUFFER,       ///< CDR 缓冲区类型 (CDR buffer type)
  FASTRTPS_SERIALIZED_DATA_TYPE_DYNAMIC_MESSAGE,  ///< 动态消息类型 (Dynamic message type)
  FASTRTPS_SERIALIZED_DATA_TYPE_ROS_MESSAGE       ///< ROS 消息类型 (ROS message type)
};

/**
 * @brief 序列化数据结构 (Serialized data structure)
 *
 * 发布者的写方法将接收指向此结构的指针。
 * (The publisher's write method will receive a pointer to this structure.)
 */
struct SerializedData {
  SerializedDataType type;  ///< 下一个字段的类型 (The type of the next field)
  void* data;        ///< 存储序列化数据的指针 (Pointer to store the serialized data)
  const void* impl;  ///< RMW 实现特定的数据 (RMW implementation specific data)
};

/**
 * @class TypeSupport
 * @brief 类 TypeSupport 是 eprosima::fastdds::dds::TopicDataType 的子类，用于支持 ROS2 中的
 * rmw_fastrtps_cpp 项目。 The TypeSupport class is a subclass of
 * eprosima::fastdds::dds::TopicDataType, used to support the rmw_fastrtps_cpp project in ROS2.
 */
class TypeSupport : public eprosima::fastdds::dds::TopicDataType {
public:
  /**
   * @brief 获取序列化后的消息大小估计值
   *        Get the estimated serialized size of the message.
   *
   * @param[in] ros_message 指向 ROS 消息的指针
   *                Pointer to the ROS message.
   * @param[in] impl 指向实现细节的指针（可选）
   *                Pointer to implementation details (optional).
   *
   * @return 返回序列化后的消息大小估计值
   *         Return the estimated serialized size of the message.
   */
  virtual size_t getEstimatedSerializedSize(const void* ros_message, const void* impl) const = 0;

  /**
   * @brief 序列化 ROS 消息 (Serialize ROS message)
   *
   * @param[in] ros_message 输入的 ROS 消息指针 (Input pointer to the ROS message)
   * @param[out] ser 输出的序列化对象 (Output serialized object)
   * @param[in] impl 类型支持实现的指针 (Pointer to the type support implementation)
   * @return 是否成功序列化 (Whether the serialization was successful)
   */
  virtual bool serializeROSmessage(
      const void* ros_message, eprosima::fastcdr::Cdr& ser, const void* impl) const = 0;

  /**
   * @brief 反序列化 ROS 消息 (Deserialize ROS message)
   *
   * @param[in] deser 输入的反序列化对象 (Input deserialized object)
   * @param[out] ros_message 输出的 ROS 消息指针 (Output pointer to the ROS message)
   * @param[in] impl 类型支持实现的指针 (Pointer to the type support implementation)
   * @return 是否成功反序列化 (Whether the deserialization was successful)
   */
  virtual bool deserializeROSmessage(
      eprosima::fastcdr::Cdr& deser, void* ros_message, const void* impl) const = 0;

  // 获取键值 (Get key value)
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  bool getKey(
      void* data,
      eprosima::fastrtps::rtps::InstanceHandle_t* ihandle,
      bool force_md5 = false) override {
    (void)data;       // 忽略数据参数 (Ignore data parameter)
    (void)ihandle;    // 忽略实例句柄参数 (Ignore instance handle parameter)
    (void)force_md5;  // 忽略强制 MD5 参数 (Ignore force MD5 parameter)
    return false;  // 返回 false，表示未实现此方法 (Return false, indicating this method is not
                   // implemented)
  }

  // 序列化数据 (Serialize data)
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  bool serialize(void* data, eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;

  // 反序列化数据 (Deserialize data)
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t* payload, void* data) override;

  // 获取序列化大小提供者 (Get serialized size provider)
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  std::function<uint32_t()> getSerializedSizeProvider(void* data) override;

  // 创建数据 (Create data)
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void* createData() override;

  // 删除数据 (Delete data)
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void deleteData(void* data) override;

  // 判断是否有边界 (Determine if there is a boundary)
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  inline bool is_bounded() const
#ifdef TOPIC_DATA_TYPE_API_HAS_IS_BOUNDED
      override
#endif
  {
    return max_size_bound_;  // 返回最大尺寸边界 (Return the maximum size boundary)
  }

  // 判断是否是简单类型 (Determine if it is a plain type)
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  inline bool is_plain() const
#ifdef TOPIC_DATA_TYPE_API_HAS_IS_PLAIN
      override
#endif
  {
    return is_plain_;  // 返回是否为简单类型 (Return whether it is a plain type)
  }

  // 析构函数 (Destructor)
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  virtual ~TypeSupport() {}

protected:
  /**
   * @brief 构造函数 (Constructor)
   *
   * @param[in] max_size_bound 最大尺寸限制 (Max size bound)
   * @param[in] is_plain 是否为简单类型 (Whether it's a plain type or not)
   */
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  TypeSupport(bool max_size_bound, bool is_plain);

  // 最大尺寸限制标志，如果为 true，则表示有最大尺寸限制；否则表示没有最大尺寸限制。
  // Max size bound flag, if true, it means there is a max size bound; otherwise, there is no max
  // size bound.
  bool max_size_bound_;

  // 是否为简单类型的标志，如果为 true，则表示是简单类型；否则表示不是简单类型。
  // Flag for whether it's a plain type or not, if true, it means it's a plain type; otherwise, it's
  // not a plain type.
  bool is_plain_;
};

/**
 * @brief 注册类型对象 (Register a type object)
 *
 * 此函数用于在 Fast RTPS 中注册给定类型的类型对象。这对于序列化和反序列化消息非常重要。
 * (This function is used to register the type object of a given type in Fast RTPS. This is
 * important for serializing and deserializing messages.)
 *
 * @param[in] type_supports 指向 ROSIDL 消息类型支持结构体的指针 (A pointer to the ROSIDL message
 * type support structure)
 * @param[in] type_name 类型名称，用于在 Fast RTPS 中唯一标识此类型 (The type name, used to uniquely
 * identify this type in Fast RTPS)
 * @return 如果成功注册类型对象，则返回 true；否则返回 false (Returns true if the type object was
 * successfully registered; otherwise, returns false)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
bool register_type_object(
    const rosidl_message_type_support_t*
        type_supports,  // 输入参数：指向 ROSIDL 消息类型支持结构体的指针 (Input parameter: A
                        // pointer to the ROSIDL message type support structure)
    const std::string&
        type_name);  // 输入参数：类型名称，用于在 Fast RTPS 中唯一标识此类型 (Input parameter: The
                     // type name, used to uniquely identify this type in Fast RTPS)

}  // namespace rmw_fastrtps_shared_cpp

#endif  // RMW_FASTRTPS_SHARED_CPP__TYPESUPPORT_HPP_
