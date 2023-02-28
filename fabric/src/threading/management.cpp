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

VMService::VMService(std::string &name) : HasName(name) {}

void VMService::interrupt() {
    this->vm::Constituent<VMThread>::get_root()->interrupt();
}

void VMService::join() {
    this->vm::Constituent<VMThread>::get_root()->join();
    this->unbind();
}

VMThread::BlockingAgent::BlockingAgent() : signal_wake(false) {}

VMThread::PauseAgent::PauseAgent() : signal_pause(false), paused(false), signal_resume(false) {}

VMThread::VMThread(Management &management) :
        idle(true), vm::Constituent<Management>(management), interrupted(false) {}

bool VMThread::start(VMService &service) {
    bool idle_state = idle.exchange(false);
    if (!idle_state) return false;

    service.bind(*this);
    embedded.start(service);
    return true;
}

void VMThread::join() {
    embedded.join();

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
    p_agent.caller_m.unlock();
}

void VMThread::pause() {
    // Pause should be requested by another thread.
    VeilAssert(embedded.id() != os::Thread::current_thread_id(), "Attempt to pause itself.");

    // Early bird exit.
    if (idle.load()) return;

    PauseAgent &agent = this->pause_agent;
    // One pause request at a time, and should not be happened simultaneously with resume action.
    agent.caller_m.lock();

    // If the idle flag is set, then the pause operation should not be continued since there will be no thread to wake
    // up the requesting thread.
    // NOTE: This line is placed behind agent.caller_m.lock() to ensure all pause requests are placed in either case:
    // -
    // Before the join (thread termination) operation, thus the agent.caller_cv.notify() in VMThread::join() can wake
    // the requester to prevent it to be slept forever.
    // -
    // After the join operation, this pause action will be cancelled.
    if (!idle.load()) {
        // Signal the pause and wait until this thread block.
        agent.signal_pause.store(true);
        while (!agent.paused.load())
            agent.caller_cv.wait();
        // Reset the signal.
        agent.signal_pause.store(false);
    }

    agent.caller_m.unlock();
}

void VMThread::check_pause() {
    PauseAgent &agent = this->pause_agent;
    // Check if the signal is raised, do nothing if false.
    if (!agent.signal_pause.load())
        return;
    // Signal the pause requester that the thread is paused.
    agent.paused.store(true);
    // Wake the pause requester thread.
    agent.caller_cv.notify();

    uint32 _; // The only error ERR_INTERRUPT is ignored since a paused thread cannot be waked up by VMThread::wake().
    while (!agent.signal_resume.load()) block(_);
    // Reset the pause state.
    agent.paused.store(false);
}

void VMThread::resume() {
    // Resume should be requested by another thread.
    VeilAssert(embedded.id() != os::Thread::current_thread_id(), "Attempt to resume itself.");

    // Early bird exit.
    if (idle.load()) return;

    PauseAgent &agent = this->pause_agent;
    // One resume action at a time, and should not be happened simultaneously with a pause action.
    agent.caller_m.lock();

    // If the idle flag is set, then the resume operation should not be continued since it will be terminated anyway.
    // NOTE: This line is placed behind agent.caller_m.lock() to ensure all pause requests are placed in either case:
    // -
    // Before the join (thread termination) operation, this operation will be continued and exit by the idle check
    // within the busy-wake.
    // -
    // After the join operation, this pause action will be cancelled.
    if (!idle.load()) {
        // Signal the thread to resume.
        agent.signal_resume.store(true);
        // Wake the paused thread until it awakes if and only if it is not idle.
        while (!idle.load() && agent.paused.load()) wake();
        // Reset the resume signal.
        agent.signal_resume.store(false);
    }

    agent.caller_m.unlock();
}

void VMThread::interrupt() {
    // A sleeping thread cannot interrupt itself, or a non-slept thread should not interrupt itself.
    VeilAssert(embedded.id() != os::Thread::current_thread_id(), "Attempt to wake the current thread itself.");

    // Early bird exit.
    if (idle.load()) return;

    PauseAgent &p_agent = this->pause_agent;
    // Ensures the interrupting thread knows the current state of this thread.
    p_agent.caller_m.lock();

    if (!idle.load()) {
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
    uint64 time_left = milliseconds;
    // Prevent spurious wakeup, if that happens the thread will sleep for the remaining time.
    while (time_left > 0) {
        agent.blocking_cv.wait_for(time_left);
        // VMThread::wake only works when called after the thread started blocking on the sleep_cv.
        if (agent.signal_wake.load()) {
            error = ERR_INTERRUPT;
            break;
        }
        time_left = milliseconds - (os::current_time_milliseconds() - now);
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

void Management::start_service(VMService &service) {
    memory::TArenaIterator<VMThread> iterator(*this);
    // Cannot start more than one service simultaneously, because the TArena<VMThread> is not thread-safe.
    thread_arena_m.lock();
    // TODO: Is that possible to store joined / idle thread in an array list?
    VMThread *current = iterator.next();
    VMThread *found = nullptr;
    while (current != nullptr && found == nullptr) {
        // We would like to find an idle thread to start our new service.
        if (current->idle.load())
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

void Management::global_pause() {
    memory::TArenaIterator<VMThread> iterator(*this);
    thread_arena_m.lock();
    VMThread *current = iterator.next();
    thread_arena_m.unlock();
}
