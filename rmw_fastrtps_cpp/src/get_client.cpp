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

#include "rmw_fastrtps_cpp/get_client.hpp"

#include "fastdds/dds/publisher/DataWriter.hpp"
#include "fastdds/dds/subscriber/DataReader.hpp"
#include "rmw_fastrtps_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/custom_client_info.hpp"

namespace rmw_fastrtps_cpp {

/**
 * @brief 获取请求的 DataWriter 对象 (Get the request DataWriter object)
 *
 * @param[in] client 指向 rmw_client_t 结构体的指针，包含了 ROS2 服务客户端的信息 (Pointer to the
 * rmw_client_t structure, containing information about the ROS2 service client)
 * @return 返回一个指向 eprosima::fastdds::dds::DataWriter 的指针，如果失败则返回 nullptr (Returns a
 * pointer to eprosima::fastdds::dds::DataWriter, or nullptr if it fails)
 */
eprosima::fastdds::dds::DataWriter *get_request_datawriter(rmw_client_t *client) {
  // 如果 client 为 nullptr，则返回 nullptr (If client is nullptr, return nullptr)
  if (!client) {
    return nullptr;
  }
  // 如果实现标识符与 eprosima_fastrtps_identifier 不匹配，则返回 nullptr (If the implementation
  // identifier does not match eprosima_fastrtps_identifier, return nullptr)
  if (client->implementation_identifier != eprosima_fastrtps_identifier) {
    return nullptr;
  }
  // 将 client->data 强制转换为 CustomClientInfo 类型，并获取 impl (Force cast client->data to
  // CustomClientInfo type and get impl)
  auto impl = static_cast<CustomClientInfo *>(client->data);
  // 返回 impl 中的 request_writer_ (Return request_writer_ in impl)
  return impl->request_writer_;
}

/**
 * @brief 获取响应的 DataReader 对象 (Get the response DataReader object)
 *
 * @param[in] client 指向 rmw_client_t 结构体的指针，包含了 ROS2 服务客户端的信息 (Pointer to the
 * rmw_client_t structure, containing information about the ROS2 service client)
 * @return 返回一个指向 eprosima::fastdds::dds::DataReader 的指针，如果失败则返回 nullptr (Returns a
 * pointer to eprosima::fastdds::dds::DataReader, or nullptr if it fails)
 */
eprosima::fastdds::dds::DataReader *get_response_datareader(rmw_client_t *client) {
  // 如果 client 为 nullptr，则返回 nullptr (If client is nullptr, return nullptr)
  if (!client) {
    return nullptr;
  }
  // 如果实现标识符与 eprosima_fastrtps_identifier 不匹配，则返回 nullptr (If the implementation
  // identifier does not match eprosima_fastrtps_identifier, return nullptr)
  if (client->implementation_identifier != eprosima_fastrtps_identifier) {
    return nullptr;
  }
  // 将 client->data 强制转换为 CustomClientInfo 类型，并获取 impl (Force cast client->data to
  // CustomClientInfo type and get impl)
  auto impl = static_cast<CustomClientInfo *>(client->data);
  // 返回 impl 中的 response_reader_ (Return response_reader_ in impl)
  return impl->response_reader_;
}

}  // namespace rmw_fastrtps_cpp
