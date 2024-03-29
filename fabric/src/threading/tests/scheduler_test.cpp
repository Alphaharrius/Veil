#include <iostream>

#include "src/threading/scheduler.hpp"
#include "src/vm/structures.hpp"

using namespace veil::threading;

class TestService : public VMService {
public:
    uint32 wait_for;

    explicit TestService(const std::string& name, uint32 wait_for) : VMService(name), wait_for(wait_for) {}

    void run() override {
        std::cout << "Service started(" + get_name() + ")...\n";
        veil::os::Thread::static_sleep(wait_for);
        std::cout << "Service ended(" + get_name() + ").\n";

        this->veil::vm::HasRoot<Scheduler>::root()->terminate();
    }
};

int main() {
    Scheduler scheduler;

    TestService service_0("test-service-0", 1000);
    Scheduler::StartServiceTask task_0(service_0);
    TestService service_1("test-service-1", 3000);
    Scheduler::StartServiceTask task_1(service_1);
    scheduler.add_task(task_0);
    scheduler.add_task(task_1);

    scheduler.start();

    return 0;
}
