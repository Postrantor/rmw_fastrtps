// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#include "rmw_fastrtps_cpp/init_rmw_context_impl.hpp"

#include <cassert>
#include <memory>

#include "rmw/error_handling.h"
#include "rmw/init.h"
#include "rmw/qos_profiles.h"
#include "rmw_dds_common/context.hpp"
#include "rmw_dds_common/msg/participant_entities_info.hpp"
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_cpp/publisher.hpp"
#include "rmw_fastrtps_cpp/subscription.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/listener_thread.hpp"
#include "rmw_fastrtps_shared_cpp/participant.hpp"
#include "rmw_fastrtps_shared_cpp/publisher.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"
#include "rmw_fastrtps_shared_cpp/subscription.hpp"
#include "rosidl_typesupport_cpp/message_type_support.hpp"

using rmw_dds_common::msg::ParticipantEntitiesInfo;

/**
 * @brief 初始化上下文实现函数 (Initialize context implementation function)
 *
 * @param[in,out] context 指向待初始化的上下文指针 (Pointer to the context to be initialized)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
static rmw_ret_t init_context_impl(rmw_context_t *context) {
  // 获取默认发布者选项 (Get default publisher options)
  // Get default publisher options
  rmw_publisher_options_t publisher_options = rmw_get_default_publisher_options();

  // 获取默认订阅者选项 (Get default subscription options)
  // Get default subscription options
  rmw_subscription_options_t subscription_options = rmw_get_default_subscription_options();

  // 当前在fastrtps中未实现此功能 (This is currently not implemented in fastrtps)
  // This is currently not implemented in fastrtps
  subscription_options.ignore_local_publications = true;

  // 创建一个新的rmw_dds_common::Context对象，并使用nothrow避免抛出异常 (Create a new
  // rmw_dds_common::Context object and use nothrow to avoid throwing exceptions) Create a new
  // rmw_dds_common::Context object and use nothrow to avoid throwing exceptions
  std::unique_ptr<rmw_dds_common::Context> common_context(new (std::nothrow)
                                                              rmw_dds_common::Context());

  // 检查common_context是否为空，如果为空则返回RMW_RET_BAD_ALLOC (Check if common_context is null,
  // if null then return RMW_RET_BAD_ALLOC) Check if common_context is null, if null then return
  // RMW_RET_BAD_ALLOC
  if (!common_context) {
    return RMW_RET_BAD_ALLOC;
  }

  /**
   * @brief 创建一个具有自定义参与者信息的唯一指针，并在销毁时调用相应的销毁函数
   * Create a unique_ptr with custom participant info and call the corresponding destroy function on
   * destruction
   */
  std::unique_ptr<CustomParticipantInfo, std::function<void(CustomParticipantInfo *)>>
      participant_info(
          rmw_fastrtps_shared_cpp::create_participant(
              eprosima_fastrtps_identifier, context->actual_domain_id,
              &context->options.security_options, &context->options.discovery_options,
              context->options.enclave, common_context.get()),
          [&](CustomParticipantInfo *participant_info) {
            if (RMW_RET_OK != rmw_fastrtps_shared_cpp::destroy_participant(participant_info)) {
              RCUTILS_SAFE_FWRITE_TO_STDERR(
                  "Failed to destroy participant after function: '" RCUTILS_STRINGIFY(
                      __function__) "' failed.\n");
            }
          });

  // 如果未能创建参与者信息，则返回错误代码
  // Return error code if participant info creation failed
  if (!participant_info) {
    return RMW_RET_BAD_ALLOC;
  }

  // 初始化 QoS (Quality of Service) 配置文件为默认值
  // Initialize QoS (Quality of Service) profile to default values
  rmw_qos_profile_t qos = rmw_qos_profile_default;

  // 设置 QoS 配置选项
  // Set QoS configuration options
  qos.avoid_ros_namespace_conventions = true;
  qos.history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;
  qos.depth = 1;
  qos.durability = RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL;
  qos.reliability = RMW_QOS_POLICY_RELIABILITY_RELIABLE;

  /**
   * @brief 创建一个具有自定义发布者信息的唯一指针，并在销毁时调用相应的销毁函数
   * Create a unique_ptr with custom publisher info and call the corresponding destroy function on
   * destruction
   */
  std::unique_ptr<rmw_publisher_t, std::function<void(rmw_publisher_t *)>> publisher(
      rmw_fastrtps_cpp::create_publisher(
          participant_info.get(),
          rosidl_typesupport_cpp::get_message_type_support_handle<ParticipantEntitiesInfo>(),
          "ros_discovery_info", &qos, &publisher_options),
      [&](rmw_publisher_t *pub) {
        if (RMW_RET_OK != rmw_fastrtps_shared_cpp::destroy_publisher(
                              eprosima_fastrtps_identifier, participant_info.get(), pub)) {
          RCUTILS_SAFE_FWRITE_TO_STDERR(
              "Failed to destroy publisher after function: '" RCUTILS_STRINGIFY(
                  __function__) "' failed.\n");
        }
      });

  // 如果未能创建发布者，则返回错误代码
  // Return error code if publisher creation failed
  if (!publisher) {
    return RMW_RET_BAD_ALLOC;
  }

  // 如果我们支持 keyed topics，这里可以使用 KEEP_LAST 和深度 1。
  // If we would have support for keyed topics, this could be KEEP_LAST and depth 1.
  qos.history = RMW_QOS_POLICY_HISTORY_KEEP_ALL;

  // 创建一个唯一指针来管理 rmw_subscription_t，使用 std::function 作为删除器
  // Create a unique_ptr to manage rmw_subscription_t, using std::function as the deleter
  std::unique_ptr<rmw_subscription_t, std::function<void(rmw_subscription_t *)>> subscription(
      // 调用 rmw_fastrtps_cpp::create_subscription 函数创建订阅
      // Call rmw_fastrtps_cpp::create_subscription function to create the subscription
      rmw_fastrtps_cpp::create_subscription(
          participant_info.get(),
          rosidl_typesupport_cpp::get_message_type_support_handle<ParticipantEntitiesInfo>(),
          "ros_discovery_info", &qos, &subscription_options,
          false),  // 我们的 fastrtps 类型支持不支持 keyed topics
                   // our fastrtps typesupport doesn't support keyed topics
      [&](rmw_subscription_t *sub) {
        if (RMW_RET_OK != rmw_fastrtps_shared_cpp::destroy_subscription(
                              eprosima_fastrtps_identifier, participant_info.get(), sub)) {
          RMW_SAFE_FWRITE_TO_STDERR(
              "Failed to destroy subscription after function: '" RCUTILS_STRINGIFY(
                  __function__) "' failed.\n");
        }
      });
  if (!subscription) {
    return RMW_RET_BAD_ALLOC;
  }

  // 创建一个唯一指针来管理 rmw_guard_condition_t，使用 std::function 作为删除器
  // Create a unique_ptr to manage rmw_guard_condition_t, using std::function as the deleter
  std::unique_ptr<rmw_guard_condition_t, std::function<void(rmw_guard_condition_t *)>>
      graph_guard_condition(
          // 调用 rmw_fastrtps_shared_cpp::__rmw_create_guard_condition 函数创建 guard condition
          // Call rmw_fastrtps_shared_cpp::__rmw_create_guard_condition function to create the guard
          // condition
          rmw_fastrtps_shared_cpp::__rmw_create_guard_condition(eprosima_fastrtps_identifier),
          [&](rmw_guard_condition_t *p) {
            rmw_ret_t ret = rmw_fastrtps_shared_cpp::__rmw_destroy_guard_condition(p);
            if (ret != RMW_RET_OK) {
              RMW_SAFE_FWRITE_TO_STDERR(
                  "Failed to destroy guard condition after function: '" RCUTILS_STRINGIFY(
                      __function__) "' failed.\n");
            }
          });
  if (!graph_guard_condition) {
    return RMW_RET_BAD_ALLOC;
  }

  /*!
   * \brief 创建一个 rmw_gid_t 对象，用于存储 Fast RTPS 参与者的 GUID。
   * Create an rmw_gid_t object to store the GUID of a Fast RTPS participant.
   */
  common_context->gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
      eprosima_fastrtps_identifier, participant_info->participant_->guid());

  /*!
   * \brief 将 publisher 指针存储到 common_context 中。
   * Store the publisher pointer in the common_context.
   */
  common_context->pub = publisher.get();

  /*!
   * \brief 将 subscription 指针存储到 common_context 中。
   * Store the subscription pointer in the common_context.
   */
  common_context->sub = subscription.get();

  /*!
   * \brief 将 graph_guard_condition 指针存储到 common_context 中。
   * Store the graph_guard_condition pointer in the common_context.
   */
  common_context->graph_guard_condition = graph_guard_condition.get();

  /*!
   * \brief 将 common_context 指针存储到 context 的 impl 结构中。
   * Store the common_context pointer in the impl structure of the context.
   */
  context->impl->common = common_context.get();

  /*!
   * \brief 将 participant_info 指针存储到 context 的 impl 结构中。
   * Store the participant_info pointer in the impl structure of the context.
   */
  context->impl->participant_info = participant_info.get();

  /*!
   * \brief 运行监听器线程。
   * Run the listener thread.
   */
  rmw_ret_t ret = rmw_fastrtps_shared_cpp::run_listener_thread(context);
  if (RMW_RET_OK != ret) {
    return ret;
  }

  /*!
   * \brief 设置图形缓存的更改回调函数。
   * Set the on_change callback for the graph cache.
   */
  common_context->graph_cache.set_on_change_callback(
      [guard_condition = graph_guard_condition.get()]() {
        rmw_fastrtps_shared_cpp::__rmw_trigger_guard_condition(
            eprosima_fastrtps_identifier, guard_condition);
      });

  /*!
   * \brief 将参与者添加到图形缓存中。
   * Add the participant to the graph cache.
   */
  common_context->graph_cache.add_participant(common_context->gid, context->options.enclave);

  /*!
   * \brief 释放 graph_guard_condition、publisher、subscription、common_context 和 participant_info
   * 的所有权。 Release ownership of graph_guard_condition, publisher, subscription, common_context,
   * and participant_info.
   */
  graph_guard_condition.release();
  publisher.release();
  subscription.release();
  common_context.release();
  participant_info.release();

  /*!
   * \brief 返回成功的 rmw_ret_t 值。
   * Return the successful rmw_ret_t value.
   */
  return RMW_RET_OK;
}

