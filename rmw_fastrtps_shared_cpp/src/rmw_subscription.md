```cpp
#include <string>
#include <utility>

#include "fastdds/dds/subscriber/DataReader.hpp"
#include "fastdds/dds/subscriber/qos/DataReaderQos.hpp"
#include "rcpputils/scope_exit.hpp"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_subscriber_info.hpp"
#include "rmw_fastrtps_shared_cpp/namespace_prefix.hpp"
#include "rmw_fastrtps_shared_cpp/qos.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"
#include "rmw_fastrtps_shared_cpp/subscription.hpp"
#include "rmw_fastrtps_shared_cpp/utils.hpp"
```

```cpp
rmw_ret_t __rmw_destroy_subscription(
    const char *identifier,
    const rmw_node_t *node,
    rmw_subscription_t *subscription,
    bool reset_cft) {}

rmw_ret_t __rmw_subscription_count_matched_publishers(
    const rmw_subscription_t *subscription, size_t *publisher_count) {}

rmw_ret_t __rmw_subscription_get_actual_qos(
    const rmw_subscription_t *subscription, rmw_qos_profile_t *qos) {}

rmw_ret_t __rmw_subscription_set_content_filter(
    rmw_subscription_t *subscription, const rmw_subscription_content_filter_options_t *options) {}

rmw_ret_t __rmw_subscription_get_content_filter(
    const rmw_subscription_t *subscription,
    rcutils_allocator_t *allocator,
    rmw_subscription_content_filter_options_t *options) {}

rmw_ret_t __rmw_subscription_set_on_new_message_callback(
    rmw_subscription_t *rmw_subscription, rmw_event_callback_t callback, const void *user_data) {}
```

这份代码是 ROS2 项目中 rmw_fastrtps 的一部分，主要包含了与订阅者（Subscription）相关的功能实现。下面是各个函数的功能和相互关系的解释：

1. `__rmw_destroy_subscription`：销毁一个已经创建的订阅者。它需要传入订阅者所属节点（Node）、订阅者对象本身以及一个布尔值，表示是否重置内容过滤器（Content Filter Topic，CFT）。

2. `__rmw_subscription_count_matched_publishers`：获取与当前订阅者匹配的发布者（Publisher）数量。它需要传入订阅者对象，返回匹配的发布者数量。

3. `__rmw_subscription_get_actual_qos`：获取订阅者的实际 QoS（Quality of Service，服务质量）配置。它需要传入订阅者对象，返回实际的 QoS 配置。

4. `__rmw_subscription_set_content_filter`：为订阅者设置内容过滤器。它需要传入订阅者对象和一个指向内容过滤器选项的指针。此函数将根据提供的选项设置订阅者的内容过滤器。

5. `__rmw_subscription_get_content_filter`：获取订阅者的内容过滤器。它需要传入订阅者对象、一个分配器（用于分配内存）以及一个指向内容过滤器选项的指针。此函数将返回订阅者当前的内容过滤器选项。

6. `__rmw_subscription_set_on_new_message_callback`：为订阅者设置新消息回调函数。它需要传入订阅者对象、回调函数以及一个指向用户数据的指针。当订阅者收到新消息时，将触发此回调函数。

这些函数之间的关系主要体现在它们都是针对订阅者对象进行操作的。例如，`__rmw_destroy_subscription` 用于销毁订阅者，而 `__rmw_subscription_count_matched_publishers` 则用于获取与该订阅者匹配的发布者数量。这些函数共同实现了 ROS2 订阅者的功能，使得用户可以方便地创建、管理和销毁订阅者，以及与发布者进行通信。
