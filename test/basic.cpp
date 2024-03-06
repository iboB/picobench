#include <doctest/doctest.h>

#define PICOBENCH_DEBUG
#define PICOBENCH_TEST
#define PICOBENCH_IMPLEMENT
#include <picobench/picobench.hpp>

#include <string>
#include <sstream>

using namespace picobench;
using namespace std;

PICOBENCH_SUITE("test a");

static void a_a(picobench::state& s)
{
    for (auto _ : s)
    {
        test::this_thread_sleep_for_ns(10);
    }
    s.set_result(s.iterations() * 2);
}
PICOBENCH(a_a);

map<int, int> a_b_samples;
static void a_b(picobench::state& s)
{
    uint64_t time = 11;
    if (a_b_samples.find(s.iterations()) == a_b_samples.end())
    {
        // slower first time
        time = 32;
    }

    ++a_b_samples[s.iterations()];
    for (auto _ : s)
    {
        test::this_thread_sleep_for_ns(time);
    }
    s.set_result(s.iterations() * 2);
}
PICOBENCH(a_b);

static void a_c(picobench::state& s)
{
    s.start_timer();
    test::this_thread_sleep_for_ns((s.iterations() - 1) * 20);
    s.stop_timer();

    s.add_custom_duration(20);
    s.set_result(s.iterations() * 2);
}
PICOBENCH(a_c);

PICOBENCH_SUITE("test empty");

PICOBENCH_SUITE("test b");

static void b_a(picobench::state& s)
{
    CHECK(s.user_data() == 9088);
    for (auto _ : s)
    {
        test::this_thread_sleep_for_ns(75);
    }
}
PICOBENCH(b_a)
.iterations({20, 30, 50})
.user_data(9088);

map<int, int> b_b_samples;

static void b_b(picobench::state& s)
{
    uint64_t time = 111;
    if (b_b_samples.find(s.iterations()) == b_b_samples.end())
    {
        // faster first time
        time = 100;
    }

    ++b_b_samples[s.iterations()];
    picobench::scope scope(s);
    test::this_thread_sleep_for_ns(s.iterations() * time);
}
PICOBENCH(b_b)
.baseline()
.label("something else")
.samples(15)
.iterations({10, 20, 30});

const picobench::report::suite& find_suite(const char* s, const picobench::report& r)
{
    auto suite = r.find_suite(s);
    REQUIRE(suite);
    return *suite;
}

#define cntof(ar) (sizeof(ar) / sizeof(ar[0]))

TEST_CASE("[picobench] test utils")
{
    const char* ar[] = {"test", "123", "asdf"};
    CHECK(cntof(ar) == 3);

    auto start = high_res_clock::now();
    test::this_thread_sleep_for_ns(1234);
    auto end = high_res_clock::now();

    auto duration = end - start;
    CHECK(duration == std::chrono::nanoseconds(1234));

    start = high_res_clock::now();
    test::this_thread_sleep_for(std::chrono::milliseconds(987));
    end = high_res_clock::now();
    duration = end - start;
    CHECK(duration == std::chrono::milliseconds(987));
}

TEST_CASE("[picobench] picostring")
{
    picostring str("test");
    CHECK(str == "test");
    CHECK(str.len == 4);
    CHECK(!str.is_start_of("tes"));
    CHECK(str.is_start_of("test"));
    CHECK(str.is_start_of("test123"));
}

TEST_CASE("[picobench] state")
{
    state s0(3);
    CHECK(s0.iterations() == 3);
    CHECK(s0.user_data() == 0);

    int i = 0;
    for (auto _ : s0)
    {
        CHECK(_ == i);
        ++i;
        test::this_thread_sleep_for_ns(1);
    }
    CHECK(s0.duration_ns() == 3);
    s0.add_custom_duration(5);
    CHECK(s0.duration_ns() == 8);

    state s(2, 123);
    CHECK(s.iterations() == 2);
    CHECK(s.user_data() == 123);

    i = 0;
    for (auto it = s.begin(); it != s.end(); ++it)
    {
        CHECK(*it == i);
        ++i;
        test::this_thread_sleep_for_ns(2);
    }
    CHECK(s.duration_ns() == 4);
}

