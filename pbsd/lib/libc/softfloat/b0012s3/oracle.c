/*
 * PBSD batch b0012s3 oracle -- the reference specification.
 *
 * Original C source concatenated verbatim; the only edit is the ref_ prefix
 * on the function name.  Function bodies are untouched.
 *
 * negxf2.c includes "softfloat-for-gcc.h", "milieu.h" and "softfloat.h".
 * Those headers are not part of this batch, so the handful of declarations
 * the source actually needs is reproduced below.  __mulxf3() and
 * __floatsixf() are softfloat routines from other batches; the differential
 * harness supplies one shared definition of each, used by both this oracle
 * and the C++ port.
 */

#include <stdint.h>

#define FLOATX80

typedef struct floatx80 {
	uint16_t high;
	uint64_t low;
} floatx80;

extern floatx80 __mulxf3(floatx80, floatx80);
extern floatx80 __floatsixf(int32_t);

/* ------------------------------------------------------------------ */
/* hbsd/src/lib/libc/softfloat/negxf2.c                                 */
/* ------------------------------------------------------------------ */

/* $NetBSD: negxf2.c,v 1.2 2004/09/27 10:16:24 he Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

#ifdef FLOATX80

floatx80 ref___negxf2(floatx80);

floatx80
ref___negxf2(floatx80 a)
{

	/* libgcc1.c says -a */
	return __mulxf3(a,__floatsixf(-1));
}
#endif /* FLOATX80 */
