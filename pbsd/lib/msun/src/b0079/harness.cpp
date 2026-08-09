// Differential test for PBSD batch b0079.
//
// Every function of the port is compared against the ref_ oracle built from
// the unmodified HardenedBSD sources.  Floating point results are compared as
// raw bit patterns so that NaN payloads and the sign of zero are significant.
// The int* output of gammaf_r is written into guarded buffers that are
// compared in their entirety, and the global signgam written by lgammaf is
// compared after every call.

import pbsd.lib.msun.src.b0079;

#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace ns = pbsd::lib_msun_src::b0079;

extern "C" {
float ref_lgammaf(float x);
int ref_finitef(float x);
float ref_gammaf_r(float x, int *signgamp);
float ref_copysignf(float x, float y);
}

// ---------------------------------------------------------------- utilities

static std::uint32_t bits_of(float f)
{
	return std::bit_cast<std::uint32_t>(f);
}

static float float_of(std::uint32_t u)
{
	return std::bit_cast<float>(u);
}

static bool same_float(float a, float b)
{
	return bits_of(a) == bits_of(b);
}

struct Stat {
	const char *name;
	long long cases;
	long long fails;
	long long printed;
};

static Stat st_lgammaf   = { "lgammaf",   0, 0, 0 };
static Stat st_finitef   = { "finitef",   0, 0, 0 };
static Stat st_gammaf_r  = { "gammaf_r",  0, 0, 0 };
static Stat st_copysignf = { "copysignf", 0, 0, 0 };

static bool should_print(Stat &st)
{
	return st.printed++ < 12;
}

// splitmix64: fixed seed, fully reproducible.
static std::uint64_t rng_state = 0x9e3779b97f4a7c15ull;

static std::uint64_t next_u64(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ull);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
	return z ^ (z >> 31);
}

static std::uint32_t next_u32(void)
{
	return (std::uint32_t)(next_u64() >> 21);
}

static std::uint32_t next_mod(std::uint32_t m)
{
	return (std::uint32_t)(next_u64() % m);
}

// Interesting float bit patterns.  Both sides of every boundary that the
// ported code touches (0, the denormal/normal edge, the finite/infinite edge
// at 0x7f800000, FLT_MAX, quiet and signalling NaNs) appear here with both
// signs, together with values that matter to lgammaf_r (integers, the
// negative poles, half and quarter integers).
static const std::uint32_t kEdge[] = {
	0x00000000u, 0x80000000u,	/* +-0			*/
	0x00000001u, 0x80000001u,	/* smallest denormal	*/
	0x00000002u, 0x80000002u,
	0x00400000u, 0x80400000u,
	0x007ffffeu, 0x807ffffeu,
	0x007fffffu, 0x807fffffu,	/* largest denormal	*/
	0x00800000u, 0x80800000u,	/* smallest normal	*/
	0x00800001u, 0x80800001u,
	0x33800000u, 0xb3800000u,	/* +-2^-24		*/
	0x3e800000u, 0xbe800000u,	/* +-0.25		*/
	0x3f000000u, 0xbf000000u,	/* +-0.5		*/
	0x3f800000u, 0xbf800000u,	/* +-1			*/
	0x3fc00000u, 0xbfc00000u,	/* +-1.5		*/
	0x40000000u, 0xc0000000u,	/* +-2			*/
	0x40400000u, 0xc0400000u,	/* +-3			*/
	0x40a00000u, 0xc0a00000u,	/* +-5			*/
	0x41200000u, 0xc1200000u,	/* +-10			*/
	0x42c80000u, 0xc2c80000u,	/* +-100		*/
	0x4b000000u, 0xcb000000u,	/* +-2^23		*/
	0x4b800000u, 0xcb800000u,	/* +-2^24		*/
	0x1f000000u, 0x9f000000u,
	0x5f000000u, 0xdf000000u,
	0x7effffffu, 0xfeffffffu,
	0x7f000000u, 0xff000000u,
	0x7f7ffffeu, 0xff7ffffeu,
	0x7f7fffffu, 0xff7fffffu,	/* +-FLT_MAX		*/
	0x7f800000u, 0xff800000u,	/* +-inf		*/
	0x7f800001u, 0xff800001u,	/* smallest sNaN	*/
	0x7fa5a5a5u, 0xffa5a5a5u,	/* sNaN, high bytes set	*/
	0x7fbfffffu, 0xffbfffffu,	/* largest sNaN		*/
	0x7fc00000u, 0xffc00000u,	/* default qNaN		*/
	0x7fc5a5a5u, 0xffc5a5a5u,
	0x7ffffffeu, 0xfffffffeu,
	0x7fffffffu, 0xffffffffu,	/* all bits set		*/
	0x55555555u, 0xd5555555u,
	0x2aaaaaaau, 0xaaaaaaaau,
	0x0f0f0f0fu, 0x8f0f0f0fu,
	0x70f0f0f0u, 0xf0f0f0f0u,
};
static const unsigned kEdgeN = (unsigned)(sizeof kEdge / sizeof kEdge[0]);

