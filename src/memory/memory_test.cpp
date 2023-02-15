#include <iostream>

#include "memory.hpp"

using namespace veil::memory;

struct CustomObject {
    int index;

    explicit CustomObject(int index) : index(index) {}
};

int main() {
    const int OBJ_COUNT = 256;

    TArena<CustomObject> arena;

    for (int i = 0; i < OBJ_COUNT; i++) {
        CustomObject *obj = arena.allocate();
        new(obj) CustomObject(i);
    }

    TArenaIterator<CustomObject> iterator(arena);
    for (int i = 0; i < OBJ_COUNT; i++) {
        CustomObject *obj = iterator.next();
        std::cout << obj->index << std::endl;
    }

    arena.free();

    return 0;
}
