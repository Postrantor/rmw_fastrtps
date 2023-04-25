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

#ifndef RMW_FASTRTPS_SHARED_CPP__CUSTOM_SERVICE_INFO_HPP_
#define RMW_FASTRTPS_SHARED_CPP__CUSTOM_SERVICE_INFO_HPP_

#include <condition_variable>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

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
#include "rmw_fastrtps_shared_cpp/guid_utils.hpp"

class ServiceListener;
class ServicePubListener;

/**
 * @brief 客户端存在状态枚举 (Client presence status enumeration)
 */
enum class client_present_t {
  FAILURE,  ///< 检查时发生错误 (An error occurred when checking)
  MAYBE,    ///< 读取器未匹配，写入器仍存在 (Reader not matched, writer still present)
  YES,      ///< 读取器匹配成功 (Reader matched)
  GONE      ///< 读取器和写入器都不存在 (Neither reader nor writer)
};

/**
 * @brief 自定义服务信息结构体 (Custom Service Information Structure)
 */
typedef struct CustomServiceInfo {
  eprosima::fastdds::dds::TypeSupport request_type_support_{
      nullptr};  ///< 请求类型支持 (Request type support)
  const void *request_type_support_impl_{
      nullptr};  ///< 请求类型支持实现 (Request type support implementation)
  eprosima::fastdds::dds::TypeSupport response_type_support_{
      nullptr};  ///< 响应类型支持 (Response type support)
  const void *response_type_support_impl_{
      nullptr};  ///< 响应类型支持实现 (Response type support implementation)
  eprosima::fastdds::dds::DataReader *request_reader_{
      nullptr};  ///< 请求数据读取器 (Request data reader)
  eprosima::fastdds::dds::DataWriter *response_writer_{
      nullptr};  ///< 响应数据写入器 (Response data writer)

  eprosima::fastdds::dds::Topic *request_topic_{nullptr};   ///< 请求主题 (Request topic)
  eprosima::fastdds::dds::Topic *response_topic_{nullptr};  ///< 响应主题 (Response topic)

  ServiceListener *listener_{nullptr};                      ///< 服务监听器 (Service listener)
  ServicePubListener *pub_listener_{nullptr};  ///< 服务发布监听器 (Service publication listener)

  const char *typesupport_identifier_{nullptr};  ///< 类型支持标识符 (Type support identifier)
} CustomServiceInfo;

/**
 * @brief 自定义服务请求结构体 (Custom Service Request Structure)
 */
typedef struct CustomServiceRequest {
  eprosima::fastrtps::rtps::SampleIdentity sample_identity_;  ///< 样本身份 (Sample identity)
  eprosima::fastcdr::FastBuffer *buffer_;                     ///< 缓冲区 (Buffer)

  CustomServiceRequest() : buffer_(nullptr) {}                ///< 构造函数 (Constructor)
} CustomServiceRequest;

/**
 * @class ServicePubListener
 * @brief 用于处理服务发布者的事件监听器 (Event listener for handling service publisher events)
 *
 * 继承自 eprosima::fastdds::dds::DataWriterListener (Inherits from
 * eprosima::fastdds::dds::DataWriterListener)
 */
class ServicePubListener : public eprosima::fastdds::dds::DataWriterListener {
  // 使用无序集合存储订阅者的 GUID (Using an unordered set to store the subscriber's GUID)
  using subscriptions_set_t = std::
      unordered_set<eprosima::fastrtps::rtps::GUID_t, rmw_fastrtps_shared_cpp::hash_fastrtps_guid>;

  // 使用无序映射存储客户端端点的 GUID (Using an unordered map to store the client endpoints' GUID)
  using clients_endpoints_map_t = std::unordered_map<
      eprosima::fastrtps::rtps::GUID_t,
      eprosima::fastrtps::rtps::GUID_t,
      rmw_fastrtps_shared_cpp::hash_fastrtps_guid>;

