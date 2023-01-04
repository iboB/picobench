# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
cmake_minimum_required(VERSION 3.2.2)

project(picobench)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # dev_mode is used below to make life easier for developers
    # it enables some configurations and the defaults for building tests and
    # examples which typically wouldn't be built if xmem is a subdirectory of
    # another project
    set(dev_mode ON)
else()
    set(dev_mode OFF)
endif()

option(PICOBENCH_BUILD_TOOLS "picobench: build tools" ${dev_mode})
option(PICOBENCH_BUILD_TESTS "picobench: build tests" ${dev_mode})
option(PICOBENCH_BUILD_EXAMPLES "picobench: build examples" ${dev_mode})
mark_as_advanced(PICOBENCH_BUILD_TOOLS PICOBENCH_BUILD_TESTS PICOBENCH_BUILD_EXAMPLES)

if(dev_mode)
    include(./dev.cmake)
endif()

add_library(picobench INTERFACE)
add_library(picobench::picobench ALIAS picobench)
target_include_directories(picobench INTERFACE include)

if(PICOBENCH_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

if(PICOBENCH_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

if(PICOBENCH_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()
