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

#ifndef RMW_FASTRTPS_SHARED_CPP__CUSTOM_PARTICIPANT_INFO_HPP_
#define RMW_FASTRTPS_SHARED_CPP__CUSTOM_PARTICIPANT_INFO_HPP_

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "fastdds/dds/domain/DomainParticipant.hpp"
#include "fastdds/dds/domain/DomainParticipantListener.hpp"
#include "fastdds/dds/publisher/Publisher.hpp"
#include "fastdds/dds/subscriber/Subscriber.hpp"
#include "fastdds/rtps/participant/ParticipantDiscoveryInfo.h"
#include "fastdds/rtps/reader/ReaderDiscoveryInfo.h"
#include "fastdds/rtps/writer/WriterDiscoveryInfo.h"
#include "rcpputils/thread_safety_annotations.hpp"
#include "rcutils/logging_macros.h"
#include "rmw/impl/cpp/key_value.hpp"
#include "rmw/qos_profiles.h"
#include "rmw/rmw.h"
#include "rmw_dds_common/context.hpp"
#include "rmw_dds_common/qos.hpp"
#include "rmw_fastrtps_shared_cpp/create_rmw_gid.hpp"
#include "rmw_fastrtps_shared_cpp/custom_event_info.hpp"
#include "rmw_fastrtps_shared_cpp/qos.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

// 使用 rmw_dds_common::operator<<;
// Use rmw_dds_common::operator<<;
using rmw_dds_common::operator<<;

// 声明 ParticipantListener 类
// Declare the ParticipantListener class
class ParticipantListener;

// 定义 publishing_mode_t 枚举类，表示发布模式
// Define the publishing_mode_t enum class, representing the publishing mode
enum class publishing_mode_t {
  ASYNCHRONOUS,  // 异步发布模式
                 // Asynchronous publishing mode

  SYNCHRONOUS,   // 同步发布模式
                 // Synchronous publishing mode

  AUTO           // 使用 XML 文件中设置的发布模式或 Fast DDS 默认值
                 // Use publishing mode set in XML file or Fast DDS default
};

// 自定义 CustomTopicListener 类，继承自 eprosima::fastdds::dds::TopicListener
// Define the CustomTopicListener class, inheriting from eprosima::fastdds::dds::TopicListener
class CustomTopicListener final : public eprosima::fastdds::dds::TopicListener {
public:
  // 显示构造函数，接收一个 EventListenerInterface 指针参数
  // Explicit constructor, takes an EventListenerInterface pointer as a parameter
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  explicit CustomTopicListener(EventListenerInterface *event_listener);

  // 重写 on_inconsistent_topic 方法
  // Override the on_inconsistent_topic method
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void on_inconsistent_topic(
      eprosima::fastdds::dds::Topic *topic,
      eprosima::fastdds::dds::InconsistentTopicStatus status) override;

  // 添加事件监听器方法
  // Add event listener method
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void add_event_listener(EventListenerInterface *event_listener);

  // 移除事件监听器方法
  // Remove event listener method
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void remove_event_listener(EventListenerInterface *event_listener);

private:
  // 定义互斥锁，保护 event_listeners_ 集合
  // Define a mutex to protect the event_listeners_ set
  std::mutex event_listeners_mutex_;

  // 定义事件监听器集合，使用 RCPPUTILS_TSA_GUARDED_BY 宏确保线程安全
  // Define the event listeners set, using the RCPPUTILS_TSA_GUARDED_BY macro to ensure thread
  // safety
  std::set<EventListenerInterface *> event_listeners_
      RCPPUTILS_TSA_GUARDED_BY(event_listeners_mutex_);
};

