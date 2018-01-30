cmake_minimum_required(VERSION 3.2.2)

project(picobench)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

if(NOT MSVC)	
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")	
else()
	add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
endif()

add_executable(picobench picobench.cpp)
