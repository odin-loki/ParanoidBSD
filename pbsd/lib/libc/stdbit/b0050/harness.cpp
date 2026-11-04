/*
 * Differential test for batch b0050: port vs. ref_ oracle.
 *
 * Every function in this batch is a pure value->value bit primitive: none
 * writes through a pointer and none returns a pointer, so the buffer-guard
 * and pointer-offset comparisons do not apply here.  Coverage instead comes
 * from exhausting the entire domain of the unsigned char and unsigned short
 * entry points, an explicit edge table that straddles both sides of every
 * boundary the 32/64-bit entry points test, and a fixed-seed randomised
 * sweep.
 */

#include <climits>
#include <cstdio>

import pbsd.lib.libc.stdbit.b0050;

namespace P = pbsd::lib_libc_stdbit::b0050;

extern "C" {
unsigned int ref_stdc_leading_zeros_uc(unsigned char);
unsigned int ref_stdc_leading_zeros_us(unsigned short);
unsigned int ref_stdc_leading_zeros_ui(unsigned int);
unsigned int ref_stdc_leading_zeros_ul(unsigned long);
unsigned int ref_stdc_leading_zeros_ull(unsigned long long);
unsigned char ref_stdc_bit_ceil_uc(unsigned char);
unsigned short ref_stdc_bit_ceil_us(unsigned short);
unsigned int ref_stdc_bit_ceil_ui(unsigned int);
unsigned long ref_stdc_bit_ceil_ul(unsigned long);
unsigned long long ref_stdc_bit_ceil_ull(unsigned long long);
}

enum {
	I_LZ_UC, I_LZ_US, I_LZ_UI, I_LZ_UL, I_LZ_ULL,
	I_BC_UC, I_BC_US, I_BC_UI, I_BC_UL, I_BC_ULL,
	NFUNCS
};

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
};

static Stat stats[NFUNCS] = {
	{ "stdc_leading_zeros_uc",  0, 0 },
	{ "stdc_leading_zeros_us",  0, 0 },
	{ "stdc_leading_zeros_ui",  0, 0 },
	{ "stdc_leading_zeros_ul",  0, 0 },
	{ "stdc_leading_zeros_ull", 0, 0 },
	{ "stdc_bit_ceil_uc",       0, 0 },
	{ "stdc_bit_ceil_us",       0, 0 },
	{ "stdc_bit_ceil_ui",       0, 0 },
	{ "stdc_bit_ceil_ul",       0, 0 },
	{ "stdc_bit_ceil_ull",      0, 0 },
};

template <typename R>
static void
chk(int idx, unsigned long long in, R got, R want)
{
	Stat &s = stats[idx];

	s.cases++;
	if (got != want) {
		s.fails++;
		if (s.fails <= 5)
			printf("  FAIL %s(0x%llx): port=0x%llx ref=0x%llx\n",
			    s.name, in, (unsigned long long)got,
			    (unsigned long long)want);
	}
}

