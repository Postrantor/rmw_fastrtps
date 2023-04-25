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

#ifndef RMW_FASTRTPS_SHARED_CPP__RMW_INIT_HPP_
#define RMW_FASTRTPS_SHARED_CPP__RMW_INIT_HPP_

#include "rmw/impl/cpp/macros.hpp"
#include "rmw/init.h"
#include "rmw/init_options.h"
#include "rmw_fastrtps_shared_cpp/visibility_control.h"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 初始化 rmw_init_options_t 结构体 (Initialize the rmw_init_options_t structure)
 *
 * @param[in] identifier 用于标识 ROS2 实现的字符串 (A string to identify the ROS2 implementation)
 * @param[out] init_options 一个指向待初始化的 rmw_init_options_t 结构体的指针 (A pointer to the
 * rmw_init_options_t structure to be initialized)
 * @param[in] allocator 分配器，用于管理内存分配 (The allocator for managing memory allocation)
 * @return 返回 rmw_ret_t 类型的结果，表示操作成功或失败 (Returns a result of type rmw_ret_t,
 * indicating success or failure of the operation)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t rmw_init_options_init(
    const char* identifier, rmw_init_options_t* init_options, rcutils_allocator_t allocator);

/**
 * @brief 复制 rmw_init_options_t 结构体 (Copy the rmw_init_options_t structure)
 *
 * @param[in] identifier 用于标识 ROS2 实现的字符串 (A string to identify the ROS2 implementation)
 * @param[in] src 指向要复制的源 rmw_init_options_t 结构体的指针 (A pointer to the source
 * rmw_init_options_t structure to be copied)
 * @param[out] dst 指向目标 rmw_init_options_t 结构体的指针，用于存储复制的内容 (A pointer to the
 * destination rmw_init_options_t structure, where the copied content will be stored)
 * @return 返回 rmw_ret_t 类型的结果，表示操作成功或失败 (Returns a result of type rmw_ret_t,
 * indicating success or failure of the operation)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t rmw_init_options_copy(
    const char* identifier, const rmw_init_options_t* src, rmw_init_options_t* dst);

/**
 * @brief 释放 rmw_init_options_t 结构体的资源 (Release resources of the rmw_init_options_t
 * structure)
 *
 * @param[in] identifier 用于标识 ROS2 实现的字符串 (A string to identify the ROS2 implementation)
 * @param[out] init_options 指向要释放资源的 rmw_init_options_t 结构体的指针 (A pointer to the
 * rmw_init_options_t structure whose resources are to be released)
 * @return 返回 rmw_ret_t 类型的结果，表示操作成功或失败 (Returns a result of type rmw_ret_t,
 * indicating success or failure of the operation)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t rmw_init_options_fini(const char* identifier, rmw_init_options_t* init_options);

}  // namespace rmw_fastrtps_shared_cpp

#endif  // RMW_FASTRTPS_SHARED_CPP__RMW_INIT_HPP_
