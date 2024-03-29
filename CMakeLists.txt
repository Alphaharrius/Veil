cmake_minimum_required(VERSION 3.22)
project(Veil)

set(CMAKE_CXX_STANDARD 17)
include_directories(fabric)

add_compile_definitions(VEIL_ENABLE_DEBUG)
add_compile_definitions(VEIL_ENABLE_PROFILING)

file(GLOB fabric_src
        fabric/src/*.hpp
        fabric/src/core/*.hpp
        fabric/src/core/*.cpp
        fabric/src/memory/*.hpp
        fabric/src/memory/*.cpp
        fabric/src/threading/*.hpp
        fabric/src/threading/*.cpp
        fabric/src/vm/*.hpp
        fabric/src/vm/*.cpp
        fabric/src/util/*.hpp
        fabric/src/util/*.cpp)

add_executable(
        ${PROJECT_NAME}
        fabric/src/veil.cpp
        ${fabric_src})

add_executable(
        memory_memory_test
        fabric/src/memory/tests/memory_test.cpp
        ${fabric_src})

add_executable(
        threading_queue_test
        fabric/src/threading/tests/queue_test.cpp
        ${fabric_src})

add_executable(
        vm_thread_test
        fabric/src/threading/tests/vm_thread_test.cpp
        ${fabric_src})

add_executable(
        thread_primitive_test
        fabric/src/threading/tests/primitive_test.cpp
        ${fabric_src})

add_executable(
        atomic_test
        fabric/src/threading/tests/atomic_test.cpp
        ${fabric_src})

add_executable(
        threading_management_test
        fabric/src/threading/tests/management_test.cpp
        ${fabric_src})

add_executable(
        scheduler_test
        fabric/src/threading/tests/scheduler_test.cpp
        ${fabric_src})

add_executable(
        jit_test
        jit_virtual_test.cpp
)
