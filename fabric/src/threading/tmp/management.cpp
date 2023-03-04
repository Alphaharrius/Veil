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

#include <iostream>
#include "src/threading/management.hpp"
#include "src/core/runtime.hpp"
#include "src/vm/os.hpp"

using namespace veil::threading;

LockFreeState::LockFreeState() : flag(CLOSED) {}

void LockFreeState::open() { flag.store(TICK); }

bool LockFreeState::tick() { return TICK == flag.compare_exchange(TICK, TOK); }

bool LockFreeState::tok() { return TOK == flag.compare_exchange(TOK, TICK); }

bool LockFreeState::is_tick() { return TICK == flag.load(); }

bool LockFreeState::is_tok() { return TOK == flag.load(); }

bool LockFreeState::close() {
    uint32 ret = flag.compare_exchange(TICK, CLOSED);
    return TICK == ret;
}

VMService::VMService(std::string &name) : HasName(name), completed(false) {}

void VMService::execute() {
    join_state.open();
    this->run();

    // Mark the service as completed, so that the join requester waiting on the join_cv can confirm of wake.
    this->completed = true;
    // Check if there are thread joining with this thread, if so wake the joining thread until it wakes.
    while (!join_state.close())
        join_cv.notify();

    // The service is finished by now, start cleaning up.
    auto *thread = this->vm::HasRoot<VMThread>::get();

    // Unlink the thread and the service.
    this->unbind();
    thread->vm::HasMember<VMService>::unbind();

    thread->finalize();
}

void VMService::sleep(uint32 milliseconds, uint32 &error) {
    this->vm::HasRoot<VMThread>::get()->sleep(milliseconds, error);
}

void VMService::interrupt() {
    this->vm::HasRoot<VMThread>::get()->interrupt();
}

void VMService::join() {
    // If the service is not completed, it can be joined.
    while (!completed &&
           // Tick the join_state so that this service knows there are someone request to join with it.
           !join_state.tick()) {
        os::Thread::static_sleep(0);
    }

    // Wait until the service is completed.
    while (!completed) join_cv.wait();
    join_state.tok();
}

void VMService::pause() {
    this->vm::HasRoot<VMThread>::get()->pause();
}

void VMService::resume() {
    this->vm::HasRoot<VMThread>::get()->resume();
}

bool VMService::check_interrupt() {
    return this->vm::HasRoot<VMThread>::get()->check_interrupt();
}

void VMService::check_pause() {
    this->vm::HasRoot<VMThread>::get()->check_pause();
}

VMThread::BlockingAgent::BlockingAgent() : signal_wake(false) {}

VMThread::PauseAgent::PauseAgent() = default;

VMThread::VMThread(Management &management) :
        vm::HasRoot<Management>(management), idle(true), interrupted(false) {}

bool VMThread::start(VMService &service) {
    if (!idle.exchange(false)) return false;

    this->vm::HasMember<VMService>::bind(service);
    service.bind(*this);
    PauseAgent &agent = this->pause_agent;
    // Open the states for pause so that pause request can be placed.
    agent.pause_state.open();
    agent.resume_state.open();

    embedded.start(service);

    return true;
}

void VMThread::join() {
    embedded.join();
}

void VMThread::finalize() {
    // Performing final clean up.
    BlockingAgent &b_agent = this->blocking_agent;
    PauseAgent &p_agent = this->pause_agent;

    // Clearing all pause request.
    while (!p_agent.pause_state.close()) {
        p_agent.pause_state.tok();
        p_agent.caller_cv.notify();
    }

    // Clearing all resume request.
    while (!p_agent.resume_state.close()) p_agent.resume_state.tok();

    // Ensure all pause & resume request have exited their corresponding method.
    p_agent.caller_m.lock();
    p_agent.caller_m.unlock();

    // Reset to default.
    b_agent.signal_wake.store(false);
    interrupted.store(false);

    // Mark this thread construct as state idle, after this the thread construct can be reused by the thread management
    // to host a new VMService.
    idle.store(true);
}

