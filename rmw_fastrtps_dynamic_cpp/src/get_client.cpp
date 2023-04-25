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

#include "rmw_fastrtps_dynamic_cpp/get_client.hpp"

#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/custom_client_info.hpp"

namespace rmw_fastrtps_dynamic_cpp {

/**
 * @brief 获取请求的 DataWriter 对象 (Get the request DataWriter object)
 *
 * @param[in] client 指向 rmw_client_t 结构体的指针 (Pointer to an rmw_client_t structure)
 * @return eprosima::fastdds::dds::DataWriter* 请求的 DataWriter 对象指针，如果失败则返回 nullptr
 * (Pointer to the request DataWriter object, or nullptr if failed)
 */
eprosima::fastdds::dds::DataWriter *get_request_datawriter(rmw_client_t *client) {
  // 检查 client 是否为 nullptr (Check if client is nullptr)
  if (!client) {
    return nullptr;
  }
  // 检查实现标识符是否与 eprosima_fastrtps_identifier 匹配 (Check if the implementation identifier
  // matches eprosima_fastrtps_identifier)
  if (client->implementation_identifier != eprosima_fastrtps_identifier) {
    return nullptr;
  }
  // 将 client->data 静态转换为 CustomClientInfo 类型，并获取 impl (Static cast client->data to
  // CustomClientInfo type and get impl)
  auto impl = static_cast<CustomClientInfo *>(client->data);
  // 返回请求的 DataWriter 对象 (Return the request DataWriter object)
  return impl->request_writer_;
}

/**
 * @brief 获取响应的 DataReader 对象 (Get the response DataReader object)
 *
 * @param[in] client 指向 rmw_client_t 结构体的指针 (Pointer to an rmw_client_t structure)
 * @return eprosima::fastdds::dds::DataReader* 响应的 DataReader 对象指针，如果失败则返回 nullptr
 * (Pointer to the response DataReader object, or nullptr if failed)
 */
eprosima::fastdds::dds::DataReader *get_response_datareader(rmw_client_t *client) {
  // 检查 client 是否为 nullptr (Check if client is nullptr)
  if (!client) {
    return nullptr;
  }
  // 检查实现标识符是否与 eprosima_fastrtps_identifier 匹配 (Check if the implementation identifier
  // matches eprosima_fastrtps_identifier)
  if (client->implementation_identifier != eprosima_fastrtps_identifier) {
    return nullptr;
  }
  // 将 client->data 静态转换为 CustomClientInfo 类型，并获取 impl (Static cast client->data to
  // CustomClientInfo type and get impl)
  auto impl = static_cast<CustomClientInfo *>(client->data);
  // 返回响应的 DataReader 对象 (Return the response DataReader object)
  return impl->response_reader_;
}

}  // namespace rmw_fastrtps_dynamic_cpp
