// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#ifndef RMW_FASTRTPS_SHARED_CPP__QOS_HPP_
#define RMW_FASTRTPS_SHARED_CPP__QOS_HPP_

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>

#include "fastrtps/qos/QosPolicies.h"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/visibility_control.h"
#include "rosidl_runtime_c/type_hash.h"

/**
 * @brief 检查给定的 QoS 策略是否有效 (Check if the given QoS policies are valid)
 *
 * @param[in] qos_policies 要检查的 QoS 策略 (The QoS policies to check)
 * @return 如果 QoS 策略有效，则返回 true，否则返回 false (Returns true if the QoS policies are
 * valid, false otherwise)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
bool is_valid_qos(const rmw_qos_profile_t& qos_policies);

/**
 * @brief 获取 DataReader 的 QoS 设置 (Get the QoS settings for a DataReader)
 *
 * @param[in] qos_policies ROS 2 QoS 策略 (ROS 2 QoS policies)
 * @param[in] type_hash 数据类型哈希值 (Data type hash value)
 * @param[out] reader_qos 用于存储 DataReader QoS 设置的对象 (Object to store the DataReader QoS
 * settings)
 * @return 如果成功获取 QoS 设置，则返回 true，否则返回 false (Returns true if the QoS settings were
 * successfully obtained, false otherwise)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
bool get_datareader_qos(
    const rmw_qos_profile_t& qos_policies,
    const rosidl_type_hash_t& type_hash,
    eprosima::fastdds::dds::DataReaderQos& reader_qos);

/**
 * @brief 获取 DataWriter 的 QoS 设置 (Get the QoS settings for a DataWriter)
 *
 * @param[in] qos_policies ROS 2 QoS 策略 (ROS 2 QoS policies)
 * @param[in] type_hash 数据类型哈希值 (Data type hash value)
 * @param[out] writer_qos 用于存储 DataWriter QoS 设置的对象 (Object to store the DataWriter QoS
 * settings)
 * @return 如果成功获取 QoS 设置，则返回 true，否则返回 false (Returns true if the QoS settings were
 * successfully obtained, false otherwise)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
bool get_datawriter_qos(
    const rmw_qos_profile_t& qos_policies,
    const rosidl_type_hash_t& type_hash,
    eprosima::fastdds::dds::DataWriterQos& writer_qos);

/**
 * @brief 获取 Topic 的 QoS 设置 (Get the QoS settings for a Topic)
 *
 * @param[in] qos_policies ROS 2 QoS 策略 (ROS 2 QoS policies)
 * @param[out] topic_qos 用于存储 Topic QoS 设置的对象 (Object to store the Topic QoS settings)
 * @return 如果成功获取 QoS 设置，则返回 true，否则返回 false (Returns true if the QoS settings were
 * successfully obtained, false otherwise)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
bool get_topic_qos(
    const rmw_qos_profile_t& qos_policies, eprosima::fastdds::dds::TopicQos& topic_qos);

/**
 * @brief 将 Fast RTPS Duration_t 转换为 RMW 时间 (Convert Fast RTPS Duration_t to RMW time)
 *
 * @param[in] duration Fast RTPS Duration_t 对象 (Fast RTPS Duration_t object)
 * @return 转换后的 RMW 时间 (Converted RMW time)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_time_t dds_duration_to_rmw(const eprosima::fastrtps::Duration_t& duration);

/**
 * @brief 将DDS QOS策略（类型为DataWriterQos或DataReaderQos）转换为rmw_qos_profile_t。
 *        Converts the DDS QOS Policy; of type DataWriterQos or DataReaderQos into
 * rmw_qos_profile_t.
 *
 * @tparam DDSQoSPolicyT DDS QoS策略类型，可以是DataWriterQos或DataReaderQos。
 *                       DDS QoS policy type, can be either DataWriterQos or DataReaderQos.
 * @param[in] dds_qos 类型为DataWriterQos或DataReaderQos的DDS QoS策略。
 *                   DDS QoS policy of type DataWriterQos or DataReaderQos.
 * @param[out] qos 等效于dds_qos数据的rmw_qos_profile_t。
 *                The equivalent of the data in dds_qos as a rmw_qos_profile_t.
 */
