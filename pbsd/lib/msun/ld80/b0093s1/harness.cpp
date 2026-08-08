/*
 * PBSD batch b0093s1 -- differential test.
 *
 * Drives the C++23 port (module pbsd.lib.msun.ld80.b0093s1) and the
 * unmodified C oracle (oracle.c, ref_cexpl) with identical inputs and
 * compares the results bit for bit.
 *
 * cexpl() takes and returns a long double complex; it writes to no caller
 * supplied buffer and returns no pointer, so the buffer-guard and
 * pointer-offset protocols do not apply.  What takes their place is that
 * nothing is compared numerically: both halves of the result are compared
 * as raw 80 bit patterns (64 mantissa bits + 16 expsign bits), so the sign
 * of a zero, the payload of a NaN and a one-ulp difference all count as
 * failures.  Inputs are likewise built from raw 80 bit patterns, so that
 * signed zeroes, subnormals, pseudo-denormals, unnormals, pseudo-infinities
 * and signalling NaNs can all be fed in exactly.
 *
 * Coverage is aimed at every branch and every arithmetic expression that
 * the port can reach:
 *
 *   - the (hy|ly)==0 early out, on both sides, including y = -0.0, which is
 *     the only input that distinguishes it from the general path;
 *   - the ((hx&0x7fff)|lx)==0 early out, on both sides;
 *   - hy >= 0x7fff and each of its three sub-branches, with +-Inf, +-NaN
 *     and NaNs carrying distinct payloads for x;
 *   - the exp_ovfl/cexp_ovfl test, at lx exactly on each threshold and one
 *     ulp either side of it, for both 0x400c and 0x400d, and with a few
 *     thousand different y per threshold so that a flipped comparison has
 *     to survive thousands of chances to differ;
 *   - the whole of __k_expl()/__ldexp_cexpl(), by sweeping x uniformly over
 *     the scaling window (11356.5234, 22755.3288) so that all 128 table
 *     entries are hit and the reduced argument r covers its full range,
 *     where even the highest order polynomial term is worth several ulps.
 */

#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.ld80.b0093s1;

namespace port = pbsd::lib_msun_ld80::b0093s1;

extern "C" _Complex long double ref_cexpl(_Complex long double);

typedef _Complex long double ldcomplex;

static_assert(sizeof(long double) == 16, "expected x87 80 bit long double");
static_assert(sizeof(ldcomplex) == 32, "expected 2 x 16 byte complex layout");

/* An 80 bit long double as raw bits: 64 mantissa bits and 16 expsign bits. */
struct Val {
	std::uint64_t	man;
	std::uint16_t	es;
};

struct Parts {
	std::uint64_t	re_man, re_es, im_man, im_es;
};

static ldcomplex
mkz(Val x, Val y)
{
	unsigned char raw[sizeof(ldcomplex)];
	ldcomplex z;

	std::memset(raw, 0, sizeof raw);
	std::memcpy(raw + 0, &x.man, 8);
	std::memcpy(raw + 8, &x.es, 2);
	std::memcpy(raw + 16, &y.man, 8);
	std::memcpy(raw + 24, &y.es, 2);
	std::memcpy(&z, raw, sizeof z);
	return z;
}

static Parts
parts(ldcomplex z)
{
	unsigned char raw[sizeof(ldcomplex)];
	std::uint64_t m;
	std::uint16_t e;
	Parts p;

	std::memcpy(raw, &z, sizeof z);
	std::memcpy(&m, raw + 0, 8);
	std::memcpy(&e, raw + 8, 2);
	p.re_man = m;
	p.re_es = e;
	std::memcpy(&m, raw + 16, 8);
	std::memcpy(&e, raw + 24, 2);
	p.im_man = m;
	p.im_es = e;
	return p;
}

/* ------------------------------------------------------------------ */
/* bookkeeping							      */
/* ------------------------------------------------------------------ */

