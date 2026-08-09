#!/bin/sh
#
# Build and run the batch b0194s3 differential test.
# Usage: sh build.sh   (from pbsd/lib/libc/db/hash/b0194s3/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

# Module flags differ between the two toolchains that ship a C++23 front end.
if "$CXX" --version 2>&1 | grep -i clang >/dev/null 2>&1; then
	MODFLAGS="-fmodule-output=pbsd.lib.libc.db.hash.b0194s3.pcm"
	IMPFLAGS="-fprebuilt-module-path=."
else
	MODFLAGS="-fmodules-ts"
	IMPFLAGS="-fmodules-ts"
	rm -rf gcm.cache
fi

rm -f oracle.o port.o harness.o harness

"$CC" -std=c11 -O2 -c oracle.c -o oracle.o

"$CXX" -std=c++23 -O2 $MODFLAGS -c -x c++ port.cppm -o port.o

"$CXX" -std=c++23 -O2 $IMPFLAGS -c harness.cpp -o harness.o

"$CXX" -std=c++23 -O2 $IMPFLAGS oracle.o port.o harness.o -o harness

exec ./harness
