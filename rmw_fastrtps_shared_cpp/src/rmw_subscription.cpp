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

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 销毁订阅者 (Destroy a subscription)
 *
 * @param[in] identifier 实现标识符 (Implementation identifier)
 * @param[in] node ROS2节点指针 (Pointer to the ROS2 node)
 * @param[in,out] subscription 订阅者指针，销毁后将被设置为nullptr (Pointer to the subscription,
 * will be set to nullptr after destruction)
 * @param[in] reset_cft 是否重置内容过滤主题 (Whether to reset content filtered topic)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_destroy_subscription(
    const char *identifier,
    const rmw_node_t *node,
    rmw_subscription_t *subscription,
    bool reset_cft) {
  // 检查节点实现标识符与传入的标识符是否匹配 (Check if the node implementation identifier matches
  // the given identifier)
  assert(node->implementation_identifier == identifier);
  // 检查订阅者实现标识符与传入的标识符是否匹配 (Check if the subscription implementation identifier
  // matches the given identifier)
  assert(subscription->implementation_identifier == identifier);

  // 初始化返回值为成功 (Initialize return value as success)
  rmw_ret_t ret = RMW_RET_OK;
  // 初始化错误状态 (Initialize error state)
  rmw_error_state_t error_state;
  // 初始化错误字符串 (Initialize error string)
  rmw_error_string_t error_string;
  // 获取节点上下文 (Get the node context)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);
  // 获取订阅者信息 (Get the subscriber information)
  auto info = static_cast<const CustomSubscriberInfo *>(subscription->data);
  {
    // 更新图 (Update graph)
    std::lock_guard<std::mutex> guard(common_context->node_update_mutex);
    rmw_dds_common::msg::ParticipantEntitiesInfo msg =
        common_context->graph_cache.dissociate_reader(
            info->subscription_gid_, common_context->gid, node->name, node->namespace_);
    // 发布更新后的图信息 (Publish the updated graph information)
    ret = rmw_fastrtps_shared_cpp::__rmw_publish(
        identifier, common_context->pub, static_cast<void *>(&msg), nullptr);
    if (RMW_RET_OK != ret) {
      // 获取错误状态和字符串 (Get error state and string)
      error_state = *rmw_get_error_state();
      error_string = rmw_get_error_string();
      // 重置错误 (Reset error)
      rmw_reset_error();
    }
  }

  // 获取参与者信息 (Get participant information)
  auto participant_info =
      static_cast<CustomParticipantInfo *>(node->context->impl->participant_info);
  // 销毁订阅者 (Destroy the subscription)
  rmw_ret_t local_ret = destroy_subscription(identifier, participant_info, subscription, reset_cft);
  if (RMW_RET_OK != local_ret) {
    if (RMW_RET_OK != ret) {
      // 输出错误字符串 (Output error string)
      RMW_SAFE_FWRITE_TO_STDERR(error_string.str);
      RMW_SAFE_FWRITE_TO_STDERR(" during '" RCUTILS_STRINGIFY(__function__) "'\n");
    }
    // 更新返回值 (Update return value)
    ret = local_ret;
  } else if (RMW_RET_OK != ret) {
    // 设置错误状态 (Set error state)
    rmw_set_error_state(error_state.message, error_state.file, error_state.line_number);
  }
  // 返回操作结果 (Return the operation result)
  return ret;
}