// Random bit pattern generator.  The mix of strategies makes sure that the
// masks and the subtraction inside finitef, and the sign bits inside
// copysignf, are exercised on both sides of every boundary rather than only
// on generic "normal" values.
static std::uint32_t rand_bits(void)
{
	std::uint32_t r = next_u32();

	switch (next_mod(10u)) {
	case 0:
		return r;				/* anything	*/
	case 1:
		return r & 0x807fffffu;			/* zero/denormal */
	case 2:
		return (r & 0x807fffffu) | 0x7f800000u;	/* inf/NaN	*/
	case 3: {					/* near 0x7f800000 */
		std::uint32_t d = next_mod(11u) - 5u;
		return (r & 0x80000000u) | ((0x7f800000u + d) & 0x7fffffffu);
	}
	case 4: {					/* near 0x00800000 */
		std::uint32_t d = next_mod(11u) - 5u;
		return (r & 0x80000000u) | ((0x00800000u + d) & 0x7fffffffu);
	}
	case 5: {					/* |x| in [2^-30,2^30] */
		std::uint32_t e = 97u + next_mod(61u);
		return (r & 0x80000000u) | (e << 23) | (r & 0x007fffffu);
	}
	case 6: {					/* quarter integers */
		int k = (int)next_mod(2049u) - 1024;
		return bits_of((float)k / 4.0f);
	}
	case 7: {					/* exact integers */
		int k = (int)next_mod(401u) - 200;
		return bits_of((float)k);
	}
	case 8: {					/* extreme exponents */
		std::uint32_t e = (next_u64() & 1u) ? next_mod(8u)
						   : 248u + next_mod(8u);
		return (r & 0x80000000u) | (e << 23) | (r & 0x007fffffu);
	}
	default:
		return r ^ next_u32();
	}
}

// ------------------------------------------------------------------ finitef

static void check_finitef(std::uint32_t xb)
{
	float x = float_of(xb);

	st_finitef.cases++;

	int p = ns::finitef(x);
	int r = ref_finitef(x);

	if (p != r) {
		st_finitef.fails++;
		if (should_print(st_finitef))
			std::printf("finitef: x=%#010x port=%d ref=%d\n",
			    xb, p, r);
	}
}

// ---------------------------------------------------------------- copysignf

static void check_copysignf(std::uint32_t xb, std::uint32_t yb)
{
	volatile float xv = float_of(xb);
	volatile float yv = float_of(yb);
	float x = xv;
	float y = yv;

	st_copysignf.cases++;

	float p = ns::copysignf(x, y);
	float r = ref_copysignf(x, y);

	bool ok = same_float(p, r);

	/* Arguments are passed by value; neither side may disturb them. */
	if (bits_of(x) != xb || bits_of(y) != yb)
		ok = false;

	if (!ok) {
		st_copysignf.fails++;
		if (should_print(st_copysignf))
			std::printf("copysignf: x=%#010x y=%#010x "
			    "port=%#010x ref=%#010x (x now %#010x, "
			    "y now %#010x)\n", xb, yb, bits_of(p), bits_of(r),
			    bits_of(x), bits_of(y));
	}
}

// ------------------------------------------------------------------ lgammaf
//
// lgammaf writes the sign of Gamma(x) through &signgam.  signgam is seeded
// with a guard value before each call so that "did not write" is
// distinguishable from "wrote 0".

static void check_lgammaf(std::uint32_t xb)
{
	const int guard = 0x7f7f7f7f;
	float x = float_of(xb);

	st_lgammaf.cases++;

	signgam = guard;
	float p = ns::lgammaf(x);
	int ps = signgam;

	signgam = guard;
	float r = ref_lgammaf(x);
	int rs = signgam;

	if (!same_float(p, r) || ps != rs) {
		st_lgammaf.fails++;
		if (should_print(st_lgammaf))
			std::printf("lgammaf: x=%#010x port=%#010x/%d "
			    "ref=%#010x/%d\n", xb, bits_of(p), ps,
			    bits_of(r), rs);
	}
}

// ----------------------------------------------------------------- gammaf_r
//
// The int* output is placed in the middle of an eight element buffer that is
// pre-filled with the guard byte 0x7f; the whole buffer is compared
// afterwards, so a write outside the nominal window is caught too.

