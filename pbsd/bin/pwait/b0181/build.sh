#!/bin/sh
#
# Build and run the b0181 differential test.  Run as `sh build.sh' from
# pbsd/bin/pwait/b0181/.  The harness's exit status is this script's exit
# status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--O2}
CXXFLAGS=${CXXFLAGS:--O2}

MODNAME=pbsd.bin.pwait.b0181

BSD_INC=""
if [ -d /usr/include/x86_64-linux-gnu/bsd ]; then
	BSD_INC=/usr/include/x86_64-linux-gnu/bsd
elif [ -d /usr/include/bsd ]; then
	BSD_INC=/usr/include/bsd
fi

EXTRA_INC=""
if [ -n "$BSD_INC" ]; then
	EXTRA_INC="-I$BSD_INC"
fi

rm -rf gcm.cache
rm -f oracle.o port.o harness.o port.pcm harness

$CC -std=c11 $CFLAGS $EXTRA_INC -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 $CXXFLAGS $EXTRA_INC -x c++-module port.cppm --precompile \
	    -o port.pcm
	$CXX -std=c++23 $CXXFLAGS $EXTRA_INC -c port.pcm -o port.o
	$CXX -std=c++23 $CXXFLAGS $EXTRA_INC -fmodule-file=$MODNAME=port.pcm \
	    -c harness.cpp -o harness.o
else
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS $EXTRA_INC -c -x c++ port.cppm \
	    -o port.o
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS $EXTRA_INC -c harness.cpp \
	    -o harness.o
fi

$CXX -std=c++23 $CXXFLAGS $EXTRA_INC \
    -Wl,--wrap=kqueue -Wl,--wrap=kevent -Wl,--wrap=sysctlbyname \
    -Wl,--wrap=getpid -Wl,--wrap=setitimer -Wl,--wrap=malloc \
    -o harness harness.o port.o oracle.o

exec ./harness
