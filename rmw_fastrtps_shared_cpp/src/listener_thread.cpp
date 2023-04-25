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

#include "rmw_fastrtps_shared_cpp/listener_thread.hpp"

#include <atomic>
#include <cassert>
#include <cstring>
#include <thread>

#include "rcutils/macros.h"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/init.h"
#include "rmw/ret_types.h"
#include "rmw/rmw.h"
#include "rmw/types.h"
#include "rmw_dds_common/context.hpp"
#include "rmw_dds_common/gid_utils.hpp"
#include "rmw_dds_common/msg/participant_entities_info.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"

using rmw_dds_common::operator<<;

static void node_listener(rmw_context_t *context);

/**
 * @brief 运行监听器线程 (Run the listener thread)
 *
 * @param[in] context 指向 rmw_context_t 结构体的指针 (Pointer to an rmw_context_t structure)
 * @return rmw_ret_t 返回执行结果 (Return the execution result)
 */
rmw_ret_t rmw_fastrtps_shared_cpp::run_listener_thread(rmw_context_t *context) {
  // 检查是否可以返回错误 (Check if it can return with an error)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_ERROR);

  // 将通用上下文转换为 rmw_dds_common::Context 类型 (Cast the common context to
  // rmw_dds_common::Context type)
  auto common_context = static_cast<rmw_dds_common::Context *>(context->impl->common);
  // 设置线程运行状态为 true (Set the thread running status to true)
  common_context->thread_is_running.store(true);
  // 创建一个守护条件 (Create a guard condition)
  common_context->listener_thread_gc =
      rmw_fastrtps_shared_cpp::__rmw_create_guard_condition(context->implementation_identifier);
  // 如果成功创建守护条件 (If the guard condition is created successfully)
  if (common_context->listener_thread_gc) {
    try {
      // 创建一个新的线程并运行 node_listener 函数 (Create a new thread and run the node_listener
      // function)
      common_context->listener_thread = std::thread(node_listener, context);
      // 返回成功状态 (Return success status)
      return RMW_RET_OK;
    } catch (const std::exception &exc) {
      // 设置错误消息，说明无法创建 std::thread (Set error message indicating failure to create
      // std::thread)
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("Failed to create std::thread: %s", exc.what());
    } catch (...) {
      // 设置错误消息，说明无法创建 std::thread (Set error message indicating failure to create
      // std::thread)
      RMW_SET_ERROR_MSG("Failed to create std::thread");
    }
  } else {
    // 设置错误消息，说明无法创建守护条件 (Set error message indicating failure to create guard
    // condition)
    RMW_SET_ERROR_MSG("Failed to create guard condition");
  }
  // 设置线程运行状态为 false (Set the thread running status to false)
  common_context->thread_is_running.store(false);
  // 如果守护条件存在 (If the guard condition exists)
  if (common_context->listener_thread_gc) {
    // 销毁守护条件 (Destroy the guard condition)
    if (RMW_RET_OK != rmw_fastrtps_shared_cpp::__rmw_destroy_guard_condition(
                          common_context->listener_thread_gc)) {
      // 输出错误信息 (Output error information)
      RCUTILS_SAFE_FWRITE_TO_STDERR(RCUTILS_STRINGIFY(__FILE__) ":" RCUTILS_STRINGIFY(
          __function__) ":" RCUTILS_STRINGIFY(__LINE__) ": Failed to destroy guard condition");
    }
  }
  // 返回错误状态 (Return error status)
  return RMW_RET_ERROR;
}

/**
 * @brief 加入监听器线程 (Join the listener thread)
 *
 * @param[in] context 指向rmw_context_t类型的指针，用于存储ROS2上下文信息 (Pointer to rmw_context_t
 * type, used to store ROS2 context information)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t rmw_fastrtps_shared_cpp::join_listener_thread(rmw_context_t *context) {
  // 将通用上下文从rmw_context_t转换为rmw_dds_common::Context类型 (Convert the common context from
  // rmw_context_t to rmw_dds_common::Context type)
  auto common_context = static_cast<rmw_dds_common::Context *>(context->impl->common);

  // 设置线程运行状态为false (Set the thread running status to false)
  common_context->thread_is_running.exchange(false);

  // 触发守护条件 (Trigger the guard condition)
  rmw_ret_t rmw_ret = rmw_fastrtps_shared_cpp::__rmw_trigger_guard_condition(
      context->implementation_identifier, common_context->listener_thread_gc);

  // 如果触发守护条件失败，则返回错误码 (If triggering the guard condition fails, return the error
  // code)
  if (RMW_RET_OK != rmw_ret) {
    return rmw_ret;
  }

  try {
    // 尝试加入监听器线程 (Try to join the listener thread)
    common_context->listener_thread.join();
  } catch (const std::exception &exc) {
    // 如果捕获到异常，设置错误消息并返回错误码 (If an exception is caught, set the error message
    // and return the error code)
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("Failed to join std::thread: %s", exc.what());
    return RMW_RET_ERROR;
  } catch (...) {
    // 如果捕获到未知异常，设置错误消息并返回错误码 (If an unknown exception is caught, set the
    // error message and return the error code)
    RMW_SET_ERROR_MSG("Failed to join std::thread");
    return RMW_RET_ERROR;
  }

  // 销毁守护条件 (Destroy the guard condition)
  rmw_ret =
      rmw_fastrtps_shared_cpp::__rmw_destroy_guard_condition(common_context->listener_thread_gc);

  // 如果销毁守护条件失败，则返回错误码 (If destroying the guard condition fails, return the error
  // code)
  if (RMW_RET_OK != rmw_ret) {
    return rmw_ret;
  }

  // 返回操作成功的结果 (Return the successful operation result)
  return RMW_RET_OK;
}

/**
 * @brief 定义一个宏，用于在发生致命错误时记录日志并输出到标准错误流。
 * Define a macro for logging fatal errors and outputting them to the standard error stream.
 *
 * @param msg 要记录的错误消息。The error message to be logged.
 */
