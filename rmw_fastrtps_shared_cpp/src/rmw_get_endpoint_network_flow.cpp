// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "fastrtps/utils/IPLocator.h"
#include "rmw/error_handling.h"
#include "rmw/get_network_flow_endpoints.h"
#include "rmw_fastrtps_shared_cpp/custom_publisher_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_subscriber_info.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

namespace rmw_fastrtps_shared_cpp {

using Locator_t = eprosima::fastrtps::rtps::Locator_t;
using LocatorList_t = eprosima::fastrtps::rtps::LocatorList_t;
using IPLocator = eprosima::fastrtps::rtps::IPLocator;

/**
 * @brief 填充网络流端点信息 (Fill the network flow endpoint information)
 *
 * @param[out] rmw_network_flow_endpoint RMW 网络流端点结构体指针 (Pointer to RMW network flow
 * endpoint structure)
 * @param[in] locator FastRTPS 定位器结构体引用 (Reference to FastRTPS locator structure)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t fill_network_flow_endpoint(
    rmw_network_flow_endpoint_t *rmw_network_flow_endpoint, const Locator_t &locator);

/**
 * @brief 获取发布者的网络流端点信息 (Get the network flow endpoints information of the publisher)
 *
 * @param[in] publisher RMW 发布者指针 (Pointer to RMW publisher)
 * @param[in] allocator RCUtils 内存分配器指针 (Pointer to RCUtils allocator)
 * @param[out] network_flow_endpoint_array RMW 网络流端点数组指针 (Pointer to RMW network flow
 * endpoint array)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_publisher_get_network_flow_endpoints(
    const rmw_publisher_t *publisher,
    rcutils_allocator_t *allocator,
    rmw_network_flow_endpoint_array_t *network_flow_endpoint_array) {
  // 初始化结果变量为成功 (Initialize the result variable as successful)
  rmw_ret_t res = RMW_RET_OK;

  // 获取发送定位器 (Retrieve the sender locators)
  CustomPublisherInfo *data = static_cast<CustomPublisherInfo *>(publisher->data);
  LocatorList_t locators;
  data->data_writer_->get_sending_locators(locators);

  // 如果定位器列表为空，则直接返回结果 (If the locator list is empty, return the result directly)
  if (locators.empty()) {
    return res;
  }

  // 检查网络流端点数组是否未初始化 (Check if the network flow endpoint array is not initialized)
  if (RMW_RET_OK !=
      (res = rmw_network_flow_endpoint_array_check_zero(network_flow_endpoint_array))) {
    return res;
  }

  // 为网络流端点数组分配内存 (Allocate memory for the network flow endpoint array)
  if (RMW_RET_OK != (res = rmw_network_flow_endpoint_array_init(
                         network_flow_endpoint_array, locators.size(), allocator))) {
    return res;
  }

  // 将定位器转换为 RMW 网络流端点，如果出错则重置数组 (Translate the locators to RMW network flow
  // endpoints, reset the array on error)
  try {
    auto rmw_nf = network_flow_endpoint_array->network_flow_endpoint;
    for (const Locator_t &loc : locators) {
      if (RMW_RET_OK != (res = fill_network_flow_endpoint(rmw_nf++, loc))) {
        throw res;
      }
    }
  } catch (rmw_ret_t) {
    // 清空网络流端点数组 (Clear the network flow endpoint array)
    rmw_network_flow_endpoint_array_fini(network_flow_endpoint_array);

    // 设置错误消息 (Set error message)
    RMW_SET_ERROR_MSG("Failed to compose network_flow_endpoint_array");
  }

  // 返回操作结果 (Return the operation result)
  return res;
}

/**
 * @brief 获取订阅者的网络流端点 (Get the network flow endpoints of a subscription)
 *
 * @param[in] subscription 订阅者指针 (Pointer to the subscription)
 * @param[in] allocator 分配器指针，用于分配内存 (Pointer to the allocator for memory allocation)
 * @param[out] network_flow_endpoint_array 存储网络流端点信息的数组 (Array to store the network flow
 * endpoint information)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_subscription_get_network_flow_endpoints(
    const rmw_subscription_t *subscription,
    rcutils_allocator_t *allocator,
    rmw_network_flow_endpoint_array_t *network_flow_endpoint_array) {
  // 初始化返回值为成功 (Initialize the return value as successful)
  rmw_ret_t res = RMW_RET_OK;

  // 获取监听定位器 (Retrieve the listener locators)
  CustomSubscriberInfo *data = static_cast<CustomSubscriberInfo *>(subscription->data);
  LocatorList_t locators;
  data->data_reader_->get_listening_locators(locators);

  // 如果定位器为空，则直接返回 (If the locators are empty, return directly)
  if (locators.empty()) {
    return res;
  }

  // 检查网络流端点数组是否未初始化 (Check if the network flow endpoint array is not initialized)
  if (RMW_RET_OK !=
      (res = rmw_network_flow_endpoint_array_check_zero(network_flow_endpoint_array))) {
    return res;
  }

  // 为网络流端点数组分配内存 (Allocate memory for the network flow endpoint array)
  if (RMW_RET_OK != (res = rmw_network_flow_endpoint_array_init(
                         network_flow_endpoint_array, locators.size(), allocator))) {
    return res;
  }

  // 将定位器转换为网络流端点，如果出错则重置数组 (Translate the locators to network flow endpoints,
  // reset the array on error)
  try {
    auto rmw_nf = network_flow_endpoint_array->network_flow_endpoint;
    for (const Locator_t &loc : locators) {
      if (RMW_RET_OK != (res = fill_network_flow_endpoint(rmw_nf++, loc))) {
        throw res;
      }
    }
  } catch (rmw_ret_t) {
    // 清除数组 (Clear the array)
    rmw_network_flow_endpoint_array_fini(network_flow_endpoint_array);

    // 设置错误消息 (Set error message)
    RMW_SET_ERROR_MSG("Failed to compose network_flow_endpoint_array");
  }

  // 返回操作结果 (Return the operation result)
  return res;
}

/**
 * @brief 将 Fast-DDS Locator_t 结构转换为 rmw_transport_protocol_t 枚举值 (Translate Fast-DDS
 * Locator_t structure to rmw_transport_protocol_t enum value)
 *
 * @param[in] loc Fast-DDS Locator_t 结构 (Fast-DDS Locator_t structure)
 * @return rmw_transport_protocol_t 枚举值 (rmw_transport_protocol_t enum value)
 */
rmw_transport_protocol_t get_transport_protocol(const Locator_t &loc) {
  // 判断传输协议是否是 UDP (Check if the transport protocol is UDP)
  if (loc.kind & (LOCATOR_KIND_UDPv4 | LOCATOR_KIND_UDPv6)) {
    return RMW_TRANSPORT_PROTOCOL_UDP;
  }
  // 判断传输协议是否是 TCP (Check if the transport protocol is TCP)
  else if (loc.kind & (LOCATOR_KIND_TCPv4 | LOCATOR_KIND_TCPv6)) {
    return RMW_TRANSPORT_PROTOCOL_TCP;
  }

  // 未知的传输协议 (Unknown transport protocol)
  return RMW_TRANSPORT_PROTOCOL_UNKNOWN;
}

/**
 * @brief 将 Fast-DDS Locator_t 结构转换为 rmw_internet_protocol_t 枚举值 (Translate Fast-DDS
 * Locator_t structure to rmw_internet_protocol_t enum value)
 *
 * @param[in] loc Fast-DDS Locator_t 结构 (Fast-DDS Locator_t structure)
 * @return rmw_internet_protocol_t 枚举值 (rmw_internet_protocol_t enum value)
 */
rmw_internet_protocol_t get_internet_protocol(const Locator_t &loc) {
  // 判断网络协议是否是 IPv4 (Check if the internet protocol is IPv4)
  if (loc.kind & (LOCATOR_KIND_UDPv4 | LOCATOR_KIND_TCPv4)) {
    return RMW_INTERNET_PROTOCOL_IPV4;
  }
  // 判断网络协议是否是 IPv6 (Check if the internet protocol is IPv6)
  else if (loc.kind & (LOCATOR_KIND_TCPv6 | LOCATOR_KIND_UDPv6)) {
    return RMW_INTERNET_PROTOCOL_IPV6;
  }

  // 未知的网络协议 (Unknown internet protocol)
  return RMW_INTERNET_PROTOCOL_UNKNOWN;
}

/**
 * @brief 填充 rmw_network_flow_endpoint_t 结构 (Fill in the rmw_network_flow_endpoint_t structure)
 *
 * @param[out] network_flow_endpoint 指向 rmw_network_flow_endpoint_t 结构的指针 (Pointer to
 * rmw_network_flow_endpoint_t structure)
 * @param[in] locator Fast-DDS Locator_t 结构 (Fast-DDS Locator_t structure)
 * @return rmw_ret_t 枚举值，表示操作结果 (rmw_ret_t enum value, indicating the operation result)
 */
rmw_ret_t fill_network_flow_endpoint(
    rmw_network_flow_endpoint_t *network_flow_endpoint, const Locator_t &locator) {
  rmw_ret_t res = RMW_RET_OK;

  // 将传输协议从 Fast-DDS Locator_t 转换为 rmw_transport_protocol_t (Translate transport protocol
  // from Fast-DDS Locator_t to rmw_transport_protocol_t)
  network_flow_endpoint->transport_protocol = get_transport_protocol(locator);

  // 将网络协议从 Fast-DDS Locator_t 转换为 rmw_internet_protocol_t (Translate internet protocol
  // from Fast-DDS Locator_t to rmw_internet_protocol_t)
  network_flow_endpoint->internet_protocol = get_internet_protocol(locator);

  // 设置端口号 (Set the port number)
  network_flow_endpoint->transport_port = IPLocator::getPhysicalPort(locator);

  // 设置网络地址 (Set the internet address)
  std::string address = IPLocator::ip_to_string(locator);

  // 将网络地址设置到 rmw_network_flow_endpoint_t 结构中 (Set the internet address to the
  // rmw_network_flow_endpoint_t structure)
  if (RMW_RET_OK != (res = rmw_network_flow_endpoint_set_internet_address(
                         network_flow_endpoint, address.c_str(), address.length()))) {
    return res;
  }

  return res;
}

}  // namespace rmw_fastrtps_shared_cpp