enum {
	G_SPECIAL,
	G_THRESHOLD,
	G_GENERAL,
	G_WINDOW,
	G_NEARTHRESH,
	G_INFNAN_Y,
	G_ZEROS,
	G_RAWBITS,
	NGROUPS
};

static const char *const group_name[NGROUPS] = {
	"hand: special x special",
	"hand: threshold x y sweep",
	"rand: general x, general y",
	"rand: x in scaling window",
	"rand: x near thresholds",
	"rand: y = Inf/NaN",
	"rand: x or y = +-0",
	"rand: raw 80 bit patterns",
};

static unsigned long long g_cases[NGROUPS];
static unsigned long long g_fails[NGROUPS];
static int reported;

static void
run(int g, Val xv, Val yv)
{
	ldcomplex z = mkz(xv, yv);
	ldcomplex got = port::cexpl(z);
	ldcomplex want = ref_cexpl(z);
	Parts a = parts(got);
	Parts b = parts(want);

	g_cases[g]++;
	if (a.re_man == b.re_man && a.re_es == b.re_es &&
	    a.im_man == b.im_man && a.im_es == b.im_es)
		return;

	g_fails[g]++;
	if (reported < 25) {
		reported++;
		std::printf("FAIL [%s]\n"
		    "  x    = %04llx:%016llx  y    = %04llx:%016llx\n"
		    "  port = %04llx:%016llx , %04llx:%016llx\n"
		    "  ref  = %04llx:%016llx , %04llx:%016llx\n",
		    group_name[g],
		    (unsigned long long)xv.es, (unsigned long long)xv.man,
		    (unsigned long long)yv.es, (unsigned long long)yv.man,
		    (unsigned long long)a.re_es, (unsigned long long)a.re_man,
		    (unsigned long long)a.im_es, (unsigned long long)a.im_man,
		    (unsigned long long)b.re_es, (unsigned long long)b.re_man,
		    (unsigned long long)b.im_es, (unsigned long long)b.im_man);
	}
}

/* ------------------------------------------------------------------ */
/* fixed seed PRNG (splitmix64)					      */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state;

static std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

/* ------------------------------------------------------------------ */
/* input generators						      */
/* ------------------------------------------------------------------ */

/* exp_ovfl and cexp_ovfl, as the mantissa bits s_cexpl.c compares against */
static const std::uint64_t EXP_OVFL_MAN = 0xb17217f7d1cf79acULL;	/* hx 0x400c */
static const std::uint64_t CEXP_OVFL_MAN = 0xb1c6a8573de9768cULL;	/* hx 0x400d */

/* A finite normal value: exponent uniform in [elo,ehi], random sign, random
   mantissa with the x87 explicit integer bit set. */
static Val
gen_normal(unsigned elo, unsigned ehi)
{
	Val v;
	std::uint64_t r = rnd();

	v.man = rnd() | 0x8000000000000000ULL;
	v.es = (std::uint16_t)(elo + (unsigned)(r % (ehi - elo + 1)));
	if (r & 0x8000000000000000ULL)
		v.es = (std::uint16_t)(v.es | 0x8000u);
	return v;
}

/* x strictly inside the scaling window (exp_ovfl, cexp_ovfl). */
static Val
gen_window(void)
{
	Val v;
	std::uint64_t r = rnd();

	if (rnd() & 1) {
		v.es = 0x400c;
		v.man = (EXP_OVFL_MAN + 1) +
		    r % (0xffffffffffffffffULL - EXP_OVFL_MAN);
	} else {
		v.es = 0x400d;
		v.man = 0x8000000000000000ULL +
		    r % (CEXP_OVFL_MAN - 0x8000000000000000ULL);
	}
	return v;
}

/* x with an expsign of 0x400c/0x400d/0xc00c/0xc00d and a mantissa within a
   few thousand ulps of one of the two thresholds. */
