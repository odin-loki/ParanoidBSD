/*
 * Differential test harness for PBSD batch b0082.
 *
 * Every function and exported constant in port.cppm is compared, bit for bit,
 * against the unmodified C reference in oracle.c.
 */

#include <cfloat>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.ld80.b0082;

namespace port = pbsd::lib_msun_ld80::b0082;

extern "C" {
long double ref___kernel_sinl(long double x, long double y, int iy);
long double ref___kernel_cosl(long double x, long double y);

extern const long double ref_pS0, ref_pS1, ref_pS2, ref_pS3, ref_pS4,
    ref_pS5, ref_pS6, ref_qS1, ref_qS2, ref_qS3, ref_qS4, ref_qS5;
extern const long double ref_atanhi[];
extern const long double ref_atanlo[];
extern const long double ref_aT[];
extern const long double ref_pi_lo;
extern const int ref_atanhi_len;
extern const int ref_atanlo_len;
extern const int ref_aT_len;
}

/* ------------------------------------------------------------------ */
/* raw-representation plumbing                                         */
/* ------------------------------------------------------------------ */

#if LDBL_MANT_DIG == 64
static const std::size_t LD_SIG = 10;
#else
static const std::size_t LD_SIG = sizeof(long double);
#endif

static const unsigned char GUARD = 0x7f;
static const std::size_t GUARD_BUF = 64;
static const std::size_t GUARD_OFF = 16;

struct ldrep {
	unsigned char b[sizeof(long double)];
};

static bool
guarded_equal(const void *pa, const void *pb, std::size_t n)
{
	unsigned char a[GUARD_BUF], b[GUARD_BUF];

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	std::memcpy(a + GUARD_OFF, pa, n);
	std::memcpy(b + GUARD_OFF, pb, n);
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
	std::size_t i;

	for (i = LD_SIG; i-- > 0;)
		std::printf("%02x", r.b[i]);
}

/* ------------------------------------------------------------------ */
/* bookkeeping                                                         */
/* ------------------------------------------------------------------ */

struct stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned reported;
};

static stat st_invtrig = { "invtrig_const", 0, 0, 0 };
static stat st_kernel_sinl = { "__kernel_sinl", 0, 0, 0 };
static stat st_kernel_cosl = { "__kernel_cosl", 0, 0, 0 };

static const unsigned MAX_REPORT = 8;

/* ------------------------------------------------------------------ */
/* invtrig constant checker                                            */
/* ------------------------------------------------------------------ */

static void
check_ld_const(const char *tag, long double p, long double o)
{
	ldrep rp, ro;

	st_invtrig.cases++;

	rp = ldbits(p);
	ro = ldbits(o);

	if (guarded_equal(rp.b, ro.b, sizeof(rp.b)))
		return;

	st_invtrig.fails++;
	if (st_invtrig.reported < MAX_REPORT) {
		st_invtrig.reported++;
		std::printf("  invtrig_const FAIL [%s] port=", tag);
		ldhex(rp);
		std::printf(" ref=");
		ldhex(ro);
		std::printf("\n");
	}
}

static void
check_invtrig_tables(void)
{
	check_ld_const("pS0", port::pS0, ref_pS0);
	check_ld_const("pS1", port::pS1, ref_pS1);
	check_ld_const("pS2", port::pS2, ref_pS2);
	check_ld_const("pS3", port::pS3, ref_pS3);
	check_ld_const("pS4", port::pS4, ref_pS4);
	check_ld_const("pS5", port::pS5, ref_pS5);
	check_ld_const("pS6", port::pS6, ref_pS6);
	check_ld_const("qS1", port::qS1, ref_qS1);
	check_ld_const("qS2", port::qS2, ref_qS2);
	check_ld_const("qS3", port::qS3, ref_qS3);
	check_ld_const("qS4", port::qS4, ref_qS4);
	check_ld_const("qS5", port::qS5, ref_qS5);
	check_ld_const("pi_lo", port::pi_lo, ref_pi_lo);

	for (int i = 0; i < ref_atanhi_len; i++)
		check_ld_const("atanhi", port::atanhi[i], ref_atanhi[i]);
	for (int i = 0; i < ref_atanlo_len; i++)
		check_ld_const("atanlo", port::atanlo[i], ref_atanlo[i]);
	for (int i = 0; i < ref_aT_len; i++)
		check_ld_const("aT", port::aT[i], ref_aT[i]);
}

