// Differential test for PBSD batch b0322.

import pbsd.lib.msun.src.b0322;

#include <cfloat>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace port = pbsd::lib_msun_src::b0322;

extern "C" {
float ref_expm1f(float x);
long double ref_modfl(long double x, long double *iptr);
double ref_asin(double x);
double ref___ldexp_exp(double x, int expt);
double _Complex ref___ldexp_cexp(double _Complex z, int expt);
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

static Stat st_expm1f = { "expm1f", 0, 0, 0 };
static Stat st_modfl = { "modfl", 0, 0, 0 };
static Stat st_asin = { "asin", 0, 0, 0 };
static Stat st_ldexp_exp = { "__ldexp_exp", 0, 0, 0 };
static Stat st_ldexp_cexp = { "__ldexp_cexp", 0, 0, 0 };

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

struct crep {
	std::uint64_t re;
	std::uint64_t im;
};

static crep cbits(double _Complex z)
{
	crep r;
	std::memcpy(&r.re, &z, sizeof(r.re));
	std::memcpy(&r.im, (char *)&z + sizeof(double), sizeof(r.im));
	return r;
}

static double _Complex mkc(std::uint64_t re, std::uint64_t im)
{
	double _Complex z;
	std::memcpy(&z, &re, sizeof(re));
	std::memcpy((char *)&z + sizeof(double), &im, sizeof(im));
	return z;
}

static std::uint64_t rng_state = 0xb0322a5a5a5a5a5aull;

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

static void check_expm1f(std::uint32_t xb, const char *tag)
{
	float x = fromfbits(xb);
	std::uint32_t p, o;

	st_expm1f.cases++;

	p = fbits(port::expm1f(x));
	o = fbits(ref_expm1f(x));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_expm1f.fails++;
	if (should_print(st_expm1f))
		std::printf("expm1f FAIL [%s] x=%#010x port=%#010x ref=%#010x\n",
		    tag, xb, p, o);
}

static bool ld_equal(long double a, long double b)
{
	ldrep pa = ldbits(a);
	ldrep pb = ldbits(b);
	return std::memcmp(pa.b, pb.b, LD_SIG) == 0;
}

static void check_modfl(long double x, const char *tag)
{
	unsigned char pbuf[GUARD_BUF], obuf[GUARD_BUF];
	long double p_iptr, o_iptr;
	ldrep pr, orr;

	st_modfl.cases++;

	std::memset(pbuf, GUARD, sizeof(pbuf));
	std::memset(obuf, GUARD, sizeof(obuf));
	std::memcpy(pbuf + GUARD_OFF, &x, sizeof(x));
	std::memcpy(obuf + GUARD_OFF, &x, sizeof(x));

	long double px;
	std::memcpy(&px, pbuf + GUARD_OFF, sizeof(px));
	long double ox;
	std::memcpy(&ox, obuf + GUARD_OFF, sizeof(ox));

	long double pr_ret, or_ret;
	pr_ret = port::modfl(px, &p_iptr);
	or_ret = ref_modfl(ox, &o_iptr);
	pr = ldbits(pr_ret);
	orr = ldbits(or_ret);

	if (ld_equal(pr_ret, or_ret) &&
	    ld_equal(p_iptr, o_iptr) &&
	    std::memcmp(pbuf, obuf, sizeof(pbuf)) == 0)
		return;

	st_modfl.fails++;
	if (should_print(st_modfl)) {
		std::size_t i;
		std::printf("modfl FAIL [%s] port_ret=", tag);
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", pr.b[i]);
		std::printf(" ref_ret=");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", orr.b[i]);
		std::printf("\n");
	}
}

static void check_asin(std::uint64_t xb, const char *tag)
{
	double x = fromdbits(xb);
	std::uint64_t p, o;

	st_asin.cases++;

	p = dbits(port::asin(x));
	o = dbits(ref_asin(x));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_asin.fails++;
	if (should_print(st_asin))
		std::printf("asin FAIL [%s] x=%#018llx port=%#018llx ref=%#018llx\n",
		    tag, (unsigned long long)xb,
		    (unsigned long long)p, (unsigned long long)o);
}

static void check_ldexp_exp(std::uint64_t xb, int expt, const char *tag)
{
	double x = fromdbits(xb);
	std::uint64_t p, o;

	st_ldexp_exp.cases++;

	p = dbits(port::__ldexp_exp(x, expt));
	o = dbits(ref___ldexp_exp(x, expt));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_ldexp_exp.fails++;
	if (should_print(st_ldexp_exp))
		std::printf("__ldexp_exp FAIL [%s] x=%#018llx expt=%d "
		    "port=%#018llx ref=%#018llx\n",
		    tag, (unsigned long long)xb, expt,
		    (unsigned long long)p, (unsigned long long)o);
}

static void check_ldexp_cexp(std::uint64_t reb, std::uint64_t imb, int expt,
    const char *tag)
{
	double _Complex z = mkc(reb, imb);
	crep p, o;

	st_ldexp_cexp.cases++;

	p = cbits(port::__ldexp_cexp(z, expt));
	o = cbits(ref___ldexp_cexp(z, expt));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_ldexp_cexp.fails++;
	if (should_print(st_ldexp_cexp))
		std::printf("__ldexp_cexp FAIL [%s] z=(%#018llx,%#018llx) "
		    "expt=%d\n",
		    tag, (unsigned long long)reb, (unsigned long long)imb,
		    expt);
}

static const std::uint32_t fvec[] = {
	0x00000000u, 0x80000000u,
	0x00000001u, 0x80000001u,
	0x00400000u, 0x80400000u,
	0x007ffffeu, 0x807ffffeu,
	0x00800000u, 0x80800000u,
	0x0f0f0f0fu, 0x8f0f0f0fu,
	0x317fffffu, 0xb17fffffu,
	0x31800000u, 0xb1800000u,
	0x31800001u, 0xb1800001u,
	0x33000000u, 0xb3000000u,
	0x33000001u, 0xb3000001u,
	0x3eb17217u, 0xbeb17217u,
	0x3eb17218u, 0xbeb17218u,
	0x3eb17219u, 0xbeb17219u,
	0x3f317180u, 0xbf317180u,
	0x3f851591u, 0xbf851591u,
	0x3f851592u, 0xbf851592u,
	0x3f851593u, 0xbf851593u,
	0x4195b843u, 0xc195b843u,
	0x4195b844u, 0xc195b844u,
	0x4195b845u, 0xc195b845u,
	0x42b1717fu, 0xc2b1717fu,
	0x42b17180u, 0xc2b17180u,
	0x42b17217u, 0xc2b17217u,
	0x42b17218u, 0xc2b17218u,
	0x42b17219u, 0xc2b17219u,
	0x4b000000u, 0xcb000000u,
	0x7f7ffffeu, 0xff7ffffeu,
	0x7f800000u, 0xff800000u,
	0x7f800001u, 0xff800001u,
	0x7fffffffu, 0xffffffffu,
	0x80808080u, 0x7f7f7f7fu,
};
static const unsigned NFVEC = (unsigned)(sizeof fvec / sizeof fvec[0]);

static const std::uint64_t dvec[] = {
	0x0000000000000000ull, 0x8000000000000000ull,
	0x0000000000000001ull, 0x8000000000000001ull,
	0x3e50000000000000ull, 0xbe50000000000000ull,
	0x3e4fffffffffffffull, 0xbe4fffffffffffffull,
	0x3e50000000000001ull, 0xbe50000000000001ull,
	0x3fe0000000000000ull, 0xbfe0000000000000ull,
	0x3fdfffffffffffffull, 0xbfdfffffffffffffull,
	0x3fe0000000000001ull, 0xbfe0000000000001ull,
	0x3fef3332ffffffffull, 0xbfef3332ffffffffull,
	0x3fef333300000000ull, 0xbfef333300000000ull,
	0x3fef333300000001ull, 0xbfef333300000001ull,
	0x3ff0000000000000ull, 0xbff0000000000000ull,
	0x3ff0000000000001ull, 0xbff0000000000001ull,
	0x3fefffffffffffffull, 0xbfefffffffffffffull,
	0x7ff0000000000000ull, 0xfff0000000000000ull,
	0x7ff8000000000000ull, 0xfff8000000000000ull,
	0x7fffffffffffffffull, 0xffffffffffffffffull,
	0x40862e41ffffffffull, 0xc0862e41ffffffffull,
	0x40862e4200000000ull, 0xc0862e4200000000ull,
	0x408633ce00000000ull, 0xc08633ce00000000ull,
	0x408633cf00000000ull, 0xc08633cf00000000ull,
	0x408f400000000000ull, 0xc08f400000000000ull,
	0x4095bf9a00000000ull, 0xc095bf9a00000000ull,
	0x4095bf9a55555555ull, 0xc095bf9a55555555ull,
	0x8080808080808080ull, 0x7f7f7f7f7f7f7f7full,
};
static const unsigned NDVEC = (unsigned)(sizeof dvec / sizeof dvec[0]);

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
	{ 0x3ffeu, 0x8000000000000000ull },
	{ 0xbffeu, 0x8000000000000000ull },
	{ 0x3fffu, 0x8000000000000000ull },
	{ 0xbfffu, 0x8000000000000000ull },
	{ 0x3fffu, 0x8000000000000001ull },
	{ 0x3ffeu, 0xffffffffffffffffull },
	{ 0x4000u, 0x8000000000000000ull },
	{ 0xc000u, 0x8000000000000000ull },
	{ 0x4000u, 0xc000000000000000ull },
	{ 0x4000u, 0xc90fdaa22168c235ull },
	{ 0x403eu, 0x8000000000000000ull },
	{ 0x403fu, 0x8000000000000000ull },
	{ 0x407fu, 0x8000000000000000ull },
	{ 0x7ffeu, 0xffffffffffffffffull },
	{ 0xfffeu, 0xffffffffffffffffull },
	{ 0x7fffu, 0x8000000000000000ull },
	{ 0xffffu, 0x8000000000000000ull },
	{ 0x7fffu, 0xc000000000000000ull },
	{ 0x7fffu, 0x8000000000000001ull },
	{ 0x3fffu, 0x8080808080808080ull },
};
static const unsigned NLDVEC = (unsigned)(sizeof ldvec / sizeof ldvec[0]);

