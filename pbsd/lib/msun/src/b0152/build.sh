#!/bin/sh
# Build and run the b0152 differential test.
# Usage: sh build.sh   (from pbsd/lib/msun/src/b0152/)
#
# Everything the build produces, including the compiled module interface,
# is written under ./_build so the source directory stays clean.
set -e

CC=${CC:-cc}
CXX=${CXX:-c++}
SRC=$(pwd)
BUILD="$SRC/_build"

rm -rf "$BUILD"
mkdir -p "$BUILD"
cd "$BUILD"

$CC -std=c11 -O2 -c "$SRC/oracle.c" -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	# clang: precompile the interface, then point the importer at it.
	$CXX -std=c++23 -O2 -x c++-module "$SRC/port.cppm" --precompile \
		-o pbsd.lib.msun.src.b0152.pcm
	$CXX -std=c++23 -O2 -c pbsd.lib.msun.src.b0152.pcm -o port.o
	$CXX -std=c++23 -O2 \
		-fmodule-file=pbsd.lib.msun.src.b0152=pbsd.lib.msun.src.b0152.pcm \
		-c "$SRC/harness.cpp" -o harness.o
	$CXX -std=c++23 -O2 -o harness harness.o port.o oracle.o -lm
else
	# gcc: -fmodules-ts, with the gcm.cache written here in $BUILD.
	MODFLAG=-fmodules-ts
	$CXX -std=c++23 $MODFLAG -x c++ -fsyntax-only /dev/null 2>/dev/null ||
		MODFLAG=-fmodules
	# The interface unit must be compiled first so that the importer finds
	# the compiled module in the cache.
	$CXX -std=c++23 $MODFLAG -O2 -x c++ -c "$SRC/port.cppm" -o port.o
	$CXX -std=c++23 $MODFLAG -O2 -c "$SRC/harness.cpp" -o harness.o
	$CXX -std=c++23 $MODFLAG -O2 -o harness harness.o port.o oracle.o -lm
fi

exec ./harness
