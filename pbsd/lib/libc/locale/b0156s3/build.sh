#!/bin/sh
#
# PBSD batch b0156s3 -- build and run the differential test.
#
# Run as `sh build.sh' from pbsd/lib/libc/locale/b0156s3/.  The exit status of
# this script is the exit status of the harness.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--O2}
CXXFLAGS=${CXXFLAGS:--O2}

MODNAME=pbsd.lib.libc.locale.b0156s3

rm -rf gcm.cache
rm -f oracle.o port.o harness.o port.pcm harness

$CC -std=c11 $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	# clang: precompile the interface, then compile it and the importer.
	$CXX -std=c++23 $CXXFLAGS --precompile -x c++-module port.cppm \
	    -o port.pcm
	$CXX -std=c++23 $CXXFLAGS -c port.pcm -o port.o
	$CXX -std=c++23 $CXXFLAGS -fmodule-file=$MODNAME=port.pcm \
	    -c harness.cpp -o harness.o
else
	# GCC: -fmodules-ts, CMI goes through ./gcm.cache.
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS -c -x c++ port.cppm -o port.o
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS -c harness.cpp -o harness.o
fi

$CXX -std=c++23 $CXXFLAGS -o harness harness.o port.o oracle.o

exec ./harness
