// PBSD b0066s1 -- C++23 port of lib/libc/gdtoa/_ldtoa.c
//
// The body below is the HardenedBSD original, transliterated to C++ with no
// changes to behaviour.  gdtoa() lives in contrib/gdtoa, outside this batch,
// and is resolved at link time exactly as it is for the C implementation.

module;

#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

export module pbsd.lib.libc.gdtoa.b0066s1;

// HardenedBSD's <float.h> defines FLT_ROUNDS as __flt_rounds(), which tracks
// fesetround(3).  GCC's <float.h> hardcodes it to 1.  Restore the target
// definition so that __ldtoa() sees the rounding mode the program is running
// under, as it does on HardenedBSD.
extern "C" int __flt_rounds(void);
#undef FLT_ROUNDS
#define	FLT_ROUNDS	__flt_rounds()

// Declarations that the batch source obtains from contrib/gdtoa's "gdtoa.h"
// and "gdtoaimp.h".
extern "C" {

typedef unsigned int ULong;

typedef struct
FPI {
	int nbits;
	int emin;
	int emax;
	int rounding;
	int sudden_underflow;
	} FPI;

char *gdtoa(FPI *fpi, int be, ULong *bits, int *kindp, int mode, int ndigits,
    int *decpt, char **rve);

}

namespace pbsd::lib_libc_gdtoa::b0066s1 {

// ------------------------------------------------------------------
// From lib/libc/include/fpmath.h (_BYTE_ORDER == _LITTLE_ENDIAN and
// _IEEE_WORD_ORDER == _LITTLE_ENDIAN, i.e. the amd64 branch).
// ------------------------------------------------------------------

union IEEEl2bits {
	long double	e;
	struct {
		unsigned int	manl	:32;
		unsigned int	manh	:32;
		unsigned int	exp	:15;
		unsigned int	sign	:1;
		unsigned int	junkl	:16;
		unsigned int	junkh	:32;
	} bits;
	struct {
		unsigned long	man	:64;
		unsigned int	expsign	:16;
		unsigned long	junk	:48;
	} xbits;
};

#define	LDBL_TO_ARRAY32(u, a) do {			\
	(a)[0] = (uint32_t)(u).bits.manl;		\
	(a)[1] = (uint32_t)(u).bits.manh;		\
} while (0)

// ------------------------------------------------------------------
// From contrib/gdtoa/gdtoa.h.
// ------------------------------------------------------------------

enum {	/* return values from strtodg */
	STRTOG_Zero	= 0,
	STRTOG_Normal	= 1,
	STRTOG_Denormal	= 2,
	STRTOG_Infinite	= 3,
	STRTOG_NaN	= 4,
	STRTOG_NaNbits	= 5,
	STRTOG_NoNumber	= 6,
	STRTOG_Retmask	= 7,

	STRTOG_Neg	= 0x08,
	STRTOG_Inexlo	= 0x10,
	STRTOG_Inexhi	= 0x20,
	STRTOG_Inexact	= 0x30,
	STRTOG_Underflow= 0x40,
	STRTOG_Overflow	= 0x80
};

/* ================================================================== */
/* lib/libc/gdtoa/_ldtoa.c						*/
/* ================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2003 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * ldtoa() is a wrapper for gdtoa() that makes it smell like dtoa(),
 * except that the floating point argument is passed by reference.
 * When dtoa() is passed a NaN or infinity, it sets expt to 9999.
 * However, a long double could have a valid exponent of 9999, so we
 * use INT_MAX in ldtoa() instead.
 */
export char *
__ldtoa(long double *ld, int mode, int ndigits, int *decpt, int *sign,
    char **rve)
{
	FPI fpi = {
		LDBL_MANT_DIG,			/* nbits */
		LDBL_MIN_EXP - LDBL_MANT_DIG,	/* emin */
		LDBL_MAX_EXP - LDBL_MANT_DIG,	/* emax */
		FLT_ROUNDS,	       		/* rounding */
#ifdef Sudden_Underflow	/* unused, but correct anyway */
		1
#else
		0
#endif
	};
	int be, kind;
	char *ret;
	union IEEEl2bits u;
	uint32_t bits[(LDBL_MANT_DIG + 31) / 32];
	void *vbits = bits;

	u.e = *ld;

	/*
	 * gdtoa doesn't know anything about the sign of the number, so
	 * if the number is negative, we need to swap rounding modes of
	 * 2 (upwards) and 3 (downwards).
	 */
	*sign = u.bits.sign;
	fpi.rounding ^= (fpi.rounding >> 1) & u.bits.sign;

	be = u.bits.exp - (LDBL_MAX_EXP - 1) - (LDBL_MANT_DIG - 1);
	LDBL_TO_ARRAY32(u, bits);

	switch (fpclassify(u.e)) {
	case FP_NORMAL:
		kind = STRTOG_Normal;
#ifdef	LDBL_IMPLICIT_NBIT
		bits[LDBL_MANT_DIG / 32] |= 1 << ((LDBL_MANT_DIG - 1) % 32);
#endif /* LDBL_IMPLICIT_NBIT */
		break;
	case FP_ZERO:
		kind = STRTOG_Zero;
		break;
	case FP_SUBNORMAL:
		kind = STRTOG_Denormal;
		be++;
		break;
	case FP_INFINITE:
		kind = STRTOG_Infinite;
		break;
	case FP_NAN:
		kind = STRTOG_NaN;
		break;
	default:
		abort();
	}

	ret = gdtoa(&fpi, be, (ULong *)vbits, &kind, mode, ndigits, decpt, rve);
	if (*decpt == -32768)
		*decpt = INT_MAX;
	return ret;
}

} // namespace pbsd::lib_libc_gdtoa::b0066s1
