/*
 * oracle.c -- reference implementation for PBSD batch b0251.
 *
 * The original HardenedBSD source from lib/msun/src/s_ilogbl.c is reproduced
 * below with the function renamed ref_ilogbl.  The function body is
 * otherwise byte-for-byte unmodified.  Supporting definitions from
 * lib/libc/amd64/_fpmath.h and <float.h> that the original obtained through
 * <fpmath.h> are reproduced verbatim.
 *
 * This file is the specification.  Do not modify any function body.
 */

#include <float.h>
#include <limits.h>
#include <math.h>

#ifndef LONG_BIT
#ifdef __LP64__
#define LONG_BIT 64
#else
#define LONG_BIT 32
#endif
#endif

/* ---- lib/libc/amd64/_fpmath.h ------------------------------------------- */

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
		unsigned long long	man	:64;
		unsigned int		expsign	:16;
		unsigned long long	junk	:48;
	} xbits;
};

#define	LDBL_NBIT	0x80000000
#define	mask_nbit_l(u)	((u).bits.manh &= ~LDBL_NBIT)

#define	LDBL_MANH_SIZE	32
#define	LDBL_MANL_SIZE	32

/* amd64/i386 use the explicit-integer-bit format: no LDBL_IMPLICIT_NBIT. */

/* ======================================================================== */
/* lib/msun/src/s_ilogbl.c                                                  */
/* ======================================================================== */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

int
ref_ilogbl(long double x)
{
	union IEEEl2bits u;
	unsigned long m;
	int b;

	u.e = x;
	if (u.bits.exp == 0) {
		if ((u.bits.manl | u.bits.manh) == 0)
			return (FP_ILOGB0);
		/* denormalized */
		if (u.bits.manh == 0) {
			m = 1lu << (LDBL_MANL_SIZE - 1);
			for (b = LDBL_MANH_SIZE; !(u.bits.manl & m); m >>= 1)
				b++;
		} else {
			m = 1lu << (LDBL_MANH_SIZE - 1);
			for (b = 0; !(u.bits.manh & m); m >>= 1)
				b++;
		}
#ifdef LDBL_IMPLICIT_NBIT
		b++;
#endif
		return (LDBL_MIN_EXP - b - 1);
	} else if (u.bits.exp < (LDBL_MAX_EXP << 1) - 1)
		return (u.bits.exp - LDBL_MAX_EXP + 1);
	else if (u.bits.manl != 0 || u.bits.manh != 0)
		return (FP_ILOGBNAN);
	else
		return (INT_MAX);
}
