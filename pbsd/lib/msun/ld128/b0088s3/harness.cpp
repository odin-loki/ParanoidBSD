/*
 * Differential test: pbsd::lib_msun_ld128::b0088s3::cexpl() vs ref_cexpl()
 * (the unmodified HardenedBSD lib/msun/ld128/s_cexpl.c body).
 *
 * Both results are written into two guard-filled (0x7f) buffers and the
 * ENTIRE buffer is compared byte for byte, so padding bytes and NaN
 * payloads are part of the verdict.
 */

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

import pbsd.lib.msun.ld128.b0088s3;

extern "C" long double _Complex ref_cexpl(long double _Complex z);

/*
 * k_expl.h's __ldexp_cexpl() is not part of this batch.  One definition is
 * supplied here and shared by the port and the reference, so the branch that
 * selects it stays observable: what it returns depends on both z and expt,
 * and differs from anything the fall-through branch can produce.
 */
extern "C" long double _Complex
__ldexp_cexpl(long double _Complex z, int expt)
{
	long double x = __real__ z;
	long double y = __imag__ z;
	long double s, c;

	sincosl(y, &s, &c);
	/* x / 4 keeps expl() finite over the whole (exp_ovfl, cexp_ovfl) window */
	long double m = expl(x / 4);
	long double _Complex r = ldexpl(m * c, expt);
	__imag__ r = ldexpl(m * s, expt);
	return (r);
}

/* ------------------------------------------------------------------ */
/* guarded result buffers                                              */
/* ------------------------------------------------------------------ */

#define GUARD 0x7f
#define BUFSZ 96
#define OFF   32

struct GBuf {
	alignas(16) unsigned char b[BUFSZ];
};

static void
gbuf_init(GBuf *g)
{
	memset(g->b, GUARD, sizeof(g->b));
}

static void
gbuf_put(GBuf *g, long double _Complex v)
{
	*reinterpret_cast<long double _Complex *>(g->b + OFF) = v;
}

static long double
gbuf_re(const GBuf *g)
{
	long double _Complex v;

	memcpy(&v, g->b + OFF, sizeof(v));
	return (__real__ v);
}

static long double
gbuf_im(const GBuf *g)
{
	long double _Complex v;

	memcpy(&v, g->b + OFF, sizeof(v));
	return (__imag__ v);
}

static void
dump_ld(const char *tag, long double v)
{
	union {
		long double d;
		unsigned char b[sizeof(long double)];
	} u;

	memset(u.b, 0, sizeof(u.b));
	u.d = v;
	printf("%s=%La [", tag, v);
	for (size_t i = 0; i < sizeof(u.b); i++)
		printf("%02x", u.b[i]);
	printf("]");
}

/* ------------------------------------------------------------------ */
/* PRNG                                                                */
/* ------------------------------------------------------------------ */

static uint64_t rng_state = 0x0088500300885003ULL;

static uint64_t
rnd(void)
{
	uint64_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return (x);
}

static long double
u01(void)
{
	return ((long double)(rnd() >> 11) / (long double)(1ULL << 53));
}

/* ------------------------------------------------------------------ */
/* interesting values                                                  */
/* ------------------------------------------------------------------ */

static const long double EXP_OVFL = 1.13565234062941439494919310779707649e+04L;
static const long double CEXP_OVFL = 2.27892930024498818830197576893019292e+04L;

static long double special[80];
static int nspecial;

static void
add_special(long double v)
{
	special[nspecial++] = v;
}

