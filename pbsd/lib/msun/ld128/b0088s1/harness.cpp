/*
 * PBSD batch b0088s1 -- differential test of the C++23 port against the
 * unmodified C oracle.
 *
 * cospil() is driven with hand-written edge cases and a fixed-seed randomised
 * sweep.  The two implementations are compared bit for bit: the 10 significant
 * bytes of each 80-bit long double result must be identical.
 */

#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.ld128.b0088s1;

namespace port = pbsd::lib_msun_ld128::b0088s1;

extern "C" {
long double ref_cospil(long double);
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

static Stat st_cospil = { "cospil", 0, 0, 0 };

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
			0.0L, 0x1p-60L, 0.25L, 0.5L, 0.75L, 1.0L,
			2.0L, 0x1p112L, 0x1p113L,
		};
		int k = (int)rng_below(2 * 8 + 1) - 8;
		v = thr[rng_below((uint32_t)(sizeof(thr) / sizeof(thr[0])))];
		for (; k > 0; k--)
			v = nextafterl(v, (long double)INFINITY);
		for (; k < 0; k--)
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

static void
case_cospil(long double x)
{
	long double p, r;

	st_cospil.cases++;
	p = port::cospil(x);
	r = ref_cospil(x);
	if (!same_ld(p, r) && fail_head(st_cospil)) {
		show_ld("x=", x);
		show_ld(" port=", p); show_ld(" ref=", r);
		std::printf("\n");
	}
}

#define	RANDOM_ITERS	200000

int
main(void)
{
	int i;

	for (i = 0; i < NEDGE; i++)
		case_cospil(edge_val(i));

	{
		static const long double tiny[] = {
			0.0L, -0.0L,
			0x1p-16445L, -0x1p-16445L,
			0x1p-100L, -0x1p-100L,
			0x1p-61L, -0x1p-61L,
		};
		for (i = 0; i < (int)(sizeof(tiny) / sizeof(tiny[0])); i++) {
			case_cospil(tiny[i]);
			case_cospil(nextafterl(tiny[i],
			    (long double)INFINITY));
			case_cospil(nextafterl(tiny[i],
			    -(long double)INFINITY));
		}
	}

	{
		long double t = 0x1p-60L;

		for (i = 0; i < 16; i++) {
			case_cospil(t);
			case_cospil(-t);
			t = nextafterl(t, (long double)INFINITY);
		}
		t = 0x1p-60L;
		for (i = 0; i < 16; i++) {
			case_cospil(-t);
			t = nextafterl(t, -(long double)INFINITY);
		}
	}

	{
		static const long double qi[] = {
			0.0L, 0.25L, 0.5L, 0.75L, 1.0L,
		};
		for (i = 0; i < (int)(sizeof(qi) / sizeof(qi[0])); i++) {
			case_cospil(qi[i]);
			case_cospil(-qi[i]);
			case_cospil(nextafterl(qi[i],
			    (long double)INFINITY));
			case_cospil(nextafterl(qi[i],
			    -(long double)INFINITY));
			case_cospil(-nextafterl(qi[i],
			    (long double)INFINITY));
			case_cospil(-nextafterl(qi[i],
			    -(long double)INFINITY));
		}
	}

	{
		static const long double big[] = {
			2.0L, 3.0L, 4.0L, 5.0L, 100.0L, 1000.0L,
			0x1p50L, 0x1p50L + 1.0L, 0x1p50L + 2.0L,
			0x1p62L, 0x1p62L + 1.0L,
			0x1p111L, 0x1p111L + 1.0L,
			0x1p112L, 0x1p112L + 1.0L, 0x1p112L + 2.0L,
			0x1p112L + 0x1p60L,
			0x1p113L, 0x1p113L + 1.0L,
			0x1p114L,
		};
		for (i = 0; i < (int)(sizeof(big) / sizeof(big[0])); i++) {
			case_cospil(big[i]);
			case_cospil(-big[i]);
			case_cospil(big[i] + 0.5L);
			case_cospil(-(big[i] + 0.5L));
			case_cospil(big[i] + 0.25L);
			case_cospil(big[i] + 0.75L);
		}
	}

	{
		long double x;

		x = 0x1p112L;
		for (i = 0; i < 32; i++) {
			case_cospil(x);
			case_cospil(-x);
			x = nextafterl(x, (long double)INFINITY);
		}
		x = 0x1p113L;
		for (i = 0; i < 32; i++) {
			case_cospil(x);
			case_cospil(-x);
			x = nextafterl(x, -(long double)INFINITY);
		}
	}

	case_cospil((long double)INFINITY);
	case_cospil(-(long double)INFINITY);
	case_cospil((long double)NAN);
	case_cospil(-(long double)NAN);

	rng_seed(0x0088b1c0ffeeULL);
	for (i = 0; i < RANDOM_ITERS; i++)
		case_cospil(rand_ld());

	std::printf("\n%-20s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("-------------------------------------------------------\n");
	std::printf("%-20s %12lld %10lld   %s\n", st_cospil.name,
	    st_cospil.cases, st_cospil.fails,
	    st_cospil.fails == 0 ? "ok" : "FAILED");
	std::printf("-------------------------------------------------------\n");
	std::printf("%-20s %12lld %10lld   %s\n", "TOTAL", st_cospil.cases,
	    st_cospil.fails, st_cospil.fails == 0 ? "ok" : "FAILED");

	return (st_cospil.fails == 0 ? 0 : 1);
}