static std::uint32_t rand_fbits(void)
{
	std::uint32_t r = next_u32();

	switch ((unsigned)(next_u64() % 14u)) {
	case 0:
		return r;
	case 1:
		return r & 0x807fffffu;
	case 2:
		return (r & 0x807fffffu) | 0x7f800000u;
	case 3: {
		std::uint32_t d = (std::uint32_t)next_mod64(11u) - 5u;
		return (r & 0x80000000u) |
		    ((0x7f800000u + d) & 0x7fffffffu);
	}
	case 4: {
		std::uint32_t d = (std::uint32_t)next_mod64(11u) - 5u;
		return (r & 0x80000000u) |
		    ((0x00800000u + d) & 0x7fffffffu);
	}
	case 5: {
		std::uint32_t e = 118u + (std::uint32_t)next_mod64(61u);
		return (r & 0x80000000u) | (e << 23) |
		    (r & 0x007fffffu);
	}
	case 6:
		return 0x33000000u ^ (r & 0x007fffffu);
	case 7:
		return 0x3eb17218u ^ (r & 0x007fffffu);
	case 8:
		return 0x4195b844u ^ (r & 0x007fffffu);
	case 9:
		return 0x42b17218u ^ (r & 0x007fffffu);
	case 10:
		return 0x3f851592u ^ (r & 0x007fffffu);
	case 11: {
		int k = (int)next_mod64(2049u) - 1024;
		return fbits((float)k / 4.0f);
	}
	default:
		return r ^ next_u32();
	}
}