/**
 * @brief 计算匹配的发布者数量 (Count the number of matched publishers)
 *
 * @param[in] subscription 指向订阅者的指针 (Pointer to the subscription)
 * @param[out] publisher_count 匹配的发布者数量 (Number of matched publishers)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
rmw_ret_t __rmw_subscription_count_matched_publishers(
    const rmw_subscription_t *subscription, size_t *publisher_count) {
  // 将订阅者数据转换为 CustomSubscriberInfo 类型 (Cast subscription data to CustomSubscriberInfo
  // type)
  auto info = static_cast<CustomSubscriberInfo *>(subscription->data);

  // 获取匹配的发布者数量 (Get the number of matched publishers)
  *publisher_count = info->subscription_event_->publisher_count();

  // 返回操作成功状态 (Return operation success status)
  return RMW_RET_OK;
}

/**
 * @brief 获取实际的 QoS 配置 (Get the actual QoS configuration)
 *
 * @param[in] subscription 指向订阅者的指针 (Pointer to the subscription)
 * @param[out] qos 存储实际 QoS 配置的结构体 (Structure to store the actual QoS configuration)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
rmw_ret_t __rmw_subscription_get_actual_qos(
    const rmw_subscription_t *subscription, rmw_qos_profile_t *qos) {
  // 将订阅者数据转换为 CustomSubscriberInfo 类型 (Cast subscription data to CustomSubscriberInfo
  // type)
  auto info = static_cast<CustomSubscriberInfo *>(subscription->data);

  // 获取 Fast DDS 数据读取器 (Get Fast DDS data reader)
  eprosima::fastdds::dds::DataReader *fastdds_dr = info->data_reader_;

  // 获取 Fast DDS QoS 配置 (Get Fast DDS QoS configuration)
  const eprosima::fastdds::dds::DataReaderQos &dds_qos = fastdds_dr->get_qos();

  // 将 Fast DDS QoS 转换为 RMW QoS (Convert Fast DDS QoS to RMW QoS)
  dds_qos_to_rmw_qos(dds_qos, qos);

  // 返回操作成功状态 (Return operation success status)
  return RMW_RET_OK;
}

/**
 * @brief 设置订阅者的内容过滤器 (Set the content filter for a subscription)
 *
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[in] options 内容过滤器选项 (Content filter options)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_subscription_set_content_filter(
    rmw_subscription_t *subscription, const rmw_subscription_content_filter_options_t *options) {
  // 将订阅者的数据转换为 CustomSubscriberInfo 类型 (Cast the subscription data to
  // CustomSubscriberInfo type)
  auto info = static_cast<CustomSubscriberInfo *>(subscription->data);

  // 获取已过滤主题 (Get the filtered topic)
  eprosima::fastdds::dds::ContentFilteredTopic *filtered_topic = info->filtered_topic_;

  // 检查过滤表达式是否为空 (Check if the filter expression is empty)
  const bool filter_expression_empty = (*options->filter_expression == '\0');

  // 如果没有过滤主题且过滤表达式为空
  // If there is no filtered topic and the filter expression is empty
  if (!filtered_topic && filter_expression_empty) {
    // 无法重置当前订阅者
    // Can't reset current subscriber
    RMW_SET_ERROR_MSG("current subscriber has no content filter topic");
    return RMW_RET_ERROR;
  } else if (filtered_topic && !filter_expression_empty) {
    // 创建表达式参数向量
    // Create a vector for expression parameters
    std::vector<std::string> expression_parameters;
    for (size_t i = 0; i < options->expression_parameters.size; ++i) {
      expression_parameters.push_back(options->expression_parameters.data[i]);
    }

    // 设置过滤表达式
    // Set the filter expression
    ReturnCode_t ret =
        filtered_topic->set_filter_expression(options->filter_expression, expression_parameters);
    if (ret != ReturnCode_t::RETCODE_OK) {
      RMW_SET_ERROR_MSG("failed to set_filter_expression");
      return RMW_RET_ERROR;
    }
    return RMW_RET_OK;
  }

  // 获取 DDS 参与者
  // Get the DDS participant
  eprosima::fastdds::dds::DomainParticipant *dds_participant = info->dds_participant_;
  // 初始化描述主题指针
  // Initialize the topic description pointer
  eprosima::fastdds::dds::TopicDescription *des_topic = nullptr;
  const char *eprosima_fastrtps_identifier = subscription->implementation_identifier;

  // 销毁订阅
  // Destroy the subscription
  rmw_ret_t ret = rmw_fastrtps_shared_cpp::__rmw_destroy_subscription(
      eprosima_fastrtps_identifier, info->node_, subscription, true /* reset_cft */);
  if (ret != RMW_RET_OK) {
    RMW_SET_ERROR_MSG("delete subscription with reset cft");
    return RMW_RET_ERROR;
  }

  // 如果没有过滤主题
  // If there is no filtered topic
  if (!filtered_topic) {
    // 创建内容过滤主题
    // Create content filtered topic
    eprosima::fastdds::dds::ContentFilteredTopic *filtered_topic = nullptr;
    if (!rmw_fastrtps_shared_cpp::create_content_filtered_topic(
            dds_participant, info->topic_, info->topic_name_mangled_, options, &filtered_topic)) {
      RMW_SET_ERROR_MSG("create_contentfilteredtopic() failed to create contentfilteredtopic");
      return RMW_RET_ERROR;
    }
    info->filtered_topic_ = filtered_topic;
    des_topic = filtered_topic;
  } else {
    // 使用现有的父主题
    // Use the existing parent topic
    des_topic = info->topic_;
  }

  // 创建数据读取器 (Create data reader)
  eprosima::fastdds::dds::Subscriber *subscriber = info->subscriber_;
  const rmw_subscription_options_t *subscription_options = &subscription->options;

  // 调用 rmw_fastrtps_shared_cpp::create_datareader 函数创建数据读取器，并检查是否成功 (Call the
  // rmw_fastrtps_shared_cpp::create_datareader function to create a data reader and check if it was
  // successful)
  if (!rmw_fastrtps_shared_cpp::create_datareader(
          info->datareader_qos_, subscription_options, subscriber, des_topic,
          info->data_reader_listener_, &info->data_reader_)) {
    // 如果创建失败，设置错误信息并返回错误代码 (If the creation fails, set the error message and
    // return an error code)
    RMW_SET_ERROR_MSG("create_datareader() could not create data reader");
    return RMW_RET_ERROR;
  }

  // 初始化 DataReader 的 StatusCondition 以在有新数据可用时得到通知 (Initialize DataReader's
  // StatusCondition to be notified when new data is available)
  info->data_reader_->get_statuscondition().set_enabled_statuses(
      eprosima::fastdds::dds::StatusMask::data_available());

  // 定义一个 lambda 函数，用于删除 datareader (Define a lambda function for deleting the
  // datareader)
  auto cleanup_datareader = rcpputils::make_scope_exit(
      [subscriber, info]() { subscriber->delete_datareader(info->data_reader_); });

  /////
  // 更新 RMW GID (Update RMW GID)
  info->subscription_gid_ = rmw_fastrtps_shared_cpp::create_rmw_gid(
      eprosima_fastrtps_identifier, info->data_reader_->guid());

  {
    rmw_dds_common::Context *common_context = info->common_context_;
    const rmw_node_t *node = info->node_;

    // 更新图 (Update graph)
    std::lock_guard<std::mutex> guard(common_context->node_update_mutex);
    rmw_dds_common::msg::ParticipantEntitiesInfo msg = common_context->graph_cache.associate_reader(
        info->subscription_gid_, common_context->gid, node->name, node->namespace_);
    rmw_ret_t rmw_ret = rmw_fastrtps_shared_cpp::__rmw_publish(
        eprosima_fastrtps_identifier, common_context->pub, static_cast<void *>(&msg), nullptr);
    if (RMW_RET_OK != rmw_ret) {
      // 如果更新失败，取消关联并返回错误代码 (If the update fails, dissociate and return an error
      // code)
      static_cast<void>(common_context->graph_cache.dissociate_reader(
          info->subscription_gid_, common_context->gid, node->name, node->namespace_));
      return RMW_RET_ERROR;
    }
  }
  // 取消删除 datareader 的操作 (Cancel the deletion of the datareader)
  cleanup_datareader.cancel();
  return RMW_RET_OK;
}

