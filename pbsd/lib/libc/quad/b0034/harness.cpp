/*
 * Differential test for PBSD batch b0034 (lib/libc/quad).
 *
 * Every case is run through both the C++23 port and the unmodified C
 * oracle, and the results are compared bit for bit.  None of the three
 * routines in this batch writes through a pointer -- they take quads by
 * value and return an int or a quad -- so instead of an output buffer
 * each argument is passed from inside a 0x7f-filled guard frame, and the
 * entire frame is compared afterwards on both sides.
 *
 * Return values are compared on their object representation (memcmp for
 * quads, integer equality for the compare helpers) so that no difference
 * can slip through an == comparison.
 */

#include <cstdio>
#include <cstdint>
#include <cstring>

import pbsd.lib.libc.quad.b0034;

extern "C" int ref___ucmpdi2(unsigned long long a, unsigned long long b);
extern "C" int ref___cmpdi2(long long a, long long b);
extern "C" long long ref___subdi3(long long a, long long b);

namespace P = pbsd::lib_libc_quad::b0034;

namespace {

using u64 = unsigned long long;
using s64 = long long;
using u32 = std::uint32_t;

constexpr int kMaxReport = 8;

struct Stat {
	const char *name;
	u64 cases;
	u64 fails;
	int reported;
};

Stat st_ucmp = { "__ucmpdi2", 0, 0, 0 };
Stat st_cmp = { "__cmpdi2", 0, 0, 0 };
Stat st_sub = { "__subdi3", 0, 0, 0 };

/*
 * Fixed-seed xorshift64*; identical sequence on every run and every host.
 */
u64 rng_state = 0x0123456789abcdefULL;

u64
nextrand(void)
{
	u64 x = rng_state;

	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return (x * 0x2545f4914f6cdd1dULL);
}

/*
 * A quad argument surrounded by guard bytes.
 */
struct Frame {
	unsigned char lead[24];
	s64 val;
	unsigned char trail[32];
};
static_assert(sizeof(Frame) == 64, "Frame must be padding-free");

void
frame_init(Frame &f, s64 v)
{
	std::memset(&f, 0x7f, sizeof f);
	f.val = v;
}

bool
frame_intact(const Frame &f, s64 v)
{
	Frame pristine;

	frame_init(pristine, v);
	return (std::memcmp(&f, &pristine, sizeof f) == 0);
}

void
fail_cmp(Stat &s, u64 a, u64 b, int got, int exp)
{
	s.fails++;
	if (s.reported < kMaxReport) {
		s.reported++;
		std::printf("  %s: MISMATCH\n", s.name);
		std::printf("    args  = 0x%016llx 0x%016llx\n", a, b);
		std::printf("    port  = %d\n", got);
		std::printf("    oracle= %d\n", exp);
	}
}

void
fail_sub(Stat &s, u64 a, u64 b, u64 got, u64 exp)
{
	s.fails++;
	if (s.reported < kMaxReport) {
		s.reported++;
		std::printf("  %s: MISMATCH\n", s.name);
		std::printf("    args  = 0x%016llx 0x%016llx\n", a, b);
		std::printf("    port  = 0x%016llx\n", got);
		std::printf("    oracle= 0x%016llx\n", exp);
	}
}

/* ---------------------------------------------------------------- */

void
chk_ucmp(u64 a, u64 b)
{
	Frame pa, pb, ra, rb;

	st_ucmp.cases++;
	frame_init(pa, (s64)a);
	frame_init(pb, (s64)b);
	frame_init(ra, (s64)a);
	frame_init(rb, (s64)b);

	int got = P::__ucmpdi2((u64)pa.val, (u64)pb.val);
	int exp = ref___ucmpdi2((u64)ra.val, (u64)rb.val);

	bool ok = got == exp &&
	    std::memcmp(&pa, &ra, sizeof pa) == 0 &&
	    std::memcmp(&pb, &rb, sizeof pb) == 0 &&
	    frame_intact(pa, (s64)a) && frame_intact(pb, (s64)b) &&
	    frame_intact(ra, (s64)a) && frame_intact(rb, (s64)b);

	if (!ok)
		fail_cmp(st_ucmp, a, b, got, exp);
}

void
chk_cmp(s64 a, s64 b)
{
	Frame pa, pb, ra, rb;

	st_cmp.cases++;
	frame_init(pa, a);
	frame_init(pb, b);
	frame_init(ra, a);
	frame_init(rb, b);

	int got = P::__cmpdi2(pa.val, pb.val);
	int exp = ref___cmpdi2(ra.val, rb.val);

	bool ok = got == exp &&
	    std::memcmp(&pa, &ra, sizeof pa) == 0 &&
	    std::memcmp(&pb, &rb, sizeof pb) == 0 &&
	    frame_intact(pa, a) && frame_intact(pb, b) &&
	    frame_intact(ra, a) && frame_intact(rb, b);

	if (!ok)
		fail_cmp(st_cmp, (u64)a, (u64)b, got, exp);
}

void
chk_sub(s64 a, s64 b)
{
	Frame pa, pb, ra, rb;

	st_sub.cases++;
	frame_init(pa, a);
	frame_init(pb, b);
	frame_init(ra, a);
	frame_init(rb, b);

	s64 got = P::__subdi3(pa.val, pb.val);
	s64 exp = ref___subdi3(ra.val, rb.val);

	bool ok = std::memcmp(&got, &exp, sizeof got) == 0 &&
	    std::memcmp(&pa, &ra, sizeof pa) == 0 &&
	    std::memcmp(&pb, &rb, sizeof pb) == 0 &&
	    frame_intact(pa, a) && frame_intact(pb, b) &&
	    frame_intact(ra, a) && frame_intact(rb, b);

	if (!ok)
		fail_sub(st_sub, (u64)a, (u64)b, (u64)got, (u64)exp);
}

/* ---------------------------------------------------------------- */

/*
 * Halfword patterns.  Each is used as both the high and the low word of a
 * quad, so every combination of interesting high/low pairs is covered.
 *
 * For __cmpdi2 the high word is compared signed (sl[H]); 0x80000000 and
 * 0x7fffffff straddle that boundary.  For __subdi3 the borrow predicate
 * (diff.ul[L] > aa.ul[L]) is true exactly when aa.ul[L] < bb.ul[L], so
 * pairs where the low halves differ are essential.
 */
const u32 kWords[] = {
	0x00000000u, 0x00000001u, 0x00000002u, 0x00000003u,
	0x0000007fu, 0x00000080u, 0x000000ffu, 0x00007fffu,
	0x00008000u, 0x0000ffffu, 0x7ffffffeu, 0x7fffffffu,
	0x80000000u, 0x80000001u, 0xfffffffeu, 0xffffffffu,
	0x55555555u, 0xaaaaaaaau, 0x7f7f7f7fu, 0x80808080u,
	0xff00ff00u, 0x00ff00ffu, 0x01010101u, 0xfefefefeu,
	0xdeadbeefu, 0x12345678u,
};
const int kNWords = (int)(sizeof kWords / sizeof kWords[0]);

u64 kVals[1024];
int kNVals = 0;

void
push(u64 v)
{
	if (kNVals < (int)(sizeof kVals / sizeof kVals[0]))
		kVals[kNVals++] = v;
}

void
build_vals(void)
{
	/* Every high/low halfword combination. */
	for (int i = 0; i < kNWords; i++)
		for (int j = 0; j < kNWords; j++)
			push(((u64)kWords[i] << 32) | (u64)kWords[j]);

	/*
	 * Single interesting byte at each of the eight byte positions, and
	 * its complement.
	 */
	static const unsigned kBytes[] = {
		0x00u, 0x01u, 0x02u, 0x7fu, 0x80u, 0xfeu, 0xffu,
	};
	for (int p = 0; p < 8; p++) {
		for (unsigned b : kBytes) {
			u64 v = (u64)b << (8 * p);
			push(v);
			push(~v);
		}
	}
}

/*
 * Hand-crafted pairs that force each branch of the compare ternaries and
 * the subdi3 borrow path independently of the halfword grid.
 */
void
targeted_pairs(void)
{
	/* __ucmpdi2: high-word <, >, == then low-word <, >, == */
	chk_ucmp(0ULL, 1ULL << 32);			/* low < */
	chk_ucmp(1ULL << 32, 0ULL);			/* low > */
	chk_ucmp(0x12345678ULL, 0x12345678ULL);		/* equal */
	chk_ucmp(0ULL, 1ULL << 63);			/* high < (unsigned) */
	chk_ucmp(1ULL << 63, 0ULL);			/* high > (unsigned) */
	chk_ucmp(1ULL << 63, (1ULL << 63) | 1ULL);	/* high ==, low < */
	chk_ucmp((1ULL << 63) | 2ULL, (1ULL << 63) | 1ULL); /* high ==, low > */
	chk_ucmp(0xffffffffffffffffULL, 0xfffffffffffffffeULL);

	/* __cmpdi2: signed high compare, unsigned low compare */
	chk_cmp(-1, 0);					/* sl[H] < */
	chk_cmp(0, -1);					/* sl[H] > */
	chk_cmp(0, 0);					/* equal */
	chk_cmp((s64)0x8000000000000000ULL,
	    (s64)0x7fffffffffffffffULL);		/* sl[H] < at sign boundary */
	chk_cmp((s64)0x7fffffffffffffffULL,
	    (s64)0x8000000000000000ULL);		/* sl[H] > at sign boundary */
	chk_cmp((s64)((1ULL << 63) | 1ULL),
	    (s64)(1ULL << 63));			/* high ==, low < */
	chk_cmp((s64)((1ULL << 63) | 2ULL),
	    (s64)((1ULL << 63) | 1ULL));		/* high ==, low > */
	chk_cmp((s64)0xffffffffffffffffULL,
	    (s64)0xfffffffffffffffeULL);

	/* __subdi3: borrow and no-borrow across low and high halves */
	chk_sub(0, 0);
	chk_sub(1, 1);
	chk_sub(5, 3);					/* no borrow */
	chk_sub(0, 1);					/* borrow, both halves 0 */
	chk_sub((s64)0x100000000ULL, 1);		/* borrow propagates */
	chk_sub((s64)0xffffffffULL, (s64)0xfffffffeULL);
	chk_sub((s64)0x8000000000000000ULL,
	    (s64)0x7fffffffffffffffULL);
	chk_sub((s64)0xffffffffffffffffULL, 0);
	chk_sub(0, (s64)0xffffffffffffffffULL);
}

void
edge_cases(void)
{
	targeted_pairs();

	for (int i = 0; i < kNVals; i++)
		for (int j = 0; j < kNVals; j++) {
			chk_ucmp(kVals[i], kVals[j]);
			chk_cmp((s64)kVals[i], (s64)kVals[j]);
			chk_sub((s64)kVals[i], (s64)kVals[j]);
		}
}

u64
randval(void)
{
	u64 r = nextrand();

	switch (r & 3) {
	case 0:
		return (nextrand());
	case 1:
		return (((u64)kWords[nextrand() % kNWords] << 32) |
		    (u64)(u32)nextrand());
	case 2:
		return (((u64)(u32)nextrand() << 32) |
		    (u64)kWords[nextrand() % kNWords]);
	default:
		return (((u64)kWords[nextrand() % kNWords] << 32) |
		    (u64)kWords[nextrand() % kNWords]);
	}
}

void
random_sweep(void)
{
	const long iters = 250000;

	for (long i = 0; i < iters; i++) {
		u64 a = randval();
		u64 b = randval();

		chk_ucmp(a, b);
		chk_cmp((s64)a, (s64)b);
		chk_sub((s64)a, (s64)b);
	}
}

void
report(const Stat &s)
{
	std::printf("  %-20s %12llu %12llu   %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "ok" : "FAIL");
}

} /* namespace */

int
main(void)
{
	build_vals();

	std::printf("b0034 differential test: %d edge values, "
	    "%d edge pairs, 250000 random iterations\n",
	    kNVals, kNVals * kNVals);

	edge_cases();
	random_sweep();

	std::printf("\n  %-20s %12s %12s   %s\n", "function", "cases",
	    "failures", "status");
	std::printf("  %-20s %12s %12s   %s\n", "--------------------",
	    "------------", "------------", "------");
	report(st_ucmp);
	report(st_cmp);
	report(st_sub);

	u64 total = st_ucmp.fails + st_cmp.fails + st_sub.fails;
	u64 cases = st_ucmp.cases + st_cmp.cases + st_sub.cases;

	std::printf("\n  total: %llu cases, %llu failures\n", cases, total);
	if (total != 0) {
		std::printf("  RESULT: FAIL\n");
		return (1);
	}
	std::printf("  RESULT: PASS\n");
	return (0);
}
