# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(icm_compiler_flags "-D_CRT_SECURE_NO_WARNINGS /Zc:__cplusplus /permissive-\
        /w34100 /w34701 /w34702 /w34703 /w34706 /w34714 /w34913\
        /wd4251 /wd4275"
    )
else()
    set(icm_compiler_flags "-Wall -Wextra")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${icm_compiler_flags}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${icm_compiler_flags}")
