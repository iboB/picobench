// picobench v0.01
//
// A micro microbenchmarking library in a single header file
//
// MIT License
//
// Copyright(c) 2017 Borislav Stanimirov
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//                  VERSION HISTORY
//
//  0.01 (2017-12-22) Initial prototype release
//
//
//                  DOCUMENTATION
//
// Simply include this file wherever you need.
// Define PICOBENCH_IMPLEMENT or define PICOBENCH_IMPLEMENT in one compilation
// unit to have the implementation compiled there.
//
//
//                  EXAMPLE
//
// void my_function(); // the function you want to benchmark
//
// // write your benchmarking code in a function like this
// static void benchmark_my_function(picobench::state& state)
// {
//     // use the state in a range-based for loop to call your code
//     for (auto _ : state)
//         my_function();
// }
// // create a picobench with your benchmarking code
// PICOBENCH(benchmark_my_function);
//
//
//                  TESTS
//
// The tests are included in the header file and use doctest (https://github.com/onqtam/doctest).
// To run them, define PICOBENCH_TEST_WITH_DOCTEST before including
// the header in a file which has doctest.h already included.
//
#pragma once

#include <cstdint>
#include <chrono>
#include <deque>
#include <memory>
#include <vector>

#if defined(PICOBENCH_TEST_WITH_DOCTEST)
#   define PICOBENCH_TEST
#endif

#if defined(PICOBENCH_DEBUG)
#   include <cassert>
#   define _PICOBENCH_ASSERT assert
#else
#   define _PICOBENCH_ASSERT(...)
#endif

#if defined(__GNUC__)
#   define PICOBENCH_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
#   define PICOBENCH_INLINE  __forceinline
#else
#   define PICOBENCH_INLINE  inline
#endif

namespace picobench
{

#if defined(_MSC_VER) || defined(__MINGW32__) || defined(PICOBENCH_TEST)
struct high_res_clock
{
    typedef long long rep;
    typedef std::nano period;
    typedef std::chrono::duration<rep, period> duration;
    typedef std::chrono::time_point<high_res_clock> time_point;
    static const bool is_steady = true;

    static time_point now();
};
#else
typedef std::chrono::high_resolution_clock high_res_clock;
#endif

class state
{
public:
    state(int num_iterations)
        : _iterations(num_iterations)
    {
        _PICOBENCH_ASSERT(_iterations > 0);
    }

    int iterations() const { return _iterations; }

    int64_t duration_ns() const { return _duration_ns; }

    void start_timer()
    {
        _start = high_res_clock::now();
    }

    void stop_timer()
    {
        auto duration = high_res_clock::now() - _start;
        _duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    }

    struct iterator
    {
        PICOBENCH_INLINE
        iterator(state* parent)
            : _counter(parent->iterations())
            , _state(parent)
        {
            _PICOBENCH_ASSERT(_counter > 0);
        }

        PICOBENCH_INLINE
        iterator()
            : _counter(0)
            , _state(nullptr)
        {}

        PICOBENCH_INLINE
        iterator& operator++()
        {
            _PICOBENCH_ASSERT(_counter > 0);
            --_counter;
            return *this;
        }

        PICOBENCH_INLINE
        bool operator!=(const iterator&) const
        {
            if (_counter) return true;
            _state->stop_timer();
            return false;
        }

        PICOBENCH_INLINE
        int operator*() const
        {
            return 0;
        }

    private:
        int _counter;
        state* _state;
    };

    PICOBENCH_INLINE
    iterator begin()
    {
        start_timer();
        return iterator(this);
    }

    PICOBENCH_INLINE
    iterator end()
    {
        return iterator();
    }

private:
    high_res_clock::time_point _start;
    int64_t _duration_ns = 0;
    int _iterations;
};

// this can be used for manual measurement
class scope
{
public:
    scope(state& s)
        : _state(s)
    {
        _state.start_timer();
    }

    ~scope()
    {
        _state.stop_timer();
    }
private:
    state& _state;
};

typedef void(*benchmark_proc)(state&);

class benchmark
{
public:
    const char* name() const { return _name; }

    benchmark& iterations(std::vector<int> data) { _state_iterations = std::move(data); return *this; }
    benchmark& samples(int n) { _samples = n; return *this; }
    benchmark& iteration_label(const char* label) { _iteration_label = label; return *this; }

private:
    friend class runner;

    benchmark(const char* name, benchmark_proc proc);

    const char* const _name;
    const benchmark_proc _proc;
    const char* _iteration_label = nullptr;

    std::vector<int> _state_iterations;
    int _samples = 0;

    // state
    std::vector<state> _states; // length is _samples * _state_iterations.size()
    std::vector<state>::iterator _istate;
};

class runner
{
public:
    runner();

    void run_benchmarks(int random_seed = -1);

    static benchmark& new_benchmark(const char* name, benchmark_proc proc);

