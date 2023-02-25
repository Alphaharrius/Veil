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

#ifndef VEIL_FABRIC_SRC_THREADING_OS_HPP
#define VEIL_FABRIC_SRC_THREADING_OS_HPP

#include "src/memory/global.hpp"
#include "src/vm/structures.hpp"
#include "src/threading/atomic.hpp"

namespace veil::os {

    /// The wrapper of the native mutex construct.
    /// <ul>
    ///     <li> For Win32 we uses \c CRITICAL_SECTION as the backend. </li>
    ///     <li> For Linux/UNIX we uses \c pthread_mutex_t as the backend. </li>
    /// </ul>
    class Mutex : public memory::ValueObject {
    public:
        Mutex();

        /// Destroys the embedded native mutex construct.
        ~Mutex();

        /// The calling thread will grant exclusive access to the mutex, all subsequent locker threads will enters block
        /// state.
        /// \attention For Win32 this operation is guaranteed to be successful; for \c pthread_mutex_t there will be
        /// two cases which a failure will result in exiting the whole process:
        /// <ul>
        ///     <li> Re-locking the mutex. </li>
        ///     <li> The maximum number of recursive locks for mutex has been exceeded. </li>
        /// </ul>
        void lock();

        /// The calling thread will give up the exclusive access to the mutex, all subsequent locker threads will exit
        /// block state and competes for the access right.
        /// \attention For Win32 this operation is guaranteed to be successful; for \c pthread_mutex_t there will be
        /// two cases which a failure will result in exiting the whole process:
        /// <ul>
        ///     <li> Unlocking a mutex that the thread hasn't owned. </li>
        ///     <li> The maximum number of recursive locks for mutex has been exceeded. </li>
        /// </ul>
        void unlock();

    private:
        /// A pointer to a structure that holds the native mutex, the type varies by platform.
        void *native_struct;

        // Condition variable requires accessing the native mutex to function.
        friend class ConditionVariable;
    };

    class ConditionVariable : public memory::ValueObject {
    public:
        ConditionVariable();

        ~ConditionVariable();

        void wait();

        bool wait_for(int32 milliseconds);

        void notify();

        void notify_all();

    private:
        Mutex associate;
        void *os_cv;
    };

    class Thread : public memory::ValueObject {
    public:
        static void static_sleep(uint32 milliseconds);

        static uint64 current_thread_id();

        Thread();

        ~Thread();

        // TODO: Move sleep, block & wake to VMThread, don't blur the domain of responsibility.

        void start(vm::Executable &callable, uint32 &error);

        void sleep(int32 milliseconds, uint32 &error);

        void block(uint32 &error);

        void wake();

        void join(uint32 &error);

        uint64 id();

    private:
        void *native_struct;
        bool started;

        ConditionVariable blocking_cv;
    };

}

#endif //VEIL_FABRIC_SRC_THREADING_OS_HPP
