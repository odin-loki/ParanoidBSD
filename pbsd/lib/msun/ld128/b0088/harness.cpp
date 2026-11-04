/*
 * PBSD batch b0088 -- differential test of the C++23 port against the
 * unmodified C oracle.
 *
 * Every function of the batch (and every kernel the batch pulls in) is driven
 * with hand written edge cases and with a fixed-seed randomised sweep, and the
 * two implementations are compared BIT FOR BIT: the 10 significant bytes of
 * each 80-bit long double result must be identical, so signed zeros, infinity
 * signs, NaN payloads and last-ulp differences are all caught.
 *
 * __k_expl() writes through pointers, so it is driven with two guard-filled
 * buffers: identical inputs, and afterwards the whole buffer is compared,
 * including the bytes past the objects being written to.
 */

#include <cfloat>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.ld128.b0088;

namespace port = pbsd::lib_msun_ld128::b0088;

extern "C" {
long double ref___kernel_cosl(long double, long double);
long double ref___kernel_sinl(long double, long double, int);
long double ref___kernel_tanl(long double, long double, int);
long double ref___kernel_cospil(long double);
long double ref___kernel_sinpil(long double);
long double ref___kernel_tanpil(long double);
long double ref_cospil(long double);
long double ref_sinpil(long double);
long double ref_tanpil(long double);
void ref___k_expl(long double, long double *, long double *, int *);
__complex__ long double ref___ldexp_cexpl(__complex__ long double, int);
__complex__ long double ref_cexpl(__complex__ long double);
}

using cld = std::complex<long double>;

/* ------------------------------------------------------------------ */
/* Bit-exact comparison and reporting.					*/
/* ------------------------------------------------------------------ */

/*
 * An x87 long double occupies 16 bytes of storage but only the first 10 hold
 * the value; the remaining 6 are padding that no store is required to write.
 */
#define	LD_SIGBYTES	10

static bool
same_ld(long double a, long double b)
{
	unsigned char ba[LD_SIGBYTES], bb[LD_SIGBYTES];

	std::memcpy(ba, &a, LD_SIGBYTES);
	std::memcpy(bb, &b, LD_SIGBYTES);
	return (std::memcmp(ba, bb, LD_SIGBYTES) == 0);
}

static void
show_ld(const char *tag, long double v)
{
	unsigned char b[LD_SIGBYTES];

	std::memcpy(b, &v, LD_SIGBYTES);
	std::printf("%s%.21Lg [", tag, v);
	for (int i = LD_SIGBYTES - 1; i >= 0; i--)
		std::printf("%02x", b[i]);
	std::printf("]");
}

struct Stat {
	const char	*name;
	long long	 cases;
	long long	 fails;
	int		 shown;
};

static Stat st_kcosl	= { "__kernel_cosl",	0, 0, 0 };
static Stat st_ksinl	= { "__kernel_sinl",	0, 0, 0 };
static Stat st_ktanl	= { "__kernel_tanl",	0, 0, 0 };
static Stat st_kcospil	= { "__kernel_cospil",	0, 0, 0 };
static Stat st_ksinpil	= { "__kernel_sinpil",	0, 0, 0 };
static Stat st_ktanpil	= { "__kernel_tanpil",	0, 0, 0 };
static Stat st_cospil	= { "cospil",		0, 0, 0 };
static Stat st_sinpil	= { "sinpil",		0, 0, 0 };
static Stat st_tanpil	= { "tanpil",		0, 0, 0 };
static Stat st_kexpl	= { "__k_expl",		0, 0, 0 };
static Stat st_ldcexpl	= { "__ldexp_cexpl",	0, 0, 0 };
static Stat st_cexpl	= { "cexpl",		0, 0, 0 };

static Stat *const all_stats[] = {
	&st_kcosl, &st_ksinl, &st_ktanl,
	&st_kcospil, &st_ksinpil, &st_ktanpil,
	&st_cospil, &st_sinpil, &st_tanpil,
	&st_kexpl, &st_ldcexpl, &st_cexpl,
};

#define	MAX_SHOWN	6

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
/* Random number generation (fixed seed, splitmix64).			*/
/* ------------------------------------------------------------------ */

static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{
	rng_state = s;
}

