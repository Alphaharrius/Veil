#include <iostream>

#include "src/threading/management.hpp"

using namespace veil::threading;

class TestService : public VMService {
public:
    explicit TestService(std::string name) : VMService(name) {}

    void run() override {
        uint32 error;
        for (int i = 0; i < 5; i++) {
            std::cout << this->get_name() << std::endl;
            this->sleep(1000, error);
            if (error == ERR_INTERRUPT) {
                std::cout << "(" << this->get_name() << ")" << std::endl;
                break;
            }
            this->check_pause();
        }
        std::cout << "=" << this->get_name() << std::endl;
    }
};

class InterruptService : public VMService {
public:
    VMService *target;

    explicit InterruptService(std::string name, VMService *target) : VMService(name), target(target) {}

    void run() override {
        uint32 _;
        this->sleep(1500, _);
        this->check_pause();
        std::cout << "Interrupting..." << std::endl;
        target->interrupt();
        std::cout << "Thread finished: " << this->get_name() << std::endl;
    }
};

class PauseService : public VMService {
public:
    Management *target;

    explicit PauseService(std::string name, Management *target) : VMService(name), target(target) {}

    void run() override {
        uint32 _;
        this->sleep(4500, _);
        std::cout << "|" << std::endl;
        target->pause_all();
        std::cout << "[]" << std::endl;
        this->sleep(5000, _);
        std::cout << "|" << std::endl;
        target->resume_all();
        std::cout << "=|" << std::endl;
    }
};

int main() {
    Management management;
    TestService service_1("1");
//    InterruptService interrupt_service("interrupt", &service_1);
    PauseService pause_service("pause", &management);
    TestService service_2("2");
    TestService service_3("3");
    TestService service_4("4");
    management.start(pause_service);
    management.start(service_1);
//    management.start(interrupt_service);
    management.start(service_2);
    management.start(service_3);
    management.start(service_4);
    service_1.join();
    pause_service.join();
//    interrupt_service.join();
    service_2.join();
    service_3.join();
    service_4.join();
}