#define RUN(idx, fn, type, val)						\
	do {								\
		type v_ = (type)(val);					\
		chk(idx, (unsigned long long)v_, P::fn(v_), ref_##fn(v_)); \
	} while (0)

static void
t_uc(unsigned long long v)
{
	RUN(I_LZ_UC, stdc_leading_zeros_uc, unsigned char, v);
	RUN(I_BC_UC, stdc_bit_ceil_uc, unsigned char, v);
}

static void
t_us(unsigned long long v)
{
	RUN(I_LZ_US, stdc_leading_zeros_us, unsigned short, v);
	RUN(I_BC_US, stdc_bit_ceil_us, unsigned short, v);
}

static void
t_ui(unsigned long long v)
{
	RUN(I_LZ_UI, stdc_leading_zeros_ui, unsigned int, v);
	RUN(I_BC_UI, stdc_bit_ceil_ui, unsigned int, v);
}

static void
t_ul(unsigned long long v)
{
	RUN(I_LZ_UL, stdc_leading_zeros_ul, unsigned long, v);
	RUN(I_BC_UL, stdc_bit_ceil_ul, unsigned long, v);
}

static void
t_ull(unsigned long long v)
{
	RUN(I_LZ_ULL, stdc_leading_zeros_ull, unsigned long long, v);
	RUN(I_BC_ULL, stdc_bit_ceil_ull, unsigned long long, v);
}

static void
t_all(unsigned long long v)
{
	t_uc(v);
	t_us(v);
	t_ui(v);
	t_ul(v);
	t_ull(v);
}

/* splitmix64, fixed seed: the sweep is bit-for-bit reproducible. */
static unsigned long long rstate = 0x0123456789ABCDEFULL;

static unsigned long long
rnd(void)
{
	unsigned long long z;

	rstate += 0x9E3779B97F4A7C15ULL;
	z = rstate;
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;

	return (z ^ (z >> 31));
}

/*
 * Bias the sweep towards exact powers of two and their immediate
 * neighbours: those are the values at which a flipped comparison or an
 * off-by-one in a shift count changes the result.
 */
static unsigned long long
gen(void)
{
	unsigned long long r = rnd();
	int w = (int)(rnd() % 65);
	unsigned long long v = (w == 0) ? 0ULL : (r >> (64 - w));

	switch (rnd() % 12) {
	case 0:
		v = 1ULL << (rnd() % 64);
		break;
	case 1:
		v = (1ULL << (rnd() % 64)) - 1ULL;
		break;
	case 2:
		v = (1ULL << (rnd() % 64)) + 1ULL;
		break;
	case 3:
		v = v + 1ULL;
		break;
	case 4:
		v = v - 1ULL;
		break;
	case 5:
		v = ~v;
		break;
	case 6:
		v = (1ULL << 31) + (rnd() % 5) - 2ULL;
		break;
	case 7:
		v = (1ULL << 63) + (rnd() % 5) - 2ULL;
		break;
	case 8:
		v = (1ULL << 15) + (rnd() % 5) - 2ULL;
		break;
	case 9:
		v = (1ULL << 7) + (rnd() % 5) - 2ULL;
		break;
	default:
		break;
	}

	return (v);
}

static unsigned long long edges[768];
static int nedges;

static void
add_edge(unsigned long long v)
{
	if (nedges < (int)(sizeof(edges) / sizeof(edges[0])))
		edges[nedges++] = v;
}

static void
build_edges(void)
{
	int i;

	for (i = 0; i <= 20; i++)
		add_edge((unsigned long long)i);

	for (i = 0; i < 64; i++) {
		unsigned long long b = 1ULL << i;

		add_edge(b - 2ULL);
		add_edge(b - 1ULL);
		add_edge(b);
		add_edge(b + 1ULL);
		add_edge(b + 2ULL);
		add_edge(~b);
	}

	add_edge(ULLONG_MAX);
	add_edge(ULLONG_MAX - 1ULL);
	add_edge(ULLONG_MAX / 2);
	add_edge(ULLONG_MAX / 2 - 1ULL);
	add_edge(ULLONG_MAX / 2 + 1ULL);
	add_edge(ULLONG_MAX / 2 + 2ULL);
	add_edge(ULONG_MAX);
	add_edge(ULONG_MAX / 2);
	add_edge(ULONG_MAX / 2 + 1ULL);
	add_edge(ULONG_MAX / 2 + 2ULL);
	add_edge(UINT_MAX);
	add_edge(UINT_MAX - 1U);
	add_edge(UINT_MAX / 2);
	add_edge(UINT_MAX / 2 - 1U);
	add_edge(UINT_MAX / 2 + 1U);
	add_edge(UINT_MAX / 2 + 2U);
	add_edge(USHRT_MAX);
	add_edge(USHRT_MAX / 2 + 1U);
	add_edge(UCHAR_MAX);
	add_edge(UCHAR_MAX / 2 + 1U);
	add_edge(0xAAAAAAAAAAAAAAAAULL);
	add_edge(0x5555555555555555ULL);
	add_edge(0x8000000000000000ULL);
	add_edge(0x7FFFFFFFFFFFFFFFULL);
	add_edge(0x00000000FFFFFFFFULL);
	add_edge(0x0000000100000000ULL);
	add_edge(0x80000000FFFFFFFFULL);
	add_edge(0xFF00FF00FF00FF00ULL);
	add_edge(0x00FF00FF00FF00FFULL);
}

int
main(void)
{
	const long ITERS = 250000;
	unsigned long long total_cases = 0, total_fails = 0;
	long it;
	int i;

	/* Exhaustive over the whole unsigned char domain. */
	for (i = 0; i <= UCHAR_MAX; i++)
		t_uc((unsigned long long)i);

	/* Exhaustive over the whole unsigned short domain. */
	for (i = 0; i <= USHRT_MAX; i++)
		t_us((unsigned long long)i);

	build_edges();
	for (i = 0; i < nedges; i++)
		t_all(edges[i]);

	for (it = 0; it < ITERS; it++)
		t_all(gen());

	printf("%-26s %12s %10s %s\n", "function", "cases", "failures",
	    "status");
	for (i = 0; i < NFUNCS; i++) {
		printf("%-26s %12llu %10llu %s\n", stats[i].name,
		    stats[i].cases, stats[i].fails,
		    stats[i].fails == 0 ? "PASS" : "FAIL");
		total_cases += stats[i].cases;
		total_fails += stats[i].fails;
	}
	printf("%-26s %12llu %10llu %s\n", "TOTAL", total_cases, total_fails,
	    total_fails == 0 ? "PASS" : "FAIL");

	return (total_fails == 0 ? 0 : 1);
}
