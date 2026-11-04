#!/bin/sh
#
# Build and run the PBSD b0001 differential test.
#
# Usage:  sh build.sh          (from pbsd/lib/msun/src/b0001/)
#
# Compiles oracle.c with the C compiler, port.cppm + harness.cpp with the
# C++ compiler in C++23 module mode, links the three together and execs
# the harness, so the harness exit status is this script's exit status.

set -e

srcdir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$srcdir"

CC=${CC:-cc}
CXX=${CXX:-c++}
MODNAME=pbsd.lib.msun.src.b0001

# These functions exist to manipulate the floating point environment, so
# both sides must be built with the semantics the C standard gives to a
# translation unit with FENV_ACCESS ON.  GCC does not implement that
# pragma; -frounding-math is its equivalent, and -fno-builtin-rint[f]
# suppresses the inline expansion of rint()/rintf() as the
# (|x| + 2^52) - 2^52 magic-number trick, which only computes
# round-to-nearest and therefore silently mis-rounds under any other
# rounding mode.  Without these, the oracle stops implementing its own
# source and the comparison measures GCC's builtin expander instead of
# the port.  Applied identically to both sides.
FENVFLAGS='-frounding-math -fno-builtin-rint -fno-builtin-rintf'

CFLAGS=${CFLAGS:--std=c11 -O2}
CXXFLAGS=${CXXFLAGS:--std=c++23}

builddir=$srcdir/build
rm -rf "$builddir"
mkdir -p "$builddir"

# All module artifacts (gcm.cache / *.pcm) are produced relative to the
# working directory, so build from inside builddir to keep the source
# directory clean.
cd "$builddir"

# shellcheck disable=SC2086
$CC $CFLAGS $FENVFLAGS -c ../oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	# Clang: explicit precompile step, then feed the BMI to the importer.
	# shellcheck disable=SC2086
	$CXX $CXXFLAGS $FENVFLAGS --precompile -x c++-module ../port.cppm -o port.pcm
	# shellcheck disable=SC2086
	$CXX $CXXFLAGS $FENVFLAGS -c port.pcm -o port.o
	# shellcheck disable=SC2086
	$CXX $CXXFLAGS $FENVFLAGS -fmodule-file=$MODNAME=port.pcm -c ../harness.cpp \
	    -o harness.o
else
	# GCC: -fmodules-ts, CMI goes into ./gcm.cache automatically.
	# g++ does not recognise the .cppm suffix, hence -x c++.
	# shellcheck disable=SC2086
	$CXX $CXXFLAGS $FENVFLAGS -fmodules-ts -x c++ -c ../port.cppm -o port.o
	# shellcheck disable=SC2086
	$CXX $CXXFLAGS $FENVFLAGS -fmodules-ts -c ../harness.cpp -o harness.o
fi

# shellcheck disable=SC2086
$CXX $CXXFLAGS oracle.o port.o harness.o -lm -o harness

exec ./harness
