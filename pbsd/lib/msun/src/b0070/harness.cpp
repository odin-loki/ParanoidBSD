/*
 * Differential test harness for PBSD batch b0070.
 *
 * Every function ported in port.cppm is compared, bit for bit, against the
 * unmodified C reference in oracle.c.  Comparison is done on raw object
 * representations (never ==, which would silently pass NaN results and would
 * conflate +0.0 with -0.0), and every result is staged through a pair of
 * 0x7f-guarded buffers so that any stray write past the nominal result window
 * is caught as well.
 */

#include <cfloat>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.src.b0070;

namespace port = pbsd::lib_msun_src::b0070;

extern "C" {
long double _Complex ref_conjl(long double _Complex z);
float ref_cimagf(float _Complex z);
double ref_cimag(double _Complex z);
long double ref_cargl(long double _Complex z);
}

/* ------------------------------------------------------------------ */
/* raw-representation plumbing                                         */
/* ------------------------------------------------------------------ */

#if LDBL_MANT_DIG == 64
/* x87 80-bit extended: only the first 10 bytes are architectural state, the
 * remaining bytes of the 12/16-byte object are padding and hold garbage. */
static const std::size_t LD_SIG = 10;
#else
static const std::size_t LD_SIG = sizeof(long double);
#endif

static const unsigned char GUARD = 0x7f;
static const std::size_t GUARD_BUF = 64;
static const std::size_t GUARD_OFF = 16;

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

static std::uint32_t
fbits(float f)
{
	std::uint32_t u;

	std::memcpy(&u, &f, sizeof(u));
	return u;
}

static float
fromfbits(std::uint32_t u)
{
	float f;

	std::memcpy(&f, &u, sizeof(f));
	return f;
}

static std::uint64_t
dbits(double d)
{
	std::uint64_t u;

	std::memcpy(&u, &d, sizeof(u));
	return u;
}

static double
fromdbits(std::uint64_t u)
{
	double d;

	std::memcpy(&d, &u, sizeof(d));
	return d;
}

struct ldrep {
	unsigned char b[sizeof(long double)];
};

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

static float _Complex
mkfc(float re, float im)
{
	float _Complex z;

	__real__ z = re;
	__imag__ z = im;
	return z;
}

static long double _Complex
mkldc(long double re, long double im)
{
	long double _Complex z;

	__real__ z = re;
	__imag__ z = im;
	return z;
}