/* ------------------------------------------------------------------ */
/* kernel function checkers                                            */
/* ------------------------------------------------------------------ */

static void
check_kernel_sinl(long double x, long double y, int iy, const char *tag)
{
	ldrep rp, ro;
	long double p, o;

	st_kernel_sinl.cases++;

	p = port::__kernel_sinl(x, y, iy);
	o = ref___kernel_sinl(x, y, iy);

	rp = ldbits(p);
	ro = ldbits(o);

	if (guarded_equal(rp.b, ro.b, sizeof(rp.b)))
		return;

	st_kernel_sinl.fails++;
	if (st_kernel_sinl.reported < MAX_REPORT) {
		st_kernel_sinl.reported++;
		std::printf("  __kernel_sinl FAIL [%s] x=", tag);
		ldhex(ldbits(x));
		std::printf(" y=");
		ldhex(ldbits(y));
		std::printf(" iy=%d port=", iy);
		ldhex(rp);
		std::printf(" ref=");
		ldhex(ro);
		std::printf("\n");
	}
}

static void
check_kernel_cosl(long double x, long double y, const char *tag)
{
	ldrep rp, ro;
	long double p, o;

	st_kernel_cosl.cases++;

	p = port::__kernel_cosl(x, y);
	o = ref___kernel_cosl(x, y);

	rp = ldbits(p);
	ro = ldbits(o);

	if (guarded_equal(rp.b, ro.b, sizeof(rp.b)))
		return;

	st_kernel_cosl.fails++;
	if (st_kernel_cosl.reported < MAX_REPORT) {
		st_kernel_cosl.reported++;
		std::printf("  __kernel_cosl FAIL [%s] x=", tag);
		ldhex(ldbits(x));
		std::printf(" y=");
		ldhex(ldbits(y));
		std::printf(" port=");
		ldhex(rp);
		std::printf(" ref=");
		ldhex(ro);
		std::printf("\n");
	}
}

/* ------------------------------------------------------------------ */
/* hand-written edge vectors                                           */
/* ------------------------------------------------------------------ */

static const long double PI4 = 0.78539816339744830962L;
static const long double NPI4 = -0.78539816339744830962L;

struct ldcase {
	std::uint16_t se;
	std::uint64_t m;
};

static const ldcase ldvec[] = {
	{ 0x0000u, 0x0000000000000000ull },	/* +0 */
	{ 0x8000u, 0x0000000000000000ull },	/* -0 */
	{ 0x0000u, 0x0000000000000001ull },	/* smallest +subnormal */
	{ 0x8000u, 0x0000000000000001ull },	/* smallest -subnormal */
	{ 0x0000u, 0x0000000000000080ull },
	{ 0x0000u, 0x7fffffffffffffffull },	/* largest +subnormal */
	{ 0x8000u, 0x7fffffffffffffffull },	/* largest -subnormal */
	{ 0x0001u, 0x8000000000000000ull },	/* smallest +normal */
	{ 0x8001u, 0x8000000000000000ull },	/* smallest -normal */
	{ 0x3ffeu, 0x8000000000000000ull },	/* +0.5 */
	{ 0xbffeu, 0x8000000000000000ull },	/* -0.5 */
	{ 0x3fffu, 0x8000000000000000ull },	/* +1 */
	{ 0xbfffu, 0x8000000000000000ull },	/* -1 */
	{ 0x3fffu, 0x8000000000000001ull },
	{ 0x3ffeu, 0xffffffffffffffffull },
	{ 0x3ffeu, 0xc90fdaa22168c235ull },	/* +pi/4 */
	{ 0xbffeu, 0xc90fdaa22168c235ull },	/* -pi/4 */
	{ 0x3ffdu, 0xc90fdaa22168c235ull },	/* just below +pi/4 */
	{ 0xbffdu, 0xc90fdaa22168c235ull },	/* just above -pi/4 */
	{ 0x3fffu, 0x8080808080808080ull },
	{ 0xbfffu, 0xff00ff00ff00ff00ull },
};
static const std::size_t NLDVEC = sizeof(ldvec) / sizeof(ldvec[0]);

