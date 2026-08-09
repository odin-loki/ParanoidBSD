#!/bin/sh
# Build and run the differential test for PBSD batch b0073.
# Usage: sh build.sh   (from pbsd/lib/msun/ld128/b0073/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE_NAME=pbsd.lib.msun.ld128.b0073

rm -rf gcm.cache pcm.cache oracle.o port.o harness.o harness \
    "$MODULE_NAME.pcm"

# These are ld128 sources: they are written for a 128-bit long double.  If the
# toolchain can give us one on this host, ask for it -- otherwise the tail
# coefficients of the polynomial fall below the rounding threshold.  The flag
# changes the long double ABI, so it must be all three translation units or
# none of them.
LDFLAG=""
cat > .ldprobe.c <<'EOF'
#include <float.h>
#if LDBL_MANT_DIG != 113
#error not binary128
#endif
int main(void) { long double x = 1.0L; return x == 0.0L; }
EOF
if $CC -std=c11 -mlong-double-128 -c .ldprobe.c -o .ldprobe.o >/dev/null 2>&1 &&
    $CXX -std=c++23 -mlong-double-128 -x c++ -c .ldprobe.c -o .ldprobe.o \
    >/dev/null 2>&1; then
	LDFLAG="-mlong-double-128"
fi
rm -f .ldprobe.c .ldprobe.o

# Keep both sides strictly source-ordered so the comparison is meaningful.
FPFLAG=""
if $CC -std=c11 -ffp-contract=off -x c -fsyntax-only /dev/null \
    >/dev/null 2>&1; then
	FPFLAG="-ffp-contract=off"
fi

$CC -std=c11 -O2 $LDFLAG $FPFLAG -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 -O2 $LDFLAG $FPFLAG -x c++-module --precompile \
	    port.cppm -o "$MODULE_NAME.pcm"
	$CXX -std=c++23 -O2 $LDFLAG $FPFLAG -c "$MODULE_NAME.pcm" -o port.o
	$CXX -std=c++23 -O2 $LDFLAG $FPFLAG \
	    -fmodule-file="$MODULE_NAME=$MODULE_NAME.pcm" \
	    -c harness.cpp -o harness.o
	$CXX -std=c++23 $LDFLAG oracle.o port.o harness.o -o harness -lm
else
	MODFLAG=""
	for f in -fmodules-ts -fmodules ""; do
		if $CXX -std=c++23 $f -x c++ -fsyntax-only /dev/null \
		    >/dev/null 2>&1; then
			MODFLAG=$f
			break
		fi
	done

	$CXX -std=c++23 $MODFLAG -O2 $LDFLAG $FPFLAG -x c++ -c port.cppm \
	    -o port.o
	$CXX -std=c++23 $MODFLAG -O2 $LDFLAG $FPFLAG -c harness.cpp \
	    -o harness.o
	$CXX -std=c++23 $MODFLAG $LDFLAG oracle.o port.o harness.o \
	    -o harness -lm
fi

exec ./harness
