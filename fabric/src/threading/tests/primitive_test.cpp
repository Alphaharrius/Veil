#include <iostream>

#include "src/threading/os.hpp"

using namespace veil::threading;

class LockFunction : public veil::vm::Callable {
public:
    explicit LockFunction(uint32 id, veil::os::Mutex *mutex) : id(id), mutex(mutex) {}

    void run() override {
        for (int i = 0; i < 3; i++) {
            this->mutex->lock();
            std::cout << "Id: " << this->id << std::endl;
            veil::os::Thread::sleep(500);
            this->mutex->unlock();
            veil::os::Thread::sleep(2);
        }
    }

private:
    uint32 id;
    veil::os::Mutex *mutex;
};

class WaitFunction : public veil::vm::Callable {
public:
    explicit WaitFunction(uint32 id, veil::os::ConditionVariable *cv): id(id), cv(cv) {}

    void run() override {
        bool notified = cv->wait_for(3000);
        if (notified)
            std::cout << "This function have been notified: " << id << std::endl;
        else
            std::cout << "This function is awaken: " << id << std::endl;
    }

private:
    uint32 id;
    veil::os::ConditionVariable *cv;
};

class NotifyFunction : public veil::vm::Callable {
public:
    explicit NotifyFunction(veil::os::ConditionVariable *cv): cv(cv) {}

    void run() override {
        veil::os::Thread::sleep(1000);
        cv->notify_all();
    }

private:
    veil::os::ConditionVariable *cv;
};

int main() {
    veil::os::Thread thread_0;
    veil::os::Thread thread_1;
    veil::os::Thread thread_2;

    veil::os::Mutex mutex;
    std::cout << "Begin test on mutex." << std::endl;
    {
        LockFunction func_0(0, &mutex);
        LockFunction func_1(1, &mutex);
        LockFunction func_2(2, &mutex);

        uint32 error;
        thread_0.start(func_0, error);
        if (error != veil::ERR_NONE) {
            std::cerr << "Thread start failed: " << error << std::endl;
        }
        thread_1.start(func_1, error);
        if (error != veil::ERR_NONE) {
            std::cerr << "Thread start failed: " << error << std::endl;
        }
        thread_2.start(func_2, error);
        if (error != veil::ERR_NONE) {
            std::cerr << "Thread start failed: " << error << std::endl;
        }
        thread_0.join(error);
        if (error != veil::ERR_NONE) {
            std::cerr << "Thread join failed: " << error << std::endl;
        }
        thread_1.join(error);
        if (error != veil::ERR_NONE) {
            std::cerr << "Thread join failed: " << error << std::endl;
        }
        thread_2.join(error);
        if (error != veil::ERR_NONE) {
            std::cerr << "Thread join failed: " << error << std::endl;
        }
    }
    std::cout << "Done!" << std::endl;
    std::cout << "Begin test on condition variable." << std::endl;
    veil::os::ConditionVariable condition_variable;
    {
        uint32 error;

        NotifyFunction notify_function(&condition_variable);
        WaitFunction wait_function_0(0, &condition_variable);
        WaitFunction wait_function_1(1, &condition_variable);
        WaitFunction wait_function_2(2, &condition_variable);

        veil::os::Thread notify_thread;

        thread_0.start(wait_function_0, error);
        if (error != veil::ERR_NONE) {
            std::cerr << "Thread start failed: " << error << std::endl;
        }
        thread_1.start(wait_function_1, error);
        if (error != veil::ERR_NONE) {
            std::cerr << "Thread start failed: " << error << std::endl;
        }
        thread_2.start(wait_function_2, error);
        if (error != veil::ERR_NONE) {
            std::cerr << "Thread start failed: " << error << std::endl;
        }
        notify_thread.start(notify_function, error);
        if (error != veil::ERR_NONE) {
            std::cerr << "Thread start failed: " << error << std::endl;
        }
        thread_0.join(error);
        if (error != veil::ERR_NONE) {
            std::cerr << "Thread join failed: " << error << std::endl;
        }
        thread_1.join(error);
        if (error != veil::ERR_NONE) {
            std::cerr << "Thread join failed: " << error << std::endl;
        }
        thread_2.join(error);
        if (error != veil::ERR_NONE) {
            std::cerr << "Thread join failed: " << error << std::endl;
        }
        notify_thread.join(error);
        if (error != veil::ERR_NONE) {
            std::cerr << "Thread join failed: " << error << std::endl;
        }
    }

    return 0;
}
