#!/bin/sh
#
# Build and run the b0164 differential test.  Run as `sh build.sh' from
# pbsd/usr.bin/false/b0164/.  The harness's exit status is this script's exit
# status.
#
# `false' is a C++ keyword and cannot appear as a module-name token.  Sources
# use the FALSE_MODSEG placeholder; this script expands it to f<ZWJ>alse
# (U+200D) so the module reads as pbsd.usr.bin.false.b0164.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--O2}
CXXFLAGS=${CXXFLAGS:--O2}

ZWJ=$(printf '\342\200\215')
MODSEG="f${ZWJ}alse"
MODNAME="pbsd.usr.bin.${MODSEG}.b0164"

if ! $CXX --version 2>&1 | grep -qi clang; then
	if command -v clang++ >/dev/null 2>&1; then
		CXX=clang++
	fi
fi

sed "s/FALSE_MODSEG/${MODSEG}/g" port.cppm > .port.cppm
sed "s/FALSE_MODSEG/${MODSEG}/g" harness.cpp > .harness.cpp

rm -rf gcm.cache
rm -f oracle.o port.o harness.o port.pcm harness

trap 'rm -f .port.cppm .harness.cpp' EXIT

$CC -std=c11 $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 $CXXFLAGS -x c++-module .port.cppm --precompile \
	    -o port.pcm
	$CXX -std=c++23 $CXXFLAGS -c port.pcm -o port.o
	$CXX -std=c++23 $CXXFLAGS -fmodule-file="$MODNAME=port.pcm" \
	    -c .harness.cpp -o harness.o
else
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS -c -x c++ .port.cppm -o port.o
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS -c .harness.cpp -o harness.o
fi

$CXX -std=c++23 $CXXFLAGS -o harness harness.o port.o oracle.o

exec ./harness