    void set_default_state_iterations(const std::vector<int>& data)
    {
        _default_state_iterations = data;
    }

    void set_default_samples(int n)
    {
        _default_samples = n;
    }

private:
    // global registration of all benchmarks
    static std::deque<std::unique_ptr<benchmark>>& benchmarks();

    // default data

    // default iterations per state per benchmark
    std::vector<int> _default_state_iterations;

    // default samples per benchmark
    int _default_samples;
};

}

#define _PICOBENCH_PP_CAT(a, b) _PICOBENCH_PP_INTERNAL_CAT(a, b)
#define _PICOBENCH_PP_INTERNAL_CAT(a, b) a##b

#define PICOBENCH(func) static auto& _PICOBENCH_PP_CAT(picobench, __LINE__) = \
    picobench::runner::new_benchmark(#func, func)

#if defined PICOBENCH_IMPLEMENT_WITH_MAIN
#   define PICOBENCH_IMPLEMENT
#   define PICOBENCH_IMPLEMENT_MAIN
#endif

#if defined PICOBENCH_IMPLEMENT

#include <random>
#include <cstdio>

namespace picobench
{

benchmark::benchmark(const char* name, benchmark_proc proc)
    : _name(name)
    , _proc(proc)
{}

std::deque<std::unique_ptr<benchmark>>& runner::benchmarks()
{
    static std::deque<std::unique_ptr<benchmark>> b;
    return b;
}

benchmark& runner::new_benchmark(const char* name, benchmark_proc proc)
{
    auto b = new benchmark(name, proc);
    benchmarks().emplace_back(b);
    return *b;
}

runner::runner()
    : _default_state_iterations({ 8, 64, 512, 4096, 8196 })
    , _default_samples(1)
{
}

void runner::run_benchmarks(int random_seed)
{
    if (random_seed == -1)
    {
        random_seed = std::random_device()();
    }

    std::minstd_rand rnd(random_seed);

    auto& registered_benchmarks = benchmarks();

    // vector of all benchmarks
    std::vector<benchmark*> benchmarks;
    benchmarks.reserve(registered_benchmarks.size());
    for (auto& rb : registered_benchmarks)
    {
        benchmarks.push_back(rb.get());
    }

    // initialize benchmarks
    for (auto b : benchmarks)
    {
        std::vector<int>& state_iterations =
            b->_state_iterations.empty() ?
            _default_state_iterations :
            b->_state_iterations;

        if (b->_samples == 0)
            b->_samples = _default_samples;

        b->_states.reserve(state_iterations.size());

        // fill states while random shuffling them
        for (auto iters : state_iterations)
        {
            for (int i = 0; i < b->_samples; ++i)
            {
                auto index = rnd() % (b->_states.size() + 1);
                auto pos = b->_states.begin() + index;
                b->_states.emplace(pos, iters);
            }
        }

        b->_istate = b->_states.begin();
    }

    // we run a random benchmark from it incrementing _istate for each
    // when _istate reaches _states.end(), we erase the benchmark
    // when the vector becomes empty, we're done
    while (!benchmarks.empty())
    {
        auto i = benchmarks.begin() + (rnd() % benchmarks.size());
        auto& b = *i;

        b->_proc(*b->_istate);

        ++b->_istate;

        if (b->_istate == b->_states.end())
        {
            benchmarks.erase(i);
        }
    }

    for (auto& b : registered_benchmarks)
    {
        int64_t total_time = 0;
        int total_iterations = 0;
        for (auto& state : b->_states)
        {
            total_time += state.duration_ns();
            total_iterations += state.iterations();
        }

        int64_t mean_time = total_time / (b->_samples * total_iterations);

        printf("%20s: %lld ns\n", b->name(), mean_time);
    }
}

#if (defined(_MSC_VER) || defined(__MINGW32__)) && !defined(PICOBENCH_TEST)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static const long long high_res_clock_freq = []() -> long long
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return frequency.QuadPart;
}();

high_res_clock::time_point high_res_clock::now()
{
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return time_point(duration((t.QuadPart * rep(period::den)) / high_res_clock_freq));
}
#endif
}

#endif

#if defined PICOBENCH_IMPLEMENT_MAIN
int main(int argc, char* argv[])
{
    picobench::runner r;
    r.run_benchmarks();
    return 0;
}
#endif

#if defined(PICOBENCH_TEST)

// fake time keeping functions for the tests
namespace picobench
{

struct fake_time
{
    uint64_t now;
};

fake_time the_time;

high_res_clock::time_point high_res_clock::now()
{
    auto ret = time_point(duration(the_time.now));
    return ret;
}

void this_thread_sleep_for_ns(uint64_t ns)
{
    the_time.now += ns;
}

template <class Rep, class Period>
void this_thread_sleep_for(const std::chrono::duration<Rep, Period>& duration)
{
    this_thread_sleep_for_ns(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
}

}

#endif
