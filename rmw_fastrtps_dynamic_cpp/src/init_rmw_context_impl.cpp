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

#include "rmw_fastrtps_dynamic_cpp/init_rmw_context_impl.hpp"

#include <cassert>
#include <memory>

#include "rmw/error_handling.h"
#include "rmw/init.h"
#include "rmw/qos_profiles.h"
#include "rmw_dds_common/context.hpp"
#include "rmw_dds_common/msg/participant_entities_info.hpp"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_dynamic_cpp/publisher.hpp"
#include "rmw_fastrtps_dynamic_cpp/subscription.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/listener_thread.hpp"
#include "rmw_fastrtps_shared_cpp/participant.hpp"
#include "rmw_fastrtps_shared_cpp/publisher.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"
#include "rmw_fastrtps_shared_cpp/subscription.hpp"
#include "rosidl_typesupport_cpp/message_type_support.hpp"

using rmw_dds_common::msg::ParticipantEntitiesInfo;

/**
 * @brief 初始化上下文实现 (Initialize the context implementation)
 *
 * @param[in] context 指向要初始化的 rmw_context_t 结构体的指针 (Pointer to the rmw_context_t
 * structure to be initialized)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
static rmw_ret_t init_context_impl(rmw_context_t *context) {
  // 获取默认发布者、订阅者选项
  rmw_publisher_options_t publisher_options = rmw_get_default_publisher_options();
  rmw_subscription_options_t subscription_options = rmw_get_default_subscription_options();

  // 当前在 fastrtps 中尚未实现此功能
  // This is currently not implemented in fastrtps
  subscription_options.ignore_local_publications = true;

  // 创建一个 rmw_dds_common::Context 类型的智能指针，用于存储公共上下文信息
  // Create a unique_ptr of type rmw_dds_common::Context to store common context information
  std::unique_ptr<rmw_dds_common::Context> common_context(new (std::nothrow)
                                                              rmw_dds_common::Context());
  // 如果 common_context 为空，则返回 RMW_RET_BAD_ALLOC 错误
  // If common_context is null, return RMW_RET_BAD_ALLOC error
  if (!common_context) {
    return RMW_RET_BAD_ALLOC;
  }

  // 创建一个 CustomParticipantInfo 类型的智能指针 participant_info，并在其析构时调用
  // destroy_participant 函数 Create a unique_ptr of type CustomParticipantInfo named
  // participant_info, and call the destroy_participant function when it's destructed
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
  // 如果 participant_info 为空，则返回 RMW_RET_BAD_ALLOC 错误
  // If participant_info is null, return RMW_RET_BAD_ALLOC error
  if (!participant_info) {
    return RMW_RET_BAD_ALLOC;
  }

  // 创建一个默认的 rmw_qos_profile_t 类型的变量 qos
  // Create a default rmw_qos_profile_t variable named qos
  rmw_qos_profile_t qos = rmw_qos_profile_default;

  // 设置 qos 的一些属性值
  // Set some attribute values for qos
  qos.avoid_ros_namespace_conventions = true;
  qos.history = RMW_QOS_POLICY_HISTORY_KEEP_LAST;
  qos.depth = 1;
  qos.durability = RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL;
  qos.reliability = RMW_QOS_POLICY_RELIABILITY_RELIABLE;

  // 创建一个 rmw_publisher_t 类型的智能指针 publisher，并在其析构时调用 destroy_publisher 函数
  // Create a unique_ptr of type rmw_publisher_t named publisher, and call the destroy_publisher
  // function when it's destructed
  std::unique_ptr<rmw_publisher_t, std::function<void(rmw_publisher_t *)>> publisher(
      rmw_fastrtps_dynamic_cpp::create_publisher(
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
  // 如果 publisher 为空，则返回 RMW_RET_BAD_ALLOC 错误
  // If publisher is null, return RMW_RET_BAD_ALLOC error
  if (!publisher) {
    return RMW_RET_BAD_ALLOC;
  }

  /**
   * @brief 创建一个 ROS2 订阅者和发布者，用于处理 ROS2 发现信息。
   * @param[in] participant_info 与 Fast RTPS 参与者相关的信息。
   * @param[in] qos 指定订阅者的 QoS 策略。
   * @param[in] subscription_options 订阅选项。
   * @return rmw_ret_t 返回创建结果状态。
   *
   * Create a ROS2 subscriber and publisher for handling ROS2 discovery information.
   * @param[in] participant_info Information related to the Fast RTPS participant.
   * @param[in] qos Specifies the QoS policy for the subscriber.
   * @param[in] subscription_options Subscription options.
   * @return rmw_ret_t Returns the creation result status.
   */
  // 如果我们支持 keyed topics，这里可以是 KEEP_LAST 和深度 1。
  // If we would have support for keyed topics, this could be KEEP_LAST and depth 1.
  qos.history = RMW_QOS_POLICY_HISTORY_KEEP_ALL;

  // 使用 unique_ptr 管理订阅者资源，并在出现异常时自动销毁。
  // Manage the subscription resource with unique_ptr and automatically destroy it in case of
  // exceptions.
  std::unique_ptr<rmw_subscription_t, std::function<void(rmw_subscription_t *)>> subscription(
      rmw_fastrtps_dynamic_cpp::create_subscription(
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

  // 使用 unique_ptr 管理 guard condition 资源，并在出现异常时自动销毁。
  // Manage the guard condition resource with unique_ptr and automatically destroy it in case of
  // exceptions.
  std::unique_ptr<rmw_guard_condition_t, std::function<void(rmw_guard_condition_t *)>>
      graph_guard_condition(
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

  // 将创建的实体分配给 common_context。
  // Assign the created entities to common_context.
  common_context->gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
      eprosima_fastrtps_identifier, participant_info->participant_->guid());
  common_context->pub = publisher.get();
  common_context->sub = subscription.get();
  common_context->graph_guard_condition = graph_guard_condition.get();

  // 将 common_context 和 participant_info 分配给 context 的 impl。
  // Assign common_context and participant_info to the impl of context.
  context->impl->common = common_context.get();
  context->impl->participant_info = participant_info.get();

  // 运行监听线程。
  // Run the listener thread.
  rmw_ret_t ret = rmw_fastrtps_shared_cpp::run_listener_thread(context);
  if (RMW_RET_OK != ret) {
    return ret;
  }

  /**
   * @brief 设置图形缓存更改回调 (Set the graph cache change callback)
   *
   * @param guard_condition 图形守卫条件指针 (Graph guard condition pointer)
   */
  common_context->graph_cache.set_on_change_callback(
      [guard_condition = graph_guard_condition.get()]() {
        // 触发图形守卫条件 (Trigger the graph guard condition)
        rmw_fastrtps_shared_cpp::__rmw_trigger_guard_condition(
            eprosima_fastrtps_identifier, guard_condition);
      });

  /**
   * @brief 添加参与者到图形缓存 (Add participant to the graph cache)
   *
   * @param gid 参与者全局ID (Participant global ID)
   * @param enclave 参与者选项中的enclave (Enclave in participant options)
   */
  common_context->graph_cache.add_participant(common_context->gid, context->options.enclave);

  // 释放图形守卫条件资源 (Release the graph guard condition resources)
  graph_guard_condition.release();
  // 释放发布者资源 (Release the publisher resources)
  publisher.release();
  // 释放订阅者资源 (Release the subscription resources)
  subscription.release();
  // 释放公共上下文资源 (Release the common context resources)
  common_context.release();
  // 释放参与者信息资源 (Release the participant info resources)
  participant_info.release();

  // 返回成功状态 (Return success status)
  return RMW_RET_OK;
}

/**
 * @brief 增加上下文实现的引用计数 (Increment the reference count of the context implementation)
 *
 * @param[in] context 指向要操作的上下文的指针 (Pointer to the context to be operated on)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_fastrtps_dynamic_cpp::increment_context_impl_ref_count(rmw_context_t *context) {
  // 断言：确保 context 不为空 (Assert: Ensure context is not null)
  assert(context);
  // 断言：确保 context 的实现部分不为空 (Assert: Ensure context's implementation part is not null)
  assert(context->impl);

  // 使用 std::lock_guard 对互斥量进行加锁，以确保线程安全 (Use std::lock_guard to lock the mutex,
  // ensuring thread safety)
  std::lock_guard<std::mutex> guard(context->impl->mutex);

  // 如果引用计数为零，则初始化上下文实现 (If the reference count is zero, initialize the context
  // implementation)
  if (!context->impl->count) {
    rmw_ret_t ret = init_context_impl(context);
    // 如果初始化失败，返回错误代码 (If initialization fails, return the error code)
    if (RMW_RET_OK != ret) {
      return ret;
    }
  }
  // 增加引用计数 (Increase the reference count)
  context->impl->count++;
  // 返回成功状态 (Return success status)
  return RMW_RET_OK;
}
