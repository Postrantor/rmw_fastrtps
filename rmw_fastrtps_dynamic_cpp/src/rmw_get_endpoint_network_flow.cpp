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

#include "rmw/error_handling.h"
#include "rmw/get_network_flow_endpoints.h"
#include "rmw/rmw.h"
#include "rmw/types.h"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 获取发布者的网络流端点 (Get the network flow endpoints of a publisher)
 *
 * @param[in] publisher 指向要获取网络流端点的发布者的指针 (Pointer to the publisher for which to
 * get the network flow endpoints)
 * @param[in] allocator 用于分配内存的分配器 (Allocator used for memory allocation)
 * @param[out] network_flow_endpoint_array 存储网络流端点的数组 (Array to store the network flow
 * endpoints)
 *
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_publisher_get_network_flow_endpoints(
    const rmw_publisher_t* publisher,
    rcutils_allocator_t* allocator,
    rmw_network_flow_endpoint_array_t* network_flow_endpoint_array) {
  // 调用共享实现中的函数来获取发布者的网络流端点 (Call the function in the shared implementation to
  // get the network flow endpoints of the publisher)
  return rmw_fastrtps_shared_cpp::__rmw_publisher_get_network_flow_endpoints(
      publisher, allocator, network_flow_endpoint_array);
}

/**
 * @brief 获取订阅者的网络流端点 (Get the network flow endpoints of a subscription)
 *
 * @param[in] subscription 指向要获取网络流端点的订阅者的指针 (Pointer to the subscription for which
 * to get the network flow endpoints)
 * @param[in] allocator 用于分配内存的分配器 (Allocator used for memory allocation)
 * @param[out] network_flow_endpoint_array 存储网络流端点的数组 (Array to store the network flow
 * endpoints)
 *
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_subscription_get_network_flow_endpoints(
    const rmw_subscription_t* subscription,
    rcutils_allocator_t* allocator,
    rmw_network_flow_endpoint_array_t* network_flow_endpoint_array) {
  // 调用共享实现中的函数来获取订阅者的网络流端点 (Call the function in the shared implementation to
  // get the network flow endpoints of the subscription)
  return rmw_fastrtps_shared_cpp::__rmw_subscription_get_network_flow_endpoints(
      subscription, allocator, network_flow_endpoint_array);
}

}  // extern "C"
