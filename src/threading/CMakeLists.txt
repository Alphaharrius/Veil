set(CMAKE_CXX_STANDARD 17)

add_executable(
        threading_queue_test
        src/threading/queue_test.cpp
        src/typedefs.hpp
        src/vm/resource.hpp
        src/vm/structures.hpp
        src/threading/queue.hpp src/threading/queue.cpp
        src/memory/memory.hpp src/memory/memory.cpp
        src/os.hpp src/os.cpp
        src/vm/details.hpp)