/*
 * Differential test for PBSD batch b0033 (lib/libc/quad).
 *
 * Every case is run through both the C++23 port and the unmodified C
 * oracle, and the results are compared bit for bit.  None of the four
 * routines in this batch writes through a pointer -- they take quads by
 * value and return a quad or a double -- so instead of an output buffer
 * each argument is passed from inside a 0x7f-filled guard frame, and the
 * entire frame is compared afterwards on both sides.  That catches a port
 * that scribbles outside the halfword window it was given as well as one
 * that merely returns the wrong number.
 *
 * Return values are compared on their object representation (memcmp for
 * quads, raw bit pattern for doubles) so that a -0.0 or a NaN payload
 * difference cannot slip through an == comparison.
 */

#include <cstdio>
#include <cstdint>
#include <cstring>

import pbsd.lib.libc.quad.b0033;

extern "C" long long ref___iordi3(long long a, long long b);
extern "C" long long ref___negdi2(long long a);
extern "C" long long ref___one_cmpldi2(long long a);
extern "C" double ref___floatunsdidf(unsigned long long x);

namespace P = pbsd::lib_libc_quad::b0033;

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

Stat st_ior = { "__iordi3", 0, 0, 0 };
Stat st_neg = { "__negdi2", 0, 0, 0 };
Stat st_not = { "__one_cmpldi2", 0, 0, 0 };
Stat st_flt = { "__floatunsdidf", 0, 0, 0 };

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

u64
dbits(double d)
{
	u64 u;

	std::memcpy(&u, &d, sizeof u);
	return (u);
}

/*
 * A quad argument surrounded by guard bytes.  There is no padding: 24 + 8
 * + 32 == 64 and the alignment of s64 is 8, so the whole object can be
 * memcmp'd.
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
fail(Stat &s, const char *fmt, u64 x, u64 y, u64 got, u64 exp)
{
	s.fails++;
	if (s.reported < kMaxReport) {
		s.reported++;
		std::printf("  %s: %s\n", s.name, "MISMATCH");
		std::printf("    args  = 0x%016llx 0x%016llx\n", x, y);
		std::printf("    port  = 0x%016llx\n", got);
		std::printf("    oracle= 0x%016llx\n", exp);
		(void)fmt;
	}
}

/* ---------------------------------------------------------------- */

void
chk_ior(s64 a, s64 b)
{
	Frame pa, pb, ra, rb;

	st_ior.cases++;
	frame_init(pa, a);
	frame_init(pb, b);
	frame_init(ra, a);
	frame_init(rb, b);

	s64 got = P::__iordi3(pa.val, pb.val);
	s64 exp = ref___iordi3(ra.val, rb.val);

	bool ok = std::memcmp(&got, &exp, sizeof got) == 0 &&
	    std::memcmp(&pa, &ra, sizeof pa) == 0 &&
	    std::memcmp(&pb, &rb, sizeof pb) == 0 &&
	    frame_intact(pa, a) && frame_intact(pb, b) &&
	    frame_intact(ra, a) && frame_intact(rb, b);

	if (!ok)
		fail(st_ior, "iordi3", (u64)a, (u64)b, (u64)got, (u64)exp);
}

void
chk_neg(s64 a)
{
	Frame pa, ra;

	st_neg.cases++;
	frame_init(pa, a);
	frame_init(ra, a);

	s64 got = P::__negdi2(pa.val);
	s64 exp = ref___negdi2(ra.val);

	bool ok = std::memcmp(&got, &exp, sizeof got) == 0 &&
	    std::memcmp(&pa, &ra, sizeof pa) == 0 &&
	    frame_intact(pa, a) && frame_intact(ra, a);

	if (!ok)
		fail(st_neg, "negdi2", (u64)a, 0, (u64)got, (u64)exp);
}

void
chk_not(s64 a)
{
	Frame pa, ra;

	st_not.cases++;
	frame_init(pa, a);
	frame_init(ra, a);

	s64 got = P::__one_cmpldi2(pa.val);
	s64 exp = ref___one_cmpldi2(ra.val);

	bool ok = std::memcmp(&got, &exp, sizeof got) == 0 &&
	    std::memcmp(&pa, &ra, sizeof pa) == 0 &&
	    frame_intact(pa, a) && frame_intact(ra, a);

	if (!ok)
		fail(st_not, "one_cmpldi2", (u64)a, 0, (u64)got, (u64)exp);
}

void
chk_flt(u64 x)
{
	Frame pa, ra;

	st_flt.cases++;
	frame_init(pa, (s64)x);
	frame_init(ra, (s64)x);

	double got = P::__floatunsdidf((u64)pa.val);
	double exp = ref___floatunsdidf((u64)ra.val);

	bool ok = dbits(got) == dbits(exp) &&
	    std::memcmp(&pa, &ra, sizeof pa) == 0 &&
	    frame_intact(pa, (s64)x) && frame_intact(ra, (s64)x);

	if (!ok)
		fail(st_flt, "floatunsdidf", x, 0, dbits(got), dbits(exp));
}

/* ---------------------------------------------------------------- */

/*
 * Halfword patterns.  Each is used as both the high and the low word of a
 * quad, so every combination of interesting high/low pairs is covered --
 * which is what makes an H/L confusion, an index flip or a dropped
 * halfword operation observable.
 *
 * 0 and 0xffffffff are the two values that straddle the `res.ul[L] > 0'
 * test in __negdi2: a low word of 0 negates to 0 (predicate false) and a
 * low word of 0xffffffff negates to 1 (predicate true, and the smallest
 * value for which it is true, so `> 0' can be told apart from `> 1').
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
	 * its complement: the all-NUL quad, a quad that is one lone byte,
	 * high-bit bytes 0x80..0xff, and the all-ones quad.
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

void
edge_cases(void)
{
	for (int i = 0; i < kNVals; i++) {
		chk_neg((s64)kVals[i]);
		chk_not((s64)kVals[i]);
		chk_flt(kVals[i]);
	}
	for (int i = 0; i < kNVals; i++)
		for (int j = 0; j < kNVals; j++)
			chk_ior((s64)kVals[i], (s64)kVals[j]);
}

/*
 * Randomised sweep.  Half the operands are uniform 64-bit noise and half
 * are built out of the halfword pattern table, so the sweep keeps hitting
 * the degenerate halfwords (0, 0xffffffff, 0x80000000) as well as dense
 * ones instead of drowning them at a probability of 2^-32.
 */
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

		chk_ior((s64)a, (s64)b);
		chk_neg((s64)a);
		chk_not((s64)a);
		chk_flt(a);
		chk_flt(b);
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

	std::printf("b0033 differential test: %d edge values, "
	    "%d edge pairs, 250000 random iterations\n",
	    kNVals, kNVals * kNVals);

	edge_cases();
	random_sweep();

	std::printf("\n  %-20s %12s %12s   %s\n", "function", "cases",
	    "failures", "status");
	std::printf("  %-20s %12s %12s   %s\n", "--------------------",
	    "------------", "------------", "------");
	report(st_ior);
	report(st_neg);
	report(st_not);
	report(st_flt);

	u64 total = st_ior.fails + st_neg.fails + st_not.fails + st_flt.fails;
	u64 cases = st_ior.cases + st_neg.cases + st_not.cases + st_flt.cases;

	std::printf("\n  total: %llu cases, %llu failures\n", cases, total);
	if (total != 0) {
		std::printf("  RESULT: FAIL\n");
		return (1);
	}
	std::printf("  RESULT: PASS\n");
	return (0);
}
