/*
 * Differential test for PBSD batch b0044 (lib/libc/quad muldi3 + qdivrem).
 *
 * Every case is run through both the C++23 port and the unmodified C
 * oracle.  Scalar arguments live inside 0x7f-filled guard frames; remainder
 * outputs use paired guard frames.  Return values are compared on their
 * object representation.
 *
 * __qdivrem(., 0, .) deliberately traps; divide-by-zero is not exercised.
 */

#include <cstdio>
#include <cstdint>
#include <cstring>

import pbsd.lib.libc.quad.b0044;

extern "C" long long ref___muldi3(long long a, long long b);
extern "C" unsigned long long ref___qdivrem(unsigned long long u,
    unsigned long long v, unsigned long long *rem);

namespace P = pbsd::lib_libc_quad::b0044;

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

Stat st_mul = { "__muldi3", 0, 0, 0 };
Stat st_div = { "__qdivrem", 0, 0, 0 };

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
	s64 val;
	unsigned char trail[32];
};
static_assert(sizeof(Frame) == 64, "Frame must be padding-free");

struct RemFrame {
	unsigned char lead[24];
	u64 rem;
	unsigned char trail[32];
};
static_assert(sizeof(RemFrame) == 64, "RemFrame must be padding-free");

void
frame_init(Frame &f, s64 v)
{
	std::memset(&f, 0x7f, sizeof f);
	f.val = v;
}

