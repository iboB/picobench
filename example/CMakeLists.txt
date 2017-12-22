cmake_minimum_required(VERSION 3.2.2)

project(picobench_example)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_executable(basic basic.cpp)
