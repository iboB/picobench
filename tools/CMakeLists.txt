cmake_minimum_required(VERSION 3.5)

project(picobench-tools)

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS=1)
endif()

add_executable(picobench-cli picobench.cpp)
target_link_libraries(picobench-cli picobench)
set_target_properties(picobench-cli PROPERTIES OUTPUT_NAME picobench)
set_target_properties(picobench-cli PROPERTIES FOLDER tools)
