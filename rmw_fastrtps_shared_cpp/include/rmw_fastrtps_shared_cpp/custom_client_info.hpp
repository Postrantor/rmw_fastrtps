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

#ifndef RMW_FASTRTPS_SHARED_CPP__CUSTOM_CLIENT_INFO_HPP_
#define RMW_FASTRTPS_SHARED_CPP__CUSTOM_CLIENT_INFO_HPP_

#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>

#include "fastcdr/FastBuffer.h"
#include "fastdds/dds/core/status/PublicationMatchedStatus.hpp"
#include "fastdds/dds/core/status/SubscriptionMatchedStatus.hpp"
#include "fastdds/dds/publisher/DataWriter.hpp"
#include "fastdds/dds/publisher/DataWriterListener.hpp"
#include "fastdds/dds/subscriber/DataReader.hpp"
#include "fastdds/dds/subscriber/DataReaderListener.hpp"
#include "fastdds/dds/subscriber/SampleInfo.hpp"
#include "fastdds/dds/subscriber/qos/DataReaderQos.hpp"
#include "fastdds/dds/topic/TypeSupport.hpp"
#include "fastdds/rtps/common/Guid.h"
#include "fastdds/rtps/common/InstanceHandle.h"
#include "fastdds/rtps/common/SampleIdentity.h"
#include "rcpputils/thread_safety_annotations.hpp"
#include "rmw/event_callback_type.h"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"

class ClientListener;
class ClientPubListener;

/**
 * @struct CustomClientInfo
 * @brief 定义客户端信息结构体，用于存储与 Cyclone DDS 实现相关的数据。
 *        Defines the client information structure, used to store data related to the Cyclone DDS
 * implementation.
 */
typedef struct CustomClientInfo {
  eprosima::fastdds::dds::TypeSupport request_type_support_{
      nullptr};  ///< 请求类型支持。Request type support.
  const void *request_type_support_impl_{
      nullptr};  ///< 请求类型支持实现。Request type support implementation.
  eprosima::fastdds::dds::TypeSupport response_type_support_{
      nullptr};  ///< 响应类型支持。Response type support.
  const void *response_type_support_impl_{
      nullptr};  ///< 响应类型支持实现。Response type support implementation.
  eprosima::fastdds::dds::DataReader *response_reader_{
      nullptr};  ///< 响应数据读取器。Response data reader.
  eprosima::fastdds::dds::DataWriter *request_writer_{
      nullptr};  ///< 请求数据写入器。Request data writer.

  std::string request_topic_name_;   ///< 请求主题名称。Request topic name.
  std::string response_topic_name_;  ///< 响应主题名称。Response topic name.

  eprosima::fastdds::dds::Topic *request_topic_{nullptr};   ///< 请求主题。Request topic.
  eprosima::fastdds::dds::Topic *response_topic_{nullptr};  ///< 响应主题。Response topic.

  ClientListener *listener_{nullptr};             ///< 客户端监听器。Client listener.
  eprosima::fastrtps::rtps::GUID_t writer_guid_;  ///< 写入器 GUID。Writer GUID.
  eprosima::fastrtps::rtps::GUID_t reader_guid_;  ///< 读取器 GUID。Reader GUID.

  const char *typesupport_identifier_{nullptr};  ///< 类型支持标识符。Type support identifier.
  ClientPubListener *pub_listener_{nullptr};  ///< 客户端发布监听器。Client publication listener.
  std::atomic_size_t response_subscriber_matched_count_;  ///< 响应订阅者匹配计数。Response
                                                          ///< subscriber matched count.
  std::atomic_size_t
      request_publisher_matched_count_;  ///< 请求发布者匹配计数。Request publisher matched count.
} CustomClientInfo;

/**
 * @struct CustomClientResponse
 * @brief 定义客户端响应结构体，用于存储与 Cyclone DDS 实现相关的响应数据。
 *        Defines the client response structure, used to store response data related to the Cyclone
 * DDS implementation.
 */
