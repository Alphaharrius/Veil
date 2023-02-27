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
#include <cerrno>
#include <sys/time.h>

#endif

#include "src/threading/os.hpp"
#include "src/vm/structures.hpp"
#include "src/vm/diagnostics.hpp"
#include "src/memory/os.hpp"
#include "src/util/conversions.hpp"

using namespace veil::os;

void Thread::static_sleep(uint32 milliseconds) {
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

uint64 Thread::current_thread_id() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-sleep
    return (uint64) GetCurrentThreadId();
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://man7.org/linux/man-pages/man3/pthread_self.3.html
    return (uint64) pthread_self();
#endif
}

// Functions to be used for Win32 & pthread calls respectively, Win32 strictly requires the macro of WINAPI which refers
// to the __stdcall binary calling convention and a return value of DWORD; the requirement of pthread is simpler with a
// return value of (void *). In both cases we will use vm::Executable as a delegate encapsulation of the custom logic we
// would like to run in the thread.

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

/// This is used to store both the thread handle and the thread id.
struct Win32ThreadStruct {
    HANDLE embedded;
    DWORD id;
};

DWORD WINAPI win32_thread_function(void *params) {
    auto *executable = (veil::vm::Executable *) params;
    executable->execute();
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
    auto *executable = (veil::vm::Executable *) params;
    executable->execute();
    return nullptr; // We will not use this return value.
}

#endif

Thread::Thread() : native_struct(nullptr), started(false) {}

Thread::~Thread() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    delete ((Win32ThreadStruct *) this->native_struct);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    delete ((PThreadStruct *) this->native_struct);
#   endif
}

void Thread::start(vm::Executable &callable) {
    if (this->started)
        veil::implementation_fault("Starting a started thread.", VeilGetLineInfo);

#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createthread
    auto *wts = (Win32ThreadStruct *) this->native_struct;
    if (wts == nullptr)
        wts = new Win32ThreadStruct();

    // HANDLE is a typedef from (void *).
    void *thread = CreateThread(nullptr, // Using default security attributes.
                                0, // Using default stack size which is 1MB (Shall this be a parameter?).
                                win32_thread_function,
                                &callable,
                                0, // Start the thread immediately.
                                (LPDWORD) &wts->id);
    if (thread == nullptr) {
        switch (GetLastError()) {
        case ERROR_NOT_ENOUGH_MEMORY:
            veil::force_exit_on_error("Insufficient resources to create another thread.", VeilGetLineInfo);
        default:
            veil::force_exit_on_error("Invalid state of Win32 error.", VeilGetLineInfo);
        }
    }

    wts->embedded = thread;
    this->native_struct = wts;
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://man7.org/linux/man-pages/man3/pthread_create.3.html

    // Allocate the memory for a pthread_t which is an integer holding the thread index, else it will result in
    // segmentation fault. This piece of memory will not be freed even after this Thread is joined as it can be
    // reused, this will be freed in the destructor.
    auto *pts = (PThreadStruct *) this->native_struct;
    if (pts == nullptr)
        pts = new PThreadStruct();

    if (pthread_create(&pts->embedded, nullptr, &pthread_thread_function, &callable)) {
        int err = errno;
        if (err == EAGAIN)
            veil::force_exit_on_error("Insufficient resources to create another thread.", VeilGetLineInfo);
        else
            veil::force_exit_on_error("Invalid state of pthread error :: " + std::to_string(err), VeilGetLineInfo);
    }
    this->native_struct = pts;
#   endif
    this->started = true;
}

void Thread::join() {
    if (!this->started)
        veil::implementation_fault("Thread joined before started.", VeilGetLineInfo);

#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject
    auto *wts = (Win32ThreadStruct *) this->native_struct;
    // NOTE: The error code of this method is not clear.
    if (WaitForSingleObject(wts->embedded, INFINITE) == WAIT_FAILED)
        veil::force_exit_on_error("Failed to join thread :: " + std::to_string(GetLastError()), VeilGetLineInfo);

    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
    if (!CloseHandle(wts->embedded)) {
        veil::force_exit_on_error("CloseHandle failed on error code :: " + std::to_string(GetLastError()),
                                  VeilGetLineInfo);
    }
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://man7.org/linux/man-pages/man3/pthread_join.3.html
    auto *pts = (PThreadStruct *) this->native_struct;
    if (pthread_join(pts->embedded, nullptr)) {
        int err = errno;
        switch (err) {
        case 0:
            return;
        case EDEADLK:
            veil::implementation_fault("Deadlock detected on thread join.", VeilGetLineInfo);
        case EINVAL:
            veil::implementation_fault("Another thread is waiting to join with this thread.", VeilGetLineInfo);
        default:
            veil::force_exit_on_error("Invalid state of pthread error :: " + std::to_string(err), VeilGetLineInfo);
        }
    }
#   endif
    this->started = false;
}