static std::uint64_t rand_dbits(void)
{
	std::uint64_t r = next_u64();

	switch ((unsigned)(next_u64() % 14u)) {
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
	case 6:
		return 0x3e50000000000000ull ^ (r & 0x000fffffffffffffull);
	case 7:
		return 0x3fe0000000000000ull ^ (r & 0x000fffffffffffffull);
	case 8:
		return 0x3fef333300000000ull ^ (r & 0x000fffffffffffffull);
	case 9:
		return 0x3ff0000000000000ull ^ (r & 0x000fffffffffffffull);
	case 10:
		return 0x40862e4200000000ull ^ (r & 0x000fffffffffffffull);
	case 11: {
		int k = (int)next_mod64(2049u) - 1024;
		return dbits((double)k / 4.0);
	}
	default:
		return r ^ next_u64();
	}
}

static std::uint64_t rand_ldexp_x(void)
{
	std::uint64_t r = next_u64();
	std::uint64_t e = 0x408u + next_mod64(0x20u);
	return (r & 0x8000000000000000ull) | (e << 52) |
	    (r & 0x000fffffffffffffull);
}

static long double rand_ld(void)
{
	std::uint16_t se = (std::uint16_t)(next_u64() & 0xffffu);
	std::uint64_t m = next_u64();
	if ((next_u64() & 7u) == 0u)
		se = (std::uint16_t)((se & 0x8000u) | next_mod64(0x7fffu));
	return mkld(se, m);
}

