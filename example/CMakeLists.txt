find_package(Threads REQUIRED)

macro(pb_example name)
    set(tgt picobench-example-${name})
    add_executable(${tgt} ${ARGN})
    target_link_libraries(${tgt} picobench ${CMAKE_THREAD_LIBS_INI})
    set_target_properties(${tgt} PROPERTIES FOLDER examples)
endmacro()

pb_example(basic basic.cpp)
pb_example(locks locks.cpp)
