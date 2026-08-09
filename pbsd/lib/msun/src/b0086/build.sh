#!/bin/sh
# Build and run the b0086 differential test.
set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CXXMODFLAGS=${CXXMODFLAGS:--fmodules-ts}

rm -rf gcm.cache oracle.o port.o harness.o harness

"$CC" -std=c11 -O2 -c oracle.c -o oracle.o
"$CXX" -std=c++23 $CXXMODFLAGS -O2 -x c++ -c port.cppm -o port.o
"$CXX" -std=c++23 $CXXMODFLAGS -O2 -c harness.cpp -o harness.o
"$CXX" port.o harness.o oracle.o -o harness -lm

exec ./harness
