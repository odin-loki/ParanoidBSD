/*
 * oracle.c -- batch b0082 reference implementation.
 *
 * The original HardenedBSD C sources concatenated verbatim, with every
 * externally visible object renamed with a "ref_" prefix so the port and the
 * reference can be linked into a single differential-test binary.  No function
 * body has been modified.
 *
 * Concatenated sources, in order:
 *   lib/msun/ld80/k_sinl.c
 *   lib/msun/ld80/invtrig.c
 *   lib/msun/ld80/k_cosl.c
 *
 * The only non-rename edits are the removal of the #include lines for the
 * private msun headers ("math_private.h", "invtrig.h"), which merely supply
 * prototypes and are not self-contained outside the msun build; nothing any
 * of the bodies below actually references comes from them.
 *
 * lib/msun/ld80/s_nanl.c is not represented here; see skipped.txt.
 */

/* ================================================================== */
/* lib/msun/ld80/k_sinl.c					      */
/* ================================================================== */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 * Copyright (c) 2008 Steven G. Kargl, David Schultz, Bruce D. Evans.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

/*
 * ld80 version of k_sin.c.  See ../src/k_sin.c for most comments.
 */

/* #include "math_private.h" */

static const double
half =  0.5;

/*
 * Domain [-0.7854, 0.7854], range ~[-1.89e-22, 1.915e-22]
 * |sin(x)/x - s(x)| < 2**-72.1
 *
 * See ../ld80/k_cosl.c for more details about the polynomial.
 */
#if defined(__amd64__) || defined(__i386__)
/* Long double constants are slow on these arches, and broken on i386. */
static const volatile double
S1hi = -0.16666666666666666,		/* -0x15555555555555.0p-55 */
S1lo = -9.2563760475949941e-18;		/* -0x15580000000000.0p-109 */
#define	S1	((long double)S1hi + S1lo)
#else
static const long double
S1 = -0.166666666666666666671L;		/* -0xaaaaaaaaaaaaaaab.0p-66 */
#endif

static const double
S2 =  0.0083333333333333332,		/*  0x11111111111111.0p-59 */
S3 = -0.00019841269841269427,		/* -0x1a01a01a019f81.0p-65 */
S4 =  0.0000027557319223597490,		/*  0x171de3a55560f7.0p-71 */
S5 = -0.000000025052108218074604,	/* -0x1ae64564f16cad.0p-78 */
S6 =  1.6059006598854211e-10,		/*  0x161242b90243b5.0p-85 */
S7 = -7.6429779983024564e-13,		/* -0x1ae42ebd1b2e00.0p-93 */
S8 =  2.6174587166648325e-15;		/*  0x179372ea0b3f64.0p-101 */

long double
ref___kernel_sinl(long double x, long double y, int iy)
{
	long double z,r,v;

	z	=  x*x;
	v	=  z*x;
	r	=  S2+z*(S3+z*(S4+z*(S5+z*(S6+z*(S7+z*S8)))));
	if(iy==0) return x+v*(S1+z*r);
	else      return x-((z*(half*y-v*r)-y)-v*S1);
}

/* ================================================================== */
/* lib/msun/ld80/invtrig.c					      */
/* ================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2008 David Schultz <das@FreeBSD.ORG>
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

/* #include "invtrig.h" */

/*
 * asinl() and acosl()
 */
const long double
ref_pS0 =  1.66666666666666666631e-01L,
ref_pS1 = -4.16313987993683104320e-01L,
ref_pS2 =  3.69068046323246813704e-01L,
ref_pS3 = -1.36213932016738603108e-01L,
ref_pS4 =  1.78324189708471965733e-02L,
ref_pS5 = -2.19216428382605211588e-04L,
ref_pS6 = -7.10526623669075243183e-06L,
ref_qS1 = -2.94788392796209867269e+00L,
ref_qS2 =  3.27309890266528636716e+00L,
ref_qS3 = -1.68285799854822427013e+00L,
ref_qS4 =  3.90699412641738801874e-01L,
ref_qS5 = -3.14365703596053263322e-02L;

/*
 * atanl()
 */
const long double ref_atanhi[] = {
	 4.63647609000806116202e-01L,
	 7.85398163397448309628e-01L,
	 9.82793723247329067960e-01L,
	 1.57079632679489661926e+00L,
};