static double _Complex
mkdc(double re, double im)
{
	double _Complex z;

	__real__ z = re;
	__imag__ z = im;
	return z;
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

static stat st_conjl = { "conjl", 0, 0, 0 };
static stat st_cimagf = { "cimagf", 0, 0, 0 };
static stat st_cimag = { "cimag", 0, 0, 0 };
static stat st_cargl = { "cargl", 0, 0, 0 };

static const unsigned MAX_REPORT = 8;

/* ------------------------------------------------------------------ */
/* per-function checkers                                               */
/* ------------------------------------------------------------------ */

static void
check_conjl(long double re, long double im, const char *tag)
{
	ldrep gp[2], go[2];
	port::long_double_complex_t pz;
	long double _Complex oz;

	st_conjl.cases++;

	pz = port::conjl(mkldc(re, im));
	oz = ref_conjl(mkldc(re, im));

	gp[0] = ldbits(__real__ pz);
	gp[1] = ldbits(__imag__ pz);
	go[0] = ldbits(__real__ oz);
	go[1] = ldbits(__imag__ oz);

	if (guarded_equal(gp, go, sizeof(gp)))
		return;

	st_conjl.fails++;
	if (st_conjl.reported < MAX_REPORT) {
		std::size_t i;

		st_conjl.reported++;
		std::printf("  conjl FAIL [%s] port=(", tag);
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", gp[0].b[i]);
		std::printf(",");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", gp[1].b[i]);
		std::printf(") ref=(");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", go[0].b[i]);
		std::printf(",");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", go[1].b[i]);
		std::printf(")\n");
	}
}

static void
check_cimagf(std::uint32_t re, std::uint32_t im, const char *tag)
{
	std::uint32_t p, o;
	port::float_complex_t z;

	st_cimagf.cases++;

	__real__ z = fromfbits(re);
	__imag__ z = fromfbits(im);

	p = fbits(port::cimagf(z));
	o = fbits(ref_cimagf(mkfc(fromfbits(re), fromfbits(im))));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_cimagf.fails++;
	if (st_cimagf.reported < MAX_REPORT) {
		st_cimagf.reported++;
		std::printf("  cimagf FAIL [%s] in=(%08x,%08x) "
		    "port=%08x ref=%08x\n", tag, re, im, p, o);
	}
}

static void
check_cimag(std::uint64_t re, std::uint64_t im, const char *tag)
{
	std::uint64_t p, o;
	port::double_complex_t z;

	st_cimag.cases++;

	__real__ z = fromdbits(re);
	__imag__ z = fromdbits(im);

	p = dbits(port::cimag(z));
	o = dbits(ref_cimag(mkdc(fromdbits(re), fromdbits(im))));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_cimag.fails++;
	if (st_cimag.reported < MAX_REPORT) {
		st_cimag.reported++;
		std::printf("  cimag FAIL [%s] in=(%016llx,%016llx) "
		    "port=%016llx ref=%016llx\n", tag,
		    (unsigned long long)re, (unsigned long long)im,
		    (unsigned long long)p, (unsigned long long)o);
	}
}

static void
check_cargl(long double re, long double im, const char *tag)
{
	ldrep p, o;

	st_cargl.cases++;

	p = ldbits(port::cargl(mkldc(re, im)));
	o = ldbits(ref_cargl(mkldc(re, im)));

	if (guarded_equal(p.b, o.b, sizeof(p.b)))
		return;

	st_cargl.fails++;
	if (st_cargl.reported < MAX_REPORT) {
		std::size_t i;

		st_cargl.reported++;
		std::printf("  cargl FAIL [%s] port=", tag);
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", p.b[i]);
		std::printf(" ref=");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", o.b[i]);
		std::printf("\n");
	}
}

/* ------------------------------------------------------------------ */
/* hand-written edge vectors                                           */
/* ------------------------------------------------------------------ */

static const std::uint32_t fvec[] = {
	0x00000000u,	/* +0 */
	0x80000000u,	/* -0 */
	0x00000001u,	/* smallest +subnormal */
	0x80000001u,	/* smallest -subnormal */
	0x00000080u,	/* subnormal, high bit of low byte */
	0x00008000u,	/* subnormal */
	0x007fffffu,	/* largest +subnormal */
	0x807fffffu,	/* largest -subnormal */
	0x00800000u,	/* smallest +normal */
	0x80800000u,	/* smallest -normal */
	0x00800001u,	/* just above smallest normal */
	0x3f000000u,	/* +0.5 */
	0xbf000000u,	/* -0.5 */
	0x3f800000u,	/* +1 */
	0xbf800000u,	/* -1 */
	0x40000000u,	/* +2 */
	0xc0000000u,	/* -2 */
	0x40490fdbu,	/* +pi */
	0xc0490fdbu,	/* -pi */
	0x4b000000u,	/* 2^23 */
	0xcb000000u,	/* -2^23 */
	0x7f7fffffu,	/* FLT_MAX */
	0xff7fffffu,	/* -FLT_MAX */
	0x7f7ffffeu,	/* just below FLT_MAX */
	0x7f800000u,	/* +inf */
	0xff800000u,	/* -inf */
	0x7f800001u,	/* smallest +sNaN */
	0xff800001u,	/* smallest -sNaN */
	0x7fa00000u,	/* +sNaN */
	0xffa00000u,	/* -sNaN */
	0x7fbfffffu,	/* largest sNaN */
	0x7fc00000u,	/* +qNaN */
	0xffc00000u,	/* -qNaN */
	0x7fc00001u,	/* +qNaN, odd payload */
	0x7fffffffu,	/* +qNaN, all payload bits */
	0xffffffffu,	/* -qNaN, all payload bits */
	0x80808080u,	/* every byte >= 0x80 */
	0x7f7f7f7fu,	/* guard-byte pattern */
	0x00ff00ffu,
	0xff00ff00u,
	0x8000ffffu,
	0x0000ff80u,
	0x33333333u,
	0xb3333333u,
};
static const std::size_t NFVEC = sizeof(fvec) / sizeof(fvec[0]);

static const std::uint64_t dvec[] = {
	0x0000000000000000ull,	/* +0 */
	0x8000000000000000ull,	/* -0 */
	0x0000000000000001ull,	/* smallest +subnormal */
	0x8000000000000001ull,	/* smallest -subnormal */
	0x0000000000000080ull,
	0x0000000080000000ull,
	0x000fffffffffffffull,	/* largest +subnormal */
	0x800fffffffffffffull,	/* largest -subnormal */
	0x0010000000000000ull,	/* smallest +normal */
	0x8010000000000000ull,	/* smallest -normal */
	0x0010000000000001ull,
	0x3fe0000000000000ull,	/* +0.5 */
	0xbfe0000000000000ull,	/* -0.5 */
	0x3ff0000000000000ull,	/* +1 */
	0xbff0000000000000ull,	/* -1 */
	0x4000000000000000ull,	/* +2 */
	0xc000000000000000ull,	/* -2 */
	0x400921fb54442d18ull,	/* +pi */
	0xc00921fb54442d18ull,	/* -pi */
	0x4330000000000000ull,	/* 2^52 */
	0xc330000000000000ull,	/* -2^52 */
	0x7fefffffffffffffull,	/* DBL_MAX */
	0xffefffffffffffffull,	/* -DBL_MAX */
	0x7feffffffffffffeull,
	0x7ff0000000000000ull,	/* +inf */
	0xfff0000000000000ull,	/* -inf */
	0x7ff0000000000001ull,	/* smallest +sNaN */
	0xfff0000000000001ull,	/* smallest -sNaN */
	0x7ff4000000000000ull,	/* +sNaN */
	0xfff4000000000000ull,	/* -sNaN */
	0x7ff7ffffffffffffull,	/* largest sNaN */
	0x7ff8000000000000ull,	/* +qNaN */
	0xfff8000000000000ull,	/* -qNaN */
	0x7ff8000000000001ull,
	0x7fffffffffffffffull,
	0xffffffffffffffffull,
	0x8080808080808080ull,	/* every byte >= 0x80 */
	0x7f7f7f7f7f7f7f7full,	/* guard-byte pattern */
	0x00ff00ff00ff00ffull,
	0xff00ff00ff00ff00ull,
	0x8000ffffffff0000ull,
	0x3333333333333333ull,
	0xb333333333333333ull,
};
static const std::size_t NDVEC = sizeof(dvec) / sizeof(dvec[0]);

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
	{ 0x0001u, 0x8000000000000001ull },
	{ 0x3ffeu, 0x8000000000000000ull },	/* +0.5 */
	{ 0xbffeu, 0x8000000000000000ull },	/* -0.5 */
	{ 0x3fffu, 0x8000000000000000ull },	/* +1 */
	{ 0xbfffu, 0x8000000000000000ull },	/* -1 */
	{ 0x3fffu, 0x8000000000000001ull },	/* nextafter(1, inf) */
	{ 0x3ffeu, 0xffffffffffffffffull },	/* nextafter(1, 0) */
	{ 0x4000u, 0x8000000000000000ull },	/* +2 */
	{ 0xc000u, 0x8000000000000000ull },	/* -2 */
	{ 0x4000u, 0xc000000000000000ull },	/* +3 */
	{ 0xc000u, 0xc000000000000000ull },	/* -3 */
	{ 0x4001u, 0xe000000000000000ull },	/* +7 */
	{ 0x4000u, 0xc90fdaa22168c235ull },	/* +pi */
	{ 0xc000u, 0xc90fdaa22168c235ull },	/* -pi */
	{ 0x403eu, 0x8000000000000000ull },	/* 2^63 */
	{ 0x403fu, 0x8000000000000000ull },	/* 2^64 */
	{ 0x407fu, 0x8000000000000000ull },	/* 2^128 */
	{ 0x7ffeu, 0xffffffffffffffffull },	/* LDBL_MAX */
	{ 0xfffeu, 0xffffffffffffffffull },	/* -LDBL_MAX */
	{ 0x7fffu, 0x8000000000000000ull },	/* +inf */
	{ 0xffffu, 0x8000000000000000ull },	/* -inf */
	{ 0x7fffu, 0xc000000000000000ull },	/* +qNaN */
	{ 0xffffu, 0xc000000000000000ull },	/* -qNaN */
	{ 0x7fffu, 0xa000000000000000ull },	/* +sNaN */
	{ 0x7fffu, 0x8000000000000001ull },	/* smallest +sNaN */
	{ 0x7fffu, 0xffffffffffffffffull },
	{ 0x3fffu, 0x8080808080808080ull },	/* high-bit byte payload */
	{ 0xbfffu, 0xff00ff00ff00ff00ull },
	{ 0x3fffu, 0xfefefefefefefefeull },
};
static const std::size_t NLDVEC = sizeof(ldvec) / sizeof(ldvec[0]);

