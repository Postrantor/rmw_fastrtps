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

#include "rmw_fastrtps_shared_cpp/participant.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "fastdds/dds/core/status/StatusMask.hpp"
#include "fastdds/dds/domain/DomainParticipantFactory.hpp"
#include "fastdds/dds/domain/qos/DomainParticipantQos.hpp"
#include "fastdds/dds/publisher/DataWriter.hpp"
#include "fastdds/dds/publisher/Publisher.hpp"
#include "fastdds/dds/publisher/qos/PublisherQos.hpp"
#include "fastdds/dds/subscriber/DataReader.hpp"
#include "fastdds/dds/subscriber/Subscriber.hpp"
#include "fastdds/dds/subscriber/qos/SubscriberQos.hpp"
#include "fastdds/rtps/attributes/PropertyPolicy.h"
#include "fastdds/rtps/common/Locator.h"
#include "fastdds/rtps/common/Property.h"
#include "fastdds/rtps/transport/UDPv4TransportDescriptor.h"
#include "fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h"
#include "fastrtps/utils/IPLocator.h"
#include "rcpputils/scope_exit.hpp"
#include "rcutils/env.h"
#include "rcutils/filesystem.h"
#include "rmw/allocators.h"
#include "rmw_dds_common/security.hpp"
#include "rmw_fastrtps_shared_cpp/custom_participant_info.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_common.hpp"
#include "rmw_fastrtps_shared_cpp/rmw_security_logging.hpp"
#include "rmw_fastrtps_shared_cpp/utils.hpp"

/**
 * @brief 创建具有QoS的参与者的私有函数 (Private function to create Participant with QoS)
 *
 * @param[in] identifier 参与者标识符 (Participant identifier)
 * @param[in] domainParticipantQos 域参与者QoS (DomainParticipant QoS)
 * @param[in] leave_middleware_default_qos 是否保留中间件默认QoS (Whether to leave middleware
 * default QoS)
 * @param[in] publishing_mode 发布模式 (Publishing mode)
 * @param[in] common_context 公共上下文 (Common context)
 * @param[in] domain_id 域ID (Domain ID)
 * @return CustomParticipantInfo* 自定义参与者信息指针 (Pointer to CustomParticipantInfo)
 */
static CustomParticipantInfo *__create_participant(
    const char *identifier,
    const eprosima::fastdds::dds::DomainParticipantQos &domainParticipantQos,
    bool leave_middleware_default_qos,
    publishing_mode_t publishing_mode,
    rmw_dds_common::Context *common_context,
    size_t domain_id) {
  CustomParticipantInfo *participant_info = nullptr;

  /////
  // 创建自定义参与者 (Create Custom Participant)
  try {
    participant_info = new CustomParticipantInfo();
  } catch (std::bad_alloc &) {
    RMW_SET_ERROR_MSG("__create_participant failed to allocate CustomParticipantInfo struct");
    return nullptr;
  }
  // lambda用于删除参与者信息 (Lambda to delete participant info)
  auto cleanup_participant_info = rcpputils::make_scope_exit([participant_info]() {
    if (nullptr != participant_info->participant_) {
      participant_info->participant_->delete_publisher(participant_info->publisher_);
      eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(
          participant_info->participant_);
    }
    delete participant_info->listener_;
    delete participant_info;
  });

  /////
  // 创建参与者监听器 (Create Participant listener)
  try {
    participant_info->listener_ = new ParticipantListener(identifier, common_context);
  } catch (std::bad_alloc &) {
    RMW_SET_ERROR_MSG("__create_participant failed to allocate participant listener");
    return nullptr;
  }

  /////
  // 创建参与者 (Create Participant)

  // 由于参与者监听器仅用于与发现相关的回调，这些回调是Fast
  // DDS对DDS标准DomainParticipantListener接口的扩展， 因此应使用空掩码让子实体处理标准DDS事件。 (As
  // the participant listener is only used for discovery related callbacks, which are Fast DDS
  // extensions to the DDS standard DomainParticipantListener interface, an empty mask should be
  // used to let child entities handle standard DDS events.)
  eprosima::fastdds::dds::StatusMask participant_mask = eprosima::fastdds::dds::StatusMask::none();

  participant_info->participant_ =
      eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
          static_cast<uint32_t>(domain_id), domainParticipantQos, participant_info->listener_,
          participant_mask);

  if (!participant_info->participant_) {
    RMW_SET_ERROR_MSG("__create_participant failed to create participant");
    return nullptr;
  }

  /////
  // 设置参与者信息参数 (Set participant info parameters)
  participant_info->leave_middleware_default_qos = leave_middleware_default_qos;
  participant_info->publishing_mode = publishing_mode;

  /////
  // 创建发布者 (Create Publisher)
  eprosima::fastdds::dds::PublisherQos publisherQos =
      participant_info->participant_->get_default_publisher_qos();
  publisherQos.entity_factory(domainParticipantQos.entity_factory());

  participant_info->publisher_ = participant_info->participant_->create_publisher(publisherQos);
  if (!participant_info->publisher_) {
    RMW_SET_ERROR_MSG("__create_participant could not create publisher");
    return nullptr;
  }

  /////
  // 创建订阅者 (Create Subscriber)
  eprosima::fastdds::dds::SubscriberQos subscriberQos =
      participant_info->participant_->get_default_subscriber_qos();
  subscriberQos.entity_factory(domainParticipantQos.entity_factory());

  participant_info->subscriber_ = participant_info->participant_->create_subscriber(subscriberQos);
  if (!participant_info->subscriber_) {
    RMW_SET_ERROR_MSG("__create_participant could not create subscriber");
    return nullptr;
  }

  cleanup_participant_info.cancel();

  return participant_info;
}

