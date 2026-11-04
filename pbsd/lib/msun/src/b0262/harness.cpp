// Differential test for PBSD batch b0262 (cimagl, truncl, asinh).

import pbsd.lib.msun.src.b0262;

#include <cfloat>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace port = pbsd::lib_msun_src::b0262;

extern "C" {
long double ref_cimagl(long double _Complex z);
long double ref_truncl(long double x);
double ref_asinh(double x);
}

#if LDBL_MANT_DIG == 64
static const std::size_t LD_SIG = 10;
#else
static const std::size_t LD_SIG = sizeof(long double);
#endif

static const unsigned char GUARD = 0x7f;
static const std::size_t GUARD_BUF = 64;
static const std::size_t GUARD_OFF = 16;

struct Stat {
	const char *name;
	long long cases;
	long long fails;
	long long printed;
};

static Stat st_cimagl = { "cimagl", 0, 0, 0 };
static Stat st_truncl = { "truncl", 0, 0, 0 };
static Stat st_asinh  = { "asinh",  0, 0, 0 };

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

static long double _Complex mkldc(long double re, long double im)
{
	long double _Complex z;
	__real__ z = re;
	__imag__ z = im;
	return z;
}

static std::uint64_t rng_state = 0xb0262a5a5a5a5a5aull;

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

static std::uint64_t next_mod64(std::uint64_t m)
{
	return next_u64() % m;
}

static void check_cimagl(long double re, long double im, const char *tag)
{
	ldrep p, o;

	st_cimagl.cases++;

	p = ldbits(port::cimagl(mkldc(re, im)));
	o = ldbits(ref_cimagl(mkldc(re, im)));

	if (guarded_equal(p.b, o.b, sizeof(p.b)))
		return;

	st_cimagl.fails++;
	if (should_print(st_cimagl)) {
		std::size_t i;
		std::printf("cimagl FAIL [%s] port=", tag);
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", p.b[i]);
		std::printf(" ref=");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", o.b[i]);
		std::printf("\n");
	}
}

static void check_truncl(long double x, const char *tag)
{
	ldrep p, o;

	st_truncl.cases++;

	p = ldbits(port::truncl(x));
	o = ldbits(ref_truncl(x));

	if (guarded_equal(p.b, o.b, sizeof(p.b)))
		return;

	st_truncl.fails++;
	if (should_print(st_truncl)) {
		std::size_t i;
		std::printf("truncl FAIL [%s] port=", tag);
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", p.b[i]);
		std::printf(" ref=");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", o.b[i]);
		std::printf("\n");
	}
}

static void check_asinh(double x, const char *tag)
{
	std::uint64_t p, o;

	st_asinh.cases++;

	p = dbits(port::asinh(x));
	o = dbits(ref_asinh(x));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_asinh.fails++;
	if (should_print(st_asinh))
		std::printf("asinh FAIL [%s] x=%#018llx port=%#018llx ref=%#018llx\n",
		    tag, (unsigned long long)dbits(x),
		    (unsigned long long)p, (unsigned long long)o);
}

struct ldcase {
	std::uint16_t se;
	std::uint64_t m;
};

