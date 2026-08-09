/*
 * PBSD batch b0088s4 -- differential test of s_tanpil.c against the oracle.
 */

#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.ld128.b0088s4;

namespace port = pbsd::lib_msun_ld128::b0088s4;

extern "C" {
long double ref___kernel_tanpil(long double);
long double ref_tanpil(long double);
}

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

static Stat st_ktanpil = { "__kernel_tanpil", 0, 0, 0 };
static Stat st_tanpil = { "tanpil", 0, 0, 0 };

static Stat *const all_stats[] = {
	&st_ktanpil,
	&st_tanpil,
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

static const long double base_vals[] = {
	0.0L,
	0x1p-16445L,
	0x1p-16400L,
	LDBL_MIN,
	0x1p-64L,
	0x1p-61L,
	0x1p-60L,
	0x1p-59L,
	0x1p-30L,
	0.1L, 0.2L,
	0.25L,
	0.3L,
	1.0L / 3.0L,
	0.4L,
	0.5L,
	0.6L,
	2.0L / 3.0L,
	0.7L,
	0.75L,
	0.8L, 0.9L, 0.99L, 0.999L,
	0.67433L, 0.67434L, 0.67435L,
	1.0L,
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
	0x1p112L,
	0x1p112L + 0x1p60L,
	0x1p113L,
	0x1p114L,
	LDBL_MAX,
	(long double)INFINITY,
	(long double)NAN,
};

#define	NBASE		((int)(sizeof(base_vals) / sizeof(base_vals[0])))
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
		v = (long double)(rng_u64() >> 1) * 0x1p-63L;
		break;
	case 2: {
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
		static const long double fr[] = {
			0.0L, 0.125L, 0.25L, 0.375L, 0.5L, 0.625L, 0.75L,
			0.875L,
		};
		uint64_t n = rng_u64() >> (14 + rng_below(38));
		v = (long double)n + fr[rng_below(8)];
		break;
	}
	case 4:
		v = (long double)(rng_u64() >> (12 + rng_below(40))) +
		    (long double)(rng_u64() >> 1) * 0x1p-63L;
		break;
	case 5:
		m = rng_u64() | (1ULL << 63);
		e = -70 + (int)rng_below(20);
		v = ldexpl((long double)m, e - 63);
		break;
	case 6:
		m = rng_u64() | (1ULL << 63);
		e = 100 + (int)rng_below(30);
		v = ldexpl((long double)m, e - 63);
		break;
	case 7:
		m = rng_u64() | (1ULL << 63);
		e = -20 + (int)rng_below(60);
		v = ldexpl((long double)m, e - 63);
		break;
	case 8:
		m = rng_u64() | (1ULL << 63);
		e = -16380 + (int)rng_below(32760);
		v = ldexpl((long double)m, e - 63);
		break;
	default:
		m = rng_u64();
		v = (long double)m * 0x1p-62L;
		break;
	}
	return ((rng_u64() & 1) ? -v : v);
}

static long double
rand_kernel(void)
{
	long double v;

	switch (rng_below(6)) {
	case 0:
		v = (long double)(rng_u64() >> 1) * 0x1p-63L * 0.5L;
		break;
	case 1:
		v = 0.25L + (long double)(rng_u64() >> 1) * 0x1p-63L * 0.25L;
		break;
	case 2:
		v = nextafterl(0.25L, (rng_u64() & 1) ?
		    (long double)INFINITY : -(long double)INFINITY);
		break;
	case 3:
		v = ldexpl((long double)(rng_u64() | (1ULL << 63)),
		    -63 - (int)rng_below(80));
		break;
	case 4:
		v = edge_val((int)rng_below(NEDGE));
		if (fabsl(v) > 0.5L)
			v = fabsl(v) - floorl(fabsl(v));
		break;
	default:
		v = (long double)(rng_u64() >> 1) * 0x1p-63L;
		break;
	}
	return ((rng_u64() & 1) ? -v : v);
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

DEFINE_UNARY_CASE(__kernel_tanpil, st_ktanpil)
DEFINE_UNARY_CASE(tanpil, st_tanpil)

#define	RANDOM_ITERS	200000

int
main(void)
{
	int i;

	for (i = 0; i < NEDGE; i++) {
		long double x = edge_val(i);

		case___kernel_tanpil(x);
		case_tanpil(x);
	}

	/* Extra hand-written boundary probes for tanpil branches. */
	{
		static const long double extra[] = {
			-0.0L,
			nextafterl(0.0L, 1.0L),
			nextafterl(0.0L, -1.0L),
			nextafterl(0x1p-60L, 0.0L),
			nextafterl(0x1p-60L, 1.0L),
			nextafterl(0.5L, 0.0L),
			nextafterl(0.5L, 1.0L),
			nextafterl(1.0L, 0.0L),
			nextafterl(1.0L, 2.0L),
			nextafterl(0x1p112L, 0.0L),
			nextafterl(0x1p112L, 2.0L),
			nextafterl(0x1p113L, 0.0L),
			-(long double)INFINITY,
			-__builtin_nanl(""),
		};
		for (i = 0; i < (int)(sizeof(extra) / sizeof(extra[0])); i++) {
			case___kernel_tanpil(extra[i]);
			case_tanpil(extra[i]);
		}
	}

	/* Integer + quarter-fraction arguments for the FFLOORL128 path. */
	for (i = 0; i < 64; i++) {
		long double n = (long double)i;
		static const long double fr[] = {
			0.0L, 0.25L, 0.5L, 0.75L,
		};
		for (int j = 0; j < 4; j++) {
			case_tanpil(n + fr[j]);
			case_tanpil(-(n + fr[j]));
		}
	}

	rng_seed(0x00884c0ffeeULL);
	for (i = 0; i < RANDOM_ITERS; i++)
		case_tanpil(rand_ld());

	rng_seed(0x00884beef01ULL);
	for (i = 0; i < RANDOM_ITERS; i++)
		case___kernel_tanpil(rand_kernel());

	long long total_cases = 0, total_fails = 0;

	std::printf("\n%-20s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("-------------------------------------------------------\n");
	for (i = 0; i < (int)(sizeof(all_stats) / sizeof(all_stats[0])); i++) {
		Stat *s = all_stats[i];

		total_cases += s->cases;
		total_fails += s->fails;
		std::printf("%-20s %12lld %10lld   %s\n", s->name, s->cases,
		    s->fails, s->fails == 0 ? "ok" : "FAILED");
	}
	std::printf("-------------------------------------------------------\n");
	std::printf("%-20s %12lld %10lld   %s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0 ? "ok" : "FAILED");

	return (total_fails == 0 ? 0 : 1);
}
