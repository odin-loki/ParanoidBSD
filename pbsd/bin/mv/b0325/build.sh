#!/bin/sh
# Build and run the PBSD b0325 differential test.
#   sh build.sh
# Exit status is the harness exit status: 0 == every case matched the oracle.
set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2 -D_GNU_SOURCE"
CXXFLAGS="-std=c++23 -O2 -D_GNU_SOURCE"
MODULE_NAME=pbsd.bin.mv.b0325

LIBS=
case "$(uname -s)" in
Linux) LIBS="-lbsd" ;;
esac

rm -rf gcm.cache oracle.o port.o harness.o port.pcm harness

"$CC" $CFLAGS -c oracle.c -o oracle.o

if "$CXX" --version 2>&1 | grep -qi clang; then
	"$CXX" $CXXFLAGS -x c++-module --precompile port.cppm -o port.pcm
	"$CXX" $CXXFLAGS -c port.pcm -o port.o
	"$CXX" $CXXFLAGS -fmodule-file="$MODULE_NAME"=port.pcm \
	    -c harness.cpp -o harness.o
	"$CXX" $CXXFLAGS -o harness port.o harness.o oracle.o $LIBS
else
	"$CXX" $CXXFLAGS -fmodules-ts -x c++ -c port.cppm -o port.o
	"$CXX" $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
	"$CXX" $CXXFLAGS -fmodules-ts -o harness port.o harness.o oracle.o $LIBS
fi

exec ./harness
