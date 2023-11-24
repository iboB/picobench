cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

include(./get_cpm.cmake)
CPMAddPackage(gh:iboB/doctest-util@0.1.2)

macro(pb_test test)
    add_doctest_lib_test(${test} picobench ${ARGN})
endmacro()

pb_test(basic basic.cpp)
pb_test(multi_cpp
    multi_cpp/main.cpp
    multi_cpp/suite_a.cpp
    multi_cpp/suite_b.cpp
    multi_cpp/suite_b_cont.cpp
)
