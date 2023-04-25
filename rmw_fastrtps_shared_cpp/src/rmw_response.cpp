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

#include <cassert>

#include "fastcdr/Cdr.h"
#include "fastdds/dds/core/StackAllocatedSequence.hpp"
#include "fastdds/rtps/common/WriteParams.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"
#include "rmw_fastrtps_shared_cpp/custom_client_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_service_info.hpp"
#include "rmw_fastrtps_shared_cpp/guid_utils.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 从服务端接收响应
 * @param[in] identifier RMW实现的标识符
 * @param[in] client 客户端对象指针
 * @param[out] request_header 请求头信息，包含时间戳和请求ID
 * @param[out] ros_response 存储反序列化后的ROS响应消息的指针
 * @param[out] taken 是否成功接收到有效的响应
 * @return rmw_ret_t 返回操作结果状态码
 *
 * @brief Receive response from the service server
 * @param[in] identifier Identifier of the RMW implementation
 * @param[in] client Pointer to the client object
 * @param[out] request_header Request header information, including timestamps and request ID
 * @param[out] ros_response Pointer to store the deserialized ROS response message
 * @param[out] taken Whether a valid response has been successfully received
 * @return rmw_ret_t Return operation result status code
 */
rmw_ret_t __rmw_take_response(
    const char *identifier,
    const rmw_client_t *client,
    rmw_service_info_t *request_header,
    void *ros_response,
    bool *taken) {
  // 检查输入参数是否为空
  // Check if input arguments are null
  RMW_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      client, client->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  RMW_CHECK_ARGUMENT_FOR_NULL(request_header, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(ros_response, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);

  *taken = false;

  // 获取客户端信息
  // Get client information
  auto info = static_cast<CustomClientInfo *>(client->data);
  assert(info);

  CustomClientResponse response;

  // 初始化FastBuffer并设置序列化数据类型
  // Initialize FastBuffer and set serialized data type
  response.buffer_.reset(new eprosima::fastcdr::FastBuffer());
  rmw_fastrtps_shared_cpp::SerializedData data;
  data.type = FASTRTPS_SERIALIZED_DATA_TYPE_CDR_BUFFER;
  data.data = response.buffer_.get();
  data.impl = nullptr;  // not used when type is FASTRTPS_SERIALIZED_DATA_TYPE_CDR_BUFFER

  eprosima::fastdds::dds::StackAllocatedSequence<void *, 1> data_values;
  const_cast<void **>(data_values.buffer())[0] = &data;
  eprosima::fastdds::dds::SampleInfoSeq info_seq{1};

  // 从响应读取器中获取响应数据
  // Get response data from the response reader
  if (ReturnCode_t::RETCODE_OK == info->response_reader_->take(data_values, info_seq, 1)) {
    if (info_seq[0].valid_data) {
      response.sample_identity_ = info_seq[0].related_sample_identity;

      // 检查响应是否来自正确的服务端
      // Check if the response is from the correct service server
      if (response.sample_identity_.writer_guid() == info->reader_guid_ ||
          response.sample_identity_.writer_guid() == info->writer_guid_) {
        auto raw_type_support = dynamic_cast<rmw_fastrtps_shared_cpp::TypeSupport *>(
            info->response_type_support_.get());
        eprosima::fastcdr::Cdr deser(
            *response.buffer_, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR);

        // 反序列化响应消息
        // Deserialize the response message
        if (raw_type_support->deserializeROSmessage(
                deser, ros_response, info->response_type_support_impl_)) {
          request_header->source_timestamp = info_seq[0].source_timestamp.to_ns();
          request_header->received_timestamp = info_seq[0].reception_timestamp.to_ns();
          request_header->request_id.sequence_number =
              ((int64_t)response.sample_identity_.sequence_number().high) << 32 |
              response.sample_identity_.sequence_number().low;

          *taken = true;
        }
      }
    }
  }

  return RMW_RET_OK;
}

/**
 * @brief 发送服务响应 (Send service response)
 *
 * @param[in] identifier 标识符，用于检查 RMW 实现是否匹配 (Identifier for checking if the RMW
 * implementation matches)
 * @param[in] service 服务对象，包含实现相关的信息 (Service object containing implementation related
 * information)
 * @param[in] request_header 请求头，包含请求的元数据 (Request header containing metadata of the
 * request)
 * @param[in] ros_response ROS 服务响应消息 (ROS service response message)
 * @return rmw_ret_t 返回操作结果 (Return the result of the operation)
 */
rmw_ret_t __rmw_send_response(
    const char *identifier,
    const rmw_service_t *service,
    rmw_request_id_t *request_header,
    void *ros_response) {
  // 检查 service 参数是否为空 (Check if the service parameter is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(service, RMW_RET_INVALID_ARGUMENT);
  // 检查类型标识符是否匹配 (Check if type identifiers match)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      service, service->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查 request_header 参数是否为空 (Check if the request_header parameter is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(request_header, RMW_RET_INVALID_ARGUMENT);
  // 检查 ros_response 参数是否为空 (Check if the ros_response parameter is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(ros_response, RMW_RET_INVALID_ARGUMENT);

  // 初始化返回值为错误 (Initialize the returned value as an error)
  rmw_ret_t returnedValue = RMW_RET_ERROR;

  // 获取服务信息 (Get the service information)
  auto info = static_cast<CustomServiceInfo *>(service->data);
  assert(info);

  // 初始化写参数 (Initialize write parameters)
  eprosima::fastrtps::rtps::WriteParams wparams;
  // 从字节数组复制到 FastRTPS GUID (Copy from byte array to FastRTPS GUID)
  rmw_fastrtps_shared_cpp::copy_from_byte_array_to_fastrtps_guid(
      request_header->writer_guid, &wparams.related_sample_identity().writer_guid());
  // 设置序列号的高位和低位 (Set high and low parts of the sequence number)
  wparams.related_sample_identity().sequence_number().high =
      (int32_t)((request_header->sequence_number & 0xFFFFFFFF00000000) >> 32);
  wparams.related_sample_identity().sequence_number().low =
      (int32_t)(request_header->sequence_number & 0xFFFFFFFF);

  // TODO(MiguelCompany) The following block is a workaround for the race on the
  // discovery of services. It is (ab)using a related_sample_identity on the request
  // with the GUID of the response reader, so we can wait here for it to be matched to
  // the server response writer. In the future, this should be done with the mechanism
  // explained on OMG DDS-RPC 1.0 spec under section 7.6.2 (Enhanced Service Mapping)

  // 根据 RTPS 第 9.3.1.2 节中可能的实体类型列表
  // 读取器将具有此位，而写入器将不会。我们使用这个来知道
  // 相关的 guid 是请求写入器还是响应读取器。
  // (According to the list of possible entity kinds in section 9.3.1.2 of RTPS,
  // readers will have this bit on, while writers will not. We use this to know
  // if the related guid is the request writer or the response reader.)
  constexpr uint8_t entity_id_is_reader_bit = 0x04;
  const eprosima::fastrtps::rtps::GUID_t &related_guid =
      wparams.related_sample_identity().writer_guid();
  if ((related_guid.entityId.value[3] & entity_id_is_reader_bit) != 0) {
    // 相关的 guid 是读取器，所以它是响应订阅 guid。
    // 等待响应写入器与其匹配。
    // (Related guid is a reader, so it is the response subscription guid.
    // Wait for the response writer to be matched with it.)
    auto listener = info->pub_listener_;
    client_present_t ret = listener->check_for_subscription(related_guid);
    if (ret == client_present_t::GONE) {
      return RMW_RET_OK;
    } else if (ret == client_present_t::MAYBE) {
      RMW_SET_ERROR_MSG("client will not receive response");
      return RMW_RET_TIMEOUT;
    }
  }

  // 初始化序列化数据 (Initialize serialized data)
  rmw_fastrtps_shared_cpp::SerializedData data;
  data.type = FASTRTPS_SERIALIZED_DATA_TYPE_ROS_MESSAGE;
  data.data = const_cast<void *>(ros_response);
  data.impl = info->response_type_support_impl_;
  // 写入响应数据 (Write the response data)
  if (info->response_writer_->write(&data, wparams)) {
    returnedValue = RMW_RET_OK;
  } else {
    RMW_SET_ERROR_MSG("cannot publish data");
  }

  // 返回操作结果 (Return the result of the operation)
  return returnedValue;
}

}  // namespace rmw_fastrtps_shared_cpp
