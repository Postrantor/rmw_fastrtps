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
#include "rmw/rmw.h"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 等待 ROS2 事件的发生，如订阅消息、服务请求等。
 *        Wait for ROS2 events to occur, such as subscription messages and service requests.
 *
 * @param[in] subscriptions 指向订阅者列表的指针。Pointer to the list of subscribers.
 * @param[in] guard_conditions 指向守护条件列表的指针。Pointer to the list of guard conditions.
 * @param[in] services 指向服务列表的指针。Pointer to the list of services.
 * @param[in] clients 指向客户端列表的指针。Pointer to the list of clients.
 * @param[in] events 指向事件列表的指针。Pointer to the list of events.
 * @param[in] wait_set 指向等待集合的指针。Pointer to the wait set.
 * @param[in] wait_timeout 等待超时时间。Wait timeout duration.
 * @return 返回 rmw_ret_t 类型的结果。Returns a result of type rmw_ret_t.
 */
rmw_ret_t rmw_wait(
    rmw_subscriptions_t* subscriptions,
    rmw_guard_conditions_t* guard_conditions,
    rmw_services_t* services,
    rmw_clients_t* clients,
    rmw_events_t* events,
    rmw_wait_set_t* wait_set,
    const rmw_time_t* wait_timeout) {
  // 调用 FastRTPS 共享 C++ 实现的 __rmw_wait 函数，并传入相应参数。
  // Call the __rmw_wait function from the FastRTPS shared C++ implementation with the given
  // arguments.
  return rmw_fastrtps_shared_cpp::__rmw_wait(
      eprosima_fastrtps_identifier, subscriptions, guard_conditions, services, clients, events,
      wait_set, wait_timeout);
}

}  // extern "C"