  /**
   * @brief 服务发布监听器类 (Service Publisher Listener class)
   */
public:
  /**
   * @brief 构造函数 (Constructor)
   * @param info 自定义服务信息指针 (Pointer to CustomServiceInfo)
   */
  explicit ServicePubListener(CustomServiceInfo *info) { (void)info; }

  /**
   * @brief 当出版匹配时的回调函数 (Callback function when publication is matched)
   * @param writer 数据写入器指针 (Pointer to DataWriter)
   * @param info 出版匹配状态信息 (PublicationMatchedStatus information)
   */
  void on_publication_matched(
      eprosima::fastdds::dds::DataWriter * /* writer */,
      const eprosima::fastdds::dds::PublicationMatchedStatus &info) final {
    // 对互斥锁进行加锁 (Lock the mutex)
    std::lock_guard<std::mutex> lock(mutex_);
    // 如果当前计数变化为1 (If the current count change is 1)
    if (info.current_count_change == 1) {
      // 将最后一个订阅句柄插入到订阅集合中 (Insert the last subscription handle into the
      // subscriptions set)
      subscriptions_.insert(eprosima::fastrtps::rtps::iHandle2GUID(info.last_subscription_handle));
    } else if (info.current_count_change == -1) {
      // 将要删除的端点 GUID 转换为最后一个订阅句柄 (Convert the endpoint GUID to be deleted to the
      // last subscription handle)
      eprosima::fastrtps::rtps::GUID_t erase_endpoint_guid =
          eprosima::fastrtps::rtps::iHandle2GUID(info.last_subscription_handle);
      // 从订阅集合中删除端点 GUID (Remove the endpoint GUID from the subscriptions set)
      subscriptions_.erase(erase_endpoint_guid);
      // 查找要删除的客户端端点 (Find the client endpoint to be deleted)
      auto endpoint = clients_endpoints_.find(erase_endpoint_guid);
      // 如果找到了要删除的客户端端点 (If the client endpoint to be deleted is found)
      if (endpoint != clients_endpoints_.end()) {
        // 从客户端端点映射中删除该端点 (Remove the endpoint from the client endpoints map)
        clients_endpoints_.erase(endpoint->second);
        clients_endpoints_.erase(erase_endpoint_guid);
      }
    } else {
      // 返回 (Return)
      return;
    }
    // 通知所有等待条件变量的线程 (Notify all threads waiting for the condition variable)
    cv_.notify_all();
  }

  /**
   * @brief 等待订阅 (Wait for subscription)
   * @tparam Rep 时间表示类型 (Time representation type)
   * @tparam Period 时间周期类型 (Time period type)
   * @param guid 要等待的订阅的 GUID (GUID of the subscription to wait for)
   * @param rel_time 相对时间 (Relative time)
   * @return 是否存在订阅 (Whether the subscription exists)
   */
  template <class Rep, class Period>
  bool wait_for_subscription(
      const eprosima::fastrtps::rtps::GUID_t &guid,
      const std::chrono::duration<Rep, Period> &rel_time) {
    // 定义一个检查 GUID 是否存在的 lambda 函数 (Define a lambda function to check if the GUID
    // exists)
    auto guid_is_present = [this, guid]() RCPPUTILS_TSA_REQUIRES(mutex_) -> bool {
      return subscriptions_.find(guid) != subscriptions_.end();
    };

    // 对互斥锁进行加锁 (Lock the mutex)
    std::unique_lock<std::mutex> lock(mutex_);
    // 等待条件变量，直到超时或 GUID 存在 (Wait for the condition variable until timeout or the GUID
    // exists)
    return cv_.wait_for(lock, rel_time, guid_is_present);
  }

