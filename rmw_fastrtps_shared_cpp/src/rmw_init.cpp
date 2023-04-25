// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#include "rmw_fastrtps_shared_cpp/rmw_init.hpp"

#include "rcutils/strdup.h"
#include "rcutils/types.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/init.h"
#include "rmw/init_options.h"

namespace rmw_fastrtps_shared_cpp {

/**
 * @brief 初始化 rmw_init_options_t 结构体
 * Initialize the rmw_init_options_t structure
 *
 * @param[in] identifier 实现的标识符
 * The implementation identifier
 * @param[out] init_options 要初始化的 rmw_init_options_t 结构体指针
 * Pointer to the rmw_init_options_t structure to be initialized
 * @param[in] allocator 分配器用于分配内存
 * Allocator used for memory allocation
 * @return RMW_RET_OK if successful, appropriate error code otherwise
 */
rmw_ret_t rmw_init_options_init(
    const char* identifier, rmw_init_options_t* init_options, rcutils_allocator_t allocator) {
  // 检查 identifier 是否为 NULL
  // Check if identifier is NULL
  assert(identifier != NULL);

  // 检查 init_options 是否为 NULL
  // Check if init_options is NULL
  RMW_CHECK_ARGUMENT_FOR_NULL(init_options, RMW_RET_INVALID_ARGUMENT);

  // 检查分配器是否有效
  // Check if allocator is valid
  RCUTILS_CHECK_ALLOCATOR(&allocator, return RMW_RET_INVALID_ARGUMENT);

  // 检查 init_options 的实现标识符是否为 NULL
  // Check if init_options' implementation_identifier is NULL
  if (NULL != init_options->implementation_identifier) {
    RMW_SET_ERROR_MSG("expected zero-initialized init_options");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 初始化 init_options 的各个字段
  // Initialize fields of init_options
  init_options->instance_id = 0;
  init_options->implementation_identifier = identifier;
  init_options->allocator = allocator;
  init_options->impl = nullptr;
  init_options->enclave = NULL;
  init_options->domain_id = RMW_DEFAULT_DOMAIN_ID;
  init_options->security_options = rmw_get_default_security_options();
  init_options->localhost_only = RMW_LOCALHOST_ONLY_DEFAULT;
  init_options->discovery_options = rmw_get_zero_initialized_discovery_options();

  // 初始化发现选项
  // Initialize discovery options
  return rmw_discovery_options_init(&(init_options->discovery_options), 0, &allocator);
}

/**
 * @brief 复制 rmw_init_options_t 结构体
 * Copy the rmw_init_options_t structure
 *
 * @param[in] identifier 实现的标识符
 * The implementation identifier
 * @param[in] src 源 rmw_init_options_t 结构体指针
 * Pointer to the source rmw_init_options_t structure
 * @param[out] dst 目标 rmw_init_options_t 结构体指针
 * Pointer to the destination rmw_init_options_t structure
 * @return RMW_RET_OK if successful, appropriate error code otherwise
 */
rmw_ret_t rmw_init_options_copy(
    const char* identifier, const rmw_init_options_t* src, rmw_init_options_t* dst) {
  // 检查 identifier 是否为 NULL
  // Check if identifier is NULL
  assert(identifier != NULL);

  // 检查源和目标结构体是否为 NULL
  // Check if source and destination structures are NULL
  RMW_CHECK_ARGUMENT_FOR_NULL(src, RMW_RET_INVALID_ARGUMENT);
  RMW_CHECK_ARGUMENT_FOR_NULL(dst, RMW_RET_INVALID_ARGUMENT);

  // 检查源结构体的实现标识符是否为 NULL
  // Check if source structure's implementation_identifier is NULL
  if (NULL == src->implementation_identifier) {
    RMW_SET_ERROR_MSG("expected initialized dst");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 检查源和目标结构体的实现标识符是否匹配
  // Check if source and destination structures' implementation_identifiers match
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      src, src->implementation_identifier, identifier, return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 检查目标结构体的实现标识符是否为 NULL
  // Check if destination structure's implementation_identifier is NULL
  if (NULL != dst->implementation_identifier) {
    RMW_SET_ERROR_MSG("expected zero-initialized dst");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 获取分配器并检查其有效性
  // Get allocator and check its validity
  rcutils_allocator_t allocator = src->allocator;
  RCUTILS_CHECK_ALLOCATOR(&allocator, return RMW_RET_INVALID_ARGUMENT);

  // 创建临时 rmw_init_options_t 结构体并复制源结构体的内容
  // Create a temporary rmw_init_options_t structure and copy the contents of the source structure
  rmw_init_options_t tmp = *src;
  tmp.enclave = rcutils_strdup(tmp.enclave, allocator);
  if (NULL != src->enclave && NULL == tmp.enclave) {
    return RMW_RET_BAD_ALLOC;
  }

  // 初始化安全选项并复制源结构体的安全选项
  // Initialize security options and copy security options from the source structure
  tmp.security_options = rmw_get_zero_initialized_security_options();
  rmw_ret_t ret =
      rmw_security_options_copy(&src->security_options, &allocator, &tmp.security_options);
  if (RMW_RET_OK != ret) {
    allocator.deallocate(tmp.enclave, allocator.state);
    return ret;
  }

  // 初始化发现选项并复制源结构体的发现选项
  // Initialize discovery options and copy discovery options from the source structure
  tmp.discovery_options = rmw_get_zero_initialized_discovery_options();
  ret = rmw_discovery_options_copy(&src->discovery_options, &allocator, &tmp.discovery_options);

  // 将临时结构体赋值给目标结构体
  // Assign the temporary structure to the destination structure
  *dst = tmp;

  // 返回成功状态
  // Return success status
  return RMW_RET_OK;
}

/**
 * @brief 释放初始化选项资源 (Release the resources of the initialization options)
 *
 * @param[in] identifier 实现标识符 (Implementation identifier)
 * @param[out] init_options 初始化选项指针 (Pointer to the initialization options)
 * @return rmw_ret_t 返回操作结果 (Return the result of the operation)
 */
rmw_ret_t rmw_init_options_fini(const char* identifier, rmw_init_options_t* init_options) {
  // 断言：确保标识符不为空 (Assert: Ensure the identifier is not NULL)
  assert(identifier != NULL);

  // 检查 init_options 是否为空，如果为空则返回无效参数错误 (Check if init_options is NULL, return
  // invalid argument error if it is)
  RMW_CHECK_ARGUMENT_FOR_NULL(init_options, RMW_RET_INVALID_ARGUMENT);

  // 如果 init_options 的实现标识符为空，则设置错误消息并返回无效参数错误 (If the implementation
  // identifier of init_options is NULL, set the error message and return an invalid argument error)
  if (NULL == init_options->implementation_identifier) {
    RMW_SET_ERROR_MSG("expected initialized init_options");
    return RMW_RET_INVALID_ARGUMENT;
  }

  // 检查类型标识符是否匹配，如果不匹配则返回错误的 RMW 实现 (Check if type identifiers match,
  // return incorrect RMW implementation if they don't)
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
      init_options, init_options->implementation_identifier, identifier,
      return RMW_RET_INCORRECT_RMW_IMPLEMENTATION);

  // 获取分配器指针 (Get the allocator pointer)
  rcutils_allocator_t* allocator = &init_options->allocator;

  // 检查分配器是否有效，如果无效则返回无效参数错误 (Check if the allocator is valid, return invalid
  // argument error if it's not)
  RCUTILS_CHECK_ALLOCATOR(allocator, return RMW_RET_INVALID_ARGUMENT);

  // 使用分配器释放 enclave 资源 (Deallocate the enclave resource using the allocator)
  allocator->deallocate(init_options->enclave, allocator->state);

  // 释放安全选项资源，并获取操作结果 (Release the security options resources and get the operation
  // result)
  rmw_ret_t ret = rmw_security_options_fini(&init_options->security_options, allocator);

  // 如果操作结果不是 RMW_RET_OK，则返回操作结果 (If the operation result is not RMW_RET_OK, return
  // the operation result)
  if (ret != RMW_RET_OK) {
    return ret;
  }

  // 释放发现选项资源，并获取操作结果 (Release the discovery options resources and get the operation
  // result)
  ret = rmw_discovery_options_fini(&init_options->discovery_options);

  // 将 init_options 设置为零初始化的初始化选项 (Set init_options to zero-initialized initialization
  // options)
  *init_options = rmw_get_zero_initialized_init_options();

  // 返回操作结果 (Return the operation result)
  return ret;
}

}  // namespace rmw_fastrtps_shared_cpp
