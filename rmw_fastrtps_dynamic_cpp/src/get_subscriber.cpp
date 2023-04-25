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

#include "rmw_fastrtps_dynamic_cpp/get_subscriber.hpp"

#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/custom_subscriber_info.hpp"

namespace rmw_fastrtps_dynamic_cpp {

/**
 * @brief 获取数据读取器 (Get the DataReader)
 *
 * @param[in] subscription RMW订阅指针 (Pointer to RMW subscription)
 * @return eprosima::fastdds::dds::DataReader* 数据读取器指针，如果订阅无效则返回nullptr (Pointer to
 * DataReader, nullptr if the subscription is invalid)
 */
eprosima::fastdds::dds::DataReader *get_datareader(rmw_subscription_t *subscription) {
  // 如果订阅为空，则返回nullptr (If the subscription is null, return nullptr)
  if (!subscription) {
    return nullptr;
  }
  // 如果实现标识符与 eprosima_fastrtps_identifier 不匹配，则返回nullptr (Return nullptr if the
  // implementation identifier does not match eprosima_fastrtps_identifier)
  if (subscription->implementation_identifier != eprosima_fastrtps_identifier) {
    return nullptr;
  }
  // 将订阅的数据指针转换为 CustomSubscriberInfo 类型 (Cast the data pointer of the subscription to
  // CustomSubscriberInfo type)
  auto impl = static_cast<CustomSubscriberInfo *>(subscription->data);
  // 返回数据读取器指针 (Return the pointer to the DataReader)
  return impl->data_reader_;
}

}  // namespace rmw_fastrtps_dynamic_cpp