void VMThread::signal_pause() {
    // Pause should be requested by another thread.
    if (embedded.id() == os::Thread::current_thread_id() || idle.load())
        return;

    // This call is
    this->pause_agent.pause_state.tick();
}

void VMThread::wait_pause() {
    // Pause should be requested by another thread.
    if (embedded.id() == os::Thread::current_thread_id() || idle.load())
        return;

    PauseAgent &agent = this->pause_agent;
    agent.caller_m.lock();
    while (agent.pause_state.is_tok()) agent.caller_cv.wait();
    agent.caller_m.unlock();
}

void VMThread::pause() {
    // Pause should be requested by another thread.
    if (embedded.id() == os::Thread::current_thread_id() || idle.load())
        return;

    PauseAgent &agent = this->pause_agent;
    // Early bird exit.
    if (!agent.pause_state.is_tick()) return;

    // One request at a time, and should not be happened simultaneously with resume action.
    agent.caller_m.lock();

    if (agent.pause_state.tick())
        while (agent.pause_state.is_tok()) agent.caller_cv.wait();

    agent.caller_m.unlock();
}

void VMThread::check_pause() {
    PauseAgent &agent = this->pause_agent;
    // Check if the signal is raised, do nothing if false.
    if (!agent.pause_state.tok()) return;
    // Signal the pause requester that the thread is paused.
    // Wake the pause requester thread.
    agent.caller_cv.notify();

    uint32 _; // The only error ERR_INTERRUPT is ignored since a paused thread cannot be waked up by VMThread::wake().
    while (!agent.resume_state.tok()) block(_);
}

void VMThread::signal_resume() {
    // Resume should be requested by another thread.
    if (embedded.id() == os::Thread::current_thread_id() || idle.load())
        return;

//    this->pause_agent.ca
}

void VMThread::resume() {
    // Resume should be requested by another thread.
    if (embedded.id() == os::Thread::current_thread_id() || idle.load())
        return;

    PauseAgent &p_agent = this->pause_agent;
    // One request at a time, and should not be happened simultaneously with pause action.
    p_agent.caller_m.lock();

    BlockingAgent &b_agent = this->blocking_agent;
    // TODO: Add some return to indicate whether the thread is idle or there is another ongoing resume.
    if (p_agent.resume_state.tick()) {
        b_agent.signal_wake.store(true);
        while (p_agent.resume_state.is_tok()) b_agent.blocking_cv.notify();
    }

    p_agent.caller_m.unlock();
}

void VMThread::interrupt() {
    // TODO: Need a new implementation.
    // A sleeping thread cannot interrupt itself, or a non-slept thread should not interrupt itself.
    VeilAssert(embedded.id() != os::Thread::current_thread_id(), "Attempt to wake the current thread itself.");

    // Signal the interrupt.
    interrupted.store(true);
    // An interrupt request is a no-blocking action, the requester who wants to wait until the current thread joins
    // should call VMThread::join() after this method.
    BlockingAgent &agent = this->blocking_agent;
    agent.signal_wake.store(true);
    agent.blocking_cv.notify();
}

bool VMThread::check_interrupt() {
    // Since interrupt is an end-of-life request for the thread, this flag will not be finalize here.
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
    bool started = false;
    while (current != nullptr) {
        if (current->start(service)) {
            goto STARTED;
        }
        current = iterator.next();
    }

    // If there are no available thread, allocate & construct a new one within the TArena<VMThread>.
    current = this->memory::TArena<VMThread>::allocate();
    new(current) VMThread(*this);
    current->start(service);

    STARTED:
    thread_arena_m.unlock();
}

void Management::pause_all() {
    memory::TArenaIterator<VMThread> signal_iterator(*this);
    memory::TArenaIterator<VMThread> wait_iterator(*this);
    thread_arena_m.lock();
    VMThread *current = signal_iterator.next();
    while (current != nullptr) {
        current->signal_pause();
        current = signal_iterator.next();
    }
    current = wait_iterator.next();
    while (current != nullptr) {
        current->wait_pause();
        current = wait_iterator.next();
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
