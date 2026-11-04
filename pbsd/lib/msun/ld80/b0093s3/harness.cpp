/*
 * PBSD batch b0093s3 -- differential test.
 *
 * Every case is fed to both pbsd::lib_msun_ld80::b0093s3::sinpil() and to
 * ref_sinpil() from oracle.c, and the two long double results are compared
 * BIT FOR BIT over the 10 significant bytes of the x87 extended format.
 *
 * A bitwise comparison is deliberate and load bearing: sinpil() distinguishes
 * +0.0L from -0.0L on several paths (the x == 0 early return, the s = 0 exact
 * integer path, and copysignl(0, x)), and a value comparison would silently
 * accept a port that got the sign of zero wrong.  NaN results are likewise
 * only comparable as bit patterns.
 *
 * sinpil() takes a scalar and writes no buffer and returns no pointer, so the
 * buffer / pointer-offset / iterator-state protocols do not apply here; the
 * whole observable result is the returned long double, which is compared in
 * full including bits that a tolerance-based check would hide.
 *
 * The input generators aim every branch, every comparison and every
 * arithmetic expression of the port at both sides of its boundary:
 *
 *   ix <  0x3fdd | 0x3fdc and 0x3fdd, plus subnormals and both zeros
 *   ix <  0x3ffd | 0x3ffc and 0x3ffd
 *   ix <  0x3ffe | 0x3ffd and 0x3ffe
 *   ix <  0x3fff | 0x3ffe and 0x3fff
 *   ix <  0x403e | 0x403d and 0x403e
 *   ix >= 0x7fff | 0x7ffe and 0x7fff
 *   lx <  0xc000000000000000 | 0xbfffffffffffffff, 0xc000000000000000 and
 *                              0xc000000000000001, both for |x| < 1 and for
 *                              the fractional part of |x| >= 1
 *   j0 <  32     | j0 = 31, 32 and 33 with bit 31 of the mantissa both set
 *                  and clear, so the two FFLOORL80 masking paths disagree
 *   j0 >  30     | j0 = 30 and 31
 *   j0 >  40     | j0 = 40 and 41
 *   ix == 0      | exact integers (zero fraction) and integers + 1 ulp
 *   j0 &  1      | consecutive integer parts, so both parities reach every
 *                  fractional branch
 *   x  == 0      | +0.0L, -0.0L and the smallest nonzero subnormals
 *   hx &  0x8000 | every single case is run for both signs
 */

#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.ld80.b0093s3;

namespace port = pbsd::lib_msun_ld80::b0093s3;

extern "C" long double ref_sinpil(long double);

/* ------------------------------------------------------------------ */
/* x87 extended precision bit twiddling				      */
/* ------------------------------------------------------------------ */

#define	LD_SIGBYTES	10		/* 8 mantissa + 2 expsign	    */

union ldbits {
	long double e;
	unsigned char b[sizeof(long double)];
};

static long double
make_ld(std::uint16_t expsign, std::uint64_t man)
{
	union ldbits u;

	std::memset(&u, 0, sizeof(u));
	u.b[0] = (unsigned char)(man >>  0);
	u.b[1] = (unsigned char)(man >>  8);
	u.b[2] = (unsigned char)(man >> 16);
	u.b[3] = (unsigned char)(man >> 24);
	u.b[4] = (unsigned char)(man >> 32);
	u.b[5] = (unsigned char)(man >> 40);
	u.b[6] = (unsigned char)(man >> 48);
	u.b[7] = (unsigned char)(man >> 56);
	u.b[8] = (unsigned char)(expsign >> 0);
	u.b[9] = (unsigned char)(expsign >> 8);
	return (u.e);
}

static void
split_ld(long double x, std::uint16_t *expsign, std::uint64_t *man)
{
	union ldbits u;
	std::uint64_t m = 0;
	int i;

	std::memset(&u, 0, sizeof(u));
	u.e = x;
	for (i = 7; i >= 0; i--)
		m = (m << 8) | u.b[i];
	*man = m;
	*expsign = (std::uint16_t)(u.b[8] | (u.b[9] << 8));
}

/* ------------------------------------------------------------------ */
/* bookkeeping							      */
/* ------------------------------------------------------------------ */

struct bucket {
	const char *name;
	long long cases;
	long long fails;
};

enum {
	B_ZERO_SUB, B_TINY, B_FRAC, B_INT, B_HUGE, B_NANINF, B_BYTES,
	B_RANDOM, B_NBUCKETS
};

