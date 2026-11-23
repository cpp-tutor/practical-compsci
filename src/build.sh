#!/bin/sh

COMPILER=g++
[ -n "$1" ] && COMPILER="$*"

$COMPILER -std=c++20 -O3 -o demo main.cpp && echo "Compilation successful! Now run: ./demo"