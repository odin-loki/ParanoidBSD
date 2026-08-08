// Differential test for PBSD batch b0079s1 (e_lgammaf.c -> lgammaf).
//
// Compares the port against the ref_ oracle on hand-written edge cases and a
// fixed-seed random sweep.  lgammaf writes the sign of Gamma(x) through the
// global signgam; that side effect is checked after every call.

import pbsd.lib.msun.src.b0079s1;

#include <bit>
#include <cstdint>
#include <cstdio>

namespace ns = pbsd::lib_msun_src::b0079s1;

extern "C" {
float ref_lgammaf(float x);
}

extern int signgam;

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

static Stat st_lgammaf = { "lgammaf", 0, 0, 0 };

static bool should_print(Stat &st)
{
	return st.printed++ < 12;
}

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

static std::uint32_t rand_bits(void)
{
	std::uint32_t r = next_u32();

	switch (next_mod(10u)) {
	case 0:
		return r;
	case 1:
		return r & 0x807fffffu;
	case 2:
		return (r & 0x807fffffu) | 0x7f800000u;
	case 3: {
		std::uint32_t d = next_mod(11u) - 5u;
		return (r & 0x80000000u) | ((0x7f800000u + d) & 0x7fffffffu);
	}
	case 4: {
		std::uint32_t d = next_mod(11u) - 5u;
		return (r & 0x80000000u) | ((0x00800000u + d) & 0x7fffffffu);
	}
	case 5: {
		std::uint32_t e = 97u + next_mod(61u);
		return (r & 0x80000000u) | (e << 23) | (r & 0x007fffffu);
	}
	case 6: {
		int k = (int)next_mod(2049u) - 1024;
		return bits_of((float)k / 4.0f);
	}
	case 7: {
		int k = (int)next_mod(401u) - 200;
		return bits_of((float)k);
	}
	case 8: {
		std::uint32_t e = (next_u64() & 1u) ? next_mod(8u)
						   : 248u + next_mod(8u);
		return (r & 0x80000000u) | (e << 23) | (r & 0x007fffffu);
	}
	default:
		return r ^ next_u32();
	}
}

// ------------------------------------------------------------------ lgammaf

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

// --------------------------------------------------------------------- main

int main(void)
{
	for (unsigned i = 0; i < kEdgeN; i++)
		check_lgammaf(kEdge[i]);

	static const std::uint32_t kWalk[] = {
		0x00000000u, 0x00800000u, 0x007fffffu, 0x7f800000u,
		0x7f7fffffu, 0x7fffffffu, 0x80000000u, 0x80800000u,
		0xff800000u, 0xffffffffu, 0x3f800000u, 0xbf800000u,
	};
	for (unsigned k = 0; k < sizeof kWalk / sizeof kWalk[0]; k++) {
		for (int d = -32; d <= 32; d++) {
			std::uint32_t xb = kWalk[k] + (std::uint32_t)d;
			check_lgammaf(xb);
		}
	}

	for (unsigned bit = 0; bit < 32; bit++) {
		std::uint32_t m = 1u << bit;
		check_lgammaf(m);
		check_lgammaf(~m);
	}

	const long long kIters = 200000;
	for (long long i = 0; i < kIters; i++)
		check_lgammaf(rand_bits());

	std::printf("\n%-12s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-12s %12s %12s\n", "------------", "------------",
	    "------------");
	std::printf("%-12s %12lld %12lld\n", st_lgammaf.name,
	    st_lgammaf.cases, st_lgammaf.fails);
	std::printf("%-12s %12s %12s\n", "------------", "------------",
	    "------------");
	std::printf("%-12s %12lld %12lld\n", "TOTAL", st_lgammaf.cases,
	    st_lgammaf.fails);

	int bad = 0;
	if (st_lgammaf.cases == 0) {
		std::printf("%s: no cases were run\n", st_lgammaf.name);
		bad = 1;
	}
	if (st_lgammaf.fails != 0)
		bad = 1;

	std::printf("%s\n", bad ? "FAIL" : "PASS");
	return bad ? 1 : 0;
}
