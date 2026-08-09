module;

#include <cfenv>
#include <cfloat>
#include <cmath>
#include <cstdint>

export module pbsd.lib.msun.src.b0327;

namespace pbsd::lib_msun_src::b0327::detail {

typedef union {
	float value;
	unsigned int word;
} ieee_float_shape_type;

#define GET_FLOAT_WORD(i, d)					\
	do {							\
		::pbsd::lib_msun_src::b0327::detail::ieee_float_shape_type gf_u; \
		gf_u.value = (d);				\
		(i) = gf_u.word;				\
	} while (0)

#define SET_FLOAT_WORD(d, i)					\
	do {							\
		::pbsd::lib_msun_src::b0327::detail::ieee_float_shape_type sf_u; \
		sf_u.word = (i);				\
		(d) = sf_u.value;				\
	} while (0)

#define STRICT_ASSIGN(type, lval, rval) ((lval) = (rval))

typedef union {
	double value;
	struct {
		std::uint32_t lsw;
		std::uint32_t msw;
	} parts;
} ieee_double_shape_type;

#define GET_HIGH_WORD(i, d)					\
	do {							\
		::pbsd::lib_msun_src::b0327::detail::ieee_double_shape_type gh_u; \
		gh_u.value = (d);				\
		(i) = static_cast<std::int32_t>(gh_u.parts.msw); \
	} while (0)

#define GET_LOW_WORD(i, d)					\
	do {							\
		::pbsd::lib_msun_src::b0327::detail::ieee_double_shape_type gl_u; \
		gl_u.value = (d);				\
		(i) = gl_u.parts.lsw;				\
	} while (0)

#define INSERT_WORDS(d, ix0, ix1)				\
	do {							\
		::pbsd::lib_msun_src::b0327::detail::ieee_double_shape_type iw_u; \
		iw_u.parts.msw = (ix0);				\
		iw_u.parts.lsw = (ix1);				\
		(d) = iw_u.value;				\
	} while (0)

#if LDBL_MANT_DIG == 64
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
#define	LDBL_NBIT	0x80000000u
#elif LDBL_MANT_DIG == 113
union IEEEl2bits {
	long double	e;
	struct {
		unsigned long	manl	:64;
		unsigned long	manh	:48;
		unsigned int	exp	:15;
		unsigned int	sign	:1;
	} bits;
	struct {
		unsigned long	manl	:64;
		unsigned long	manh	:48;
		unsigned int	expsign	:16;
	} xbits;
};
#define	LDBL_NBIT	0x8000000000000000ull
#else
#error "Unsupported long double format"
#endif

#define	GET_LDBL_EXPSIGN(i,d)					\
do {								\
	::pbsd::lib_msun_src::b0327::detail::IEEEl2bits ge_u;	\
	ge_u.e = (d);						\
	(i) = ge_u.xbits.expsign;				\
} while (0)

#define	SET_LDBL_EXPSIGN(d,v)					\
do {								\
	::pbsd::lib_msun_src::b0327::detail::IEEEl2bits se_u;	\
	se_u.e = (d);						\
	se_u.xbits.expsign = (v);				\
	(d) = se_u.e;						\
} while (0)

#ifdef __i386__
#define	LD80C(m, ex, v) {					\
	.xbits.man = __CONCAT(m, ULL),				\
	.xbits.expsign = (0x3fff + (ex)) | ((v) < 0 ? 0x8000 : 0), \
}
#else
#define	LD80C(m, ex, v)	{ .e = (v), }
#endif

#define	ENTERI()
#define	RETURNI(x)	return (x)

static inline long double
rnintl(long double x)
{
	return (x + 0x1.8p64L / 2 - 0x1.8p64L / 2);
}

#define irint(x) ((int)(x))

#if LDBL_MANT_DIG == 64
/* from: FreeBSD: head/lib/msun/ld80/s_expl.c 251343 2013-06-03 19:51:32Z kargl */

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
 * See s_expl.c for more comments about __k_expl().
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
L1 =  5.4152123484527692e-3,		/*  0x162e42ff000000.0p-60 */
L2 = -3.2819649005320973e-13,		/* -0x1718432a1b0e26.0p-94 */
/*
 * Domain [-0.002708, 0.002708], range ~[-5.7136e-24, 5.7110e-24]:
 * |exp(x) - p(x)| < 2**-77.2
 * (0.002708 is ln2/(2*INTERVALS) rounded up a little).
 */
A2 =  0.5,
A3 =  1.6666666666666119e-1,		/*  0x15555555555490.0p-55 */
A4 =  4.1666666666665887e-2,		/*  0x155555555554e5.0p-57 */
A5 =  8.3333354987869413e-3,		/*  0x1111115b789919.0p-59 */
A6 =  1.3888891738560272e-3;		/*  0x16c16c651633ae.0p-62 */

