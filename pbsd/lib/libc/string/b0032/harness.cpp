/*
 * Differential test harness for PBSD batch b0032.
 *
 * Compares the C++23 module port in pbsd.lib.libc.string.b0032 against the
 * unmodified reference implementation in oracle.c.
 */

import pbsd.lib.libc.string.b0032;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

extern "C" int ref_memcmp(const void *s1, const void *s2, std::size_t n);

namespace {

/*
 * Buffer layout.  Every buffer is CAP bytes and is pre-filled with the guard
 * byte 0x7f.  The comparison window handed to memcmp() starts at DOFF, so a
 * port that walks backwards off the start of its argument lands inside the
 * "pre" region, whose contents differ between the two operand buffers.  The
 * region after DOFF + total stays guard-filled in all four buffers.
 */
constexpr std::size_t CAP = 320;
constexpr std::size_t DOFF = 64;
constexpr std::size_t MAXTOTAL = 192;
constexpr unsigned char GUARD = 0x7f;

unsigned char in1[CAP];
unsigned char in2[CAP];
std::size_t g_act; /* number of meaningful bytes in in1/in2 */

unsigned long long g_cases = 0;
unsigned long long g_fails = 0;
int g_shown = 0;

int sgn(int v)
{
	return (v > 0) - (v < 0);
}

void begin_case(std::size_t total)
{
	std::memset(in1, GUARD, CAP);
	std::memset(in2, GUARD, CAP);
	g_act = DOFF + total;
}

/* Give the pre-region contents that differ between the two operands. */
void pre_pattern(unsigned char a, unsigned char b)
{
	for (std::size_t i = 0; i < DOFF; i++) {
		in1[i] = static_cast<unsigned char>(a + i);
		in2[i] = static_cast<unsigned char>(b - i);
	}
}

void show(const char *tag, std::size_t n, int rp, int ro, const char *what)
{
	if (g_shown >= 15)
		return;
	g_shown++;
	std::printf("  FAIL [%s] n=%zu %s: port=%d oracle=%d\n", tag, n, what,
	    rp, ro);
	std::printf("        buf1:");
	for (std::size_t i = 0; i < n && i < 24; i++)
		std::printf(" %02x", in1[DOFF + i]);
	std::printf("\n        buf2:");
	for (std::size_t i = 0; i < n && i < 24; i++)
		std::printf(" %02x", in2[DOFF + i]);
	std::printf("\n");
}

void run_case(const char *tag, std::size_t n)
{
	static unsigned char pa[CAP], pb[CAP], oa[CAP], ob[CAP];
	static unsigned char ka[CAP], kb[CAP];

	std::memset(pa, GUARD, CAP);
	std::memset(pb, GUARD, CAP);
	std::memset(oa, GUARD, CAP);
	std::memset(ob, GUARD, CAP);
	std::memset(ka, GUARD, CAP);
	std::memset(kb, GUARD, CAP);

	std::memcpy(pa, in1, g_act);
	std::memcpy(oa, in1, g_act);
	std::memcpy(ka, in1, g_act);
	std::memcpy(pb, in2, g_act);
	std::memcpy(ob, in2, g_act);
	std::memcpy(kb, in2, g_act);

	int rp = pbsd::lib_libc_string::b0032::memcmp(pa + DOFF, pb + DOFF, n);
	int ro = ref_memcmp(oa + DOFF, ob + DOFF, n);

	g_cases++;

	bool ok = true;
	if (rp != ro) {
		ok = false;
		show(tag, n, rp, ro, "return value");
	} else if (sgn(rp) != sgn(ro)) {
		ok = false;
		show(tag, n, rp, ro, "return sign");
	}
	/* memcmp() must not write; the whole buffer, guards included, matches. */
	if (std::memcmp(pa, oa, CAP) != 0 || std::memcmp(pb, ob, CAP) != 0) {
		if (ok)
			show(tag, n, rp, ro, "buffer divergence");
		ok = false;
	}
	if (std::memcmp(pa, ka, CAP) != 0 || std::memcmp(pb, kb, CAP) != 0 ||
	    std::memcmp(oa, ka, CAP) != 0 || std::memcmp(ob, kb, CAP) != 0) {
		if (ok)
			show(tag, n, rp, ro, "buffer modified");
		ok = false;
	}
	if (!ok)
		g_fails++;
}

/* ------------------------------------------------------------------ */
/* hand written edge cases                                            */
/* ------------------------------------------------------------------ */

struct Pair {
	unsigned char a, b;
};

/*
 * Interesting operand pairs.  Includes equal pairs, both orders of every
 * difference, high-bit bytes (which must be compared as unsigned char), the
 * 0x7f/0x80 signedness boundary, and pairs where a+b != a-b so that swapping
 * the subtraction in the return expression is observable.
 */
const Pair kPairs[] = {
	{ 0x00, 0x00 }, { 0x01, 0x01 }, { 0x7f, 0x7f }, { 0x80, 0x80 },
	{ 0xff, 0xff }, { 0x00, 0x01 }, { 0x01, 0x00 }, { 0x00, 0xff },
	{ 0xff, 0x00 }, { 0x7f, 0x80 }, { 0x80, 0x7f }, { 0x00, 0x80 },
	{ 0x80, 0x00 }, { 0x80, 0xff }, { 0xff, 0x80 }, { 0x01, 0x80 },
	{ 0x80, 0x01 }, { 0x41, 0x61 }, { 0x61, 0x41 }, { 0x05, 0x03 },
	{ 0x03, 0x05 }, { 0xfe, 0xff }, { 0xff, 0xfe }, { 0x7f, 0x00 },
	{ 0x00, 0x7f }, { 0x02, 0x04 }, { 0x04, 0x02 },
};
constexpr std::size_t kNPairs = sizeof(kPairs) / sizeof(kPairs[0]);

const unsigned char kFills[] = { 0x00, 0x01, 0x7f, 0x80, 0xff, 0x41, 0xaa };
constexpr std::size_t kNFills = sizeof(kFills) / sizeof(kFills[0]);

void fill_data(std::size_t total, unsigned char v)
{
	for (std::size_t i = 0; i < total; i++) {
		in1[DOFF + i] = v;
		in2[DOFF + i] = v;
	}
}

void edge_cases()
{
	/* n == 0: the result is 0 no matter what the buffers hold. */
	for (std::size_t p = 0; p < kNPairs; p++) {
		begin_case(8);
		pre_pattern(0x11, 0xee);
		fill_data(8, 0x33);
		for (std::size_t i = 0; i < 8; i++) {
			in1[DOFF + i] = kPairs[p].a;
			in2[DOFF + i] = kPairs[p].b;
		}
		run_case("n0", 0);
	}
	/* n == 0 with zero-length data region as well. */
	begin_case(0);
	pre_pattern(0x11, 0xee);
	run_case("n0-empty", 0);

	/* Single byte, every interesting pair, with and without differing tail. */
	for (std::size_t p = 0; p < kNPairs; p++) {
		begin_case(1);
		pre_pattern(0x11, 0xee);
		in1[DOFF] = kPairs[p].a;
		in2[DOFF] = kPairs[p].b;
		run_case("n1-tight", 1);

		begin_case(4);
		pre_pattern(0x11, 0xee);
		fill_data(4, 0x33);
		in1[DOFF] = kPairs[p].a;
		in2[DOFF] = kPairs[p].b;
		/* bytes past the window differ, over-reads become visible */
		in1[DOFF + 1] = 0x10;
		in2[DOFF + 1] = 0x9c;
		in1[DOFF + 2] = 0xc4;
		in2[DOFF + 2] = 0x07;
		in1[DOFF + 3] = 0x00;
		in2[DOFF + 3] = 0xff;
		run_case("n1-tail", 1);
	}

	/*
	 * For every window length, place the first difference at every possible
	 * offset, and also just past the end of the window.
	 */
	for (std::size_t n = 1; n <= 48; n++) {
		for (std::size_t f = 0; f < kNFills; f++) {
			/* identical operands of length n */
			begin_case(n);
			pre_pattern(0x11, 0xee);
			fill_data(n, kFills[f]);
			run_case("equal-tight", n);

			/* identical window, differing tail: still equal */
			begin_case(n + 4);
			pre_pattern(0x11, 0xee);
			fill_data(n + 4, kFills[f]);
			for (std::size_t t = 0; t < 4; t++) {
				in1[DOFF + n + t] = static_cast<unsigned char>(0x10 + t);
				in2[DOFF + n + t] = static_cast<unsigned char>(0xf0 - t);
			}
			run_case("equal-tail-differs", n);

			for (std::size_t pos = 0; pos < n; pos++) {
				for (std::size_t p = 0; p < kNPairs; p++) {
					if (kPairs[p].a == kPairs[p].b)
						continue;
					begin_case(n + 3);
					pre_pattern(0x11, 0xee);
					fill_data(n + 3, kFills[f]);
					in1[DOFF + pos] = kPairs[p].a;
					in2[DOFF + pos] = kPairs[p].b;
					/*
					 * Trailing bytes differ so that reading
					 * one past the mismatch is observable.
					 */
					in1[DOFF + n] = 0x00;
					in2[DOFF + n] = 0xff;
					in1[DOFF + n + 1] = 0xff;
					in2[DOFF + n + 1] = 0x00;
					in1[DOFF + n + 2] = 0x5a;
					in2[DOFF + n + 2] = 0xa5;
					run_case("diff-at-pos", n);
				}
			}

			/* first difference exactly one byte past the window */
			begin_case(n + 2);
			pre_pattern(0x11, 0xee);
			fill_data(n + 2, kFills[f]);
			in1[DOFF + n] = 0x00;
			in2[DOFF + n] = 0xff;
			in1[DOFF + n + 1] = 0xff;
			in2[DOFF + n + 1] = 0x00;
			run_case("diff-past-window", n);

			/* difference at the very last byte of the window only */
			begin_case(n + 2);
			pre_pattern(0x11, 0xee);
			fill_data(n + 2, kFills[f]);
			in1[DOFF + n - 1] = 0x02;
			in2[DOFF + n - 1] = 0x40;
			run_case("diff-last-byte", n);
		}
	}

	/*
	 * Two differences: the first one must win, and the second must not be
	 * able to change the answer.
	 */
	for (std::size_t n = 2; n <= 32; n++) {
		for (std::size_t i = 0; i + 1 < n; i++) {
			for (std::size_t j = i + 1; j < n; j++) {
				begin_case(n);
				pre_pattern(0x11, 0xee);
				fill_data(n, 0x40);
				in1[DOFF + i] = 0x10;
				in2[DOFF + i] = 0x06;
				in1[DOFF + j] = 0x01;
				in2[DOFF + j] = 0xfe;
				run_case("two-diffs", n);
			}
		}
	}

	/* NUL-heavy data with one non-zero byte on either side. */
	for (std::size_t n = 1; n <= 40; n++) {
		for (std::size_t pos = 0; pos < n; pos++) {
			begin_case(n);
			pre_pattern(0x11, 0xee);
			fill_data(n, 0x00);
			in1[DOFF + pos] = 0x01;
			run_case("nul-hi1", n);

			begin_case(n);
			pre_pattern(0x11, 0xee);
			fill_data(n, 0x00);
			in2[DOFF + pos] = 0x01;
			run_case("nul-hi2", n);

			begin_case(n);
			pre_pattern(0x11, 0xee);
			fill_data(n, 0x00);
			in1[DOFF + pos] = 0x80;
			run_case("nul-hi3", n);

			begin_case(n);
			pre_pattern(0x11, 0xee);
			fill_data(n, 0x00);
			in2[DOFF + pos] = 0xff;
			run_case("nul-hi4", n);
		}
	}

	/* All high-bit data: unsigned char semantics must hold throughout. */
	for (std::size_t n = 1; n <= 40; n++) {
		for (std::size_t pos = 0; pos < n; pos++) {
			begin_case(n);
			pre_pattern(0x11, 0xee);
			for (std::size_t i = 0; i < n; i++) {
				in1[DOFF + i] = static_cast<unsigned char>(0x80 + (i & 0x7f));
				in2[DOFF + i] = static_cast<unsigned char>(0x80 + (i & 0x7f));
			}
			in1[DOFF + pos] = 0xff;
			in2[DOFF + pos] = 0x80;
			run_case("high-a", n);

			begin_case(n);
			pre_pattern(0x11, 0xee);
			for (std::size_t i = 0; i < n; i++) {
				in1[DOFF + i] = static_cast<unsigned char>(0xff - (i & 0x7f));
				in2[DOFF + i] = static_cast<unsigned char>(0xff - (i & 0x7f));
			}
			in1[DOFF + pos] = 0x80;
			in2[DOFF + pos] = 0xff;
			run_case("high-b", n);
		}
	}

	/* Long windows, up to the full data area. */
	for (std::size_t n = MAXTOTAL - 8; n <= MAXTOTAL; n++) {
		begin_case(n);
		pre_pattern(0x11, 0xee);
		for (std::size_t i = 0; i < n; i++) {
			in1[DOFF + i] = static_cast<unsigned char>(i * 7 + 3);
			in2[DOFF + i] = static_cast<unsigned char>(i * 7 + 3);
		}
		run_case("long-equal", n);

		begin_case(n);
		pre_pattern(0x11, 0xee);
		for (std::size_t i = 0; i < n; i++) {
			in1[DOFF + i] = static_cast<unsigned char>(i * 7 + 3);
			in2[DOFF + i] = static_cast<unsigned char>(i * 7 + 3);
		}
		in1[DOFF + n - 1] = 0xfe;
		in2[DOFF + n - 1] = 0x01;
		run_case("long-diff-end", n);

		begin_case(n);
		pre_pattern(0x11, 0xee);
		for (std::size_t i = 0; i < n; i++) {
			in1[DOFF + i] = static_cast<unsigned char>(i * 7 + 3);
			in2[DOFF + i] = static_cast<unsigned char>(i * 7 + 3);
		}
		in1[DOFF] = 0x01;
		in2[DOFF] = 0xfe;
		run_case("long-diff-start", n);
	}
}

/* ------------------------------------------------------------------ */
/* fixed-seed randomised sweep                                        */
/* ------------------------------------------------------------------ */

std::uint64_t rng_state = 0x123456789abcdefULL;

std::uint64_t rnd()
{
	std::uint64_t x = rng_state;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return x;
}

std::uint32_t rnd_below(std::uint32_t m)
{
	return static_cast<std::uint32_t>(rnd() % m);
}

unsigned char alpha_byte(int alphabet)
{
	unsigned char v = static_cast<unsigned char>(rnd() & 0xff);
	switch (alphabet) {
	case 0:
		return v;
	case 1:
		return (v & 1) ? 0xff : 0x00;
	case 2:
		return static_cast<unsigned char>(v & 1);
	case 3:
		return (v & 1) ? 0x80 : 0x7f;
	case 4:
		return static_cast<unsigned char>(v | 0x80);
	case 5:
		return static_cast<unsigned char>(v & 0x7f);
	case 6:
		return 0x00;
	case 7:
		return (v & 1) ? 0x00 : 0x80;
	default:
		return v;
	}
}

void random_sweep(unsigned long iters)
{
	for (unsigned long it = 0; it < iters; it++) {
		int alphabet = static_cast<int>(rnd_below(8));
		std::size_t total = rnd_below(MAXTOTAL + 1);
		std::size_t n;
		std::uint32_t nsel = rnd_below(10);
		if (nsel == 0)
			n = 0;
		else if (nsel <= 5 || total == 0)
			n = total; /* window ends exactly at the data */
		else
			n = rnd_below(static_cast<std::uint32_t>(total));

		begin_case(total);
		for (std::size_t i = 0; i < DOFF; i++) {
			in1[i] = static_cast<unsigned char>(rnd() & 0xff);
			in2[i] = static_cast<unsigned char>(rnd() & 0xff);
		}
		for (std::size_t i = 0; i < total; i++) {
			unsigned char v = alpha_byte(alphabet);
			in1[DOFF + i] = v;
			in2[DOFF + i] = v;
		}

		std::uint32_t mode = rnd_below(100);
		if (mode < 35) {
			/* identical operands */
		} else if (mode < 85 && total > 0) {
			std::uint32_t k = 1 + rnd_below(3);
			for (std::uint32_t c = 0; c < k; c++) {
				std::size_t pos =
				    rnd_below(static_cast<std::uint32_t>(total));
				in2[DOFF + pos] = alpha_byte(alphabet);
			}
		} else if (total > n) {
			/* differ only outside the window */
			std::uint32_t span =
			    static_cast<std::uint32_t>(total - n);
			std::size_t pos = n + rnd_below(span);
			in2[DOFF + pos] = static_cast<unsigned char>(
			    in1[DOFF + pos] ^ 0xff);
		}

		run_case("random", n);
	}
}

} // namespace

int main()
{
	std::printf("PBSD b0032 differential test (memcmp)\n\n");

	edge_cases();
	unsigned long long edge_cases_run = g_cases;
	unsigned long long edge_fails = g_fails;

	random_sweep(250000UL);
	unsigned long long rand_cases = g_cases - edge_cases_run;
	unsigned long long rand_fails = g_fails - edge_fails;

	std::printf("\n%-16s %14s %14s %14s\n", "function", "cases",
	    "random cases", "failures");
	std::printf("%-16s %14s %14s %14s\n", "----------------",
	    "--------------", "--------------", "--------------");
	std::printf("%-16s %14llu %14llu %14llu\n", "memcmp", g_cases,
	    rand_cases, g_fails);
	std::printf("\nedge cases: %llu (%llu failed), random cases: %llu (%llu failed)\n",
	    edge_cases_run, edge_fails, rand_cases, rand_fails);

	if (g_fails != 0) {
		std::printf("\nRESULT: FAIL (%llu of %llu cases diverged)\n",
		    g_fails, g_cases);
		return 1;
	}
	std::printf("\nRESULT: PASS (%llu of %llu cases matched)\n", g_cases,
	    g_cases);
	return 0;
}