static struct bucket buckets[B_NBUCKETS] = {
	{ "sinpil / zero+subnormal",		0, 0 },
	{ "sinpil / |x| < 0x1p-34",		0, 0 },
	{ "sinpil / 0x1p-34 <= |x| < 1",	0, 0 },
	{ "sinpil / 1 <= |x| < 0x1p63",		0, 0 },
	{ "sinpil / |x| >= 0x1p63 finite",	0, 0 },
	{ "sinpil / inf and nan",		0, 0 },
	{ "sinpil / raw byte patterns",		0, 0 },
	{ "sinpil / randomised sweep",		0, 0 },
};

static int reported;

static void
check(int b, long double x)
{
	union ldbits got, want;
	std::uint16_t xes, ges, wes;
	std::uint64_t xman, gman, wman;

	buckets[b].cases++;

	std::memset(&got, 0, sizeof(got));
	std::memset(&want, 0, sizeof(want));
	got.e = port::sinpil(x);
	want.e = ref_sinpil(x);

	if (std::memcmp(got.b, want.b, LD_SIGBYTES) == 0)
		return;

	buckets[b].fails++;
	if (reported < 20) {
		reported++;
		split_ld(x, &xes, &xman);
		split_ld(got.e, &ges, &gman);
		split_ld(want.e, &wes, &wman);
		std::printf("  MISMATCH [%s] x=%04x:%016llx  "
		    "port=%04x:%016llx  ref=%04x:%016llx\n",
		    buckets[b].name,
		    (unsigned)xes, (unsigned long long)xman,
		    (unsigned)ges, (unsigned long long)gman,
		    (unsigned)wes, (unsigned long long)wman);
	}
}

/* Run a magnitude through both signs. */
static void
check_both(int b, std::uint16_t ix, std::uint64_t man)
{
	check(b, make_ld(ix, man));
	check(b, make_ld((std::uint16_t)(ix | 0x8000), man));
}

/* Run a value and its negation. */
static void
check_val(int b, long double v)
{
	check(b, v);
	check(b, -v);
}

/* ------------------------------------------------------------------ */
/* mantissas that straddle every mantissa comparison in the port      */
/* ------------------------------------------------------------------ */

static const std::uint64_t mantissas[] = {
	0x0000000000000000ull,
	0x0000000000000001ull,
	0x00000000ffffffffull,
	0x0000000100000000ull,
	0x000000007fffffffull,
	0x0000000080000000ull,
	0x7fffffffffffffffull,
	0x8000000000000000ull,
	0x8000000000000001ull,
	0x8000000000000003ull,
	0x8000000080000000ull,
	0x8000000100000000ull,
	0x80000000ffffffffull,
	0x9000000000000000ull,
	0xa000000000000000ull,
	0xaaaaaaaaaaaaaaaaull,
	0xbfffffff7fffffffull,
	0xbfffffffffffffffull,
	0xc000000000000000ull,
	0xc000000000000001ull,
	0xc000000080000000ull,
	0xd555555555555555ull,
	0xe000000000000000ull,
	0xfffffffeffffffffull,
	0xffffffff00000000ull,
	0xffffffffffffffffull,
};
static const int nmantissas = (int)(sizeof(mantissas) / sizeof(mantissas[0]));

/* ------------------------------------------------------------------ */
/* deterministic PRNG (xorshift64*, fixed seed)			      */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state = 0x9e3779b97f4a7c15ull;

static std::uint64_t
rng(void)
{
	std::uint64_t x = rng_state;

	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return (x * 0x2545f4914f6cdd1dull);
}

/* ------------------------------------------------------------------ */

static void
edge_zero_subnormal(void)
{
	static const std::uint64_t mans[] = {
		0x0000000000000000ull, 0x0000000000000001ull,
		0x0000000000000002ull, 0x000000000000007full,
		0x0000000000000080ull, 0x00000000000000ffull,
		0x000000007fffffffull, 0x0000000080000000ull,
		0x00000000ffffffffull, 0x0000000100000000ull,
		0x3fffffffffffffffull, 0x4000000000000000ull,
		0x7fffffffffffffffull, 0x8000000000000000ull,
		0x8000000000000001ull, 0xbfffffffffffffffull,
		0xc000000000000000ull, 0xffffffffffffffffull,
	};
	int i;

	for (i = 0; i < (int)(sizeof(mans) / sizeof(mans[0])); i++)
		check_both(B_ZERO_SUB, 0x0000, mans[i]);
}

static void
edge_tiny(void)
{
	/* ix < 0x3fdd is the |x| < 0x1p-34 path; 0x3fdd is the first ix
	 * that is NOT taken, so both sides of the comparison are covered. */
	static const std::uint16_t ixs[] = {
		0x0001, 0x0002, 0x00ff, 0x1000, 0x2000, 0x3000, 0x3f00,
		0x3fd8, 0x3fd9, 0x3fda, 0x3fdb, 0x3fdc, 0x3fdd, 0x3fde,
		0x3fdf, 0x3fe0,
	};
	int i, j;

	for (i = 0; i < (int)(sizeof(ixs) / sizeof(ixs[0])); i++)
		for (j = 0; j < nmantissas; j++)
			check_both(ixs[i] < 0x3fdd ? B_TINY : B_FRAC,
			    ixs[i], mantissas[j]);
}

