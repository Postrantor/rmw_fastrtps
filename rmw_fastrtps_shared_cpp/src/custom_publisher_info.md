##

```cpp
EventListenerInterface *CustomPublisherInfo::get_listener() const {}

CustomDataWriterListener::CustomDataWriterListener(RMWPublisherEvent *pub_event)
    : publisher_event_(pub_event) {}
void CustomDataWriterListener::on_publication_matched(
    eprosima::fastdds::dds::DataWriter *writer,
    const eprosima::fastdds::dds::PublicationMatchedStatus &status) {}
void CustomDataWriterListener::on_offered_deadline_missed(
    eprosima::fastdds::dds::DataWriter *writer,
    const eprosima::fastdds::dds::OfferedDeadlineMissedStatus &status) {}
void CustomDataWriterListener::on_liveliness_lost(
    eprosima::fastdds::dds::DataWriter *writer,
    const eprosima::fastdds::dds::LivelinessLostStatus &status) {}
void CustomDataWriterListener::on_offered_incompatible_qos(
    eprosima::fastdds::dds::DataWriter *writer,
    const eprosima::fastdds::dds::OfferedIncompatibleQosStatus &status) {}
```

### CustomPublisherInfo 类

`CustomPublisherInfo` 类是一个自定义的发布者信息类，它主要用于存储与 Fast RTPS 发布者相关的信息。这些信息包括发布者的监听器（`EventListenerInterface` 类型）等。在 ROS2 的 Fast RTPS 中间件实现中，`CustomPublisherInfo` 类起到了桥接作用，将 ROS2 的发布者与 Fast RTPS 的底层实现连接起来。

`CustomPublisherInfo` 类只有一个函数：

```cpp
EventListenerInterface *CustomPublisherInfo::get_listener() const {}
```

这个函数的作用是获取当前 `CustomPublisherInfo` 对象的监听器（`EventListenerInterface` 类型）。监听器用于处理与发布者相关的事件，例如匹配订阅者、错过期限、失去活跃状态以及提供不兼容的 QoS 等。通过这个函数，我们可以在需要时获取到发布者的监听器，从而处理相应的事件。

### CustomDataWriterListener 类

`CustomDataWriterListener` 类继承自 Fast DDS 的 `DataWriterListener` 类，用于处理与数据写入相关的事件。这个类的主要作用是在 Fast RTPS 中间件实现中捕获与数据写入相关的事件，并将这些事件传递给 ROS2 的发布者。这样，ROS2 的发布者就可以根据这些事件来执行相应的操作，例如更新匹配订阅者的数量、处理错过的期限等。

`CustomDataWriterListener` 类包含以下成员函数：

1. 构造函数：

```cpp
CustomDataWriterListener::CustomDataWriterListener(RMWPublisherEvent *pub_event)
    : publisher_event_(pub_event) {}
```

构造函数接收一个 `RMWPublisherEvent` 类型的指针，并将其赋值给成员变量 `publisher_event_`。这个指针用于存储与发布者相关的事件信息。当 Fast RTPS 中间件实现捕获到与数据写入相关的事件时，它会通过这个指针将事件传递给 ROS2 的发布者。

2. on_publication_matched：

```cpp
void CustomDataWriterListener::on_publication_matched(
    eprosima::fastdds::dds::DataWriter *writer,
    const eprosima::fastdds::dds::PublicationMatchedStatus &status) {}
```

当数据写入器（`DataWriter`）匹配到一个订阅者时，此函数会被调用。它接收一个 `DataWriter` 指针和一个 `PublicationMatchedStatus` 对象，用于处理匹配事件。在这个函数中，我们可以根据 `PublicationMatchedStatus` 对象的信息来更新匹配订阅者的数量，从而让 ROS2 的发布者知道有多少订阅者正在接收它的消息。

3. on_offered_deadline_missed：

```cpp
void CustomDataWriterListener::on_offered_deadline_missed(
    eprosima::fastdds::dds::DataWriter *writer,
    const eprosima::fastdds::dds::OfferedDeadlineMissedStatus &status) {}
```

当数据写入器（`DataWriter`）错过了一个期限时，此函数会被调用。它接收一个 `DataWriter` 指针和一个 `OfferedDeadlineMissedStatus` 对象，用于处理错过期限的事件。在这个函数中，我们可以根据 `OfferedDeadlineMissedStatus` 对象的信息来处理错过的期限，例如记录日志或者通知 ROS2 的发布者。

