// Differential test for PBSD batch b0303 (__signbit, __signbitf, __signbitl,
// atanhl, cproj).

#include <cfloat>
#include <climits>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.src.b0303;

namespace port = pbsd::lib_msun_src::b0303;

extern "C" {
int ref___signbit(double d);
int ref___signbitf(float f);
int ref___signbitl(long double e);
long double ref_atanhl(long double x);
double _Complex ref_cproj(double _Complex z);
}

#if LDBL_MANT_DIG == 64
static const std::size_t LD_SIG = 10;
static const int EXP_TINY = -34;
#else
static const std::size_t LD_SIG = sizeof(long double);
static const int EXP_TINY = -58;
#endif

static const int LD_BIAS = LDBL_MAX_EXP - 1;
static const unsigned LD_TINY_IX = (unsigned)(LD_BIAS + EXP_TINY);

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

static Stat st_signbit = { "__signbit", 0, 0, 0 };
static Stat st_signbitf = { "__signbitf", 0, 0, 0 };
static Stat st_signbitl = { "__signbitl", 0, 0, 0 };
static Stat st_atanhl = { "atanhl", 0, 0, 0 };
static Stat st_cproj = { "cproj", 0, 0, 0 };

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
	std::memcpy(&u, &f, sizeof(u));
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

static double _Complex mkdc(double re, double im)
{
	double _Complex z;

	__real__ z = re;
	__imag__ z = im;
	return z;
}

static std::uint64_t rng_state = 0xb0303a5a5a5a5a5aull;

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

static void check_signbit(double x, const char *tag)
{
	int p, o;

	st_signbit.cases++;

	p = port::__signbit(x);
	o = ref___signbit(x);

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_signbit.fails++;
	if (should_print(st_signbit))
		std::printf("__signbit FAIL [%s] x=%#018llx port=%d ref=%d\n",
		    tag, (unsigned long long)dbits(x), p, o);
}

static void check_signbitf(float x, const char *tag)
{
	int p, o;

	st_signbitf.cases++;

	p = port::__signbitf(x);
	o = ref___signbitf(x);

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_signbitf.fails++;
	if (should_print(st_signbitf))
		std::printf("__signbitf FAIL [%s] x=%#010x port=%d ref=%d\n",
		    tag, fbits(x), p, o);
}

static void check_signbitl(long double x, const char *tag)
{
	int p, o;

	st_signbitl.cases++;

	p = port::__signbitl(x);
	o = ref___signbitl(x);

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_signbitl.fails++;
	if (should_print(st_signbitl)) {
		ldrep r = ldbits(x);
		std::printf("__signbitl FAIL [%s] x=", tag);
		for (std::size_t i = LD_SIG; i-- > 0;)
			std::printf("%02x", r.b[i]);
		std::printf(" port=%d ref=%d\n", p, o);
	}
}

static void check_atanhl(long double x, const char *tag)
{
	ldrep p, o;

	st_atanhl.cases++;

	p = ldbits(port::atanhl(x));
	o = ldbits(ref_atanhl(x));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_atanhl.fails++;
	if (should_print(st_atanhl)) {
		std::size_t i;
		ldrep xb = ldbits(x);

		std::printf("atanhl FAIL [%s] x=", tag);
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", xb.b[i]);
		std::printf(" port=");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", p.b[i]);
		std::printf(" ref=");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", o.b[i]);
		std::printf("\n");
	}
}

static void check_cproj(double re, double im, const char *tag)
{
	std::uint64_t gp[2], go[2];
	std::complex<double> pz;
	double _Complex oz;

	st_cproj.cases++;

	pz = port::cproj(std::complex<double>(re, im));
	oz = ref_cproj(mkdc(re, im));

	gp[0] = dbits(pz.real());
	gp[1] = dbits(pz.imag());
	go[0] = dbits(__real__ oz);
	go[1] = dbits(__imag__ oz);

	if (guarded_equal(gp, go, sizeof(gp)))
		return;

	st_cproj.fails++;
	if (should_print(st_cproj))
		std::printf("cproj FAIL [%s] re=%#018llx im=%#018llx "
			    "port=(%#018llx,%#018llx) ref=(%#018llx,%#018llx)\n",
		    tag, (unsigned long long)dbits(re),
		    (unsigned long long)dbits(im),
		    (unsigned long long)gp[0], (unsigned long long)gp[1],
		    (unsigned long long)go[0], (unsigned long long)go[1]);
}

