/*
 * PBSD batch b0095 -- C++23 module port of
 *
 *	lib/msun/ld80/e_powl.c
 *	lib/msun/ld80/s_logl.c
 *
 * This is a faithful transliteration.  Control flow, evaluation order,
 * integer signedness (note the int16_t hx in log1pl, whose sign extension of
 * the 16 bit expsign field is load-bearing), the thread-local scratch
 * globals of e_powl.c and the dead branches of both files are all preserved
 * exactly.  Nothing has been improved.
 */

module;

#include <stdint.h>
#include <float.h>
#include <math.h>

export module pbsd.lib.msun.ld80.b0095;

/* ------------------------------------------------------------------------
 * Support definitions taken from lib/libc/amd64/_fpmath.h and
 * lib/msun/src/math_private.h.  The bit-field unions of the originals are
 * expressed with __builtin_memcpy here, which is the same object
 * representation traffic without relying on C++ union type punning.
 * ------------------------------------------------------------------------ */

#define	EXTRACT_LDBL80_WORDS(ix0,ix1,d)					\
do {									\
	long double ew_e = (d);						\
	unsigned char ew_b[sizeof(long double)];			\
	unsigned long long ew_man;					\
	unsigned short ew_expsign;					\
	__builtin_memcpy(ew_b, &ew_e, sizeof(ew_b));			\
	__builtin_memcpy(&ew_man, ew_b, 8);				\
	__builtin_memcpy(&ew_expsign, ew_b + 8, 2);			\
	(ix0) = ew_expsign;						\
	(ix1) = ew_man;							\
} while (0)

#define	GET_LDBL_EXPSIGN(i,d)						\
do {									\
	long double ge_e = (d);						\
	unsigned short ge_expsign;					\
	__builtin_memcpy(&ge_expsign,					\
	    reinterpret_cast<const unsigned char *>(&ge_e) + 8, 2);	\
	(i) = ge_expsign;						\
} while (0)

#define	SET_LDBL_EXPSIGN(d,v)						\
do {									\
	long double se_e = (d);						\
	unsigned short se_expsign = (unsigned short)(v);		\
	__builtin_memcpy(reinterpret_cast<unsigned char *>(&se_e) + 8,	\
	    &se_expsign, 2);						\
	(d) = se_e;							\
} while (0)

#define	SET_FLOAT_WORD(d,i)						\
do {									\
	unsigned int sf_w = (unsigned int)(i);				\
	float sf_v;							\
	__builtin_memcpy(&sf_v, &sf_w, 4);				\
	(d) = sf_v;							\
} while (0)

/* amd64: no i387 precision juggling is required. */
#define	ENTERI()
#define	ENTERIT(x)
#define	RETURNI(x)	RETURNF(x)
#define	RETURNF(v)	return (v)

#define	_2sumF(a, b) do {	\
	__typeof__(a) __w;	\
				\
	__w = (a) + (b);	\
	(b) = ((a) - __w) + (b); \
	(a) = __w;		\
} while (0)

#define	_3sumF(a, b, c) do {	\
	__typeof__(a) __tmp;	\
				\
	__tmp = (c);		\
	_2sumF(__tmp, (a));	\
	(b) += (a);		\
	(a) = __tmp;		\
} while (0)

#define	nan_mix(x, y)		(nan_mix_op((x), (y), +))
#define	nan_mix_op(x, y, op)	(((x) + 0.0L) op ((y) + 0))

#define	RETURNSPI(rp) do {		\
	if (!(rp)->lo_set)		\
		RETURNI((rp)->hi);	\
	RETURNI((rp)->hi + (rp)->lo);	\
} while (0)

namespace pbsd::lib_msun_ld80::b0095 {

/* ========================================================================
 * lib/msun/ld80/e_powl.c
 * ======================================================================== */

/*-
 * Copyright (c) 2008 Stephen L. Moshier <steve@moshier.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Polynomial evaluator:
 *  P[0] x^n  +  P[1] x^(n-1)  +  ...  +  P[n]
 */
static inline long double
__polevll(long double x, const long double *PP, int n)
{
	long double y;
	const long double *P;

	P = PP;
	y = *P++;
	do {
		y = y * x + *P++;
	} while (--n);

	return (y);
}

/*
 * Polynomial evaluator:
 *  x^n  +  P[0] x^(n-1)  +  P[1] x^(n-2)  +  ...  +  P[n]
 */
static inline long double
__p1evll(long double x, const long double *PP, int n)
{
	long double y;
	const long double *P;

	P = PP;
	n -= 1;
	y = x + *P++;
	do {
		y = y * x + *P++;
	} while (--n);

	return (y);
}

/*							powl.c
 *
 *	Power function, long double precision
 *
 * See the original source for the full description.
 */

/* Table size */
#define NXT 32
/* log2(Table size) */
#define LNXT 5

/* log(1+x) =  x - .5x^2 + x^3 *  P(z)/Q(z)
 * on the domain  2^(-1/32) - 1  <=  x  <=  2^(1/32) - 1
 */
static const long double P[] = {
 8.3319510773868690346226E-4L,
 4.9000050881978028599627E-1L,
 1.7500123722550302671919E0L,
 1.4000100839971580279335E0L,
};
static const long double Q[] = {
/* 1.0000000000000000000000E0L,*/
 5.2500282295834889175431E0L,
 8.4000598057587009834666E0L,
 4.2000302519914740834728E0L,
};
/* A[i] = 2^(-i/32), rounded to IEEE long double precision.
 * If i is even, A[i] + B[i/2] gives additional accuracy.
 */
static const long double A[33] = {
 1.0000000000000000000000E0L,
 9.7857206208770013448287E-1L,
 9.5760328069857364691013E-1L,
 9.3708381705514995065011E-1L,
 9.1700404320467123175367E-1L,
 8.9735453750155359320742E-1L,
 8.7812608018664974155474E-1L,
 8.5930964906123895780165E-1L,
 8.4089641525371454301892E-1L,
 8.2287773907698242225554E-1L,
 8.0524516597462715409607E-1L,
 7.8799042255394324325455E-1L,
 7.7110541270397041179298E-1L,
 7.5458221379671136985669E-1L,
 7.3841307296974965571198E-1L,
 7.2259040348852331001267E-1L,
 7.0710678118654752438189E-1L,
 6.9195494098191597746178E-1L,
 6.7712777346844636413344E-1L,
 6.6261832157987064729696E-1L,
 6.4841977732550483296079E-1L,
 6.3452547859586661129850E-1L,
 6.2092890603674202431705E-1L,
 6.0762367999023443907803E-1L,
 5.9460355750136053334378E-1L,
 5.8186242938878875689693E-1L,
 5.6939431737834582684856E-1L,
 5.5719337129794626814472E-1L,
 5.4525386633262882960438E-1L,
 5.3357020033841180906486E-1L,
 5.2213689121370692017331E-1L,
 5.1094857432705833910408E-1L,
 5.0000000000000000000000E-1L,
};
static const long double B[17] = {
 0.0000000000000000000000E0L,
 2.6176170809902549338711E-20L,
-1.0126791927256478897086E-20L,
 1.3438228172316276937655E-21L,
 1.2207982955417546912101E-20L,
-6.3084814358060867200133E-21L,
 1.3164426894366316434230E-20L,
-1.8527916071632873716786E-20L,
 1.8950325588932570796551E-20L,
 1.5564775779538780478155E-20L,
 6.0859793637556860974380E-21L,
-2.0208749253662532228949E-20L,
 1.4966292219224761844552E-20L,
 3.3540909728056476875639E-21L,
-8.6987564101742849540743E-22L,
-1.2327176863327626135542E-20L,
 0.0000000000000000000000E0L,
};

/* 2^x = 1 + x P(x),
 * on the interval -1/32 <= x <= 0
 */
static const long double R[] = {
 1.5089970579127659901157E-5L,
 1.5402715328927013076125E-4L,
 1.3333556028915671091390E-3L,
 9.6181291046036762031786E-3L,
 5.5504108664798463044015E-2L,
 2.4022650695910062854352E-1L,
 6.9314718055994530931447E-1L,
};

#define douba(k) A[k]
#define doubb(k) B[k]
#define MEXP (NXT*16384.0L)
/* The following if denormal numbers are supported, else -MEXP: */
#define MNEXP (-NXT*(16384.0L+64.0L))
/* log2(e) - 1 */
#define LOG2EA 0.44269504088896340735992L

#define F W
#define Fa Wa
#define Fb Wb
#define G W
#define Ga Wa
#define Gb u
#define H W
#define Ha Wb
#define Hb Wb

static const long double MAXLOGL = 1.1356523406294143949492E4L;
static const long double MINLOGL = -1.13994985314888605586758E4L;
static const long double LOGE2L = 6.9314718055994530941723E-1L;
static thread_local volatile long double z;
static thread_local long double w, W, Wa, Wb, ya, yb, u;
static const long double huge = 0x1p10000L;
#if 0 /* XXX Prevent gcc from erroneously constant folding this. */
static const long double twom10000 = 0x1p-10000L;
#else
static thread_local volatile long double twom10000 = 0x1p-10000L;
#endif

static long double reducl( long double );
static long double powil ( long double, int );

} /* namespace */

