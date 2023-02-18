/// This file is part of the Veil distribution (https://github.com/Alphaharrius/Veil).
/// Copyright (c) 2023 Alphaharrius.
///
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, version 3.
///
/// This program is distributed in the hope that it will be useful, but
/// WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
/// General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <http://www.gnu.org/licenses/>.

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

#include <windows.h>

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)

#include <pthread.h>

#endif

#include "src/threading/os.hpp"
#include "src/vm/structures.hpp"
#include "src/os.hpp"

using namespace veil::os;

void OSThread::sleep(uint32 milliseconds) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    Sleep(milliseconds);
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#endif
}

OSThread::OSThread() : os_thread(nullptr) {}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

DWORD WINAPI win32_thread_function(void *params) {
    auto *callable = (veil::vm::Callable *) params;
    callable->run();
    return 0; // We will not use this return value.
}

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)

void *pthread_thread_function(void *params) {
    auto *callable = (veil::vm::Callable *) params;
    callable->run();
    return nullptr; // We will not use this return value.
}

#endif

void OSThread::start(vm::Callable &callable, uint32 &error) {
    error = veil::ERR_NONE;
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    LPDWORD _;
    this->os_thread = CreateThread(nullptr,
                                   0,
                                   win32_thread_function,
                                   &callable,
                                   0,
                                   _);
    if (!this->os_thread) {
        switch (GetLastError()) {
        case ERROR_NOT_ENOUGH_MEMORY: {
            error = threading::ERR_NO_RES;
            return;
        }
        default: {
            std::string filename = __FILE__;
            std::string function_name = __func__;
            os::force_exit_on_error("Invalid state of Win32 error.", filename, function_name, __LINE__);
        }
        }
    }
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    int pthread_error = pthread_create((pthread_t *) this->os_thread, nullptr, &pthread_thread_function, nullptr);
    switch (pthread_error) {
    case 0: return;
    case EAGAIN: {
        error = threading::ERR_NO_RES;
        return;
    }
    default: {
        std::string filename = __FILE__;
        std::string function_name = __func__;
        os::force_exit_on_error("Invalid state of pthread error.", filename, function_name, __LINE__);
    }
    }
#   endif
}

void OSThread::join(uint32 &error) {
    error = veil::ERR_NONE;
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    DWORD stat = WaitForSingleObject(this->os_thread, INFINITE);
    // NOTE: The error code of this method is not clear, thus we will use \c threading::ERR_INV_JOIN as a failed status.
    if (stat == WAIT_FAILED) {
        error = threading::ERR_INV_JOIN;
        return;
    }
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    int pthread_error = pthread_join((pthread_t) this->os_thread, nullptr);
    switch (pthread_error) {
    case 0: return;
    case EDEADLK:
        error = threading::ERR_DEADLOCK;
        return;
    case EINVAL:
        error = threading::ERR_INV_JOIN;
        return;
    default: {
        std::string filename = __FILE__;
        std::string function_name = __func__;
        os::force_exit_on_error("Invalid state of pthread error.", filename, function_name, __LINE__);
    }
    }
#   endif
}
