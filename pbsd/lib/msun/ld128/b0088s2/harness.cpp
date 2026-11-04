/*
 * PBSD batch b0088s2 -- differential test of the C++23 port against the
 * unmodified C oracle.
 *
 * Every function reachable in the batch is compared: sinpil() itself and the
 * four helpers it is built from (__kernel_sinpil, __kernel_cospil,
 * __kernel_sinl, __kernel_cosl).  The helpers are exercised directly as well
 * as through sinpil() because sinpil() only ever reaches __kernel_sinl() with
 * iy == 1, which would otherwise leave that function's iy == 0 arm untested.
 *
 * None of these functions writes through a pointer, so there is no output
 * buffer to guard; the entire observable result is the return value.  It is
 * therefore compared bit for bit -- all 16 bytes of the binary128 value --
 * rather than with ==, so that -0.0 vs +0.0 and differing NaN payloads are
 * caught too.
 *
 * Each function gets hand-written edge cases sitting on both sides of every
 * comparison the port performs, plus a fixed-seed randomised sweep of
 * 200000 iterations.
 */

#include <cfloat>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <math.h>
#include <quadmath.h>

#if LDBL_MANT_DIG != 113
#error "b0088s2 is ld128 code: compile with -mlong-double-128"
#endif

import pbsd.lib.msun.ld128.b0088s2;

namespace port = pbsd::lib_msun_ld128::b0088s2;

extern "C" {
long double ref_sinpil(long double);
long double ref___kernel_sinpil(long double);
long double ref___kernel_cospil(long double);
long double ref___kernel_sinl(long double, long double, int);
long double ref___kernel_cosl(long double, long double);
}

/* ------------------------------------------------------------------ */
/* bit-level helpers							*/
/* ------------------------------------------------------------------ */

static long double
ld_bits(std::uint64_t hi, std::uint64_t lo)
{
	unsigned char b[16];
	long double v;

	std::memcpy(b + 0, &lo, 8);
	std::memcpy(b + 8, &hi, 8);
	std::memcpy(&v, b, 16);
	return (v);
}

static bool
same_ld(long double a, long double b)
{
	unsigned char ba[16], bb[16];

	std::memcpy(ba, &a, 16);
	std::memcpy(bb, &b, 16);
	return (std::memcmp(ba, bb, 16) == 0);
}

static void
show_ld(const char *tag, long double v)
{
	unsigned char b[16];
	char txt[128];

	std::memcpy(b, &v, 16);
	quadmath_snprintf(txt, sizeof(txt), "%.36Qg", (__float128)v);
	std::printf("%s%s [", tag, txt);
	for (int i = 15; i >= 0; i--)
		std::printf("%02x", b[i]);
	std::printf("]");
}

static long double
nextup(long double v)
{
	return ((long double)nextafterq((__float128)v, HUGE_VALQ));
}

static long double
nextdown(long double v)
{
	return ((long double)nextafterq((__float128)v, -HUGE_VALQ));
}

static long double
ulps(long double v, int n)
{
	for (; n > 0; n--)
		v = nextup(v);
	for (; n < 0; n++)
		v = nextdown(v);
	return (v);
}

static long double
ld_pow2(int e)
{
	return ((long double)ldexpq((__float128)1, e));
}

static const long double ld_inf = ld_bits(0x7fff000000000000ULL, 0);
static const long double ld_nan = ld_bits(0x7fff800000000000ULL, 0);
static const long double ld_nan2 = ld_bits(0x7fff000000000000ULL, 3);

/* ------------------------------------------------------------------ */
/* statistics								*/
/* ------------------------------------------------------------------ */

struct Stat {
	const char	*name;
	long long	 cases;
	long long	 fails;
	int		 shown;
};

static Stat st_sinpil	= { "sinpil", 0, 0, 0 };
static Stat st_ksinpil	= { "__kernel_sinpil", 0, 0, 0 };
static Stat st_kcospil	= { "__kernel_cospil", 0, 0, 0 };
static Stat st_ksinl	= { "__kernel_sinl", 0, 0, 0 };
static Stat st_kcosl	= { "__kernel_cosl", 0, 0, 0 };

