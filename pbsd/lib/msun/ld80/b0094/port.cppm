// PBSD port of HardenedBSD lib/msun/ld80 -- batch b0094.
//
// Sources ported here (faithfully, bug-for-bug):
//   lib/msun/ld80/k_tanl.c   -- __kernel_tanl()
//   lib/msun/ld80/b_expl.c   -- __exp__D()
//   lib/msun/ld80/s_exp2l.c  -- exp2l()
//
// lib/msun/ld80/s_expl.c is not ported here; see skipped.txt.

export module pbsd.lib.msun.ld80.b0094;

#include <cmath>
#include <cstdint>
#include <cfloat>

namespace pbsd::lib_msun_ld80::b0094 {

#define BIAS (LDBL_MAX_EXP - 1)

#define ENTERI()
#define RETURNI(x) return (x)

union IEEEl2bits {
	long double e;
	struct {
		std::uint64_t man;
		std::uint16_t expsign;
		std::uint32_t manl;
		std::uint32_t manh;
	} xbits;
	struct {
		std::uint32_t manl;
		std::uint32_t manh;
		std::uint16_t expsign;
		std::uint32_t pad;
	} bits;
};

#define LD80C(man, exp, val) { .e = (val) }

} /* namespace pbsd::lib_msun_ld80::b0094 */

