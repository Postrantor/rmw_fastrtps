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
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

extern "C" {

/**
 * @brief 等待 ROS2 中的实体（订阅、服务、客户端等）变为可用或超时。
 *        Wait for entities (subscriptions, services, clients, etc.) in ROS2 to become available or
 * timeout.
 *
 * @param[in] subscriptions 指向订阅者列表的指针。
 *                          Pointer to the list of subscribers.
 * @param[in] guard_conditions 指向保护条件列表的指针。
 *                              Pointer to the list of guard conditions.
 * @param[in] services 指向服务列表的指针。
 *                     Pointer to the list of services.
 * @param[in] clients 指向客户端列表的指针。
 *                    Pointer to the list of clients.
 * @param[in] events 指向事件列表的指针。
 *                   Pointer to the list of events.
 * @param[in] wait_set 指向等待集的指针，用于存储等待的实体。
 *                     Pointer to the wait set, used to store the waiting entities.
 * @param[in] wait_timeout 指向等待超时时间的指针。如果为 nullptr，则无限期等待。
 *                         Pointer to the wait timeout duration. If nullptr, wait indefinitely.
 *
 * @return rmw_ret_t 返回操作结果。成功返回 RMW_RET_OK，否则返回相应的错误代码。
 *                   Return the operation result. On success, return RMW_RET_OK, otherwise return
 * the corresponding error code.
 */
rmw_ret_t rmw_wait(
    rmw_subscriptions_t* subscriptions,
    rmw_guard_conditions_t* guard_conditions,
    rmw_services_t* services,
    rmw_clients_t* clients,
    rmw_events_t* events,
    rmw_wait_set_t* wait_set,
    const rmw_time_t* wait_timeout) {
  // 调用 Fast RTPS 共享 C++ 实现的 __rmw_wait 函数，传入 eprosima_fastrtps_identifier
  // 作为实现标识符。 Call the __rmw_wait function of the Fast RTPS shared C++ implementation,
  // passing in eprosima_fastrtps_identifier as the implementation identifier.
  return rmw_fastrtps_shared_cpp::__rmw_wait(
      eprosima_fastrtps_identifier, subscriptions, guard_conditions, services, clients, events,
      wait_set, wait_timeout);
}

}  // extern "C"
