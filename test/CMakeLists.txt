cmake_minimum_required(VERSION 3.2.2)

project(picobench_test)

enable_testing()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/doctest
)

if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

add_executable(basic basic.cpp)

add_test(basic basic)
