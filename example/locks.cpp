#define PICOBENCH_DEBUG
#define PICOBENCH_IMPLEMENT_WITH_MAIN
#define PICOBENCH_DEFAULT_ITERATIONS {1000, 10000, 100000}
#include "picobench/picobench.hpp"

#include <future>
#include <mutex>
#include <atomic>
#include <functional>

volatile int sum;

std::mutex mut;

template <typename Locker>
int calc_sum(bool inc, const int n, Locker& lock)
{
    for(int i=0; i<n; ++i)
    {
        std::lock_guard<Locker> guard(lock);
        if (inc) sum += 2;
        else sum -= 3;
    }
    return n;
}

template <typename Locker>
void bench(picobench::state& s)
{
    Locker lock;
    sum = 0;
    picobench::scope time(s);
    auto f = std::async(std::launch::async, std::bind(calc_sum<Locker>, true, s.iterations(), std::ref(lock)));
    calc_sum(false, s.iterations(), lock);
    f.wait();
    s.set_result(picobench::result_t(sum));
}

struct spinlock
{
    void lock()
    {
        while(std::atomic_flag_test_and_set_explicit(
            &spin_flag,
            std::memory_order_acquire));
    }

    void unlock()
    {
        std::atomic_flag_clear_explicit(
            &spin_flag,
            std::memory_order_release);
    }

    std::atomic_flag spin_flag = ATOMIC_FLAG_INIT;
} spin;

using namespace std;

PICOBENCH(bench<mutex>);
PICOBENCH(bench<spinlock>);
