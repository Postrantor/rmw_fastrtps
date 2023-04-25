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

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/get_topic_endpoint_info.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_dds_common/context.hpp"
#include "rmw_dds_common/msg/participant_entities_info.hpp"
#include "rmw_dds_common/qos.hpp"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_dynamic_cpp/publisher.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_publisher_info.hpp"
#include "rmw_fastrtps_shared_cpp/publisher.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"
#include "type_support_common.hpp"
#include "type_support_registry.hpp"

extern "C" {

/**
 * @brief 初始化发布者分配
 * @param type_support 消息类型支持
 * @param message_bounds 消息边界
 * @param allocation 发布者分配
 * @return rmw_ret_t 返回值状态
 *
 * @brief Initialize publisher allocation
 * @param type_support Message type support
 * @param message_bounds Message bounds
 * @param allocation Publisher allocation
 * @return rmw_ret_t Return value status
 */
rmw_ret_t rmw_init_publisher_allocation(
    const rosidl_message_type_support_t *type_support,
    const rosidl_runtime_c__Sequence__bound *message_bounds,
    rmw_publisher_allocation_t *allocation) {
  // 当前实现中未使用
  // Unused in current implementation.
  (void)type_support;
  (void)message_bounds;
  (void)allocation;

  // 设置错误消息为 "未实现"
  // Set error message to "unimplemented"
  RMW_SET_ERROR_MSG("unimplemented");

  // 返回不支持的状态
  // Return unsupported status
  return RMW_RET_UNSUPPORTED;
}

/**
 * @brief 结束发布者分配
 * @param allocation 发布者分配
 * @return rmw_ret_t 返回值状态
 *
 * @brief Finalize publisher allocation
 * @param allocation Publisher allocation
 * @return rmw_ret_t Return value status
 */
rmw_ret_t rmw_fini_publisher_allocation(rmw_publisher_allocation_t *allocation) {
  // 当前实现中未使用
  // Unused in current implementation.
  (void)allocation;

  // 设置错误消息为 "未实现"
  // Set error message to "unimplemented"
  RMW_SET_ERROR_MSG("unimplemented");

  // 返回不支持的状态
  // Return unsupported status
  return RMW_RET_UNSUPPORTED;
}

/**
 * @brief 创建一个发布者 (Create a publisher)
 *
 * @param[in] node 指向要创建发布者的节点的指针 (Pointer to the node where the publisher will be
 * created)
 * @param[in] type_supports 消息类型支持 (Message type support)
 * @param[in] topic_name 要发布的主题名称 (Topic name to publish)
 * @param[in] qos_policies 要使用的QoS策略 (QoS policies to use)
 * @param[in] publisher_options 发布者选项 (Publisher options)
 * @return rmw_publisher_t* 成功时返回一个指向新创建的发布者的指针，失败时返回nullptr (A pointer to
 * the newly created publisher on success, nullptr on failure)
 */
rmw_publisher_t *rmw_create_publisher(
    const rmw_node_t *node,
    const rosidl_message_type_support_t *type_supports,
    const char *topic_name,
    const rmw_qos_profile_t *qos_policies,
    const rmw_publisher_options_t *publisher_options) {
  // 检查输入参数是否为空 (Check if input arguments are null)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, nullptr);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, eprosima_fastrtps_identifier, return nullptr);
  RMW_CHECK_ARGUMENT_FOR_NULL(qos_policies, nullptr);

  // 适配任何“最佳可用”的QoS选项 (Adapt any 'best available' QoS options)
  rmw_qos_profile_t adapted_qos_policies = *qos_policies;
  rmw_ret_t ret = rmw_dds_common::qos_profile_get_best_available_for_topic_publisher(
      node, topic_name, &adapted_qos_policies, rmw_get_subscriptions_info_by_topic);
  if (RMW_RET_OK != ret) {
    return nullptr;
  }

  // 获取参与者信息 (Get participant info)
  auto participant_info =
      static_cast<CustomParticipantInfo *>(node->context->impl->participant_info);
  // 创建发布者 (Create publisher)
  rmw_publisher_t *publisher = rmw_fastrtps_dynamic_cpp::create_publisher(
      participant_info, type_supports, topic_name, &adapted_qos_policies, publisher_options);

  if (!publisher) {
    return nullptr;
  }

  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);

  auto info = static_cast<const CustomPublisherInfo *>(publisher->data);
  {
    // 更新图 (Update graph)
    std::lock_guard<std::mutex> guard(common_context->node_update_mutex);
    rmw_dds_common::msg::ParticipantEntitiesInfo msg = common_context->graph_cache.associate_writer(
        info->publisher_gid, common_context->gid, node->name, node->namespace_);
    rmw_ret_t rmw_ret = rmw_fastrtps_shared_cpp::__rmw_publish(
        eprosima_fastrtps_identifier, common_context->pub, static_cast<void *>(&msg), nullptr);
    if (RMW_RET_OK != rmw_ret) {
      rmw_error_state_t error_state = *rmw_get_error_state();
      rmw_reset_error();
      static_cast<void>(common_context->graph_cache.dissociate_writer(
          info->publisher_gid, common_context->gid, node->name, node->namespace_));
      rmw_ret = rmw_fastrtps_shared_cpp::destroy_publisher(
          eprosima_fastrtps_identifier, participant_info, publisher);
      if (RMW_RET_OK != rmw_ret) {
        RMW_SAFE_FWRITE_TO_STDERR(rmw_get_error_string().str);
        RMW_SAFE_FWRITE_TO_STDERR(" during '" RCUTILS_STRINGIFY(__function__) "' cleanup\n");
        rmw_reset_error();
      }
      rmw_set_error_state(error_state.message, error_state.file, error_state.line_number);
      return nullptr;
    }
  }
  return publisher;
}