uint64 Thread::id() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    auto *wts = (Win32ThreadStruct *) this->native_struct;
    return static_cast<uint64>(wts->id);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    auto *pts = (PThreadStruct *) this->native_struct;
    return (uint64) pts->embedded;
#   endif
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

Mutex::Mutex() : native_struct(nullptr) {
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
        veil::force_exit_on_error(
                "InitializeCriticalSectionAndSpinCount failed on error code :: " + std::to_string(GetLastError()),
                VeilGetLineInfo);
    }
    this->native_struct = m;
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/007904875/functions/pthread_mutex_init.html

    // Allocate the memory for pthread_mutex_t structure before mutex initialization, or will result in segmentation
    // fault, this memory will be freed in destructor.
    auto *pms = new PThreadMutexStruct();
    int err = pthread_mutex_init(&pms->embedded, nullptr);
    switch (err) {
    case 0:
        break;
    case EAGAIN:
    case ENOMEM:
        veil::force_exit_on_error("Insufficient resources to create a mutex.", VeilGetLineInfo);
    case EPERM:
        veil::force_exit_on_error("No permission to create a mutex.", VeilGetLineInfo);
    case EBUSY:
    case EINVAL:
    default:
        veil::implementation_fault("Should not reach here :: " + std::to_string(err), VeilGetLineInfo);
    }
    this->native_struct = pms;
#   endif
}

Mutex::~Mutex() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/sync/using-critical-section-objects
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-deletecriticalsection
    auto *ms = (Win32MutexStruct *) this->native_struct;
    DeleteCriticalSection(&ms->embedded);
    delete ((Win32MutexStruct *) this->native_struct);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/007904875/functions/pthread_mutex_destroy.html
    auto *pms = (PThreadMutexStruct *) this->native_struct;
    int err = pthread_mutex_destroy(&pms->embedded);
    switch (err) {
    case 0:
        break;
    case EBUSY:
        // This case will be raised as critical error as all mutex usage should be properly handled in the higher
        // abstraction level.
        veil::implementation_fault("Attempt to destroy a locked mutex.", VeilGetLineInfo);
    case EINVAL: // The mutex is always initialized as it is being done in the constructor.
    default:
        veil::force_exit_on_error("Should not reach here :: " + std::to_string(err), VeilGetLineInfo);
    }
    delete pms;
#   endif
}

void Mutex::lock() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/sync/using-critical-section-objects
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-entercriticalsection
    auto *ms = (Win32MutexStruct *) this->native_struct;
    EnterCriticalSection(&ms->embedded);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutex_lock.html
    auto *pms = (PThreadMutexStruct *) this->native_struct;
    int err = pthread_mutex_lock(&pms->embedded);
    switch (err) {
    case 0:
        break;
    case EDEADLK:
        veil::implementation_fault("Attempt to lock a owned mutex.", VeilGetLineInfo);
    case EAGAIN:
        veil::force_exit_on_error("The maximum number of recursive locks for mutex has been exceeded.",
                                  VeilGetLineInfo);
    case EINVAL:
    default:
        veil::implementation_fault("Should not reach here :: " + std::to_string(err), VeilGetLineInfo);
    }
#   endif
}

