#include <iostream>
#include <utility>

#include "src/threading/scheduler.hpp"

using namespace veil::threading;

class Task : public ScheduledTask {
public:
    std::string name;

    explicit Task(std::string name) : name(std::move(name)) {}

    void run() override {
        std::cout << "Executed task: " << name << std::endl;
    }
};

int main() {
    Scheduler scheduler;

    for (int i = 0; i < 10; i++) {
        Task *task = new Task("Task-" + std::to_string(i));
        task->wait_for_completion(false);
        scheduler.add_task(*task);
    }

    scheduler.start();

    return 0;
}
