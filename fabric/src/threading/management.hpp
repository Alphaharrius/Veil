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

        void interrupt();
    };

    class VMThread : public memory::ArenaObject, public vm::Constituent<Management> {
    private:
        struct BlockingAgent {
            veil::os::ConditionVariable blocking_cv;
            os::atomic_bool wake;

            BlockingAgent();
        };

    public:
        explicit VMThread(Management &management);

        void start(VMService &service, uint32 &error);

        void join(uint32 &error);

        // TODO: Implement pause API.

        void wake();

    protected:
        void sleep(uint32 milliseconds, uint32 &error);

        void block(uint32 &error);

    private:
        os::Thread embedded;

        BlockingAgent blocking_agent;

        void interrupt();

        friend class Management;
        friend void VMService::interrupt();
    };

    class Management : public memory::HeapObject, public vm::Constituent<Runtime>, private memory::TArena<VMThread> {
    public:
        void start_service(VMService &service, uint32 &error);
    };

}

#endif //VEIL_FABRIC_SRC_THREADING_MANAGEMENT_HPP