/**
 * @brief 获取订阅者的内容过滤选项 (Get the content filter options of a subscription)
 *
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[in] allocator 分配器指针，用于分配内存 (Pointer to the allocator for memory allocation)
 * @param[out] options 内容过滤选项的指针，用于存储结果 (Pointer to the content filter options to
 * store the result)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_subscription_get_content_filter(
    const rmw_subscription_t *subscription,
    rcutils_allocator_t *allocator,
    rmw_subscription_content_filter_options_t *options) {
  // 将订阅者的数据转换为 CustomSubscriberInfo 类型 (Cast the subscription data to
  // CustomSubscriberInfo type)
  auto info = static_cast<CustomSubscriberInfo *>(subscription->data);
  // 获取过滤主题 (Get the filtered topic)
  eprosima::fastdds::dds::ContentFilteredTopic *filtered_topic = info->filtered_topic_;

  // 检查过滤主题是否为空 (Check if the filtered topic is nullptr)
  if (nullptr == filtered_topic) {
    RMW_SET_ERROR_MSG("this subscriber has not created a ContentFilteredTopic");
    return RMW_RET_ERROR;
  }
  // 创建一个字符串向量，用于存储表达式参数 (Create a string vector to store expression parameters)
  std::vector<std::string> expression_parameters;
  // 获取过滤主题的表达式参数 (Get the expression parameters of the filtered topic)
  ReturnCode_t ret = filtered_topic->get_expression_parameters(expression_parameters);
  // 检查获取操作是否成功 (Check if the get operation is successful)
  if (ret != ReturnCode_t::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to get_expression_parameters");
    return RMW_RET_ERROR;
  }

  // 创建一个字符指针向量，用于存储字符串数组 (Create a vector of const char pointers to store the
  // string array)
  std::vector<const char *> string_array;
  // 遍历表达式参数向量 (Iterate through the expression parameters vector)
  for (size_t i = 0; i < expression_parameters.size(); ++i) {
    // 将表达式参数添加到字符串数组中 (Add the expression parameter to the string array)
    string_array.push_back(expression_parameters[i].c_str());
  }

  // 初始化内容过滤选项并返回结果 (Initialize the content filter options and return the result)
  return rmw_subscription_content_filter_options_init(
      filtered_topic->get_filter_expression().c_str(), string_array.size(), string_array.data(),
      allocator, options);
}

/**
 * @brief 设置订阅者的新消息回调函数 (Set the new message callback function for a subscription)
 *
 * @param[in] rmw_subscription 订阅者指针 (Pointer to the subscription)
 * @param[in] callback 回调函数 (Callback function)
 * @param[in] user_data 用户数据指针，传递给回调函数 (Pointer to user data to be passed to the
 * callback function)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_subscription_set_on_new_message_callback(
    rmw_subscription_t *rmw_subscription, rmw_event_callback_t callback, const void *user_data) {
  // 将订阅者的数据转换为 CustomSubscriberInfo 类型 (Cast the subscription data to
  // CustomSubscriberInfo type)
  auto custom_subscriber_info = static_cast<CustomSubscriberInfo *>(rmw_subscription->data);
  // 设置新消息回调函数和用户数据 (Set the new message callback function and user data)
  custom_subscriber_info->subscription_event_->set_on_new_message_callback(user_data, callback);
  // 返回操作成功 (Return operation success)
  return RMW_RET_OK;
}

}  // namespace rmw_fastrtps_shared_cpp
