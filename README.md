# picobench
[![Language](https://img.shields.io/badge/language-C++-blue.svg)](https://isocpp.org/) [![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization) [![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)

[![Build Status](https://travis-ci.org/iboB/picobench.svg?branch=master)](https://travis-ci.org/iboB/picobench)

picobench is tiny (micro) microbenchmarking library in a single header file.

It's designed to be easy to use and integrate and fast to compile while covering the most common features of a microbenchmarking library.

## Example usage

Here's the complete code of a microbenchmark which compares adding elements to a `std::vector` with and without using `reserve`:

```c++
#define PICOBENCH_IMPLEMENT_WITH_MAIN
#include "picobench/picobench.hpp"

#include <vector>
#include <cstdlib> // for rand

// Benchmarking function written by the user:
static void rand_vector(picobench::state& s)
{
    std::vector<int> v;
    for (auto _ : s)
    {
        v.push_back(rand());
    }
}
PICOBENCH(rand_vector); // Register the above function with picobench

// Another benchmarking function:
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
            rand_vector * |       8 |     0.001 |     167 |      - |  5974607.9
      rand_vector_reserve |       8 |     0.000 |      55 |  0.329 | 18181818.1
            rand_vector * |      64 |     0.004 |      69 |      - | 14343343.8
      rand_vector_reserve |      64 |     0.002 |      27 |  0.400 | 35854341.7
            rand_vector * |     512 |     0.017 |      33 |      - | 30192239.7
      rand_vector_reserve |     512 |     0.012 |      23 |  0.710 | 42496679.9
            rand_vector * |    4096 |     0.181 |      44 |      - | 22607850.9
      rand_vector_reserve |    4096 |     0.095 |      23 |  0.527 | 42891848.9
            rand_vector * |    8196 |     0.266 |      32 |      - | 30868196.3
      rand_vector_reserve |    8196 |     0.207 |      25 |  0.778 | 39668749.5
===============================================================================
```

...which tells us that we see a noticeable performance gain when we use `reserve` but the effect gets less prominent for bigger numbers of elements inserted.

## Documentation

To use picobench, you need to include `picobench.hpp` by either copying it inside your project or adding this repo as a submodule to yours.

In one compilation unit (.cpp file) in the module (typically the benchmark executable) in which you use picobench, you need to define `PICOBENCH_IMPLEMENT_WITH_MAIN` (or `PICOBENCH_IMPLEMENT` if you want to write your own `main` function).

### Creating benchmarks

A benchmark is a function which you're written with the signature `void (picobench::state& s)`. You need to register the function with the macro `PICOBENCH(func_name)` where the only argument is the function's name as shown in the example above.

The library will run the benchmark function several times with different numbers of iterations, to simulate different problem spaces, then collect the results in a report.

Typically a benchmark has a loop. To run the loop, use the `picobench::state` argument in a range-based for loop in your function. The time spent looping is measured for the benchmark. You can have initialization/deinitialization code outside of the loop and it won't be measured.

You can have multiple benchmarks in multiple files. All of them will be run when the executable starts.

Use `state::iterations` as shown in the example to make initialization based on how many iterations the loop will make.

If you don't want the automatic time measurement, you can use `state::start_timer` and `state::stop_timer` to manually measure it, or use the RAII class `picobench::scope` for semi-automatic measurement.

Here's an example of a couple of benchmarks, which does not use the range-based for loop for time measurement:

```c++
void my_func(); // Function you want to benchmark
static void benchmark_my_func(picobench::state& s)
{
    s.start_timer(); // Manual start
    for (int i=0; i<s.iterations(); ++i)
        my_func();
    s.stop_timer(); // Manual stop
}
PICOBENCH(benchmark_my_func);

void my_func2();
static void benchmark_my_func2(picobench::state& s)
{
    custom_init(); // Some user-defined initialization
    picobench::scope scope(s); // Constructor starts measurement. Destrucror stops it
    for (int i=0; i<s.iterations(); ++i)
        my_func2();
}
PICOBENCH(benchmark_my_func2);
```

### Custom main function

If you write your own `main` function, you need to add the following to it in order to run the benchmarks:

```c++
    picobench::runner runner;
    // Optionally parse command line
    runner.parse_cmd_line(argc, argv);
    if (runner.should_run()) // Cmd line may have disabled benchmarks
    {
        auto report = runner.run_benchmarks();
        // Then to output the data in the report use
        report.to_text(std::cout); // Default
        // or
        report.to_text_consise(std::cout); // No iterations breakdown
        // or
        report.to_csv(std::cout); // Otputs in csv format. Most detailed
    }
```

Instead of `std::cout` you may want to use another `std::ostream` instance of your choice.

As mentioned above `report.to_text_concise(ostream)` outputs a report without the iterations breakdown. With the first example of benchmarking adding elements to a `std::vector`, the output would be this:

```
===============================================================================
   Name (baseline is *)   |  ns/op  | Baseline |  Ops/second
===============================================================================
            rand_vector * |      36 |        - |  27427782.7
      rand_vector_reserve |      24 |    0.667 |  40754573.7
===============================================================================
```

Note that in this case the information that the effect of using `reserve` gets less prominent with more elements is lost.

### Suites

You can optionally create suites of benchmarks. If you don't, all benchmarks in the module are assumed to be in the default suite.

To create a suite, write `PICOBENCH_SUITE("suite name in quotes");` and then every benchmark below this line will be a part of this suite. You can have benchmakrs in many files in the same suite. Just use the same string for its name.

### Baseline

All benchmarks in a suite are assumed to be related and one of them is dubbed a "baseline". In the report at the end, all others will be compared to it.

By default the first benchmark added to a suite is the baseline, but you can change this by adding `.baseline()` to the registration like so: `PICOBENCH(my_benchmark).baseline()`.

### Samples

Sometimes the code being benchmarked is very sensitive to external factors such as syscalls (which include memory allocation and deallocation). Those external factors can have take greatly different times between runs. In such cases several samples of a benchmark might be needed to more precisely measure the time it takes to complete. By default the library makes two samples of each benchmark, but you can change this by adding `.samples(n)` to the registration like so: `PICOBENCH(my_benchmark).samples(10)`.

Note that the time written to the report is the one of the *fastest* sample.

### Other options

Other characteristics of a benchmark are:

* **Iterations**: (or "problem spaces") a vector of integers describing the set of iterations to be made for a benchmark. Set with `.iterations({i1, i2, i3...})`. The default is {8, 64, 512, 4096, 8196}.
* **Label**: a string which is used for this benchmark in the report instead of the function name. Set with `.label("my label")`

You can combine the options by concatenating them like this: `PICOBENCH(my_func).label("My Function").samples(2).iterations({1000, 10000, 50000});`

If you write your own main function, you can set the default iterations and samples for all benchmarks with `runner::set_default_state_iterations` and `runner::set_default_samples` *before* calling `runner::run_benchmarks`.

If you parse the command line or use the library-provided `main` function you can also set the iterations and samples with command line args:
* `--iters=1000,5000,10000` will set the iterations for benchmarks which don't explicitly override them
* `--samples=5` will set the samples for benchmarks which don't explicitly override them

### Other command line arguments

If you're using the library-provided `main` function, it will also handle the following command line arguments:
* `--out-fmt=<txt|con|csv>` - sets the output report format to either full text, concise text or csv.
* `--output=<filename>` - writes the output report to a given file

### Misc

* The runner randomizes the benchmarks. To have the same order on every run and every platform, set an integer seed to `runner::run_benchmarks`.

Here's another example of a custom main function incporporating the above:

```c++
#define PICOBENCH_IMPLEMENT
#include "picobench/picobench.hpp"
...
int main()
{
    // User-defined code which makes global initializations
    custom_global_init();

    picobench::runner runner;
    // Disregard command-line for simplicity

    // Two sets of iterations
    runner.set_default_state_iterations({10000, 50000});

    // One sample per benchmark because the huge numbers are expected to compensate
    // for external factors
    runner.set_default_samples(1);

    // Run the benchmarks with some seed which guarantees the same order every time
    auto report = runner.run_benchmarks(123);

    // Output to some file
    report.to_csv(ofstream("my.csv"));

    return 0;
}
```

## Contributing

Contributions in the form of issues and pull requests are welcome.

## License

This software is distributed under the MIT Software License.

See accompanying file LICENSE.txt or copy [here](https://opensource.org/licenses/MIT).

Copyright &copy; 2017-2018 [Borislav Stanimirov](http://github.com/iboB)
