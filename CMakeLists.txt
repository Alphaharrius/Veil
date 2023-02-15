cmake_minimum_required(VERSION 3.22)
project(Veil)

set(CMAKE_CXX_STANDARD 17)
include_directories("src")
add_executable(
        Veil
        main.cpp
        src/typedefs.hpp
        src/vm/structures.hpp
        src/memory/management.hpp src/memory/management.cpp
        src/os.hpp src/os.cpp
        src/vm/resource.hpp
        src/errors.hpp
        src/runtime/runtime.cpp src/runtime/runtime.h
        src/threading/queue.cpp src/threading/queue.hpp
        src/memory/memory.hpp src/memory/memory.cpp src/memory/memory_test.cpp)

include(src/threading/CMakeLists.txt)
include(src/memory/CMakeLists.txt)
