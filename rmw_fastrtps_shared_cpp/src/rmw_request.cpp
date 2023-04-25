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
#include "fastcdr/FastBuffer.h"
#include "fastdds/dds/core/StackAllocatedSequence.hpp"
#include "fastdds/rtps/common/WriteParams.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw/types.h"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"
#include "rmw_fastrtps_shared_cpp/custom_client_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_service_info.hpp"
#include "rmw_fastrtps_shared_cpp/guid_utils.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 发送请求到服务端 (Send a request to the server)
 *
 * @param[in] identifier RMW 实现的标识符 (Identifier of the RMW implementation)
 * @param[in] client 客户端指针，用于与服务端通信 (Pointer to the client used for communication with
 * the server)
 * @param[in] ros_request ROS 请求消息 (The ROS request message)
 * @param[out] sequence_id 请求的序列号 (Sequence number of the request)
 * @return rmw_ret_t 返回操作结果 (Return the operation result)
 */
rmw_ret_t __rmw_send_request(
    const char *identifier,
    const rmw_client_t *client,
    const void *ros_request,
    int64_t *sequence_id) {
  // 检查 client 是否为空 (Check if the client is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(client, RMW_RET_INVALID_ARGUMENT);
  // 检查 client 的实现标识符是否与传入的标识符匹配 (Check if the client's implementation identifier
  // matches the given identifier)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      client, client->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  // 检查 ros_request 是否为空 (Check if the ros_request is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(ros_request, RMW_RET_INVALID_ARGUMENT);
  // 检查 sequence_id 是否为空 (Check if the sequence_id is NULL)
  RMW_CHECK_ARGUMENT_FOR_NULL(sequence_id, RMW_RET_INVALID_ARGUMENT);

  // 初始化返回值为错误 (Initialize the returned value as an error)
  rmw_ret_t returnedValue = RMW_RET_ERROR;

  // 获取客户端信息 (Get the client information)
  auto info = static_cast<CustomClientInfo *>(client->data);
  assert(info);

  // 初始化写参数 (Initialize write parameters)
  eprosima::fastrtps::rtps::WriteParams wparams;
  // 初始化序列化数据 (Initialize serialized data)
  rmw_fastrtps_shared_cpp::SerializedData data;
  data.type = FASTRTPS_SERIALIZED_DATA_TYPE_ROS_MESSAGE;
  data.data = const_cast<void *>(ros_request);
  data.impl = info->request_type_support_impl_;
  // 设置相关的样本标识符 (Set the related sample identity)
  wparams.related_sample_identity().writer_guid() = info->reader_guid_;
  // 尝试将请求写入 Fast RTPS (Try to write the request into Fast RTPS)
  if (info->request_writer_->write(&data, wparams)) {
    // 如果成功，设置返回值为 RMW_RET_OK 并计算序列号 (If successful, set the returned value to
    // RMW_RET_OK and calculate the sequence number)
    returnedValue = RMW_RET_OK;
    *sequence_id = ((int64_t)wparams.sample_identity().sequence_number().high) << 32 |
                   wparams.sample_identity().sequence_number().low;
  } else {
    // 如果失败，设置错误消息 (If failed, set the error message)
    RMW_SET_ERROR_MSG("cannot publish data");
  }

  // 返回操作结果 (Return the operation result)
  return returnedValue;
}

/**
 * @brief 从服务端接收请求并将其反序列化为 ROS 消息。
 *        Receive a request from the service server and deserialize it into a ROS message.
 *
 * @param[in] identifier RMW 实现的标识符。Identifier of the RMW implementation.
 * @param[in] service 服务对象指针。Pointer to the service object.
 * @param[out] request_header 请求头信息。Request header information.
 * @param[out] ros_request 反序列化后的 ROS 请求消息。Deserialized ROS request message.
 * @param[out] taken 是否成功接收到请求。Whether the request was successfully received.
 * @return rmw_ret_t RMW 返回值。RMW return value.
 */
