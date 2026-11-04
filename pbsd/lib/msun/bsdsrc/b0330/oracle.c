/*
 * PBSD batch b0330 oracle: concatenated HardenedBSD lib/msun/bsdsrc sources
 * with every function renamed ref_*.  Bodies are otherwise unmodified.
 *
 * Sources: b_log.c, b_exp.c, b_tgamma.c (includes inlined, not duplicated).
 */

#define _GNU_SOURCE
#include <float.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef LONG_BIT
#define LONG_BIT (8 * sizeof(long))
#endif

double ref_tgamma(double);

double
sinpi(double x)
{
	return sin(M_PI * x);
}

double
cospi(double x)
{
	return cos(M_PI * x);
}

/* Used in b_log.c and below. */
struct Double {
	double a;
	double b;
};

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992, 1993
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

/* Table-driven natural logarithm.
 *
 * This code was derived, with minor modifications, from:
 *	Peter Tang, "Table-Driven Implementation of the
 *	Logarithm in IEEE Floating-Point arithmetic." ACM Trans.
 *	Math Software, vol 16. no 4, pp 378-400, Dec 1990).
 *
 * Calculates log(2^m*F*(1+f/F)), |f/F| <= 1/256,
 * where F = j/128 for j an integer in [0, 128].
 *
 * log(2^m) = log2_hi*m + log2_tail*m
 * The leading term is exact, because m is an integer,
 * m has at most 10 digits (for subnormal numbers),
 * and log2_hi has 11 trailing zero bits.
 *
 * log(F) = logF_hi[j] + logF_lo[j] is in table below.
 * logF_hi[] + 512 is exact.
 *
 * log(1+f/F) = 2*f/(2*F + f) + 1/12 * (2*f/(2*F + f))**3 + ...
 *
 * The leading term is calculated to extra precision in two
 * parts, the larger of which adds exactly to the dominant
 * m and F terms.
 *
 * There are two cases:
 *	1. When m and j are non-zero (m | j), use absolute
 *	   precision for the leading term.
 *	2. When m = j = 0, |1-x| < 1/256, and log(x) ~= (x-1).
 *	   In this case, use a relative precision of 24 bits.
 * (This is done differently in the original paper)
 *
 * Special cases:
 *	0	return signalling -Inf
 *	neg	return signalling NaN
 *	+Inf	return +Inf
 */

#define N 128

/*
 * Coefficients in the polynomial approximation of log(1+f/F).
 * Domain of x is [0,1./256] with 2**(-64.187) precision.
 */
static const double
    A1 =  8.3333333333333329e-02, /* 0x3fb55555, 0x55555555 */
    A2 =  1.2499999999943598e-02, /* 0x3f899999, 0x99991a98 */
    A3 =  2.2321527525957776e-03; /* 0x3f624929, 0xe24e70be */

/*
 * Table of log(Fj) = logF_head[j] + logF_tail[j], for Fj = 1+j/128.
 * Used for generation of extend precision logarithms.
 * The constant 35184372088832 is 2^45, so the divide is exact.
 * It ensures correct reading of logF_head, even for inaccurate
 * decimal-to-binary conversion routines.  (Everybody gets the
 * right answer for integers less than 2^53.)
 * Values for log(F) were generated using error < 10^-57 absolute
 * with the bc -l package.
 */
