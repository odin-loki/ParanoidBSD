#!/bin/sh
# Build and run the PBSD batch b0088s2 differential test.
#
# Usage: sh build.sh   (from pbsd/lib/msun/ld128/b0088s2/)
#
# s_sinpil.c is the ld128 implementation: it, and the union IEEEl2bits it
# relies on, require `long double' to be IEEE 754 binary128.  On x86-64 the
# default long double is the x87 80-bit format, whose 16-byte object has six
# bytes of padding exactly where union IEEEl2bits keeps the exponent, so the
# sources are compiled with -mlong-double-128.  The binary128 entry points of
# the libm routines the sources call come from libquadmath.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

MODULE=pbsd.lib.msun.ld128.b0088s2

LD128=-mlong-double-128

check128() {
	cat > conf$$.c <<-'EOF'
	#include <float.h>
	#if LDBL_MANT_DIG != 113
	#error not binary128
	#endif
	int conf_ok(void) { return 0; }
	EOF
	if $1 $LD128 -c conf$$.c -o conf$$.o >/dev/null 2>&1; then
		rm -f conf$$.c conf$$.o
		return 0
	fi
	rm -f conf$$.c conf$$.o
	return 1
}

if ! check128 "$CC" || ! check128 "$CXX"; then
	echo "build.sh: $CC/$CXX cannot provide a binary128 long double;" >&2
	echo "build.sh: -mlong-double-128 is required for ld128 sources." >&2
	exit 1
fi

CFLAGS="-std=c11 -O2 -ffp-contract=off $LD128"
CXXFLAGS="-std=c++23 -O2 -ffp-contract=off $LD128"

rm -rf gcm.cache pcm.cache
rm -f oracle.o port.o harness.o harness

$CC $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	mkdir -p pcm.cache
	$CXX $CXXFLAGS -x c++-module --precompile port.cppm \
	    -o "pcm.cache/$MODULE.pcm"
	$CXX $CXXFLAGS -c "pcm.cache/$MODULE.pcm" -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODULE=pcm.cache/$MODULE.pcm" \
	    -c harness.cpp -o harness.o
	$CXX $CXXFLAGS -o harness harness.o port.o oracle.o -lquadmath -lm
else
	$CXX $CXXFLAGS -fmodules-ts -x c++ -c port.cppm -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
	$CXX $CXXFLAGS -fmodules-ts -o harness harness.o port.o oracle.o \
	    -lquadmath -lm
fi

exec ./harness
