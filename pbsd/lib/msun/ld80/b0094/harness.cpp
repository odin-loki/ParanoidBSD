/*
 * Differential test harness for PBSD batch b0094.
 *
 * Compares every exported function in port.cppm against the unmodified C
 * reference in oracle.c, bit-for-bit on long double representations.
 */

#include <cfloat>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.ld80.b0094;

namespace port = pbsd::lib_msun_ld80::b0094;

extern "C" {
long double ref___kernel_tanl(long double x, long double y, int iy);
long double ref___exp__D(long double x, long double c);
long double ref_expl(long double x);
long double ref_expm1l(long double x);
long double ref_exp2l(long double x);
}

#if LDBL_MANT_DIG == 64
static const std::size_t LD_SIG = 10;
#else
static const std::size_t LD_SIG = sizeof(long double);
#endif

struct ldrep {
	unsigned char b[sizeof(long double)];
};

static bool
guarded_equal(const void *pa, const void *pb, std::size_t n)
{
	unsigned char a[64], b[64];

	std::memset(a, 0x7f, sizeof(a));
	std::memset(b, 0x7f, sizeof(b));
	std::memcpy(a + 16, pa, n);
	std::memcpy(b + 16, pb, n);
	return (std::memcmp(a, b, sizeof(a)) == 0);
}

static ldrep
ldbits(long double x)
{
	ldrep r;

	std::memset(r.b, 0, sizeof(r.b));
	std::memcpy(r.b, &x, LD_SIG);
	return r;
}

static long double
mkld(std::uint16_t se, std::uint64_t m)
{
	unsigned char b[sizeof(long double)];
	long double x;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &m, sizeof(m));
	std::memcpy(b + 8, &se, sizeof(se));
	std::memcpy(&x, b, sizeof(x));
	return x;
}

static void
ldhex(const ldrep &r)
{
	for (std::size_t i = LD_SIG; i-- > 0;)
		std::printf("%02x", r.b[i]);
}

struct stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned reported;
};

static stat st_kernel_tanl = { "__kernel_tanl", 0, 0, 0 };
static stat st_exp__D = { "__exp__D", 0, 0, 0 };
static stat st_expl = { "expl", 0, 0, 0 };
static stat st_expm1l = { "expm1l", 0, 0, 0 };
static stat st_exp2l = { "exp2l", 0, 0, 0 };

static const unsigned MAX_REPORT = 8;

static void
check_ld(stat &st, long double p, long double o, const char *tag)
{
	ldrep rp, ro;

	st.cases++;
	rp = ldbits(p);
	ro = ldbits(o);

	if (guarded_equal(rp.b, ro.b, sizeof(rp.b)))
		return;

	st.fails++;
	if (st.reported < MAX_REPORT) {
		st.reported++;
		std::printf("  %s FAIL [%s] port=", st.name, tag);
		ldhex(rp);
		std::printf(" ref=");
		ldhex(ro);
		std::printf("\n");
	}
}

static void
check_kernel_tanl(long double x, long double y, int iy, const char *tag)
{
	check_ld(st_kernel_tanl, port::__kernel_tanl(x, y, iy),
	    ref___kernel_tanl(x, y, iy), tag);
}

static void
check_exp__D(long double x, long double c, const char *tag)
{
	check_ld(st_exp__D, port::__exp__D(x, c), ref___exp__D(x, c), tag);
}

static void
check_expl(long double x, const char *tag)
{
	check_ld(st_expl, port::expl(x), ref_expl(x), tag);
}

static void
check_expm1l(long double x, const char *tag)
{
	check_ld(st_expm1l, port::expm1l(x), ref_expm1l(x), tag);
}

static void
check_exp2l(long double x, const char *tag)
{
	check_ld(st_exp2l, port::exp2l(x), ref_exp2l(x), tag);
}

/* ------------------------------------------------------------------ */
/* hand-written edge cases                                             */
/* ------------------------------------------------------------------ */

static const long double TAN_BOUND = 0.67434L;
static const long double EXPM1_T1 = -0.1659L;
static const long double EXPM1_T2 = 0.1659L;
static const long double O_THRESH = 11356.5234062941439488L;
static const long double U_THRESH = -11399.4985314888605581L;

struct ldcase {
	std::uint16_t se;
	std::uint64_t m;
};

