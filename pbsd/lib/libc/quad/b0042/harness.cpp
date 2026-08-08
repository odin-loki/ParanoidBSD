/*
 * Differential test for PBSD batch b0042 (lib/libc/quad).
 *
 * Every case is run through both the C++23 port and the unmodified C
 * oracle.  Scalar arguments live inside 0x7f-filled guard frames; return
 * values are compared on their object representation.
 */

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cmath>

import pbsd.lib.libc.quad.b0042;

extern "C" float ref___floatdisf(long long x);
extern "C" long long ref___ashrdi3(long long a, unsigned int shift);
extern "C" unsigned long long ref___fixunsdfdi(double x);
extern "C" unsigned long long ref___fixunssfdi(float f);

namespace P = pbsd::lib_libc_quad::b0042;

namespace {

using u64 = unsigned long long;
using s64 = long long;
using u32 = std::uint32_t;
using f32 = float;
using f64 = double;

constexpr int kMaxReport = 8;
constexpr u64 UQUAD_MAX = ~0ULL;

struct Stat {
	const char *name;
	u64 cases;
	u64 fails;
	int reported;
};

Stat st_flt = { "__floatdisf", 0, 0, 0 };
Stat st_ashr = { "__ashrdi3", 0, 0, 0 };
Stat st_fixd = { "__fixunsdfdi", 0, 0, 0 };
Stat st_fixf = { "__fixunssfdi", 0, 0, 0 };

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

struct Frame64 {
	unsigned char lead[24];
	s64 val;
	unsigned char trail[32];
};
static_assert(sizeof(Frame64) == 64, "Frame64 must be padding-free");

struct FrameShift {
	unsigned char lead[16];
	unsigned int shift;
	unsigned char mid[16];
	s64 val;
	unsigned char trail[32];
};
static_assert(sizeof(FrameShift) == 80, "FrameShift must be padding-free");

struct FrameDbl {
	unsigned char lead[24];
	f64 val;
	unsigned char trail[32];
};
static_assert(sizeof(FrameDbl) == 64, "FrameDbl must be padding-free");

struct FrameFlt {
	unsigned char lead[24];
	f32 val;
	unsigned char trail[32];
};
static_assert(sizeof(FrameFlt) == 64, "FrameFlt must be padding-free");

void
frame64_init(Frame64 &f, s64 v)
{
	std::memset(&f, 0x7f, sizeof f);
	f.val = v;
}

void
frame_shift_init(FrameShift &f, s64 v, unsigned int sh)
{
	std::memset(&f, 0x7f, sizeof f);
	f.val = v;
	f.shift = sh;
}

void
frame_dbl_init(FrameDbl &f, f64 v)
{
	std::memset(&f, 0x7f, sizeof f);
	f.val = v;
}

void
frame_flt_init(FrameFlt &f, f32 v)
{
	std::memset(&f, 0x7f, sizeof f);
	f.val = v;
}

template<typename T>
bool
bytes_eq(const T &a, const T &b)
{
	return (std::memcmp(&a, &b, sizeof(T)) == 0);
}

void
fail_msg(Stat &s, const char *detail)
{
	s.fails++;
	if (s.reported < kMaxReport) {
		s.reported++;
		std::printf("  %s: MISMATCH %s\n", s.name, detail);
	}
}

/* ---------------------------------------------------------------- */

void
chk_floatdisf(s64 x)
{
	Frame64 pa, ra;

	st_flt.cases++;
	frame64_init(pa, x);
	frame64_init(ra, x);

	f32 got = P::__floatdisf(pa.val);
	f32 exp = ref___floatdisf(ra.val);

	if (!bytes_eq(got, exp) || !bytes_eq(pa, ra))
		fail_msg(st_flt, "");
}

void
chk_ashrdi3(s64 a, unsigned int shift)
{
	FrameShift pa, ra;

	st_ashr.cases++;
	frame_shift_init(pa, a, shift);
	frame_shift_init(ra, a, shift);

	s64 got = P::__ashrdi3(pa.val, pa.shift);
	s64 exp = ref___ashrdi3(ra.val, ra.shift);

	if (!bytes_eq(got, exp) || !bytes_eq(pa, ra))
		fail_msg(st_ashr, "");
}

void
chk_fixunsdfdi(f64 x)
{
	FrameDbl px, rx;

	st_fixd.cases++;
	frame_dbl_init(px, x);
	frame_dbl_init(rx, x);

	u64 got = P::__fixunsdfdi(px.val);
	u64 exp = ref___fixunsdfdi(rx.val);

	if (!bytes_eq(got, exp) || !bytes_eq(px, rx))
		fail_msg(st_fixd, "");
}

void
chk_fixunssfdi(f32 f)
{
	FrameFlt pf, rf;

	st_fixf.cases++;
	frame_flt_init(pf, f);
	frame_flt_init(rf, f);

	u64 got = P::__fixunssfdi(pf.val);
	u64 exp = ref___fixunssfdi(rf.val);

	if (!bytes_eq(got, exp) || !bytes_eq(pf, rf))
		fail_msg(st_fixf, "");
}

/* ---------------------------------------------------------------- */

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

s64 kQuads[1024];
int kNQuads = 0;

void
push_quad(s64 v)
{
	if (kNQuads < (int)(sizeof kQuads / sizeof kQuads[0]))
		kQuads[kNQuads++] = v;
}

void
build_quads(void)
{
	for (int i = 0; i < kNWords; i++)
		for (int j = 0; j < kNWords; j++)
			push_quad((s64)(((u64)kWords[i] << 32) |
			    (u64)kWords[j]));

	static const unsigned kBytes[] = {
		0x00u, 0x01u, 0x02u, 0x7fu, 0x80u, 0xfeu, 0xffu,
	};
	for (int p = 0; p < 8; p++) {
		for (unsigned b : kBytes) {
			u64 v = (u64)b << (8 * p);
			push_quad((s64)v);
			push_quad((s64)~v);
		}
	}
}

/*
 * Shift values that straddle every branch boundary in __ashrdi3.
 * LONG_BITS and QUAD_BITS are 32 and 64 on the target decomposition.
 */
const unsigned int kShifts[] = {
	0u, 1u, 2u, 15u, 16u, 30u, 31u,
	32u, 33u, 47u, 48u, 62u, 63u, 64u, 65u, 96u, 127u, 255u,
};
const int kNShifts = (int)(sizeof kShifts / sizeof kShifts[0]);

void
targeted_floatdisf(void)
{
	chk_floatdisf(0);
	chk_floatdisf(1);
	chk_floatdisf(-1);
	chk_floatdisf(2);
	chk_floatdisf(-2);
	chk_floatdisf((s64)0x7fffffffffffffffLL);
	chk_floatdisf((s64)0x8000000000000000ULL);
	chk_floatdisf((s64)0xffffffffffffffffULL);
	chk_floatdisf((s64)0x100000000LL);
	chk_floatdisf((s64)0x100000001LL);
	chk_floatdisf((s64)-0x100000000LL);
	chk_floatdisf((s64)-0x100000001LL);
	chk_floatdisf((s64)0x80000000LL);
	chk_floatdisf((s64)0x7fffffffLL);
	chk_floatdisf((s64)-0x80000000LL);
	chk_floatdisf((s64)-0x7fffffffLL);
}

void
targeted_ashrdi3(void)
{
	for (unsigned int sh : kShifts) {
		chk_ashrdi3(0, sh);
		chk_ashrdi3(1, sh);
		chk_ashrdi3(-1, sh);
		chk_ashrdi3((s64)0x7fffffffffffffffLL, sh);
		chk_ashrdi3((s64)0x8000000000000000ULL, sh);
		chk_ashrdi3((s64)0xffffffffffffffffULL, sh);
		chk_ashrdi3((s64)0x80000000LL, sh);
		chk_ashrdi3((s64)0x7fffffffLL, sh);
		chk_ashrdi3((s64)-0x80000000LL, sh);
		chk_ashrdi3((s64)0x123456789abcdef0ULL, sh);
		chk_ashrdi3((s64)0x8000000100000000ULL, sh);
	}
}

void
targeted_fixunsdfdi(void)
{
	const f64 ONE = 4294967296.0;
	const f64 ONE_HALF = 2147483648.0;
	const f64 UMAX = 18446744073709551615.0;

	chk_fixunsdfdi(-1.0);
	chk_fixunsdfdi(-0.0);
	chk_fixunsdfdi(0.0);
	chk_fixunsdfdi(0.5);
	chk_fixunsdfdi(1.0);
	chk_fixunsdfdi(UMAX);
	chk_fixunsdfdi(UMAX - 1.0);
	chk_fixunsdfdi(UMAX + 1.0);
	chk_fixunsdfdi(ONE - 1.0);
	chk_fixunsdfdi(ONE);
	chk_fixunsdfdi(ONE + 1.0);
	chk_fixunsdfdi(ONE_HALF);
	chk_fixunsdfdi(ONE_HALF - 1.0);
	chk_fixunsdfdi(ONE_HALF + 1.0);
	chk_fixunsdfdi(2.0 * ONE);
	chk_fixunsdfdi(2.0 * ONE - 1.0);
	chk_fixunsdfdi(2.0 * ONE + 1.0);
	chk_fixunsdfdi((f64)0xffffffffu);
	chk_fixunsdfdi((f64)0x100000000ULL);
	chk_fixunsdfdi((f64)0x100000001ULL);
	chk_fixunsdfdi((f64)0x1ffffffffULL);
	chk_fixunsdfdi((f64)0x200000000ULL);
	chk_fixunsdfdi(1.5 * ONE);
	chk_fixunsdfdi(1.5 * ONE - 0.5);
	chk_fixunsdfdi(1.5 * ONE + 0.5);
}

void
targeted_fixunssfdi(void)
{
	const f32 ONE = 4294967296.0f;
	const f32 ONE_HALF = 2147483648.0f;
	const f32 UMAX = 18446744073709551615.0f;

	chk_fixunssfdi(-1.0f);
	chk_fixunssfdi(-0.0f);
	chk_fixunssfdi(0.0f);
	chk_fixunssfdi(0.5f);
	chk_fixunssfdi(1.0f);
	chk_fixunssfdi(UMAX);
	chk_fixunssfdi(UMAX - 1.0f);
	chk_fixunssfdi(ONE - 1.0f);
	chk_fixunssfdi(ONE);
	chk_fixunssfdi(ONE + 1.0f);
	chk_fixunssfdi(ONE_HALF);
	chk_fixunssfdi(2.0f * ONE);
	chk_fixunssfdi((f32)0xffffffffu);
	chk_fixunssfdi((f32)0x100000000ULL);
	chk_fixunssfdi((f32)0x100000001ULL);
	chk_fixunssfdi(16777216.0f);
	chk_fixunssfdi(16777215.0f);
	chk_fixunssfdi(16777217.0f);
}

void
edge_cases(void)
{
	targeted_floatdisf();
	targeted_ashrdi3();
	targeted_fixunsdfdi();
	targeted_fixunssfdi();

	for (int i = 0; i < kNQuads; i++) {
		chk_floatdisf(kQuads[i]);
		for (int s = 0; s < kNShifts; s++)
			chk_ashrdi3(kQuads[i], kShifts[s]);
	}

	for (int i = 0; i < kNQuads; i++) {
		f64 d = (f64)(u64)kQuads[i];
		chk_fixunsdfdi(d);
		chk_fixunsdfdi(d + 0.25);
		chk_fixunsdfdi(d - 0.25);
		chk_fixunssfdi((f32)d);
	}
}

f64
rand_double(void)
{
	u64 r = nextrand();
	int kind = (int)(r & 7);

	switch (kind) {
	case 0:
		return ((f64)(s64)nextrand());
	case 1:
		return ((f64)(u64)nextrand());
	case 2:
		return ((f64)(u32)nextrand());
	case 3:
		return ((f64)(u32)nextrand() * 4294967296.0);
	case 4: {
		int exp = (int)(nextrand() % 80) - 10;
		f64 m = (f64)(nextrand() | 1ULL);
		return (std::ldexp(m, exp));
	}
	case 5:
		return (18446744073709551615.0 + (f64)((s64)nextrand() % 17 - 8));
	case 6:
		return (-(f64)(nextrand() & 0xffff));
	default:
		return ((f64)kWords[nextrand() % kNWords] * 4294967296.0 +
		    (f64)kWords[nextrand() % kNWords]);
	}
}

f32
rand_float(void)
{
	u64 r = nextrand();
	int kind = (int)(r & 7);

	switch (kind) {
	case 0:
		return ((f32)(s64)nextrand());
	case 1:
		return ((f32)(u32)nextrand());
	case 2:
		return ((f32)(u32)nextrand() * 16777216.0f);
	case 3: {
		int exp = (int)(nextrand() % 40) - 10;
		f32 m = (f32)(nextrand() | 1ULL);
		return ((f32)std::ldexp((f64)m, exp));
	}
	case 4:
		return (-(f32)(nextrand() & 0xff));
	case 5:
		return (4294967296.0f + (f32)((int)(nextrand() % 1025) - 512));
	default:
		return ((f32)kWords[nextrand() % kNWords]);
	}
}

s64
rand_quad(void)
{
	u64 r = nextrand();

	switch (r & 3) {
	case 0:
		return ((s64)nextrand());
	case 1:
		return ((s64)(((u64)kWords[nextrand() % kNWords] << 32) |
		    (u64)(u32)nextrand()));
	case 2:
		return ((s64)(((u64)(u32)nextrand() << 32) |
		    (u64)kWords[nextrand() % kNWords]));
	default:
		return ((s64)(((u64)kWords[nextrand() % kNWords] << 32) |
		    (u64)kWords[nextrand() % kNWords]));
	}
}

unsigned int
rand_shift(void)
{
	u64 r = nextrand();

	switch (r & 3) {
	case 0:
		return ((unsigned int)(r & 0xff));
	case 1:
		return ((unsigned int)(31 + (r & 3)));
	case 2:
		return ((unsigned int)(62 + (r & 3)));
	default:
		return ((unsigned int)nextrand());
	}
}

void
random_sweep(void)
{
	const long iters = 250000;

	for (long i = 0; i < iters; i++) {
		chk_floatdisf(rand_quad());
		chk_ashrdi3(rand_quad(), rand_shift());
		chk_fixunsdfdi(rand_double());
		chk_fixunssfdi(rand_float());
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
	build_quads();

	std::printf("b0042 differential test: %d edge quads, "
	    "%d shift values, 250000 random iterations\n",
	    kNQuads, kNShifts);

	edge_cases();
	random_sweep();

	std::printf("\n  %-20s %12s %12s   %s\n", "function", "cases",
	    "failures", "status");
	std::printf("  %-20s %12s %12s   %s\n", "--------------------",
	    "------------", "------------", "------");
	report(st_flt);
	report(st_ashr);
	report(st_fixd);
	report(st_fixf);

	u64 total = st_flt.fails + st_ashr.fails + st_fixd.fails + st_fixf.fails;
	u64 cases = st_flt.cases + st_ashr.cases + st_fixd.cases + st_fixf.cases;

	std::printf("\n  total: %llu cases, %llu failures\n", cases, total);
	if (total != 0) {
		std::printf("  RESULT: FAIL\n");
		return (1);
	}
	std::printf("  RESULT: PASS\n");
	return (0);
}