/*
 * 2^(i/INTERVALS) for i in [0,INTERVALS] is represented by two values where
 * the first 53 bits of the significand are stored in hi and the next 53
 * bits are in lo.  Tang's paper states that the trailing 6 bits of hi must
 * be zero for his algorithm in both single and double precision, because
 * the table is re-used in the implementation of expm1() where a floating
 * point addition involving hi must be exact.  Here hi is double, so
 * converting it to long double gives 11 trailing zero bits.
 */
static const struct {
	double	hi;
	double	lo;
} tbl[INTERVALS] = {
	{ 0x1p+0, 0x0p+0 },
	/*
	 * XXX hi is rounded down, and the formatting is not quite normal.
	 * But I rather like both.  The 0x1.*p format is good for 4N+1
	 * mantissa bits.  Rounding down makes the lo terms positive,
	 * so that the columnar formatting can be simpler.
	 */
	{ 0x1.0163da9fb3335p+0, 0x1.b61299ab8cdb7p-54 },
	{ 0x1.02c9a3e778060p+0, 0x1.dcdef95949ef4p-53 },
	{ 0x1.04315e86e7f84p+0, 0x1.7ae71f3441b49p-53 },
	{ 0x1.059b0d3158574p+0, 0x1.d73e2a475b465p-55 },
	{ 0x1.0706b29ddf6ddp+0, 0x1.8db880753b0f6p-53 },
	{ 0x1.0874518759bc8p+0, 0x1.186be4bb284ffp-57 },
	{ 0x1.09e3ecac6f383p+0, 0x1.1487818316136p-54 },
	{ 0x1.0b5586cf9890fp+0, 0x1.8a62e4adc610bp-54 },
	{ 0x1.0cc922b7247f7p+0, 0x1.01edc16e24f71p-54 },
	{ 0x1.0e3ec32d3d1a2p+0, 0x1.03a1727c57b53p-59 },
	{ 0x1.0fb66affed31ap+0, 0x1.e464123bb1428p-53 },
	{ 0x1.11301d0125b50p+0, 0x1.49d77e35db263p-53 },
	{ 0x1.12abdc06c31cbp+0, 0x1.f72575a649ad2p-53 },
	{ 0x1.1429aaea92ddfp+0, 0x1.66820328764b1p-53 },
	{ 0x1.15a98c8a58e51p+0, 0x1.2406ab9eeab0ap-55 },
	{ 0x1.172b83c7d517ap+0, 0x1.b9bef918a1d63p-53 },
	{ 0x1.18af9388c8de9p+0, 0x1.777ee1734784ap-53 },
	{ 0x1.1a35beb6fcb75p+0, 0x1.e5b4c7b4968e4p-55 },
	{ 0x1.1bbe084045cd3p+0, 0x1.3563ce56884fcp-53 },
	{ 0x1.1d4873168b9aap+0, 0x1.e016e00a2643cp-54 },
	{ 0x1.1ed5022fcd91cp+0, 0x1.71033fec2243ap-53 },
	{ 0x1.2063b88628cd6p+0, 0x1.dc775814a8495p-55 },
	{ 0x1.21f49917ddc96p+0, 0x1.2a97e9494a5eep-55 },
	{ 0x1.2387a6e756238p+0, 0x1.9b07eb6c70573p-54 },
	{ 0x1.251ce4fb2a63fp+0, 0x1.ac155bef4f4a4p-55 },
	{ 0x1.26b4565e27cddp+0, 0x1.2bd339940e9d9p-55 },
	{ 0x1.284dfe1f56380p+0, 0x1.2d9e2b9e07941p-53 },
	{ 0x1.29e9df51fdee1p+0, 0x1.612e8afad1255p-55 },
	{ 0x1.2b87fd0dad98fp+0, 0x1.fbbd48ca71f95p-53 },
	{ 0x1.2d285a6e4030bp+0, 0x1.0024754db41d5p-54 },
	{ 0x1.2ecafa93e2f56p+0, 0x1.1ca0f45d52383p-56 },
	{ 0x1.306fe0a31b715p+0, 0x1.6f46ad23182e4p-55 },
	{ 0x1.32170fc4cd831p+0, 0x1.a9ce78e18047cp-55 },
	{ 0x1.33c08b26416ffp+0, 0x1.32721843659a6p-54 },
	{ 0x1.356c55f929ff0p+0, 0x1.928c468ec6e76p-53 },
	{ 0x1.371a7373aa9cap+0, 0x1.4e28aa05e8a8fp-53 },
	{ 0x1.38cae6d05d865p+0, 0x1.0b53961b37da2p-53 },
	{ 0x1.3a7db34e59ff6p+0, 0x1.d43792533c144p-53 },
	{ 0x1.3c32dc313a8e4p+0, 0x1.08003e4516b1ep-53 },
	{ 0x1.3dea64c123422p+0, 0x1.ada0911f09ebcp-55 },
	{ 0x1.3fa4504ac801bp+0, 0x1.417ee03548306p-53 },
	{ 0x1.4160a21f72e29p+0, 0x1.f0864b71e7b6cp-53 },
	{ 0x1.431f5d950a896p+0, 0x1.b8e088728219ap-53 },
	{ 0x1.44e086061892dp+0, 0x1.89b7a04ef80d0p-59 },
	{ 0x1.46a41ed1d0057p+0, 0x1.c944bd1648a76p-54 },
	{ 0x1.486a2b5c13cd0p+0, 0x1.3c1a3b69062f0p-56 },
	{ 0x1.4a32af0d7d3dep+0, 0x1.9cb62f3d1be56p-54 },
	{ 0x1.4bfdad5362a27p+0, 0x1.d4397afec42e2p-56 },
	{ 0x1.4dcb299fddd0dp+0, 0x1.8ecdbbc6a7833p-54 },
	{ 0x1.4f9b2769d2ca6p+0, 0x1.5a67b16d3540ep-53 },
	{ 0x1.516daa2cf6641p+0, 0x1.8225ea5909b04p-53 },
	{ 0x1.5342b569d4f81p+0, 0x1.be1507893b0d5p-53 },
	{ 0x1.551a4ca5d920ep+0, 0x1.8a5d8c4048699p-53 },
	{ 0x1.56f4736b527dap+0, 0x1.9bb2c011d93adp-54 },
	{ 0x1.58d12d497c7fdp+0, 0x1.295e15b9a1de8p-55 },
	{ 0x1.5ab07dd485429p+0, 0x1.6324c054647adp-54 },
	{ 0x1.5c9268a5946b7p+0, 0x1.c4b1b816986a2p-60 },
	{ 0x1.5e76f15ad2148p+0, 0x1.ba6f93080e65ep-54 },
	{ 0x1.605e1b976dc08p+0, 0x1.60edeb25490dcp-53 },
	{ 0x1.6247eb03a5584p+0, 0x1.63e1f40dfa5b5p-53 },
	{ 0x1.6434634ccc31fp+0, 0x1.8edf0e2989db3p-53 },
	{ 0x1.6623882552224p+0, 0x1.224fb3c5371e6p-53 },
	{ 0x1.68155d44ca973p+0, 0x1.038ae44f73e65p-57 },
	{ 0x1.6a09e667f3bccp+0, 0x1.21165f626cdd5p-53 },
	{ 0x1.6c012750bdabep+0, 0x1.daed533001e9ep-53 },
	{ 0x1.6dfb23c651a2ep+0, 0x1.e441c597c3775p-53 },
	{ 0x1.6ff7df9519483p+0, 0x1.9f0fc369e7c42p-53 },
	{ 0x1.71f75e8ec5f73p+0, 0x1.ba46e1e5de15ap-53 },
	{ 0x1.73f9a48a58173p+0, 0x1.7ab9349cd1562p-53 },
	{ 0x1.75feb564267c8p+0, 0x1.7edd354674916p-53 },
	{ 0x1.780694fde5d3fp+0, 0x1.866b80a02162dp-54 },
	{ 0x1.7a11473eb0186p+0, 0x1.afaa2047ed9b4p-53 },
	{ 0x1.7c1ed0130c132p+0, 0x1.f124cd1164dd6p-54 },
	{ 0x1.7e2f336cf4e62p+0, 0x1.05d02ba15797ep-56 },
	{ 0x1.80427543e1a11p+0, 0x1.6c1bccec9346bp-53 },
	{ 0x1.82589994cce12p+0, 0x1.159f115f56694p-53 },
	{ 0x1.8471a4623c7acp+0, 0x1.9ca5ed72f8c81p-53 },
	{ 0x1.868d99b4492ecp+0, 0x1.01c83b21584a3p-53 },
	{ 0x1.88ac7d98a6699p+0, 0x1.994c2f37cb53ap-54 },
	{ 0x1.8ace5422aa0dbp+0, 0x1.6e9f156864b27p-54 },
	{ 0x1.8cf3216b5448bp+0, 0x1.de55439a2c38bp-53 },
	{ 0x1.8f1ae99157736p+0, 0x1.5cc13a2e3976cp-55 },
	{ 0x1.9145b0b91ffc5p+0, 0x1.114c368d3ed6ep-53 },
	{ 0x1.93737b0cdc5e4p+0, 0x1.e8a0387e4a814p-53 },
	{ 0x1.95a44cbc8520ep+0, 0x1.d36906d2b41f9p-53 },
	{ 0x1.97d829fde4e4fp+0, 0x1.173d241f23d18p-53 },
	{ 0x1.9a0f170ca07b9p+0, 0x1.7462137188ce7p-53 },
	{ 0x1.9c49182a3f090p+0, 0x1.c7c46b071f2bep-56 },
	{ 0x1.9e86319e32323p+0, 0x1.824ca78e64c6ep-56 },
	{ 0x1.a0c667b5de564p+0, 0x1.6535b51719567p-53 },
	{ 0x1.a309bec4a2d33p+0, 0x1.6305c7ddc36abp-54 },
	{ 0x1.a5503b23e255cp+0, 0x1.1684892395f0fp-53 },
	{ 0x1.a799e1330b358p+0, 0x1.bcb7ecac563c7p-54 },
	{ 0x1.a9e6b5579fdbfp+0, 0x1.0fac90ef7fd31p-54 },
	{ 0x1.ac36bbfd3f379p+0, 0x1.81b72cd4624ccp-53 },
	{ 0x1.ae89f995ad3adp+0, 0x1.7a1cd345dcc81p-54 },
	{ 0x1.b0e07298db665p+0, 0x1.2108559bf8deep-53 },
	{ 0x1.b33a2b84f15fap+0, 0x1.ed7fa1cf7b290p-53 },
	{ 0x1.b59728de55939p+0, 0x1.1c7102222c90ep-53 },
	{ 0x1.b7f76f2fb5e46p+0, 0x1.d54f610356a79p-53 },
	{ 0x1.ba5b030a10649p+0, 0x1.0819678d5eb69p-53 },
	{ 0x1.bcc1e904bc1d2p+0, 0x1.23dd07a2d9e84p-55 },
	{ 0x1.bf2c25bd71e08p+0, 0x1.0811ae04a31c7p-53 },
	{ 0x1.c199bdd85529cp+0, 0x1.11065895048ddp-55 },
	{ 0x1.c40ab5fffd07ap+0, 0x1.b4537e083c60ap-54 },
	{ 0x1.c67f12e57d14bp+0, 0x1.2884dff483cadp-54 },
	{ 0x1.c8f6d9406e7b5p+0, 0x1.1acbc48805c44p-56 },
	{ 0x1.cb720dcef9069p+0, 0x1.503cbd1e949dbp-56 },
	{ 0x1.cdf0b555dc3f9p+0, 0x1.889f12b1f58a3p-53 },
	{ 0x1.d072d4a07897bp+0, 0x1.1a1e45e4342b2p-53 },
	{ 0x1.d2f87080d89f1p+0, 0x1.15bc247313d44p-53 },
	{ 0x1.d5818dcfba487p+0, 0x1.2ed02d75b3707p-55 },
	{ 0x1.d80e316c98397p+0, 0x1.7709f3a09100cp-53 },
	{ 0x1.da9e603db3285p+0, 0x1.c2300696db532p-54 },
	{ 0x1.dd321f301b460p+0, 0x1.2da5778f018c3p-54 },
	{ 0x1.dfc97337b9b5ep+0, 0x1.72d195873da52p-53 },
	{ 0x1.e264614f5a128p+0, 0x1.424ec3f42f5b5p-53 },
	{ 0x1.e502ee78b3ff6p+0, 0x1.39e8980a9cc8fp-55 },
	{ 0x1.e7a51fbc74c83p+0, 0x1.2d522ca0c8de2p-54 },
	{ 0x1.ea4afa2a490d9p+0, 0x1.0b1ee7431ebb6p-53 },
	{ 0x1.ecf482d8e67f0p+0, 0x1.1b60625f7293ap-53 },
	{ 0x1.efa1bee615a27p+0, 0x1.dc7f486a4b6b0p-54 },
	{ 0x1.f252b376bba97p+0, 0x1.3a1a5bf0d8e43p-54 },
	{ 0x1.f50765b6e4540p+0, 0x1.9d3e12dd8a18bp-54 },
	{ 0x1.f7bfdad9cbe13p+0, 0x1.1227697fce57bp-53 },
	{ 0x1.fa7c1819e90d8p+0, 0x1.74853f3a5931ep-55 },
	{ 0x1.fd3c22b8f71f1p+0, 0x1.2eb74966579e7p-57 }
};

