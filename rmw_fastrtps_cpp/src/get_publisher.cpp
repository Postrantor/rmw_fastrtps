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

#include "rmw_fastrtps_cpp/get_publisher.hpp"

#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/custom_publisher_info.hpp"

namespace rmw_fastrtps_cpp {

/*
  这几个 get 的文件的里面的逻辑基本一致。
  1 ~ 4 步骤，定义有 Custom* 结构，通过强制类型转换可以得到对应的对象
*/

/**
 * @brief 获取数据写入器 (Get the data writer)
 *
 * @param[in] publisher RMW发布者指针 (Pointer to the RMW publisher)
 * @return eprosima::fastdds::dds::DataWriter* 数据写入器指针，如果失败则返回nullptr (Pointer to the
 * data writer, nullptr if failed)
 */
eprosima::fastdds::dds::DataWriter *get_datawriter(rmw_publisher_t *publisher) {
  // 1. 检查传入的发布者是否为空 (Check if the input publisher is nullptr)
  if (!publisher) {
    return nullptr;  // 如果发布者为空，则返回nullptr (Return nullptr if the publisher is nullptr)
  }

  // 2. 检查发布者的实现标识符是否为 eprosima_fastrtps_identifier (Check if the publisher's
  // implementation_identifier is eprosima_fastrtps_identifier)
  if (publisher->implementation_identifier != eprosima_fastrtps_identifier) {
    return nullptr;  // 如果实现标识符不匹配，则返回nullptr (Return nullptr if the
                     // implementation_identifier does not match)
  }

  // 3. 将发布者的数据转换为 CustomPublisherInfo 类型 (Cast the publisher's data to
  // CustomPublisherInfo type)
  auto impl = static_cast<CustomPublisherInfo *>(publisher->data);

  // 4. 返回数据写入器指针 (Return the data writer pointer)
  return impl->data_writer_;
}

}  // namespace rmw_fastrtps_cpp
