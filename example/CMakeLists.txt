macro(pb_example name)
    set(tgt picobench-example-${name})
    add_executable(${tgt} ${ARGN})
    target_link_libraries(${tgt} picobench::picobench)
    set_target_properties(${tgt} PROPERTIES FOLDER example)
    add_custom_target(
        picobench-run-example-${name}
        COMMAND ${tgt}
    )
endmacro()

pb_example(basic basic.cpp)
pb_example(locks locks.cpp)
