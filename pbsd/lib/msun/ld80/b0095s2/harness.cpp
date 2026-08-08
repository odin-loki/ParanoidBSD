/*
 * Differential test harness for PBSD batch b0095s2.
 *
 * Every exported function of port.cppm is compared against the unmodified C
 * reference in oracle.c.
 *
 * None of the functions in this batch writes through a caller supplied
 * pointer, and none of them is a stateful iterator: all four return a
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

import pbsd.lib.msun.ld80.b0095s2;

namespace port = pbsd::lib_msun_ld80::b0095s2;

extern "C" {
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

	rng_seed(0x5eed9502ULL);
	for (n = 0; n < ITERS; n++)
		chk_logl(rnd_ld(), "random");

	rng_seed(0x5eed0953ULL);
	for (n = 0; n < ITERS; n++)
		chk_log1pl(rnd_ld(), "random");

	rng_seed(0x5eed0954ULL);
	for (n = 0; n < ITERS; n++)
		chk_log10l(rnd_ld(), "random");

	rng_seed(0x5eed0955ULL);
	for (n = 0; n < ITERS; n++)
		chk_log2l(rnd_ld(), "random");

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

	std::printf("PBSD b0095s2 differential test (msun/ld80: s_logl.c)\n");

	edge_logs();
	sweep();

	std::printf("\n  %-10s %12s %12s  %s\n", "function", "cases",
	    "failures", "status");
	std::printf("  ---------- ------------ ------------  ------\n");
	row(st_logl);
	row(st_log1pl);
	row(st_log10l);
	row(st_log2l);

	total_fail = st_logl.fails + st_log1pl.fails +
	    st_log10l.fails + st_log2l.fails;
	std::printf("\n  total failures: %lld\n", total_fail);

	return (total_fail == 0 ? 0 : 1);
}
