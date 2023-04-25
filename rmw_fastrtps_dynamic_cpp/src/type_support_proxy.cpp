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

#include "rmw_fastrtps_dynamic_cpp/TypeSupport.hpp"

namespace rmw_fastrtps_dynamic_cpp {

/**
 * @brief 构造函数，用于初始化 TypeSupportProxy 对象 (Constructor for initializing the
 * TypeSupportProxy object)
 *
 * @param inner_type 内部类型支持对象的指针 (Pointer to the internal type support object)
 */
TypeSupportProxy::TypeSupportProxy(rmw_fastrtps_shared_cpp::TypeSupport *inner_type) {
  // 设置 TypeSupportProxy 对象的名称为内部类型支持对象的名称 (Set the name of the TypeSupportProxy
  // object to the name of the internal type support object)
  setName(inner_type->getName());

  // 设置 TypeSupportProxy 对象的类型大小为内部类型支持对象的类型大小 (Set the type size of the
  // TypeSupportProxy object to the type size of the internal type support object)
  m_typeSize = inner_type->m_typeSize;

  // 设置 TypeSupportProxy 对象的 is_plain_ 属性为内部类型支持对象的 is_plain() 方法返回值 (Set the
  // is_plain_ attribute of the TypeSupportProxy object to the return value of the is_plain() method
  // of the internal type support object)
  is_plain_ = inner_type->is_plain();

  // 设置 TypeSupportProxy 对象的 max_size_bound_ 属性为内部类型支持对象的 is_bounded() 方法返回值
  // (Set the max_size_bound_ attribute of the TypeSupportProxy object to the return value of the
  // is_bounded() method of the internal type support object)
  max_size_bound_ = inner_type->is_bounded();
}

/**
 * @brief 获取序列化后的估计大小 (Get the estimated serialized size)
 *
 * @param ros_message ROS 消息的指针 (Pointer to the ROS message)
 * @param impl 类型支持实现的指针 (Pointer to the type support implementation)
 * @return size_t 序列化后的估计大小 (Estimated serialized size)
 */
size_t TypeSupportProxy::getEstimatedSerializedSize(
    const void *ros_message, const void *impl) const {
  // 将 impl 转换为 rmw_fastrtps_shared_cpp::TypeSupport 类型的指针 (Cast impl to a pointer of type
  // rmw_fastrtps_shared_cpp::TypeSupport)
  auto type_impl = static_cast<const rmw_fastrtps_shared_cpp::TypeSupport *>(impl);

  // 调用类型实现的 getEstimatedSerializedSize 方法并返回结果 (Call the getEstimatedSerializedSize
  // method of the type implementation and return the result)
  return type_impl->getEstimatedSerializedSize(ros_message, impl);
}

/**
 * @brief 序列化 ROS 消息 (Serialize the ROS message)
 *
 * @param ros_message ROS 消息的指针 (Pointer to the ROS message)
 * @param ser 序列化器对象 (Serializer object)
 * @param impl 类型支持实现的指针 (Pointer to the type support implementation)
 * @return bool 序列化是否成功 (Whether the serialization is successful)
 */
bool TypeSupportProxy::serializeROSmessage(
    const void *ros_message, eprosima::fastcdr::Cdr &ser, const void *impl) const {
  // 将 impl 转换为 rmw_fastrtps_shared_cpp::TypeSupport 类型的指针 (Cast impl to a pointer of type
  // rmw_fastrtps_shared_cpp::TypeSupport)
  auto type_impl = static_cast<const rmw_fastrtps_shared_cpp::TypeSupport *>(impl);

  // 调用类型实现的 serializeROSmessage 方法并返回结果 (Call the serializeROSmessage method of the
  // type implementation and return the result)
  return type_impl->serializeROSmessage(ros_message, ser, impl);
}

/**
 * @brief 反序列化 ROS 消息 (Deserialize the ROS message)
 *
 * @param deser 反序列化器对象 (Deserializer object)
 * @param ros_message ROS 消息的指针 (Pointer to the ROS message)
 * @param impl 类型支持实现的指针 (Pointer to the type support implementation)
 * @return bool 反序列化是否成功 (Whether the deserialization is successful)
 */
bool TypeSupportProxy::deserializeROSmessage(
    eprosima::fastcdr::Cdr &deser, void *ros_message, const void *impl) const {
  // 将 impl 转换为 rmw_fastrtps_shared_cpp::TypeSupport 类型的指针 (Cast impl to a pointer of type
  // rmw_fastrtps_shared_cpp::TypeSupport)
  auto type_impl = static_cast<const rmw_fastrtps_shared_cpp::TypeSupport *>(impl);

  // 调用类型实现的 deserializeROSmessage 方法并返回结果 (Call the deserializeROSmessage method of
  // the type implementation and return the result)
  return type_impl->deserializeROSmessage(deser, ros_message, impl);
}

}  // namespace rmw_fastrtps_dynamic_cpp
