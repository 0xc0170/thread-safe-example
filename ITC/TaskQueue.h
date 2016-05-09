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
#ifndef MBED_ITC_TASK_QUEUE_H_
#define MBED_ITC_TASK_QUEUE_H_

#include <cstddef>
#include <core-util/FunctionPointerBind.h>
#include <core-util/FunctionPointer.h>
#include <stdint.h>
#include <cmsis_os.h>

template<std::size_t ElementCount>
class TaskQueue {

public:
    typedef mbed::util::FunctionPointerBind<void> QueueElement_t;

    TaskQueue() :
        _os_mailQ_q(), _queue_id(0) {
        _os_mailQ_p[0] = _os_mailQ_q;
        _os_mailQ_p[1] = _os_mailQ_m;
        _queue_def.queue_sz = ElementCount;
        _queue_def.item_sz = sizeof(QueueElement_t);
        _queue_def.pool = _os_mailQ_p;
    }


    bool run() {
        if (_queue_id) {
            return false;
        }

        _queue_id = osMailCreate(&_queue_def, osThreadGetId());

        while(true) {
            osEvent ev = osMailGet(_queue_id, osWaitForever );

            switch (ev.status) {
                case osEventMail: {
                    // execute the function received
                    QueueElement_t* f = static_cast<QueueElement_t*>(ev.value.p);
                    f->call();

                    // release the mail
                    // TODO: what to do in case of error ()
                    /* osStatus err = */ osMailFree(_queue_id, ev.value.p);
                    break;
                }

                case osOK: //no mail is available in the queue and no timeout was specified
                case osEventTimeout: //no mail has arrived during the given timeout period
                case osErrorParameter: //a parameter is invalid or outside of a permitted range.
                default:
                    break;
            }
        }
    }

    bool post(const QueueElement_t& e, uint32_t millisec = osWaitForever) {
        if (!_queue_id) {
            return false;
        }

        void* storage = osMailAlloc(_queue_id, millisec);
        if (!storage) {
            return false;
        }

        QueueElement_t* obj = new(storage) QueueElement_t(e);
        if (osMailPut(_queue_id, storage) != osOK) {
            obj->~QueueElement_t();
            osMailFree(_queue_id, storage);
            return false;
        }

        return true;
    }

    bool post(void (*f)(), uint32_t millisec = osWaitForever) {
        return post(mbed::util::FunctionPointer0<void>(f).bind(), millisec);
    }

    template<typename T>
    bool post(T* instance, void (T::*f)(), uint32_t millisec = osWaitForever) {
        return post(mbed::util::FunctionPointer0<void>(instance, f).bind(), millisec);
    }

private:
    // this type is not copy constructible or copyable
    TaskQueue(const TaskQueue&);
    TaskQueue& operator=(const TaskQueue&);

    // storage for the queue, addapted from osMailQDef macro
    uint32_t _os_mailQ_q[4 + ElementCount];
    uint32_t _os_mailQ_m[3 + ((sizeof(QueueElement_t) + 3) / 4) * ElementCount];
    void* _os_mailQ_p[2];
    osMailQDef_t _queue_def;
    osMailQId _queue_id;
};

#endif
