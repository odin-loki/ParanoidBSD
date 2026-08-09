// PBSD b0066 -- C++23 port of
//	lib/libc/gdtoa/_ldtoa.c
//	lib/libc/gdtoa/_hldtoa.c
//	lib/libc/gdtoa/_hdtoa.c
//
// The bodies below are the HardenedBSD originals, transliterated to C++ with
// no changes to behaviour: same signedness, same evaluation order, same
// pointer arithmetic, same truncation in the digit loops.  rv_alloc(),
// nrv_alloc() and gdtoa() live in contrib/gdtoa, outside this batch, and are
// resolved at link time exactly as they are for the C implementation.

module;

#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __i386__
#include <ieeefp.h>
#endif

// HardenedBSD's <float.h> defines FLT_ROUNDS as __flt_rounds(), which tracks
// fesetround(3).  GCC's <float.h> hardcodes it to 1.  Restore the target
// definition so that __ldtoa() sees the rounding mode the program is running
// under, as it does on HardenedBSD.
extern "C" int __flt_rounds(void);
#undef FLT_ROUNDS
#define	FLT_ROUNDS	__flt_rounds()

// Declarations that the batch sources obtain from contrib/gdtoa's "gdtoa.h"
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

char *rv_alloc(int);
char *nrv_alloc(const char *, char **, int);
char *gdtoa(FPI *fpi, int be, ULong *bits, int *kindp, int mode, int ndigits,
    int *decpt, char **rve);

}

export module pbsd.lib.libc.gdtoa.b0066;

