/*
 * Reference oracle for PBSD batch b0084.
 *
 * Original HardenedBSD lib/msun/ld128 sources concatenated; every function
 * renamed with a ref_ prefix.  Bodies are otherwise unmodified.
 */

#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef __always_inline
#define __always_inline __inline__
#endif

union IEEEl2bits {
	long double e;
	struct {
		unsigned long manl : 64;
		unsigned long manh : 48;
		unsigned int exp : 15;
		unsigned int sign : 1;
	} bits;
	struct {
		unsigned long manl : 64;
		unsigned long manh : 48;
		unsigned int expsign : 16;
	} xbits;
};

#define EXTRACT_LDBL128_WORDS(ix0, ix1, ix2, d) do { \
	union IEEEl2bits ew_u; \
	ew_u.e = (d); \
	(ix0) = ew_u.xbits.expsign; \
	(ix1) = ew_u.xbits.manh; \
	(ix2) = ew_u.xbits.manl; \
} while (0)

#define INSERT_LDBL128_WORDS(d, ix0, ix1, ix2) do { \
	union IEEEl2bits iw_u; \
	iw_u.xbits.expsign = (ix0); \
	iw_u.xbits.manh = (ix1); \
	iw_u.xbits.manl = (ix2); \
	(d) = iw_u.e; \
} while (0)

#define SET_LDBL_EXPSIGN(d, v) do { \
	union IEEEl2bits se_u; \
	se_u.e = (d); \
	se_u.xbits.expsign = (v); \
	(d) = se_u.e; \
} while (0)

#define GET_FLOAT_WORD(i,d) do { \
	union { float value; unsigned int word; } gf_u; \
	gf_u.value = (d); \
	(i) = gf_u.word; \
} while (0)

#define SET_FLOAT_WORD(d,i) do { \
	union { float value; unsigned int word; } sf_u; \
	sf_u.word = (i); \
	(d) = sf_u.value; \
} while (0)

#define ENTERI()
#define RETURNF(v) return (v)
#define RETURNI(x) return (x)

#define SUM2P(x, y) ({ \
	const __typeof__(x) __x = (x); \
	const __typeof__(y) __y = (y); \
	__x + __y; \
})

static inline double rnint(double x)
{
	return ((double)(x + 0x1.8p52) - 0x1.8p52);
}

#define irint(x) ((int)(x))

/* ===== k_sinl.c (support) ===== */
#define half __b0084_ksin_half
#define S1 __b0084_ksin_S1
#define S2 __b0084_ksin_S2
#define S3 __b0084_ksin_S3
#define S4 __b0084_ksin_S4
#define S5 __b0084_ksin_S5
#define S6 __b0084_ksin_S6
#define S7 __b0084_ksin_S7
#define S8 __b0084_ksin_S8
#define S9 __b0084_ksin_S9
#define S10 __b0084_ksin_S10
#define S11 __b0084_ksin_S11
#define S12 __b0084_ksin_S12
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
 * ld128 version of k_sin.c.  See ../src/k_sin.c for most comments.
 */


static const double
half =  0.5;

/*
 * Domain [-0.7854, 0.7854], range ~[-1.53e-37, 1.659e-37]
 * |sin(x)/x - s(x)| < 2**-122.1
 *
 * See ../ld80/k_cosl.c for more details about the polynomial.
 */
static const long double
S1 = -0.16666666666666666666666666666666666606732416116558L,
S2 =  0.0083333333333333333333333333333331135404851288270047L,
S3 = -0.00019841269841269841269841269839935785325638310428717L,
S4 =  0.27557319223985890652557316053039946268333231205686e-5L,
S5 = -0.25052108385441718775048214826384312253862930064745e-7L,
S6 =  0.16059043836821614596571832194524392581082444805729e-9L,
S7 = -0.76471637318198151807063387954939213287488216303768e-12L,
S8 =  0.28114572543451292625024967174638477283187397621303e-14L;

static const double
S9  = -0.82206352458348947812512122163446202498005154296863e-17,
S10 =  0.19572940011906109418080609928334380560135358385256e-19,
S11 = -0.38680813379701966970673724299207480965452616911420e-22,
S12 =  0.64038150078671872796678569586315881020659912139412e-25;

long double
__kernel_sinl(long double x, long double y, int iy)
{
	long double z,r,v;

	z	=  x*x;
	v	=  z*x;
	r	=  S2+z*(S3+z*(S4+z*(S5+z*(S6+z*(S7+z*(S8+
	    z*(S9+z*(S10+z*(S11+z*S12)))))))));
	if(iy==0) return x+v*(S1+z*r);
	else      return x-((z*(half*y-v*r)-y)-v*S1);
}
#undef half
#undef S1
#undef S2
#undef S3
#undef S4
#undef S5
#undef S6
#undef S7
#undef S8
#undef S9
#undef S10
#undef S11
#undef S12

/* ===== k_cosl.c (support) ===== */
#define one __b0084_kcos_one
#define C1 __b0084_kcos_C1
#define C2 __b0084_kcos_C2
#define C3 __b0084_kcos_C3
#define C4 __b0084_kcos_C4
#define C5 __b0084_kcos_C5
#define C6 __b0084_kcos_C6
#define C7 __b0084_kcos_C7
#define C8 __b0084_kcos_C8
#define C9 __b0084_kcos_C9
#define C10 __b0084_kcos_C10
#define C11 __b0084_kcos_C11
#define C12 __b0084_kcos_C12
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
 * ld128 version of k_cos.c.  See ../src/k_cos.c for most comments.
 */


/*
 * Domain [-0.7854, 0.7854], range ~[-1.17e-39, 1.19e-39]:
 * |cos(x) - c(x))| < 2**-129.3
 *
 * 113-bit precision requires more care than 64-bit precision, since
 * simple methods give a minimax polynomial with coefficient for x^2
 * that is 1 ulp below 0.5, but we want it to be precisely 0.5.  See
 * ../ld80/k_cosl.c for more details.
 */
static const double
one = 1.0;
static const long double
C1 =  4.16666666666666666666666666666666667e-02L,
C2 = -1.38888888888888888888888888888888834e-03L,
C3 =  2.48015873015873015873015873015446795e-05L,
C4 = -2.75573192239858906525573190949988493e-07L,
C5 =  2.08767569878680989792098886701451072e-09L,
C6 = -1.14707455977297247136657111139971865e-11L,
C7 =  4.77947733238738518870113294139830239e-14L,
C8 = -1.56192069685858079920640872925306403e-16L,
C9 =  4.11031762320473354032038893429515732e-19L,
C10= -8.89679121027589608738005163931958096e-22L,
C11=  1.61171797801314301767074036661901531e-24L,
C12= -2.46748624357670948912574279501044295e-27L;

long double
__kernel_cosl(long double x, long double y)
{
	long double hz,z,r,w;

	z  = x*x;
	r  = z*(C1+z*(C2+z*(C3+z*(C4+z*(C5+z*(C6+z*(C7+
	    z*(C8+z*(C9+z*(C10+z*(C11+z*C12)))))))))));
	hz = 0.5*z;
	w  = one-hz;
	return w + (((one-w)-hz) + (z*r-x*y));
}
#undef one
#undef C1
#undef C2
#undef C3
#undef C4
#undef C5
#undef C6
#undef C7
#undef C8
#undef C9
#undef C10
#undef C11
#undef C12

/* ===== k_expl.h (support) ===== */
/* from: FreeBSD: head/lib/msun/ld128/s_expl.c 251345 2013-06-03 20:09:22Z kargl */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2009-2013 Steven G. Kargl
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
 *
 * Optimized by Bruce D. Evans.
 */

/*
 * ld128 version of k_expl.h.  See ../ld80/s_expl.c for most comments.
 *
 * See ../src/e_exp.c and ../src/k_exp.h for precision-independent comments
 * about the secondary kernels.
 */

#define	INTERVALS	128
#define	LOG2_INTERVALS	7
#define	BIAS	(LDBL_MAX_EXP - 1)

static const double
/*
 * ln2/INTERVALS = L1+L2 (hi+lo decomposition for multiplication).  L1 must
 * have at least 22 (= log2(|LDBL_MIN_EXP-extras|) + log2(INTERVALS)) lowest
 * bits zero so that multiplication of it by n is exact.
 */
INV_L = 1.8466496523378731e+2,		/*  0x171547652b82fe.0p-45 */
L2 = -1.0253670638894731e-29;		/* -0x1.9ff0342542fc3p-97 */
static const long double
/* 0x1.62e42fefa39ef35793c768000000p-8 */
L1 =  5.41521234812457272982212595914567508e-3L;

/*
 * XXX values in hex in comments have been lost (or were never present)
 * from here.
 */
static const long double
/*
 * Domain [-0.002708, 0.002708], range ~[-2.4021e-38, 2.4234e-38]:
 * |exp(x) - p(x)| < 2**-124.9
 * (0.002708 is ln2/(2*INTERVALS) rounded up a little).
 *
 * XXX the coeffs aren't very carefully rounded, and I get 3.6 more bits.
 */
