#include "src/threading/scheduler.hpp"

using namespace veil::threading;

ScheduledTask::ScheduledTask() : prev(this), next(this), completed(false) {}

void ScheduledTask::wait() { while (!completed) scheduled_cv.wait(); }

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

Scheduler::Scheduler() : paused(true), current_task(nullptr) {
}

void Scheduler::start() {
    ScheduledTask *selected;
    // Fetch a task to run.
    Fetch: {
        os::CriticalSection _(scheduler_m);

        if (terminated) goto TERMINATE;

        // If there are no task left to do, the scheduler thread will be paused to avoid occupying the CPU.
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
    selected->completed = true; // Set the task as completed.
    // After the completion of the task, we have to wake up the thread that owns the task.
    selected->scheduled_cv.notify();
    goto Fetch;

    Pause:
    paused = true;
    pause_cv.wait();
    paused = false;
    goto Fetch;
    TERMINATE: return;
}

void Scheduler::add(ScheduledTask &task) {
    os::CriticalSection _(scheduler_m);

    // Connect the new task to the circle task list.
    if (this->current_task == nullptr) current_task = &task;
    else current_task->connect(task);

    // Just in case if the scheduler is slept, attempt to wake it up.
    while (paused) {
        pause_cv.notify();
        // Abandon the time slice of the underlying thread to other OS threads, especially the thread running the
        // scheduler, to run.
        os::Thread::static_sleep(0);
    }
}
