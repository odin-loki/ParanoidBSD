/*
 * Differential test harness for PBSD batch b0095.
 *
 * Every exported function of port.cppm is compared against the unmodified C
 * reference in oracle.c.
 *
 * None of the functions in this batch writes through a caller supplied
 * pointer, and none of them is a stateful iterator: all five return a
 * long double by value.  The comparison is therefore made on the full 80 bit
 * object representation of the result, and it is made through a pair of
 * guard-filled buffers so that any byte the copy did not write is compared
 * too.  Bit equality is strictly stronger than == (it separates +0 from -0
 * and distinguishes NaN payloads), which is what a mutation test needs.
 */

#include <cfloat>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.ld80.b0095;

namespace port = pbsd::lib_msun_ld80::b0095;

extern "C" {
long double ref_powl(long double x, long double y);
long double ref_logl(long double x);
long double ref_log1pl(long double x);
long double ref_log10l(long double x);
long double ref_log2l(long double x);
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

/*
 * Copy each result into its own buffer that has been pre-filled with the
 * guard byte, then compare the whole buffer, not just the copied window.
 */
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
static Stat st_logl = { "logl", 0, 0, 0 };
static Stat st_log1pl = { "log1pl", 0, 0, 0 };
static Stat st_log10l = { "log10l", 0, 0, 0 };
static Stat st_log2l = { "log2l", 0, 0, 0 };

static const int MAX_SHOW = 8;

static void
report1(Stat &s, const char *tag, long double x, long double got,
    long double want)
{
	char bx[32], bg[32], bw[32];

	s.fails++;
	if (s.shown++ >= MAX_SHOW)
		return;
	ldstr(bx, x);
	ldstr(bg, got);
	ldstr(bw, want);
	std::printf("  FAIL %-7s [%s] x=%s port=%s ref=%s\n", s.name, tag,
	    bx, bg, bw);
}

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

/* ---------------------------------------------------------- check calls -- */

static void
chk_logl(long double x, const char *tag)
{
	long double a, b;

	st_logl.cases++;
	a = port::logl(x);
	b = ref_logl(x);
	if (!same_result(a, b))
		report1(st_logl, tag, x, a, b);
}

static void
chk_log1pl(long double x, const char *tag)
{
	long double a, b;

	st_log1pl.cases++;
	a = port::log1pl(x);
	b = ref_log1pl(x);
	if (!same_result(a, b))
		report1(st_log1pl, tag, x, a, b);
}

static void
chk_log10l(long double x, const char *tag)
{
	long double a, b;

	st_log10l.cases++;
	a = port::log10l(x);
	b = ref_log10l(x);
	if (!same_result(a, b))
		report1(st_log10l, tag, x, a, b);
}

static void
chk_log2l(long double x, const char *tag)
{
	long double a, b;

	st_log2l.cases++;
	a = port::log2l(x);
	b = ref_log2l(x);
	if (!same_result(a, b))
		report1(st_log2l, tag, x, a, b);
}

static void
chk_logs(long double x, const char *tag)
{
	chk_logl(x, tag);
	chk_log1pl(x, tag);
	chk_log10l(x, tag);
	chk_log2l(x, tag);
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

/*
 * Hand-picked 80 bit encodings.  This deliberately includes the encodings
 * that only exist on x87 -- pseudo-denormals, unnormals, pseudo-infinities
 * and pseudo-NaNs -- because s_logl.c has explicit branches for them.
 */
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

/*
 * The antilog table of e_powl.c, copied so that the harness can drive the
 * "find significand in antilog table" ladder onto and just off every one of
 * its comparison points.  A <= that became a < has to change the chosen
 * index for at least one of these.
 */
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
edge_logs(void)
{
	int i, j, s;

	/* Every special encoding. */
	for (i = 0; i < nspecials; i++)
		chk_logs(specials[i], "special");

	/*
	 * Drive i = (ix + 2**55) >> 56 onto both sides of every one of its
	 * 129 boundaries, at several exponents (including k = 0, k = -1 --
	 * the exact-cancellation case -- and the extremes) and with the
	 * subnormal rescaling path.
	 */
	static const std::uint16_t exps[] = {
		0x0001, 0x0002, 0x1000, 0x3ffd, 0x3ffe, 0x3fff, 0x4000,
		0x407e, 0x407f, 0x4080, 0x6000, 0x7ffd, 0x7ffe,
	};

	for (i = 0; i <= 128; i++) {
		std::uint64_t base, cand[3];
		int c;

		base = (i == 0) ? 0 : ((std::uint64_t)i << 56) -
		    (1ULL << 55);
		cand[0] = base;
		cand[1] = (base == 0) ? 0 : base - 1;
		cand[2] = base + 1;
		for (c = 0; c < 3; c++) {
			std::uint64_t ix = cand[c] & 0x7fffffffffffffffULL;

			for (j = 0; j < (int)(sizeof(exps) / sizeof(exps[0]));
			    j++) {
				for (s = 0; s < 2; s++) {
					std::uint16_t e;

					e = (std::uint16_t)(exps[j] |
					    (s ? 0x8000 : 0));
					chk_logs(mkld(e,
					    ix | 0x8000000000000000ULL),
					    "iboundary");
				}
			}
			/* Same significand as a subnormal / pseudo-denormal. */
			chk_logs(mkld(0x0000,
			    ix | 0x8000000000000000ULL), "sub-i");
			chk_logs(mkld(0x0000, ix), "sub-i2");
			chk_logs(mkld(0x8000, ix), "sub-i3");
			/* Unnormal: same significand with the integer bit off. */
			chk_logs(mkld(0x3fff, ix), "unnormal");
			chk_logs(mkld(0x4000, ix), "unnormal");
		}
	}

	/* log1pl cares about the neighbourhood of -1 and of 0 specifically. */
	{
		long double v;
		int n;

		v = 1.0L;
		for (n = 0; n < 200; n++) {
			chk_log1pl(v, "near0+");
			chk_log1pl(-v, "near0-");
			chk_logs(v, "pow2ish");
			v = std::nextafterl(v, 0.0L);
			v = v / 2;
		}
		v = -1.0L;
		for (n = 0; n < 64; n++) {
			chk_log1pl(v, "near-1");
			v = std::nextafterl(v, 0.0L);
		}
		v = -1.0L;
		for (n = 0; n < 64; n++) {
			chk_log1pl(v, "below-1");
			v = std::nextafterl(v, -2.0L);
		}
		v = 1.0L;
		for (n = 0; n < 64; n++) {
			chk_log1pl(v, "near1");
			chk_logs(v, "near1");
			v = std::nextafterl(v, 0.0L);
		}
		v = 1.0L;
		for (n = 0; n < 64; n++) {
			chk_log1pl(v, "above1");
			chk_logs(v, "above1");
			v = std::nextafterl(v, 2.0L);
		}
	}

	/* Exact powers of two over the whole exponent range. */
	for (i = 1; i <= 0x7ffe; i += 7) {
		chk_logs(mkld((std::uint16_t)i, 0x8000000000000000ULL),
		    "pow2");
		chk_logs(mkld((std::uint16_t)(i | 0x8000),
		    0x8000000000000000ULL), "npow2");
	}
}

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
	/* Values that steer the special-case ladder of powl. */
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

	/*
	 * Pairs chosen to land on specific interior branches: powil's
	 * overflow/underflow/denormal-reciprocal arms, powl's own MEXP and
	 * MNEXP tests, and the odd/even fix-up for negative x.
	 */
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

	/*
	 * Land frexpl(x) exactly on, just below and just above every entry of
	 * the antilog table A[], at several binary exponents.
	 */
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

	rng_seed(0x5eed0095ULL);
	for (n = 0; n < ITERS; n++)
		chk_logl(rnd_ld(), "random");

	rng_seed(0x5eed0096ULL);
	for (n = 0; n < ITERS; n++)
		chk_log1pl(rnd_ld(), "random");

	rng_seed(0x5eed0097ULL);
	for (n = 0; n < ITERS; n++)
		chk_log10l(rnd_ld(), "random");

	rng_seed(0x5eed0098ULL);
	for (n = 0; n < ITERS; n++)
		chk_log2l(rnd_ld(), "random");

	rng_seed(0x5eed0099ULL);
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
	long long total_fail;

	build_specials();

	std::printf("PBSD b0095 differential test (msun/ld80: e_powl.c, "
	    "s_logl.c)\n");

	edge_logs();
	edge_pow();
	sweep();

	std::printf("\n  %-10s %12s %12s  %s\n", "function", "cases",
	    "failures", "status");
	std::printf("  ---------- ------------ ------------  ------\n");
	row(st_powl);
	row(st_logl);
	row(st_log1pl);
	row(st_log10l);
	row(st_log2l);

	total_fail = st_powl.fails + st_logl.fails + st_log1pl.fails +
	    st_log10l.fails + st_log2l.fails;
	std::printf("\n  total failures: %lld\n", total_fail);

	return (total_fail == 0 ? 0 : 1);
}
