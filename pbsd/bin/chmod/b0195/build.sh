#!/bin/sh
#
# Build and run the b0195 differential test.  Run as `sh build.sh' from
# pbsd/bin/chmod/b0195/.  The harness's exit status is this script's exit
# status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--O2}
CXXFLAGS=${CXXFLAGS:--O2}

MODNAME=pbsd.bin.chmod.b0195

LIBS=
case "$(uname -s)" in
Linux) LIBS="-lbsd" ;;
esac

WRAP_LDFLAGS="-Wl,--wrap=fchmodat -Wl,--wrap=fts_open -Wl,--wrap=fts_read \
    -Wl,--wrap=fts_set -Wl,--wrap=fts_close -Wl,--wrap=pathconf \
    -Wl,--wrap=lpathconf -Wl,--wrap=exit"

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

$CXX -std=c++23 $CXXFLAGS $WRAP_LDFLAGS -o harness harness.o port.o oracle.o $LIBS

exec ./harness
