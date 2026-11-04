// Differential test for PBSD batch b0327 (atan, sinhl, exp2f, sqrtl).

#include <cfenv>
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.src.b0327;

namespace port = pbsd::lib_msun_src::b0327;

extern "C" {
double ref_atan(double x);
long double ref_sinhl(long double x);
float ref_exp2f(float x);
long double ref_sqrtl(long double x);
}

#if LDBL_MANT_DIG == 64
static const std::size_t LD_SIG = 10;
#else
static const std::size_t LD_SIG = sizeof(long double);
#endif

static const unsigned char GUARD = 0x7f;
static const std::size_t GUARD_BUF = 64;
static const std::size_t GUARD_OFF = 16;
static const unsigned MAX_REPORT = 12;
static const long long RANDOM_ITERS = 50000;

struct Stat {
	const char *name;
	long long cases;
	long long fails;
	long long printed;
};

static Stat st_atan = { "atan", 0, 0, 0 };
static Stat st_sinhl = { "sinhl", 0, 0, 0 };
static Stat st_exp2f = { "exp2f", 0, 0, 0 };
static Stat st_sqrtl = { "sqrtl", 0, 0, 0 };

static bool should_print(Stat &st)
{
	return st.printed++ < MAX_REPORT;
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
	std::uint32_t u;
	std::memcpy(&u, &f, sizeof(f));
	return u;
}

static float fromfbits(std::uint32_t u)
{
	float f;
	std::memcpy(&f, &u, sizeof(f));
	return f;
}

static std::uint64_t dbits(double d)
{
	std::uint64_t u;
	std::memcpy(&u, &d, sizeof(u));
	return u;
}

static double fromdbits(std::uint64_t u)
{
	double d;
	std::memcpy(&d, &u, sizeof(d));
	return d;
}

struct ldrep {
	unsigned char b[sizeof(long double)];
};

static ldrep ldbits(long double x)
{
	ldrep r;
	std::memset(r.b, 0, sizeof(r.b));
	std::memcpy(r.b, &x, LD_SIG);
	return r;
}

static long double mkld(std::uint16_t se, std::uint64_t m)
{
	unsigned char b[sizeof(long double)];
	long double x;
	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &m, sizeof(m));
	std::memcpy(b + 8, &se, sizeof(se));
	std::memcpy(&x, b, sizeof(x));
	return x;
}

static std::uint64_t rng_state = 0xb0327a5a5a5a5a5aull;

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

static void check_atan(double x, const char *tag)
{
	std::uint64_t p, o;

	st_atan.cases++;
	p = dbits(port::atan(x));
	o = dbits(ref_atan(x));
	if (guarded_equal(&p, &o, sizeof(p)))
		return;
	st_atan.fails++;
	if (should_print(st_atan))
		std::printf("atan FAIL [%s] x=%#018llx port=%#018llx ref=%#018llx\n",
		    tag, (unsigned long long)dbits(x), (unsigned long long)p,
		    (unsigned long long)o);
}

static void check_sinhl(long double x, const char *tag)
{
	ldrep p, o;

	st_sinhl.cases++;
	p = ldbits(port::sinhl(x));
	o = ldbits(ref_sinhl(x));
	if (guarded_equal(&p, &o, sizeof(p)))
		return;
	st_sinhl.fails++;
	if (should_print(st_sinhl)) {
		ldrep xb = ldbits(x);
		std::printf("sinhl FAIL [%s] x=", tag);
		for (std::size_t i = LD_SIG; i-- > 0;)
			std::printf("%02x", xb.b[i]);
		std::printf(" port=");
		for (std::size_t i = LD_SIG; i-- > 0;)
			std::printf("%02x", p.b[i]);
		std::printf(" ref=");
		for (std::size_t i = LD_SIG; i-- > 0;)
			std::printf("%02x", o.b[i]);
		std::printf("\n");
	}
}

static void check_exp2f(float x, const char *tag)
{
	std::uint32_t p, o;

	st_exp2f.cases++;
	p = fbits(port::exp2f(x));
	o = fbits(ref_exp2f(x));
	if (guarded_equal(&p, &o, sizeof(p)))
		return;
	st_exp2f.fails++;
	if (should_print(st_exp2f))
		std::printf("exp2f FAIL [%s] x=%#010x port=%#010x ref=%#010x\n",
		    tag, fbits(x), p, o);
}

