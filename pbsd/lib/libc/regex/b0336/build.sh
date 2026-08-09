#!/bin/sh
#
# Build and run the b0336 differential test.  Run as `sh build.sh' from
# pbsd/lib/libc/regex/b0336/.  The harness's exit status is this script's exit
# status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

MODNAME=pbsd.lib.libc.regex.b0336
ROOT=$(cd ../../../../.. && pwd)
HBSD=$ROOT/hbsd
REGEXDIR=$HBSD/src/lib/libc/regex

TMPDIR=$(mktemp -d)
PREREQ=$(mktemp)
trap 'rm -rf "$TMPDIR"; rm -f "$PREREQ"' EXIT

cat > "$PREREQ" <<'EOF'
#ifndef B0336_PREREQ_H
#define B0336_PREREQ_H

#define LIBREGEX 1

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <limits.h>

#ifndef __unused
#define __unused
#endif

#ifndef LONG_BIT
#ifdef __LP64__
#define LONG_BIT 64
#else
#define LONG_BIT 32
#endif
#endif

#ifndef _POSIX2_RE_DUP_MAX
#define _POSIX2_RE_DUP_MAX 255
#endif

typedef off_t regoff_t;

struct re_guts;

typedef struct {
	int re_magic;
	size_t re_nsub;
	const char *re_endp;
	struct re_guts *re_g;
} regex_t;

typedef struct {
	regoff_t rm_so;
	regoff_t rm_eo;
} regmatch_t;

#define REG_BASIC     0000
#define REG_EXTENDED  0001
#define REG_ICASE     0002
#define REG_NOSUB     0004
#define REG_NEWLINE   0010
#define REG_NOSPEC    0020
#define REG_PEND      0040
#define REG_DUMP      0200
#define REG_POSIX     0400

#define REG_ENOSYS    (-1)
#define REG_NOMATCH   1
#define REG_BADPAT    2
#define REG_ECOLLATE  3
#define REG_ECTYPE    4
#define REG_EESCAPE   5
#define REG_ESUBREG   6
#define REG_EBRACK    7
#define REG_EPAREN    8
#define REG_EBRACE    9
#define REG_BADBR     10
#define REG_ERANGE    11
#define REG_ESPACE    12
#define REG_BADRPT    13
#define REG_EMPTY     14
#define REG_ASSERT    15
#define REG_INVARG    16
#define REG_ILLSEQ    17
#define REG_ATOI      255
#define REG_ITOA      0400

#define REG_NOTBOL    00001
#define REG_NOTEOL    00002
#define REG_STARTEND  00004
#define REG_TRACE     00400
#define REG_LARGE     01000
#define REG_BACKR     02000

#define _REGEX_H_
#define _REGEX_H

struct _RuneLocale {
	const char *__encoding;
};
extern struct _RuneLocale *_CurrentRuneLocale;

void *reallocarray(void *optr, size_t nmemb, size_t size);
void regfree(regex_t *preg);

#endif /* B0336_PREREQ_H */
EOF

cat > "$TMPDIR/glue.c" <<'EOF'
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

void *
reallocarray(void *optr, size_t nmemb, size_t size)
{
	if (nmemb != 0 && size > SIZE_MAX / nmemb) {
		errno = ENOMEM;
		return (NULL);
	}
	return (realloc(optr, nmemb * size));
}

struct _RuneLocale *_CurrentRuneLocale;
static struct _RuneLocale utf8_rune_locale = { "UTF-8" };

__attribute__((constructor))
static void
init_rune_locale(void)
{
	_CurrentRuneLocale = &utf8_rune_locale;
}
EOF

CFLAGS="-std=c11 -O2 -DLIBREGEX -include $PREREQ -I$REGEXDIR"
CXXFLAGS="-std=c++23 -O2 -I$REGEXDIR"

rm -rf gcm.cache
rm -f oracle.o port.o harness.o regfree.o glue.o port.pcm harness

$CC $CFLAGS -c oracle.c -o oracle.o
$CC $CFLAGS -c "$TMPDIR/glue.c" -o glue.o
$CC $CFLAGS -c "$REGEXDIR/regfree.c" -o regfree.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX $CXXFLAGS -x c++-module port.cppm --precompile -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODNAME"=port.pcm -c harness.cpp -o harness.o
else
	$CXX $CXXFLAGS -fmodules-ts -c -x c++ port.cppm -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS -o harness harness.o port.o oracle.o regfree.o glue.o

exec ./harness
