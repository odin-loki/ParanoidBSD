/*
 * Differential test harness for PBSD batch b0095s1.
 *
 * powl from port.cppm is compared against ref_powl in oracle.c.
 *
 * The function returns a long double by value.  The comparison is made on
 * the full 80 bit object representation through guard-filled buffers so that
 * any byte the copy did not write is compared too.  Bit equality is strictly
 * stronger than == (it separates +0 from -0 and distinguishes NaN payloads).
 */

#include <cfloat>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.ld80.b0095s1;

namespace port = pbsd::lib_msun_ld80::b0095s1;

extern "C" {
long double ref_powl(long double x, long double y);
}

/* Bytes of a long double that actually hold the value on ld80 targets. */
static const std::size_t LD_SIG = 10;
static const std::size_t GUARD_BUF = 32;
static const unsigned char GUARD = 0x7f;

/* ---------------------------------------------------------------- bits -- */

static long double
mkld(std::uint16_t expsign, std::uint64_t man)
{
	unsigned char b[sizeof(long double)];
	long double v;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &man, 8);
	std::memcpy(b + 8, &expsign, 2);
	std::memcpy(&v, b, sizeof(v));
	return (v);
}

static std::uint16_t
ld_expsign(long double v)
{
	std::uint16_t es;

	std::memcpy(&es, reinterpret_cast<const unsigned char *>(&v) + 8, 2);
	return (es);
}

static std::uint64_t
ld_man(long double v)
{
	std::uint64_t m;

	std::memcpy(&m, &v, 8);
	return (m);
}

static void
ldstr(char *out, long double v)
{
	std::snprintf(out, 32, "%04x:%016llx", (unsigned)ld_expsign(v),
	    (unsigned long long)ld_man(v));
}

static bool
same_result(long double a, long double b)
{
	unsigned char ba[GUARD_BUF], bb[GUARD_BUF];

	std::memset(ba, GUARD, sizeof(ba));
	std::memset(bb, GUARD, sizeof(bb));
	std::memcpy(ba, &a, LD_SIG);
	std::memcpy(bb, &b, LD_SIG);
	return (std::memcmp(ba, bb, GUARD_BUF) == 0);
}

/* --------------------------------------------------------------- stats -- */

struct Stat {
	const char *name;
	long long cases;
	long long fails;
	int shown;
};

static Stat st_powl = { "powl", 0, 0, 0 };

static const int MAX_SHOW = 8;

static void
report2(Stat &s, const char *tag, long double x, long double y,
    long double got, long double want)
{
	char bx[32], by[32], bg[32], bw[32];

	s.fails++;
	if (s.shown++ >= MAX_SHOW)
		return;
	ldstr(bx, x);
	ldstr(by, y);
	ldstr(bg, got);
	ldstr(bw, want);
	std::printf("  FAIL %-7s [%s] x=%s y=%s port=%s ref=%s\n", s.name,
	    tag, bx, by, bg, bw);
}

static void
chk_powl(long double x, long double y, const char *tag)
{
	long double a, b;

	st_powl.cases++;
	a = port::powl(x, y);
	b = ref_powl(x, y);
	if (!same_result(a, b))
		report2(st_powl, tag, x, y, a, b);
}

/* ----------------------------------------------------------------- rng -- */

static std::uint64_t rng_state;

static void
rng_seed(std::uint64_t s)
{
	rng_state = s;
}

