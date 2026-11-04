/*
 * PBSD batch b0093 -- differential test of the C++23 port against the
 * unmodified C sources in oracle.c.
 *
 *	cospil(), sinpil(), tanpil()	long double -> long double
 *	cexpl()				long double complex -> long double complex
 *
 * None of these writes through a caller supplied pointer, so the buffer
 * discipline is applied to the values instead: every case gives the port and
 * the oracle its own 0x7f filled buffer, copies the identical input bytes
 * into both, has each side read its argument out of its own buffer and store
 * its result back into it, and then compares the buffers in their entirety,
 * guard bytes included.  Only the ten architectural bytes of an ld80 value
 * are copied, so the 0x7f guards also cover the bytes the results do not
 * occupy (the six padding bytes of a 16 byte long double are indeterminate
 * and are never copied).
 *
 * Comparison is bit exact: -0.0 differs from +0.0 and NaN payloads must
 * match.  In addition the raised floating point exception flags are compared
 * per call, which pins down paths that return the same value by a different
 * route (cospil()'s |x| < 0x1p-34 shortcut returns exactly 1 for the whole
 * reachable domain, so only the flags distinguish it from falling through to
 * __kernel_cospil()).
 */

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cfenv>

import pbsd.lib.msun.ld80.b0093;

namespace P = pbsd::lib_msun_ld80::b0093;

extern "C" {
long double ref_cospil(long double);
long double ref_sinpil(long double);
long double ref_tanpil(long double);
_Complex long double ref_cexpl(_Complex long double);
long double nextafterl(long double, long double);
}

using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

/* ------------------------------------------------------------------ */
/* ld80 bit twiddling						      */
/* ------------------------------------------------------------------ */

static long double
mkld(uint16_t se, uint64_t man)
{
	unsigned char b[16];
	long double r;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &man, 8);
	std::memcpy(b + 8, &se, 2);
	std::memcpy(&r, b, 16);
	return (r);
}

static void
ldbits(long double x, uint16_t *se, uint64_t *man)
{
	unsigned char b[16];

	std::memcpy(b, &x, 16);
	std::memcpy(man, b, 8);
	std::memcpy(se, b + 8, 2);
}

#define	LD_INF		mkld(0x7fff, 0x8000000000000000ULL)
#define	LD_NINF		mkld(0xffff, 0x8000000000000000ULL)

static const unsigned char GUARD = 0x7f;

/* ------------------------------------------------------------------ */
/* statistics							      */
/* ------------------------------------------------------------------ */

struct Fstat {
	const char	*name;
	long long	cases;
	long long	fails;
	long long	valfails;
	long long	flagfails;
	int		shown;
};

static Fstat st_cospil = { "cospil", 0, 0, 0, 0, 0 };
static Fstat st_sinpil = { "sinpil", 0, 0, 0, 0, 0 };
static Fstat st_tanpil = { "tanpil", 0, 0, 0, 0, 0 };
static Fstat st_cexpl  = { "cexpl",  0, 0, 0, 0, 0 };

#define	MAXSHOW	8

static void
show_ld(const char *label, long double v)
{
	uint16_t se;
	uint64_t man;

	ldbits(v, &se, &man);
	std::printf("      %s = %04x:%016llx (%.21Lg)\n", label, se,
	    (unsigned long long)man, v);
}

/* ------------------------------------------------------------------ */
/* one argument, one result					      */
/* ------------------------------------------------------------------ */

