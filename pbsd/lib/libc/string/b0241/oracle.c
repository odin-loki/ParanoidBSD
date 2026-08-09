/*
 * Reference oracle for PBSD batch b0241.
 *
 * The original HardenedBSD C sources concatenated verbatim; every function has
 * been renamed with a `ref_` prefix and nothing else has been touched.
 *
 * Sources:
 *   hbsd/src/lib/libc/string/strchr.c  (skipped — see skipped.txt)
 */

#include <limits.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif
