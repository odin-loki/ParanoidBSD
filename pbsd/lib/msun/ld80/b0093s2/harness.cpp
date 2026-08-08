/*
 * PBSD batch b0093s2 -- differential test of the C++23 port against the
 * unmodified C source in oracle.c.
 *
 * cospil()	long double -> long double
 *
 * cospil() does not write through a caller supplied pointer, so the buffer
 * discipline is applied to the values instead: every case gives the port and
 * the oracle its own 0x7f filled buffer, copies the identical input bytes
 * into both, has each side read its argument out of its own buffer and store
 * its result back into it, and then compares the buffers in their entirety,
 * guard bytes included.  Only the ten architectural bytes of an ld80 value
 * are copied, so the 0x7f guards also cover the bytes the results do not
 * occupy (the six padding bytes of a 16 byte long double are indeterminate
 * and are never copied).
 *
 * Comparison is bit exact: -0.0 differs from +0.0 and NaN payloads must
 * match.  In addition the raised floating point exception flags are compared
 * per call, which pins down paths that return the same value by a different
 * route (cospil()'s |x| < 0x1p-34 shortcut returns exactly 1 for the whole
 * reachable domain, so only the flags distinguish it from falling through to
 * __kernel_cospil()).
 */

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cfenv>

import pbsd.lib.msun.ld80.b0093s2;

namespace P = pbsd::lib_msun_ld80::b0093s2;

extern "C" {
long double ref_cospil(long double);
long double nextafterl(long double, long double);
}

using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

/* ------------------------------------------------------------------ */
/* ld80 bit twiddling						      */
/* ------------------------------------------------------------------ */

static long double
mkld(uint16_t se, uint64_t man)
{
	unsigned char b[16];
	long double r;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &man, 8);
	std::memcpy(b + 8, &se, 2);
	std::memcpy(&r, b, 16);
	return (r);
}

static void
ldbits(long double x, uint16_t *se, uint64_t *man)
{
	unsigned char b[16];

	std::memcpy(b, &x, 16);
	std::memcpy(man, b, 8);
	std::memcpy(se, b + 8, 2);
}

#define	LD_INF		mkld(0x7fff, 0x8000000000000000ULL)
#define	LD_NINF		mkld(0xffff, 0x8000000000000000ULL)

static const unsigned char GUARD = 0x7f;

/* ------------------------------------------------------------------ */
/* statistics							      */
/* ------------------------------------------------------------------ */

struct Fstat {
	const char	*name;
	long long	cases;
	long long	fails;
	long long	valfails;
	long long	flagfails;
	int		shown;
};

static Fstat st_cospil = { "cospil", 0, 0, 0, 0, 0 };

#define	MAXSHOW	8

static void
show_ld(const char *label, long double v)
{
	uint16_t se;
	uint64_t man;

	ldbits(v, &se, &man);
	std::printf("      %s = %04x:%016llx (%.21Lg)\n", label, se,
	    (unsigned long long)man, v);
}

/* ------------------------------------------------------------------ */
/* one argument, one result					      */
/* ------------------------------------------------------------------ */

static void
check1(long double x, const char *tag)
{
	alignas(16) unsigned char ba[64], bb[64];
	long double xa, xb, ra, rb;
	int fa, fb, bad;
	unsigned i;

	std::memset(ba, GUARD, sizeof(ba));
	std::memset(bb, GUARD, sizeof(bb));
	std::memcpy(ba, &x, 10);
	std::memcpy(bb, &x, 10);

	std::memcpy(&xa, ba, 16);
	std::memcpy(&xb, bb, 16);

	std::feclearexcept(FE_ALL_EXCEPT);
	ra = P::cospil(xa);
	std::memcpy(ba + 16, &ra, 10);
	fa = std::fetestexcept(FE_ALL_EXCEPT);

	std::feclearexcept(FE_ALL_EXCEPT);
	rb = ref_cospil(xb);
	std::memcpy(bb + 16, &rb, 10);
	fb = std::fetestexcept(FE_ALL_EXCEPT);

	st_cospil.cases++;
	bad = 0;

	if (std::memcmp(ba, bb, sizeof(ba)) != 0) {
		bad = 1;
		st_cospil.valfails++;
	}
	/* The guards must still be intact in both buffers. */
	for (i = 10; i < 16; i++)
		if (ba[i] != GUARD || bb[i] != GUARD)
			bad = 1;
	for (i = 26; i < sizeof(ba); i++)
		if (ba[i] != GUARD || bb[i] != GUARD)
			bad = 1;
	if (fa != fb) {
		bad = 1;
		st_cospil.flagfails++;
	}

	if (bad) {
		st_cospil.fails++;
		if (st_cospil.shown < MAXSHOW) {
			st_cospil.shown++;
			std::printf("  cospil FAIL [%s]\n", tag);
			show_ld("x   ", x);
			show_ld("port", ra);
			show_ld("ref ", rb);
			std::printf("      flags port=%#x ref=%#x\n", fa, fb);
		}
	}
}

/* ------------------------------------------------------------------ */
/* hand written case list					      */
/* ------------------------------------------------------------------ */

static long double vals[16384];
static int nvals;

static void
add(long double x)
{
	if (nvals < (int)(sizeof(vals) / sizeof(vals[0])))
		vals[nvals++] = x;
}

static void
add_pm(long double x)
{
	add(x);
	add(-x);
}

/* x, and k ulps to either side of it, with both signs. */
static void
add_neigh(long double x, int k)
{
	long double up = x, dn = x;
	int i;

	add_pm(x);
	for (i = 0; i < k; i++) {
		up = nextafterl(up, LD_INF);
		dn = nextafterl(dn, LD_NINF);
		add_pm(up);
		add_pm(dn);
	}
}

static void
build_vals(void)
{
	static const uint16_t exps[] = {
		0x0000, 0x0001, 0x0002, 0x1000, 0x3f00,
		0x3fda, 0x3fdb, 0x3fdc, 0x3fdd, 0x3fde, 0x3fdf,
		0x3ff0, 0x3ffb, 0x3ffc, 0x3ffd, 0x3ffe, 0x3fff,
		0x4000, 0x4001, 0x4002, 0x4008,
		0x401c, 0x401d, 0x401e, 0x401f, 0x4020,
		0x4025, 0x4026, 0x4027, 0x4028, 0x4029,
		0x403b, 0x403c, 0x403d, 0x403e, 0x403f, 0x4040, 0x4041,
		0x4100, 0x7ffc, 0x7ffd, 0x7ffe, 0x7fff
	};
	static const uint64_t mans[] = {
		0x0000000000000000ULL, 0x0000000000000001ULL,
		0x4000000000000000ULL, 0x7fffffffffffffffULL,
		0x8000000000000000ULL, 0x8000000000000001ULL,
		0x8000000100000000ULL, 0xa000000000000000ULL,
		0xbfffffffffffffffULL, 0xc000000000000000ULL,
		0xc000000000000001ULL, 0xe000000000000000ULL,
		0xfffffffffffffffeULL, 0xffffffffffffffffULL
	};
	unsigned e, m;
	int i;

	/* Zeroes, subnormals, pseudo denormals, smallest normal. */
	add(mkld(0x0000, 0x0000000000000000ULL));
	add(mkld(0x8000, 0x0000000000000000ULL));
	add_pm(mkld(0x0000, 0x0000000000000001ULL));
	add_pm(mkld(0x0000, 0x0000000080000000ULL));
	add_pm(mkld(0x0000, 0x7fffffffffffffffULL));
	add_pm(mkld(0x0000, 0x8000000000000000ULL));
	add_pm(mkld(0x0000, 0xffffffffffffffffULL));
	add_pm(mkld(0x0001, 0x8000000000000000ULL));

	/* The exponent x mantissa grid, both signs. */
	for (e = 0; e < sizeof(exps) / sizeof(exps[0]); e++)
		for (m = 0; m < sizeof(mans) / sizeof(mans[0]); m++) {
			add(mkld(exps[e], mans[m]));
			add(mkld((uint16_t)(exps[e] | 0x8000), mans[m]));
		}

	/* Every branch boundary, with a few ulps to either side. */
	add_neigh(0x1p-16445L, 2);
	add_neigh(0x1p-40L, 2);
	add_neigh(0x1p-35L, 2);
	add_neigh(0x1p-34L, 3);		/* the 0x3fdd cut */
	add_neigh(0x1p-33L, 2);
	add_neigh(0.125L, 2);
	add_neigh(0.25L, 3);		/* the 0x3ffd cut */
	add_neigh(0.375L, 2);
	add_neigh(0.5L, 3);		/* the 0x3ffe cut */
	add_neigh(0.625L, 2);
	add_neigh(0.75L, 3);		/* the lx < 0xc000... cut */
	add_neigh(0.875L, 2);
	add_neigh(1.0L, 3);		/* the 0x3fff cut */
	add_neigh(1.25L, 2);
	add_neigh(1.5L, 3);
	add_neigh(1.75L, 2);
	add_neigh(2.0L, 3);
	add_neigh(2.25L, 2);
	add_neigh(2.5L, 2);
	add_neigh(2.75L, 2);
	add_neigh(3.0L, 2);
	add_neigh(3.5L, 2);
	add_neigh(4.0L, 2);
	add_neigh(0x1p29L, 2);
	add_neigh(0x1p30L, 3);		/* the j0 > 30 cut */
	add_neigh(0x1p31L, 3);		/* the FFLOORL80 j0 < 32 cut */
	add_neigh(0x1p32L, 3);
	add_neigh(0x1p39L, 2);
	add_neigh(0x1p40L, 3);		/* the j0 > 40 cut */
	add_neigh(0x1p41L, 3);
	add_neigh(0x1p61L, 2);
	add_neigh(0x1p62L, 3);		/* the 0x403e cut is just above */
	add_neigh(0x1p63L, 3);
	add_neigh(0x1p64L, 3);		/* the 0x403f cut */
	add_neigh(0x1p65L, 2);
	add_neigh(0x1p100L, 2);
	add_neigh(0x1p16383L, 2);

	/*
	 * Integers and integers plus a fraction at every j0, so that the
	 * argument reduction, the FFLOORL80 masking, the 0x1p40/0x1p30
	 * folding and the parity of the integer part are all driven with
	 * both parities.  For the larger exponents the addition rounds,
	 * which is exactly the interesting case.
	 */
	for (i = 0; i <= 62; i++) {
		long double base = 1.0L, n;
		int j, k;
		static const long double fr[] = {
			0.0L, 0.25L, 0.5L, 0.75L, 0.125L, 0.375L, 0.9375L
		};

		for (j = 0; j < i; j++)
			base *= 2.0L;
		for (k = 0; k < 4; k++) {
			n = k == 0 ? base : k == 1 ? base + 1.0L :
			    k == 2 ? base * 2.0L - 1.0L : base * 2.0L - 2.0L;
			for (j = 0; j < (int)(sizeof(fr) / sizeof(fr[0])); j++)
				add_pm(n + fr[j]);
		}
	}

	/* Infinities, NaNs of several payloads, unnormals. */
	add(LD_INF);
	add(LD_NINF);
	add_pm(mkld(0x7fff, 0xc000000000000000ULL));	/* quiet NaN */
	add_pm(mkld(0x7fff, 0xffffffffffffffffULL));
	add_pm(mkld(0x7fff, 0xc000000000000001ULL));
	add_pm(mkld(0x7fff, 0x8000000000000001ULL));	/* signalling NaN */
	add_pm(mkld(0x7fff, 0xbfffffffffffffffULL));	/* pseudo NaN */
	add_pm(mkld(0x7fff, 0x0000000000000000ULL));	/* pseudo infinity */
	add_pm(mkld(0x4000, 0x4000000000000000ULL));	/* unnormal */
	add_pm(mkld(0x403e, 0x0000000000000001ULL));
	add_pm(mkld(0x403d, 0x7fffffffffffffffULL));

	/*
	 * Values where (int)x == 0 but ix >= 0x3fdd, to distinguish the
	 * |x| < 0x1p-34 shortcut from __kernel_cospil() via flags.
	 */
	add_pm(mkld(0x3fdc, 0x8000000000000000ULL));
	add_pm(mkld(0x3fdc, 0xc000000000000000ULL));
	add_pm(mkld(0x3fdd, 0x0000000000000000ULL));
	add_pm(mkld(0x3fdd, 0x4000000000000000ULL));

	/* Exact ax == 0.5 in the |x| < 0.75 branch. */
	add_pm(0.5L);
	add_pm(mkld(0x3ffe, 0x8000000000000000ULL));

	/* lx == 0xc000000000000000ull boundary. */
	add_pm(mkld(0x3ffe, 0xc000000000000000ULL));
	add_pm(mkld(0x3ffe, 0xbfffffffffffffffULL));
	add_pm(mkld(0x3ffe, 0xc000000000000001ULL));

	/* ix == 0 after reduction (integer input). */
	for (i = 1; i <= 100; i++)
		add_pm((long double)i);
	for (i = 1; i <= 20; i++)
		add_pm((long double)i + 0.5L);

	/* 0x1p63 <= |x| < 0x1p64: even/odd integer via lx & 1. */
	add_pm(mkld(0x403e, 0x8000000000000000ULL));
	add_pm(mkld(0x403e, 0x8000000000000001ULL));
	add_pm(mkld(0x403f, 0x0000000000000000ULL));
	add_pm(mkld(0x403f, 0x8000000000000001ULL));
}

/* ------------------------------------------------------------------ */
/* fixed seed randomised sweep					      */
/* ------------------------------------------------------------------ */

static uint64_t rng = 0x0093b2c0ffeeULL;

static uint64_t
nextrand(void)
{
	uint64_t z;

	rng += 0x9e3779b97f4a7c15ULL;
	z = rng;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static const uint16_t rexps[] = {
	0x0000, 0x0001, 0x2000, 0x3f00, 0x3fd0,
	0x3fdb, 0x3fdc, 0x3fdd, 0x3fde,
	0x3ff8, 0x3ffb, 0x3ffc, 0x3ffd, 0x3ffe, 0x3fff,
	0x4000, 0x4001, 0x4003, 0x400c, 0x4010,
	0x401d, 0x401e, 0x401f, 0x4020,
	0x4026, 0x4027, 0x4028,
	0x403c, 0x403d, 0x403e, 0x403f, 0x4040,
	0x4200, 0x7ffe, 0x7fff
};

static long double
randval(int mode)
{
	uint64_t r, man;
	uint16_t se;
	long double v, f;
	int sh, i;

	switch (mode) {
	case 0:				/* interesting exponent, random man */
		se = rexps[nextrand() % (sizeof(rexps) / sizeof(rexps[0]))];
		man = nextrand();
		if ((nextrand() & 3) != 0)
			man |= 0x8000000000000000ULL;	/* normalised */
		if (nextrand() & 1)
			se |= 0x8000;
		return (mkld(se, man));

	case 1:				/* integer of random width + fraction */
		sh = (int)(nextrand() % 63);
		v = (long double)(uint64_t)(nextrand() >> sh);
		f = (long double)(nextrand() >> 11) * 0x1p-53L;
		v = v + f;
		return ((nextrand() & 1) ? -v : v);

	case 2:				/* wholly random bit pattern */
		return (mkld((uint16_t)nextrand(), nextrand()));

	case 3:				/* somewhere in [0,1) */
		se = (uint16_t)(0x3ffe - (nextrand() % 45));
		man = nextrand() | 0x8000000000000000ULL;
		if (nextrand() & 1)
			se |= 0x8000;
		return (mkld(se, man));

	case 4: {			/* a few ulps off a boundary */
		static const long double bases[] = {
			0x1p-34L, 0x1p-35L, 0.125L, 0.25L, 0.5L, 0.75L,
			1.0L, 1.5L, 2.0L, 2.5L, 0x1p29L, 0x1p30L, 0x1p31L,
			0x1p32L, 0x1p39L, 0x1p40L, 0x1p41L, 0x1p62L,
			0x1p63L, 0x1p64L
		};
		v = bases[nextrand() % (sizeof(bases) / sizeof(bases[0]))];
		r = nextrand();
		sh = (int)(r % 9) - 4;
		for (i = 0; i < (sh < 0 ? -sh : sh); i++)
			v = nextafterl(v, sh < 0 ? LD_NINF : LD_INF);
		return ((r & 0x100) ? -v : v);
	}

	default: {			/* integer + exact quarter */
		static const long double q[] = { 0.0L, 0.25L, 0.5L, 0.75L };
		sh = (int)(nextrand() % 63);
		v = (long double)(uint64_t)(nextrand() >> sh);
		v = v + q[nextrand() & 3];
		return ((nextrand() & 1) ? -v : v);
	}
	}
}

#define	SWEEP	200000

int
main(void)
{
	long long i;
	int j;
	char tag[64];

	build_vals();

	std::printf("b0093s2 differential test: %d hand written values\n",
	    nvals);

	/* --- hand written cases ------------------------------------- */
	for (j = 0; j < nvals; j++) {
		std::snprintf(tag, sizeof(tag), "edge #%d", j);
		check1(vals[j], tag);
	}

	/* --- randomised sweep --------------------------------------- */
	for (i = 0; i < SWEEP; i++) {
		long double x = randval((int)(i % 6));

		std::snprintf(tag, sizeof(tag), "rand #%lld", i);
		check1(x, tag);
	}

	/* --- report ------------------------------------------------ */
	{
		long long tc = st_cospil.cases;
		long long tf = st_cospil.fails;

		std::printf("\n%-10s %12s %12s %12s %12s\n", "function",
		    "cases", "failures", "value", "flags");
		std::printf("%-10s %12s %12s %12s %12s\n", "--------",
		    "-----", "--------", "-----", "-----");
		std::printf("%-10s %12lld %12lld %12lld %12lld\n",
		    st_cospil.name, st_cospil.cases, st_cospil.fails,
		    st_cospil.valfails, st_cospil.flagfails);
		std::printf("%-10s %12lld %12lld\n", "TOTAL", tc, tf);
		std::printf("\n%s\n", tf == 0 ? "PASS" : "FAIL");
		return (tf == 0 ? 0 : 1);
	}
}
