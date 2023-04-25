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

#include "fastdds/dds/publisher/DataWriter.hpp"
#include "fastdds/dds/publisher/qos/DataWriterQos.hpp"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_publisher_info.hpp"
#include "rmw_fastrtps_shared_cpp/namespace_prefix.hpp"
#include "rmw_fastrtps_shared_cpp/publisher.hpp"
#include "rmw_fastrtps_shared_cpp/qos.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"
#include "time_utils.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 销毁一个发布者 (Destroy a publisher)
 *
 * @param[in] identifier 发布者的实现标识符 (Implementation identifier of the publisher)
 * @param[in] node 要销毁的发布者所属的节点 (Node to which the publisher to be destroyed belongs)
 * @param[in,out] publisher 要销毁的发布者 (Publisher to be destroyed)
 * @return rmw_ret_t 操作结果状态码 (Operation result status code)
 */
rmw_ret_t __rmw_destroy_publisher(
    const char *identifier, const rmw_node_t *node, rmw_publisher_t *publisher) {
  // 检查节点和发布者的实现标识符是否匹配 (Check if the implementation identifiers of the node and
  // publisher match)
  assert(node->implementation_identifier == identifier);
  assert(publisher->implementation_identifier == identifier);

  // 初始化返回值为成功 (Initialize the return value as successful)
  rmw_ret_t ret = RMW_RET_OK;
  // 定义错误状态变量 (Define error state variable)
  rmw_error_state_t error_state;
  // 获取节点上下文中的通用上下文 (Get the common context from the node context)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);
  // 获取发布者信息 (Get publisher information)
  auto info = static_cast<const CustomPublisherInfo *>(publisher->data);
  {
    // 更新图 (Update graph)
    std::lock_guard<std::mutex> guard(common_context->node_update_mutex);
    rmw_dds_common::msg::ParticipantEntitiesInfo msg =
        common_context->graph_cache.dissociate_writer(
            info->publisher_gid, common_context->gid, node->name, node->namespace_);
    // 发布消息 (Publish message)
    rmw_ret_t publish_ret =
        rmw_fastrtps_shared_cpp::__rmw_publish(identifier, common_context->pub, &msg, nullptr);
    // 检查发布结果 (Check the publishing result)
    if (RMW_RET_OK != publish_ret) {
      error_state = *rmw_get_error_state();
      ret = publish_ret;
      rmw_reset_error();
    }
  }

  // 获取参与者信息 (Get participant information)
  auto participant_info =
      static_cast<CustomParticipantInfo *>(node->context->impl->participant_info);
  // 销毁发布者 (Destroy publisher)
  rmw_ret_t inner_ret = destroy_publisher(identifier, participant_info, publisher);
  // 检查销毁结果 (Check the destruction result)
  if (RMW_RET_OK != inner_ret) {
    if (RMW_RET_OK != ret) {
      RMW_SAFE_FWRITE_TO_STDERR(rmw_get_error_string().str);
      RMW_SAFE_FWRITE_TO_STDERR(" during '" RCUTILS_STRINGIFY(__function__) "'\n");
    } else {
      error_state = *rmw_get_error_state();
      ret = inner_ret;
    }
    rmw_reset_error();
  }

  // 设置错误状态 (Set error state)
  if (RMW_RET_OK != ret) {
    rmw_set_error_state(error_state.message, error_state.file, error_state.line_number);
  }
  // 返回操作结果 (Return operation result)
  return ret;
}

/**
 * @brief 计算匹配的订阅者数量 (Count matched subscriptions)
 *
 * @param[in] publisher 发布者实例 (Publisher instance)
 * @param[out] subscription_count 匹配的订阅者数量 (Number of matched subscriptions)
 * @return rmw_ret_t 操作结果状态码 (Operation result status code)
 */
rmw_ret_t __rmw_publisher_count_matched_subscriptions(
    const rmw_publisher_t *publisher, size_t *subscription_count) {
  // 获取发布者信息 (Get publisher information)
  auto info = static_cast<CustomPublisherInfo *>(publisher->data);

  // 计算匹配的订阅者数量 (Count the number of matched subscriptions)
  *subscription_count = info->publisher_event_->subscription_count();

  // 返回操作结果 (Return operation result)
  return RMW_RET_OK;
}

