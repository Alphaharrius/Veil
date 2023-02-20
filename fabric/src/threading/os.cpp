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

void Thread::sleep(uint32 milliseconds) {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-sleep
    Sleep(milliseconds);
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://man7.org/linux/man-pages/man3/usleep.3.html
    // This method sleep a period with precision of microseconds, according to the documentation this method will fail
    // if the host os doesn't support a sleep period of more than 1000000ms.
    uint32 microseconds = milliseconds * 1000;
    if (::usleep(microseconds) != 0 && errno == EINVAL) {
        uint32 loop_count = microseconds / 1000000;
        while (loop_count--) {
            ::usleep(1000000);
        }
        ::usleep(microseconds % 1000000);
    }
#endif
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

/// Since pthread_t is a structure contains necessary attributes for instantiation, but pthread_create will not allocate
/// the necessary memory for this structure, thus we have to wrap this object (follow strictly to the VM coding rules)
/// inorder to allocate the memory before the init procedure.
struct PThreadStruct : veil::memory::HeapObject {
    pthread_t embedded;
};

void *pthread_thread_function(void *params) {
    auto *callable = (veil::vm::Callable *) params;
    callable->run();
    return nullptr; // We will not use this return value.
}

#endif

Thread::Thread() : os_thread(nullptr), status(Status::Idle) {}

Thread::~Thread() {
#   if defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Free the memory allocated for the pthread_t as a thread index.
    delete ((PThreadStruct *) this->os_thread);
#   endif
}

void Thread::start(vm::Callable &callable, uint32 &error) {
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
    // segmentation fault. This piece of memory will not be freed even after this Thread is joined as it can be
    // reused, this will be freed in the destructor.
    auto *pts = (PThreadStruct *) this->os_thread;
    if (pts == nullptr)
        pts = new PThreadStruct();

    if (pthread_create(&pts->embedded, nullptr, &pthread_thread_function, &callable)) {
        switch (errno) {
        case EAGAIN: {
            error = threading::ERR_NO_RES;
            return;
        }
        default: VeilForceExitOnError("Invalid state of pthread error.");
        }
    }
    this->os_thread = pts;
#   endif
    this->status = Status::Started;
}

void Thread::join(uint32 &error) {
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
    VeilForceExitOnError(message);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://man7.org/linux/man-pages/man3/pthread_join.3.html
    auto *pts = (PThreadStruct *) this->os_thread;
    if (pthread_join(pts->embedded, nullptr)) {
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

Thread::Status Thread::get_status() {
    return this->status;
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

struct Win32MutexStruct : veil::memory::HeapObject {
    CRITICAL_SECTION embedded;
};

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)

struct PThreadMutexStruct : veil::memory::HeapObject {
    pthread_mutex_t embedded;
};

#endif

Mutex::Mutex() : os_mutex(nullptr) {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // In Win32, the representation of mutex is a critical section, the mutex defined in the Win32 API is a construct
    // able to share between processes, which is overkill and less performant.

    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializecriticalsectionandspincount
    // https://learn.microsoft.com/en-us/windows/win32/sync/using-critical-section-objects
    auto *m = new Win32MutexStruct();
    if (!InitializeCriticalSectionAndSpinCount(&m->embedded, threading::config::mutex_spin_count)) {
        // According to source: Starting with Windows Vista, the InitializeCriticalSectionAndSpinCount function always
        // succeeds, even in low memory situations.
        char message[64];
        ::sprintf(message,
                  "InitializeCriticalSectionAndSpinCount failed on error code (%d).",
                  (int) GetLastError());
        VeilForceExitOnError(message);
    }
    this->os_mutex = m;
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/007904875/functions/pthread_mutex_init.html

    // Allocate the memory for pthread_mutex_t structure before mutex initialization, or will result in segmentation
    // fault, this memory will be freed in destructor.
    auto *pms = new PThreadMutexStruct();
    if (pthread_mutex_init(&pms->embedded, nullptr)) {
        switch (errno) {
        case EAGAIN:
        case ENOMEM:
            VeilForceExitOnError("Insufficient resources to create a mutex.");
        case EPERM:
            VeilForceExitOnError("Owner of the VM process have no permission to create a mutex.");
        case EBUSY:
        case EINVAL:
        default:
            VeilExitOnImplementationFault("Should not reach here.");
        }
    }
    this->os_mutex = pms;
#   endif
}

Mutex::~Mutex() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/sync/using-critical-section-objects
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-deletecriticalsection
    auto *ms = (Win32MutexStruct *) this->os_mutex;
    DeleteCriticalSection(&ms->embedded);
    delete ((Win32MutexStruct *) this->os_mutex);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/007904875/functions/pthread_mutex_destroy.html
    auto *pms = (PThreadMutexStruct *) this->os_mutex;
    if (pthread_mutex_destroy(&pms->embedded)) {
        switch (errno) {
        case EBUSY:
            // This case will be raised as critical error as all mutex usage should be properly handled in the higher
            // abstraction level.
            VeilExitOnImplementationFault("Attempt to destroy a locked mutex.");
        case EINVAL: // The mutex is always initialized as it is being done in the constructor.
        default:
            VeilForceExitOnError("Should not reach here.");
        }
    }
    delete pms;
#   endif
}

void Mutex::lock() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/sync/using-critical-section-objects
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-entercriticalsection
    auto *ms = (Win32MutexStruct *) this->os_mutex;
    EnterCriticalSection(&ms->embedded);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutex_lock.html
    auto *pms = (PThreadMutexStruct *) this->os_mutex;
    if (pthread_mutex_lock(&pms->embedded)) {
        switch (errno) {
        case EDEADLK:
            VeilExitOnImplementationFault("Attempt to lock a owned mutex.");
        case EAGAIN:
            VeilForceExitOnError("The maximum number of recursive locks for mutex has been exceeded.");
        case EINVAL:
        default:
            VeilExitOnImplementationFault("Should not reach here.");
        }
    }
#   endif
}

void Mutex::unlock() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/sync/using-critical-section-objects
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-leavecriticalsection
    auto *ms = (Win32MutexStruct *) this->os_mutex;
    LeaveCriticalSection(&ms->embedded);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutex_unlock.html
    auto *pms = (PThreadMutexStruct *) this->os_mutex;
    if (pthread_mutex_unlock(&pms->embedded)) {
        switch (errno) {
        case EPERM:
            VeilExitOnImplementationFault("Attempt to unlock a mutex without a prior lock on it.");
        case EAGAIN:
            VeilForceExitOnError(
                    "The mutex could not be unlocked because the maximum number of recursive locks for mutex has been "
                    "exceeded.");
        case EINVAL:
        default:
            VeilExitOnImplementationFault("Should not reach here.");
        }
    }
#   endif
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

struct Win32ConVarStruct : veil::memory::HeapObject {
    /// \c CONDITION_VARIABLE is a structure with a single attribute of type ( \c void \c* ).
    CONDITION_VARIABLE embedded;
};

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)

#endif

ConditionVariable::ConditionVariable() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/sync/using-condition-variables
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializeconditionvariable
    this->os_cv = new Win32ConVarStruct();
    InitializeConditionVariable((PCONDITION_VARIABLE) this->os_cv);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   endif
}

ConditionVariable::~ConditionVariable() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Win32 condition variable does not need to be destroyed, thus we free the memory directly.
    delete ((Win32ConVarStruct *) this->os_cv);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   endif
}

void ConditionVariable::wait() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/sync/using-condition-variables
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-sleepconditionvariablecs

    // To ensure the condition variable to be checked atomically, we have to lock the associated mutex.
    associate.lock();
    PCONDITION_VARIABLE cv = &((Win32ConVarStruct *) this->os_cv)->embedded;
    PCRITICAL_SECTION cs = &((Win32MutexStruct *) associate.os_mutex)->embedded;
    BOOL success = SleepConditionVariableCS(cv, cs, INFINITE);
    associate.unlock();
    if (!success) {
        char message[64];
        ::sprintf(message, "SleepConditionVariableCS failed on error code (%d).", (int) GetLastError());
        VeilForceExitOnError(message);
    }
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
#   endif
}
