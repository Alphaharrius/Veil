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
    /// The threading management of this runtime is designed to be a facade manager, except of having all threads
    /// manages their own life cycle after spawning (pause/resume/termination), it will all be handled by the scheduler
    /// in a single-threaded task loop. Each task (sub classes of <code>ScheduledTask</code>) will encapsulate the
    /// requests to control or signal the thread's lifecycle, thus maximal thread-safety during these events are tightly
    /// guaranteed.
    class Scheduler;

    /// A subclass of this class encapsulate a request to control or signal a thread's lifecycle, each task will be
    /// queued in the task loop in the scheduler and processed one by one.
    /// \attention Beware of the program going out of scope since the destructor will be called immediately and will
    /// effectively breaks the task loop, to tackle this problem in an assertion is tested in the destructor of this
    /// class to make sure segmentation fault never happens.<br><br>
    /// If the calling thread is the thread who runs the scheduler process loop, this warning can be safely ignored;
    /// else if the calling thread <b>does not</b> call the method <code>ScheduledTask::wait_for_completion()</code>
    /// then make sure the scheduled task is allocated on the heap and is properly deallocated after the task completion
    /// (This is problematic thus is <b>not suggested</b>.).<br><br>
    /// The <b>best</b> way is to instantiate this class on the stack like <code>ScheduledTask task()</code> and wait
    /// for the task completion using <code>ScheduledTask::wait_for_completion()</code>, this makes sure that the
    /// reference of the task in the scheduler task loop is cleared before the destructor of the task is called.
    class ScheduledTask;

    class VMThread;

    class VMService;

    class Scheduler : public memory::ValueObject, private memory::TArena<threading::VMThread> {
    public:
        static const uint64 NULL_SERVICE_ID = 0;
        static const uint64 SCHEDULER_SERVICE_ID = 1;

        class StartServiceTask;

        class ThreadReturnTask;

        class ThreadPauseTask;

        class ThreadResumeTask;

        Scheduler();

        /// \brief Start the task loop of the scheduler.
        /// This method will kick start the task loop of the scheduler, which handles the spawning of a new thread, the
        /// termination of a job-completed thread, pause and resume of a running thread. Since all events are happening
        /// in a single threaded loop, synchronization of thread events are guaranteed.
        /// \attention This method will not return until the scheduler is terminated, thus we should choose carefully
        /// which thread will host the scheduler itself. To kept the scheduler manageable, it is best to be hosted on
        /// the main thread, which is not part of the scheduler managed threads.
        void start();

        void terminate();

        bool is_terminated();

        void add_task(ScheduledTask &task);

        void add_realtime_task(ScheduledTask &task);

        void notify();

    private:
        os::atomic_u64_t service_id_distribution;
        /// This flag determines whether the scheduler <b>will be</b> terminated, if this is set to <code>true</code>
        /// then the scheduler will be terminated at the next process cycle and <code>Scheduler::start()</code> will
        /// return.
        os::atomic_bool_t termination_requested;
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

        VMThread &idle_thread();

        /// Internal method to be called within <code>start()</code> only if the flag <code>termination_requested</code>
        /// is set <code>true</code>.
        /// \attention The action of <code>Scheduler::terminate()</code> will not use the scheduler process loop, it
        /// should independently interrupt all existing threads and wait for all to terminate. Since there will be no
        /// new threads spawning, pausing or terminating after the scheduler exited the task loop, thus this method does
        /// not rely much on thread synchronizations.
        void finalization_on_termination();
    };

    class ScheduledTask : public vm::HasRoot<Scheduler> {
    public:
        ScheduledTask();

        virtual ~ScheduledTask();

        /// \brief Wait until the task is being processed by the scheduler.
        /// By calling this method the calling thread will be blocked on the <code>request_thread_cv</code> until the
        /// scheduler notifies it after the task is completed.
        /// \attention If this method is called before <code>Scheduler::add_task(ScheduledTask)</code>, the calling
        /// thread will enters an unrecoverable sleep. Please don't call this method if the calling thread is the thread
        /// that runs the scheduler task loop, it would result in another unrecoverable sleep since there is no other
        /// thread that can notify <code>request_thread_cv</code> than the scheduler itself.
        void wait_for_completion();

        virtual void run() = 0;

    private:
        ScheduledTask *prev;
        ScheduledTask *next;
        os::ConditionVariable request_thread_cv;
        bool volatile request_thread_waiting;
        bool volatile signal_completed;
        bool volatile slept_thread_awake;

        void connect_last(ScheduledTask &task);

        void connect_next(ScheduledTask &task);

        void disconnect();

        ScheduledTask *get_next();

        ScheduledTask *get_prev();

        friend void Scheduler::start();

        friend void Scheduler::add_task(ScheduledTask &task);

        friend void Scheduler::add_realtime_task(ScheduledTask &task);
    };

    class Scheduler::StartServiceTask : public memory::ValueObject, public ScheduledTask {
    public:
        explicit StartServiceTask(VMService &target_service);

        void run() override;

    private:
        VMService *target_service;
    };

    class Scheduler::ThreadReturnTask : public memory::ValueObject, public ScheduledTask {
    public:
        explicit ThreadReturnTask(VMThread &target_thread);

        void run() override;

    private:
        VMThread *target_thread;
    };

    class Scheduler::ThreadPauseTask : public memory::ValueObject, public ScheduledTask {
    public:
        explicit ThreadPauseTask(VMThread &target_thread);

        void run() override;

    private:
        VMThread *target_thread;
    };

    class Scheduler::ThreadResumeTask : public memory::ValueObject, public ScheduledTask {
    public:
        explicit ThreadResumeTask(VMThread &target_thread);

        void run() override;

    private:
        VMThread *target_thread;
    };

    class VMService : public vm::HasName, public vm::Executable,
                      public vm::HasRoot<Scheduler>, public vm::HasRoot<VMThread> {
    public:
        explicit VMService(const std::string& name);

        virtual ~VMService();

        [[nodiscard]] uint64 get_id() const;

        void execute() override;

        virtual void run() = 0;

    private:
        uint64 id;

        void set_id(uint64 service_id);

        friend void Scheduler::start();
        friend void Scheduler::StartServiceTask::run();
    };

    class VMThread : public memory::ArenaObject, public vm::HasRoot<Scheduler>, public vm::HasMember<VMService> {
    public:
        VMThread();

        void host(VMService &service);

        [[nodiscard]] bool is_idle() const;

    protected:
        bool sleep(uint32 milliseconds);

    private:
        bool volatile idle;
        uint64 current_service_id;
        os::Thread embedded_os_thread;

        os::ConditionVariable self_blocking_cv;
        os::ConditionVariable requester_waiting_cv;
        HandShake pause_handshake;
        HandShake resume_handshake;
        HandShake wake_handshake;
        os::atomic_bool_t signaled_interrupt;

        bool volatile thread_join_negotiated;
        os::ConditionVariable thread_join_blocking_cv;

        void wake();

        void interrupt();

        bool check_if_interrupted();

        bool request_pause(uint32 wait_milliseconds);

        void resume();

        void pause_if_requested();

        Scheduler::ThreadReturnTask self_return_task;

        friend void VMService::execute();
        friend class Scheduler;
    };

    /// This method is designed to be used for debugging and diagnostic purposes, for example: which <code>VMService
    /// </code> have acquired a mutex for a long period of time. This notions stands due to <code>VMService</code> of
    /// the running thread should be available from using <code>this</code> keyword if called from <code>VMService.run()
    /// </code>, thus the only use case are those methods that does not have access to the calling service task.
    /// \attention This method should only be called by threads that host a <code>VMService</code> managed by the <code>
    /// Scheduler</code> as only those will be stored and mapped with the corresponding thread id retrieved from <code>
    /// os::Thread::current_thread_id()</code>. <b>Invalid use of this method will lead to process abortion</b>.
    /// \return The current <code>VMService</code> of the calling thread.
    VMService &current_service();

}

#endif //VEIL_FABRIC_SRC_THREADING_SCHEDULER_HPP