static void
init_specials(void)
{
	long double inf = HUGE_VALL;
	long double n1 = nanl("1");
	long double n2 = nanl("0x2a5");

	add_special(0.0L);
	add_special(-0.0L);
	add_special(1.0L);
	add_special(-1.0L);
	add_special(0.5L);
	add_special(-0.5L);
	add_special(2.0L);
	add_special(-2.0L);
	add_special(LDBL_MIN);
	add_special(-LDBL_MIN);
	add_special(LDBL_TRUE_MIN);
	add_special(-LDBL_TRUE_MIN);
	add_special(LDBL_EPSILON);
	add_special(LDBL_MAX);
	add_special(-LDBL_MAX);
	add_special(1e-30L);
	add_special(1e30L);
	add_special(3.14159265358979323846264338327950288L / 2);
	add_special(3.14159265358979323846264338327950288L);
	add_special(-3.14159265358979323846264338327950288L / 2);
	add_special(2 * 3.14159265358979323846264338327950288L);
	add_special(1e10L);
	add_special(-1e10L);
	add_special(1e18L);
	add_special(100.0L);
	add_special(-100.0L);
	add_special(708.0L);
	add_special(-708.0L);
	/* both sides of exp_ovfl */
	add_special(EXP_OVFL);
	add_special(nextafterl(EXP_OVFL, 0.0L));
	add_special(nextafterl(EXP_OVFL, 1e9L));
	add_special(11356.0L);
	add_special(11357.0L);
	/* inside the scaling window */
	add_special(12000.0L);
	add_special(17000.0L);
	add_special(20000.0L);
	add_special(22000.0L);
	add_special(22789.0L);
	/* both sides of cexp_ovfl */
	add_special(CEXP_OVFL);
	add_special(nextafterl(CEXP_OVFL, 0.0L));
	add_special(nextafterl(CEXP_OVFL, 1e9L));
	add_special(22790.0L);
	add_special(23000.0L);
	add_special(30000.0L);
	add_special(-11356.0L);
	add_special(-22789.0L);
	add_special(-30000.0L);
	add_special(inf);
	add_special(-inf);
	add_special(n1);
	add_special(-n1);
	add_special(n2);
}

static long double
gen(void)
{
	uint64_t r = rnd();
	long double sign = (r & 0x10000) ? -1.0L : 1.0L;

	switch ((int)(r % 16)) {
	case 0:
		return (sign * 0.0L);
	case 1:
		return (sign * 1.0L);
	case 2:
	case 3:
		return (sign * u01() * 30000.0L);
	case 4:
	case 5:
		/* dense across the (exp_ovfl, cexp_ovfl) window and its edges */
		return (sign * (EXP_OVFL - 2.0L +
		    u01() * (CEXP_OVFL - EXP_OVFL + 4.0L)));
	case 6:
		return (special[rnd() % (uint64_t)nspecial]);
	case 7:
		return (sign * u01() * 10.0L);
	case 8:
		/* full magnitude range, including subnormals */
		return (sign * ldexpl(0.5L + u01() / 2,
		    (int)(rnd() % 32700) - 16350));
	case 9:
		return (sign * u01() * 1e6L);
	case 10:
		return (sign * ldexpl(0.5L + u01() / 2,
		    -(int)(rnd() % 80)));
	case 11:
		return (sign * (11000.0L + u01() * 1000.0L));
	case 12:
		return (sign * (22000.0L + u01() * 1500.0L));
	case 13:
		return (sign * u01() * 800.0L);
	case 14:
		return (nextafterl(special[rnd() % (uint64_t)nspecial],
		    sign * HUGE_VALL));
	default:
		return (sign * u01() * 25000.0L);
	}
}

/* ------------------------------------------------------------------ */
/* the check                                                           */
/* ------------------------------------------------------------------ */

static long long cases_cexpl;
static long long fails_cexpl;
static int reported;

static void
check(long double x, long double y, const char *what)
{
	GBuf ga, gb;
	long double _Complex z;

	gbuf_init(&ga);
	gbuf_init(&gb);

	z = x;
	__imag__ z = y;

	gbuf_put(&ga, pbsd::lib_msun_ld128::b0088s3::cexpl(z));
	gbuf_put(&gb, ref_cexpl(z));

	cases_cexpl++;
	if (memcmp(ga.b, gb.b, BUFSZ) != 0) {
		fails_cexpl++;
		if (reported < 20) {
			reported++;
			printf("FAIL [%s] ", what);
			dump_ld("x", x);
			printf(" ");
			dump_ld("y", y);
			printf("\n      port: ");
			dump_ld("re", gbuf_re(&ga));
			printf(" ");
			dump_ld("im", gbuf_im(&ga));
			printf("\n      ref : ");
			dump_ld("re", gbuf_re(&gb));
			printf(" ");
			dump_ld("im", gbuf_im(&gb));
			printf("\n");
		}
	}
}

