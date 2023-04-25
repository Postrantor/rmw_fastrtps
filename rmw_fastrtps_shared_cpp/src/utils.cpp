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

#include "rmw_fastrtps_shared_cpp/utils.hpp"

#include <string>

#include "fastdds/dds/topic/Topic.hpp"
#include "fastdds/dds/topic/TopicDescription.hpp"
#include "fastdds/dds/topic/TypeSupport.hpp"
#include "fastrtps/types/TypesBase.h"
#include "rmw/rmw.h"

using ReturnCode_t = eprosima::fastrtps::types::ReturnCode_t;

const char *const CONTENT_FILTERED_TOPIC_POSTFIX = "_filtered_name";

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 将 DDS 错误代码转换为 RMW 错误代码 (Converts DDS error codes to RMW error codes)
 *
 * @param code DDS 错误代码 (DDS error code)
 * @return rmw_ret_t 对应的 RMW 错误代码 (Corresponding RMW error code)
 */
rmw_ret_t cast_error_dds_to_rmw(ReturnCode_t code) {
  // 不使用 switch，因为它不是枚举类 (Not using switch because it is not an enum class)
  if (ReturnCode_t::RETCODE_OK == code) {
    return RMW_RET_OK;
  } else if (ReturnCode_t::RETCODE_ERROR == code) {
    // 重复错误以避免过多的 'if' 比较 (Repeats the error to avoid too many 'if' comparisons)
    return RMW_RET_ERROR;
  } else if (ReturnCode_t::RETCODE_TIMEOUT == code) {
    return RMW_RET_TIMEOUT;
  } else if (ReturnCode_t::RETCODE_UNSUPPORTED == code) {
    return RMW_RET_UNSUPPORTED;
  } else if (ReturnCode_t::RETCODE_BAD_PARAMETER == code) {
    return RMW_RET_INVALID_ARGUMENT;
  } else if (ReturnCode_t::RETCODE_OUT_OF_RESOURCES == code) {
    // 可能资源不足来自于与错误分配不同的来源 (Could be that out of resources comes from a different
    // source than a bad allocation)
    return RMW_RET_BAD_ALLOC;
  } else {
    return RMW_RET_ERROR;
  }
}

/**
 * @brief 查找并检查主题和类型 (Find and check topic and type)
 *
 * @param participant_info 参与者信息 (Participant information)
 * @param topic_name 主题名称 (Topic name)
 * @param type_name 类型名称 (Type name)
 * @param[out] returned_topic 返回的主题描述 (Returned topic description)
 * @param[out] returned_type 返回的类型支持 (Returned type support)
 * @return bool 如果找到并检查成功，则返回 true，否则返回 false (Returns true if found and checked
 * successfully, otherwise returns false)
 */
bool find_and_check_topic_and_type(
    const CustomParticipantInfo *participant_info,
    const std::string &topic_name,
    const std::string &type_name,
    eprosima::fastdds::dds::TopicDescription **returned_topic,
    eprosima::fastdds::dds::TypeSupport *returned_type) {
  // 搜索已经存在的主题 (Searches for an already existing topic)
  *returned_topic = participant_info->participant_->lookup_topicdescription(topic_name);
  if (nullptr != *returned_topic) {
    if ((*returned_topic)->get_type_name() != type_name) {
      return false;
    }
  }

  // 注意(methylDragon): 这只会找到之前注册到参与者的类型 (NOTE(methylDragon): This only finds a
  // type that's been previously registered to the participant)
  *returned_type = participant_info->participant_->find_type(type_name);
  return true;
}

/**
 * @brief 删除主题和类型 (Remove topic and type)
 *
 * @param[in] participant_info 自定义参与者信息 (Custom participant information)
 * @param[in] event_listener 事件监听器接口 (Event listener interface)
 * @param[in] topic_desc 主题描述 (Topic description)
 * @param[in] type 类型支持 (Type support)
 */
void remove_topic_and_type(
    CustomParticipantInfo *participant_info,
    EventListenerInterface *event_listener,
    const eprosima::fastdds::dds::TopicDescription *topic_desc,
    const eprosima::fastdds::dds::TypeSupport &type) {
  // TODO(MiguelCompany): 我们目前只创建 Topic 实例，但如果我们开始支持其他类型的
  // TopicDescription（如 ContentFilteredTopic），这可能会在将来发生变化。 (We only create Topic
  // instances at the moment, but this may change in the future if we start supporting other kinds
  // of TopicDescription like ContentFilteredTopic)
  auto topic = dynamic_cast<const eprosima::fastdds::dds::Topic *>(topic_desc);

  // 如果 topic 不为空，则删除该主题 (If topic is not null, delete the topic)
  if (nullptr != topic) {
    participant_info->delete_topic(topic, event_listener);
  }

  // 如果类型有效，则注销类型 (If type is valid, unregister the type)
  if (type) {
    participant_info->participant_->unregister_type(type.get_type_name());
  }
}

/**
 * @brief 创建内容过滤主题 (Create a content filtered topic)
 *
 * @param[in] participant Fast DDS 域参与者指针 (Pointer to the Fast DDS DomainParticipant)
 * @param[in] topic_desc 主题描述指针 (Pointer to the TopicDescription)
 * @param[in] topic_name_mangled 修改后的主题名称 (Mangled topic name)
 * @param[in] options 订阅内容过滤选项 (Subscription content filter options)
 * @param[out] content_filtered_topic 内容过滤主题指针的引用 (Reference to the pointer of the
 * ContentFilteredTopic)
 * @return 成功创建返回 true，否则返回 false (Returns true if successfully created, otherwise
 * returns false)
 */
