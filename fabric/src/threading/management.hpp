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

        void join();
    };

    class VMThread : public memory::ArenaObject, public vm::Constituent<Management> {
    private:
        struct BlockingAgent {
            os::ConditionVariable blocking_cv;
            os::atomic_bool signal_wake;

            BlockingAgent();
        };

        struct PauseAgent {
            os::Mutex caller_m;
            os::ConditionVariable caller_cv;

            os::atomic_bool signal_pause;
            os::atomic_bool paused;
            os::atomic_bool signal_resume;

            PauseAgent();
        };

    public:
        explicit VMThread(Management &management);

        bool start(VMService &service);

        void join();

        void wake();

    protected:
        void sleep(uint32 milliseconds, uint32 &error);

        void block(uint32 &error);

        bool check_interrupt();

        void check_pause();

    private:
        os::atomic_bool idle;

        os::Thread embedded;

        BlockingAgent blocking_agent;
        PauseAgent pause_agent;

        os::atomic_bool interrupted;

        void interrupt();

        void pause(uint32 &error);

        void resume();

        friend class Management;
        friend void VMService::interrupt();
    };

    class Management : public memory::HeapObject, public vm::Constituent<Runtime>, private memory::TArena<VMThread> {
    public:
        void start_service(VMService &service, uint32 &error);
    };

}

#endif //VEIL_FABRIC_SRC_THREADING_MANAGEMENT_HPP