static const int iyvec[] = {
	0, 1, -1, 2, -2, 3, -3, INT_MAX, INT_MIN, 0x7f, -0x7f, 0x80, -0x80,
};
static const std::size_t NIYVEC = sizeof(iyvec) / sizeof(iyvec[0]);

static void
edge_cases(void)
{
	std::size_t i, j, k;

	check_invtrig_tables();

	/*
	 * Domain [-0.7854, 0.7854]: cross every edge x with every edge y and
	 * every iy branch selector.
	 */
	for (i = 0; i < NLDVEC; i++)
		for (j = 0; j < NLDVEC; j++)
			for (k = 0; k < NIYVEC; k++) {
				long double x = mkld(ldvec[i].se, ldvec[i].m);
				long double y = mkld(ldvec[j].se, ldvec[j].m);

				check_kernel_sinl(x, y, iyvec[k], "cross");
				check_kernel_cosl(x, y, "cross");
			}

	/*
	 * Explicit domain boundary values and neighbours: both sides of pi/4.
	 */
	{
		static const long double xvals[] = {
			0.0L, -0.0L,
			PI4, NPI4,
			PI4 * 0.999999999999999L,
			NPI4 * 0.999999999999999L,
			PI4 * 1.000000000000001L,
			NPI4 * 1.000000000000001L,
			1e-4932L, -1e-4932L,
			1e-100L, -1e-100L,
			0.1L, -0.1L, 0.25L, -0.25L, 0.5L, -0.5L,
			LDBL_TRUE_MIN, -LDBL_TRUE_MIN,
			LDBL_MIN, -LDBL_MIN,
		};
		static const long double yvals[] = {
			0.0L, -0.0L, 1.0L, -1.0L, 2.0L, -2.0L,
			PI4, NPI4, 0.5L, -0.5L,
			1e-30L, -1e-30L, 1e30L, -1e30L,
			LDBL_TRUE_MIN, -LDBL_TRUE_MIN,
		};
		static const int iys[] = { 0, 1, -1, 42, -42 };
		std::size_t a, b, c;
		const std::size_t nx = sizeof(xvals) / sizeof(xvals[0]);
		const std::size_t ny = sizeof(yvals) / sizeof(yvals[0]);
		const std::size_t ni = sizeof(iys) / sizeof(iys[0]);

		for (a = 0; a < nx; a++)
			for (b = 0; b < ny; b++) {
				check_kernel_cosl(xvals[a], yvals[b], "domain");
				for (c = 0; c < ni; c++)
					check_kernel_sinl(xvals[a], yvals[b],
					    iys[c], "domain");
			}
	}

	/*
	 * iy==0 vs iy!=0: same (x,y) with both branches so a flipped
	 * comparison is observable.
	 */
	{
		static const long double pairs[][2] = {
			{ 0.3L, 0.7L },
			{ -0.3L, -0.7L },
			{ PI4 * 0.5L, 1.0L },
			{ NPI4 * 0.5L, -1.0L },
			{ 1e-20L, 1e-10L },
			{ -1e-20L, -1e-10L },
			{ 0.785L, 0.785L },
			{ -0.785L, -0.785L },
		};
		std::size_t n;

		for (n = 0; n < sizeof(pairs) / sizeof(pairs[0]); n++) {
			check_kernel_sinl(pairs[n][0], pairs[n][1], 0, "iy0");
			check_kernel_sinl(pairs[n][0], pairs[n][1], 1, "iy1");
			check_kernel_sinl(pairs[n][0], pairs[n][1], -1, "iym1");
		}
	}

	/*
	 * y terms in the polynomials: x near zero with non-zero y, and
	 * x*y products that exercise the subtraction paths.
	 */
	{
		static const long double xs[] = {
			0.0L, -0.0L, 1e-4000L, -1e-4000L,
			0.001L, -0.001L, 0.01L, -0.01L,
		};
		static const long double ys[] = {
			0.0L, -0.0L, 1.0L, -1.0L, PI4, NPI4,
			1e-20L, -1e-20L, 100.0L, -100.0L,
		};
		std::size_t a, b;

		for (a = 0; a < sizeof(xs) / sizeof(xs[0]); a++)
			for (b = 0; b < sizeof(ys) / sizeof(ys[0]); b++) {
				check_kernel_cosl(xs[a], ys[b], "poly");
				check_kernel_sinl(xs[a], ys[b], 0, "poly");
				check_kernel_sinl(xs[a], ys[b], 1, "poly");
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

/*
 * Random long double biased toward the kernel domain [-pi/4, pi/4] but also
 * sampling zeroes, subnormals, and out-of-domain magnitudes.
 */
static long double
rng_domain_x(void)
{
	std::uint64_t r = rng_next();
	unsigned kind = (unsigned)(r % 100u);
	long double s;

	if (kind < 5)
		return (r & 1) ? -0.0L : 0.0L;
	if (kind < 10) {
		std::uint64_t m = rng_next() | ((std::uint64_t)1 << 63);
		return mkld((std::uint16_t)((r & 1) ? 0x8000u : 0x0000u),
		    m & 0x7fffffffffffffffull);
	}
	if (kind < 15)
		return (r & 1) ? NPI4 : PI4;

	s = (long double)(rng_next() % 1000000u) / 1000000.0L;
	if (r & 1)
		s = -s;
	return s * PI4 * (1.0L + (long double)(rng_next() % 100u) / 1e15L);
}

static long double
rng_y(void)
{
	std::uint64_t r = rng_next();
	unsigned kind = (unsigned)(r % 100u);

	if (kind < 8)
		return (r & 1) ? -0.0L : 0.0L;
	if (kind < 16)
		return (r & 1) ? -1.0L : 1.0L;
	if (kind < 24)
		return (r & 1) ? -PI4 : PI4;

	return rng_domain_x() * (long double)(2 + (int)(rng_next() % 7u));
}

static const unsigned long long ITERS = 250000ull;

static void
random_sweep(void)
{
	unsigned long long i;

	rng_state = 0x243f6a8885a308d3ull;
	for (i = 0; i < ITERS; i++) {
		long double x = rng_domain_x();
		long double y = rng_y();
		int iy = (int)((i & 1) ? 0 : (1 + (rng_next() % 255u)));

		if ((i % 97) == 0)
			iy = 0;
		if ((i % 89) == 0)
			iy = -1;
		if ((i % 83) == 0)
			iy = 1;
		if ((i % 79) == 0)
			iy = (int)rng_next();

		check_kernel_sinl(x, y, iy, "random");
		check_kernel_cosl(x, y, "random");
	}
}

/* ------------------------------------------------------------------ */

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

	std::printf("pbsd batch b0082 differential test\n");
	std::printf("LDBL_MANT_DIG=%d, comparing %zu significant bytes of "
	    "long double\n\n", (int)LDBL_MANT_DIG, LD_SIG);

	edge_cases();
	random_sweep();

	std::printf("\n  %-18s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  ------------------------------------------------------\n");
	row(st_invtrig);
	row(st_kernel_sinl);
	row(st_kernel_cosl);

	fails = st_invtrig.fails + st_kernel_sinl.fails + st_kernel_cosl.fails;
	std::printf("\n%s: %llu total failures\n",
	    fails == 0 ? "PASS" : "FAIL", fails);

	return (fails == 0 ? 0 : 1);
}