static const ldcase ldvec[] = {
	{ 0x0000u, 0x0000000000000000ull },
	{ 0x8000u, 0x0000000000000000ull },
	{ 0x0000u, 0x0000000000000001ull },
	{ 0x8000u, 0x0000000000000001ull },
	{ 0x0000u, 0x0000000000000080ull },
	{ 0x0000u, 0x7fffffffffffffffull },
	{ 0x8000u, 0x7fffffffffffffffull },
	{ 0x0001u, 0x8000000000000000ull },
	{ 0x8001u, 0x8000000000000000ull },
	{ 0x3ffeu, 0x8000000000000000ull },
	{ 0xbffeu, 0x8000000000000000ull },
	{ 0x3fffu, 0x8000000000000000ull },
	{ 0xbfffu, 0x8000000000000000ull },
	{ 0x3fffu, 0x8000000000000001ull },
	{ 0x3fffu, 0x8080808080808080ull },
	{ 0xbfffu, 0xff00ff00ff00ff00ull },
	{ 0x7fffu, 0x8000000000000000ull },
	{ 0xffffu, 0x8000000000000000ull },
	{ 0x7fffu, 0xc000000000000000ull },
	{ 0xffffu, 0xc000000000000000ull },
	{ 0x7fffu, 0xffffffffffffffffull },
	{ 0xffffu, 0xffffffffffffffffull },
};
static const std::size_t NLDVEC = sizeof(ldvec) / sizeof(ldvec[0]);

static const int iyvec[] = {
	0, 1, -1, 2, -2, INT_MAX, INT_MIN, 0x7f, -0x7f, 0x80, -0x80,
};

static void
edge_cases(void)
{
	std::size_t i, j, k;

	/* __kernel_tanl: cross edge bit patterns, both sides of 0.67434 */
	for (i = 0; i < NLDVEC; i++)
		for (j = 0; j < NLDVEC; j++)
			for (k = 0; k < sizeof(iyvec) / sizeof(iyvec[0]); k++) {
				long double x = mkld(ldvec[i].se, ldvec[i].m);
				long double y = mkld(ldvec[j].se, ldvec[j].m);
				check_kernel_tanl(x, y, iyvec[k], "cross");
			}

	{
		static const long double xvals[] = {
			0.0L, -0.0L,
			TAN_BOUND, -TAN_BOUND,
			TAN_BOUND * 0.999999999999999L,
			-TAN_BOUND * 0.999999999999999L,
			TAN_BOUND * 1.000000000000001L,
			-TAN_BOUND * 1.000000000000001L,
			0.1L, -0.1L, 0.5L, -0.5L,
			1e-4932L, -1e-4932L,
			1e-100L, -1e-100L,
			LDBL_TRUE_MIN, -LDBL_TRUE_MIN,
		};
		static const long double yvals[] = {
			0.0L, -0.0L, 1.0L, -1.0L, TAN_BOUND, -TAN_BOUND,
			1e-30L, -1e-30L, 1e30L, -1e30L,
		};
		static const int iys[] = { 0, 1, -1 };
		std::size_t a, b, c;

		for (a = 0; a < sizeof(xvals) / sizeof(xvals[0]); a++)
			for (b = 0; b < sizeof(yvals) / sizeof(yvals[0]); b++)
				for (c = 0; c < sizeof(iys) / sizeof(iys[0]); c++)
					check_kernel_tanl(xvals[a], yvals[b],
					    iys[c], "tan-domain");
	}

	/* __exp__D: NaN, overflow/underflow boundaries, normal range */
	for (i = 0; i < NLDVEC; i++) {
		long double x = mkld(ldvec[i].se, ldvec[i].m);
		long double c = x * 0.1L;
		check_exp__D(x, c, "ldvec");
		check_exp__D(x, 0.0L, "ldvec-c0");
		check_exp__D(x, -c, "ldvec-cneg");
	}

	{
		static const long double xs[] = {
			0.0L, -0.0L, 1.0L, -1.0L, 0.5L, -0.5L,
			11356.0L, -11356.0L, 11400.0L, -11400.0L,
			1e-4000L, -1e-4000L, 1e4L, -1e4L,
			LDBL_MAX, -LDBL_MAX,
		};
		static const long double cs[] = {
			0.0L, 1e-20L, -1e-20L, 0.1L, -0.1L,
		};
		std::size_t a, b;

		for (a = 0; a < sizeof(xs) / sizeof(xs[0]); a++)
			for (b = 0; b < sizeof(cs) / sizeof(cs[0]); b++)
				check_exp__D(xs[a], cs[b], "expD-domain");
	}

	/* expl / expm1l / exp2l: exceptional and boundary values */
	for (i = 0; i < NLDVEC; i++) {
		long double x = mkld(ldvec[i].se, ldvec[i].m);
		check_expl(x, "ldvec");
		check_expm1l(x, "ldvec");
		check_exp2l(x, "ldvec");
	}

	{
		static const long double xs[] = {
			0.0L, -0.0L, 1.0L, -1.0L,
			0x1p-76L, -0x1p-76L, 0x1p-74L, -0x1p-74L,
			0x1p-65L, -0x1p-65L, 0x1p-75L, -0x1p-75L,
			64.0L, -64.0L, 65.0L, -65.0L,
			8192.0L, -8192.0L, 8191.0L, -8191.0L,
			16384.0L, -16384.0L, 16383.0L, -16383.0L,
			-16446.0L, -16447.0L, 16446.0L,
			O_THRESH, O_THRESH + 1.0L, O_THRESH - 1.0L,
			U_THRESH, U_THRESH + 1.0L, U_THRESH - 1.0L,
			EXPM1_T1, EXPM1_T2,
			EXPM1_T1 - 1e-6L, EXPM1_T2 + 1e-6L,
			EXPM1_T1 + 1e-6L, EXPM1_T2 - 1e-6L,
			0.1658L, -0.1658L, 0.1660L, -0.1660L,
			1e-20L, -1e-20L, 1e20L, -1e20L,
			LDBL_MAX, -LDBL_MAX,
			LDBL_MIN, -LDBL_MIN,
		};
		std::size_t a;

		for (a = 0; a < sizeof(xs) / sizeof(xs[0]); a++) {
			check_expl(xs[a], "expl-domain");
			check_expm1l(xs[a], "expm1l-domain");
			check_exp2l(xs[a], "exp2l-domain");
		}
	}
}

