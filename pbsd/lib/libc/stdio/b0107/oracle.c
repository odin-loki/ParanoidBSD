/*
 * oracle.c -- reference implementation for PBSD batch b0107.
 *
 * hbsd/src/lib/libc/stdio/fgetwc.c, snprintf.c, and gets_s.c concatenated.
 * Functions would be renamed with a ref_ prefix; bodies unmodified.
 *
 * Every source file in this batch depends on libc-private headers and
 * symbols (local.h, __vfprintf, __sgetc, FLOCKFILE, etc.) that cannot be
 * satisfied here without inventing behaviour.  See skipped.txt.
 */

#include <limits.h>
#include <stddef.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif
