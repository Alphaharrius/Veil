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
#include <unistd.h>

#endif

#include "src/threading/os.hpp"
#include "src/vm/structures.hpp"
#include "src/vm/os.hpp"
#include "src/memory/os.hpp"
#include "src/threading/config.hpp"

using namespace veil::os;

void OSThread::sleep(uint32 milliseconds) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-sleep
    Sleep(milliseconds);
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://man7.org/linux/man-pages/man3/usleep.3.html
    // This method sleep a period with precision of microseconds, according to the documentation this method will fail
    // if the host os doesn't support a sleep period of more than 1000000ms.
    if (::usleep(milliseconds) != 0 && errno == EINVAL) {
        uint32 microseconds = milliseconds * 1000;
        uint32 loop_count = microseconds / 1000000;
        while (loop_count--) {
            ::usleep(1000000);
        }
        ::usleep(microseconds % 1000000);
    }
#endif
}

OSThread::OSThread() : os_thread(nullptr), status(Status::Idle) {}

OSThread::~OSThread() {
#   if defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Free the memory allocated for the pthread_t as a thread index.
    os::free((pthread_t *) this->os_thread);
#   endif
}

// Functions to be used for Win32 & pthread calls respectively, Win32 strictly requires the macro of WINAPI which refers
// to the __stdcall binary calling convention and a return value of DWORD; the requirement of pthread is simpler with a
// return value of (void *). In both cases we will use vm::Callable as a delegate encapsulation of the custom logic we
// would like to run in the thread.

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
    if (this->status == Status::Started)
        VeilExitOnImplementationFault("Starting a started thread.");
    if (this->status == Status::Joined)
        VeilExitOnImplementationFault("Starting a joined thread.");

    error = veil::ERR_NONE;
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createthread
    LPDWORD _;
    // HANDLE is a typedef from (void *).
    this->os_thread = CreateThread(nullptr, // Using default security attributes.
                                   0, // Using default stack size which is 1MB (Shall this be a parameter?).
                                   win32_thread_function,
                                   &callable,
                                   0, // Start the thread immediately.
                                   _); // We don't need the thread id.
    if (!this->os_thread) {
        switch (GetLastError()) {
        case ERROR_NOT_ENOUGH_MEMORY: {
            error = threading::ERR_NO_RES;
            return;
        }
        default:
            VeilForceExitOnError("Invalid state of Win32 error.");
        }
    }
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://man7.org/linux/man-pages/man3/pthread_create.3.html

    // Allocate the memory for a pthread_t which is an integer holding the thread index, else it will result in
    // segmentation fault. This piece of memory will not be freed even after this OSThread is joined as it can be
    // reused, this will be freed in the destructor.
    if (this->os_thread == nullptr)
        this->os_thread = os::malloc(sizeof(pthread_t));
    int os_status = pthread_create((pthread_t *) this->os_thread, nullptr, &pthread_thread_function, &callable);
    if (os_status) {
        switch (errno) {
        case EAGAIN: {
            error = threading::ERR_NO_RES;
            return;
        }
        default: VeilForceExitOnError("Invalid state of pthread error.");
        }
    }
#   endif
    this->status = Status::Started;
}

void OSThread::join(uint32 &error) {
    if (this->status != Status::Started)
        VeilExitOnImplementationFault("Thread joined before started.");

    error = veil::ERR_NONE;
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject
    DWORD os_status = WaitForSingleObject(this->os_thread, INFINITE);
    // NOTE: The error code of this method is not clear, thus we will use \c threading::ERR_INV_JOIN as a failed status.
    if (os_status == WAIT_FAILED) {
        error = threading::ERR_INV_JOIN;
        return;
    }
    this->status = Status::Joined;
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
    os_status = CloseHandle(this->os_thread);
    if (os_status) return;

    char message[64];
    ::sprintf(message, "CloseHandle failed on error code (%d).", (int) GetLastError());
    VeilForceExitOnError("CloseHandle failed on error code ().");
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://man7.org/linux/man-pages/man3/pthread_join.3.html
    int os_status = pthread_join((pthread_t) *((pthread_t *) this->os_thread), nullptr);
    if (os_status) {
        switch (errno) {
        case 0: return;
        case EDEADLK:error = threading::ERR_DEADLOCK;
            return;
        case EINVAL:error = threading::ERR_INV_JOIN;
            return;
        default: VeilForceExitOnError("Invalid state of pthread error.");
        }
    }
#   endif
    this->status = Status::Idle;
}

OSThread::Status OSThread::get_status() {
    return this->status;
}