/*
 * Kernel for expl(x).  x must be finite and not tiny or huge.
 * "tiny" is anything that would make us underflow (|A6*x^6| < ~LDBL_MIN).
 * "huge" is anything that would make fn*L1 inexact (|x| > ~2**17*ln2).
 */
static inline void
__k_expl(long double x, long double *hip, long double *lop, int *kp)
{
	long double fn, q, r, r1, r2, t, z;
	int n, n2;

	/* Reduce x to (k*ln2 + endpoint[n2] + r1 + r2). */
	fn = rnintl(x * INV_L);
	r = x - fn * L1 - fn * L2;	/* r = r1 + r2 done independently. */
	n = irint(fn);
	n2 = (unsigned)n % INTERVALS;
	/* Depend on the sign bit being propagated: */
	*kp = n >> LOG2_INTERVALS;
	r1 = x - fn * L1;
	r2 = fn * -L2;

	/* Evaluate expl(endpoint[n2] + r1 + r2) = tbl[n2] * expl(r1 + r2). */
	z = r * r;
#if 0
	q = r2 + z * (A2 + r * A3) + z * z * (A4 + r * A5) + z * z * z * A6;
#else
	q = r2 + z * A2 + z * r * (A3 + r * A4 + z * (A5 + r * A6));
#endif
	t = (long double)tbl[n2].lo + tbl[n2].hi;
	*hip = tbl[n2].hi;
	*lop = tbl[n2].lo + t * (q + r1);
}

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