static uint64_t
rng_u64(void)
{
	uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static uint32_t
rng_below(uint32_t n)
{
	return ((uint32_t)(rng_u64() % n));
}

/* ------------------------------------------------------------------ */
/* Interesting values.							*/
/* ------------------------------------------------------------------ */

/*
 * Every threshold the batch tests against, both sides of every one of them,
 * every quarter-integer (the cospil/sinpil/tanpil argument reduction keys on
 * 0, 0.25, 0.5 and 0.75 exactly), the 0x1p112 / 0x1p113 huge-argument
 * thresholds, the 0x1p-60 tiny-argument threshold and the extremes of the
 * format.
 */
static const long double base_vals[] = {
	0.0L,
	0x1p-16445L,			/* LDBL_TRUE_MIN (denormal) */
	0x1p-16400L,
	LDBL_MIN,
	0x1p-64L,
	0x1p-61L,
	0x1p-60L,			/* tiny-argument threshold */
	0x1p-59L,
	0x1p-30L,
	0.1L, 0.2L,
	0.25L,				/* reduction boundary */
	0.3L,
	1.0L / 3.0L,
	0.4L,
	0.5L,				/* reduction boundary */
	0.6L,
	2.0L / 3.0L,
	0.7L,
	0.75L,				/* reduction boundary */
	0.8L, 0.9L, 0.99L, 0.999L,
	0.67433L, 0.67434L, 0.67435L,	/* __kernel_tanl branch */
	1.0L,				/* |x| <= 1 boundary */
	1.25L, 1.5L, 1.75L,
	2.0L, 2.25L, 2.5L, 2.75L,
	3.0L, 3.25L, 3.5L, 3.75L,
	4.0L, 4.5L, 5.0L, 5.5L, 6.0L, 7.0L, 8.0L,
	100.0L, 100.25L, 100.5L, 100.75L,
	12345.678L,
	1e5L, 100000.25L, 100000.5L,
	1e10L, 10000000000.25L, 10000000000.5L, 10000000000.75L,
	0x1p50L, 0x1p50L + 0.25L, 0x1p50L + 0.5L, 0x1p50L + 0.75L,
	0x1p52L + 0.25L,
	0x1p62L, 0x1p62L + 0.5L,
	0x1p63L, 0x1p63L + 1.0L,
	0x1p64L, 0x1p64L + 2.0L,
	0x1p100L,
	0x1p111L,
	0x1p112L,			/* huge-argument threshold */
	0x1p112L + 0x1p60L,
	0x1p113L,			/* huge-argument threshold */
	0x1p114L,
	LDBL_MAX,
	(long double)INFINITY,
	(long double)NAN,
};

#define	NBASE		((int)(sizeof(base_vals) / sizeof(base_vals[0])))
/* v, -v, nextafter up, nextafter down, and the negations of those two. */
#define	NPERBASE	6
#define	NEDGE		(NBASE * NPERBASE)

static long double
edge_val(int i)
{
	long double v = base_vals[i / NPERBASE];

	switch (i % NPERBASE) {
	case 0:	return (v);
	case 1:	return (-v);
	case 2:	return (nextafterl(v, (long double)INFINITY));
	case 3:	return (nextafterl(v, -(long double)INFINITY));
	case 4:	return (-nextafterl(v, (long double)INFINITY));
	default: return (-nextafterl(v, -(long double)INFINITY));
	}
}

/*
 * A random long double.  The classes are chosen so that the whole of each
 * function's decision tree gets hit: tiny arguments, arguments inside each
 * quarter of [0,1), arguments a few ulps either side of a threshold, large
 * arguments with an exactly-representable quarter-integer fraction, and
 * arguments spread over the entire exponent range.
 */
static long double
rand_ld(void)
{
	uint64_t m;
	long double v;
	int e;

	switch (rng_below(10)) {
	case 0:
		return (edge_val((int)rng_below(NEDGE)));
	case 1:
		/* Uniform in [0,1). */
		v = (long double)(rng_u64() >> 1) * 0x1p-63L;
		break;
	case 2: {
		/* A few ulps either side of a threshold. */
		static const long double thr[] = {
			0.0L, 0x1p-60L, 0.25L, 0.5L, 0.67434L, 0.75L, 1.0L,
			2.0L, 0x1p112L, 0x1p113L,
		};
		int k = (int)rng_below(2 * 8 + 1) - 8;
		v = thr[rng_below((uint32_t)(sizeof(thr) / sizeof(thr[0])))];
		for (; k > 0; k--)
			v = nextafterl(v, (long double)INFINITY);
		for (; k < 0; k++)
			v = nextafterl(v, -(long double)INFINITY);
		break;
	}
	case 3: {
		/* Integer part + an exact quarter/eighth fraction. */
		static const long double fr[] = {
			0.0L, 0.125L, 0.25L, 0.375L, 0.5L, 0.625L, 0.75L,
			0.875L,
		};
		uint64_t n = rng_u64() >> (14 + rng_below(38));
		v = (long double)n + fr[rng_below(8)];
		break;
	}
	case 4:
		/* Integer part + a random fraction. */
		v = (long double)(rng_u64() >> (12 + rng_below(40))) +
		    (long double)(rng_u64() >> 1) * 0x1p-63L;
		break;
	case 5:
		/* Tiny, around the 0x1p-60 threshold. */
		m = rng_u64() | (1ULL << 63);
		e = -70 + (int)rng_below(20);
		v = ldexpl((long double)m, e - 63);
		break;
	case 6:
		/* Huge, around the 0x1p112 / 0x1p113 thresholds. */
		m = rng_u64() | (1ULL << 63);
		e = 100 + (int)rng_below(30);
		v = ldexpl((long double)m, e - 63);
		break;
	case 7:
		/* Moderate. */
		m = rng_u64() | (1ULL << 63);
		e = -20 + (int)rng_below(60);
		v = ldexpl((long double)m, e - 63);
		break;
	case 8:
		/* Anywhere in the exponent range. */
		m = rng_u64() | (1ULL << 63);
		e = -16380 + (int)rng_below(32760);
		v = ldexpl((long double)m, e - 63);
		break;
	default:
		/* Random bit soup in [0,4). */
		m = rng_u64();
		v = (long double)m * 0x1p-62L;
		break;
	}
	return ((rng_u64() & 1) ? -v : v);
}

/* A small argument, the domain the sin/cos/tan kernels are called on. */
static long double
rand_small(void)
{
	long double v;

	switch (rng_below(6)) {
	case 0:
		v = (long double)(rng_u64() >> 1) * 0x1p-63L * 0.7854L;
		break;
	case 1:
		v = (long double)(rng_u64() >> 1) * 0x1p-63L * 0.67434L;
		break;
	case 2:
		v = nextafterl(0.67434L, (rng_u64() & 1) ?
		    (long double)INFINITY : -(long double)INFINITY);
		break;
	case 3:
		v = ldexpl((long double)(rng_u64() | (1ULL << 63)),
		    -63 - (int)rng_below(80));
		break;
	case 4:
		v = edge_val((int)rng_below(NEDGE));
		break;
	default:
		v = (long double)(rng_u64() >> 1) * 0x1p-63L;
		break;
	}
	return ((rng_u64() & 1) ? -v : v);
}

/* ------------------------------------------------------------------ */
/* Per-function case runners.						*/
/* ------------------------------------------------------------------ */

static void
case_kcosl(long double x, long double y)
{
	long double p, r;

	st_kcosl.cases++;
	p = port::__kernel_cosl(x, y);
	r = ref___kernel_cosl(x, y);
	if (!same_ld(p, r) && fail_head(st_kcosl)) {
		show_ld("x=", x); show_ld(" y=", y);
		show_ld(" port=", p); show_ld(" ref=", r);
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
		show_ld("x=", x); show_ld(" y=", y);
		std::printf(" iy=%d", iy);
		show_ld(" port=", p); show_ld(" ref=", r);
		std::printf("\n");
	}
}

static void
case_ktanl(long double x, long double y, int iy)
{
	long double p, r;

	st_ktanl.cases++;
	p = port::__kernel_tanl(x, y, iy);
	r = ref___kernel_tanl(x, y, iy);
	if (!same_ld(p, r) && fail_head(st_ktanl)) {
		show_ld("x=", x); show_ld(" y=", y);
		std::printf(" iy=%d", iy);
		show_ld(" port=", p); show_ld(" ref=", r);
		std::printf("\n");
	}
}

#define	DEFINE_UNARY_CASE(fn, stat)					\
static void								\
case_##fn(long double x)						\
{									\
	long double p, r;						\
									\
	stat.cases++;							\
	p = port::fn(x);						\
	r = ref_##fn(x);						\
	if (!same_ld(p, r) && fail_head(stat)) {			\
		show_ld("x=", x);					\
		show_ld(" port=", p); show_ld(" ref=", r);		\
		std::printf("\n");					\
	}								\
}

DEFINE_UNARY_CASE(__kernel_cospil, st_kcospil)
DEFINE_UNARY_CASE(__kernel_sinpil, st_ksinpil)
DEFINE_UNARY_CASE(__kernel_tanpil, st_ktanpil)
DEFINE_UNARY_CASE(cospil, st_cospil)
DEFINE_UNARY_CASE(sinpil, st_sinpil)
DEFINE_UNARY_CASE(tanpil, st_tanpil)

/*
 * __k_expl() stores two long doubles and an int through pointers.  Both
 * implementations get their own buffer, both prefilled with the guard byte
 * 0x7f, and the whole buffer is inspected afterwards.
 */
#define	KBUF_SIZE	96
#define	OFF_HI		0
#define	OFF_LO		16
#define	OFF_K		32
#define	OFF_TAIL	40		/* nothing may be written from here */
#define	GUARD		0x7f

struct kbuf {
	alignas(16) unsigned char b[KBUF_SIZE];
};

static void
case_kexpl(long double x)
{
	struct kbuf pb, rb;
	long double phi, plo, rhi, rlo;
	int pk, rk;
	bool bad = false;

	st_kexpl.cases++;
	std::memset(pb.b, GUARD, sizeof(pb.b));
	std::memset(rb.b, GUARD, sizeof(rb.b));

	port::__k_expl(x, (long double *)(pb.b + OFF_HI),
	    (long double *)(pb.b + OFF_LO), (int *)(pb.b + OFF_K));
	ref___k_expl(x, (long double *)(rb.b + OFF_HI),
	    (long double *)(rb.b + OFF_LO), (int *)(rb.b + OFF_K));

	std::memcpy(&phi, pb.b + OFF_HI, sizeof(phi));
	std::memcpy(&plo, pb.b + OFF_LO, sizeof(plo));
	std::memcpy(&pk, pb.b + OFF_K, sizeof(pk));
	std::memcpy(&rhi, rb.b + OFF_HI, sizeof(rhi));
	std::memcpy(&rlo, rb.b + OFF_LO, sizeof(rlo));
	std::memcpy(&rk, rb.b + OFF_K, sizeof(rk));

	if (!same_ld(phi, rhi) || !same_ld(plo, rlo) || pk != rk)
		bad = true;
	/* Nothing at all may be touched past the three output objects. */
	for (int i = OFF_TAIL; i < KBUF_SIZE; i++)
		if (pb.b[i] != GUARD || rb.b[i] != GUARD)
			bad = true;
	/* The 4 bytes between the int and the tail must be untouched too. */
	for (int i = OFF_K + 4; i < OFF_TAIL; i++)
		if (pb.b[i] != GUARD || rb.b[i] != GUARD)
			bad = true;

	if (bad && fail_head(st_kexpl)) {
		show_ld("x=", x);
		show_ld(" port.hi=", phi); show_ld(" ref.hi=", rhi);
		show_ld(" port.lo=", plo); show_ld(" ref.lo=", rlo);
		std::printf(" port.k=%d ref.k=%d\n", pk, rk);
		std::printf("     port buf:");
		for (int i = 0; i < KBUF_SIZE; i++)
			std::printf("%s%02x", i == OFF_TAIL ? "|" : " ",
			    pb.b[i]);
		std::printf("\n     ref  buf:");
		for (int i = 0; i < KBUF_SIZE; i++)
			std::printf("%s%02x", i == OFF_TAIL ? "|" : " ",
			    rb.b[i]);
		std::printf("\n");
	}
}

static void
case_ldcexpl(long double x, long double y, int expt)
{
	__complex__ long double rz, rr;
	cld pr;

	st_ldcexpl.cases++;
	__real__ rz = x;
	__imag__ rz = y;
	pr = port::__ldexp_cexpl(cld(x, y), expt);
	rr = ref___ldexp_cexpl(rz, expt);
	if ((!same_ld(pr.real(), __real__ rr) ||
	    !same_ld(pr.imag(), __imag__ rr)) && fail_head(st_ldcexpl)) {
		show_ld("x=", x); show_ld(" y=", y);
		std::printf(" expt=%d", expt);
		show_ld(" port.re=", pr.real());
		show_ld(" ref.re=", __real__ rr);
		show_ld(" port.im=", pr.imag());
		show_ld(" ref.im=", __imag__ rr);
		std::printf("\n");
	}
}

static void
case_cexpl(long double x, long double y)
{
	__complex__ long double rz, rr;
	cld pr;

	st_cexpl.cases++;
	__real__ rz = x;
	__imag__ rz = y;
	pr = port::cexpl(cld(x, y));
	rr = ref_cexpl(rz);
	if ((!same_ld(pr.real(), __real__ rr) ||
	    !same_ld(pr.imag(), __imag__ rr)) && fail_head(st_cexpl)) {
		show_ld("x=", x); show_ld(" y=", y);
		show_ld(" port.re=", pr.real());
		show_ld(" ref.re=", __real__ rr);
		show_ld(" port.im=", pr.imag());
		show_ld(" ref.im=", __imag__ rr);
		std::printf("\n");
	}
}

/* ------------------------------------------------------------------ */
/* cexpl()-specific interesting values.					*/
/* ------------------------------------------------------------------ */

/*
 * exp_ovfl and cexp_ovfl as spelled in s_cexpl.c; the harness needs them to
 * be able to sit on both sides of the scaling window.
 */
static const long double h_cexp_ovfl = 2.27892930024498818830197576893019292e+04L;
static const long double h_exp_ovfl = 1.13565234062941439494919310779707649e+04L;

static const long double cexp_x_vals[] = {
	0.0L, -0.0L,
	0x1p-16445L, -0x1p-16445L,
	1e-30L, -1e-30L,
	1.0L, -1.0L, 2.5L, -2.5L,
	700.0L, -700.0L,
	11000.0L, -11000.0L,
	11356.0L,
	11356.5L,
	11357.0L,
	11360.0L,
	11400.0L,
	12000.0L,
	20000.0L,
	22700.0L,
	22789.0L,
	23000.0L,
	30000.0L,
	-11400.0L,
	-30000.0L,
	1e100L, -1e100L,
	LDBL_MAX, -LDBL_MAX,
	(long double)INFINITY, -(long double)INFINITY,
	(long double)NAN, -(long double)NAN,
};

static const long double cexp_y_vals[] = {
	0.0L, -0.0L,
	0x1p-16445L, -0x1p-16445L,
	1e-30L,
	0.5L, -0.5L,
	1.0L, -1.0L,
	1.5707963267948966192L, -1.5707963267948966192L,
	3.1415926535897932385L,
	100.0L, -100.0L,
	1e10L,
	1e30L,
	LDBL_MAX,
	(long double)INFINITY, -(long double)INFINITY,
	(long double)NAN, -(long double)NAN,
};

#define	NCEXPX	((int)(sizeof(cexp_x_vals) / sizeof(cexp_x_vals[0])))
#define	NCEXPY	((int)(sizeof(cexp_y_vals) / sizeof(cexp_y_vals[0])))

/*
 * An argument __k_expl() is defined for: finite, and small enough that the
 * (int) conversion of x * INV_L is in range.
 */
static long double
rand_kexpl_arg(void)
{
	long double v;

	switch (rng_below(6)) {
	case 0:
		v = h_exp_ovfl +
		    (long double)(rng_u64() >> 1) * 0x1p-63L * 200.0L;
		break;
	case 1:
		v = (long double)(rng_u64() >> 1) * 0x1p-63L * 1400.0L;
		break;
	case 2:
		v = (long double)(rng_u64() >> 1) * 0x1p-63L * 90000.0L;
		break;
	case 3:
		v = ldexpl((long double)(rng_u64() | (1ULL << 63)),
		    -63 - (int)rng_below(120));
		break;
	case 4:
		v = (long double)(rng_u64() >> 1) * 0x1p-63L * 0.006L;
		break;
	default:
		v = (long double)(rng_u64() >> 1) * 0x1p-63L * 11400.0L;
		break;
	}
	return ((rng_u64() & 1) ? -v : v);
}

/* An argument for the imaginary part: sincosl() eats anything finite. */
static long double
rand_y(void)
{
	long double v;

	switch (rng_below(5)) {
	case 0:
		return (cexp_y_vals[rng_below(NCEXPY)]);
	case 1:
		v = (long double)(rng_u64() >> 1) * 0x1p-63L * 10.0L;
		break;
	case 2:
		v = (long double)(rng_u64() >> 1) * 0x1p-63L * 1e6L;
		break;
	case 3:
		v = ldexpl((long double)(rng_u64() | (1ULL << 63)),
		    -63 - (int)rng_below(200));
		break;
	default:
		v = rand_ld();
		break;
	}
	return ((rng_u64() & 1) ? -v : v);
}

/* ------------------------------------------------------------------ */

#define	RANDOM_ITERS	200000

int
main(void)
{
	int i, j;

	/* ---------------- hand written edge cases ---------------- */

	for (i = 0; i < NEDGE; i++) {
		long double x = edge_val(i);

		case___kernel_cospil(x);
		case___kernel_sinpil(x);
		case___kernel_tanpil(x);
		case_cospil(x);
		case_sinpil(x);
		case_tanpil(x);
	}

	/*
	 * The kernels take a two-part argument; pair every edge value with a
	 * set of fixed tails, and walk every iy.
	 */
	{
		static const long double tails[] = {
			0.0L, -0.0L, 0x1p-70L, -0x1p-70L, 1e-20L, -1e-20L,
			0.5L, -0.5L, (long double)INFINITY, (long double)NAN,
		};
		for (i = 0; i < NEDGE; i++) {
			long double x = edge_val(i);

			for (j = 0; j < (int)(sizeof(tails) /
			    sizeof(tails[0])); j++) {
				case_kcosl(x, tails[j]);
				case_ksinl(x, tails[j], 0);
				case_ksinl(x, tails[j], 1);
				case_ksinl(x, tails[j], -1);
				case_ktanl(x, tails[j], 1);
				case_ktanl(x, tails[j], -1);
				case_ktanl(x, tails[j], 0);
				case_ktanl(x, tails[j], 2);
			}
		}
	}

	/* __k_expl() edge cases: everything finite and in (int) range. */
	{
		static const long double kx[] = {
			0.0L, -0.0L, 0x1p-16445L, -0x1p-16445L, 1e-40L,
			0.0027L, -0.0027L, 0.002708L, -0.002708L,
			0.005415212348L, 0.00541521234812457272982212595914L,
			0.5L, -0.5L, 1.0L, -1.0L, 2.0L, -2.0L,
			10.0L, -10.0L, 100.0L, -100.0L, 700.0L, -700.0L,
			11356.0L, 11356.5L, 11357.0L, 11400.0L, 12000.0L,
			20000.0L, 22789.0L, -11400.0L, -20000.0L,
			90000.0L, -90000.0L, 11356.523406294143949L,
			22789.293002449881883L,
		};
		for (i = 0; i < (int)(sizeof(kx) / sizeof(kx[0])); i++) {
			case_kexpl(kx[i]);
			case_kexpl(nextafterl(kx[i], (long double)INFINITY));
			case_kexpl(nextafterl(kx[i], -(long double)INFINITY));
		}
		/* Every table entry must be reachable. */
		for (i = 0; i < 256; i++)
			case_kexpl((long double)i * 0.0054152123481245727L);
	}

	/* cexpl(): the full cross product of the interesting parts. */
	for (i = 0; i < NCEXPX; i++)
		for (j = 0; j < NCEXPY; j++) {
			case_cexpl(cexp_x_vals[i], cexp_y_vals[j]);
			case_cexpl(nextafterl(cexp_x_vals[i],
			    (long double)INFINITY), cexp_y_vals[j]);
			case_cexpl(nextafterl(cexp_x_vals[i],
			    -(long double)INFINITY), cexp_y_vals[j]);
		}
	/* Straddle both ends of the scaling window one ulp at a time. */
	{
		long double x;

		x = h_exp_ovfl;
		for (i = 0; i < 8; i++)
			x = nextafterl(x, -(long double)INFINITY);
		for (i = 0; i < 16; i++) {
			for (j = 0; j < NCEXPY; j++)
				case_cexpl(x, cexp_y_vals[j]);
			x = nextafterl(x, (long double)INFINITY);
		}
		x = h_cexp_ovfl;
		for (i = 0; i < 8; i++)
			x = nextafterl(x, -(long double)INFINITY);
		for (i = 0; i < 16; i++) {
			for (j = 0; j < NCEXPY; j++)
				case_cexpl(x, cexp_y_vals[j]);
			x = nextafterl(x, (long double)INFINITY);
		}
	}

	/* __ldexp_cexpl() edge cases. */
	{
		static const long double lx[] = {
			0.0L, -0.0L, 1.0L, -1.0L, 700.0L, -700.0L,
			11356.0L, 11357.0L, 11400.0L, 12000.0L, 20000.0L,
			22789.0L, -11400.0L, 1e-30L,
		};
		static const int lexpt[] = {
			0, 1, -1, 2, -2, 16382, -16382, 16383, -16383,
			1000, -1000, 32000, -32000,
		};
		for (i = 0; i < (int)(sizeof(lx) / sizeof(lx[0])); i++)
			for (j = 0; j < (int)(sizeof(lexpt) /
			    sizeof(lexpt[0])); j++) {
				case_ldcexpl(lx[i], 1.0L, lexpt[j]);
				case_ldcexpl(lx[i], 0.0L, lexpt[j]);
				case_ldcexpl(lx[i], -0.0L, lexpt[j]);
				case_ldcexpl(lx[i], 1e5L, lexpt[j]);
				case_ldcexpl(lx[i],
				    1.5707963267948966192L, lexpt[j]);
			}
	}

	/* ---------------- randomised sweeps ---------------- */

	rng_seed(0x0088c0ffeeULL);
	for (i = 0; i < RANDOM_ITERS; i++) {
		long double x = rand_ld();

		case_cospil(x);
		case_sinpil(x);
		case_tanpil(x);
	}

	rng_seed(0x0088beef01ULL);
	for (i = 0; i < RANDOM_ITERS; i++) {
		long double x = rand_ld();

		case___kernel_cospil(x);
		case___kernel_sinpil(x);
		case___kernel_tanpil(x);
	}

	rng_seed(0x0088d00d02ULL);
	for (i = 0; i < RANDOM_ITERS; i++) {
		long double x = rand_small();
		long double y = ldexpl(rand_small(),
		    -40 - (int)rng_below(40));

		case_kcosl(x, y);
		case_ksinl(x, y, (int)rng_below(2));
		case_ktanl(x, y, (int)rng_below(4) - 1);
	}

	rng_seed(0x0088abcd03ULL);
	for (i = 0; i < RANDOM_ITERS; i++)
		case_kexpl(rand_kexpl_arg());

	rng_seed(0x0088123404ULL);
	for (i = 0; i < RANDOM_ITERS; i++)
		case_ldcexpl(rand_kexpl_arg(), rand_y(),
		    (int)rng_below(70000) - 35000);

	rng_seed(0x0088feed05ULL);
	for (i = 0; i < RANDOM_ITERS; i++) {
		long double x;

		/* Bias towards the branch points of cexpl(). */
		switch (rng_below(4)) {
		case 0:
			x = cexp_x_vals[rng_below(NCEXPX)];
			break;
		case 1:
			x = h_exp_ovfl +
			    (long double)((int64_t)rng_u64() % 2000) * 0.5L;
			break;
		case 2:
			x = h_cexp_ovfl +
			    (long double)((int64_t)rng_u64() % 2000) * 0.5L;
			break;
		default:
			x = rand_ld();
			break;
		}
		case_cexpl(x, rand_y());
	}

	/* ---------------- report ---------------- */

	long long total_cases = 0, total_fails = 0;

	std::printf("\n%-20s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("------------------------------------------------------"
	    "-\n");
	for (i = 0; i < (int)(sizeof(all_stats) / sizeof(all_stats[0])); i++) {
		Stat *s = all_stats[i];

		total_cases += s->cases;
		total_fails += s->fails;
		std::printf("%-20s %12lld %10lld   %s\n", s->name, s->cases,
		    s->fails, s->fails == 0 ? "ok" : "FAILED");
	}
	std::printf("------------------------------------------------------"
	    "-\n");
	std::printf("%-20s %12lld %10lld   %s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "ok" : "FAILED");

	return (total_fails == 0 ? 0 : 1);
}
