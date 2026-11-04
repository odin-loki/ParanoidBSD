/*
 * Differential test for PBSD batch b0051 (lib/libc/quad).
 *
 * Every case is run through both the C++23 port and the unmodified C
 * oracle.  Scalar arguments live inside 0x7f-filled guard frames; return
 * values are compared on their object representation.
 */

#include <cstdio>
#include <cstdint>
#include <cstring>

import pbsd.lib.libc.quad.b0051;

extern "C" unsigned long long ref___udivdi3(unsigned long long a,
    unsigned long long b);
extern "C" unsigned long long ref___umoddi3(unsigned long long a,
    unsigned long long b);
extern "C" long long ref___anddi3(long long a, long long b);
extern "C" long long ref___xordi3(long long a, long long b);

namespace P = pbsd::lib_libc_quad::b0051;

namespace {

using u64 = unsigned long long;
using s64 = long long;
using u32 = std::uint32_t;

constexpr int kMaxReport = 8;
constexpr u64 UQUAD_MAX = ~0ULL;

struct Stat {
	const char *name;
	u64 cases;
	u64 fails;
	int reported;
};

Stat st_udiv = { "__udivdi3", 0, 0, 0 };
Stat st_umod = { "__umoddi3", 0, 0, 0 };
Stat st_and = { "__anddi3", 0, 0, 0 };
Stat st_xor = { "__xordi3", 0, 0, 0 };

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

struct Frame {
	unsigned char lead[24];
	u64 val;
	unsigned char trail[32];
};
static_assert(sizeof(Frame) == 64, "Frame must be padding-free");

void
frame_init(Frame &f, u64 v)
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
chk_udiv(u64 a, u64 b)
{
	Frame pa, pb, ra, rb;

	st_udiv.cases++;
	frame_init(pa, a);
	frame_init(pb, b);
	frame_init(ra, a);
	frame_init(rb, b);

	u64 got = P::__udivdi3(pa.val, pb.val);
	u64 exp = ref___udivdi3(ra.val, rb.val);

	if (!bytes_eq(got, exp) || !bytes_eq(pa, ra) || !bytes_eq(pb, rb))
		fail_msg(st_udiv, "");
}

void
chk_umod(u64 a, u64 b)
{
	Frame pa, pb, ra, rb;

	st_umod.cases++;
	frame_init(pa, a);
	frame_init(pb, b);
	frame_init(ra, a);
	frame_init(rb, b);

	u64 got = P::__umoddi3(pa.val, pb.val);
	u64 exp = ref___umoddi3(ra.val, rb.val);

	if (!bytes_eq(got, exp) || !bytes_eq(pa, ra) || !bytes_eq(pb, rb))
		fail_msg(st_umod, "");
}

void
chk_and(s64 a, s64 b)
{
	Frame pa, pb, ra, rb;

	st_and.cases++;
	frame_init(pa, (u64)a);
	frame_init(pb, (u64)b);
	frame_init(ra, (u64)a);
	frame_init(rb, (u64)b);

	s64 got = P::__anddi3((s64)pa.val, (s64)pb.val);
	s64 exp = ref___anddi3((s64)ra.val, (s64)rb.val);

	if (!bytes_eq(got, exp) || !bytes_eq(pa, ra) || !bytes_eq(pb, rb))
		fail_msg(st_and, "");
}

void
chk_xor(s64 a, s64 b)
{
	Frame pa, pb, ra, rb;

	st_xor.cases++;
	frame_init(pa, (u64)a);
	frame_init(pb, (u64)b);
	frame_init(ra, (u64)a);
	frame_init(rb, (u64)b);

	s64 got = P::__xordi3((s64)pa.val, (s64)pb.val);
	s64 exp = ref___xordi3((s64)ra.val, (s64)rb.val);

	if (!bytes_eq(got, exp) || !bytes_eq(pa, ra) || !bytes_eq(pb, rb))
		fail_msg(st_xor, "");
}

/* ---------------------------------------------------------------- */

const u32 kWords[] = {
	0x00000000u, 0x00000001u, 0x00000002u, 0x00000003u,
	0x0000007fu, 0x00000080u, 0x000000ffu, 0x00007fffu,
	0x00008000u, 0x0000ffffu, 0x7ffffffeu, 0x7fffffffu,
	0x80000000u, 0x80000001u, 0xfffffffeu, 0xffffffffu,
	0x55555555u, 0xaaaaaaaau, 0x7f7f7f7fu, 0x80808080u,
	0xff00ff00u, 0x00ff00ffu, 0x01010101u, 0xfefefefeu,
	0xdeadbeefu, 0x12345678u, 0x00004000u, 0x00008000u,
	0x00010000u, 0x00020000u, 0x00ff0000u, 0xff000000u,
};
const int kNWords = (int)(sizeof kWords / sizeof kWords[0]);

u64 kQuads[2048];
int kNQuads = 0;

void
push_quad(u64 v)
{
	if (kNQuads < (int)(sizeof kQuads / sizeof kQuads[0]))
		kQuads[kNQuads++] = v;
}

void
build_quads(void)
{
	for (int i = 0; i < kNWords; i++)
		for (int j = 0; j < kNWords; j++)
			push_quad(((u64)kWords[i] << 32) | (u64)kWords[j]);

	static const unsigned kBytes[] = {
		0x00u, 0x01u, 0x02u, 0x7fu, 0x80u, 0xfeu, 0xffu,
	};
	for (int p = 0; p < 8; p++) {
		for (unsigned b : kBytes) {
			u64 v = (u64)b << (8 * p);
			push_quad(v);
			push_quad(~v);
		}
	}
}

void
chk_div_pair(u64 a, u64 b)
{
	if (b == 0)
		return;
	chk_udiv(a, b);
	chk_umod(a, b);
}

void
targeted_div(void)
{
	static const u64 kAs[] = {
		0ULL, 1ULL, 2ULL, 3ULL,
		0x7fffffffffffffffULL, 0x8000000000000000ULL,
		0xffffffffffffffffULL,
		0x100000000ULL, 0x100000001ULL,
		0x123456789abcdef0ULL,
		0x0000000100000002ULL,
		0x0000800000000001ULL,
		0x0001000000000000ULL,
		0x00ff000000ff0000ULL,
		0xdeadbeefcafebabeULL,
		0x000000007fffffffULL,
		0x0000000080000000ULL,
		0x00000000fffffffeULL,
		0x00000000ffffffffULL,
		0xffffffff00000000ULL,
		0xffffffffffffffffULL,
		0x0000ffff0000ffffULL,
		0x000100000000ffffULL,
		0x123400000000abcdULL,
		0x000000000000ff80ULL,
		0x0000000000800000ULL,
		0x8000000000000001ULL,
	};
	static const u64 kBs[] = {
		1ULL, 2ULL, 3ULL, 7ULL, 15ULL, 16ULL, 17ULL,
		0x7fffULL, 0x8000ULL, 0x8001ULL,
		0xffffULL, 0x10000ULL, 0x10001ULL,
		0x7fffffffULL, 0x80000000ULL,
		0xffffffffULL, 0x100000000ULL,
		0x100000001ULL, 0x12345678ULL,
		0x0000ffffULL, 0x00010000ULL,
		0x00008000ULL, 0x00004000ULL,
		0x123456789abcdef0ULL,
		0x0000000100000001ULL,
		0x00ff000000ff0000ULL,
		0xdeadbeefULL,
		0x000000000000ff80ULL,
		0x0000000000800000ULL,
		0x8000000000000001ULL,
		0xffffffffffffffffULL,
	};

	for (u64 a : kAs) {
		for (u64 b : kBs)
			chk_div_pair(a, b);
		chk_div_pair(a, a);
		chk_div_pair(a, a + 1);
		if (a > 0)
			chk_div_pair(a, a - 1);
	}

	/* u < v early-return path */
	chk_div_pair(5ULL, 10ULL);
	chk_div_pair(0xffffffffULL, 0x100000000ULL);
	chk_div_pair(0x1234ULL, 0x5678ULL);

	/* n == 1 single-digit divisor path */
	chk_div_pair(0x123456789abcdef0ULL, 0x1234ULL);
	chk_div_pair(0x0000000100000002ULL, 0xffffULL);
	chk_div_pair(0xffffffffffffffffULL, 0x0000ffffULL);
	chk_div_pair(0x0000000000000001ULL, 0x00000002ULL);
	chk_div_pair(0x000000000000fffeULL, 0x0000fffeULL);

	/* normalization shift d > 0 (v[1] < B/2) */
	chk_div_pair(0x1234567890abcdefULL, 0x1234567890abcdefULL);
	chk_div_pair(0xfedcba0987654321ULL, 0x0001234500006789ULL);
	chk_div_pair(0x0123456789abcdefULL, 0x0000123400005678ULL);
	chk_div_pair(0x8000000000000001ULL, 0x0000400000000001ULL);
	chk_div_pair(0x000000007fff0000ULL, 0x0000000000007fffULL);

	/* quotient near B and borrow/add-back territory */
	chk_div_pair(0xffffffffffffffffULL, 0x0000000100000001ULL);
	chk_div_pair(0x00000001000000010000ULL, 0x0000000100000000ULL);
	chk_div_pair(0x0000ffff0000ffffULL, 0x0000ffffULL);
	chk_div_pair(0x0001000000000000ULL, 0x0000800000000001ULL);
}

void
targeted_bitwise(void)
{
	static const s64 kVals[] = {
		0LL, 1LL, -1LL, 2LL, -2LL,
		(s64)0x7fffffffffffffffLL,
		(s64)0x8000000000000000ULL,
		(s64)0xffffffffffffffffULL,
		(s64)0x123456789abcdef0LL,
		(s64)0x000000ff00000080LL,
		(s64)0x00000080000000ffLL,
		(s64)0x00ff00ff00ff00ffLL,
		(s64)0x7f7f7f7f80808080LL,
		(s64)0x000000007fffffffLL,
		(s64)0x0000000080000000LL,
		(s64)0xffffffff00000000LL,
		(s64)0x00000000ffffffffLL,
		(s64)0x5555555555555555LL,
		(s64)0xaaaaaaaaaaaaaaaaLL,
	};

	for (s64 a : kVals) {
		for (s64 b : kVals) {
			chk_and(a, b);
			chk_xor(a, b);
		}
	}

	/* isolate ul[0] vs ul[1] limbs */
	chk_and((s64)0x00000000ffffffffULL, (s64)0xffffffff00000000ULL);
	chk_xor((s64)0x00000000ffffffffULL, (s64)0xffffffff00000000ULL);
	chk_and((s64)0x00000000ff80ff80ULL, (s64)0x000000007f007f00ULL);
	chk_xor((s64)0x00000000ff80ff80ULL, (s64)0x000000007f007f00ULL);
	chk_and((s64)0xff80ff8000000000ULL, (s64)0x7f007f0000000000ULL);
	chk_xor((s64)0xff80ff8000000000ULL, (s64)0x7f007f0000000000ULL);
}

void
edge_cases(void)
{
	targeted_div();
	targeted_bitwise();

	for (int i = 0; i < kNQuads; i++) {
		for (int j = 0; j < kNQuads; j++) {
			u64 a = kQuads[i];
			u64 b = kQuads[j];
			if (b != 0) {
				chk_udiv(a, b);
				chk_umod(a, b);
			}
			chk_and((s64)a, (s64)b);
			chk_xor((s64)a, (s64)b);
		}
	}
}

u64
rand_uquad(void)
{
	u64 r = nextrand();

	switch (r & 7) {
	case 0:
		return (nextrand());
	case 1:
		return (((u64)kWords[nextrand() % kNWords] << 32) |
		    (u64)(u32)nextrand());
	case 2:
		return (((u64)(u32)nextrand() << 32) |
		    (u64)kWords[nextrand() % kNWords]);
	case 3:
		return (((u64)kWords[nextrand() % kNWords] << 32) |
		    (u64)kWords[nextrand() % kNWords]);
	case 4:
		return ((u64)(std::uint16_t)nextrand());
	case 5:
		return ((u64)(std::uint16_t)nextrand() << 48) |
		    (nextrand() & 0xffffffffffffULL);
	case 6:
		return (UQUAD_MAX - (nextrand() & 0xffffULL));
	default:
		return (nextrand() ^ (nextrand() << 17));
	}
}

u64
rand_divisor(void)
{
	u64 b;

	do {
		b = rand_uquad();
	} while (b == 0);
	return (b);
}

void
random_sweep(void)
{
	const long iters = 200000;

	for (long i = 0; i < iters; i++) {
		u64 a = rand_uquad();
		u64 b = rand_divisor();

		chk_udiv(a, b);
		chk_umod(a, b);
		chk_and((s64)a, (s64)b);
		chk_xor((s64)a, (s64)b);
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

	std::printf("b0051 differential test: %d edge quads, "
	    "200000 random iterations\n", kNQuads);

	edge_cases();
	random_sweep();

	std::printf("\n  %-20s %12s %12s   %s\n", "function", "cases",
	    "failures", "status");
	std::printf("  %-20s %12s %12s   %s\n", "--------------------",
	    "------------", "------------", "------");
	report(st_udiv);
	report(st_umod);
	report(st_and);
	report(st_xor);

	u64 total = st_udiv.fails + st_umod.fails + st_and.fails + st_xor.fails;
	u64 cases = st_udiv.cases + st_umod.cases + st_and.cases + st_xor.cases;

	std::printf("\n  total: %llu cases, %llu failures\n", cases, total);
	if (total != 0) {
		std::printf("  RESULT: FAIL\n");
		return (1);
	}
	std::printf("  RESULT: PASS\n");
	return (0);
}