void
rem_init(RemFrame &f)
{
	std::memset(&f, 0x7f, sizeof f);
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
chk_muldi3(s64 a, s64 b)
{
	Frame pa, pb, ra, rb;

	st_mul.cases++;
	frame_init(pa, a);
	frame_init(pb, b);
	frame_init(ra, a);
	frame_init(rb, b);

	s64 got = P::__muldi3(pa.val, pb.val);
	s64 exp = ref___muldi3(ra.val, rb.val);

	if (!bytes_eq(got, exp) || !bytes_eq(pa, ra) || !bytes_eq(pb, rb))
		fail_msg(st_mul, "");
}

void
chk_qdivrem(u64 u, u64 v, bool want_rem)
{
	Frame pu, pv, ru, rv;
	RemFrame pr, rr;

	st_div.cases++;
	frame_init(pu, (s64)u);
	frame_init(pv, (s64)v);
	frame_init(ru, (s64)u);
	frame_init(rv, (s64)v);
	rem_init(pr);
	rem_init(rr);

	u64 *prem = want_rem ? &pr.rem : nullptr;
	u64 *rrem = want_rem ? &rr.rem : nullptr;

	u64 qgot = P::__qdivrem((u64)pu.val, (u64)pv.val, prem);
	u64 qexp = ref___qdivrem((u64)ru.val, (u64)rv.val, rrem);

	if (!bytes_eq(qgot, qexp) || !bytes_eq(pu, ru) || !bytes_eq(pv, rv))
		fail_msg(st_div, "quotient/args");
	if (want_rem && (!bytes_eq(pr, rr)))
		fail_msg(st_div, "remainder frame");
}

/* ---------------------------------------------------------------- */

const u32 kWords[] = {
	0x00000000u, 0x00000001u, 0x00000002u, 0x00000003u,
	0x0000007fu, 0x00000080u, 0x000000ffu, 0x00007fffu,
	0x00008000u, 0x0000ffffu, 0x7ffffffeu, 0x7fffffffu,
	0x80000000u, 0x80000001u, 0xfffffffeu, 0xffffffffu,
	0x55555555u, 0xaaaaaaaau, 0x7f7f7f7fu, 0x80808080u,
	0xff00ff00u, 0x00ff00ffu, 0x01010101u, 0xfefefefeu,
	0xdeadbeefu, 0x12345678u, 0x00010000u, 0xffff0000u,
	0x8000ffffu, 0xffff8000u,
};
const int kNWords = (int)(sizeof kWords / sizeof kWords[0]);

s64 kQuads[4096];
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
			push_quad((s64)(((u64)kWords[i] << 32) | (u64)kWords[j]));

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

void
targeted_muldi3(void)
{
	/* sign / zero */
	chk_muldi3(0, 0);
	chk_muldi3(0, 1);
	chk_muldi3(1, 0);
	chk_muldi3(1, 1);
	chk_muldi3(-1, 1);
	chk_muldi3(1, -1);
	chk_muldi3(-1, -1);
	chk_muldi3(2, 3);
	chk_muldi3(-2, 3);
	chk_muldi3(2, -3);
	chk_muldi3(-2, -3);

	/* u1==0 && v1==0 fast path (both high 32-bit halves zero) */
	chk_muldi3(0x000000007fffffffLL, 0x000000007fffffffLL);
	chk_muldi3(0x00000000ffffffffu, 0x00000000ffffffffu);
	chk_muldi3(0x0000000080000000u, 0x0000000000000002u);
	chk_muldi3(0x00000000deadbeefu, 0x0000000012345678u);

	/* full path: nonzero high halves */
	chk_muldi3(0x7fffffffffffffffLL, 0x7fffffffffffffffLL);
	chk_muldi3(0x8000000000000000ULL, 0x8000000000000000ULL);
	chk_muldi3(0xffffffffffffffffULL, 0xffffffffffffffffULL);
	chk_muldi3(0x1000000000000000ULL, 0x0000000000000001ULL);
	chk_muldi3(0x0000000100000000ULL, 0x0000000100000000ULL);
	chk_muldi3(0xffff0000ffff0000ULL, 0x0000ffff0000ffffULL);

	/* u1>=u0 / v0>=v1 branch boundaries */
	chk_muldi3(0x0001000000000000ULL, 0x0002000000000000ULL);
	chk_muldi3(0x0002000000000000ULL, 0x0001000000000000ULL);
	chk_muldi3(0x8000000100000001ULL, 0x8000000200000002ULL);
	chk_muldi3(0x0000800000000000ULL, 0x0000400000000000ULL);

	/* __lmulq carry / borrow paths via products */
	chk_muldi3(0x0000ffff0000ffffULL, 0x0000ffff0000ffffULL);
	chk_muldi3(0x0001000000000001ULL, 0x0001000000000001ULL);
	chk_muldi3(0x7fff00007fff0000ULL, 0x00007fff00007fffULL);
	chk_muldi3(0x8000ffff8000ffffULL, 0x8000ffff8000ffffULL);
	chk_muldi3(0x123456789abcdef0ULL, 0xfedcba9876543210ULL);
}

void
targeted_qdivrem(void)
{
	/* u < v */
	chk_qdivrem(0, 1, true);
	chk_qdivrem(1, 2, true);
	chk_qdivrem(0xffffffffu, 0x100000000ULL, true);
	chk_qdivrem(0x123456789abcdef0ULL, 0xfedcba9876543210ULL, true);

	/* exact / remainder */
	chk_qdivrem(0x100000000ULL, 0x10000ULL, true);
	chk_qdivrem(0x123456789abcdef0ULL, 0x10000ULL, true);
	chk_qdivrem(0xffffffffffffffffULL, 1, true);
	chk_qdivrem(0xffffffffffffffffULL, 0xffffffffULL, true);
	chk_qdivrem(0x8000000000000000ULL, 2, true);

	/* n==1 exercise-16 path (vq < 0x10000, nonzero low 16 bits) */
	chk_qdivrem(0x0000000100000000ULL, 0x00000000ffffULL, true);
	chk_qdivrem(0x000000020000abcdULL, 0x00000000fffeULL, true);
	chk_qdivrem(0x0000000300000001ULL, 0x000000000001ULL, true);
	chk_qdivrem(0xffffffffffffffffULL, 0x00000000ffffULL, true);

	/* n=2,3,4 digit lengths */
	chk_qdivrem(0x0001000000000000ULL, 0x0000000100000000ULL, true);
	chk_qdivrem(0x123456789abcdef0ULL, 0x0000000100000001ULL, true);
	chk_qdivrem(0xabcdef0123456789ULL, 0x0123456789abcdefULL, true);

	/* qhat == B (uj0 == v1) */
	chk_qdivrem(0x0001ffff00020000ULL, 0x0000000100000001ULL, true);
	chk_qdivrem(0xffff0000ffff0001ULL, 0x00010000ffff0001ULL, true);

	/* normalization shift d > 0 */
	chk_qdivrem(0x0000800000000001ULL, 0x0000000000008001ULL, true);
	chk_qdivrem(0x0000400000000001ULL, 0x0000000000004001ULL, true);

	/* add-back (rare borrow) stress */
	chk_qdivrem(0x00000000fffffffeULL, 0x00000000ffffffffULL, true);
	chk_qdivrem(0x0000000100000000ULL, 0x00000000fffffffeULL, true);

	/* without remainder pointer */
	chk_qdivrem(0xdeadbeefcafebabeULL, 0x0000000100000001ULL, false);
	chk_qdivrem(0xffffffffffffffffULL, 0x100000000ULL, false);
	chk_qdivrem(0x000000007fffffffULL, 0x00000000ffffULL, false);
}

void
edge_cases(void)
{
	targeted_muldi3();
	targeted_qdivrem();

	for (int i = 0; i < kNQuads; i++) {
		for (int j = 0; j < kNQuads; j++) {
			chk_muldi3(kQuads[i], kQuads[j]);
			u64 u = (u64)kQuads[i];
			u64 v = (u64)kQuads[j];
			if (v == 0)
				continue;
			chk_qdivrem(u, v, true);
			chk_qdivrem(u, v, false);
		}
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

void
random_sweep(void)
{
	for (long i = 0; i < kRandIters; i++) {
		s64 a = rand_quad();
		s64 b = rand_quad();
		chk_muldi3(a, b);

		u64 u = (u64)rand_quad();
		u64 v = (u64)rand_quad();
		if (v == 0)
			v = 1 + (nextrand() & 0xffff);
		chk_qdivrem(u, v, (nextrand() & 1) != 0);
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

	std::printf("b0044 differential test: %d edge quads, "
	    "%ld random iterations\n", kNQuads, kRandIters);

	edge_cases();
	random_sweep();

	std::printf("\n  %-20s %12s %12s   %s\n", "function", "cases",
	    "failures", "status");
	std::printf("  %-20s %12s %12s   %s\n", "--------------------",
	    "------------", "------------", "------");
	report(st_mul);
	report(st_div);

	u64 total = st_mul.fails + st_div.fails;
	u64 cases = st_mul.cases + st_div.cases;

	std::printf("\n  total: %llu cases, %llu failures\n", cases, total);
	if (total != 0) {
		std::printf("  RESULT: FAIL\n");
		return (1);
	}
	std::printf("  RESULT: PASS\n");
	return (0);
}