const vector<int> default_iters = { 8, 64, 512, 4096, 8192 };
const int default_samples = 2;

TEST_CASE("[picobench] cmd line")
{
    {
        local_runner r;
        bool b = r.parse_cmd_line(0, {});
        CHECK(b);
        CHECK(r.should_run());
        CHECK(r.error() == 0);
        CHECK(r.default_state_iterations() == default_iters);
        CHECK(r.default_samples() == default_samples);
        CHECK(!r.preferred_output_filename());
        CHECK(r.preferred_output_format() == report_output_format::text);
        CHECK(!r.compare_results_across_benchmarks());
        CHECK(!r.compare_results_across_samples());
    }

    {
        local_runner r;
        ostringstream sout, serr;
        r.set_output_streams(sout, serr);
        const char* cmd_line[] = { "", "-asdf" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line, "-");
        CHECK(sout.str().empty());
        CHECK(serr.str() == "Error: Unknown command-line argument: -asdf\n");
        CHECK(!b);
        CHECK(!r.should_run());
        CHECK(r.error() == error_unknown_cmd_line_argument);
    }

    {
        local_runner r;
        const char* cmd_line[] = { "", "--no-run", "--iters=1,2,3", "--samples=54", "--out-fmt=con", "--output=stdout" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line);
        CHECK(b);
        CHECK(!r.should_run());
        CHECK(r.error() == 0);
        CHECK(r.default_samples() == 54);
        CHECK(r.default_state_iterations() == vector<int>({ 1, 2, 3 }));
        CHECK(!r.preferred_output_filename());
        CHECK(r.preferred_output_format() == report_output_format::concise_text);
        CHECK(!r.compare_results_across_benchmarks());
        CHECK(!r.compare_results_across_samples());
    }

    {
        local_runner r;
        const char* cmd_line[] = { "", "--pb-no-run", "--pb-iters=1000,2000,3000", "-other-cmd1", "--pb-samples=54",
            "-other-cmd2", "--pb-out-fmt=csv", "--pb-output=foo.csv", "--pb-compare-results" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line, "--pb");
        CHECK(b);
        CHECK(!r.should_run());
        CHECK(r.error() == 0);
        CHECK(r.default_samples() == 54);
        CHECK(r.default_state_iterations() == vector<int>({ 1000, 2000, 3000 }));
        CHECK(strcmp(r.preferred_output_filename(), "foo.csv") == 0);
        CHECK(r.preferred_output_format() == report_output_format::csv);
        CHECK(r.compare_results_across_benchmarks());
        CHECK(r.compare_results_across_samples());

    }

    {
        local_runner r;
        ostringstream sout, serr;
        r.set_output_streams(sout, serr);
        const char* cmd_line[] = { "", "--samples=xxx" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line, "-");
        CHECK(sout.str().empty());
        CHECK(serr.str() == "Error: Bad command-line argument: --samples=xxx\n");
        CHECK(!b);
        CHECK(!r.should_run());
        CHECK(r.error() == error_bad_cmd_line_argument);
        CHECK(r.default_samples() == default_samples);
    }

    {
        local_runner r;
        ostringstream sout, serr;
        r.set_output_streams(sout, serr);
        const char* cmd_line[] = { "", "--iters=1,xxx,2" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line, "-");
        CHECK(sout.str().empty());
        CHECK(serr.str() == "Error: Bad command-line argument: --iters=1,xxx,2\n");
        CHECK(!b);
        CHECK(!r.should_run());
        CHECK(r.error() == error_bad_cmd_line_argument);
        CHECK(r.default_state_iterations() == default_iters);
    }

    {
        local_runner r;
        ostringstream sout, serr;
        r.set_output_streams(sout, serr);
        const char* cmd_line[] = { "", "--out-fmt=asdf" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line, "-");
        CHECK(sout.str().empty());
        CHECK(serr.str() == "Error: Bad command-line argument: --out-fmt=asdf\n");
        CHECK(!b);
        CHECK(!r.should_run());
        CHECK(r.error() == error_bad_cmd_line_argument);
        CHECK(r.preferred_output_format() == report_output_format::text);
    }

#define PB_VERSION_INFO "picobench " PICOBENCH_VERSION_STR "\n"

    {
        const char* v = PB_VERSION_INFO;

        local_runner r;
        ostringstream sout, serr;
        r.set_output_streams(sout, serr);
        const char* cmd_line[] = { "", "--pb-version" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line, "--pb");
        CHECK(sout.str() == v);
        CHECK(serr.str().empty());
        CHECK(b);
        CHECK(!r.should_run());
        CHECK(r.error() == 0);
    }

#define PB_HELP \
        " --pb-iters=<n1,n2,n3,...>  Sets default iterations for benchmarks\n" \
        " --pb-samples=<n>           Sets default number of samples for benchmarks\n" \
        " --pb-out-fmt=<txt|con|csv> Outputs text or concise or csv\n" \
        " --pb-output=<filename>     Sets output filename or `stdout`\n" \
        " --pb-compare-results       Compare benchmark results\n" \
        " --pb-no-run                Doesn't run benchmarks\n" \
        " --pb-run-suite=<suite>     Runs only benchmarks from suite\n" \
        " --pb-run-only=<b1,b2,...>  Runs only selected benchmarks\n" \
        " --pb-list                  Lists available benchmarks\n" \
        " --pb-version               Show version info\n" \
        " --pb-help                  Prints help\n"

    {
        const char* help =
            PB_VERSION_INFO
            PB_HELP;

        local_runner r;
        ostringstream sout, serr;
        r.set_output_streams(sout, serr);
        const char* cmd_line[] = { "", "--pb-help" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line, "--pb");
        CHECK(sout.str() == help);
        CHECK(serr.str().empty());
        CHECK(b);
        CHECK(!r.should_run());
        CHECK(r.error() == 0);
    }

    {
        const char* help =
            PB_VERSION_INFO
            " --pb-cmd-hi                Custom help\n"
            " --pb-cmd-bi=123            More custom help\n"
            PB_HELP;

        local_runner r;

        auto handler_hi = [](uintptr_t data, const char* cmd) -> bool {
            CHECK(data == 123);
            CHECK(*cmd == 0);
            return true;
        };

        r.add_cmd_opt("-cmd-hi", "", "Custom help", handler_hi, 123);

        auto handler_bi = [](uintptr_t data, const char* cmd) -> bool {
            CHECK(data == 98);
            CHECK(strcmp(cmd, "123") == 0);
            return true;
        };

        r.add_cmd_opt("-cmd-bi=", "123", "More custom help", handler_bi, 98);

        ostringstream sout, serr;
        r.set_output_streams(sout, serr);
        const char* cmd_line[] = { "", "--pb-help" };
        bool b = r.parse_cmd_line(cntof(cmd_line), cmd_line, "--pb");
        CHECK(sout.str() == help);
        CHECK(serr.str().empty());
        CHECK(b);
        CHECK(!r.should_run());
        CHECK(r.error() == 0);

        sout.str(std::string());
        serr.str(std::string());

        const char* cmd_line2[] = { "",  "--zz-cmd-bi=123", "--zz-cmd-hi" };
        b = r.parse_cmd_line(cntof(cmd_line2), cmd_line2, "--zz");

        CHECK(sout.str().empty());
        CHECK(serr.str().empty());
        CHECK(b);
        CHECK(r.error() == 0);
    }
}

