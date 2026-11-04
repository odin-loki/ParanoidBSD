// Differential test for PBSD batch b0135.
//
// Every function of the port is compared against the ref_ oracle built from
// the unmodified HardenedBSD sources.  Floating point results are compared as
// raw bit patterns so that NaN payloads and the sign of zero are significant.
// The global signgam written by gammaf is compared after every call.

import pbsd.lib.msun.src.b0135;

#include <bit>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace ns = pbsd::lib_msun_src::b0135;

extern "C" {
float ref_scalbnf(float x, int n);
float ref_fabsf(float x);
double ref_copysign(double x, double y);
float ref_gammaf(float x);
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

static std::uint64_t bits_of(double d)
{
	return std::bit_cast<std::uint64_t>(d);
}

static double double_of(std::uint64_t u)
{
	return std::bit_cast<double>(u);
}

static bool same_float(float a, float b)
{
	return bits_of(a) == bits_of(b);
}

static bool same_double(double a, double b)
{
	return bits_of(a) == bits_of(b);
}

struct Stat {
	const char *name;
	long long cases;
	long long fails;
	long long printed;
};

static Stat st_scalbnf  = { "scalbnf",  0, 0, 0 };
static Stat st_fabsf    = { "fabsf",    0, 0, 0 };
static Stat st_copysign = { "copysign", 0, 0, 0 };
static Stat st_gammaf   = { "gammaf",   0, 0, 0 };

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

static const std::uint32_t kEdgeF[] = {
	0x00000000u, 0x80000000u,
	0x00000001u, 0x80000001u,
	0x00000002u, 0x80000002u,
	0x00400000u, 0x80400000u,
	0x007ffffeu, 0x807ffffeu,
	0x007fffffu, 0x807fffffu,
	0x00800000u, 0x80800000u,
	0x00800001u, 0x80800001u,
	0x33800000u, 0xb3800000u,
	0x3e800000u, 0xbe800000u,
	0x3f000000u, 0xbf000000u,
	0x3f800000u, 0xbf800000u,
	0x3fc00000u, 0xbfc00000u,
	0x40000000u, 0xc0000000u,
	0x40400000u, 0xc0400000u,
	0x40a00000u, 0xc0a00000u,
	0x41200000u, 0xc1200000u,
	0x42c80000u, 0xc2c80000u,
	0x4b000000u, 0xcb000000u,
	0x4b800000u, 0xcb800000u,
	0x1f000000u, 0x9f000000u,
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
	0x0f0f0f0fu, 0x8f0f0f0fu,
	0x70f0f0f0u, 0xf0f0f0f0u,
};
static const unsigned kEdgeFN = (unsigned)(sizeof kEdgeF / sizeof kEdgeF[0]);

static const std::uint64_t kEdgeD[] = {
	0x0000000000000000ull, 0x8000000000000000ull,
	0x0000000000000001ull, 0x8000000000000001ull,
	0x000fffffffffffffull, 0x800fffffffffffffull,
	0x0010000000000000ull, 0x8010000000000000ull,
	0x000ffffffffffffeull, 0x800ffffffffffffeull,
	0x3fe0000000000000ull, 0xbfe0000000000000ull,
	0x3ff0000000000000ull, 0xbff0000000000000ull,
	0x3ff8000000000000ull, 0xbff8000000000000ull,
	0x4000000000000000ull, 0xc000000000000000ull,
	0x4008000000000000ull, 0xc008000000000000ull,
	0x4014000000000000ull, 0xc014000000000000ull,
	0x4024000000000000ull, 0xc024000000000000ull,
	0x4059000000000000ull, 0xc059000000000000ull,
	0x40f0000000000000ull, 0xc0f0000000000000ull,
	0x41f0000000000000ull, 0xc1f0000000000000ull,
	0x4350000000000000ull, 0xc350000000000000ull,
	0x7fefffffffffffffull, 0xffefffffffffffffull,
	0x7ff0000000000000ull, 0xfff0000000000000ull,
	0x7ff0000000000001ull, 0xfff0000000000001ull,
	0x7ff5555555555555ull, 0xfff5555555555555ull,
	0x7ff7ffffffffffffull, 0xfff7ffffffffffffull,
	0x7ff8000000000000ull, 0xfff8000000000000ull,
	0x7ffc5a5a5a5a5a5aull, 0xfffc5a5a5a5a5a5aull,
	0x7ffffffffffffffeull, 0xfffffffffffffffeull,
	0x7fffffffffffffffull, 0xffffffffffffffffull,
	0x0123456789abcdefull, 0x8123456789abcdefull,
	0x0fedcba987654321ull, 0x8fedcba987654321ull,
	0x5555555555555555ull, 0xd555555555555555ull,
	0xaaaaaaaaaaaaaaaaull, 0xaaaaaaaaaaaaaaabull,
};
static const unsigned kEdgeDN = (unsigned)(sizeof kEdgeD / sizeof kEdgeD[0]);

static const int kNEdge[] = {
	INT_MIN,
	-1000000, -300000, -10000, -1000, -500, -300, -252, -251, -200,
	-150, -127, -126, -125, -103, -102, -101, -100, -50, -25, -24, -23,
	-2, -1, 0, 1, 2, 23, 24, 25, 50, 100, 101, 102, 103,
	125, 126, 127, 128, 129, 150, 200, 251, 252, 254, 255, 256, 300,
	1000, 10000, 1000000,
	INT_MAX,
};
static const unsigned kNEdgeN = (unsigned)(sizeof kNEdge / sizeof kNEdge[0]);

static std::uint32_t rand_bits_f(void)
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

static std::uint64_t rand_bits_d(void)
{
	std::uint64_t r = next_u64();

	switch (next_mod(12u)) {
	case 0:
		return r;
	case 1:
		return r & 0x800fffffffffffffull;
	case 2:
		return (r & 0x800fffffffffffffull) | 0x7ff0000000000000ull;
	case 3: {
		std::uint64_t d = (std::uint64_t)next_mod(11u) - 5u;
		return (r & 0x8000000000000000ull) |
		    (((0x7ff0000000000000ull + d) & 0x7fffffffffffffffull));
	}
	case 4: {
		std::uint64_t d = (std::uint64_t)next_mod(11u) - 5u;
		return (r & 0x8000000000000000ull) |
		    (((0x0010000000000000ull + d) & 0x7fffffffffffffffull));
	}
	case 5: {
		std::uint64_t e = 970ull + next_mod(61u);
		return (r & 0x8000000000000000ull) | (e << 52) |
		    (r & 0x000fffffffffffffull);
	}
	case 6: {
		int k = (int)next_mod(2049u) - 1024;
		return bits_of((double)k / 4.0);
	}
	case 7: {
		int k = (int)next_mod(401u) - 200;
		return bits_of((double)k);
	}
	case 8: {
		std::uint64_t e = (next_u64() & 1u) ? next_mod(8u)
						    : 2040ull + next_mod(8u);
		return (r & 0x8000000000000000ull) | (e << 52) |
		    (r & 0x000fffffffffffffull);
	}
	default:
		return r ^ next_u64();
	}
}

static int rand_n(void)
{
	switch (next_mod(14u)) {
	case 0:
		return (int)next_u32();
	case 1:
		return 127 + (int)next_mod(41u) - 20;
	case 2:
		return -126 + (int)next_mod(41u) - 20;
	case 3:
		return (int)(next_u64() % 1023ull) - 511;
	case 4:
		return (int)(next_u64() % 511ull) - 255;
	case 5:
		return (int)(next_u64() % 257ull) + 127;
	case 6:
		return -(int)(next_u64() % 257ull) - 126;
	case 7:
		return (int)next_mod(256u);
	case 8:
		return -(int)next_mod(256u) - 1;
	case 9:
		return (int)(INT_MIN + (next_u64() % 1000ull));
	case 10:
		return (int)(INT_MAX - (next_u64() % 1000ull));
	default:
		return (int)(next_u64() ^ (next_u64() >> 32));
	}
}

// ----------------------------------------------------------------- scalbnf

static void check_scalbnf(std::uint32_t xb, int n)
{
	float x = float_of(xb);

	st_scalbnf.cases++;

	float p = ns::scalbnf(x, n);
	float r = ref_scalbnf(x, n);

	if (!same_float(p, r)) {
		st_scalbnf.fails++;
		if (should_print(st_scalbnf))
			std::printf("scalbnf: x=%#010x n=%d port=%#010x "
			    "ref=%#010x\n", xb, n, bits_of(p), bits_of(r));
	}
}

// ------------------------------------------------------------------- fabsf

static void check_fabsf(std::uint32_t xb)
{
	volatile float xv = float_of(xb);
	float x = xv;

	st_fabsf.cases++;

	float p = ns::fabsf(x);
	float r = ref_fabsf(x);

	bool ok = same_float(p, r);

	if (bits_of(x) != xb)
		ok = false;

	if (!ok) {
		st_fabsf.fails++;
		if (should_print(st_fabsf))
			std::printf("fabsf: x=%#010x port=%#010x ref=%#010x "
			    "(x now %#010x)\n", xb, bits_of(p), bits_of(r),
			    bits_of(x));
	}
}

// ---------------------------------------------------------------- copysign

static void check_copysign(std::uint64_t xb, std::uint64_t yb)
{
	volatile double xv = double_of(xb);
	volatile double yv = double_of(yb);
	double x = xv;
	double y = yv;

	st_copysign.cases++;

	double p = ns::copysign(x, y);
	double r = ref_copysign(x, y);

	bool ok = same_double(p, r);

	if (bits_of(x) != xb || bits_of(y) != yb)
		ok = false;

	if (!ok) {
		st_copysign.fails++;
		if (should_print(st_copysign))
			std::printf("copysign: x=%#018llx y=%#018llx "
			    "port=%#018llx ref=%#018llx\n",
			    (unsigned long long)xb, (unsigned long long)yb,
			    (unsigned long long)bits_of(p),
			    (unsigned long long)bits_of(r));
	}
}

// ------------------------------------------------------------------- gammaf

static void check_gammaf(std::uint32_t xb)
{
	const int guard = 0x7f7f7f7f;
	float x = float_of(xb);

	st_gammaf.cases++;

	signgam = guard;
	float p = ns::gammaf(x);
	int ps = signgam;

	signgam = guard;
	float r = ref_gammaf(x);
	int rs = signgam;

	if (!same_float(p, r) || ps != rs) {
		st_gammaf.fails++;
		if (should_print(st_gammaf))
			std::printf("gammaf: x=%#010x port=%#010x/%d "
			    "ref=%#010x/%d\n", xb, bits_of(p), ps,
			    bits_of(r), rs);
	}
}

// --------------------------------------------------------------------- main

static void check_all(std::uint32_t xb, std::uint64_t xdb, std::uint64_t ydb,
    int n)
{
	check_scalbnf(xb, n);
	check_fabsf(xb);
	check_copysign(xdb, ydb);
	check_gammaf(xb);
}

int main(void)
{
	for (unsigned i = 0; i < kEdgeFN; i++) {
		check_fabsf(kEdgeF[i]);
		check_gammaf(kEdgeF[i]);
		for (unsigned j = 0; j < kNEdgeN; j++)
			check_scalbnf(kEdgeF[i], kNEdge[j]);
	}

	for (unsigned i = 0; i < kEdgeDN; i++)
		check_copysign(kEdgeD[i], kEdgeD[i]);

	for (unsigned i = 0; i < kEdgeDN; i++)
		for (unsigned j = 0; j < kEdgeDN; j++)
			check_copysign(kEdgeD[i], kEdgeD[j]);

	static const std::uint32_t kWalkF[] = {
		0x00000000u, 0x00800000u, 0x007fffffu, 0x7f800000u,
		0x7f7fffffu, 0x7fffffffu, 0x80000000u, 0x80800000u,
		0xff800000u, 0xffffffffu, 0x3f800000u, 0xbf800000u,
	};
	for (unsigned k = 0; k < sizeof kWalkF / sizeof kWalkF[0]; k++) {
		for (int d = -32; d <= 32; d++) {
			std::uint32_t xb = kWalkF[k] + (std::uint32_t)d;
			check_fabsf(xb);
			check_gammaf(xb);
			for (int n = 120; n <= 135; n++)
				check_scalbnf(xb, n);
			for (int n = -135; n <= -120; n++)
				check_scalbnf(xb, n);
		}
	}

	static const std::uint64_t kWalkD[] = {
		0x0000000000000000ull, 0x0010000000000000ull,
		0x000fffffffffffffull, 0x7ff0000000000000ull,
		0x7fefffffffffffffull, 0x7fffffffffffffffull,
		0x8000000000000000ull, 0x8010000000000000ull,
		0xfff0000000000000ull, 0xffffffffffffffffull,
		0x3ff0000000000000ull, 0xbff0000000000000ull,
	};
	for (unsigned k = 0; k < sizeof kWalkD / sizeof kWalkD[0]; k++) {
		for (int d = -32; d <= 32; d++) {
			std::uint64_t xb = kWalkD[k] + (std::uint64_t)d;
			check_copysign(xb, kWalkD[k]);
			check_copysign(kWalkD[k], xb);
			check_copysign(xb, xb ^ 0x8000000000000000ull);
		}
	}

	for (unsigned s = 0; s < 4; s++) {
		for (unsigned e = 0; e < 256; e++) {
			std::uint32_t xb = ((s & 1u) << 31) | (e << 23) |
			    0x00123456u;
			check_fabsf(xb);
			check_gammaf(xb);
			check_scalbnf(xb, 127);
			check_scalbnf(xb, 128);
			check_scalbnf(xb, -126);
			check_scalbnf(xb, -127);
		}
	}

	for (unsigned bit = 0; bit < 32; bit++) {
		std::uint32_t m = 1u << bit;
		check_fabsf(m);
		check_fabsf(~m);
		check_gammaf(m);
		check_scalbnf(m, 0);
		check_scalbnf(m, 127);
		check_scalbnf(m, 128);
		check_scalbnf(m, 255);
		check_scalbnf(m, 256);
		check_scalbnf(m, -126);
		check_scalbnf(m, -127);
		check_scalbnf(m, -200);
	}

	for (unsigned bit = 0; bit < 64; bit++) {
		std::uint64_t m = 1ull << bit;
		check_copysign(m, 0ull);
		check_copysign(m, 0x8000000000000000ull);
		check_copysign(0ull, m);
		check_copysign(0x7fffffffffffffffull, m);
		check_copysign(~m, m);
		check_copysign(m, ~m);
	}

	for (unsigned i = 0; i < kNEdgeN; i++) {
		int n = kNEdge[i];
		check_scalbnf(0x3f800000u, n);
		check_scalbnf(0xbf800000u, n);
		check_scalbnf(0x00800000u, n);
		check_scalbnf(0x80800000u, n);
		check_scalbnf(0x7f7fffffu, n);
		check_scalbnf(0xff7fffffu, n);
		check_scalbnf(0x7f800000u, n);
		check_scalbnf(0xff800000u, n);
		check_scalbnf(0x7fc00000u, n);
		check_scalbnf(0x00000000u, n);
		check_scalbnf(0x80000000u, n);
	}

	const long long kIters = 210000;
	for (long long i = 0; i < kIters; i++)
		check_all(rand_bits_f(), rand_bits_d(), rand_bits_d(), rand_n());

	Stat *all[] = { &st_scalbnf, &st_fabsf, &st_copysign, &st_gammaf };
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
