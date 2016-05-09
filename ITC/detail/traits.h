/* mbed Microcontroller Library
 * Copyright (c) 2006-2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef ITC_DETAIL_TRAITS_H_
#define ITC_DETAIL_TRAITS_H_

namespace itc {
namespace detail {

template<bool B, typename T, typename F>
struct conditional {
    typedef T type;
};

template<typename T, typename F>
struct conditional<false, T, F> {
    typedef F type;
};


template<bool B, typename ReturnType = void>
struct enable_if;

template<typename ReturnType>
struct enable_if<true, ReturnType> {
    typedef ReturnType type;
};

} // namespace detail
} // namespace itc



#endif /* ITC_DETAIL_TRAITS_H_ */
