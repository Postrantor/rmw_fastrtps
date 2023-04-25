// Copyright 2019 Open Source Robotics Foundation, Inc.
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
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_cpp/subscription.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_subscriber_info.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"
#include "rmw_fastrtps_shared_cpp/subscription.hpp"

extern "C" {

/**
 * @brief 初始化订阅分配
 * @param type_support 消息类型支持结构体的指针
 * @param message_bounds 消息边界的指针
 * @param allocation 分配结构体的指针
 * @return 返回 rmw_ret_t 类型的结果
 *
 * Initialize subscription allocation
 * @param type_support Pointer to the message type support structure
 * @param message_bounds Pointer to the message bounds
 * @param allocation Pointer to the allocation structure
 * @return Returns a result of type rmw_ret_t
 */
rmw_ret_t rmw_init_subscription_allocation(
    const rosidl_message_type_support_t *type_support,
    const rosidl_runtime_c__Sequence__bound *message_bounds,
    rmw_subscription_allocation_t *allocation) {
  // 当前实现中未使用
  // Unused in current implementation.
  (void)type_support;
  (void)message_bounds;
  (void)allocation;

  // 设置错误消息为 "unimplemented"
  // Set error message to "unimplemented"
  RMW_SET_ERROR_MSG("unimplemented");

  // 返回不支持的结果
  // Return unsupported result
  return RMW_RET_UNSUPPORTED;
}

/**
 * @brief 结束订阅分配
 * @param allocation 分配结构体的指针
 * @return 返回 rmw_ret_t 类型的结果
 *
 * Finalize subscription allocation
 * @param allocation Pointer to the allocation structure
 * @return Returns a result of type rmw_ret_t
 */
rmw_ret_t rmw_fini_subscription_allocation(rmw_subscription_allocation_t *allocation) {
  // 当前实现中未使用
  // Unused in current implementation.
  (void)allocation;

  // 设置错误消息为 "unimplemented"
  // Set error message to "unimplemented"
  RMW_SET_ERROR_MSG("unimplemented");

  // 返回不支持的结果
  // Return unsupported result
  return RMW_RET_UNSUPPORTED;
}

/**
 * @brief 创建一个订阅者 (Create a subscription)
 *
 * @param node ROS2节点指针 (Pointer to the ROS2 node)
 * @param type_supports 消息类型支持 (Message type support)
 * @param topic_name 订阅的话题名称 (Name of the topic to subscribe to)
 * @param qos_policies QoS策略 (QoS policies)
 * @param subscription_options 订阅选项 (Subscription options)
 * @return rmw_subscription_t* 成功时返回创建的订阅者，失败时返回nullptr (Returns the created
 * subscription on success, nullptr on failure)
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

  // 适配任何“最佳可用”QoS选项 (Adapt any 'best available' QoS options)
  rmw_qos_profile_t adapted_qos_policies = *qos_policies;
  rmw_ret_t ret = rmw_dds_common::qos_profile_get_best_available_for_topic_subscription(
      node, topic_name, &adapted_qos_policies, rmw_get_publishers_info_by_topic);
  if (RMW_RET_OK != ret) {
    return nullptr;
  }

  // 获取参与者信息 (Get participant info)
  auto participant_info =
      static_cast<CustomParticipantInfo *>(node->context->impl->participant_info);
  // 创建订阅者 (Create the subscription)
  rmw_subscription_t *subscription = rmw_fastrtps_cpp::create_subscription(
      participant_info, type_supports, topic_name, &adapted_qos_policies, subscription_options,
      false);  // 使用无键话题 (use no keyed topic)
  if (!subscription) {
    return nullptr;
  }

  // 获取通用上下文和订阅者信息 (Get common context and subscriber info)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);
  auto info = static_cast<CustomSubscriberInfo *>(subscription->data);

  {
    // 更新图 (Update graph)
    std::lock_guard<std::mutex> guard(common_context->node_update_mutex);
    rmw_dds_common::msg::ParticipantEntitiesInfo msg = common_context->graph_cache.associate_reader(
        info->subscription_gid_, common_context->gid, node->name, node->namespace_);
    // 发布更新后的图信息 (Publish updated graph info)
    rmw_ret_t rmw_ret = rmw_fastrtps_shared_cpp::__rmw_publish(
        eprosima_fastrtps_identifier, common_context->pub, static_cast<void *>(&msg), nullptr);
    if (RMW_RET_OK != rmw_ret) {
      // 错误处理 (Error handling)
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
  // 设置节点和通用上下文 (Set the node and common context)
  info->node_ = node;
  info->common_context_ = common_context;

  return subscription;
}

/**
 * @brief 计算匹配的发布者数量 (Count the number of matched publishers)
 *
 * @param[in] subscription 订阅指针 (Pointer to the subscription)
 * @param[out] publisher_count 匹配的发布者数量指针 (Pointer to store the count of matched
 * publishers)
 * @return rmw_ret_t 返回状态 (Return status)
 */
rmw_ret_t rmw_subscription_count_matched_publishers(
    const rmw_subscription_t *subscription, size_t *publisher_count) {
  // 检查订阅参数是否为空 (Check if the subscription argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);

  // 检查订阅类型标识符是否匹配 (Check if the subscription type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      subscription, subscription->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查publisher_count参数是否为空 (Check if the publisher_count argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(publisher_count, RMW_RET_INVALID_ARGUMENT);

  // 调用共享实现计算匹配的发布者数量 (Call the shared implementation to count the matched
  // publishers)
  return rmw_fastrtps_shared_cpp::__rmw_subscription_count_matched_publishers(
      subscription, publisher_count);
}

/**
 * @brief 获取订阅的实际QoS配置 (Get the actual QoS configuration of the subscription)
 *
 * @param[in] subscription 订阅指针 (Pointer to the subscription)
 * @param[out] qos 存储实际QoS配置的指针 (Pointer to store the actual QoS configuration)
 * @return rmw_ret_t 返回状态 (Return status)
 */
rmw_ret_t rmw_subscription_get_actual_qos(
    const rmw_subscription_t *subscription, rmw_qos_profile_t *qos) {
  // 检查订阅参数是否为空 (Check if the subscription argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);

  // 检查订阅类型标识符是否匹配 (Check if the subscription type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      subscription, subscription->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查qos参数是否为空 (Check if the qos argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);

  // 调用共享实现获取实际QoS配置 (Call the shared implementation to get the actual QoS
  // configuration)
  return rmw_fastrtps_shared_cpp::__rmw_subscription_get_actual_qos(subscription, qos);
}

/**
 * @brief 设置订阅的内容过滤器 (Set the content filter for the subscription)
 *
 * @param[in,out] subscription 订阅指针 (Pointer to the subscription)
 * @param[in] options 内容过滤器选项指针 (Pointer to the content filter options)
 * @return rmw_ret_t 返回状态 (Return status)
 */
rmw_ret_t rmw_subscription_set_content_filter(
    rmw_subscription_t *subscription, const rmw_subscription_content_filter_options_t *options) {
  // 检查订阅参数是否为空 (Check if the subscription argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);

  // 检查options参数是否为空 (Check if the options argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(options, RMW_RET_INVALID_ARGUMENT);

  // 检查订阅类型标识符是否匹配 (Check if the subscription type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      subscription, subscription->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 调用共享实现设置内容过滤器 (Call the shared implementation to set the content filter)
  rmw_ret_t ret =
      rmw_fastrtps_shared_cpp::__rmw_subscription_set_content_filter(subscription, options);

  // 更新is_cft_enabled状态 (Update the is_cft_enabled status)
  auto info = static_cast<const CustomSubscriberInfo *>(subscription->data);
  subscription->is_cft_enabled = (info && info->filtered_topic_);

  // 返回结果 (Return the result)
  return ret;
}

/**
 * @brief 获取订阅的内容过滤器 (Get the content filter for the subscription)
 *
 * @param[in] subscription 订阅指针 (Pointer to the subscription)
 * @param[in] allocator 分配器指针 (Pointer to the allocator)
 * @param[out] options 存储内容过滤器选项的指针 (Pointer to store the content filter options)
 * @return rmw_ret_t 返回状态 (Return status)
 */
rmw_ret_t rmw_subscription_get_content_filter(
    const rmw_subscription_t *subscription,
    rcutils_allocator_t *allocator,
    rmw_subscription_content_filter_options_t *options) {
  // 检查订阅参数是否为空 (Check if the subscription argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);

  // 检查分配器参数是否为空 (Check if the allocator argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(allocator, RMW_RET_INVALID_ARGUMENT);

  // 检查options参数是否为空 (Check if the options argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(options, RMW_RET_INVALID_ARGUMENT);

  // 检查订阅类型标识符是否匹配 (Check if the subscription type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      subscription, subscription->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 调用共享实现获取内容过滤器 (Call the shared implementation to get the content filter)
  return rmw_fastrtps_shared_cpp::__rmw_subscription_get_content_filter(
      subscription, allocator, options);
}

/**
 * @brief 销毁订阅 (Destroy the subscription)
 *
 * @param[in] node 节点指针 (Pointer to the node)
 * @param[in,out] subscription 订阅指针 (Pointer to the subscription)
 * @return rmw_ret_t 返回状态 (Return status)
 */
rmw_ret_t rmw_destroy_subscription(rmw_node_t *node, rmw_subscription_t *subscription) {
  // 检查节点参数是否为空 (Check if the node argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);

  // 检查订阅参数是否为空 (Check if the subscription argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription, RMW_RET_INVALID_ARGUMENT);

  // 检查节点类型标识符是否匹配 (Check if the node type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查订阅类型标识符是否匹配 (Check if the subscription type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      subscription, subscription->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 调用共享实现销毁订阅 (Call the shared implementation to destroy the subscription)
  return rmw_fastrtps_shared_cpp::__rmw_destroy_subscription(
      eprosima_fastrtps_identifier, node, subscription);
}

/**
 * @brief 设置新消息回调 (Set the on new message callback)
 *
 * @param[in,out] rmw_subscription 订阅指针 (Pointer to the subscription)
 * @param[in] callback 新消息回调函数 (New message callback function)
 * @param[in] user_data 用户数据指针 (Pointer to user data)
 * @return rmw_ret_t 返回状态 (Return status)
 */
rmw_ret_t rmw_subscription_set_on_new_message_callback(
    rmw_subscription_t *rmw_subscription, rmw_event_callback_t callback, const void *user_data) {
  // 检查订阅参数是否为空 (Check if the subscription argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(rmw_subscription, RMW_RET_INVALID_ARGUMENT);

  return rmw_fastrtps_shared_cpp::__rmw_subscription_set_on_new_message_callback(
      rmw_subscription, callback, user_data);
}

}  // extern "C"
