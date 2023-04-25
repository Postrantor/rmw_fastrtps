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

#include <algorithm>
#include <array>
#include <cassert>
#include <condition_variable>
#include <limits>
#include <list>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "rcutils/logging_macros.h"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"
#include "rmw_fastrtps_shared_cpp/create_rmw_gid.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_service_info.hpp"
#include "rmw_fastrtps_shared_cpp/namespace_prefix.hpp"
#include "rmw_fastrtps_shared_cpp/qos.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_context_impl.hpp"
#include "rmw_fastrtps_shared_cpp/utils.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 销毁一个 ROS 服务 (Destroy a ROS service)
 *
 * @param[in] identifier 用于标识 rmw 实现的字符串 (A string to identify the rmw implementation)
 * @param[in] node 指向要销毁服务的节点的指针 (Pointer to the node where the service is being
 * destroyed)
 * @param[in] service 要销毁的服务的指针 (Pointer to the service to be destroyed)
 * @return rmw_ret_t 返回操作结果 (Return the result of the operation)
 */
rmw_ret_t __rmw_destroy_service(const char *identifier, rmw_node_t *node, rmw_service_t *service) {
  // 初始化最终返回值为成功 (Initialize the final return value as successful)
  rmw_ret_t final_ret = RMW_RET_OK;

  // 获取节点上下文 (Get the node context)
  auto common_context = static_cast<rmw_dds_common::Context *>(node->context->impl->common);
  // 获取参与者信息 (Get participant information)
  auto participant_info =
      static_cast<CustomParticipantInfo *>(node->context->impl->participant_info);
  // 获取自定义服务信息 (Get custom service information)
  auto info = static_cast<CustomServiceInfo *>(service->data);

  // 更新图 (Update graph)
  {
    std::lock_guard<std::mutex> guard(common_context->node_update_mutex);
    rmw_gid_t gid =
        rmw_fastrtps_shared_cpp::create_rmw_gid(identifier, info->request_reader_->guid());
    common_context->graph_cache.dissociate_reader(
        gid, common_context->gid, node->name, node->namespace_);
    gid = rmw_fastrtps_shared_cpp::create_rmw_gid(identifier, info->response_writer_->guid());
    rmw_dds_common::msg::ParticipantEntitiesInfo msg =
        common_context->graph_cache.dissociate_writer(
            gid, common_context->gid, node->name, node->namespace_);
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

  // 删除 DataWriter 和 DataReader (Delete DataWriter and DataReader)
  {
    std::lock_guard<std::mutex> lck(participant_info->entity_creation_mutex_);

    // 删除 DataReader (Delete DataReader)
    ReturnCode_t ret = participant_info->subscriber_->delete_datareader(info->request_reader_);
    if (ret != ReturnCode_t::RETCODE_OK) {
      show_previous_error();
      RMW_SET_ERROR_MSG("Fail in delete datareader");
      final_ret = RMW_RET_ERROR;
      info->request_reader_->set_listener(nullptr);
    }

    // 删除 DataReader 监听器 (Delete DataReader listener)
    if (nullptr != info->listener_) {
      delete info->listener_;
      info->listener_ = nullptr;
    }

    // 删除 DataWriter (Delete DataWriter)
    ret = participant_info->publisher_->delete_datawriter(info->response_writer_);
    if (ret != ReturnCode_t::RETCODE_OK) {
      show_previous_error();
      RMW_SET_ERROR_MSG("Fail in delete datawriter");
      final_ret = RMW_RET_ERROR;
      info->response_writer_->set_listener(nullptr);
    }

    // 删除 DataWriter 监听器 (Delete DataWriter listener)
    if (nullptr != info->pub_listener_) {
      delete info->pub_listener_;
      info->pub_listener_ = nullptr;
    }

    // 删除主题和注销类型 (Remove topics and unregister types)
    remove_topic_and_type(
        participant_info, nullptr, info->request_topic_, info->request_type_support_);
    remove_topic_and_type(
        participant_info, nullptr, info->response_topic_, info->response_type_support_);

    // 删除 CustomServiceInfo 结构 (Delete CustomServiceInfo structure)
    delete info;
  }

  // 释放服务名称内存 (Free the memory of service name)
  rmw_free(const_cast<char *>(service->service_name));
  // 释放服务内存 (Free the memory of service)
  rmw_service_free(service);

  // 完成时返回错误 (Return error on completion)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_ERROR);
  return final_ret;
}

/**
 * @brief 获取服务响应发布者的实际QoS配置 (Get the actual QoS settings of the service response
 * publisher)
 *
 * @param[in] service 服务指针，用于获取服务相关信息 (Pointer to the service, used to get
 * service-related information)
 * @param[out] qos 存储实际QoS配置的指针 (Pointer to store the actual QoS settings)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_service_response_publisher_get_actual_qos(
    const rmw_service_t *service, rmw_qos_profile_t *qos) {
  // 将服务数据转换为自定义服务信息类型 (Cast the service data to the CustomServiceInfo type)
  auto srv = static_cast<CustomServiceInfo *>(service->data);
  // 获取服务响应写入器 (Get the service response writer)
  eprosima::fastdds::dds::DataWriter *fastdds_rw = srv->response_writer_;
  // 将DDS QoS配置转换为RMW QoS配置 (Convert DDS QoS settings to RMW QoS settings)
  dds_qos_to_rmw_qos(fastdds_rw->get_qos(), qos);
  // 返回操作成功 (Return operation success)
  return RMW_RET_OK;
}

/**
 * @brief 获取服务请求订阅者的实际QoS配置 (Get the actual QoS settings of the service request
 * subscriber)
 *
 * @param[in] service 服务指针，用于获取服务相关信息 (Pointer to the service, used to get
 * service-related information)
 * @param[out] qos 存储实际QoS配置的指针 (Pointer to store the actual QoS settings)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_service_request_subscription_get_actual_qos(
    const rmw_service_t *service, rmw_qos_profile_t *qos) {
  // 将服务数据转换为自定义服务信息类型 (Cast the service data to the CustomServiceInfo type)
  auto srv = static_cast<CustomServiceInfo *>(service->data);
  // 获取服务请求读取器 (Get the service request reader)
  eprosima::fastdds::dds::DataReader *fastdds_rr = srv->request_reader_;
  // 将DDS QoS配置转换为RMW QoS配置 (Convert DDS QoS settings to RMW QoS settings)
  dds_qos_to_rmw_qos(fastdds_rr->get_qos(), qos);
  // 返回操作成功 (Return operation success)
  return RMW_RET_OK;
}

/**
 * @brief 设置服务的新请求回调函数 (Set the new request callback function for the service)
 *
 * @param[in] rmw_service 服务指针，用于获取服务相关信息 (Pointer to the service, used to get
 * service-related information)
 * @param[in] callback 新请求回调函数 (New request callback function)
 * @param[in] user_data 用户数据，传递给回调函数 (User data, passed to the callback function)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_service_set_on_new_request_callback(
    rmw_service_t *rmw_service, rmw_event_callback_t callback, const void *user_data) {
  // 将服务数据转换为自定义服务信息类型 (Cast the service data to the CustomServiceInfo type)
  auto custom_service_info = static_cast<CustomServiceInfo *>(rmw_service->data);
  // 设置新请求回调函数及用户数据 (Set the new request callback function and user data)
  custom_service_info->listener_->set_on_new_request_callback(user_data, callback);
  // 返回操作成功 (Return operation success)
  return RMW_RET_OK;
}

}  // namespace rmw_fastrtps_shared_cpp