4. on_liveliness_lost：

```cpp
void CustomDataWriterListener::on_liveliness_lost(
    eprosima::fastdds::dds::DataWriter *writer,
    const eprosima::fastdds::dds::LivelinessLostStatus &status) {}
```

当数据写入器（`DataWriter`）失去活跃状态时，此函数会被调用。它接收一个 `DataWriter` 指针和一个 `LivelinessLostStatus` 对象，用于处理失去活跃状态的事件。在这个函数中，我们可以根据 `LivelinessLostStatus` 对象的信息来处理失去活跃状态的情况，例如重新建立连接或者通知 ROS2 的发布者。

5. on_offered_incompatible_qos：

```cpp
void CustomDataWriterListener::on_offered_incompatible_qos(
    eprosima::fastdds::dds::DataWriter *writer,
    const eprosima::fastdds::dds::OfferedIncompatibleQosStatus &status) {}
```

当数据写入器（`DataWriter`）提供了不兼容的 QoS 时，此函数会被调用。它接收一个 `DataWriter` 指针和一个 `OfferedIncompatibleQosStatus` 对象，用于处理不兼容 QoS 的事件。在这个函数中，我们可以根据 `OfferedIncompatibleQosStatus` 对象的信息来处理不兼容的 QoS，例如调整 QoS 设置或者通知 ROS2 的发布者。

总结：这份文件主要定义了两个类，`CustomPublisherInfo` 类用于存储与 Fast RTPS 发布者相关的信息，并提供获取发布者监听器的功能；而 `CustomDataWriterListener` 类则用于捕获与数据写入相关的事件，并将这些事件传递给 ROS2 的发布者。这样，ROS2 的发布者就可以根据这些事件来执行相应的操作，例如更新匹配订阅者的数量、处理错过的期限等。这两个类在 ROS2 的 Fast RTPS 中间件实现中起到了桥接作用，将 ROS2 的发布者与 Fast RTPS 的底层实现连接起来。

##

```cpp
RMWPublisherEvent::RMWPublisherEvent(CustomPublisherInfo *info)
    : publisher_info_(info),
      deadline_changed_(false),
      liveliness_changed_(false),
      incompatible_qos_changed_(false),
      matched_changes_(false) {}
eprosima::fastdds::dds::StatusCondition &RMWPublisherEvent::get_statuscondition() const {}
bool RMWPublisherEvent::take_event(rmw_event_type_t event_type, void *event_info) {}
void RMWPublisherEvent::set_on_new_event_callback(
    rmw_event_type_t event_type, const void *user_data, rmw_event_callback_t callback) {}
void RMWPublisherEvent::track_unique_subscription(eprosima::fastrtps::rtps::GUID_t guid) {}
void RMWPublisherEvent::untrack_unique_subscription(eprosima::fastrtps::rtps::GUID_t guid) {}
size_t RMWPublisherEvent::subscription_count() const {}
void RMWPublisherEvent::update_deadline(uint32_t total_count, uint32_t total_count_change) {}
void RMWPublisherEvent::update_liveliness_lost(uint32_t total_count, uint32_t total_count_change) {}
void RMWPublisherEvent::update_offered_incompatible_qos(
    eprosima::fastdds::dds::QosPolicyId_t last_policy_id,
    uint32_t total_count,
    uint32_t total_count_change) {}
void RMWPublisherEvent::update_inconsistent_topic(
    uint32_t total_count, uint32_t total_count_change) {}
void RMWPublisherEvent::update_matched(
    int32_t total_count,
    int32_t total_count_change,
    int32_t current_count,
    int32_t current_count_change) {}
void RMWPublisherEvent::trigger_event(rmw_event_type_t event_type) {}
```

`RMWPublisherEvent` 类是一个用于处理 ROS2 发布者相关事件的类。它提供了一系列函数，用于管理和跟踪发布者与订阅者之间的通信状态、匹配情况以及 QoS 策略等。此外，它还支持设置回调函数，以便在特定类型的事件发生时触发相应的操作。

主要功能包括：

1. 跟踪和取消跟踪唯一订阅
2. 获取和设置事件回调函数
3. 更新和触发各种事件，如截止日期、失活、不兼容 QoS 和不一致主题等

