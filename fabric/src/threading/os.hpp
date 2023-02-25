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

    class Mutex : public memory::ValueObject {
    public:
        Mutex();

        ~Mutex();

        void lock();

        void unlock();

    private:
        void *os_mutex;

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

        Thread();

        ~Thread();

        void start(vm::Executable &callable, uint32 &error);

        void sleep(int32 milliseconds, uint32 &error);

        void block(uint32 &error);

        void wake();

        void join(uint32 &error);

    private:
        void *os_thread;
        bool started;

        ConditionVariable blocking_cv;
    };

}

#endif //VEIL_FABRIC_SRC_THREADING_OS_HPP