// 定义 UseCountTopic 结构体，包含一个 eprosima::fastdds::dds::Topic 指针、一个 CustomTopicListener
// 指针和一个 use_count 计数 Define the UseCountTopic struct, containing an
// eprosima::fastdds::dds::Topic pointer, a CustomTopicListener pointer, and a use_count count
typedef struct UseCountTopic {
  eprosima::fastdds::dds::Topic *topic{nullptr};  // Topic 指针，默认为 nullptr
                                                  // Topic pointer, default to nullptr

  CustomTopicListener *topic_listener{nullptr};   // CustomTopicListener 指针，默认为 nullptr
                                                  // CustomTopicListener pointer, default to nullptr

  size_t use_count{0};                            // 使用计数，默认为 0
                                                  // Use count, default to 0
} UseCountTopic;

/**
 * @brief 自定义参与者信息结构体 (Custom participant information structure)
 *
 */
typedef struct CustomParticipantInfo {
  // 域参与者指针 (DomainParticipant pointer)
  eprosima::fastdds::dds::DomainParticipant *participant_{nullptr};
  // 参与者监听器指针 (ParticipantListener pointer)
  ParticipantListener *listener_{nullptr};

  // 保护 topic_name_to_topic_ 的互斥锁 (Mutex to protect topic_name_to_topic_)
  std::mutex topic_name_to_topic_mutex_;
  // 截至 2023-02-07，Fast-DDS 每个 DomainParticipant 只允许使用相同的主题名创建一个
  // create_topic()。 因此，我们需要检查该主题是否已经创建。如果已创建，则增加我们跟踪的
  // use_count，并返回现有主题。
  // 如果没有创建，则创建一个新主题并开始跟踪它。一旦删除了所有主题的用户，我们将删除该主题。
  // (As of 2023-02-07, Fast-DDS only allows one create_topic() with the same
  // topic name per DomainParticipant. Thus, we need to check if the topic
  // was already created. If it did, then we just increase the use_count
  // that we are tracking, and return the existing topic. If it
  // didn't, then we create a new one and start tracking it. Once all
  // users of the topic are removed, we will delete the topic.)
  std::map<std::string, std::unique_ptr<UseCountTopic>> topic_name_to_topic_;

  // 发布者指针 (Publisher pointer)
  eprosima::fastdds::dds::Publisher *publisher_{nullptr};
  // 订阅者指针 (Subscriber pointer)
  eprosima::fastdds::dds::Subscriber *subscriber_{nullptr};

  // 保护主题、读取器和写入器的创建和销毁的互斥锁 (Mutex to protect creation and destruction of
  // topics, readers and writers)
  mutable std::mutex entity_creation_mutex_;

  // 标志，用于确定 DomainParticipant 的 QoS、其 DataWriters 和 DataReaders 的 QoS
  // 是否仅从 XML 文件中配置，还是通过代码使用默认配置覆盖它们的设置。
  // (Flag to establish if the QoS of the DomainParticipant,
  // its DataWriters, and its DataReaders are going
  // to be configured only from an XML file or if
  // their settings are going to be overwritten by code
  // with the default configuration.)
  bool leave_middleware_default_qos;
  publishing_mode_t publishing_mode;

  // 查找或创建主题的函数 (Function to find or create a topic)
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  eprosima::fastdds::dds::Topic *find_or_create_topic(
      const std::string &topic_name,
      const std::string &type_name,
      const eprosima::fastdds::dds::TopicQos &topic_qos,
      EventListenerInterface *event_listener);

  // 删除主题的函数 (Function to delete a topic)
  RMW_FASTRTPS_SHARED_CPP_PUBLIC
  void delete_topic(
      const eprosima::fastdds::dds::Topic *topic, EventListenerInterface *event_listener);
} CustomParticipantInfo;

/**
 * @class ParticipantListener
 * @brief 自定义参与者监听器，继承自 eprosima::fastdds::dds::DomainParticipantListener.
 *        Custom participant listener, inherits from
 * eprosima::fastdds::dds::DomainParticipantListener.
 */