static void check_gammaf_r(std::uint32_t xb)
{
	const int guard = 0x7f7f7f7f;
	float x = float_of(xb);
	int a[8], b[8];

	st_gammaf_r.cases++;

	std::memset(a, 0x7f, sizeof a);
	std::memset(b, 0x7f, sizeof b);

	float p = ns::gammaf_r(x, &a[3]);
	float r = ref_gammaf_r(x, &b[3]);

	bool ok = same_float(p, r) && std::memcmp(a, b, sizeof a) == 0;

	for (unsigned i = 0; i < 8; i++) {
		if (i == 3)
			continue;
		if (a[i] != guard || b[i] != guard)
			ok = false;
	}

	if (!ok) {
		st_gammaf_r.fails++;
		if (should_print(st_gammaf_r)) {
			std::printf("gammaf_r: x=%#010x port=%#010x/%d "
			    "ref=%#010x/%d\n", xb, bits_of(p), a[3],
			    bits_of(r), b[3]);
			std::printf("  port buffer:");
			for (unsigned i = 0; i < 8; i++)
				std::printf(" %#010x", (unsigned)a[i]);
			std::printf("\n  ref  buffer:");
			for (unsigned i = 0; i < 8; i++)
				std::printf(" %#010x", (unsigned)b[i]);
			std::printf("\n");
		}
	}
}

// --------------------------------------------------------------------- main

static void check_all(std::uint32_t xb, std::uint32_t yb)
{
	check_finitef(xb);
	check_lgammaf(xb);
	check_gammaf_r(xb);
	check_copysignf(xb, yb);
}

int main(void)
{
	/* Hand written edge cases. */
	for (unsigned i = 0; i < kEdgeN; i++) {
		check_finitef(kEdge[i]);
		check_lgammaf(kEdge[i]);
		check_gammaf_r(kEdge[i]);
	}

	/* Full cross product of the edge list for the two argument function. */
	for (unsigned i = 0; i < kEdgeN; i++)
		for (unsigned j = 0; j < kEdgeN; j++)
			check_copysignf(kEdge[i], kEdge[j]);

	/* Dense walks across every boundary constant in the ported code. */
	static const std::uint32_t kWalk[] = {
		0x00000000u, 0x00800000u, 0x007fffffu, 0x7f800000u,
		0x7f7fffffu, 0x7fffffffu, 0x80000000u, 0x80800000u,
		0xff800000u, 0xffffffffu, 0x3f800000u, 0xbf800000u,
	};
	for (unsigned k = 0; k < sizeof kWalk / sizeof kWalk[0]; k++) {
		for (int d = -32; d <= 32; d++) {
			std::uint32_t xb = kWalk[k] + (std::uint32_t)d;
			check_finitef(xb);
			check_lgammaf(xb);
			check_gammaf_r(xb);
			check_copysignf(xb, kWalk[k]);
			check_copysignf(kWalk[k], xb);
			check_copysignf(xb, xb ^ 0x80000000u);
		}
	}

	/* Every single sign/exponent combination for copysignf, plus a sweep
	 * of one bit patterns so each bit of both masks is load bearing. */
	for (unsigned s = 0; s < 4; s++) {
		for (unsigned e = 0; e < 256; e++) {
			std::uint32_t xb = ((s & 1u) << 31) | (e << 23) |
			    0x00123456u;
			std::uint32_t yb = ((s >> 1) << 31) | (e << 23) |
			    0x0065432au;
			check_copysignf(xb, yb);
			check_finitef(xb);
			check_finitef(yb);
		}
	}
	for (unsigned bit = 0; bit < 32; bit++) {
		std::uint32_t m = 1u << bit;
		check_copysignf(m, 0u);
		check_copysignf(m, 0x80000000u);
		check_copysignf(0u, m);
		check_copysignf(0x7fffffffu, m);
		check_copysignf(~m, m);
		check_copysignf(m, ~m);
		check_finitef(m);
		check_finitef(~m);
		check_lgammaf(m);
		check_gammaf_r(m);
	}

	/* Fixed seed randomised sweep. */
	const long long kIters = 260000;
	for (long long i = 0; i < kIters; i++)
		check_all(rand_bits(), rand_bits());

	Stat *all[] = { &st_lgammaf, &st_finitef, &st_gammaf_r,
	    &st_copysignf };
	const unsigned nall = (unsigned)(sizeof all / sizeof all[0]);

	long long total_cases = 0;
	long long total_fails = 0;

	std::printf("\n%-12s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-12s %12s %12s\n", "------------", "------------",
	    "------------");
	for (unsigned i = 0; i < nall; i++) {
		std::printf("%-12s %12lld %12lld\n", all[i]->name,
		    all[i]->cases, all[i]->fails);
		total_cases += all[i]->cases;
		total_fails += all[i]->fails;
	}
	std::printf("%-12s %12s %12s\n", "------------", "------------",
	    "------------");
	std::printf("%-12s %12lld %12lld\n", "TOTAL", total_cases,
	    total_fails);

	int bad = 0;
	for (unsigned i = 0; i < nall; i++) {
		if (all[i]->cases == 0) {
			std::printf("%s: no cases were run\n", all[i]->name);
			bad = 1;
		}
		if (all[i]->fails != 0)
			bad = 1;
	}

	std::printf("%s\n", bad ? "FAIL" : "PASS");
	return bad ? 1 : 0;
}
