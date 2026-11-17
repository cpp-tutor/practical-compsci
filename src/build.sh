#!/bin/sh

COMPILER=g++
[ -n "$1" ] && COMPILER="$*"

$COMPILER -std=c++23 -O3 -o demo main.cpp && echo "Compilation successful! Now run: ./demo"