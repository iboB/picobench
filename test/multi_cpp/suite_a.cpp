#include "picobench_configured.hpp"

PICOBENCH_SUITE("suite a");

static void a_a(pb::state& s)
{
    for (auto _ : s)
    {
        pb::test::this_thread_sleep_for_ns(10);
    }
}
PICOBENCH(a_a);

static void a_b(pb::state& s)
{
    for (auto _ : s)
    {
        pb::test::this_thread_sleep_for_ns(15);
    }
}
PICOBENCH(a_b);