int
main(void)
{
	init_specials();

	/* hand-written edge cases: full cross product of the special values */
	for (int i = 0; i < nspecial; i++)
		for (int j = 0; j < nspecial; j++)
			check(special[i], special[j], "special-cross");

	/* explicit pairs that pin down individual branches */
	{
		long double inf = HUGE_VALL;
		long double nn = nanl("1");
		static const long double xs[] = {
			0.0L, -0.0L, 1.0L, -1.0L, EXP_OVFL,
			11356.0L, 11357.0L, 15000.0L, 22789.0L,
			22790.0L, 30000.0L, -30000.0L
		};

		for (size_t i = 0; i < sizeof(xs) / sizeof(xs[0]); i++) {
			check(xs[i], 0.0L, "y-zero");
			check(xs[i], -0.0L, "y-negzero");
			check(xs[i], 1.0L, "y-one");
			check(xs[i], -1.0L, "y-negone");
			check(xs[i], inf, "y-inf");
			check(xs[i], -inf, "y-neginf");
			check(xs[i], nn, "y-nan");
			check(0.0L, xs[i], "x-zero");
			check(-0.0L, xs[i], "x-negzero");
			check(1.0L, xs[i], "x-one");
			check(inf, xs[i], "x-inf");
			check(-inf, xs[i], "x-neginf");
			check(nn, xs[i], "x-nan");
		}

		/* the exact non-finite corners */
		check(inf, inf, "inf-inf");
		check(inf, -inf, "inf-neginf");
		check(-inf, inf, "neginf-inf");
		check(-inf, -inf, "neginf-neginf");
		check(inf, nn, "inf-nan");
		check(-inf, nn, "neginf-nan");
		check(nn, inf, "nan-inf");
		check(nn, nn, "nan-nan");
		check(nn, 0.0L, "nan-zero");
		check(0.0L, nn, "zero-nan");
		check(inf, 0.0L, "inf-zero");
		check(-inf, 0.0L, "neginf-zero");
		check(inf, 1.0L, "inf-one");
		check(-inf, 1.0L, "neginf-one");

		/* walk across both thresholds one ulp at a time, and probe
		 * their neighbourhoods at shrinking relative distances */
		static const long double thr[2] = { EXP_OVFL, CEXP_OVFL };

		for (size_t t = 0; t < 2; t++) {
			long double v = thr[t];

			for (int k = 0; k < 32; k++)
				v = nextafterl(v, 0.0L);
			for (int k = 0; k < 65; k++) {
				check(v, 1.0L, "threshold-ulp-walk");
				check(v, -1.0L, "threshold-ulp-walk");
				check(v, 3.0L, "threshold-ulp-walk");
				v = nextafterl(v, 1e9L);
			}
			for (int e = 10; e < 70; e++) {
				long double d = thr[t] * ldexpl(1.0L, -e);

				check(thr[t] - d, 1.0L, "threshold-relative");
				check(thr[t] + d, 1.0L, "threshold-relative");
				check(thr[t] - d, -2.5L, "threshold-relative");
				check(thr[t] + d, -2.5L, "threshold-relative");
			}
		}
	}

	/* fixed-seed randomised sweep */
	for (long i = 0; i < 250000; i++) {
		long double x = gen();
		long double y = gen();

		check(x, y, "random");
	}

	printf("\n%-16s %12s %12s\n", "function", "cases", "failures");
	printf("%-16s %12lld %12lld\n", "cexpl", cases_cexpl, fails_cexpl);

	if (fails_cexpl != 0) {
		printf("\nRESULT: FAIL\n");
		return (1);
	}
	printf("\nRESULT: PASS\n");
	return (0);
}
