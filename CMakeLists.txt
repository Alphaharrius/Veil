cmake_minimum_required(VERSION 3.22)
project(Veil)

set(CMAKE_CXX_STANDARD 17)
include_directories("src")
add_executable(Veil main.cpp src/typedefs.h src/memory/memory.h src/diagnostics.h src/memory/memory.cpp)