cmake_minimum_required(VERSION 3.0)
project(winevfs)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_BUILD_TYPE Debug)
set (CMAKE_CXX_FLAGS "-Wall -O3")

find_package(Threads)

add_subdirectory(src)
