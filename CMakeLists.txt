cmake_minimum_required(VERSION 3.22)
project(Veil)

set(CMAKE_CXX_STANDARD 17)
include_directories("src")
add_executable(
        Veil
        main.cpp
        src/typedefs.h
        src/structures.h
        src/memory/memory.h src/memory/memory.cpp
        src/natives.h src/natives.cpp
        src/resource.h src/resource-impl.h
        src/errors.h
        src/runtime/runtime.cpp src/runtime/runtime.h)