OSMutex::OSMutex() : os_mutex(nullptr) {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createmutexa

    // HANDLE is a typedef from (void *)
    this->os_mutex = CreateMutexA(nullptr, false, nullptr);
    if (this->os_mutex != nullptr)
        return;

    DWORD error = GetLastError();
    switch (error) {
    case ERROR_INVALID_HANDLE:
    case ERROR_ALREADY_EXISTS:
        VeilExitOnImplementationFault("These two error should not happen if the mutex is unnamed");
    default:
        char message[64];
        ::sprintf(message, "CreateMutexA failed on error code (%d).", (int) error);
        VeilForceExitOnError(message);
    }
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/007904875/functions/pthread_mutex_init.html

    // Allocate the memory for pthread_mutex_t structure before mutex initialization, or will result in segmentation
    // fault, this memory will be freed in destructor.
    this->os_mutex = os::malloc(sizeof(pthread_mutex_t));
    int status = pthread_mutex_init((pthread_mutex_t *) this->os_mutex, nullptr);
    if (!status) return;

    switch (errno) {
    case EAGAIN:
    case ENOMEM: VeilForceExitOnError("Insufficient resources to create a mutex.");
    case EPERM: VeilForceExitOnError("Owner of the VM process have no permission to create a mutex.");
    case EBUSY:
    case EINVAL:
    default: VeilExitOnImplementationFault("Should not reach here.");
    }
#   endif
}

OSMutex::~OSMutex() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
    DWORD status = CloseHandle(this->os_mutex);
    if (status) return;

    DWORD error = GetLastError();
    switch (error) {
    // The mutex is always initialized as it is being done in the constructor.
    case ERROR_INVALID_HANDLE: VeilForceExitOnError("Should not reach here.");
    default:
        char message[64];
        ::sprintf(message, "CloseHandle failed on error code (%d).", (int) error);
        VeilForceExitOnError(message);
    }
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/007904875/functions/pthread_mutex_destroy.html
    int status = pthread_mutex_destroy((pthread_mutex_t *) this->os_mutex);
    os::free((pthread_mutex_t *) this->os_mutex);
    if (!status) return;

    switch (errno) {
    case EBUSY:
        // This case will be raised as critical error as all mutex usage should be properly handled in the higher
        // abstraction level.
        VeilExitOnImplementationFault("Attempt to destroy a locked mutex.");
    case EINVAL: // The mutex is always initialized as it is being done in the constructor.
    default: VeilForceExitOnError("Should not reach here.");
    }
#   endif
}

void OSMutex::lock() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    DWORD status = WaitForSingleObject(this->os_mutex, INFINITE);
    switch (status) {
    case WAIT_ABANDONED:
    case WAIT_OBJECT_0: return;
    case WAIT_TIMEOUT: VeilExitOnImplementationFault("Show not reach here as we wait indefinitely.");
    case WAIT_FAILED:
        // NOTE: It is worthy to investigate the set of error given here, but there lacks a function to convert DWORD to
        // (char *) at this moment.
        char message[64];
        ::sprintf(message, "WaitForSingleObject failed on error code (%d).", (int) GetLastError());
        VeilForceExitOnError(message);
    default: VeilExitOnImplementationFault("Show not reach here.");
    }
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutex_lock.html
    int status = pthread_mutex_lock((pthread_mutex_t *) this->os_mutex);
    if (!status) return;

    switch (errno) {
    case EDEADLK: VeilExitOnImplementationFault("Attempt to lock a owned mutex.");
    case EAGAIN: VeilForceExitOnError("The maximum number of recursive locks for mutex has been exceeded.");
    case EINVAL:
    default: VeilExitOnImplementationFault("Should not reach here.");
    }
#   endif
}

void OSMutex::unlock() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-releasemutex
    bool success = ReleaseMutex(this->os_mutex);
    if (success) return;

    char message[64];
    ::sprintf(message, "ReleaseMutex failed on error code (%d).", (int) GetLastError());
    VeilForceExitOnError(message);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutex_unlock.html
    int status = pthread_mutex_unlock((pthread_mutex_t *) this->os_mutex);
    if (!status) return;

    switch (errno) {
    case EPERM: VeilExitOnImplementationFault("Attempt to unlock a mutex without a prior lock on it.");
    case EAGAIN:VeilForceExitOnError(
                "The mutex could not be unlocked because the maximum number of recursive locks for mutex has been "
                "exceeded.");
    case EINVAL:
    default: VeilExitOnImplementationFault("Should not reach here.");
    }
#   endif
}
