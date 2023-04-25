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

#ifndef RMW_FASTRTPS_CPP__SERVICETYPESUPPORT_HPP_
#define RMW_FASTRTPS_CPP__SERVICETYPESUPPORT_HPP_

#include <cassert>

#include "TypeSupport.hpp"
#include "fastcdr/Cdr.h"
#include "fastcdr/FastBuffer.h"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"
#include "rosidl_typesupport_fastrtps_cpp/service_type_support.h"

namespace rmw_fastrtps_cpp {

/**
 * @class ServiceTypeSupport
 * @brief 服务类型支持基类 (Service Type Support base class)
 */
class ServiceTypeSupport : public TypeSupport {
protected:
  /**
   * @brief 构造函数 (Constructor)
   */
  ServiceTypeSupport();
};

/**
 * @class RequestTypeSupport
 * @brief 请求类型支持类，继承自 ServiceTypeSupport (Request Type Support class, inherits from
 * ServiceTypeSupport)
 */
class RequestTypeSupport : public ServiceTypeSupport {
public:
  /**
   * @brief 构造函数 (Constructor)
   * @param members 服务类型支持回调结构体指针 (Pointer to the service type support callbacks
   * structure)
   */
  explicit RequestTypeSupport(const service_type_support_callbacks_t* members);
};

/**
 * @class ResponseTypeSupport
 * @brief 响应类型支持类，继承自 ServiceTypeSupport (Response Type Support class, inherits from
 * ServiceTypeSupport)
 */
class ResponseTypeSupport : public ServiceTypeSupport {
public:
  /**
   * @brief 构造函数 (Constructor)
   * @param members 服务类型支持回调结构体指针 (Pointer to the service type support callbacks
   * structure)
   */
  explicit ResponseTypeSupport(const service_type_support_callbacks_t* members);
};

}  // namespace rmw_fastrtps_cpp

#endif  // RMW_FASTRTPS_CPP__SERVICETYPESUPPORT_HPP_