A2  =  0.5,
A3  =  1.66666666666666666666666666651085500e-1L,
A4  =  4.16666666666666666666666666425885320e-2L,
A5  =  8.33333333333333333334522877160175842e-3L,
A6  =  1.38888888888888888889971139751596836e-3L;

static const double
A7  =  1.9841269841269470e-4,		/*  0x1.a01a01a019f91p-13 */
A8  =  2.4801587301585286e-5,		/*  0x1.71de3ec75a967p-19 */
A9  =  2.7557324277411235e-6,		/*  0x1.71de3ec75a967p-19 */
A10 =  2.7557333722375069e-7;		/*  0x1.27e505ab56259p-22 */

static const struct {
	/*
	 * hi must be rounded to at most 106 bits so that multiplication
	 * by r1 in expm1l() is exact, but it is rounded to 88 bits due to
	 * historical accidents.
	 *
	 * XXX it is wasteful to use long double for both hi and lo.  ld128
	 * exp2l() uses only float for lo (in a very differently organized
	 * table; ld80 exp2l() is different again.  It uses 2 doubles in a
	 * table organized like this one.  1 double and 1 float would
	 * suffice).  There are different packing/locality/alignment/caching
	 * problems with these methods.
	 *
	 * XXX C's bad %a format makes the bits unreadable.  They happen
	 * to all line up for the hi values 1 before the point and 88
	 * in 22 nybbles, but for the low values the nybbles are shifted
	 * randomly.
	 */
	long double	hi;
	long double	lo;
} tbl[INTERVALS] = {
	0x1p0L, 0x0p0L,
	0x1.0163da9fb33356d84a66aep0L, 0x3.36dcdfa4003ec04c360be2404078p-92L,
	0x1.02c9a3e778060ee6f7cacap0L, 0x4.f7a29bde93d70a2cabc5cb89ba10p-92L,
	0x1.04315e86e7f84bd738f9a2p0L, 0xd.a47e6ed040bb4bfc05af6455e9b8p-96L,
	0x1.059b0d31585743ae7c548ep0L, 0xb.68ca417fe53e3495f7df4baf84a0p-92L,
	0x1.0706b29ddf6ddc6dc403a8p0L, 0x1.d87b27ed07cb8b092ac75e311753p-88L,
	0x1.0874518759bc808c35f25cp0L, 0x1.9427fa2b041b2d6829d8993a0d01p-88L,
	0x1.09e3ecac6f3834521e060cp0L, 0x5.84d6b74ba2e023da730e7fccb758p-92L,
	0x1.0b5586cf9890f6298b92b6p0L, 0x1.1842a98364291408b3ceb0a2a2bbp-88L,
	0x1.0cc922b7247f7407b705b8p0L, 0x9.3dc5e8aac564e6fe2ef1d431fd98p-92L,
	0x1.0e3ec32d3d1a2020742e4ep0L, 0x1.8af6a552ac4b358b1129e9f966a4p-88L,
	0x1.0fb66affed31af232091dcp0L, 0x1.8a1426514e0b627bda694a400a27p-88L,
	0x1.11301d0125b50a4ebbf1aep0L, 0xd.9318ceac5cc47ab166ee57427178p-92L,
	0x1.12abdc06c31cbfb92bad32p0L, 0x4.d68e2f7270bdf7cedf94eb1cb818p-92L,
	0x1.1429aaea92ddfb34101942p0L, 0x1.b2586d01844b389bea7aedd221d4p-88L,
	0x1.15a98c8a58e512480d573cp0L, 0x1.d5613bf92a2b618ee31b376c2689p-88L,
	0x1.172b83c7d517adcdf7c8c4p0L, 0x1.0eb14a792035509ff7d758693f24p-88L,
	0x1.18af9388c8de9bbbf70b9ap0L, 0x3.c2505c97c0102e5f1211941d2840p-92L,
	0x1.1a35beb6fcb753cb698f68p0L, 0x1.2d1c835a6c30724d5cfae31b84e5p-88L,
	0x1.1bbe084045cd39ab1e72b4p0L, 0x4.27e35f9acb57e473915519a1b448p-92L,
	0x1.1d4873168b9aa7805b8028p0L, 0x9.90f07a98b42206e46166cf051d70p-92L,
	0x1.1ed5022fcd91cb8819ff60p0L, 0x1.121d1e504d36c47474c9b7de6067p-88L,
	0x1.2063b88628cd63b8eeb028p0L, 0x1.50929d0fc487d21c2b84004264dep-88L,
	0x1.21f49917ddc962552fd292p0L, 0x9.4bdb4b61ea62477caa1dce823ba0p-92L,
	0x1.2387a6e75623866c1fadb0p0L, 0x1.c15cb593b0328566902df69e4de2p-88L,
	0x1.251ce4fb2a63f3582ab7dep0L, 0x9.e94811a9c8afdcf796934bc652d0p-92L,
	0x1.26b4565e27cdd257a67328p0L, 0x1.d3b249dce4e9186ddd5ff44e6b08p-92L,
	0x1.284dfe1f5638096cf15cf0p0L, 0x3.ca0967fdaa2e52d7c8106f2e262cp-92L,
	0x1.29e9df51fdee12c25d15f4p0L, 0x1.a24aa3bca890ac08d203fed80a07p-88L,
	0x1.2b87fd0dad98ffddea4652p0L, 0x1.8fcab88442fdc3cb6de4519165edp-88L,
	0x1.2d285a6e4030b40091d536p0L, 0xd.075384589c1cd1b3e4018a6b1348p-92L,
	0x1.2ecafa93e2f5611ca0f45cp0L, 0x1.523833af611bdcda253c554cf278p-88L,
	0x1.306fe0a31b7152de8d5a46p0L, 0x3.05c85edecbc27343629f502f1af2p-92L,
	0x1.32170fc4cd8313539cf1c2p0L, 0x1.008f86dde3220ae17a005b6412bep-88L,
	0x1.33c08b26416ff4c9c8610cp0L, 0x1.96696bf95d1593039539d94d662bp-88L,
	0x1.356c55f929ff0c94623476p0L, 0x3.73af38d6d8d6f9506c9bbc93cbc0p-92L,
	0x1.371a7373aa9caa7145502ep0L, 0x1.4547987e3e12516bf9c699be432fp-88L,
	0x1.38cae6d05d86585a9cb0d8p0L, 0x1.bed0c853bd30a02790931eb2e8f0p-88L,
	0x1.3a7db34e59ff6ea1bc9298p0L, 0x1.e0a1d336163fe2f852ceeb134067p-88L,
	0x1.3c32dc313a8e484001f228p0L, 0xb.58f3775e06ab66353001fae9fca0p-92L,
	0x1.3dea64c12342235b41223ep0L, 0x1.3d773fba2cb82b8244267c54443fp-92L,
	0x1.3fa4504ac801ba0bf701aap0L, 0x4.1832fb8c1c8dbdff2c49909e6c60p-92L,
	0x1.4160a21f72e29f84325b8ep0L, 0x1.3db61fb352f0540e6ba05634413ep-88L,
	0x1.431f5d950a896dc7044394p0L, 0x1.0ccec81e24b0caff7581ef4127f7p-92L,
	0x1.44e086061892d03136f408p0L, 0x1.df019fbd4f3b48709b78591d5cb5p-88L,
	0x1.46a41ed1d005772512f458p0L, 0x1.229d97df404ff21f39c1b594d3a8p-88L,
	0x1.486a2b5c13cd013c1a3b68p0L, 0x1.062f03c3dd75ce8757f780e6ec99p-88L,
	0x1.4a32af0d7d3de672d8bcf4p0L, 0x6.f9586461db1d878b1d148bd3ccb8p-92L,
	0x1.4bfdad5362a271d4397afep0L, 0xc.42e20e0363ba2e159c579f82e4b0p-92L,
	0x1.4dcb299fddd0d63b36ef1ap0L, 0x9.e0cc484b25a5566d0bd5f58ad238p-92L,
	0x1.4f9b2769d2ca6ad33d8b68p0L, 0x1.aa073ee55e028497a329a7333dbap-88L,
	0x1.516daa2cf6641c112f52c8p0L, 0x4.d822190e718226177d7608d20038p-92L,
	0x1.5342b569d4f81df0a83c48p0L, 0x1.d86a63f4e672a3e429805b049465p-88L,
	0x1.551a4ca5d920ec52ec6202p0L, 0x4.34ca672645dc6c124d6619a87574p-92L,
	0x1.56f4736b527da66ecb0046p0L, 0x1.64eb3c00f2f5ab3d801d7cc7272dp-88L,
	0x1.58d12d497c7fd252bc2b72p0L, 0x1.43bcf2ec936a970d9cc266f0072fp-88L,
	0x1.5ab07dd48542958c930150p0L, 0x1.91eb345d88d7c81280e069fbdb63p-88L,
	0x1.5c9268a5946b701c4b1b80p0L, 0x1.6986a203d84e6a4a92f179e71889p-88L,
	0x1.5e76f15ad21486e9be4c20p0L, 0x3.99766a06548a05829e853bdb2b52p-92L,
	0x1.605e1b976dc08b076f592ap0L, 0x4.86e3b34ead1b4769df867b9c89ccp-92L,
	0x1.6247eb03a5584b1f0fa06ep0L, 0x1.d2da42bb1ceaf9f732275b8aef30p-88L,
	0x1.6434634ccc31fc76f8714cp0L, 0x4.ed9a4e41000307103a18cf7a6e08p-92L,
	0x1.66238825522249127d9e28p0L, 0x1.b8f314a337f4dc0a3adf1787ff74p-88L,
	0x1.68155d44ca973081c57226p0L, 0x1.b9f32706bfe4e627d809a85dcc66p-88L,
	0x1.6a09e667f3bcc908b2fb12p0L, 0x1.66ea957d3e3adec17512775099dap-88L,
	0x1.6c012750bdabeed76a9980p0L, 0xf.4f33fdeb8b0ecd831106f57b3d00p-96L,
	0x1.6dfb23c651a2ef220e2cbep0L, 0x1.bbaa834b3f11577ceefbe6c1c411p-92L,
	0x1.6ff7df9519483cf87e1b4ep0L, 0x1.3e213bff9b702d5aa477c12523cep-88L,
	0x1.71f75e8ec5f73dd2370f2ep0L, 0xf.0acd6cb434b562d9e8a20adda648p-92L,
	0x1.73f9a48a58173bd5c9a4e6p0L, 0x8.ab1182ae217f3a7681759553e840p-92L,
	0x1.75feb564267c8bf6e9aa32p0L, 0x1.a48b27071805e61a17b954a2dad8p-88L,
	0x1.780694fde5d3f619ae0280p0L, 0x8.58b2bb2bdcf86cd08e35fb04c0f0p-92L,
	0x1.7a11473eb0186d7d51023ep0L, 0x1.6cda1f5ef42b66977960531e821bp-88L,
	0x1.7c1ed0130c1327c4933444p0L, 0x1.937562b2dc933d44fc828efd4c9cp-88L,
	0x1.7e2f336cf4e62105d02ba0p0L, 0x1.5797e170a1427f8fcdf5f3906108p-88L,
	0x1.80427543e1a11b60de6764p0L, 0x9.a354ea706b8e4d8b718a672bf7c8p-92L,
	0x1.82589994cce128acf88afap0L, 0xb.34a010f6ad65cbbac0f532d39be0p-92L,
	0x1.8471a4623c7acce52f6b96p0L, 0x1.c64095370f51f48817914dd78665p-88L,
	0x1.868d99b4492ec80e41d90ap0L, 0xc.251707484d73f136fb5779656b70p-92L,
	0x1.88ac7d98a669966530bcdep0L, 0x1.2d4e9d61283ef385de170ab20f96p-88L,
	0x1.8ace5422aa0db5ba7c55a0p0L, 0x1.92c9bb3e6ed61f2733304a346d8fp-88L,
	0x1.8cf3216b5448bef2aa1cd0p0L, 0x1.61c55d84a9848f8c453b3ca8c946p-88L,
	0x1.8f1ae991577362b982745cp0L, 0x7.2ed804efc9b4ae1458ae946099d4p-92L,
	0x1.9145b0b91ffc588a61b468p0L, 0x1.f6b70e01c2a90229a4c4309ea719p-88L,
	0x1.93737b0cdc5e4f4501c3f2p0L, 0x5.40a22d2fc4af581b63e8326efe9cp-92L,
	0x1.95a44cbc8520ee9b483694p0L, 0x1.a0fc6f7c7d61b2b3a22a0eab2cadp-88L,
	0x1.97d829fde4e4f8b9e920f8p0L, 0x1.1e8bd7edb9d7144b6f6818084cc7p-88L,
	0x1.9a0f170ca07b9ba3109b8cp0L, 0x4.6737beb19e1eada6825d3c557428p-92L,
	0x1.9c49182a3f0901c7c46b06p0L, 0x1.1f2be58ddade50c217186c90b457p-88L,
	0x1.9e86319e323231824ca78ep0L, 0x6.4c6e010f92c082bbadfaf605cfd4p-92L,
	0x1.a0c667b5de564b29ada8b8p0L, 0xc.ab349aa0422a8da7d4512edac548p-92L,
	0x1.a309bec4a2d3358c171f76p0L, 0x1.0daad547fa22c26d168ea762d854p-88L,
	0x1.a5503b23e255c8b424491cp0L, 0xa.f87bc8050a405381703ef7caff50p-92L,
	0x1.a799e1330b3586f2dfb2b0p0L, 0x1.58f1a98796ce8908ae852236ca94p-88L,
	0x1.a9e6b5579fdbf43eb243bcp0L, 0x1.ff4c4c58b571cf465caf07b4b9f5p-88L,
	0x1.ac36bbfd3f379c0db966a2p0L, 0x1.1265fc73e480712d20f8597a8e7bp-88L,
	0x1.ae89f995ad3ad5e8734d16p0L, 0x1.73205a7fbc3ae675ea440b162d6cp-88L,
	0x1.b0e07298db66590842acdep0L, 0x1.c6f6ca0e5dcae2aafffa7a0554cbp-88L,
	0x1.b33a2b84f15faf6bfd0e7ap0L, 0x1.d947c2575781dbb49b1237c87b6ep-88L,
	0x1.b59728de559398e3881110p0L, 0x1.64873c7171fefc410416be0a6525p-88L,
	0x1.b7f76f2fb5e46eaa7b081ap0L, 0xb.53c5354c8903c356e4b625aacc28p-92L,
	0x1.ba5b030a10649840cb3c6ap0L, 0xf.5b47f297203757e1cc6eadc8bad0p-92L,
	0x1.bcc1e904bc1d2247ba0f44p0L, 0x1.b3d08cd0b20287092bd59be4ad98p-88L,
	0x1.bf2c25bd71e088408d7024p0L, 0x1.18e3449fa073b356766dfb568ff4p-88L,
	0x1.c199bdd85529c2220cb12ap0L, 0x9.1ba6679444964a36661240043970p-96L,
	0x1.c40ab5fffd07a6d14df820p0L, 0xf.1828a5366fd387a7bdd54cdf7300p-92L,
	0x1.c67f12e57d14b4a2137fd2p0L, 0xf.2b301dd9e6b151a6d1f9d5d5f520p-96L,
	0x1.c8f6d9406e7b511acbc488p0L, 0x5.c442ddb55820171f319d9e5076a8p-96L,
	0x1.cb720dcef90691503cbd1ep0L, 0x9.49db761d9559ac0cb6dd3ed599e0p-92L,
	0x1.cdf0b555dc3f9c44f8958ep0L, 0x1.ac51be515f8c58bdfb6f5740a3a4p-88L,
	0x1.d072d4a07897b8d0f22f20p0L, 0x1.a158e18fbbfc625f09f4cca40874p-88L,
	0x1.d2f87080d89f18ade12398p0L, 0x9.ea2025b4c56553f5cdee4c924728p-92L,
	0x1.d5818dcfba48725da05aeap0L, 0x1.66e0dca9f589f559c0876ff23830p-88L,
	0x1.d80e316c98397bb84f9d04p0L, 0x8.805f84bec614de269900ddf98d28p-92L,
	0x1.da9e603db3285708c01a5ap0L, 0x1.6d4c97f6246f0ec614ec95c99392p-88L,
	0x1.dd321f301b4604b695de3cp0L, 0x6.30a393215299e30d4fb73503c348p-96L,
	0x1.dfc97337b9b5eb968cac38p0L, 0x1.ed291b7225a944efd5bb5524b927p-88L,
	0x1.e264614f5a128a12761fa0p0L, 0x1.7ada6467e77f73bf65e04c95e29dp-88L,
	0x1.e502ee78b3ff6273d13014p0L, 0x1.3991e8f49659e1693be17ae1d2f9p-88L,
	0x1.e7a51fbc74c834b548b282p0L, 0x1.23786758a84f4956354634a416cep-88L,
	0x1.ea4afa2a490d9858f73a18p0L, 0xf.5db301f86dea20610ceee13eb7b8p-92L,
	0x1.ecf482d8e67f08db0312fap0L, 0x1.949cef462010bb4bc4ce72a900dfp-88L,
	0x1.efa1bee615a27771fd21a8p0L, 0x1.2dac1f6dd5d229ff68e46f27e3dfp-88L,
	0x1.f252b376bba974e8696fc2p0L, 0x1.6390d4c6ad5476b5162f40e1d9a9p-88L,
	0x1.f50765b6e4540674f84b76p0L, 0x2.862baff99000dfc4352ba29b8908p-92L,
	0x1.f7bfdad9cbe138913b4bfep0L, 0x7.2bd95c5ce7280fa4d2344a3f5618p-92L,
	0x1.fa7c1819e90d82e90a7e74p0L, 0xb.263c1dc060c36f7650b4c0f233a8p-92L,
	0x1.fd3c22b8f71f10975ba4b2p0L, 0x1.2bcf3a5e12d269d8ad7c1a4a8875p-88L
};