class ParticipantListener : public eprosima::fastdds::dds::DomainParticipantListener {
public:
  /**
   * @brief 构造函数，初始化参与者监听器。
   *        Constructor, initializes the participant listener.
   *
   * @param identifier 指向一个字符串的指针，表示唯一标识符。Pointer to a string representing a
   * unique identifier.
   * @param context 指向 rmw_dds_common::Context 的指针，用于存储 ROS2 上下文信息。Pointer to
   * rmw_dds_common::Context for storing ROS2 context information.
   */
  explicit ParticipantListener(const char *identifier, rmw_dds_common::Context *context)
      : context(context), identifier_(identifier) {}

  /**
   * @brief 参与者发现回调函数 (Participant discovery callback function)
   *
   * @param[in] participant 指向 DomainParticipant 的指针 (Pointer to the DomainParticipant)
   * @param[in] info 参与者发现信息结构体 (ParticipantDiscoveryInfo structure)
   * @param[out] should_be_ignored 是否应该忽略此参与者 (Whether this participant should be ignored)
   */
  void on_participant_discovery(
      eprosima::fastdds::dds::DomainParticipant *,
      eprosima::fastrtps::rtps::ParticipantDiscoveryInfo &&info,
      bool &should_be_ignored) override {
    // 初始化 should_be_ignored 为 false
    // Initialize should_be_ignored to false
    should_be_ignored = false;

    // 根据参与者发现状态进行处理
    // Process according to participant discovery status
    switch (info.status) {
      case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT: {
        // 解析用户数据中的键值对
        // Parse key-value pairs in user data
        auto map = rmw::impl::cpp::parse_key_value(info.info.m_userData);

        // 查找 "enclave" 键
        // Find the "enclave" key
        auto name_found = map.find("enclave");

        // 如果未找到 "enclave" 键，则返回
        // If the "enclave" key is not found, return
        if (name_found == map.end()) {
          return;
        }

        // 获取 enclave 字符串
        // Get the enclave string
        auto enclave = std::string(name_found->second.begin(), name_found->second.end());

        // 向图缓存中添加参与者
        // Add participant to the graph cache
        context->graph_cache.add_participant(
            rmw_fastrtps_shared_cpp::create_rmw_gid(identifier_, info.info.m_guid), enclave);
        break;
      }
      case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT:
      // fall through
      case eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT:
        // 从图缓存中移除参与者
        // Remove participant from the graph cache
        context->graph_cache.remove_participant(
            rmw_fastrtps_shared_cpp::create_rmw_gid(identifier_, info.info.m_guid));
        break;

      // 对于其他状态，直接返回
      // For other statuses, return directly
      default:
        return;
    }
  }

  /**
   * @brief 当订阅者被发现时的回调函数 (Callback function when a subscriber is discovered)
   *
   * @param[in] participant 域参与者指针 (Pointer to the DomainParticipant)
   * @param[in] info 订阅者发现信息 (Reader discovery information)
   */
  void on_subscriber_discovery(
      eprosima::fastdds::dds::DomainParticipant *,
      eprosima::fastrtps::rtps::ReaderDiscoveryInfo &&info) override {
    // 检查订阅者的状态是否为 CHANGED_QOS_READER (Check if the status of the subscriber is
    // CHANGED_QOS_READER)
    if (eprosima::fastrtps::rtps::ReaderDiscoveryInfo::CHANGED_QOS_READER != info.status) {
      // 判断订阅者是否存活 (Determine if the subscriber is alive)
      bool is_alive =
          eprosima::fastrtps::rtps::ReaderDiscoveryInfo::DISCOVERED_READER == info.status;
      // 处理订阅者发现信息 (Process the subscriber discovery information)
      process_discovery_info(info.info, is_alive, true);
    }
  }

