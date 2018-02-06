## picobench Tools

### picobench.cpp

An executable which allows the user to benchmark commands.

Usage:

`$ picobench <baseline benchmark> [<benchmark 1> ... <benchmark n>] [args]`

The default number of iterations is one. And the default number of samples is two.

It supports the command-line arguments for the picobench library plus an additional one:

`--bfile=<filename>` - Sets a filename which lists the commands to test as benchmarks

The benchmark file format is:

```
title for baseline
command line for baseline

title for other benchmark 1
command file for benchmark 2

[...]

title for benchmark N
command line for benchmark N
```

Empty lines are ignored.

Examples:

* `$ picobench "sleep 1" "sleep 1.2"`
* `$ picobench --bfile=benchmarks.txt --samples=10 --output=data.csv --out-fmt=csv`


