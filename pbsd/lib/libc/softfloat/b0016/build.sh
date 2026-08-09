#!/bin/sh
#
# build.sh -- build and run the PBSD b0016 differential test.
#
# Compiles oracle.c with cc -std=c11 -O2, port.cppm and harness.cpp with
# c++ -std=c++23 (plus the module flags this toolchain needs), links the
# three objects and execs the harness, so the harness exit status is the
# exit status of this script.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODNAME=pbsd.lib.libc.softfloat.b0016

rm -rf gcm.cache port.pcm port.o oracle.o harness.o harness

"$CC" -std=c11 -O2 -c oracle.c -o oracle.o

if "$CXX" --version 2>&1 | grep -qi clang; then
	# clang: precompile the module interface, then compile it to an object.
	"$CXX" -std=c++23 -O2 -x c++-module --precompile port.cppm -o port.pcm
	"$CXX" -std=c++23 -O2 -c port.pcm -o port.o
	"$CXX" -std=c++23 -O2 -fmodule-file="$MODNAME"=port.pcm \
	    -c harness.cpp -o harness.o
else
	# gcc: named modules live in gcm.cache; the interface must come first.
	"$CXX" -std=c++23 -O2 -fmodules-ts -x c++ -c port.cppm -o port.o
	"$CXX" -std=c++23 -O2 -fmodules-ts -c harness.cpp -o harness.o
fi

"$CXX" -std=c++23 -O2 -o harness harness.o port.o oracle.o

exec ./harness
