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

#include "src/threading/config.hpp"
#include "src/threading/scheduler.hpp"
#include "src/vm/os.hpp"
#include "src/util/hash.hpp"

using namespace veil::threading;

static const uint64 NULL_SERVICE_IDENTIFIER = 0;
static veil::os::atomic_u64_t global_vm_service_id_distribution(NULL_SERVICE_IDENTIFIER + 1);

ScheduledTask::ScheduledTask() : request_thread_waiting(false), prev(this), next(this), signal_completed(false),
                                 task_active(true) {}

ScheduledTask::~ScheduledTask() {
    // As the ScheduledTask will typically being registered into the task loop of the Scheduler, it cannot be destructed
    // before being processed and the awakening of the request thread waiting for the task completion.
    VeilAssert(!task_active.load() || signal_completed && !request_thread_waiting, "Invalid going out of scope.");
}

void ScheduledTask::wait_for_completion() {
    this->request_thread_waiting = true;
    // Wait until the task is being completed by the scheduler.
    while (this->signal_completed) this->request_thread_cv.wait();
    // Since the scheduler will wait until the waiting thread signals its wake, we have to set this flag to true.
    this->request_thread_waiting = false;
}

void ScheduledTask::reset_state_for_reuse() {
    task_active.store(true);
    request_thread_waiting = false;
    signal_completed = false;
}

void ScheduledTask::inactivate() { this->task_active.store(false); }

void ScheduledTask::connect_last(ScheduledTask &task) {
    // Connect the previous task to the new task.
    this->prev->next = &task;
    task.prev = this->prev;

    // Connect this task to the new task.
    this->prev = &task;
    task.next = this;
}

void ScheduledTask::connect_next(ScheduledTask &task) {
    // Connect the next task to the new task.
    this->next->prev = &task;
    task.next = this->next;

    // Connect this task to the new task.
    this->next = &task;
    task.prev = this;
}

void ScheduledTask::disconnect() {
    // Connect the previous task to the next task.
    this->prev->next = this->next;
    this->next->prev = this->prev;
}

ScheduledTask *ScheduledTask::get_next() { return next; }

ScheduledTask *ScheduledTask::get_prev() { return prev; }

Scheduler::Scheduler() : process_cycle_paused(true), current_task(nullptr), termination_requested(false) {}

class VMServiceTable {
private:
    struct Entry;

public:
    static const uint32 SLOT_COUNT = 4096;

    explicit VMServiceTable();

    void put(uint64 os_thread_id, VMService &service);

    VMService *get(uint64 os_thread_id);

    void remove(uint64 os_thread_id);

private:
    veil::os::Mutex table_access_m;

    Entry *slots[SLOT_COUNT]{};
    Entry *reusable_entries;
};

struct VMServiceTable::Entry : veil::memory::HeapObject {
    uint64 os_thread_id = 0;
    VMService *target_service = nullptr;
    Entry *next_entry = nullptr;
};

VMServiceTable::VMServiceTable() : reusable_entries(nullptr) {}

void VMServiceTable::put(uint64 os_thread_id, VMService &service) {
    veil::os::CriticalSection _(table_access_m);

    uint64 hashed_value = veil::util::standard_u64_hash_function(os_thread_id);
    uint32 slot_index = hashed_value % SLOT_COUNT;

    Entry *current_entry = slots[slot_index];
    while (current_entry != nullptr) {
        if (current_entry->os_thread_id == os_thread_id) break;
        current_entry = current_entry->next_entry;
    }

    if (current_entry == nullptr) {
        if (reusable_entries != nullptr) {
            current_entry = reusable_entries;
            reusable_entries = current_entry->next_entry;
        } else current_entry = new Entry();
        current_entry->next_entry = slots[slot_index];
        slots[slot_index] = current_entry;
    }

    current_entry->os_thread_id = os_thread_id;
    current_entry->target_service = &service;
}

VMService *VMServiceTable::get(uint64 os_thread_id) {
    veil::os::CriticalSection _(table_access_m);

    uint64 hashed_value = veil::util::standard_u64_hash_function(os_thread_id);
    uint32 slot_index = hashed_value % SLOT_COUNT;

    Entry *current_entry = slots[slot_index];
    while (current_entry != nullptr) {
        if (current_entry->os_thread_id == os_thread_id) break;
        current_entry = current_entry->next_entry;
    }

    return current_entry != nullptr ? current_entry->target_service : nullptr;
}

