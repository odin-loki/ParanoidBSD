#!/bin/sh
#
# build.sh -- build and run the b0208s4 differential test.
#
# Usage: sh build.sh   (from pbsd/sys/compat/linux/b0208s4/)
#
# Compiles the C oracle, the C++23 module port and the harness, links the
# three together and execs the harness so its exit status is this script's
# exit status.
set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"
MODFLAGS="-fmodules-ts"		# g++ named-module support

rm -rf gcm.cache oracle.o port.o harness.o harness

$CC $CFLAGS -c oracle.c -o oracle.o
$CXX $CXXFLAGS $MODFLAGS -x c++ -c port.cppm -o port.o
$CXX $CXXFLAGS $MODFLAGS -c harness.cpp -o harness.o
$CXX $CXXFLAGS $MODFLAGS -o harness harness.o port.o oracle.o

exec ./harness