typedef struct CustomClientResponse {
  eprosima::fastrtps::rtps::SampleIdentity sample_identity_;  ///< 样本标识。Sample identity.
  std::unique_ptr<eprosima::fastcdr::FastBuffer> buffer_;     ///< 快速缓冲区。Fast buffer.
} CustomClientResponse;

/**
 * @class ClientListener
 * @brief 侦听器类，用于处理 eprosima::fastdds::dds::DataReader 相关事件
 *
 * Listener class for handling events related to eprosima::fastdds::dds::DataReader.
 */
class ClientListener : public eprosima::fastdds::dds::DataReaderListener {
public:
  /**
   * @brief 构造函数
   * @param info CustomClientInfo 类型的指针，存储客户端相关信息
   *
   * Constructor.
   * @param info Pointer to CustomClientInfo, storing client-related information.
   */
  explicit ClientListener(CustomClientInfo *info) : info_(info) {}

  /**
   * @brief 当数据可用时触发的回调函数
   * @param 无
   *
   * Callback function triggered when data is available.
   * @param None
   */
  void on_data_available(eprosima::fastdds::dds::DataReader *) {
    std::unique_lock<std::mutex> lock_mutex(on_new_response_m_);  // 对互斥量加锁

    if (on_new_response_cb_) {                         // 如果设置了新响应的回调函数
      auto unread_responses = get_unread_responses();  // 获取未读响应数量

      if (0 < unread_responses) {                      // 如果有未读响应
        on_new_response_cb_(user_data_, unread_responses);  // 调用回调函数
      }
    }
  }

  /**
   * @brief 当订阅匹配时触发的回调函数
   * @param info SubscriptionMatchedStatus 类型，包含匹配状态信息
   *
   * Callback function triggered when a subscription is matched.
   * @param info SubscriptionMatchedStatus type, containing matching status information.
   */
  void on_subscription_matched(
      eprosima::fastdds::dds::DataReader *,
      const eprosima::fastdds::dds::SubscriptionMatchedStatus &info) final {
    if (info_ == nullptr) {  // 如果客户端信息为空，直接返回
      return;
    }
    if (info.current_count_change == 1) {  // 如果当前计数变化为 1
      publishers_.insert(
          eprosima::fastrtps::rtps::iHandle2GUID(info.last_publication_handle));  // 插入发布者
    } else if (info.current_count_change == -1) {  // 如果当前计数变化为 -1
      publishers_.erase(
          eprosima::fastrtps::rtps::iHandle2GUID(info.last_publication_handle));  // 移除发布者
    } else {
      return;  // 其他情况直接返回
    }
    info_->response_subscriber_matched_count_.store(publishers_.size());  // 更新匹配的订阅者数量
  }

  /**
   * @brief 获取未读响应数量
   * @return size_t 类型，表示未读响应数量
   *
   * Get the number of unread responses.
   * @return size_t type, representing the number of unread responses.
   */
  size_t get_unread_responses() { return info_->response_reader_->get_unread_count(true); }

