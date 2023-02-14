cmake_minimum_required(VERSION 3.22)
project(Veil)

set(CMAKE_CXX_STANDARD 17)
include_directories("src")
add_executable(
        Veil
        main.cpp
        src/typedefs.hpp
        src/vm/structures.hpp
        src/memory/memory.hpp src/memory/memory.cpp
        src/natives.hpp src/natives.cpp
        src/vm/resource.hpp
        src/errors.hpp
        src/runtime/runtime.cpp src/runtime/runtime.h
        src/threading/queue.cpp src/threading/queue.hpp
)

add_executable(
        threading_queue_test
        src/threading/queue_test.cpp
        src/typedefs.hpp
        src/vm/resource.hpp
        src/vm/structures.hpp
        src/threading/queue.hpp src/threading/queue.cpp
)