void Mutex::unlock() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/sync/using-critical-section-objects
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-leavecriticalsection
    auto *ms = (Win32MutexStruct *) this->native_struct;
    LeaveCriticalSection(&ms->embedded);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutex_unlock.html
    auto *pms = (PThreadMutexStruct *) this->native_struct;
    int err = pthread_mutex_unlock(&pms->embedded);
    switch (err) {
    case 0:
        break;
    case EPERM:
        veil::implementation_fault("Attempt to unlock a mutex without a prior lock on it.", VeilGetLineInfo);
    case EAGAIN:
        veil::force_exit_on_error(
                "The mutex could not be unlocked because the maximum number of recursive locks for mutex has been "
                "exceeded.", VeilGetLineInfo);
    case EINVAL:
    default:
        veil::implementation_fault("Should not reach here :: " + std::to_string(err), VeilGetLineInfo);
    }
#   endif
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)

struct Win32ConVarStruct : veil::memory::HeapObject {
    /// \c CONDITION_VARIABLE is a structure with a single attribute of type ( \c void \c* ).
    CONDITION_VARIABLE embedded;
};

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)

struct PThreadConVarStruct : veil::memory::HeapObject {
    pthread_cond_t embedded;
};

#endif

ConditionVariable::ConditionVariable() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/sync/using-condition-variables
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializeconditionvariable
    auto *cvs = new Win32ConVarStruct();
    InitializeConditionVariable(&cvs->embedded);
    this->os_cv = cvs;
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_cond_init.html
    auto *cvs = new PThreadConVarStruct();
    // Returns 0 if successful.
    int err = pthread_cond_init(&cvs->embedded, nullptr);
    switch (err) {
    case 0:
        break;
    case EAGAIN:
    case ENOMEM:
        veil::force_exit_on_error("Insufficient resources to create a condition variable.", VeilGetLineInfo);
    case EINVAL: // This case will never happen as passing nullptr as the pthread_attr means using default value.
    case EBUSY: // The pthread_cond_t in embedded structure is newly allocated, it cannot be a used copy.
    default: // No other error code will be possible in the current implementation of pthread.
        veil::implementation_fault("Should not reach here :: " + std::to_string(err), VeilGetLineInfo);
    }
    this->os_cv = cvs;
#   endif
}

ConditionVariable::~ConditionVariable() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Win32 condition variable does not need to be destroyed, thus we free the memory directly.
    delete ((Win32ConVarStruct *) this->os_cv);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_cond_destroy.html
    auto *cvs = (PThreadConVarStruct *) this->os_cv;
    // Returns 0 if successful.
    if (pthread_cond_destroy(&cvs->embedded)) {
        int err = errno;
        switch (err) {
        case EBUSY:
            veil::force_exit_on_error("Attempt to destroy a busy condition variable", VeilGetLineInfo);
        case EINVAL:
            // The only error code EINVAL happens only if the condition variable is not initialized, which will not
            // happen if it is properly instantiated. If this happens please check the implementations of the
            // constructor of ConditionVariable.
        default: // No other error code is available.
            veil::implementation_fault("Should not reach here :: " + std::to_string(err), VeilGetLineInfo);
        }
    }
    delete cvs;
#   endif
}

bool ConditionVariable::wait_for(uint32 milliseconds) {
    bool timed_out = false;
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/sync/using-condition-variables
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-sleepconditionvariablecs

    // To ensure the condition variable to be checked atomically, we have to lock the associated mutex.
    this->associate.lock();
    auto *cvs = (Win32ConVarStruct *) this->os_cv;
    auto *ms = (Win32MutexStruct *) associate.native_struct;
    BOOL success = SleepConditionVariableCS(&cvs->embedded, &ms->embedded, (DWORD) milliseconds);
    this->associate.unlock();
    if (!success) {
        uint32 err = GetLastError();
        if (err == ERROR_TIMEOUT) {
            timed_out = true;
        } else {
            veil::force_exit_on_error("SleepConditionVariableCS failed on error code :: " + std::to_string(err),
                                      VeilGetLineInfo);
        }
    }
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    struct timeval now = {};
    gettimeofday(&now, nullptr);

    int64 abs_nsecs = now.tv_usec * 1000LL + util::to_signed(milliseconds) * 1000000LL;

    struct timespec ts = {};
    ts.tv_sec = now.tv_sec + abs_nsecs / 1000000000LL;
    ts.tv_nsec = abs_nsecs % 1000000000LL;

    // Lock the associated mutex for thread safety.
    this->associate.lock();
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_cond_wait.html
    auto *cvs = (PThreadConVarStruct *) this->os_cv;
    auto *pms = (PThreadMutexStruct *) this->associate.native_struct;
    // Returns 0 if successful.
    int err = pthread_cond_timedwait(&cvs->embedded, &pms->embedded, &ts);
    switch (err) {
    case 0:
        break;
    case ETIMEDOUT:
        timed_out = true;
        break;
    case EINVAL:
    default:
        // The only error code EINVAL happens only if:
        // -
        // The condition variable & mutex as the parameter is not initialized, which will not happen if both are
        // properly instantiated. If this happens please check the implementations of the constructors of Mutex &
        // ConditionVariable.
        // -
        // Different mutexes were supplied for concurrent pthread_cond_wait() or pthread_cond_timedwait() operations on
        // the same condition variable, which will not happen as the mutex we use is embedded within this instance.
        // -
        // The mutex was not owned by the current thread at the time of the call, which will not happen as the mutex we
        // use is embedded within this instance.
        veil::implementation_fault("Should not reach here :: " + std::to_string(err), VeilGetLineInfo);
    }
    this->associate.unlock();
#   endif
    return !timed_out;
}