/**
 * @brief 增加 rmw_context_t 实例的引用计数，并在需要时初始化实现。
 *        Increment reference count for an rmw_context_t instance and initialize implementation if
 * needed.
 *
 * @param context 指向一个 rmw_context_t 实例的指针。
 *                Pointer to an rmw_context_t instance.
 * @return rmw_ret_t 返回操作结果，成功返回 RMW_RET_OK，失败则返回对应的错误码。
 *                  Return the operation result, success returns RMW_RET_OK, failure returns
 * corresponding error code.
 */
rmw_ret_t rmw_fastrtps_cpp::increment_context_impl_ref_count(rmw_context_t *context) {
  // 断言：确保 context 不为空
  // Assert: Ensure that context is not null
  assert(context);

  // 断言：确保 context->impl 不为空
  // Assert: Ensure that context->impl is not null
  assert(context->impl);

  // 创建互斥锁保护，以防止多线程中的数据竞争
  // Create a mutex lock guard to prevent data race in multithreading
  std::lock_guard<std::mutex> guard(context->impl->mutex);

  // 如果引用计数为0，则初始化 context 实现
  // If the reference count is 0, initialize the context implementation
  if (!context->impl->count) {
    rmw_ret_t ret = init_context_impl(context);
    // 如果初始化失败，返回相应的错误码
    // If initialization fails, return the corresponding error code
    if (RMW_RET_OK != ret) {
      return ret;
    }
  }

  // 增加引用计数
  // Increase the reference count
  context->impl->count++;

  // 返回成功操作的结果码
  // Return the result code for successful operation
  return RMW_RET_OK;
}
