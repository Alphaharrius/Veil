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

        virtual void run() = 0;

        void execute() override;

        void sleep(uint32 milliseconds, uint32 &error);

        void interrupt();

        void join();

        void pause();

        void resume();

        bool check_interrupt();

        void check_pause();

    private:
        os::ConditionVariable join_cv;
        os::atomic_bool join_flag;

        volatile bool completed;
    };

    class VMThread : public memory::ArenaObject, public vm::Constituent<Management>, public vm::Composite<VMService> {
    private:
        struct BlockingAgent {
            os::ConditionVariable blocking_cv;
            os::atomic_bool signal_wake;

            BlockingAgent();
        };

        struct PauseAgent {
            os::atomic_bool caller_flag; // TODO: Use this to ensure thread safety of state between caller & target.
            os::Mutex caller_m; // TODO: Use this for pause synchronization only.
            os::ConditionVariable caller_cv;

            os::atomic_bool signal_pause;
            os::atomic_bool paused;
            os::atomic_bool signal_resume;

            PauseAgent();
        };

    public:
        explicit VMThread(Management &management);

        void start(VMService &service);

        void join();

    protected:
        void sleep(uint32 milliseconds, uint32 &error);

        void block(uint32 &error);

        bool check_interrupt();

        void check_pause();

    private:
        os::atomic_bool idle;
        os::atomic_bool stopped;

        os::Thread embedded;

        BlockingAgent blocking_agent;
        PauseAgent pause_agent;

        os::atomic_bool interrupted;

        void interrupt();

        void pause();

        void resume();

        void reset();

        friend class Management;
        friend void VMService::sleep(uint32 milliseconds, uint32 &error);
        friend bool VMService::check_interrupt();
        friend void VMService::check_pause();
        friend void VMService::interrupt();
        friend void VMService::pause();
        friend void VMService::resume();
        friend void VMService::execute();
    };

    class Management : public memory::HeapObject, public vm::Constituent<Runtime>, private memory::TArena<VMThread> {
    public:
        void start(VMService &service);

        void pause_all();

        void resume_all();

    private:
        os::Mutex thread_arena_m;
    };

}

#endif //VEIL_FABRIC_SRC_THREADING_MANAGEMENT_HPP