static Val
gen_near_threshold(void)
{
	static const std::uint16_t esv[4] = { 0x400c, 0x400d, 0xc00c, 0xc00d };
	std::uint64_t r = rnd();
	std::uint64_t base = (r & 1) ? EXP_OVFL_MAN : CEXP_OVFL_MAN;
	std::int64_t d = (std::int64_t)(rnd() % 8192) - 4096;
	Val v;

	v.es = esv[(r >> 1) & 3];
	v.man = base + (std::uint64_t)d;
	return v;
}

/* +-Inf or a +-NaN with an arbitrary payload. */
static Val
gen_infnan(void)
{
	std::uint64_t r = rnd();
	Val v;

	v.es = (std::uint16_t)(0x7fff | ((r & 1) ? 0x8000u : 0u));
	switch ((r >> 1) % 5) {
	case 0:	v.man = 0x8000000000000000ULL; break;		/* Inf */
	case 1:	v.man = 0xc000000000000000ULL; break;		/* default qNaN */
	case 2:	v.man = 0x8000000000000001ULL; break;		/* sNaN */
	case 3:	v.man = rnd() | 0xc000000000000000ULL; break;	/* qNaN */
	default: v.man = rnd(); break;				/* anything */
	}
	return v;
}

/* Any 80 bit pattern at all, including unnormals and pseudo-denormals. */
static Val
gen_raw(void)
{
	Val v;

	v.man = rnd();
	v.es = (std::uint16_t)(rnd() & 0xffffu);
	return v;
}

/* ------------------------------------------------------------------ */
/* hand written edge cases					      */
/* ------------------------------------------------------------------ */