#elif LDBL_MANT_DIG == 113
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

#endif

static inline long double
inc(long double x)
{
	IEEEl2bits u;

	u.e = x;
	if (++u.bits.manl == 0) {
		if (++u.bits.manh == 0) {
			u.bits.exp++;
			u.bits.manh |= LDBL_NBIT;
		}
	}
	return (u.e);
}

static inline long double
dec(long double x)
{
	IEEEl2bits u;

	u.e = x;
	if (u.bits.manl-- == 0) {
		if (u.bits.manh-- == LDBL_NBIT) {
			u.bits.exp--;
			u.bits.manh |= LDBL_NBIT;
		}
	}
	return (u.e);
}

} // namespace pbsd::lib_msun_src::b0327::detail

namespace pbsd::lib_msun_src::b0327 {

#define one atan_one
#define huge atan_huge
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

/* atan(x)
 * Method
 *   1. Reduce x to positive by atan(x) = -atan(-x).
 *   2. According to the integer k=4t+0.25 chopped, t=x, the argument
 *      is further reduced to one of the following intervals and the
 *      arctangent of t is evaluated by the corresponding formula:
 *
 *      [0,7/16]      atan(x) = t-t^3*(a1+t^2*(a2+...(a10+t^2*a11)...)
 *      [7/16,11/16]  atan(x) = atan(1/2) + atan( (t-0.5)/(1+t/2) )
 *      [11/16.19/16] atan(x) = atan( 1 ) + atan( (t-1)/(1+t) )
 *      [19/16,39/16] atan(x) = atan(3/2) + atan( (t-1.5)/(1+1.5t) )
 *      [39/16,INF]   atan(x) = atan(INF) + atan( -1/t )
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */



static const double atanhi[] = {
  4.63647609000806093515e-01, /* atan(0.5)hi 0x3FDDAC67, 0x0561BB4F */
  7.85398163397448278999e-01, /* atan(1.0)hi 0x3FE921FB, 0x54442D18 */
  9.82793723247329054082e-01, /* atan(1.5)hi 0x3FEF730B, 0xD281F69B */
  1.57079632679489655800e+00, /* atan(inf)hi 0x3FF921FB, 0x54442D18 */
};

static const double atanlo[] = {
  2.26987774529616870924e-17, /* atan(0.5)lo 0x3C7A2B7F, 0x222F65E2 */
  3.06161699786838301793e-17, /* atan(1.0)lo 0x3C81A626, 0x33145C07 */
  1.39033110312309984516e-17, /* atan(1.5)lo 0x3C700788, 0x7AF0CBBD */
  6.12323399573676603587e-17, /* atan(inf)lo 0x3C91A626, 0x33145C07 */
};

static const double aT[] = {
  3.33333333333329318027e-01, /* 0x3FD55555, 0x5555550D */
 -1.99999999998764832476e-01, /* 0xBFC99999, 0x9998EBC4 */
  1.42857142725034663711e-01, /* 0x3FC24924, 0x920083FF */
 -1.11111104054623557880e-01, /* 0xBFBC71C6, 0xFE231671 */
  9.09088713343650656196e-02, /* 0x3FB745CD, 0xC54C206E */
 -7.69187620504482999495e-02, /* 0xBFB3B0F2, 0xAF749A6D */
  6.66107313738753120669e-02, /* 0x3FB10D66, 0xA0D03D51 */
 -5.83357013379057348645e-02, /* 0xBFADDE2D, 0x52DEFD9A */
  4.97687799461593236017e-02, /* 0x3FA97B4B, 0x24760DEB */
 -3.65315727442169155270e-02, /* 0xBFA2B444, 0x2C6A6C2F */
  1.62858201153657823623e-02, /* 0x3F90AD3A, 0xE322DA11 */
};

