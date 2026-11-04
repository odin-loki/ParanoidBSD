#!/bin/sh
#
# PBSD batch b0209 -- build and run the differential test.
#
# Usage: sh build.sh

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
OBJ=".b0209.obj"
BIN="./b0209_harness"

# GCC's named module support needs -fmodules-ts, and it only treats .cppm as
# C++ source when told explicitly.  Compiled module interfaces land in
# gcm.cache/ next to the sources.
CXXMODFLAGS="-fmodules-ts"

rm -rf gcm.cache "$OBJ" "$BIN"
mkdir -p "$OBJ"

$CC -std=c11 -O2 -c oracle.c -o "$OBJ/oracle.o"
$CXX -std=c++23 $CXXMODFLAGS -O2 -x c++ -c port.cppm -o "$OBJ/port.o"
$CXX -std=c++23 $CXXMODFLAGS -O2 -c harness.cpp -o "$OBJ/harness.o"
$CXX -std=c++23 $CXXMODFLAGS -O2 -o "$BIN" \
    "$OBJ/oracle.o" "$OBJ/port.o" "$OBJ/harness.o"

rm -rf gcm.cache "$OBJ"

exec "$BIN"