/*
 * Kernel for expl(x).  x must be finite and not tiny or huge.
 * "tiny" is anything that would make us underflow (|A6*x^6| < ~LDBL_MIN).
 * "huge" is anything that would make fn*L1 inexact (|x| > ~2**17*ln2).
 */
static inline void
__k_expl(long double x, long double *hip, long double *lop, int *kp)
{
	long double q, r, r1, t;
	double dr, fn, r2;
	int n, n2;

	/* Reduce x to (k*ln2 + endpoint[n2] + r1 + r2). */
	fn = rnint((double)x * INV_L);
	n = irint(fn);
	n2 = (unsigned)n % INTERVALS;
	/* Depend on the sign bit being propagated: */
	*kp = n >> LOG2_INTERVALS;
	r1 = x - fn * L1;
	r2 = fn * -L2;
	r = r1 + r2;

	/* Evaluate expl(endpoint[n2] + r1 + r2) = tbl[n2] * expl(r1 + r2). */
	dr = r;
	q = r2 + r * r * (A2 + r * (A3 + r * (A4 + r * (A5 + r * (A6 +
	    dr * (A7 + dr * (A8 + dr * (A9 + dr * A10))))))));
	t = tbl[n2].lo + tbl[n2].hi;
	*hip = tbl[n2].hi;
	*lop = tbl[n2].lo + t * (q + r1);
}

