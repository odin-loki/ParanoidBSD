#!/bin/sh
#
# Build and run the b0061 differential test.  Run as `sh build.sh' from
# pbsd/bin/csh/b0061/.  The exit status is the harness's exit status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}
CFLAGS=${CFLAGS:--O2}
CXXFLAGS=${CXXFLAGS:--O2}

MODNAME=pbsd.bin.csh.b0061

rm -rf gcm.cache mocks
rm -f oracle.o port.o harness.o port.pcm harness

mkdir -p mocks/good mocks/no_open mocks/no_engine mocks/no_close

# Deterministic mock libiconv.so variants for branch coverage.
cat > mocks/good.c <<'EOF'
#include <stddef.h>
#include <stdint.h>
void *libiconv_open(const char *t, const char *f)
{
	uintptr_t h = 5381;
	const unsigned char *p;
	if (t != NULL)
		for (p = (const unsigned char *)t; *p; ++p)
			h = ((h << 5) + h) + *p;
	if (f != NULL)
		for (p = (const unsigned char *)f; *p; ++p)
			h = ((h << 5) + h) + *p;
	return ((void *)(h | 0x100));
}
size_t libiconv(void *cd, char **inbuf, size_t *inleft, char **outbuf,
    size_t *outleft)
{
	(void)cd;
	(void)inbuf;
	(void)inleft;
	(void)outbuf;
	(void)outleft;
	return (0);
}
int libiconv_close(void *cd)
{
	(void)cd;
	return (0);
}
EOF

cat > mocks/no_open.c <<'EOF'
#include <stddef.h>
size_t libiconv(void *cd, char **inbuf, size_t *inleft, char **outbuf,
    size_t *outleft)
{
	(void)cd;
	(void)inbuf;
	(void)inleft;
	(void)outbuf;
	(void)outleft;
	return (0);
}
int libiconv_close(void *cd)
{
	(void)cd;
	return (0);
}
EOF

cat > mocks/no_engine.c <<'EOF'
#include <stddef.h>
#include <stdint.h>
void *libiconv_open(const char *t, const char *f)
{
	(void)t;
	(void)f;
	return ((void *)0x200);
}
int libiconv_close(void *cd)
{
	(void)cd;
	return (0);
}
EOF

cat > mocks/no_close.c <<'EOF'
#include <stddef.h>
#include <stdint.h>
void *libiconv_open(const char *t, const char *f)
{
	(void)t;
	(void)f;
	return ((void *)0x300);
}
size_t libiconv(void *cd, char **inbuf, size_t *inleft, char **outbuf,
    size_t *outleft)
{
	(void)cd;
	(void)inbuf;
	(void)inleft;
	(void)outbuf;
	(void)outleft;
	return (0);
}
EOF

$CC -std=c11 $CFLAGS -fPIC -shared -o mocks/good/libiconv.so mocks/good.c
$CC -std=c11 $CFLAGS -fPIC -shared -o mocks/no_open/libiconv.so mocks/no_open.c
$CC -std=c11 $CFLAGS -fPIC -shared -o mocks/no_engine/libiconv.so mocks/no_engine.c
$CC -std=c11 $CFLAGS -fPIC -shared -o mocks/no_close/libiconv.so mocks/no_close.c

$CC -std=c11 $CFLAGS -c oracle.c -o oracle.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX -std=c++23 $CXXFLAGS -x c++-module port.cppm --precompile -o port.pcm
	$CXX -std=c++23 $CXXFLAGS -c port.pcm -o port.o
	$CXX -std=c++23 $CXXFLAGS -fmodule-file=$MODNAME=port.pcm \
	    -c harness.cpp -o harness.o
else
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS -c -x c++ port.cppm -o port.o
	$CXX -std=c++23 -fmodules-ts $CXXFLAGS -c harness.cpp -o harness.o
fi

$CXX -std=c++23 $CXXFLAGS -o harness harness.o port.o oracle.o -ldl

exec ./harness
