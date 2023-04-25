// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#ifndef RMW_FASTRTPS_SHARED_CPP__LISTENER_THREAD_HPP_
#define RMW_FASTRTPS_SHARED_CPP__LISTENER_THREAD_HPP_

#include "rmw/init.h"
#include "rmw_fastrtps_shared_cpp/visibility_control.h"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 启动监听线程 (Start the listener thread)
 *
 * @param[in] context ROS2 上下文对象指针 (Pointer to the ROS2 context object)
 * @return rmw_ret_t 返回操作结果 (Returns the operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t run_listener_thread(rmw_context_t* context);

/**
 * @brief 加入监听线程 (Join the listener thread)
 *
 * @param[in] context ROS2 上下文对象指针 (Pointer to the ROS2 context object)
 * @return rmw_ret_t 返回操作结果 (Returns the operation result)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t join_listener_thread(rmw_context_t* context);

}  // namespace rmw_fastrtps_shared_cpp
#endif  // RMW_FASTRTPS_SHARED_CPP__LISTENER_THREAD_HPP_
