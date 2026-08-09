// Differential test for PBSD batch b0302 (cprojl, fdim, fdimf, fdiml).

#include <cfloat>
#include <climits>
#include <complex>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.src.b0302;

namespace port = pbsd::lib_msun_src::b0302;

extern "C" {
long double _Complex ref_cprojl(long double _Complex z);
double ref_fdim(double x, double y);
float ref_fdimf(float x, float y);
long double ref_fdiml(long double x, long double y);
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

static Stat st_cprojl = { "cprojl", 0, 0, 0 };
static Stat st_fdim = { "fdim", 0, 0, 0 };
static Stat st_fdimf = { "fdimf", 0, 0, 0 };
static Stat st_fdiml = { "fdiml", 0, 0, 0 };

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

static long double _Complex mkldc(long double re, long double im)
{
	long double _Complex z;

	__real__ z = re;
	__imag__ z = im;
	return z;
}

static std::uint64_t rng_state = 0xb0302a5a5a5a5a5aull;

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

static void check_cprojl(long double re, long double im, const char *tag)
{
	ldrep gp[2], go[2];
	std::complex<long double> pz;
	long double _Complex oz;

	st_cprojl.cases++;

	pz = port::cprojl(std::complex<long double>(re, im));
	oz = ref_cprojl(mkldc(re, im));

	gp[0] = ldbits(pz.real());
	gp[1] = ldbits(pz.imag());
	go[0] = ldbits(__real__ oz);
	go[1] = ldbits(__imag__ oz);

	if (guarded_equal(gp, go, sizeof(gp)))
		return;

	st_cprojl.fails++;
	if (should_print(st_cprojl)) {
		std::size_t i;

		std::printf("cprojl FAIL [%s] port=(", tag);
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", gp[0].b[i]);
		std::printf(",");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", gp[1].b[i]);
		std::printf(") ref=(");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", go[0].b[i]);
		std::printf(",");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", go[1].b[i]);
		std::printf(")\n");
	}
}

static void check_fdim(double x, double y, const char *tag)
{
	std::uint64_t p, o;

	st_fdim.cases++;

	p = dbits(port::fdim(x, y));
	o = dbits(ref_fdim(x, y));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_fdim.fails++;
	if (should_print(st_fdim))
		std::printf("fdim FAIL [%s] x=%#018llx y=%#018llx port=%#018llx ref=%#018llx\n",
		    tag, (unsigned long long)dbits(x),
		    (unsigned long long)dbits(y), (unsigned long long)p,
		    (unsigned long long)o);
}

static void check_fdimf(float x, float y, const char *tag)
{
	std::uint32_t p, o;

	st_fdimf.cases++;

	p = fbits(port::fdimf(x, y));
	o = fbits(ref_fdimf(x, y));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_fdimf.fails++;
	if (should_print(st_fdimf))
		std::printf("fdimf FAIL [%s] x=%#010x y=%#010x port=%#010x ref=%#010x\n",
		    tag, fbits(x), fbits(y), p, o);
}

static void check_fdiml(long double x, long double y, const char *tag)
{
	ldrep p, o;

	st_fdiml.cases++;

	p = ldbits(port::fdiml(x, y));
	o = ldbits(ref_fdiml(x, y));

	if (guarded_equal(&p, &o, LD_SIG))
		return;

	st_fdiml.fails++;
	if (should_print(st_fdiml)) {
		std::size_t i;

		std::printf("fdiml FAIL [%s] port=", tag);
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", p.b[i]);
		std::printf(" ref=");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", o.b[i]);
		std::printf("\n");
	}
}

static const std::uint32_t fvec[] = {
	0x00000000u, 0x80000000u,
	0x00000001u, 0x80000001u,
	0x00000080u, 0x80000080u,
	0x00400000u, 0x80400000u,
	0x007ffffeu, 0x807ffffeu,
	0x007fffffu, 0x807fffffu,
	0x00800000u, 0x80800000u,
	0x3f000000u, 0xbf000000u,
	0x3f800000u, 0xbf800000u,
	0x40000000u, 0xc0000000u,
	0x40400000u, 0xc0400000u,
	0x7f7fffffu, 0xff7fffffu,
	0x7f800000u, 0xff800000u,
	0x7f800001u, 0xff800001u,
	0x7fc00000u, 0xffc00000u,
	0x7ff00000u, 0xfff00000u,
	0x7fffffffu, 0xffffffffu,
	0x80808080u, 0x7f7f7f7fu,
	0x00ff00ffu, 0xff00ff00u,
};
static const unsigned NFVEC = (unsigned)(sizeof fvec / sizeof fvec[0]);

static const std::uint64_t dvec[] = {
	0x0000000000000000ull, 0x8000000000000000ull,
	0x0000000000000001ull, 0x8000000000000001ull,
	0x0000000000000080ull, 0x8000000000000080ull,
	0x0010000000000000ull, 0x8010000000000000ull,
	0x3fe0000000000000ull, 0xbfe0000000000000ull,
	0x3ff0000000000000ull, 0xbff0000000000000ull,
	0x4000000000000000ull, 0xc000000000000000ull,
	0x7fefffffffffffffull, 0xffefffffffffffffull,
	0x7ff0000000000000ull, 0xfff0000000000000ull,
	0x7ff0000000000001ull, 0xfff0000000000001ull,
	0x7ff8000000000000ull, 0xfff8000000000000ull,
	0x7fffffffffffffffull, 0xffffffffffffffffull,
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
	{ 0x4000u, 0x8000000000000000ull },
	{ 0xc000u, 0x8000000000000000ull },
	{ 0x7ffeu, 0xffffffffffffffffull },
	{ 0xfffeu, 0xffffffffffffffffull },
	{ 0x7fffu, 0x8000000000000000ull },
	{ 0xffffu, 0x8000000000000000ull },
	{ 0x7fffu, 0xc000000000000000ull },
	{ 0xffffu, 0xc000000000000000ull },
	{ 0x7fffu, 0x8000000000000001ull },
	{ 0x3fffu, 0x8080808080808080ull },
	{ 0xbfffu, 0xff00ff00ff00ff00ull },
};
static const unsigned NLDVEC = (unsigned)(sizeof ldvec / sizeof ldvec[0]);

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
		return (r & 0x80000000u) |
		    ((0x7f800000u + d) & 0x7fffffffu);
	}
	case 4: {
		std::uint32_t e = 118u + next_mod(61u);
		return (r & 0x80000000u) | (e << 23) |
		    (r & 0x007fffffu);
	}
	case 5:
		return 0x7f800000u ^ (r & 0x007fffffu);
	case 6:
		return 0x7fc00000u ^ (r & 0x003fffffu);
	case 7: {
		int k = (int)next_mod(401u) - 200;
		return fbits((float)k);
	}
	case 8:
		return r ^ next_u32();
	default:
		return (r & 0x80000000u) | 0x7f800000u;
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
		std::uint32_t d = next_mod(11u) - 5u;
		return (r & 0x8000000000000000ull) |
		    ((0x7ff0000000000000ull + d) & 0x7fffffffffffffffull);
	}
	case 4: {
		std::uint32_t e = 1006u + next_mod(61u);
		return (r & 0x8000000000000000ull) | ((std::uint64_t)e << 52) |
		    (r & 0x000fffffffffffffull);
	}
	case 5:
		return 0x7ff0000000000000ull ^ (r & 0x000fffffffffffffull);
	case 6:
		return 0x7ff8000000000000ull ^ (r & 0x0007ffffffffffffull);
	case 7: {
		int k = (int)next_mod(401u) - 200;
		return dbits((double)k);
	}
	case 8:
		return r ^ next_u64();
	default:
		return (r & 0x8000000000000000ull) | 0x7ff0000000000000ull;
	}
}