bool create_content_filtered_topic(
    eprosima::fastdds::dds::DomainParticipant *participant,
    eprosima::fastdds::dds::TopicDescription *topic_desc,
    const std::string &topic_name_mangled,
    const rmw_subscription_content_filter_options_t *options,
    eprosima::fastdds::dds::ContentFilteredTopic **content_filtered_topic) {
  // 初始化表达式参数向量 (Initialize the expression parameters vector)
  std::vector<std::string> expression_parameters;
  // 遍历选项中的表达式参数并添加到向量中 (Iterate through the expression parameters in the options
  // and add them to the vector)
  for (size_t i = 0; i < options->expression_parameters.size; ++i) {
    expression_parameters.push_back(options->expression_parameters.data[i]);
  }

  // 将主题描述动态转换为主题指针 (Dynamically cast the TopicDescription to a Topic pointer)
  auto topic = dynamic_cast<eprosima::fastdds::dds::Topic *>(topic_desc);
  // 为内容过滤主题创建一个名称 (Create a name for the content filtered topic)
  std::string cft_topic_name = topic_name_mangled + CONTENT_FILTERED_TOPIC_POSTFIX;
  // 使用给定参数创建内容过滤主题 (Create the content filtered topic with the given parameters)
  eprosima::fastdds::dds::ContentFilteredTopic *filtered_topic =
      participant->create_contentfilteredtopic(
          cft_topic_name, topic, options->filter_expression, expression_parameters);
  // 检查内容过滤主题是否创建成功 (Check if the content filtered topic was created successfully)
  if (filtered_topic == nullptr) {
    return false;
  }

  // 将创建的内容过滤主题指针赋值给输出参数 (Assign the created content filtered topic pointer to
  // the output parameter)
  *content_filtered_topic = filtered_topic;
  // 返回成功创建标志 (Return the successful creation flag)
  return true;
}

/**
 * @brief 创建数据读取器 (Create a DataReader)
 *
 * @param[in] datareader_qos 数据读取器的 QoS 配置 (QoS configuration for the DataReader)
 * @param[in] subscription_options 订阅选项 (Subscription options)
 * @param[in] subscriber 订阅者实例 (Subscriber instance)
 * @param[in] des_topic 主题描述 (Topic description)
 * @param[in] listener 自定义数据读取器监听器 (Custom DataReader listener)
 * @param[out] data_reader 输出创建的数据读取器指针 (Output pointer to the created DataReader)
 * @return 是否成功创建数据读取器 (Whether the DataReader was successfully created)
 */
bool create_datareader(
    const eprosima::fastdds::dds::DataReaderQos &datareader_qos,
    const rmw_subscription_options_t *subscription_options,
    eprosima::fastdds::dds::Subscriber *subscriber,
    eprosima::fastdds::dds::TopicDescription *des_topic,
    CustomDataReaderListener *listener,
    eprosima::fastdds::dds::DataReader **data_reader) {
  // 更新 QoS 配置 (Update QoS configuration)
  eprosima::fastdds::dds::DataReaderQos updated_qos = datareader_qos;

  // 根据订阅选项设置唯一网络流端点要求 (Set unique network flow endpoint requirements based on
  // subscription options)
  switch (subscription_options->require_unique_network_flow_endpoints) {
    default:
    case RMW_UNIQUE_NETWORK_FLOW_ENDPOINTS_SYSTEM_DEFAULT:
    case RMW_UNIQUE_NETWORK_FLOW_ENDPOINTS_NOT_REQUIRED:
      // 不需要唯一的网络流端点，我们将决策留给 XML 配置文件 (Unique network flow endpoints not
      // required. We leave the decision to the XML profile)
      break;

    case RMW_UNIQUE_NETWORK_FLOW_ENDPOINTS_OPTIONALLY_REQUIRED:
    case RMW_UNIQUE_NETWORK_FLOW_ENDPOINTS_STRICTLY_REQUIRED:
      // 确保我们请求唯一的网络流端点 (Ensure we request unique network flow endpoints)
      using PropertyPolicyHelper = eprosima::fastrtps::rtps::PropertyPolicyHelper;
      if (nullptr == PropertyPolicyHelper::find_property(
                         updated_qos.properties(), "fastdds.unique_network_flows")) {
        updated_qos.properties().properties().emplace_back("fastdds.unique_network_flows", "");
      }
      break;
  }

  // 创建数据读取器（使用订阅者名称以不更改名称策略）(Creates DataReader (with subscriber name to
  // not change name policy))
  *data_reader = subscriber->create_datareader(
      des_topic, updated_qos, listener, eprosima::fastdds::dds::StatusMask::subscription_matched());

  // 如果创建失败且需要可选的唯一网络流端点，则尝试使用原始 QoS 配置创建数据读取器 (If creation
  // fails and unique network flow endpoints are optionally required, try creating the DataReader
  // with the original QoS configuration)
  if (!data_reader && (RMW_UNIQUE_NETWORK_FLOW_ENDPOINTS_OPTIONALLY_REQUIRED ==
                       subscription_options->require_unique_network_flow_endpoints)) {
    *data_reader = subscriber->create_datareader(
        des_topic, datareader_qos, listener,
        eprosima::fastdds::dds::StatusMask::subscription_matched());
  }
  return true;
}

}  // namespace rmw_fastrtps_shared_cpp