const long double ref_atanlo[] = {
	 1.18469937025062860669e-20L,
	-1.25413940316708300586e-20L,
	 2.55232234165405176172e-20L,
	-2.50827880633416601173e-20L,
};

const long double ref_aT[] = {
	 3.33333333333333333017e-01L,
	-1.99999999999999632011e-01L,
	 1.42857142857046531280e-01L,
	-1.11111111100562372733e-01L,
	 9.09090902935647302252e-02L,
	-7.69230552476207730353e-02L,
	 6.66661718042406260546e-02L,
	-5.88158892835030888692e-02L,
	 5.25499891539726639379e-02L,
	-4.70119845393155721494e-02L,
	 4.03539201366454414072e-02L,
	-2.91303858419364158725e-02L,
	 1.24822046299269234080e-02L,
};

const long double ref_pi_lo = -5.01655761266833202345e-20L;

/*
 * Element counts of the tables above, exported so the differential test can
 * check the port's table lengths against the reference's.  These are additions
 * to the original file; no original declaration was altered.
 */
const int ref_atanhi_len = (int)(sizeof(ref_atanhi) / sizeof(ref_atanhi[0]));
const int ref_atanlo_len = (int)(sizeof(ref_atanlo) / sizeof(ref_atanlo[0]));
const int ref_aT_len = (int)(sizeof(ref_aT) / sizeof(ref_aT[0]));

/* ================================================================== */
/* lib/msun/ld80/k_cosl.c					      */
/* ================================================================== */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 * Copyright (c) 2008 Steven G. Kargl, David Schultz, Bruce D. Evans.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

/*
 * ld80 version of k_cos.c.  See ../src/k_cos.c for most comments.
 */

/* #include "math_private.h" */

/*
 * Domain [-0.7854, 0.7854], range ~[-2.43e-23, 2.425e-23]:
 * |cos(x) - c(x)| < 2**-75.1
 *
 * The coefficients of c(x) were generated by a pari-gp script using
 * a Remez algorithm that searches for the best higher coefficients
 * after rounding leading coefficients to a specified precision.
 *
 * Simpler methods like Chebyshev or basic Remez barely suffice for
 * cos() in 64-bit precision, because we want the coefficient of x^2
 * to be precisely -0.5 so that multiplying by it is exact, and plain
 * rounding of the coefficients of a good polynomial approximation only
 * gives this up to about 64-bit precision.  Plain rounding also gives
 * a mediocre approximation for the coefficient of x^4, but a rounding
 * error of 0.5 ulps for this coefficient would only contribute ~0.01
 * ulps to the final error, so this is unimportant.  Rounding errors in
 * higher coefficients are even less important.
 *
 * In fact, coefficients above the x^4 one only need to have 53-bit
 * precision, and this is more efficient.  We get this optimization
 * almost for free from the complications needed to search for the best
 * higher coefficients.
 */
static const double
one = 1.0;

#if defined(__amd64__) || defined(__i386__)
/* Long double constants are slow on these arches, and broken on i386. */
static const volatile double
C1hi = 0.041666666666666664,		/*  0x15555555555555.0p-57 */
C1lo = 2.2598839032744733e-18;		/*  0x14d80000000000.0p-111 */
#define	C1	((long double)C1hi + C1lo)
#else
static const long double
C1 =  0.0416666666666666666136L;	/*  0xaaaaaaaaaaaaaa9b.0p-68 */
#endif

static const double
C2 = -0.0013888888888888874,		/* -0x16c16c16c16c10.0p-62 */
C3 =  0.000024801587301571716,		/*  0x1a01a01a018e22.0p-68 */
C4 = -0.00000027557319215507120,	/* -0x127e4fb7602f22.0p-74 */
C5 =  0.0000000020876754400407278,	/*  0x11eed8caaeccf1.0p-81 */
C6 = -1.1470297442401303e-11,		/* -0x19393412bd1529.0p-89 */
C7 =  4.7383039476436467e-14;		/*  0x1aac9d9af5c43e.0p-97 */

long double
ref___kernel_cosl(long double x, long double y)
{
	long double hz,z,r,w;

	z  = x*x;
	r  = z*(C1+z*(C2+z*(C3+z*(C4+z*(C5+z*(C6+z*C7))))));
	hz = 0.5*z;
	w  = one-hz;
	return w + (((one-w)-hz) + (z*r-x*y));
}
