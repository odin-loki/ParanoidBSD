#!/bin/sh
# Build and run the differential test for PBSD batch b0079s2.
# Usage: sh build.sh   (from pbsd/lib/msun/src/b0079s2/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODULE_NAME=pbsd.lib.msun.src.b0079s2

rm -rf gcm.cache pcm.cache oracle.o port.o harness.o harness \
    "$MODULE_NAME.pcm"

# The oracle is plain C11 and needs no extras.
$CC -std=c11 -O2 -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 -x c++-module --precompile port.cppm \
	    -o "$MODULE_NAME.pcm"
	$CXX -std=c++23 -c "$MODULE_NAME.pcm" -o port.o
	$CXX -std=c++23 -fmodule-file="$MODULE_NAME=$MODULE_NAME.pcm" \
	    -c harness.cpp -o harness.o
	$CXX -std=c++23 oracle.o port.o harness.o -o harness -lm
else
	# GCC: -fmodules-ts (or -fmodules on newer releases).  The module
	# interface unit must be compiled first so that gcm.cache is populated
	# before harness.cpp is translated.
	MODFLAG=""
	for f in -fmodules-ts -fmodules ""; do
		if $CXX -std=c++23 $f -x c++ -fsyntax-only /dev/null \
		    >/dev/null 2>&1; then
			MODFLAG=$f
			break
		fi
	done

	$CXX -std=c++23 $MODFLAG -O2 -x c++ -c port.cppm -o port.o
	$CXX -std=c++23 $MODFLAG -O2 -c harness.cpp -o harness.o
	$CXX -std=c++23 $MODFLAG oracle.o port.o harness.o -o harness -lm
fi

exec ./harness
