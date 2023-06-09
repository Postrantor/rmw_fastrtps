// Copyright 2017 Open Source Robotics Foundation, Inc.
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

#include "rmw_fastrtps_cpp/get_subscriber.hpp"

#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/custom_subscriber_info.hpp"

namespace rmw_fastrtps_cpp {

/**
 * @brief 获取与给定订阅关联的 eprosima::fastdds::dds::DataReader 对象
 *
 * @param[in] subscription 要获取其关联的 DataReader 的 rmw_subscription_t 对象指针
 * @return eprosima::fastdds::dds::DataReader* 成功时返回与订阅关联的 DataReader 指针，否则返回
 * nullptr
 */
eprosima::fastdds::dds::DataReader *get_datareader(rmw_subscription_t *subscription) {
  // 如果订阅指针为空，则直接返回 nullptr
  if (!subscription) {
    return nullptr;
  }

  // 检查订阅的实现标识符是否为 eprosima_fastrtps_identifier，如果不是，则返回 nullptr
  if (subscription->implementation_identifier != eprosima_fastrtps_identifier) {
    return nullptr;
  }

  // 将订阅的数据指针强制转换为 CustomSubscriberInfo 类型，并获取 impl 变量
  auto impl = static_cast<CustomSubscriberInfo *>(subscription->data);

  // 返回与订阅关联的 DataReader 对象指针
  return impl->data_reader_;
}

}  // namespace rmw_fastrtps_cpp
