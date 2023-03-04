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

#include "src/threading/scheduler.hpp"

using namespace veil::threading;

ScheduledTask::ScheduledTask() : prev(this), next(this), signal_completed(false) {}

void ScheduledTask::wait_for_completion() {
    // Wait until the task is being completed by the scheduler.
    while (this->signal_completed) this->request_thread_cv.wait();
    // Since the scheduler will wait until the waiting thread signals its wake, we have to set this flag to true.
    this->slept_thread_awake = true;
}

void ScheduledTask::connect(ScheduledTask &task) {
    // Connect the previous task to the new task.
    this->prev->next = &task;
    task.prev = this->prev;

    // Connect this task to the new task.
    this->prev = &task;
    task.next = this;
}

void ScheduledTask::disconnect() {
    // Connect the previous task to the next task.
    this->prev->next = this->next;
    this->next->prev = this->prev;
}

ScheduledTask *ScheduledTask::get_next() { return next; }

ScheduledTask *ScheduledTask::get_prev() { return prev; }

Scheduler::Scheduler() : process_cycle_paused(true), current_task(nullptr) {
}

void Scheduler::start() {
    ScheduledTask *selected;
    // Fetch a task to run.
    Fetch:
    {
        os::CriticalSection _(scheduler_action_m);

        if (termination_requested) goto Terminate;

            // If there are no task left to do, the scheduler thread will be paused to avoid occupying the CPU.
            // NOTE: current_task == nullptr is count as explicit information to signify the scheduler is now free,
            // since the scheduler loop is protected by the mutex scheduler_action_m, no new task will be added until
            // this cycle ends, thus we can safely head to the pause state.
        else if (current_task == nullptr) goto Pause;

            // If there are only one task left, fetch it and set current_task to nullptr.
        else if (current_task->get_next() == current_task) {
            selected = current_task;
            // Setting this to nullptr signals the scheduler to pause on the next round.
            current_task = nullptr;
        }

            // Fetch the current task and set the next task as the current task.
        else {
            selected = current_task;
            current_task = current_task->get_next();
        }
    }

    selected->run();
    selected->disconnect(); // Disconnect the task from the circle task list as it is completed.
    selected->signal_completed = true; // Set the task as completed.
    // After the completion of the task, we have to wake up the thread that owns the task.
    while (!selected->slept_thread_awake) {
        selected->request_thread_cv.notify();
        os::Thread::static_sleep(0);
    }
    goto Fetch;

    Pause:
    process_cycle_paused = true;
    process_cycle_pause_cv.wait();
    process_cycle_paused = false;
    goto Fetch;

    // NOTE: The action of Scheduler::terminate() will not use the scheduler process loop, it should independently
    // interrupt all existing threads and wait for all to terminate. A sweet spot is here due to the inactivity of the
    // scheduler loop, there will be no new threads spawning, pausing or terminating at this point, thus this can be
    // handled effortlessly (should be).
    Terminate:
    finalization_on_termination();
}

void Scheduler::finalization_on_termination() {
    // TODO: Implement method.
}

void Scheduler::add_task(ScheduledTask &task) {
    // Scheduler state specific task is protected by the mutex scheduler_action_m.
    // The following process is to:
    // 1. Connect the new task to the left side of the current task or the 'end' of the circle list according to the
    //    direction of process flow (from left to right); if there are no task in the circle list place the new task
    //    in the position of the current task.
    // 2. If the scheduler is in pause state, attempt to resume it to start processing new tasks.
    os::CriticalSection _(scheduler_action_m);

    // Connect the new task to the circle task list.
    if (this->current_task == nullptr) current_task = &task;
    else current_task->connect(task);

    // Just in case if the scheduler is slept, attempt to wake it up.
    while (process_cycle_paused) {
        process_cycle_pause_cv.notify();
        // Abandon the time slice of the underlying thread to other OS threads, especially the thread running the
        // scheduler, to run.
        os::Thread::static_sleep(0);
    }
}