void VMServiceTable::remove(uint64 os_thread_id) {
    veil::os::CriticalSection _(table_access_m);

    uint64 hashed_value = veil::util::standard_u64_hash_function(os_thread_id);
    uint32 slot_index = hashed_value % SLOT_COUNT;

    Entry *previous_entry = nullptr, *current_entry = slots[slot_index];
    while (current_entry != nullptr) {
        if (current_entry->os_thread_id == os_thread_id) break;
        previous_entry = current_entry;
        current_entry = current_entry->next_entry;
    }

    if (current_entry == nullptr) return;

    if (previous_entry == nullptr) slots[slot_index] = current_entry->next_entry; // Case of matching the first entry.

    else previous_entry->next_entry = current_entry->next_entry; // Remove current entry from the linked list.

    current_entry->next_entry = reusable_entries;
    reusable_entries = current_entry;
}

static VMServiceTable global_vm_service_table;

class SchedulerService : public VMService {
public:
    SchedulerService();

    void run() override;
};

SchedulerService::SchedulerService() : VMService("Runtime:ThreadScheduler") {}

void SchedulerService::run() {} // This is a dummy definition, as it will never be used.

void Scheduler::start() {
    SchedulerService scheduler_service;
    global_vm_service_table.put(os::Thread::current_thread_id(), scheduler_service);

    ScheduledTask *selected;
    Fetch:
    {
        os::CriticalSection _(scheduler_action_m);

        if (termination_requested.load()) goto Terminate;

        else if (current_task == nullptr)
            // If there are no task left to do, the scheduler thread will be paused to avoid occupying the CPU.
            // NOTE: current_task == nullptr is count as explicit information to signify the scheduler is now free,
            // since the scheduler loop is protected by the mutex scheduler_action_m, no new task will be added until
            // this cycle ends, thus we can safely head to the pause state.
            goto Pause;

        else if (current_task->get_next() == current_task) {
            // If there are only one task left, fetch it and set current_task to nullptr.
            selected = current_task;
            // Setting this to nullptr signals the scheduler to pause on the next round.
            current_task = nullptr;
        } else {
            // Fetch the current task and set the next task as the current task.
            selected = current_task;
            current_task = current_task->get_next();
        }
    }

    // Check if the task is active, if not we will skip this task and move on to the next fetching operation.
    if (!selected->task_active.load()) goto Fetch;

    // start: process the selected task.
    selected->vm::HasRoot<Scheduler>::bind(*this);
    selected->run();
    selected->disconnect(); // Disconnect the task from the circle task list as it is completed.
    selected->signal_completed = true; // Set the task as completed.
    // After the completion of the task, we have to wake up the thread that owns the task if the request thread is
    // blocked on the request_thread_cv.
    while (selected->request_thread_waiting) {
        selected->request_thread_cv.notify();
        os::Thread::static_sleep(0);
    }
    // end: process the selected task.

    goto Fetch;

    // start: idle state.
    Pause:
    process_cycle_paused = true;
    process_cycle_pause_cv.wait();
    process_cycle_paused = false;
    // end: idle state.

    goto Fetch;

    // NOTE: The action of Scheduler::terminate() will not use the scheduler process loop, it should independently
    // interrupt all existing threads and wait for all to terminate. A sweet spot is here due to the inactivity of the
    // scheduler loop, there will be no new threads spawning, pausing or terminating at this point, thus this can be
    // handled effortlessly (should be).
    Terminate:
    finalization_on_termination();
    global_vm_service_table.remove(os::Thread::current_thread_id());
}

void Scheduler::terminate() {
    termination_requested.store(true);
    notify();
}

bool Scheduler::is_terminated() { return termination_requested.load(); }

void Scheduler::finalization_on_termination() {
    memory::TArenaIterator<VMThread> iterator_for_interrupt(*this);
    VMThread *current = iterator_for_interrupt.next();
    while (current != nullptr) {
        current->interrupt();
        current = iterator_for_interrupt.next();
    }
    memory::TArenaIterator<VMThread> iterator_for_join(*this);
    current = iterator_for_join.next();
    while (current != nullptr) {
        current->embedded_os_thread.join();
        current = iterator_for_join.next();
    }
    this->TArena<VMThread>::destruct_objects();
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
    else current_task->connect_last(task);
}

void Scheduler::add_realtime_task(ScheduledTask &task) {
    // Scheduler state specific task is protected by the mutex scheduler_action_m.
    // The following process is to:
    // 1. Connect the new task to the right side of the current task or the 'next' of the circle list according to the
    //    direction of process flow (from left to right); if there are no task in the circle list place the new task
    //    in the position of the current task.
    // 2. If the scheduler is in pause state, attempt to resume it to start processing new tasks.
    os::CriticalSection _(scheduler_action_m);

    // Connect the new task to the circle task list.
    if (this->current_task == nullptr) current_task = &task;
    else
        // Put the task to the next processing slot as it is important, if there are other high priority tasks placed
        // before this, they will be displaced backwards.
        current_task->connect_next(task);
}