/**
 * @brief Assert the liveliness of a publisher.
 *
 * @param[in] identifier The implementation identifier of the rmw implementation.
 * @param[in] publisher The publisher to assert its liveliness.
 * @return RMW_RET_OK if successful, or an appropriate error code.
 */
// 声明一个函数，用于确认发布者的活跃性
// Declare a function to assert the liveliness of a publisher
rmw_ret_t __rmw_publisher_assert_liveliness(
    const char *identifier, const rmw_publisher_t *publisher) {
  // 检查传入的发布者是否为空，如果为空则返回无效参数错误
  // Check if the given publisher is null, return invalid argument error if it is
  RMW_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  // 检查发布者的实现标识符是否与传入的标识符匹配，如果不匹配则返回不正确的 RMW 实现错误
  // Check if the publisher's implementation identifier matches the given identifier, return
  // incorrect RMW implementation error if not
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      publisher, publisher->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 将发布者的数据转换为 CustomPublisherInfo 类型
  // Cast the publisher's data to CustomPublisherInfo type
  auto info = static_cast<CustomPublisherInfo *>(publisher->data);
  // 如果转换后的数据为空，则设置错误消息并返回错误
  // If the casted data is null, set error message and return error
  if (nullptr == info) {
    RMW_SET_ERROR_MSG("publisher internal data is invalid");
    return RMW_RET_ERROR;
  }

  // 调用 data_writer_ 的 assert_liveliness() 方法来确认发布者的活跃性
  // Call the assert_liveliness() method of data_writer_ to assert the liveliness of the publisher
  info->data_writer_->assert_liveliness();
  // 返回成功状态
  // Return success status
  return RMW_RET_OK;
}

/**
 * @brief Wait for all acknowledgments from subscribers.
 *
 * @param[in] identifier The implementation identifier of the rmw implementation.
 * @param[in] publisher The publisher to wait for acknowledgments.
 * @param[in] wait_timeout The maximum time to wait for acknowledgments.
 * @return RMW_RET_OK if successful, or an appropriate error code.
 */
// 声明一个函数，用于等待所有订阅者的确认
// Declare a function to wait for all acknowledgments from subscribers
rmw_ret_t __rmw_publisher_wait_for_all_acked(
    const char *identifier, const rmw_publisher_t *publisher, rmw_time_t wait_timeout) {
  // 检查传入的发布者是否为空，如果为空则返回无效参数错误
  // Check if the given publisher is null, return invalid argument error if it is
  RMW_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  // 检查发布者的实现标识符是否与传入的标识符匹配，如果不匹配则返回不正确的 RMW 实现错误
  // Check if the publisher's implementation identifier matches the given identifier, return
  // incorrect RMW implementation error if not
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      publisher, publisher->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 将发布者的数据转换为 CustomPublisherInfo 类型
  // Cast the publisher's data to CustomPublisherInfo type
  auto info = static_cast<CustomPublisherInfo *>(publisher->data);

  // 将 rmw_time_t 类型的等待超时时间转换为 eprosima::fastrtps::Duration_t 类型
  // Convert the wait_timeout of type rmw_time_t to eprosima::fastrtps::Duration_t type
  eprosima::fastrtps::Duration_t timeout = rmw_time_to_fastrtps(wait_timeout);

  // 调用 data_writer_ 的 wait_for_acknowledgments() 方法来等待所有订阅者的确认，将返回值存储在 ret
  // 变量中 Call the wait_for_acknowledgments() method of data_writer_ to wait for all
  // acknowledgments from subscribers, store the return value in the ret variable
  ReturnCode_t ret = info->data_writer_->wait_for_acknowledgments(timeout);
  // 如果返回值为 RETCODE_OK，则返回成功状态
  // If the return value is RETCODE_OK, return success status
  if (ReturnCode_t::RETCODE_OK == ret) {
    return RMW_RET_OK;
  }

  // 返回超时错误
  // Return timeout error
  return RMW_RET_TIMEOUT;
}

/**
 * @brief Get the actual QoS settings of a publisher.
 *
 * @param[in] publisher The publisher to get its actual QoS settings.
 * @param[out] qos The actual QoS settings of the publisher.
 * @return RMW_RET_OK if successful, or an appropriate error code.
 */
