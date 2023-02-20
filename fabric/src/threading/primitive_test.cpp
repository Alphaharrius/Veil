#include <iostream>

#include "src/threading/os.hpp"

using namespace veil::threading;

class Function : public veil::vm::Callable {
public:
    explicit Function(uint32 id, veil::os::Mutex *mutex) : id(id), mutex(mutex) {}

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

int main() {
    veil::os::Mutex mutex;
    std::cout << "Begin test on thread primitives." << std::endl;
    {
        Function func_0(0, &mutex);
        Function func_1(1, &mutex);
        Function func_2(2, &mutex);

        uint32 error;

        veil::os::Thread thread_0;
        veil::os::Thread thread_1;
        veil::os::Thread thread_2;
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

    return 0;
}