  /**
   * @brief 设置新响应到达时的回调函数
   * @param user_data 用户数据，传递给回调函数
   * @param callback 回调函数，当有新响应到达时触发
   *
   * Set the callback function for when a new response arrives.
   * @param user_data User data passed to the callback function.
   * @param callback Callback function triggered when a new response arrives.
   */
  void set_on_new_response_callback(const void *user_data, rmw_event_callback_t callback) {
    if (callback) {                                                // 如果设置了回调函数
      auto unread_responses = get_unread_responses();              // 获取未读响应数量

      std::lock_guard<std::mutex> lock_mutex(on_new_response_m_);  // 对互斥量加锁

      if (0 < unread_responses) {                                  // 如果有未读响应
        callback(user_data, unread_responses);                     // 调用回调函数
      }

      user_data_ = user_data;          // 设置用户数据
      on_new_response_cb_ = callback;  // 设置新响应回调函数

      eprosima::fastdds::dds::StatusMask status_mask =
          info_->response_reader_->get_status_mask();                       // 获取状态掩码
      status_mask |= eprosima::fastdds::dds::StatusMask::data_available();  // 添加数据可用的状态
      info_->response_reader_->set_listener(this, status_mask);  // 设置监听器及其状态掩码
    } else {                                                     // 如果没有设置回调函数
      std::lock_guard<std::mutex> lock_mutex(on_new_response_m_);  // 对互斥量加锁

      eprosima::fastdds::dds::StatusMask status_mask =
          info_->response_reader_->get_status_mask();                        // 获取状态掩码
      status_mask &= ~eprosima::fastdds::dds::StatusMask::data_available();  // 移除数据可用的状态
      info_->response_reader_->set_listener(this, status_mask);  // 设置监听器及其状态掩码

      user_data_ = nullptr;                                      // 清除用户数据
      on_new_response_cb_ = nullptr;                             // 清除新响应回调函数
    }
  }

private:
  CustomClientInfo *info_;                                 // 存储客户端相关信息的指针

  std::set<eprosima::fastrtps::rtps::GUID_t> publishers_;  // 存储发布者的集合

  rmw_event_callback_t on_new_response_cb_{nullptr};       // 新响应到达时的回调函数

  const void *user_data_{nullptr};                         // 用户数据，传递给回调函数

  std::mutex on_new_response_m_;  // 用于保护新响应回调函数的互斥量
};

/**
 * @class ClientPubListener
 * @brief 客户端发布监听器类，继承自 eprosima::fastdds::dds::DataWriterListener。
 *        Client publication listener class, inherits from
 * eprosima::fastdds::dds::DataWriterListener.
 */
class ClientPubListener : public eprosima::fastdds::dds::DataWriterListener {
public:
  /**
   * @brief 构造函数，初始化客户端信息。
   *        Constructor, initializes the custom client information.
   * @param info 自定义客户端信息指针。
   *             Pointer to the custom client information.
   */
  explicit ClientPubListener(CustomClientInfo *info) : info_(info) {}

  /**
   * @brief 当出版物匹配时的回调函数。
   *        Callback function when a publication is matched.
   * @param writer 未使用的数据写入器指针。
   *               Unused data writer pointer.
   * @param info 出版物匹配状态信息。
   *             Publication matched status information.
   */
  void on_publication_matched(
      eprosima::fastdds::dds::DataWriter * /* writer */,
      const eprosima::fastdds::dds::PublicationMatchedStatus &info) final {
    // 如果客户端信息为空，则返回。
    // If the custom client information is nullptr, return.
    if (info_ == nullptr) {
      return;
    }
    // 如果当前计数变化为1，将最后一个订阅句柄插入到订阅集合中。
    // If the current count change is 1, insert the last subscription handle into the subscriptions
    // set.
    if (info.current_count_change == 1) {
      subscriptions_.insert(eprosima::fastrtps::rtps::iHandle2GUID(info.last_subscription_handle));
    }
    // 如果当前计数变化为-1，从订阅集合中删除最后一个订阅句柄。
    // If the current count change is -1, erase the last subscription handle from the subscriptions
    // set.
    else if (info.current_count_change == -1) {
      subscriptions_.erase(eprosima::fastrtps::rtps::iHandle2GUID(info.last_subscription_handle));
    }
    // 其他情况下返回。
    // In other cases, return.
    else {
      return;
    }
    // 将订阅集合的大小存储到客户端信息的请求发布者匹配计数中。
    // Store the size of the subscriptions set in the custom client information's request publisher
    // matched count.
    info_->request_publisher_matched_count_.store(subscriptions_.size());
  }

private:
  CustomClientInfo *info_;  ///< 自定义客户端信息指针。Pointer to the custom client information.
  std::set<eprosima::fastrtps::rtps::GUID_t> subscriptions_;  ///< 订阅集合。Subscriptions set.
};

#endif  // RMW_FASTRTPS_SHARED_CPP__CUSTOM_CLIENT_INFO_HPP_
