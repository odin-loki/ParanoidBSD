/*
 * Differential test harness for PBSD batch b0088.
 *
 * Compares cospil(), sinpil(), tanpil(), and cexpl() against the ref_
 * oracle, bit-for-bit.
 */

#include <cfloat>
#include <climits>
#include <cmath>
#define complex _Complex
#include <complex.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.ld128.b0088;

namespace port = pbsd::lib_msun_ld128::b0088;

extern "C" {
long double ref_cospil(long double);
long double ref_sinpil(long double);
long double ref_tanpil(long double);
long double _Complex ref_cexpl(long double _Complex);
}

static const std::size_t LD_BYTES = sizeof(long double);
static const std::size_t CX_BYTES = sizeof(long double _Complex);
static const unsigned long long RANDOM_ITERS = 200000ull;
static const unsigned MAX_REPORT = 8;

struct stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned reported;
};

static stat st_cospil = { "cospil", 0, 0, 0 };
static stat st_sinpil = { "sinpil", 0, 0, 0 };
static stat st_tanpil = { "tanpil", 0, 0, 0 };
static stat st_cexpl = { "cexpl", 0, 0, 0 };

#define creall __real__
#define cimagl __imag__

static bool
ld_equal(long double a, long double b)
{
	return std::memcmp(&a, &b, LD_BYTES) == 0;
}

static bool
cx_equal(long double _Complex a, long double _Complex b)
{
	return std::memcmp(&a, &b, CX_BYTES) == 0;
}

static void
ldhex(long double x)
{
	const auto *p = reinterpret_cast<const unsigned char *>(&x);

	for (std::size_t i = LD_BYTES; i-- > 0;)
		std::printf("%02x", p[i]);
}

static void
report_ld_fail(stat &s, const char *tag, long double got, long double want)
{
	s.fails++;
	if (s.reported >= MAX_REPORT)
		return;
	s.reported++;
	std::printf("  %s FAIL [%s] port=", s.name, tag);
	ldhex(got);
	std::printf(" ref=");
	ldhex(want);
	std::printf("\n");
}

static void
report_cx_fail(stat &s, const char *tag, long double _Complex got,
    long double _Complex want)
{
	s.fails++;
	if (s.reported >= MAX_REPORT)
		return;
	s.reported++;
	std::printf("  %s FAIL [%s] port=", s.name, tag);
	ldhex(__real__(got));
	std::printf("+");
	ldhex(__imag__(got));
	std::printf("i ref=");
	ldhex(__real__(want));
	std::printf("+");
	ldhex(__imag__(want));
	std::printf("i\n");
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
	report_ld_fail(st_cospil, tag, p, o);
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
	report_ld_fail(st_sinpil, tag, p, o);
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
	report_ld_fail(st_tanpil, tag, p, o);
}

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
	report_cx_fail(st_cexpl, tag, p, o);
}

static long double
mkld(std::uint16_t expsign, std::uint64_t manh, std::uint64_t manl)
{
	unsigned char b[16];
	long double x;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &manl, sizeof(manl));
	std::memcpy(b + 8, &manh, 6);
	std::memcpy(b + 14, &expsign, sizeof(expsign));
	std::memcpy(&x, b, sizeof(x));
	return x;
}

static long double _Complex
mkcx(long double re, long double im)
{
	long double _Complex z = 0;

	__real__ z = re;
	__imag__ z = im;
	return z;
}

