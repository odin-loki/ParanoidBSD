/*
 * Differential test: pbsd port vs. unmodified HardenedBSD C oracle.
 *
 * Batch b0026:
 *   lib/libc/stdbit/stdc_count_ones.c
 *   lib/libc/stdbit/stdc_count_zeros.c
 *
 * None of the functions in this batch write through a pointer or return a
 * pointer, so every case is compared on its full return value.  The uc and us
 * variants are swept exhaustively over their entire domain; the ui, ul and ull
 * variants get hand-written edge cases plus a fixed-seed randomised sweep of
 * 200000 iterations each, drawn from bit patterns chosen so that a one-bit
 * change anywhere in the port's expression is observable.
 */

#include <climits>
#include <cstdio>
#include <cstdlib>

import pbsd.lib.libc.stdbit.b0026;

namespace port = pbsd::lib_libc_stdbit::b0026;

extern "C" {
unsigned int ref_stdc_count_ones_uc(unsigned char);
unsigned int ref_stdc_count_ones_us(unsigned short);
unsigned int ref_stdc_count_ones_ui(unsigned int);
unsigned int ref_stdc_count_ones_ul(unsigned long);
unsigned int ref_stdc_count_ones_ull(unsigned long long);
unsigned int ref_stdc_count_zeros_uc(unsigned char);
unsigned int ref_stdc_count_zeros_us(unsigned short);
unsigned int ref_stdc_count_zeros_ui(unsigned int);
unsigned int ref_stdc_count_zeros_ul(unsigned long);
unsigned int ref_stdc_count_zeros_ull(unsigned long long);
}

namespace {

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
};

Stat stats[] = {
	{ "stdc_count_ones_uc",   0, 0 },
	{ "stdc_count_ones_us",   0, 0 },
	{ "stdc_count_ones_ui",   0, 0 },
	{ "stdc_count_ones_ul",   0, 0 },
	{ "stdc_count_ones_ull",  0, 0 },
	{ "stdc_count_zeros_uc",  0, 0 },
	{ "stdc_count_zeros_us",  0, 0 },
	{ "stdc_count_zeros_ui",  0, 0 },
	{ "stdc_count_zeros_ul",  0, 0 },
	{ "stdc_count_zeros_ull", 0, 0 },
};

enum {
	F_ONES_UC, F_ONES_US, F_ONES_UI, F_ONES_UL, F_ONES_ULL,
	F_ZEROS_UC, F_ZEROS_US, F_ZEROS_UI, F_ZEROS_UL, F_ZEROS_ULL,
	F_COUNT
};

unsigned long long reported[F_COUNT];

void
record(int f, bool ok, const char *what, unsigned long long input,
    unsigned int got, unsigned int want)
{
	stats[f].cases++;
	if (ok)
		return;

	stats[f].fails++;
	if (reported[f] < 8) {
		reported[f]++;
		std::printf("FAIL %-24s [%s] input=0x%016llx port=%u ref=%u\n",
		    stats[f].name, what, input, got, want);
	}
}

void
check_uc(unsigned char x, const char *what)
{
	record(F_ONES_UC,
	    port::stdc_count_ones_uc(x) == ref_stdc_count_ones_uc(x),
	    what, x, port::stdc_count_ones_uc(x), ref_stdc_count_ones_uc(x));
	record(F_ZEROS_UC,
	    port::stdc_count_zeros_uc(x) == ref_stdc_count_zeros_uc(x),
	    what, x, port::stdc_count_zeros_uc(x), ref_stdc_count_zeros_uc(x));
}

void
check_us(unsigned short x, const char *what)
{
	record(F_ONES_US,
	    port::stdc_count_ones_us(x) == ref_stdc_count_ones_us(x),
	    what, x, port::stdc_count_ones_us(x), ref_stdc_count_ones_us(x));
	record(F_ZEROS_US,
	    port::stdc_count_zeros_us(x) == ref_stdc_count_zeros_us(x),
	    what, x, port::stdc_count_zeros_us(x), ref_stdc_count_zeros_us(x));
}

void
check_ui(unsigned int x, const char *what)
{
	record(F_ONES_UI,
	    port::stdc_count_ones_ui(x) == ref_stdc_count_ones_ui(x),
	    what, x, port::stdc_count_ones_ui(x), ref_stdc_count_ones_ui(x));
	record(F_ZEROS_UI,
	    port::stdc_count_zeros_ui(x) == ref_stdc_count_zeros_ui(x),
	    what, x, port::stdc_count_zeros_ui(x), ref_stdc_count_zeros_ui(x));
}

void
check_ul(unsigned long x, const char *what)
{
	record(F_ONES_UL,
	    port::stdc_count_ones_ul(x) == ref_stdc_count_ones_ul(x),
	    what, (unsigned long long)x,
	    port::stdc_count_ones_ul(x), ref_stdc_count_ones_ul(x));
	record(F_ZEROS_UL,
	    port::stdc_count_zeros_ul(x) == ref_stdc_count_zeros_ul(x),
	    what, (unsigned long long)x,
	    port::stdc_count_zeros_ul(x), ref_stdc_count_zeros_ul(x));
}

void
check_ull(unsigned long long x, const char *what)
{
	record(F_ONES_ULL,
	    port::stdc_count_ones_ull(x) == ref_stdc_count_ones_ull(x),
	    what, x, port::stdc_count_ones_ull(x), ref_stdc_count_ones_ull(x));
	record(F_ZEROS_ULL,
	    port::stdc_count_zeros_ull(x) == ref_stdc_count_zeros_ull(x),
	    what, x, port::stdc_count_zeros_ull(x), ref_stdc_count_zeros_ull(x));
}