template <typename DDSQoSPolicyT>
void dds_qos_to_rmw_qos(const DDSQoSPolicyT& dds_qos, rmw_qos_profile_t* qos) {
  // 根据DDS QoS的可靠性类型设置对应的RMW QoS可靠性策略
  // Set the corresponding RMW QoS reliability policy based on the DDS QoS reliability kind
  switch (dds_qos.reliability().kind) {
    case eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS:
      qos->reliability = RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT;
      break;
    case eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS:
      qos->reliability = RMW_QOS_POLICY_RELIABILITY_RELIABLE;
      break;
    default:
      qos->reliability = RMW_QOS_POLICY_RELIABILITY_UNKNOWN;
      break;
  }

  // 根据DDS QoS的持久性类型设置对应的RMW QoS持久性策略
  // Set the corresponding RMW QoS durability policy based on the DDS QoS durability kind
  switch (dds_qos.durability().kind) {
    case eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS:
      qos->durability = RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL;
      break;
    case eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS:
      qos->durability = RMW_QOS_POLICY_DURABILITY_VOLATILE;
      break;
    default:
      qos->durability = RMW_QOS_POLICY_DURABILITY_UNKNOWN;
      break;
  }

  // 将DDS QoS的deadline转换为RMW QoS的deadline
  // Convert the DDS QoS deadline to the RMW QoS deadline
  qos->deadline = dds_duration_to_rmw(dds_qos.deadline().period);

  // 将DDS QoS的lifespan转换为RMW QoS的lifespan
  // Convert the DDS QoS lifespan to the RMW QoS lifespan
  qos->lifespan = dds_duration_to_rmw(dds_qos.lifespan().duration);

  // 根据DDS QoS的活跃度类型设置对应的RMW QoS活跃度策略
  // Set the corresponding RMW QoS liveliness policy based on the DDS QoS liveliness kind
  switch (dds_qos.liveliness().kind) {
    case eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS:
      qos->liveliness = RMW_QOS_POLICY_LIVELINESS_AUTOMATIC;
      break;
    case eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS:
      qos->liveliness = RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_TOPIC;
      break;
    default:
      qos->liveliness = RMW_QOS_POLICY_LIVELINESS_UNKNOWN;
      break;
  }

  // 将DDS QoS的活跃度租期转换为RMW QoS的活跃度租期
  // Convert the DDS QoS liveliness lease duration to the RMW QoS liveliness lease duration
  qos->liveliness_lease_duration = dds_duration_to_rmw(dds_qos.liveliness().lease_duration);

  // 根据DDS QoS的历史类型设置对应的RMW QoS历史策略
  // Set the corresponding RMW QoS history policy based on the DDS QoS history kind
  switch (dds_qos.history().kind) {
    case eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS:
      qos->history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;
      break;
    case eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS:
      qos->history = RMW_QOS_POLICY_HISTORY_KEEP_ALL;
      break;
    default:
      qos->history = RMW_QOS_POLICY_HISTORY_UNKNOWN;
      break;
  }

  // 将DDS QoS的历史深度转换为RMW QoS的历史深度
  // Convert the DDS QoS history depth to the RMW QoS history depth
  qos->depth = static_cast<size_t>(dds_qos.history().depth);
}

/**
 * \brief 将 RTPS QOS 策略（类型为 WriterQos 或 ReaderQos）转换为 rmw_qos_profile_t。
 *        由于 WriterQos 或 ReaderQos 没有关于历史和深度的信息，所以这些值不会被此函数设置。
 *        Convert the RTPS QOS Policy (of type WriterQos or ReaderQos) into rmw_qos_profile_t.
 *        Since WriterQos or ReaderQos do not have information about history and depth,
 *        these values are not set by this function.
 *
 * \param[in] rtps_qos 类型为 WriterQos 或 ReaderQos 的 RTPS QOS 策略
 *            rtps_qos of type WriterQos or ReaderQos
 * \param[out] qos 等效于 rtps_qos 中的数据作为 rmw_qos_profile_t
 *             qos the equivalent of the data in rtps_qos as a rmw_qos_profile_t
 */
template <typename RTPSQoSPolicyT>
void rtps_qos_to_rmw_qos(const RTPSQoSPolicyT& rtps_qos, rmw_qos_profile_t* qos) {
  // 根据 rtps_qos.m_reliability.kind 设置 qos->reliability
  // Set qos->reliability based on rtps_qos.m_reliability.kind
  switch (rtps_qos.m_reliability.kind) {
    case eprosima::fastrtps::BEST_EFFORT_RELIABILITY_QOS:
      qos->reliability = RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT;
      break;
    case eprosima::fastrtps::RELIABLE_RELIABILITY_QOS:
      qos->reliability = RMW_QOS_POLICY_RELIABILITY_RELIABLE;
      break;
    default:
      qos->reliability = RMW_QOS_POLICY_RELIABILITY_UNKNOWN;
      break;
  }

  // 根据 rtps_qos.m_durability.kind 设置 qos->durability
  // Set qos->durability based on rtps_qos.m_durability.kind
  switch (rtps_qos.m_durability.kind) {
    case eprosima::fastrtps::TRANSIENT_LOCAL_DURABILITY_QOS:
      qos->durability = RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL;
      break;
    case eprosima::fastrtps::VOLATILE_DURABILITY_QOS:
      qos->durability = RMW_QOS_POLICY_DURABILITY_VOLATILE;
      break;
    default:
      qos->durability = RMW_QOS_POLICY_DURABILITY_UNKNOWN;
      break;
  }

  // 将 rtps_qos.m_deadline.period 转换为 rmw 时间并设置 qos->deadline
  // Convert rtps_qos.m_deadline.period to rmw time and set qos->deadline
  qos->deadline = dds_duration_to_rmw(rtps_qos.m_deadline.period);

  // 将 rtps_qos.m_lifespan.duration 转换为 rmw 时间并设置 qos->lifespan
  // Convert rtps_qos.m_lifespan.duration to rmw time and set qos->lifespan
  qos->lifespan = dds_duration_to_rmw(rtps_qos.m_lifespan.duration);

  // 根据 rtps_qos.m_liveliness.kind 设置 qos->liveliness
  // Set qos->liveliness based on rtps_qos.m_liveliness.kind
  switch (rtps_qos.m_liveliness.kind) {
    case eprosima::fastrtps::AUTOMATIC_LIVELINESS_QOS:
      qos->liveliness = RMW_QOS_POLICY_LIVELINESS_AUTOMATIC;
      break;
    case eprosima::fastrtps::MANUAL_BY_TOPIC_LIVELINESS_QOS:
      qos->liveliness = RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_TOPIC;
      break;
    default:
      qos->liveliness = RMW_QOS_POLICY_LIVELINESS_UNKNOWN;
      break;
  }

  // 将 rtps_qos.m_liveliness.lease_duration 转换为 rmw 时间并设置 qos->liveliness_lease_duration
  // Convert rtps_qos.m_liveliness.lease_duration to rmw time and set qos->liveliness_lease_duration
  qos->liveliness_lease_duration = dds_duration_to_rmw(rtps_qos.m_liveliness.lease_duration);
}

