#!/bin/sh
#
# Build and run the b0292 differential test.  Run as `sh build.sh' from
# pbsd/lib/libc/regex/b0292/.  The harness's exit status is this script's exit
# status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

MODNAME=pbsd.lib.libc.regex.b0292
HBSD=../../../../../hbsd
HBSD_INC="-I${HBSD}/src/include -I${HBSD}/src/sys -I${HBSD}/src/lib/libc/regex -I${HBSD}/src/lib/libc/locale"
CFLAGS_COMMON="-std=c11 -O2 ${HBSD_INC} -D__unused= -include ${HBSD}/src/lib/libc/include/nostdinc.h 2>/dev/null || true"

rm -rf gcm.cache
rm -f oracle.o port.o harness.o regcomp.o regfree.o port.pcm harness

$CC -std=c11 -O2 ${HBSD_INC} -D__unused= -c oracle.c -o oracle.o
$CC -std=c11 -O2 ${HBSD_INC} -D__unused= -c ${HBSD}/src/lib/libc/regex/regcomp.c -o regcomp.o
$CC -std=c11 -O2 ${HBSD_INC} -D__unused= -c ${HBSD}/src/lib/libc/regex/regfree.c -o regfree.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 -O2 ${HBSD_INC} -D__unused= -x c++-module port.cppm --precompile -o port.pcm
	$CXX -std=c++23 -O2 -c port.pcm -o port.o
	$CXX -std=c++23 -O2 ${HBSD_INC} -D__unused= -fmodule-file="$MODNAME"=port.pcm -c harness.cpp \
	    -o harness.o
else
	$CXX -std=c++23 -O2 ${HBSD_INC} -D__unused= -fmodules-ts -c -x c++ port.cppm -o port.o
	$CXX -std=c++23 -O2 ${HBSD_INC} -D__unused= -fmodules-ts -c harness.cpp -o harness.o
fi

$CXX -std=c++23 -O2 -o harness harness.o port.o oracle.o regcomp.o regfree.o

exec ./harness