/*
 * XXX: the rest of the functions are identical for ld80 and ld128.
 * However, we should use scalbnl() for ld128, since long double
 * multiplication was very slow on sparc64 and no new evaluation has
 * been made for aarch64 and/or riscv.
 */

static inline void
k_hexpl(long double x, long double *hip, long double *lop)
{
	float twopkm1;
	int k;

	__k_expl(x, hip, lop, &k);
	SET_FLOAT_WORD(twopkm1, 0x3f800000 + ((k - 1) << 23));
	*hip *= twopkm1;
	*lop *= twopkm1;
}

static inline long double
hexpl(long double x)
{
	long double hi, lo, twopkm2;
	int k;

	twopkm2 = 1;
	__k_expl(x, &hi, &lo, &k);
	SET_LDBL_EXPSIGN(twopkm2, BIAS + k - 2);
	return (lo + hi) * 2 * twopkm2;
}

#ifdef _COMPLEX_H
/*
 * See ../src/k_exp.c for details.
 */
static inline long double complex
__ldexp_cexpl(long double complex z, int expt)
{
	long double c, exp_x, hi, lo, s;
	long double x, y, scale1, scale2;
	int half_expt, k;

	x = creall(z);
	y = cimagl(z);
	__k_expl(x, &hi, &lo, &k);

	exp_x = (lo + hi) * 0x1p16382L;
	expt += k - 16382;

	scale1 = 1;
	half_expt = expt / 2;
	SET_LDBL_EXPSIGN(scale1, BIAS + half_expt);
	scale2 = 1;
	SET_LDBL_EXPSIGN(scale2, BIAS + expt - half_expt);

	sincosl(y, &s, &c);
	return (CMPLXL(c * exp_x * scale1 * scale2,
	    s * exp_x * scale1 * scale2));
}
#endif /* _COMPLEX_H */

/* ===== invtrig.c ===== */
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


/*
 * asinl() and acosl()
 */
const long double
ref_pS0 =  1.66666666666666666666666666666700314e-01L,
ref_pS1 = -7.32816946414566252574527475428622708e-01L,
ref_pS2 =  1.34215708714992334609030036562143589e+00L,
ref_pS3 = -1.32483151677116409805070261790752040e+00L,
ref_pS4 =  7.61206183613632558824485341162121989e-01L,
ref_pS5 = -2.56165783329023486777386833928147375e-01L,
ref_pS6 =  4.80718586374448793411019434585413855e-02L,
ref_pS7 = -4.42523267167024279410230886239774718e-03L,
ref_pS8 =  1.44551535183911458253205638280410064e-04L,
ref_pS9 = -2.10558957916600254061591040482706179e-07L,
ref_qS1 = -4.84690167848739751544716485245697428e+00L,
ref_qS2 =  9.96619113536172610135016921140206980e+00L,
ref_qS3 = -1.13177895428973036660836798461641458e+01L,
ref_qS4 =  7.74004374389488266169304117714658761e+00L,
ref_qS5 = -3.25871986053534084709023539900339905e+00L,
ref_qS6 =  8.27830318881232209752469022352928864e-01L,
ref_qS7 = -1.18768052702942805423330715206348004e-01L,
ref_qS8 =  8.32600764660522313269101537926539470e-03L,
ref_qS9 = -1.99407384882605586705979504567947007e-04L;

/*
 * atanl()
 */
const long double ref_atanhi[] = {
	 4.63647609000806116214256231461214397e-01L,
	 7.85398163397448309615660845819875699e-01L,       
	 9.82793723247329067985710611014666038e-01L,       
	 1.57079632679489661923132169163975140e+00L,
};

const long double ref_atanlo[] = {
	 4.89509642257333492668618435220297706e-36L,
	 2.16795253253094525619926100651083806e-35L,
	-2.31288434538183565909319952098066272e-35L,
	 4.33590506506189051239852201302167613e-35L,
};

const long double ref_aT[] = {
	 3.33333333333333333333333333333333125e-01L,
	-1.99999999999999999999999999999180430e-01L,
	 1.42857142857142857142857142125269827e-01L,
	-1.11111111111111111111110834490810169e-01L,
	 9.09090909090909090908522355708623681e-02L,
	-7.69230769230769230696553844935357021e-02L,
	 6.66666666666666660390096773046256096e-02L,
	-5.88235294117646671706582985209643694e-02L,
	 5.26315789473666478515847092020327506e-02L,
	-4.76190476189855517021024424991436144e-02L,
	 4.34782608678695085948531993458097026e-02L,
	-3.99999999632663469330634215991142368e-02L,
	 3.70370363987423702891250829918659723e-02L,
	-3.44827496515048090726669907612335954e-02L,
	 3.22579620681420149871973710852268528e-02L,
	-3.03020767654269261041647570626778067e-02L,
	 2.85641979882534783223403715930946138e-02L,
	-2.69824879726738568189929461383741323e-02L,
	 2.54194698498808542954187110873675769e-02L,
	-2.35083879708189059926183138130183215e-02L,
	 2.04832358998165364349957325067131428e-02L,
	-1.54489555488544397858507248612362957e-02L,
	 8.64492360989278761493037861575248038e-03L,
	-2.58521121597609872727919154569765469e-03L,
};

const long double ref_pi_lo = 8.67181013012378102479704402604335225e-35L;

const size_t ref_atanhi_n = sizeof(ref_atanhi) / sizeof(ref_atanhi[0]);
const size_t ref_atanlo_n = sizeof(ref_atanlo) / sizeof(ref_atanlo[0]);
const size_t ref_aT_n = sizeof(ref_aT) / sizeof(ref_aT[0]);

/* ===== s_expl.c ===== */
#define huge __b0084_sexpl_huge
#define tiny __b0084_sexpl_tiny
#define twom10000 __b0084_sexpl_twom10000
#define o_threshold __b0084_sexpl_o_threshold
#define u_threshold __b0084_sexpl_u_threshold
#define T1 __b0084_sexpl_T1
#define T2 __b0084_sexpl_T2
#define T3 __b0084_sexpl_T3
#define C3 __b0084_sexpl_C3
#define C4 __b0084_sexpl_C4
#define C5 __b0084_sexpl_C5
#define C6 __b0084_sexpl_C6
#define C7 __b0084_sexpl_C7
#define C8 __b0084_sexpl_C8
#define C9 __b0084_sexpl_C9
#define C10 __b0084_sexpl_C10
#define C11 __b0084_sexpl_C11
#define C12 __b0084_sexpl_C12
#define C13 __b0084_sexpl_C13
#define C14 __b0084_sexpl_C14
#define C15 __b0084_sexpl_C15
#define C16 __b0084_sexpl_C16
#define C17 __b0084_sexpl_C17
#define C18 __b0084_sexpl_C18
#define D3 __b0084_sexpl_D3
#define D4 __b0084_sexpl_D4
#define D5 __b0084_sexpl_D5
#define D6 __b0084_sexpl_D6
#define D7 __b0084_sexpl_D7
#define D8 __b0084_sexpl_D8
#define D9 __b0084_sexpl_D9
#define D10 __b0084_sexpl_D10
#define D11 __b0084_sexpl_D11
#define D12 __b0084_sexpl_D12
#define D13 __b0084_sexpl_D13
#define D14 __b0084_sexpl_D14
#define D15 __b0084_sexpl_D15
#define D16 __b0084_sexpl_D16
#define D17 __b0084_sexpl_D17
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2009-2013 Steven G. Kargl
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
 *
 * Optimized by Bruce D. Evans.
 */

