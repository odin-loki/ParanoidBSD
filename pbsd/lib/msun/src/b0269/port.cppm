// PBSD port of HardenedBSD lib/msun/src batch b0269.
//
// Sources ported in this module:
//   s_fabsl.c       -> fabsl
//   s_nexttoward.c  -> nexttoward
//
// Original copyright headers are reproduced verbatim above each ported
// function.  The ports are literal transcriptions.

module;

#include <cfloat>
#include <climits>
#include <cstdint>
#include <endian.h>
#include <sys/types.h>

export module pbsd.lib.msun.src.b0269;

namespace pbsd::lib_msun_src::b0269 {

/* from lib/libc/amd64/_fpmath.h */
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

#define	EXTRACT_WORDS(ix0,ix1,d)				\
do {								\
	union {							\
		double value;					\
		struct {					\
#if __BYTE_ORDER == __BIG_ENDIAN				\
			u_int32_t msw;				\
			u_int32_t lsw;				\
#else							\
			u_int32_t lsw;				\
			u_int32_t msw;				\
#endif							\
		} parts;					\
	} ew_u;							\
	ew_u.value = (d);					\
	(ix0) = ew_u.parts.msw;					\
	(ix1) = ew_u.parts.lsw;					\
} while (0)

#define	INSERT_WORDS(d,ix0,ix1)					\
do {								\
	union {							\
		double value;					\
		struct {					\
#if __BYTE_ORDER == __BIG_ENDIAN				\
			u_int32_t msw;				\
			u_int32_t lsw;				\
#else							\
			u_int32_t lsw;				\
			u_int32_t msw;				\
#endif							\
		} parts;					\
	} iw_u;							\
	iw_u.parts.msw = (ix0);					\
	iw_u.parts.lsw = (ix1);					\
	(d) = iw_u.value;					\
} while (0)

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2003 Dag-Erling Smørgrav
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

export long double
fabsl(long double x)
{
	union IEEEl2bits u;

	u.e = x;
	u.bits.sign = 0;
	return (u.e);
}

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

/*
 * We assume that a long double has a 15-bit exponent.  On systems
 * where long double is the same as double, nexttoward() is an alias
 * for nextafter(), so we don't use this routine.
 */

#if LDBL_MAX_EXP != 0x4000
#error "Unsupported long double format"
#endif

export double
nexttoward(double x, long double y)
{
	union IEEEl2bits uy;
	volatile double t;
	int32_t hx,ix;
	u_int32_t lx;

	EXTRACT_WORDS(hx,lx,x);
	ix = hx&0x7fffffff;		/* |x| */
	uy.e = y;

	if(((ix>=0x7ff00000)&&((ix-0x7ff00000)|lx)!=0) ||
	    (uy.bits.exp == 0x7fff &&
	     ((uy.bits.manh&~LDBL_NBIT)|uy.bits.manl) != 0))
	   return x+y;	/* x or y is nan */
	if(x==y) return (double)y;		/* x=y, return y */
	if(x==0.0) {
	    INSERT_WORDS(x,uy.bits.sign<<31,1);	/* return +-minsubnormal */
	    t = x*x;
	    if(t==x) return t; else return x;	/* raise underflow flag */
	}
	if(hx>0.0 ^ x < y) {			/* x -= ulp */
	    if(lx==0) hx -= 1;
	    lx -= 1;
	} else {				/* x += ulp */
	    lx += 1;
	    if(lx==0) hx += 1;
	}
	ix = hx&0x7ff00000;
	if(ix>=0x7ff00000) return x+x;	/* overflow  */
	if(ix<0x00100000) {		/* underflow */
	    t = x*x;
	    if(t!=x) {		/* raise underflow flag */
	        INSERT_WORDS(x,hx,lx);
		return x;
	    }
	}
	INSERT_WORDS(x,hx,lx);
	return x;
}

} // namespace pbsd::lib_msun_src::b0269