static void
check1(Fstat &st, long double (*pf)(long double), long double (*rf)(long double),
    long double x, const char *tag)
{
	alignas(16) unsigned char ba[64], bb[64];
	long double xa, xb, ra, rb;
	int fa, fb, bad;
	unsigned i;

	std::memset(ba, GUARD, sizeof(ba));
	std::memset(bb, GUARD, sizeof(bb));
	std::memcpy(ba, &x, 10);
	std::memcpy(bb, &x, 10);

	std::memcpy(&xa, ba, 16);
	std::memcpy(&xb, bb, 16);

	std::feclearexcept(FE_ALL_EXCEPT);
	ra = pf(xa);
	std::memcpy(ba + 16, &ra, 10);
	fa = std::fetestexcept(FE_ALL_EXCEPT);

	std::feclearexcept(FE_ALL_EXCEPT);
	rb = rf(xb);
	std::memcpy(bb + 16, &rb, 10);
	fb = std::fetestexcept(FE_ALL_EXCEPT);

	st.cases++;
	bad = 0;

	if (std::memcmp(ba, bb, sizeof(ba)) != 0) {
		bad = 1;
		st.valfails++;
	}
	/* The guards must still be intact in both buffers. */
	for (i = 10; i < 16; i++)
		if (ba[i] != GUARD || bb[i] != GUARD)
			bad = 1;
	for (i = 26; i < sizeof(ba); i++)
		if (ba[i] != GUARD || bb[i] != GUARD)
			bad = 1;
	if (fa != fb) {
		bad = 1;
		st.flagfails++;
	}

	if (bad) {
		st.fails++;
		if (st.shown < MAXSHOW) {
			st.shown++;
			std::printf("  %s FAIL [%s]\n", st.name, tag);
			show_ld("x   ", x);
			show_ld("port", ra);
			show_ld("ref ", rb);
			std::printf("      flags port=%#x ref=%#x\n", fa, fb);
		}
	}
}

/* ------------------------------------------------------------------ */
/* complex argument, complex result				      */
/* ------------------------------------------------------------------ */

static void
check_cexpl(long double re, long double im, const char *tag)
{
	alignas(16) unsigned char ba[96], bb[96];
	_Complex long double za, zb, ra, rb;
	long double t;
	int fa, fb, bad;
	unsigned i;

	std::memset(ba, GUARD, sizeof(ba));
	std::memset(bb, GUARD, sizeof(bb));
	std::memcpy(ba, &re, 10);
	std::memcpy(bb, &re, 10);
	std::memcpy(ba + 16, &im, 10);
	std::memcpy(bb + 16, &im, 10);

	std::memcpy(&t, ba, 16);
	__real__ za = t;
	std::memcpy(&t, ba + 16, 16);
	__imag__ za = t;
	std::memcpy(&t, bb, 16);
	__real__ zb = t;
	std::memcpy(&t, bb + 16, 16);
	__imag__ zb = t;

	std::feclearexcept(FE_ALL_EXCEPT);
	ra = P::cexpl(za);
	t = __real__ ra;
	std::memcpy(ba + 32, &t, 10);
	t = __imag__ ra;
	std::memcpy(ba + 48, &t, 10);
	fa = std::fetestexcept(FE_ALL_EXCEPT);

	std::feclearexcept(FE_ALL_EXCEPT);
	rb = ref_cexpl(zb);
	t = __real__ rb;
	std::memcpy(bb + 32, &t, 10);
	t = __imag__ rb;
	std::memcpy(bb + 48, &t, 10);
	fb = std::fetestexcept(FE_ALL_EXCEPT);

	st_cexpl.cases++;
	bad = 0;

	if (std::memcmp(ba, bb, sizeof(ba)) != 0) {
		bad = 1;
		st_cexpl.valfails++;
	}
	for (i = 0; i < sizeof(ba); i++) {
		if (i < 10 || (i >= 16 && i < 26) ||
		    (i >= 32 && i < 42) || (i >= 48 && i < 58))
			continue;	/* value bytes */
		if (ba[i] != GUARD || bb[i] != GUARD)
			bad = 1;
	}
	if (fa != fb) {
		bad = 1;
		st_cexpl.flagfails++;
	}

	if (bad) {
		st_cexpl.fails++;
		if (st_cexpl.shown < MAXSHOW) {
			st_cexpl.shown++;
			std::printf("  cexpl FAIL [%s]\n", tag);
			show_ld("re  ", re);
			show_ld("im  ", im);
			show_ld("p.re", __real__ ra);
			show_ld("r.re", __real__ rb);
			show_ld("p.im", __imag__ ra);
			show_ld("r.im", __imag__ rb);
			std::printf("      flags port=%#x ref=%#x\n", fa, fb);
		}
	}
}