export namespace pbsd::lib_msun_ld80::b0095 {

long double
powl(long double x, long double y)
{
/* double F, Fa, Fb, G, Ga, Gb, H, Ha, Hb */
int i, nflg, iyflg, yoddint;
long e;

if( y == 0.0L )
	return( 1.0L );

if( x == 1.0L )
	return( 1.0L );

if( isnan(x) )
	return ( nan_mix(x, y) );
if( isnan(y) )
	return ( nan_mix(x, y) );

if( y == 1.0L )
	return( x );

if( !isfinite(y) && x == -1.0L )
	return( 1.0L );

if( y >= LDBL_MAX )
	{
	if( x > 1.0L )
		return( INFINITY );
	if( x > 0.0L && x < 1.0L )
		return( 0.0L );
	if( x < -1.0L )
		return( INFINITY );
	if( x > -1.0L && x < 0.0L )
		return( 0.0L );
	}
if( y <= -LDBL_MAX )
	{
	if( x > 1.0L )
		return( 0.0L );
	if( x > 0.0L && x < 1.0L )
		return( INFINITY );
	if( x < -1.0L )
		return( 0.0L );
	if( x > -1.0L && x < 0.0L )
		return( INFINITY );
	}
if( x >= LDBL_MAX )
	{
	if( y > 0.0L )
		return( INFINITY );
	return( 0.0L );
	}

w = floorl(y);
/* Set iyflg to 1 if y is an integer.  */
iyflg = 0;
if( w == y )
	iyflg = 1;

/* Test for odd integer y.  */
yoddint = 0;
if( iyflg )
	{
	ya = fabsl(y);
	ya = floorl(0.5L * ya);
	yb = 0.5L * fabsl(w);
	if( ya != yb )
		yoddint = 1;
	}

if( x <= -LDBL_MAX )
	{
	if( y > 0.0L )
		{
		if( yoddint )
			return( -INFINITY );
		return( INFINITY );
		}
	if( y < 0.0L )
		{
		if( yoddint )
			return( -0.0L );
		return( 0.0 );
		}
	}


nflg = 0;	/* flag = 1 if x<0 raised to integer power */
if( x <= 0.0L )
	{
	if( x == 0.0L )
		{
		if( y < 0.0 )
			{
			if( signbit(x) && yoddint )
				return( -INFINITY );
			return( INFINITY );
			}
		if( y > 0.0 )
			{
			if( signbit(x) && yoddint )
				return( -0.0L );
			return( 0.0 );
			}
		if( y == 0.0L )
			return( 1.0L );  /*   0**0   */
		else
			return( 0.0L );  /*   0**y   */
		}
	else
		{
		if( iyflg == 0 )
			return (x - x) / (x - x); /* (x<0)**(non-int) is NaN */
		nflg = 1;
		}
	}

/* Integer power of an integer.  */

if( iyflg )
	{
	i = w;
	w = floorl(x);
	if( (w == x) && (fabsl(y) < 32768.0) )
		{
		w = powil( x, (int) y );
		return( w );
		}
	}


if( nflg )
	x = fabsl(x);

/* separate significand from exponent */
x = frexpl( x, &i );
e = i;

/* find significand in antilog table A[] */
i = 1;
if( x <= douba(17) )
	i = 17;
if( x <= douba(i+8) )
	i += 8;
if( x <= douba(i+4) )
	i += 4;
if( x <= douba(i+2) )
	i += 2;
if( x >= douba(1) )
	i = -1;
i += 1;


/* Find (x - A[i])/A[i]
 * in order to compute log(x/A[i]):
 *
 * log(x) = log( a x/a ) = log(a) + log(x/a)
 *
 * log(x/a) = log(1+v),  v = x/a - 1 = (x-a)/a
 */
x -= douba(i);
x -= doubb(i/2);
x /= douba(i);


/* rational approximation for log(1+v):
 *
 * log(1+v)  =  v  -  v**2/2  +  v**3 P(v) / Q(v)
 */
z = x*x;
w = x * ( z * __polevll( x, P, 3 ) / __p1evll( x, Q, 3 ) );
w = w - ldexpl( z, -1 );   /*  w - 0.5 * z  */

/* Convert to base 2 logarithm:
 * multiply by log2(e) = 1 + LOG2EA
 */
z = LOG2EA * w;
z += w;
z += LOG2EA * x;
z += x;

/* Compute exponent term of the base 2 logarithm. */
w = -i;
w = ldexpl( w, -LNXT );	/* divide by NXT */
w += e;
/* Now base 2 log of x is w + z. */

/* Multiply base 2 log by y, in extended precision. */

/* separate y into large part ya
 * and small part yb less than 1/NXT
 */
ya = reducl(y);
yb = y - ya;

/* (w+z)(ya+yb)
 * = w*ya + w*yb + z*y
 */
F = z * y  +  w * yb;
Fa = reducl(F);
Fb = F - Fa;

G = Fa + w * ya;
Ga = reducl(G);
Gb = G - Ga;

H = Fb + Gb;
Ha = reducl(H);
w = ldexpl( Ga+Ha, LNXT );

/* Test the power of 2 for overflow */
if( w > MEXP )
	return (huge * huge);		/* overflow */

if( w < MNEXP )
	return (twom10000 * twom10000);	/* underflow */

e = w;
Hb = H - Ha;

if( Hb > 0.0L )
	{
	e += 1;
	Hb -= (1.0L/NXT);  /*0.0625L;*/
	}

/* Now the product y * log2(x)  =  Hb + e/NXT.
 *
 * Compute base 2 exponential of Hb,
 * where -0.0625 <= Hb <= 0.
 */
z = Hb * __polevll( Hb, R, 6 );  /*    z  =  2**Hb - 1    */

/* Express e/NXT as an integer plus a negative number of (1/NXT)ths.
 * Find lookup table entry for the fractional power of 2.
 */
if( e < 0 )
	i = 0;
else
	i = 1;
i = e/NXT + i;
e = NXT*i - e;
w = douba( e );
z = w * z;      /*    2**-e * ( 1 + (2**Hb-1) )    */
z = z + w;
z = ldexpl( z, i );  /* multiply by integer power of 2 */

if( nflg )
	{
/* For negative x,
 * find out if the integer exponent
 * is odd or even.
 */
	w = ldexpl( y, -1 );
	w = floorl(w);
	w = ldexpl( w, 1 );
	if( w != y )
		z = -z; /* odd exponent */
	}

return( z );
}

} /* export namespace */

