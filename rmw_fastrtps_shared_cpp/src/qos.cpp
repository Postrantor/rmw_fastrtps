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

#include "rmw_fastrtps_shared_cpp/qos.hpp"

#include <limits>
#include <vector>

#include "fastdds/dds/publisher/qos/DataWriterQos.hpp"
#include "fastdds/dds/subscriber/qos/DataReaderQos.hpp"
#include "fastdds/dds/topic/qos/TopicQos.hpp"
#include "rcutils/logging_macros.h"
#include "rmw/error_handling.h"
#include "rmw_dds_common/qos.hpp"
#include "rosidl_runtime_c/type_hash.h"
#include "time_utils.hpp"

/**
 * @brief 判断给定的 rmw_time_t 是否为未指定的持续时间 (Determine if the given rmw_time_t is an
 * unspecified duration)
 *
 * @param time 一个 rmw_time_t 类型的时间变量 (A rmw_time_t type time variable)
 * @return 如果时间等于 RMW_DURATION_UNSPECIFIED，则返回 true，否则返回 false (Returns true if the
 * time is equal to RMW_DURATION_UNSPECIFIED, otherwise returns false)
 */
static bool is_rmw_duration_unspecified(const rmw_time_t& time) {
  // 比较给定的时间与未指定的持续时间常量 (Compare the given time with the unspecified duration
  // constant)
  return rmw_time_equal(time, RMW_DURATION_UNSPECIFIED);
}

/**
 * @brief 将 eprosima::fastrtps::Duration_t 类型的持续时间转换为 rmw_time_t 类型 (Convert
 * eprosima::fastrtps::Duration_t type duration to rmw_time_t type)
 *
 * @param duration 一个 eprosima::fastrtps::Duration_t 类型的持续时间变量 (An
 * eprosima::fastrtps::Duration_t type duration variable)
 * @return 转换后的 rmw_time_t 类型的持续时间 (The converted rmw_time_t type duration)
 */
rmw_time_t dds_duration_to_rmw(const eprosima::fastrtps::Duration_t& duration) {
  // 如果给定的持续时间等于无穷大持续时间常量 (If the given duration is equal to the infinite
  // duration constant)
  if (duration == eprosima::fastrtps::rtps::c_RTPSTimeInfinite) {
    // 返回无穷大持续时间常量 (Return the infinite duration constant)
    return RMW_DURATION_INFINITE;
  }
  // 创建一个 rmw_time_t 结构体并用给定的持续时间初始化它 (Create an rmw_time_t struct and
  // initialize it with the given duration)
  rmw_time_t result = {(uint64_t)duration.seconds, (uint64_t)duration.nanosec};
  // 返回转换后的 rmw_time_t 类型的持续时间 (Return the converted rmw_time_t type duration)
  return result;
}

/**
 * @brief 从 RMW QoS 配置文件中填充实体的 DDS QoS。
 *        Fill the entity's DDS QoS from the RMW QoS profile.
 *
 * @tparam DDSEntityQos DDS 实体 QoS 类型。DDS Entity QoS type.
 * @param[in] qos_policies RMW QoS 配置文件。RMW QoS profile.
 * @param[out] entity_qos 要填充的 DDS 实体 QoS。The DDS Entity QoS to be filled.
 * @return 如果成功填充实体 QoS，则返回 true，否则返回 false。
 *         Return true if the entity QoS is successfully filled, otherwise return false.
 */
