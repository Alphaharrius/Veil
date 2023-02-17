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

#include "threading/os.hpp"

#include <thread>

using namespace veil::os;

void OSThread::sleep(uint32 milliseconds) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    Sleep(milliseconds);
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#endif
}

OSThread::OSThread() : os_thread(nullptr), os_thread_id(0) {}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
DWORD WINAPI win32_thread_function(void *params) {
    auto *callable = (Callable *) params;
    callable->run();
    return 0; // We will not use this return value.
}
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#endif

void OSThread::start(Callable &callable) {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    this->os_thread = CreateThread(nullptr,
                                   0,
                                   win32_thread_function,
                                   &callable,
                                   0,
                                   (LPDWORD) this->os_thread_id);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   endif
}

void OSThread::join() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    WaitForSingleObject(this->os_thread, INFINITE);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   endif
}
