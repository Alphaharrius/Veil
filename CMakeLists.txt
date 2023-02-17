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
        fabric/src/os.hpp fabric/src/os.cpp
        fabric/src/vm/resource.hpp
        fabric/src/errors.hpp
        fabric/src/threading/queue.cpp fabric/src/threading/queue.hpp
        fabric/src/memory/memory.hpp fabric/src/memory/memory.cpp
        fabric/src/threading/os.cpp fabric/src/threading/os.hpp
        fabric/src/threading/management.cpp fabric/src/threading/management.hpp
        fabric/src/core/runtime.cpp fabric/src/core/runtime.hpp fabric/src/core/runtime.forward.hpp fabric/src/memory/os.cpp fabric/src/memory/os.hpp)

add_executable(
        memory_memory_test
        fabric/src/memory/memory_test.cpp
        fabric/src/typedefs.hpp
        fabric/src/vm/structures.hpp fabric/src/vm/structures.cpp
        fabric/src/memory/management.hpp fabric/src/memory/management.cpp
        fabric/src/os.hpp fabric/src/os.cpp
        fabric/src/vm/resource.hpp
        fabric/src/errors.hpp
        fabric/src/threading/queue.cpp fabric/src/threading/queue.hpp
        fabric/src/memory/memory.hpp fabric/src/memory/memory.cpp
        fabric/src/threading/os.cpp fabric/src/threading/os.hpp
        fabric/src/threading/management.cpp fabric/src/threading/management.hpp
        fabric/src/core/runtime.cpp fabric/src/core/runtime.hpp fabric/src/core/runtime.forward.hpp fabric/src/memory/os.cpp fabric/src/memory/os.hpp)

add_executable(
        threading_queue_test
        fabric/src/threading/queue_test.cpp
        fabric/src/typedefs.hpp
        fabric/src/vm/structures.hpp fabric/src/vm/structures.cpp
        fabric/src/memory/management.hpp fabric/src/memory/management.cpp
        fabric/src/os.hpp fabric/src/os.cpp
        fabric/src/vm/resource.hpp
        fabric/src/errors.hpp
        fabric/src/threading/queue.cpp fabric/src/threading/queue.hpp
        fabric/src/memory/memory.hpp fabric/src/memory/memory.cpp
        fabric/src/threading/os.cpp fabric/src/threading/os.hpp
        fabric/src/threading/management.cpp fabric/src/threading/management.hpp
        fabric/src/core/runtime.cpp fabric/src/core/runtime.hpp fabric/src/core/runtime.forward.hpp fabric/src/memory/os.cpp fabric/src/memory/os.hpp)

add_executable(
        vm_thread_test
        fabric/src/threading/vm_thread_test.cpp
        fabric/src/typedefs.hpp
        fabric/src/vm/structures.hpp fabric/src/vm/structures.cpp
        fabric/src/memory/management.hpp fabric/src/memory/management.cpp
        fabric/src/os.hpp fabric/src/os.cpp
        fabric/src/vm/resource.hpp
        fabric/src/errors.hpp
        fabric/src/threading/queue.cpp fabric/src/threading/queue.hpp
        fabric/src/memory/memory.hpp fabric/src/memory/memory.cpp
        fabric/src/threading/os.cpp fabric/src/threading/os.hpp
        fabric/src/threading/management.cpp fabric/src/threading/management.hpp
        fabric/src/core/runtime.cpp fabric/src/core/runtime.hpp fabric/src/core/runtime.forward.hpp fabric/src/memory/os.cpp fabric/src/memory/os.hpp)