namespace pbsd::lib_msun_ld80::b0095 {

/* Find a multiple of 1/NXT that is within 1/NXT of x. */
static inline long double
reducl(long double x)
{
long double t;

t = ldexpl( x, LNXT );
t = floorl( t );
t = ldexpl( t, -LNXT );
return(t);
}

/*							powil.c
 *
 *	Real raised to integer power, long double precision
 *
 * See the original source for the full description.
 */

static long double
powil(long double x, int nn)
{
long double ww, y;
long double s;
int n, e, sign, asign, lx;

if( x == 0.0L )
	{
	if( nn == 0 )
		return( 1.0L );
	else if( nn < 0 )
		return( LDBL_MAX );
	else
		return( 0.0L );
	}

if( nn == 0 )
	return( 1.0L );


if( x < 0.0L )
	{
	asign = -1;
	x = -x;
	}
else
	asign = 0;


if( nn < 0 )
	{
	sign = -1;
	n = -nn;
	}
else
	{
	sign = 1;
	n = nn;
	}

/* Overflow detection */

/* Calculate approximate logarithm of answer */
s = x;
s = frexpl( s, &lx );
e = (lx - 1)*n;
if( (e == 0) || (e > 64) || (e < -64) )
	{
	s = (s - 7.0710678118654752e-1L) / (s +  7.0710678118654752e-1L);
	s = (2.9142135623730950L * s - 0.5L + lx) * nn * LOGE2L;
	}
else
	{
	s = LOGE2L * e;
	}

if( s > MAXLOGL )
	return (huge * huge);		/* overflow */

if( s < MINLOGL )
	return (twom10000 * twom10000);	/* underflow */
/* Handle tiny denormal answer, but with less accuracy
 * since roundoff error in 1.0/x will be amplified.
 * The precise demarcation should be the gradual underflow threshold.
 */
if( s < (-MAXLOGL+2.0L) )
	{
	x = 1.0L/x;
	sign = -sign;
	}

/* First bit of the power */
if( n & 1 )
	y = x;

else
	{
	y = 1.0L;
	asign = 0;
	}

ww = x;
n >>= 1;
while( n )
	{
	ww = ww * ww;	/* arg to the 2-to-the-kth power */
	if( n & 1 )	/* if that bit is set, then include in product */
		y *= ww;
	n >>= 1;
	}

if( asign )
	y = -y; /* odd power of negative number */
if( sign < 0 )
	y = 1.0L/y;
return(y);
}

} /* namespace */

/* ========================================================================
 * End of e_powl.c.  Drop its object-like macros so that they do not collide
 * with the function-like macros of s_logl.c.
 * ======================================================================== */

#undef F
#undef Fa
#undef Fb
#undef G
#undef Ga
#undef Gb
#undef H
#undef Ha
#undef Hb
#undef douba
#undef doubb
#undef MEXP
#undef MNEXP
#undef LOG2EA
#undef NXT
#undef LNXT

/* ========================================================================
 * lib/msun/ld80/s_logl.c
 * ======================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2007-2013 Bruce D. Evans
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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

/**
 * Implementation of the natural logarithm of x for Intel 80-bit format.
 * See the original source for the full description of the algorithm.
 */

#define	i386_SSE_GOOD
#ifndef NO_STRUCT_RETURN
#define	STRUCT_RETURN
#endif

#if !defined(NO_UTAB) && !defined(NO_UTABL)
#define	USE_UTAB
#endif

