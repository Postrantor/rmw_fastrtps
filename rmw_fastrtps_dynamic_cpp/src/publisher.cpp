// Copyright 2019 Open Source Robotics Foundation, Inc.
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

#include "fastdds/dds/publisher/Publisher.hpp"

#include <string>

#include "fastdds/dds/core/policy/QosPolicies.hpp"
#include "fastdds/dds/domain/DomainParticipant.hpp"
#include "fastdds/dds/publisher/qos/DataWriterQos.hpp"
#include "fastdds/dds/topic/Topic.hpp"
#include "fastdds/dds/topic/TopicDescription.hpp"
#include "fastdds/dds/topic/TypeSupport.hpp"
#include "fastdds/dds/topic/qos/TopicQos.hpp"
#include "fastdds/rtps/resources/ResourceManagement.h"
#include "rcpputils/scope_exit.hpp"
#include "rcutils/error_handling.h"
#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/rmw.h"
#include "rmw/validate_full_topic_name.h"
#include "rmw_fastrtps_dynamic_cpp/identifier.hpp"
#include "rmw_fastrtps_dynamic_cpp/publisher.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/custom_publisher_info.hpp"
#include "rmw_fastrtps_shared_cpp/names.hpp"
#include "rmw_fastrtps_shared_cpp/namespace_prefix.hpp"
#include "rmw_fastrtps_shared_cpp/qos.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/utils.hpp"
#include "type_support_common.hpp"
#include "type_support_registry.hpp"

using TypeSupportProxy = rmw_fastrtps_dynamic_cpp::TypeSupportProxy;

/**
 * @brief 创建一个 FastRTPS 动态类型的 ROS2 发布者 (Create a FastRTPS dynamic type ROS2 publisher)
 *
 * @param[in] participant_info 参与者信息，包含了 FastRTPS 的参与者实例 (Participant information,
 * contains the FastRTPS participant instance)
 * @param[in] type_supports 消息类型支持，用于识别和处理特定的消息类型 (Message type support, used
 * to identify and handle specific message types)
 * @param[in] topic_name 要发布的主题名称 (The topic name to be published)
 * @param[in] qos_policies 发布者的 QoS 策略 (Quality of Service policies for the publisher)
 * @param[in] publisher_options 发布者选项，包括安全选项等 (Publisher options, including security
 * options etc.)
 *
 * @return rmw_publisher_t* 成功时返回一个指向新创建的发布者的指针，失败时返回 nullptr (A pointer to
 * the newly created publisher on success, nullptr on failure)
 */