template <typename DDSEntityQos>
bool fill_entity_qos_from_profile(const rmw_qos_profile_t& qos_policies, DDSEntityQos& entity_qos) {
  // 根据历史记录策略设置实体 QoS 的历史记录类型
  // Set the history kind of the entity QoS based on the history policy
  switch (qos_policies.history) {
    case RMW_QOS_POLICY_HISTORY_KEEP_LAST:
      entity_qos.history().kind = eprosima::fastdds::dds::KEEP_LAST_HISTORY_QOS;
      break;
    case RMW_QOS_POLICY_HISTORY_KEEP_ALL:
      entity_qos.history().kind = eprosima::fastdds::dds::KEEP_ALL_HISTORY_QOS;
      break;
    case RMW_QOS_POLICY_HISTORY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS history policy");
      return false;
  }

  // 根据持久性策略设置实体 QoS 的持久性类型
  // Set the durability kind of the entity QoS based on the durability policy
  switch (qos_policies.durability) {
    case RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL:
      entity_qos.durability().kind = eprosima::fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS;
      break;
    case RMW_QOS_POLICY_DURABILITY_VOLATILE:
      entity_qos.durability().kind = eprosima::fastdds::dds::VOLATILE_DURABILITY_QOS;
      break;
    case RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS durability policy");
      return false;
  }

  // 根据可靠性策略设置实体 QoS 的可靠性类型
  // Set the reliability kind of the entity QoS based on the reliability policy
  switch (qos_policies.reliability) {
    case RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT:
      entity_qos.reliability().kind = eprosima::fastdds::dds::BEST_EFFORT_RELIABILITY_QOS;
      break;
    case RMW_QOS_POLICY_RELIABILITY_RELIABLE:
      entity_qos.reliability().kind = eprosima::fastdds::dds::RELIABLE_RELIABILITY_QOS;
      break;
    case RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS reliability policy");
      return false;
  }

  // 确保历史深度至少与请求的队列大小一致
  // Ensure the history depth is at least the requested queue size
  assert(entity_qos.history().depth >= 0);
  if (qos_policies.depth != RMW_QOS_POLICY_DEPTH_SYSTEM_DEFAULT &&
      static_cast<size_t>(entity_qos.history().depth) < qos_policies.depth) {
    if (qos_policies.depth > static_cast<size_t>((std::numeric_limits<int32_t>::max)())) {
      RMW_SET_ERROR_MSG(
          "failed to set history depth since the requested queue size exceeds the DDS type");
      return false;
    }
    entity_qos.history().depth = static_cast<int32_t>(qos_policies.depth);
  }

  // 如果生命周期不是未指定的，则设置实体 QoS 的生命周期
  // Set the entity QoS lifespan if it is not unspecified
  if (!is_rmw_duration_unspecified(qos_policies.lifespan)) {
    entity_qos.lifespan().duration =
        rmw_fastrtps_shared_cpp::rmw_time_to_fastrtps(qos_policies.lifespan);
  }

  // 如果截止时间不是未指定的，则设置实体 QoS 的截止时间
  // Set the entity QoS deadline if it is not unspecified
  if (!is_rmw_duration_unspecified(qos_policies.deadline)) {
    entity_qos.deadline().period =
        rmw_fastrtps_shared_cpp::rmw_time_to_fastrtps(qos_policies.deadline);
  }

  // 根据活跃度策略设置实体 QoS 的活跃度类型
  // Set the liveliness kind of the entity QoS based on the liveliness policy
  switch (qos_policies.liveliness) {
    case RMW_QOS_POLICY_LIVELINESS_AUTOMATIC:
      entity_qos.liveliness().kind = eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS;
      break;
    case RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_TOPIC:
      entity_qos.liveliness().kind = eprosima::fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS;
      break;
    case RMW_QOS_POLICY_LIVELINESS_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS Liveliness policy");
      return false;
  }

  // 如果活跃度租期持续时间不是未指定的，则设置实体 QoS 的活跃度租期持续时间
  // Set the entity QoS liveliness lease duration if it is not unspecified
  if (!is_rmw_duration_unspecified(qos_policies.liveliness_lease_duration)) {
    entity_qos.liveliness().lease_duration =
        rmw_fastrtps_shared_cpp::rmw_time_to_fastrtps(qos_policies.liveliness_lease_duration);

    // 文档建议将其设置为不高于 0.7 * lease_duration，选择 2/3 以提供安全缓冲。
    // 参见文档：https://github.com/eProsima/Fast-RTPS/blob/
    //   a8691a40be6b8460b01edde36ad8563170a3a35a/include/fastrtps/qos/QosPolicies.h#L223-L232
    double period_in_ns = entity_qos.liveliness().lease_duration.to_ns() * 2.0 / 3.0;
    double period_in_s = RCUTILS_NS_TO_S(period_in_ns);
    entity_qos.liveliness().announcement_period = eprosima::fastrtps::Duration_t(period_in_s);
  }

  return true;
}

/**
 * @brief 从 QoS 配置文件中填充数据实体 QoS (Fill data entity QoS from QoS profile)
 *
 * @tparam DDSEntityQos DDS 实体 QoS 类型 (DDS entity QoS type)
 * @param[in] qos_policies QoS 策略配置文件 (QoS policies profile)
 * @param[in] type_hash 数据类型哈希值 (Data type hash value)
 * @param[out] entity_qos 填充后的 DDS 实体 QoS 对象 (Filled DDS entity QoS object)
 * @return 成功返回 true，失败返回 false (Return true if successful, false otherwise)
 */
template <typename DDSEntityQos>
bool fill_data_entity_qos_from_profile(
    const rmw_qos_profile_t& qos_policies,
    const rosidl_type_hash_t& type_hash,
    DDSEntityQos& entity_qos) {
  // 尝试从 QoS 配置文件中填充实体 QoS (Try to fill entity QoS from QoS profile)
  if (!fill_entity_qos_from_profile(qos_policies, entity_qos)) {
    return false;
  }
  std::string user_data_str;
  // 编码类型哈希以用于用户数据 QoS (Encode type hash for user data QoS)
  if (RMW_RET_OK != rmw_dds_common::encode_type_hash_for_user_data_qos(type_hash, user_data_str)) {
    // 如果编码失败，记录警告并清除用户数据字符串 (If encoding fails, log a warning and clear the
    // user data string)
    RCUTILS_LOG_WARN_NAMED(
        "rmw_fastrtps_shared_cpp",
        "Failed to encode type hash for topic, will not distribute it in USER_DATA.");
    user_data_str.clear();
  }
  // 将用户数据字符串转换为字节向量 (Convert the user data string to a byte vector)
  std::vector<uint8_t> user_data(user_data_str.begin(), user_data_str.end());
  // 调整实体 QoS 的用户数据大小并设置值 (Resize the entity QoS user data and set its value)
  entity_qos.user_data().resize(user_data.size());
  entity_qos.user_data().setValue(user_data);
  return true;
}

