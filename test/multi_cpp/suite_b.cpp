#include "picobench_configured.hpp"

PICOBENCH_SUITE("suite b");

static void a_a(picobench::state& s)
{
    for (auto _ : s)
    {
        picobench::this_thread_sleep_for_ns(15);
    }
}
PICOBENCH(a_a);

static void a_b(size_t stime, picobench::state& s)
{
    s.start_timer();
    picobench::this_thread_sleep_for_ns(s.iterations() * size_t(stime));
    s.stop_timer();
}
PICOBENCH([](picobench::state& s) { a_b(30, s); }).label("a_b").baseline();
