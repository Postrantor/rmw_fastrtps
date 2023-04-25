// Copyright 2023 Open Source Robotics Foundation, Inc.
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

#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "fastdds/dds/topic/Topic.hpp"
#include "fastdds/dds/topic/qos/TopicQos.hpp"
#include "rcutils/logging_macros.h"

/**
 * @brief 构造函数，创建一个 CustomTopicListener 对象 (Constructor, creates a CustomTopicListener
 * object)
 * @param event_listener 事件监听器接口指针 (Pointer to the EventListenerInterface)
 */
CustomTopicListener::CustomTopicListener(EventListenerInterface* event_listener) {
  // 添加事件监听器 (Add event listener)
  add_event_listener(event_listener);
}

/**
 * @brief 当主题不一致时的回调函数 (Callback function when there is an inconsistent topic)
 * @param topic 主题指针 (Pointer to the Topic)
 * @param status 不一致主题状态 (InconsistentTopicStatus)
 */
void CustomTopicListener::on_inconsistent_topic(
    eprosima::fastdds::dds::Topic* topic, eprosima::fastdds::dds::InconsistentTopicStatus status) {
  // 如果主题为空，则返回 (If the topic is nullptr, return)
  if (topic == nullptr) {
    return;
  }

  // 使用互斥锁保护事件监听器列表 (Protect the event listeners list with a mutex lock)
  std::lock_guard<std::mutex> lck(event_listeners_mutex_);
  // 遍历事件监听器列表，更新不一致主题计数 (Iterate through the event listeners list and update the
  // inconsistent topic count)
  for (EventListenerInterface* listener : event_listeners_) {
    listener->update_inconsistent_topic(status.total_count, status.total_count_change);
  }
}

/**
 * @brief 添加事件监听器 (Add an event listener)
 * @param event_listener 事件监听器接口指针 (Pointer to the EventListenerInterface)
 */
void CustomTopicListener::add_event_listener(EventListenerInterface* event_listener) {
  // 允许空的事件监听器，但不会对它们进行报告 (Allow for null event listeners, but they will not be
  // reported on)
  if (event_listener == nullptr) {
    return;
  }

  // 使用互斥锁保护事件监听器列表 (Protect the event listeners list with a mutex lock)
  std::lock_guard<std::mutex> lck(event_listeners_mutex_);
  // 将事件监听器插入到列表中 (Insert the event listener into the list)
  event_listeners_.insert(event_listener);
}

/**
 * @brief 移除事件监听器 (Remove an event listener)
 * @param event_listener 事件监听器接口指针 (Pointer to the EventListenerInterface)
 */
void CustomTopicListener::remove_event_listener(EventListenerInterface* event_listener) {
  // 如果事件监听器为空，则返回 (If the event listener is nullptr, return)
  if (event_listener == nullptr) {
    return;
  }

  // 使用互斥锁保护事件监听器列表 (Protect the event listeners list with a mutex lock)
  std::lock_guard<std::mutex> lck(event_listeners_mutex_);
  // 从列表中删除事件监听器 (Erase the event listener from the list)
  event_listeners_.erase(event_listener);
}

/**
 * @brief 查找或创建一个主题 (Find or create a topic)
 *
 * @param[in] topic_name 主题名称 (Topic name)
 * @param[in] type_name 类型名称 (Type name)
 * @param[in] topic_qos 主题的服务质量 (Quality of Service for the topic)
 * @param[in] event_listener 事件监听器接口 (Event Listener Interface)
 * @return 返回创建或找到的主题指针 (Returns a pointer to the created or found topic)
 */
eprosima::fastdds::dds::Topic* CustomParticipantInfo::find_or_create_topic(
    const std::string& topic_name,
    const std::string& type_name,
    const eprosima::fastdds::dds::TopicQos& topic_qos,
    EventListenerInterface* event_listener) {
  // 初始化主题指针为空 (Initialize the topic pointer as nullptr)
  eprosima::fastdds::dds::Topic* topic = nullptr;

  // 对topic_name_to_topic_mutex_进行加锁，防止多线程冲突 (Lock the topic_name_to_topic_mutex_ to
  // prevent multi-threading conflicts)
  std::lock_guard<std::mutex> lck(topic_name_to_topic_mutex_);
  // 查找主题名称是否已存在于映射中 (Check if the topic name already exists in the map)
  std::map<std::string, std::unique_ptr<UseCountTopic>>::const_iterator it =
      topic_name_to_topic_.find(topic_name);
  if (it == topic_name_to_topic_.end()) {
    // 如果映射中不存在，则需要添加新主题 (If not in the map, we need to add a new topic)
    topic = participant_->create_topic(topic_name, type_name, topic_qos);

    // 创建一个新的UseCountTopic对象 (Create a new UseCountTopic object)
    auto uct = std::make_unique<UseCountTopic>();
    uct->topic = topic;
    uct->topic_listener = new CustomTopicListener(event_listener);
    uct->use_count = 1;
    // 设置主题监听器 (Set the topic listener)
    topic->set_listener(uct->topic_listener);
    // 将新创建的主题添加到映射中 (Add the newly created topic to the map)
    topic_name_to_topic_[topic_name] = std::move(uct);
  } else {
    // 如果映射中已存在，则增加使用计数 (If already in the map, just increase the use count)
    it->second->use_count++;
    it->second->topic_listener->add_event_listener(event_listener);
    topic = it->second->topic;
  }

  return topic;
}

/**
 * @brief 删除一个主题 (Delete a topic)
 *
 * @param[in] topic 要删除的主题指针 (Pointer to the topic to be deleted)
 * @param[in] event_listener 事件监听器接口 (Event Listener Interface)
 */
void CustomParticipantInfo::delete_topic(
    const eprosima::fastdds::dds::Topic* topic, EventListenerInterface* event_listener) {
  // 如果主题为空，则直接返回 (If the topic is nullptr, return directly)
  if (topic == nullptr) {
    return;
  }

  // 对topic_name_to_topic_mutex_进行加锁，防止多线程冲突 (Lock the topic_name_to_topic_mutex_ to
  // prevent multi-threading conflicts)
  std::lock_guard<std::mutex> lck(topic_name_to_topic_mutex_);
  // 查找主题名称是否已存在于映射中 (Check if the topic name already exists in the map)
  std::map<std::string, std::unique_ptr<UseCountTopic>>::const_iterator it =
      topic_name_to_topic_.find(topic->get_name());

  if (it != topic_name_to_topic_.end()) {
    // 减少使用计数 (Decrease the use count)
    it->second->use_count--;
    it->second->topic_listener->remove_event_listener(event_listener);
    // 如果使用计数为0，则删除主题 (If the use count is 0, delete the topic)
    if (it->second->use_count <= 0) {
      participant_->delete_topic(it->second->topic);

      // 删除主题监听器 (Delete the topic listener)
      delete it->second->topic_listener;

      // 从映射中移除主题 (Remove the topic from the map)
      topic_name_to_topic_.erase(it);
    }
  } else {
    // 如果映射中不存在该主题，则发出警告 (If the topic does not exist in the map, issue a warning)
    RCUTILS_LOG_WARN_NAMED(
        "rmw_fastrtps_shared_cpp",
        "Attempted to delete topic '%s', but it was never created.  Ignoring",
        topic->get_name().c_str());
  }
}