/*
 * ld128 version of s_expl.c.  See ../ld80/s_expl.c for most comments.
 */



/* XXX Prevent compilers from erroneously constant folding these: */
static const volatile long double
huge = 0x1p10000L,
tiny = 0x1p-10000L;

static const long double
twom10000 = 0x1p-10000L;

static const long double
/* log(2**16384 - 0.5) rounded towards zero: */
/* log(2**16384 - 0.5 + 1) rounded towards zero for ref_expm1l() is the same: */
o_threshold =  11356.523406294143949491931077970763428L,
/* log(2**(-16381-64-1)) rounded towards zero: */
u_threshold = -11433.462743336297878837243843452621503L;

long double
ref_expl(long double x)
{
	union IEEEl2bits u;
	long double hi, lo, t, twopk;
	int k;
	uint16_t hx, ix;

	/* Filter out exceptional cases. */
	u.e = x;
	hx = u.xbits.expsign;
	ix = hx & 0x7fff;
	if (ix >= BIAS + 13) {		/* |x| >= 8192 or x is NaN */
		if (ix == BIAS + LDBL_MAX_EXP) {
			if (hx & 0x8000)  /* x is -Inf or -NaN */
				RETURNF(-1 / x);
			RETURNF(x + x);	/* x is +Inf or +NaN */
		}
		if (x > o_threshold)
			RETURNF(huge * huge);
		if (x < u_threshold)
			RETURNF(tiny * tiny);
	} else if (ix < BIAS - 114) {	/* |x| < 0x1p-114 */
		RETURNF(1 + x);		/* 1 with inexact iff x != 0 */
	}

	ENTERI();

	twopk = 1;
	__k_expl(x, &hi, &lo, &k);
	t = SUM2P(hi, lo);

	/* Scale by 2**k. */
	/*
	 * XXX sparc64 multiplication was so slow that scalbnl() is faster,
	 * but performance on aarch64 and riscv hasn't yet been quantified.
	 */
	if (k >= LDBL_MIN_EXP) {
		if (k == LDBL_MAX_EXP)
			RETURNI(t * 2 * 0x1p16383L);
		SET_LDBL_EXPSIGN(twopk, BIAS + k);
		RETURNI(t * twopk);
	} else {
		SET_LDBL_EXPSIGN(twopk, BIAS + k + 10000);
		RETURNI(t * twopk * twom10000);
	}
}

/*
 * Our T1 and T2 are chosen to be approximately the points where method
 * A and method B have the same accuracy.  Tang's T1 and T2 are the
 * points where method A's accuracy changes by a full bit.  For Tang,
 * this drop in accuracy makes method A immediately less accurate than
 * method B, but our larger INTERVALS makes method A 2 bits more
 * accurate so it remains the most accurate method significantly
 * closer to the origin despite losing the full bit in our extended
 * range for it.
 *
 * Split the interval [T1, T2] into two intervals [T1, T3] and [T3, T2].
 * Setting T3 to 0 would require the |x| < 0x1p-113 condition to appear
 * in both subintervals, so set T3 = 2**-5, which places the condition
 * into the [T1, T3] interval.
 *
 * XXX we now do this more to (partially) balance the number of terms
 * in the C and D polys than to avoid checking the condition in both
 * intervals.
 *
 * XXX these micro-optimizations are excessive.
 */
static const double
T1 = -0.1659,				/* ~-30.625/128 * log(2) */
T2 =  0.1659,				/* ~30.625/128 * log(2) */
T3 =  0.03125;

/*
 * Domain [-0.1659, 0.03125], range ~[2.9134e-44, 1.8404e-37]:
 * |(exp(x)-1-x-x**2/2)/x - p(x)| < 2**-122.03
 *
 * XXX none of the long double C or D coeffs except C10 is correctly printed.
 * If you re-print their values in %.35Le format, the result is always
 * different.  For example, the last 2 digits in C3 should be 59, not 67.
 * 67 is apparently from rounding an extra-precision value to 36 decimal
 * places.
 */
static const long double
C3  =  1.66666666666666666666666666666666667e-1L,
C4  =  4.16666666666666666666666666666666645e-2L,
C5  =  8.33333333333333333333333333333371638e-3L,
C6  =  1.38888888888888888888888888891188658e-3L,
C7  =  1.98412698412698412698412697235950394e-4L,
C8  =  2.48015873015873015873015112487849040e-5L,
C9  =  2.75573192239858906525606685484412005e-6L,
C10 =  2.75573192239858906612966093057020362e-7L,
C11 =  2.50521083854417203619031960151253944e-8L,
C12 =  2.08767569878679576457272282566520649e-9L,
C13 =  1.60590438367252471783548748824255707e-10L;

/*
 * XXX this has 1 more coeff than needed.
 * XXX can start the double coeffs but not the double mults at C10.
 * With my coeffs (C10-C17 double; s = best_s):
 * Domain [-0.1659, 0.03125], range ~[-1.1976e-37, 1.1976e-37]:
 * |(exp(x)-1-x-x**2/2)/x - p(x)| ~< 2**-122.65
 */
static const double
C14 =  1.1470745580491932e-11,		/*  0x1.93974a81dae30p-37 */
C15 =  7.6471620181090468e-13,		/*  0x1.ae7f3820adab1p-41 */
C16 =  4.7793721460260450e-14,		/*  0x1.ae7cd18a18eacp-45 */
C17 =  2.8074757356658877e-15,		/*  0x1.949992a1937d9p-49 */
C18 =  1.4760610323699476e-16;		/*  0x1.545b43aabfbcdp-53 */

/*
 * Domain [0.03125, 0.1659], range ~[-2.7676e-37, -1.0367e-38]:
 * |(exp(x)-1-x-x**2/2)/x - p(x)| < 2**-121.44
 */
static const long double
D3  =  1.66666666666666666666666666666682245e-1L,
D4  =  4.16666666666666666666666666634228324e-2L,
D5  =  8.33333333333333333333333364022244481e-3L,
D6  =  1.38888888888888888888887138722762072e-3L,
D7  =  1.98412698412698412699085805424661471e-4L,
D8  =  2.48015873015873015687993712101479612e-5L,
D9  =  2.75573192239858944101036288338208042e-6L,
D10 =  2.75573192239853161148064676533754048e-7L,
D11 =  2.50521083855084570046480450935267433e-8L,
D12 =  2.08767569819738524488686318024854942e-9L,
D13 =  1.60590442297008495301927448122499313e-10L;

/*
 * XXX this has 1 more coeff than needed.
 * XXX can start the double coeffs but not the double mults at D11.
 * With my coeffs (D11-D16 double):
 * Domain [0.03125, 0.1659], range ~[-1.1980e-37, 1.1980e-37]:
 * |(exp(x)-1-x-x**2/2)/x - p(x)| ~< 2**-122.65
 */
static const double
D14 =  1.1470726176204336e-11,		/*  0x1.93971dc395d9ep-37 */
D15 =  7.6478532249581686e-13,		/*  0x1.ae892e3D16fcep-41 */
D16 =  4.7628892832607741e-14,		/*  0x1.ad00Dfe41feccp-45 */
D17 =  3.0524857220358650e-15;		/*  0x1.D7e8d886Df921p-49 */

