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

#include "rmw_fastrtps_shared_cpp/publisher.hpp"

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_publisher_info.hpp"
#include "rmw_fastrtps_shared_cpp/utils.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 销毁发布者 (Destroy a publisher)
 *
 * @param[in] identifier 发布者的实现标识符 (Implementation identifier of the publisher)
 * @param[in] participant_info 自定义参与者信息结构体指针 (Pointer to the custom participant info
 * structure)
 * @param[in,out] publisher 要销毁的发布者指针 (Pointer to the publisher to be destroyed)
 * @return rmw_ret_t RMW_RET_OK 表示成功，其他值表示失败 (RMW_RET_OK for success, other values for
 * failure)
 */
rmw_ret_t destroy_publisher(
    const char *identifier, CustomParticipantInfo *participant_info, rmw_publisher_t *publisher) {
  // 检查发布者的实现标识符是否匹配 (Check if the publisher's implementation identifier matches)
  assert(publisher->implementation_identifier == identifier);
  static_cast<void>(identifier);

  {
    // 使用互斥锁保护实体创建过程 (Protect entity creation process with a mutex lock)
    std::lock_guard<std::mutex> lck(participant_info->entity_creation_mutex_);

    // 获取 RMW 发布者 (Get the RMW publisher)
    auto info = static_cast<CustomPublisherInfo *>(publisher->data);

    // 删除 DataWriter (Delete the DataWriter)
    ReturnCode_t ret = participant_info->publisher_->delete_datawriter(info->data_writer_);
    if (ReturnCode_t::RETCODE_OK != ret) {
      // 设置错误消息 (Set error message)
      RMW_SET_ERROR_MSG("Failed to delete datawriter");
      // 这是此函数的第一个失败，且我们没有改变状态。这意味着返回错误应该是安全的 (This is the first
      // failure on this function, and we have not changed state. This means it should be safe to
      // return an error)
      return RMW_RET_ERROR;
    }

    // 删除 DataWriter 监听器 (Delete the DataWriter listener)
    delete info->data_writer_listener_;

    // 删除主题并注销类型 (Delete the topic and unregister the type)
    remove_topic_and_type(
        participant_info, info->publisher_event_, info->topic_, info->type_support_);

    // 删除发布者事件 (Delete the publisher event)
    delete info->publisher_event_;

    // 删除 CustomPublisherInfo 结构体 (Delete the CustomPublisherInfo structure)
    delete info;
  }

  // 释放发布者的主题名称内存 (Free the memory of the publisher's topic name)
  rmw_free(const_cast<char *>(publisher->topic_name));
  // 释放发布者内存 (Free the publisher memory)
  rmw_publisher_free(publisher);

  // 完成时返回错误 (Return error on completion)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_ERROR);
  // 返回成功状态 (Return success status)
  return RMW_RET_OK;
}

}  // namespace rmw_fastrtps_shared_cpp
