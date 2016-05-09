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
#ifndef MBED_UTIL_EXPECTED_H_
#define MBED_UTIL_EXPECTED_H_

#include <mbed_interface.h>
#include "detail/AlignedStorage.h"
#include "detail/traits.h"

namespace itc {

/**
 * Simple container for unexpected errors, it is not expected that the user
 * directly use this class.
 */
template<typename E>
struct UnexpectedError {
    UnexpectedError(const E& err) : _error(err) { }

    const E& get() const {
        return _error;
    }

    E& get() {
        return _error;
    }

private:
    E _error;
};


template<>
struct UnexpectedError<void> {
    UnexpectedError() { }
};

/**
 * Constrcut an unexpected error, this factory function is intended to be
 * used as the constructor of Error for Expected<T, E>
 */
template<typename E>
UnexpectedError<E> makeUnexpectedError(const E& err) {
    return UnexpectedError<E>(err);
}

UnexpectedError<void> makeUnexpectedError() {
    return UnexpectedError<void>();
}

template<typename T, typename E>
class Expected {

   typedef typename detail::conditional<(sizeof(T) > sizeof(E)), T, E>::type biggest_type_t;

public:
    Expected(const T& val) : _storage(), _status(true) {
        new(_storage.get_storage()) T(val);
    }

    Expected(const UnexpectedError<E>& err) : _storage(), _status(false) {
        new(_storage.get_storage()) E(err.get());
    }

    Expected(const Expected& other) : _storage(), _status(other._status) {
        if(other.has_value()) {
            new(_storage.get_storage()) T(other.value());
        } else {
            new(_storage.get_storage()) E(other.error());
        }
    }

    ~Expected() {
        if(has_value()) {
            static_cast<T*>(_storage.get_storage())->~T();
        } else {
            static_cast<E*>(_storage.get_storage())->~E();
        }
    }

    Expected& operator=(const Expected& other) {
        if(has_value() && other.has_value()) {
            value() = other.value();
        } else if(has_error() && other.has_error()) {
            error() = other.error();
        } else {
            if(has_value()) {
                static_cast<T*>(_storage.get_storage())->~T();
            } else {
                static_cast<E*>(_storage.get_storage())->~E();
            }

            _status = other._status;

            if(other.has_value()) {
                new(_storage.get_storage()) T(other.value());
            } else {
                new(_storage.get_storage()) E(other.error());
            }
        }
        return *this;
    }

    bool has_error() const {
        return _status == false;
    }

    bool has_value() const {
        return _status == true;
    }

    T& value() {
        if(has_error()) {
            mbed_die();
        }

        return *(static_cast<T*>(_storage.get_storage()));
    }

    const T& value() const {
        if(has_error()) {
            mbed_die();
        }

        return *(static_cast<const T*>(_storage.get_storage()));
    }

    E& error() {
        if(has_value()) {
            // undefined behavior (same as deferencing a NULL ptr), just die.
            mbed_die();
        }

        return *(static_cast<E*>(_storage.get_storage()));
    }

    const E& error() const {
        if(has_value()) {
            mbed_die();
        }

        return *(static_cast<const E*>(_storage.get_storage()));
    }

private:
    detail::AlignedStorage<biggest_type_t> _storage;
    bool _status;
};

template<typename E>
class Expected<void, E> {

public:
    Expected() : _storage(), _status(true) {
    }

    Expected(const UnexpectedError<E>& err) : _storage(), _status(false) {
        new(_storage.get_storage()) E(err.get());
    }

    Expected(const Expected& other) : _storage(), _status(other._status) {
        if(other.has_error()) {
            new(_storage.get_storage()) E(other.error());
        }
    }

    ~Expected() {
        if(has_error()) {
            static_cast<E*>(_storage.get_storage())->~E();
        }
    }

    Expected& operator=(const Expected& other) {
        if(has_error() && other.has_error()) {
            error() = other.error();
        } else {
            if(has_error()) {
                static_cast<E*>(_storage.get_storage())->~E();
            } else {
                new(_storage.get_storage()) E(other.error());
            }
            _status = other._status;
        }
        return *this;
    }

    bool has_error() const {
        return _status == false;
    }

    E& error() {
        if(has_error() == false) {
            // undefined behavior (same as deferencing a NULL ptr), just die.
            mbed_die();
        }

        return *static_cast<E*>(_storage.get_storage());
    }

    const E& error() const {
        if(has_error() == false) {
            mbed_die();
        }

        return *static_cast<const E*>(_storage.get_storage());
    }

private:
    detail::AlignedStorage<E> _storage;
    bool _status;
};

} // namespace itc

#endif /* MBED_UTIL_EXPECTED_H_ */