TEST_CASE("[picobench] test")
{
    runner r;
    CHECK(r.default_state_iterations() == default_iters);
    CHECK(r.default_samples() == default_samples);

    r.set_compare_results_across_benchmarks(true);
    r.set_compare_results_across_samples(true);

    ostringstream sout;
    ostringstream serr;
    r.set_output_streams(sout, serr);

    r.run_benchmarks();
    auto report = r.generate_report();

    CHECK(serr.str().empty());

    const char* warnings =
            "Warning: Benchmark something else @10 has a single instance and cannot be compared to others.\n"
            "Warning: Benchmark b_a @50 has a single instance and cannot be compared to others.\n";
    CHECK(sout.str() == warnings);

    CHECK(report.suites.size() == 2);
    CHECK(!report.find_suite("asdf"));

    auto& a = find_suite("test a", report);
    CHECK(strcmp(a.name, "test a") == 0);
    CHECK(a.benchmarks.size() == 3);
    CHECK(!a.find_benchmark("b_a"));

    auto& aa = a.benchmarks[0];
    CHECK(a.find_baseline() == &aa);
    CHECK(a.find_benchmark("a_a") == &aa);
    CHECK(strcmp(aa.name, "a_a") == 0);
    CHECK(aa.is_baseline);
    CHECK(aa.data.size() == r.default_state_iterations().size());

    for (size_t i = 0; i<aa.data.size(); ++i)
    {
        auto& d = aa.data[i];
        CHECK(d.dimension == r.default_state_iterations()[i]);
        CHECK(d.samples == r.default_samples());
        CHECK(d.total_time_ns == d.dimension * 10);
    }

    auto& ab = a.benchmarks[1];
    CHECK(a.find_benchmark("a_b") == &ab);
    CHECK(strcmp(ab.name, "a_b") == 0);
    CHECK(!ab.is_baseline);
    CHECK(ab.data.size() == r.default_state_iterations().size());

    for (size_t i = 0; i<ab.data.size(); ++i)
    {
        auto& d = ab.data[i];
        CHECK(d.dimension == r.default_state_iterations()[i]);
        CHECK(d.samples == r.default_samples());
        CHECK(d.total_time_ns == d.dimension * 11);
    }
    size_t j = 0;
    for (auto& elem : a_b_samples)
    {
        CHECK(elem.first == default_iters[j]);
        CHECK(elem.second == r.default_samples());
        ++j;
    }

    auto& ac = a.benchmarks[2];
    CHECK(a.find_benchmark("a_c") == &ac);
    CHECK(strcmp(ac.name, "a_c") == 0);
    CHECK(!ac.is_baseline);
    CHECK(ac.data.size() == r.default_state_iterations().size());

    for (size_t i = 0; i<ac.data.size(); ++i)
    {
        auto& d = ac.data[i];
        CHECK(d.dimension == r.default_state_iterations()[i]);
        CHECK(d.samples == r.default_samples());
        CHECK(d.total_time_ns == d.dimension * 20);
    }

    auto& b = find_suite("test b", report);
    CHECK(strcmp(b.name, "test b") == 0);
    CHECK(b.benchmarks.size() == 2);
    CHECK(!b.find_benchmark("b_b"));

    auto& ba = b.benchmarks[0];
    CHECK(strcmp(ba.name, "b_a") == 0);
    CHECK(!ba.is_baseline);
    CHECK(ba.data.size() == 3);

    int state_iters_a[] = { 20, 30, 50 };
    for (size_t i = 0; i<ba.data.size(); ++i)
    {
        auto& d = ba.data[i];
        CHECK(d.dimension == state_iters_a[i]);
        CHECK(d.samples == r.default_samples());
        CHECK(d.total_time_ns == d.dimension * 75);
    }

    auto& bb = b.benchmarks[1];
    CHECK(b.find_benchmark("something else") == &bb);
    CHECK(b.find_baseline() == &bb);
    CHECK(strcmp(bb.name, "something else") == 0);
    CHECK(bb.is_baseline);
    CHECK(bb.data.size() == 3);

    int state_iters_b[] = { 10, 20, 30 };
    for (size_t i = 0; i<bb.data.size(); ++i)
    {
        auto& d = bb.data[i];
        CHECK(d.dimension == state_iters_b[i]);
        CHECK(d.samples == 15);
        CHECK(d.total_time_ns == d.dimension * 100);
    }

    j = 0;
    for (auto& elem : b_b_samples)
    {
        CHECK(elem.first == state_iters_b[j]);
        CHECK(elem.second == 15);
        ++j;
    }

    sout.str(string());
    report.to_text_concise(sout);
    std::string concise =
        "## test a:\n"
        "\n"
        " Name (* = baseline)      |  ns/op  | Baseline |  Ops/second\n"
        "--------------------------|--------:|---------:|-----------:\n"
        " a_a *                    |      10 |        - | 100000000.0\n"
        " a_b                      |      11 |    1.100 |  90909090.9\n"
        " a_c                      |      20 |    2.000 |  50000000.0\n"
        "\n"
        "## test b:\n"
        "\n"
        " Name (* = baseline)      |  ns/op  | Baseline |  Ops/second\n"
        "--------------------------|--------:|---------:|-----------:\n"
        " b_a                      |      75 |    0.750 |  13333333.3\n"
        " something else *         |     100 |        - |  10000000.0\n"
        "\n";

    CHECK(sout.str() == concise);

    std::string txt =
        "## test a:\n"
        "\n"
        " Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second\n"
        "--------------------------|--------:|----------:|--------:|-------:|----------:\n"
        " a_a *                    |       8 |     0.000 |      10 |      - |100000000.0\n"
        " a_b                      |       8 |     0.000 |      11 |  1.100 | 90909090.9\n"
        " a_c                      |       8 |     0.000 |      20 |  2.000 | 50000000.0\n"
        " a_a *                    |      64 |     0.001 |      10 |      - |100000000.0\n"
        " a_b                      |      64 |     0.001 |      11 |  1.100 | 90909090.9\n"
        " a_c                      |      64 |     0.001 |      20 |  2.000 | 50000000.0\n"
        " a_a *                    |     512 |     0.005 |      10 |      - |100000000.0\n"
        " a_b                      |     512 |     0.006 |      11 |  1.100 | 90909090.9\n"
        " a_c                      |     512 |     0.010 |      20 |  2.000 | 50000000.0\n"
        " a_a *                    |    4096 |     0.041 |      10 |      - |100000000.0\n"
        " a_b                      |    4096 |     0.045 |      11 |  1.100 | 90909090.9\n"
        " a_c                      |    4096 |     0.082 |      20 |  2.000 | 50000000.0\n"
        " a_a *                    |    8192 |     0.082 |      10 |      - |100000000.0\n"
        " a_b                      |    8192 |     0.090 |      11 |  1.100 | 90909090.9\n"
        " a_c                      |    8192 |     0.164 |      20 |  2.000 | 50000000.0\n"
        "\n"
        "## test b:\n"
        "\n"
        " Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second\n"
        "--------------------------|--------:|----------:|--------:|-------:|----------:\n"
        " something else *         |      10 |     0.001 |     100 |      - | 10000000.0\n"
        " b_a                      |      20 |     0.002 |      75 |  0.750 | 13333333.3\n"
        " something else *         |      20 |     0.002 |     100 |      - | 10000000.0\n"
        " b_a                      |      30 |     0.002 |      75 |  0.750 | 13333333.3\n"
        " something else *         |      30 |     0.003 |     100 |      - | 10000000.0\n"
        " b_a                      |      50 |     0.004 |      75 |    ??? | 13333333.3\n"
        "\n";

    sout.str(string());
    report.to_text(sout);
    CHECK(sout.str() == txt);

    const char* csv =
        "Suite,Benchmark,b,D,S,\"Total ns\",Result,\"ns/op\",Baseline\n"
        "\"test a\",\"a_a\",*,8,2,80,16,10,1.000\n"
        "\"test a\",\"a_a\",*,64,2,640,128,10,1.000\n"
        "\"test a\",\"a_a\",*,512,2,5120,1024,10,1.000\n"
        "\"test a\",\"a_a\",*,4096,2,40960,8192,10,1.000\n"
        "\"test a\",\"a_a\",*,8192,2,81920,16384,10,1.000\n"
        "\"test a\",\"a_b\",,8,2,88,16,11,1.100\n"
        "\"test a\",\"a_b\",,64,2,704,128,11,1.100\n"
        "\"test a\",\"a_b\",,512,2,5632,1024,11,1.100\n"
        "\"test a\",\"a_b\",,4096,2,45056,8192,11,1.100\n"
        "\"test a\",\"a_b\",,8192,2,90112,16384,11,1.100\n"
        "\"test a\",\"a_c\",,8,2,160,16,20,2.000\n"
        "\"test a\",\"a_c\",,64,2,1280,128,20,2.000\n"
        "\"test a\",\"a_c\",,512,2,10240,1024,20,2.000\n"
        "\"test a\",\"a_c\",,4096,2,81920,8192,20,2.000\n"
        "\"test a\",\"a_c\",,8192,2,163840,16384,20,2.000\n"
        "\"test b\",\"b_a\",,20,2,1500,0,75,0.750\n"
        "\"test b\",\"b_a\",,30,2,2250,0,75,0.750\n"
        "\"test b\",\"b_a\",,50,2,3750,0,75,\n"
        "\"test b\",\"something else\",*,10,15,1000,0,100,1.000\n"
        "\"test b\",\"something else\",*,20,15,2000,0,100,1.000\n"
        "\"test b\",\"something else\",*,30,15,3000,0,100,1.000\n";

    sout.str(string());
    report.to_csv(sout);
    CHECK(sout.str() == csv);

    CHECK(serr.str().empty());
}