namespace pbsd::lib_msun_ld80::b0095 {

/*
 * Domain [-0.005280, 0.004838], range ~[-5.1736e-22, 5.1738e-22]:
 * |log(1 + d)/d - p(d)| < 2**-70.7
 */
static const double
P2 = -0.5,
P3 =  3.3333333333333359e-1,		/*  0x1555555555555a.0p-54 */
P4 = -2.5000000000004424e-1,		/* -0x1000000000031d.0p-54 */
P5 =  1.9999999992970016e-1,		/*  0x1999999972f3c7.0p-55 */
P6 = -1.6666666072191585e-1,		/* -0x15555548912c09.0p-55 */
P7 =  1.4286227413310518e-1,		/*  0x12494f9d9def91.0p-55 */
P8 = -1.2518388626763144e-1;		/* -0x1006068cc0b97c.0p-55 */

static volatile const double zero = 0;

#define	INTERVALS	128
#define	LOG2_INTERVALS	7
#define	TSIZE		(INTERVALS + 1)
#define	G(i)		(T[(i)].G)
#define	F_hi(i)		(T[(i)].F_hi)
#define	F_lo(i)		(T[(i)].F_lo)
#define	ln2_hi		F_hi(TSIZE - 1)
#define	ln2_lo		F_lo(TSIZE - 1)
#define	E(i)		(U[(i)].E)
#define	H(i)		(U[(i)].H)

static const struct {
	float	G;			/* 1/(1 + i/128) rounded to 8/9 bits */
	float	F_hi;			/* log(1 / G_i) rounded (see below) */
	double	F_lo;			/* next 53 bits for log(1 / G_i) */
} T[TSIZE] = {
	 { 0x800000.0p-23,  0,               0 },
	 { 0xfe0000.0p-24,  0x8080ac.0p-30, -0x14ee431dae6675.0p-84 },
	 { 0xfc0000.0p-24,  0x8102b3.0p-29, -0x1db29ee2d83718.0p-84 },
	 { 0xfa0000.0p-24,  0xc24929.0p-29,  0x1191957d173698.0p-83 },
	 { 0xf80000.0p-24,  0x820aec.0p-28,  0x13ce8888e02e79.0p-82 },
	 { 0xf60000.0p-24,  0xa33577.0p-28, -0x17a4382ce6eb7c.0p-82 },
	 { 0xf48000.0p-24,  0xbc42cb.0p-28, -0x172a21161a1076.0p-83 },
	 { 0xf30000.0p-24,  0xd57797.0p-28, -0x1e09de07cb9589.0p-82 },
	 { 0xf10000.0p-24,  0xf7518e.0p-28,  0x1ae1eec1b036c5.0p-91 },
	 { 0xef0000.0p-24,  0x8cb9df.0p-27, -0x1d7355325d560e.0p-81 },
	 { 0xed8000.0p-24,  0x999ec0.0p-27, -0x1f9f02d256d503.0p-82 },
	 { 0xec0000.0p-24,  0xa6988b.0p-27, -0x16fc0a9d12c17a.0p-83 },
	 { 0xea0000.0p-24,  0xb80698.0p-27,  0x15d581c1e8da9a.0p-81 },
	 { 0xe80000.0p-24,  0xc99af3.0p-27, -0x1535b3ba8f150b.0p-83 },
	 { 0xe70000.0p-24,  0xd273b2.0p-27,  0x163786f5251af0.0p-85 },
	 { 0xe50000.0p-24,  0xe442c0.0p-27,  0x1bc4b2368e32d5.0p-84 },
	 { 0xe38000.0p-24,  0xf1b83f.0p-27,  0x1c6090f684e676.0p-81 },
	 { 0xe20000.0p-24,  0xff448a.0p-27, -0x1890aa69ac9f42.0p-82 },
	 { 0xe08000.0p-24,  0x8673f6.0p-26,  0x1b9985194b6b00.0p-80 },
	 { 0xdf0000.0p-24,  0x8d515c.0p-26, -0x1dc08d61c6ef1e.0p-83 },
	 { 0xdd8000.0p-24,  0x943a9e.0p-26, -0x1f72a2dac729b4.0p-82 },
	 { 0xdc0000.0p-24,  0x9b2fe6.0p-26, -0x1fd4dfd3a0afb9.0p-80 },
	 { 0xda8000.0p-24,  0xa2315d.0p-26, -0x11b26121629c47.0p-82 },
	 { 0xd90000.0p-24,  0xa93f2f.0p-26,  0x1286d633e8e569.0p-81 },
	 { 0xd78000.0p-24,  0xb05988.0p-26,  0x16128eba936770.0p-84 },
	 { 0xd60000.0p-24,  0xb78094.0p-26,  0x16ead577390d32.0p-80 },
	 { 0xd50000.0p-24,  0xbc4c6c.0p-26,  0x151131ccf7c7b7.0p-81 },
	 { 0xd38000.0p-24,  0xc3890a.0p-26, -0x115e2cd714bd06.0p-80 },
	 { 0xd20000.0p-24,  0xcad2d7.0p-26, -0x1847f406ebd3b0.0p-82 },
	 { 0xd10000.0p-24,  0xcfb620.0p-26,  0x1c2259904d6866.0p-81 },
	 { 0xcf8000.0p-24,  0xd71653.0p-26,  0x1ece57a8d5ae55.0p-80 },
	 { 0xce0000.0p-24,  0xde843a.0p-26, -0x1f109d4bc45954.0p-81 },
	 { 0xcd0000.0p-24,  0xe37fde.0p-26,  0x1bc03dc271a74d.0p-81 },
	 { 0xcb8000.0p-24,  0xeb050c.0p-26, -0x1bf2badc0df842.0p-85 },
	 { 0xca0000.0p-24,  0xf29878.0p-26, -0x18efededd89fbe.0p-87 },
	 { 0xc90000.0p-24,  0xf7ad6f.0p-26,  0x1373ff977baa69.0p-81 },
	 { 0xc80000.0p-24,  0xfcc8e3.0p-26,  0x196766f2fb3283.0p-80 },
	 { 0xc68000.0p-24,  0x823f30.0p-25,  0x19bd076f7c434e.0p-79 },
	 { 0xc58000.0p-24,  0x84d52c.0p-25, -0x1a327257af0f46.0p-79 },
	 { 0xc40000.0p-24,  0x88bc74.0p-25,  0x113f23def19c5a.0p-81 },
	 { 0xc30000.0p-24,  0x8b5ae6.0p-25,  0x1759f6e6b37de9.0p-79 },
	 { 0xc20000.0p-24,  0x8dfccb.0p-25,  0x1ad35ca6ed5148.0p-81 },
	 { 0xc10000.0p-24,  0x90a22b.0p-25,  0x1a1d71a87deba4.0p-79 },
	 { 0xbf8000.0p-24,  0x94a0d8.0p-25, -0x139e5210c2b731.0p-80 },
	 { 0xbe8000.0p-24,  0x974f16.0p-25, -0x18f6ebcff3ed73.0p-81 },
	 { 0xbd8000.0p-24,  0x9a00f1.0p-25, -0x1aa268be39aab7.0p-79 },
	 { 0xbc8000.0p-24,  0x9cb672.0p-25, -0x14c8815839c566.0p-79 },
	 { 0xbb0000.0p-24,  0xa0cda1.0p-25,  0x1eaf46390dbb24.0p-81 },
	 { 0xba0000.0p-24,  0xa38c6e.0p-25,  0x138e20d831f698.0p-81 },
	 { 0xb90000.0p-24,  0xa64f05.0p-25, -0x1e8d3c41123616.0p-82 },
	 { 0xb80000.0p-24,  0xa91570.0p-25,  0x1ce28f5f3840b2.0p-80 },
	 { 0xb70000.0p-24,  0xabdfbb.0p-25, -0x186e5c0a424234.0p-79 },
	 { 0xb60000.0p-24,  0xaeadef.0p-25, -0x14d41a0b2a08a4.0p-83 },
	 { 0xb50000.0p-24,  0xb18018.0p-25,  0x16755892770634.0p-79 },
	 { 0xb40000.0p-24,  0xb45642.0p-25, -0x16395ebe59b152.0p-82 },
	 { 0xb30000.0p-24,  0xb73077.0p-25,  0x1abc65c8595f09.0p-80 },
	 { 0xb20000.0p-24,  0xba0ec4.0p-25, -0x1273089d3dad89.0p-79 },
	 { 0xb10000.0p-24,  0xbcf133.0p-25,  0x10f9f67b1f4bbf.0p-79 },
	 { 0xb00000.0p-24,  0xbfd7d2.0p-25, -0x109fab90486409.0p-80 },
	 { 0xaf0000.0p-24,  0xc2c2ac.0p-25, -0x1124680aa43333.0p-79 },
	 { 0xae8000.0p-24,  0xc439b3.0p-25, -0x1f360cc4710fc0.0p-80 },
	 { 0xad8000.0p-24,  0xc72afd.0p-25, -0x132d91f21d89c9.0p-80 },
	 { 0xac8000.0p-24,  0xca20a2.0p-25, -0x16bf9b4d1f8da8.0p-79 },
	 { 0xab8000.0p-24,  0xcd1aae.0p-25,  0x19deb5ce6a6a87.0p-81 },
	 { 0xaa8000.0p-24,  0xd0192f.0p-25,  0x1a29fb48f7d3cb.0p-79 },
	 { 0xaa0000.0p-24,  0xd19a20.0p-25,  0x1127d3c6457f9d.0p-81 },
	 { 0xa90000.0p-24,  0xd49f6a.0p-25, -0x1ba930e486a0ac.0p-81 },
	 { 0xa80000.0p-24,  0xd7a94b.0p-25, -0x1b6e645f31549e.0p-79 },
	 { 0xa70000.0p-24,  0xdab7d0.0p-25,  0x1118a425494b61.0p-80 },
	 { 0xa68000.0p-24,  0xdc40d5.0p-25,  0x1966f24d29d3a3.0p-80 },
	 { 0xa58000.0p-24,  0xdf566d.0p-25, -0x1d8e52eb2248f1.0p-82 },
	 { 0xa48000.0p-24,  0xe270ce.0p-25, -0x1ee370f96e6b68.0p-80 },
	 { 0xa40000.0p-24,  0xe3ffce.0p-25,  0x1d155324911f57.0p-80 },
	 { 0xa30000.0p-24,  0xe72179.0p-25, -0x1fe6e2f2f867d9.0p-80 },
	 { 0xa20000.0p-24,  0xea4812.0p-25,  0x1b7be9add7f4d4.0p-80 },
	 { 0xa18000.0p-24,  0xebdd3d.0p-25,  0x1b3cfb3f7511dd.0p-79 },
	 { 0xa08000.0p-24,  0xef0b5b.0p-25, -0x1220de1f730190.0p-79 },
	 { 0xa00000.0p-24,  0xf0a451.0p-25, -0x176364c9ac81cd.0p-80 },
	 { 0x9f0000.0p-24,  0xf3da16.0p-25,  0x1eed6b9aafac8d.0p-81 },
	 { 0x9e8000.0p-24,  0xf576e9.0p-25,  0x1d593218675af2.0p-79 },
	 { 0x9d8000.0p-24,  0xf8b47c.0p-25, -0x13e8eb7da053e0.0p-84 },
	 { 0x9d0000.0p-24,  0xfa553f.0p-25,  0x1c063259bcade0.0p-79 },
	 { 0x9c0000.0p-24,  0xfd9ac5.0p-25,  0x1ef491085fa3c1.0p-79 },
	 { 0x9b8000.0p-24,  0xff3f8c.0p-25,  0x1d607a7c2b8c53.0p-79 },
	 { 0x9a8000.0p-24,  0x814697.0p-24, -0x12ad3817004f3f.0p-78 },
	 { 0x9a0000.0p-24,  0x821b06.0p-24, -0x189fc53117f9e5.0p-81 },
	 { 0x990000.0p-24,  0x83c5f8.0p-24,  0x14cf15a048907b.0p-79 },
	 { 0x988000.0p-24,  0x849c7d.0p-24,  0x1cbb1d35fb8287.0p-78 },
	 { 0x978000.0p-24,  0x864ba6.0p-24,  0x1128639b814f9c.0p-78 },
	 { 0x970000.0p-24,  0x87244c.0p-24,  0x184733853300f0.0p-79 },
	 { 0x968000.0p-24,  0x87fdaa.0p-24,  0x109d23aef77dd6.0p-80 },
	 { 0x958000.0p-24,  0x89b293.0p-24, -0x1a81ef367a59de.0p-78 },
	 { 0x950000.0p-24,  0x8a8e20.0p-24, -0x121ad3dbb2f452.0p-78 },
	 { 0x948000.0p-24,  0x8b6a6a.0p-24, -0x1cfb981628af72.0p-79 },
	 { 0x938000.0p-24,  0x8d253a.0p-24, -0x1d21730ea76cfe.0p-79 },
	 { 0x930000.0p-24,  0x8e03c2.0p-24,  0x135cc00e566f77.0p-78 },
	 { 0x928000.0p-24,  0x8ee30d.0p-24, -0x10fcb5df257a26.0p-80 },
	 { 0x918000.0p-24,  0x90a3ee.0p-24, -0x16e171b15433d7.0p-79 },
	 { 0x910000.0p-24,  0x918587.0p-24, -0x1d050da07f3237.0p-79 },
	 { 0x908000.0p-24,  0x9267e7.0p-24,  0x1be03669a5268d.0p-79 },
	 { 0x8f8000.0p-24,  0x942f04.0p-24,  0x10b28e0e26c337.0p-79 },
	 { 0x8f0000.0p-24,  0x9513c3.0p-24,  0x1a1d820da57cf3.0p-78 },
	 { 0x8e8000.0p-24,  0x95f950.0p-24, -0x19ef8f13ae3cf1.0p-79 },
	 { 0x8e0000.0p-24,  0x96dfab.0p-24, -0x109e417a6e507c.0p-78 },
	 { 0x8d0000.0p-24,  0x98aed2.0p-24,  0x10d01a2c5b0e98.0p-79 },
	 { 0x8c8000.0p-24,  0x9997a2.0p-24, -0x1d6a50d4b61ea7.0p-78 },
	 { 0x8c0000.0p-24,  0x9a8145.0p-24,  0x1b3b190b83f952.0p-78 },
	 { 0x8b8000.0p-24,  0x9b6bbf.0p-24,  0x13a69fad7e7abe.0p-78 },
	 { 0x8b0000.0p-24,  0x9c5711.0p-24, -0x11cd12316f576b.0p-78 },
	 { 0x8a8000.0p-24,  0x9d433b.0p-24,  0x1c95c444b807a2.0p-79 },
	 { 0x898000.0p-24,  0x9f1e22.0p-24, -0x1b9c224ea698c3.0p-79 },
	 { 0x890000.0p-24,  0xa00ce1.0p-24,  0x125ca93186cf0f.0p-81 },
	 { 0x888000.0p-24,  0xa0fc80.0p-24, -0x1ee38a7bc228b3.0p-79 },
	 { 0x880000.0p-24,  0xa1ed00.0p-24, -0x1a0db876613d20.0p-78 },
	 { 0x878000.0p-24,  0xa2de62.0p-24,  0x193224e8516c01.0p-79 },
	 { 0x870000.0p-24,  0xa3d0a9.0p-24,  0x1fa28b4d2541ad.0p-79 },
	 { 0x868000.0p-24,  0xa4c3d6.0p-24,  0x1c1b5760fb4572.0p-78 },
	 { 0x858000.0p-24,  0xa6acea.0p-24,  0x1fed5d0f65949c.0p-80 },
	 { 0x850000.0p-24,  0xa7a2d4.0p-24,  0x1ad270c9d74936.0p-80 },
	 { 0x848000.0p-24,  0xa899ab.0p-24,  0x199ff15ce53266.0p-79 },
	 { 0x840000.0p-24,  0xa99171.0p-24,  0x1a19e15ccc45d2.0p-79 },
	 { 0x838000.0p-24,  0xaa8a28.0p-24, -0x121a14ec532b36.0p-80 },
	 { 0x830000.0p-24,  0xab83d1.0p-24,  0x1aee319980bff3.0p-79 },
	 { 0x828000.0p-24,  0xac7e6f.0p-24, -0x18ffd9e3900346.0p-80 },
	 { 0x820000.0p-24,  0xad7a03.0p-24, -0x1e4db102ce29f8.0p-80 },
	 { 0x818000.0p-24,  0xae768f.0p-24,  0x17c35c55a04a83.0p-81 },
	 { 0x810000.0p-24,  0xaf7415.0p-24,  0x1448324047019b.0p-78 },
	 { 0x808000.0p-24,  0xb07298.0p-24, -0x1750ee3915a198.0p-78 },
	 { 0x800000.0p-24,  0xb17218.0p-24, -0x105c610ca86c39.0p-81 },
};

#ifdef USE_UTAB
static const struct {
	float	H;			/* 1 + i/INTERVALS (exact) */
	float	E;			/* H(i) * G(i) - 1 (exact) */
} U[TSIZE] = {
	 { 0x800000.0p-23,  0 },
	 { 0x810000.0p-23, -0x800000.0p-37 },
	 { 0x820000.0p-23, -0x800000.0p-35 },
	 { 0x830000.0p-23, -0x900000.0p-34 },
	 { 0x840000.0p-23, -0x800000.0p-33 },
	 { 0x850000.0p-23, -0xc80000.0p-33 },
	 { 0x860000.0p-23, -0xa00000.0p-36 },
	 { 0x870000.0p-23,  0x940000.0p-33 },
	 { 0x880000.0p-23,  0x800000.0p-35 },
	 { 0x890000.0p-23, -0xc80000.0p-34 },
	 { 0x8a0000.0p-23,  0xe00000.0p-36 },
	 { 0x8b0000.0p-23,  0x900000.0p-33 },
	 { 0x8c0000.0p-23, -0x800000.0p-35 },
	 { 0x8d0000.0p-23, -0xe00000.0p-33 },
	 { 0x8e0000.0p-23,  0x880000.0p-33 },
	 { 0x8f0000.0p-23, -0xa80000.0p-34 },
	 { 0x900000.0p-23, -0x800000.0p-35 },
	 { 0x910000.0p-23,  0x800000.0p-37 },
	 { 0x920000.0p-23,  0x900000.0p-35 },
	 { 0x930000.0p-23,  0xd00000.0p-35 },
	 { 0x940000.0p-23,  0xe00000.0p-35 },
	 { 0x950000.0p-23,  0xc00000.0p-35 },
	 { 0x960000.0p-23,  0xe00000.0p-36 },
	 { 0x970000.0p-23, -0x800000.0p-38 },
	 { 0x980000.0p-23, -0xc00000.0p-35 },
	 { 0x990000.0p-23, -0xd00000.0p-34 },
	 { 0x9a0000.0p-23,  0x880000.0p-33 },
	 { 0x9b0000.0p-23,  0xe80000.0p-35 },
	 { 0x9c0000.0p-23, -0x800000.0p-35 },
	 { 0x9d0000.0p-23,  0xb40000.0p-33 },
	 { 0x9e0000.0p-23,  0x880000.0p-34 },
	 { 0x9f0000.0p-23, -0xe00000.0p-35 },
	 { 0xa00000.0p-23,  0x800000.0p-33 },
	 { 0xa10000.0p-23, -0x900000.0p-36 },
	 { 0xa20000.0p-23, -0xb00000.0p-33 },
	 { 0xa30000.0p-23, -0xa00000.0p-36 },
	 { 0xa40000.0p-23,  0x800000.0p-33 },
	 { 0xa50000.0p-23, -0xf80000.0p-35 },
	 { 0xa60000.0p-23,  0x880000.0p-34 },
	 { 0xa70000.0p-23, -0x900000.0p-33 },
	 { 0xa80000.0p-23, -0x800000.0p-35 },
	 { 0xa90000.0p-23,  0x900000.0p-34 },
	 { 0xaa0000.0p-23,  0xa80000.0p-33 },
	 { 0xab0000.0p-23, -0xac0000.0p-34 },
	 { 0xac0000.0p-23, -0x800000.0p-37 },
	 { 0xad0000.0p-23,  0xf80000.0p-35 },
	 { 0xae0000.0p-23,  0xf80000.0p-34 },
	 { 0xaf0000.0p-23, -0xac0000.0p-33 },
	 { 0xb00000.0p-23, -0x800000.0p-33 },
	 { 0xb10000.0p-23, -0xb80000.0p-34 },
	 { 0xb20000.0p-23, -0x800000.0p-34 },
	 { 0xb30000.0p-23, -0xb00000.0p-35 },
	 { 0xb40000.0p-23, -0x800000.0p-35 },
	 { 0xb50000.0p-23, -0xe00000.0p-36 },
	 { 0xb60000.0p-23, -0x800000.0p-35 },
	 { 0xb70000.0p-23, -0xb00000.0p-35 },
	 { 0xb80000.0p-23, -0x800000.0p-34 },
	 { 0xb90000.0p-23, -0xb80000.0p-34 },
	 { 0xba0000.0p-23, -0x800000.0p-33 },
	 { 0xbb0000.0p-23, -0xac0000.0p-33 },
	 { 0xbc0000.0p-23,  0x980000.0p-33 },
	 { 0xbd0000.0p-23,  0xbc0000.0p-34 },
	 { 0xbe0000.0p-23,  0xe00000.0p-36 },
	 { 0xbf0000.0p-23, -0xb80000.0p-35 },
	 { 0xc00000.0p-23, -0x800000.0p-33 },
	 { 0xc10000.0p-23,  0xa80000.0p-33 },
	 { 0xc20000.0p-23,  0x900000.0p-34 },
	 { 0xc30000.0p-23, -0x800000.0p-35 },
	 { 0xc40000.0p-23, -0x900000.0p-33 },
	 { 0xc50000.0p-23,  0x820000.0p-33 },
	 { 0xc60000.0p-23,  0x800000.0p-38 },
	 { 0xc70000.0p-23, -0x820000.0p-33 },
	 { 0xc80000.0p-23,  0x800000.0p-33 },
	 { 0xc90000.0p-23, -0xa00000.0p-36 },
	 { 0xca0000.0p-23, -0xb00000.0p-33 },
	 { 0xcb0000.0p-23,  0x840000.0p-34 },
	 { 0xcc0000.0p-23, -0xd00000.0p-34 },
	 { 0xcd0000.0p-23,  0x800000.0p-33 },
	 { 0xce0000.0p-23, -0xe00000.0p-35 },
	 { 0xcf0000.0p-23,  0xa60000.0p-33 },
	 { 0xd00000.0p-23, -0x800000.0p-35 },
	 { 0xd10000.0p-23,  0xb40000.0p-33 },
	 { 0xd20000.0p-23, -0x800000.0p-35 },
	 { 0xd30000.0p-23,  0xaa0000.0p-33 },
	 { 0xd40000.0p-23, -0xe00000.0p-35 },
	 { 0xd50000.0p-23,  0x880000.0p-33 },
	 { 0xd60000.0p-23, -0xd00000.0p-34 },
	 { 0xd70000.0p-23,  0x9c0000.0p-34 },
	 { 0xd80000.0p-23, -0xb00000.0p-33 },
	 { 0xd90000.0p-23, -0x800000.0p-38 },
	 { 0xda0000.0p-23,  0xa40000.0p-33 },
	 { 0xdb0000.0p-23, -0xdc0000.0p-34 },
	 { 0xdc0000.0p-23,  0xc00000.0p-35 },
	 { 0xdd0000.0p-23,  0xca0000.0p-33 },
	 { 0xde0000.0p-23, -0xb80000.0p-34 },
	 { 0xdf0000.0p-23,  0xd00000.0p-35 },
	 { 0xe00000.0p-23,  0xc00000.0p-33 },
	 { 0xe10000.0p-23, -0xf40000.0p-34 },
	 { 0xe20000.0p-23,  0x800000.0p-37 },
	 { 0xe30000.0p-23,  0x860000.0p-33 },
	 { 0xe40000.0p-23, -0xc80000.0p-33 },
	 { 0xe50000.0p-23, -0xa80000.0p-34 },
	 { 0xe60000.0p-23,  0xe00000.0p-36 },
	 { 0xe70000.0p-23,  0x880000.0p-33 },
	 { 0xe80000.0p-23, -0xe00000.0p-33 },
	 { 0xe90000.0p-23, -0xfc0000.0p-34 },
	 { 0xea0000.0p-23, -0x800000.0p-35 },
	 { 0xeb0000.0p-23,  0xe80000.0p-35 },
	 { 0xec0000.0p-23,  0x900000.0p-33 },
	 { 0xed0000.0p-23,  0xe20000.0p-33 },
	 { 0xee0000.0p-23, -0xac0000.0p-33 },
	 { 0xef0000.0p-23, -0xc80000.0p-34 },
	 { 0xf00000.0p-23, -0x800000.0p-35 },
	 { 0xf10000.0p-23,  0x800000.0p-35 },
	 { 0xf20000.0p-23,  0xb80000.0p-34 },
	 { 0xf30000.0p-23,  0x940000.0p-33 },
	 { 0xf40000.0p-23,  0xc80000.0p-33 },
	 { 0xf50000.0p-23, -0xf20000.0p-33 },
	 { 0xf60000.0p-23, -0xc80000.0p-33 },
	 { 0xf70000.0p-23, -0xa20000.0p-33 },
	 { 0xf80000.0p-23, -0x800000.0p-33 },
	 { 0xf90000.0p-23, -0xc40000.0p-34 },
	 { 0xfa0000.0p-23, -0x900000.0p-34 },
	 { 0xfb0000.0p-23, -0xc80000.0p-35 },
	 { 0xfc0000.0p-23, -0x800000.0p-35 },
	 { 0xfd0000.0p-23, -0x900000.0p-36 },
	 { 0xfe0000.0p-23, -0x800000.0p-37 },
	 { 0xff0000.0p-23, -0x800000.0p-39 },
	 { 0x800000.0p-22,  0 },
};
#endif /* USE_UTAB */

#ifdef STRUCT_RETURN
#define	RETURN1(rp, v) do {	\
	(rp)->hi = (v);		\
	(rp)->lo_set = 0;	\
	return;			\
} while (0)