// 声明一个模板函数，用于将 eprosima::fastdds::dds::DataWriterQos 类型的 DDS QoS 转换为 RMW QoS。
// Declare a template function for converting DDS QoS of type eprosima::fastdds::dds::DataWriterQos
// to RMW QoS.
extern template RMW_FASTRTPS_SHARED_CPP_PUBLIC void
dds_qos_to_rmw_qos<eprosima::fastdds::dds::DataWriterQos>(
    const eprosima::fastdds::dds::DataWriterQos&
        dds_qos,              // 输入参数：DDS QoS 对象（类型为 DataWriterQos）
                              // Input parameter: DDS QoS object (type DataWriterQos)
    rmw_qos_profile_t* qos);  // 输出参数：指向 RMW QoS 对象的指针
                              // Output parameter: Pointer to the RMW QoS object

// 声明一个模板函数，用于将 eprosima::fastdds::dds::DataReaderQos 类型的 DDS QoS 转换为 RMW QoS。
// Declare a template function for converting DDS QoS of type eprosima::fastdds::dds::DataReaderQos
// to RMW QoS.
extern template RMW_FASTRTPS_SHARED_CPP_PUBLIC void
dds_qos_to_rmw_qos<eprosima::fastdds::dds::DataReaderQos>(
    const eprosima::fastdds::dds::DataReaderQos&
        dds_qos,              // 输入参数：DDS QoS 对象（类型为 DataReaderQos）
                              // Input parameter: DDS QoS object (type DataReaderQos)
    rmw_qos_profile_t* qos);  // 输出参数：指向 RMW QoS 对象的指针
                              // Output parameter: Pointer to the RMW QoS object

// 声明一个模板函数，用于将 DDS 属性转换为 RMW QoS。
// Declare a template function for converting DDS attributes to RMW QoS.
template <typename AttributeT>
void dds_attributes_to_rmw_qos(
    const AttributeT& dds_qos,  // 输入参数：DDS 属性对象（类型为 AttributeT）
                                // Input parameter: DDS attributes object (type AttributeT)
    rmw_qos_profile_t* qos);    // 输出参数：指向 RMW QoS 对象的指针
                                // Output parameter: Pointer to the RMW QoS object

// 声明一个模板函数，用于将 eprosima::fastrtps::PublisherAttributes 类型的 DDS 属性转换为 RMW QoS。
// Declare a template function for converting DDS attributes of type
// eprosima::fastrtps::PublisherAttributes to RMW QoS.
extern template RMW_FASTRTPS_SHARED_CPP_PUBLIC void
dds_attributes_to_rmw_qos<eprosima::fastrtps::PublisherAttributes>(
    const eprosima::fastrtps::PublisherAttributes&
        dds_qos,              // 输入参数：DDS 属性对象（类型为 PublisherAttributes）
                              // Input parameter: DDS attributes object (type PublisherAttributes)
    rmw_qos_profile_t* qos);  // 输出参数：指向 RMW QoS 对象的指针
                              // Output parameter: Pointer to the RMW QoS object

// 声明一个模板函数，用于将 eprosima::fastrtps::SubscriberAttributes 类型的 DDS 属性转换为 RMW QoS。
// Declare a template function for converting DDS attributes of type
// eprosima::fastrtps::SubscriberAttributes to RMW QoS.
extern template RMW_FASTRTPS_SHARED_CPP_PUBLIC void
dds_attributes_to_rmw_qos<eprosima::fastrtps::SubscriberAttributes>(
    const eprosima::fastrtps::SubscriberAttributes&
        dds_qos,              // 输入参数：DDS 属性对象（类型为 SubscriberAttributes）
                              // Input parameter: DDS attributes object (type SubscriberAttributes)
    rmw_qos_profile_t* qos);  // 输出参数：指向 RMW QoS 对象的指针
                              // Output parameter: Pointer to the RMW QoS object

#endif                        // RMW_FASTRTPS_SHARED_CPP__QOS_HPP_
