set(CMAKE_CXX_STANDARD 17)

add_executable(memory_test
        src/memory/memory_test.cpp
        src/memory/memory.hpp src/memory/memory.cpp

        src/os.hpp src/os.cpp
        )