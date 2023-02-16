#include "picobench_configured.hpp"

PICOBENCH_SUITE("suite b");

static void a_a(pb::state& s)
{
    for (auto _ : s)
    {
        pb::test::this_thread_sleep_for_ns(15);
    }
}
PICOBENCH(a_a);

static void a_b(size_t stime, pb::state& s)
{
    s.start_timer();
    pb::test::this_thread_sleep_for_ns(s.iterations() * size_t(stime));
    s.stop_timer();
}
PICOBENCH([](pb::state& s) { a_b(30, s); }).label("a_b").baseline();