  /**
   * @brief 检查订阅是否存在 (Check if the subscription exists)
   * @param guid 要检查的订阅的 GUID (GUID of the subscription to check)
   * @return 订阅是否存在的状态 (Status of whether the subscription exists)
   */
  client_present_t check_for_subscription(const eprosima::fastrtps::rtps::GUID_t &guid) {
    {
      // 对互斥锁进行加锁 (Lock the mutex)
      std::lock_guard<std::mutex> lock(mutex_);
      // 检查 GUID 是否仍在映射中 (Check if the GUID is still in the map)
      if (clients_endpoints_.find(guid) == clients_endpoints_.end()) {
        // 客户端已经消失 (Client is gone)
        return client_present_t::GONE;
      }
    }
    // 等待订阅 (Wait for subscription)
    if (!wait_for_subscription(guid, std::chrono::milliseconds(100))) {
      // 可能存在订阅 (Subscription may exist)
      return client_present_t::MAYBE;
    }
    // 订阅存在 (Subscription exists)
    return client_present_t::YES;
  }

  /**
   * @brief 删除指定的端点，如果存在的话 (Remove the specified endpoint if it exists)
   *
   * @param[in] endpointGuid 要删除的端点的 GUID (The GUID of the endpoint to be removed)
   */
  void endpoint_erase_if_exists(const eprosima::fastrtps::rtps::GUID_t &endpointGuid) {
    std::lock_guard<std::mutex> lock(mutex_);  // 对互斥锁进行上锁 (Lock the mutex)
    auto endpoint =
        clients_endpoints_.find(endpointGuid);  // 查找指定的端点 (Find the specified endpoint)

    // 如果找到了指定的端点 (If the specified endpoint is found)
    if (endpoint != clients_endpoints_.end()) {
      clients_endpoints_.erase(
          endpoint->second);  // 删除与之关联的端点 (Remove the associated endpoint)
      clients_endpoints_.erase(endpointGuid);  // 删除指定的端点 (Remove the specified endpoint)
    }
  }

  /**
   * @brief 添加读者和写者端点 (Add reader and writer endpoints)
   *
   * @param[in] readerGuid 读者端点的 GUID (The GUID of the reader endpoint)
   * @param[in] writerGuid 写者端点的 GUID (The GUID of the writer endpoint)
   */
  void endpoint_add_reader_and_writer(
      const eprosima::fastrtps::rtps::GUID_t &readerGuid,
      const eprosima::fastrtps::rtps::GUID_t &writerGuid) {
    std::lock_guard<std::mutex> lock(mutex_);  // 对互斥锁进行上锁 (Lock the mutex)
    clients_endpoints_.emplace(readerGuid, writerGuid);  // 将读者和写者端点添加到映射中 (Add the
                                                         // reader and writer endpoints to the map)
    clients_endpoints_.emplace(writerGuid, readerGuid);  // 将写者和读者端点添加到映射中 (Add the
                                                         // writer and reader endpoints to the map)
  }

private:
  std::mutex
      mutex_;  // 互斥锁，用于保护以下数据结构 (Mutex for protecting the following data structures)
  subscriptions_set_t subscriptions_
      RCPPUTILS_TSA_GUARDED_BY(mutex_);  // 订阅集合 (Set of subscriptions)
  clients_endpoints_map_t clients_endpoints_
      RCPPUTILS_TSA_GUARDED_BY(mutex_);  // 客户端端点映射 (Map of client endpoints)
  std::condition_variable cv_;  // 条件变量，用于同步操作 (Condition variable for synchronization)
};

/**
 * @class ServiceListener
 * @brief 一个继承自 eprosima::fastdds::dds::DataReaderListener 的类，用于处理服务监听器的相关操作。
 *        A class that inherits from eprosima::fastdds::dds::DataReaderListener, used to handle
 * service listener related operations.
 */
class ServiceListener : public eprosima::fastdds::dds::DataReaderListener {
public:
  /**
   * @brief 构造函数，初始化 info_ 成员变量。
   *        Constructor, initializes the info_ member variable.
   * @param info 自定义服务信息指针。Pointer to custom service information.
   */
  explicit ServiceListener(CustomServiceInfo *info) : info_(info) {}

