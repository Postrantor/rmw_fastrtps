// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#include <array>
#include <mutex>
#include <set>
#include <string>
#include <utility>

#include "rcpputils/scope_exit.hpp"
#include "rcutils/logging_macros.h"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw/validate_namespace.h"
#include "rmw/validate_node_name.h"
#include "rmw_dds_common/context.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 创建一个 ROS2 节点 (Create a ROS2 node)
 *
 * @param[in] context 指向 rmw_context_t 结构体的指针，包含了 ROS2 节点所需的上下文信息 (Pointer to
 * the rmw_context_t structure, containing the context information required for the ROS2 node)
 * @param[in] identifier 节点的实现标识符，用于确保与上下文中的实现标识符相匹配 (Implementation
 * identifier of the node, used to ensure it matches the implementation identifier in the context)
 * @param[in] name 节点的名称 (Name of the node)
 * @param[in] namespace_ 节点的命名空间 (Namespace of the node)
 * @return 成功时返回一个指向新创建的 rmw_node_t 结构体的指针，否则返回 nullptr (On success, returns
 * a pointer to the newly created rmw_node_t structure; otherwise, returns nullptr)
 */
rmw_node_t *__rmw_create_node(
    rmw_context_t *context, const char *identifier, const char *name, const char *namespace_) {
  // 确保传入的标识符与上下文中的实现标识符相匹配 (Ensure the passed identifier matches the
  // implementation identifier in the context)
  assert(identifier == context->implementation_identifier);

  // 验证节点名称的有效性 (Validate the node name for validity)
  int validation_result = RMW_NODE_NAME_VALID;
  rmw_ret_t ret = rmw_validate_node_name(name, &validation_result, nullptr);
  if (RMW_RET_OK != ret) {
    return nullptr;
  }
  if (RMW_NODE_NAME_VALID != validation_result) {
    const char *reason = rmw_node_name_validation_result_string(validation_result);
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("invalid node name: %s", reason);
    return nullptr;
  }
  // 验证命名空间的有效性 (Validate the namespace for validity)
  validation_result = RMW_NAMESPACE_VALID;
  ret = rmw_validate_namespace(namespace_, &validation_result, nullptr);
  if (RMW_RET_OK != ret) {
    return nullptr;
  }
  if (RMW_NAMESPACE_VALID != validation_result) {
    const char *reason = rmw_node_name_validation_result_string(validation_result);
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("invalid node namespace: %s", reason);
    return nullptr;
  }

  // 获取上下文中的图缓存 (Get the graph cache from the context)
  auto common_context = static_cast<rmw_dds_common::Context *>(context->impl->common);
  rmw_dds_common::GraphCache &graph_cache = common_context->graph_cache;

  // 分配一个新的 rmw_node_t 结构体 (Allocate a new rmw_node_t structure)
  rmw_node_t *node_handle = rmw_node_allocate();
  if (nullptr == node_handle) {
    RMW_SET_ERROR_MSG("failed to allocate node");
    return nullptr;
  }

  // 创建一个清理节点的作用域退出对象 (Create a scope exit object for cleaning up the node)
  auto cleanup_node = rcpputils::make_scope_exit([node_handle]() {
    rmw_free(const_cast<char *>(node_handle->name));
    rmw_free(const_cast<char *>(node_handle->namespace_));
    rmw_node_free(node_handle);
  });

  // 设置节点的实现标识符 (Set the node's implementation identifier)
  node_handle->implementation_identifier = identifier;
  node_handle->data = nullptr;

  // 分配并复制节点名称 (Allocate and copy the node name)
  node_handle->name = static_cast<const char *>(rmw_allocate(sizeof(char) * strlen(name) + 1));
  if (nullptr == node_handle->name) {
    RMW_SET_ERROR_MSG("failed to copy node name");
    return nullptr;
  }
  memcpy(const_cast<char *>(node_handle->name), name, strlen(name) + 1);

  // 分配并复制节点命名空间 (Allocate and copy the node namespace)
  node_handle->namespace_ =
      static_cast<const char *>(rmw_allocate(sizeof(char) * strlen(namespace_) + 1));
  if (nullptr == node_handle->namespace_) {
    RMW_SET_ERROR_MSG("failed to copy node namespace");
    return nullptr;
  }
  memcpy(const_cast<char *>(node_handle->namespace_), namespace_, strlen(namespace_) + 1);

  // 设置节点的上下文 (Set the node's context)
  node_handle->context = context;

  // 更新图缓存并发布参与者实体信息 (Update the graph cache and publish participant entities info)
  {
    // Though graph_cache methods are thread safe, both cache update and publishing have to also
    // be atomic.
    // If not, the following race condition is possible:
    // node1-update-get-message / node2-update-get-message / node2-publish / node1-publish
    // In that case, the last message published is not accurate.
    std::lock_guard<std::mutex> guard(common_context->node_update_mutex);
    rmw_dds_common::msg::ParticipantEntitiesInfo participant_msg =
        graph_cache.add_node(common_context->gid, name, namespace_);
    if (RMW_RET_OK != __rmw_publish(
                          node_handle->implementation_identifier, common_context->pub,
                          static_cast<void *>(&participant_msg), nullptr)) {
      return nullptr;
    }
  }

  // 取消节点清理操作 (Cancel the node cleanup operation)
  cleanup_node.cancel();

  // 返回新创建的节点句柄 (Return the newly created node handle)
  return node_handle;
}

