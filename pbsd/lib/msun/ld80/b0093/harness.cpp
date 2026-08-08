/*
 * Differential test harness for PBSD batch b0093.
 *
 * cexpl, cospil, sinpil, tanpil compared bit-for-bit against oracle.c.
 */

#include <climits>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#define complex _Complex
#include <complex.h>

import pbsd.lib.msun.ld80.b0093;

namespace port = pbsd::lib_msun_ld80::b0093;

extern "C" {
long double _Complex ref_cexpl(long double _Complex z);
long double ref_cospil(long double x);
long double ref_sinpil(long double x);
long double ref_tanpil(long double x);
}

#if LDBL_MANT_DIG == 64
static const std::size_t LD_SIG = 10;
#else
static const std::size_t LD_SIG = sizeof(long double);
#endif

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

static bool
ld_equal(long double a, long double b)
{
	ldrep ra = ldbits(a);
	ldrep rb = ldbits(b);

	return (std::memcmp(ra.b, rb.b, LD_SIG) == 0);
}

static void
ldhex(const ldrep &r)
{
	std::size_t i;

	for (i = LD_SIG; i-- > 0;)
		std::printf("%02x", r.b[i]);
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

static long double _Complex
mkcx(std::uint16_t re_se, std::uint64_t re_m,
    std::uint16_t im_se, std::uint64_t im_m)
{
	return CMPLXL(mkld(re_se, re_m), mkld(im_se, im_m));
}

struct stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned reported;
};

static stat st_cexpl = { "cexpl", 0, 0, 0 };
static stat st_cospil = { "cospil", 0, 0, 0 };
static stat st_sinpil = { "sinpil", 0, 0, 0 };
static stat st_tanpil = { "tanpil", 0, 0, 0 };

static const unsigned MAX_REPORT = 8;

static void
check_cexpl(long double _Complex z, const char *tag)
{
	long double _Complex p, o;
	long double pr, pi, orr, oi;

	st_cexpl.cases++;

	p = port::cexpl(z);
	o = ref_cexpl(z);

	pr = creall(p);
	pi = cimagl(p);
	orr = creall(o);
	oi = cimagl(o);

	if (ld_equal(pr, orr) && ld_equal(pi, oi))
		return;

	st_cexpl.fails++;
	if (st_cexpl.reported < MAX_REPORT) {
		st_cexpl.reported++;
		std::printf("  cexp FAIL [%s] z_re=", tag);
		ldhex(ldbits(creall(z)));
		std::printf(" z_im=");
		ldhex(ldbits(cimagl(z)));
		std::printf(" port_re=");
		ldhex(ldbits(pr));
		std::printf(" port_im=");
		ldhex(ldbits(pi));
		std::printf(" ref_re=");
		ldhex(ldbits(orr));
		std::printf(" ref_im=");
		ldhex(ldbits(oi));
		std::printf("\n");
	}
}

static void
check_cospil(long double x, const char *tag)
{
	long double p, o;

	st_cospil.cases++;

	p = port::cospil(x);
	o = ref_cospil(x);

	if (ld_equal(p, o))
		return;

	st_cospil.fails++;
	if (st_cospil.reported < MAX_REPORT) {
		st_cospil.reported++;
		std::printf("  cospil FAIL [%s] x=", tag);
		ldhex(ldbits(x));
		std::printf(" port=");
		ldhex(ldbits(p));
		std::printf(" ref=");
		ldhex(ldbits(o));
		std::printf("\n");
	}
}

static void
check_sinpil(long double x, const char *tag)
{
	long double p, o;

	st_sinpil.cases++;

	p = port::sinpil(x);
	o = ref_sinpil(x);

	if (ld_equal(p, o))
		return;

	st_sinpil.fails++;
	if (st_sinpil.reported < MAX_REPORT) {
		st_sinpil.reported++;
		std::printf("  sinpil FAIL [%s] x=", tag);
		ldhex(ldbits(x));
		std::printf(" port=");
		ldhex(ldbits(p));
		std::printf(" ref=");
		ldhex(ldbits(o));
		std::printf("\n");
	}
}