#define	RETURN2(rp, h, l) do {	\
	(rp)->hi = (h);		\
	(rp)->lo = (l);		\
	(rp)->lo_set = 1;	\
	return;			\
} while (0)

struct ld {
	long double hi;
	long double lo;
	int	lo_set;
};
#else
#define	RETURN1(rp, v)	RETURNF(v)
#define	RETURN2(rp, h, l)	RETURNI((h) + (l))
#endif

#ifdef STRUCT_RETURN
static __attribute__((__always_inline__)) inline void
k_logl(long double x, struct ld *rp)
#else
long double
logl(long double x)
#endif
{
	long double d, dk, val_hi, val_lo, z;
	uint64_t ix, lx;
	int i, k;
	uint16_t hx;

	EXTRACT_LDBL80_WORDS(hx, lx, x);
	k = -16383;
#if 0 /* Hard to do efficiently.  Don't do it until we support all modes. */
	if (x == 1)
		RETURN1(rp, 0);		/* log(1) = +0 in all rounding modes */
#endif
	if (hx == 0 || hx >= 0x8000) {	/* zero, negative or subnormal? */
		if (((hx & 0x7fff) | lx) == 0)
			RETURN1(rp, -1 / zero);	/* log(+-0) = -Inf */
		if (hx != 0)
			/* log(neg or [pseudo-]NaN) = qNaN: */
			RETURN1(rp, (x - x) / zero);
		x *= 0x1.0p65;		/* subnormal; scale up x */
					/* including pseudo-subnormals */
		EXTRACT_LDBL80_WORDS(hx, lx, x);
		k = -16383 - 65;
	} else if (hx >= 0x7fff || (lx & 0x8000000000000000ULL) == 0)
		RETURN1(rp, x + x);	/* log(Inf or NaN) = Inf or qNaN */
					/* log(pseudo-Inf) = qNaN */
					/* log(pseudo-NaN) = qNaN */
					/* log(unnormal) = qNaN */
#ifndef STRUCT_RETURN
	ENTERI();
#endif
	k += hx;
	ix = lx & 0x7fffffffffffffffULL;
	dk = k;

	/* Scale x to be in [1, 2). */
	SET_LDBL_EXPSIGN(x, 0x3fff);

	/* 0 <= i <= INTERVALS: */
#define	L2I	(64 - LOG2_INTERVALS)
	i = (ix + (1LL << (L2I - 2))) >> (L2I - 1);

	if (0)
		d = x * G(i) - 1;
	else {
#ifdef USE_UTAB
		d = (x - H(i)) * G(i) + E(i);
#else
		long double x_hi, x_lo;
		float fx_hi;

		/*
		 * Split x into x_hi + x_lo to calculate x*G(i)-1 exactly.
		 * G(i) has at most 9 bits, so the splitting point is not
		 * critical.
		 */
		SET_FLOAT_WORD(fx_hi, (lx >> 40) | 0x3f800000);
		x_hi = fx_hi;
		x_lo = x - x_hi;
		d = x_hi * G(i) - 1 + x_lo * G(i);
#endif
	}

	/*
	 * Our algorithm depends on exact cancellation of F_lo(i) and
	 * F_hi(i) with dk*ln_2_lo and dk*ln2_hi when k is -1 and i is
	 * at the end of the table.
	 */
	z = d * d;
	val_lo = z * d * z * (z * (d * P8 + P7) + (d * P6 + P5)) +
	    (F_lo(i) + dk * ln2_lo + z * d * (d * P4 + P3)) + z * P2;
	val_hi = d;

	_3sumF(val_hi, val_lo, F_hi(i) + dk * ln2_hi);
	RETURN2(rp, val_hi, val_lo);
}

} /* namespace */

