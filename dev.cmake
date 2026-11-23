# Copyright (c) Borislav Stanimirov
# SPDX-License-Identifier: MIT
#
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(
        -W4 -utf-8
        -Zc:__cplusplus -permissive-
        -volatile:iso -Zc:throwingNew -Zc:templateScope
        -wd4251 -wd4275 -wd5030
    )
else()
    add_compile_options(
        -Wall -Wextra
    )
endif()

if(WIN32)
    add_compile_options(
        -D_CRT_SECURE_NO_WARNINGS
        -DNOMINMAX=1
    )
endif()

if(SAN_ADDR)
    if(MSVC)
        add_compile_options(/fsanitize=address)
    else()
        if(APPLE)
            # apple clang doesn't support the leak sanitizer
            set(icm_san_flags -fsanitize=address,undefined -pthread -g)
        else()
            set(icm_san_flags -fsanitize=address,undefined,leak -pthread -g)
        endif()
        add_compile_options(${icm_san_flags})
        add_link_options(${icm_san_flags})
    endif()
endif()