/**
 * @brief 获取 DataReader QoS (Get DataReader QoS)
 *
 * @param[in] qos_policies QoS 策略配置文件 (QoS policies profile)
 * @param[in] type_hash 数据类型哈希值 (Data type hash value)
 * @param[out] datareader_qos 填充后的 DataReader QoS 对象 (Filled DataReader QoS object)
 * @return 成功返回 true，失败返回 false (Return true if successful, false otherwise)
 */
bool get_datareader_qos(
    const rmw_qos_profile_t& qos_policies,
    const rosidl_type_hash_t& type_hash,
    eprosima::fastdds::dds::DataReaderQos& datareader_qos) {
  return fill_data_entity_qos_from_profile(qos_policies, type_hash, datareader_qos);
}

/**
 * @brief 获取 DataWriter QoS (Get DataWriter QoS)
 *
 * @param[in] qos_policies QoS 策略配置文件 (QoS policies profile)
 * @param[in] type_hash 数据类型哈希值 (Data type hash value)
 * @param[out] datawriter_qos 填充后的 DataWriter QoS 对象 (Filled DataWriter QoS object)
 * @return 成功返回 true，失败返回 false (Return true if successful, false otherwise)
 */
bool get_datawriter_qos(
    const rmw_qos_profile_t& qos_policies,
    const rosidl_type_hash_t& type_hash,
    eprosima::fastdds::dds::DataWriterQos& datawriter_qos) {
  return fill_data_entity_qos_from_profile(qos_policies, type_hash, datawriter_qos);
}

/**
 * @brief 获取 Topic QoS (Get Topic QoS)
 *
 * @param[in] qos_policies QoS 策略配置文件 (QoS policies profile)
 * @param[out] topic_qos 填充后的 Topic QoS 对象 (Filled Topic QoS object)
 * @return 成功返回 true，失败返回 false (Return true if successful, false otherwise)
 */
bool get_topic_qos(
    const rmw_qos_profile_t& qos_policies, eprosima::fastdds::dds::TopicQos& topic_qos) {
  return fill_entity_qos_from_profile(qos_policies, topic_qos);
}

/**
 * @brief 判断 QoS 配置是否有效 (Check if the QoS configuration is valid)
 *
 * @param[in] qos_policies QoS 配置 (QoS configuration)
 * @return bool 返回 true，表示 QoS 配置有效 (Return true, indicating that the QoS configuration is
 * valid)
 */
bool is_valid_qos(const rmw_qos_profile_t& /* qos_policies */) { return true; }

/**
 * @brief 将 DDS 属性转换为 RMW QoS 配置 (Convert DDS attributes to RMW QoS configuration)
 *
 * @tparam AttributeT DDS 属性类型 (DDS attribute type)
 * @param[in] dds_qos DDS 属性 (DDS attributes)
 * @param[out] qos RMW QoS 配置 (RMW QoS configuration)
 */
template <typename AttributeT>
void dds_attributes_to_rmw_qos(const AttributeT& dds_qos, rmw_qos_profile_t* qos) {
  // 根据 DDS QoS 历史配置设置 RMW QoS 历史配置 (Set RMW QoS history configuration based on DDS QoS
  // history configuration)
  switch (dds_qos.topic.historyQos.kind) {
    case eprosima::fastrtps::KEEP_LAST_HISTORY_QOS:
      qos->history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;
      break;
    case eprosima::fastrtps::KEEP_ALL_HISTORY_QOS:
      qos->history = RMW_QOS_POLICY_HISTORY_KEEP_ALL;
      break;
    default:
      qos->history = RMW_QOS_POLICY_HISTORY_UNKNOWN;
      break;
  }
  // 设置 RMW QoS 深度配置 (Set RMW QoS depth configuration)
  qos->depth = static_cast<size_t>(dds_qos.topic.historyQos.depth);
  // 转换 RTPS QoS 配置为 RMW QoS 配置 (Convert RTPS QoS configuration to RMW QoS configuration)
  rtps_qos_to_rmw_qos(dds_qos.qos, qos);
}

// 显式实例化模板函数 (Explicit instantiation of template functions)
template void dds_attributes_to_rmw_qos<eprosima::fastrtps::PublisherAttributes>(
    const eprosima::fastrtps::PublisherAttributes& dds_qos, rmw_qos_profile_t* qos);

template void dds_attributes_to_rmw_qos<eprosima::fastrtps::SubscriberAttributes>(
    const eprosima::fastrtps::SubscriberAttributes& dds_qos, rmw_qos_profile_t* qos);

template void dds_qos_to_rmw_qos<eprosima::fastdds::dds::DataWriterQos>(
    const eprosima::fastdds::dds::DataWriterQos& dds_qos, rmw_qos_profile_t* qos);

template void dds_qos_to_rmw_qos<eprosima::fastdds::dds::DataReaderQos>(
    const eprosima::fastdds::dds::DataReaderQos& dds_qos, rmw_qos_profile_t* qos);
