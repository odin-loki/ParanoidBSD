#!/bin/sh
#
# Build and run the b0183 differential test.  Run as `sh build.sh' from
# pbsd/bin/cpuset/b0183/.  The harness's exit status is this script's exit
# status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--O2}
CXXFLAGS=${CXXFLAGS:--O2}

MODNAME=pbsd.bin.cpuset.b0183

WRAP="-Wl,--wrap=cpuset_getaffinity -Wl,--wrap=cpuset_getdomain"
WRAP="$WRAP -Wl,--wrap=cpuset_getid -Wl,--wrap=cpuset_setid"
WRAP="$WRAP -Wl,--wrap=cpuset -Wl,--wrap=cpuset_setaffinity"
WRAP="$WRAP -Wl,--wrap=cpuset_setdomain -Wl,--wrap=jail_getid"
WRAP="$WRAP -Wl,--wrap=execvp"

rm -rf gcm.cache
rm -f oracle.o port.o harness.o port.pcm harness

$CC -std=c11 $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 $CXXFLAGS -x c++-module port.cppm --precompile \
	    -o port.pcm
	$CXX -std=c++23 $CXXFLAGS -c port.pcm -o port.o
	$CXX -std=c++23 $CXXFLAGS -fmodule-file=$MODNAME=port.pcm \
	    -c harness.cpp -o harness.o
else
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS -c -x c++ port.cppm -o port.o
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS -c harness.cpp -o harness.o
fi

$CXX -std=c++23 $CXXFLAGS $WRAP -o harness harness.o port.o oracle.o

exec ./harness