/**
 * @brief 计算与发布者匹配的订阅者数量 (Count the number of matched subscriptions for a publisher)
 *
 * @param[in] publisher 指向要查询的发布者的指针 (Pointer to the publisher to query)
 * @param[out] subscription_count 匹配的订阅者数量 (Number of matched subscriptions)
 * @return rmw_ret_t RMW操作返回值 (RMW operation return value)
 */
rmw_ret_t rmw_publisher_count_matched_subscriptions(
    const rmw_publisher_t *publisher, size_t *subscription_count) {
  // 检查publisher参数是否为空 (Check if the publisher argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  // 检查publisher的实现标识符是否与期望的一致 (Check if the publisher's implementation identifier
  // matches the expected one)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      publisher, publisher->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查subscription_count参数是否为空 (Check if the subscription_count argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription_count, RMW_RET_INVALID_ARGUMENT);

  // 调用共享实现函数并返回结果 (Call the shared implementation function and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_publisher_count_matched_subscriptions(
      publisher, subscription_count);
}

/**
 * @brief 断言发布者的活跃状态 (Assert the liveliness of a publisher)
 *
 * @param[in] publisher 指向要断言的发布者的指针 (Pointer to the publisher to assert)
 * @return rmw_ret_t RMW操作返回值 (RMW operation return value)
 */
rmw_ret_t rmw_publisher_assert_liveliness(const rmw_publisher_t *publisher) {
  // 调用共享实现函数并返回结果 (Call the shared implementation function and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_publisher_assert_liveliness(
      eprosima_fastrtps_identifier, publisher);
}

/**
 * @brief 等待发布者的所有消息被确认 (Wait for all messages of a publisher to be acknowledged)
 *
 * @param[in] publisher 指向要等待的发布者的指针 (Pointer to the publisher to wait for)
 * @param[in] wait_timeout 等待超时时间 (Wait timeout duration)
 * @return rmw_ret_t RMW操作返回值 (RMW operation return value)
 */
rmw_ret_t rmw_publisher_wait_for_all_acked(
    const rmw_publisher_t *publisher, rmw_time_t wait_timeout) {
  // 调用共享实现函数并返回结果 (Call the shared implementation function and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_publisher_wait_for_all_acked(
      eprosima_fastrtps_identifier, publisher, wait_timeout);
}

/**
 * @brief 获取发布者的实际QoS配置 (Get the actual QoS configuration of a publisher)
 *
 * @param[in] publisher 指向要查询的发布者的指针 (Pointer to the publisher to query)
 * @param[out] qos 存储实际QoS配置的结构体 (Structure to store the actual QoS configuration)
 * @return rmw_ret_t RMW操作返回值 (RMW operation return value)
 */
