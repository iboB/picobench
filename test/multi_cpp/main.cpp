#include <doctest/doctest.h>

#define PICOBENCH_IMPLEMENT
#define PICOBENCH_DONT_BIND_TO_ONE_CORE
#include "picobench_configured.hpp"

#include <string>
#include <map>

using namespace std;

std::map<int, int> g_num_samples;

const pb::report::suite& find_suite(const char* s, const pb::report& r)
{
    auto suite = r.find_suite(s);
    REQUIRE(suite);
    return *suite;
}

TEST_CASE("[picobench] multi cpp test")
{
    using namespace pb;
    runner r;

    const vector<int> iters = { 100, 2000, 5000 };
    r.set_default_state_iterations(iters);

    const int samples = 13;
    r.set_default_samples(samples);

    r.run_benchmarks();
    auto report = r.generate_report();
    CHECK(report.suites.size() == 2);

    CHECK(g_num_samples.size() == iters.size());
    size_t i = 0;
    for (auto& elem : g_num_samples)
    {
        CHECK(elem.first == iters[i]);
        CHECK(elem.second == samples);
        ++i;
    }

    auto& a = find_suite("suite a", report);
    CHECK(strcmp(a.name, "suite a") == 0);
    CHECK(a.benchmarks.size() == 2);

    auto& aa = a.benchmarks[0];
    CHECK(strcmp(aa.name, "a_a") == 0);
    CHECK(aa.is_baseline);
    CHECK(aa.data.size() == iters.size());

    for (size_t i = 0; i<aa.data.size(); ++i)
    {
        auto& d = aa.data[i];
        CHECK(d.dimension == iters[i]);
        CHECK(d.samples == samples);
        CHECK(d.total_time_ns == d.dimension * 10);
    }

    auto& ab = a.benchmarks[1];
    CHECK(strcmp(ab.name, "a_b") == 0);
    CHECK(!ab.is_baseline);
    CHECK(ab.data.size() == iters.size());

    for (size_t i = 0; i<ab.data.size(); ++i)
    {
        auto& d = ab.data[i];
        CHECK(d.dimension == iters[i]);
        CHECK(d.samples == samples);
        CHECK(d.total_time_ns == d.dimension * 15);
    }

    auto& b = find_suite("suite b", report);
    CHECK(strcmp(b.name, "suite b") == 0);
    CHECK(b.benchmarks.size() == 4);

    auto& aab = b.benchmarks[0].name[0] == 'a' ? b.benchmarks[0] : b.benchmarks[2];
    CHECK(strcmp(aab.name, "a_a") == 0);
    CHECK(!aab.is_baseline);
    CHECK(aab.data.size() == iters.size());

    for (size_t i = 0; i<aab.data.size(); ++i)
    {
        auto& d = aab.data[i];
        CHECK(d.dimension == iters[i]);
        CHECK(d.samples == samples);
        CHECK(d.total_time_ns == d.dimension * 15);
    }

    auto& abb = b.benchmarks[0].name[0] == 'a' ? b.benchmarks[1] : b.benchmarks[3];
    CHECK(strcmp(abb.name, "a_b") == 0);
    CHECK(abb.is_baseline);
    CHECK(abb.data.size() == iters.size());

    for (size_t i = 0; i<abb.data.size(); ++i)
    {
        auto& d = abb.data[i];
        CHECK(d.dimension == iters[i]);
        CHECK(d.samples == samples);
        CHECK(d.total_time_ns == d.dimension * 30);
    }

    auto& bab = b.benchmarks[0].name[0] == 'a' ? b.benchmarks[2] : b.benchmarks[0];
    CHECK(strcmp(bab.name, "b_a") == 0);
    CHECK(!bab.is_baseline);
    CHECK(bab.data.size() == iters.size());

    for (size_t i = 0; i<bab.data.size(); ++i)
    {
        auto& d = bab.data[i];
        CHECK(d.dimension == iters[i]);
        CHECK(d.samples == samples);
        CHECK(d.total_time_ns == d.dimension * 20);
    }

    auto& bbb = b.benchmarks[0].name[0] == 'a' ? b.benchmarks[3] : b.benchmarks[1];
    CHECK(strcmp(bbb.name, "b_b") == 0);
    CHECK(!bbb.is_baseline);
    CHECK(bbb.data.size() == iters.size());

    for (size_t i = 0; i<bbb.data.size(); ++i)
    {
        auto& d = bbb.data[i];
        CHECK(d.dimension == iters[i]);
        CHECK(d.samples == samples);
        CHECK(d.total_time_ns == d.dimension * 25);
    }
}

