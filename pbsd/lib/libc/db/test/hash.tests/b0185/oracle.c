/*
 * oracle.c -- reference implementation for PBSD batch b0185.
 *
 * hbsd/src/lib/libc/db/test/hash.tests/tseq.c, tcreat3.c, tread2.c, and
 * tverify.c concatenated.  Functions would be renamed with a ref_ prefix;
 * bodies unmodified.
 *
 * Every source file in this batch contains only main() and depends on <db.h>,
 * dbopen(), and live hash-table state that cannot be satisfied here without
 * inventing behaviour.  See skipped.txt.
 */

#include <limits.h>
#include <stddef.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif
