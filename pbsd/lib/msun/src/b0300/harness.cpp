// Differential test for PBSD batch b0300 (asinhf).

import pbsd.lib.msun.src.b0300;

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace port = pbsd::lib_msun_src::b0300;

extern "C" {
float ref_asinhf(float x);
}

static const unsigned char GUARD = 0x7f;
static const std::size_t GUARD_BUF = 64;
static const std::size_t GUARD_OFF = 16;

struct Stat {
	const char *name;
	long long cases;
	long long fails;
	long long printed;
};

static Stat st_asinhf = { "asinhf", 0, 0, 0 };

static bool should_print(Stat &st)
{
	return st.printed++ < 12;
}

static bool guarded_equal(const void *pa, const void *pb, std::size_t n)
{
	unsigned char a[GUARD_BUF], b[GUARD_BUF];

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	std::memcpy(a + GUARD_OFF, pa, n);
	std::memcpy(b + GUARD_OFF, pb, n);
	return std::memcmp(a, b, sizeof(a)) == 0;
}

static std::uint32_t fbits(float f)
{
	return std::bit_cast<std::uint32_t>(f);
}

static float fromfbits(std::uint32_t u)
{
	return std::bit_cast<float>(u);
}

static std::uint64_t rng_state = 0xb0300a5a5a5a5a5aull;

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

static void check_asinhf(std::uint32_t xb, const char *tag)
{
	float x = fromfbits(xb);
	std::uint32_t p, o;

	st_asinhf.cases++;

	p = fbits(port::asinhf(x));
	o = fbits(ref_asinhf(x));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_asinhf.fails++;
	if (should_print(st_asinhf))
		std::printf("asinhf FAIL [%s] x=%#010x port=%#010x ref=%#010x\n",
		    tag, xb, p, o);
}

static const std::uint32_t fvec[] = {
	0x00000000u, 0x80000000u,
	0x00000001u, 0x80000001u,
	0x00000002u, 0x80000002u,
	0x00400000u, 0x80400000u,
	0x007ffffeu, 0x807ffffeu,
	0x007fffffu, 0x807fffffu,
	0x00800000u, 0x80800000u,
	0x00ffffffu, 0x80ffffffu,
	0x0f0f0f0fu, 0x8f0f0f0fu,
	0x317fffffu, 0xb17fffffu,
	0x31800000u, 0xb1800000u,
	0x31800001u, 0xb1800001u,
	0x33800000u, 0xb3800000u,
	0x3f000000u, 0xbf000000u,
	0x3f800000u, 0xbf800000u,
	0x3fffffffu, 0xbfffffffu,
	0x40000000u, 0xc0000000u,
	0x40000001u, 0xc0000001u,
	0x40400000u, 0xc0400000u,
	0x40a00000u, 0xc0a00000u,
	0x41200000u, 0xc1200000u,
	0x42c80000u, 0xc2c80000u,
	0x4b000000u, 0xcb000000u,
	0x4d7fffffu, 0xcd7fffffu,
	0x4d800000u, 0xcd800000u,
	0x4d800001u, 0xcd800001u,
	0x4b800000u, 0xcb800000u,
	0x5f000000u, 0xdf000000u,
	0x7effffffu, 0xfeffffffu,
	0x7f000000u, 0xff000000u,
	0x7f7ffffeu, 0xff7ffffeu,
	0x7f7fffffu, 0xff7fffffu,
	0x7f800000u, 0xff800000u,
	0x7f800001u, 0xff800001u,
	0x7fa5a5a5u, 0xffa5a5a5u,
	0x7fbfffffu, 0xffbfffffu,
	0x7fc00000u, 0xffc00000u,
	0x7fc5a5a5u, 0xffc5a5a5u,
	0x7ffffffeu, 0xfffffffeu,
	0x7fffffffu, 0xffffffffu,
	0x55555555u, 0xd5555555u,
	0x2aaaaaaau, 0xaaaaaaaau,
	0x70f0f0f0u, 0xf0f0f0f0u,
	0x80808080u, 0x7f7f7f7fu,
	0x00ff00ffu, 0xff00ff00u,
};
static const unsigned NFVEC = (unsigned)(sizeof fvec / sizeof fvec[0]);

static const std::uint32_t kWalkF[] = {
	0x00000000u, 0x31800000u,
	0x317fffffu, 0x31800001u,
	0x40000000u, 0x3fffffffu,
	0x40000001u, 0x4d7fffffu,
	0x4d800000u, 0x4d800001u,
	0x7f800000u, 0xff800000u,
};