static const Val specials[] = {
	/* zeroes -- the sign of the zero is observable through the
	   (hy|ly)==0 early out, which returns y unchanged */
	{ 0x0000000000000000ULL, 0x0000 },	/* +0 */
	{ 0x0000000000000000ULL, 0x8000 },	/* -0 */

	/* subnormals: smallest, a high-bit-bytes one, largest */
	{ 0x0000000000000001ULL, 0x0000 },
	{ 0x0000000000000001ULL, 0x8000 },
	{ 0x00000000000000ffULL, 0x0000 },
	{ 0x7fffffffffffffffULL, 0x0000 },
	{ 0x7fffffffffffffffULL, 0x8000 },

	/* smallest normal, and one ulp above it */
	{ 0x8000000000000000ULL, 0x0001 },
	{ 0x8000000000000001ULL, 0x0001 },

	/* small but ordinary */
	{ 0x8000000000000000ULL, 0x3fbf },	/* 2^-64 */
	{ 0x8000000000000000ULL, 0x3ffe },	/* 0.5 */
	{ 0xbfffffffffffffffULL, 0xbffe },	/* -0.7499... */

	/* around one */
	{ 0x8000000000000000ULL, 0x3fff },	/* 1 */
	{ 0x8000000000000001ULL, 0x3fff },	/* 1+ulp */
	{ 0xffffffffffffffffULL, 0x3fff },	/* 2-ulp */
	{ 0x8000000000000000ULL, 0xbfff },	/* -1 */

	/* pi and friends, to move sincosl off the axes */
	{ 0xc90fdaa22168c235ULL, 0x4000 },	/* pi */
	{ 0xc90fdaa22168c235ULL, 0xc000 },	/* -pi */
	{ 0xc90fdaa22168c235ULL, 0x3fff },	/* pi/2 */
	{ 0xc90fdaa22168c235ULL, 0x4001 },	/* 2pi */
	{ 0xa2f9836e4e441529ULL, 0x3ffd },	/* 1/pi */

	/* big enough to need argument reduction in sincosl */
	{ 0x8000000000000000ULL, 0x403e },	/* 2^63 */
	{ 0xdeadbeefcafe1234ULL, 0x4040 },
	{ 0xffffffffffffffffULL, 0x40ff },
	{ 0x9abcdef012345678ULL, 0xc07f },

	/* just under the exp_ovfl / cexp_ovfl thresholds and just over */
	{ 0xb17217f7d1cf79abULL, 0x400c },
	{ 0xb17217f7d1cf79acULL, 0x400c },	/* == exp_ovfl */
	{ 0xb17217f7d1cf79adULL, 0x400c },
	{ 0xffffffffffffffffULL, 0x400c },	/* top of the 0x400c binade */
	{ 0x8000000000000000ULL, 0x400d },	/* bottom of 0x400d */
	{ 0xb1c6a8573de9768bULL, 0x400d },
	{ 0xb1c6a8573de9768cULL, 0x400d },	/* == cexp_ovfl */
	{ 0xb1c6a8573de9768dULL, 0x400d },
	{ 0xb17217f7d1cf79adULL, 0xc00c },	/* the negative mirrors: these */
	{ 0xb1c6a8573de9768bULL, 0xc00d },	/* must NOT scale */
	{ 0xffffffffffffffffULL, 0x400b },	/* just below the 0x400c binade */
	{ 0x8000000000000000ULL, 0x400e },	/* just above the 0x400d binade */

	/* huge finite */
	{ 0xffffffffffffffffULL, 0x7ffe },	/* LDBL_MAX */
	{ 0xffffffffffffffffULL, 0xfffe },	/* -LDBL_MAX */

	/* infinities */
	{ 0x8000000000000000ULL, 0x7fff },
	{ 0x8000000000000000ULL, 0xffff },

	/* NaNs: default quiet, quiet with a payload, signalling, and one
	   whose payload is all high-bit bytes */
	{ 0xc000000000000000ULL, 0x7fff },
	{ 0xc000000000000000ULL, 0xffff },
	{ 0xc00deadbeef00001ULL, 0x7fff },
	{ 0x8000000000000001ULL, 0x7fff },
	{ 0x8000000000000001ULL, 0xffff },
	{ 0xffffffffffffffffULL, 0x7fff },
	{ 0xa5a5a5a5a5a5a5a5ULL, 0xffff },

	/* x87 "unsupported" encodings: pseudo-denormal, unnormal,
	   pseudo-infinity, pseudo-NaN */
	{ 0x8000000000000000ULL, 0x0000 },	/* pseudo-denormal */
	{ 0x0000000000000001ULL, 0x3fff },	/* unnormal */
	{ 0x0000000000000000ULL, 0x7fff },	/* pseudo-infinity */
	{ 0x4000000000000000ULL, 0x7fff },	/* pseudo-NaN */
	{ 0x0000000000000000ULL, 0x4000 },	/* unnormal, zero mantissa */
};

static const unsigned NSPECIAL = sizeof specials / sizeof specials[0];

/* The six mantissas that straddle the two thresholds in the scaling test. */
static const Val thresholds[] = {
	{ 0xb17217f7d1cf79abULL, 0x400c },
	{ 0xb17217f7d1cf79acULL, 0x400c },
	{ 0xb17217f7d1cf79adULL, 0x400c },
	{ 0xb1c6a8573de9768bULL, 0x400d },
	{ 0xb1c6a8573de9768cULL, 0x400d },
	{ 0xb1c6a8573de9768dULL, 0x400d },
};

static const unsigned NTHRESH = sizeof thresholds / sizeof thresholds[0];

/* ------------------------------------------------------------------ */

