#!/bin/sh
# b0154 — build the C oracle, the C++23 port module and the differential
# harness, link them together and exec the test.  POSIX sh.
#
#	sh build.sh
#
# The script's exit status is the harness' exit status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
BUILD=${BUILD:-build}
MODNAME=pbsd.lib.libc.nameser.b0154

# C++23 named modules need clang >= 16 or gcc >= 14.  Pick a usable compiler
# and work out the right flags for it.
detect_cxx() {
	for cand in "$CXX" clang++ clang++-18 clang++-17 clang++-16 g++-14 g++; do
		[ -n "$cand" ] || continue
		command -v "$cand" >/dev/null 2>&1 || continue
		v=$("$cand" --version 2>/dev/null | head -n 1)
		case "$v" in
		*clang*)
			maj=$("$cand" -dumpversion 2>/dev/null | cut -d. -f1)
			[ -n "$maj" ] || continue
			[ "$maj" -ge 16 ] 2>/dev/null || continue
			echo "$cand clang"
			return 0
			;;
		*)
			maj=$("$cand" -dumpversion 2>/dev/null | cut -d. -f1)
			[ -n "$maj" ] || continue
			[ "$maj" -ge 14 ] 2>/dev/null || continue
			echo "$cand gcc"
			return 0
			;;
		esac
	done
	return 1
}

set -- $(detect_cxx) || true
if [ $# -lt 2 ]; then
	echo "build.sh: no C++ compiler with named-module support found" >&2
	exit 1
fi
CXX=$1
KIND=$2

CFLAGS=${CFLAGS:--std=c11 -O2}
CXXFLAGS=${CXXFLAGS:--std=c++23 -O2}

# dn_expand()/dn_skipname() live in libc on glibc >= 2.34 and in libresolv
# before that.
LIBS=
if ! echo 'int dn_skipname(const unsigned char *, const unsigned char *);
int main(void){return dn_skipname(0,0);}' > /tmp/b0154_probe$$.c 2>/dev/null; then
	:
fi
if $CC -o /tmp/b0154_probe$$ /tmp/b0154_probe$$.c >/dev/null 2>&1; then
	LIBS=
else
	LIBS=-lresolv
fi
rm -f /tmp/b0154_probe$$ /tmp/b0154_probe$$.c

rm -rf "$BUILD"
mkdir -p "$BUILD"

echo "== oracle.c   ($CC $CFLAGS)"
$CC $CFLAGS -c oracle.c -o "$BUILD/oracle.o"

if [ "$KIND" = clang ]; then
	echo "== port.cppm  ($CXX $CXXFLAGS, clang named modules)"
	$CXX $CXXFLAGS --precompile -x c++-module port.cppm \
	    -o "$BUILD/$MODNAME.pcm"
	$CXX $CXXFLAGS -c "$BUILD/$MODNAME.pcm" -o "$BUILD/port.o"
	echo "== harness.cpp"
	$CXX $CXXFLAGS -fmodule-file=$MODNAME="$BUILD/$MODNAME.pcm" \
	    -c harness.cpp -o "$BUILD/harness.o"
else
	echo "== port.cppm  ($CXX $CXXFLAGS, gcc named modules)"
	$CXX $CXXFLAGS -fmodules-ts -fmodule-mapper="|@g++-mapper-server" \
	    -x c++ -c port.cppm -o "$BUILD/port.o"
	echo "== harness.cpp"
	$CXX $CXXFLAGS -fmodules-ts -fmodule-mapper="|@g++-mapper-server" \
	    -c harness.cpp -o "$BUILD/harness.o"
fi

echo "== link"
$CXX $CXXFLAGS -o "$BUILD/b0154_test" "$BUILD/port.o" "$BUILD/harness.o" \
    "$BUILD/oracle.o" $LIBS

echo "== run"
exec "$BUILD/b0154_test"