static long double rand_ld(void)
{
	std::uint16_t se;
	std::uint64_t m;

	switch (next_mod(10u)) {
	case 0:
		se = (std::uint16_t)(next_u64() & 0xffffu);
		m = next_u64();
		break;
	case 1:
		se = (std::uint16_t)(next_mod(0x10000u));
		m = next_u64() & 0x7fffffffffffffffull;
		break;
	case 2:
		se = (std::uint16_t)(0x7fffu ^ (next_mod(8u)));
		m = next_u64() | 0x8000000000000000ull;
		break;
	case 3:
		se = (std::uint16_t)(next_mod(0x4000u));
		m = next_u64();
		break;
	case 4:
		se = (std::uint16_t)(0x3fffu + next_mod(5u) - 2u);
		m = next_u64();
		break;
	case 5:
		return mkld(ldvec[next_mod(NLDVEC)].se,
		    ldvec[next_mod(NLDVEC)].m);
	case 6:
		return (long double)((int)next_mod(10001u) - 5000);
	case 7:
		return (long double)next_u64() / 17.0L;
	case 8:
		se = (std::uint16_t)(next_u64() & 0x8000u);
		m = next_u64();
		break;
	default:
		se = 0x7fffu;
		m = next_u64();
		break;
	}
	return mkld(se, m);
}