static Stat *const all_stats[] = {
	&st_sinpil, &st_ksinpil, &st_kcospil, &st_ksinl, &st_kcosl,
};
#define	NSTATS	((int)(sizeof(all_stats) / sizeof(all_stats[0])))

#define	MAX_SHOWN	5

static bool
fail_head(Stat &s)
{
	s.fails++;
	if (s.shown >= MAX_SHOWN)
		return (false);
	s.shown++;
	std::printf("FAIL %s: ", s.name);
	return (true);
}

/* ------------------------------------------------------------------ */
/* case drivers								*/
/* ------------------------------------------------------------------ */

static void
case_sinpil(long double x)
{
	long double p, r;

	st_sinpil.cases++;
	p = port::sinpil(x);
	r = ref_sinpil(x);
	if (!same_ld(p, r) && fail_head(st_sinpil)) {
		show_ld("x=", x);
		show_ld(" port=", p);
		show_ld(" ref=", r);
		std::printf("\n");
	}
}

static void
case_ksinpil(long double x)
{
	long double p, r;

	st_ksinpil.cases++;
	p = port::__kernel_sinpil(x);
	r = ref___kernel_sinpil(x);
	if (!same_ld(p, r) && fail_head(st_ksinpil)) {
		show_ld("x=", x);
		show_ld(" port=", p);
		show_ld(" ref=", r);
		std::printf("\n");
	}
}

static void
case_kcospil(long double x)
{
	long double p, r;

	st_kcospil.cases++;
	p = port::__kernel_cospil(x);
	r = ref___kernel_cospil(x);
	if (!same_ld(p, r) && fail_head(st_kcospil)) {
		show_ld("x=", x);
		show_ld(" port=", p);
		show_ld(" ref=", r);
		std::printf("\n");
	}
}

static void
case_ksinl(long double x, long double y, int iy)
{
	long double p, r;

	st_ksinl.cases++;
	p = port::__kernel_sinl(x, y, iy);
	r = ref___kernel_sinl(x, y, iy);
	if (!same_ld(p, r) && fail_head(st_ksinl)) {
		show_ld("x=", x);
		show_ld(" y=", y);
		std::printf(" iy=%d", iy);
		show_ld(" port=", p);
		show_ld(" ref=", r);
		std::printf("\n");
	}
}

static void
case_kcosl(long double x, long double y)
{
	long double p, r;

	st_kcosl.cases++;
	p = port::__kernel_cosl(x, y);
	r = ref___kernel_cosl(x, y);
	if (!same_ld(p, r) && fail_head(st_kcosl)) {
		show_ld("x=", x);
		show_ld(" y=", y);
		show_ld(" port=", p);
		show_ld(" ref=", r);
		std::printf("\n");
	}
}

/* ------------------------------------------------------------------ */
/* fixed-seed PRNG (splitmix64)						*/
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state;

static void
rng_seed(std::uint64_t s)
{
	rng_state = s;
}