  /**
   * @brief 当订阅匹配时触发的回调函数。
   *        Callback function triggered when a subscription is matched.
   * @param reader DataReader 指针。Pointer to the DataReader.
   * @param info 订阅匹配状态信息。Subscription matched status information.
   */
  void on_subscription_matched(
      eprosima::fastdds::dds::DataReader *,
      const eprosima::fastdds::dds::SubscriptionMatchedStatus &info) final {
    // 如果当前计数变化为 -1，则从 pub_listener_ 中删除对应的端点。
    // If the current count change is -1, remove the corresponding endpoint from pub_listener_.
    if (info.current_count_change == -1) {
      info_->pub_listener_->endpoint_erase_if_exists(
          eprosima::fastrtps::rtps::iHandle2GUID(info.last_publication_handle));
    }
  }

  /**
   * @brief 获取未读请求的数量。
   *        Get the number of unread requests.
   * @return 未读请求的数量。Number of unread requests.
   */
  size_t get_unread_resquests() { return info_->request_reader_->get_unread_count(true); }

  /**
   * @brief 当有数据可用时触发的回调函数。
   *        Callback function triggered when data is available.
   * @param reader DataReader 指针。Pointer to the DataReader.
   */
  void on_data_available(eprosima::fastdds::dds::DataReader *) final {
    std::unique_lock<std::mutex> lock_mutex(on_new_request_m_);

    auto unread_requests = get_unread_resquests();

    // 如果有未读请求，则执行回调函数。
    // If there are unread requests, execute the callback function.
    if (0u < unread_requests) {
      on_new_request_cb_(user_data_, unread_requests);
    }
  }

  /**
   * @brief 设置新请求回调函数，当监听器收到新事件时执行相应操作。
   *        Set the new request callback function to perform corresponding operations when the
   * listener receives a new event.
   * @param user_data 用户数据指针。Pointer to user data.
   * @param callback 回调函数。Callback function.
   */
  void set_on_new_request_callback(const void *user_data, rmw_event_callback_t callback) {
    if (callback) {
      auto unread_requests = get_unread_resquests();

      std::lock_guard<std::mutex> lock_mutex(on_new_request_m_);

      // 如果有未读请求，则执行回调函数。
      // If there are unread requests, execute the callback function.
      if (0 < unread_requests) {
        callback(user_data, unread_requests);
      }

      user_data_ = user_data;
      on_new_request_cb_ = callback;

      // 设置监听器的状态掩码。
      // Set the status mask of the listener.
      eprosima::fastdds::dds::StatusMask status_mask = info_->request_reader_->get_status_mask();
      status_mask |= eprosima::fastdds::dds::StatusMask::data_available();
      info_->request_reader_->set_listener(this, status_mask);
    } else {
      std::lock_guard<std::mutex> lock_mutex(on_new_request_m_);

      // 取消监听器的状态掩码。
      // Unset the status mask of the listener.
      eprosima::fastdds::dds::StatusMask status_mask = info_->request_reader_->get_status_mask();
      status_mask &= ~eprosima::fastdds::dds::StatusMask::data_available();
      info_->request_reader_->set_listener(this, status_mask);

      user_data_ = nullptr;
      on_new_request_cb_ = nullptr;
    }
  }

private:
  CustomServiceInfo *info_;  ///< 自定义服务信息指针。Pointer to custom service information.

  rmw_event_callback_t on_new_request_cb_{
      nullptr};                     ///< 新请求回调函数。New request callback function.

  const void *user_data_{nullptr};  ///< 用户数据指针。Pointer to user data.

  std::mutex on_new_request_m_;     ///< 新请求互斥锁。Mutex for new requests.
};

#endif  // RMW_FASTRTPS_SHARED_CPP__CUSTOM_SERVICE_INFO_HPP_
