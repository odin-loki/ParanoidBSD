#!/bin/sh
#
# Build and run the b0164 differential test.  Run as `sh build.sh' from
# pbsd/usr.bin/false/b0164/.  The harness's exit status is this script's exit
# status.
#
# port.cppm and harness.cpp spell the module name in plain ASCII as
# pbsd.usr.bin.false.b0164.  `false' is a C++ keyword and therefore not a
# legal module-name token, so this script rewrites that one segment in
# throwaway build copies of the sources before compiling.  clang accepts
# f<ZWJ>alse (U+200D ZERO WIDTH JOINER), which reads as `false'; gcc's module
# mapper cannot cope with a non-ASCII module name, so the gcc fallback path
# uses the ASCII segment `false_' instead.  The sources themselves are never
# modified.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--O2}
CXXFLAGS=${CXXFLAGS:--O2}

ASCII_MOD='pbsd\.usr\.bin\.false\.b0164'

rm -rf gcm.cache
rm -f oracle.o port.o harness.o port.pcm harness .port.cppm .harness.cpp

trap 'rm -f .port.cppm .harness.cpp' EXIT INT HUP TERM

$CC -std=c11 $CFLAGS -c oracle.c -o oracle.o

gen_sources() {
	sed "s/$ASCII_MOD/$1/g" port.cppm > .port.cppm
	sed "s/$ASCII_MOD/$1/g" harness.cpp > .harness.cpp
}

build_clang() {
	MOD="pbsd.usr.bin.f$(printf '\342\200\215')alse.b0164"
	gen_sources "$MOD"
	rm -f port.pcm port.o harness.o harness
	$1 -std=c++23 $CXXFLAGS -Wno-unicode-zero-width \
	    -x c++-module .port.cppm --precompile -o port.pcm &&
	$1 -std=c++23 $CXXFLAGS -c port.pcm -o port.o &&
	$1 -std=c++23 $CXXFLAGS -Wno-unicode-zero-width \
	    -fmodule-file="$MOD=port.pcm" -c .harness.cpp -o harness.o &&
	$1 -std=c++23 $CXXFLAGS -o harness harness.o port.o oracle.o
}

build_gcc() {
	MOD=pbsd.usr.bin.false_.b0164
	gen_sources "$MOD"
	rm -rf gcm.cache
	rm -f port.o harness.o harness
	$1 -std=c++23 -fmodules-ts $CXXFLAGS -c -x c++ .port.cppm -o port.o &&
	$1 -std=c++23 -fmodules-ts $CXXFLAGS -c .harness.cpp -o harness.o &&
	$1 -std=c++23 $CXXFLAGS -o harness harness.o port.o oracle.o
}

set +e
built=no

if $CXX --version 2>&1 | grep -qi clang; then
	if build_clang "$CXX"; then built=yes; fi
fi

if [ "$built" = no ] && command -v clang++ >/dev/null 2>&1; then
	if build_clang clang++; then built=yes; fi
fi

if [ "$built" = no ]; then
	if build_gcc "$CXX"; then built=yes; fi
fi

if [ "$built" = no ]; then
	echo "build.sh: failed to build the C++23 module" >&2
	exit 1
fi
set -e

rm -f .port.cppm .harness.cpp

exec ./harness