void Scheduler::notify() {
    // Just in case if the scheduler is slept, attempt to wake it up.
    while (process_cycle_paused) {
        process_cycle_pause_cv.notify();
        // Abandon the time slice of the underlying thread to other OS threads, especially the thread running the
        // scheduler, to run.
        os::Thread::static_sleep(0);
    }
}

VMThread &Scheduler::idle_thread() {
    memory::TArenaIterator<VMThread> iterator(*this);
    VMThread *current = iterator.next();
    while (current != nullptr) {
        if (current->is_idle()) return *current;
        current = iterator.next();
    }

    current = this->memory::TArena<VMThread>::allocate();
    new(current) VMThread();

    return *current;
}

VMThread::VMThread() : idle(true), current_service_identifier(NULL_SERVICE_IDENTIFIER), signaled_interrupt(false),
                       thread_join_negotiated(false), self_return_task(*this) {}

bool VMThread::is_idle() const { return idle; }

void VMThread::host(VMService &service) {
    // Since this operation is protected by the single threaded nature of the scheduler task loop, the idle thread
    // retrieved will only be available for hosting the target_service, thus setting the idle flag here will not cause
    // another hosting request to mistake this thread as an idle thread.
    idle = false;
    // Reset the all thread states for a fresh start.
    signaled_interrupt.store(false);
    thread_join_negotiated = false;

    // The scheduler requires to access each running services via this link between the VMService and the VMThread,
    // which VMThread is a member of Scheduler. This have to be un-bind before the service completed its lifecycle.
    this->vm::HasMember<VMService>::bind(service);
    // The VMService needs to access some functionality of the VMThread, for example sleep. This have to be un-bind
    // before the service completed its lifecycle.
    service.vm::HasRoot<VMThread>::bind(*this);
    this->current_service_identifier = service.get_identifier(); // Set the current service identifier of this thread.
    this->embedded_os_thread.start(service); // Start the service with the embedded os thread.
}

bool VMThread::sleep(uint32 milliseconds) {
    // Only a thread can sleep itself due to the implementation is based on condition variable, no other thread can
    // request the current thread to sleep unless having some handshake mechanism.
    // Allowing a thread to sleep another thread is also dangerous since the request thread have no information about
    // the safe-points of the thread to be slept, thus is easier to lead to deadlock or other related issues.
    VeilAssert(embedded_os_thread.id() == os::Thread::current_thread_id(), "Sleep invoked by another thread.");

    wake_handshake.tok(); // Just to make sure the state of wake_handshake is reset.

    // It is better to check for interrupt before sleeping. Since the thread is deemed to be 'killed', thus this sleep
    // is just prolonged its trivial existence. If the interrupt thread is joining with this thread, executing sleep
    // will have it waited for more time which is bad for performance.
    if (wake_handshake.tok()) return false;

    // The following logic if for the handling of spurious wakeup when blocking on the self_blocking_cv, this thread
    // must sleep for a period equals or more than the requested period. In case of a false alarm (not interrupted), we
    // should let this thread to block on self_blocking_cv again for the remaining time.
    uint64 now = os::current_time_milliseconds();
    auto time_left = static_cast<uint64>(milliseconds);
    while (time_left > 0) {
        // Checking if interrupted before executing the sleep, the check here need to handle if the blocking on the
        // self_blocking_cv is being notified on interrupt, which the sleep should be over.
        if (wake_handshake.tok()) return false;
        self_blocking_cv.wait_for(time_left);
        // Calculate the time elapsed in the blocking state.
        uint64 diff = os::current_time_milliseconds() - now;
        // The value of time_left can be negative if subtract directly, since the type of time_left is unsigned,
        // negative value means greater than 0 again, thus we have to set it to 0 if negative.
        time_left = milliseconds > diff ? milliseconds - diff : 0;
    }

    return true; // This shows that the thread have completed the sleeping period without being interrupted.
}

void VMThread::wake() {
    // No need to check whether the thread is idle since the states mutated here will be reset upon other
    // actions which uses these states.
    wake_handshake.tik();
    self_blocking_cv.notify();
}

void VMThread::interrupt() {
    // No need to check whether the thread is idle since the states mutated here will be reset upon other
    // actions which uses these states.
    signaled_interrupt.store(true);
    wake();
}

bool VMThread::check_if_interrupted() { return signaled_interrupt.load(); }

