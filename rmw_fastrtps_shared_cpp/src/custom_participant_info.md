##

```cpp
CustomTopicListener::CustomTopicListener(EventListenerInterface* event_listener) {}
void CustomTopicListener::on_inconsistent_topic(
    eprosima::fastdds::dds::Topic* topic, eprosima::fastdds::dds::InconsistentTopicStatus status) {}
void CustomTopicListener::add_event_listener(EventListenerInterface* event_listener) {}
void CustomTopicListener::remove_event_listener(EventListenerInterface* event_listener) {}
eprosima::fastdds::dds::Topic* CustomParticipantInfo::find_or_create_topic(
    const std::string& topic_name,
    const std::string& type_name,
    const eprosima::fastdds::dds::TopicQos& topic_qos,
    EventListenerInterface* event_listener) {}
void CustomParticipantInfo::delete_topic(
    const eprosima::fastdds::dds::Topic* topic, EventListenerInterface* event_listener) {}
```

### CustomTopicListener 类

`CustomTopicListener` 类是一个自定义的主题监听器，用于处理与主题相关的事件。它包含以下成员函数：

1. **构造函数**：该构造函数接收一个指向 `EventListenerInterface` 的指针，用于初始化监听器。

2. **on_inconsistent_topic**：当发现不一致的主题时，此函数会被调用。它接收一个指向 `eprosima::fastdds::dds::Topic` 的指针和一个 `eprosima::fastdds::dds::InconsistentTopicStatus` 状态对象。

3. **add_event_listener**：`void add_event_listener(EventListenerInterface* event_listener)`。此函数用于添加一个事件监听器，以便在发生与主题相关的事件时通知它。

4. **remove_event_listener**：`void remove_event_listener(EventListenerInterface* event_listener)`。此函数用于从监听器列表中移除一个事件监听器。

### CustomParticipantInfo 类

`CustomParticipantInfo` 类主要用于管理 Fast DDS 参与者（即节点）的信息。它包含以下成员函数：

1. **find_or_create_topic**：此函数用于查找或创建一个主题。它接收主题名称、类型名称、主题 QoS 和事件监听器作为参数。如果主题已经存在，它将返回指向该主题的指针；否则，它将创建一个新的主题并返回指向它的指针。

2. **delete_topic**：指针和一个事件监听器作为参数。在删除主题之前，它会从 `CustomTopicListener` 中移除对应的事件监听器。

总结：这份代码主要用于管理 ROS2 项目中 `rmw_fastrtps` 相关的主题和参与者信息。`CustomTopicListener` 类负责处理与主题相关的事件，而 `CustomParticipantInfo` 类负责管理 Fast DDS 参与者的信息。这些类及其成员函数之间的相互联系主要体现在事件监听器的添加、移除以及主题的查找、创建和删除等操作上。

---