/* ------------------------------------------------------------------ */
/* fixed-seed randomised sweep                                         */
/* ------------------------------------------------------------------ */

static std::uint64_t rng_state;

static std::uint64_t
rng_next(void)
{
	std::uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ull;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
	return (z ^ (z >> 31));
}

static long double
rng_ld(void)
{
	std::uint64_t r = rng_next();
	std::uint16_t se = (std::uint16_t)(r & 0xffffu);
	std::uint64_t m = rng_next();

	if ((r >> 16) & 1)
		se |= 0x8000u;
	return mkld(se, m);
}

static long double
rng_small(void)
{
	long double s = (long double)(rng_next() % 1000000u) / 1000000.0L;
	if (rng_next() & 1)
		s = -s;
	return s;
}

static const unsigned long long ITERS = 200000ull;

static void
random_sweep(void)
{
	unsigned long long i;

	rng_state = 0x243f6a8885a308d3ull;
	for (i = 0; i < ITERS; i++) {
		long double x = rng_ld();
		long double y = rng_ld();
		int iy = (int)(rng_next() & 0xffu);

		if ((i % 97) == 0)
			iy = 0;
		if ((i % 89) == 0)
			iy = 1;
		if ((i % 83) == 0)
			iy = -1;

		check_kernel_tanl(x, y, iy, "random");

		x = rng_ld();
		long double c = rng_small();
		check_exp__D(x, c, "random");
		check_exp__D(x, 0.0L, "random-c0");

		x = rng_ld();
		check_expl(x, "random");
		check_expm1l(x, "random");
		check_exp2l(x, "random");
	}
}

static void
row(const stat &s)
{
	std::printf("  %-18s %12llu %10llu   %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "PASS" : "FAIL");
}

int
main(void)
{
	unsigned long long fails;

	std::printf("pbsd batch b0094 differential test\n");
	std::printf("LDBL_MANT_DIG=%d, comparing %zu significant bytes of "
	    "long double\n\n", (int)LDBL_MANT_DIG, LD_SIG);

	edge_cases();
	random_sweep();

	std::printf("\n  %-18s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  ------------------------------------------------------\n");
	row(st_kernel_tanl);
	row(st_exp__D);
	row(st_expl);
	row(st_expm1l);
	row(st_exp2l);

	fails = st_kernel_tanl.fails + st_exp__D.fails + st_expl.fails +
	    st_expm1l.fails + st_exp2l.fails;
	std::printf("\n%s: %llu total failures\n",
	    fails == 0 ? "PASS" : "FAIL", fails);

	return (fails == 0 ? 0 : 1);
}