	static const double
one   = 1.0,
huge   = 1.0e300;

export double
atan(double x)
{
	double w,s1,s2,z;
	std::int32_t ix,hx,id;

	GET_HIGH_WORD(hx,x);
	ix = hx&0x7fffffff;
	if(ix>=0x44100000) {	/* if |x| >= 2^66 */
	    std::uint32_t low;
	    GET_LOW_WORD(low,x);
	    if(ix>0x7ff00000||
		(ix==0x7ff00000&&(low!=0)))
		return x+x;		/* NaN */
	    if(hx>0) return  atanhi[3]+*(volatile double *)&atanlo[3];
	    else     return -atanhi[3]-*(volatile double *)&atanlo[3];
	} if (ix < 0x3fdc0000) {	/* |x| < 0.4375 */
	    if (ix < 0x3e400000) {	/* |x| < 2^-27 */
		if(huge+x>one) return x;	/* raise inexact */
	    }
	    id = -1;
	} else {
	x = fabs(x);
	if (ix < 0x3ff30000) {		/* |x| < 1.1875 */
	    if (ix < 0x3fe60000) {	/* 7/16 <=|x|<11/16 */
		id = 0; x = (2.0*x-one)/(2.0+x);
	    } else {			/* 11/16<=|x|< 19/16 */
		id = 1; x  = (x-one)/(x+one);
	    }
	} else {
	    if (ix < 0x40038000) {	/* |x| < 2.4375 */
		id = 2; x  = (x-1.5)/(one+1.5*x);
	    } else {			/* 2.4375 <= |x| < 2^66 */
		id = 3; x  = -1.0/x;
	    }
	}}
    /* end of argument reduction */
	z = x*x;
	w = z*z;
    /* break sum from i=0 to 10 aT[i]z**(i+1) into odd and even poly */
	s1 = z*(aT[0]+w*(aT[2]+w*(aT[4]+w*(aT[6]+w*(aT[8]+w*aT[10])))));
	s2 = w*(aT[1]+w*(aT[3]+w*(aT[5]+w*(aT[7]+w*aT[9]))));
	if (id<0) return x - x*(s1+s2);
	else {
	    z = atanhi[id] - ((x*(s1+s2) - atanlo[id]) - x);
	    return (hx<0)? -z:z;
	}
}

#undef one
#undef huge

/* from: FreeBSD: head/lib/msun/src/e_sinhl.c XXX */

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
 * See e_sinh.c for complete comments.
 *
 * Converted to long double by Bruce D. Evans.
 */

#ifdef __i386__
#endif


#define	BIAS	(LDBL_MAX_EXP - 1)

static const long double shuge = 0x1p16383L;
#if LDBL_MANT_DIG == 64
/*
 * Domain [-1, 1], range ~[-6.6749e-22, 6.6749e-22]:
 * |sinh(x)/x - s(x)| < 2**-70.3
 */
static const detail::IEEEl2bits
S3u = LD80C(0xaaaaaaaaaaaaaaaa, -3,  1.66666666666666666658e-1L);
#define	S3	S3u.e
static const double
S5  =  8.3333333333333332e-3,		/*  0x11111111111111.0p-59 */
S7  =  1.9841269841270074e-4,		/*  0x1a01a01a01a070.0p-65 */
S9  =  2.7557319223873889e-6,		/*  0x171de3a5565fe6.0p-71 */
S11 =  2.5052108406704084e-8,		/*  0x1ae6456857530f.0p-78 */
S13 =  1.6059042748655297e-10,		/*  0x161245fa910697.0p-85 */
S15 =  7.6470006914396920e-13,		/*  0x1ae7ce4eff2792.0p-93 */
S17 =  2.8346142308424267e-15;		/*  0x19882ce789ffc6.0p-101 */
#elif LDBL_MANT_DIG == 113
/*
 * Domain [-1, 1], range ~[-2.9673e-36, 2.9673e-36]:
 * |sinh(x)/x - s(x)| < 2**-118.0
 */
static const long double
S3  =  1.66666666666666666666666666666666033e-1L,	/*  0x1555555555555555555555555553b.0p-115L */
S5  =  8.33333333333333333333333333337643193e-3L,	/*  0x111111111111111111111111180f5.0p-119L */
S7  =  1.98412698412698412698412697391263199e-4L,	/*  0x1a01a01a01a01a01a01a0176aad11.0p-125L */
S9  =  2.75573192239858906525574406205464218e-6L,	/*  0x171de3a556c7338faac243aaa9592.0p-131L */
S11 =  2.50521083854417187749675637460977997e-8L,	/*  0x1ae64567f544e38fe59b3380d7413.0p-138L */
S13 =  1.60590438368216146368737762431552702e-10L,	/*  0x16124613a86d098059c7620850fc2.0p-145L */
S15 =  7.64716373181980539786802470969096440e-13L,	/*  0x1ae7f3e733b814193af09ce723043.0p-153L */
S17 =  2.81145725434775409870584280722701574e-15L;	/*  0x1952c77030c36898c3fd0b6dfc562.0p-161L */
static const double
S19=  8.2206352435411005e-18,		/*  0x12f49b4662b86d.0p-109 */
S21=  1.9572943931418891e-20,		/*  0x171b8f2fab9628.0p-118 */
S23 =  3.8679983530666939e-23,		/*  0x17617002b73afc.0p-127 */
S25 =  6.5067867911512749e-26;		/*  0x1423352626048a.0p-136 */
#else
#error "Unsupported long double format"
#endif /* LDBL_MANT_DIG == 64 */

/* log(2**16385 - 0.5) rounded up: */
static const float
o_threshold =  1.13572168e4;		/*  0xb174de.0p-10 */

export long double
sinhl(long double x)
{
	long double hi,lo,x2,x4;
#if LDBL_MANT_DIG == 113
	double dx2;
#endif
	double s;
	std::int16_t ix,jx;

	GET_LDBL_EXPSIGN(jx,x);
	ix = jx&0x7fff;

    /* x is INF or NaN */
	if(ix>=0x7fff) return x+x;

	ENTERI();

	s = 1;
	if (jx<0) s = -1;

    /* |x| < 64, return x, s(x), or accurate s*(exp(|x|)/2-1/exp(|x|)/2) */
	if (ix<0x4005) {		/* |x|<64 */
	    if (ix<BIAS-(LDBL_MANT_DIG+1)/2) 	/* |x|<TINY */
		if(shuge+x>1) RETURNI(x);  /* sinh(tiny) = tiny with inexact */
	    if (ix<0x3fff) {		/* |x|<1 */
		x2 = x*x;
#if LDBL_MANT_DIG == 64
		x4 = x2*x2;
		RETURNI(((S17*x2 + S15)*x4 + (S13*x2 + S11))*(x2*x*x4*x4) +
		    ((S9*x2 + S7)*x2 + S5)*(x2*x*x2) + S3*(x2*x) + x);
#elif LDBL_MANT_DIG == 113
		dx2 = x2;
		RETURNI(((((((((((S25*dx2 + S23)*dx2 +
		    S21)*x2 + S19)*x2 +
		    S17)*x2 + S15)*x2 + S13)*x2 + S11)*x2 + S9)*x2 + S7)*x2 +
		    S5)* (x2*x*x2) +
		    S3*(x2*x) + x);
#endif
	    }
	    detail::k_hexpl(fabsl(x), &hi, &lo);
	    RETURNI(s*(lo - 0.25/(hi + lo) + hi));
	}

    /* |x| in [64, o_threshold], return correctly-overflowing s*exp(|x|)/2 */
	if (fabsl(x) <= o_threshold)
	    RETURNI(s*detail::hexpl(fabsl(x)));

    /* |x| > o_threshold, sinh(x) overflow */
	return x*shuge;
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2005 David Schultz <das@FreeBSD.ORG>
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



#define	TBLBITS	4
#define	TBLSIZE	(1 << TBLBITS)

static const float
    redux   = 0x1.8p23f / TBLSIZE,
    P1	    = 0x1.62e430p-1f,
    P2	    = 0x1.ebfbe0p-3f,
    P3	    = 0x1.c6b348p-5f,
    P4	    = 0x1.3b2c9cp-7f;

static volatile float
    huge    = 0x1p100f,
    twom100 = 0x1p-100f;

static const double exp2ft[TBLSIZE] = {
	0x1.6a09e667f3bcdp-1,
	0x1.7a11473eb0187p-1,
	0x1.8ace5422aa0dbp-1,
	0x1.9c49182a3f090p-1,
	0x1.ae89f995ad3adp-1,
	0x1.c199bdd85529cp-1,
	0x1.d5818dcfba487p-1,
	0x1.ea4afa2a490dap-1,
	0x1.0000000000000p+0,
	0x1.0b5586cf9890fp+0,
	0x1.172b83c7d517bp+0,
	0x1.2387a6e756238p+0,
	0x1.306fe0a31b715p+0,
	0x1.3dea64c123422p+0,
	0x1.4bfdad5362a27p+0,
	0x1.5ab07dd485429p+0,
};
	
/*
 * exp2f(x): compute the base 2 exponential of x
 *
 * Accuracy: Peak error < 0.501 ulp; location of peak: -0.030110927.
 *
 * Method: (equally-spaced tables)
 *
 *   Reduce x:
 *     x = k + y, for integer k and |y| <= 1/2.
 *     Thus we have exp2f(x) = 2**k * exp2(y).
 *
 *   Reduce y:
 *     y = i/TBLSIZE + z for integer i near y * TBLSIZE.
 *     Thus we have exp2(y) = exp2(i/TBLSIZE) * exp2(z),
 *     with |z| <= 2**-(TBLSIZE+1).
 *
 *   We compute exp2(i/TBLSIZE) via table lookup and exp2(z) via a
 *   degree-4 minimax polynomial with maximum error under 1.4 * 2**-33.
 *   Using double precision for everything except the reduction makes
 *   roundoff error insignificant and simplifies the scaling step.
 *
 *   This method is due to Tang, but I do not use his suggested parameters:
 *
 *	Tang, P.  Table-driven Implementation of the Exponential Function
 *	in IEEE Floating-Point Arithmetic.  TOMS 15(2), 144-157 (1989).
 */
export float
exp2f(float x)
{
	double tv, twopk, u, z;
	float t;
	std::uint32_t hx, ix, i0;
	std::int32_t k;

	/* Filter out exceptional cases. */
	GET_FLOAT_WORD(hx, x);
	ix = hx & 0x7fffffff;		/* high word of |x| */
	if(ix >= 0x43000000) {			/* |x| >= 128 */
		if(ix >= 0x7f800000) {
			if ((ix & 0x7fffff) != 0 || (hx & 0x80000000) == 0)
				return (x + x);	/* x is NaN or +Inf */
			else 
				return (0.0);	/* x is -Inf */
		}
		if(x >= 0x1.0p7f)
			return (huge * huge);	/* overflow */
		if(x <= -0x1.2cp7f)
			return (twom100 * twom100); /* underflow */
	} else if (ix <= 0x33000000) {		/* |x| <= 0x1p-25 */
		return (1.0f + x);
	}

	/* Reduce x, computing z, i0, and k. */
	STRICT_ASSIGN(float, t, x + redux);
	GET_FLOAT_WORD(i0, t);
	i0 += TBLSIZE / 2;
	k = (i0 >> TBLBITS) << 20;
	i0 &= TBLSIZE - 1;
	t -= redux;
	z = x - t;
	INSERT_WORDS(twopk, 0x3ff00000 + k, 0);

	/* Compute r = exp2(y) = exp2ft[i0] * p(z). */
	tv = exp2ft[i0];
	u = tv * z;
	tv = tv + u * (P1 + z * P2) + u * (z * z) * (P3 + z * P4);

	/* Scale by 2**(k>>20). */
	return (tv * twopk);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2007 Steven G. Kargl
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



#pragma STDC FENV_ACCESS ON

/*
 * This is slow, but simple and portable. You should use hardware sqrt
 * if possible.
 */

export long double
sqrtl(long double x)
{
	detail::IEEEl2bits u;
	int k, r;
	long double lo, xn;
	fenv_t env;

	u.e = x;

	/* If x = NaN, then sqrt(x) = NaN. */
	/* If x = Inf, then sqrt(x) = Inf. */
	/* If x = -Inf, then sqrt(x) = NaN. */
	if (u.bits.exp == LDBL_MAX_EXP * 2 - 1)
		return (x * x + x);

	/* If x = +-0, then sqrt(x) = +-0. */
	if ((u.bits.manh | u.bits.manl | u.bits.exp) == 0)
		return (x);

	/* If x < 0, then raise invalid and return NaN */
	if (u.bits.sign)
		return ((x - x) / (x - x));

	feholdexcept(&env);

	if (u.bits.exp == 0) {
		/* Adjust subnormal numbers. */
		u.e *= 0x1.0p514;
		k = -514;
	} else {
		k = 0;
	}
	/*
	 * u.e is a normal number, so break it into u.e = e*2^n where
	 * u.e = (2*e)*2^2k for odd n and u.e = (4*e)*2^2k for even n.
	 */
	if ((u.bits.exp - 0x3ffe) & 1) {	/* n is odd.     */
		k += u.bits.exp - 0x3fff;	/* 2k = n - 1.   */
		u.bits.exp = 0x3fff;		/* u.e in [1,2). */
	} else {
		k += u.bits.exp - 0x4000;	/* 2k = n - 2.   */
		u.bits.exp = 0x4000;		/* u.e in [2,4). */
	}

	/*
	 * Newton's iteration.
	 * Split u.e into a high and low part to achieve additional precision.
	 */
	xn = sqrt(u.e);			/* 53-bit estimate of sqrtl(x). */
#if LDBL_MANT_DIG > 100
	xn = (xn + (u.e / xn)) * 0.5;	/* 106-bit estimate. */
#endif
	lo = u.e;
	u.bits.manl = 0;		/* Zero out lower bits. */
	lo = (lo - u.e) / xn;		/* Low bits divided by xn. */
	xn = xn + (u.e / xn);		/* High portion of estimate. */
	u.e = xn + lo;			/* Combine everything. */
	u.bits.exp += (k >> 1) - 1;

	feclearexcept(FE_INEXACT);
	r = fegetround();
	fesetround(FE_TOWARDZERO);	/* Set to round-toward-zero. */
	xn = x / u.e;			/* Chopped quotient (inexact?). */

	if (!fetestexcept(FE_INEXACT)) { /* Quotient is exact. */
		if (xn == u.e) {
			fesetenv(&env);
			return (u.e);
		}
		/* Round correctly for inputs like x = y**2 - ulp. */
		xn = detail::dec(xn);		/* xn = xn - ulp. */
	}

	if (r == FE_TONEAREST) {
		xn = detail::inc(xn);		/* xn = xn + ulp. */
	} else if (r == FE_UPWARD) {
		u.e = detail::inc(u.e);		/* u.e = u.e + ulp. */
		xn = detail::inc(xn);		/* xn  = xn + ulp. */
	}
	u.e = u.e + xn;				/* Chopped sum. */
	feupdateenv(&env);	/* Restore env and raise inexact */
	u.bits.exp--;
	return (u.e);
}

} // namespace pbsd::lib_msun_src::b0327
