Colour formatter
================

A command-line colour formatter, using colours defined in a `config` file,
given as assignment for BCIT's COMP 3512 (Object-Oriented Programming in C++).

For example, for a config file with the following commands defined, using
colours instead of bolding, etc.:

    bold        \e[0;31m # red
    italic      \e[0;34m # blue
    underline   \e[0;32m # green
    normal      \e[0;37m # grey

then this given input produces the following input:

![Smoke test showing capabilities.](/readme-img/smoke.png)

The program comes with extensive error messages, detected by line:

![Sample error messages, from the invalid tests.](/readme-img/invalid.png)

The test suite in `a1tests/` was provided by the instructor. My work is found
solely in `a1.cpp`.
