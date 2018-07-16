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

add_executable(multi_cpp
    multi_cpp/main.cpp
    multi_cpp/suite_a.cpp
    multi_cpp/suite_b.cpp
    multi_cpp/suite_b_cont.cpp
)

target_link_libraries(multi_cpp pthread)

add_test(basic basic)
add_test(multi_cpp multi_cpp)
