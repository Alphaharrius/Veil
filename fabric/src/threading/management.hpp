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

#ifndef VEIL_FABRIC_SRC_THREADING_MANAGEMENT_HPP
#define VEIL_FABRIC_SRC_THREADING_MANAGEMENT_HPP

#include <atomic>
#include <string>

#include "src/memory/global.hpp"
#include "src/threading/os.hpp"
#include "src/vm/structures.hpp"
#include "src/core/runtime.forward.hpp"

namespace veil::threading {

    class Management;

    class VMThread;

    class VMService : public memory::HeapObject, public vm::HasName, public vm::Executable,
                      public vm::Constituent<VMThread> {
    public:
        explicit VMService(std::string &name);
    };

    class VMThread : public memory::HeapObject, public vm::HasName,
                     public vm::Constituent<Runtime>, public vm::Constituent<Management> {
    public:
        explicit VMThread(std::string &name, Management &management);

        void start(VMService &service, uint32 &error);

        void join(uint32 &error);

        void interrupt();

    protected:
        bool is_interrupted();

    private:
        os::Thread embedded;
        std::atomic_bool interrupted;
    };

    class Management : public memory::HeapObject, public vm::Constituent<Runtime>, private memory::TArena<VMThread *> {
    private:
        void register_thread(VMThread &thread);
    };

}

#endif //VEIL_FABRIC_SRC_THREADING_MANAGEMENT_HPP