long double
ref_expm1l(long double x)
{
	union IEEEl2bits u, v;
	long double hx2_hi, hx2_lo, q, r, r1, t, twomk, twopk, x_hi;
	long double x_lo, x2;
	double dr, dx, fn, r2;
	int k, n, n2;
	uint16_t hx, ix;

	/* Filter out exceptional cases. */
	u.e = x;
	hx = u.xbits.expsign;
	ix = hx & 0x7fff;
	if (ix >= BIAS + 7) {		/* |x| >= 128 or x is NaN */
		if (ix == BIAS + LDBL_MAX_EXP) {
			if (hx & 0x8000)  /* x is -Inf or -NaN */
				RETURNF(-1 / x - 1);
			RETURNF(x + x);	/* x is +Inf or +NaN */
		}
		if (x > o_threshold)
			RETURNF(huge * huge);
		/*
		 * ref_expm1l() never underflows, but it must avoid
		 * unrepresentable large negative exponents.  We used a
		 * much smaller threshold for large |x| above than in
		 * ref_expl() so as to handle not so large negative exponents
		 * in the same way as large ones here.
		 */
		if (hx & 0x8000)	/* x <= -128 */
			RETURNF(tiny - 1);	/* good for x < -114ln2 - eps */
	}

	ENTERI();

	if (T1 < x && x < T2) {
		x2 = x * x;
		dx = x;

		if (x < T3) {
			if (ix < BIAS - 113) {	/* |x| < 0x1p-113 */
				/* x (rounded) with inexact if x != 0: */
				RETURNI(x == 0 ? x :
				    (0x1p200 * x + fabsl(x)) * 0x1p-200);
			}
			q = x * x2 * C3 + x2 * x2 * (C4 + x * (C5 + x * (C6 +
			    x * (C7 + x * (C8 + x * (C9 + x * (C10 +
			    x * (C11 + x * (C12 + x * (C13 +
			    dx * (C14 + dx * (C15 + dx * (C16 +
			    dx * (C17 + dx * C18))))))))))))));
		} else {
			q = x * x2 * D3 + x2 * x2 * (D4 + x * (D5 + x * (D6 +
			    x * (D7 + x * (D8 + x * (D9 + x * (D10 +
			    x * (D11 + x * (D12 + x * (D13 +
			    dx * (D14 + dx * (D15 + dx * (D16 +
			    dx * D17)))))))))))));
		}

		x_hi = (float)x;
		x_lo = x - x_hi;
		hx2_hi = x_hi * x_hi / 2;
		hx2_lo = x_lo * (x + x_hi) / 2;
		if (ix >= BIAS - 7)
			RETURNI((hx2_hi + x_hi) + (hx2_lo + x_lo + q));
		else
			RETURNI(x + (hx2_lo + q + hx2_hi));
	}

	/* Reduce x to (k*ln2 + endpoint[n2] + r1 + r2). */
	fn = rnint((double)x * INV_L);
	n = irint(fn);
	n2 = (unsigned)n % INTERVALS;
	k = n >> LOG2_INTERVALS;
	r1 = x - fn * L1;
	r2 = fn * -L2;
	r = r1 + r2;

	/* Prepare scale factor. */
	v.e = 1;
	v.xbits.expsign = BIAS + k;
	twopk = v.e;

	/*
	 * Evaluate lower terms of
	 * ref_expl(endpoint[n2] + r1 + r2) = tbl[n2] * ref_expl(r1 + r2).
	 */
	dr = r;
	q = r2 + r * r * (A2 + r * (A3 + r * (A4 + r * (A5 + r * (A6 +
	    dr * (A7 + dr * (A8 + dr * (A9 + dr * A10))))))));

	t = tbl[n2].lo + tbl[n2].hi;

	if (k == 0) {
		t = SUM2P(tbl[n2].hi - 1, tbl[n2].lo * (r1 + 1) + t * q +
		    tbl[n2].hi * r1);
		RETURNI(t);
	}
	if (k == -1) {
		t = SUM2P(tbl[n2].hi - 2, tbl[n2].lo * (r1 + 1) + t * q +
		    tbl[n2].hi * r1);
		RETURNI(t / 2);
	}
	if (k < -7) {
		t = SUM2P(tbl[n2].hi, tbl[n2].lo + t * (q + r1));
		RETURNI(t * twopk - 1);
	}
	if (k > 2 * LDBL_MANT_DIG - 1) {
		t = SUM2P(tbl[n2].hi, tbl[n2].lo + t * (q + r1));
		if (k == LDBL_MAX_EXP)
			RETURNI(t * 2 * 0x1p16383L - 1);
		RETURNI(t * twopk - 1);
	}

	v.xbits.expsign = BIAS - k;
	twomk = v.e;

	if (k > LDBL_MANT_DIG - 1)
		t = SUM2P(tbl[n2].hi, tbl[n2].lo - twomk + t * (q + r1));
	else
		t = SUM2P(tbl[n2].hi - twomk, tbl[n2].lo + t * (q + r1));
	RETURNI(t * twopk);
}
#undef huge
#undef tiny
#undef twom10000
#undef o_threshold
#undef u_threshold
#undef T1
#undef T2
#undef T3
#undef C3
#undef C4
#undef C5
#undef C6
#undef C7
#undef C8
#undef C9
#undef C10
#undef C11
#undef C12
#undef C13
#undef C14
#undef C15
#undef C16
#undef C17
#undef C18
#undef D3
#undef D4
#undef D5
#undef D6
#undef D7
#undef D8
#undef D9
#undef D10
#undef D11
#undef D12
#undef D13
#undef D14
#undef D15
#undef D16
#undef D17

/* ===== e_lgammal_r.c ===== */
#define vzero __b0084_elg_vzero
#define zero __b0084_elg_zero
#define half __b0084_elg_half
#define one __b0084_elg_one
#define pi __b0084_elg_pi
#define a0 __b0084_elg_a0
#define a1 __b0084_elg_a1
#define a2 __b0084_elg_a2
#define a3 __b0084_elg_a3
#define a4 __b0084_elg_a4
#define a5 __b0084_elg_a5
#define a6 __b0084_elg_a6
#define a7 __b0084_elg_a7
#define a8 __b0084_elg_a8
#define a9 __b0084_elg_a9
#define a10 __b0084_elg_a10
#define a11 __b0084_elg_a11
#define a12 __b0084_elg_a12
#define a13 __b0084_elg_a13
#define a14 __b0084_elg_a14
#define a15 __b0084_elg_a15
#define a16 __b0084_elg_a16
#define a17 __b0084_elg_a17
#define a18 __b0084_elg_a18
#define a19 __b0084_elg_a19
#define a20 __b0084_elg_a20
#define a21 __b0084_elg_a21
#define a22 __b0084_elg_a22
#define a23 __b0084_elg_a23
#define tc __b0084_elg_tc
#define tf __b0084_elg_tf
#define tt __b0084_elg_tt
#define t0 __b0084_elg_t0
#define t1 __b0084_elg_t1
#define t2 __b0084_elg_t2
#define t3 __b0084_elg_t3
#define t4 __b0084_elg_t4
#define t5 __b0084_elg_t5
#define t6 __b0084_elg_t6
#define t7 __b0084_elg_t7
#define t8 __b0084_elg_t8
#define t9 __b0084_elg_t9
#define t10 __b0084_elg_t10
#define t11 __b0084_elg_t11
#define t12 __b0084_elg_t12
#define t13 __b0084_elg_t13
#define t14 __b0084_elg_t14
#define t15 __b0084_elg_t15
#define t16 __b0084_elg_t16
#define t17 __b0084_elg_t17
#define t18 __b0084_elg_t18
#define t19 __b0084_elg_t19
#define t20 __b0084_elg_t20
#define t21 __b0084_elg_t21
#define t22 __b0084_elg_t22
#define t23 __b0084_elg_t23
#define t24 __b0084_elg_t24
#define t25 __b0084_elg_t25
#define t26 __b0084_elg_t26
#define t27 __b0084_elg_t27
#define t28 __b0084_elg_t28
#define t29 __b0084_elg_t29
#define t30 __b0084_elg_t30
#define t31 __b0084_elg_t31
#define t32 __b0084_elg_t32
#define u0 __b0084_elg_u0
#define u1 __b0084_elg_u1
#define u2 __b0084_elg_u2
#define u3 __b0084_elg_u3
#define u4 __b0084_elg_u4
#define u5 __b0084_elg_u5
#define u6 __b0084_elg_u6
#define u7 __b0084_elg_u7
#define u8 __b0084_elg_u8
#define u9 __b0084_elg_u9
#define u10 __b0084_elg_u10
#define v1 __b0084_elg_v1
#define v2 __b0084_elg_v2
#define v3 __b0084_elg_v3
#define v4 __b0084_elg_v4
#define v5 __b0084_elg_v5
#define v6 __b0084_elg_v6
#define v7 __b0084_elg_v7
#define v8 __b0084_elg_v8
#define v9 __b0084_elg_v9
#define v10 __b0084_elg_v10
#define v11 __b0084_elg_v11
#define s0 __b0084_elg_s0
#define s1 __b0084_elg_s1
#define s2 __b0084_elg_s2
#define s3 __b0084_elg_s3
#define s4 __b0084_elg_s4
#define s5 __b0084_elg_s5
#define s6 __b0084_elg_s6
#define s7 __b0084_elg_s7
#define s8 __b0084_elg_s8
#define s9 __b0084_elg_s9
#define s10 __b0084_elg_s10
#define s11 __b0084_elg_s11
#define r1 __b0084_elg_r1
#define r2 __b0084_elg_r2
#define r3 __b0084_elg_r3
#define r4 __b0084_elg_r4
#define r5 __b0084_elg_r5
#define r6 __b0084_elg_r6
#define r7 __b0084_elg_r7
#define r8 __b0084_elg_r8
#define r9 __b0084_elg_r9
#define r10 __b0084_elg_r10
#define r11 __b0084_elg_r11
#define w0 __b0084_elg_w0
#define w1 __b0084_elg_w1
#define w2 __b0084_elg_w2
#define w3 __b0084_elg_w3
#define w4 __b0084_elg_w4
#define w5 __b0084_elg_w5
#define w6 __b0084_elg_w6
#define w7 __b0084_elg_w7
#define w8 __b0084_elg_w8
#define w9 __b0084_elg_w9
#define w10 __b0084_elg_w10
#define w11 __b0084_elg_w11
#define w12 __b0084_elg_w12
#define w13 __b0084_elg_w13
#define w14 __b0084_elg_w14
#define w15 __b0084_elg_w15
#define w16 __b0084_elg_w16
#define w17 __b0084_elg_w17
#define w18 __b0084_elg_w18
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * See e_lgamma_r.c for complete comments.
 *
 * Converted to long double by Steven G. Kargl.
 */