export namespace pbsd::lib_msun_ld80::b0095 {

long double
log1pl(long double x)
{
	long double d, d_hi, d_lo, dk, f_lo, val_hi, val_lo, z;
	long double f_hi, twopminusk;
	uint64_t ix, lx;
	int i, k;
	int16_t ax, hx;

	EXTRACT_LDBL80_WORDS(hx, lx, x);
	if (hx < 0x3fff) {		/* x < 1, or x neg NaN */
		ax = hx & 0x7fff;
		if (ax >= 0x3fff) {	/* x <= -1, or x neg NaN */
			if (ax == 0x3fff && lx == 0x8000000000000000ULL)
				RETURNF(-1 / zero);	/* log1p(-1) = -Inf */
			/* log1p(x < 1, or x [pseudo-]NaN) = qNaN: */
			RETURNF((x - x) / (x - x));
		}
		if (ax <= 0x3fbe) {	/* |x| < 2**-64 */
			if ((int)x == 0)
				RETURNF(x);	/* x with inexact if x != 0 */
		}
		f_hi = 1;
		f_lo = x;
	} else if (hx >= 0x7fff) {	/* x +Inf or non-neg NaN */
		RETURNF(x + x);		/* log1p(Inf or NaN) = Inf or qNaN */
					/* log1p(pseudo-Inf) = qNaN */
					/* log1p(pseudo-NaN) = qNaN */
					/* log1p(unnormal) = qNaN */
	} else if (hx < 0x407f) {	/* 1 <= x < 2**128 */
		f_hi = x;
		f_lo = 1;
	} else {			/* 2**128 <= x < +Inf */
		f_hi = x;
		f_lo = 0;		/* avoid underflow of the P5 term */
	}
	ENTERI();
	x = f_hi + f_lo;
	f_lo = (f_hi - x) + f_lo;

	EXTRACT_LDBL80_WORDS(hx, lx, x);
	k = -16383;

	k += hx;
	ix = lx & 0x7fffffffffffffffULL;
	dk = k;

	SET_LDBL_EXPSIGN(x, 0x3fff);
	twopminusk = 1;
	SET_LDBL_EXPSIGN(twopminusk, 0x7ffe - (hx & 0x7fff));
	f_lo *= twopminusk;

	i = (ix + (1LL << (L2I - 2))) >> (L2I - 1);

	/*
	 * x*G(i)-1 (with a reduced x) can be represented exactly, as
	 * above, but now we need to evaluate the polynomial on d =
	 * (x+f_lo)*G(i)-1 and extra precision is needed for that.
	 */
	if (0)
		d_hi = x * G(i) - 1;
	else {
#ifdef USE_UTAB
		d_hi = (x - H(i)) * G(i) + E(i);
#else
		long double x_hi, x_lo;
		float fx_hi;

		SET_FLOAT_WORD(fx_hi, (lx >> 40) | 0x3f800000);
		x_hi = fx_hi;
		x_lo = x - x_hi;
		d_hi = x_hi * G(i) - 1 + x_lo * G(i);
#endif
	}
	d_lo = f_lo * G(i);

	/*
	 * This is _2sumF(d_hi, d_lo) inlined.
	 */
	d = d_hi + d_lo;
	d_lo = d_hi - d + d_lo;
	d_hi = d;

	z = d * d;
	val_lo = z * d * z * (z * (d * P8 + P7) + (d * P6 + P5)) +
	    (F_lo(i) + dk * ln2_lo + d_lo + z * d * (d * P4 + P3)) + z * P2;
	val_hi = d_hi;

	_3sumF(val_hi, val_lo, F_hi(i) + dk * ln2_hi);
	RETURNI(val_hi + val_lo);
}

#ifdef STRUCT_RETURN

long double
logl(long double x)
{
	struct ld r;

	ENTERI();
	k_logl(x, &r);
	RETURNSPI(&r);
}

/* Use macros since GCC < 8 rejects static const expressions in initializers. */
#define	invln10_hi	4.3429448190317999e-1	/*  0x1bcb7b1526e000.0p-54 */
#define	invln10_lo	7.1842412889749798e-14	/*  0x1438ca9aadd558.0p-96 */
#define	invln2_hi	1.4426950408887933e0	/*  0x171547652b8000.0p-52 */
#define	invln2_lo	1.7010652264631490e-13	/*  0x17f0bbbe87fed0.0p-95 */

} /* export namespace */