static double logF_head[N+1] = {
	0.,
	.007782140442060381246,
	.015504186535963526694,
	.023167059281547608406,
	.030771658666765233647,
	.038318864302141264488,
	.045809536031242714670,
	.053244514518837604555,
	.060624621816486978786,
	.067950661908525944454,
	.075223421237524235039,
	.082443669210988446138,
	.089612158689760690322,
	.096729626458454731618,
	.103796793681567578460,
	.110814366340264314203,
	.117783035656430001836,
	.124703478501032805070,
	.131576357788617315236,
	.138402322859292326029,
	.145182009844575077295,
	.151916042025732167530,
	.158605030176659056451,
	.165249572895390883786,
	.171850256926518341060,
	.178407657472689606947,
	.184922338493834104156,
	.191394852999565046047,
	.197825743329758552135,
	.204215541428766300668,
	.210564769107350002741,
	.216873938300523150246,
	.223143551314024080056,
	.229374101064877322642,
	.235566071312860003672,
	.241719936886966024758,
	.247836163904594286577,
	.253915209980732470285,
	.259957524436686071567,
	.265963548496984003577,
	.271933715484010463114,
	.277868451003087102435,
	.283768173130738432519,
	.289633292582948342896,
	.295464212893421063199,
	.301261330578199704177,
	.307025035294827830512,
	.312755710004239517729,
	.318453731118097493890,
	.324119468654316733591,
	.329753286372579168528,
	.335355541920762334484,
	.340926586970454081892,
	.346466767346100823488,
	.351976423156884266063,
	.357455888922231679316,
	.362905493689140712376,
	.368325561158599157352,
	.373716409793814818840,
	.379078352934811846353,
	.384411698910298582632,
	.389716751140440464951,
	.394993808240542421117,
	.400243164127459749579,
	.405465108107819105498,
	.410659924985338875558,
	.415827895143593195825,
	.420969294644237379543,
	.426084395310681429691,
	.431173464818130014464,
	.436236766774527495726,
	.441274560805140936281,
	.446287102628048160113,
	.451274644139630254358,
	.456237433481874177232,
	.461175715122408291790,
	.466089729924533457960,
	.470979715219073113985,
	.475845904869856894947,
	.480688529345570714212,
	.485507815781602403149,
	.490303988045525329653,
	.495077266798034543171,
	.499827869556611403822,
	.504556010751912253908,
	.509261901790523552335,
	.513945751101346104405,
	.518607764208354637958,
	.523248143765158602036,
	.527867089620485785417,
	.532464798869114019908,
	.537041465897345915436,
	.541597282432121573947,
	.546132437597407260909,
	.550647117952394182793,
	.555141507540611200965,
	.559615787935399566777,
	.564070138285387656651,
	.568504735352689749561,
	.572919753562018740922,
	.577315365035246941260,
	.581691739635061821900,
	.586049045003164792433,
	.590387446602107957005,
	.594707107746216934174,
	.599008189645246602594,
	.603290851438941899687,
	.607555250224322662688,
	.611801541106615331955,
	.616029877215623855590,
	.620240409751204424537,
	.624433288012369303032,
	.628608659422752680256,
	.632766669570628437213,
	.636907462236194987781,
	.641031179420679109171,
	.645137961373620782978,
	.649227946625615004450,
	.653301272011958644725,
	.657358072709030238911,
	.661398482245203922502,
	.665422632544505177065,
	.669430653942981734871,
	.673422675212350441142,
	.677398823590920073911,
	.681359224807238206267,
	.685304003098281100392,
	.689233281238557538017,
	.693147180560117703862
};

