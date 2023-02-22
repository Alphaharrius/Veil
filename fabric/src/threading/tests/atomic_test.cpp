#include <iostream>

#include "src/threading/os.hpp"

using namespace veil::os;

struct Object {
    int x = 0;
    int y = 0;
};

int main() {
    auto *obj = new Object();

    std::cout << "Initial address: 0x" << std::hex << reinterpret_cast<uint64>(obj) << std::endl;

    atomic_ptr<Object> a_ptr(obj);

    std::cout << "Address loaded: 0x" << std::hex << reinterpret_cast<uint64>(a_ptr.load()) << std::endl;

    std::cout << "Address compare-exchanged: 0x" << std::hex
              << reinterpret_cast<uint64>(a_ptr.compare_exchange(nullptr, obj)) << std::endl;

    std::cout << "Address exchanged: 0x" << std::hex << reinterpret_cast<uint64>(a_ptr.exchange(nullptr)) << std::endl;

    std::cout << "Address compare-exchanged: 0x" << std::hex
              << reinterpret_cast<uint64>(a_ptr.compare_exchange(nullptr, obj)) << std::endl;

    delete obj;
}