static void
edge_cases(void)
{
	std::size_t i, j;

	for (i = 0; i < NFVEC; i++)
		for (j = 0; j < NFVEC; j++)
			check_cimagf(fvec[i], fvec[j], "cross");

	for (i = 0; i < NDVEC; i++)
		for (j = 0; j < NDVEC; j++)
			check_cimag(dvec[i], dvec[j], "cross");

	for (i = 0; i < NLDVEC; i++)
		for (j = 0; j < NLDVEC; j++) {
			long double re = mkld(ldvec[i].se, ldvec[i].m);
			long double im = mkld(ldvec[j].se, ldvec[j].m);

			check_conjl(re, im, "cross");
			check_cargl(re, im, "cross");
		}

	/*
	 * conjl negates the imaginary component: walk values where imag is
	 * non-zero and distinct from real so a sign flip is observable.
	 */
	{
		static const long double vals[] = {
			1.0L, -1.0L, 0.5L, -0.5L, 2.0L, -2.0L, 3.0L, -3.0L,
			0.1L, -0.1L, 1e-30L, -1e-30L, 1e30L, -1e30L,
			__builtin_infl(), -__builtin_infl(),
			__builtin_nanl(""), -__builtin_nanl(""),
			LDBL_MIN, -LDBL_MIN, LDBL_MAX, -LDBL_MAX,
			LDBL_TRUE_MIN, -LDBL_TRUE_MIN,
		};
		std::size_t a, b;
		const std::size_t n = sizeof(vals) / sizeof(vals[0]);

		for (a = 0; a < n; a++)
			for (b = 0; b < n; b++) {
				check_conjl(vals[a], vals[b], "quadrant");
				check_cargl(vals[a], vals[b], "quadrant");
			}
	}

	/*
	 * cimag/cimagf read the imaginary lane via a union: make sure real and
	 * imag lanes differ so swapping REALPART/IMAGPART would fail.
	 */
	{
		static const std::uint32_t fpairs[][2] = {
			{ 0x3f800000u, 0x40000000u },	/* 1, 2 */
			{ 0x40000000u, 0x3f800000u },	/* 2, 1 */
			{ 0x00000000u, 0x3f800000u },	/* +0, 1 */
			{ 0x80000000u, 0xbf800000u },	/* -0, -1 */
			{ 0x7f800000u, 0xff800000u },	/* +inf, -inf */
			{ 0x7fc00000u, 0xffc00000u },	/* qNaN pair */
			{ 0x80808080u, 0x7f7f7f7fu },
			{ 0x00000001u, 0x80000001u },
		};
		static const std::uint64_t dpairs[][2] = {
			{ 0x3ff0000000000000ull, 0x4000000000000000ull },
			{ 0x4000000000000000ull, 0x3ff0000000000000ull },
			{ 0x0000000000000000ull, 0x3ff0000000000000ull },
			{ 0x8000000000000000ull, 0xbff0000000000000ull },
			{ 0x7ff0000000000000ull, 0xfff0000000000000ull },
			{ 0x7ff8000000000000ull, 0xfff8000000000000ull },
			{ 0x8080808080808080ull, 0x7f7f7f7f7f7f7f7full },
			{ 0x0000000000000001ull, 0x8000000000000001ull },
		};
		std::size_t k;

		for (k = 0; k < sizeof(fpairs) / sizeof(fpairs[0]); k++)
			check_cimagf(fpairs[k][0], fpairs[k][1], "lane");
		for (k = 0; k < sizeof(dpairs) / sizeof(dpairs[0]); k++)
			check_cimag(dpairs[k][0], dpairs[k][1], "lane");
	}

	/*
	 * cargl is atan2l(imag, real): every quadrant, both axes, signed
	 * zeroes and infinities.
	 */
	{
		static const long double lq[] = {
			0.0L, -0.0L, 1.0L, -1.0L, 2.0L, -2.0L,
			__builtin_infl(), -__builtin_infl(),
			__builtin_nanl(""), -__builtin_nanl(""),
			LDBL_MIN, -LDBL_MIN, LDBL_MAX, -LDBL_MAX,
			LDBL_TRUE_MIN, -LDBL_TRUE_MIN,
			1e-4932L, -1e-4932L,
		};
		std::size_t a, b;
		const std::size_t n = sizeof(lq) / sizeof(lq[0]);

		for (a = 0; a < n; a++)
			for (b = 0; b < n; b++)
				check_cargl(lq[a], lq[b], "axis");
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
 * A valid x87 extended value with the exponent drawn from a window around 1.0
 * (plus a slice of zeroes, subnormals, infinities and NaNs).
 */
static long double
rng_ld(int spread)
{
	std::uint64_t m = rng_next();
	std::uint64_t r = rng_next();
	std::uint16_t sign = (r & 1) ? 0x8000u : 0x0000u;
	unsigned kind = (unsigned)((r >> 1) % 100u);
	std::uint16_t e;

	if (kind < 4)
		e = 0x0000u;
	else if (kind < 7)
		e = 0x7fffu;
	else
		e = (std::uint16_t)(0x3fff +
		    (int)(rng_next() % (std::uint64_t)(2 * spread + 1)) -
		    spread);

	if (e == 0x0000u)
		m &= ~(std::uint64_t)1 << 63;
	else
		m |= (std::uint64_t)1 << 63;

	return mkld((std::uint16_t)(sign | e), m);
}

static const unsigned long long ITERS = 250000ull;

static void
random_sweep(void)
{
	unsigned long long i;

	/* cimagf: unrestricted 32-bit patterns (every float is one). */
	rng_state = 0xd1ce4e5b91234567ull;
	for (i = 0; i < ITERS; i++) {
		std::uint64_t r = rng_next();
		std::uint32_t re = (std::uint32_t)r;
		std::uint32_t im = (std::uint32_t)(r >> 32);

		if ((i & 7) == 0)
			im = (i & 8) ? 0x80000000u : 0x00000000u;
		if ((i & 7) == 1)
			re = (i & 8) ? 0x80000000u : 0x00000000u;

		check_cimagf(re, im, "random");
	}

	/* cimag: unrestricted 64-bit patterns. */
	rng_state = 0x243f6a8885a308d3ull;
	for (i = 0; i < ITERS; i++) {
		std::uint64_t re = rng_next();
		std::uint64_t im = rng_next();

		if ((i & 7) == 0)
			im = (i & 8) ? 0x8000000000000000ull : 0ull;
		if ((i & 7) == 1)
			re = (i & 8) ? 0x8000000000000000ull : 0ull;

		check_cimag(re, im, "random");
	}

	/* conjl / cargl. */
	rng_state = 0x13198a2e03707344ull;
	for (i = 0; i < ITERS; i++) {
		long double re, im;
		unsigned mode = (unsigned)(i % 4u);

		switch (mode) {
		case 0:
			re = rng_ld(6);
			im = rng_ld(6);
			break;
		case 1:
			re = rng_ld(60);
			im = rng_ld(60);
			break;
		case 2:
			re = rng_ld(6);
			im = 0.0L;
			if (rng_next() & 1)
				im = -0.0L;
			break;
		default:
			re = rng_ld(16000);
			im = rng_ld(16000);
			break;
		}

		check_conjl(re, im, "random");
		check_cargl(re, im, "random");
	}
}

/* ------------------------------------------------------------------ */

static void
row(const stat &s)
{
	std::printf("  %-14s %12llu %10llu   %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "PASS" : "FAIL");
}

int
main(void)
{
	unsigned long long fails;

	std::printf("pbsd batch b0070 differential test\n");
	std::printf("LDBL_MANT_DIG=%d, comparing %zu significant bytes of "
	    "long double\n\n", (int)LDBL_MANT_DIG, LD_SIG);

	edge_cases();
	random_sweep();

	std::printf("\n  %-14s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  ----------------------------------------------------\n");
	row(st_conjl);
	row(st_cimagf);
	row(st_cimag);
	row(st_cargl);

	fails = st_conjl.fails + st_cimagf.fails + st_cimag.fails +
	    st_cargl.fails;
	std::printf("\n%s: %llu total failures\n",
	    fails == 0 ? "PASS" : "FAIL", fails);

	return (fails == 0 ? 0 : 1);
}
