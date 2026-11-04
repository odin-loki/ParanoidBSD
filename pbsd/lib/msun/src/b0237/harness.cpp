// Differential test for PBSD batch b0237.
//
// Every function of the port is compared against the ref_ oracle built from
// the unmodified HardenedBSD sources.  Floating point results are compared as
// raw bit patterns so that NaN payloads and the sign of zero are significant.
// The int* output of gamma_r is written into guarded buffers that are
// compared in their entirety, and the global signgam written by lgamma is
// compared after every call.

import pbsd.lib.msun.src.b0237;

#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace ns = pbsd::lib_msun_src::b0237;

extern "C" {
float ref_significandf(float x);
double ref_significand(double x);
double ref_gamma_r(double x, int *signgamp);
double ref_lgamma(double x);
}

// ---------------------------------------------------------------- utilities

static std::uint32_t fbits_of(float f)
{
	return std::bit_cast<std::uint32_t>(f);
}

static float float_of(std::uint32_t u)
{
	return std::bit_cast<float>(u);
}

static std::uint64_t dbits_of(double d)
{
	return std::bit_cast<std::uint64_t>(d);
}

static double double_of(std::uint64_t u)
{
	return std::bit_cast<double>(u);
}

static bool same_float(float a, float b)
{
	return fbits_of(a) == fbits_of(b);
}

static bool same_double(double a, double b)
{
	return dbits_of(a) == dbits_of(b);
}

struct Stat {
	const char *name;
	long long cases;
	long long fails;
	long long printed;
};

static Stat st_significandf = { "significandf", 0, 0, 0 };
static Stat st_significand  = { "significand",  0, 0, 0 };
static Stat st_gamma_r      = { "gamma_r",      0, 0, 0 };
static Stat st_lgamma       = { "lgamma",       0, 0, 0 };

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

static std::uint64_t next_u64_hi(void)
{
	return next_u64();
}

static std::uint32_t next_mod(std::uint32_t m)
{
	return (std::uint32_t)(next_u64() % m);
}

static std::uint64_t next_mod64(std::uint64_t m)
{
	return next_u64() % m;
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
	0x0000000000000002ull, 0x8000000000000002ull,
	0x000f000000000000ull, 0x800f000000000000ull,
	0x000fffffffffffffull, 0x800fffffffffffffull,
	0x0010000000000000ull, 0x8010000000000000ull,
	0x0010000000000001ull, 0x8010000000000001ull,
	0x3ca0000000000000ull, 0xbca0000000000000ull,
	0x3fd0000000000000ull, 0xbfd0000000000000ull,
	0x3fe0000000000000ull, 0xbfe0000000000000ull,
	0x3ff0000000000000ull, 0xbff0000000000000ull,
	0x3ff8000000000000ull, 0xbff8000000000000ull,
	0x4000000000000000ull, 0xc000000000000000ull,
	0x4008000000000000ull, 0xc008000000000000ull,
	0x4014000000000000ull, 0xc014000000000000ull,
	0x4024000000000000ull, 0xc024000000000000ull,
	0x4059000000000000ull, 0xc059000000000000ull,
	0x42b0000000000000ull, 0xc2b0000000000000ull,
	0x4330000000000000ull, 0xc330000000000000ull,
	0x4340000000000000ull, 0xc340000000000000ull,
	0x7fefffffffffffffull, 0xffefffffffffffffull,
	0x7ff0000000000000ull, 0xfff0000000000000ull,
	0x7ff0000000000001ull, 0xfff0000000000001ull,
	0x7ff5a5a5a5a5a5a5ull, 0xfff5a5a5a5a5a5a5ull,
	0x7ff7ffffffffffffull, 0xfff7ffffffffffffull,
	0x7ff8000000000000ull, 0xfff8000000000000ull,
	0x7ff8000000000001ull, 0xfff8000000000001ull,
	0x7ffc5a5a5a5a5a5aull, 0xfffc5a5a5a5a5a5aull,
	0x7ffffffffffffffeull, 0xfffffffffffffffeull,
	0x7fffffffffffffffull, 0xffffffffffffffffull,
	0x5555555555555555ull, 0xd555555555555555ull,
	0x2aaaaaaaaaaaaaaaull, 0xaaaaaaaaaaaaaaaull,
	0x0f0f0f0f0f0f0f0full, 0x8f0f0f0f0f0f0f0full,
	0x70f0f0f0f0f0f0f0ull, 0xf0f0f0f0f0f0f0f0ull,
};
static const unsigned kEdgeDN = (unsigned)(sizeof kEdgeD / sizeof kEdgeD[0]);