static void check_sqrtl(long double x, const char *tag)
{
	ldrep p, o;

	st_sqrtl.cases++;
	p = ldbits(port::sqrtl(x));
	o = ldbits(ref_sqrtl(x));
	if (guarded_equal(&p, &o, sizeof(p)))
		return;
	st_sqrtl.fails++;
	if (should_print(st_sqrtl)) {
		ldrep xb = ldbits(x);
		std::printf("sqrtl FAIL [%s] x=", tag);
		for (std::size_t i = LD_SIG; i-- > 0;)
			std::printf("%02x", xb.b[i]);
		std::printf(" port=");
		for (std::size_t i = LD_SIG; i-- > 0;)
			std::printf("%02x", p.b[i]);
		std::printf(" ref=");
		for (std::size_t i = LD_SIG; i-- > 0;)
			std::printf("%02x", o.b[i]);
		std::printf("\n");
	}
}

static std::uint64_t rand_dbits(void)
{
	std::uint64_t r = next_u64();
	switch (next_mod(10u)) {
	case 0:
		return r;
	case 1:
		return r & 0x800fffffffffffffull;
	case 2:
		return (r & 0x800fffffffffffffull) | 0x7ff0000000000000ull;
	case 3: {
		std::uint64_t e = 1000u + next_mod(100u);
		return (r & 0x8000000000000000ull) | (e << 52) |
		    (r & 0x000fffffffffffffull);
	}
	case 4:
		return (r & 0x8000000000000000ull) |
		    ((0x7ff0000000000000ull + next_mod(17u) - 8u) &
		    0x7fffffffffffffffull);
	case 5:
		return r ^ (1ull << (next_mod(64u)));
	default:
		return r ^ next_u64();
	}
}

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
		std::uint32_t e = 118u + next_mod(61u);
		return (r & 0x80000000u) | (e << 23) | (r & 0x007fffffu);
	}
	case 4:
		return (r & 0x80000000u) |
		    ((0x7f800000u + next_mod(17u) - 8u) & 0x7fffffffu);
	case 5:
		return r ^ (1u << (next_mod(32u)));
	default:
		return r ^ next_u32();
	}
}

static long double rand_ld(void)
{
	std::uint16_t se = (std::uint16_t)(next_u64() & 0xffffu);
	std::uint64_t m = next_u64();
	switch (next_mod(12u)) {
	case 0:
		return mkld(se, m);
	case 1:
		se = (std::uint16_t)((se & 0x8000u) |
		    (0x3fffu + (next_mod(9u) - 4u)));
		return mkld(se, m);
	case 2:
		se = (std::uint16_t)((se & 0x8000u) |
		    (0x3ffeu + (next_mod(5u) - 2u)));
		return mkld(se, m | 0x8000000000000000ull);
	case 3:
		se = (std::uint16_t)((se & 0x8000u) | 0x7fffu);
		return mkld(se, m);
	case 4:
		se = (std::uint16_t)((se & 0x8000u) | 0x4005u);
		return mkld(se, m);
	case 5:
		se = (std::uint16_t)((se & 0x8000u) | 0x0000u);
		return mkld(se, m);
	case 6:
		return mkld(0x3fffu, m);
	case 7:
		return mkld(0xbfffu, m);
	case 8:
		return mkld(0x4000u, m);
	case 9:
		return mkld(0xc000u, m);
	default:
		return mkld(se, m ^ next_u64());
	}
}

