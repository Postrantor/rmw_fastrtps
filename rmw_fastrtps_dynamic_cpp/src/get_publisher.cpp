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

#include "rmw_fastrtps_dynamic_cpp/get_publisher.hpp"

#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/custom_publisher_info.hpp"

namespace rmw_fastrtps_dynamic_cpp {

/**
 * @brief 获取 DataWriter 对象 (Get the DataWriter object)
 *
 * @param[in] publisher rmw_publisher_t 类型的指针，用于获取 DataWriter 对象 (Pointer to an
 * rmw_publisher_t, used to get the DataWriter object)
 * @return eprosima::fastdds::dds::DataWriter* 返回 DataWriter 对象的指针，如果出现错误则返回
 * nullptr (Returns a pointer to the DataWriter object, or nullptr if an error occurs)
 */
eprosima::fastdds::dds::DataWriter *get_datawriter(rmw_publisher_t *publisher) {
  // 检查 publisher 是否为空指针 (Check if the publisher is a nullptr)
  if (!publisher) {
    return nullptr;  // 如果为空指针，则返回 nullptr (If it's a nullptr, return nullptr)
  }

  // 检查 publisher 的实现标识符是否为 eprosima_fastrtps_identifier (Check if the publisher's
  // implementation_identifier is eprosima_fastrtps_identifier)
  if (publisher->implementation_identifier != eprosima_fastrtps_identifier) {
    return nullptr;  // 如果不是 eprosima_fastrtps_identifier，则返回 nullptr (If it's not
                     // eprosima_fastrtps_identifier, return nullptr)
  }

  // 将 publisher->data 转换为 CustomPublisherInfo 类型的指针 (Cast publisher->data to a pointer of
  // type CustomPublisherInfo)
  auto impl = static_cast<CustomPublisherInfo *>(publisher->data);

  // 返回 impl 中的 data_writer_ 成员 (Return the data_writer_ member in impl)
  return impl->data_writer_;
}

}  // namespace rmw_fastrtps_dynamic_cpp