/* ------------------------------------------------------------------ */
/* hand written case list for the real argument functions	      */
/* ------------------------------------------------------------------ */

static long double vals[16384];
static int nvals;

static void
add(long double x)
{
	if (nvals < (int)(sizeof(vals) / sizeof(vals[0])))
		vals[nvals++] = x;
}

static void
add_pm(long double x)
{
	add(x);
	add(-x);
}

/* x, and k ulps to either side of it, with both signs. */
static void
add_neigh(long double x, int k)
{
	long double up = x, dn = x;
	int i;

	add_pm(x);
	for (i = 0; i < k; i++) {
		up = nextafterl(up, LD_INF);
		dn = nextafterl(dn, LD_NINF);
		add_pm(up);
		add_pm(dn);
	}
}

static void
build_vals(void)
{
	static const uint16_t exps[] = {
		0x0000, 0x0001, 0x0002, 0x1000, 0x3f00,
		0x3fda, 0x3fdb, 0x3fdc, 0x3fdd, 0x3fde, 0x3fdf,
		0x3ff0, 0x3ffb, 0x3ffc, 0x3ffd, 0x3ffe, 0x3fff,
		0x4000, 0x4001, 0x4002, 0x4008,
		0x401c, 0x401d, 0x401e, 0x401f, 0x4020,
		0x4025, 0x4026, 0x4027, 0x4028, 0x4029,
		0x403b, 0x403c, 0x403d, 0x403e, 0x403f, 0x4040, 0x4041,
		0x4100, 0x7ffc, 0x7ffd, 0x7ffe, 0x7fff
	};
	static const uint64_t mans[] = {
		0x0000000000000000ULL, 0x0000000000000001ULL,
		0x4000000000000000ULL, 0x7fffffffffffffffULL,
		0x8000000000000000ULL, 0x8000000000000001ULL,
		0x8000000100000000ULL, 0xa000000000000000ULL,
		0xbfffffffffffffffULL, 0xc000000000000000ULL,
		0xc000000000000001ULL, 0xe000000000000000ULL,
		0xfffffffffffffffeULL, 0xffffffffffffffffULL
	};
	unsigned e, m;
	int i;

	/* Zeroes, subnormals, pseudo denormals, smallest normal. */
	add(mkld(0x0000, 0x0000000000000000ULL));
	add(mkld(0x8000, 0x0000000000000000ULL));
	add_pm(mkld(0x0000, 0x0000000000000001ULL));
	add_pm(mkld(0x0000, 0x0000000080000000ULL));
	add_pm(mkld(0x0000, 0x7fffffffffffffffULL));
	add_pm(mkld(0x0000, 0x8000000000000000ULL));
	add_pm(mkld(0x0000, 0xffffffffffffffffULL));
	add_pm(mkld(0x0001, 0x8000000000000000ULL));

	/* The exponent x mantissa grid, both signs. */
	for (e = 0; e < sizeof(exps) / sizeof(exps[0]); e++)
		for (m = 0; m < sizeof(mans) / sizeof(mans[0]); m++) {
			add(mkld(exps[e], mans[m]));
			add(mkld((uint16_t)(exps[e] | 0x8000), mans[m]));
		}

	/* Every branch boundary, with a few ulps to either side. */
	add_neigh(0x1p-16445L, 2);
	add_neigh(0x1p-40L, 2);
	add_neigh(0x1p-35L, 2);
	add_neigh(0x1p-34L, 3);		/* the 0x3fdd cut */
	add_neigh(0x1p-33L, 2);
	add_neigh(0.125L, 2);
	add_neigh(0.25L, 3);		/* the 0x3ffd cut, tanpi's t = 1 */
	add_neigh(0.375L, 2);
	add_neigh(0.5L, 3);		/* the 0x3ffe cut, tanpi pole */
	add_neigh(0.625L, 2);
	add_neigh(0.75L, 3);		/* the lx < 0xc000... cut */
	add_neigh(0.875L, 2);
	add_neigh(1.0L, 3);		/* the 0x3fff cut */
	add_neigh(1.25L, 2);
	add_neigh(1.5L, 3);
	add_neigh(1.75L, 2);
	add_neigh(2.0L, 3);
	add_neigh(2.25L, 2);
	add_neigh(2.5L, 2);
	add_neigh(2.75L, 2);
	add_neigh(3.0L, 2);
	add_neigh(3.5L, 2);
	add_neigh(4.0L, 2);
	add_neigh(0x1p29L, 2);
	add_neigh(0x1p30L, 3);		/* the j0 > 30 cut */
	add_neigh(0x1p31L, 3);		/* the FFLOORL80 j0 < 32 cut */
	add_neigh(0x1p32L, 3);
	add_neigh(0x1p39L, 2);
	add_neigh(0x1p40L, 3);		/* the j0 > 40 cut */
	add_neigh(0x1p41L, 3);
	add_neigh(0x1p61L, 2);
	add_neigh(0x1p62L, 3);		/* the 0x403e cut is just above */
	add_neigh(0x1p63L, 3);
	add_neigh(0x1p64L, 3);		/* the 0x403f cut */
	add_neigh(0x1p65L, 2);
	add_neigh(0x1p100L, 2);
	add_neigh(0x1p16383L, 2);

	/*
	 * Integers and integers plus a fraction at every j0, so that the
	 * argument reduction, the FFLOORL80 masking, the 0x1p40/0x1p30
	 * folding and the parity of the integer part are all driven with
	 * both parities.  For the larger exponents the addition rounds,
	 * which is exactly the interesting case.
	 */
	for (i = 0; i <= 62; i++) {
		long double base = 1.0L, n;
		int j, k;
		static const long double fr[] = {
			0.0L, 0.25L, 0.5L, 0.75L, 0.125L, 0.375L, 0.9375L
		};

		for (j = 0; j < i; j++)
			base *= 2.0L;
		for (k = 0; k < 4; k++) {
			n = k == 0 ? base : k == 1 ? base + 1.0L :
			    k == 2 ? base * 2.0L - 1.0L : base * 2.0L - 2.0L;
			for (j = 0; j < (int)(sizeof(fr) / sizeof(fr[0])); j++)
				add_pm(n + fr[j]);
		}
	}

	/* Infinities, NaNs of several payloads, unnormals. */
	add(LD_INF);
	add(LD_NINF);
	add_pm(mkld(0x7fff, 0xc000000000000000ULL));	/* quiet NaN */
	add_pm(mkld(0x7fff, 0xffffffffffffffffULL));
	add_pm(mkld(0x7fff, 0xc000000000000001ULL));
	add_pm(mkld(0x7fff, 0x8000000000000001ULL));	/* signalling NaN */
	add_pm(mkld(0x7fff, 0xbfffffffffffffffULL));	/* pseudo NaN */
	add_pm(mkld(0x7fff, 0x0000000000000000ULL));	/* pseudo infinity */
	add_pm(mkld(0x4000, 0x4000000000000000ULL));	/* unnormal */
	add_pm(mkld(0x403e, 0x0000000000000001ULL));
	add_pm(mkld(0x403d, 0x7fffffffffffffffULL));
}