static void
edge_fraction(void)
{
	static const std::uint16_t ixs[] = {
		0x3fdd, 0x3fde, 0x3fe8, 0x3ff0, 0x3ff8, 0x3ffa, 0x3ffb,
		0x3ffc, 0x3ffd, 0x3ffe,
	};
	int i, j;

	for (i = 0; i < (int)(sizeof(ixs) / sizeof(ixs[0])); i++)
		for (j = 0; j < nmantissas; j++)
			check_both(B_FRAC, ixs[i], mantissas[j]);

	/* Exact quarter/eighth boundaries of |x| < 1. */
	{
		long double q;
		int k;

		for (k = 1; k < 64; k++) {
			q = (long double)k / 64.0L;
			check_val(B_FRAC, q);
		}
		check_val(B_FRAC, 0.25L);
		check_val(B_FRAC, 0.5L);
		check_val(B_FRAC, 0.75L);
		check_val(B_FRAC, 0.25L - 0x1p-70L);
		check_val(B_FRAC, 0.25L + 0x1p-70L);
		check_val(B_FRAC, 0.5L - 0x1p-70L);
		check_val(B_FRAC, 0.5L + 0x1p-70L);
		check_val(B_FRAC, 0.75L - 0x1p-70L);
		check_val(B_FRAC, 0.75L + 0x1p-70L);
		check_val(B_FRAC, 1.0L - 0x1p-64L);
	}
}

static void
edge_integer_region(void)
{
	std::uint16_t ix;
	int j, k, f;

	/* Sweep the entire 1 <= |x| < 0x1p63 exponent range.  This walks j0
	 * from 1 to 63, covering FFLOORL80's j0 < 32 boundary at 31/32/33
	 * and the j0 > 30 and j0 > 40 boundaries at 30/31 and 40/41. */
	for (ix = 0x3fff; ix <= 0x403d; ix++)
		for (j = 0; j < nmantissas; j++)
			check_both(B_INT, ix, mantissas[j]);

	/* Consecutive integers plus every eighth, so both parities of the
	 * integer part reach every fractional branch. */
	for (k = 1; k <= 48; k++) {
		for (f = 0; f < 8; f++)
			check_val(B_INT, (long double)k +
			    (long double)f / 8.0L);
		check_val(B_INT, (long double)k + 1.0L - 0x1p-40L);
		check_val(B_INT, (long double)k - 0x1p-40L);
	}

	/* Straddle the j0 > 30 and j0 > 40 thresholds with both parities of
	 * the integer part and a nonzero fraction. */
	{
		static const int expos[] = { 28, 29, 30, 31, 32, 33, 38, 39,
		    40, 41, 42, 50, 61, 62 };
		static const long double fracs[] = { 0.0L, 0.125L, 0.25L,
		    0.375L, 0.5L, 0.625L, 0.75L, 0.875L };
		long double base;
		int e, d;

		for (e = 0; e < (int)(sizeof(expos) / sizeof(expos[0])); e++) {
			base = 1.0L;
			for (d = 0; d < expos[e]; d++)
				base *= 2.0L;
			for (d = -3; d <= 3; d++)
				for (f = 0; f < (int)(sizeof(fracs) /
				    sizeof(fracs[0])); f++)
					check_val(B_INT, base +
					    (long double)d + fracs[f]);
		}
	}
}

static void
edge_huge(void)
{
	static const std::uint16_t ixs[] = {
		0x403c, 0x403d, 0x403e, 0x403f, 0x4040, 0x4100, 0x5000,
		0x6000, 0x7ffc, 0x7ffd, 0x7ffe,
	};
	int i, j;

	for (i = 0; i < (int)(sizeof(ixs) / sizeof(ixs[0])); i++)
		for (j = 0; j < nmantissas; j++)
			check_both(ixs[i] < 0x403e ? B_INT : B_HUGE,
			    ixs[i], mantissas[j]);
}

static void
edge_naninf(void)
{
	int j;

	for (j = 0; j < nmantissas; j++)
		check_both(B_NANINF, 0x7fff, mantissas[j]);
}

static void
edge_byte_patterns(void)
{
	static const unsigned char fills[] = {
		0x00, 0x01, 0x02, 0x7e, 0x7f, 0x80, 0x81, 0xfe, 0xff,
	};
	union ldbits u;
	int i, k, n;

	for (i = 0; i < (int)sizeof(fills); i++) {
		std::memset(&u, 0, sizeof(u));
		std::memset(u.b, fills[i], LD_SIGBYTES);
		check(B_BYTES, u.e);

		/* one byte at a time poked into an otherwise uniform value */
		for (k = 0; k < LD_SIGBYTES; k++) {
			for (n = 0; n < (int)sizeof(fills); n++) {
				std::memset(&u, 0, sizeof(u));
				std::memset(u.b, fills[i], LD_SIGBYTES);
				u.b[k] = fills[n];
				check(B_BYTES, u.e);
			}
		}
	}
}

