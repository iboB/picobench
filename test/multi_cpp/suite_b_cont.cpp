#include "picobench_configured.hpp"
#include <map>

extern std::map<int, int> g_num_samples;

PICOBENCH_SUITE("suite b");

static void b_a(picobench::state& s)
{
    ++g_num_samples[s.iterations()];
    for (auto _ : s)
    {
        picobench::test::this_thread_sleep_for_ns(20);
    }
}
PICOBENCH(b_a);

static void b_b(picobench::state& s)
{
    s.start_timer();
    picobench::test::this_thread_sleep_for_ns(s.iterations() * size_t(25));
    s.stop_timer();
}
PICOBENCH(b_b);