Crafted by [Genie](https://marketplace.visualstudio.com/items?itemName=genieai.chatgpt-vscode)

---

## You

下面这段代码是在 ros2 项目中 rmw_fastrtps 相关的代码(custom_publisher_info.cpp)，请联系所有函数之间可能的相互联系，梳理、解释这份文件的功能、含义，以及函数之间的功能调用关系（以中文语言输出，不限制字数）

```cpp
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
 * @brief 构造函数，创建一个 CustomTopicListener 对象
 * @param event_listener 事件监听器接口指针
 */
CustomTopicListener::CustomTopicListener(EventListenerInterface* event_listener) {
  // 添加事件监听器 (Add event listener)
  add_event_listener(event_listener);
}

/**
 * @brief 当主题不一致时的回调函数
 * @param topic 主题指针
 * @param status 不一致主题状态
 */
void CustomTopicListener::on_inconsistent_topic(
    eprosima::fastdds::dds::Topic* topic, eprosima::fastdds::dds::InconsistentTopicStatus status) {
  // 如果主题为空，则返回 (If the topic is nullptr, return)
  if (topic == nullptr) {
    return;
  }

  // 使用互斥锁保护事件监听器列表
  std::lock_guard<std::mutex> lck(event_listeners_mutex_);
  // 遍历事件监听器列表，更新不一致主题计数
  for (EventListenerInterface* listener : event_listeners_) {
    listener->update_inconsistent_topic(status.total_count, status.total_count_change);
  }
}

/**
 * @brief 添加事件监听器
 * @param event_listener 事件监听器接口指针
 */
void CustomTopicListener::add_event_listener(EventListenerInterface* event_listener) {
  // 允许空的事件监听器，但不会对它们进行报告
  if (event_listener == nullptr) {
    return;
  }

  // 使用互斥锁保护事件监听器列表
  std::lock_guard<std::mutex> lck(event_listeners_mutex_);
  // 将事件监听器插入到列表中
  event_listeners_.insert(event_listener);
}

/**
 * @brief 移除事件监听器 (Remove an event listener)
 * @param event_listener 事件监听器接口指针
 */
void CustomTopicListener::remove_event_listener(EventListenerInterface* event_listener) {
  // 如果事件监听器为空，则返回 (If the event listener is nullptr, return)
  if (event_listener == nullptr) {
    return;
  }

  // 使用互斥锁保护事件监听器列表
  std::lock_guard<std::mutex> lck(event_listeners_mutex_);
  // 从列表中删除事件监听器
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

  // 对topic_name_to_topic_mutex_进行加锁，防止多线程冲突
  std::lock_guard<std::mutex> lck(topic_name_to_topic_mutex_);
  // 查找主题名称是否已存在于映射中
  std::map<std::string, std::unique_ptr<UseCountTopic>>::const_iterator it =
      topic_name_to_topic_.find(topic_name);
  if (it == topic_name_to_topic_.end()) {
    // 如果映射中不存在，则需要添加新主题
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
    // 如果映射中已存在，则增加使用计数
    it->second->use_count++;
    it->second->topic_listener->add_event_listener(event_listener);
    topic = it->second->topic;
  }

  return topic;
}

/**
 * @brief 删除一个主题
 *
 * @param[in] topic 要删除的主题指针
 * @param[in] event_listener 事件监听器接口
 */
void CustomParticipantInfo::delete_topic(
    const eprosima::fastdds::dds::Topic* topic, EventListenerInterface* event_listener) {
  // 如果主题为空，则直接返回
  if (topic == nullptr) {
    return;
  }

  // 对topic_name_to_topic_mutex_进行加锁，防止多线程冲突
  std::lock_guard<std::mutex> lck(topic_name_to_topic_mutex_);
  // 查找主题名称是否已存在于映射中
  std::map<std::string, std::unique_ptr<UseCountTopic>>::const_iterator it =
      topic_name_to_topic_.find(topic->get_name());

  if (it != topic_name_to_topic_.end()) {
    // 减少使用计数
    it->second->use_count--;
    it->second->topic_listener->remove_event_listener(event_listener);
    // 如果使用计数为0，则删除主题
    if (it->second->use_count <= 0) {
      participant_->delete_topic(it->second->topic);

      // 删除主题监听器
      delete it->second->topic_listener;

      // 从映射中移除主题
      topic_name_to_topic_.erase(it);
    }
  } else {
    // 如果映射中不存在该主题，则发出警告
    RCUTILS_LOG_WARN_NAMED(
        "rmw_fastrtps_shared_cpp",
        "Attempted to delete topic '%s', but it was never created.  Ignoring",
        topic->get_name().c_str());
  }
}

```

## Trantor

这份文件是 ROS2 项目中 rmw_fastrtps 相关的代码，主要包含了一个 `CustomTopicListener` 类和一个 `CustomParticipantInfo` 类。这两个类分别用于处理主题监听器的功能和参与者信息。

### CustomTopicListener 类

`CustomTopicListener` 类主要负责管理事件监听器，以及处理不一致主题的回调函数。

1. 构造函数：创建一个 `CustomTopicListener` 对象，并添加事件监听器。
2. `on_inconsistent_topic`：当主题不一致时的回调函数，遍历事件监听器列表，更新不一致主题计数。
3. `add_event_listener`：添加事件监听器到列表中。
4. `remove_event_listener`：从列表中移除事件监听器。

### CustomParticipantInfo 类

`CustomParticipantInfo` 类主要负责查找或创建主题，以及删除主题。

1. `find_or_create_topic`：查找或创建一个主题。如果映射中不存在该主题，则创建新主题并设置主题监听器；如果映射中已存在该主题，则增加使用计数。
2. `delete_topic`：删除一个主题。减少使用计数，如果使用计数为 0，则删除主题并移除主题监听器。

### 函数调用关系

- `CustomTopicListener` 类的构造函数会调用 `add_event_listener` 方法。
- `on_inconsistent_topic` 方法会在主题不一致时被调用，它会遍历事件监听器列表并更新不一致主题计数。
- `CustomParticipantInfo` 类的 `find_or_create_topic` 方法会调用 `CustomTopicListener` 类的 `add_event_listener` 方法，以添加事件监听器。
- `CustomParticipantInfo` 类的 `delete_topic` 方法会调用 `CustomTopicListener` 类的 `remove_event_listener` 方法，以移除事件监听器。

总之，这份文件主要实现了 ROS2 rmw_fastrtps 中对于主题监听器和参与者信息的管理功能。

## You

类似的，我希望能按照这个文件中的代码在 rmw_cyclonedds_cpp 也实现一份同样的功能，请给出完整的示例代码

## Trantor

为了在 `rmw_cyclonedds_cpp` 中实现类似的功能，我们需要创建一个新的头文件（例如 `custom_publisher_info_cyclonedds.hpp`）和相应的源文件（例如 `custom_publisher_info_cyclonedds.cpp`）。以下是示例代码：

**custom_publisher_info_cyclonedds.hpp**

```cpp
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

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "dds/dds.h"
#include "rcutils/logging_macros.h"

class EventListenerInterface
{
public:
  virtual void update_inconsistent_topic(int32_t total_count, int32_t total_count_change) = 0;
};

class CustomTopicListener
{
public:
  explicit CustomTopicListener(EventListenerInterface* event_listener);
  void on_inconsistent_topic(dds_entity_t topic, const dds_requested_incompatible_qos_status_t& status);
  void add_event_listener(EventListenerInterface* event_listener);
  void remove_event_listener(EventListenerInterface* event_listener);

private:
  std::set<EventListenerInterface*> event_listeners_;
  std::mutex event_listeners_mutex_;
};

class CustomParticipantInfo
{
public:
  dds_entity_t find_or_create_topic(
    const std::string& topic_name,
    const std::string& type_name,
    const dds_qos_t* topic_qos,
    EventListenerInterface* event_listener);
  void delete_topic(dds_entity_t topic, EventListenerInterface* event_listener);

private:
  struct UseCountTopic
  {
    dds_entity_t topic;
    CustomTopicListener* topic_listener;
    int32_t use_count;
  };

  std::map<std::string, std::unique_ptr<UseCountTopic>> topic_name_to_topic_;
  std::mutex topic_name_to_topic_mutex_;
};
```

**custom_publisher_info_cyclonedds.cpp**

```cpp
#include "custom_publisher_info_cyclonedds.hpp"

CustomTopicListener::CustomTopicListener(EventListenerInterface* event_listener)
{
  add_event_listener(event_listener);
}

void CustomTopicListener::on_inconsistent_topic(
  dds_entity_t topic, const dds_requested_incompatible_qos_status_t& status)
{
  if (topic == DDS_ENTITY_NIL) {
    return;
  }

  std::lock_guard<std::mutex> lck(event_listeners_mutex_);
  for (EventListenerInterface* listener : event_listeners_) {
    listener->update_inconsistent_topic(status.total_count, status.total_count_change);
  }
}

void CustomTopicListener::add_event_listener(EventListenerInterface* event_listener)
{
  if (event_listener == nullptr) {
    return;
  }

  std::lock_guard<std::mutex> lck(event_listeners_mutex_);
  event_listeners_.insert(event_listener);
}

void CustomTopicListener::remove_event_listener(EventListenerInterface* event_listener)
{
  if (event_listener == nullptr) {
    return;
  }

  std::lock_guard<std::mutex> lck(event_listeners_mutex_);
  event_listeners_.erase(event_listener);
}

dds_entity_t CustomParticipantInfo::find_or_create_topic(
  const std::string& topic_name,
  const std::string& type_name,
  const dds_qos_t* topic_qos,
  EventListenerInterface* event_listener)
{
  dds_entity_t topic = DDS_ENTITY_NIL;

  std::lock_guard<std::mutex> lck(topic_name_to_topic_mutex_);
  auto it = topic_name_to_topic_.find(topic_name);
  if (it == topic_name_to_topic_.end()) {
    topic = dds_create_topic(participant_, type_name.c_str(), topic_name.c_str(), topic_qos, nullptr);

    auto uct = std::make_unique<UseCountTopic>();
    uct->topic = topic;
    uct->topic_listener = new CustomTopicListener(event_listener);
    uct->use_count = 1;

    topic_name_to_topic_[topic_name] = std::move(uct);
  } else {
    it->second->use_count++;
    it->second->topic_listener->add_event_listener(event_listener);
    topic = it->second->topic;
  }

  return topic;
}

void CustomParticipantInfo::delete_topic(dds_entity_t topic, EventListenerInterface* event_listener)
{
  if (topic == DDS_ENTITY_NIL) {
    return;
  }

  std::lock_guard<std::mutex> lck(topic_name_to_topic_mutex_);
  auto it = topic_name_to_topic_.begin();
  while (it != topic_name_to_topic_.end()) {
    if (it->second->topic == topic) {
      break;
    }
    ++it;
  }

  if (it != topic_name_to_topic_.end()) {
    it->second->use_count--;
    it->second->topic_listener->remove_event_listener(event_listener);
    if (it->second->use_count <= 0) {
      dds_delete(it->second->topic);

      delete it->second->topic_listener;

      topic_name_to_topic_.erase(it);
    }
  } else {
    RCUTILS_LOG_WARN_NAMED(
      "rmw_cyclonedds_cpp",
      "Attempted to delete topic '%s', but it was never created. Ignoring",
      dds_get_topic_name(topic));
  }
}
```

这个示例代码实现了类似于 `rmw_fastrtps` 的功能，但针对的是 `rmw_cyclonedds_cpp`。请注意，为了使这个示例代码正常工作，您可能需要根据实际的 `rmw_cyclonedds_cpp` 项目结构进行一些调整。
