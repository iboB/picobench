#pragma once

#define PICOBENCH_DEBUG
#define PICOBENCH_TEST
#define PICOBENCH_STD_FUNCTION_BENCHMARKS
#define PICOBENCH_NAMESPACE pb
extern int global_start_counter, global_stop_counter;
#define PICOBENCH_START_INSTRUMENTATION ++global_start_counter
#define PICOBENCH_STOP_INSTRUMENTATION ++global_stop_counter
#include <picobench/picobench.hpp>