// 声明一个函数，用于获取发布者的实际 QoS 设置
// Declare a function to get the actual QoS settings of a publisher
rmw_ret_t __rmw_publisher_get_actual_qos(const rmw_publisher_t *publisher, rmw_qos_profile_t *qos) {
  // 将发布者的数据转换为 CustomPublisherInfo 类型
  // Cast the publisher's data to CustomPublisherInfo type
  auto info = static_cast<CustomPublisherInfo *>(publisher->data);
  eprosima::fastdds::dds::DataWriter *fastdds_dw = info->data_writer_;
  const eprosima::fastdds::dds::DataWriterQos &dds_qos = fastdds_dw->get_qos();

  dds_qos_to_rmw_qos(dds_qos, qos);

  return RMW_RET_OK;
}

/**
 * @brief 借用已分配的消息 (Borrow a loaned message)
 *
 * @param[in] identifier 指示符 (Identifier)
 * @param[in] publisher 发布者指针 (Pointer to the publisher)
 * @param[in] type_support 消息类型支持 (Message type support)
 * @param[out] ros_message 输出的 ROS 消息指针 (Output pointer to the ROS message)
 * @return rmw_ret_t RMW 返回值 (RMW return value)
 */
rmw_ret_t __rmw_borrow_loaned_message(
    const char *identifier,
    const rmw_publisher_t *publisher,
    const rosidl_message_type_support_t *type_support,
    void **ros_message) {
  // 检查发布者是否为空 (Check if the publisher is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  // 检查类型标识符是否匹配 (Check if the type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      publisher, publisher->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查是否支持借用消息 (Check if loaning messages is supported)
  if (!publisher->can_loan_messages) {
    RMW_SET_ERROR_MSG("Loaning is not supported");
    return RMW_RET_UNSUPPORTED;
  }

  // 检查类型支持是否为空 (Check if the type support is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(type_support, RMW_RET_INVALID_ARGUMENT);
  // 检查 ROS 消息是否为空 (Check if the ROS message is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(ros_message, RMW_RET_INVALID_ARGUMENT);
  // 检查 ROS 消息是否已分配 (Check if the ROS message is already allocated)
  if (nullptr != *ros_message) {
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 获取自定义发布者信息 (Get the custom publisher info)
  auto info = static_cast<CustomPublisherInfo *>(publisher->data);
  // 借用样本 (Loan a sample)
  if (!info->data_writer_->loan_sample(*ros_message)) {
    return RMW_RET_ERROR;
  }

  return RMW_RET_OK;
}

/**
 * @brief 从发布者返回借用的消息 (Return a loaned message from the publisher)
 *
 * @param[in] identifier 指示符 (Identifier)
 * @param[in] publisher 发布者指针 (Pointer to the publisher)
 * @param[in] loaned_message 借用的消息指针 (Pointer to the loaned message)
 * @return rmw_ret_t RMW 返回值 (RMW return value)
 */
rmw_ret_t __rmw_return_loaned_message_from_publisher(
    const char *identifier, const rmw_publisher_t *publisher, void *loaned_message) {
  // 检查发布者是否为空 (Check if the publisher is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(publisher, RMW_RET_INVALID_ARGUMENT);
  // 检查类型标识符是否匹配 (Check if the type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      publisher, publisher->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查是否支持借用消息 (Check if loaning messages is supported)
  if (!publisher->can_loan_messages) {
    RMW_SET_ERROR_MSG("Loaning is not supported");
    return RMW_RET_UNSUPPORTED;
  }

  // 检查借用的消息是否为空 (Check if the loaned message is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(loaned_message, RMW_RET_INVALID_ARGUMENT);

  // 获取自定义发布者信息 (Get the custom publisher info)
  auto info = static_cast<CustomPublisherInfo *>(publisher->data);
  // 丢弃借用的消息 (Discard the loaned message)
  if (!info->data_writer_->discard_loan(loaned_message)) {
    return RMW_RET_ERROR;
  }

  return RMW_RET_OK;
}

}  // namespace rmw_fastrtps_shared_cpp
