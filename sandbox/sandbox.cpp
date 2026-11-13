#define PICOBENCH_IMPLEMENT
#include <picobench/picobench.hpp>

int main(int argc, char** argv) {
    picobench::runner runner;
    runner.parse_cmd_line(argc, argv);
    runner.default_samples();
    runner.run();
    return 0;
}
