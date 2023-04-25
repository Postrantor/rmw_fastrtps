// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#ifndef RMW_FASTRTPS_DYNAMIC_CPP__MACROS_HPP_
#define RMW_FASTRTPS_DYNAMIC_CPP__MACROS_HPP_

#include <limits>
#include <string>

// clang-format off
/*!
 * \brief 为 C 类型的序列定义一个通用的结构体模板，并提供初始化和销毁函数。
 *        Define a generic structure template for C type sequences, and provide initialization and destruction functions.
 * \param C_NAME 序列的名称。The name of the sequence.
 * \param C_TYPE 序列中元素的类型。The type of elements in the sequence.
 */
#define SPECIALIZE_GENERIC_C_SEQUENCE(C_NAME, C_TYPE)                                      \
  /*!                                                                                      \
   * \brief 特化 GenericCSequence 结构体模板，使其适用于特定的 C 类型序列。                  \
   *        Specialize the GenericCSequence structure template to make it applicable to a specific C type sequence. \
   */                                                                                      \
  template <>                                                                              \
  struct GenericCSequence<C_TYPE> {                                                        \
    /*!                                                                                    \
     * \brief 使用 rosidl_runtime_c 中的相应序列类型作为此特化结构体的类型。                \
     *        Use the corresponding sequence type in rosidl_runtime_c as the type for this specialized structure. \
     */                                                                                    \
    using type = rosidl_runtime_c__##C_NAME##__Sequence;                                   \
                                                                                           \
    /*!                                                                                    \
     * \brief 销毁指定的序列。                                                               \
     *        Destroy the specified sequence.                                               \
     * \param[in] array 要销毁的序列。                                                       \
     *               The sequence to be destroyed.                                          \
     */                                                                                    \
    static void fini(type* array) { rosidl_runtime_c__##C_NAME##__Sequence__fini(array); } \
                                                                                           \
    /*!                                                                                    \
     * \brief 初始化指定的序列，并设置其大小。                                               \
     *        Initialize the specified sequence and set its size.                           \
     * \param[in,out] array 要初始化的序列。                                                 \
     *                 The sequence to be initialized.                                      \
     * \param[in] size 序列的大小。                                                         \
     *               The size of the sequence.                                              \
     * \return 如果成功初始化，则返回 true，否则返回 false。                                 \
     *         Return true if successfully initialized, otherwise return false.            \
     */                                                                                    \
    static bool init(type* array, size_t size) {                                           \
      return rosidl_runtime_c__##C_NAME##__Sequence__init(array, size);                    \
    }                                                                                      \
  };

#endif  // RMW_FASTRTPS_DYNAMIC_CPP__MACROS_HPP_
