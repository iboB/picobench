cmake_minimum_required(VERSION 3.2.2)

project(picobench-test)

enable_testing()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/doctest
)

add_executable(test-basic basic.cpp)
target_link_libraries(test-basic picobench)
set_target_properties(test-basic PROPERTIES FOLDER tests)

add_executable(test-multi_cpp
    multi_cpp/main.cpp
    multi_cpp/suite_a.cpp
    multi_cpp/suite_b.cpp
    multi_cpp/suite_b_cont.cpp
)
target_link_libraries(test-multi_cpp picobench pthread)
set_target_properties(test-multi_cpp PROPERTIES FOLDER tests)

add_test(basic test-basic)
add_test(multi_cpp test-multi_cpp)
