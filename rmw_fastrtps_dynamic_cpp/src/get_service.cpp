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

#include "rmw_fastrtps_dynamic_cpp/get_service.hpp"

#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_shared_cpp/custom_service_info.hpp"

namespace rmw_fastrtps_dynamic_cpp {

/**
 * @brief 获取请求的 DataReader 对象 (Get the request DataReader object)
 *
 * @param[in] service 服务对象指针 (Pointer to the service object)
 * @return eprosima::fastdds::dds::DataReader* 请求的 DataReader 对象指针，如果失败则返回 nullptr
 * (Pointer to the request DataReader object, nullptr if failed)
 */
eprosima::fastdds::dds::DataReader *get_request_datareader(rmw_service_t *service) {
  // 检查 service 是否为空指针 (Check if service is a nullptr)
  if (!service) {
    return nullptr;
  }
  // 检查实现标识符是否为 eprosima_fastrtps_identifier (Check if the implementation identifier is
  // eprosima_fastrtps_identifier)
  if (service->implementation_identifier != eprosima_fastrtps_identifier) {
    return nullptr;
  }
  // 将 service 的 data 成员转换为 CustomServiceInfo 类型 (Cast the data member of service to
  // CustomServiceInfo type)
  auto impl = static_cast<CustomServiceInfo *>(service->data);
  // 返回请求的 DataReader 对象 (Return the request DataReader object)
  return impl->request_reader_;
}

/**
 * @brief 获取响应的 DataWriter 对象 (Get the response DataWriter object)
 *
 * @param[in] service 服务对象指针 (Pointer to the service object)
 * @return eprosima::fastdds::dds::DataWriter* 响应的 DataWriter 对象指针，如果失败则返回 nullptr
 * (Pointer to the response DataWriter object, nullptr if failed)
 */
eprosima::fastdds::dds::DataWriter *get_response_datawriter(rmw_service_t *service) {
  // 检查 service 是否为空指针 (Check if service is a nullptr)
  if (!service) {
    return nullptr;
  }
  // 检查实现标识符是否为 eprosima_fastrtps_identifier (Check if the implementation identifier is
  // eprosima_fastrtps_identifier)
  if (service->implementation_identifier != eprosima_fastrtps_identifier) {
    return nullptr;
  }
  // 将 service 的 data 成员转换为 CustomServiceInfo 类型 (Cast the data member of service to
  // CustomServiceInfo type)
  auto impl = static_cast<CustomServiceInfo *>(service->data);
  // 返回响应的 DataWriter 对象 (Return the response DataWriter object)
  return impl->response_writer_;
}

}  // namespace rmw_fastrtps_dynamic_cpp