static std::uint64_t
rnd64(void)
{
	std::uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static unsigned
rndn(unsigned n)
{
	return ((unsigned)(rnd64() % n));
}

/* ------------------------------------------------------- special values -- */

static long double specials[128];
static int nspecials;

static void
add_special(long double v)
{
	if (nspecials < (int)(sizeof(specials) / sizeof(specials[0])))
		specials[nspecials++] = v;
}

static void
build_specials(void)
{
	add_special(mkld(0x0000, 0x0000000000000000ULL));	/* +0 */
	add_special(mkld(0x8000, 0x0000000000000000ULL));	/* -0 */
	add_special(mkld(0x0000, 0x0000000000000001ULL));	/* tiny sub */
	add_special(mkld(0x8000, 0x0000000000000001ULL));
	add_special(mkld(0x0000, 0x4000000000000000ULL));
	add_special(mkld(0x0000, 0x7fffffffffffffffULL));	/* max sub */
	add_special(mkld(0x8000, 0x7fffffffffffffffULL));
	add_special(mkld(0x0000, 0x8000000000000000ULL));	/* pseudo-den */
	add_special(mkld(0x0000, 0xffffffffffffffffULL));	/* pseudo-den */
	add_special(mkld(0x8000, 0x8000000000000000ULL));
	add_special(mkld(0x0001, 0x8000000000000000ULL));	/* LDBL_MIN */
	add_special(mkld(0x8001, 0x8000000000000000ULL));
	add_special(mkld(0x0001, 0x0000000000000001ULL));	/* unnormal */
	add_special(mkld(0x0001, 0x0000000000000000ULL));	/* unnormal */
	add_special(mkld(0x0002, 0x8000000000000001ULL));
	add_special(mkld(0x3ffd, 0x8000000000000000ULL));	/* 0.25 */
	add_special(mkld(0x3ffe, 0x8000000000000000ULL));	/* 0.5 */
	add_special(mkld(0x3ffe, 0xffffffffffffffffULL));	/* 1-ulp */
	add_special(mkld(0x3fff, 0x8000000000000000ULL));	/* 1 */
	add_special(mkld(0x3fff, 0x8000000000000001ULL));	/* 1+ulp */
	add_special(mkld(0xbfff, 0x8000000000000000ULL));	/* -1 */
	add_special(mkld(0xbfff, 0x8000000000000001ULL));	/* -1-ulp */
	add_special(mkld(0xbffe, 0xffffffffffffffffULL));	/* -(1-ulp) */
	add_special(mkld(0x4000, 0x8000000000000000ULL));	/* 2 */
	add_special(mkld(0xc000, 0x8000000000000000ULL));	/* -2 */
	add_special(mkld(0x4000, 0x0000000000000000ULL));	/* unnormal */
	add_special(mkld(0x4000, 0x4000000000000000ULL));	/* unnormal */
	add_special(mkld(0x407e, 0x8000000000000000ULL));	/* 2**127 */
	add_special(mkld(0x407f, 0x8000000000000000ULL));	/* 2**128 */
	add_special(mkld(0x407f, 0xffffffffffffffffULL));
	add_special(mkld(0x4080, 0x8000000000000000ULL));	/* 2**129 */
	add_special(mkld(0xc07f, 0x8000000000000000ULL));	/* -2**128 */
	add_special(mkld(0x7ffd, 0xffffffffffffffffULL));
	add_special(mkld(0x7ffe, 0xffffffffffffffffULL));	/* LDBL_MAX */
	add_special(mkld(0xfffe, 0xffffffffffffffffULL));	/* -LDBL_MAX */
	add_special(mkld(0x7ffe, 0x8000000000000000ULL));
	add_special(mkld(0x7fff, 0x8000000000000000ULL));	/* +Inf */
	add_special(mkld(0xffff, 0x8000000000000000ULL));	/* -Inf */
	add_special(mkld(0x7fff, 0xc000000000000000ULL));	/* +qNaN */
	add_special(mkld(0xffff, 0xc000000000000000ULL));	/* -qNaN */
	add_special(mkld(0x7fff, 0xa000000000000000ULL));	/* sNaN-ish */
	add_special(mkld(0x7fff, 0x8000000000000001ULL));	/* sNaN */
	add_special(mkld(0x7fff, 0x0000000000000000ULL));	/* pseudo-Inf */
	add_special(mkld(0x7fff, 0x4000000000000000ULL));	/* pseudo-NaN */
	add_special(mkld(0xffff, 0x4000000000000000ULL));	/* -pseudo-NaN */
	add_special(mkld(0x3fff, 0xc000000000000000ULL));	/* 1.5 */
	add_special(mkld(0xbfff, 0xc000000000000000ULL));	/* -1.5 */
	add_special(mkld(0x4000, 0xa000000000000000ULL));	/* 2.5 */
	add_special(mkld(0xc000, 0xa000000000000000ULL));	/* -2.5 */
	add_special(mkld(0x3fbe, 0xffffffffffffffffULL));	/* just < 2**-64 */
	add_special(mkld(0x3fbe, 0x8000000000000000ULL));	/* 2**-65 */
	add_special(mkld(0xbfbe, 0xffffffffffffffffULL));
	add_special(mkld(0xbfbe, 0x8000000000000000ULL));
	add_special(mkld(0x3fbf, 0x8000000000000000ULL));	/* 2**-64 */
	add_special(mkld(0xbfbf, 0x8000000000000000ULL));	/* -2**-64 */
	add_special(mkld(0x3fbf, 0x8000000000000001ULL));
	add_special(mkld(0x3fc0, 0x8000000000000000ULL));	/* 2**-63 */
	add_special(3.0L);
	add_special(-3.0L);
	add_special(10.0L);
	add_special(-10.0L);
	add_special(0.1L);
	add_special(-0.1L);
	add_special(1000.0L);
	add_special(-1000.0L);
	add_special(32767.0L);
	add_special(32768.0L);
	add_special(-32768.0L);
	add_special(1e30L);
	add_special(-1e30L);
	add_special(1e-30L);
	add_special(1e4000L);
	add_special(1e-4000L);
	add_special(-1e-4000L);
}

/* ------------------------------------------------ the A[] table of powl -- */

static const long double Atab[33] = {
 1.0000000000000000000000E0L,
 9.7857206208770013448287E-1L,
 9.5760328069857364691013E-1L,
 9.3708381705514995065011E-1L,
 9.1700404320467123175367E-1L,
 8.9735453750155359320742E-1L,
 8.7812608018664974155474E-1L,
 8.5930964906123895780165E-1L,
 8.4089641525371454301892E-1L,
 8.2287773907698242225554E-1L,
 8.0524516597462715409607E-1L,
 7.8799042255394324325455E-1L,
 7.7110541270397041179298E-1L,
 7.5458221379671136985669E-1L,
 7.3841307296974965571198E-1L,
 7.2259040348852331001267E-1L,
 7.0710678118654752438189E-1L,
 6.9195494098191597746178E-1L,
 6.7712777346844636413344E-1L,
 6.6261832157987064729696E-1L,
 6.4841977732550483296079E-1L,
 6.3452547859586661129850E-1L,
 6.2092890603674202431705E-1L,
 6.0762367999023443907803E-1L,
 5.9460355750136053334378E-1L,
 5.8186242938878875689693E-1L,
 5.6939431737834582684856E-1L,
 5.5719337129794626814472E-1L,
 5.4525386633262882960438E-1L,
 5.3357020033841180906486E-1L,
 5.2213689121370692017331E-1L,
 5.1094857432705833910408E-1L,
 5.0000000000000000000000E-1L,
};

/* ------------------------------------------------------- edge case sets -- */

static void
edge_pow(void)
{
	static long double xs[160];
	static long double ys[160];
	int nx = 0, ny = 0;
	int i, j;

	for (i = 0; i < nspecials; i++) {
		xs[nx++] = specials[i];
		ys[ny++] = specials[i];
	}
	static const long double extra[] = {
		0.0L, -0.0L, 1.0L, -1.0L, 2.0L, -2.0L, 0.5L, -0.5L,
		0.25L, -0.25L, 3.0L, -3.0L, 4.0L, -4.0L, 5.0L, -5.0L,
		2.5L, -2.5L, 1.5L, -1.5L, 0.7L, -0.7L, 1.0000001L,
		0.9999999L, 100.0L, -100.0L, 1000.0L, -1000.0L,
		32766.0L, 32767.0L, 32768.0L, 32769.0L,
		-32766.0L, -32767.0L, -32768.0L, -32769.0L,
		16400.0L, -16400.0L, 16383.0L, -16383.0L,
		10000.0L, -10000.0L, 1e6L, -1e6L, 1e12L, -1e12L,
		3.3L, -3.3L, 0.3L, -0.3L, 3.7L, -3.7L,
	};
	for (i = 0; i < (int)(sizeof(extra) / sizeof(extra[0])); i++) {
		if (nx < (int)(sizeof(xs) / sizeof(xs[0])))
			xs[nx++] = extra[i];
		if (ny < (int)(sizeof(ys) / sizeof(ys[0])))
			ys[ny++] = extra[i];
	}

	for (i = 0; i < nx; i++)
		for (j = 0; j < ny; j++)
			chk_powl(xs[i], ys[j], "cross");

	static const long double pairs[][2] = {
		{ 3.0L, 5.0L }, { -3.0L, 5.0L }, { -3.0L, 6.0L },
		{ 3.0L, -5.0L }, { -3.0L, -5.0L }, { -3.0L, -6.0L },
		{ -1.0L, 5.0L }, { -1.0L, 6.0L }, { -1.0L, -5.0L },
		{ 2.0L, 1.0L }, { 2.0L, 2.0L }, { 2.0L, 3.0L },
		{ 2.0L, 16383.0L }, { 2.0L, 16384.0L }, { 2.0L, -16382.0L },
		{ 2.0L, -16400.0L }, { 2.0L, -16445.0L }, { 2.0L, -16446.0L },
		{ 2.0L, 32767.0L }, { 2.0L, -32767.0L },
		{ 1000.0L, 10000.0L }, { 1000.0L, -10000.0L },
		{ 10.0L, 5000.0L }, { 10.0L, -5000.0L },
		{ 3.7L, 1e6L }, { 3.7L, -1e6L },
		{ 0.3L, 1e6L }, { 0.3L, -1e6L },
		{ -2.5L, 3.0L }, { -2.5L, 4.0L }, { -2.5L, -3.0L },
		{ -2.5L, 0.5L }, { 2.5L, 0.5L },
		{ 1.0000001L, 1e12L }, { 1.0000001L, -1e12L },
		{ 0.9999999L, 1e12L }, { 0.9999999L, -1e12L },
		{ 7.0L, 3.0L }, { 7.0L, 11.0L }, { -7.0L, 11.0L },
		{ 65536.0L, 1024.0L }, { 65536.0L, -1024.0L },
	};
	for (i = 0; i < (int)(sizeof(pairs) / sizeof(pairs[0])); i++)
		chk_powl(pairs[i][0], pairs[i][1], "branch");

	static const long double yy[] = {
		1.5L, -1.5L, 0.75L, -0.75L, 3.25L, -3.25L, 700.5L, -700.5L,
	};
	for (i = 0; i <= 32; i++) {
		long double base[3];
		int k, e, m;

		base[0] = Atab[i];
		base[1] = std::nextafterl(Atab[i], 0.0L);
		base[2] = std::nextafterl(Atab[i], 2.0L);
		for (k = 0; k < 3; k++) {
			for (e = -12; e <= 12; e += 4) {
				long double x = std::ldexpl(base[k], e);

				for (m = 0; m < (int)(sizeof(yy) /
				    sizeof(yy[0])); m++) {
					chk_powl(x, yy[m], "Atable");
					chk_powl(-x, yy[m], "Atable-");
				}
			}
		}
	}

	/* Drive y == 0, x == 1, y == 1, NaN, infinity boundary pairs. */
	chk_powl(2.0L, 0.0L, "y0");
	chk_powl(1.0L, 0.0L, "y0");
	chk_powl(-1.0L, 0.0L, "y0");
	chk_powl(1.0L, 1.0L, "x1");
	chk_powl(1.0L, 2.0L, "x1");
	chk_powl(1.0L, -1.0L, "y1");
	chk_powl(5.0L, 1.0L, "y1");
	chk_powl(-1.0L, INFINITY, "neg1-inf");
	chk_powl(-1.0L, -INFINITY, "neg1-ninf");
	chk_powl(2.0L, LDBL_MAX, "ymax");
	chk_powl(0.5L, LDBL_MAX, "ymax");
	chk_powl(-2.0L, LDBL_MAX, "ymax");
	chk_powl(-0.5L, LDBL_MAX, "ymax");
	chk_powl(2.0L, -LDBL_MAX, "ymin");
	chk_powl(0.5L, -LDBL_MAX, "ymin");
	chk_powl(-2.0L, -LDBL_MAX, "ymin");
	chk_powl(-0.5L, -LDBL_MAX, "ymin");
	chk_powl(LDBL_MAX, 2.0L, "xmax");
	chk_powl(LDBL_MAX, -2.0L, "xmax");
	chk_powl(-LDBL_MAX, 2.0L, "nxmax");
	chk_powl(-LDBL_MAX, -2.0L, "nxmax");
	chk_powl(-LDBL_MAX, 3.0L, "nxmax-odd");
	chk_powl(-LDBL_MAX, 4.0L, "nxmax-even");
	chk_powl(-0.0L, -1.0L, "nzero");
	chk_powl(-0.0L, 1.0L, "nzero");
	chk_powl(-0.0L, 3.0L, "nzero-odd");
	chk_powl(0.0L, -1.0L, "pzero");
	chk_powl(0.0L, 1.0L, "pzero");
	chk_powl(-2.0L, 0.5L, "neg-nonint");
	chk_powl(-2.0L, 2.0L, "neg-int");
	chk_powl(-2.0L, 3.0L, "neg-oddint");
}

/* ------------------------------------------------------ random sweeps -- */

static long double
rnd_ld(void)
{
	std::uint64_t man;
	std::uint16_t es;
	unsigned mode, sign;

	mode = rndn(9);
	man = rnd64();
	sign = (unsigned)(rnd64() & 1);

	switch (mode) {
	case 0:				/* completely random encoding */
		es = (std::uint16_t)(rnd64() & 0xffffu);
		break;
	case 1:				/* normal, full exponent range */
		es = (std::uint16_t)((rnd64() % 0x7fffu) | (sign << 15));
		man |= 0x8000000000000000ULL;
		break;
	case 2:				/* normal, close to 1 */
		es = (std::uint16_t)((0x3ffdu + rndn(4)) | (sign << 15));
		man |= 0x8000000000000000ULL;
		man &= ~((1ULL << rndn(63)) - 1);
		break;
	case 3:				/* normal, moderate magnitude */
		es = (std::uint16_t)((0x3fffu + rndn(129) - 64u) |
		    (sign << 15));
		man |= 0x8000000000000000ULL;
		break;
	case 4:				/* subnormal / pseudo-denormal */
		es = (std::uint16_t)(sign << 15);
		man >>= rndn(64);
		break;
	case 5:				/* unnormal */
		es = (std::uint16_t)((1u + rndn(0x7ffeu)) | (sign << 15));
		man &= ~0x8000000000000000ULL;
		break;
	case 6:				/* one of the special encodings */
		return (specials[rndn((unsigned)nspecials)]);
	case 7: {			/* small rationals, often integral */
		long double v;

		v = (long double)((long long)(rnd64() % 70001ULL) - 35000LL);
		switch (rndn(4)) {
		case 0: break;
		case 1: v += 0.5L; break;
		case 2: v /= 4; break;
		default: v /= (long double)(1u + rndn(7)); break;
		}
		return (v);
	}
	default:			/* tiny, around the 2**-64 cliff */
		es = (std::uint16_t)((0x3fb0u + rndn(24)) | (sign << 15));
		man |= 0x8000000000000000ULL;
		break;
	}
	return (mkld(es, man));
}

static const long ITERS = 250000;

static void
sweep(void)
{
	long n;

	rng_seed(0x5eed95b1ULL);
	for (n = 0; n < ITERS; n++) {
		long double x = rnd_ld();
		long double y = rnd_ld();

		chk_powl(x, y, "random");
	}
}

/* ---------------------------------------------------------------- main -- */

static void
row(const Stat &s)
{
	std::printf("  %-10s %12lld %12lld  %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "ok" : "FAILED");
}

int
main(void)
{
	build_specials();

	std::printf("PBSD b0095s1 differential test (msun/ld80: e_powl.c)\n");

	edge_pow();
	sweep();

	std::printf("\n  %-10s %12s %12s  %s\n", "function", "cases",
	    "failures", "status");
	std::printf("  ---------- ------------ ------------  ------\n");
	row(st_powl);

	std::printf("\n  total failures: %lld\n", st_powl.fails);

	return (st_powl.fails == 0 ? 0 : 1);
}
