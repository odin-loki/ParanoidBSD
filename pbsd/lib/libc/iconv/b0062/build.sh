#!/bin/sh
# Build and run the b0062 differential test.
# Usage:  sh build.sh   (from pbsd/lib/libc/iconv/b0062/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

OUT=./b0062_harness

MODFLAGS=""
if $CXX -std=c++23 -fmodules-ts -E -x c++ /dev/null >/dev/null 2>&1; then
	MODFLAGS="-fmodules-ts"
elif $CXX -std=c++23 -fmodules -E -x c++ /dev/null >/dev/null 2>&1; then
	MODFLAGS="-fmodules"
fi

rm -rf gcm.cache pcm.cache
mkdir -p pcm.cache

$CC $CFLAGS -c oracle.c -o oracle.o

case "$MODFLAGS" in
-fmodules-ts)
	$CXX $CXXFLAGS $MODFLAGS -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS $MODFLAGS -c harness.cpp -o harness.o
	;;
-fmodules)
	$CXX $CXXFLAGS $MODFLAGS --precompile -x c++-module port.cppm \
	    -o pcm.cache/pbsd.lib.libc.iconv.b0062.pcm
	$CXX $CXXFLAGS $MODFLAGS -c \
	    pcm.cache/pbsd.lib.libc.iconv.b0062.pcm -o port.o
	$CXX $CXXFLAGS $MODFLAGS \
	    -fmodule-file=pbsd.lib.libc.iconv.b0062=pcm.cache/pbsd.lib.libc.iconv.b0062.pcm \
	    -c harness.cpp -o harness.o
	;;
*)
	echo "build.sh: no C++20 module support found in $CXX" >&2
	exit 2
	;;
esac

$CXX $CXXFLAGS -o "$OUT" harness.o port.o oracle.o

exec "$OUT"