int compare_counter = 0;

TEST_CASE("[picobench] compare")
{
    const int TSEED = 13;
    {
        local_runner r;
        ostringstream sout, serr;
        r.set_output_streams(sout, serr);

        r.set_compare_results_across_benchmarks(true);
        r.set_compare_results_across_samples(true);

        auto func = [](state& s)
        {
            s.add_custom_duration(s.iterations());
            s.set_result(s.iterations() * 2);
        };

        r.add_benchmark("b1", func);
        r.add_benchmark("b2", func);
        r.add_benchmark("b2-twist", func).user_data(1);
        r.add_benchmark("b3", func);

        r.run_benchmarks();
        r.generate_report();

        CHECK(r.error() == no_error);
        CHECK(sout.str() ==
              "Warning: b1 and b2 are benchmarks of the same function.\n"
              "Warning: b1 and b3 are benchmarks of the same function.\n"
              "Warning: b2 and b3 are benchmarks of the same function.\n"
              );
        CHECK(serr.str().empty());

    }

    {
        local_runner r;
        ostringstream sout, serr;
        r.set_output_streams(sout, serr);

        r.add_benchmark("b1", [](state& s)
        {
            s.add_custom_duration(s.iterations());
            s.set_result(s.iterations() + compare_counter++);
        });
        r.add_benchmark("b2", [](state& s)
        {
            s.add_custom_duration(s.iterations());
            s.set_result(s.iterations() + compare_counter++);
        });

        r.run_benchmarks();
        r.generate_report();

        CHECK(r.error() == no_error);

        r.set_compare_results_across_samples(true);

        r.run_benchmarks(TSEED);
        r.generate_report();

        CHECK(r.error() == error_sample_compare);
        CHECK(serr.str() ==
              "Error: Two samples of b1 @4096 produced different results: 4121 and 4122\n"
              "Error: Two samples of b1 @8 produced different results: 36 and 37\n"
              "Error: Two samples of b1 @64 produced different results: 88 and 100\n"
              "Error: Two samples of b1 @512 produced different results: 545 and 549\n"
              "Error: Two samples of b1 @8192 produced different results: 8213 and 8230\n"
              "Error: Two samples of b2 @8 produced different results: 31 and 35\n"
              "Error: Two samples of b2 @512 produced different results: 532 and 542\n"
              "Error: Two samples of b2 @4096 produced different results: 4127 and 4130\n"
              "Error: Two samples of b2 @64 produced different results: 96 and 99\n"
              "Error: Two samples of b2 @8192 produced different results: 8214 and 8231\n"
              );
        CHECK(sout.str().empty());

        r.set_error(no_error);
        r.set_compare_results_across_samples(false);
        r.set_compare_results_across_benchmarks(true);
        sout.str(string());
        serr.str(string());


        r.run_benchmarks(TSEED);
        r.generate_report();

        CHECK(r.error() == error_benchmark_compare);
        CHECK(serr.str() ==
              "Error: Benchmarks b1 and b2 @8 produce different results: 56 and 51\n"
              "Error: Benchmarks b1 and b2 @64 produce different results: 108 and 116\n"
              "Error: Benchmarks b1 and b2 @512 produce different results: 565 and 552\n"
              "Error: Benchmarks b1 and b2 @4096 produce different results: 4141 and 4147\n"
              "Error: Benchmarks b1 and b2 @8192 produce different results: 8233 and 8234\n"
              );
        CHECK(sout.str().empty());
    }
}
