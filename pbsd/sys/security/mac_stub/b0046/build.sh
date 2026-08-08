#!/bin/sh
set -e
cd "$(dirname "$0")"

cc -std=c11 -O2 -c -o oracle.o oracle.c

if c++ -std=c++23 -fmodules-ts -xc++-system-header cstdint 2>/dev/null; then
    MODFLAG=-fmodules-ts
else
    MODFLAG=-fmodules
fi

for hdr in cstdint cstdio cstdlib cstring vector string; do
    c++ -std=c++23 $MODFLAG -xc++-system-header "$hdr"
done

c++ -std=c++23 $MODFLAG -c port.cppm -o port.o
c++ -std=c++23 $MODFLAG -c harness.cpp -o harness.o
c++ -std=c++23 $MODFLAG -o harness oracle.o port.o harness.o

./harness
