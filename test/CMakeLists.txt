cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

include(./get_cpm.cmake)
CPMAddPackage(
    NAME doctest-lib
    VERSION 2.4.8a
    GITHUB_REPOSITORY iboB/doctest-lib
    OPTIONS "DOCTEST_LIB_TREAT_CHAR_STAR_AS_STRING ON"
)
CPMAddPackage(gh:iboB/icm@1.3.3)
find_package(Threads REQUIRED)
include(${icm_SOURCE_DIR}/icm_testing.cmake)

macro(pb_test test)
    icm_add_doctest_lib_test(${test} picobench ${ARGN} LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
endmacro()

pb_test(basic basic.cpp)
pb_test(multi_cpp
    multi_cpp/main.cpp
    multi_cpp/suite_a.cpp
    multi_cpp/suite_b.cpp
    multi_cpp/suite_b_cont.cpp
)
