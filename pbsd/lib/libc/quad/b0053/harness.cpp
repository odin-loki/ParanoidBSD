/*
 * Differential test for PBSD batch b0053 (lib/libc/quad).
 *
 * Every case is run through both the C++23 port and the unmodified C
 * oracle.  Scalar arguments live inside 0x7f-filled guard frames; return
 * values are compared on their object representation.
 */

#include <cstdio>
#include <cstdint>
#include <cstring>

import pbsd.lib.libc.quad.b0053;

extern "C" long long ref___moddi3(long long a, long long b);
extern "C" long long ref___lshrdi3(long long a, unsigned int shift);

namespace P = pbsd::lib_libc_quad::b0053;

namespace {

using u64 = unsigned long long;
using s64 = long long;
using u32 = std::uint32_t;

constexpr int kMaxReport = 8;
constexpr long kRandIters = 200000;

struct Stat {
	const char *name;
	u64 cases;
	u64 fails;
	int reported;
};

Stat st_mod = { "__moddi3", 0, 0, 0 };
Stat st_lshr = { "__lshrdi3", 0, 0, 0 };

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

struct FramePair {
	unsigned char lead[16];
	s64 a;
	unsigned char mid[16];
	s64 b;
	unsigned char trail[32];
};
static_assert(sizeof(FramePair) == 80, "FramePair must be padding-free");

struct FrameShift {
	unsigned char lead[16];
	unsigned int shift;
	unsigned char mid[16];
	s64 val;
	unsigned char trail[32];
};
static_assert(sizeof(FrameShift) == 80, "FrameShift must be padding-free");

void
frame_pair_init(FramePair &f, s64 a, s64 b)
{
	std::memset(&f, 0x7f, sizeof f);
	f.a = a;
	f.b = b;
}

void
frame_shift_init(FrameShift &f, s64 v, unsigned int sh)
{
	std::memset(&f, 0x7f, sizeof f);
	f.val = v;
	f.shift = sh;
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

void
chk_moddi3(s64 a, s64 b)
{
	FramePair pa, ra;

	st_mod.cases++;
	frame_pair_init(pa, a, b);
	frame_pair_init(ra, a, b);

	s64 got = P::__moddi3(pa.a, pa.b);
	s64 exp = ref___moddi3(ra.a, ra.b);

	if (!bytes_eq(got, exp) || !bytes_eq(pa, ra))
		fail_msg(st_mod, "");
}

void
chk_lshrdi3(s64 a, unsigned int shift)
{
	FrameShift pa, ra;

	st_lshr.cases++;
	frame_shift_init(pa, a, shift);
	frame_shift_init(ra, a, shift);

	s64 got = P::__lshrdi3(pa.val, pa.shift);
	s64 exp = ref___lshrdi3(ra.val, ra.shift);

	if (!bytes_eq(got, exp) || !bytes_eq(pa, ra))
		fail_msg(st_lshr, "");
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

const unsigned int kShifts[] = {
	0u, 1u, 2u, 15u, 16u, 30u, 31u,
	32u, 33u, 47u, 48u, 62u, 63u, 64u, 65u, 96u, 127u, 255u,
};
const int kNShifts = (int)(sizeof kShifts / sizeof kShifts[0]);

void
targeted_moddi3(void)
{
	chk_moddi3(0, 1);
	chk_moddi3(0, -1);
	chk_moddi3(0, 0);
	chk_moddi3(1, 1);
	chk_moddi3(1, 2);
	chk_moddi3(2, 1);
	chk_moddi3(-1, 1);
	chk_moddi3(1, -1);
	chk_moddi3(-1, -1);
	chk_moddi3(-2, 1);
	chk_moddi3(1, -2);
	chk_moddi3(-3, 2);
	chk_moddi3(3, -2);
	chk_moddi3((s64)0x7fffffffffffffffLL, 1);
	chk_moddi3((s64)0x7fffffffffffffffLL, 2);
	chk_moddi3((s64)0x8000000000000000ULL, 1);
	chk_moddi3((s64)0x8000000000000000ULL, 2);
	chk_moddi3((s64)0xffffffffffffffffULL, 1);
	chk_moddi3((s64)0xffffffffffffffffULL, 2);
	chk_moddi3((s64)0x8000000000000000ULL, (s64)0x8000000000000000ULL);
	chk_moddi3((s64)0x7fffffffffffffffLL, (s64)0x8000000000000000ULL);
	chk_moddi3((s64)0x123456789abcdef0ULL, 7);
	chk_moddi3((s64)-0x123456789abcdef0ULL, 7);
	chk_moddi3((s64)0x123456789abcdef0ULL, -7);
	chk_moddi3((s64)0x100000000LL, 3);
	chk_moddi3((s64)-0x100000000LL, 3);
	chk_moddi3((s64)0x100000001LL, 0x10000);
	chk_moddi3((s64)0x8000000100000000ULL, 0x10000);
	chk_moddi3((s64)0x0000000100000001ULL, 0x10000);
	chk_moddi3((s64)0x0001000000000000ULL, 0x100000);
	chk_moddi3((s64)0x0000000100000000ULL, 0x10000);
	chk_moddi3((s64)0x00000000ffffffffULL, 0x10000);
	chk_moddi3((s64)0xffffffff00000000ULL, 0x10000);
}

void
targeted_lshrdi3(void)
{
	for (unsigned int sh : kShifts) {
		chk_lshrdi3(0, sh);
		chk_lshrdi3(1, sh);
		chk_lshrdi3(-1, sh);
		chk_lshrdi3((s64)0x7fffffffffffffffLL, sh);
		chk_lshrdi3((s64)0x8000000000000000ULL, sh);
		chk_lshrdi3((s64)0xffffffffffffffffULL, sh);
		chk_lshrdi3((s64)0x80000000LL, sh);
		chk_lshrdi3((s64)0x7fffffffLL, sh);
		chk_lshrdi3((s64)-0x80000000LL, sh);
		chk_lshrdi3((s64)0x123456789abcdef0ULL, sh);
		chk_lshrdi3((s64)0x8000000100000000ULL, sh);
		chk_lshrdi3((s64)0x0000000100000001ULL, sh);
		chk_lshrdi3((s64)0xffffffff00000000ULL, sh);
		chk_lshrdi3((s64)0x00000000ffffffffULL, sh);
	}
}

void
edge_cases(void)
{
	targeted_moddi3();
	targeted_lshrdi3();

	for (int i = 0; i < kNQuads; i++) {
		for (int j = 0; j < kNQuads; j++)
			chk_moddi3(kQuads[i], kQuads[j]);
		for (int s = 0; s < kNShifts; s++)
			chk_lshrdi3(kQuads[i], kShifts[s]);
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
	for (long i = 0; i < kRandIters; i++) {
		if ((i & 1) == 0)
			chk_moddi3(rand_quad(), rand_quad());
		else
			chk_lshrdi3(rand_quad(), rand_shift());
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

	std::printf("b0053 differential test: %d edge quads, "
	    "%d shift values, %ld random iterations\n",
	    kNQuads, kNShifts, kRandIters);

	edge_cases();
	random_sweep();

	std::printf("\n  %-20s %12s %12s   %s\n", "function", "cases",
	    "failures", "status");
	std::printf("  %-20s %12s %12s   %s\n", "--------------------",
	    "------------", "------------", "------");
	report(st_mod);
	report(st_lshr);

	u64 total = st_mod.fails + st_lshr.fails;
	u64 cases = st_mod.cases + st_lshr.cases;

	std::printf("\n  total: %llu cases, %llu failures\n", cases, total);
	if (total != 0) {
		std::printf("  RESULT: FAIL\n");
		return (1);
	}
	std::printf("  RESULT: PASS\n");
	return (0);
}