rmw_ret_t rmw_publisher_get_actual_qos(const rmw_publisher_t *publisher, rmw_qos_profile_t *qos) {
  // 检查publisher参数是否为空 (Check if the publisher argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  // 检查publisher的实现标识符是否与期望的一致 (Check if the publisher's implementation identifier
  // matches the expected one)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      publisher, publisher->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查qos参数是否为空 (Check if the qos argument is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);

  // 调用共享实现函数并返回结果 (Call the shared implementation function and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_publisher_get_actual_qos(publisher, qos);
}

/**
 * @brief 借用发布者的已分配消息 (Borrow a loaned message from a publisher)
 *
 * @param[in] publisher 指向要借用消息的发布者的指针 (Pointer to the publisher to borrow the message
 * from)
 * @param[in] type_support 消息类型支持 (Message type support)
 * @param[out] ros_message 存储借用消息的指针 (Pointer to store the borrowed message)
 * @return rmw_ret_t RMW操作返回值 (RMW operation return value)
 */
rmw_ret_t rmw_borrow_loaned_message(
    const rmw_publisher_t *publisher,
    const rosidl_message_type_support_t *type_support,
    void **ros_message) {
  // 调用共享实现函数并返回结果 (Call the shared implementation function and return the result)
  return rmw_fastrtps_shared_cpp::__rmw_borrow_loaned_message(
      eprosima_fastrtps_identifier, publisher, type_support, ros_message);
}

/**
 * @brief 返回已借用的消息给发布者 (Return a loaned message to the publisher)
 *
 * @param[in] publisher 发布者指针 (Pointer to the publisher)
 * @param[in] loaned_message 已借用的消息指针 (Pointer to the loaned message)
 * @return rmw_ret_t RMW操作返回值 (RMW operation return value)
 */
rmw_ret_t rmw_return_loaned_message_from_publisher(
    const rmw_publisher_t *publisher, void *loaned_message) {
  // 调用共享实现，返回已借用的消息给发布者 (Call shared implementation to return the loaned message to the publisher)
  return rmw_fastrtps_shared_cpp::__rmw_return_loaned_message_from_publisher(
      eprosima_fastrtps_identifier, publisher, loaned_message);
}

// 定义基本类型支持别名 (Define BaseTypeSupport alias)
using BaseTypeSupport = rmw_fastrtps_dynamic_cpp::BaseTypeSupport;

/**
 * @brief 销毁发布者 (Destroy the publisher)
 *
 * @param[in] node 节点指针 (Pointer to the node)
 * @param[in] publisher 发布者指针 (Pointer to the publisher)
 * @return rmw_ret_t RMW操作返回值 (RMW operation return value)
 */
rmw_ret_t rmw_destroy_publisher(rmw_node_t *node, rmw_publisher_t *publisher) {
  // 检查节点参数是否为空 (Check if the node argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  // 检查发布者参数是否为空 (Check if the publisher argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  // 检查节点实现标识符是否匹配 (Check if the node implementation identifier matches)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查发布者实现标识符是否匹配 (Check if the publisher implementation identifier matches)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      publisher, publisher->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 获取自定义发布者信息 (Get custom publisher information)
  auto info = static_cast<CustomPublisherInfo *>(publisher->data);
  // 获取基本类型支持实现 (Get base type support implementation)
  auto impl = static_cast<const BaseTypeSupport *>(info->type_support_impl_);
  // 获取ROS消息类型支持 (Get ROS message type support)
  auto ros_type_support =
      static_cast<const rosidl_message_type_support_t *>(impl->ros_type_support());

  // 获取类型支持注册实例 (Get type support registry instance)
  TypeSupportRegistry &type_registry = TypeSupportRegistry::get_instance();
  // 返回消息类型支持 (Return message type support)
  type_registry.return_message_type_support(ros_type_support);

  // 调用共享实现，销毁发布者 (Call shared implementation to destroy the publisher)
  return rmw_fastrtps_shared_cpp::__rmw_destroy_publisher(
      eprosima_fastrtps_identifier, node, publisher);
}

}  // extern "C"