static std::uint32_t rand_fbits(void)
{
	std::uint32_t r = next_u32();

	switch (next_mod(12u)) {
	case 0:
		return r;
	case 1:
		return r & 0x807fffffu;
	case 2:
		return (r & 0x807fffffu) | 0x7f800000u;
	case 3: {
		std::uint32_t d = next_mod(11u) - 5u;
		return (r & 0x80000000u) |
		    ((0x7f800000u + d) & 0x7fffffffu);
	}
	case 4: {
		std::uint32_t d = next_mod(11u) - 5u;
		return (r & 0x80000000u) |
		    ((0x00800000u + d) & 0x7fffffffu);
	}
	case 5: {
		std::uint32_t e = 118u + next_mod(61u);
		return (r & 0x80000000u) | (e << 23) |
		    (r & 0x007fffffu);
	}
	case 6: {
		int k = (int)next_mod(2049u) - 1024;
		return fbits((float)k / 4.0f);
	}
	case 7: {
		int k = (int)next_mod(401u) - 200;
		return fbits((float)k);
	}
	case 8: {
		std::uint32_t e = (next_u64() & 1u) ? next_mod(8u)
						    : 248u + next_mod(8u);
		return (r & 0x80000000u) | (e << 23) |
		    (r & 0x007fffffu);
	}
	case 9:
		return 0x31800000u ^ (r & 0x007fffffu);
	case 10:
		return 0x4d800000u ^ (r & 0x007fffffu);
	default:
		return r ^ next_u32();
	}
}

int main(void)
{
	unsigned i, j;

	for (i = 0; i < NFVEC; i++)
		check_asinhf(fvec[i], "fvec");

	for (unsigned k = 0; k < sizeof kWalkF / sizeof kWalkF[0]; k++) {
		for (int d = -64; d <= 64; d++) {
			std::uint32_t xb = kWalkF[k] + (std::uint32_t)d;
			check_asinhf(xb, "walk");
		}
	}

	for (unsigned s = 0; s < 2; s++) {
		for (unsigned e = 0; e < 256; e++) {
			std::uint32_t xb = ((std::uint32_t)s << 31) |
			    ((std::uint32_t)e << 23) | 0x00123456u;
			check_asinhf(xb, "exp-sweep");
		}
	}

	for (unsigned bit = 0; bit < 32; bit++) {
		std::uint32_t m = 1u << bit;
		check_asinhf(m, "bit");
		check_asinhf(~m, "bit-not");
	}

	for (unsigned e = 0; e < 0x100u; e += 4u) {
		for (unsigned s = 0; s < 2; s++) {
			std::uint32_t xb = ((std::uint32_t)(s & 1u) << 31) |
			    (e << 23) | 0x808080u;
			check_asinhf(xb, "exp-grid");
		}
	}

	for (unsigned t = 0; t < 0x100u; t++) {
		check_asinhf(0x31800000u ^ t, "small-boundary");
		check_asinhf(0xb1800000u ^ t, "small-boundary-neg");
		check_asinhf(0x40000000u ^ t, "two-boundary");
		check_asinhf(0xc0000000u ^ t, "two-boundary-neg");
		check_asinhf(0x4d800000u ^ t, "large-boundary");
		check_asinhf(0xcd800000u ^ t, "large-boundary-neg");
	}

	for (i = 0; i < NFVEC; i++) {
		for (j = 0; j < NFVEC; j++) {
			std::uint32_t xb = fvec[i] ^ (fvec[j] & 0x007fffffu);
			check_asinhf(xb, "cross");
		}
	}

	const long long kIters = 50000;
	for (long long n = 0; n < kIters; n++)
		check_asinhf(rand_fbits(), "rand");

	Stat *all[] = { &st_asinhf };
	const unsigned nall = (unsigned)(sizeof all / sizeof all[0]);
	long long total_cases = 0;
	long long total_fails = 0;

	std::printf("\n%-12s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-12s %12s %12s\n", "------------", "------------",
	    "------------");
	for (unsigned k = 0; k < nall; k++) {
		std::printf("%-12s %12lld %12lld\n", all[k]->name,
		    all[k]->cases, all[k]->fails);
		total_cases += all[k]->cases;
		total_fails += all[k]->fails;
	}
	std::printf("%-12s %12s %12s\n", "------------", "------------",
	    "------------");
	std::printf("%-12s %12lld %12lld\n", "TOTAL", total_cases,
	    total_fails);

	int bad = 0;
	for (unsigned k = 0; k < nall; k++) {
		if (all[k]->cases == 0) {
			std::printf("%s: no cases were run\n", all[k]->name);
			bad = 1;
		}
		if (all[k]->fails != 0)
			bad = 1;
	}

	std::printf("%s\n", bad ? "FAIL" : "PASS");
	return bad ? 1 : 0;
}
