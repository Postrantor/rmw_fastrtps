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

#include "fastdds/dds/core/condition/GuardCondition.hpp"
#include "fastdds/dds/core/condition/WaitSet.hpp"
#include "fastdds/dds/subscriber/DataReader.hpp"
#include "rcutils/macros.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/custom_client_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_service_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_subscriber_info.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "types/event_types.hpp"

namespace rmw_fastrtps_shared_cpp {
/// 检查实体集合中的任何条件是否有触发条件。
/// Check if any condition in the set of entities has a triggered condition.
/**
 * 如果在等待之前有任何条件被触发，那么我们可以跳过一些设置、拆卸和实际等待。
 * If any condition is triggered before waiting, then we can skip some set-up, tear-down, and the
 * actual wait.
 *
 * \param[in] subscriptions 要检查的订阅
 * \param[in] guard_conditions 要检查的保护条件
 * \param[in] services 要检查的服务
 * \param[in] clients 要检查的客户端
 * \param[in] events 要检查的事件
 * \return 如果有任何条件被触发，则返回 true，否则返回 false
 */
static bool has_triggered_condition(
    rmw_subscriptions_t *subscriptions,
    rmw_guard_conditions_t *guard_conditions,
    rmw_services_t *services,
    rmw_clients_t *clients,
    rmw_events_t *events) {
  // `get_first_untaken_info` 相对于检查保护条件更昂贵，
  // 因此应尽可能跳过。
  // Subscriptions, services, and clients typically have additional waitables
  // connected (eg receive event or intraprocess waitable), so we can hit those first
  // before having to query SampleInfo.
  if (guard_conditions) {
    for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
      void *data = guard_conditions->guard_conditions[i];
      auto guard_condition = static_cast<eprosima::fastdds::dds::GuardCondition *>(data);
      // 如果保护条件的触发值为 true，则返回 true
      if (guard_condition->get_trigger_value()) {
        return true;
      }
    }
  }

  if (events) {
    for (size_t i = 0; i < events->event_count; ++i) {
      auto event = static_cast<rmw_event_t *>(events->events[i]);
      auto custom_event_info = static_cast<CustomEventInfo *>(event->data);
      // 如果事件监听器的状态条件触发值为 true 或者事件类型对应的事件保护触发值为 true，则返回 true
      if (custom_event_info->get_listener()->get_statuscondition().get_trigger_value() ||
          custom_event_info->get_listener()
              ->get_event_guard(event->event_type)
              .get_trigger_value()) {
        return true;
      }
    }
  }

  if (subscriptions) {
    for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
      void *data = subscriptions->subscribers[i];
      auto custom_subscriber_info = static_cast<CustomSubscriberInfo *>(data);
      eprosima::fastdds::dds::SampleInfo sample_info;
      // 如果获取第一个未处理的 SampleInfo 成功，则返回 true
      if (ReturnCode_t::RETCODE_OK ==
          custom_subscriber_info->data_reader_->get_first_untaken_info(&sample_info)) {
        return true;
      }
    }
  }

  if (clients) {
    for (size_t i = 0; i < clients->client_count; ++i) {
      void *data = clients->clients[i];
      auto custom_client_info = static_cast<CustomClientInfo *>(data);
      eprosima::fastdds::dds::SampleInfo sample_info;
      // 如果获取第一个未处理的响应 SampleInfo 成功，则返回 true
      if (ReturnCode_t::RETCODE_OK ==
          custom_client_info->response_reader_->get_first_untaken_info(&sample_info)) {
        return true;
      }
    }
  }

  if (services) {
    for (size_t i = 0; i < services->service_count; ++i) {
      void *data = services->services[i];
      auto custom_service_info = static_cast<CustomServiceInfo *>(data);
      eprosima::fastdds::dds::SampleInfo sample_info;
      // 如果获取第一个未处理的请求 SampleInfo 成功，则返回 true
      if (ReturnCode_t::RETCODE_OK ==
          custom_service_info->request_reader_->get_first_untaken_info(&sample_info)) {
        return true;
      }
    }
  }
  // 如果没有任何条件被触发，则返回 false
  return false;
}

/**
 * @brief 等待函数，用于等待订阅、守卫条件、服务、客户端和事件的触发。
 *        Wait function, used to wait for the triggering of subscriptions,
 *        guard conditions, services, clients, and events.
 *
 * @param[in] identifier 实现标识符。Implementation identifier.
 * @param[in] subscriptions 订阅集合。Subscriptions set.
 * @param[in] guard_conditions 守卫条件集合。Guard conditions set.
 * @param[in] services 服务集合。Services set.
 * @param[in] clients 客户端集合。Clients set.
 * @param[in] events 事件集合。Events set.
 * @param[in] wait_set 等待集合。Wait set.
 * @param[in] wait_timeout 等待超时时间。Wait timeout duration.
 *
 * @return rmw_ret_t 返回操作结果。Return the operation result.
 */