static const std::uint32_t fvec[] = {
	0x00000000u, 0x80000000u,
	0x00000001u, 0x80000001u,
	0x00400000u, 0x80400000u,
	0x007ffffeu, 0x807ffffeu,
	0x007fffffu, 0x807fffffu,
	0x00800000u, 0x80800000u,
	0x0f0f0f0fu, 0x8f0f0f0fu,
	0x3f000000u, 0xbf000000u,
	0x3f800000u, 0xbf800000u,
	0x3fffffffu, 0xbfffffffu,
	0x40000000u, 0xc0000000u,
	0x7effffffu, 0xfeffffffu,
	0x7f000000u, 0xff000000u,
	0x7f7ffffeu, 0xff7ffffeu,
	0x7f7fffffu, 0xff7fffffu,
	0x7f800000u, 0xff800000u,
	0x7f800001u, 0xff800001u,
	0x7fc00000u, 0xffc00000u,
	0x7fffffffu, 0xffffffffu,
	0x55555555u, 0xd5555555u,
	0x80808080u, 0x7f7f7f7fu,
};
static const unsigned NFVEC = (unsigned)(sizeof fvec / sizeof fvec[0]);

static const std::uint64_t dvec[] = {
	0x0000000000000000ull, 0x8000000000000000ull,
	0x0000000000000001ull, 0x8000000000000001ull,
	0x000fffffffffffffull, 0x800fffffffffffffull,
	0x0010000000000000ull, 0x8010000000000000ull,
	0x3fefffffffffffffull, 0xbfefffffffffffffull,
	0x3ff0000000000000ull, 0xbff0000000000000ull,
	0x3ff0000000000001ull, 0xbff0000000000001ull,
	0x3fffffffffffffffull, 0xbfffffffffffffffull,
	0x7fefffffffffffffull, 0xffefffffffffffffull,
	0x7ff0000000000000ull, 0xfff0000000000000ull,
	0x7ff0000000000001ull, 0xfff0000000000001ull,
	0x7ff8000000000000ull, 0xfff8000000000000ull,
	0x7fffffffffffffffull, 0xffffffffffffffffull,
	0xdeadbeefcafebabeull, 0x5eadbeefcafebabeull,
};
static const unsigned NDVEC = (unsigned)(sizeof dvec / sizeof dvec[0]);

static std::uint32_t rand_fbits(void)
{
	std::uint32_t r = next_u32();

	switch (next_mod(8u)) {
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
		return (r & 0x80000000u) | ((0x7f800000u + next_mod(17u) - 8u)
		    & 0x7fffffffu);
	case 5:
		return r ^ (1u << (next_mod(32u)));
	default:
		return r ^ next_u32();
	}
}

