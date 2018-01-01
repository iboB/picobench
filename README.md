# picobench
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization) [![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)

[![Build Status](https://travis-ci.org/iboB/picobench.svg?branch=master)](https://travis-ci.org/iboB/picobench)

picobench is tiny (micro) microbenchmarking library in a single header file.

It's designed to be easy to use and integrate and fast to compile while covering the most common features of a microbenchmarking library.

## Example usage

Here the complete code microbenchmark which compares adding elements to a `std::vector` with and without using `reserve`:

```c++
#define PICOBENCH_DEBUG
#define PICOBENCH_IMPLEMENT_WITH_MAIN
#include "picobench/picobench.hpp"

#include <vector>
#include <cstdlib> // for rand

static void rand_vector(picobench::state& s)
{
    std::vector<int> v;
    for (auto _ : s)
    {
        v.push_back(rand());
    }
}
PICOBENCH(rand_vector);

static void rand_vector_reserve(picobench::state& s)
{
    std::vector<int> v;
    v.reserve(s.iterations());
    for (auto _ : s)
    {
        v.push_back(rand());
    }
}
PICOBENCH(rand_vector_reserve);
```

The output of this benchmark might look like this:

```
===============================================================================
   Name (baseline is *)   |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
===============================================================================
            rand_vector * |       8 |     0.003 |     320 |      - |  3118908.4
      rand_vector_reserve |       8 |     0.001 |     106 |  0.333 |  9356725.1
            rand_vector * |      64 |     0.009 |     133 |      - |  7482754.6
      rand_vector_reserve |      64 |     0.003 |      40 |  0.300 | 24941543.3
            rand_vector * |     512 |     0.039 |      76 |      - | 13157218.5
      rand_vector_reserve |     512 |     0.013 |      25 |  0.330 | 39909579.9
            rand_vector * |    4096 |     0.257 |      62 |      - | 15937185.1
      rand_vector_reserve |    4096 |     0.202 |      49 |  0.787 | 20249960.4
            rand_vector * |    8196 |     0.466 |      56 |      - | 17583341.7
      rand_vector_reserve |    8196 |     0.360 |      43 |  0.772 | 22789329.4
===============================================================================
```

Which tells us that we see a noticeable performance gain when we use `reserve` but the effect gets less prominent for bigger numbers of elements inserted.

## Documentation

To use picobench you need to include `picobench.hpp` by either copying it inside your project or adding this repo as a submodule to yours.

In one compilation unit (.cpp file) in the module (typically the benchmark executable) in which you use picobench, you need to define `PICOBENCH_IMPLEMENT_WITH_MAIN` (or `PICOBENCH_IMPLEMENT` if you want to write your own `main` function).

If you write your own `main` function, you need to add the following to it, in order to run the benchmarks:

```c++
    picobench::runner runner;
    auto report = runner.run_benchmarks();
    report.to_text(std::cout);
```

Insteaf of `std::cout` you may want to use another `std::ostream` instance of your choice.

### Creating benchmarks

A benchmark is a function which you're written with the signature `void (picobench::state& s)`. You need to register the function with the macro `PICOBENCH(func_name)` where the only argument is the function's name.

You can have multiple benchmarks in multiple files. All will be run when the executable starts.

Typically a benchmark has a loop. To run the loop use the state argument in a range-based for loop in your function. The time spent looping is measured for the benchmark. You can have initialization/deinitialization code outside of the loop and it won't be measured.

Use `state::iterations` as shown in the example to make initialization based on how many iterations the loop will make.

If you don't want the automatic time measurement, you can use `state::start_timer` and `state::stop_timer` to manually measure it.

### Suites

You can optionally create suites of benchmarks. If you don't all benchmarks in the module are assumed to be in the default suite.

To create a suite, write `PICOBENCH_SUITE("suite name in quotes");` and then every benchmark below this line, will be a part of this suite. You can have benchmakrs in many files in the same suite. Just use the same string for its name.

### Baseline

All benchmarks in a suite are assumed to be related and one of them is dubbed "baseline". In the report at the end, all others will be compared to it.

By default the first benchmark added to a suite is the baseline, but you can change this by adding `.baseline()` to the registration like so: `PICOBENCH(my_benchmark).baseline()`.

### Other options

Other characteristics of a benchmark are:

* Iterations: a vector of integers describing the set of iterations to be made for a benchmark. Set with `.iterations({i1, i2, i3...})`. The default is {8, 64, 512, 4096, 8196}.
* Samples: an integer which shows how many samples of a benchmark to make. Set with `.samples(i)`. The default is 1.
* Label: a string which is used for this benchmark in the report instead of the function name. Set with `.label("my label")`

You can combine the options by concatenating them like this: `PICOBENCH(my_func).label("My Function").samples(2).iterations({1000, 10000, 50000});`

If you write your own main function, you can set the default iterations and samples for all benchmarks with `runner::set_default_state_iterations` and `runner::set_default_samples` *before* calling `runner::run_benchmarks`

### Misc

* The runner randomizes the benchmarks. To have the same order on every run and every platform, set an integer seed to `runner::run_benchmarks`.
* Use `report::to_text_concise(ostream&)` for a shorter report which has no breakdown of the different iterations.

## Contributing

Contributions in the form of issues and pull requests are welcome.

## License

This software is distributed under the MIT Software License.

See accompanying file LICENSE.txt or copy [here](https://opensource.org/licenses/MIT).

Copyright &copy; 2017-2018 [Borislav Stanimirov](http://github.com/iboB)