int main(void)
{
	unsigned i, j;

	for (i = 0; i < NFVEC; i++)
		check_expm1f(fvec[i], "fvec");

	static const std::uint32_t kWalkF[] = {
		0x00000000u, 0x33000000u,
		0x33000001u, 0x3eb17218u,
		0x3eb17219u, 0x3f851592u,
		0x4195b844u, 0x42b17218u,
		0x7f800000u, 0xff800000u,
	};
	for (unsigned k = 0; k < sizeof kWalkF / sizeof kWalkF[0]; k++) {
		for (int d = -64; d <= 64; d++) {
			std::uint32_t xb = kWalkF[k] + (std::uint32_t)d;
			check_expm1f(xb, "walk");
		}
	}

	for (unsigned s = 0; s < 2; s++) {
		for (unsigned e = 0; e < 256; e++) {
			std::uint32_t xb = (s << 31) | (e << 23) |
			    0x00123456u;
			check_expm1f(xb, "exp-sweep");
		}
	}

	for (i = 0; i < NLDVEC; i++)
		check_modfl(mkld(ldvec[i].se, ldvec[i].m), "ldvec");

	for (unsigned e = 0; e < 0x8000u; e += 0x100u) {
		for (unsigned s = 0; s < 2; s++) {
			std::uint64_t m = ((std::uint64_t)(s & 1u) << 63) |
			    ((std::uint64_t)e << 48) |
			    (0x80808080ull << 16) | 0x80808080ull;
			check_modfl(mkld((std::uint16_t)e, m), "exp-sweep");
		}
	}

	for (unsigned e = 0; e < 64; e++) {
		std::uint16_t se = (std::uint16_t)(0x3fffu + (e / 2));
		std::uint64_t mh = (1ull << (e % 32)) | 0x80000000ull;
		std::uint64_t ml = (e > 16) ? (1ull << (e - 16)) : 0ull;
		check_modfl(mkld(se, (mh << 32) | ml), "mant-boundary");
		check_modfl(mkld((std::uint16_t)(se | 0x8000u),
		    (mh << 32) | ml), "mant-boundary-neg");
	}

	for (i = 0; i < NDVEC; i++)
		check_asin(dvec[i], "dvec");

	static const std::uint64_t kWalkD[] = {
		0x0000000000000000ull, 0x3e50000000000000ull,
		0x3e4fffffffffffffull, 0x3e50000000000001ull,
		0x3fe0000000000000ull, 0x3fdfffffffffffffull,
		0x3fe0000000000001ull, 0x3fef333300000000ull,
		0x3ff0000000000000ull, 0x3fefffffffffffffull,
	};
	for (unsigned k = 0; k < sizeof kWalkD / sizeof kWalkD[0]; k++) {
		for (int d = -64; d <= 64; d++) {
			std::uint64_t xb = kWalkD[k] + (std::uint64_t)d;
			check_asin(xb, "walk");
		}
	}

	for (unsigned s = 0; s < 2; s++) {
		for (unsigned e = 0; e < 2048; e++) {
			std::uint64_t xb = ((std::uint64_t)s << 63) |
			    ((std::uint64_t)e << 52) | 0x00123456789abcull;
			check_asin(xb, "exp-sweep");
		}
	}

	static const int expts[] = { -2, -1, 0, 1, 2 };
	for (i = 0; i < NDVEC; i++) {
		for (j = 0; j < sizeof expts / sizeof expts[0]; j++)
			check_ldexp_exp(dvec[i], expts[j], "dvec");
	}

	for (unsigned s = 0; s < 2; s++) {
		for (unsigned e = 0x408u; e < 0x409u; e++) {
			for (int d = -128; d <= 128; d++) {
				std::uint64_t xb =
				    ((std::uint64_t)s << 63) |
				    ((std::uint64_t)e << 52) |
				    ((std::uint64_t)(unsigned)d &
					0x000fffffffffffffull);
				check_ldexp_exp(xb, 0, "boundary");
				check_ldexp_exp(xb, -1, "boundary");
			}
		}
	}

	for (i = 0; i < NDVEC; i++) {
		for (j = 0; j < sizeof expts / sizeof expts[0]; j++)
			check_ldexp_cexp(dvec[i], dvec[(i + 1) % NDVEC],
			    expts[j], "dvec");
	}

	const long long kIters = 52000;
	for (long long n = 0; n < kIters; n++) {
		check_expm1f(rand_fbits(), "rand");
		check_modfl(rand_ld(), "rand");
		check_asin(rand_dbits(), "rand");
		std::uint64_t xb = rand_ldexp_x();
		int expt = (int)(next_mod64(5u)) - 2;
		check_ldexp_exp(xb, expt, "rand");
		check_ldexp_cexp(xb, rand_dbits(), expt, "rand");
	}

	Stat *all[] = {
		&st_expm1f, &st_modfl, &st_asin,
		&st_ldexp_exp, &st_ldexp_cexp
	};
	const unsigned nall = (unsigned)(sizeof all / sizeof all[0]);
	long long total_cases = 0;
	long long total_fails = 0;

	std::printf("\n%-14s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-14s %12s %12s\n", "--------------", "------------",
	    "------------");
	for (unsigned k = 0; k < nall; k++) {
		std::printf("%-14s %12lld %12lld\n", all[k]->name,
		    all[k]->cases, all[k]->fails);
		total_cases += all[k]->cases;
		total_fails += all[k]->fails;
	}
	std::printf("%-14s %12s %12s\n", "--------------", "------------",
	    "------------");
	std::printf("%-14s %12lld %12lld\n", "TOTAL", total_cases,
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

	return bad ? 1 : 0;
}