static std::uint32_t rand_fbits(void)
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
		return fbits_of((float)k / 4.0f);
	}
	case 7: {
		int k = (int)next_mod(401u) - 200;
		return fbits_of((float)k);
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

static std::uint64_t rand_dbits(void)
{
	std::uint64_t r = next_u64_hi();

	switch (next_mod(10u)) {
	case 0:
		return r;
	case 1:
		return r & 0x800fffffffffffffull;
	case 2:
		return (r & 0x800fffffffffffffull) | 0x7ff0000000000000ull;
	case 3: {
		std::uint64_t d = next_mod64(11u) - 5u;
		return (r & 0x8000000000000000ull) |
		    ((0x7ff0000000000000ull + d) & 0x7fffffffffffffffull);
	}
	case 4: {
		std::uint64_t d = next_mod64(11u) - 5u;
		return (r & 0x8000000000000000ull) |
		    ((0x0010000000000000ull + d) & 0x7fffffffffffffffull);
	}
	case 5: {
		std::uint64_t e = 970ull + next_mod64(61u);
		return (r & 0x8000000000000000ull) | (e << 52) |
		    (r & 0x000fffffffffffffull);
	}
	case 6: {
		int k = (int)next_mod64(2049u) - 1024;
		return dbits_of((double)k / 4.0);
	}
	case 7: {
		int k = (int)next_mod64(401u) - 200;
		return dbits_of((double)k);
	}
	case 8: {
		std::uint64_t e = (next_u64() & 1u) ? next_mod64(8u)
						    : 2040ull + next_mod64(8u);
		return (r & 0x8000000000000000ull) | (e << 52) |
		    (r & 0x000fffffffffffffull);
	}
	default:
		return r ^ next_u64_hi();
	}
}

// ------------------------------------------------------------- significandf

static void check_significandf(std::uint32_t xb)
{
	float x = float_of(xb);

	st_significandf.cases++;

	float p = ns::significandf(x);
	float r = ref_significandf(x);

	if (!same_float(p, r)) {
		st_significandf.fails++;
		if (should_print(st_significandf))
			std::printf("significandf: x=%#010x port=%#010x ref=%#010x\n",
			    xb, fbits_of(p), fbits_of(r));
	}
}

// -------------------------------------------------------------- significand

static void check_significand(std::uint64_t xb)
{
	double x = double_of(xb);

	st_significand.cases++;

	double p = ns::significand(x);
	double r = ref_significand(x);

	if (!same_double(p, r)) {
		st_significand.fails++;
		if (should_print(st_significand))
			std::printf("significand: x=%#018llx port=%#018llx "
			    "ref=%#018llx\n", (unsigned long long)xb,
			    (unsigned long long)dbits_of(p),
			    (unsigned long long)dbits_of(r));
	}
}

// ------------------------------------------------------------------ gamma_r

static void check_gamma_r(std::uint64_t xb)
{
	const int guard = 0x7f7f7f7f;
	double x = double_of(xb);
	int a[8], b[8];

	st_gamma_r.cases++;

	std::memset(a, 0x7f, sizeof a);
	std::memset(b, 0x7f, sizeof b);

	double p = ns::gamma_r(x, &a[3]);
	double r = ref_gamma_r(x, &b[3]);

	bool ok = same_double(p, r) && std::memcmp(a, b, sizeof a) == 0;

	for (unsigned i = 0; i < 8; i++) {
		if (i == 3)
			continue;
		if (a[i] != guard || b[i] != guard)
			ok = false;
	}

	if (!ok) {
		st_gamma_r.fails++;
		if (should_print(st_gamma_r)) {
			std::printf("gamma_r: x=%#018llx port=%#018llx/%d "
			    "ref=%#018llx/%d\n", (unsigned long long)xb,
			    (unsigned long long)dbits_of(p), a[3],
			    (unsigned long long)dbits_of(r), b[3]);
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

// ------------------------------------------------------------------- lgamma

static void check_lgamma(std::uint64_t xb)
{
	const int guard = 0x7f7f7f7f;
	double x = double_of(xb);

	st_lgamma.cases++;

	signgam = guard;
	double p = ns::lgamma(x);
	int ps = signgam;

	signgam = guard;
	double r = ref_lgamma(x);
	int rs = signgam;

	if (!same_double(p, r) || ps != rs) {
		st_lgamma.fails++;
		if (should_print(st_lgamma))
			std::printf("lgamma: x=%#018llx port=%#018llx/%d "
			    "ref=%#018llx/%d\n", (unsigned long long)xb,
			    (unsigned long long)dbits_of(p), ps,
			    (unsigned long long)dbits_of(r), rs);
	}
}

// --------------------------------------------------------------------- main

static void check_all(std::uint32_t xfb, std::uint64_t xdb)
{
	check_significandf(xfb);
	check_significand(xdb);
	check_gamma_r(xdb);
	check_lgamma(xdb);
}

int main(void)
{
	for (unsigned i = 0; i < kEdgeFN; i++)
		check_significandf(kEdgeF[i]);

	for (unsigned i = 0; i < kEdgeDN; i++) {
		check_significand(kEdgeD[i]);
		check_gamma_r(kEdgeD[i]);
		check_lgamma(kEdgeD[i]);
	}

	static const std::uint32_t kWalkF[] = {
		0x00000000u, 0x00800000u, 0x007fffffu, 0x7f800000u,
		0x7f7fffffu, 0x7fffffffu, 0x80000000u, 0x80800000u,
		0xff800000u, 0xffffffffu, 0x3f800000u, 0xbf800000u,
	};
	for (unsigned k = 0; k < sizeof kWalkF / sizeof kWalkF[0]; k++) {
		for (int d = -32; d <= 32; d++) {
			std::uint32_t xb = kWalkF[k] + (std::uint32_t)d;
			check_significandf(xb);
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
			check_significand(xb);
			check_gamma_r(xb);
			check_lgamma(xb);
		}
	}

	for (unsigned s = 0; s < 4; s++) {
		for (unsigned e = 0; e < 256; e++) {
			std::uint32_t xb = ((s & 1u) << 31) | (e << 23) |
			    0x00123456u;
			check_significandf(xb);
		}
	}

	for (unsigned s = 0; s < 4; s++) {
		for (unsigned e = 0; e < 2048; e++) {
			std::uint64_t xb = ((std::uint64_t)(s & 1u) << 63) |
			    ((std::uint64_t)e << 52) | 0x00123456789abcull;
			check_significand(xb);
			check_gamma_r(xb);
			check_lgamma(xb);
		}
	}

	for (unsigned bit = 0; bit < 32; bit++) {
		std::uint32_t m = 1u << bit;
		check_significandf(m);
		check_significandf(~m);
	}

	for (unsigned bit = 0; bit < 64; bit++) {
		std::uint64_t m = 1ull << bit;
		check_significand(m);
		check_significand(~m);
		check_gamma_r(m);
		check_lgamma(m);
	}

	const long long kIters = 260000;
	for (long long i = 0; i < kIters; i++) {
		std::uint32_t xfb = rand_fbits();
		std::uint64_t xdb = rand_dbits();
		check_all(xfb, xdb);
	}

	Stat *all[] = { &st_significandf, &st_significand, &st_gamma_r,
	    &st_lgamma };
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