static const ldcase ldvec[] = {
	{ 0x0000u, 0x0000000000000000ull },
	{ 0x8000u, 0x0000000000000000ull },
	{ 0x0000u, 0x0000000000000001ull },
	{ 0x8000u, 0x0000000000000001ull },
	{ 0x0000u, 0x0000000000000080ull },
	{ 0x0000u, 0x7fffffffffffffffull },
	{ 0x8000u, 0x7fffffffffffffffull },
	{ 0x0001u, 0x8000000000000000ull },
	{ 0x8001u, 0x8000000000000000ull },
	{ 0x0001u, 0x8000000000000001ull },
	{ 0x3ffeu, 0x8000000000000000ull },
	{ 0xbffeu, 0x8000000000000000ull },
	{ 0x3fffu, 0x8000000000000000ull },
	{ 0xbfffu, 0x8000000000000000ull },
	{ 0x3fffu, 0x8000000000000001ull },
	{ 0x3ffeu, 0xffffffffffffffffull },
	{ 0x4000u, 0x8000000000000000ull },
	{ 0xc000u, 0x8000000000000000ull },
	{ 0x4000u, 0xc000000000000000ull },
	{ 0xc000u, 0xc000000000000000ull },
	{ 0x4001u, 0xe000000000000000ull },
	{ 0x4000u, 0xc90fdaa22168c235ull },
	{ 0x403eu, 0x8000000000000000ull },
	{ 0x403fu, 0x8000000000000000ull },
	{ 0x407fu, 0x8000000000000000ull },
	{ 0x7ffeu, 0xffffffffffffffffull },
	{ 0xfffeu, 0xffffffffffffffffull },
	{ 0x7fffu, 0x8000000000000000ull },
	{ 0xffffu, 0x8000000000000000ull },
	{ 0x7fffu, 0xc000000000000000ull },
	{ 0xffffu, 0xc000000000000000ull },
	{ 0x7fffu, 0xa000000000000000ull },
	{ 0x7fffu, 0x8000000000000001ull },
	{ 0x7fffu, 0xffffffffffffffffull },
	{ 0x3fffu, 0x8080808080808080ull },
	{ 0xbfffu, 0xff00ff00ff00ff00ull },
	{ 0x3fffu, 0xfefefefefefefefeull },
	{ 0x3ffeu, 0xaaaaaaaaaaaaaaaaull },
	{ 0x401eu, 0x123456789abcdef0ull },
	{ 0xc01eu, 0x123456789abcdef0ull },
	{ 0x402eu, 0x00000000ffffffffull },
	{ 0x402eu, 0xffffffff00000000ull },
};
static const unsigned NLDVEC = (unsigned)(sizeof ldvec / sizeof ldvec[0]);

static const std::uint64_t dvec[] = {
	0x0000000000000000ull, 0x8000000000000000ull,
	0x0000000000000001ull, 0x8000000000000001ull,
	0x0000000000000080ull, 0x8000000000000080ull,
	0x000fffffffffffffull, 0x800fffffffffffffull,
	0x0010000000000000ull, 0x8010000000000000ull,
	0x0010000000000001ull, 0x8010000000000001ull,
	0x3fe0000000000000ull, 0xbfe0000000000000ull,
	0x3ff0000000000000ull, 0xbff0000000000000ull,
	0x3ff0000000000001ull, 0xbff0000000000001ull,
	0x3fefffffffffffffull, 0xbfefffffffffffffull,
	0x4000000000000000ull, 0xc000000000000000ull,
	0x4000000000000001ull, 0xc000000000000001ull,
	0x3fffffffffffffffull, 0xbfffffffffffffffull,
	0x41b0000000000000ull, 0xc1b0000000000000ull,
	0x41afffffffffffffull, 0xc1afffffffffffffull,
	0x41b0000000000001ull, 0xc1b0000000000001ull,
	0x3e30000000000000ull, 0xbe30000000000000ull,
	0x3e2fffffffffffffull, 0xbe2fffffffffffffull,
	0x3e30000000000001ull, 0xbe30000000000001ull,
	0x7fefffffffffffffull, 0xffefffffffffffffull,
	0x7ff0000000000000ull, 0xfff0000000000000ull,
	0x7ff0000000000001ull, 0xfff0000000000001ull,
	0x7ff8000000000000ull, 0xfff8000000000000ull,
	0x7ff8000000000001ull, 0xfff8000000000001ull,
	0x7fffffffffffffffull, 0xffffffffffffffffull,
	0x4330000000000000ull, 0xc330000000000000ull,
	0x4024000000000000ull, 0xc024000000000000ull,
	0x40490fdb00000000ull, 0xc0490fdb00000000ull,
	0x8080808080808080ull, 0x7f7f7f7f7f7f7f7full,
	0x00ff00ff00ff00ffull, 0xff00ff00ff00ff00ull,
};
static const unsigned NDVEC = (unsigned)(sizeof dvec / sizeof dvec[0]);

static long double rand_ld(void)
{
	std::uint16_t se = (std::uint16_t)(next_u64() & 0xffffu);
	std::uint64_t m = next_u64();
	if ((next_u64() & 7u) == 0u)
		se = (std::uint16_t)((se & 0x8000u) | (next_mod64(0x7fffu)));
	return mkld(se, m);
}

