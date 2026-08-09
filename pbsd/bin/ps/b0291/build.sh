#!/bin/sh
set -e
cd "$(dirname "$0")"
CC=${CC:-cc}; CXX=${CXX:-c++}; MOD=pbsd.bin.ps.b0291
rm -rf gcm.cache oracle.o port.o harness.o port.pcm harness
$CC -std=c11 -O2 -c oracle.c -o oracle.o
if $CXX --version 2>&1|grep -qi clang; then
 $CXX -std=c++23 -O2 -x c++-module port.cppm --precompile -o port.pcm
 $CXX -std=c++23 -O2 -c port.pcm -o port.o
 $CXX -std=c++23 -O2 -fmodule-file=$MOD=port.pcm -c harness.cpp -o harness.o
else
 $CXX -std=c++23 -fmodules-ts -O2 -c -x c++ port.cppm -o port.o
 $CXX -std=c++23 -fmodules-ts -O2 -fmodule-file=$MOD=gcm.cache/$MOD.gcm -c harness.cpp -o harness.o
fi
$CXX -std=c++23 -O2 -o harness harness.o port.o oracle.o -lm -lbsd -Wl,--wrap=malloc -Wl,--wrap=exit
exec ./harness