  /**
   * @brief 当发布者被发现时的回调函数 (Callback function when a publisher is discovered)
   *
   * @param[in] participant 域参与者指针 (Pointer to the DomainParticipant)
   * @param[in] info 发布者发现信息 (Writer discovery information)
   */
  void on_publisher_discovery(
      eprosima::fastdds::dds::DomainParticipant *,
      eprosima::fastrtps::rtps::WriterDiscoveryInfo &&info) override {
    // 检查发布者的状态是否为 CHANGED_QOS_WRITER (Check if the status of the publisher is
    // CHANGED_QOS_WRITER)
    if (eprosima::fastrtps::rtps::WriterDiscoveryInfo::CHANGED_QOS_WRITER != info.status) {
      // 判断发布者是否存活 (Determine if the publisher is alive)
      bool is_alive =
          eprosima::fastrtps::rtps::WriterDiscoveryInfo::DISCOVERED_WRITER == info.status;
      // 处理发布者发现信息 (Process the publisher discovery information)
      process_discovery_info(info.info, is_alive, false);
    }
  }

private:
  /**
   * @brief 处理实体发现信息 (Process discovery information of an entity)
   *
   * @tparam T 实体代理数据类型 (Entity proxy data type)
   * @param[in] proxyData 实体代理数据 (Entity proxy data)
   * @param[in] is_alive 实体是否存活 (Whether the entity is alive or not)
   * @param[in] is_reader 实体是否为读者 (Whether the entity is a reader or not)
   */
  template <class T>
  void process_discovery_info(T &proxyData, bool is_alive, bool is_reader) {
    // 如果实体存活 (If the entity is alive)
    if (is_alive) {
      // 初始化 QoS 配置文件 (Initialize QoS profile)
      rmw_qos_profile_t qos_profile = rmw_qos_profile_unknown;
      // 将 RTPS QoS 转换为 RMW QoS (Convert RTPS QoS to RMW QoS)
      rtps_qos_to_rmw_qos(proxyData.m_qos, &qos_profile);

      // 获取用户数据值 (Get user data value)
      const auto &userDataValue = proxyData.m_qos.m_userData.getValue();
      // 初始化类型哈希 (Initialize type hash)
      rosidl_type_hash_t type_hash;
      // 从用户数据中解析类型哈希 (Parse type hash from user data)
      if (RMW_RET_OK != rmw_dds_common::parse_type_hash_from_user_data(
                            userDataValue.data(), userDataValue.size(), type_hash)) {
        // 使用 eProsima 的日志机制避免死锁，不尝试获取 rclcpp 的全局日志互斥锁
        // (Avoid deadlock trying to acquire rclcpp's global logging mutex by using eProsima's
        // logging mechanism) 当此问题修复时，请重新审查：https://github.com/ros2/rclcpp/issues/2147
        // (TODO(sloretz) revisit when this is fixed: https://github.com/ros2/rclcpp/issues/2147)
        EPROSIMA_LOG_WARNING("rmw_fastrtps_shared_cpp", "Failed to parse a type hash for a topic");
        // 初始化为零的类型哈希 (Initialize type hash with zero)
        type_hash = rosidl_get_zero_initialized_type_hash();
      }

      // 向图缓存中添加实体 (Add entity to the graph cache)
      context->graph_cache.add_entity(
          rmw_fastrtps_shared_cpp::create_rmw_gid(identifier_, proxyData.guid()),
          proxyData.topicName().to_string(), proxyData.typeName().to_string(), type_hash,
          rmw_fastrtps_shared_cpp::create_rmw_gid(
              identifier_, iHandle2GUID(proxyData.RTPSParticipantKey())),
          qos_profile, is_reader);
    } else {
      // 从图缓存中移除实体 (Remove entity from the graph cache)
      context->graph_cache.remove_entity(
          rmw_fastrtps_shared_cpp::create_rmw_gid(identifier_, proxyData.guid()), is_reader);
    }
  }

  // DDS 公共上下文指针 (Pointer to DDS common context)
  rmw_dds_common::Context *context;
  // 标识符常量指针 (Constant pointer to identifier)
  const char *const identifier_;
};

#endif  // RMW_FASTRTPS_SHARED_CPP__CUSTOM_PARTICIPANT_INFO_HPP_
