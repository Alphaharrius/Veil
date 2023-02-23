cmake_minimum_required(VERSION 3.22)
project(Veil)

set(CMAKE_CXX_STANDARD 17)
include_directories(fabric)

file(GLOB src
        fabric/src/*.hpp
        fabric/src/*.cpp)

file(GLOB src_core
        fabric/src/core/*.hpp
        fabric/src/core/*.cpp)

file(GLOB src_memory
        fabric/src/memory/*.hpp
        fabric/src/memory/*.cpp)

file(GLOB src_threading
        fabric/src/threading/*.hpp
        fabric/src/threading/*.cpp)

file(GLOB src_vm
        fabric/src/vm/*.hpp
        fabric/src/vm/*.cpp)

add_executable(
        ${PROJECT_NAME}
        main.cpp
        ${src} ${src_core} ${src_memory} ${src_threading} ${src_vm})

add_executable(
        memory_memory_test
        fabric/src/memory/tests/memory_test.cpp
        ${src} ${src_core} ${src_memory} ${src_threading} ${src_vm})

add_executable(
        threading_queue_test
        fabric/src/threading/tests/queue_test.cpp
        ${src} ${src_core} ${src_memory} ${src_threading} ${src_vm})

add_executable(
        vm_thread_test
        fabric/src/threading/tests/vm_thread_test.cpp
        ${src} ${src_core} ${src_memory} ${src_threading} ${src_vm})

add_executable(
        thread_primitive_test
        fabric/src/threading/tests/primitive_test.cpp
        ${src} ${src_core} ${src_memory} ${src_threading} ${src_vm})

add_executable(
        atomic_test
        fabric/src/threading/tests/atomic_test.cpp
        ${src} ${src_core} ${src_memory} ${src_threading} ${src_vm})
