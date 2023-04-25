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

#include "rmw_fastrtps_shared_cpp/init_rmw_context_impl.hpp"

#include <cassert>

#include "rmw/error_handling.h"
#include "rmw/init.h"
#include "rmw_dds_common/context.hpp"
#include "rmw_dds_common/msg/participant_entities_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/listener_thread.hpp"
#include "rmw_fastrtps_shared_cpp/participant.hpp"
#include "rmw_fastrtps_shared_cpp/publisher.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"
#include "rmw_fastrtps_shared_cpp/subscription.hpp"

/**
 * @brief 减少 context 实现的引用计数 (Decrease the reference count of the context implementation)
 *
 * @param[in] context 指向 rmw_context_t 结构体的指针 (Pointer to an rmw_context_t structure)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_fastrtps_shared_cpp::decrement_context_impl_ref_count(rmw_context_t *context) {
  // 对 context->impl->mutex 上锁，防止多线程竞争 (Lock the context->impl->mutex to prevent
  // multi-threading competition)
  std::lock_guard<std::mutex> guard(context->impl->mutex);

  // 断言检查 context 及其内部结构是否有效 (Assert to check if context and its internal structures
  // are valid)
  assert(context);
  assert(context->impl);
  assert(0u < context->impl->count);

  // 如果引用计数大于 1，则减少引用计数并返回 RMW_RET_OK (If the reference count is greater than 1,
  // decrease the reference count and return RMW_RET_OK)
  if (--context->impl->count > 0) {
    return RMW_RET_OK;
  }

  rmw_ret_t err = RMW_RET_OK;
  rmw_ret_t ret = RMW_RET_OK;
  rmw_error_string_t error_string;

  // 加入监听线程 (Join the listener thread)
  ret = rmw_fastrtps_shared_cpp::join_listener_thread(context);
  if (RMW_RET_OK != ret) {
    return ret;
  }

  // 转换 context->impl->common 和 context->impl->participant_info 的类型 (Type conversion for
  // context->impl->common and context->impl->participant_info)
  auto common_context = static_cast<rmw_dds_common::Context *>(context->impl->common);
  auto participant_info = static_cast<CustomParticipantInfo *>(context->impl->participant_info);

  // 从 graph_cache 中移除参与者 (Remove participant from graph_cache)
  if (!common_context->graph_cache.remove_participant(common_context->gid)) {
    RMW_SAFE_FWRITE_TO_STDERR(RCUTILS_STRINGIFY(__function__) ":" RCUTILS_STRINGIFY(
        __line__) ": "
                  "couldn't remove Participant gid from graph_cache when destroying Participant");
  }

  // 销毁订阅者 (Destroy the subscription)
  ret = rmw_fastrtps_shared_cpp::destroy_subscription(
      context->implementation_identifier, participant_info, common_context->sub);
  // 如果上述操作失败，尝试清理其他对象 (If the above operation fails, try to clean up other
  // objects)
  if (RMW_RET_OK != ret) {
    error_string = rmw_get_error_string();
    rmw_reset_error();
  }
  // 销毁发布者 (Destroy the publisher)
  err = rmw_fastrtps_shared_cpp::destroy_publisher(
      context->implementation_identifier, participant_info, common_context->pub);
  if (RMW_RET_OK != ret && RMW_RET_OK != err) {
    // 我们只能返回一个错误，记录之前的错误 (We can only return one error, log about the previous
    // one)
    RMW_SAFE_FWRITE_TO_STDERR(RCUTILS_STRINGIFY(__function__) ":" RCUTILS_STRINGIFY(
        __LINE__) ": 'destroy_subscription' failed\n");
    ret = err;
    error_string = rmw_get_error_string();
    rmw_reset_error();
  }
  // 销毁参与者 (Destroy the participant)
  err = rmw_fastrtps_shared_cpp::destroy_participant(participant_info);
  if (RMW_RET_OK != ret && RMW_RET_OK != err) {
    RMW_SAFE_FWRITE_TO_STDERR(RCUTILS_STRINGIFY(__function__) ":" RCUTILS_STRINGIFY(
        __LINE__) ": 'destroy_publisher' failed\n");
    ret = err;
  } else if (RMW_RET_OK != ret) {
    RMW_SET_ERROR_MSG(error_string.str);
  }

  // 清除图形缓存中的更改回调 (Clear the change callback in the graph cache)
  common_context->graph_cache.clear_on_change_callback();
  // 销毁图形保护条件 (Destroy the graph guard condition)
  if (RMW_RET_OK != rmw_fastrtps_shared_cpp::__rmw_destroy_guard_condition(
                        common_context->graph_guard_condition)) {
    RMW_SAFE_FWRITE_TO_STDERR(RCUTILS_STRINGIFY(__function__) ":" RCUTILS_STRINGIFY(
        __line__) ": "
                  "couldn't destroy graph_guard_condtion");
  }

  // 删除 common_context，并将其指针设置为 nullptr (Delete common_context and set its pointer to
  // nullptr)
  delete common_context;
  context->impl->common = nullptr;
  context->impl->participant_info = nullptr;
  return ret;
}
