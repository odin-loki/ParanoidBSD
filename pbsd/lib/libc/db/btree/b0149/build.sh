#!/bin/sh
# Build and run the PBSD b0149 differential test.
#
# Usage:  sh build.sh          (from pbsd/lib/libc/db/btree/b0149/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODNAME=pbsd.lib.libc.db.btree.b0149

# The C++ driver must understand C++20 modules.  Prefer whatever "c++" is if
# it is clang, otherwise look for a usable clang++/g++.
pick_cxx() {
	gccfallback=
	for cand in "$CXX" clang++ clang++-18 clang++-17 clang++-16 g++ g++-14 g++-13; do
		command -v "$cand" >/dev/null 2>&1 || continue
		if "$cand" --version 2>&1 | grep -qi clang; then
			echo "$cand clang"
			return 0
		fi
		# Remember the first gcc we saw, but keep looking for clang.
		if [ -z "$gccfallback" ]; then
			gccfallback=$cand
		fi
	done
	if [ -n "$gccfallback" ]; then
		echo "$gccfallback gcc"
		return 0
	fi
	return 1
}

set -- $(pick_cxx)
CXX=$1
FLAVOUR=$2

CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

rm -rf gcm.cache
rm -f oracle.o port.o port.pcm harness.o b0149

$CC $CFLAGS -c oracle.c -o oracle.o

case "$FLAVOUR" in
clang)
	$CXX $CXXFLAGS -x c++-module --precompile port.cppm -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file=$MODNAME=port.pcm -c harness.cpp \
	    -o harness.o
	;;
gcc)
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
	;;
esac

$CXX $CXXFLAGS -o b0149 port.o harness.o oracle.o

exec ./b0149