static double logF_tail[N+1] = {
	0.,
	-.00000000000000543229938420049,
	 .00000000000000172745674997061,
	-.00000000000001323017818229233,
	-.00000000000001154527628289872,
	-.00000000000000466529469958300,
	 .00000000000005148849572685810,
	-.00000000000002532168943117445,
	-.00000000000005213620639136504,
	-.00000000000001819506003016881,
	 .00000000000006329065958724544,
	 .00000000000008614512936087814,
	-.00000000000007355770219435028,
	 .00000000000009638067658552277,
	 .00000000000007598636597194141,
	 .00000000000002579999128306990,
	-.00000000000004654729747598444,
	-.00000000000007556920687451336,
	 .00000000000010195735223708472,
	-.00000000000017319034406422306,
	-.00000000000007718001336828098,
	 .00000000000010980754099855238,
	-.00000000000002047235780046195,
	-.00000000000008372091099235912,
	 .00000000000014088127937111135,
	 .00000000000012869017157588257,
	 .00000000000017788850778198106,
	 .00000000000006440856150696891,
	 .00000000000016132822667240822,
	-.00000000000007540916511956188,
	-.00000000000000036507188831790,
	 .00000000000009120937249914984,
	 .00000000000018567570959796010,
	-.00000000000003149265065191483,
	-.00000000000009309459495196889,
	 .00000000000017914338601329117,
	-.00000000000001302979717330866,
	 .00000000000023097385217586939,
	 .00000000000023999540484211737,
	 .00000000000015393776174455408,
	-.00000000000036870428315837678,
	 .00000000000036920375082080089,
	-.00000000000009383417223663699,
	 .00000000000009433398189512690,
	 .00000000000041481318704258568,
	-.00000000000003792316480209314,
	 .00000000000008403156304792424,
	-.00000000000034262934348285429,
	 .00000000000043712191957429145,
	-.00000000000010475750058776541,
	-.00000000000011118671389559323,
	 .00000000000037549577257259853,
	 .00000000000013912841212197565,
	 .00000000000010775743037572640,
	 .00000000000029391859187648000,
	-.00000000000042790509060060774,
	 .00000000000022774076114039555,
	 .00000000000010849569622967912,
	-.00000000000023073801945705758,
	 .00000000000015761203773969435,
	 .00000000000003345710269544082,
	-.00000000000041525158063436123,
	 .00000000000032655698896907146,
	-.00000000000044704265010452446,
	 .00000000000034527647952039772,
	-.00000000000007048962392109746,
	 .00000000000011776978751369214,
	-.00000000000010774341461609578,
	 .00000000000021863343293215910,
	 .00000000000024132639491333131,
	 .00000000000039057462209830700,
	-.00000000000026570679203560751,
	 .00000000000037135141919592021,
	-.00000000000017166921336082431,
	-.00000000000028658285157914353,
	-.00000000000023812542263446809,
	 .00000000000006576659768580062,
	-.00000000000028210143846181267,
	 .00000000000010701931762114254,
	 .00000000000018119346366441110,
	 .00000000000009840465278232627,
	-.00000000000033149150282752542,
	-.00000000000018302857356041668,
	-.00000000000016207400156744949,
	 .00000000000048303314949553201,
	-.00000000000071560553172382115,
	 .00000000000088821239518571855,
	-.00000000000030900580513238244,
	-.00000000000061076551972851496,
	 .00000000000035659969663347830,
	 .00000000000035782396591276383,
	-.00000000000046226087001544578,
	 .00000000000062279762917225156,
	 .00000000000072838947272065741,
	 .00000000000026809646615211673,
	-.00000000000010960825046059278,
	 .00000000000002311949383800537,
	-.00000000000058469058005299247,
	-.00000000000002103748251144494,
	-.00000000000023323182945587408,
	-.00000000000042333694288141916,
	-.00000000000043933937969737844,
	 .00000000000041341647073835565,
	 .00000000000006841763641591466,
	 .00000000000047585534004430641,
	 .00000000000083679678674757695,
	-.00000000000085763734646658640,
	 .00000000000021913281229340092,
	-.00000000000062242842536431148,
	-.00000000000010983594325438430,
	 .00000000000065310431377633651,
	-.00000000000047580199021710769,
	-.00000000000037854251265457040,
	 .00000000000040939233218678664,
	 .00000000000087424383914858291,
	 .00000000000025218188456842882,
	-.00000000000003608131360422557,
	-.00000000000050518555924280902,
	 .00000000000078699403323355317,
	-.00000000000067020876961949060,
	 .00000000000016108575753932458,
	 .00000000000058527188436251509,
	-.00000000000035246757297904791,
	-.00000000000018372084495629058,
	 .00000000000088606689813494916,
	 .00000000000066486268071468700,
	 .00000000000063831615170646519,
	 .00000000000025144230728376072,
	-.00000000000017239444525614834
};
/*
 * Extra precision variant, returning struct {double a, b;};
 * log(x) = a+b to 63 bits, with 'a' rounded to 24 bits.
 */
