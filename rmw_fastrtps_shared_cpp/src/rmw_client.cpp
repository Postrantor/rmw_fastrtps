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

#include <string>

#include "rcutils/logging_macros.h"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"
#include "rmw_fastrtps_shared_cpp/custom_client_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/namespace_prefix.hpp"
#include "rmw_fastrtps_shared_cpp/qos.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"
#include "rmw_fastrtps_shared_cpp/utils.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 销毁一个客户端 (Destroy a client)
 *
 * @param[in] identifier 用于创建 rmw_gid 的标识符 (Identifier used to create rmw_gid)
 * @param[in] node 要销毁的客户端所属的节点 (The node to which the client to be destroyed belongs)
 * @param[in] client 要销毁的客户端 (The client to be destroyed)
 * @return rmw_ret_t 操作结果 (Operation result)
 */
rmw_ret_t __rmw_destroy_client(const char *identifier, rmw_node_t *node, rmw_client_t *client) {
  // 初始化最终返回值为成功 (Initialize the final return value as successful)
  rmw_ret_t final_ret = RMW_RET_OK;
  // 获取节点上下文中的公共上下文 (Get the common context in the node context)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);
  // 获取参与者信息 (Get participant information)
  auto participant_info =
      static_cast<CustomParticipantInfo *>(node->context->impl->participant_info);
  // 获取客户端信息 (Get client information)
  auto info = static_cast<CustomClientInfo *>(client->data);

  {
    // 更新图 (Update graph)
    std::lock_guard<std::mutex> guard(common_context->node_update_mutex);
    // 创建请求 DataWriter 的 gid (Create gid for request DataWriter)
    rmw_gid_t gid =
        rmw_fastrtps_shared_cpp::create_rmw_gid(identifier, info->request_writer_->guid());
    // 从图缓存中解除关联 DataWriter (Dissociate DataWriter from graph cache)
    common_context->graph_cache.dissociate_writer(
        gid, common_context->gid, node->name, node->namespace_);
    // 创建响应 DataReader 的 gid (Create gid for response DataReader)
    gid = rmw_fastrtps_shared_cpp::create_rmw_gid(identifier, info->response_reader_->guid());
    // 从图缓存中解除关联 DataReader (Dissociate DataReader from graph cache)
    rmw_dds_common::msg::ParticipantEntitiesInfo msg =
        common_context->graph_cache.dissociate_reader(
            gid, common_context->gid, node->name, node->namespace_);
    // 发布更新后的图信息 (Publish the updated graph information)
    final_ret = rmw_fastrtps_shared_cpp::__rmw_publish(
        identifier, common_context->pub, static_cast<void *>(&msg), nullptr);
  }

  // 显示之前的错误 (Show previous error)
  auto show_previous_error = [&final_ret]() {
    if (RMW_RET_OK != final_ret) {
      RMW_SAFE_FWRITE_TO_STDERR(rmw_get_error_string().str);
      RMW_SAFE_FWRITE_TO_STDERR(" during '" RCUTILS_STRINGIFY(__function__) "'\n");
      rmw_reset_error();
    }
  };

  /////
  // 删除 DataWriter 和 DataReader (Delete DataWriter and DataReader)
  {
    std::lock_guard<std::mutex> lck(participant_info->entity_creation_mutex_);

    // 删除 DataReader (Delete DataReader)
    ReturnCode_t ret = participant_info->subscriber_->delete_datareader(info->response_reader_);
    if (ret != ReturnCode_t::RETCODE_OK) {
      show_previous_error();
      RMW_SET_ERROR_MSG("destroy_client() failed to delete datareader");
      final_ret = RMW_RET_ERROR;
      info->response_reader_->set_listener(nullptr);
    }

    // 删除 DataReader 监听器 (Delete DataReader listener)
    if (nullptr != info->listener_) {
      delete info->listener_;
    }

    // 删除 DataWriter (Delete DataWriter)
    ret = participant_info->publisher_->delete_datawriter(info->request_writer_);
    if (ret != ReturnCode_t::RETCODE_OK) {
      show_previous_error();
      RMW_SET_ERROR_MSG("destroy_client() failed to delete datawriter");
      final_ret = RMW_RET_ERROR;
      info->request_writer_->set_listener(nullptr);
    }

    // 删除 DataWriter 监听器 (Delete DataWriter listener)
    if (nullptr != info->pub_listener_) {
      delete info->pub_listener_;
    }

    // 删除主题和注销类型 (Remove topics and unregister types)
    remove_topic_and_type(
        participant_info, nullptr, info->request_topic_, info->request_type_support_);
    remove_topic_and_type(
        participant_info, nullptr, info->response_topic_, info->response_type_support_);

    // 删除 CustomClientInfo 结构 (Delete CustomClientInfo structure)
    delete info;
  }

  // 释放客户端服务名内存 (Free the memory of the client service name)
  rmw_free(const_cast<char *>(client->service_name));
  // 释放客户端内存 (Free the client memory)
  rmw_client_free(client);

  // 完成时返回错误 (Return error on completion)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_ERROR);
  return final_ret;
}

