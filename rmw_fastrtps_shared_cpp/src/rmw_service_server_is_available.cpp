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

#include "demangle.hpp"
#include "fastrtps/subscriber/Subscriber.h"
#include "rcutils/logging_macros.h"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw/types.h"
#include "rmw_dds_common/context.hpp"
#include "rmw_fastrtps_shared_cpp/custom_client_info.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 检查服务端是否可用 (Check if the service server is available)
 *
 * @param[in] identifier 实现的标识符 (Implementation identifier)
 * @param[in] node ROS2节点句柄 (ROS2 node handle)
 * @param[in] client 客户端句柄 (Client handle)
 * @param[out] is_available 服务端是否可用的布尔值指针 (Pointer to a boolean value indicating
 * whether the service server is available)
 * @return rmw_ret_t 返回操作状态 (Return operation status)
 */
rmw_ret_t __rmw_service_server_is_available(
    const char *identifier,
    const rmw_node_t *node,
    const rmw_client_t *client,
    bool *is_available) {
  // 检查节点是否为空 (Check if the node is null)
  if (!node) {
    RMW_SET_ERROR_MSG("node handle is null");
    return RMW_RET_ERROR;
  }

  // 检查类型标识符是否匹配 (Check if the type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      node handle, node->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查客户端是否为空 (Check if the client is null)
  if (!client) {
    RMW_SET_ERROR_MSG("client handle is null");
    return RMW_RET_ERROR;
  }

  // 检查类型标识符是否匹配 (Check if the type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      client handle, client->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查 is_available 是否为空 (Check if is_available is null)
  if (!is_available) {
    RMW_SET_ERROR_MSG("is_available is null");
    return RMW_RET_ERROR;
  }

  // 获取客户端信息 (Get client information)
  auto client_info = static_cast<CustomClientInfo *>(client->data);
  if (!client_info) {
    RMW_SET_ERROR_MSG("client info handle is null");
    return RMW_RET_ERROR;
  }

  // 获取发布主题名称 (Get the publisher topic name)
  auto pub_topic_name = client_info->request_topic_name_;

  // 获取订阅主题名称 (Get the subscriber topic name)
  auto sub_topic_name = client_info->response_topic_name_;

  // 初始化 is_available 为 false (Initialize is_available to false)
  *is_available = false;
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);

  // 获取请求订阅者数量 (Get the number of request subscribers)
  size_t number_of_request_subscribers = 0;
  rmw_ret_t ret =
      common_context->graph_cache.get_reader_count(pub_topic_name, &number_of_request_subscribers);
  if (ret != RMW_RET_OK) {
    // 错误 (Error)
    return ret;
  }
  if (0 == number_of_request_subscribers) {
    // 未准备好 (Not ready)
    return RMW_RET_OK;
  }

  // 获取响应发布者数量 (Get the number of response publishers)
  size_t number_of_response_publishers = 0;
  ret =
      common_context->graph_cache.get_writer_count(sub_topic_name, &number_of_response_publishers);
  if (ret != RMW_RET_OK) {
    // 错误 (Error)
    return ret;
  }
  if (0 == number_of_response_publishers) {
    // 未准备好 (Not ready)
    return RMW_RET_OK;
  }

  // 检查请求订阅者和响应发布者数量是否相等 (Check if the number of request subscribers and response
  // publishers are equal)
  if (number_of_request_subscribers != number_of_response_publishers) {
    // 未准备好 (Not ready)
    return RMW_RET_OK;
  }

  // 获取匹配的请求发布者数量 (Get the number of matched request publishers)
  size_t matched_request_pubs = client_info->request_publisher_matched_count_.load();
  if (0 == matched_request_pubs) {
    // 未准备好 (Not ready)
    return RMW_RET_OK;
  }
  // 获取匹配的响应订阅者数量 (Get the number of matched response subscribers)
  size_t matched_response_subs = client_info->response_subscriber_matched_count_.load();
  if (0 == matched_response_subs) {
    // 未准备好 (Not ready)
    return RMW_RET_OK;
  }
  if (matched_request_pubs != matched_response_subs) {
    // 未准备好 (Not ready)
    return RMW_RET_OK;
  }

  // 所有条件满足，服务端可用 (All conditions met, the service server is available)
  *is_available = true;
  return RMW_RET_OK;
}
}  // namespace rmw_fastrtps_shared_cpp
