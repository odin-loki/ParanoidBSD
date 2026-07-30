#!/bin/sh
# Build and run the PBSD b0065 differential test.
#
# Usage: sh build.sh   (from pbsd/lib/libc/stdlib/b0065/)

set -e

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS="-std=c11 -O2"
CXXFLAGS="-std=c++23 -O2"

dir=$(dirname "$0")
cd "$dir"

OUT=./b0065_test
BUILD=./.build
rm -rf "$BUILD"
mkdir -p "$BUILD"

# Work out which named-module dialect this toolchain speaks.
probe_dir=$BUILD/probe
mkdir -p "$probe_dir"
cat > "$probe_dir/probe.cppm" <<'EOF'
export module pbsd_probe_module;
export int pbsd_probe(void) { return 0; }
EOF

MODFLAGS=""
if (cd "$probe_dir" && "$CXX" -std=c++23 -fmodules-ts -x c++ -c probe.cppm \
    -o probe.o) >/dev/null 2>&1; then
	MODFLAGS="-fmodules-ts"
elif (cd "$probe_dir" && "$CXX" -std=c++23 -fmodules -x c++-module -c \
    probe.cppm -o probe.o) >/dev/null 2>&1; then
	MODFLAGS="-fmodules"
else
	echo "build.sh: $CXX does not support C++23 named modules" >&2
	exit 1
fi
rm -rf "$probe_dir"

echo "build.sh: CC=$CC CXX=$CXX module flags: $MODFLAGS"

# 1. The C oracle.
$CC $CFLAGS -c oracle.c -o "$BUILD/oracle.o"

# 2. The C++23 module interface unit.
case "$MODFLAGS" in
-fmodules-ts)
	# GCC: the BMI lands in ./gcm.cache and is found automatically.
	$CXX $CXXFLAGS $MODFLAGS -x c++ -c port.cppm -o "$BUILD/port.o"
	$CXX $CXXFLAGS $MODFLAGS -c harness.cpp -o "$BUILD/harness.o"
	;;
*)
	# Clang: precompile the interface, then feed the BMI to the consumer.
	$CXX $CXXFLAGS $MODFLAGS -x c++-module --precompile port.cppm \
	    -o "$BUILD/port.pcm"
	$CXX $CXXFLAGS $MODFLAGS -c "$BUILD/port.pcm" -o "$BUILD/port.o"
	$CXX $CXXFLAGS $MODFLAGS \
	    -fmodule-file=pbsd.lib.libc.stdlib.b0065="$BUILD/port.pcm" \
	    -c harness.cpp -o "$BUILD/harness.o"
	;;
esac

# 3. Link and run.
$CXX $CXXFLAGS $MODFLAGS -o "$OUT" "$BUILD/harness.o" "$BUILD/port.o" \
    "$BUILD/oracle.o"

exec "$OUT"
