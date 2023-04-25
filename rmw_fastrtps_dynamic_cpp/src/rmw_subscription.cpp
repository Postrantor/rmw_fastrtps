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

#include <string>
#include <utility>

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/get_topic_endpoint_info.h"
#include "rmw/rmw.h"
#include "rmw_dds_common/qos.hpp"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_dynamic_cpp/subscription.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_subscriber_info.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"
#include "rmw_fastrtps_shared_cpp/subscription.hpp"
#include "type_support_common.hpp"
#include "type_support_registry.hpp"

extern "C" {

/**
 * @brief 初始化订阅分配（Initialize the subscription allocation）
 *
 * @param[in] type_support 消息类型支持（Message type support）
 * @param[in] message_bounds 消息边界（Message bounds）
 * @param[out] allocation 分配的订阅指针（Pointer to the allocated subscription）
 *
 * @return 返回 rmw_ret_t 类型的结果（Return a result of type rmw_ret_t）
 */
rmw_ret_t rmw_init_subscription_allocation(
    const rosidl_message_type_support_t *type_support,
    const rosidl_runtime_c__Sequence__bound *message_bounds,
    rmw_subscription_allocation_t *allocation) {
  // 当前实现中未使用（Unused in current implementation）
  (void)type_support;
  // 当前实现中未使用（Unused in current implementation）
  (void)message_bounds;
  // 当前实现中未使用（Unused in current implementation）
  (void)allocation;

  // 设置错误消息为 "unimplemented"（Set error message to "unimplemented"）
  RMW_SET_ERROR_MSG("unimplemented");
  // 返回不支持的错误代码（Return unsupported error code）
  return RMW_RET_UNSUPPORTED;
}

/**
 * @brief 终止订阅分配（Finalize the subscription allocation）
 *
 * @param[in,out] allocation 分配的订阅指针（Pointer to the allocated subscription）
 *
 * @return 返回 rmw_ret_t 类型的结果（Return a result of type rmw_ret_t）
 */
rmw_ret_t rmw_fini_subscription_allocation(rmw_subscription_allocation_t *allocation) {
  // 当前实现中未使用（Unused in current implementation）
  (void)allocation;

  // 设置错误消息为 "unimplemented"（Set error message to "unimplemented"）
  RMW_SET_ERROR_MSG("unimplemented");
  // 返回不支持的错误代码（Return unsupported error code）
  return RMW_RET_UNSUPPORTED;
}

/**
 * @brief 创建一个订阅者 (Create a subscription)
 *
 * @param[in] node 指向要创建订阅者的节点的指针 (Pointer to the node where the subscription will be
 * created)
 * @param[in] type_supports 消息类型支持结构体 (Message type support structure)
 * @param[in] topic_name 订阅的主题名称 (Name of the topic to subscribe to)
 * @param[in] qos_policies 要使用的QoS策略 (QoS policies to use)
 * @param[in] subscription_options 订阅选项 (Subscription options)
 * @return rmw_subscription_t* 成功时返回新创建的订阅者指针，失败时返回nullptr (Pointer to the newly
 * created subscription on success, nullptr on failure)
 */
rmw_subscription_t *rmw_create_subscription(
    const rmw_node_t *node,
    const rosidl_message_type_support_t *type_supports,
    const char *topic_name,
    const rmw_qos_profile_t *qos_policies,
    const rmw_subscription_options_t *subscription_options) {
  // 检查输入参数是否为空 (Check if input arguments are null)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, nullptr);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, eprosima_fastrtps_identifier, return nullptr);
  RMW_CHECK_ARGUMENT_FOR_NULL(qos_policies, nullptr);

  // 适配任何'最佳可用' QoS 选项 (Adapt any 'best available' QoS options)
  rmw_qos_profile_t adapted_qos_policies = *qos_policies;
  rmw_ret_t ret = rmw_dds_common::qos_profile_get_best_available_for_topic_subscription(
      node, topic_name, &adapted_qos_policies, rmw_get_publishers_info_by_topic);
  if (RMW_RET_OK != ret) {
    return nullptr;
  }

  // 获取参与者信息 (Get participant information)
  auto participant_info =
      static_cast<CustomParticipantInfo *>(node->context->impl->participant_info);

  // 创建订阅 (Create subscription)
  rmw_subscription_t *subscription = rmw_fastrtps_dynamic_cpp::create_subscription(
      participant_info, type_supports, topic_name, &adapted_qos_policies, subscription_options,
      false);
  if (!subscription) {
    return nullptr;
  }

  // 获取公共上下文和订阅者信息 (Get common context and subscriber information)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);
  auto info = static_cast<CustomSubscriberInfo *>(subscription->data);
  {
    // 更新图 (Update graph)
    std::lock_guard<std::mutex> guard(common_context->node_update_mutex);
    rmw_dds_common::msg::ParticipantEntitiesInfo msg = common_context->graph_cache.associate_reader(
        info->subscription_gid_, common_context->gid, node->name, node->namespace_);
    rmw_ret_t rmw_ret = rmw_fastrtps_shared_cpp::__rmw_publish(
        eprosima_fastrtps_identifier, common_context->pub, static_cast<void *>(&msg), nullptr);
    if (RMW_RET_OK != rmw_ret) {
      rmw_error_state_t error_state = *rmw_get_error_state();
      rmw_reset_error();
      static_cast<void>(common_context->graph_cache.dissociate_reader(
          info->subscription_gid_, common_context->gid, node->name, node->namespace_));
      rmw_ret = rmw_fastrtps_shared_cpp::destroy_subscription(
          eprosima_fastrtps_identifier, participant_info, subscription);
      if (RMW_RET_OK != rmw_ret) {
        RMW_SAFE_FWRITE_TO_STDERR(rmw_get_error_string().str);
        RMW_SAFE_FWRITE_TO_STDERR(" during '" RCUTILS_STRINGIFY(__function__) "' cleanup\n");
        rmw_reset_error();
      }
      rmw_set_error_state(error_state.message, error_state.file, error_state.line_number);
      return nullptr;
    }
  }
  // 设置节点和公共上下文信息 (Set node and common context information)
  info->node_ = node;
  info->common_context_ = common_context;

  // 返回创建的订阅 (Return the created subscription)
  return subscription;
}

/**
 * @brief 计算匹配的发布者数量 (Count the number of matched publishers)
 *
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[out] publisher_count 匹配的发布者数量 (Number of matched publishers)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
rmw_ret_t rmw_subscription_count_matched_publishers(
    const rmw_subscription_t *subscription, size_t *publisher_count) {
  // 检查订阅者参数是否为空 (Check if the subscription argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);
  // 检查类型标识符是否匹配 (Check if type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      subscription, subscription->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查发布者计数参数是否为空 (Check if the publisher_count argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(publisher_count, RMW_RET_INVALID_ARGUMENT);

  // 调用共享实现并返回结果 (Call shared implementation and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_subscription_count_matched_publishers(
      subscription, publisher_count);
}

/**
 * @brief 获取实际的 QoS 配置 (Get the actual QoS configuration)
 *
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[out] qos QoS 配置 (QoS configuration)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
rmw_ret_t rmw_subscription_get_actual_qos(
    const rmw_subscription_t *subscription, rmw_qos_profile_t *qos) {
  // 检查订阅者参数是否为空 (Check if the subscription argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);
  // 检查类型标识符是否匹配 (Check if type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      subscription, subscription->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查 qos 参数是否为空 (Check if the qos argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);

  // 调用共享实现并返回结果 (Call shared implementation and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_subscription_get_actual_qos(subscription, qos);
}

/**
 * @brief 设置内容过滤器 (Set content filter)
 *
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[in] options 内容过滤器选项 (Content filter options)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
rmw_ret_t rmw_subscription_set_content_filter(
    rmw_subscription_t *subscription, const rmw_subscription_content_filter_options_t *options) {
  // 当前实现中未使用 (Unused in current implementation)
  (void)subscription;
  (void)options;
  // 设置错误消息并返回不支持的状态 (Set error message and return unsupported status)
  RMW_SET_ERROR_MSG("unimplemented");
  return RMW_RET_UNSUPPORTED;
}

/**
 * @brief 获取内容过滤器 (Get content filter)
 *
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[in] allocator 分配器 (Allocator)
 * @param[out] options 内容过滤器选项 (Content filter options)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
rmw_ret_t rmw_subscription_get_content_filter(
    const rmw_subscription_t *subscription,
    rcutils_allocator_t *allocator,
    rmw_subscription_content_filter_options_t *options) {
  // 当前实现中未使用 (Unused in current implementation)
  (void)subscription;
  (void)allocator;
  (void)options;
  // 设置错误消息并返回不支持的状态 (Set error message and return unsupported status)
  RMW_SET_ERROR_MSG("unimplemented");
  return RMW_RET_UNSUPPORTED;
}

using BaseTypeSupport = rmw_fastrtps_dynamic_cpp::BaseTypeSupport;

/**
 * @brief 销毁订阅者 (Destroy a subscription)
 *
 * @param[in] node 要销毁的订阅者所属的节点 (The node to which the subscription belongs)
 * @param[in] subscription 要销毁的订阅者 (The subscription to be destroyed)
 * @return rmw_ret_t 返回操作结果 (Return the result of the operation)
 */
rmw_ret_t rmw_destroy_subscription(rmw_node_t *node, rmw_subscription_t *subscription) {
  // 检查节点是否为空，如果为空返回无效参数错误 (Check if the node is null, return invalid argument
  // error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  // 检查订阅者是否为空，如果为空返回无效参数错误 (Check if the subscription is null, return invalid
  // argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);
  // 检查节点的实现标识符是否与期望的一致，如果不一致返回错误的 RMW 实现错误 (Check if the node's
  // implementation identifier matches the expected one, return incorrect RMW implementation error
  // if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查订阅者的实现标识符是否与期望的一致，如果不一致返回错误的 RMW 实现错误 (Check if the
  // subscription's implementation identifier matches the expected one, return incorrect RMW
  // implementation error if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      subscription, subscription->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 获取订阅者的自定义信息 (Get the custom information of the subscriber)
  auto info = static_cast<CustomSubscriberInfo *>(subscription->data);
  // 获取类型支持实现 (Get the type support implementation)
  auto impl = static_cast<const BaseTypeSupport *>(info->type_support_impl_);
  // 获取 ROS 类型支持 (Get the ROS type support)
  auto ros_type_support =
      static_cast<const rosidl_message_type_support_t *>(impl->ros_type_support());

  // 获取类型支持注册表实例 (Get the instance of the type support registry)
  TypeSupportRegistry &type_registry = TypeSupportRegistry::get_instance();
  // 返回消息类型支持 (Return the message type support)
  type_registry.return_message_type_support(ros_type_support);

  // 销毁订阅者 (Destroy the subscription)
  return rmw_fastrtps_shared_cpp::__rmw_destroy_subscription(
      eprosima_fastrtps_identifier, node, subscription);
}

/**
 * @brief 设置新消息回调函数 (Set the new message callback function)
 *
 * @param[in] rmw_subscription 订阅者 (The subscription)
 * @param[in] callback 回调函数 (The callback function)
 * @param[in] user_data 用户数据 (User data)
 * @return rmw_ret_t 返回操作结果 (Return the result of the operation)
 */
rmw_ret_t rmw_subscription_set_on_new_message_callback(
    rmw_subscription_t *rmw_subscription, rmw_event_callback_t callback, const void *user_data) {
  // 检查订阅者是否为空，如果为空返回无效参数错误 (Check if the subscription is null, return invalid
  // argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(rmw_subscription, RMW_RET_INVALID_ARGUMENT);

  // 设置新消息回调函数 (Set the new message callback function)
  return rmw_fastrtps_shared_cpp::__rmw_subscription_set_on_new_message_callback(
      rmw_subscription, callback, user_data);
}

}  // extern "C"
