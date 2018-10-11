cmake_minimum_required(VERSION 3.2.2)

project(picobench-example)

add_executable(basic basic.cpp)
target_link_libraries(basic picobench)
set_target_properties(basic PROPERTIES FOLDER examples)

add_executable(locks locks.cpp)
target_link_libraries(locks picobench)
set_target_properties(locks PROPERTIES FOLDER examples)