export namespace pbsd::lib_msun_ld80::b0094 {

/*
 * ====================================================
 * Copyright 2004 Sun Microsystems, Inc.  All Rights Reserved.
 * Copyright (c) 2008 Steven G. Kargl, David Schultz, Bruce D. Evans.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * ld80 version of k_tan.c.  See ../src/k_tan.c for most comments.
 */

/*
 * Domain [-0.67434, 0.67434], range ~[-2.25e-22, 1.921e-22]
 * |tan(x)/x - t(x)| < 2**-71.9
 *
 * See k_cosl.c for more details about the polynomial.
 */
#if defined(__amd64__) || defined(__i386__)
/* Long double constants are slow on these arches, and broken on i386. */
static const volatile double
T3hi =  0.33333333333333331,		/*  0x15555555555555.0p-54 */
T3lo =  1.8350121769317163e-17,		/*  0x15280000000000.0p-108 */
T5hi =  0.13333333333333336,		/*  0x11111111111112.0p-55 */
T5lo =  1.3051083651294260e-17,		/*  0x1e180000000000.0p-109 */
T7hi =  0.053968253968250494,		/*  0x1ba1ba1ba1b827.0p-57 */
T7lo =  3.1509625637859973e-18,		/*  0x1d100000000000.0p-111 */
pio4_hi =  0.78539816339744828,		/*  0x1921fb54442d18.0p-53 */
pio4_lo =  3.0628711372715500e-17,	/*  0x11a80000000000.0p-107 */
pio4lo_hi = -1.2541394031670831e-20,	/* -0x1d9cceba3f91f2.0p-119 */
pio4lo_lo =  6.1493048227390915e-37;	/*  0x1a280000000000.0p-173 */
#define	T3	((long double)T3hi + T3lo)
#define	T5	((long double)T5hi + T5lo)
#define	T7	((long double)T7hi + T7lo)
#define	pio4	((long double)pio4_hi + pio4_lo)
#define	pio4lo	((long double)pio4lo_hi + pio4lo_lo)
#else
static const long double
T3 =   0.333333333333333333180L,	/*  0xaaaaaaaaaaaaaaa5.0p-65 */
T5 =   0.133333333333333372290L,	/*  0x88888888888893c3.0p-66 */
T7 =   0.0539682539682504975744L,	/*  0xdd0dd0dd0dc13ba2.0p-68 */
pio4 = 0.785398163397448309628L,	/*  0xc90fdaa22168c235.0p-64 */
pio4lo = -1.25413940316708300586e-20L;	/* -0xece675d1fc8f8cbb.0p-130 */
#endif

static const double
T9  =  0.021869488536312216,		/*  0x1664f4882cc1c2.0p-58 */
T11 =  0.0088632355256619590,		/*  0x1226e355c17612.0p-59 */
T13 =  0.0035921281113786528,		/*  0x1d6d3d185d7ff8.0p-61 */
T15 =  0.0014558334756312418,		/*  0x17da354aa3f96b.0p-62 */
T17 =  0.00059003538700862256,		/*  0x13559358685b83.0p-63 */
T19 =  0.00023907843576635544,		/*  0x1f56242026b5be.0p-65 */
T21 =  0.000097154625656538905,		/*  0x1977efc26806f4.0p-66 */
T23 =  0.000038440165747303162,		/*  0x14275a09b3ceac.0p-67 */
T25 =  0.000018082171885432524,		/*  0x12f5e563e5487e.0p-68 */
T27 =  0.0000024196006108814377,	/*  0x144c0d80cc6896.0p-71 */
T29 =  0.0000078293456938132840,	/*  0x106b59141a6cb3.0p-69 */
T31 = -0.0000032609076735050182,	/* -0x1b5abef3ba4b59.0p-71 */
T33 =  0.0000023261313142559411;	/*  0x13835436c0c87f.0p-71 */

long double
__kernel_tanl(long double x, long double y, int iy) {
	long double z, r, v, w, s;
	long double osign;
	int i;

	iy = (iy == 1 ? -1 : 1);	/* XXX recover original interface */
	osign = (x >= 0 ? 1.0 : -1.0);	/* XXX slow, probably wrong for -0 */
	if (fabsl(x) >= 0.67434) {
		if (x < 0) {
			x = -x;
			y = -y;
		}
		z = pio4 - x;
		w = pio4lo - y;
		x = z + w;
		y = 0.0;
		i = 1;
	} else
		i = 0;
	z = x * x;
	w = z * z;
	r = T5 + w * (T9 + w * (T13 + w * (T17 + w * (T21 +
	    w * (T25 + w * (T29 + w * T33))))));
	v = z * (T7 + w * (T11 + w * (T15 + w * (T19 + w * (T23 +
	    w * (T27 + w * T31))))));
	s = z * x;
	r = y + z * (s * (r + v) + y);
	r += T3 * s;
	w = x + r;
	if (i == 1) {
		v = (long double) iy;
		return osign *
			(v - 2.0 * (x - (w * w / (w + v) - r)));
	}
	if (iy == 1)
		return w;
	else {
		/*
		 * if allow error up to 2 ulp, simply return
		 * -1.0 / (x+r) here
		 */
		/* compute -1.0 / (x+r) accurately */
		long double a, t;
		z = w;
		z = z + 0x1p32 - 0x1p32;
		v = r - (z - x);	/* z+v = r+x */
		t = a = -1.0 / w;	/* a = -1.0/w */
		t = t + 0x1p32 - 0x1p32;
		s = 1.0 + t * z;
		return t + a * (s + t * v);
	}
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1985, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * See bsdsrc/b_exp.c for implementation details.
 *
 * bsdrc/b_exp.c converted to long double by Steven G. Kargl.
 */

static const union IEEEl2bits
    p0u = LD80C(0xaaaaaaaaaaaaaaab,    -3,  1.66666666666666666671e-01L),
    p1u = LD80C(0xb60b60b60b60b59a,    -9, -2.77777777777777775377e-03L),
    p2u = LD80C(0x8ab355e008a3cfce,   -14,  6.61375661375629297465e-05L),
    p3u = LD80C(0xddebbc994b0c1376,   -20, -1.65343915327882529784e-06L),
    p4u = LD80C(0xb354784cb4ef4c41,   -25,  4.17535101591534118469e-08L),
    p5u = LD80C(0x913e8a718382ce75,   -30, -1.05679137034774806475e-09L),
    p6u = LD80C(0xe8f0042aa134502e,   -36,  2.64819349895429516863e-11L);
#define	p1	(p0u.e)
#define	p2	(p1u.e)
#define	p3	(p2u.e)
#define	p4	(p3u.e)
#define	p5	(p4u.e)
#define	p6	(p5u.e)
#define	p7	(p6u.e)

/*
 * lnhuge = (LDBL_MAX_EXP + 9) * log(2.)
 * lntiny = (LDBL_MIN_EXP - 64 - 10) * log(2.)
 * invln2 = 1 / log(2.)
 */
static const union IEEEl2bits
ln2hiu  = LD80C(0xb17217f700000000,  -1,  6.93147180369123816490e-01L),
ln2lou  = LD80C(0xd1cf79abc9e3b398, -33,  1.90821492927058781614e-10L),
lnhugeu = LD80C(0xb18b0c0330a8fad9,  13,  1.13627617309191834574e+04L),
lntinyu = LD80C(0xb236f28a68bc3bd7,  13, -1.14057368561139000667e+04L),
invln2u = LD80C(0xb8aa3b295c17f0bc,   0,  1.44269504088896340739e+00L);
#define	ln2hi	(ln2hiu.e)
#define ln2lo	(ln2lou.e)
#define lnhuge	(lnhugeu.e)
#define	lntiny	(lntinyu.e)
#define	invln2	(invln2u.e)

/* returns exp(r = x + c) for |c| < |x| with no overlap.  */

long double
__exp__D(long double x, long double c)
{
	long double hi, lo, z;
	int k;

	if (x != x)	/* x is NaN. */
		return(x);

	if (x <= lnhuge) {
		if (x >= lntiny) {
			/* argument reduction: x --> x - k*ln2 */
			z = invln2 * x;
			k = z + copysignl(0.5L, x);

		    	/*
			 * Express (x + c) - k * ln2 as hi - lo.
			 * Let x = hi - lo rounded.
			 */
			hi = x - k * ln2hi;	/* Exact. */
			lo = k * ln2lo - c;
			x = hi - lo;

			/* Return 2^k*[1+x+x*c/(2+c)]  */
			z = x * x;
			c = x - z * (p1 + z * (p2 + z * (p3 + z * (p4 +
			    z * (p5 + z * (p6 + z * p7))))));
			c = (x * c) / (2 - c);

			return (ldexpl(1 + (hi - (lo - c)), k));
		} else {
			/* exp(-INF) is 0. exp(-big) underflows to 0.  */
			return (isfinite(x) ? ldexpl(1., -5000) : 0);
		}
	} else
		/* exp(INF) is INF, exp(+big#) overflows to INF */
		return (isfinite(x) ? ldexpl(1., 5000) : x);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2005-2008 David Schultz <das@FreeBSD.ORG>
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

#define	TBLBITS	7
#define	TBLSIZE	(1 << TBLBITS)

static volatile long double
    huge = 0x1p10000L,
    twom10000 = 0x1p-10000L;

static const union IEEEl2bits
P1 = LD80C(0xb17217f7d1cf79ac, -1, 6.93147180559945309429e-1L);

static const double
redux = 0x1.8p63 / TBLSIZE,
/*
 * Domain [-0.00390625, 0.00390625], range ~[-1.7079e-23, 1.7079e-23]
 * |exp(x) - p(x)| < 2**-75.6
 */
P2 = 2.4022650695910072e-1,		/*  0x1ebfbdff82c58f.0p-55 */
P3 = 5.5504108664816879e-2,		/*  0x1c6b08d7049e1a.0p-57 */
P4 = 9.6181291055695180e-3,		/*  0x13b2ab6fa8321a.0p-59 */
P5 = 1.3333563089183052e-3,		/*  0x15d8806f67f251.0p-62 */
P6 = 1.5413361552277414e-4;		/*  0x1433ddacff3441.0p-65 */

static const double tbl[TBLSIZE * 2] = {
	0x1.6a09e667f3bcdp-1,	-0x1.bdd3413b2648p-55,
	0x1.6c012750bdabfp-1,	-0x1.2895667ff0cp-57,
	0x1.6dfb23c651a2fp-1,	-0x1.bbe3a683c88p-58,
	0x1.6ff7df9519484p-1,	-0x1.83c0f25860fp-56,
	0x1.71f75e8ec5f74p-1,	-0x1.16e4786887bp-56,
	0x1.73f9a48a58174p-1,	-0x1.0a8d96c65d5p-55,
	0x1.75feb564267c9p-1,	-0x1.0245957316ep-55,
	0x1.780694fde5d3fp-1,	 0x1.866b80a0216p-55,
	0x1.7a11473eb0187p-1,	-0x1.41577ee0499p-56,
	0x1.7c1ed0130c132p-1,	 0x1.f124cd1164ep-55,
	0x1.7e2f336cf4e62p-1,	 0x1.05d02ba157ap-57,
	0x1.80427543e1a12p-1,	-0x1.27c86626d97p-55,
	0x1.82589994cce13p-1,	-0x1.d4c1dd41533p-55,
	0x1.8471a4623c7adp-1,	-0x1.8d684a341cep-56,
	0x1.868d99b4492edp-1,	-0x1.fc6f89bd4f68p-55,
	0x1.88ac7d98a6699p-1,	 0x1.994c2f37cb5p-55,
	0x1.8ace5422aa0dbp-1,	 0x1.6e9f156864bp-55,
	0x1.8cf3216b5448cp-1,	-0x1.0d55e32e9e4p-57,
	0x1.8f1ae99157736p-1,	 0x1.5cc13a2e397p-56,
	0x1.9145b0b91ffc6p-1,	-0x1.dd6792e5825p-55,
	0x1.93737b0cdc5e5p-1,	-0x1.75fc781b58p-58,
	0x1.95a44cbc8520fp-1,	-0x1.64b7c96a5fp-57,
	0x1.97d829fde4e5p-1,	-0x1.d185b7c1b86p-55,
	0x1.9a0f170ca07bap-1,	-0x1.173bd91cee6p-55,
	0x1.9c49182a3f09p-1,	 0x1.c7c46b071f2p-57,
	0x1.9e86319e32323p-1,	 0x1.824ca78e64cp-57,
	0x1.a0c667b5de565p-1,	-0x1.359495d1cd5p-55,
	0x1.a309bec4a2d33p-1,	 0x1.6305c7ddc368p-55,
	0x1.a5503b23e255dp-1,	-0x1.d2f6edb8d42p-55,
	0x1.a799e1330b358p-1,	 0x1.bcb7ecac564p-55,
	0x1.a9e6b5579fdbfp-1,	 0x1.0fac90ef7fdp-55,
	0x1.ac36bbfd3f37ap-1,	-0x1.f9234cae76dp-56,
	0x1.ae89f995ad3adp-1,	 0x1.7a1cd345dcc8p-55,
	0x1.b0e07298db666p-1,	-0x1.bdef54c80e4p-55,
	0x1.b33a2b84f15fbp-1,	-0x1.2805e3084d8p-58,
	0x1.b59728de5593ap-1,	-0x1.c71dfbbba6ep-55,
	0x1.b7f76f2fb5e47p-1,	-0x1.5584f7e54acp-57,
	0x1.ba5b030a1064ap-1,	-0x1.efcd30e5429p-55,
	0x1.bcc1e904bc1d2p-1,	 0x1.23dd07a2d9fp-56,
	0x1.bf2c25bd71e09p-1,	-0x1.efdca3f6b9c8p-55,
	0x1.c199bdd85529cp-1,	 0x1.11065895049p-56,
	0x1.c40ab5fffd07ap-1,	 0x1.b4537e083c6p-55,
	0x1.c67f12e57d14bp-1,	 0x1.2884dff483c8p-55,
	0x1.c8f6d9406e7b5p-1,	 0x1.1acbc48805cp-57,
	0x1.cb720dcef9069p-1,	 0x1.503cbd1e94ap-57,
	0x1.cdf0b555dc3fap-1,	-0x1.dd83b53829dp-56,
	0x1.d072d4a07897cp-1,	-0x1.cbc3743797a8p-55,
	0x1.d2f87080d89f2p-1,	-0x1.d487b719d858p-55,
	0x1.d5818dcfba487p-1,	 0x1.2ed02d75b37p-56,
	0x1.d80e316c98398p-1,	-0x1.11ec18bedep-55,
	0x1.da9e603db3285p-1,	 0x1.c2300696db5p-55,
	0x1.dd321f301b46p-1,	 0x1.2da5778f019p-55,
	0x1.dfc97337b9b5fp-1,	-0x1.1a5cd4f184b8p-55,
	0x1.e264614f5a129p-1,	-0x1.7b627817a148p-55,
	0x1.e502ee78b3ff6p-1,	 0x1.39e8980a9cdp-56,
	0x1.e7a51fbc74c83p-1,	 0x1.2d522ca0c8ep-55,
	0x1.ea4afa2a490dap-1,	-0x1.e9c23179c288p-55,
	0x1.ecf482d8e67f1p-1,	-0x1.c93f3b411ad8p-55,
	0x1.efa1bee615a27p-1,	 0x1.dc7f486a4b68p-55,
	0x1.f252b376bba97p-1,	 0x1.3a1a5bf0d8e8p-55,
	0x1.f50765b6e454p-1,	 0x1.9d3e12dd8a18p-55,
	0x1.f7bfdad9cbe14p-1,	-0x1.dbb12d00635p-55,
	0x1.fa7c1819e90d8p-1,	 0x1.74853f3a593p-56,
	0x1.fd3c22b8f71f1p-1,	 0x1.2eb74966578p-58,
	0x1p+0,	 0x0p+0,
	0x1.0163da9fb3335p+0,	 0x1.b61299ab8cd8p-54,
	0x1.02c9a3e778061p+0,	-0x1.19083535b08p-56,
	0x1.04315e86e7f85p+0,	-0x1.0a31c1977c98p-54,
	0x1.059b0d3158574p+0,	 0x1.d73e2a475b4p-55,
	0x1.0706b29ddf6dep+0,	-0x1.c91dfe2b13cp-55,
	0x1.0874518759bc8p+0,	 0x1.186be4bb284p-57,
	0x1.09e3ecac6f383p+0,	 0x1.14878183161p-54,
	0x1.0b5586cf9890fp+0,	 0x1.8a62e4adc61p-54,
	0x1.0cc922b7247f7p+0,	 0x1.01edc16e24f8p-54,
	0x1.0e3ec32d3d1a2p+0,	 0x1.03a1727c58p-59,
	0x1.0fb66affed31bp+0,	-0x1.b9bedc44ebcp-57,
	0x1.11301d0125b51p+0,	-0x1.6c51039449bp-54,
	0x1.12abdc06c31ccp+0,	-0x1.1b514b36ca8p-58,
	0x1.1429aaea92dep+0,	-0x1.32fbf9af1368p-54,
	0x1.15a98c8a58e51p+0,	 0x1.2406ab9eeabp-55,
	0x1.172b83c7d517bp+0,	-0x1.19041b9d78ap-55,
	0x1.18af9388c8deap+0,	-0x1.11023d1970f8p-54,
	0x1.1a35beb6fcb75p+0,	 0x1.e5b4c7b4969p-55,
	0x1.1bbe084045cd4p+0,	-0x1.95386352ef6p-54,
	0x1.1d4873168b9aap+0,	 0x1.e016e00a264p-54,
	0x1.1ed5022fcd91dp+0,	-0x1.1df98027bb78p-54,
	0x1.2063b88628cd6p+0,	 0x1.dc775814a85p-55,
	0x1.21f49917ddc96p+0,	 0x1.2a97e9494a6p-55,
	0x1.2387a6e756238p+0,	 0x1.9b07eb6c7058p-54,
	0x1.251ce4fb2a63fp+0,	 0x1.ac155bef4f5p-55,
	0x1.26b4565e27cddp+0,	 0x1.2bd339940eap-55,
	0x1.284dfe1f56381p+0,	-0x1.a4c3a8c3f0d8p-54,
	0x1.29e9df51fdee1p+0,	 0x1.612e8afad12p-55,
	0x1.2b87fd0dad99p+0,	-0x1.10adcd6382p-59,
	0x1.2d285a6e4030bp+0,	 0x1.0024754db42p-54,
	0x1.2ecafa93e2f56p+0,	 0x1.1ca0f45d524p-56,
	0x1.306fe0a31b715p+0,	 0x1.6f46ad23183p-55,
	0x1.32170fc4cd831p+0,	 0x1.a9ce78e1804p-55,
	0x1.33c08b26416ffp+0,	 0x1.327218436598p-54,
	0x1.356c55f929ff1p+0,	-0x1.b5cee5c4e46p-55,
	0x1.371a7373aa9cbp+0,	-0x1.63aeabf42ebp-54,
	0x1.38cae6d05d866p+0,	-0x1.e958d3c99048p-54,
	0x1.3a7db34e59ff7p+0,	-0x1.5e436d661f6p-56,
	0x1.3c32dc313a8e5p+0,	-0x1.efff8375d2ap-54,
	0x1.3dea64c123422p+0,	 0x1.ada0911f09fp-55,
	0x1.3fa4504ac801cp+0,	-0x1.7d023f956fap-54,
	0x1.4160a21f72e2ap+0,	-0x1.ef3691c309p-58,
	0x1.431f5d950a897p+0,	-0x1.1c7dde35f7ap-55,
	0x1.44e086061892dp+0,	 0x1.89b7a04ef8p-59,
	0x1.46a41ed1d0057p+0,	 0x1.c944bd1648a8p-54,
	0x1.486a2b5c13cdp+0,	 0x1.3c1a3b69062p-56,
	0x1.4a32af0d7d3dep+0,	 0x1.9cb62f3d1be8p-54,
	0x1.4bfdad5362a27p+0,	 0x1.d4397afec42p-56,
	0x1.4dcb299fddd0dp+0,	 0x1.8ecdbbc6a78p-54,
	0x1.4f9b2769d2ca7p+0,	-0x1.4b309d25958p-54,
	0x1.516daa2cf6642p+0,	-0x1.f768569bd94p-55,
	0x1.5342b569d4f82p+0,	-0x1.07abe1db13dp-55,
	0x1.551a4ca5d920fp+0,	-0x1.d689cefede6p-55,
	0x1.56f4736b527dap+0,	 0x1.9bb2c011d938p-54,
	0x1.58d12d497c7fdp+0,	 0x1.295e15b9a1ep-55,
	0x1.5ab07dd485429p+0,	 0x1.6324c0546478p-54,
	0x1.5c9268a5946b7p+0,	 0x1.c4b1b81698p-60,
	0x1.5e76f15ad2148p+0,	 0x1.ba6f93080e68p-54,
	0x1.605e1b976dc09p+0,	-0x1.3e2429b56de8p-54,
	0x1.6247eb03a5585p+0,	-0x1.383c17e40b48p-54,
	0x1.6434634ccc32p+0,	-0x1.c483c759d89p-55,
	0x1.6623882552225p+0,	-0x1.bb60987591cp-54,
	0x1.68155d44ca973p+0,	 0x1.038ae44f74p-57,
};

/**
 * Compute the base 2 exponential of x for Intel 80-bit format.
 */
long double
exp2l(long double x)
{
	union IEEEl2bits u, v;
	long double r, twopk, twopkp10000, z;
	std::uint32_t hx, ix, i0;
	int k;

	/* Filter out exceptional cases. */
	u.e = x;
	hx = u.xbits.expsign;
	ix = hx & 0x7fff;
	if (ix >= BIAS + 14) {		/* |x| >= 16384 or x is NaN */
		if (ix == BIAS + LDBL_MAX_EXP) {
			if (hx & 0x8000 && u.xbits.man == 1ULL << 63)
				return (0.0L);	/* x is -Inf */
			return (x + x); /* x is +Inf, NaN or unsupported */
		}
		if (x >= 16384)
			return (huge * huge);	/* overflow */
		if (x <= -16446)
			return (twom10000 * twom10000);	/* underflow */
	} else if (ix <= BIAS - 66) {	/* |x| < 0x1p-65 (includes pseudos) */
		return (1.0L + x);	/* 1 with inexact */
	}

	ENTERI();

	/*
	 * Reduce x, computing z, i0, and k. The low bits of x + redux
	 * contain the 16-bit integer part of the exponent (k) followed by
	 * TBLBITS fractional bits (i0). We use bit tricks to extract these
	 * as integers, then set z to the remainder.
	 */
	u.e = x + redux;
	i0 = u.bits.manl + TBLSIZE / 2;
	k = (int)i0 >> TBLBITS;
	i0 = (i0 & (TBLSIZE - 1)) << 1;
	u.e -= redux;
	z = x - u.e;
	v.xbits.man = 1ULL << 63;
	if (k >= LDBL_MIN_EXP) {
		v.xbits.expsign = BIAS + k;
		twopk = v.e;
	} else {
		v.xbits.expsign = BIAS + k + 10000;
		twopkp10000 = v.e;
	}

	/* Compute r = exp2l(y) = exp2lt[i0] * p(z). */
	long double t_hi = tbl[i0];
	long double t_lo = tbl[i0 + 1];
	r = t_lo + (t_hi + t_lo) * z * (P1.e + z * (P2 + z * (P3 + z * (P4
	    + z * (P5 + z * P6))))) + t_hi;

	/* Scale by 2**k. */
	if (k >= LDBL_MIN_EXP) {
		if (k == LDBL_MAX_EXP)
			RETURNI(r * 2.0 * 0x1p16383L);
		RETURNI(r * twopk);
	} else {
		RETURNI(r * twopkp10000 * twom10000);
	}
}

} /* namespace pbsd::lib_msun_ld80::b0094 */
