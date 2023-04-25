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

#ifndef RMW_FASTRTPS_DYNAMIC_CPP__IDENTIFIER_HPP_
#define RMW_FASTRTPS_DYNAMIC_CPP__IDENTIFIER_HPP_

// 声明一个外部常量字符串 eprosima_fastrtps_identifier
// Declare an external constant string named eprosima_fastrtps_identifier
/* `extern const char* const` 是一个 C++
  修饰符组合，它的作用是声明一个外部的、常量指针，指向一个常量字符。我们可以逐个解释这些修饰符：

  1. `extern`:
  这个关键字表示该变量是在其他源文件中定义的，当前源文件只是引用它。这样，多个源文件可以共享同一个全局变量。

  2. `const char*`: 这是一个指向常量字符（`const char`）的指针。由于 `const` 修饰了
  `char`，所以指针指向的字符内容不能被修改。但是，指针本身可以改变，即可以指向其他常量字符。

  3. `const` (第二个): 这个 `const`
  修饰了指针本身，表示指针是一个常量，不能被修改。因此，这个指针既不能修改其指向的字符内容，也不能改变指向其他字符的地址。

  综上所述，`extern const char* const`
  声明了一个外部的、常量指针，指向一个常量字符。这意味着指针本身和它指向的字符都不能被修改。
*/
extern const char* const eprosima_fastrtps_identifier;

#endif  // RMW_FASTRTPS_DYNAMIC_CPP__IDENTIFIER_HPP_
