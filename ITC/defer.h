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
#ifndef MBED_ITC_DEFER_H_
#define MBED_ITC_DEFER_H_

#include <Expected.h>
#include <TaskQueue.h>
#include <core-util/FunctionPointerBind.h>
#include <core-util/FunctionPointer.h>

#include <stdio.h>

#define WAKEUP_SIGNAL_ID 0x00000001

namespace itc {
namespace detail {

template<typename ReturnType>
void deferred_call(mbed::util::FunctionPointerBind<ReturnType>* function, ReturnType* return_value, osThreadId waiting_thread) {
    *return_value = function->call();
    osSignalSet(waiting_thread, WAKEUP_SIGNAL_ID);
}

inline void deferred_call(mbed::util::FunctionPointerBind<void>* function, osThreadId waiting_thread) {
    function->call();
    osSignalSet(waiting_thread, WAKEUP_SIGNAL_ID);
}

} // namespace detail

template<typename ReturnType, std::size_t ElementCount>
Expected<ReturnType, bool> defer_call(TaskQueue<ElementCount>& executor, mbed::util::FunctionPointerBind<ReturnType> f) {
    osThreadId this_thread = osThreadGetId();
    osSignalClear(this_thread, WAKEUP_SIGNAL_ID);
    ReturnType result;

    bool defer_res = executor.post(
        mbed::util::FunctionPointer3<void, mbed::util::FunctionPointerBind<ReturnType>*, ReturnType*, osThreadId>(
            &detail::deferred_call<ReturnType>
        ).bind(
            &f,
            &result,
            this_thread
        )
    );

    if(!defer_res) {
        return makeUnexpectedError(false);
    }

    // call transmited to the other thread, just wait for the result
    osSignalWait(WAKEUP_SIGNAL_ID, osWaitForever);
    osSignalClear(this_thread, WAKEUP_SIGNAL_ID);

    return result;
}


template<std::size_t ElementCount>
Expected<void, bool> defer_call(TaskQueue<ElementCount>& executor, mbed::util::FunctionPointerBind<void> f) {
    osThreadId this_thread = osThreadGetId();
    osSignalClear(this_thread, WAKEUP_SIGNAL_ID);

    bool defer_res = executor.post(
        mbed::util::FunctionPointer2<void, mbed::util::FunctionPointerBind<void>*, osThreadId>(
            &detail::deferred_call
        ).bind(
            &f,
            this_thread
        )
    );

    if(!defer_res) {
        return makeUnexpectedError(false);
    }

    // call transmited to the other thread, just wait for the result
    osSignalWait(WAKEUP_SIGNAL_ID, osWaitForever);
    osSignalClear(this_thread, WAKEUP_SIGNAL_ID);

    return Expected<void, bool>();
}

} // namespace itc

#endif