rmw_ret_t __rmw_take_request(
    const char *identifier,
    const rmw_service_t *service,
    rmw_service_info_t *request_header,
    void *ros_request,
    bool *taken) {
  // 检查输入参数是否为空。Check input arguments for null.
  RMW_CHECK_ARGUMENT_FOR_NULL(service, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      service, service->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);
  RMW_CHECK_ARGUMENT_FOR_NULL(request_header, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(ros_request, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(taken, RMW_RET_INVALID_ARGUMENT);

  // 初始化 taken 为 false。Initialize taken to false.
  *taken = false;

  // 获取服务的自定义信息。Get the custom service info.
  auto info = static_cast<CustomServiceInfo *>(service->data);
  assert(info);

  // 创建一个自定义服务请求。Create a custom service request.
  CustomServiceRequest request;

  // 分配 FastBuffer 内存。Allocate memory for FastBuffer.
  request.buffer_ = new eprosima::fastcdr::FastBuffer();

  if (request.buffer_ != nullptr) {
    // 初始化序列化数据对象。Initialize the serialized data object.
    rmw_fastrtps_shared_cpp::SerializedData data;
    data.type = FASTRTPS_SERIALIZED_DATA_TYPE_CDR_BUFFER;
    data.data = request.buffer_;
    data.impl = nullptr;  // not used when type is FASTRTPS_SERIALIZED_DATA_TYPE_CDR_BUFFER

    // 创建数据值和样本信息序列。Create data values and sample info sequences.
    eprosima::fastdds::dds::StackAllocatedSequence<void *, 1> data_values;
    const_cast<void **>(data_values.buffer())[0] = &data;
    eprosima::fastdds::dds::SampleInfoSeq info_seq{1};

    // 尝试从请求阅读器中获取数据。Try to take data from the request reader.
    if (ReturnCode_t::RETCODE_OK == info->request_reader_->take(data_values, info_seq, 1)) {
      if (info_seq[0].valid_data) {
        // 设置请求的样本标识。Set the request's sample identity.
        request.sample_identity_ = info_seq[0].sample_identity;
        // 使用响应订阅者 GUID（在 related_sample_identity 上）（如果存在）。Use response subscriber
        // guid (on related_sample_identity) when present.
        const eprosima::fastrtps::rtps::GUID_t &reader_guid =
            info_seq[0].related_sample_identity.writer_guid();
        if (reader_guid != eprosima::fastrtps::rtps::GUID_t::unknown()) {
          request.sample_identity_.writer_guid() = reader_guid;
        }

        // 在 clients_endpoints 映射中保存两个 GUID。Save both guids in the clients_endpoints map.
        const eprosima::fastrtps::rtps::GUID_t &writer_guid =
            info_seq[0].sample_identity.writer_guid();
        info->pub_listener_->endpoint_add_reader_and_writer(reader_guid, writer_guid);

        // 获取原始类型支持。Get raw type support.
        auto raw_type_support = dynamic_cast<rmw_fastrtps_shared_cpp::TypeSupport *>(
            info->response_type_support_.get());
        // 创建反序列化器。Create deserializer.
        eprosima::fastcdr::Cdr deser(
            *request.buffer_, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN,
            eprosima::fastcdr::Cdr::DDS_CDR);
        // 反序列化 ROS 消息。Deserialize the ROS message.
        if (raw_type_support->deserializeROSmessage(
                deser, ros_request, info->request_type_support_impl_)) {
          // 获取请求头信息。Get the request header information.
          rmw_fastrtps_shared_cpp::copy_from_fastrtps_guid_to_byte_array(
              request.sample_identity_.writer_guid(), request_header->request_id.writer_guid);
          request_header->request_id.sequence_number =
              ((int64_t)request.sample_identity_.sequence_number().high) << 32 |
              request.sample_identity_.sequence_number().low;
          request_header->source_timestamp = info_seq[0].source_timestamp.to_ns();
          request_header->received_timestamp = info_seq[0].source_timestamp.to_ns();
          // 设置 taken 为 true。Set taken to true.
          *taken = true;
        }
      }
    }

    // 删除 FastBuffer。Delete the FastBuffer.
    delete request.buffer_;
  }

  return RMW_RET_OK;
}

}  // namespace rmw_fastrtps_shared_cpp
