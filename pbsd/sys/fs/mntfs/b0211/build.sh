#!/bin/sh
#
# build.sh -- compile and run the PBSD b0211 differential test.
#
# Usage: sh build.sh   (from pbsd/sys/fs/mntfs/b0211/)

set -e

dir=`CDPATH= cd -- "\`dirname -- "$0"\`" && pwd`
cd "$dir"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--std=c11 -O2}
CXXFLAGS=${CXXFLAGS:--std=c++23 -O2}
MODULE_NAME=pbsd.sys.fs.mntfs.b0211
BIN=b0211_test

rm -rf gcm.cache oracle.o port.o harness.o port.pcm "$BIN"

$CC $CFLAGS -c oracle.c -o oracle.o

# GCC spells C++20 modules -fmodules-ts (13.x) or -fmodules (15.x); clang wants
# an explicit precompile step.  Probe for whichever this toolchain accepts.
modflags=

for f in -fmodules-ts -fmodules; do
	if $CXX $CXXFLAGS $f -x c++ -c port.cppm -o port.o >/dev/null 2>&1; then
		modflags=$f
		break
	fi
	rm -rf gcm.cache port.o
done

if [ -n "$modflags" ]; then
	$CXX $CXXFLAGS $modflags -c harness.cpp -o harness.o
	$CXX $CXXFLAGS $modflags -o "$BIN" harness.o port.o oracle.o
else
	rm -rf gcm.cache port.o
	$CXX $CXXFLAGS -x c++-module port.cppm --precompile -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODULE_NAME"=port.pcm \
	    -c harness.cpp -o harness.o
	$CXX $CXXFLAGS -o "$BIN" harness.o port.o oracle.o
fi

exec ./"$BIN"
