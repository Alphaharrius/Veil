cmake_minimum_required(VERSION 3.22)
project(Veil)

set(CMAKE_CXX_STANDARD 17)
include_directories("src")
add_executable(
        Veil
        main.cpp
        src/typedefs.hpp
        src/vm/structures.hpp src/vm/structures.cpp
        src/memory/management.hpp src/memory/management.cpp
        src/os.hpp src/os.cpp
        src/vm/resource.hpp
        src/errors.hpp
        src/core/runtime.cpp src/core/runtime.h
        src/threading/queue.cpp src/threading/queue.hpp
        src/memory/memory.hpp src/memory/memory.cpp
        src/threading/os.cpp src/threading/os.hpp
        src/threading/management.cpp src/threading/management.hpp)

add_executable(
        memory_memory_test
        src/memory/memory_test.cpp
        src/typedefs.hpp
        src/vm/structures.hpp src/vm/structures.cpp
        src/memory/management.hpp src/memory/management.cpp
        src/os.hpp src/os.cpp
        src/vm/resource.hpp
        src/errors.hpp
        src/core/runtime.cpp src/core/runtime.h
        src/threading/queue.cpp src/threading/queue.hpp
        src/memory/memory.hpp src/memory/memory.cpp
        src/threading/os.cpp src/threading/os.hpp
        src/threading/management.cpp src/threading/management.hpp)

add_executable(
        threading_queue_test
        src/threading/queue_test.cpp
        src/typedefs.hpp
        src/vm/structures.hpp src/vm/structures.cpp
        src/memory/management.hpp src/memory/management.cpp
        src/os.hpp src/os.cpp
        src/vm/resource.hpp
        src/errors.hpp
        src/core/runtime.cpp src/core/runtime.h
        src/threading/queue.cpp src/threading/queue.hpp
        src/memory/memory.hpp src/memory/memory.cpp
        src/threading/os.cpp src/threading/os.hpp
        src/threading/management.cpp src/threading/management.hpp)

add_executable(
        vm_thread_test
        src/threading/vm_thread_test.cpp
        src/typedefs.hpp
        src/vm/structures.hpp src/vm/structures.cpp
        src/memory/management.hpp src/memory/management.cpp
        src/os.hpp src/os.cpp
        src/vm/resource.hpp
        src/errors.hpp
        src/core/runtime.cpp src/core/runtime.h
        src/threading/queue.cpp src/threading/queue.hpp
        src/memory/memory.hpp src/memory/memory.cpp
        src/threading/os.cpp src/threading/os.hpp
        src/threading/management.cpp src/threading/management.hpp)
