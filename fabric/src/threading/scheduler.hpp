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
        /// This flag determines whether the scheduler <b>will be</b> terminated, if this is set to \c true then the
        /// scheduler will be terminated at the next process cycle and the method \c Scheduler::start() will return.
        volatile bool terminated;
        /// This is used by the scheduler to pause itself when there are no task left to do, and should only be notified
        /// by the method \c Scheduler::add() only.
        os::ConditionVariable pause_cv;
        /// This flag determines whether the scheduler <b>is</b> paused, and should not be modified by all but the
        /// method \c Scheduler::start() to ensure the explicitness of state.
        volatile bool paused;
        /// This is used to ensure only one thread will fiddle with the state of the scheduler, all action within the
        /// scheduler which will mutate the state must lock this mutex.
        os::Mutex scheduler_m;
        /// This is the anchor element of the circle task list of the scheduler, the list have the structure of:<br>
        /// \c ...-[added_task]-[current_task]-[next_task]-...-[added_task]-... <br>
        /// Which all added task will be connected on the left side of the current task, and the circle will rotate
        /// when the scheduler processes the tasks. <br>
        /// This pointer will be \c nullptr if there are no task left to do.
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
