// Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef RMW_FASTRTPS_SHARED_CPP__LOCKED_OBJECT_HPP_
#define RMW_FASTRTPS_SHARED_CPP__LOCKED_OBJECT_HPP_

#include <mutex>

#include "rcpputils/thread_safety_annotations.hpp"

// clang-format off
/* markdown
  > [!NOTE]
  > `ros/rmw_locked_object_*_.md`
  > 这里提供一个模板在多线程调用，访问某一个对象的时候提供线程安全的能力，即通过mutex加锁
  > 另外，这里提供了对 `operator()` 的重载，目的是可以在使用这个功能的时候，直接通过
  `my_locked_object().some_operation();` 的方式进行访问，因为重载了 `()`，这里使用 `my_*()` 类似于提供了一个 `getter` 的方法，使用更方便得多。
  >  ```cpp
  >  LockedObject<MyClass> my_locked_object;
  >  // 对象访问
  >  {
  >    std::lock_guard<std::mutex> lock(my_locked_object.getMutex()); // 对mutex加锁
  >    my_locked_object().some_operation(); // 使用重载操作符()访问object_并调用操作
  >  }
  >  ```
*/
// clang-format on

/**
 * @brief 一个用于保护对象访问的模板类 (A template class for protecting object access)
 *
 * @tparam T 被保护的对象类型 (The type of the object being protected)
 */
template <class T>
class LockedObject {
private:
  // 互斥锁，用于保护对 object_ 的访问 (Mutex for protecting access to object_)
  mutable std::mutex mutex_;

  // 被保护的对象 (The protected object)
  T object_ RCPPUTILS_TSA_GUARDED_BY(mutex_);

public:
  /**
   * @brief 获取用于锁定此对象的互斥锁引用 (Get a reference to the mutex used to lock this object)
   *
   * @return 对互斥锁的引用 (A reference to the mutex)
   */
  std::mutex& getMutex() const RCPPUTILS_TSA_RETURN_CAPABILITY(mutex_) { return mutex_; }

  /**
   * @brief 重载括号运算符，返回非常量对象引用 (Overload the parenthesis operator, returning a
   * non-const object reference)
   *
   * @return 对被保护对象的非常量引用 (A non-const reference to the protected object)
   */
  T& operator()() { return object_; }

  /**
   * @brief 重载括号运算符，返回常量对象引用 (Overload the parenthesis operator, returning a const
   * object reference)
   *
   * @return 对被保护对象的常量引用 (A const reference to the protected object)
   */
  const T& operator()() const { return object_; }
};

#endif  // RMW_FASTRTPS_SHARED_CPP__LOCKED_OBJECT_HPP_
