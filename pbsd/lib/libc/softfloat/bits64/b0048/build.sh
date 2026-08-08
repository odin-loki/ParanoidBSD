#!/bin/sh
# Build and run the b0048 differential test.
#
#   sh build.sh
#
# oracle.c is compiled as C11, port.cppm and harness.cpp as C++23, the three
# objects are linked, and the harness is exec'd so that its exit status is this
# script's exit status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"
MODNAME=pbsd.lib.libc.softfloat.bits64.b0048
BUILD=build

rm -rf "$BUILD"
mkdir -p "$BUILD"

# Pick a C++ driver that can actually build a module interface unit.  Clang
# wants --precompile plus -fprebuilt-module-path; GCC wants -fmodules-ts.
module_flavour() {
    if "$1" -std=c++23 -fmodules-ts -x c++ -fsyntax-only /dev/null \
        >/dev/null 2>&1; then
        echo gcc
        return 0
    fi
    if "$1" -std=c++23 -x c++-module --precompile /dev/null \
        -o "$BUILD/probe.pcm" >/dev/null 2>&1; then
        rm -f "$BUILD/probe.pcm"
        echo clang
        return 0
    fi
    return 1
}

FLAVOUR=$(module_flavour "$CXX" || true)
if [ -z "$FLAVOUR" ]; then
    for cand in clang++ clang++-18 clang++-17 clang++-16 g++ g++-14 g++-13; do
        command -v "$cand" >/dev/null 2>&1 || continue
        FLAVOUR=$(module_flavour "$cand" || true)
        if [ -n "$FLAVOUR" ]; then
            CXX=$cand
            break
        fi
    done
fi
if [ -z "$FLAVOUR" ]; then
    echo "build.sh: no C++ compiler with module support found" >&2
    exit 1
fi

echo "using $CXX ($FLAVOUR modules), $CC for the oracle"

$CC $CFLAGS -c oracle.c -o "$BUILD/oracle.o"

case "$FLAVOUR" in
clang)
    $CXX $CXXFLAGS --precompile -x c++-module port.cppm \
        -o "$BUILD/$MODNAME.pcm"
    $CXX $CXXFLAGS -c "$BUILD/$MODNAME.pcm" -o "$BUILD/port.o"
    $CXX $CXXFLAGS -fprebuilt-module-path="$BUILD" -c harness.cpp \
        -o "$BUILD/harness.o"
    ;;
gcc)
    $CXX $CXXFLAGS -fmodules-ts -fmodule-mapper= -x c++ -c port.cppm \
        -o "$BUILD/port.o" 2>/dev/null ||
        $CXX $CXXFLAGS -fmodules-ts -x c++ -c port.cppm -o "$BUILD/port.o"
    $CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o "$BUILD/harness.o"
    ;;
esac

$CXX $CXXFLAGS "$BUILD/harness.o" "$BUILD/port.o" "$BUILD/oracle.o" \
    -o "$BUILD/b0048_test"

exec "$BUILD/b0048_test"