namespace pbsd::lib_libc_gdtoa::b0066 {

// ------------------------------------------------------------------
// From lib/libc/include/fpmath.h (_BYTE_ORDER == _LITTLE_ENDIAN and
// _IEEE_WORD_ORDER == _LITTLE_ENDIAN, i.e. the amd64 branch).
// ------------------------------------------------------------------

#define	DBL_MANH_SIZE	20
#define	DBL_MANL_SIZE	32

union IEEEd2bits {
	double	d;
	struct {
		unsigned int	manl	:32;
		unsigned int	manh	:20;
		unsigned int	exp	:11;
		unsigned int	sign	:1;
	} bits;
};

// ------------------------------------------------------------------
// From lib/libc/amd64/_fpmath.h.
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

#define	LDBL_NBIT	0x80000000
#define	mask_nbit_l(u)	((u).bits.manh &= ~LDBL_NBIT)

#define	LDBL_MANH_SIZE	32
#define	LDBL_MANL_SIZE	32

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

/* ================================================================== */
/* lib/libc/gdtoa/_hldtoa.c						*/
/* ================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2004-2008 David Schultz <das@FreeBSD.ORG>
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

// The file-scope `one' of each source file is given a distinct spelling so
// that the three files can share a translation unit.  The bodies below are
// unchanged; the macro does the renaming.
#define	one	one_hldtoa

#if (LDBL_MANT_DIG > DBL_MANT_DIG)

/* Strings values used by dtoa() */
#define	INFSTR	"Infinity"
#define	NANSTR	"NaN"

#ifdef LDBL_IMPLICIT_NBIT
#define	MANH_SIZE	LDBL_MANH_SIZE
#else
#define	MANH_SIZE	(LDBL_MANH_SIZE - 1)
#endif

#if MANH_SIZE > 32
typedef uint64_t manh_t;
#else
typedef uint32_t manh_t;
#endif

#if LDBL_MANL_SIZE > 32
typedef uint64_t manl_t;
#else
typedef uint32_t manl_t;
#endif

#define	LDBL_ADJ	(LDBL_MAX_EXP - 2)
#define	SIGFIGS		((LDBL_MANT_DIG + 3) / 4 + 1)

static const float one[] = { 1.0f, -1.0f };

/*
 * This is the long double version of __hdtoa().
 */
export char *
__hldtoa(long double e, const char *xdigs, int ndigits, int *decpt, int *sign,
    char **rve)
{
	union IEEEl2bits u;
	char *s, *s0;
	manh_t manh;
	manl_t manl;
	int bufsize;
#ifdef __i386__
	fp_prec_t oldprec;
#endif

	u.e = e;
	*sign = u.bits.sign;

	switch (fpclassify(e)) {
	case FP_NORMAL:
		*decpt = u.bits.exp - LDBL_ADJ;
		break;
	case FP_ZERO:
		*decpt = 1;
		return (nrv_alloc("0", rve, 1));
	case FP_SUBNORMAL:
#ifdef __i386__
		oldprec = fpsetprec(FP_PE);
#endif
		u.e *= 0x1p514L;
		*decpt = u.bits.exp - (514 + LDBL_ADJ);
#ifdef __i386__
		fpsetprec(oldprec);
#endif
		break;
	case FP_INFINITE:
		*decpt = INT_MAX;
		return (nrv_alloc(INFSTR, rve, sizeof(INFSTR) - 1));
	default:	/* FP_NAN or unrecognized */
		*decpt = INT_MAX;
		return (nrv_alloc(NANSTR, rve, sizeof(NANSTR) - 1));
	}

	/* FP_NORMAL or FP_SUBNORMAL */

	if (ndigits == 0)		/* dtoa() compatibility */
		ndigits = 1;

	/*
	 * If ndigits < 0, we are expected to auto-size, so we allocate
	 * enough space for all the digits.
	 */
	bufsize = (ndigits > 0) ? ndigits : SIGFIGS;
	s0 = rv_alloc(bufsize);

	/* Round to the desired number of digits. */
	if (SIGFIGS > ndigits && ndigits > 0) {
		float redux = one[u.bits.sign];
		int offset = 4 * ndigits + LDBL_MAX_EXP - 4 - LDBL_MANT_DIG;
#ifdef __i386__
		oldprec = fpsetprec(FP_PE);
#endif
		u.bits.exp = offset;
		u.e += redux;
		u.e -= redux;
		*decpt += u.bits.exp - offset;
#ifdef __i386__
		fpsetprec(oldprec);
#endif
	}

	mask_nbit_l(u);
	manh = u.bits.manh;
	manl = u.bits.manl;	
	*s0 = '1';
	for (s = s0 + 1; s < s0 + bufsize; s++) {
		*s = xdigs[(manh >> (MANH_SIZE - 4)) & 0xf];
		manh = (manh << 4) | (manl >> (LDBL_MANL_SIZE - 4));
		manl <<= 4;
	}

	/* If ndigits < 0, we are expected to auto-size the precision. */
	if (ndigits < 0) {
		for (ndigits = SIGFIGS; s0[ndigits - 1] == '0'; ndigits--)
			;
	}

	s = s0 + ndigits;
	*s = '\0';
	if (rve != NULL)
		*rve = s;
	return (s0);
}

#else	/* (LDBL_MANT_DIG == DBL_MANT_DIG) */

export char *__hdtoa(double, const char *, int, int *, int *, char **);

export char *
__hldtoa(long double e, const char *xdigs, int ndigits, int *decpt, int *sign,
    char **rve)
{

	return (__hdtoa((double)e, xdigs, ndigits, decpt, sign, rve));
}

#endif	/* (LDBL_MANT_DIG == DBL_MANT_DIG) */

#undef one
#undef INFSTR
#undef NANSTR
#undef SIGFIGS

/* ================================================================== */
/* lib/libc/gdtoa/_hdtoa.c						*/
/* ================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2004-2008 David Schultz <das@FreeBSD.ORG>
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

#define	one	one_hdtoa

/* Strings values used by dtoa() */
#define	INFSTR	"Infinity"
#define	NANSTR	"NaN"

#define	DBL_ADJ	(DBL_MAX_EXP - 2)
#define	SIGFIGS	((DBL_MANT_DIG + 3) / 4 + 1)

static const float one[] = { 1.0f, -1.0f };

/*
 * This procedure converts a double-precision number in IEEE format
 * into a string of hexadecimal digits and an exponent of 2.  Its
 * behavior is bug-for-bug compatible with dtoa() in mode 2, with the
 * following exceptions:
 *
 * - An ndigits < 0 causes it to use as many digits as necessary to
 *   represent the number exactly.
 * - The additional xdigs argument should point to either the string
 *   "0123456789ABCDEF" or the string "0123456789abcdef", depending on
 *   which case is desired.
 * - This routine does not repeat dtoa's mistake of setting decpt
 *   to 9999 in the case of an infinity or NaN.  INT_MAX is used
 *   for this purpose instead.
 *
 * Note that the C99 standard does not specify what the leading digit
 * should be for non-zero numbers.  For instance, 0x1.3p3 is the same
 * as 0x2.6p2 is the same as 0x4.cp3.  This implementation always makes
 * the leading digit a 1. This ensures that the exponent printed is the
 * actual base-2 exponent, i.e., ilogb(d).
 *
 * Inputs:	d, xdigs, ndigits
 * Outputs:	decpt, sign, rve
 */
export char *
__hdtoa(double d, const char *xdigs, int ndigits, int *decpt, int *sign,
    char **rve)
{
	union IEEEd2bits u;
	char *s, *s0;
	int bufsize;
	uint32_t manh, manl;

	u.d = d;
	*sign = u.bits.sign;

	switch (fpclassify(d)) {
	case FP_NORMAL:
		*decpt = u.bits.exp - DBL_ADJ;
		break;
	case FP_ZERO:
		*decpt = 1;
		return (nrv_alloc("0", rve, 1));
	case FP_SUBNORMAL:
		u.d *= 0x1p514;
		*decpt = u.bits.exp - (514 + DBL_ADJ);
		break;
	case FP_INFINITE:
		*decpt = INT_MAX;
		return (nrv_alloc(INFSTR, rve, sizeof(INFSTR) - 1));
	default:	/* FP_NAN or unrecognized */
		*decpt = INT_MAX;
		return (nrv_alloc(NANSTR, rve, sizeof(NANSTR) - 1));
	}

	/* FP_NORMAL or FP_SUBNORMAL */

	if (ndigits == 0)		/* dtoa() compatibility */
		ndigits = 1;

	/*
	 * If ndigits < 0, we are expected to auto-size, so we allocate
	 * enough space for all the digits.
	 */
	bufsize = (ndigits > 0) ? ndigits : SIGFIGS;
	s0 = rv_alloc(bufsize);

	/* Round to the desired number of digits. */
	if (SIGFIGS > ndigits && ndigits > 0) {
		float redux = one[u.bits.sign];
		int offset = 4 * ndigits + DBL_MAX_EXP - 4 - DBL_MANT_DIG;
		u.bits.exp = offset;
		u.d += redux;
		u.d -= redux;
		*decpt += u.bits.exp - offset;
	}

	manh = u.bits.manh;
	manl = u.bits.manl;
	*s0 = '1';
	for (s = s0 + 1; s < s0 + bufsize; s++) {
		*s = xdigs[(manh >> (DBL_MANH_SIZE - 4)) & 0xf];
		manh = (manh << 4) | (manl >> (DBL_MANL_SIZE - 4));
		manl <<= 4;
	}

	/* If ndigits < 0, we are expected to auto-size the precision. */
	if (ndigits < 0) {
		for (ndigits = SIGFIGS; s0[ndigits - 1] == '0'; ndigits--)
			;
	}

	s = s0 + ndigits;
	*s = '\0';
	if (rve != NULL)
		*rve = s;
	return (s0);
}

} // namespace pbsd::lib_libc_gdtoa::b0066
