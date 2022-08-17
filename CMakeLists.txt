cmake_minimum_required(VERSION 3.22)
project(Veil)

set(CMAKE_CXX_STANDARD 17)
include_directories("src")
add_executable(Veil main.cpp src/util/type-alias.h src/util/runtime-resources.h src/core/heap.cpp src/core/heap.h src/util/synchronization.cpp src/util/synchronization.h src/util/diagnostics.h src/util/natives.h src/util/natives.cpp src/core/runtime.cpp src/core/runtime.h)