static std::uint64_t rand_dbits(void)
{
	std::uint64_t r = next_u64();
	switch ((unsigned)(next_u64() % 12u)) {
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
		return dbits((double)k / 4.0);
	}
	case 7: {
		int k = (int)next_mod64(401u) - 200;
		return dbits((double)k);
	}
	case 8: {
		std::uint64_t e = (next_u64() & 1u) ? next_mod64(8u)
						    : 2040ull + next_mod64(8u);
		return (r & 0x8000000000000000ull) | (e << 52) |
		    (r & 0x000fffffffffffffull);
	}
	case 9:
		return 0x3e30000000000000ull ^ (r & 0x000fffffffffffffull);
	case 10:
		return 0x41b0000000000000ull ^ (r & 0x000fffffffffffffull);
	default:
		return r ^ next_u64();
	}
}

int main(void)
{
	unsigned i, j;

	for (i = 0; i < NLDVEC; i++) {
		long double re = mkld(ldvec[i].se, ldvec[i].m);
		for (j = 0; j < NLDVEC; j++) {
			long double im = mkld(ldvec[j].se, ldvec[j].m);
			check_cimagl(re, im, "cross");
			check_truncl(re, "ldvec-re");
		}
		check_truncl(mkld(ldvec[i].se, ldvec[i].m), "ldvec");
	}

	for (i = 0; i < NDVEC; i++)
		check_asinh(fromdbits(dvec[i]), "dvec");

	static const std::uint64_t kWalkD[] = {
		0x0000000000000000ull, 0x3e30000000000000ull,
		0x3e2fffffffffffffull, 0x3e30000000000001ull,
		0x4000000000000000ull, 0x3fffffffffffffffull,
		0x4000000000000001ull, 0x41afffffffffffffull,
		0x41b0000000000000ull, 0x41b0000000000001ull,
		0x7ff0000000000000ull, 0xfff0000000000000ull,
	};
	for (unsigned k = 0; k < sizeof kWalkD / sizeof kWalkD[0]; k++) {
		for (int d = -64; d <= 64; d++) {
			std::uint64_t xb = kWalkD[k] + (std::uint64_t)d;
			check_asinh(fromdbits(xb), "walk");
		}
	}

	for (unsigned s = 0; s < 2; s++) {
		for (unsigned e = 0; e < 2048; e++) {
			std::uint64_t xb = ((std::uint64_t)s << 63) |
			    ((std::uint64_t)e << 52) | 0x00123456789abcull;
			check_asinh(fromdbits(xb), "exp-sweep");
		}
	}

	for (unsigned bit = 0; bit < 64; bit++) {
		std::uint64_t m = 1ull << bit;
		check_asinh(fromdbits(m), "bit");
		check_asinh(fromdbits(~m), "bit-not");
	}

	for (unsigned expv = 0; expv < 0x8000u; expv += 0x80u) {
		for (unsigned s = 0; s < 2; s++) {
			std::uint64_t m = ((std::uint64_t)(s & 1u) << 63) |
			    ((std::uint64_t)expv << 48) |
			    (0x80808080ull << 16) | 0x80808080ull;
			long double x = mkld((std::uint16_t)expv,
			    m & 0xffffffffffffffffull);
			check_truncl(x, "exp-sweep");
		}
	}

	for (unsigned e = 0; e < 64; e++) {
		std::uint16_t se = (std::uint16_t)(0x3fffu + (e / 2));
		std::uint64_t mh = (1ull << (e % 32)) | 0x80000000ull;
		std::uint64_t ml = (e > 16) ? (1ull << (e - 16)) : 0ull;
		check_truncl(mkld(se, (mh << 32) | ml), "mant-boundary");
		check_truncl(mkld((std::uint16_t)(se | 0x8000u),
		    (mh << 32) | ml), "mant-boundary-neg");
	}

	const long long kIters = 210000;
	for (long long n = 0; n < kIters; n++) {
		long double re = rand_ld();
		long double im = rand_ld();
		check_cimagl(re, im, "rand");
		check_truncl(rand_ld(), "rand");
		check_asinh(fromdbits(rand_dbits()), "rand");
	}

	Stat *all[] = { &st_cimagl, &st_truncl, &st_asinh };
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
