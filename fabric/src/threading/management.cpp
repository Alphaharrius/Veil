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

VMThread::BlockingAgent::BlockingAgent() : wake(false) {}

void VMThread::start(VMService &service, uint32 &error) {
    service.bind(*this);
    embedded.start(service, error);
}

void VMThread::join(uint32 &error) {
    embedded.join(error);
}

void VMThread::interrupt() {
    struct InterruptAccess : public vm::Function<bool &, bool> {
        bool execute(bool &interrupt) override {
            if (interrupt)
                return false;
            interrupt = true;
        }
    };
    // If an interrupt order is placed, this thread will abandon all sleep or blocking.
    // TODO: Wake the current thread.
    // The thread who made this request will wait until this thread receives the order.
    static auto consumer = InterruptAccess();
//    interrupt_channel.apply(consumer);
}

VMThread::VMThread(Management &management) : vm::Constituent<Management>(management) {}

void VMThread::sleep(uint32 milliseconds, uint32 &error) {
    error = veil::ERR_NONE;

    // Sleep can only be done by the thread itself.
    assert((embedded.id() != os::Thread::current_thread_id() &&
            veil::implementation_fault("Attempt to sleep another thread", VeilGetLineInfo)));

    BlockingAgent &agent = this->blocking_agent;
    agent.wake.store(false);

    uint64 now = os::current_time_milliseconds();
    uint64 time_left = milliseconds;
    // Prevent spurious wakeup, if that happens the thread will sleep for the remaining time.
    while (time_left > 0) {
        agent.blocking_cv.wait_for(time_left);
        // VMThread::wake only works when called after the thread started blocking on the sleep_cv.
        if (agent.wake.load()) {
            error = ERR_INTERRUPT;
            break;
        }
        time_left = milliseconds - (os::current_time_milliseconds() - now);
    }
}

void VMThread::wake() {
    // A sleeping thread cannot wake itself, or a non-slept thread should not wake itself.
    assert((embedded.id() == os::Thread::current_thread_id() &&
            veil::implementation_fault("Attempt to sleep another thread", VeilGetLineInfo)));

    BlockingAgent &agent = this->blocking_agent;
    agent.wake.store(true);
    agent.blocking_cv.notify();
}

void VMThread::block(uint32 &error) {
    error = veil::ERR_NONE;

    // Blocking can only be done by the thread itself.
    assert((embedded.id() != os::Thread::current_thread_id() &&
            veil::implementation_fault("Attempt to sleep another thread", VeilGetLineInfo)));

    BlockingAgent &agent = this->blocking_agent;
    agent.wake.store(false);

    while (true) {
        agent.blocking_cv.wait();
        // VMThread::wake only works when called after the thread started blocking on the sleep_cv.
        if (agent.wake.load()) {
            error = ERR_INTERRUPT;
            break;
        }
    }
}