/* ------------------------------------------------------------------ */

#define	RANDOM_ITERATIONS	250000

static void
random_sweep(void)
{
	static const std::uint16_t hot_ix[] = {
		0x0000, 0x0001, 0x3fdc, 0x3fdd, 0x3ffc, 0x3ffd, 0x3ffe,
		0x3fff, 0x4000, 0x401c, 0x401d, 0x401e, 0x401f, 0x4025,
		0x4026, 0x4027, 0x4028, 0x403c, 0x403d, 0x403e, 0x403f,
		0x7ffe, 0x7fff,
	};
	static const std::uint64_t hot_man[] = {
		0x0000000000000000ull, 0x0000000000000001ull,
		0x7fffffffffffffffull, 0x8000000000000000ull,
		0x8000000000000001ull, 0x8000000080000000ull,
		0xbfffffffffffffffull, 0xc000000000000000ull,
		0xc000000000000001ull, 0xffffffff00000000ull,
		0xffffffffffffffffull,
	};
	std::uint64_t r, man;
	std::uint16_t es, ix;
	long double v;
	int i;

	for (i = 0; i < RANDOM_ITERATIONS; i++) {
		r = rng();
		switch (i & 7) {
		case 0:
			/* dense around 1.0, walking j0 across all of its
			 * interesting values */
			ix = (std::uint16_t)(0x3ff0 + (r % 0x60));
			man = rng() | 0x8000000000000000ull;
			break;
		case 1:
			ix = (std::uint16_t)(0x3fd0 + (r % 0x80));
			man = rng();
			break;
		case 2:
			ix = (std::uint16_t)(r % 0x8000);
			man = rng();
			break;
		case 3:
			ix = hot_ix[r % (sizeof(hot_ix) / sizeof(hot_ix[0]))];
			man = rng() | 0x8000000000000000ull;
			break;
		case 4:
			ix = (std::uint16_t)(0x3fdd + (rng() % 0x62));
			man = hot_man[r %
			    (sizeof(hot_man) / sizeof(hot_man[0]))];
			break;
		case 5: {
			/* integer + a quarter, near the reduction thresholds */
			static const int expos[] = { 0, 1, 4, 10, 20, 29, 30,
			    31, 32, 39, 40, 41, 50, 62 };
			long double base = 1.0L;
			int e = expos[r % (sizeof(expos) / sizeof(expos[0]))];
			int k;

			for (k = 0; k < e; k++)
				base *= 2.0L;
			v = base + (long double)(std::int64_t)(rng() % 64) +
			    (long double)(rng() % 8) / 8.0L;
			if (rng() & 1)
				v = -v;
			check(B_RANDOM, v);
			continue;
		}
		case 6: {
			std::uint64_t n = rng() % (1ull << 41);
			v = (long double)(std::int64_t)n +
			    (long double)(rng() % 256) / 256.0L;
			if (rng() & 1)
				v = -v;
			check(B_RANDOM, v);
			continue;
		}
		default:
			/* fully random 80 bits */
			ix = (std::uint16_t)(rng() & 0x7fff);
			man = rng();
			break;
		}
		es = (std::uint16_t)(ix | ((rng() & 1) ? 0x8000 : 0));
		check(B_RANDOM, make_ld(es, man));
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	long long total_cases = 0, total_fails = 0;
	int i;

	edge_zero_subnormal();
	edge_tiny();
	edge_fraction();
	edge_integer_region();
	edge_huge();
	edge_naninf();
	edge_byte_patterns();
	random_sweep();

	std::printf("\n");
	std::printf("%-34s %12s %12s\n", "case group", "cases", "failures");
	std::printf("%-34s %12s %12s\n",
	    "----------------------------------",
	    "------------", "------------");
	for (i = 0; i < B_NBUCKETS; i++) {
		std::printf("%-34s %12lld %12lld\n", buckets[i].name,
		    buckets[i].cases, buckets[i].fails);
		total_cases += buckets[i].cases;
		total_fails += buckets[i].fails;
	}
	std::printf("%-34s %12s %12s\n",
	    "----------------------------------",
	    "------------", "------------");
	std::printf("%-34s %12lld %12lld\n", "sinpil TOTAL",
	    total_cases, total_fails);
	std::printf("\n%s\n", total_fails == 0 ? "PASS" : "FAIL");

	return (total_fails == 0 ? 0 : 1);
}
