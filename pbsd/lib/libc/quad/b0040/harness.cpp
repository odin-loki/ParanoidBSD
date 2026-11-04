/*
 * Differential test for PBSD batch b0040 (lib/libc/quad).
 *
 * Every case is run through both the C++23 port and the unmodified C
 * oracle, and the results are compared bit for bit.  None of the four
 * routines in this batch writes through a pointer -- they take quads by
 * value and return a quad or a double -- so instead of an output buffer
 * each argument is passed from inside a 0x7f-filled guard frame, and the
 * entire frame is compared afterwards on both sides.
 *
 * Return values are compared on their object representation (memcmp for
 * quads, raw bit pattern for doubles) so that a -0.0 or a NaN payload
 * difference cannot slip through an == comparison.
 */

#include <cstdio>
#include <cstdint>
#include <cstring>

import pbsd.lib.libc.quad.b0040;

extern "C" long long ref___adddi3(long long a, long long b);
extern "C" long long ref___ashldi3(long long a, unsigned int shift);
extern "C" long long ref___lshldi3(long long a, unsigned int shift);
extern "C" double ref___floatdidf(long long x);

namespace P = pbsd::lib_libc_quad::b0040;

namespace {

using u64 = unsigned long long;
using s64 = long long;
using u32 = std::uint32_t;

constexpr int kMaxReport = 8;
constexpr int LONG_BITS = 32;
constexpr int QUAD_BITS = 64;

struct Stat {
	const char *name;
	u64 cases;
	u64 fails;
	int reported;
};

Stat st_add = { "__adddi3", 0, 0, 0 };
Stat st_ashl = { "__ashldi3", 0, 0, 0 };
Stat st_lshl = { "__lshldi3", 0, 0, 0 };
Stat st_flt = { "__floatdidf", 0, 0, 0 };

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

struct Frame {
	unsigned char lead[24];
	s64 val;
	unsigned char trail[32];
};
static_assert(sizeof(Frame) == 64, "Frame must be padding-free");

struct ShiftFrame {
	unsigned char lead[28];
	unsigned int shift;
	unsigned char trail[32];
};
static_assert(sizeof(ShiftFrame) == 64, "ShiftFrame must be padding-free");

void
frame_init(Frame &f, s64 v)
{
	std::memset(&f, 0x7f, sizeof f);
	f.val = v;
}

void
shift_init(ShiftFrame &f, unsigned int s)
{
	std::memset(&f, 0x7f, sizeof f);
	f.shift = s;
}

bool
frame_intact(const Frame &f, s64 v)
{
	Frame pristine;

	frame_init(pristine, v);
	return (std::memcmp(&f, &pristine, sizeof f) == 0);
}

bool
shift_intact(const ShiftFrame &f, unsigned int s)
{
	ShiftFrame pristine;

	shift_init(pristine, s);
	return (std::memcmp(&f, &pristine, sizeof f) == 0);
}

void
fail_quad(Stat &s, u64 a, u64 b, u64 got, u64 exp)
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

void
fail_flt(Stat &s, u64 x, u64 got, u64 exp)
{
	s.fails++;
	if (s.reported < kMaxReport) {
		s.reported++;
		std::printf("  %s: MISMATCH\n", s.name);
		std::printf("    arg   = 0x%016llx\n", x);
		std::printf("    port  = 0x%016llx\n", got);
		std::printf("    oracle= 0x%016llx\n", exp);
	}
}

void
chk_add(s64 a, s64 b)
{
	Frame pa, pb, ra, rb;

	st_add.cases++;
	frame_init(pa, a);
	frame_init(pb, b);
	frame_init(ra, a);
	frame_init(rb, b);

	s64 got = P::__adddi3(pa.val, pb.val);
	s64 exp = ref___adddi3(ra.val, rb.val);

	bool ok = std::memcmp(&got, &exp, sizeof got) == 0 &&
	    std::memcmp(&pa, &ra, sizeof pa) == 0 &&
	    std::memcmp(&pb, &rb, sizeof pb) == 0 &&
	    frame_intact(pa, a) && frame_intact(pb, b) &&
	    frame_intact(ra, a) && frame_intact(rb, b);

	if (!ok)
		fail_quad(st_add, (u64)a, (u64)b, (u64)got, (u64)exp);
}

void
chk_ashl(s64 a, unsigned int shift)
{
	Frame pa, ra;
	ShiftFrame ps, rs;

	st_ashl.cases++;
	frame_init(pa, a);
	frame_init(ra, a);
	shift_init(ps, shift);
	shift_init(rs, shift);

	s64 got = P::__ashldi3(pa.val, ps.shift);
	s64 exp = ref___ashldi3(ra.val, rs.shift);

	bool ok = std::memcmp(&got, &exp, sizeof got) == 0 &&
	    std::memcmp(&pa, &ra, sizeof pa) == 0 &&
	    std::memcmp(&ps, &rs, sizeof ps) == 0 &&
	    frame_intact(pa, a) && frame_intact(ra, a) &&
	    shift_intact(ps, shift) && shift_intact(rs, shift);

	if (!ok)
		fail_quad(st_ashl, (u64)a, (u64)shift, (u64)got, (u64)exp);
}

void
chk_lshl(s64 a, unsigned int shift)
{
	Frame pa, ra;
	ShiftFrame ps, rs;

	st_lshl.cases++;
	frame_init(pa, a);
	frame_init(ra, a);
	shift_init(ps, shift);
	shift_init(rs, shift);

	s64 got = P::__lshldi3(pa.val, ps.shift);
	s64 exp = ref___lshldi3(ra.val, rs.shift);

	bool ok = std::memcmp(&got, &exp, sizeof got) == 0 &&
	    std::memcmp(&pa, &ra, sizeof pa) == 0 &&
	    std::memcmp(&ps, &rs, sizeof ps) == 0 &&
	    frame_intact(pa, a) && frame_intact(ra, a) &&
	    shift_intact(ps, shift) && shift_intact(rs, shift);

	if (!ok)
		fail_quad(st_lshl, (u64)a, (u64)shift, (u64)got, (u64)exp);
}

void
chk_flt(s64 x)
{
	Frame pa, ra;

	st_flt.cases++;
	frame_init(pa, x);
	frame_init(ra, x);

	double got = P::__floatdidf(pa.val);
	double exp = ref___floatdidf(ra.val);

	bool ok = dbits(got) == dbits(exp) &&
	    std::memcmp(&pa, &ra, sizeof pa) == 0 &&
	    frame_intact(pa, x) && frame_intact(ra, x);

	if (!ok)
		fail_flt(st_flt, (u64)x, dbits(got), dbits(exp));
}

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

unsigned int kShifts[128];
int kNShifts = 0;

void
push(u64 v)
{
	if (kNVals < (int)(sizeof kVals / sizeof kVals[0]))
		kVals[kNVals++] = v;
}

void
push_shift(unsigned int s)
{
	if (kNShifts < (int)(sizeof kShifts / sizeof kShifts[0]))
		kShifts[kNShifts++] = s;
}

void
build_vals(void)
{
	for (int i = 0; i < kNWords; i++)
		for (int j = 0; j < kNWords; j++)
			push(((u64)kWords[i] << 32) | (u64)kWords[j]);

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
build_shifts(void)
{
	/*
	 * Every boundary of the shift helpers:
	 *   shift == 0            (neither branch)
	 *   shift == 1, 31        (0 < shift < LONG_BITS)
	 *   shift == 32, 33, 63  (shift >= LONG_BITS, shift < QUAD_BITS)
	 *   shift == 64, 65       (shift >= QUAD_BITS)
	 */
	push_shift(0);
	push_shift(1);
	push_shift(LONG_BITS - 2);
	push_shift(LONG_BITS - 1);
	push_shift(LONG_BITS);
	push_shift(LONG_BITS + 1);
	push_shift(QUAD_BITS - 2);
	push_shift(QUAD_BITS - 1);
	push_shift(QUAD_BITS);
	push_shift(QUAD_BITS + 1);
	push_shift(QUAD_BITS + 2);
	push_shift(96);
	push_shift(127);
	push_shift(255);

	for (int s = 0; s <= QUAD_BITS + 8; s++)
		push_shift((unsigned int)s);
}

void
targeted_add(void)
{
	/* no carry in the low half */
	chk_add(0, 0);
	chk_add(1, 2);
	chk_add(5, 3);
	chk_add((s64)0x0000000100000000ULL, 1);

	/*
	 * sum.ul[L] == bb.ul[L] with no carry: aa.ul[L] must be 0 so the
	 * (sum.ul[L] < bb.ul[L]) predicate is false, not merely <=.
	 */
	chk_add(0, (s64)0xffffffffULL);
	chk_add((s64)0x100000000ULL, 0);

	/* low-half carry propagates into the high half */
	chk_add((s64)0xffffffffULL, 1);
	chk_add((s64)0xffffffffULL, (s64)0xffffffffULL);
	chk_add((s64)0xfffffffeULL, 2);
	chk_add((s64)0x80000000ffffffffULL, 1);
	chk_add((s64)0xffffffffffffffffULL, 1);
}

void
targeted_shift(void)
{
	static const u64 kOperands[] = {
		0x0000000000000000ULL,
		0x0000000000000001ULL,
		0x000000007f800000ULL,
		0x00000000ffffffffULL,
		0x0000000100000000ULL,
		0x0000000180000000ULL,
		0x00000001ffffffffULL,
		0x7fffffff00000000ULL,
		0x8000000000000000ULL,
		0x8000000012345678ULL,
		0xffffffff00000000ULL,
		0xffffffffffffffffULL,
		0x123456789abcdef0ULL,
		0x00ff00ff00ff00ffULL,
		0xff00ff00ff00ff00ULL,
	};

	for (u64 v : kOperands) {
		chk_ashl((s64)v, 0);
		chk_lshl((s64)v, 0);
		chk_ashl((s64)v, 1);
		chk_lshl((s64)v, 1);
		chk_ashl((s64)v, (unsigned int)(LONG_BITS - 1));
		chk_lshl((s64)v, (unsigned int)(LONG_BITS - 1));
		chk_ashl((s64)v, (unsigned int)LONG_BITS);
		chk_lshl((s64)v, (unsigned int)LONG_BITS);
		chk_ashl((s64)v, (unsigned int)(LONG_BITS + 1));
		chk_lshl((s64)v, (unsigned int)(LONG_BITS + 1));
		chk_ashl((s64)v, (unsigned int)(QUAD_BITS - 1));
		chk_lshl((s64)v, (unsigned int)(QUAD_BITS - 1));
		chk_ashl((s64)v, (unsigned int)QUAD_BITS);
		chk_lshl((s64)v, (unsigned int)QUAD_BITS);
		chk_ashl((s64)v, (unsigned int)(QUAD_BITS + 1));
		chk_lshl((s64)v, (unsigned int)(QUAD_BITS + 1));
	}

	/*
	 * Low word bits that move into the high word through the middle
	 * branch, and through the large-shift branch.
	 */
	chk_ashl((s64)0x0000000180000000ULL, 1);
	chk_lshl((s64)0x0000000180000000ULL, 1);
	chk_ashl((s64)0x00000000c0000000ULL, 2);
	chk_lshl((s64)0x00000000c0000000ULL, 2);
	chk_ashl((s64)0x80000000ULL, (unsigned int)LONG_BITS);
	chk_lshl((s64)0x80000000ULL, (unsigned int)LONG_BITS);
	chk_ashl((s64)0x40000000ULL, (unsigned int)(LONG_BITS + 1));
	chk_lshl((s64)0x40000000ULL, (unsigned int)(LONG_BITS + 1));
}

void
targeted_flt(void)
{
	chk_flt(0);
	chk_flt(1);
	chk_flt(-1);
	chk_flt(2);
	chk_flt(-2);
	chk_flt((s64)0x7fffffffffffffffULL);
	chk_flt((s64)0x8000000000000000ULL);
	chk_flt((s64)0xffffffffffffffffULL);

	/* only low word */
	chk_flt((s64)0x00000000ffffffffULL);
	chk_flt((s64)0x000000007fffffffULL);
	chk_flt((s64)0x0000000080000000ULL);
	chk_flt((s64)0x00000000fffffffeULL);

	/* only high word */
	chk_flt((s64)0x0000000100000000ULL);
	chk_flt((s64)0x0000000180000000ULL);
	chk_flt((s64)0x7fffffff00000000ULL);

	/* both halves */
	chk_flt((s64)0x123456789abcdef0ULL);
	chk_flt((s64)0x8000000012345678ULL);

	/* negatives mirroring the above */
	chk_flt((s64)0xffffffff00000001ULL);
	chk_flt((s64)0xfffffffeffffffffULL);
	chk_flt((s64)0x80000000ffffffffULL);
}

void
edge_cases(void)
{
	targeted_add();
	targeted_shift();
	targeted_flt();

	for (int i = 0; i < kNVals; i++)
		for (int j = 0; j < kNVals; j++)
			chk_add((s64)kVals[i], (s64)kVals[j]);

	for (int i = 0; i < kNVals; i++)
		for (int s = 0; s < kNShifts; s++) {
			chk_ashl((s64)kVals[i], kShifts[s]);
			chk_lshl((s64)kVals[i], kShifts[s]);
		}

	for (int i = 0; i < kNVals; i++)
		chk_flt((s64)kVals[i]);
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

unsigned int
randshift(void)
{
	u64 r = nextrand();

	switch (r & 7) {
	case 0:
		return ((unsigned int)(r % (QUAD_BITS + 9)));
	case 1:
		return ((unsigned int)(LONG_BITS + (r & 3)));
	case 2:
		return ((unsigned int)(QUAD_BITS + (r & 3)));
	case 3:
		return (0);
	case 4:
		return (1);
	case 5:
		return ((unsigned int)LONG_BITS);
	case 6:
		return ((unsigned int)QUAD_BITS);
	default:
		return ((unsigned int)(r & 0xff));
	}
}

void
random_sweep(void)
{
	const long iters = 250000;

	for (long i = 0; i < iters; i++) {
		u64 a = randval();
		u64 b = randval();
		unsigned int sh = randshift();

		chk_add((s64)a, (s64)b);
		chk_ashl((s64)a, sh);
		chk_lshl((s64)b, sh);
		chk_flt((s64)a);
		chk_flt((s64)b);
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
	build_shifts();

	std::printf("b0040 differential test: %d edge values, "
	    "%d edge pairs, %d shift values, 250000 random iterations\n",
	    kNVals, kNVals * kNVals, kNShifts);

	edge_cases();
	random_sweep();

	std::printf("\n  %-20s %12s %12s   %s\n", "function", "cases",
	    "failures", "status");
	std::printf("  %-20s %12s %12s   %s\n", "--------------------",
	    "------------", "------------", "------");
	report(st_add);
	report(st_ashl);
	report(st_lshl);
	report(st_flt);

	u64 total = st_add.fails + st_ashl.fails + st_lshl.fails + st_flt.fails;
	u64 cases = st_add.cases + st_ashl.cases + st_lshl.cases + st_flt.cases;

	std::printf("\n  total: %llu cases, %llu failures\n", cases, total);
	if (total != 0) {
		std::printf("  RESULT: FAIL\n");
		return (1);
	}
	std::printf("  RESULT: PASS\n");
	return (0);
}