static std::uint64_t
rng_u64(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static std::uint32_t
rng_below(std::uint32_t n)
{
	return ((std::uint32_t)(rng_u64() % n));
}

/* Uniform binary128 in [0, 1). */
static long double
rng_unit(void)
{
	long double v;

	v = (long double)(rng_u64() >> 1) * 0x1p-63L;
	v += (long double)(rng_u64() >> 1) * 0x1p-126L;
	return (v);
}

/* A binary128 with the given unbiased exponent and a random mantissa. */
static long double
rng_scaled(int e)
{
	std::uint64_t manh = rng_u64() & 0x0000ffffffffffffULL;
	std::uint64_t manl = rng_u64();
	int be = e + 16383;

	if (be < 1)
		be = 1;
	if (be > 0x7ffe)
		be = 0x7ffe;
	return (ld_bits(((std::uint64_t)be << 48) | manh, manl));
}

/* ------------------------------------------------------------------ */
/* sinpil() inputs							*/
/* ------------------------------------------------------------------ */

/*
 * Every constant sinpil() compares against, so the randomised sweep is not
 * the only thing straddling a boundary.
 */
static const long double sinpil_anchors[] = {
	0.0L,
	0x1p-16494L,			/* LDBL_TRUE_MIN */
	0x1p-16400L,
	LDBL_MIN,
	0x1p-113L,
	0x1p-62L,
	0x1p-61L,
	0x1p-60L,			/* tiny-argument threshold */
	0x1p-59L,
	0x1p-30L,
	0.1L,
	0.2L,
	0.25L,				/* sinpil/cospil switch */
	0.3L,
	0.4L,
	0.5L,				/* cospil argument reflection */
	0.6L,
	0.7L,
	0.75L,				/* cospil/sinpil switch */
	0.8L,
	0.9L,
	0.999L,
	1.0L,				/* |x| < 1 switch */
	1.25L, 1.5L, 1.75L,
	2.0L, 2.25L, 2.5L, 2.75L,
	3.0L, 3.25L, 3.5L, 3.75L,
	4.0L, 5.0L, 6.0L, 7.0L, 8.0L, 9.0L,
	100.0L, 100.25L, 100.5L, 100.75L,
	101.0L, 101.25L, 101.5L, 101.75L,
	12345.678L,
	0x1p47L, 0x1p47L + 0.25L, 0x1p47L + 0.5L, 0x1p47L + 0.75L,
	0x1p48L - 1.0L, 0x1p48L - 0.25L,
	0x1p48L,			/* FFLOORL128 e < 48 switch */
	0x1p48L + 0.25L, 0x1p48L + 0.5L, 0x1p48L + 0.75L,
	0x1p48L + 1.0L, 0x1p48L + 1.25L,
	0x1p49L + 0.5L,
	0x1p64L, 0x1p64L + 0.25L, 0x1p64L + 0.5L,
	0x1p100L, 0x1p100L + 0.5L,
	0x1p111L, 0x1p111L + 0.5L,
	0x1p112L - 1.0L, 0x1p112L - 0.5L,
	0x1p112L,			/* huge-argument threshold */
	0x1p112L + 4.0L,
	0x1p113L,
	0x1p114L,
	LDBL_MAX,
};

#define	NANCHOR	((int)(sizeof(sinpil_anchors) / sizeof(sinpil_anchors[0])))

static long double
rng_sinpil_arg(void)
{
	static const long double q[] = { 0.0L, 0.25L, 0.5L, 0.75L };
	long double v;
	std::uint64_t n;
	int k;

	switch (rng_below(14)) {
	case 0:				/* anchor +- a few ulps */
		v = ulps(sinpil_anchors[rng_below(NANCHOR)],
		    (int)rng_below(7) - 3);
		break;
	case 1:				/* below the 0x1p-60 threshold */
		v = rng_scaled(-61 - (int)rng_below(80));
		break;
	case 2:				/* [0x1p-60, 0.25) */
		v = rng_scaled(-60 + (int)rng_below(58));
		break;
	case 3:				/* [0.25, 1) */
		v = 0.25L + rng_unit() * 0.75L;
		break;
	case 4:				/* uniform in [0, 1) */
		v = rng_unit();
		break;
	case 5:				/* integer, exponent < 48 */
		n = rng_u64() >> (17 + rng_below(47));
		v = (long double)n;
		break;
	case 6:				/* integer + exact quarter, e < 48 */
		n = rng_u64() >> (17 + rng_below(47));
		v = (long double)n + q[rng_below(4)];
		break;
	case 7:				/* integer + exact quarter, e >= 48 */
		k = 48 + (int)rng_below(63);
		v = (long double)ldexpq((__float128)(long double)
		    (rng_u64() | (1ULL << 63)), k - 63);
		v = v + q[rng_below(4)];
		break;
	case 8:				/* integer + arbitrary fraction */
		n = rng_u64() >> (17 + rng_below(47));
		v = (long double)n + rng_unit();
		break;
	case 9:				/* a few ulps either side of a whole */
		n = rng_u64() >> (17 + rng_below(47));
		v = ulps((long double)n, (int)rng_below(9) - 4);
		break;
	case 10:			/* around the 0x1p112 threshold */
		v = ulps(0x1p112L, (int)rng_below(9) - 4);
		break;
	case 11:			/* huge */
		v = rng_scaled(112 + (int)rng_below(4000));
		break;
	case 12:			/* subnormal */
		v = ld_bits(rng_u64() & 0x0000ffffffffffffULL, rng_u64());
		break;
	default:			/* anything at all, inf and nan too */
		if (rng_below(4) == 0)
			v = (rng_u64() & 1) ? ld_inf :
			    ((rng_u64() & 1) ? ld_nan : ld_nan2);
		else
			v = ld_bits(rng_u64() & 0x7fffffffffffffffULL,
			    rng_u64());
		break;
	}
	return ((rng_u64() & 1) ? -v : v);
}

/* ------------------------------------------------------------------ */
/* kernel inputs							*/
/* ------------------------------------------------------------------ */

/*
 * __kernel_sinl()/__kernel_cosl() only ever see the reduced argument pair
 * (hi, lo) produced by __kernel_sinpil()/__kernel_cospil(): hi in
 * [-pi/4, pi/4] with lo a tail 60-odd bits below it.  Both that realistic
 * shape and deliberately wild values are used.
 */
static void
rng_kernel_pair(long double &x, long double &y)
{
	switch (rng_below(8)) {
	case 0:
		x = 0.0L;
		y = 0.0L;
		break;
	case 1:
		x = (rng_unit() * 2 - 1) * 0.7853981633974483096L;
		y = 0.0L;
		break;
	case 2:
		x = (rng_unit() * 2 - 1) * 0.7853981633974483096L;
		y = (long double)ldexpq((__float128)(rng_unit() * 2 - 1),
		    -60 - (int)rng_below(20));
		break;
	case 3:
		x = (rng_unit() * 2 - 1) * 0.7853981633974483096L;
		y = (rng_unit() * 2 - 1) * x;
		break;
	case 4:
		x = rng_scaled(-(int)rng_below(140));
		y = rng_scaled(-(int)rng_below(140));
		if (rng_u64() & 1)
			x = -x;
		if (rng_u64() & 1)
			y = -y;
		break;
	case 5:
		x = rng_scaled((int)rng_below(200) - 100);
		y = rng_scaled((int)rng_below(200) - 100);
		if (rng_u64() & 1)
			x = -x;
		if (rng_u64() & 1)
			y = -y;
		break;
	case 6:
		x = ld_bits(rng_u64() & 0x7ffeffffffffffffULL, rng_u64());
		y = ld_bits(rng_u64() & 0x7ffeffffffffffffULL, rng_u64());
		break;
	default:
		x = (rng_u64() & 1) ? ld_inf : ld_nan;
		y = rng_unit();
		if (rng_u64() & 1) {
			long double t = x;

			x = y;
			y = t;
		}
		break;
	}
}

/* Arguments __kernel_sinpil()/__kernel_cospil() actually see: [0, 0.5]. */
static long double
rng_kpi_arg(void)
{
	switch (rng_below(8)) {
	case 0:
		return (0.0L);
	case 1:
		return (rng_unit() * 0.25L);
	case 2:
		return (0.25L + rng_unit() * 0.25L);
	case 3:
		return (ulps(0.25L, (int)rng_below(9) - 4));
	case 4:
		return (rng_scaled(-1 - (int)rng_below(130)));
	case 5:
		return (rng_scaled(-(int)rng_below(20)) *
		    ((rng_u64() & 1) ? -1 : 1));
	case 6:
		return (ld_bits(rng_u64() & 0x7ffeffffffffffffULL,
		    rng_u64()));
	default:
		return ((rng_u64() & 1) ? ld_inf : ld_nan);
	}
}

/* ------------------------------------------------------------------ */
/* hand-written edge cases						*/
/* ------------------------------------------------------------------ */

#define	RANDOM_ITERS	200000

static void
edge_sinpil(void)
{
	static const long double frac8[] = {
		0.0L, 0.125L, 0.25L, 0.375L, 0.5L, 0.625L, 0.75L, 0.875L,
	};
	static const long double frac4[] = { 0.0L, 0.25L, 0.5L, 0.75L };
	int i, j;

	/* Zeroes, infinities and NaNs, both signs. */
	case_sinpil(0.0L);
	case_sinpil(-0.0L);
	case_sinpil(ld_inf);
	case_sinpil(-ld_inf);
	case_sinpil(ld_nan);
	case_sinpil(-ld_nan);
	case_sinpil(ld_nan2);
	case_sinpil(-ld_nan2);

	/* Both sides of every threshold, both signs. */
	for (i = 0; i < NANCHOR; i++)
		for (int k = -2; k <= 2; k++) {
			long double w = ulps(sinpil_anchors[i], k);

			case_sinpil(w);
			case_sinpil(-w);
		}

	/*
	 * Small integers with every exact eighth: drives ar == 0,
	 * ar <= 0.25, 0.25 < ar < 0.5, ar == 0.5, 0.5 < ar < 0.75,
	 * ar == 0.75 and ar > 0.75 against both even and odd integer parts.
	 */
	for (i = 0; i < 260; i++)
		for (j = 0; j < 8; j++) {
			long double v = (long double)i + frac8[j];

			case_sinpil(v);
			case_sinpil(-v);
			case_sinpil(nextup(v));
			case_sinpil(nextdown(v));
			case_sinpil(-nextup(v));
		}

	/*
	 * The same quarters at every exponent, so both arms of the
	 * FFLOORL128 `e < 48' split and the whole 0 <= e <= 111 shift range
	 * are covered, along with the 0x1p112 cut-off.
	 */
	for (i = 0; i <= 116; i++) {
		long double base = ld_pow2(i);

		for (j = 0; j < 4; j++) {
			case_sinpil(base + frac4[j]);
			case_sinpil(-(base + frac4[j]));
			case_sinpil(base + 1 + frac4[j]);
			case_sinpil(-(base + 1 + frac4[j]));
			case_sinpil(base - 1 + frac4[j]);
			case_sinpil(base + 2 + frac4[j]);
		}
		case_sinpil(nextup(base));
		case_sinpil(nextdown(base));
		case_sinpil(-nextup(base));
	}
}

static void
edge_kpi(void)
{
	static const long double v[] = {
		0.0L, 0x1p-16494L, LDBL_MIN, 0x1p-120L, 0x1p-113L, 0x1p-60L,
		0x1p-53L, 0x1p-30L, 0.0625L, 0.125L, 0.1L, 0.2L, 0.25L,
		0.3L, 0.375L, 0.4L, 0.5L, 0.6L, 0.75L, 0.9L, 1.0L, 1.5L,
		2.0L, 3.0L, 100.0L, 0x1p60L, 0x1p112L, LDBL_MAX,
	};
	int n = (int)(sizeof(v) / sizeof(v[0]));

	for (int i = 0; i < n; i++)
		for (int k = -2; k <= 2; k++) {
			long double w = ulps(v[i], k);

			case_ksinpil(w);
			case_ksinpil(-w);
			case_kcospil(w);
			case_kcospil(-w);
		}

	case_ksinpil(ld_inf);
	case_ksinpil(-ld_inf);
	case_ksinpil(ld_nan);
	case_kcospil(ld_inf);
	case_kcospil(-ld_inf);
	case_kcospil(ld_nan);
}

static void
edge_kernels(void)
{
	static const long double xs[] = {
		0.0L, 0x1p-16494L, LDBL_MIN, 0x1p-113L, 0x1p-60L, 0x1p-20L,
		0.0009765625L, 0.1L, 0.25L, 0.5L, 0.7853981633974483096L,
		1.0L, 1.5L, 3.14159265358979323846L, 10.0L, 1e10L,
		0x1p60L, LDBL_MAX,
	};
	static const long double ys[] = {
		0.0L, 0x1p-16494L, 0x1p-120L, 0x1p-70L, 0x1p-60L, 0x1p-30L,
		1e-10L, 0.5L, 1.0L, 2.0L, 1e10L,
	};
	static const int iys[] = { 0, 1, -1, 2, 0x7fffffff, -0x7fffffff - 1 };
	int nx = (int)(sizeof(xs) / sizeof(xs[0]));
	int ny = (int)(sizeof(ys) / sizeof(ys[0]));
	int ni = (int)(sizeof(iys) / sizeof(iys[0]));

	for (int i = 0; i < nx; i++)
		for (int j = 0; j < ny; j++)
			for (int sx = 0; sx < 2; sx++)
				for (int sy = 0; sy < 2; sy++) {
					long double x = sx ? -xs[i] : xs[i];
					long double y = sy ? -ys[j] : ys[j];

					for (int k = 0; k < ni; k++)
						case_ksinl(x, y, iys[k]);
					case_kcosl(x, y);
				}

	for (int k = 0; k < ni; k++) {
		case_ksinl(ld_inf, 1.0L, iys[k]);
		case_ksinl(-ld_inf, 1.0L, iys[k]);
		case_ksinl(ld_nan, 1.0L, iys[k]);
		case_ksinl(1.0L, ld_inf, iys[k]);
		case_ksinl(1.0L, ld_nan, iys[k]);
		case_ksinl(0.0L, 0.0L, iys[k]);
	}
	case_kcosl(ld_inf, 1.0L);
	case_kcosl(-ld_inf, 1.0L);
	case_kcosl(ld_nan, 1.0L);
	case_kcosl(1.0L, ld_inf);
	case_kcosl(1.0L, ld_nan);
	case_kcosl(0.0L, ld_inf);
	case_kcosl(ld_inf, 0.0L);
	case_kcosl(0.0L, 0.0L);
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	long long tot_cases = 0, tot_fails = 0;
	int i;

	edge_sinpil();
	edge_kpi();
	edge_kernels();

	rng_seed(0x5eed0088c0ffee02ULL);
	for (i = 0; i < RANDOM_ITERS; i++)
		case_sinpil(rng_sinpil_arg());

	rng_seed(0x5eed0088c0ffee03ULL);
	for (i = 0; i < RANDOM_ITERS; i++)
		case_ksinpil(rng_kpi_arg());

	rng_seed(0x5eed0088c0ffee04ULL);
	for (i = 0; i < RANDOM_ITERS; i++)
		case_kcospil(rng_kpi_arg());

	rng_seed(0x5eed0088c0ffee05ULL);
	for (i = 0; i < RANDOM_ITERS; i++) {
		long double x, y;
		int iy;

		rng_kernel_pair(x, y);
		switch (rng_below(4)) {
		case 0:
			iy = 0;
			break;
		case 1:
			iy = 1;
			break;
		case 2:
			iy = -1;
			break;
		default:
			iy = (int)rng_u64();
			break;
		}
		case_ksinl(x, y, iy);
	}

	rng_seed(0x5eed0088c0ffee06ULL);
	for (i = 0; i < RANDOM_ITERS; i++) {
		long double x, y;

		rng_kernel_pair(x, y);
		case_kcosl(x, y);
	}

	std::printf("\n%-20s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("---------------------------------------------------"
	    "----\n");
	for (i = 0; i < NSTATS; i++) {
		std::printf("%-20s %12lld %10lld   %s\n", all_stats[i]->name,
		    all_stats[i]->cases, all_stats[i]->fails,
		    all_stats[i]->fails == 0 ? "ok" : "FAILED");
		tot_cases += all_stats[i]->cases;
		tot_fails += all_stats[i]->fails;
	}
	std::printf("---------------------------------------------------"
	    "----\n");
	std::printf("%-20s %12lld %10lld   %s\n", "TOTAL", tot_cases,
	    tot_fails, tot_fails == 0 ? "ok" : "FAILED");

	return (tot_fails == 0 ? 0 : 1);
}
