set(CMAKE_CXX_STANDARD 17)

add_executable(
        threading_queue_test
        src/threading/queue_test.cpp
        src/typedefs.hpp
        src/vm/structures.hpp
        src/memory/management.hpp src/memory/management.cpp
        src/os.hpp src/os.cpp
        src/vm/resource.hpp
        src/errors.hpp
        src/core/runtime.cpp src/core/runtime.h
        src/threading/queue.cpp src/threading/queue.hpp
        src/memory/memory.hpp src/memory/memory.cpp)