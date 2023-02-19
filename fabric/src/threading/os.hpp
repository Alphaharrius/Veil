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

#include "src/memory/memory.hpp"
#include "src/vm/structures.hpp"

namespace veil::os {

    class OSThread : public memory::ValueObject {
    public:
        static void sleep(uint32 milliseconds);

        OSThread();

        ~OSThread();

        void start(vm::Callable &callable, uint32 &error);

        void join(uint32 &error);

    private:
        void *os_thread;
    };

    class OSMutex : public memory::ValueObject {
    public:
        OSMutex();

        ~OSMutex();

        void lock(uint32 &error);

        void unlock(uint32 &error);

    private:
        void *os_mutex;
    };

}

#endif //VEIL_FABRIC_SRC_THREADING_OS_HPP