#define LOG_THREAD_FATAL_ERROR(msg)                                                                                    \
  {                                                                                                                    \
    /* 将文件名、函数名和行号转换为字符串，并将其与错误消息一起写入标准错误流。 */ \
    /* Convert the file name, function name, and line number to strings, and write them */                             \
    /* along with the error message to the standard error stream. */                                                   \
    RCUTILS_SAFE_FWRITE_TO_STDERR(                                                                                     \
        RCUTILS_STRINGIFY(__FILE__) ":" RCUTILS_STRINGIFY(__function__) ":" RCUTILS_STRINGIFY(                         \
            __LINE__)                                                                                                  \
            RCUTILS_STRINGIFY(msg) ": ros discovery info listener thread will shutdown ...\n");                        \
  }

/**
 * @brief 监听节点的函数，用于处理订阅和守护条件 (Node listener function for handling subscriptions
 * and guard conditions)
 *
 * @param[in] context 指向 rmw_context_t 结构体的指针，包含 ROS2 上下文信息 (Pointer to an
 * rmw_context_t structure containing ROS2 context information)
 */
void node_listener(rmw_context_t *context) {
  // 确保 context 不是空指针 (Ensure that context is not a null pointer)
  assert(nullptr != context);
  // 确保 context->impl 不是空指针 (Ensure that context->impl is not a null pointer)
  assert(nullptr != context->impl);
  // 确保 context->impl->common 不是空指针 (Ensure that context->impl->common is not a null pointer)
  assert(nullptr != context->impl->common);

  // 将 context->impl->common 转换为 rmw_dds_common::Context 类型的指针 (Cast context->impl->common
  // to a pointer of type rmw_dds_common::Context)
  auto common_context = static_cast<rmw_dds_common::Context *>(context->impl->common);

  // 订阅的条件数量为 2 (The number of conditions for a subscription is 2)
  rmw_wait_set_t *wait_set = rmw_fastrtps_shared_cpp::__rmw_create_wait_set(
      context->implementation_identifier, context, 2);

  // 如果 wait_set 是空指针，则创建失败 (If wait_set is a null pointer, the creation failed)
  if (nullptr == wait_set) {
    LOG_THREAD_FATAL_ERROR("failed to create waitset");
    return;
  }

  // 当线程正在运行时 (While the thread is running)
  while (common_context->thread_is_running.load()) {
    // 确保 common_context->sub 不是空指针 (Ensure that common_context->sub is not a null pointer)
    assert(nullptr != common_context->sub);
    // 确保 common_context->sub->data 不是空指针 (Ensure that common_context->sub->data is not a
    // null pointer)
    assert(nullptr != common_context->sub->data);

    // 初始化订阅和守护条件缓冲区 (Initialize subscription and guard condition buffers)
    void *subscriptions_buffer[] = {common_context->sub->data};
    void *guard_conditions_buffer[] = {common_context->listener_thread_gc->data};

    // 初始化 rmw_subscriptions_t 和 rmw_guard_conditions_t 结构体 (Initialize rmw_subscriptions_t
    // and rmw_guard_conditions_t structures)
    rmw_subscriptions_t subscriptions;
    rmw_guard_conditions_t guard_conditions;
    subscriptions.subscriber_count = 1;
    subscriptions.subscribers = subscriptions_buffer;
    guard_conditions.guard_condition_count = 1;
    guard_conditions.guard_conditions = guard_conditions_buffer;

    // 调用 __rmw_wait 函数，等待订阅和守护条件 (Call the __rmw_wait function to wait for
    // subscriptions and guard conditions)
    if (RMW_RET_OK != rmw_fastrtps_shared_cpp::__rmw_wait(
                          context->implementation_identifier, &subscriptions, &guard_conditions,
                          nullptr, nullptr, nullptr, wait_set, nullptr)) {
      LOG_THREAD_FATAL_ERROR("rmw_wait failed");
      break;
    }

    // 如果订阅缓冲区中有数据 (If there is data in the subscription buffer)
    if (subscriptions_buffer[0]) {
      rmw_dds_common::msg::ParticipantEntitiesInfo msg;
      bool taken = true;

      // 当数据被获取时 (While data is taken)
      while (taken) {
        if (RMW_RET_OK != rmw_fastrtps_shared_cpp::__rmw_take(
                              context->implementation_identifier, common_context->sub,
                              static_cast<void *>(&msg), &taken, nullptr)) {
          LOG_THREAD_FATAL_ERROR("__rmw_take failed");
          break;
        }

        // 如果成功获取数据 (If data is successfully taken)
        if (taken) {
          // 如果消息是本地消息，则忽略 (If the message is a local message, ignore it)
          if (std::memcmp(
                  reinterpret_cast<char *>(common_context->gid.data),
                  reinterpret_cast<char *>(&msg.gid.data), RMW_GID_STORAGE_SIZE) == 0) {
            continue;
          }
          // 更新参与者实体信息 (Update participant entities information)
          common_context->graph_cache.update_participant_entities(msg);
        }
      }
    }
  }

  // 销毁 wait_set (Destroy the wait_set)
  if (RMW_RET_OK != rmw_fastrtps_shared_cpp::__rmw_destroy_wait_set(
                        context->implementation_identifier, wait_set)) {
    LOG_THREAD_FATAL_ERROR("failed to destroy waitset");
  }
}
