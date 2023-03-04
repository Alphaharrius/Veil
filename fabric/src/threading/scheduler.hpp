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

#ifndef VEIL_FABRIC_SRC_THREADING_SCHEDULER_HPP
#define VEIL_FABRIC_SRC_THREADING_SCHEDULER_HPP

#include "src/memory/global.hpp"
#include "src/threading/os.hpp"
#include "src/threading/handshake.hpp"
#include "src/vm/structures.hpp"

namespace veil::threading {
    /// The threading management of this runtime is designed to be single-point manageable, except of having all threads
    /// manages their own life cycle after spawning (pause/resume/termination), it will all be handled by the scheduler
    /// in a single-threaded task loop. Each task (sub classes of <code>ScheduledTask</code>) will encapsulate the
    /// requests to control or signal the thread's lifecycle, thus maximal thread-safety during these events are tightly
    /// guaranteed.
    class Scheduler;

    class ScheduledTask;

    class VMThread;

    class VMService;

    class Scheduler : public memory::ValueObject, private memory::TArena<threading::VMThread> {
    public:
        Scheduler();

        /// \brief Start the task loop of the scheduler.
        /// This method will kick start the task loop of the scheduler, which handles the spawning of a new thread, the
        /// termination of a job-completed thread, pause and resume of a running thread. Since all events are happening
        /// in a single threaded loop, synchronization of thread events are guaranteed.
        /// \attention This method will not return until the scheduler is terminated, thus we should choose carefully
        /// which thread will host the scheduler itself. To kept the scheduler manageable, it is best to be hosted on
        /// the main thread, which is not part of the scheduler managed threads.
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
        bool volatile signal_completed;
        bool volatile slept_thread_awake;

        friend void Scheduler::start();
        friend void Scheduler::add_task(ScheduledTask &task);
    };

    class VMThread : public memory::ArenaObject, public vm::HasRoot<Scheduler>, public vm::HasMember<VMService> {
    public:
        static const uint64 NULL_SERVICE_IDENTIFIER = 0;

        VMThread();

        void host(VMService &service);

        bool sleep(uint32 milliseconds);

    private:
        bool volatile idle;
        uint64 service_identifier;
        os::Thread embedded_os_thread;

        os::ConditionVariable self_blocking_cv;
        HandShake pause_handshake;
        HandShake resume_handshake;
        os::atomic_bool_t signaled_interrupt;

        bool volatile thread_join_negotiated;
        os::ConditionVariable thread_join_blocking_cv;

        bool check_if_interrupted();

        void pause_if_requested();
    };

    class VMService : public vm::HasName, public vm::Executable, public vm::HasRoot<VMThread> {
    public:
        void execute() override;

        uint64 get_unique_identifier();

        virtual void run_task() = 0;
    };
}

#endif //VEIL_FABRIC_SRC_THREADING_SCHEDULER_HPP