int
main(void)
{
	unsigned i, j;
	unsigned long long n;

	rng_state = 0x00930001b0093001ULL;	/* fixed seed */

	/* Sanity: the raw-bit plumbing must round-trip, otherwise every
	   comparison below would be vacuous. */
	{
		Val a = { 0x0123456789abcdefULL, 0x1234 };
		Val b = { 0xfedcba9876543210ULL, 0xa5a5 };
		Parts p = parts(mkz(a, b));

		if (p.re_man != a.man || p.re_es != a.es ||
		    p.im_man != b.man || p.im_es != b.es) {
			std::printf("raw bit plumbing is broken\n");
			return 1;
		}
	}

	/* 1. every hand written value against every other one */
	for (i = 0; i < NSPECIAL; i++)
		for (j = 0; j < NSPECIAL; j++)
			run(G_SPECIAL, specials[i], specials[j]);

	/* 2. sit exactly on each side of each overflow threshold and vary y,
	      so that a flipped '<' or '>' has thousands of chances to show */
	for (i = 0; i < NTHRESH; i++) {
		for (n = 0; n < 4000; n++)
			run(G_THRESHOLD, thresholds[i],
			    gen_normal(0x3fe0, 0x4010));
		for (j = 0; j < NSPECIAL; j++)
			run(G_THRESHOLD, thresholds[i], specials[j]);
	}

	/* 3. general finite arguments, 2^-32 .. 2^32 */
	for (n = 0; n < 40000; n++)
		run(G_GENERAL, gen_normal(0x3fdf, 0x401f),
		    gen_normal(0x3fdf, 0x401f));

	/* 4. the scaling window: this is the only way into __k_expl() and
	      __ldexp_cexpl(), and it must cover all 128 table entries and
	      the whole range of the reduced argument */
	for (n = 0; n < 90000; n++)
		run(G_WINDOW, gen_window(), gen_normal(0x3fe0, 0x4041));

	/* 5. crowd the thresholds from both sides, both signs of x */
	for (n = 0; n < 20000; n++)
		run(G_NEARTHRESH, gen_near_threshold(),
		    gen_normal(0x3fe0, 0x4010));

	/* 6. y infinite or NaN: the hy >= 0x7fff block and its three arms */
	for (n = 0; n < 20000; n++) {
		Val xv;

		switch (n % 4) {
		case 0:	xv = gen_normal(0x3fdf, 0x401f); break;
		case 1:	xv = gen_infnan(); break;
		case 2:	xv = gen_near_threshold(); break;
		default: xv = gen_raw(); break;
		}
		run(G_INFNAN_Y, xv, gen_infnan());
	}

	/* 7. the two early outs: y == +-0, and x == +-0 */
	for (n = 0; n < 20000; n++) {
		Val zero = { 0x0000000000000000ULL,
		    (std::uint16_t)((n & 1) ? 0x8000 : 0x0000) };
		Val other;

		switch (n % 6) {
		case 0:	other = gen_normal(0x3fdf, 0x401f); break;
		case 1:	other = gen_window(); break;
		case 2:	other = gen_infnan(); break;
		case 3:	other = gen_near_threshold(); break;
		case 4:	other = gen_raw(); break;
		default: other = gen_normal(0x0001, 0x7ffe); break;
		}
		if ((n / 6) & 1)
			run(G_ZEROS, other, zero);
		else
			run(G_ZEROS, zero, other);
	}

	/* 8. completely unconstrained 80 bit patterns */
	for (n = 0; n < 30000; n++)
		run(G_RAWBITS, gen_raw(), gen_raw());

	/* ------------------------------------------------------------ */

	{
		unsigned long long tot_c = 0, tot_f = 0;

		std::printf("\n");
		std::printf("%-34s %12s %12s\n", "group", "cases", "failures");
		std::printf("%-34s %12s %12s\n",
		    "----------------------------------",
		    "------------", "------------");
		for (i = 0; i < NGROUPS; i++) {
			std::printf("%-34s %12llu %12llu\n", group_name[i],
			    g_cases[i], g_fails[i]);
			tot_c += g_cases[i];
			tot_f += g_fails[i];
		}

		std::printf("\n");
		std::printf("%-34s %12s %12s\n", "function", "cases",
		    "failures");
		std::printf("%-34s %12s %12s\n",
		    "----------------------------------",
		    "------------", "------------");
		std::printf("%-34s %12llu %12llu\n", "cexpl", tot_c, tot_f);
		std::printf("\n%s\n", tot_f == 0 ? "PASS" : "FAIL");

		return tot_f == 0 ? 0 : 1;
	}
}
