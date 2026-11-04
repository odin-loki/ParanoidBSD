#!/bin/sh
# Build and run the PBSD b0037 differential test.
#
# Usage: sh build.sh    (from pbsd/sys/x86/linux/b0037/)

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"
OUT=b0037_test

MODNAME=pbsd.sys.x86.linux.b0037

rm -rf "$OUT" oracle.o port.o harness.o "$MODNAME.pcm" gcm.cache

$CC $CFLAGS -c oracle.c -o oracle.o

flavour_of() {
	if "$1" --version 2>/dev/null | head -n 1 | grep -qi clang; then
		echo clang
	else
		echo gcc
	fi
}

build_cxx_clang() {
	cxx="$1"
	"$cxx" $CXXFLAGS --precompile -x c++-module port.cppm \
	    -o "$MODNAME.pcm" &&
	"$cxx" $CXXFLAGS -c "$MODNAME.pcm" -o port.o &&
	"$cxx" $CXXFLAGS -fmodule-file="$MODNAME=$MODNAME.pcm" \
	    -c harness.cpp -o harness.o
}

build_cxx_gcc() {
	cxx="$1"
	"$cxx" $CXXFLAGS -fmodules-ts -c -x c++ port.cppm -o port.o &&
	"$cxx" $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
}

build_with() {
	cxx="$1"
	case "$(flavour_of "$cxx")" in
	clang)	build_cxx_clang "$cxx" ;;
	*)	build_cxx_gcc "$cxx" ;;
	esac || return 1
	"$cxx" $CXXFLAGS port.o harness.o oracle.o -o "$OUT" || return 1
	return 0
}

if ! build_with "$CXX"; then
	rm -rf port.o harness.o "$MODNAME.pcm" gcm.cache
	alt=
	if [ "$(flavour_of "$CXX")" = clang ]; then
		for c in g++ ; do
			command -v "$c" >/dev/null 2>&1 && alt="$c" && break
		done
	else
		for c in clang++ clang++-19 clang++-18 clang++-17; do
			command -v "$c" >/dev/null 2>&1 && alt="$c" && break
		done
	fi
	[ -n "$alt" ] || { echo "no usable C++23 modules compiler" >&2; exit 1; }
	echo "note: falling back to $alt for C++ modules" >&2
	build_with "$alt"
fi

exec ./"$OUT"