static const volatile double vzero = 0;

static const double
zero=  0,
half=  0.5,
one =  1;

static const long double
pi  =  3.14159265358979323846264338327950288e+00L;
/*
 * Domain y in [0x1p-119, 0.28], range ~[-1.4065e-36, 1.4065e-36]:
 * |(lgamma(2 - y) + y / 2) / y - a(y)| < 2**-119.1
 */
static const long double
a0  =  7.72156649015328606065120900824024296e-02L,
a1  =  3.22467033424113218236207583323018498e-01L,
a2  =  6.73523010531980951332460538330282217e-02L,
a3  =  2.05808084277845478790009252803463129e-02L,
a4  =  7.38555102867398526627292839296001626e-03L,
a5  =  2.89051033074152328576829509522483468e-03L,
a6  =  1.19275391170326097618357349881842913e-03L,
a7  =  5.09669524743042462515256340206203019e-04L,
a8  =  2.23154758453578096143609255559576017e-04L,
a9  =  9.94575127818397632126978731542755129e-05L,
a10 =  4.49262367375420471287545895027098145e-05L,
a11 =  2.05072127845117995426519671481628849e-05L,
a12 =  9.43948816959096748454087141447939513e-06L,
a13 =  4.37486780697359330303852050718287419e-06L,
a14 =  2.03920783892362558276037363847651809e-06L,
a15 =  9.55191070057967287877923073200324649e-07L,
a16 =  4.48993286185740853170657139487620560e-07L,
a17 =  2.13107543597620911675316728179563522e-07L,
a18 =  9.70745379855304499867546549551023473e-08L,
a19 =  5.61889970390290257926487734695402075e-08L,
a20 =  6.42739653024130071866684358960960951e-09L,
a21 =  3.34491062143649291746195612991870119e-08L,
a22 = -1.57068547394315223934653011440641472e-08L,
a23 =  1.30812825422415841213733487745200632e-08L;
/*
 * Domain x in [tc-0.24, tc+0.28], range ~[-6.3201e-37, 6.3201e-37]:
 * |(lgamma(x) - tf) - t(x - tc)| < 2**-120.3.
 */
static const long double
tc  =  1.46163214496836234126265954232572133e+00L,
tf  = -1.21486290535849608095514557177691584e-01L,
tt  =  1.57061739945077675484237837992951704e-36L,
t0  = -1.99238329499314692728655623767019240e-36L,
t1  = -6.08453430711711404116887457663281416e-35L,
t2  =  4.83836122723810585213722380854828904e-01L,
t3  = -1.47587722994530702030955093950668275e-01L,
t4  =  6.46249402389127526561003464202671923e-02L,
t5  = -3.27885410884813055008502586863748063e-02L,
t6  =  1.79706751152103942928638276067164935e-02L,
t7  = -1.03142230366363872751602029672767978e-02L,
t8  =  6.10053602051788840313573150785080958e-03L,
t9  = -3.68456960831637325470641021892968954e-03L,
t10 =  2.25976482322181046611440855340968560e-03L,
t11 = -1.40225144590445082933490395950664961e-03L,
t12 =  8.78232634717681264035014878172485575e-04L,
t13 = -5.54194952796682301220684760591403899e-04L,
t14 =  3.51912956837848209220421213975000298e-04L,
t15 = -2.24653443695947456542669289367055542e-04L,
t16 =  1.44070395420840737695611929680511823e-04L,
t17 = -9.27609865550394140067059487518862512e-05L,
t18 =  5.99347334438437081412945428365433073e-05L,
t19 = -3.88458388854572825603964274134801009e-05L,
t20 =  2.52476631610328129217896436186551043e-05L,
t21 = -1.64508584981658692556994212457518536e-05L,
t22 =  1.07434583475987007495523340296173839e-05L,
t23 = -7.03070407519397260929482550448878399e-06L,
t24 =  4.60968590693753579648385629003100469e-06L,
t25 = -3.02765473778832036018438676945512661e-06L,
t26 =  1.99238771545503819972741288511303401e-06L,
t27 = -1.31281299822614084861868817951788579e-06L,
t28 =  8.60844432267399655055574642052370223e-07L,
t29 = -5.64535486432397413273248363550536374e-07L,
t30 =  3.99357783676275660934903139592727737e-07L,
t31 = -2.95849029193433121795495215869311610e-07L,
t32 =  1.37790144435073124976696250804940384e-07L;
/*
 * Domain y in [-0.1, 0.232], range ~[-1.4046e-37, 1.4181e-37]:
 * |(lgamma(1 + y) + 0.5 * y) / y - u(y) / v(y)| < 2**-122.8
 */
static const long double
u0  = -7.72156649015328606065120900824024311e-02L,
u1  =  4.24082772271938167430983113242482656e-01L,
u2  =  2.96194003481457101058321977413332171e+00L,
u3  =  6.49503267711258043997790983071543710e+00L,
u4  =  7.40090051288150177152835698948644483e+00L,
u5  =  4.94698036296756044610805900340723464e+00L,
u6  =  2.00194224610796294762469550684947768e+00L,
u7  =  4.82073087750608895996915051568834949e-01L,
u8  =  6.46694052280506568192333848437585427e-02L,
u9  =  4.17685526755100259316625348933108810e-03L,
u10 =  9.06361003550314327144119307810053410e-05L,
v1  =  5.15937098592887275994320496999951947e+00L,
v2  =  1.14068418766251486777604403304717558e+01L,
v3  =  1.41164839437524744055723871839748489e+01L,
v4  =  1.07170702656179582805791063277960532e+01L,
v5  =  5.14448694179047879915042998453632434e+00L,
v6  =  1.55210088094585540637493826431170289e+00L,
v7  =  2.82975732849424562719893657416365673e-01L,
v8  =  2.86424622754753198010525786005443539e-02L,
v9  =  1.35364253570403771005922441442688978e-03L,
v10 =  1.91514173702398375346658943749580666e-05L,
v11 = -3.25364686890242327944584691466034268e-08L;
/*
 * Domain x in (2, 3], range ~[-1.3341e-36, 1.3536e-36]:
 * |(lgamma(y+2) - 0.5 * y) / y - s(y)/r(y)| < 2**-120.1
 * with y = x - 2.
 */
static const long double
s0  = -7.72156649015328606065120900824024297e-02L,
s1  =  1.23221687850916448903914170805852253e-01L,
s2  =  5.43673188699937239808255378293820020e-01L,
s3  =  6.31998137119005233383666791176301800e-01L,
s4  =  3.75885340179479850993811501596213763e-01L,
s5  =  1.31572908743275052623410195011261575e-01L,
s6  =  2.82528453299138685507186287149699749e-02L,
s7  =  3.70262021550340817867688714880797019e-03L,
s8  =  2.83374000312371199625774129290973648e-04L,
s9  =  1.15091830239148290758883505582343691e-05L,
s10 =  2.04203474281493971326506384646692446e-07L,
s11 =  9.79544198078992058548607407635645763e-10L,
r1  =  2.58037466655605285937112832039537492e+00L,
r2  =  2.86289413392776399262513849911531180e+00L,
r3  =  1.78691044735267497452847829579514367e+00L,
r4  =  6.89400381446725342846854215600008055e-01L,
r5  =  1.70135865462567955867134197595365343e-01L,
r6  =  2.68794816183964420375498986152766763e-02L,
r7  =  2.64617234244861832870088893332006679e-03L,
r8  =  1.52881761239180800640068128681725702e-04L,
r9  =  4.63264813762296029824851351257638558e-06L,
r10 =  5.89461519146957343083848967333671142e-08L,
r11 =  1.79027678176582527798327441636552968e-10L;
/*
 * Domain z in [8, 0x1p70], range ~[-9.8214e-35, 9.8214e-35]:
 * |lgamma(x) - (x - 0.5) * (log(x) - 1) - w(1/x)| < 2**-113.0
 */
