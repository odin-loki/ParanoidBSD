#!/bin/sh
# build.sh -- build and run the b0014 differential test.
#
# Compiles the C oracle, the C++23 module and the harness, links them and
# execs the harness so that its exit status is this script's exit status.
# Run as:  sh build.sh

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"
MODULE_NAME=pbsd.lib.libc.softfloat.b0014

rm -rf gcm.cache port.pcm port.o oracle.o harness.o harness

$CC $CFLAGS -c oracle.c -o oracle.o

build_gcc() {
	rm -rf gcm.cache port.o harness.o
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c port.cppm -o port.o &&
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o &&
	$CXX $CXXFLAGS -fmodules-ts -o harness harness.o port.o oracle.o
}

build_clang() {
	rm -f port.pcm port.o harness.o
	$1 $CXXFLAGS --precompile -x c++-module port.cppm -o port.pcm &&
	$1 $CXXFLAGS -c port.pcm -o port.o &&
	$1 $CXXFLAGS -fmodule-file=$MODULE_NAME=port.pcm -c harness.cpp \
	    -o harness.o &&
	$1 $CXXFLAGS -o harness harness.o port.o oracle.o
}

set +e
built=no

if $CXX --version 2>&1 | grep -qi clang; then
	if build_clang "$CXX"; then built=yes; fi
else
	if build_gcc; then built=yes; fi
fi

if [ "$built" = no ]; then
	# Fall back to the other toolchain if it is available.
	if $CXX --version 2>&1 | grep -qi clang; then
		if build_gcc; then built=yes; fi
	elif command -v clang++ >/dev/null 2>&1; then
		if build_clang clang++; then built=yes; fi
	fi
fi

if [ "$built" = no ]; then
	echo "build.sh: failed to build the C++23 module with $CXX" >&2
	exit 1
fi
set -e

exec ./harness