static std::uint64_t rand_dbits(void)
{
	std::uint64_t r = next_u64();

	switch (next_mod(8u)) {
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

static long double rand_ld(void)
{
	std::uint16_t se = (std::uint16_t)(next_u64() & 0xffffu);
	std::uint64_t m = next_u64();

	switch (next_mod(10u)) {
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
		se = (std::uint16_t)((se & 0x8000u) |
		    (LD_TINY_IX + (next_mod(5u) - 2u)));
		return mkld(se, m);
	case 4:
		se = (std::uint16_t)((se & 0x8000u) | 0x7fffu);
		return mkld(se, m);
	case 5:
		se = (std::uint16_t)((se & 0x8000u) | 0x3fffu);
		return mkld(se, 0x8000000000000000ull);
	case 6:
		se = (std::uint16_t)((se & 0x8000u) | 0xbfffu);
		return mkld(se, 0x8000000000000000ull);
	case 7:
		return mkld(0x3fffu, m);
	case 8:
		return mkld(0xbfffu, m);
	default:
		return mkld(se, m ^ next_u64());
	}
}

static void run_signbit_tests(void)
{
	unsigned i, j;

	for (i = 0; i < NDVEC; i++)
		check_signbit(fromdbits(dvec[i]), "dvec");

	for (i = 0; i < NFVEC; i++)
		check_signbitf(fromfbits(fvec[i]), "fvec");

	for (unsigned bit = 0; bit < 64; bit++) {
		std::uint64_t m = 1ull << bit;
		check_signbit(fromdbits(m), "bit");
		check_signbit(fromdbits(~m), "bit-not");
	}

	for (unsigned bit = 0; bit < 32; bit++) {
		std::uint32_t m = 1u << bit;
		check_signbitf(fromfbits(m), "bit");
		check_signbitf(fromfbits(~m), "bit-not");
	}

	const std::uint16_t ld_se[] = {
		0x0000u, 0x8000u, 0x3fffu, 0xbfffu, 0x3ffeu, 0xbffeu,
		0x3ffdu, 0xbffdu, 0x4000u, 0xc000u, 0x7fffu, 0xffffu,
	};
	for (i = 0; i < sizeof ld_se / sizeof ld_se[0]; i++) {
		check_signbitl(mkld(ld_se[i], 0), "se-zero");
		check_signbitl(mkld(ld_se[i], 0x8000000000000000ull), "se-one");
		check_signbitl(mkld(ld_se[i], 0xffffffffffffffffull), "se-all");
		check_signbitl(mkld(ld_se[i], next_u64()), "se-rand");
	}

	for (int d = -32; d <= 32; d++) {
		for (i = 0; i < sizeof ld_se / sizeof ld_se[0]; i++) {
			std::uint16_t se =
			    (std::uint16_t)(ld_se[i] + (std::uint16_t)d);
			check_signbitl(mkld(se, next_u64()), "se-walk");
		}
	}

	for (i = 0; i < NDVEC; i++) {
		for (j = 0; j < NDVEC; j++) {
			std::uint64_t xb = dvec[i] ^ (dvec[j] & 0x000fffffffffffffull);
			check_signbit(fromdbits(xb), "cross");
		}
	}

	for (long long n = 0; n < RANDOM_ITERS; n++)
		check_signbit(fromdbits(rand_dbits()), "rand");
	for (long long n = 0; n < RANDOM_ITERS; n++)
		check_signbitf(fromfbits(rand_fbits()), "rand");
	for (long long n = 0; n < RANDOM_ITERS; n++)
		check_signbitl(rand_ld(), "rand");
}

static void run_atanhl_tests(void)
{
	unsigned i, d;

	check_atanhl(0.0L, "zero");
	check_atanhl(-0.0L, "neg-zero");
	check_atanhl(1.0L, "one");
	check_atanhl(-1.0L, "neg-one");
	check_atanhl(0.5L, "half");
	check_atanhl(-0.5L, "neg-half");
	check_atanhl(0.25L, "quarter");
	check_atanhl(-0.25L, "neg-quarter");
	check_atanhl(2.0L, "two");
	check_atanhl(-2.0L, "neg-two");

	const std::uint16_t walk_se[] = {
		0x3ffcu, 0x3ffdu, 0x3ffeu, 0x3fffu, 0x4000u,
		0xbffcu, 0xbffdu, 0xbffeu, 0xbfffu, 0xc000u,
		0x7fffu, 0xffffu,
	};
	for (i = 0; i < sizeof walk_se / sizeof walk_se[0]; i++) {
		for (d = 0; d < 256; d++) {
			std::uint64_t m = (std::uint64_t)d << 56 |
			    (next_u64() & 0x00ffffffffffffffull);
			check_atanhl(mkld(walk_se[i], m), "se-grid");
			check_atanhl(mkld(walk_se[i],
			    m | 0x8000000000000000ull), "se-grid-nbit");
		}
	}

	for (d = -64; d <= 64; d++) {
		std::uint16_t se =
		    (std::uint16_t)(LD_TINY_IX + (std::uint16_t)d);
		check_atanhl(mkld(se, next_u64()), "tiny-walk");
		check_atanhl(mkld((std::uint16_t)(se | 0x8000u), next_u64()),
		    "tiny-walk-neg");
	}

	for (d = -64; d <= 64; d++) {
		std::uint16_t se =
		    (std::uint16_t)(0x3ffeu + (std::uint16_t)d);
		check_atanhl(mkld(se, 0x8000000000000000ull), "half-walk");
		check_atanhl(mkld((std::uint16_t)(se | 0x8000u),
		    0x8000000000000000ull), "half-walk-neg");
	}

	for (d = -64; d <= 64; d++) {
		std::uint16_t se =
		    (std::uint16_t)(0x3fffu + (std::uint16_t)d);
		check_atanhl(mkld(se, 0x8000000000000000ull), "one-walk");
		check_atanhl(mkld((std::uint16_t)(se | 0x8000u),
		    0x8000000000000000ull), "one-walk-neg");
	}

	const std::uint64_t mant[] = {
		0x0000000000000000ull, 0x8000000000000000ull,
		0x0000000000000001ull, 0xffffffffffffffffull,
		0x4000000000000000ull, 0xc000000000000000ull,
	};
	for (i = 0; i < sizeof mant / sizeof mant[0]; i++) {
		check_atanhl(mkld(0x3fffu, mant[i]), "one-mant");
		check_atanhl(mkld(0xbfffu, mant[i]), "neg-one-mant");
		check_atanhl(mkld(0x3ffeu, mant[i]), "half-mant");
		check_atanhl(mkld(0xbffeu, mant[i]), "neg-half-mant");
		check_atanhl(mkld(0x7fffu, mant[i]), "nan-mant");
	}

	for (long long n = 0; n < RANDOM_ITERS; n++)
		check_atanhl(rand_ld(), "rand");
}

static void run_cproj_tests(void)
{
	unsigned i, j;

	check_cproj(0.0, 0.0, "origin");
	check_cproj(1.0, -1.0, "finite");
	check_cproj(-3.5, 2.25, "finite-neg");

	const double inf = INFINITY;
	const double nan = NAN;

	check_cproj(inf, 0.0, "inf-re");
	check_cproj(-inf, 0.0, "neg-inf-re");
	check_cproj(0.0, inf, "inf-im");
	check_cproj(0.0, -inf, "neg-inf-im");
	check_cproj(inf, inf, "inf-both");
	check_cproj(-inf, inf, "mixed-inf");
	check_cproj(inf, nan, "inf-nan");
	check_cproj(nan, inf, "nan-inf");
	check_cproj(nan, nan, "nan-nan");

	for (i = 0; i < NDVEC; i++) {
		double re = fromdbits(dvec[i]);
		for (j = 0; j < NDVEC; j++) {
			double im = fromdbits(dvec[j]);
			check_cproj(re, im, "cross");
		}
	}

	const double inf_axis[][2] = {
		{ inf, 0.0 }, { -inf, 0.0 }, { 0.0, inf }, { 0.0, -inf },
		{ inf, 1.0 }, { -inf, 1.0 }, { 1.0, inf }, { 1.0, -inf },
		{ inf, inf }, { -inf, -inf }, { inf, -inf }, { -inf, inf },
	};
	for (i = 0; i < sizeof inf_axis / sizeof inf_axis[0]; i++)
		check_cproj(inf_axis[i][0], inf_axis[i][1], "inf-axis");

	for (long long n = 0; n < RANDOM_ITERS; n++) {
		double re = fromdbits(rand_dbits());
		double im = fromdbits(rand_dbits());
		if (next_mod(16u) == 0u)
			re = (next_u64() & 1u) ? inf : -inf;
		if (next_mod(16u) == 0u)
			im = (next_u64() & 1u) ? inf : -inf;
		check_cproj(re, im, "rand");
	}
}

int main(void)
{
	run_signbit_tests();
	run_atanhl_tests();
	run_cproj_tests();

	Stat *all[] = {
		&st_signbit, &st_signbitf, &st_signbitl, &st_atanhl, &st_cproj,
	};
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
