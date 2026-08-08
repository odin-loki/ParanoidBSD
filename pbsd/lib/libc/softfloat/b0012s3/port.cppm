// PBSD batch b0012s3 -- port of hbsd/src/lib/libc/softfloat/negxf2.c
//
// C++23 module interface.  The port is a literal transliteration of the
// original C: same expression, same operand order, same constants.

module;

#include <stdint.h>

export module pbsd.lib.libc.softfloat.b0012s3;

export namespace pbsd::lib_libc_softfloat::b0012s3 {

// Declarations that negxf2.c picks up from "softfloat-for-gcc.h", "milieu.h"
// and "softfloat.h".  floatx80 has the layout of the softfloat extended
// double; __mulxf3() and __floatsixf() keep C language linkage and are
// supplied by other batches -- negxf2.c only calls them.

struct floatx80 {
	uint16_t high;
	uint64_t low;
};

extern "C" floatx80 __mulxf3(floatx80, floatx80);
extern "C" floatx80 __floatsixf(int32_t);

/* $NetBSD: negxf2.c,v 1.2 2004/09/27 10:16:24 he Exp $ */

/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

// The whole of negxf2.c sits inside #ifdef FLOATX80, which this batch builds.

floatx80 __negxf2(floatx80);

floatx80
__negxf2(floatx80 a)
{

	/* libgcc1.c says -a */
	return __mulxf3(a,__floatsixf(-1));
}

} // namespace pbsd::lib_libc_softfloat::b0012s3