/**
 * @brief 创建一个自定义参与者 (Create a custom participant)
 *
 * @param[in] identifier 参与者的标识符 (Identifier of the participant)
 * @param[in] domain_id 域 ID (Domain ID)
 * @param[in] security_options 安全选项 (Security options)
 * @param[in] discovery_options 发现选项 (Discovery options)
 * @param[in] enclave 参与者的隔离区域 (Enclave of the participant)
 * @param[in,out] common_context 共享上下文 (Shared context)
 *
 * @return 返回创建的 CustomParticipantInfo 指针，如果失败则返回 nullptr (Returns a pointer to the
 * created CustomParticipantInfo, or nullptr if failed)
 */
CustomParticipantInfo *rmw_fastrtps_shared_cpp::create_participant(
    const char *identifier,
    size_t domain_id,
    const rmw_security_options_t *security_options,
    const rmw_discovery_options_t *discovery_options,
    const char *enclave,
    rmw_dds_common::Context *common_context) {
  // 检查是否可以返回空指针错误 (Check if it can return with an error of nullptr)
  RCUTILS_CAN_RETURN_WITH_ERROR_OF(nullptr);

  // 检查security_options是否为空 (Check if security_options is null)
  if (!security_options) {
    RMW_SET_ERROR_MSG("security_options is null");
    return nullptr;
  }

  // 加载默认的XML配置文件 (Load default XML profile)
  eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_profiles();
  eprosima::fastdds::dds::DomainParticipantQos domainParticipantQos =
      eprosima::fastdds::dds::DomainParticipantFactory::get_instance()
          ->get_default_participant_qos();

  // 配置发现选项 (Configure discovery options)
  switch (discovery_options->automatic_discovery_range) {
    case RMW_AUTOMATIC_DISCOVERY_RANGE_NOT_SET:
      RMW_SET_ERROR_MSG("automatic discovery range must be set");
      return nullptr;
      break;
    case RMW_AUTOMATIC_DISCOVERY_RANGE_OFF: {
      // 将参与者数量限制为1（本地参与者）(Limit the number of participants to 1 (the local
      // participant))
      domainParticipantQos.allocation().participants.initial = 1;
      domainParticipantQos.allocation().participants.maximum = 1;
      domainParticipantQos.allocation().participants.increment = 0;
      // 清除多播监听定位器列表 (Clear the list of multicast listening locators)
      domainParticipantQos.wire_protocol().builtin.metatrafficMulticastLocatorList.clear();
      // 添加一个单播定位器以防止创建默认的多播定位器 (Add a unicast locator to prevent creation of
      // default multicast locator)
      eprosima::fastrtps::rtps::Locator_t default_unicast_locator;
      domainParticipantQos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(
          default_unicast_locator);
      break;
    }
    case RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST: {
      // 清除多播监听定位器列表 (Clear the list of multicast listening locators)
      domainParticipantQos.wire_protocol().builtin.metatrafficMulticastLocatorList.clear();
      // 添加一个单播定位器以防止创建默认的多播定位器 (Add a unicast locator to prevent creation of
      // default multicast locator)
      eprosima::fastrtps::rtps::Locator_t default_unicast_locator;
      domainParticipantQos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(
          default_unicast_locator);
      // 禁用内置传输，因为我们正在配置我们自己的传输 (Disable built-in transports, since we are
      // configuring our own)
      domainParticipantQos.transport().use_builtin_transports = false;
      // 添加共享内存传输 (Add a shared memory transport)
      auto shm_transport =
          std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>();
      domainParticipantQos.transport().user_transports.push_back(shm_transport);
      // 添加UDP传输并增加最大初始对等点 (Add UDP transport with increased max initial peers)
      auto udp_transport = std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
      udp_transport->maxInitialPeersRange = 32;
      domainParticipantQos.transport().user_transports.push_back(udp_transport);
      break;
    }
    case RMW_AUTOMATIC_DISCOVERY_RANGE_SUBNET:
      // 无需执行任何操作；使用默认的FastDDS行为 (Nothing to do; use the default FastDDS behaviour)
      break;
    case RMW_AUTOMATIC_DISCOVERY_RANGE_SYSTEM_DEFAULT:
      // 无需执行任何操作；使用默认的FastDDS行为 (Nothing to do; use the default FastDDS behaviour)
      break;
    default:
      RMW_SET_ERROR_MSG("automatic_discovery_range is an unknown value");
      return nullptr;
      break;
  }

  /**
   * @brief 添加初始对等节点，如果使用了 LOCALHOST 或 SUBNET。
   *        Add initial peers if LOCALHOST or SUBNET are used.
   *
   * @param[in] discovery_options 用于配置发现选项的结构体指针。
   *                              Pointer to the structure for configuring discovery options.
   */
  // 如果自动发现范围为 LOCALHOST 或 SUBNET，则添加初始对等节点。
  // Add initial peers if LOCALHOST or SUBNET are used in automatic discovery range.
  if (RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST == discovery_options->automatic_discovery_range ||
      RMW_AUTOMATIC_DISCOVERY_RANGE_SUBNET == discovery_options->automatic_discovery_range) {
    // 遍历静态对等节点。
    // Iterate through static peers.
    for (size_t ii = 0; ii < discovery_options->static_peers_count; ++ii) {
      eprosima::fastrtps::rtps::Locator_t peer;
      // 解析对等节点地址的 DNS 名称。
      // Resolve the DNS name of the peer address.
      auto response = eprosima::fastrtps::rtps::IPLocator::resolveNameDNS(
          discovery_options->static_peers[ii].peer_address);
      // 获取第一个返回的 IPv4 地址。
      // Get the first returned IPv4 address.
      if (response.first.size() > 0) {
        eprosima::fastrtps::rtps::IPLocator::setIPv4(peer, response.first.begin()->data());
      } else {
        // 无法解析对等节点，设置错误消息并返回 nullptr。
        // Unable to resolve peer, set error message and return nullptr.
        RMW_SET_ERROR_MSG_WITH_FORMAT_STRING(
            "Unable to resolve peer %s\n", discovery_options->static_peers[ii].peer_address);
        return nullptr;
      }
      // 不指定对等节点的端口意味着 FastDDS 将尝试所有可能的参与者端口，根据 RTPS 规范第 9.6.1.1
      // 节的端口计算公式，直到 maxInitialPeersRange 中指定的对等节点数量。 Not specifying the port
      // of the peer means FastDDS will try all possible participant ports according to the port
      // calculation equation in the RTPS spec section 9.6.1.1, up to the number of peers specified
      // in maxInitialPeersRange.
      domainParticipantQos.wire_protocol().builtin.initialPeersList.push_back(peer);
    }
  }

  // 如果自动发现范围为 LOCALHOST，则将 localhost 添加为静态对等节点。
  // If automatic discovery range is LOCALHOST, add localhost as a static peer.
  if (RMW_AUTOMATIC_DISCOVERY_RANGE_LOCALHOST == discovery_options->automatic_discovery_range) {
    eprosima::fastrtps::rtps::Locator_t peer;
    eprosima::fastrtps::rtps::IPLocator::setIPv4(peer, "127.0.0.1");
    domainParticipantQos.wire_protocol().builtin.initialPeersList.push_back(peer);
  }

  // 如果自动发现范围为 SUBNET 且初始对等节点列表中有元素，则确保在多播地址上发送通告。
  // If automatic discovery range is SUBNET and there are elements in the initial peers list, make
  // sure to send an announcement on the multicast address.
  if (RMW_AUTOMATIC_DISCOVERY_RANGE_SUBNET == discovery_options->automatic_discovery_range &&
      domainParticipantQos.wire_protocol().builtin.initialPeersList.size()) {
    eprosima::fastrtps::rtps::Locator_t locator;
    eprosima::fastrtps::rtps::IPLocator::setIPv4(locator, 239, 255, 0, 1);
    domainParticipantQos.wire_protocol().builtin.initialPeersList.push_back(locator);
  }

  // 计算用户数据缓冲区所需的长度。
  // Calculate the required length for the user_data buffer.
  size_t length = snprintf(nullptr, 0, "enclave=%s;", enclave) + 1;
  domainParticipantQos.user_data().resize(length);

  // 将 enclave 写入用户数据缓冲区。
  // Write the enclave into the user_data buffer.
  int written = snprintf(
      reinterpret_cast<char *>(domainParticipantQos.user_data().data()), length, "enclave=%s;",
      enclave);
  // 如果写入失败或超出缓冲区长度，设置错误消息并返回 nullptr。
  // If writing fails or exceeds the buffer length, set error message and return nullptr.
  if (written < 0 || written > static_cast<int>(length) - 1) {
    RMW_SET_ERROR_MSG("failed to populate user_data buffer");
    return nullptr;
  }
  // 设置域参与者 QoS 的名称为 enclave。
  // Set the name of the domain participant QoS to enclave.
  domainParticipantQos.name(enclave);

  bool leave_middleware_default_qos = false;
  publishing_mode_t publishing_mode = publishing_mode_t::SYNCHRONOUS;
  const char *env_value;
  const char *error_str;

  // 获取环境变量 "RMW_FASTRTPS_USE_QOS_FROM_XML"
  // Get environment variable "RMW_FASTRTPS_USE_QOS_FROM_XML"
  error_str = rcutils_get_env("RMW_FASTRTPS_USE_QOS_FROM_XML", &env_value);
  if (error_str != NULL) {
    RCUTILS_LOG_DEBUG_NAMED("rmw_fastrtps_shared_cpp", "Error getting env var: %s\n", error_str);
    return nullptr;
  }

  // 检查环境变量是否为 "1"
  // Check if the environment variable is "1"
  if (env_value != nullptr) {
    leave_middleware_default_qos = strcmp(env_value, "1") == 0;
  }

  // 如果不使用中间件默认的QoS设置
  // If not using middleware default QoS settings
  if (!leave_middleware_default_qos) {
    // 获取环境变量 "RMW_FASTRTPS_PUBLICATION_MODE"
    // Get environment variable "RMW_FASTRTPS_PUBLICATION_MODE"
    error_str = rcutils_get_env("RMW_FASTRTPS_PUBLICATION_MODE", &env_value);
    if (error_str != NULL) {
      RMW_SET_ERROR_MSG_WITH_FORMAT_STRING("Error getting env var: %s\n", error_str);
      return nullptr;
    }

    // 根据环境变量值设置发布模式
    // Set publishing mode based on environment variable value
    if (env_value != nullptr) {
      if (strcmp(env_value, "SYNCHRONOUS") == 0) {
        publishing_mode = publishing_mode_t::SYNCHRONOUS;
      } else if (strcmp(env_value, "ASYNCHRONOUS") == 0) {
        publishing_mode = publishing_mode_t::ASYNCHRONOUS;
      } else if (strcmp(env_value, "AUTO") == 0) {
        publishing_mode = publishing_mode_t::AUTO;
      } else if (strcmp(env_value, "") != 0) {
        RCUTILS_LOG_WARN_NAMED(
            "rmw_fastrtps_shared_cpp",
            "Value %s unknown for environment variable RMW_FASTRTPS_PUBLICATION_MODE"
            ". Using default SYNCHRONOUS publishing mode.",
            env_value);
      }
    }
  }

  // 允许重新分配内存以支持大于5000字节的发现消息
  // Allow reallocation of memory to support discovery messages larger than 5000 bytes
  if (!leave_middleware_default_qos) {
    domainParticipantQos.wire_protocol().builtin.readerHistoryMemoryPolicy =
        eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    domainParticipantQos.wire_protocol().builtin.writerHistoryMemoryPolicy =
        eprosima::fastrtps::rtps::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
  }

  // 如果提供了security_root_path，尝试查找密钥和证书文件
  // If security_root_path is provided, try to find the key and certificate files
  if (security_options->security_root_path) {
#if HAVE_SECURITY
    // 定义一个无序映射，用于存储安全文件的路径
    // Define an unordered map to store the paths of security files
    std::unordered_map<std::string, std::string> security_files_paths;

    // 如果成功获取到安全文件，则进行以下操作
    // If successfully get the security files, do the following operations
    if (rmw_dds_common::get_security_files(
            "file://", security_options->security_root_path, security_files_paths)) {
      // 创建一个 PropertyPolicy 对象
      // Create a PropertyPolicy object
      eprosima::fastrtps::rtps::PropertyPolicy property_policy;

      // 添加认证插件属性
      // Add authentication plugin property
      property_policy.properties().emplace_back("dds.sec.auth.plugin", "builtin.PKI-DH");

      // 添加身份验证 CA 文件路径属性
      // Add identity CA file path property
      property_policy.properties().emplace_back(
          "dds.sec.auth.builtin.PKI-DH.identity_ca", security_files_paths["IDENTITY_CA"]);

      // 添加身份证书文件路径属性
      // Add identity certificate file path property
      property_policy.properties().emplace_back(
          "dds.sec.auth.builtin.PKI-DH.identity_certificate", security_files_paths["CERTIFICATE"]);

      // 添加私钥文件路径属性
      // Add private key file path property
      property_policy.properties().emplace_back(
          "dds.sec.auth.builtin.PKI-DH.private_key", security_files_paths["PRIVATE_KEY"]);

      // 添加加密插件属性
      // Add encryption plugin property
      property_policy.properties().emplace_back("dds.sec.crypto.plugin", "builtin.AES-GCM-GMAC");

      // 添加访问控制插件属性
      // Add access control plugin property
      property_policy.properties().emplace_back(
          "dds.sec.access.plugin", "builtin.Access-Permissions");

      // 添加访问控制权限 CA 文件路径属性
      // Add access control permissions CA file path property
      property_policy.properties().emplace_back(
          "dds.sec.access.builtin.Access-Permissions.permissions_ca",
          security_files_paths["PERMISSIONS_CA"]);

      // 添加治理文件路径属性
      // Add governance file path property
      property_policy.properties().emplace_back(
          "dds.sec.access.builtin.Access-Permissions.governance",
          security_files_paths["GOVERNANCE"]);

      // 添加权限文件路径属性
      // Add permissions file path property
      property_policy.properties().emplace_back(
          "dds.sec.access.builtin.Access-Permissions.permissions",
          security_files_paths["PERMISSIONS"]);

      // 如果存在 CRL 文件，则添加 CRL 文件路径属性
      // If there is a CRL file, add the CRL file path property
      if (security_files_paths.count("CRL") > 0) {
        property_policy.properties().emplace_back(
            "dds.sec.auth.builtin.PKI-DH.identity_crl", security_files_paths["CRL"]);
      }

      // 配置安全日志记录
      // Configure security logging
      if (!apply_security_logging_configuration(property_policy)) {
        return nullptr;
      }

      // 将 PropertyPolicy 应用于 domainParticipantQos
      // Apply the PropertyPolicy to domainParticipantQos
      domainParticipantQos.properties(property_policy);

      // 如果未找到所有安全文件且强制执行安全选项，则返回错误信息
      // If not all security files are found and enforce_security option is enabled, return an error
      // message
    } else if (security_options->enforce_security) {
      RMW_SET_ERROR_MSG("couldn't find all security files!");
      return nullptr;
    }
#else
    RMW_SET_ERROR_MSG(
        "This Fast DDS version doesn't have the security libraries\n"
        "Please compile Fast DDS using the -DSECURITY=ON CMake option");
    return nullptr;
#endif
  }
  return __create_participant(
      identifier, domainParticipantQos, leave_middleware_default_qos, publishing_mode,
      common_context, domain_id);
}