/**
 * @brief 销毁一个 ROS2 节点 (Destroy a ROS2 node)
 *
 * @param[in] identifier 节点的实现标识符 (The implementation identifier of the node)
 * @param[in,out] node 要销毁的节点指针 (Pointer to the node to be destroyed)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_destroy_node(const char *identifier, rmw_node_t *node) {
  // 确保节点的实现标识符与传入的标识符相同 (Ensure the node's implementation identifier is the same
  // as the input identifier)
  assert(node->implementation_identifier == identifier);

  // 初始化返回值为成功 (Initialize the return value to success)
  rmw_ret_t ret = RMW_RET_OK;

  // 将节点上下文中的通用实现转换为 rmw_dds_common::Context 类型 (Cast the common implementation in
  // the node context to rmw_dds_common::Context type)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);

  // 获取图缓存引用 (Get the reference to the graph cache)
  rmw_dds_common::GraphCache &graph_cache = common_context->graph_cache;

  // 使用互斥锁保护节点更新操作 (Protect node update operations with a mutex lock)
  {
    std::lock_guard<std::mutex> guard(common_context->node_update_mutex);

    // 从图缓存中移除节点并获取参与者消息 (Remove the node from the graph cache and get the
    // participant message)
    rmw_dds_common::msg::ParticipantEntitiesInfo participant_msg =
        graph_cache.remove_node(common_context->gid, node->name, node->namespace_);

    // 发布参与者消息 (Publish the participant message)
    ret = __rmw_publish(
        identifier, common_context->pub, static_cast<void *>(&participant_msg), nullptr);
  }

  // 释放节点名称内存 (Free the memory of the node name)
  rmw_free(const_cast<char *>(node->name));

  // 释放节点命名空间内存 (Free the memory of the node namespace)
  rmw_free(const_cast<char *>(node->namespace_));

  // 释放节点内存 (Free the memory of the node)
  rmw_node_free(node);

  // 返回操作结果 (Return the operation result)
  return ret;
}

/**
 * @brief 获取节点的图形保护条件 (Get the graph guard condition of the node)
 *
 * @param[in] node 要获取图形保护条件的节点指针 (Pointer to the node to get the graph guard
 * condition)
 * @return const rmw_guard_condition_t* 返回节点的图形保护条件指针 (Return the pointer to the node's
 * graph guard condition)
 */
const rmw_guard_condition_t *__rmw_node_get_graph_guard_condition(const rmw_node_t *node) {
  // 将节点上下文中的通用实现转换为 rmw_dds_common::Context 类型 (Cast the common implementation in
  // the node context to rmw_dds_common::Context type)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);

  // 检查通用上下文是否为空 (Check if the common context is null)
  if (!common_context) {
    RMW_SET_ERROR_MSG("common_context is nullptr");
    return nullptr;
  }

  // 返回节点的图形保护条件 (Return the graph guard condition of the node)
  return common_context->graph_guard_condition;
}

}  // namespace rmw_fastrtps_shared_cpp
