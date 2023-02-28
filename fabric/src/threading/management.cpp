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

#include "src/threading/management.hpp"
#include "src/core/runtime.hpp"
#include "src/vm/os.hpp"

using namespace veil::threading;

VMService::VMService(std::string &name) : HasName(name), join_flag(false), completed(false) {}

void VMService::execute() {
    this->run();
    this->completed = true;

    // Check if there are thread joining with this thread.
    if (join_flag.exchange(true))
        // Wake the joining thread until it wakes.
        while (join_flag.load()) join_cv.notify();

    // The service is finished by now, start cleaning up.
    auto *thread = this->vm::Constituent<VMThread>::get_root();

    // Unlink the thread and the service.
    this->unbind();
    thread->vm::Composite<VMService>::unbind();

    thread->reset();
}

void VMService::sleep(uint32 milliseconds, uint32 &error) {
    this->vm::Constituent<VMThread>::get_root()->sleep(milliseconds, error);
}

void VMService::interrupt() {
    this->vm::Constituent<VMThread>::get_root()->interrupt();
}

void VMService::join() {
    if (join_flag.exchange(true)) return;

    while (!completed) join_cv.wait();

    // Signal this thread that the joining thread is awake.
    join_flag.store(false);
}

void VMService::pause() {
    this->vm::Constituent<VMThread>::get_root()->pause();
}

void VMService::resume() {
    this->vm::Constituent<VMThread>::get_root()->resume();
}

bool VMService::check_interrupt() {
    return this->vm::Constituent<VMThread>::get_root()->check_interrupt();
}

void VMService::check_pause() {
    this->vm::Constituent<VMThread>::get_root()->check_pause();
}

VMThread::BlockingAgent::BlockingAgent() : signal_wake(false) {}

VMThread::PauseAgent::PauseAgent() :
        caller_flag(false), signal_pause(false), paused(false), signal_resume(false) {}

VMThread::VMThread(Management &management) :
        vm::Constituent<Management>(management), idle(true), stopped(true), interrupted(false) {}

void VMThread::start(VMService &service) {
    idle.store(false);
    stopped.store(false);
    this->vm::Composite<VMService>::bind(service);
    service.bind(*this);
    embedded.start(service);
    stopped.store(true); // TODO: Use exchange of some flag to check if there are threads waiting for caller_cv.
}

void VMThread::join() {
    embedded.join();
}

void VMThread::reset() {
    // Performing final clean up.
    BlockingAgent &b_agent = this->blocking_agent;
    PauseAgent &p_agent = this->pause_agent;
    p_agent.caller_m.lock();

    // Notify whoever placed the pause request to prevent them from sleeping forever.
    p_agent.caller_cv.notify();

    // Reset all state to default.
    b_agent.signal_wake.store(false);

    p_agent.signal_pause.store(false);
    p_agent.paused.store(false);
    p_agent.signal_resume.store(false);

    interrupted.store(false);

    // Mark this thread construct as state idle.
    idle.store(true);
    p_agent.caller_m.unlock(); // A new service can be hosted by the thread again.
}

void VMThread::pause() { // TODO: Pause action should have bool return to indicate if its the first call.
    // Pause should be requested by another thread.
    if (embedded.id() == os::Thread::current_thread_id())
        return;

    // Early bird exit.
    if (stopped.load()) return; // TODO: Use exchange of some flag to check if pause is permitted.

    PauseAgent *agent = &this->pause_agent;
    // One pause request at a time, and should not be happened simultaneously with resume action.
    agent->caller_m.lock();

    // If the stopped flag is set, then the pause operation should not be continued since there will be no thread to
    // wake up the requesting thread.
    // NOTE: This line is placed behind agent.caller_m.lock() to ensure all pause requests are placed in either case:
    // -
    // Before the join (thread termination) operation, thus the agent.caller_cv.notify() in VMThread::join() can wake
    // the requester to prevent it to be slept forever.
    // -
    // After the join operation, this pause action will be cancelled.
    if (!stopped.load()) {
        // Signal the pause and wait until this thread block.
        agent->signal_pause.store(true);
        while (!stopped.load() && !agent->paused.load())
            agent->caller_cv.wait();
        // Reset the signal.
        agent->signal_pause.store(false);
    }

    agent->caller_m.unlock();
}

void VMThread::check_pause() {
    PauseAgent *agent = &this->pause_agent;
    // Check if the signal is raised, do nothing if false.
    if (!agent->signal_pause.load())
        return;
    // Signal the pause requester that the thread is paused.
    agent->paused.store(true);
    // Wake the pause requester thread.
    agent->caller_cv.notify();

    uint32 _; // The only error ERR_INTERRUPT is ignored since a paused thread cannot be waked up by VMThread::wake().
    while (!agent->signal_resume.load()) block(_);
    // Reset the pause state.
    agent->paused.store(false);
}