/* ------------------------------------------------------------------ */
/* fixed seed randomised sweep					      */
/* ------------------------------------------------------------------ */

static uint64_t rng = 0x0093c0ffeeULL;

static uint64_t
nextrand(void)
{
	uint64_t z;

	rng += 0x9e3779b97f4a7c15ULL;
	z = rng;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static const uint16_t rexps[] = {
	0x0000, 0x0001, 0x2000, 0x3f00, 0x3fd0,
	0x3fdb, 0x3fdc, 0x3fdd, 0x3fde,
	0x3ff8, 0x3ffb, 0x3ffc, 0x3ffd, 0x3ffe, 0x3fff,
	0x4000, 0x4001, 0x4003, 0x400c, 0x4010,
	0x401d, 0x401e, 0x401f, 0x4020,
	0x4026, 0x4027, 0x4028,
	0x403c, 0x403d, 0x403e, 0x403f, 0x4040,
	0x4200, 0x7ffe, 0x7fff
};

static long double
randval(int mode)
{
	uint64_t r, man;
	uint16_t se;
	long double v, f;
	int sh, i;

	switch (mode) {
	case 0:				/* interesting exponent, random man */
		se = rexps[nextrand() % (sizeof(rexps) / sizeof(rexps[0]))];
		man = nextrand();
		if ((nextrand() & 3) != 0)
			man |= 0x8000000000000000ULL;	/* normalised */
		if (nextrand() & 1)
			se |= 0x8000;
		return (mkld(se, man));

	case 1:				/* integer of random width + fraction */
		sh = (int)(nextrand() % 63);
		v = (long double)(uint64_t)(nextrand() >> sh);
		f = (long double)(nextrand() >> 11) * 0x1p-53L;
		v = v + f;
		return ((nextrand() & 1) ? -v : v);

	case 2:				/* wholly random bit pattern */
		return (mkld((uint16_t)nextrand(), nextrand()));

	case 3:				/* somewhere in [0,1) */
		se = (uint16_t)(0x3ffe - (nextrand() % 45));
		man = nextrand() | 0x8000000000000000ULL;
		if (nextrand() & 1)
			se |= 0x8000;
		return (mkld(se, man));

	case 4: {			/* a few ulps off a boundary */
		static const long double bases[] = {
			0x1p-34L, 0x1p-35L, 0.125L, 0.25L, 0.5L, 0.75L,
			1.0L, 1.5L, 2.0L, 2.5L, 0x1p29L, 0x1p30L, 0x1p31L,
			0x1p32L, 0x1p39L, 0x1p40L, 0x1p41L, 0x1p62L,
			0x1p63L, 0x1p64L
		};
		v = bases[nextrand() % (sizeof(bases) / sizeof(bases[0]))];
		r = nextrand();
		sh = (int)(r % 9) - 4;
		for (i = 0; i < (sh < 0 ? -sh : sh); i++)
			v = nextafterl(v, sh < 0 ? LD_NINF : LD_INF);
		return ((r & 0x100) ? -v : v);
	}

	default: {			/* integer + exact quarter */
		static const long double q[] = { 0.0L, 0.25L, 0.5L, 0.75L };
		sh = (int)(nextrand() % 63);
		v = (long double)(uint64_t)(nextrand() >> sh);
		v = v + q[nextrand() & 3];
		return ((nextrand() & 1) ? -v : v);
	}
	}
}

/* ------------------------------------------------------------------ */
/* cexpl inputs							      */
/* ------------------------------------------------------------------ */

/*
 * exp_ovfl  = 11356.5234062941439497 == 0x400c:b17217f7d1cf79ac
 * cexp_ovfl = 22755.3287906024445633 == 0x400d:b1c6a8573de9768c
 * cexpl() compares the mantissa against those two words, so both sides of
 * both comparisons have to be visited exactly.
 */
static long double xre[512];
static int nxre;
static long double xim[256];
static int nxim;

static void
addre(long double x)
{
	if (nxre < (int)(sizeof(xre) / sizeof(xre[0])))
		xre[nxre++] = x;
}

static void
addim(long double x)
{
	if (nxim < (int)(sizeof(xim) / sizeof(xim[0])))
		xim[nxim++] = x;
}

static void
build_cexpl_vals(void)
{
	long double eo = mkld(0x400c, 0xb17217f7d1cf79acULL);
	long double co = mkld(0x400d, 0xb1c6a8573de9768cULL);
	long double v;
	int i;

	addre(mkld(0x0000, 0));
	addre(mkld(0x8000, 0));
	addre(mkld(0x0000, 1));
	addre(-mkld(0x0000, 1));
	addre(1.0L);
	addre(-1.0L);
	addre(2.0L);
	addre(-2.0L);
	addre(0.5L);
	addre(-0.5L);
	addre(700.0L);
	addre(-700.0L);
	addre(1e-20L);
	addre(-1e-20L);
	addre(11356.0L);
	addre(-11356.0L);

	/* Both sides of lx > 0xb17217f7d1cf79ac, hx == 0x400c and 0xc00c. */
	v = eo;
	for (i = 0; i < 3; i++)
		v = nextafterl(v, LD_NINF);
	for (i = 0; i < 7; i++) {
		addre(v);
		addre(-v);
		v = nextafterl(v, LD_INF);
	}
	addre(eo + 0.25L);
	addre(eo + 0.5L);
	addre(eo + 0.75L);
	addre(eo + 1.0L);
	addre(-(eo + 0.5L));
	addre(11357.0L);
	addre(11400.0L);
	addre(16383.0L);
	addre(16384.0L);		/* hx becomes 0x400d */
	addre(nextafterl(16384.0L, LD_NINF));
	addre(20000.0L);
	addre(-20000.0L);

	/* Both sides of lx < 0xb1c6a8573de9768c, hx == 0x400d. */
	v = co;
	for (i = 0; i < 3; i++)
		v = nextafterl(v, LD_NINF);
	for (i = 0; i < 7; i++) {
		addre(v);
		addre(-v);
		v = nextafterl(v, LD_INF);
	}
	addre(22756.0L);
	addre(30000.0L);
	addre(-30000.0L);
	addre(32767.0L);
	addre(32768.0L);		/* hx becomes 0x400e */
	addre(1e300L);
	addre(-1e300L);
	addre(mkld(0x7ffe, 0xffffffffffffffffULL));	/* LDBL_MAX */
	addre(-mkld(0x7ffe, 0xffffffffffffffffULL));
	addre(LD_INF);
	addre(LD_NINF);
	addre(mkld(0x7fff, 0xc000000000000000ULL));
	addre(mkld(0xffff, 0xc000000000000000ULL));
	addre(mkld(0x7fff, 0x8000000000000001ULL));
	addre(mkld(0x7fff, 0xffffffffffffffffULL));

	addim(mkld(0x0000, 0));		/* +0: the (hy|ly) == 0 branch */
	addim(mkld(0x8000, 0));		/* -0 */
	addim(mkld(0x0000, 1));		/* smallest subnormal */
	addim(-mkld(0x0000, 1));
	addim(1e-20L);
	addim(0.5L);
	addim(-0.5L);
	addim(1.0L);
	addim(-1.0L);
	addim(2.0L);
	addim(-2.0L);
	addim(3.0L);
	addim(1.5707963267948966192L);	/* ~pi/2, cos(y) tiny */
	addim(-1.5707963267948966192L);
	addim(3.1415926535897932385L);
	addim(1e10L);
	addim(-1e10L);
	addim(1e300L);
	addim(0x1p16383L);
	addim(LD_INF);
	addim(LD_NINF);
	addim(mkld(0x7fff, 0xc000000000000000ULL));
	addim(mkld(0xffff, 0xc000000000000000ULL));
	addim(mkld(0x7fff, 0x8000000000000001ULL));
	addim(mkld(0x7fff, 0x0000000000000000ULL));
}

static long double
rand_re(int mode)
{
	long double eo = mkld(0x400c, 0xb17217f7d1cf79acULL);
	long double co = mkld(0x400d, 0xb1c6a8573de9768cULL);
	long double d;
	uint16_t se;
	uint64_t man;

	switch (mode) {
	case 0:
		se = rexps[nextrand() % (sizeof(rexps) / sizeof(rexps[0]))];
		man = nextrand() | 0x8000000000000000ULL;
		if (nextrand() & 1)
			se |= 0x8000;
		return (mkld(se, man));
	case 1:				/* dense around exp_ovfl */
		d = (long double)(int64_t)(nextrand() % 4001 - 2000) / 1000.0L;
		return ((nextrand() & 1) ? eo + d : -(eo + d));
	case 2:				/* dense around cexp_ovfl */
		d = (long double)(int64_t)(nextrand() % 4001 - 2000) / 1000.0L;
		return ((nextrand() & 1) ? co + d : -(co + d));
	case 3:				/* mantissa sweep at hx 0x400c/0x400d */
		se = (nextrand() & 1) ? 0x400c : 0x400d;
		if (nextrand() & 1)
			se |= 0x8000;
		return (mkld(se, nextrand() | 0x8000000000000000ULL));
	case 4:				/* moderate magnitudes */
		d = (long double)(int64_t)(nextrand() % 60001 - 30000) +
		    (long double)(nextrand() >> 11) * 0x1p-53L;
		return (d);
	default:
		return (mkld((uint16_t)nextrand(), nextrand()));
	}
}

static long double
rand_im(int mode)
{
	uint16_t se;
	uint64_t man;
	long double v;

	switch (mode) {
	case 0:
		v = (long double)(int64_t)(nextrand() % 2000001 - 1000000) /
		    1000.0L;
		return (v);
	case 1:
		se = (uint16_t)(0x3fff + (int)(nextrand() % 80) - 40);
		man = nextrand() | 0x8000000000000000ULL;
		if (nextrand() & 1)
			se |= 0x8000;
		return (mkld(se, man));
	case 2:
		return (xim[nextrand() % (uint64_t)nxim]);
	default:
		se = (uint16_t)(nextrand() % 0x4300);
		man = nextrand();
		if (nextrand() & 1)
			se |= 0x8000;
		return (mkld(se, man));
	}
}

/* ------------------------------------------------------------------ */

#define	SWEEP1	300000
#define	SWEEPC	250000

int
main(void)
{
	long long i;
	int j;
	char tag[64];

	build_vals();
	build_cexpl_vals();

	std::printf("b0093 differential test: %d hand written values, "
	    "%d/%d cexpl components\n", nvals, nxre, nxim);

	/* --- hand written cases ------------------------------------- */
	for (j = 0; j < nvals; j++) {
		std::snprintf(tag, sizeof(tag), "edge #%d", j);
		check1(st_cospil, P::cospil, ref_cospil, vals[j], tag);
		check1(st_sinpil, P::sinpil, ref_sinpil, vals[j], tag);
		check1(st_tanpil, P::tanpil, ref_tanpil, vals[j], tag);
	}

	for (j = 0; j < nxre; j++) {
		int k;

		for (k = 0; k < nxim; k++) {
			std::snprintf(tag, sizeof(tag), "edge #%d,%d", j, k);
			check_cexpl(xre[j], xim[k], tag);
		}
	}

	/* --- randomised sweeps -------------------------------------- */
	for (i = 0; i < SWEEP1; i++) {
		long double x = randval((int)(i % 6));

		std::snprintf(tag, sizeof(tag), "rand #%lld", i);
		check1(st_cospil, P::cospil, ref_cospil, x, tag);
		x = randval((int)((i + 1) % 6));
		check1(st_sinpil, P::sinpil, ref_sinpil, x, tag);
		x = randval((int)((i + 2) % 6));
		check1(st_tanpil, P::tanpil, ref_tanpil, x, tag);
	}

	for (i = 0; i < SWEEPC; i++) {
		long double re = rand_re((int)(i % 6));
		long double im = rand_im((int)(i % 4));

		std::snprintf(tag, sizeof(tag), "rand #%lld", i);
		check_cexpl(re, im, tag);
	}

	/* --- report -------------------------------------------------- */
	{
		Fstat *all[4] = { &st_cospil, &st_sinpil, &st_tanpil,
		    &st_cexpl };
		long long tc = 0, tf = 0;

		std::printf("\n%-10s %12s %12s %12s %12s\n", "function",
		    "cases", "failures", "value", "flags");
		std::printf("%-10s %12s %12s %12s %12s\n", "--------",
		    "-----", "--------", "-----", "-----");
		for (j = 0; j < 4; j++) {
			std::printf("%-10s %12lld %12lld %12lld %12lld\n",
			    all[j]->name, all[j]->cases, all[j]->fails,
			    all[j]->valfails, all[j]->flagfails);
			tc += all[j]->cases;
			tf += all[j]->fails;
		}
		std::printf("%-10s %12lld %12lld\n", "TOTAL", tc, tf);
		std::printf("\n%s\n", tf == 0 ? "PASS" : "FAIL");
		return (tf == 0 ? 0 : 1);
	}
}