struct Double
ref___log__D(double x)
{
	int m, j;
	double F, f, g, q, u, v, u1, u2;
	struct Double r;

	/*
	 * Argument reduction: 1 <= g < 2; x/2^m = g;
	 * y = F*(1 + f/F) for |f| <= 2^-8
	 */
	g = frexp(x, &m);
	g *= 2;
	m--;
	if (m == -1022) {
		j = ilogb(g);
		m += j;
		g = ldexp(g, -j);
	}
	j = N * (g - 1) + 0.5;
	F = (1. / N) * j + 1;
	f = g - F;

	g = 1 / (2 * F + f);
	u = 2 * f * g;
	v = u * u;
	q = u * v * (A1 + v * (A2 + v * A3));
	if (m | j) {
		u1 = u + 513;
		u1 -= 513;
	} else {
		u1 = (float)u;
	}
	u2 = (2 * (f - F * u1) - u1 * f) * g;

	u1 += m * logF_head[N] + logF_head[j];

	u2 += logF_tail[j];
	u2 += q;
	u2 += logF_tail[N] * m;
	r.a = (float)(u1 + u2);		/* Only difference is here. */
	r.b = (u1 - r.a) + u2;
	return (r);
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

/* EXP(X)
 * RETURN THE EXPONENTIAL OF X
 * DOUBLE PRECISION (IEEE 53 bits, VAX D FORMAT 56 BITS)
 * CODED IN C BY K.C. NG, 1/19/85;
 * REVISED BY K.C. NG on 2/6/85, 2/15/85, 3/7/85, 3/24/85, 4/16/85, 6/14/86.
 *
 * Required system supported functions:
 *	ldexp(x,n)
 *	copysign(x,y)
 *	isfinite(x)
 *
 * Method:
 *	1. Argument Reduction: given the input x, find r and integer k such
 *	   that
 *	        x = k*ln2 + r,  |r| <= 0.5*ln2.
 *	   r will be represented as r := z+c for better accuracy.
 *
 *	2. Compute exp(r) by
 *
 *		exp(r) = 1 + r + r*R1/(2-R1),
 *	   where
 *		R1 = x - x^2*(p1+x^2*(p2+x^2*(p3+x^2*(p4+p5*x^2)))).
 *
 *	3. exp(x) = 2^k * exp(r) .
 *
 * Special cases:
 *	exp(INF) is INF, exp(NaN) is NaN;
 *	exp(-INF)=  0;
 *	for finite argument, only exp(0)=1 is exact.
 *
 * Accuracy:
 *	exp(x) returns the exponential of x nearly rounded. In a test run
 *	with 1,156,000 random arguments on a VAX, the maximum observed
 *	error was 0.869 ulps (units in the last place).
 */
static const double
    p1 =  1.6666666666666660e-01, /* 0x3fc55555, 0x55555553 */
    p2 = -2.7777777777564776e-03, /* 0xbf66c16c, 0x16c0ac3c */
    p3 =  6.6137564717940088e-05, /* 0x3f11566a, 0xb5c2ba0d */
    p4 = -1.6534060280704225e-06, /* 0xbebbbd53, 0x273e8fb7 */
    p5 =  4.1437773411069054e-08; /* 0x3e663f2a, 0x09c94b6c */

static const double
    ln2hi = 0x1.62e42fee00000p-1,   /* High 32 bits round-down. */
    ln2lo = 0x1.a39ef35793c76p-33;  /* Next 53 bits round-to-nearst. */

static const double
    lnhuge =  0x1.6602b15b7ecf2p9,  /* (DBL_MAX_EXP + 9) * log(2.) */
    lntiny = -0x1.77af8ebeae354p9,  /* (DBL_MIN_EXP - 53 - 10) * log(2.) */
    invln2 =  0x1.71547652b82fep0;  /* 1 / log(2.) */

/* returns exp(r = x + c) for |c| < |x| with no overlap.  */

double
ref___exp__D(double x, double c)
{
	double hi, lo, z;
	int k;

	if (x != x)	/* x is NaN. */
		return(x);

	if (x <= lnhuge) {
		if (x >= lntiny) {
			/* argument reduction: x --> x - k*ln2 */
			z = invln2 * x;
			k = z + copysign(0.5, x);

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
			    z * p5))));
			c = (x * c) / (2 - c);

			return (ldexp(1 + (hi - (lo - c)), k));
		} else {
			/* exp(-INF) is 0. exp(-big) underflows to 0.  */
			return (isfinite(x) ? ldexp(1., -5000) : 0);
		}
	} else
	/* exp(INF) is INF, exp(+big#) overflows to INF */
		return (isfinite(x) ? ldexp(1., 5000) : x);
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992, 1993
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
 * The original code, FreeBSD's old svn r93211, contained the following
 * attribution:
 *
 *    This code by P. McIlroy, Oct 1992;
 *
 *    The financial support of UUNET Communications Services is greatfully
 *    acknowledged.
 *
 *  The algorithm remains, but the code has been re-arranged to facilitate
 *  porting to other precisions.
 */