static void run_atan_tests(void)
{
	const std::uint64_t dvec[] = {
		0x0000000000000000ull, 0x8000000000000000ull,
		0x0000000000000001ull, 0x8000000000000001ull,
		0x3ca0000000000000ull, 0xbca0000000000000ull,
		0x3dc0000000000000ull, 0xbdc0000000000000ull,
		0x3fdc000000000000ull, 0xbfdc000000000000ull,
		0x3fdc000000000001ull, 0xbfdc000000000001ull,
		0x3fe6000000000000ull, 0xbfe6000000000000ull,
		0x3fe6000000000001ull, 0xbfe6000000000001ull,
		0x3ff3000000000000ull, 0xbff3000000000000ull,
		0x3ff3000000000001ull, 0xbff3000000000001ull,
		0x4003800000000000ull, 0xc003800000000000ull,
		0x4003800000000001ull, 0xc003800000000001ull,
		0x4410000000000000ull, 0xc410000000000000ull,
		0x4410000000000001ull, 0xc410000000000001ull,
		0x7ff0000000000000ull, 0xfff0000000000000ull,
		0x7ff0000000000001ull, 0xfff0000000000001ull,
		0x7ff8000000000000ull, 0xfff8000000000000ull,
		0x3fd5555555555555ull, 0xbfd5555555555555ull,
		0x3ff921fb54442d18ull, 0xbff921fb54442d18ull,
		0x4059000000000000ull, 0xc059000000000000ull,
	};
	for (std::uint64_t u : dvec)
		check_atan(fromdbits(u), "dvec");

	for (int d = -8; d <= 8; d++) {
		const std::uint64_t bases[] = {
			0x3fdc000000000000ull, 0x3fe6000000000000ull,
			0x3ff3000000000000ull, 0x4003800000000000ull,
			0x4410000000000000ull, 0x3e40000000000000ull,
		};
		for (std::uint64_t b : bases) {
			std::uint64_t u = b + (std::uint64_t)d;
			check_atan(fromdbits(u), "walk");
			check_atan(fromdbits(u ^ 0x8000000000000000ull), "walk-neg");
		}
	}

	for (unsigned bit = 0; bit < 64; bit++) {
		std::uint64_t m = 1ull << bit;
		check_atan(fromdbits(m), "bit");
		check_atan(fromdbits(m | 0x8000000000000000ull), "bit-neg");
	}

	for (long long n = 0; n < RANDOM_ITERS; n++)
		check_atan(fromdbits(rand_dbits()), "rand");
}

static void run_sinhl_tests(void)
{
	const std::uint16_t se_walk[] = {
		0x0000u, 0x8000u, 0x3ffeu, 0x3fffu, 0x4000u, 0x4004u, 0x4005u,
		0x4006u, 0x7fffu, 0xffffu, 0xbffeu, 0xbfffu, 0xc000u, 0xc004u,
	};
	for (std::uint16_t se : se_walk) {
		check_sinhl(mkld(se, 0), "se-zero");
		check_sinhl(mkld(se, 0x8000000000000000ull), "se-one");
		check_sinhl(mkld(se, 0xffffffffffffffffull), "se-all");
		check_sinhl(mkld(se, next_u64()), "se-rand");
	}

	for (int d = -64; d <= 64; d++) {
		std::uint16_t se = (std::uint16_t)(0x3fffu + d);
		check_sinhl(mkld(se, next_u64()), "one-walk");
		check_sinhl(mkld((std::uint16_t)(se | 0x8000u), next_u64()),
		    "one-walk-neg");
		se = (std::uint16_t)(0x4005u + d);
		check_sinhl(mkld(se, next_u64()), "64-walk");
		check_sinhl(mkld((std::uint16_t)(se | 0x8000u), next_u64()),
		    "64-walk-neg");
	}

	check_sinhl(0.0L, "zero");
	check_sinhl(-0.0L, "neg-zero");
	check_sinhl(0.5L, "half");
	check_sinhl(-0.5L, "neg-half");
	check_sinhl(1.0L, "one");
	check_sinhl(-1.0L, "neg-one");
	check_sinhl(63.0L, "below64");
	check_sinhl(-63.0L, "neg-below64");
	check_sinhl(64.0L, "at64");
	check_sinhl(-64.0L, "neg-at64");
	check_sinhl(11357.0L, "near-thresh");
	check_sinhl(-11357.0L, "neg-near-thresh");
	check_sinhl(20000.0L, "overflow");
	check_sinhl(-20000.0L, "neg-overflow");

	for (long long n = 0; n < RANDOM_ITERS; n++)
		check_sinhl(rand_ld(), "rand");
}