void ConditionVariable::wait() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // wait_for will never return false as the time to wait is set to INFINITE.
    this->wait_for(INFINITE);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Lock the associated mutex for thread safety.
    this->associate.lock();
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_cond_wait.html
    auto *cvs = (PThreadConVarStruct *) this->os_cv;
    auto *pms = (PThreadMutexStruct *) this->associate.native_struct;
    // Returns 0 if successful.
    int err = pthread_cond_wait(&cvs->embedded, &pms->embedded);
    if (err) {
        // The only error code EINVAL happens only if:
        // -
        // The condition variable & mutex as the parameter is not initialized, which will not happen if both are
        // properly instantiated. If this happens please check the implementations of the constructors of Mutex &
        // ConditionVariable.
        // -
        // Different mutexes were supplied for concurrent pthread_cond_wait() or pthread_cond_timedwait() operations on
        // the same condition variable, which will not happen as the mutex we use is embedded within this instance.
        // -
        // The mutex was not owned by the current thread at the time of the call, which will not happen as the mutex we
        // use is embedded within this instance.
        veil::implementation_fault("Should not reach here :: " + std::to_string(err), VeilGetLineInfo);
    }
    this->associate.unlock();
#   endif
}

void ConditionVariable::notify() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/sync/using-condition-variables
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-wakeconditionvariable
    auto *cvs = (Win32ConVarStruct *) this->os_cv;
    WakeConditionVariable(&cvs->embedded);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Lock the associated mutex for thread safety.
    this->associate.lock();
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_cond_signal.html
    auto *cvs = (PThreadConVarStruct *) this->os_cv;
    // Returns 0 if successful.
    int err = pthread_cond_signal(&cvs->embedded);
    if (err) {
        // The only error code EINVAL happens only if the condition variable is not initialized, which will not happen
        // if it is properly instantiated. If this happens please check the implementations of the constructor of
        // ConditionVariable.
        veil::implementation_fault("Should not reach here :: " + std::to_string(err), VeilGetLineInfo);
    }
    this->associate.unlock();
#   endif
}

void ConditionVariable::notify_all() {
#   if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    // Implementation of the following have taken reference from:
    // https://learn.microsoft.com/en-us/windows/win32/sync/using-condition-variables
    // https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-wakeallconditionvariable
    auto *cvs = (Win32ConVarStruct *) this->os_cv;
    WakeAllConditionVariable(&cvs->embedded);
#   elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__CYGWIN__)
    // Lock the associated mutex for thread safety.
    this->associate.lock();
    // Implementation of the following have taken reference from:
    // https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_cond_broadcast.html
    auto *cvs = (PThreadConVarStruct *) this->os_cv;
    // Returns 0 if successful.
    int err = pthread_cond_broadcast(&cvs->embedded);
    if (err) {
        // The only error code EINVAL happens only if the condition variable is not initialized, which will not happen
        // if it is properly instantiated. If this happens please check the implementations of the constructor of
        // ConditionVariable.
        veil::implementation_fault("Should not reach here :: " + std::to_string(err), VeilGetLineInfo);
    }
    this->associate.unlock();
#   endif
}
