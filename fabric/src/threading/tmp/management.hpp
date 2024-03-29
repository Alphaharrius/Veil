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

    struct LockFreeState {
    public:
        static const uint32 TICK = 0;
        static const uint32 TOK = 1;
        static const uint32 CLOSED = 2;

        LockFreeState();

        void open();

        bool tick();

        bool tok();

        bool close();

        bool is_tick();

        bool is_tok();

    private:
        os::atomic_u32_t flag;
    };

    class VMService : public memory::HeapObject, public vm::HasName, public vm::Executable,
                      public vm::HasRoot<VMThread> {
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
        LockFreeState join_state;

        volatile bool completed;
    };

    class VMThread : public memory::ArenaObject, public vm::HasRoot<Management>, public vm::HasMember<VMService> {
    public:
        struct BlockingAgent {
            os::ConditionVariable blocking_cv;
            os::atomic_bool_t signal_wake;

            BlockingAgent();
        };

        struct PauseAgent {
            LockFreeState pause_state;
            LockFreeState resume_state;
            os::Mutex caller_m; // TODO: Use this for pause synchronization only.
            os::ConditionVariable caller_cv;

            PauseAgent();
        };

        explicit VMThread(Management &management);

        bool start(VMService &service);

        void join();

    protected:
        void sleep(uint32 milliseconds, uint32 &error);

        void block(uint32 &error);

        bool check_interrupt();

        void check_pause();

    private:
        os::atomic_bool_t idle;

        os::Thread embedded;

        BlockingAgent blocking_agent;
        PauseAgent pause_agent;

        os::atomic_bool_t interrupted;

        void interrupt();

        void signal_pause();

        void wait_pause();

        void pause();

        void signal_resume();

        void wait_resume();

        void resume();

        void finalize();

        friend class Management;
        friend void VMService::sleep(uint32 milliseconds, uint32 &error);
        friend bool VMService::check_interrupt();
        friend void VMService::check_pause();
        friend void VMService::interrupt();
        friend void VMService::pause();
        friend void VMService::resume();
        friend void VMService::execute();
    };

    class Management : public memory::HeapObject, public vm::HasRoot<Runtime>, private memory::TArena<VMThread> {
    public:
        void start(VMService &service);

        void pause_all();

        void resume_all();

    private:
        os::Mutex thread_arena_m;
    };

}

#endif //VEIL_FABRIC_SRC_THREADING_MANAGEMENT_HPP