/*
 * The range is broken into several subranges.  Each is handled by its
 * helper functions.
 *
 *         x >=   6.0: ref_large_gam(x)
 *   6.0 > x >= xleft: ref_small_gam(x) where xleft = 1 + left + x0.
 * xleft > x >   iota: ref_smaller_gam(x) where iota = 1e-17.
 *  iota > x >  -itoa: Handle x near 0.
 * -iota > x         : ref_neg_gam
 *
 * Special values:
 *	-Inf:			return NaN and raise invalid;
 *	negative integer:	return NaN and raise invalid;
 *	other x ~< 177.79:	return +-0 and raise underflow;
 *	+-0:			return +-Inf and raise divide-by-zero;
 *	finite x ~> 171.63:	return +Inf and raise overflow;
 *	+Inf:			return +Inf;
 *	NaN: 			return NaN.
 *
 * Accuracy: ref_tgamma(x) is accurate to within
 *	x > 0:  error provably < 0.9ulp.
 *	Maximum observed in 1,000,000 trials was .87ulp.
 *	x < 0:
 *	Maximum observed error < 4ulp in 1,000,000 trials.
 */

/*
 * Constants for large x approximation (x in [6, Inf])
 * (Accurate to 2.8*10^-19 absolute)
 */

static const double zero = 0.;
static const volatile double tiny = 1e-300;
/*
 * x >= 6
 *
 * Use the asymptotic approximation (Stirling's formula) adjusted fof
 * equal-ripples:
 *
 * log(G(x)) ~= (x-0.5)*(log(x)-1) + 0.5(log(2*pi)-1) + 1/x*P(1/(x*x))
 *
 * Keep extra precision in multiplying (x-.5)(log(x)-1), to avoid
 * premature round-off.
 *
 * Accurate to max(ulp(1/128) absolute, 2^-66 relative) error.
 */
static const double
    ln2pi_hi =  0.41894531250000000,
    ln2pi_lo = -6.7792953272582197e-6,
    Pa0 =  8.3333333333333329e-02, /* 0x3fb55555, 0x55555555 */
    Pa1 = -2.7777777777735404e-03, /* 0xbf66c16c, 0x16c145ec */
    Pa2 =  7.9365079044114095e-04, /* 0x3f4a01a0, 0x183de82d */
    Pa3 = -5.9523715464225254e-04, /* 0xbf438136, 0x0e681f62 */
    Pa4 =  8.4161391899445698e-04, /* 0x3f4b93f8, 0x21042a13 */
    Pa5 = -1.9065246069191080e-03, /* 0xbf5f3c8b, 0x357cb64e */
    Pa6 =  5.9047708485785158e-03, /* 0x3f782f99, 0xdaf5d65f */
    Pa7 = -1.6484018705183290e-02; /* 0xbf90e12f, 0xc4fb4df0 */

