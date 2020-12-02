# C++ Solutions to Advent of Code 2020

This repository contains solutions to the [Advent of Code 2020](https://adventofcode.com/2020)
Advent puzzles.  All solutions are written in C++.

Each Puzzle lies in its own directory named `puzzle-<day>-<number>`.

The solutions are authored by Matthew Gretton-Dann, and Copyright 2020, Matthew Gretton-Dann

## Build and Run

The build system uses CMake:

```sh
git clone https://github.com/matt-gretton-dann/advent-of-code-2020/
cd advent-of-code-2020
cmake -Bbuild -S.
cmake --build build
```

To run each puzzle then do:

```sh
DAY=01
PUZZLE=01
./build/puzzle-${DAY}-${PUZZLE}/puzzle-${DAY}-${PUZZLE} < puzzle-${DAY}-${PUZZLE}/input.txt
```