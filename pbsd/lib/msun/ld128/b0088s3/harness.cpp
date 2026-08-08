/*
 * PBSD batch b0088s3 -- differential test of cexpl() and its kernels.
 */

#include <cfloat>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <cstring>

#define complex _Complex
#include <complex.h>

import pbsd.lib.msun.ld128.b0088s3;

namespace port = pbsd::lib_msun_ld128::b0088s3;

extern "C" {
void ref___k_expl(long double, long double *, long double *, int *);
long double complex ref___ldexp_cexpl(long double complex, int);
long double complex ref_cexpl(long double complex);
}

using cld = std::complex<long double>;

static bool
same_ld(long double a, long double b)
{
	return std::memcmp(&a, &b, sizeof(long double)) == 0;
}

static void
show_ld(const char *tag, long double v)
{
	const unsigned char *b = (const unsigned char *)&v;

	std::printf("%s%.33Lg [", tag, v);
	for (std::size_t i = 0; i < sizeof(long double); i++)
		std::printf("%02x", b[i]);
	std::printf("]");
}

struct Stat {
	const char	*name;
	long long	 cases;
	long long	 fails;
	int		 shown;
};

static Stat st_kexpl	= { "__k_expl",		0, 0, 0 };
static Stat st_ldcexpl	= { "__ldexp_cexpl",	0, 0, 0 };
static Stat st_cexpl	= { "cexpl",		0, 0, 0 };

static Stat *const all_stats[] = {
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

#define	KBUF_SIZE	96
#define	OFF_HI		0
#define	OFF_LO		16
#define	OFF_K		32
#define	OFF_TAIL	40
#define	GUARD		0x7f

static void
case_kexpl(long double x)
{
	alignas(16) unsigned char pb[KBUF_SIZE];
	alignas(16) unsigned char rb[KBUF_SIZE];
	long double phi, plo, rhi, rlo;
	int pk, rk;
	bool bad = false;

	st_kexpl.cases++;
	std::memset(pb, GUARD, sizeof(pb));
	std::memset(rb, GUARD, sizeof(rb));

	port::__k_expl(x, (long double *)(pb + OFF_HI),
	    (long double *)(pb + OFF_LO), (int *)(pb + OFF_K));
	ref___k_expl(x, (long double *)(rb + OFF_HI),
	    (long double *)(rb + OFF_LO), (int *)(rb + OFF_K));

	std::memcpy(&phi, pb + OFF_HI, sizeof(phi));
	std::memcpy(&plo, pb + OFF_LO, sizeof(plo));
	std::memcpy(&pk, pb + OFF_K, sizeof(pk));
	std::memcpy(&rhi, rb + OFF_HI, sizeof(rhi));
	std::memcpy(&rlo, rb + OFF_LO, sizeof(rlo));
	std::memcpy(&rk, rb + OFF_K, sizeof(rk));

	if (!same_ld(phi, rhi) || !same_ld(plo, rlo) || pk != rk)
		bad = true;
	for (int i = OFF_TAIL; i < KBUF_SIZE; i++)
		if (pb[i] != GUARD || rb[i] != GUARD)
			bad = true;
	for (int i = OFF_K + 4; i < OFF_TAIL; i++)
		if (pb[i] != GUARD || rb[i] != GUARD)
			bad = true;

	if (bad && fail_head(st_kexpl)) {
		show_ld("x=", x);
		show_ld(" port.hi=", phi); show_ld(" ref.hi=", rhi);
		show_ld(" port.lo=", plo); show_ld(" ref.lo=", rlo);
		std::printf(" port.k=%d ref.k=%d\n", pk, rk);
	}
}

static void
case_ldcexpl(long double x, long double y, int expt)
{
	long double complex rz, pr, rr;

	st_ldcexpl.cases++;
	rz = CMPLXL(x, y);
	pr = port::__ldexp_cexpl(rz, expt);
	rr = ref___ldexp_cexpl(rz, expt);
	if ((!same_ld(creall(pr), creall(rr)) ||
	    !same_ld(cimagl(pr), cimagl(rr))) && fail_head(st_ldcexpl)) {
		show_ld("x=", x); show_ld(" y=", y);
		std::printf(" expt=%d", expt);
		show_ld(" port.re=", creall(pr));
		show_ld(" ref.re=", creall(rr));
		show_ld(" port.im=", cimagl(pr));
		show_ld(" ref.im=", cimagl(rr));
		std::printf("\n");
	}
}

static void
case_cexpl(long double x, long double y)
{
	long double complex rz, pr, rr;

	st_cexpl.cases++;
	rz = CMPLXL(x, y);
	pr = port::cexpl(rz);
	rr = ref_cexpl(rz);
	if ((!same_ld(creall(pr), creall(rr)) ||
	    !same_ld(cimagl(pr), cimagl(rr))) && fail_head(st_cexpl)) {
		show_ld("x=", x); show_ld(" y=", y);
		show_ld(" port.re=", creall(pr));
		show_ld(" ref.re=", creall(rr));
		show_ld(" port.im=", cimagl(pr));
		show_ld(" ref.im=", cimagl(rr));
		std::printf("\n");
	}
}

static const long double h_cexp_ovfl =
    2.27892930024498818830197576893019292e+04L;
static const long double h_exp_ovfl =
    1.13565234062941439494919310779707649e+04L;

static const long double cexp_x_vals[] = {
	0.0L, -0.0L,
	0x1p-16493L, -0x1p-16493L,
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
	0x1p-16493L, -0x1p-16493L,
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
		v = (long double)(rng_u64() >> 1) * 0x1p-63L * 100.0L;
		break;
	}
	return ((rng_u64() & 1) ? -v : v);
}

