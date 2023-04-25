// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef RMW_FASTRTPS_SHARED_CPP__UTILS_HPP_
#define RMW_FASTRTPS_SHARED_CPP__UTILS_HPP_

#include <mutex>
#include <string>

#include "fastdds/dds/topic/TopicDescription.hpp"
#include "fastdds/dds/topic/TypeSupport.hpp"
#include "fastrtps/types/TypesBase.h"
#include "rmw/rmw.h"
#include "rmw_fastrtps_shared_cpp/TypeSupport.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_subscriber_info.hpp"

namespace rmw_fastrtps_shared_cpp {

/**
 * 将 Fast DDS 返回码转换为相应的 rmw_ret_t
 * Convert a Fast DDS return code into the corresponding rmw_ret_t
 * \param[in] code 要转换的 Fast DDS 返回码
 *               The Fast DDS return code to be translated
 * \return 对应的 rmw_ret_t 值
 *         the corresponding rmw_ret_t value
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
rmw_ret_t cast_error_dds_to_rmw(eprosima::fastrtps::types::ReturnCode_t code);

/**
 * 尝试查找已注册的主题和类型。
 * Tries to find already registered topic and type.
 *
 * \param[in]  participant_info 与上下文关联的 CustomParticipantInfo。
 *                              CustomParticipantInfo associated to the context.
 * \param[in]  topic_name       正在创建的实体的主题名称。
 *                              Name of the topic for the entity being created.
 * \param[in]  type_name        正在创建的实体的类型名称。
 *                              Name of the type for the entity being created.
 * \param[out] returned_topic   topic_name 的 TopicDescription
 *                              TopicDescription for topic_name
 * \param[out] returned_type    type_name 的 TypeSupport
 *                              TypeSupport for type_name
 *
 * \return 如果 `topic_name` 之前使用不同的类型名称创建，则返回 false。
 *         false if `topic_name` was previously created with a different type name.
 * \return 当没有此类冲突时返回 true。如果参与者以前未注册返回的主题和类型，它们可能为空
 *         true when there is no such conflict. Returned topic and type may be null
 *              if they were not previously registered on the participant.
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
bool find_and_check_topic_and_type(
    const CustomParticipantInfo* participant_info,
    const std::string& topic_name,
    const std::string& type_name,
    eprosima::fastdds::dds::TopicDescription** returned_topic,
    eprosima::fastdds::dds::TypeSupport* returned_type);

/**
 * 执行关联主题和类型的删除。
 * Performs removal of associated topic and type.
 *
 * \param[in] participant_info 与上下文关联的 CustomParticipantInfo。
 *                              CustomParticipantInfo associated to the context.
 * \param[in] event_listener   与主题关联的 EventListenerInterface。
 *                              The EventListenerInterface associated with the topic.
 * \param[in] topic            正在删除的实体的主题。
 *                              Topic of the entity being deleted.
 * \param[in] type             正在删除的实体的 TypeSupport。
 *                              TypeSupport of the entity being deleted.
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
void remove_topic_and_type(
    CustomParticipantInfo* participant_info,
    EventListenerInterface* event_listener,
    const eprosima::fastdds::dds::TopicDescription* topic,
    const eprosima::fastdds::dds::TypeSupport& type);

/**
 * 创建内容过滤主题。
 * Create content filtered topic.
 *
 * \param[in]  participant             将在其中创建主题的域参与者。DomainParticipant where the topic
 * will be created. \param[in]  topic_desc              由 find_and_check_topic_and_type 返回的
 * TopicDescription。TopicDescription returned by find_and_check_topic_and_type. \param[in]
 * topic_name_mangled      主题的混淆名称。Mangled Name of the topic. \param[in]  options
 * 内容过滤主题的选项。Options of the content filtered topic. \param[out] content_filtered_topic
 * 将保存指向内容过滤主题的指针以及删除所需的必要信息。Will hold the pointer to the content filtered
 * topic along with the necessary information for its deletion.
 *
 * \return 当内容过滤主题被创建时返回 true。true when the content filtered topic was created
 * \return 当内容过滤主题无法创建时返回 false。false when the content filtered topic could not be
 * created
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
bool create_content_filtered_topic(
    eprosima::fastdds::dds::DomainParticipant* participant,
    eprosima::fastdds::dds::TopicDescription* topic_desc,
    const std::string& topic_name_mangled,
    const rmw_subscription_content_filter_options_t* options,
    eprosima::fastdds::dds::ContentFilteredTopic** content_filtered_topic);

/**
 * 创建数据阅读器。
 * Create data reader.
 *
 * \param[in]  datareader_qos         数据阅读器的 QoS。QoS of data reader.
 * \param[in]  subscription_options   订阅的选项。Options of the subscription.
 * \param[in]  subscriber             创建数据阅读器的订阅者。A subsciber to create the data reader.
 * \param[in]  des_topic              由 find_and_check_topic_and_type 返回的
 * TopicDescription。TopicDescription returned by find_and_check_topic_and_type. \param[in] listener
 * 数据阅读器的监听器。The listener of the data reader. \param[out] data_reader
 * 将保存指向数据阅读器的指针。Will hold the pointer to the data reader.
 *
 * \return 当数据阅读器被创建时返回 true。true when the data reader was created
 * \return 当数据阅读器无法创建时返回 false。false when the data reader could not be created
 */
RMW_FASTRTPS_SHARED_CPP_PUBLIC
bool create_datareader(
    const eprosima::fastdds::dds::DataReaderQos& datareader_qos,
    const rmw_subscription_options_t* subscription_options,
    eprosima::fastdds::dds::Subscriber* subscriber,
    eprosima::fastdds::dds::TopicDescription* des_topic,
    CustomDataReaderListener* listener,
    eprosima::fastdds::dds::DataReader** data_reader);

}  // namespace rmw_fastrtps_shared_cpp

#endif  // RMW_FASTRTPS_SHARED_CPP__UTILS_HPP_
