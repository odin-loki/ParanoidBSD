#!/bin/sh
set -e
cd "$(dirname "$0")"
CC=${CC:-cc}
CXX=${CXX:-c++}
MOD=pbsd.bin.ps.b0334
rm -rf gcm.cache oracle.o port.o harness.o "$MOD.pcm" b0334_run
$CC -std=c11 -D_GNU_SOURCE -O2 -c oracle.c -o oracle.o
if $CXX --version 2>&1 | grep -qi clang; then
    $CXX -std=c++23 -O2 -x c++-module port.cppm --precompile -o "$MOD.pcm"
    $CXX -std=c++23 -O2 -c "$MOD.pcm" -o port.o
    $CXX -std=c++23 -O2 -fmodule-file=$MOD="$MOD.pcm" -c harness.cpp -o harness.o
else
    $CXX -std=c++23 -fmodules-ts -O2 -c -x c++ port.cppm -o port.o
    $CXX -std=c++23 -fmodules-ts -O2 -fmodule-file=$MOD=gcm.cache/$MOD.gcm -c harness.cpp -o harness.o
fi
$CXX -std=c++23 -O2 -o b0334_run harness.o port.o oracle.o -lm \
    -Wl,--wrap=malloc -Wl,--wrap=exit
exec ./b0334_run