static void run_exp2f_tests(void)
{
	const std::uint32_t fvec[] = {
		0x00000000u, 0x80000000u,
		0x00000001u, 0x80000001u,
		0x33000000u, 0xb3000000u,
		0x33000001u, 0xb3000001u,
		0x3f000000u, 0xbf000000u,
		0x3f800000u, 0xbf800000u,
		0x42fffffeu, 0xc2fffffeu,
		0x43000000u, 0xc3000000u,
		0x43000001u, 0xc3000001u,
		0x42c00000u, 0xc2c00000u,
		0x42c00001u, 0xc2c00001u,
		0x42bfffffu, 0xc2bfffffu,
		0x4b000000u, 0xcb000000u,
		0x7f000000u, 0xff000000u,
		0x7f800000u, 0xff800000u,
		0x7f800001u, 0xff800001u,
		0x7fc00000u, 0xffc00000u,
		0x3b800000u, 0xbb800000u,
		0x4a800000u, 0xca800000u,
	};
	for (std::uint32_t u : fvec)
		check_exp2f(fromfbits(u), "fvec");

	for (int d = -8; d <= 8; d++) {
		const std::uint32_t bases[] = {
			0x33000000u, 0x43000000u, 0x42c00000u, 0x7f800000u,
		};
		for (std::uint32_t b : bases) {
			std::uint32_t u = b + (std::uint32_t)d;
			check_exp2f(fromfbits(u), "walk");
			check_exp2f(fromfbits(u ^ 0x80000000u), "walk-neg");
		}
	}

	for (unsigned bit = 0; bit < 32; bit++) {
		std::uint32_t m = 1u << bit;
		check_exp2f(fromfbits(m), "bit");
		check_exp2f(fromfbits(m | 0x80000000u), "bit-neg");
	}

	for (long long n = 0; n < RANDOM_ITERS; n++)
		check_exp2f(fromfbits(rand_fbits()), "rand");
}

static void run_sqrtl_tests(void)
{
	const std::uint16_t se_walk[] = {
		0x0000u, 0x8000u, 0x0001u, 0x8001u, 0x3ffeu, 0xbffeu,
		0x3fffu, 0xbfffu, 0x4000u, 0xc000u, 0x4001u, 0xc001u,
		0x7ffeu, 0xfffeu, 0x7fffu, 0xffffu,
	};
	for (std::uint16_t se : se_walk) {
		check_sqrtl(mkld(se, 0), "se-zero");
		check_sqrtl(mkld(se, 0x8000000000000000ull), "se-one");
		check_sqrtl(mkld(se, 0xffffffffffffffffull), "se-all");
		check_sqrtl(mkld(se, next_u64()), "se-rand");
	}

	check_sqrtl(0.0L, "zero");
	check_sqrtl(-0.0L, "neg-zero");
	check_sqrtl(1.0L, "one");
	check_sqrtl(4.0L, "four");
	check_sqrtl(2.0L, "two");
	check_sqrtl(-1.0L, "neg");
	check_sqrtl(INFINITY, "inf");
	check_sqrtl(-INFINITY, "neg-inf");
	check_sqrtl(NAN, "nan");

	for (int d = -32; d <= 32; d++) {
		std::uint16_t se = (std::uint16_t)(0x3fffu + d);
		check_sqrtl(mkld(se, 0x8000000000000000ull), "norm-walk");
		check_sqrtl(mkld((std::uint16_t)(se | 0x8000u),
		    0x8000000000000000ull), "norm-walk-neg");
	}

	const int rounds[] = { FE_TONEAREST, FE_TOWARDZERO, FE_UPWARD, FE_DOWNWARD };
	for (int r : rounds) {
		int old = std::fegetround();
		std::fesetround(r);
		for (long long n = 0; n < 5000; n++) {
			long double x = rand_ld();
			if ((next_u64() & 15u) == 0u)
				x = mkld((std::uint16_t)(0x3fffu + (next_mod(9u) - 4u)),
				    next_u64() | 0x8000000000000000ull);
			check_sqrtl(x, "round");
		}
		std::fesetround(old);
	}

	for (long long n = 0; n < RANDOM_ITERS; n++)
		check_sqrtl(rand_ld(), "rand");
}

int main(void)
{
	run_atan_tests();
	run_sinhl_tests();
	run_exp2f_tests();
	run_sqrtl_tests();

	Stat *all[] = { &st_atan, &st_sinhl, &st_exp2f, &st_sqrtl };
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