/**
 * @brief 销毁参与者 (Destroy a participant)
 *
 * @param[in] participant_info 自定义参与者信息 (Custom participant information)
 * @return rmw_ret_t 返回操作结果 (Return operation result)
 */
rmw_ret_t rmw_fastrtps_shared_cpp::destroy_participant(CustomParticipantInfo *participant_info) {
  // 检查 participant_info 是否为空 (Check if participant_info is null)
  if (!participant_info) {
    RMW_SET_ERROR_MSG("participant_info is null on destroy_participant");
    return RMW_RET_ERROR;
  }

  // 让参与者停止监听发现 (Make the participant stop listening to discovery)
  participant_info->participant_->set_listener(nullptr);

  ReturnCode_t ret = ReturnCode_t::RETCODE_OK;

  // 收集需要删除的主题 (Collect topics that should be deleted)
  std::vector<const eprosima::fastdds::dds::TopicDescription *> topics_to_remove;

  // 从参与者中移除数据写入器和发布者 (Remove datawriters and publisher from participant)
  {
    std::vector<eprosima::fastdds::dds::DataWriter *> writers;
    participant_info->publisher_->get_datawriters(writers);
    for (auto writer : writers) {
      topics_to_remove.push_back(writer->get_topic());
      participant_info->publisher_->delete_datawriter(writer);
    }
    ret = participant_info->participant_->delete_publisher(participant_info->publisher_);
    if (ReturnCode_t::RETCODE_OK != ret) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to delete dds publisher from participant");
    }
  }

  // 从参与者中移除数据读取器和订阅者 (Remove datareaders and subscriber from participant)
  {
    std::vector<eprosima::fastdds::dds::DataReader *> readers;
    participant_info->subscriber_->get_datareaders(readers);
    for (auto reader : readers) {
      topics_to_remove.push_back(reader->get_topicdescription());
      participant_info->subscriber_->delete_datareader(reader);
    }
    ret = participant_info->participant_->delete_subscriber(participant_info->subscriber_);
    if (ReturnCode_t::RETCODE_OK != ret) {
      RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to delete dds subscriber from participant");
    }
  }

  // 移除主题 (Remove topics)
  eprosima::fastdds::dds::TypeSupport dummy_type;
  for (auto topic : topics_to_remove) {
    // 将 nullptr 作为 EventListenerInterface 参数传递意味着
    // remove_topic_and_type() -> participant_info->delete_topic() 不会从
    // CustomTopicListener::event_listeners_ 集合中移除
    // EventListenerInterface。这将构成内存泄漏，但由于参与者将在下面被删除，因此整个集合将被销毁并释放所有内存。
    // (Passing nullptr as the EventListenerInterface argument means that
    // remove_topic_and_type() -> participant_info->delete_topic() will not remove an
    // the EventListenerInterface from the CustomTopicListener::event_listeners_ set.  That would
    // constitute a memory leak, except for the fact that the participant is going to be deleted
    // right below.  At that point, the entire set will be destroyed and all memory freed.)
    remove_topic_and_type(participant_info, nullptr, topic, dummy_type);
  }

  // 删除域参与者 (Delete Domain Participant)
  ret = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(
      participant_info->participant_);

  if (ReturnCode_t::RETCODE_OK != ret) {
    RCUTILS_SAFE_FWRITE_TO_STDERR("Failed to delete participant");
  }

  // 删除监听器 (Delete Listener)
  delete participant_info->listener_;

  // 删除自定义参与者 (Delete Custom Participant)
  delete participant_info;

  RCUTILS_CAN_RETURN_WITH_ERROR_OF(RMW_RET_ERROR);  // on completion

  return RMW_RET_OK;
}