static void
check_tanpil(long double x, const char *tag)
{
	long double p, o;

	st_tanpil.cases++;

	p = port::tanpil(x);
	o = ref_tanpil(x);

	if (ld_equal(p, o))
		return;

	st_tanpil.fails++;
	if (st_tanpil.reported < MAX_REPORT) {
		st_tanpil.reported++;
		std::printf("  tanpil FAIL [%s] x=", tag);
		ldhex(ldbits(x));
		std::printf(" port=");
		ldhex(ldbits(p));
		std::printf(" ref=");
		ldhex(ldbits(o));
		std::printf("\n");
	}
}

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
	{ 0x3ffdu, 0x8000000000000000ull },
	{ 0xbffdu, 0x8000000000000000ull },
	{ 0x3ffdu, 0x8000000000000001ull },
	{ 0x3ffcu, 0x8000000000000000ull },
	{ 0xbffcu, 0x8000000000000000ull },
	{ 0x3ffeu, 0x8000000000000000ull },
	{ 0xbffeu, 0x8000000000000000ull },
	{ 0x3ffeu, 0xc000000000000000ull },
	{ 0xbffeu, 0xc000000000000000ull },
	{ 0x3fffu, 0x8000000000000000ull },
	{ 0xbfffu, 0x8000000000000000ull },
	{ 0x4000u, 0x8000000000000000ull },
	{ 0xc000u, 0x8000000000000000ull },
	{ 0x400cu, 0xb17217f7d1cf79acull },
	{ 0x400cu, 0xb17217f7d1cf79abull },
	{ 0x400cu, 0xb17217f7d1cf79adull },
	{ 0x400du, 0xb1c6a8573de9768cull },
	{ 0x400du, 0xb1c6a8573de9768bULL },
	{ 0x400du, 0xb1c6a8573de9768dULL },
	{ 0x403eu, 0x8000000000000000ull },
	{ 0xc03eu, 0x8000000000000000ull },
	{ 0x403fu, 0x8000000000000000ull },
	{ 0x403eu, 0x8000000000000001ull },
	{ 0x7fffu, 0x8000000000000000ull },
	{ 0xffffu, 0x8000000000000000ull },
	{ 0x7fffu, 0xc000000000000000ull },
	{ 0xffffu, 0xc000000000000000ull },
	{ 0x3fffu, 0x8080808080808080ull },
	{ 0xbfffu, 0xff00ff00ff00ff00ull },
};
static const std::size_t NLDVEC = sizeof(ldvec) / sizeof(ldvec[0]);

static void
edge_cases(void)
{
	std::size_t i, j;

	for (i = 0; i < NLDVEC; i++) {
		long double x = mkld(ldvec[i].se, ldvec[i].m);

		check_cospil(x, "ldvec");
		check_sinpil(x, "ldvec");
		check_tanpil(x, "ldvec");
		check_cexpl(mkcx(ldvec[i].se, ldvec[i].m, 0x0000u, 0ull),
		    "real_only");
		check_cexpl(mkcx(0x0000u, 0ull, ldvec[i].se, ldvec[i].m),
		    "imag_only");
	}

	for (i = 0; i < NLDVEC; i++)
		for (j = 0; j < NLDVEC; j++)
			check_cexpl(mkcx(ldvec[i].se, ldvec[i].m,
			    ldvec[j].se, ldvec[j].m), "cross");

	{
		static const long double xs[] = {
			0.0L, -0.0L,
			0x1p-35L, -0x1p-35L,
			0x1p-34L, -0x1p-34L,
			0.25L, -0.25L,
			0.25L - 0x1p-70L, -(0.25L - 0x1p-70L),
			0.25L + 0x1p-70L, -(0.25L + 0x1p-70L),
			0.5L, -0.5L,
			0.5L - 0x1p-70L, -(0.5L - 0x1p-70L),
			0.5L + 0x1p-70L, -(0.5L + 0x1p-70L),
			0.75L, -0.75L,
			0.75L - 0x1p-70L, -(0.75L - 0x1p-70L),
			0.75L + 0x1p-70L, -(0.75L + 0x1p-70L),
			1.0L, -1.0L,
			1.5L, -1.5L,
			2.0L, -2.0L,
			3.0L, -3.0L,
			0x1p40L, -0x1p40L,
			0x1p63L - 1.0L, -(0x1p63L - 1.0L),
			0x1p63L, -0x1p63L,
			0x1p63L + 1.0L, -(0x1p63L + 1.0L),
			0x1p64L, -0x1p64L,
			1e-4932L, -1e-4932L,
			1e-100L, -1e-100L,
			1e100L, -1e100L,
		};
		std::size_t n;

		for (n = 0; n < sizeof(xs) / sizeof(xs[0]); n++) {
			check_cospil(xs[n], "domain");
			check_sinpil(xs[n], "domain");
			check_tanpil(xs[n], "domain");
		}
	}

	{
		static const ldcase re_ovfl[] = {
			{ 0x400cu, 0xb17217f7d1cf79abull },
			{ 0x400cu, 0xb17217f7d1cf79acull },
			{ 0x400cu, 0xb17217f7d1cf79adull },
			{ 0x400du, 0xb1c6a8573de9768bULL },
			{ 0x400du, 0xb1c6a8573de9768cULL },
			{ 0x400du, 0xb1c6a8573de9768dULL },
			{ 0x7fffu, 0x8000000000000000ull },
			{ 0xffffu, 0x8000000000000000ull },
			{ 0x0000u, 0x0000000000000000ull },
		};
		static const ldcase im_special[] = {
			{ 0x0000u, 0x0000000000000000ull },
			{ 0x3fffu, 0x8000000000000000ull },
			{ 0x7fffu, 0x8000000000000000ull },
			{ 0xffffu, 0x8000000000000000ull },
			{ 0x7fffu, 0xc000000000000000ull },
			{ 0x3ffeu, 0xc90fdaa22168c235ull },
		};
		std::size_t a, b;

		for (a = 0; a < sizeof(re_ovfl) / sizeof(re_ovfl[0]); a++)
			for (b = 0; b < sizeof(im_special) / sizeof(im_special[0]); b++)
				check_cexpl(mkcx(re_ovfl[a].se, re_ovfl[a].m,
				    im_special[b].se, im_special[b].m),
				    "ovfl");
	}

	{
		static const long double ints[] = {
			0x1p63L - 2.0L, 0x1p63L - 1.0L, 0x1p63L,
			0x1p63L + 1.0L, 0x1p63L + 2.0L,
			0x1p64L - 2.0L, 0x1p64L - 1.0L, 0x1p64L,
			0x1p64L + 1.0L,
			-(0x1p63L - 1.0L), -0x1p63L, -(0x1p63L + 1.0L),
		};
		std::size_t n;

		for (n = 0; n < sizeof(ints) / sizeof(ints[0]); n++) {
			check_cospil(ints[n], "parity");
			check_sinpil(ints[n], "parity");
			check_tanpil(ints[n], "parity");
		}
	}

	{
		static const long double tan_edges[] = {
			0.25L, -0.25L,
			0.25L - 0x1p-80L, -(0.25L - 0x1p-80L),
			0.25L + 0x1p-80L, -(0.25L + 0x1p-80L),
			0.5L, -0.5L,
			1.5L, -1.5L,
			2.5L, -2.5L,
		};
		std::size_t n;

		for (n = 0; n < sizeof(tan_edges) / sizeof(tan_edges[0]); n++)
			check_tanpil(tan_edges[n], "tan_edge");
	}
}

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
	unsigned kind = (unsigned)(r % 100u);
	std::uint16_t se;
	std::uint64_t m;

	if (kind < 6)
		return (r & 1) ? -0.0L : 0.0L;
	if (kind < 12) {
		m = rng_next() & 0x7fffffffffffffffull;
		if (m == 0)
			m = 1;
		se = (std::uint16_t)((r & 1) ? 0x8000u : 0x0000u);
		return mkld(se, m);
	}
	if (kind < 18) {
		se = (std::uint16_t)(0x3fffu + (rng_next() % 20u));
		if (r & 1)
			se |= 0x8000u;
		m = (rng_next() & 0x7fffffffffffffffull) | 0x8000000000000000ull;
		return mkld(se, m);
	}
	if (kind < 24) {
		se = (std::uint16_t)(0x400cu + (rng_next() % 4u));
		if (r & 2)
			se |= 0x8000u;
		m = rng_next();
		return mkld(se, m);
	}
	if (kind < 30) {
		se = (std::uint16_t)((r & 1) ? 0x7fffu : 0xffffu);
		m = rng_next() | 0x8000000000000000ull;
		return mkld(se, m);
	}

	se = (std::uint16_t)(0x3ffc + (rng_next() % 0x4040));
	if (r & 1)
		se |= 0x8000u;
	m = rng_next();
	if ((m & 0x8000000000000000ull) == 0)
		m |= 0x8000000000000000ull;
	return mkld(se, m);
}

