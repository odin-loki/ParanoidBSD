#!/bin/sh
# b0155s3 — build oracle, C++23 port module and differential harness.
# Usage: sh build.sh   (from pbsd/lib/libc/db/btree/b0155s3/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"
OUT=./build
MODNAME=pbsd.lib.libc.db.btree.b0155s3

rm -rf "$OUT"
mkdir -p "$OUT"

$CC $CFLAGS -c oracle.c -o "$OUT/oracle.o"

MODFLAGS=""
if $CXX -std=c++23 -fmodules-ts -E -x c++ /dev/null >/dev/null 2>&1; then
	MODFLAGS="-fmodules-ts"
elif $CXX -std=c++23 -fmodules -E -x c++ /dev/null >/dev/null 2>&1; then
	MODFLAGS="-fmodules"
fi

if [ "$MODFLAGS" = "-fmodules-ts" ]; then
	$CXX $CXXFLAGS $MODFLAGS -fmodule-mapper="|@g++-mapper-server" \
	    -x c++ -c port.cppm -o "$OUT/port.o"
	$CXX $CXXFLAGS $MODFLAGS -fmodule-mapper="|@g++-mapper-server" \
	    -c harness.cpp -o "$OUT/harness.o"
else
	$CXX $CXXFLAGS $MODFLAGS -c -x c++ port.cppm -o "$OUT/port.o"
	$CXX $CXXFLAGS $MODFLAGS -c harness.cpp -o "$OUT/harness.o"
fi

$CXX $CXXFLAGS $MODFLAGS "$OUT/port.o" "$OUT/harness.o" "$OUT/oracle.o" \
    -o "$OUT/harness"

exec "$OUT/harness"