void VMThread::resume() {
    // Resume should be requested by another thread.
    if (embedded.id() == os::Thread::current_thread_id())
        return;

    // Early bird exit.
    if (stopped.load()) return;

    PauseAgent &p_agent = this->pause_agent;

    // Early bird exit.
    if (!p_agent.paused.load()) return;

    // One resume action at a time, and should not be happened simultaneously with a pause action.
    p_agent.caller_m.lock();

    // If the stopped flag is set, then the resume operation should not be continued since it will be terminated anyway.
    // NOTE: This line is placed behind agent.caller_m.lock() to ensure all pause requests are placed in either case:
    // -
    // Before the join (thread termination) operation, this operation will be continued and exit by the stopped check
    // within the busy-wake.
    // -
    // After the join operation, this pause action will be cancelled.
    if (!stopped.load()) {
        // Signal the thread to resume.
        p_agent.signal_resume.store(true);
        // Wake the paused thread until it awakes if and only if it is not stopped.
        BlockingAgent &b_agent = this->blocking_agent;
        while (!stopped.load() && p_agent.paused.load()) {
            b_agent.signal_wake.store(true);
            b_agent.blocking_cv.notify();
        }
        // Reset the resume signal.
        p_agent.signal_resume.store(false);
    }

    p_agent.caller_m.unlock();
}

void VMThread::interrupt() {
    // A sleeping thread cannot interrupt itself, or a non-slept thread should not interrupt itself.
    VeilAssert(embedded.id() != os::Thread::current_thread_id(), "Attempt to wake the current thread itself.");

    // Early bird exit.
    if (stopped.load()) return;

    PauseAgent &p_agent = this->pause_agent;
    // Ensures the interrupting thread knows the current state of this thread.
    p_agent.caller_m.lock();

    if (!stopped.load()) {
        // Signal the interrupt.
        interrupted.store(true);
        // An interrupt request is a no-blocking action, the requester who wants to wait until the current thread joins
        // should call VMThread::join() after this method.
        BlockingAgent &b_agent = this->blocking_agent;
        b_agent.signal_wake.store(true);
        b_agent.blocking_cv.notify();
    }

    p_agent.caller_m.unlock();
}

bool VMThread::check_interrupt() {
    // Since interrupt is an end-of-life request for the thread, this flag will not be reset here.
    return interrupted.load();
}

void VMThread::sleep(uint32 milliseconds, uint32 &error) {
    error = veil::ERR_NONE;

    // Sleep can only be done by the thread itself.
    VeilAssert(embedded.id() == os::Thread::current_thread_id(), "Attempt to sleep another thread.");

    BlockingAgent &agent = this->blocking_agent;
    agent.signal_wake.store(false);

    uint64 now = os::current_time_milliseconds();
    auto time_left = static_cast<uint64>(milliseconds);
    // Prevent spurious wakeup, if that happens the thread will sleep for the remaining time.
    while (time_left > 0) {
        agent.blocking_cv.wait_for(time_left);
        // VMThread::wake only works when called after the thread started blocking on the sleep_cv.
        if (agent.signal_wake.load()) {
            error = ERR_INTERRUPT;
            break;
        }
        uint64 diff = os::current_time_milliseconds() - now;
        time_left = milliseconds > diff ? milliseconds - diff : 0;
    }
}

void VMThread::block(uint32 &error) {
    error = veil::ERR_NONE;

    // Blocking can only be done by the thread itself.
    VeilAssert(embedded.id() == os::Thread::current_thread_id(), "Attempt to block another thread.");

    BlockingAgent &agent = this->blocking_agent;
    agent.signal_wake.store(false);

    while (true) {
        agent.blocking_cv.wait();
        // VMThread::signal_wake only works when called after the thread started blocking on the sleep_cv.
        if (agent.signal_wake.load()) {
            error = ERR_INTERRUPT;
            break;
        }
    }
}

void Management::start(VMService &service) {
    memory::TArenaIterator<VMThread> iterator(*this);
    // Cannot start more than one service simultaneously, because the TArena<VMThread> is not thread-safe.
    thread_arena_m.lock();
    // TODO: Is that possible to store joined / idle thread in an array list?
    VMThread *current = iterator.next();
    VMThread *found = nullptr;
    while (current != nullptr && found == nullptr) {
        // We would like to find an idle thread to start our new service.
        if (current->idle.load()) // TODO: use compare exchange.
            found = current;
        current = iterator.next();
    }

    // If there are no available thread, allocate & construct a new one within the TArena<VMThread>.
    if (found == nullptr) {
        found = this->memory::TArena<VMThread>::allocate();
        new(found) VMThread(*this);
    }

    found->start(service);
    thread_arena_m.unlock();
}

void Management::pause_all() {
    memory::TArenaIterator<VMThread> iterator(*this);
    thread_arena_m.lock();
    VMThread *current = iterator.next();
    while (current != nullptr) {
        current->pause();
        current = iterator.next();
    }
    thread_arena_m.unlock();
}

void Management::resume_all() {
    memory::TArenaIterator<VMThread> iterator(*this);
    thread_arena_m.lock();
    VMThread *current = iterator.next();
    while (current != nullptr) {
        current->resume();
        current = iterator.next();
    }
    thread_arena_m.unlock();
}
