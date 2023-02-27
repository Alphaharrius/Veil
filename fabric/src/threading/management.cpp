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

VMThread::BlockingAgent::BlockingAgent() : signal_wake(false) {}

VMThread::PauseAgent::PauseAgent() : signal_pause(false), paused(false), signal_resume(false) {}

VMThread::VMThread(Management &management) : vm::Constituent<Management>(management), interrupted(false) {}

void VMThread::start(VMService &service, uint32 &error) {
    service.bind(*this);
    embedded.start(service, error);
}

void VMThread::join(uint32 &error) {
    embedded.join(error);
}

void VMThread::pause(uint32 &error) {
    // Pause should be requested by another thread.
    VeilAssert(embedded.id() != os::Thread::current_thread_id(), "Attempt to pause itself.");

    PauseAgent &agent = this->pause_agent;
    // One pause request at a time, and should not be happened simultaneously with resume action.
    agent.caller_m.lock();
    // Signal the pause and wait until this thread block.
    agent.signal_pause.store(true);
    while (!agent.paused.load())
        agent.caller_cv.wait();
    // Reset the signal.
    agent.signal_pause.store(false);
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

    PauseAgent &agent = this->pause_agent;
    // One resume action at a time, and should not be happened simultaneously with a pause action.
    agent.caller_m.lock();
    // Signal the thread to resume.
    agent.signal_resume.store(true);
    // Wake the paused thread until it awakes.
    while (agent.paused.load()) wake();
    // Reset the resume signal.
    agent.signal_resume.store(false);
    agent.caller_m.unlock();
}

void VMThread::interrupt() {
    // Signal the interrupt.
    interrupted.store(true);
    // An interrupt request is a no-blocking action, the requester who wants to wait until the current thread joins
    // should call VMThread::join() after this method.
    wake();
}

bool VMThread::check_interrupt() {
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
        // VMThread::signal_wake only works when called after the thread started blocking on the sleep_cv.
        if (agent.signal_wake.load()) {
            error = ERR_INTERRUPT;
            break;
        }
        time_left = milliseconds - (os::current_time_milliseconds() - now);
    }
}

void VMThread::wake() {
    // A sleeping thread cannot signal_wake itself, or a non-slept thread should not signal_wake itself.
    VeilAssert(embedded.id() != os::Thread::current_thread_id(), "Attempt to signal_wake the current thread itself.");

    BlockingAgent &agent = this->blocking_agent;
    agent.signal_wake.store(true);
    agent.blocking_cv.notify();
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
