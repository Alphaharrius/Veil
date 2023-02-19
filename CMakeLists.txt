cmake_minimum_required(VERSION 3.22)
project(Veil)

set(CMAKE_CXX_STANDARD 17)
include_directories("fabric")
add_executable(
        Veil
        main.cpp
        fabric/src/typedefs.hpp
        fabric/src/vm/structures.hpp fabric/src/vm/structures.cpp
        fabric/src/memory/management.hpp fabric/src/memory/management.cpp
        fabric/src/vm/os.hpp fabric/src/vm/os.cpp
        fabric/src/vm/resource.hpp
        fabric/src/vm/errors.hpp
        fabric/src/threading/queue.cpp fabric/src/threading/queue.hpp
        fabric/src/memory/global.hpp fabric/src/memory/global.cpp
        fabric/src/threading/os.cpp fabric/src/threading/os.hpp
        fabric/src/threading/management.cpp fabric/src/threading/management.hpp
        fabric/src/core/runtime.cpp fabric/src/core/runtime.hpp fabric/src/core/runtime.forward.hpp fabric/src/memory/os.cpp fabric/src/memory/os.hpp)

add_executable(
        memory_memory_test
        fabric/src/memory/memory_test.cpp
        fabric/src/typedefs.hpp
        fabric/src/vm/structures.hpp fabric/src/vm/structures.cpp
        fabric/src/memory/management.hpp fabric/src/memory/management.cpp
        fabric/src/vm/os.hpp fabric/src/vm/os.cpp
        fabric/src/vm/resource.hpp
        fabric/src/vm/errors.hpp
        fabric/src/threading/queue.cpp fabric/src/threading/queue.hpp
        fabric/src/memory/global.hpp fabric/src/memory/global.cpp
        fabric/src/threading/os.cpp fabric/src/threading/os.hpp
        fabric/src/threading/management.cpp fabric/src/threading/management.hpp
        fabric/src/core/runtime.cpp fabric/src/core/runtime.hpp fabric/src/core/runtime.forward.hpp fabric/src/memory/os.cpp fabric/src/memory/os.hpp)

add_executable(
        threading_queue_test
        fabric/src/threading/queue_test.cpp
        fabric/src/typedefs.hpp
        fabric/src/vm/structures.hpp fabric/src/vm/structures.cpp
        fabric/src/memory/management.hpp fabric/src/memory/management.cpp
        fabric/src/vm/os.hpp fabric/src/vm/os.cpp
        fabric/src/vm/resource.hpp
        fabric/src/vm/errors.hpp
        fabric/src/threading/queue.cpp fabric/src/threading/queue.hpp
        fabric/src/memory/global.hpp fabric/src/memory/global.cpp
        fabric/src/threading/os.cpp fabric/src/threading/os.hpp
        fabric/src/threading/management.cpp fabric/src/threading/management.hpp
        fabric/src/core/runtime.cpp fabric/src/core/runtime.hpp fabric/src/core/runtime.forward.hpp fabric/src/memory/os.cpp fabric/src/memory/os.hpp)

add_executable(
        vm_thread_test
        fabric/src/threading/vm_thread_test.cpp
        fabric/src/typedefs.hpp
        fabric/src/vm/structures.hpp fabric/src/vm/structures.cpp
        fabric/src/memory/management.hpp fabric/src/memory/management.cpp
        fabric/src/vm/os.hpp fabric/src/vm/os.cpp
        fabric/src/vm/resource.hpp
        fabric/src/vm/errors.hpp
        fabric/src/threading/queue.cpp fabric/src/threading/queue.hpp
        fabric/src/memory/global.hpp fabric/src/memory/global.cpp
        fabric/src/threading/os.cpp fabric/src/threading/os.hpp
        fabric/src/threading/management.cpp fabric/src/threading/management.hpp
        fabric/src/core/runtime.cpp fabric/src/core/runtime.hpp fabric/src/core/runtime.forward.hpp fabric/src/memory/os.cpp fabric/src/memory/os.hpp)

add_executable(
        thread_primitive_test
        fabric/src/threading/primitive_test.cpp
        fabric/src/typedefs.hpp
        fabric/src/vm/structures.hpp fabric/src/vm/structures.cpp
        fabric/src/memory/management.hpp fabric/src/memory/management.cpp
        fabric/src/vm/os.hpp fabric/src/vm/os.cpp
        fabric/src/vm/resource.hpp
        fabric/src/vm/errors.hpp
        fabric/src/threading/queue.cpp fabric/src/threading/queue.hpp
        fabric/src/memory/global.hpp fabric/src/memory/global.cpp
        fabric/src/threading/os.cpp fabric/src/threading/os.hpp
        fabric/src/threading/management.cpp fabric/src/threading/management.hpp
        fabric/src/core/runtime.cpp fabric/src/core/runtime.hpp fabric/src/core/runtime.forward.hpp fabric/src/memory/os.cpp fabric/src/memory/os.hpp)

add_executable(
        jit_virtual_test
        jit_virtual_test.cpp
)
