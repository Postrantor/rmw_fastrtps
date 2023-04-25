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

#include "type_support_common.hpp"

#include "rmw/error_handling.h"
#include "rmw_fastrtps_dynamic_cpp/TypeSupport.hpp"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"

/**
 * @brief 判断是否使用 introspection C 类型支持 (Determine if using introspection C typesupport)
 *
 * @param typesupport_identifier 类型支持标识符 (Type support identifier)
 * @return true 如果使用 introspection C 类型支持 (If using introspection C typesupport)
 * @return false 如果不使用 introspection C 类型支持 (If not using introspection C typesupport)
 */
bool using_introspection_c_typesupport(const char* typesupport_identifier) {
  // 比较类型支持标识符与 rosidl_typesupport_introspection_c__identifier 是否相等
  // (Compare the type support identifier with rosidl_typesupport_introspection_c__identifier for
  // equality)
  return typesupport_identifier == rosidl_typesupport_introspection_c__identifier;
}

/**
 * @brief 判断是否使用 introspection C++ 类型支持 (Determine if using introspection C++ typesupport)
 *
 * @param typesupport_identifier 类型支持标识符 (Type support identifier)
 * @return true 如果使用 introspection C++ 类型支持 (If using introspection C++ typesupport)
 * @return false 如果不使用 introspection C++ 类型支持 (If not using introspection C++ typesupport)
 */
bool using_introspection_cpp_typesupport(const char* typesupport_identifier) {
  // 比较类型支持标识符与 rosidl_typesupport_introspection_cpp::typesupport_identifier 是否相等
  // (Compare the type support identifier with
  // rosidl_typesupport_introspection_cpp::typesupport_identifier for equality)
  return typesupport_identifier == rosidl_typesupport_introspection_cpp::typesupport_identifier;
}
