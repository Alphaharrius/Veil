#include <iostream>
#include <thread>

#include "src/core/runtime.hpp"
#include "src/management.hpp"

using namespace veil::threading;

class PrintNameAtIntervalTask : public VMThread {
public:
    PrintNameAtIntervalTask(std::string name, veil::Runtime &runtime) : VMThread(name, runtime) {}

    void run() override {
        for (int i = 0; i < 5; i++) {
            std::cout << "my name is " << get_name() << " and my secret is: " << secret << num << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

private:
    std::string secret = "I am a test with no=";
    int num = 123;
};

int main() {
//    veil::Runtime runtime;
//    auto *task = new PrintNameAtIntervalTask("TaskABC", runtime);
//    task->start();
//    task->join();
//    std::cout << "Done!" << std::endl;
//    delete task;
}
