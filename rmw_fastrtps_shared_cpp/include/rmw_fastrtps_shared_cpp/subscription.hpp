// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#ifndef RMW_FASTRTPS_SHARED_CPP__SUBSCRIPTION_HPP_
#define RMW_FASTRTPS_SHARED_CPP__SUBSCRIPTION_HPP_

#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/visibility_control.h"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 初始化用于借贷的订阅 (Initialize the subscription for loans)
 *
 * @param[in] subscription 指向要初始化的订阅对象的指针 (Pointer to the subscription object to be
 * initialized)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
void __init_subscription_for_loans(rmw_subscription_t* subscription);

/**
 * @brief 销毁订阅 (Destroy the subscription)
 *
 * @param[in] identifier 用于标识 rmw 实现的字符串 (String used to identify the rmw implementation)
 * @param[in] participant_info 指向自定义参与者信息结构体的指针 (Pointer to the custom participant
 * info structure)
 * @param[in,out] subscription 要销毁的订阅对象 (The subscription object to be destroyed)
 * @param[in] reset_cft 是否重置内容过滤主题，默认为 false (Whether to reset the content filtered
 * topic, default is false)
 * @return rmw_ret_t 返回操作结果，成功返回 RMW_RET_OK，失败返回相应错误代码 (Return the operation
 * result, success returns RMW_RET_OK, failure returns the corresponding error code)
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t destroy_subscription(
    const char* identifier,
    CustomParticipantInfo* participant_info,
    rmw_subscription_t* subscription,
    bool reset_cft = false);

}  // namespace rmw_fastrtps_shared_cpp

#endif  // RMW_FASTRTPS_SHARED_CPP__SUBSCRIPTION_HPP_