static const long double
w0  =  4.18938533204672741780329736405617738e-01L,
w1  =  8.33333333333333333333333333332852026e-02L,
w2  = -2.77777777777777777777777727810123528e-03L,
w3  =  7.93650793650793650791708939493907380e-04L,
w4  = -5.95238095238095234390450004444370959e-04L,
w5  =  8.41750841750837633887817658848845695e-04L,
w6  = -1.91752691752396849943172337347259743e-03L,
w7  =  6.41025640880333069429106541459015557e-03L,
w8  = -2.95506530801732133437990433080327074e-02L,
w9  =  1.79644237328444101596766586979576927e-01L,
w10 = -1.39240539108367641920172649259736394e+00L,
w11 =  1.33987701479007233325288857758641761e+01L,
w12 = -1.56363596431084279780966590116006255e+02L,
w13 =  2.14830978044410267201172332952040777e+03L,
w14 = -3.28636067474227378352761516589092334e+04L,
w15 =  5.06201257747865138432663574251462485e+05L,
w16 = -6.79720123352023636706247599728048344e+06L,
w17 =  6.57556601705472106989497289465949255e+07L,
w18 = -3.26229058141181783534257632389415580e+08L;

static long double
sin_pil(long double x)
{
	volatile long double vz;
	long double y,z;
	uint64_t lx, n;
	uint16_t hx;

	y = -x;

	vz = y+0x1.p112;
	z = vz-0x1.p112;
	if (z == y)
	    return zero;

	vz = y+0x1.p110;
	EXTRACT_LDBL128_WORDS(hx,lx,n,vz);
	z = vz-0x1.p110;
	if (z > y) {
	    z -= 0.25;
	    n--;
	}
	n &= 7;
	y = y - z + n * 0.25;

	switch (n) {
	    case 0:   y =  __kernel_sinl(pi*y,zero,0); break;
	    case 1:
	    case 2:   y =  __kernel_cosl(pi*(0.5-y),zero); break;
	    case 3:
	    case 4:   y =  __kernel_sinl(pi*(one-y),zero,0); break;
	    case 5:
	    case 6:   y = -__kernel_cosl(pi*(y-1.5),zero); break;
	    default:  y =  __kernel_sinl(pi*(y-2.0),zero,0); break;
	    }
	return -y;
}

long double
ref_lgammal_r(long double x, int *signgamp)
{
	long double nadj,p,p1,p2,p3,q,r,t,w,y,z;
	uint64_t llx,lx;
	int i;
	uint16_t hx,ix;

	EXTRACT_LDBL128_WORDS(hx,lx,llx,x);

    /* purge +-Inf and NaNs */
	*signgamp = 1;
	ix = hx&0x7fff;
	if(ix==0x7fff) return x*x;

   /* purge +-0 and tiny arguments */
	*signgamp = 1-2*(hx>>15);
	if(ix<0x3fff-116) {		/* |x|<2**-(p+3), return -log(|x|) */
	    if((ix|lx|llx)==0)
		return one/vzero;
	    return -logl(fabsl(x));
	}

    /* purge negative integers and start evaluation for other x < 0 */
	if(hx&0x8000) {
	    *signgamp = 1;
	    if(ix>=0x3fff+112) 		/* |x|>=2**(p-1), must be -integer */
		return one/vzero;
	    t = sin_pil(x);
	    if(t==zero) return one/vzero;
	    nadj = logl(pi/fabsl(t*x));
	    if(t<zero) *signgamp = -1;
	    x = -x;
	}

    /* purge 1 and 2 */
	if((ix==0x3fff || ix==0x4000) && (lx|llx)==0) r = 0;
    /* for x < 2.0 */
	else if(ix<0x4000) {
	    if(x<=8.9999961853027344e-01) {
		r = -logl(x);
		if(x>=7.3159980773925781e-01) {y = 1-x; i= 0;}
		else if(x>=2.3163998126983643e-01) {y= x-(tc-1); i=1;}
	  	else {y = x; i=2;}
	    } else {
		r = 0;
	        if(x>=1.7316312789916992e+00) {y=2-x;i=0;}
	        else if(x>=1.2316322326660156e+00) {y=x-tc;i=1;}
		else {y=x-1;i=2;}
	    }
	    switch(i) {
	      case 0:
		z = y*y;
		p1 = a0+z*(a2+z*(a4+z*(a6+z*(a8+z*(a10+z*(a12+z*(a14+z*(a16+
		    z*(a18+z*(a20+z*a22))))))))));
		p2 = z*(a1+z*(a3+z*(a5+z*(a7+z*(a9+z*(a11+z*(a13+z*(a15+
		    z*(a17+z*(a19+z*(a21+z*a23)))))))))));
		p  = y*p1+p2;
		r  += p-y/2; break;
	      case 1:
		p = t0+y*t1+tt+y*y*(t2+y*(t3+y*(t4+y*(t5+y*(t6+y*(t7+y*(t8+
		    y*(t9+y*(t10+y*(t11+y*(t12+y*(t13+y*(t14+y*(t15+y*(t16+
		    y*(t17+y*(t18+y*(t19+y*(t20+y*(t21+y*(t22+y*(t23+
		    y*(t24+y*(t25+y*(t26+y*(t27+y*(t28+y*(t29+y*(t30+
		    y*(t31+y*t32))))))))))))))))))))))))))))));
		r += tf + p; break;
	      case 2:
		p1 = y*(u0+y*(u1+y*(u2+y*(u3+y*(u4+y*(u5+y*(u6+y*(u7+
		    y*(u8+y*(u9+y*u10))))))))));
		p2 = one+y*(v1+y*(v2+y*(v3+y*(v4+y*(v5+y*(v6+y*(v7+
		    y*(v8+y*(v9+y*(v10+y*v11))))))))));
		r += p1/p2-y/2;
	    }
	}
    /* x < 8.0 */
	else if(ix<0x4002) {
	    i = x;
	    y = x-i;
	    p = y*(s0+y*(s1+y*(s2+y*(s3+y*(s4+y*(s5+y*(s6+y*(s7+y*(s8+
		y*(s9+y*(s10+y*s11)))))))))));
	    q = one+y*(r1+y*(r2+y*(r3+y*(r4+y*(r5+y*(r6+y*(r7+y*(r8+
		y*(r9+y*(r10+y*r11))))))))));
	    r = y/2+p/q;
	    z = 1;	/* lgamma(1+s) = log(s) + lgamma(s) */
	    switch(i) {
	    case 7: z *= (y+6);		/* FALLTHRU */
	    case 6: z *= (y+5);		/* FALLTHRU */
	    case 5: z *= (y+4);		/* FALLTHRU */
	    case 4: z *= (y+3);		/* FALLTHRU */
	    case 3: z *= (y+2);		/* FALLTHRU */
		    r += logl(z); break;
	    }
    /* 8.0 <= x < 2**(p+3) */
	} else if (ix<0x3fff+116) {
	    t = logl(x);
	    z = one/x;
	    y = z*z;
	    w = w0+z*(w1+y*(w2+y*(w3+y*(w4+y*(w5+y*(w6+y*(w7+y*(w8+
		y*(w9+y*(w10+y*(w11+y*(w12+y*(w13+y*(w14+y*(w15+y*(w16+
		y*(w17+y*w18)))))))))))))))));
	    r = (x-half)*(t-one)+w;
    /* 2**(p+3) <= x <= inf */
	} else 
	    r =  x*(logl(x)-1);
	if(hx&0x8000) r = nadj - r;
	return r;
}
#undef vzero
#undef zero
#undef half
#undef one
#undef pi
#undef a0
#undef a1
#undef a2
#undef a3
#undef a4
#undef a5
#undef a6
#undef a7
#undef a8
#undef a9
#undef a10
#undef a11
#undef a12
#undef a13
#undef a14
#undef a15
#undef a16
#undef a17
#undef a18
#undef a19
#undef a20
#undef a21
#undef a22
#undef a23
#undef tc
#undef tf
#undef tt
#undef t0
#undef t1
#undef t2
#undef t3
#undef t4
#undef t5
#undef t6
#undef t7
#undef t8
#undef t9
#undef t10
#undef t11
#undef t12
#undef t13
#undef t14
#undef t15
#undef t16
#undef t17
#undef t18
#undef t19
#undef t20
#undef t21
#undef t22
#undef t23
#undef t24
#undef t25
#undef t26
#undef t27
#undef t28
#undef t29
#undef t30
#undef t31
#undef t32
#undef u0
#undef u1
#undef u2
#undef u3
#undef u4
#undef u5
#undef u6
#undef u7
#undef u8
#undef u9
#undef u10
#undef v1
#undef v2
#undef v3
#undef v4
#undef v5
#undef v6
#undef v7
#undef v8
#undef v9
#undef v10
#undef v11
#undef s0
#undef s1
#undef s2
#undef s3
#undef s4
#undef s5
#undef s6
#undef s7
#undef s8
#undef s9
#undef s10
#undef s11
#undef r1
#undef r2
#undef r3
#undef r4
#undef r5
#undef r6
#undef r7
#undef r8
#undef r9
#undef r10
#undef r11
#undef w0
#undef w1
#undef w2
#undef w3
#undef w4
#undef w5
#undef w6
#undef w7
#undef w8
#undef w9
#undef w10
#undef w11
#undef w12
#undef w13
#undef w14
#undef w15
#undef w16
#undef w17
#undef w18
