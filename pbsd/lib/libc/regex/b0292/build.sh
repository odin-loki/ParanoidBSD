#!/bin/sh
#
# Build and run the b0292 differential test.  Run as `sh build.sh' from
# pbsd/lib/libc/regex/b0292/.  The harness's exit status is this script's exit
# status.

set -e

cd "$(dirname "$0")"

CC=${CC:-cc}
CXX=${CXX:-c++}

MODNAME=pbsd.lib.libc.regex.b0292
ROOT=$(cd ../../../../.. && pwd)
HBSD=$ROOT/hbsd
REGEXDIR=$HBSD/src/lib/libc/regex

TMPDIR=$(mktemp -d)
PREREQ=$(mktemp)
trap 'rm -rf "$TMPDIR"; rm -f "$PREREQ"' EXIT

cat > "$PREREQ" <<'EOF'
#ifndef B0292_PREREQ_H
#define B0292_PREREQ_H

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

#endif /* B0292_PREREQ_H */
EOF

mkdir -p "$TMPDIR/locale"
cat > "$TMPDIR/locale/collate.h" <<'EOF'
#ifndef _COLLATE_H_
#define _COLLATE_H_
#include <wchar.h>
#include <limits.h>
#ifndef COLL_WEIGHTS_MAX
#define COLL_WEIGHTS_MAX 10
#endif
struct xlocale_collate { int __collate_load_error; };
struct xlocale { void *components[8]; };
struct xlocale * __get_locale(void);
int __wcollate_range_cmp(wint_t, wint_t);
#endif
EOF

cat > "$TMPDIR/locale/xlocale_private.h" <<'EOF'
#ifndef _XLOCALE_PRIVATE__H_
#define _XLOCALE_PRIVATE__H_
#include "collate.h"
enum { XLC_COLLATE = 0, XLC_CTYPE, XLC_MONETARY, XLC_NUMERIC, XLC_TIME,
    XLC_MESSAGES, XLC_LAST };
#endif
EOF

cat > "$TMPDIR/glue.c" <<'EOF'
#include <wchar.h>
#include "collate.h"
static struct xlocale the_locale;
struct xlocale * __get_locale(void) { return &the_locale; }
int __wcollate_range_cmp(wint_t a, wint_t b)
{
	return (a > b) - (a < b);
}
EOF

CFLAGS="-std=c11 -O2 -include $PREREQ -I$TMPDIR/locale -I$REGEXDIR"
CXXFLAGS="-std=c++23 -O2 -include $PREREQ -I$REGEXDIR"

rm -rf gcm.cache
rm -f oracle.o port.o harness.o regcomp.o regfree.o glue.o port.pcm harness

$CC $CFLAGS -c oracle.c -o oracle.o
$CC $CFLAGS -c "$TMPDIR/glue.c" -o glue.o
$CC $CFLAGS -c $REGEXDIR/regcomp.c -o regcomp.o
$CC $CFLAGS -c $REGEXDIR/regfree.c -o regfree.o

if $CXX --version 2>&1 | grep -qi clang; then
	$CXX $CXXFLAGS -x c++-module port.cppm --precompile -o port.pcm
	$CXX $CXXFLAGS -c port.pcm -o port.o
	$CXX $CXXFLAGS -fmodule-file="$MODNAME"=port.pcm -c harness.cpp -o harness.o
else
	$CXX $CXXFLAGS -fmodules-ts -c -x c++ port.cppm -o port.o
	$CXX $CXXFLAGS -fmodules-ts -c harness.cpp -o harness.o
fi

$CXX $CXXFLAGS -o harness harness.o port.o oracle.o regcomp.o regfree.o glue.o

exec ./harness