/* Feed one 64-bit pattern to every width, truncating as needed. */
void
check_all(unsigned long long v, const char *what)
{
	check_uc((unsigned char)v, what);
	check_us((unsigned short)v, what);
	check_ui((unsigned int)v, what);
	check_ul((unsigned long)v, what);
	check_ull(v, what);
}

unsigned long long rng_state;

unsigned long long
next_raw(void)
{
	unsigned long long z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;

	return (z ^ (z >> 31));
}

/*
 * Draw a value from a mixture of distributions.  A uniform draw almost always
 * has a popcount near 32 and almost never has a run boundary in an interesting
 * place, so we deliberately also emit sparse words, dense words, low/high
 * masks, single set bits, single clear bits and byte-aligned patterns.  Every
 * bit position of every operand width is exercised many times in both states.
 */
unsigned long long
next_value(void)
{
	unsigned long long r = next_raw();
	unsigned long long s = next_raw();
	unsigned sh = (unsigned)(s & 63);

	switch ((unsigned)(r % 12)) {
	case 0:
		return (s);
	case 1:
		return (1ULL << sh);
	case 2:
		return (~(1ULL << sh));
	case 3:
		return (sh == 63 ? ~0ULL : (1ULL << (sh + 1)) - 1);
	case 4:
		return (~0ULL << sh);
	case 5:
		return (s & next_raw() & next_raw());
	case 6:
		return (s | next_raw() | next_raw());
	case 7:
		return (0x5555555555555555ULL << (sh & 1));
	case 8:
		return (0x0f0f0f0f0f0f0f0fULL ^ s);
	case 9:
		return (s & 0xffULL);
	case 10:
		return ((s & 0xffffULL) | (1ULL << sh));
	default:
		return (s ^ (s << 1));
	}
}

void
edge_cases(void)
{
	static const unsigned long long fixed[] = {
		0ULL,
		1ULL,
		2ULL,
		3ULL,
		0x7fULL,
		0x80ULL,
		0x81ULL,
		0xfeULL,
		0xffULL,
		0x100ULL,
		0x101ULL,
		0x7fffULL,
		0x8000ULL,
		0xfffeULL,
		0xffffULL,
		0x10000ULL,
		0x7fffffffULL,
		0x80000000ULL,
		0xfffffffeULL,
		0xffffffffULL,
		0x100000000ULL,
		0x7fffffffffffffffULL,
		0x8000000000000000ULL,
		0xfffffffffffffffeULL,
		0xffffffffffffffffULL,
		0x5555555555555555ULL,
		0xaaaaaaaaaaaaaaaaULL,
		0x0f0f0f0f0f0f0f0fULL,
		0xf0f0f0f0f0f0f0f0ULL,
		0x00ff00ff00ff00ffULL,
		0xff00ff00ff00ff00ULL,
		0xdeadbeefcafebabeULL,
		0x0123456789abcdefULL,
	};

	for (unsigned i = 0; i < sizeof(fixed) / sizeof(fixed[0]); i++)
		check_all(fixed[i], "fixed");

	/* every single bit set, and every single bit clear, at every width */
	for (unsigned b = 0; b < 64; b++) {
		check_all(1ULL << b, "one-bit");
		check_all(~(1ULL << b), "one-hole");
		check_all((1ULL << b) - 1, "low-mask");
		check_all(~0ULL << b, "high-mask");
	}

	/* every high-bit byte value 0x80-0xff in the low byte, and splattered */
	for (unsigned v = 0x80; v <= 0xff; v++) {
		check_all((unsigned long long)v, "high-byte");
		check_all((unsigned long long)v * 0x0101010101010101ULL,
		    "byte-splat");
		check_all((unsigned long long)v << 56, "byte-top");
	}

	/* exhaustive over the whole unsigned char domain */
	for (unsigned v = 0; v <= 0xff; v++)
		check_uc((unsigned char)v, "exhaustive-uc");

	/* exhaustive over the whole unsigned short domain */
	for (unsigned v = 0; v <= 0xffff; v++)
		check_us((unsigned short)v, "exhaustive-us");
}

void
random_sweep(void)
{
	rng_state = 0x0026b00126b00126ULL;

	for (unsigned long i = 0; i < 200000UL; i++) {
		unsigned long long v = next_value();

		check_all(v, "random");
	}
}

} /* namespace */

int
main(void)
{
	unsigned long long total_fails = 0;

	edge_cases();
	random_sweep();

	std::printf("\n%-24s %12s %10s %s\n",
	    "function", "cases", "failures", "result");
	std::printf("%-24s %12s %10s %s\n",
	    "------------------------", "------------", "----------",
	    "------");

	for (int i = 0; i < F_COUNT; i++) {
		total_fails += stats[i].fails;
		std::printf("%-24s %12llu %10llu %s\n",
		    stats[i].name, stats[i].cases, stats[i].fails,
		    stats[i].fails == 0 ? "PASS" : "FAIL");
	}

	std::printf("\n%s: %llu failing case(s)\n",
	    total_fails == 0 ? "ALL PASS" : "FAILURE", total_fails);

	return (total_fails == 0 ? 0 : 1);
}