/**
 * @brief 获取客户端请求发布者的实际QoS配置 (Get the actual QoS configuration of the client request
 * publisher)
 *
 * @param[in] client 指向rmw_client_t结构体的指针 (Pointer to the rmw_client_t structure)
 * @param[out] qos 用于存储实际QoS配置的rmw_qos_profile_t结构体指针 (Pointer to the
 * rmw_qos_profile_t structure for storing the actual QoS configuration)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t __rmw_client_request_publisher_get_actual_qos(
    const rmw_client_t *client, rmw_qos_profile_t *qos) {
  // 将client->data转换为CustomClientInfo类型指针 (Convert client->data to a CustomClientInfo type
  // pointer)
  auto cli = static_cast<CustomClientInfo *>(client->data);
  // 获取请求发布者的Fast-DDS DataWriter对象 (Get the Fast-DDS DataWriter object of the request
  // publisher)
  eprosima::fastdds::dds::DataWriter *fastdds_rw = cli->request_writer_;
  // 将Fast-DDS QoS配置转换为RMW QoS配置 (Convert Fast-DDS QoS configuration to RMW QoS
  // configuration)
  dds_qos_to_rmw_qos(fastdds_rw->get_qos(), qos);
  // 返回操作成功 (Return operation success)
  return RMW_RET_OK;
}

/**
 * @brief 获取客户端响应订阅者的实际QoS配置 (Get the actual QoS configuration of the client response
 * subscriber)
 *
 * @param[in] client 指向rmw_client_t结构体的指针 (Pointer to the rmw_client_t structure)
 * @param[out] qos 用于存储实际QoS配置的rmw_qos_profile_t结构体指针 (Pointer to the
 * rmw_qos_profile_t structure for storing the actual QoS configuration)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t __rmw_client_response_subscription_get_actual_qos(
    const rmw_client_t *client, rmw_qos_profile_t *qos) {
  // 将client->data转换为CustomClientInfo类型指针 (Convert client->data to a CustomClientInfo type
  // pointer)
  auto cli = static_cast<CustomClientInfo *>(client->data);
  // 获取响应订阅者的Fast-DDS DataReader对象 (Get the Fast-DDS DataReader object of the response
  // subscriber)
  eprosima::fastdds::dds::DataReader *fastdds_dr = cli->response_reader_;
  // 将Fast-DDS QoS配置转换为RMW QoS配置 (Convert Fast-DDS QoS configuration to RMW QoS
  // configuration)
  dds_qos_to_rmw_qos(fastdds_dr->get_qos(), qos);
  // 返回操作成功 (Return operation success)
  return RMW_RET_OK;
}

/**
 * @brief 设置客户端新响应回调函数 (Set the new response callback function for the client)
 *
 * @param[in,out] rmw_client 指向rmw_client_t结构体的指针 (Pointer to the rmw_client_t structure)
 * @param[in] callback 新响应事件回调函数 (New response event callback function)
 * @param[in] user_data 用户数据，将传递给回调函数 (User data that will be passed to the callback
 * function)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t __rmw_client_set_on_new_response_callback(
    rmw_client_t *rmw_client, rmw_event_callback_t callback, const void *user_data) {
  // 将rmw_client->data转换为CustomClientInfo类型指针 (Convert rmw_client->data to a
  // CustomClientInfo type pointer)
  auto custom_client_info = static_cast<CustomClientInfo *>(rmw_client->data);
  // 设置新响应回调函数和用户数据 (Set the new response callback function and user data)
  custom_client_info->listener_->set_on_new_response_callback(user_data, callback);
  // 返回操作成功 (Return operation success)
  return RMW_RET_OK;
}

}  // namespace rmw_fastrtps_shared_cpp