rmw_ret_t __rmw_wait(
    const char *identifier,
    rmw_subscriptions_t *subscriptions,
    rmw_guard_conditions_t *guard_conditions,
    rmw_services_t *services,
    rmw_clients_t *clients,
    rmw_events_t *events,
    rmw_wait_set_t *wait_set,
    const rmw_time_t *wait_timeout) {
  // 检查是否有无效参数。Check for invalid arguments.
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_INVALID_ARGUMENT);
  // 检查是否有不正确的 RMW 实现。Check for incorrect RMW implementation.
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查 wait_set 是否为空。Check if wait_set is null.
  RMW_CHECK_ARGUMENT_FOR_NULL(wait_set, RMW_RET_INVALID_ARGUMENT);
  // 检查类型标识符是否匹配。Check if type identifiers match.
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      wait set handle, wait_set->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 如果 wait_set_info 为 nullptr，可能是以下三种情况之一：
  // - 等待集无效。调用者未遵循前置条件。
  // - 实现逻辑错误。绝对不是我们想要处理的正常错误。
  // - 堆损坏。
  // 在这三种情况下，最好尽早崩溃。
  // If wait_set_info is ever nullptr, it can only mean one of three things:
  // - Wait set is invalid. Caller did not respect preconditions.
  // - Implementation is logically broken. Definitely not something we want to treat as a normal
  // error.
  // - Heap is corrupt.
  // In all three cases, it's better if this crashes soon enough.
  auto fastdds_wait_set = static_cast<eprosima::fastdds::dds::WaitSet *>(wait_set->data);

  // 在等待之前检查是否有任何条件已经为 true，
  // 这样可以跳过附加/分离的一些工作。
  // Check if any conditions are already true before waiting,
  // allowing us to skip some work of attaching/detaching.
  bool skip_wait =
      has_triggered_condition(subscriptions, guard_conditions, services, clients, events);
  // 等待结果。Wait result.
  bool wait_result = true;
  // 已附加条件的向量。Vector of attached conditions.
  std::vector<eprosima::fastdds::dds::Condition *> attached_conditions;

  /*!
   * \brief 该函数用于处理不同类型的条件并将它们添加到等待集中，以便在需要时进行等待。
   *        This function is used to handle different types of conditions and add them to the
   * waitset for waiting when needed.
   *
   * \param[in] skip_wait 是否跳过等待。Whether to skip waiting.
   * \param[in] subscriptions 订阅者信息。Subscription information.
   * \param[in] clients 客户端信息。Client information.
   * \param[in] services 服务信息。Service information.
   * \param[in] events 事件信息。Event information.
   * \param[in] guard_conditions 守护条件信息。Guard condition information.
   */
  if (!skip_wait) {
    // 如果需要等待（没有触发条件），收集要添加到等待集的条件。
    // In the case that a wait is needed (no triggered conditions), gather the conditions to be
    // added to the waitset.
    if (subscriptions) {
      for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
        void *data = subscriptions->subscribers[i];
        // 将订阅者数据转换为自定义订阅者信息。
        // Cast subscriber data to custom subscriber info.
        auto custom_subscriber_info = static_cast<CustomSubscriberInfo *>(data);
        // 将状态条件添加到附加条件列表中。
        // Add the status condition to the attached conditions list.
        attached_conditions.push_back(&custom_subscriber_info->data_reader_->get_statuscondition());
      }
    }

    if (clients) {
      for (size_t i = 0; i < clients->client_count; ++i) {
        void *data = clients->clients[i];
        // 将客户端数据转换为自定义客户端信息。
        // Cast client data to custom client info.
        auto custom_client_info = static_cast<CustomClientInfo *>(data);
        // 将状态条件添加到附加条件列表中。
        // Add the status condition to the attached conditions list.
        attached_conditions.push_back(&custom_client_info->response_reader_->get_statuscondition());
      }
    }

    if (services) {
      for (size_t i = 0; i < services->service_count; ++i) {
        void *data = services->services[i];
        // 将服务数据转换为自定义服务信息。
        // Cast service data to custom service info.
        auto custom_service_info = static_cast<CustomServiceInfo *>(data);
        // 将状态条件添加到附加条件列表中。
        // Add the status condition to the attached conditions list.
        attached_conditions.push_back(&custom_service_info->request_reader_->get_statuscondition());
      }
    }

    if (events) {
      for (size_t i = 0; i < events->event_count; ++i) {
        auto event = static_cast<rmw_event_t *>(events->events[i]);
        // 将事件数据转换为自定义事件信息。
        // Cast event data to custom event info.
        auto custom_event_info = static_cast<CustomEventInfo *>(event->data);
        // 将状态条件和事件保护添加到附加条件列表中。
        // Add the status condition and event guard to the attached conditions list.
        attached_conditions.push_back(&custom_event_info->get_listener()->get_statuscondition());
        attached_conditions.push_back(
            &custom_event_info->get_listener()->get_event_guard(event->event_type));
      }
    }

    if (guard_conditions) {
      for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
        void *data = guard_conditions->guard_conditions[i];
        // 将守护条件数据转换为 Fast DDS 守护条件。
        // Cast guard condition data to Fast DDS guard condition.
        attached_conditions.push_back(static_cast<eprosima::fastdds::dds::GuardCondition *>(data));
      }
    }

    /**
     * @brief 为 wait set 添加所有条件。
     * @details 在 ros2 项目中 rmw_fastrtps_cpp 层相关的代码，用于将所有条件附加到 wait set。
     *          当上游具有附加一组条件的功能时，请切换到该 API。
     *
     * Attach all of the conditions to the wait set.
     * In the context of the ros2 project in the rmw_fastrtps_cpp layer, this code is used to attach
     * all conditions to the wait set.
     * TODO(mjcarroll): When upstream has the ability to attach a vector of conditions, switch to
     * that API.
     */
    for (auto &condition : attached_conditions) {
      // 将每个条件附加到 fastdds_wait_set
      // Attach each condition to the fastdds_wait_set
      fastdds_wait_set->attach_condition(*condition);
    }

    // 设置超时时间
    // Set the timeout duration
    Duration_t timeout =
        (wait_timeout)
            // 如果 wait_timeout 存在，则使用给定的秒和纳秒值创建 Duration_t 对象
            // If wait_timeout exists, create a Duration_t object with the given seconds and
            // nanoseconds values
            ? Duration_t{static_cast<int32_t>(wait_timeout->sec), static_cast<uint32_t>(wait_timeout->nsec)}
            // 否则，使用 eprosima::fastrtps::c_TimeInfinite 作为超时时间
            // Otherwise, use eprosima::fastrtps::c_TimeInfinite as the timeout duration
            : eprosima::fastrtps::c_TimeInfinite;

    eprosima::fastdds::dds::ConditionSeq triggered_conditions;
    // 等待触发的条件
    // Wait for triggered conditions
    ReturnCode_t ret_code = fastdds_wait_set->wait(triggered_conditions, timeout);
    // 检查 wait 的结果
    // Check the result of the wait
    wait_result = (ret_code == ReturnCode_t::RETCODE_OK);

    /**
     * @brief 从 wait set 中分离所有条件。
     * @details 在 ros2 项目中 rmw_fastrtps_cpp 层相关的代码，用于将所有条件从 wait set 中分离。
     *          当上游具有分离一组条件的功能时，请切换到该 API。
     *
     * Detach all of the conditions from the wait set.
     * In the context of the ros2 project in the rmw_fastrtps_cpp layer, this code is used to detach
     * all conditions from the wait set.
     * TODO(mjcarroll): When upstream has the ability to detach a vector of conditions, switch to
     * that API.
     */
    for (auto &condition : attached_conditions) {
      // 将每个条件从 fastdds_wait_set 中分离
      // Detach each condition from the fastdds_wait_set
      fastdds_wait_set->detach_condition(*condition);
    }
  }

  /**
   * @brief 检查等待结果，并根据情况标记准备好的实体。
   * @brief Check the results of the wait, and mark ready entities accordingly.
   */
  // 检查等待结果，并根据情况标记准备好的实体。
  // Check the results of the wait, and mark ready entities accordingly.
  if (subscriptions) {
    for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
      // 获取订阅者数据
      // Get subscriber data
      void *data = subscriptions->subscribers[i];
      // 将订阅者数据转换为 CustomSubscriberInfo 类型
      // Cast subscriber data to CustomSubscriberInfo type
      auto custom_subscriber_info = static_cast<CustomSubscriberInfo *>(data);

      // 创建 SampleInfo 对象
      // Create a SampleInfo object
      eprosima::fastdds::dds::SampleInfo sample_info;
      // 检查是否有未处理的消息
      // Check if there are any untaken messages
      if (ReturnCode_t::RETCODE_OK !=
          custom_subscriber_info->data_reader_->get_first_untaken_info(&sample_info)) {
        // 如果没有未处理的消息，将订阅者设置为 0
        // If there are no untaken messages, set the subscriber to 0
        subscriptions->subscribers[i] = 0;
      }
    }
  }

  if (clients) {
    for (size_t i = 0; i < clients->client_count; ++i) {
      // 获取客户端数据
      // Get client data
      void *data = clients->clients[i];
      // 将客户端数据转换为 CustomClientInfo 类型
      // Cast client data to CustomClientInfo type
      auto custom_client_info = static_cast<CustomClientInfo *>(data);

      // 创建 SampleInfo 对象
      // Create a SampleInfo object
      eprosima::fastdds::dds::SampleInfo sample_info;
      // 检查是否有未处理的响应
      // Check if there are any untaken responses
      if (ReturnCode_t::RETCODE_OK !=
          custom_client_info->response_reader_->get_first_untaken_info(&sample_info)) {
        // 如果没有未处理的响应，将客户端设置为 0
        // If there are no untaken responses, set the client to 0
        clients->clients[i] = 0;
      }
    }
  }

  if (services) {
    for (size_t i = 0; i < services->service_count; ++i) {
      // 获取服务数据
      // Get service data
      void *data = services->services[i];
      // 将服务数据转换为 CustomServiceInfo 类型
      // Cast service data to CustomServiceInfo type
      auto custom_service_info = static_cast<CustomServiceInfo *>(data);

      // 创建 SampleInfo 对象
      // Create a SampleInfo object
      eprosima::fastdds::dds::SampleInfo sample_info;
      // 检查是否有未处理的请求
      // Check if there are any untaken requests
      if (ReturnCode_t::RETCODE_OK !=
          custom_service_info->request_reader_->get_first_untaken_info(&sample_info)) {
        // 如果没有未处理的请求，将服务设置为 0
        // If there are no untaken requests, set the service to 0
        services->services[i] = 0;
      }
    }
  }

  // 如果存在事件 (If there are events)
  if (events) {
    // 遍历事件数组 (Iterate through the event array)
    for (size_t i = 0; i < events->event_count; ++i) {
      // 获取事件并进行类型转换 (Get the event and perform type casting)
      auto event = static_cast<rmw_event_t *>(events->events[i]);
      // 获取自定义事件信息并进行类型转换 (Get custom event info and perform type casting)
      auto custom_event_info = static_cast<CustomEventInfo *>(event->data);

      // 获取状态条件 (Get the status condition)
      eprosima::fastdds::dds::StatusCondition &status_condition =
          custom_event_info->get_listener()->get_statuscondition();

      // 获取守卫条件 (Get the guard condition)
      eprosima::fastdds::dds::GuardCondition &guard_condition =
          custom_event_info->get_listener()->get_event_guard(event->event_type);

      // 初始化活动状态为 false (Initialize active state as false)
      bool active = false;

      // 如果等待结果为真 (If wait result is true)
      if (wait_result) {
        // 获取实体 (Get the entity)
        eprosima::fastdds::dds::Entity *entity = status_condition.get_entity();
        // 获取状态变更 (Get the status changes)
        eprosima::fastdds::dds::StatusMask changed_statuses = entity->get_status_changes();
        // 如果状态变更与事件类型匹配 (If the status change matches the event type)
        if (changed_statuses.is_active(
                rmw_fastrtps_shared_cpp::internal::rmw_event_to_dds_statusmask(
                    event->event_type))) {
          // 设置活动状态为真 (Set active state as true)
          active = true;
        }

        // 如果守卫条件的触发值为真 (If the guard condition's trigger value is true)
        if (guard_condition.get_trigger_value()) {
          // 设置活动状态为真 (Set active state as true)
          active = true;
          // 设置守卫条件的触发值为假 (Set the guard condition's trigger value as false)
          guard_condition.set_trigger_value(false);
        }
      }

      // 如果活动状态为假 (If active state is false)
      if (!active) {
        // 将事件数组中对应位置置零 (Set the corresponding position in the event array to zero)
        events->events[i] = 0;
      }
    }
  }

  // 如果存在守卫条件 (If there are guard conditions)
  if (guard_conditions) {
    // 遍历守卫条件数组 (Iterate through the guard condition array)
    for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
      // 获取数据 (Get the data)
      void *data = guard_conditions->guard_conditions[i];
      // 进行类型转换 (Perform type casting)
      auto condition = static_cast<eprosima::fastdds::dds::GuardCondition *>(data);
      // 如果守卫条件的触发值为假 (If the guard condition's trigger value is false)
      if (!condition->get_trigger_value()) {
        // 将守卫条件数组中对应位置置零 (Set the corresponding position in the guard condition array
        // to zero)
        guard_conditions->guard_conditions[i] = 0;
      }
      // 设置守卫条件的触发值为假 (Set the guard condition's trigger value as false)
      condition->set_trigger_value(false);
    }
  }

  return (skip_wait || wait_result) ? RMW_RET_OK : RMW_RET_TIMEOUT;
}
}  // namespace rmw_fastrtps_shared_cpp
