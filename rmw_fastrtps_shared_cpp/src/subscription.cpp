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

#include "rmw_fastrtps_shared_cpp/subscription.hpp"

#include <string>
#include <utility>

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_subscriber_info.hpp"
#include "rmw_fastrtps_shared_cpp/namespace_prefix.hpp"
#include "rmw_fastrtps_shared_cpp/qos.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/utils.hpp"

namespace rmw_fastrtps_shared_cpp {

/*
  > [!NOTE]:
  > `dds/cft_*_.md`
  > Content Filtered Topic
  用于过滤感兴趣的数据，这样订阅者（Subscriber）只收到它真正关心的部分数据。CFT
  的实现依赖于数据的主题（Topic）和一个过滤器（Filter），过滤器使用布尔表达式（Boolean
  expression）来定义过滤规则。通过在传输过程中应用这些规则，可以提高系统的效率和带宽利用率。
*/
/**
 * @brief 销毁订阅者 (Destroy a subscription)
 *
 * @param[in] identifier 指定的实现标识符 (The implementation identifier specified)
 * @param[in] participant_info 自定义参与者信息 (Custom participant information)
 * @param[in,out] subscription 要销毁的订阅者 (The subscription to be destroyed)
 * @param[in] reset_cft 是否重置内容过滤主题 (Whether to reset the content filtered topic)
 * @return rmw_ret_t RMW_RET_OK 表示成功，其他值表示失败 (RMW_RET_OK indicates success, other values
 * indicate failure)
 */
rmw_ret_t destroy_subscription(
    const char *identifier,
    CustomParticipantInfo *participant_info,
    rmw_subscription_t *subscription,
    bool reset_cft) {
  // 确保订阅者的实现标识符与指定的标识符匹配 (Ensure the subscriber's implementation identifier
  // matches the specified one)
  assert(subscription->implementation_identifier == identifier);
  static_cast<void>(identifier);

  {
    // 使用互斥锁保护实体创建过程 (Protect entity creation process with a mutex lock)
    std::lock_guard<std::mutex> lck(participant_info->entity_creation_mutex_);

    // 获取 RMW 订阅者 (Get the RMW Subscriber)
    auto info = static_cast<CustomSubscriberInfo *>(subscription->data);

    // 删除 DataReader (Delete the DataReader)
    ReturnCode_t ret = participant_info->subscriber_->delete_datareader(info->data_reader_);
    if (ReturnCode_t::RETCODE_OK != ret) {
      RMW_SET_ERROR_MSG("Failed to delete datareader");
      // 这是此函数的第一个失败，我们尚未更改状态。这意味着返回错误应该是安全的 (This is the first
      // failure in this function, and we have not changed state. This means it should be safe to
      // return an error)
      return RMW_RET_ERROR;
    }

    // 删除 ContentFilteredTopic (Delete the ContentFilteredTopic)
    if (nullptr != info->filtered_topic_) {
      participant_info->participant_->delete_contentfilteredtopic(info->filtered_topic_);
      info->filtered_topic_ = nullptr;
    }

    // 如果需要重置内容过滤主题，则返回成功 (If resetting the content filtered topic is required,
    // return success)
    if (reset_cft) {
      return RMW_RET_OK;
    }

    // 删除 DataReader 监听器 (Delete the DataReader listener)
    delete info->data_reader_listener_;

    // 删除主题并注销类型 (Remove the topic and unregister the type)
    remove_topic_and_type(
        participant_info, info->subscription_event_, info->topic_, info->type_support_);

    // 删除订阅事件 (Delete the subscription event)
    delete info->subscription_event_;

    // 删除 CustomSubscriberInfo 结构 (Delete the CustomSubscriberInfo structure)
    delete info;
  }

  // 释放订阅者的主题名称内存 (Free the memory of the subscriber's topic name)
  rmw_free(const_cast<char *>(subscription->topic_name));
  // 释放订阅者内存 (Free the subscription memory)
  rmw_subscription_free(subscription);

  // 完成时可以返回错误 (Can return with an error upon completion)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_ERROR);
  return RMW_RET_OK;
}
}  // namespace rmw_fastrtps_shared_cpp