int main(void)
{
	unsigned i, j;

	for (i = 0; i < NLDVEC; i++) {
		for (j = 0; j < NLDVEC; j++) {
			long double re = mkld(ldvec[i].se, ldvec[i].m);
			long double im = mkld(ldvec[j].se, ldvec[j].m);

			check_cprojl(re, im, "cross");
		}
	}

	{
		static const long double vals[] = {
			0.0L, -0.0L, 1.0L, -1.0L, 0.5L, -0.5L,
			2.0L, -2.0L, 1e-30L, -1e-30L, 1e30L, -1e30L,
			__builtin_infl(), -__builtin_infl(),
			__builtin_nanl(""), -__builtin_nanl("payload"),
			LDBL_MIN, -LDBL_MIN, LDBL_MAX, -LDBL_MAX,
			LDBL_TRUE_MIN, -LDBL_TRUE_MIN,
		};
		const unsigned n = (unsigned)(sizeof vals / sizeof vals[0]);

		for (i = 0; i < n; i++)
			for (j = 0; j < n; j++)
				check_cprojl(vals[i], vals[j], "quadrant");
	}

	{
		static const long double inf_pairs[][2] = {
			{ __builtin_infl(), 0.0L },
			{ __builtin_infl(), -0.0L },
			{ __builtin_infl(), 1.0L },
			{ __builtin_infl(), -1.0L },
			{ -__builtin_infl(), 0.0L },
			{ -__builtin_infl(), -0.0L },
			{ 0.0L, __builtin_infl() },
			{ -0.0L, __builtin_infl() },
			{ 1.0L, __builtin_infl() },
			{ -1.0L, -__builtin_infl() },
			{ __builtin_infl(), __builtin_infl() },
			{ __builtin_infl(), -__builtin_infl() },
			{ -__builtin_infl(), __builtin_infl() },
			{ __builtin_nanl(""), 1.0L },
			{ 1.0L, __builtin_nanl("") },
		};
		const unsigned n = (unsigned)(sizeof inf_pairs / sizeof inf_pairs[0]);

		for (i = 0; i < n; i++)
			check_cprojl(inf_pairs[i][0], inf_pairs[i][1], "inf-axis");
	}

	for (i = 0; i < NFVEC; i++) {
		for (j = 0; j < NFVEC; j++) {
			check_fdimf(fromfbits(fvec[i]), fromfbits(fvec[j]),
			    "cross");
		}
	}

	for (i = 0; i < NDVEC; i++) {
		for (j = 0; j < NDVEC; j++) {
			check_fdim(fromdbits(dvec[i]), fromdbits(dvec[j]),
			    "cross");
		}
	}

	for (i = 0; i < NLDVEC; i++) {
		for (j = 0; j < NLDVEC; j++) {
			check_fdiml(mkld(ldvec[i].se, ldvec[i].m),
			    mkld(ldvec[j].se, ldvec[j].m), "cross");
		}
	}

	{
		static const std::uint32_t fpairs[][2] = {
			{ 0x7fc00000u, 0x3f800000u },
			{ 0x3f800000u, 0x7fc00000u },
			{ 0x40000000u, 0x3f800000u },
			{ 0x3f800000u, 0x40000000u },
			{ 0x3f800000u, 0x3f800000u },
			{ 0xbf800000u, 0x3f800000u },
			{ 0x7f800000u, 0x3f800000u },
			{ 0x3f800000u, 0x7f800000u },
		};
		const unsigned n = (unsigned)(sizeof fpairs / sizeof fpairs[0]);

		for (i = 0; i < n; i++)
			check_fdimf(fromfbits(fpairs[i][0]),
			    fromfbits(fpairs[i][1]), "branch");
	}

	{
		static const std::uint64_t dpairs[][2] = {
			{ 0x7ff8000000000000ull, 0x3ff0000000000000ull },
			{ 0x3ff0000000000000ull, 0x7ff8000000000000ull },
			{ 0x4000000000000000ull, 0x3ff0000000000000ull },
			{ 0x3ff0000000000000ull, 0x4000000000000000ull },
			{ 0x3ff0000000000000ull, 0x3ff0000000000000ull },
			{ 0xbff0000000000000ull, 0x3ff0000000000000ull },
			{ 0x7ff0000000000000ull, 0x3ff0000000000000ull },
			{ 0x3ff0000000000000ull, 0x7ff0000000000000ull },
		};
		const unsigned n = (unsigned)(sizeof dpairs / sizeof dpairs[0]);

		for (i = 0; i < n; i++)
			check_fdim(fromdbits(dpairs[i][0]),
			    fromdbits(dpairs[i][1]), "branch");
	}

	for (unsigned bit = 0; bit < 32; bit++) {
		std::uint32_t m = 1u << bit;
		check_fdimf(fromfbits(m), fromfbits(~m), "bit");
	}

	for (unsigned bit = 0; bit < 64; bit++) {
		std::uint64_t m = 1ull << bit;
		check_fdim(fromdbits(m), fromdbits(~m), "bit");
	}

	for (long long n = 0; n < RANDOM_ITERS; n++) {
		check_cprojl(rand_ld(), rand_ld(), "rand");
		check_fdimf(fromfbits(rand_fbits()), fromfbits(rand_fbits()),
		    "rand");
		check_fdim(fromdbits(rand_dbits()), fromdbits(rand_dbits()),
		    "rand");
		check_fdiml(rand_ld(), rand_ld(), "rand");
	}

	Stat *all[] = { &st_cprojl, &st_fdim, &st_fdimf, &st_fdiml };
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