struct Double
ref_large_gam(double x)
{
	double p, z, thi, tlo, xhi, xlo;
	struct Double u;

	z = 1 / (x * x);
	p = Pa0 + z * (Pa1 + z * (Pa2 + z * (Pa3 + z * (Pa4 + z * (Pa5 +
	    z * (Pa6 + z * Pa7))))));
	p = p / x;

	u = ref___log__D(x);
	u.a -= 1;

	/* Split (x - 0.5) in high and low parts. */
	x -= 0.5;
	xhi = (float)x;
	xlo = x - xhi;

	/* Compute  t = (x-.5)*(log(x)-1) in extra precision. */
	thi = xhi * u.a;
	tlo = xlo * u.a + x * u.b;

	/* Compute thi + tlo + ln2pi_hi + ln2pi_lo + p. */
	tlo += ln2pi_lo;
	tlo += p;
	u.a = ln2pi_hi + tlo;
	u.a += thi;
	u.b = thi - u.a;
	u.b += ln2pi_hi;
	u.b += tlo;
	return (u);
}
/*
 * Rational approximation, A0 + x * x * P(x) / Q(x), on the interval
 * [1.066.., 2.066..] accurate to 4.25e-19.
 *
 * Returns r.a + r.b = a0 + (z + c)^2 * p / q, with r.a truncated.
 */
static const double
#if 0
    a0_hi =  8.8560319441088875e-1,
    a0_lo = -4.9964270364690197e-17,
#else
    a0_hi =  8.8560319441088875e-01, /* 0x3fec56dc, 0x82a74aef */
    a0_lo = -4.9642368725563397e-17, /* 0xbc8c9deb, 0xaa64afc3 */
#endif
    P0 =  6.2138957182182086e-1,
    P1 =  2.6575719865153347e-1,
    P2 =  5.5385944642991746e-3,
    P3 =  1.3845669830409657e-3,
    P4 =  2.4065995003271137e-3,
    Q0 =  1.4501953125000000e+0,
    Q1 =  1.0625852194801617e+0,
    Q2 = -2.0747456194385994e-1,
    Q3 = -1.4673413178200542e-1,
    Q4 =  3.0787817615617552e-2,
    Q5 =  5.1244934798066622e-3,
    Q6 = -1.7601274143166700e-3,
    Q7 =  9.3502102357378894e-5,
    Q8 =  6.1327550747244396e-6;

struct Double
ref_ratfun_gam(double z, double c)
{
	double p, q, thi, tlo;
	struct Double r;

	q = Q0 + z * (Q1 + z * (Q2 + z * (Q3 + z * (Q4 + z * (Q5 + 
	    z * (Q6 + z * (Q7 + z * Q8)))))));
	p = P0 + z * (P1 + z * (P2 + z * (P3 + z * P4)));
	p = p / q;

	/* Split z into high and low parts. */
	thi = (float)z;
	tlo = (z - thi) + c;
	tlo *= (thi + z);

	/* Split (z+c)^2 into high and low parts. */
	thi *= thi;
	q = thi;
	thi = (float)thi;
	tlo += (q - thi);

	/* Split p/q into high and low parts. */
	r.a = (float)p;
	r.b = p - r.a;

	tlo = tlo * p + thi * r.b + a0_lo;
	thi *= r.a;				/* t = (z+c)^2*(P/Q) */
	r.a = (float)(thi + a0_hi);
	r.b = ((a0_hi - r.a) + thi) + tlo;
	return (r);				/* r = a0 + t */
}
/*
 * x < 6
 *
 * Use argument reduction G(x+1) = xG(x) to reach the range [1.066124,
 * 2.066124].  Use a rational approximation centered at the minimum
 * (x0+1) to ensure monotonicity.
 *
 * Good to < 1 ulp.  (provably .90 ulp; .87 ulp on 1,000,000 runs.)
 * It also has correct monotonicity.
 */
static const double
    left = -0.3955078125,	/* left boundary for rat. approx */
    x0 = 4.6163214496836236e-1;	/* xmin - 1 */