static const unsigned long long ITERS = 250000ull;

static void
random_sweep(void)
{
	unsigned long long i;

	rng_state = 0x243f6a8885a308d3ull;
	for (i = 0; i < ITERS; i++) {
		long double x = rng_ld();

		check_cospil(x, "random");
		check_sinpil(x, "random");
		check_tanpil(x, "random");
	}

	rng_state = 0x85ebca6b0919c7a9ull;
	for (i = 0; i < ITERS; i++) {
		long double re = rng_ld();
		long double im = rng_ld();
		long double _Complex z;

		if ((i % 17) == 0)
			im = 0.0L;
		if ((i % 19) == 0)
			re = 0.0L;
		if ((i % 23) == 0)
			im = (rng_next() & 1) ? INFINITY : -INFINITY;
		if ((i % 29) == 0)
			re = (rng_next() & 1) ? INFINITY : -INFINITY;

		z = CMPLXL(re, im);
		check_cexpl(z, "random");
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

	std::printf("pbsd batch b0093 differential test\n");
	std::printf("LDBL_MANT_DIG=%d, comparing %zu significant bytes of "
	    "long double\n\n", (int)LDBL_MANT_DIG, LD_SIG);

	edge_cases();
	random_sweep();

	std::printf("\n  %-18s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  ------------------------------------------------------\n");
	row(st_cexpl);
	row(st_cospil);
	row(st_sinpil);
	row(st_tanpil);

	fails = st_cexpl.fails + st_cospil.fails + st_sinpil.fails +
	    st_tanpil.fails;
	std::printf("\n%s: %llu total failures\n",
	    fails == 0 ? "PASS" : "FAIL", fails);

	return (fails == 0 ? 0 : 1);
}
