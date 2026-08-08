#!/bin/sh
set -e
cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

rm -f oracle.o port.o harness.o harness \
	pbsd.lib.libc.secure.b0240.o \
	pbsd.lib.libc.secure.b0240.gcm

"$CC" -std=c11 -O2 -DPIC -c oracle.c -o oracle.o

if "$CXX" --version 2>/dev/null | grep -qi clang; then
	MODFLAGS="-std=c++23 -fprebuilt-module-path=."
	"$CXX" $MODFLAGS -DPIC -x c++-module -c port.cppm -o pbsd.lib.libc.secure.b0240.o
	"$CXX" $MODFLAGS -DPIC -c harness.cpp -o harness.o
	"$CXX" $MODFLAGS harness.o port.o oracle.o -o harness
else
	MODFLAGS="-std=c++23 -fmodules-ts"
	"$CXX" $MODFLAGS -DPIC -c port.cppm -o port.o
	"$CXX" $MODFLAGS -DPIC -c harness.cpp -o harness.o
	"$CXX" $MODFLAGS harness.o port.o oracle.o -o harness
fi

exec ./harness