通过这些功能，`RMWPublisherEvent` 类有助于实现 ROS2 发布者与订阅者之间的有效通信和协调。

1. **RMWPublisherEvent::RMWPublisherEvent(CustomPublisherInfo \*info)**：构造函数，初始化 `RMWPublisherEvent` 对象。它接收一个指向 `CustomPublisherInfo` 的指针作为参数，并设置发布者信息、事件状态等。

2. **eprosima::fastdds::dds::StatusCondition &RMWPublisherEvent::get_statuscondition() const**：此函数返回当前对象的状态条件，这是一个引用。状态条件用于表示与实体相关的状态变化，例如发布者的 QoS 改变或通信中断。

3. **bool RMWPublisherEvent::take_event(rmw_event_type_t event_type, void \*event_info)**：此函数根据传入的事件类型（`event_type`）获取事件信息（`event_info`）。如果成功获取事件信息，则返回 true，否则返回 false。

4. **void RMWPublisherEvent::set_on_new_event_callback(rmw_event_type_t event_type, const void \*user_data, rmw_event_callback_t callback)**：此函数用于设置新事件回调函数。当指定类型的事件发生时，将调用此回调函数。它接收三个参数：事件类型（`event_type`）、用户数据（`user_data`）和回调函数（`callback`）。

5. **void RMWPublisherEvent::track_unique_subscription(eprosima::fastrtps::rtps::GUID_t guid)**：此函数用于跟踪唯一订阅。它接收一个 GUID（全局唯一标识符）作为参数，以便在内部数据结构中跟踪订阅。

6. **void RMWPublisherEvent::untrack_unique_subscription(eprosima::fastrtps::rtps::GUID_t guid)**：此函数用于取消跟踪唯一订阅。它接收一个 GUID 作为参数，并从内部数据结构中移除对应的订阅。

7. **size_t RMWPublisherEvent::subscription_count() const**：此函数返回当前跟踪的订阅数量。这有助于了解发布者与多少个订阅者进行通信。

8. **void RMWPublisherEvent::update_deadline(uint32_t total_count, uint32_t total_count_change)**：此函数用于更新截止日期事件。它接收两个参数：总计数（`total_count`）和总计数变化（`total_count_change`）。当发布者未能在指定时间内发送消息时，将触发截止日期事件。

9. **void RMWPublisherEvent::update_liveliness_lost(uint32_t total_count, uint32_t total_count_change)**：此函数用于更新失活事件。它接收两个参数：总计数（`total_count`）和总计数变化（`total_count_change`）。当发布者未能在指定时间内维持其活跃状态时，将触发失活事件。

10. **void RMWPublisherEvent::update_offered_incompatible_qos(eprosima::fastdds::dds::QosPolicyId_t last_policy_id, uint32_t total_count, uint32_t total_count_change)**：此函数用于更新提供的不兼容 QoS 事件。它接收三个参数：最后一次策略 ID（`last_policy_id`）、总计数（`total_count`）和总计数变化（`total_count_change`）。当发布者提供的 QoS 策略与订阅者的 QoS 策略不兼容时，将触发此事件。

11. **void RMWPublisherEvent::update_inconsistent_topic(uint32_t total_count, uint32_t total_count_change)**：此函数用于更新不一致主题事件。它接收两个参数：总计数（`total_count`）和总计数变化（`total_count_change`）。当发布者与订阅者之间的主题不一致时，将触发此事件。

12. **void RMWPublisherEvent::update_matched(int32_t total_count, int32_t total_count_change, int32_t current_count, int32_t current_count_change)**：此函数用于更新匹配事件。它接收四个参数：总计数（`total_count`）、总计数变化（`total_count_change`）、当前计数（`current_count`）和当前计数变化（`current_count_change`）。当发布者与订阅者匹配或取消匹配时，将触发此事件。

13. **void RMWPublisherEvent::trigger_event(rmw_event_type_t event_type)**：此函数用于触发指定类型的事件。它接收一个参数：事件类型（`event_type`）。

通过这些函数，`RMWPublisherEvent` 类可以处理与发布者相关的各种事件，如订阅跟踪、截止日期、失活、不兼容 QoS 等。这有助于实现 ROS2 发布者与订阅者之间的通信和协调。