static long double
rand_ld(void)
{
	uint64_t m;
	long double v;
	int e;

	switch (rng_below(8)) {
	case 0:
		return (cexp_x_vals[rng_below(NCEXPX)]);
	case 1:
		v = (long double)(rng_u64() >> 1) * 0x1p-63L;
		break;
	case 2: {
		static const long double thr[] = {
			0.0L, h_exp_ovfl, h_cexp_ovfl,
		};
		int k = (int)rng_below(17) - 8;
		v = thr[rng_below(3)];
		for (; k > 0; k--)
			v = nextafterl(v, (long double)INFINITY);
		for (; k < 0; k++)
			v = nextafterl(v, -(long double)INFINITY);
		break;
	}
	case 3:
		v = h_exp_ovfl +
		    (long double)((int64_t)rng_u64() % 2000) * 0.5L;
		break;
	case 4:
		v = h_cexp_ovfl +
		    (long double)((int64_t)rng_u64() % 2000) * 0.5L;
		break;
	case 5:
		m = rng_u64() | (1ULL << 63);
		e = -70 + (int)rng_below(40);
		v = ldexpl((long double)m, e - 63);
		break;
	case 6:
		m = rng_u64() | (1ULL << 63);
		e = -16380 + (int)rng_below(32760);
		v = ldexpl((long double)m, e - 63);
		break;
	default:
		m = rng_u64() | (1ULL << 63);
		e = 10 + (int)rng_below(20);
		v = ldexpl((long double)m, e - 63);
		break;
	}
	return ((rng_u64() & 1) ? -v : v);
}

#define	RANDOM_ITERS	200000

int
main(void)
{
	int i, j;

	{
		static const long double kx[] = {
			0.0L, -0.0L, 0x1p-16493L, -0x1p-16493L, 1e-40L,
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
		for (i = 0; i < 256; i++)
			case_kexpl((long double)i * 0.0054152123481245727L);
	}

	for (i = 0; i < NCEXPX; i++)
		for (j = 0; j < NCEXPY; j++) {
			case_cexpl(cexp_x_vals[i], cexp_y_vals[j]);
			case_cexpl(nextafterl(cexp_x_vals[i],
			    (long double)INFINITY), cexp_y_vals[j]);
			case_cexpl(nextafterl(cexp_x_vals[i],
			    -(long double)INFINITY), cexp_y_vals[j]);
		}

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

	{
		static const long double nx[] = {
			0.0L, -0.0L, 1.0L, -1.0L, 11400.0L, -11400.0L,
			(long double)INFINITY, -(long double)INFINITY,
			(long double)NAN,
		};
		static const long double ny[] = {
			(long double)INFINITY, -(long double)INFINITY,
			(long double)NAN,
		};
		for (i = 0; i < (int)(sizeof(nx) / sizeof(nx[0])); i++)
			for (j = 0; j < (int)(sizeof(ny) / sizeof(ny[0])); j++)
				case_cexpl(nx[i], ny[j]);
	}

	for (i = 0; i < NCEXPY; i++) {
		case_cexpl(0.0L, cexp_y_vals[i]);
		case_cexpl(-0.0L, cexp_y_vals[i]);
	}
	for (i = 0; i < NCEXPX; i++) {
		case_cexpl(cexp_x_vals[i], 0.0L);
		case_cexpl(cexp_x_vals[i], -0.0L);
	}

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

	rng_seed(0x0088abcd03ULL);
	for (i = 0; i < RANDOM_ITERS; i++)
		case_kexpl(rand_kexpl_arg());

	rng_seed(0x0088123404ULL);
	for (i = 0; i < RANDOM_ITERS; i++)
		case_ldcexpl(rand_kexpl_arg(), rand_y(),
		    (int)rng_below(70000) - 35000);

	rng_seed(0x0088feed05ULL);
	for (i = 0; i < RANDOM_ITERS; i++)
		case_cexpl(rand_ld(), rand_y());

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
