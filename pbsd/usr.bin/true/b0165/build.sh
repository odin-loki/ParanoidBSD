#!/bin/sh
#
# Build and run the b0165 differential test.  Run as `sh build.sh' from
# pbsd/usr.bin/true/b0165/.  The harness's exit status is this script's exit
# status.
#
# The logical module name is pbsd.usr.bin.true.b0165, but "true" is a C++
# keyword and cannot appear in a module-name token.  Sources keep the ASCII
# spelling; this script substitutes a Cyrillic-homoglyph segment only for
# compilation (tru + CYRILLIC SMALL LETTER IE).

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--O2}
CXXFLAGS=${CXXFLAGS:--O2}

ASCII_MOD=pbsd.usr.bin.true.b0165
CYR_MOD=$(python3 -c "print('pbsd.usr.bin.tru\u0435.b0165')")

rm -rf gcm.cache
rm -f oracle.o port.o harness.o port.pcm harness port_build.cppm harness_build.cpp

$CC -std=c11 $CFLAGS -c oracle.c -o oracle.o

python3 - "$ASCII_MOD" "$CYR_MOD" <<'PY'
import sys
ascii_mod, cyr_mod = sys.argv[1], sys.argv[2]
for src, dst in (("port.cppm", "port_build.cppm"), ("harness.cpp", "harness_build.cpp")):
    with open(src) as f:
        text = f.read()
    with open(dst, "w") as f:
        f.write(text.replace(ascii_mod, cyr_mod))
PY

build_clang() {
	$1 -std=c++23 $CXXFLAGS -x c++-module port_build.cppm --precompile \
	    -o port.pcm
	$1 -std=c++23 $CXXFLAGS -c port.pcm -o port.o
	$1 -std=c++23 $CXXFLAGS -fmodule-file="$CYR_MOD=port.pcm" \
	    -c harness_build.cpp -o harness.o
	$1 -std=c++23 $CXXFLAGS -o harness harness.o port.o oracle.o
}

build_gcc() {
	$1 -std=c++23 -fmodules-ts $CXXFLAGS -c -x c++ port_build.cppm -o port.o
	$1 -std=c++23 -fmodules-ts $CXXFLAGS -c harness_build.cpp -o harness.o
	$1 -std=c++23 $CXXFLAGS -o harness harness.o port.o oracle.o
}

set +e
built=no

if command -v clang++ >/dev/null 2>&1; then
	if build_clang clang++; then built=yes; fi
fi

if [ "$built" = no ]; then
	if $CXX --version 2>&1 | grep -qi clang; then
		if build_clang "$CXX"; then built=yes; fi
	elif build_gcc "$CXX"; then built=yes; fi
fi

if [ "$built" = no ]; then
	if build_gcc "$CXX"; then built=yes; fi
fi

if [ "$built" = no ]; then
	echo "build.sh: failed to build the C++23 module" >&2
	exit 1
fi
set -e

exec ./harness
