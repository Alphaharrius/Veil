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

#include "src/memory/memory.hpp"
#include "src/threading/os.hpp"
#include "src/vm/structures.hpp"
#include "src/core/runtime.forward.hpp"

namespace veil::threading {

    class VMThread :
            public vm::Constituent<Runtime>, public memory::HeapObject, private os::Callable, public vm::HasName {
    public:
        explicit VMThread(std::string &name, Runtime &runtime);

        void start();

        void join();

        void interrupt();

    protected:
        bool is_interrupted();

    private:
        os::OSThread embedded;

        std::atomic_bool interrupted;
    };

    class Management : public memory::HeapObject, public vm::Constituent<Runtime>, private memory::TArena<VMThread *> {
    private:
        void register_thread(VMThread &thread);

        friend void VMThread::start();
    };

}

#endif //VEIL_FABRIC_SRC_THREADING_MANAGEMENT_HPP
