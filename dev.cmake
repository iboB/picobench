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

if(SAN_ADDR)
    if(MSVC)
        set(icm_san_flags "/fsanitize=address")
    elseif(APPLE)
        # apple clang doesn't support the leak sanitizer
        set(icm_san_flags "-fsanitize=address,undefined -pthread -g")
    else()
        set(icm_san_flags "-fsanitize=address,undefined,leak -pthread -g")
    endif()
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${icm_compiler_flags} ${icm_san_flags}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${icm_compiler_flags} ${icm_san_flags}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${icm_san_flags}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${icm_san_flags}")
