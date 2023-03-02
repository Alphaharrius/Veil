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

        void add_task(ScheduledTask &task);

    private:
        /// This flag determines whether the scheduler <b>will be</b> terminated, if this is set to <code>true</code>
        /// then the scheduler will be terminated at the next process cycle and <code>Scheduler::start()</code> will
        /// return.
        volatile bool termination_requested;
        /// This is used by the scheduler to pause itself when there are no task left to do, and should only be notified
        /// by the method <code>Scheduler::add_task()</code> only.
        os::ConditionVariable process_cycle_pause_cv;
        /// This flag determines whether the scheduler <b>is</b> paused, and should not be modified by all but the
        /// method <code>Scheduler::start()</code> to ensure the explicitness of state.
        volatile bool process_cycle_paused;
        /// This is used to ensure only one thread will fiddle with the state of the scheduler, all action within the
        /// scheduler which will mutate the state must lock this mutex.
        os::Mutex scheduler_action_m;
        /// This is the anchor element of the circle task list of the scheduler, the list have the structure of:<br>
        /// <pre>...-[added_task]-[current_task]-[next_task]-...-[added_task]-...</pre><br>
        /// Which all added task will be connected on the left side of the current task, and the circle will rotate
        /// when the scheduler processes the tasks. <br>
        /// This pointer will be <code>nullptr</code> if there are no task left to do.
        ScheduledTask *current_task;

        /// Internal method to be called within <code>start()</code> only if the flag <code>termination_requested</code>
        /// is set <code>true</code>.
        /// \attention The action of <code>Scheduler::terminate()</code> will not use the scheduler process loop, it
        /// should independently interrupt all existing threads and wait for all to terminate. Since there will be no
        /// new threads spawning, pausing or terminating after the scheduler exited the task loop, thus this method does
        /// not rely much on thread synchronizations.
        void finalization_on_termination();
    };

    class ScheduledTask {
    public:
        ScheduledTask();

        void wait_for_completion();

        void connect(ScheduledTask &task);

        void disconnect();

        ScheduledTask *get_next();

        ScheduledTask *get_prev();

        virtual void run() = 0;

    private:
        ScheduledTask *prev;
        ScheduledTask *next;
        os::ConditionVariable request_thread_cv;
        volatile bool completed;
        volatile bool slept_thread_awake;

        friend void Scheduler::start();
        friend void Scheduler::add_task(ScheduledTask &task);
    };

    class VMThread : memory::ArenaObject {
    };
}

#endif //VEIL_FABRIC_SRC_THREADING_SCHEDULER_HPP
