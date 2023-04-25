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

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/get_topic_endpoint_info.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_dds_common/context.hpp"
#include "rmw_dds_common/msg/participant_entities_info.hpp"
#include "rmw_dds_common/qos.hpp"
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_cpp/publisher.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_publisher_info.hpp"
#include "rmw_fastrtps_shared_cpp/publisher.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"

extern "C" {

/**
 * @brief 初始化发布者内存分配（Initialize publisher allocation）
 *
 * @param[in] type_support 消息类型支持（Message type support）
 * @param[in] message_bounds 消息边界（Message bounds）
 * @param[out] allocation 发布者内存分配指针（Pointer to publisher allocation）
 * @return rmw_ret_t 返回操作状态（Return operation status）
 */
rmw_ret_t rmw_init_publisher_allocation(
    const rosidl_message_type_support_t *type_support,
    const rosidl_runtime_c__Sequence__bound *message_bounds,
    rmw_publisher_allocation_t *allocation) {
  // 当前实现中未使用（Unused in current implementation）
  (void)type_support;
  (void)message_bounds;
  (void)allocation;

  // 设置错误信息（Set error message）
  RMW_SET_ERROR_MSG("unimplemented");

  // 返回不支持的状态（Return unsupported status）
  return RMW_RET_UNSUPPORTED;
}

/**
 * @brief 终止发布者内存分配（Finalize publisher allocation）
 *
 * @param[in,out] allocation 发布者内存分配指针（Pointer to publisher allocation）
 * @return rmw_ret_t 返回操作状态（Return operation status）
 */
rmw_ret_t rmw_fini_publisher_allocation(rmw_publisher_allocation_t *allocation) {
  // 当前实现中未使用（Unused in current implementation）
  (void)allocation;

  // 设置错误信息（Set error message）
  RMW_SET_ERROR_MSG("unimplemented");

  // 返回不支持的状态（Return unsupported status）
  return RMW_RET_UNSUPPORTED;
}

/**
 * @brief 创建一个 ROS2 发布者 (Create a ROS2 publisher)
 *
 * @param[in] node 指向要创建发布者的节点的指针 (Pointer to the node where the publisher will be
 * created)
 * @param[in] type_supports 消息类型支持结构体 (Message type support structure)
 * @param[in] topic_name 要发布的主题名称 (Name of the topic to be published)
 * @param[in] qos_policies 质量服务策略 (Quality of Service policies)
 * @param[in] publisher_options 发布者选项 (Publisher options)
 * @return rmw_publisher_t* 创建成功返回指向发布者的指针，否则返回 nullptr (On success, returns a
 * pointer to the publisher, otherwise returns nullptr)
 */
rmw_publisher_t *rmw_create_publisher(
    const rmw_node_t *node,
    const rosidl_message_type_support_t *type_supports,
    const char *topic_name,
    const rmw_qos_profile_t *qos_policies,
    const rmw_publisher_options_t *publisher_options) {
  // 检查 node 是否为空 (Check if node is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, nullptr);
  // 检查实现标识符是否匹配 (Check if implementation identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, eprosima_fastrtps_identifier, return nullptr);
  // 检查 qos_policies 是否为空 (Check if qos_policies is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos_policies, nullptr);

  // 适配 'best available' QoS 选项 (Adapt any 'best available' QoS options)
  rmw_qos_profile_t adapted_qos_policies = *qos_policies;
  rmw_ret_t ret = rmw_dds_common::qos_profile_get_best_available_for_topic_publisher(
      node, topic_name, &adapted_qos_policies, rmw_get_subscriptions_info_by_topic);
  if (RMW_RET_OK != ret) {
    return nullptr;
  }

  // 获取参与者信息 (Get participant information)
  auto participant_info =
      static_cast<CustomParticipantInfo *>(node->context->impl->participant_info);

  // 创建发布者 (Create the publisher)
  rmw_publisher_t *publisher = rmw_fastrtps_cpp::create_publisher(
      participant_info, type_supports, topic_name, &adapted_qos_policies, publisher_options);

  // 检查是否创建成功 (Check if the publisher was created successfully)
  if (!publisher) {
    return nullptr;
  }

  // 获取公共上下文 (Get common context)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);

  // 获取发布者信息 (Get publisher information)
  auto info = static_cast<const CustomPublisherInfo *>(publisher->data);
  {
    // 更新图形 (Update graph)
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
  // 返回创建的发布者 (Return the created publisher)
  return publisher;
}

/**
 * @brief 计算匹配订阅的发布者数量 (Count the number of matched subscriptions for a publisher)
 *
 * @param[in] publisher 待查询的发布者指针 (Pointer to the publisher to query)
 * @param[out] subscription_count 匹配订阅数量的输出参数 (Output parameter for the count of matched
 * subscriptions)
 * @return rmw_ret_t RMW操作返回值 (Return value for RMW operation)
 */
rmw_ret_t rmw_publisher_count_matched_subscriptions(
    const rmw_publisher_t *publisher, size_t *subscription_count) {
  // 检查输入参数是否为空，如果为空则返回无效参数错误 (Check if input arguments are NULL, return
  // invalid argument error if so)
  RMW_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  // 检查发布者类型与FastRTPS实现是否匹配，如果不匹配则返回错误 (Check if publisher type matches
  // FastRTPS implementation, return error if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      publisher, publisher->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查subscription_count参数是否为空，如果为空则返回无效参数错误 (Check if subscription_count
  // argument is NULL, return invalid argument error if so)
  RMW_CHECK_ARGUMENT_FOR_NULL(subscription_count, RMW_RET_INVALID_ARGUMENT);

  // 调用共享代码库中的函数，执行真正的计算操作 (Call the function in the shared code library to
  // perform the actual counting operation)
  return rmw_fastrtps_shared_cpp::__rmw_publisher_count_matched_subscriptions(
      publisher, subscription_count);
}

/**
 * @brief 断言发布者的活跃性 (Assert the liveliness of a publisher)
 *
 * @param[in] publisher 待断言活跃性的发布者指针 (Pointer to the publisher to assert liveliness)
 * @return rmw_ret_t RMW操作返回值 (Return value for RMW operation)
 */
rmw_ret_t rmw_publisher_assert_liveliness(const rmw_publisher_t *publisher) {
  // 调用共享代码库中的函数，执行真正的断言操作 (Call the function in the shared code library to
  // perform the actual assertion operation)
  return rmw_fastrtps_shared_cpp::__rmw_publisher_assert_liveliness(
      eprosima_fastrtps_identifier, publisher);
}

/**
 * @brief 等待所有已发布消息被确认 (Wait for all published messages to be acknowledged)
 *
 * @param[in] publisher 待等待确认的发布者指针 (Pointer to the publisher to wait for
 * acknowledgements)
 * @param[in] wait_timeout 等待超时时间 (Wait timeout duration)
 * @return rmw_ret_t RMW操作返回值 (Return value for RMW operation)
 */
rmw_ret_t rmw_publisher_wait_for_all_acked(
    const rmw_publisher_t *publisher, rmw_time_t wait_timeout) {
  // 调用共享代码库中的函数，执行真正的等待操作 (Call the function in the shared code library to
  // perform the actual waiting operation)
  return rmw_fastrtps_shared_cpp::__rmw_publisher_wait_for_all_acked(
      eprosima_fastrtps_identifier, publisher, wait_timeout);
}

/**
 * @brief 获取发布者的实际QoS配置 (Get the actual QoS settings of a publisher)
 *
 * @param[in] publisher 待获取QoS配置的发布者指针 (Pointer to the publisher to get QoS settings)
 * @param[out] qos 输出参数，用于存储实际的QoS配置 (Output parameter for storing the actual QoS
 * settings)
 * @return rmw_ret_t RMW操作返回值 (Return value for RMW operation)
 */
rmw_ret_t rmw_publisher_get_actual_qos(const rmw_publisher_t *publisher, rmw_qos_profile_t *qos) {
  // 检查输入参数是否为空，如果为空则返回无效参数错误
  // (Check if input arguments are NULL, return invalid argument error if so)
  RMW_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  // 检查发布者类型与FastRTPS实现是否匹配，如果不匹配则返回错误
  // (Check if publisher type matches FastRTPS implementation, return error if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      publisher, publisher->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查qos参数是否为空，如果为空则返回无效参数错误
  // (Check if qos argument is NULL, return invalid argument error if so)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos, RMW_RET_INVALID_ARGUMENT);

  // 调用共享代码库中的函数，执行真正的QoS获取操作 (Call the function in the shared code library to
  // perform the actual QoS retrieval operation)
  return rmw_fastrtps_shared_cpp::__rmw_publisher_get_actual_qos(publisher, qos);
}

/**
 * @brief 借用发布者的消息 (Borrow a message from a publisher)
 *
 * @param[in] publisher 发布者指针 (Pointer to the publisher)
 * @param[in] type_support 消息类型支持 (Message type support)
 * @param[out] ros_message 输出参数，用于存储借用消息的指针 (Output parameter for storing the
 * pointer to the borrowed message)
 * @return rmw_ret_t RMW操作返回值 (Return value for RMW operation)
 */
rmw_ret_t rmw_borrow_loaned_message(
    const rmw_publisher_t *publisher,
    const rosidl_message_type_support_t *type_support,
    void **ros_message) {
  // 调用共享代码库中的函数，执行真正的消息借用操作 (Call the function in the shared code library to
  // perform the actual message borrowing operation)
  return rmw_fastrtps_shared_cpp::__rmw_borrow_loaned_message(
      eprosima_fastrtps_identifier, publisher, type_support, ros_message);
}

/**
 * @brief 归还发布者的借用消息 (Return a borrowed message to a publisher)
 *
 * @param[in] publisher 发布者指针 (Pointer to the publisher)
 * @param[in] loaned_message 借用消息的指针 (Pointer to the borrowed message)
 * @return rmw_ret_t RMW操作返回值 (Return value for RMW operation)
 */
rmw_ret_t rmw_return_loaned_message_from_publisher(
    const rmw_publisher_t *publisher, void *loaned_message) {
  // 调用共享代码库中的函数，执行真正的消息归还操作 (Call the function in the shared code library to
  // perform the actual message returning operation)
  return rmw_fastrtps_shared_cpp::__rmw_return_loaned_message_from_publisher(
      eprosima_fastrtps_identifier, publisher, loaned_message);
}

/**
 * @brief 销毁发布者 (Destroy a publisher)
 *
 * @param[in] node 节点指针 (Pointer to the node)
 * @param[in] publisher 发布者指针 (Pointer to the publisher)
 * @return rmw_ret_t RMW操作返回值 (Return value for RMW operation)
 */
rmw_ret_t rmw_destroy_publisher(rmw_node_t *node, rmw_publisher_t *publisher) {
  // 检查输入参数是否为空，如果为空则返回无效参数错误 (Check if input arguments are NULL, return
  // invalid argument error if so)
  RMW_CHECK_ARGUMENT_FOR_NULL(node, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  // 检查节点类型与FastRTPS实现是否匹配，如果不匹配则返回错误 (Check if node type matches FastRTPS
  // implementation, return error if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node, node->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查发布者类型与FastRTPS实现是否匹配，如果不匹配则返回错误 (Check if publisher type matches
  // FastRTPS implementation, return error if not)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      publisher, publisher->implementation_identifier, eprosima_fastrtps_identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 调用共享代码库中的函数，执行真正的销毁操作 (Call the function in the shared code library to
  // perform the actual destruction operation)
  return rmw_fastrtps_shared_cpp::__rmw_destroy_publisher(
      eprosima_fastrtps_identifier, node, publisher);
}

}  // extern "C"