bool VMThread::request_pause(uint32 wait_milliseconds) {
    VeilAssert(!idle, "Attempt to pause an idle thread.");

    if (!pause_handshake.tik()) return false;
    // A pause request should wake the thread from sleep state, this will make the sleeping period not guaranteed.
    wake();
    uint64 now = os::current_time_milliseconds();
    auto time_left = static_cast<uint64>(wait_milliseconds);
    while (time_left > 0) {
        requester_waiting_cv.wait_for(time_left);
        // Since in the previous action we have 'tik-ed' the thread.pause_handshake, if it is tik again it means the
        // thread have received the request
        if (pause_handshake.is_tik()) return true;
        // Calculate the time elapsed in the waiting state.
        uint64 diff = os::current_time_milliseconds() - now;
        // The value of time_left can be negative if subtract directly, since the type of time_left is unsigned,
        // negative value means greater than 0 again, thus we have to set it to 0 if negative.
        time_left = wait_milliseconds > diff ? wait_milliseconds - diff : 0;
    }
    return false;
}

void VMThread::resume() {
    VeilAssert(!idle, "Attempt to resume an idle thread.");

    if (pause_handshake.is_tok() && !resume_handshake.tik()) return;
    while (pause_handshake.is_tok() && !resume_handshake.is_tik()) {
        self_blocking_cv.notify();
        os::Thread::static_sleep(0);
    }
}

void VMThread::pause_if_requested() {
    if (!pause_handshake.tok()) return;
    requester_waiting_cv.notify();
    while (!resume_handshake.tok()) self_blocking_cv.wait();
}

void VMService::execute() {
    global_vm_service_table.put(os::Thread::current_thread_id(), *this);
    run();
    global_vm_service_table.remove(os::Thread::current_thread_id());

    // NOTE: The service will be completed by the following means:
    // - The service is interrupted and after proper wrap-up process, the run() method of the service returns thus
    //   ending the service's lifecycle.
    // - The service is gracefully died the run() method of the service returns thus ending the service's lifecycle.

    Scheduler *scheduler = this->vm::HasRoot<Scheduler>::get();

    // Unbind the root thread just for the sake of tidiness, since we cannot determine whether the owner of this service
    // will reuse this service structure.
    VMThread *host_thread = this->vm::HasRoot<VMThread>::get();
    this->vm::HasRoot<VMThread>::unbind();

    // If the scheduler is being terminated, there is no need to start a ThreadReturnTask as there will not be a new
    // service being spawned.
    if (scheduler->is_terminated()) return;
    scheduler->add_realtime_task(host_thread->self_return_task);
    scheduler->notify();

    // The current service's lifecycle ends gracefully here.
}

VMService::VMService(const std::string &name) : vm::HasName("Service:" + name) {
    // Retrieve the identifier which is unique within the entire process.
    identifier = global_vm_service_id_distribution.fetch_add(1);
}

uint64 VMService::get_identifier() const { return this->identifier; }

VMService::~VMService() = default;

Scheduler::StartServiceTask::StartServiceTask(VMService &target_service) : target_service(&target_service) {}

void Scheduler::StartServiceTask::run() {
    Scheduler *scheduler = this->vm::HasRoot<Scheduler>::get();

    target_service->vm::HasRoot<Scheduler>::bind(*scheduler);

    scheduler->idle_thread().host(*target_service);
}

Scheduler::ThreadReturnTask::ThreadReturnTask(VMThread &target_thread) : target_thread(&target_thread) {}

void Scheduler::ThreadReturnTask::run() {
    target_thread->vm::HasMember<VMService>::unbind();

    target_thread->embedded_os_thread.join();
    target_thread->idle = true;
}

Scheduler::ThreadPauseTask::ThreadPauseTask(VMThread &target_thread) : target_thread(&target_thread) {}

void Scheduler::ThreadPauseTask::run() {
    if (!target_thread->request_pause(config::pause_request_wait_milliseconds)) {
        std::string service_name = target_thread->vm::HasMember<VMService>::get()->get_name();
        veil::force_exit_on_error("Pausing thread of (" + service_name + ") takes too long...", VeilGetLineInfo);
    }
}

Scheduler::ThreadResumeTask::ThreadResumeTask(VMThread &target_thread) : target_thread(&target_thread) {}

void Scheduler::ThreadResumeTask::run() { target_thread->resume(); }

VMService &veil::threading::current_service() {
    VMService *service = global_vm_service_table.get(os::Thread::current_thread_id());
    VeilAssert(service != nullptr,
               "Failed to get current service from thread identifier:" +
               std::to_string(os::Thread::current_thread_id()));
    return *service;
}
