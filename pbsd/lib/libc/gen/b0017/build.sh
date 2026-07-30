#!/bin/sh
#
# Build and run the batch b0017 differential test.
#
#	sh build.sh
#
# Compiles the unmodified C oracle, the C++23 module port and the harness,
# links them together and execs the harness, so the exit code of this script is
# the exit code of the test.

set -eu

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:-"-O2"}
CXXFLAGS=${CXXFLAGS:-"-O2"}

MODULE=pbsd.lib.libc.gen.b0017

rm -rf gcm.cache port.pcm oracle.o port.o harness.o harness

"$CC" -std=c11 $CFLAGS -c oracle.c -o oracle.o

# The two toolchains spell the module flags differently: GCC drives everything
# off -fmodules-ts and a gcm.cache directory, clang wants an explicit precompile
# step and the resulting .pcm handed to the importer.
build_cxx() {
	cxx=$1

	if "$cxx" --version 2>&1 | grep -qi clang; then
		"$cxx" -std=c++23 $CXXFLAGS --precompile -x c++-module \
		    port.cppm -o port.pcm
		"$cxx" -std=c++23 $CXXFLAGS -c port.pcm -o port.o
		"$cxx" -std=c++23 $CXXFLAGS \
		    -fmodule-file="$MODULE"=port.pcm -c harness.cpp -o harness.o
		"$cxx" -std=c++23 $CXXFLAGS -o harness \
		    oracle.o port.o harness.o
	else
		rm -rf gcm.cache
		"$cxx" -std=c++23 $CXXFLAGS -fmodules-ts -c -x c++ \
		    port.cppm -o port.o
		"$cxx" -std=c++23 $CXXFLAGS -fmodules-ts -c \
		    harness.cpp -o harness.o
		"$cxx" -std=c++23 $CXXFLAGS -fmodules-ts -o harness \
		    oracle.o port.o harness.o
	fi
}

if ! build_cxx "$CXX"; then
	# Named modules are still patchy in some releases; fall back to any
	# other C++ driver on the box rather than skipping the test.
	built=0
	for alt in clang++ clang++-18 clang++-17 g++-14 g++-13; do
		if command -v "$alt" >/dev/null 2>&1; then
			echo "build.sh: $CXX failed on modules, retrying with $alt" >&2
			rm -rf gcm.cache port.pcm port.o harness.o
			if build_cxx "$alt"; then
				built=1
				break
			fi
		fi
	done
	if [ "$built" -ne 1 ]; then
		echo "build.sh: no C++ compiler on this box could build the module" >&2
		exit 2
	fi
fi

exec ./harness
