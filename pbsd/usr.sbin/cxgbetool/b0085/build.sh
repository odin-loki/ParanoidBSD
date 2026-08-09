#!/bin/sh
# Build and run the PBSD batch b0085 differential test.
#
# Usage: sh build.sh   (from pbsd/usr.sbin/cxgbetool/b0085/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"
MODNAME=pbsd.usr.sbin.cxgbetool.b0085

MODFLAG=""
for f in -fmodules-ts -fmodules ""; do
	if $CXX $CXXFLAGS $f -x c++ -fsyntax-only /dev/null >/dev/null 2>&1; then
		MODFLAG=$f
		break
	fi
done

rm -rf gcm.cache pcm.cache
mkdir -p pcm.cache gcm.cache
rm -f oracle.o port.o harness.o harness

$CC $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX $CXXFLAGS -x c++-module --precompile port.cppm \
	    -o pcm.cache/$MODNAME.pcm
	$CXX $CXXFLAGS -c pcm.cache/$MODNAME.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file=$MODNAME=pcm.cache/$MODNAME.pcm \
	    -c harness.cpp -o harness.o
else
	$CXX $CXXFLAGS $MODFLAG -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS $MODFLAG -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS $MODFLAG -o harness harness.o port.o oracle.o

exec ./harness