namespace pbsd::lib_msun_ld80::b0095 {
/* Let the compiler pre-calculate this sum to avoid FE_INEXACT at run time. */
static const double invln10_lo_plus_hi = invln10_lo + invln10_hi;
static const double invln2_lo_plus_hi = invln2_lo + invln2_hi;
}

export namespace pbsd::lib_msun_ld80::b0095 {

long double
log10l(long double x)
{
	struct ld r;
	long double hi, lo;

	ENTERI();
	k_logl(x, &r);
	if (!r.lo_set)
		RETURNI(r.hi);
	_2sumF(r.hi, r.lo);
	hi = (float)r.hi;
	lo = r.lo + (r.hi - hi);
	RETURNI(invln10_hi * hi + 
	    (invln10_lo_plus_hi * lo + invln10_lo * hi));
}

long double
log2l(long double x)
{
	struct ld r;
	long double hi, lo;

	ENTERI();
	k_logl(x, &r);
	if (!r.lo_set)
		RETURNI(r.hi);
	_2sumF(r.hi, r.lo);
	hi = (float)r.hi;
	lo = r.lo + (r.hi - hi);
	RETURNI(invln2_hi * hi +
	    (invln2_lo_plus_hi * lo + invln2_lo * hi));
}

#endif /* STRUCT_RETURN */

} /* export namespace */