double
ref_small_gam(double x)
{
	double t, y, ym1;
	struct Double yy, r;

	y = x - 1;
	if (y <= 1 + (left + x0)) {
		yy = ref_ratfun_gam(y - x0, 0);
		return (yy.a + yy.b);
	}

	r.a = (float)y;
	yy.a = r.a - 1;
	y = y - 1 ;
	r.b = yy.b = y - yy.a;

	/* Argument reduction: G(x+1) = x*G(x) */
	for (ym1 = y - 1; ym1 > left + x0; y = ym1--, yy.a--) {
		t = r.a * yy.a;
		r.b = r.a * yy.b + y * r.b;
		r.a = (float)t;
		r.b += (t - r.a);
	}

	/* Return r*ref_tgamma(y). */
	yy = ref_ratfun_gam(y - x0, 0);
	y = r.b * (yy.a + yy.b) + r.a * yy.b;
	y += yy.a * r.a;
	return (y);
}
/*
 * Good on (0, 1+x0+left].  Accurate to 1 ulp.
 */
double
ref_smaller_gam(double x)
{
	double d, t, xhi, xlo;
	struct Double r;

	if (x < x0 + left) {
		t = (float)x;
		d = (t + x) * (x - t);
		t *= t;
		xhi = (float)(t + x);
		xlo = x - xhi;
		xlo += t;
		xlo += d;
		t = 1 - x0;
		t += x;
		d = 1 - x0;
		d -= t;
		d += x;
		x = xhi + xlo;
	} else {
		xhi = (float)x;
		xlo = x - xhi;
		t = x - x0;
		d = - x0 - t;
		d += x;
	}

	r = ref_ratfun_gam(t, d);
	d = (float)(r.a / x);
	r.a -= d * xhi;
	r.a -= d * xlo;
	r.a += r.b;

	return (d + r.a / x);
}
/*
 * x < 0
 *
 * Use reflection formula, G(x) = pi/(sin(pi*x)*x*G(x)).
 * At negative integers, return NaN and raise invalid.
 */
double
ref_neg_gam(double x)
{
	int sgn = 1;
	struct Double lg, lsine;
	double y, z;

	y = ceil(x);
	if (y == x)		/* Negative integer. */
		return ((x - x) / zero);

	z = y - x;
	if (z > 0.5)
		z = 1 - z;

	y = y / 2;
	if (y == ceil(y))
		sgn = -1;

	if (z < 0.25)
		z = sinpi(z);
	else
		z = cospi(0.5 - z);

	/* Special case: G(1-x) = Inf; G(x) may be nonzero. */
	if (x < -170) {

		if (x < -190)
			return (sgn * tiny * tiny);

		y = 1 - x;			/* exact: 128 < |x| < 255 */
		lg = ref_large_gam(y);
		lsine = ref___log__D(M_PI / z);	/* = TRUNC(log(u)) + small */
		lg.a -= lsine.a;		/* exact (opposite signs) */
		lg.b -= lsine.b;
		y = -(lg.a + lg.b);
		z = (y + lg.a) + lg.b;
		y = ref___exp__D(y, z);
		if (sgn < 0) y = -y;
		return (y);
	}

	y = 1 - x;
	if (1 - y == x)
		y = ref_tgamma(y);
	else		/* 1-x is inexact */
		y = - x * ref_tgamma(-x);

	if (sgn < 0) y = -y;
	return (M_PI / (y * z));
}
/*
 * xmax comes from lgamma(xmax) - emax * log(2) = 0.
 * static const float  xmax = 35.040095f
 * static const double xmax = 171.624376956302725;
 * ld80: LD80C(0xdb718c066b352e20, 10, 1.75554834290446291689e+03L),
 * ld128: 1.75554834290446291700388921607020320e+03L,
 *
 * iota is a sloppy threshold to isolate x = 0.
 */
static const double xmax = 171.624376956302725;
static const double iota = 0x1p-56;

double
ref_tgamma(double x)
{
	struct Double u;

	if (x >= 6) {
		if (x > xmax)
			return (x / zero);
		u = ref_large_gam(x);
		return (ref___exp__D(u.a, u.b));
	}

	if (x >= 1 + left + x0)
		return (ref_small_gam(x));

	if (x > iota)
		return (ref_smaller_gam(x));

	if (x > -iota) {
		if (x != 0.)
			u.a = 1 - tiny;	/* raise inexact */
		return (1 / x);
	}

	if (!isfinite(x))
		return (x - x);		/* x is NaN or -Inf */

	return (ref_neg_gam(x));
}
