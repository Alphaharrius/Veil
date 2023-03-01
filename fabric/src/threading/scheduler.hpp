#ifndef VEIL_FABRIC_SRC_THREADING_SCHEDULER_HPP
#define VEIL_FABRIC_SRC_THREADING_SCHEDULER_HPP

#include "src/memory/global.hpp"
#include "src/threading/os.hpp"

namespace veil::threading {
    class Scheduler;

    class ScheduledTask;

    class VMThread;

    class VMService;

    class Scheduler : memory::TArena<threading::VMThread> {
    public:
        Scheduler();

        void start();

        void add(ScheduledTask &task);

    private:
        volatile bool terminated;
        os::ConditionVariable pause_cv;
        volatile bool paused;
        os::Mutex scheduler_m;
        ScheduledTask *current_task;
    };

    class ScheduledTask {
    public:
        ScheduledTask();

        void wait();

        void connect(ScheduledTask &task);

        void disconnect();

        ScheduledTask *get_next();

        ScheduledTask *get_prev();

        virtual void run() = 0;

    private:
        ScheduledTask *prev;
        ScheduledTask *next;
        os::ConditionVariable scheduled_cv;
        volatile bool completed;

        friend void Scheduler::start();
    };

    class VMThread : memory::ArenaObject {
    };
}

#endif //VEIL_FABRIC_SRC_THREADING_SCHEDULER_HPP
