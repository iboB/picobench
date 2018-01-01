#define PICOBENCH_DEBUG
#define PICOBENCH_TEST
#include <picobench/picobench.hpp>

PICOBENCH_SUITE("suite a");

static void a_a(picobench::state& s)
{
    for (auto _ : s)
    {
        picobench::this_thread_sleep_for_ns(10);
    }
}
PICOBENCH(a_a);

static void a_b(picobench::state& s)
{
    for (auto _ : s)
    {
        picobench::this_thread_sleep_for_ns(15);
    }
}
PICOBENCH(a_b);