static void
edge_cases(void)
{
	static const long double xs[] = {
		0.0L, -0.0L,
		1e-4932L, -1e-4932L,
		0x1p-61L, -0x1p-61L,
		0x1p-60L, -0x1p-60L,
		0x1p-59L, -0x1p-59L,
		0.1L, -0.1L,
		0.25L, -0.25L,
		0.25L - 0x1p-100L, -(0.25L - 0x1p-100L),
		0.25L + 0x1p-100L, -(0.25L + 0x1p-100L),
		0.5L, -0.5L,
		0.5L - 0x1p-100L, -(0.5L - 0x1p-100L),
		0.5L + 0x1p-100L, -(0.5L + 0x1p-100L),
		0.75L, -0.75L,
		0.75L - 0x1p-100L, -(0.75L - 0x1p-100L),
		0.75L + 0x1p-100L, -(0.75L + 0x1p-100L),
		1.0L, -1.0L,
		1.0L - 0x1p-100L, -(1.0L - 0x1p-100L),
		1.5L, -1.5L,
		2.0L, -2.0L,
		2.5L, -2.5L,
		3.0L, -3.0L,
		10.0L, -10.0L,
		0x1p112L, -0x1p112L,
		0x1p112L + 0.1L, -(0x1p112L + 0.1L),
		0x1p112L + 0.5L, -(0x1p112L + 0.5L),
		0x1p112L + 0.75L, -(0x1p112L + 0.75L),
		0x1p112L + 1.0L, -(0x1p112L + 1.0L),
		0x1p113L - 1.0L, -(0x1p113L - 1.0L),
		0x1p113L, -0x1p113L,
		0x1p113L + 1.0L, -(0x1p113L + 1.0L),
		LDBL_TRUE_MIN, -LDBL_TRUE_MIN,
		LDBL_MIN, -LDBL_MIN,
		1.0L / 0.0L, -1.0L / 0.0L,
		0.0L / 0.0L,
	};
	struct ldcase {
		std::uint16_t se;
		std::uint64_t manh;
		std::uint64_t manl;
	};
	static const ldcase ldvec[] = {
		{ 0x0000u, 0x0000000000000000ull, 0x0000000000000001ull },
		{ 0x8000u, 0x0000000000000000ull, 0x0000000000000001ull },
		{ 0x3fffu, 0x8000000000000000ull, 0x0000000000000000ull },
		{ 0xbfffu, 0x8000000000000000ull, 0x0000000000000000ull },
		{ 0x3ffeu, 0x8000000000000000ull, 0x0000000000000000ull },
		{ 0xbffeu, 0x8000000000000000ull, 0x0000000000000000ull },
	};
	std::size_t i, j;

	for (i = 0; i < sizeof(xs) / sizeof(xs[0]); i++) {
		check_cospil(xs[i], "edge");
		check_sinpil(xs[i], "edge");
		check_tanpil(xs[i], "edge");
	}

	for (i = 0; i < sizeof(ldvec) / sizeof(ldvec[0]); i++) {
		long double x = mkld(ldvec[i].se, ldvec[i].manh, ldvec[i].manl);
		check_cospil(x, "bits");
		check_sinpil(x, "bits");
		check_tanpil(x, "bits");
	}

	static const long double cexp_xs[] = {
		0.0L, -0.0L, 1.0L, -1.0L, 0.5L,
		1.13565234062941439494919310779707649e+04L,
		1.13565234062941439494919310779707650e+04L,
		2.27892930024498818830197576893019292e+04L,
		2.27892930024498818830197576893019293e+04L,
		1.0L / 0.0L, -1.0L / 0.0L,
		0.0L / 0.0L,
	};
	static const long double cexp_ys[] = {
		0.0L, -0.0L, 1.0L, -1.0L,
		0x1.921fb54442d1846p+1L, -0x1.921fb54442d1846p+1L,
		1.0L / 0.0L, -1.0L / 0.0L,
		0.0L / 0.0L,
	};
	for (i = 0; i < sizeof(cexp_xs) / sizeof(cexp_xs[0]); i++)
		for (j = 0; j < sizeof(cexp_ys) / sizeof(cexp_ys[0]); j++)
			check_cexpl(mkcx(cexp_xs[i], cexp_ys[j]), "edge");

	check_cexpl(mkcx(1.0L, 1.0L / 0.0L), "cx-inf");
	check_cexpl(mkcx(1.0L / 0.0L, 1.0L), "cx-inf");
	check_cexpl(mkcx(-1.0L / 0.0L, 1.0L / 0.0L), "cx-inf");
	check_cexpl(mkcx(1.0L / 0.0L, 0.0L / 0.0L), "cx-nan");
	check_cexpl(mkcx(0.0L / 0.0L, 1.0L), "cx-nan");
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
	unsigned char b[16];
	std::uint64_t r = rng_next();
	unsigned kind = (unsigned)(r % 100u);

	std::memset(b, 0, sizeof(b));
	if (kind < 6) {
		std::uint16_t expsign = (std::uint16_t)((r & 1) ? 0x8000u : 0x0000u);
		std::memcpy(b + 14, &expsign, 2);
	} else if (kind < 12) {
		std::uint64_t manl = rng_next();
		std::uint16_t expsign = (std::uint16_t)(rng_next() & 0xffffu);
		std::memcpy(b, &manl, 8);
		std::memcpy(b + 14, &expsign, 2);
	} else if (kind < 20) {
		std::uint64_t manh = rng_next() & 0x0000ffffffffffffull;
		std::uint64_t manl = rng_next();
		std::uint16_t expsign = (std::uint16_t)(0x3fffu + (rng_next() % 64u));
		if (r & 1)
			expsign |= 0x8000u;
		std::memcpy(b, &manl, 8);
		std::memcpy(b + 8, &manh, 6);
		std::memcpy(b + 14, &expsign, 2);
	} else {
		long double s = (long double)((rng_next() % 2000001u) - 1000000u);
		s *= 0x1p-100L * (long double)(1 + (rng_next() % 1000u));
		if (kind < 40)
			s *= 0x1p100L;
		if (kind < 55)
			s += (long double)((rng_next() % 7u) - 3u) * 0x1p112L;
		std::memcpy(b, &s, sizeof(s));
	}
	long double x;
	std::memcpy(&x, b, sizeof(x));
	return x;
}

static void
random_sweep(void)
{
	unsigned long long i;

	rng_state = 0x243f6a8885a308d3ull;
	for (i = 0; i < RANDOM_ITERS; i++) {
		long double x = rng_ld();

		check_cospil(x, "random");
		check_sinpil(x, "random");
		check_tanpil(x, "random");
		check_cexpl(mkcx(rng_ld(), rng_ld()), "random");
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

	std::printf("pbsd batch b0088 differential test\n");
	std::printf("LDBL_MANT_DIG=%d, comparing %zu-byte long double / "
	    "%zu-byte complex\n\n", (int)LDBL_MANT_DIG, LD_BYTES, CX_BYTES);

	edge_cases();
	random_sweep();

	std::printf("\n  %-18s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  ------------------------------------------------------\n");
	row(st_cospil);
	row(st_sinpil);
	row(st_tanpil);
	row(st_cexpl);

	fails = st_cospil.fails + st_sinpil.fails + st_tanpil.fails +
	    st_cexpl.fails;
	std::printf("\n%s: %llu total failures\n",
	    fails == 0 ? "PASS" : "FAIL", fails);

	return (fails == 0 ? 0 : 1);
}