rmw_publisher_t *rmw_fastrtps_dynamic_cpp::create_publisher(
    CustomParticipantInfo *participant_info,
    const rosidl_message_type_support_t *type_supports,
    const char *topic_name,
    const rmw_qos_profile_t *qos_policies,
    const rmw_publisher_options_t *publisher_options) {
  /////
  // 检查输入参数 (Check input parameters)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(nullptr);

  // 验证 participant_info 参数是否为空 (Verify if the participant_info argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(participant_info, nullptr);
  // 验证 type_supports 参数是否为空 (Verify if the type_supports argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(type_supports, nullptr);
  // 验证 topic_name 参数是否为空 (Verify if the topic_name argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(topic_name, nullptr);
  // 如果 topic_name 为空字符串，则返回错误 (If topic_name is an empty string, return an error)
  if (0 == strlen(topic_name)) {
    RMW_SET_ERROR_MSG("create_publisher() called with an empty topic_name argument");
    return nullptr;
  }
  // 验证 qos_policies 参数是否为空 (Verify if the qos_policies argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(qos_policies, nullptr);
  // 如果避免 ROS 命名空间约定为 false (If avoid_ros_namespace_conventions is false)
  if (!qos_policies->avoid_ros_namespace_conventions) {
    int validation_result = RMW_TOPIC_VALID;
    // 验证完整的主题名称 (Validate the full topic name)
    rmw_ret_t ret = rmw_validate_full_topic_name(topic_name, &validation_result, nullptr);
    // 如果验证结果不是 RMW_RET_OK，则返回 nullptr (If the validation result is not RMW_RET_OK,
    // return nullptr)
    if (RMW_RET_OK != ret) {
      return nullptr;
    }
    // 如果验证结果不是 RMW_TOPIC_VALID，则设置错误消息并返回 nullptr (If the validation result is
    // not RMW_TOPIC_VALID, set the error message and return nullptr)
    if (RMW_TOPIC_VALID != validation_result) {
      const char *reason = rmw_full_topic_name_validation_result_string(validation_result);
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "create_publisher() called with invalid topic name: %s", reason);
      return nullptr;
    }
  }
  // 验证 publisher_options 参数是否为空 (Verify if the publisher_options argument is null)
  RMW_CHECK_ARGUMENT_FOR_NULL(publisher_options, nullptr);

  // 如果要求唯一的网络流端点严格为 RMW_UNIQUE_NETWORK_FLOW_ENDPOINTS_STRICTLY_REQUIRED
  // 则设置错误消息并返回 nullptr (If require_unique_network_flow_endpoints is set to
  // RMW_UNIQUE_NETWORK_FLOW_ENDPOINTS_STRICTLY_REQUIRED, set the error message and return nullptr)
  if (RMW_UNIQUE_NETWORK_FLOW_ENDPOINTS_STRICTLY_REQUIRED ==
      publisher_options->require_unique_network_flow_endpoints) {
    RMW_SET_ERROR_MSG("Unique network flow endpoints not supported on publishers");
    return nullptr;
  }

  /////
  // 检查 RMW QoS (Check RMW QoS)
  if (!is_valid_qos(*qos_policies)) {
    // 如果 QoS 策略无效，则设置错误消息 (If the QoS policies are invalid, set an error message)
    RMW_SET_ERROR_MSG("create_publisher() called with invalid QoS");
    // 返回空指针，表示创建发布者失败 (Return nullptr, indicating that the publisher creation
    // failed)
    return nullptr;
  }

  /////
  /**
   * @brief 获取 RMW 类型支持 (Get RMW Type Support)
   *
   * @param[in] type_supports 消息类型支持列表 (List of message type supports)
   * @return rosidl_message_type_support_t* 返回找到的类型支持指针，如果没有找到则返回 nullptr
   * (Returns the found type support pointer, or nullptr if not found)
   */
  const rosidl_message_type_support_t *type_support =
      // 从类型支持列表中获取与 rosidl_typesupport_introspection_c__identifier 匹配的类型支持句柄
      // Get the type support handle matching with rosidl_typesupport_introspection_c__identifier
      // from the list of type supports
      get_message_typesupport_handle(type_supports, rosidl_typesupport_introspection_c__identifier);

  if (!type_support) {
    // 获取先前的错误字符串
    // Get the previous error string
    rcutils_error_string_t prev_error_string = rcutils_get_error_string();
    // 重置错误
    // Reset the error
    rcutils_reset_error();

    // 从类型支持列表中获取与 rosidl_typesupport_introspection_cpp::typesupport_identifier
    // 匹配的类型支持句柄 Get the type support handle matching with
    // rosidl_typesupport_introspection_cpp::typesupport_identifier from the list of type supports
    type_support = get_message_typesupport_handle(
        type_supports, rosidl_typesupport_introspection_cpp::typesupport_identifier);

    if (!type_support) {
      // 获取当前的错误字符串
      // Get the current error string
      rcutils_error_string_t error_string = rcutils_get_error_string();
      // 重置错误
      // Reset the error
      rcutils_reset_error();

      // 设置错误消息，包括先前和当前的错误字符串
      // Set the error message, including the previous and current error strings
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
          "Type support not from this implementation. Got:\n"
          "    %s\n"
          "    %s\n"
          "while fetching it",
          prev_error_string.str, error_string.str);
      return nullptr;
    }
  }

  // 对 participant_info 的实体创建互斥锁进行加锁，以确保线程安全
  // Lock the entity creation mutex of participant_info to ensure thread safety
  std::lock_guard<std::mutex> lck(participant_info->entity_creation_mutex_);

  /////
  // 找到并检查现有的主题和类型
  // Find and check existing topic and type

  // 创建主题和类型名称
  // Create Topic and Type names
  std::string type_name =
      _create_type_name(type_support->data, type_support->typesupport_identifier);
  auto topic_name_mangled =
      _create_topic_name(qos_policies, ros_topic_prefix, topic_name).to_string();

  // 定义 eprosima::fastdds::dds::TypeSupport 变量 fastdds_type
  // Define eprosima::fastdds::dds::TypeSupport variable fastdds_type
  eprosima::fastdds::dds::TypeSupport fastdds_type;

  // 定义 eprosima::fastdds::dds::TopicDescription 指针 des_topic，并初始化为 nullptr
  // Define eprosima::fastdds::dds::TopicDescription pointer des_topic and initialize it to nullptr
  eprosima::fastdds::dds::TopicDescription *des_topic = nullptr;

  // 调用 find_and_check_topic_and_type 函数，检查参与者信息、主题名称和类型名称是否匹配
  // Call the find_and_check_topic_and_type function to check if participant_info,
  // topic_name_mangled, and type_name match
  if (!rmw_fastrtps_shared_cpp::find_and_check_topic_and_type(
          participant_info, topic_name_mangled, type_name, &des_topic, &fastdds_type)) {
    // 如果不匹配，设置错误消息并返回 nullptr
    // If they don't match, set an error message and return nullptr
    RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
        "create_publisher() called with existing topic name %s with incompatible type %s",
        topic_name_mangled.c_str(), type_name.c_str());
    return nullptr;
  }

  /////
  // 获取参与者（Participant）和发布者（Publisher）
  // Get Participant and Publisher
  eprosima::fastdds::dds::DomainParticipant *dds_participant = participant_info->participant_;
  eprosima::fastdds::dds::Publisher *publisher = participant_info->publisher_;

  /////
  // 创建自定义发布者结构体（info）
  // Create the custom Publisher struct (info)
  auto info = new (std::nothrow) CustomPublisherInfo();
  if (!info) {
    // 如果分配失败，设置错误信息
    // If allocation fails, set error message
    RMW_SET_ERROR_MSG("create_publisher() failed to allocate CustomPublisherInfo");
    return nullptr;
  }

  // 创建清理 info 的作用域退出对象
  // Create a scope exit object for cleaning up info
  /**
   * @brief 创建一个自定义发布者结构体（CustomPublisherInfo）并进行初始化。
   *        Create and initialize a custom Publisher struct (CustomPublisherInfo).
   *
   * @param[in] participant_info 指向 DomainParticipant 和 Publisher 的指针。
   *                             Pointer to DomainParticipant and Publisher.
   * @param[out] info 创建的自定义发布者结构体（CustomPublisherInfo）。
   *                  The created custom Publisher struct (CustomPublisherInfo).
   *
   * @return 如果创建成功，返回指向 CustomPublisherInfo 的指针；否则返回 nullptr。
   *         If creation is successful, return a pointer to CustomPublisherInfo; otherwise, return
   * nullptr.
   */
  auto cleanup_info = rcpputils::make_scope_exit([info, participant_info]() {
    // 移除主题（topic）和类型（type）以及相关的事件（event）
    // Remove topic and type along with their associated event
    rmw_fastrtps_shared_cpp::remove_topic_and_type(
        participant_info, info->publisher_event_, info->topic_, info->type_support_);
    // 删除数据写入监听器（data_writer_listener）和发布者事件（publisher_event）
    // Delete data_writer_listener and publisher_event
    delete info->data_writer_listener_;
    delete info->publisher_event_;
    // 删除 info
    // Delete info
    delete info;
  });

  /////
  // 创建 Type Support 结构体 (Create the Type Support struct)
  TypeSupportRegistry &type_registry = TypeSupportRegistry::get_instance();

  // 从类型支持注册表中获取消息类型支持实现 (Get the message type support implementation from the
  // type support registry)
  auto type_impl = type_registry.get_message_type_support(type_support);
  if (!type_impl) {
    // 设置错误消息，表示无法获取 message_type_support (Set error message indicating that getting
    // message_type_support failed)
    RMW_SET_ERROR_MSG("create_publisher() failed to get message_type_support");
    return nullptr;
  }

  // 当离开作用域时，返回类型支持 (Return the type support when leaving the scope)
  auto return_type_support = rcpputils::make_scope_exit([&type_registry, type_support]() {
    type_registry.return_message_type_support(type_support);
  });

  // 将类型支持标识符和实现存储在 info 中 (Store the typesupport identifier and implementation in
  // info)
  info->typesupport_identifier_ = type_support->typesupport_identifier;
  info->type_support_impl_ = type_impl;

  // 如果 fastdds_type 不存在 (If fastdds_type does not exist)
  if (!fastdds_type) {
    // 创建一个新的 TypeSupportProxy (Create a new TypeSupportProxy)
    auto tsupport = new (std::nothrow) TypeSupportProxy(type_impl);
    if (!tsupport) {
      // 设置错误消息，表示无法分配 TypeSupportProxy (Set error message indicating that allocating
      // TypeSupportProxy failed)
      RMW_SET_ERROR_MSG("create_publisher() failed to allocate TypeSupportProxy");
      return nullptr;
    }

    // 将所有权转移到 fastdds_type (Transfer ownership to fastdds_type)
    fastdds_type.reset(tsupport);
  }

  // 在 dds_participant 中注册类型 (Register the type in dds_participant)
  if (ReturnCode_t::RETCODE_OK != fastdds_type.register_type(dds_participant)) {
    // 设置错误消息，表示无法注册类型 (Set error message indicating that registering type failed)
    RMW_SET_ERROR_MSG("create_publisher() failed to register type");
    return nullptr;
  }

  // 将 fastdds_type 存储在 info 中 (Store the fastdds_type in info)
  info->type_support_ = fastdds_type;

  /////
  /**
   * @brief 创建监听器 (Create Listener)
   *
   * @param[in] info 传入的发布者信息结构体指针 (Pointer to the publisher information structure)
   *
   * @return nullptr 如果创建失败，否则返回创建的监听器对象 (nullptr if creation fails, otherwise
   * returns the created listener object)
   */

  // 创建 RMWPublisherEvent 对象并分配内存 (Create an RMWPublisherEvent object and allocate memory)
  // 使用 std::nothrow 避免抛出异常 (Use std::nothrow to avoid throwing exceptions)
  info->publisher_event_ = new (std::nothrow) RMWPublisherEvent(info);
  if (!info->publisher_event_) {
    // 如果创建失败，设置错误消息 (If creation fails, set the error message)
    RMW_SET_ERROR_MSG("create_publisher() could not create publisher event");
    return nullptr;
  }

  // 创建 CustomDataWriterListener 对象并分配内存 (Create a CustomDataWriterListener object and
  // allocate memory) 使用 std::nothrow 避免抛出异常 (Use std::nothrow to avoid throwing exceptions)
  info->data_writer_listener_ = new (std::nothrow) CustomDataWriterListener(info->publisher_event_);
  if (!info->data_writer_listener_) {
    // 如果创建失败，设置错误消息 (If creation fails, set the error message)
    RMW_SET_ERROR_MSG("create_publisher() could not create publisher data writer listener");
    return nullptr;
  }

  /////
  // 创建并注册 Topic（Create and register Topic）
  eprosima::fastdds::dds::TopicQos topic_qos = dds_participant->get_default_topic_qos();

  // 获取 Topic 的 QoS 设置，如果失败则返回错误信息（Get the Topic QoS settings, and return an error
  // message if it fails） 参数列表说明（Parameter list description）: *qos_policies: 输入的 QoS
  // 策略（Input QoS policies） topic_qos: 输出的 Topic QoS 设置（Output Topic QoS settings）
  if (!get_topic_qos(*qos_policies, topic_qos)) {
    RMW_SET_ERROR_MSG("create_publisher() failed setting topic QoS");
    return nullptr;
  }

  // 查找或创建一个主题（Find or create a topic）
  // 参数列表说明（Parameter list description）:
  // topic_name_mangled: 主题名称（Topic name）
  // type_name: 类型名称（Type name）
  // topic_qos: 主题 QoS 设置（Topic QoS settings）
  // info->publisher_event_: 发布者事件（Publisher event）
  info->topic_ = participant_info->find_or_create_topic(
      topic_name_mangled, type_name, topic_qos, info->publisher_event_);

  // 如果无法创建主题，则返回错误信息（If the topic cannot be created, return an error message）
  if (!info->topic_) {
    RMW_SET_ERROR_MSG("create_publisher() failed to create topic");
    return nullptr;
  }

  /////
  /**
   * @brief Create DataWriter
   * FASTRTPS_DEFAULT_PROFILES_FILE: 环境变量，用户可以通过此变量定义一个 XML 文件来配置 Fast DDS 的
   QoS。
   * topic_name: 主题名称，用于匹配配置文件中的 profile 名称。
   * publisher: 发布者对象，用于创建和管理 DataWriter。
   * participant_info: 参与者信息，包含了一些与 QoS 相关的设置。
   * qos_policies: RMW 层的 QoS 策略。
   * type_supports: 类型支持对象，用于获取类型哈希函数。
   * info: DataWriter 信息，包含了创建 DataWriter 所需的一些参数。
   */
  // 如果用户通过环境变量 "FASTRTPS_DEFAULT_PROFILES_FILE" 定义了一个 XML 文件，尝试加载与
  // topic_name 匹配的 profile 名称的 datawriter。 如果此类配置文件不存在，则使用默认的 Fast DDS
  // QoS。 If the user defined an XML file via env "FASTRTPS_DEFAULT_PROFILES_FILE", try to load
  // datawriter which profile name matches with topic_name. If such profile does not exist,
  // then use the default Fast DDS QoS.
  eprosima::fastdds::dds::DataWriterQos writer_qos = publisher->get_default_datawriter_qos();

  // 尝试使用 topic_name 加载配置文件
  // 不需要检查返回代码，因为如果配置文件不存在，QoS 已经是默认值
  // Try to load the profile with the topic name
  // It does not need to check the return code, as if the profile does not exist,
  // the QoS is already the default
  publisher->get_datawriter_qos_from_profile(topic_name, writer_qos);

  // 修改特定的 DataWriter Qos
  // Modify specific DataWriter Qos
  if (!participant_info->leave_middleware_default_qos) {
    if (participant_info->publishing_mode == publishing_mode_t::ASYNCHRONOUS) {
      writer_qos.publish_mode().kind = eprosima::fastrtps::ASYNCHRONOUS_PUBLISH_MODE;
    } else if (participant_info->publishing_mode == publishing_mode_t::SYNCHRONOUS) {
      writer_qos.publish_mode().kind = eprosima::fastrtps::SYNCHRONOUS_PUBLISH_MODE;
    }

    writer_qos.endpoint().history_memory_policy =
        eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;

    writer_qos.data_sharing().off();
  }

  // 从 RMW 获取 QoS
  // Get QoS from RMW
  if (!get_datawriter_qos(
          *qos_policies, *type_supports->get_type_hash_func(type_supports), writer_qos)) {
    RMW_SET_ERROR_MSG("create_publisher() failed setting data writer QoS");
    return nullptr;
  }

  // 创建 DataWriter（使用 publisher 名称以不更改名称策略）
  // Creates DataWriter (with publisher name to not change name policy)
  info->data_writer_ = publisher->create_datawriter(
      info->topic_, writer_qos, info->data_writer_listener_,
      eprosima::fastdds::dds::StatusMask::publication_matched());

  if (!info->data_writer_) {
    RMW_SET_ERROR_MSG("create_publisher() could not create data writer");
    return nullptr;
  }

  info->data_writer_->get_statuscondition().set_enabled_statuses(
      eprosima::fastdds::dds::StatusMask::none());

  // lambda 函数用于删除 datawriter
  // lambda to delete datawriter
  auto cleanup_datawriter = rcpputils::make_scope_exit(
      [publisher, info]() { publisher->delete_datawriter(info->data_writer_); });

  /////
  /**
   * @brief 创建 RMW GID (Create RMW GID)
   *
   * @param[in] eprosima_fastrtps_identifier 唯一标识符，用于创建 RMW GID (Unique identifier used to
   * create RMW GID)
   * @param[in] guid info->data_writer_ 的 GUID，作为参数传递 (The GUID of info->data_writer_,
   * passed as a parameter)
   *
   * @return 返回创建的 RMW GID (Returns the created RMW GID)
   */
  info->publisher_gid = rmw_fastrtps_shared_cpp::create_rmw_gid(
      eprosima_fastrtps_identifier, info->data_writer_->guid());

  /////
  // 为发布者分配内存（Allocate memory for the publisher）
  rmw_publisher_t *rmw_publisher = rmw_publisher_allocate();
  // 如果无法分配内存，则设置错误信息并返回空指针（If memory allocation fails, set error message and
  // return nullptr）
  if (!rmw_publisher) {
    RMW_SET_ERROR_MSG("create_publisher() failed to allocate rmw_publisher");
    return nullptr;
  }
  // 创建一个作用域退出对象，用于在函数退出时清理 rmw_publisher 资源（Create a scope exit object to
  // clean up rmw_publisher resources when the function exits）
  auto cleanup_rmw_publisher = rcpputils::make_scope_exit([rmw_publisher]() {
    rmw_free(const_cast<char *>(rmw_publisher->topic_name));
    rmw_publisher_free(rmw_publisher);
  });

  // 设置发布者是否可以借用消息的标志（Set the flag indicating whether the publisher can loan
  // messages or not）
  rmw_publisher->can_loan_messages = info->type_support_->is_plain();
  // 设置实现标识符（Set the implementation identifier）
  rmw_publisher->implementation_identifier = eprosima_fastrtps_identifier;
  // 将 info 数据结构与发布者关联（Associate the info data structure with the publisher）
  rmw_publisher->data = info;

  // 为发布者主题名称分配内存（Allocate memory for the publisher's topic name）
  rmw_publisher->topic_name = static_cast<char *>(rmw_allocate(strlen(topic_name) + 1));
  // 如果无法分配内存，则设置错误信息并返回空指针（If memory allocation fails, set error message and
  // return nullptr）
  if (!rmw_publisher->topic_name) {
    RMW_SET_ERROR_MSG("create_publisher() failed to allocate memory for rmw_publisher topic name");
    return nullptr;
  }
  // 将主题名称复制到发布者的 topic_name 成员中（Copy the topic name into the publisher's topic_name
  // member）
  memcpy(const_cast<char *>(rmw_publisher->topic_name), topic_name, strlen(topic_name) + 1);

  // 设置发布者选项（Set the publisher options）
  rmw_publisher->options = *publisher_options;

  // 取消清理操作，因为已成功创建发布者（Cancel cleanup operations as the publisher has been
  // successfully created）
  cleanup_rmw_publisher.cancel();
  cleanup_datawriter.cancel();
  return_type_support.cancel();
  cleanup_info.cancel();

  // 返回创建的发布者指针（Return the created publisher pointer）
  return rmw_publisher;
}
