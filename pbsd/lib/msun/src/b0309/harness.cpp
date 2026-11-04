// Differential test for PBSD batch b0309 (fmaximum_magl, hypotf, fminimum_magf,
// isnan, __isnanf, __isnanl).

#include <cmath>
#include <cfloat>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.src.b0309;

namespace port = pbsd::lib_msun_src::b0309;

extern "C" {
long double ref_fmaximum_magl(long double x, long double y);
float ref_hypotf(float x, float y);
float ref_fminimum_magf(float x, float y);
int ref_isnan(double d);
int ref___isnanf(float f);
int ref___isnanl(long double e);
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

static Stat st_fmaximum_magl = { "fmaximum_magl", 0, 0, 0 };
static Stat st_hypotf = { "hypotf", 0, 0, 0 };
static Stat st_fminimum_magf = { "fminimum_magf", 0, 0, 0 };
static Stat st_isnan = { "isnan", 0, 0, 0 };
static Stat st_isnanf = { "__isnanf", 0, 0, 0 };
static Stat st_isnanl = { "__isnanl", 0, 0, 0 };

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

static void check_fmaximum_magl(long double x, long double y, const char *tag)
{
	ldrep p, o;

	st_fmaximum_magl.cases++;

	p = ldbits(port::fmaximum_magl(x, y));
	o = ldbits(ref_fmaximum_magl(x, y));

	if (guarded_equal(p.b, o.b, sizeof(p.b)))
		return;

	st_fmaximum_magl.fails++;
	if (should_print(st_fmaximum_magl)) {
		std::size_t i;

		std::printf("fmaximum_magl FAIL [%s] port=", tag);
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", p.b[i]);
		std::printf(" ref=");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", o.b[i]);
		std::printf("\n");
	}
}

static void check_hypotf(float x, float y, const char *tag)
{
	std::uint32_t p, o;

	st_hypotf.cases++;

	p = fbits(port::hypotf(x, y));
	o = fbits(ref_hypotf(x, y));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_hypotf.fails++;
	if (should_print(st_hypotf))
		std::printf("hypotf FAIL [%s] x=%#010x y=%#010x port=%#010x ref=%#010x\n",
		    tag, fbits(x), fbits(y), p, o);
}

static void check_fminimum_magf(float x, float y, const char *tag)
{
	std::uint32_t p, o;

	st_fminimum_magf.cases++;

	p = fbits(port::fminimum_magf(x, y));
	o = fbits(ref_fminimum_magf(x, y));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_fminimum_magf.fails++;
	if (should_print(st_fminimum_magf))
		std::printf("fminimum_magf FAIL [%s] x=%#010x y=%#010x port=%#010x ref=%#010x\n",
		    tag, fbits(x), fbits(y), p, o);
}

static void check_isnan(double x, const char *tag)
{
	int p, o;

	st_isnan.cases++;

	p = port::isnan(x);
	o = ref_isnan(x);

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_isnan.fails++;
	if (should_print(st_isnan))
		std::printf("isnan FAIL [%s] x=%#018llx port=%d ref=%d\n",
		    tag, (unsigned long long)dbits(x), p, o);
}

static void check_isnanf(float x, const char *tag)
{
	int p, o;

	st_isnanf.cases++;

	p = port::__isnanf(x);
	o = ref___isnanf(x);

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_isnanf.fails++;
	if (should_print(st_isnanf))
		std::printf("__isnanf FAIL [%s] x=%#010x port=%d ref=%d\n",
		    tag, fbits(x), p, o);
}

static void check_isnanl(long double x, const char *tag)
{
	int p, o;

	st_isnanl.cases++;

	p = port::__isnanl(x);
	o = ref___isnanl(x);

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_isnanl.fails++;
	if (should_print(st_isnanl)) {
		ldrep r = ldbits(x);
		std::printf("__isnanl FAIL [%s] x=", tag);
		for (std::size_t i = LD_SIG; i-- > 0;)
			std::printf("%02x", r.b[i]);
		std::printf(" port=%d ref=%d\n", p, o);
	}
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
	0x4a800000u, 0xca800000u,
	0x58800000u, 0xd8800000u,
	0x58800001u, 0xd8800001u,
	0x5f000000u, 0xdf000000u,
	0x7effffffu, 0xfeffffffu,
	0x7f000000u, 0xff000000u,
	0x7f7ffffeu, 0xff7ffffeu,
	0x7f7fffffu, 0xff7fffffu,
	0x7f800000u, 0xff800000u,
	0x7f800001u, 0xff800001u,
	0x7fc00000u, 0xffc00000u,
	0x7fffffffu, 0xffffffffu,
	0x26800000u, 0xa6800000u,
	0x267fffffu, 0xa67fffffu,
	0x55555555u, 0xd5555555u,
	0x80808080u, 0x7f7f7f7fu,
};
static const unsigned NFVEC = (unsigned)(sizeof fvec / sizeof fvec[0]);

static const std::uint64_t dvec[] = {
	0x0000000000000000ull, 0x8000000000000000ull,
	0x0000000000000001ull, 0x8000000000000001ull,
	0x0000000000000080ull, 0x8000000000000080ull,
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

static std::uint64_t rng_state = 0xb0309a5a5a5a5a5aull;

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
		std::uint32_t e = 118u + next_mod(61u);
		return (r & 0x80000000u) | (e << 23) | (r & 0x007fffffu);
	}
	case 4:
		return (r & 0x80000000u) | ((0x7f800000u + next_mod(17u) - 8u)
		    & 0x7fffffffu);
	case 5:
		return (r & 0x80000000u) |
		    ((0x58800000u + next_mod(9u) - 4u) & 0x7fffffffu);
	case 6:
		return (r & 0x80000000u) |
		    ((0x26800000u + next_mod(9u) - 4u) & 0x7fffffffu);
	case 7:
		return (r & 0x80000000u) |
		    ((0x007fffffu + next_mod(5u) - 2u) & 0x7fffffffu);
	case 8:
		return r ^ (1u << (next_mod(32u)));
	case 9:
		return 0x7f800000u ^ (r & 0x007fffffu);
	case 10:
		return 0x7fc00000u ^ (r & 0x003fffffu);
	default:
		return r ^ next_u32();
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
	case 6:
		return 0x7ff0000000000000ull ^ (r & 0x000fffffffffffffull);
	case 7:
		return 0x7ff8000000000000ull ^ (r & 0x0007ffffffffffffull);
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
		    (0x7fffu + (next_mod(5u) - 2u)));
		return mkld(se, m | 0x8000000000000000ull);
	case 3:
		se = (std::uint16_t)((se & 0x8000u) | 0x7fffu);
		return mkld(se, m);
	case 4:
		se = (std::uint16_t)((se & 0x8000u) | 0x3fffu);
		return mkld(se, 0x8000000000000000ull);
	case 5:
		return mkld(0x3fffu, m);
	case 6:
		return mkld(0xbfffu, m);
	case 7:
		return mkld(ldvec[next_mod(NLDVEC)].se,
		    ldvec[next_mod(NLDVEC)].m);
	default:
		return mkld(se, m ^ next_u64());
	}
}

static void run_fmaximum_magl_tests(void)
{
	unsigned i, j;

	for (i = 0; i < NLDVEC; i++) {
		for (j = 0; j < NLDVEC; j++)
			check_fmaximum_magl(mkld(ldvec[i].se, ldvec[i].m),
			    mkld(ldvec[j].se, ldvec[j].m), "cross");
	}

	check_fmaximum_magl(0.0L, -0.0L, "signed-zero");
	check_fmaximum_magl(-0.0L, 0.0L, "signed-zero");
	check_fmaximum_magl(2.0L, -3.0L, "mag-order");
	check_fmaximum_magl(-3.0L, 2.0L, "mag-order");
	check_fmaximum_magl(2.0L, -2.0L, "mag-tie");
	check_fmaximum_magl(-2.0L, 2.0L, "mag-tie");
	check_fmaximum_magl(2.0L, 2.0L, "mag-equal");
	check_fmaximum_magl(-2.0L, -2.0L, "mag-equal");

	for (i = 0; i <= 32767u; i++) {
		std::uint16_t infse = (std::uint16_t)i;
		std::uint16_t nanse = (std::uint16_t)(i | 0x8000u);
		std::uint64_t infm = 0x8000000000000000ull;
		std::uint64_t nanm = 0x8000000000000001ull;
		long double normal = 1.0L;

		check_fmaximum_magl(normal, mkld(infse, infm), "ld-nan-boundary");
		check_fmaximum_magl(mkld(infse, nanm), normal, "ld-nan-boundary");
		check_fmaximum_magl(normal, mkld(nanse, nanm), "ld-nan-boundary");
		check_fmaximum_magl(mkld(nanse, nanm), normal, "ld-nan-boundary");
	}

	for (i = 0; i < NLDVEC; i++) {
		long double x = mkld(ldvec[i].se, ldvec[i].m);

		check_fmaximum_magl(x, x + 1.0L, "cmp-order");
		check_fmaximum_magl(x, x - 1.0L, "cmp-order");
		check_fmaximum_magl(x, -x, "cmp-neg");
		check_fmaximum_magl(x, x, "cmp-tie");
	}

	for (long long n = 0; n < RANDOM_ITERS; n++)
		check_fmaximum_magl(rand_ld(), rand_ld(), "rand");
}

static void run_hypotf_tests(void)
{
	unsigned i, j;

	for (i = 0; i < NFVEC; i++) {
		for (j = 0; j < NFVEC; j++)
			check_hypotf(fromfbits(fvec[i]), fromfbits(fvec[j]),
			    "cross");
	}

	check_hypotf(0.0f, 0.0f, "zero");
	check_hypotf(-0.0f, 0.0f, "neg-zero");
	check_hypotf(3.0f, 4.0f, "pythag");
	check_hypotf(-3.0f, 4.0f, "pythag-sign");
	check_hypotf(1.0f, 1.0f, "unit");
	check_hypotf(INFINITY, 1.0f, "inf-x");
	check_hypotf(1.0f, INFINITY, "inf-y");
	check_hypotf(-INFINITY, 2.0f, "neg-inf");
	check_hypotf(NAN, 1.0f, "nan-x");
	check_hypotf(1.0f, NAN, "nan-y");
	check_hypotf(NAN, INFINITY, "nan-inf");
	check_hypotf(INFINITY, NAN, "inf-nan");

	/* hb > ha swap */
	check_hypotf(fromfbits(0x3f800000u), fromfbits(0x40000000u), "swap");
	check_hypotf(fromfbits(0x40000000u), fromfbits(0x3f800000u), "no-swap");

	/* (ha-hb) > 0xf000000 early return */
	check_hypotf(fromfbits(0x7f000000u), fromfbits(0x00800000u), "ratio");
	check_hypotf(fromfbits(0x5f000000u), fromfbits(0x00800000u), "ratio2");

	/* ha > 0x58800000 scale down path */
	check_hypotf(fromfbits(0x58800001u), fromfbits(0x3f800000u), "huge-a");
	check_hypotf(fromfbits(0x5f000000u), fromfbits(0x4f000000u), "huge-pair");

	/* hb == 0 */
	check_hypotf(fromfbits(0x3f800000u), 0.0f, "b-zero");
	check_hypotf(fromfbits(0x58800001u), 0.0f, "huge-b-zero");

	/* subnormal b */
	check_hypotf(fromfbits(0x3f800000u), fromfbits(0x00000001u), "subnorm");
	check_hypotf(fromfbits(0x3f800000u), fromfbits(0x007fffffu),
	    "max-subnorm");

	/* hb < 0x26800000 scale up (not subnormal) */
	check_hypotf(fromfbits(0x4a800000u), fromfbits(0x26800000u), "scale-up");
	check_hypotf(fromfbits(0x58800001u), fromfbits(0x01000000u),
	    "scale-up2");

	/* inf/nan in huge path */
	check_hypotf(fromfbits(0x7f800000u), fromfbits(0x3f800000u), "inf-a");
	check_hypotf(fromfbits(0x3f800000u), fromfbits(0x7f800000u), "inf-b");
	check_hypotf(fromfbits(0x7f800001u), fromfbits(0x3f800000u), "snan-a");
	check_hypotf(fromfbits(0x7fc00000u), fromfbits(0x7f800000u), "qnan-inf");

	/* medium path w>b vs w<=b */
	check_hypotf(fromfbits(0x4a000000u), fromfbits(0x3f000000u), "wgt-b");
	check_hypotf(fromfbits(0x40800000u), fromfbits(0x40000000u), "wle-b");
	check_hypotf(fromfbits(0x3fa00000u), fromfbits(0x3f900000u), "close");

	for (unsigned ha = 0x58800000u; ha <= 0x58800010u; ha++) {
		for (unsigned hb = 0u; hb <= 0x10u; hb++) {
			check_hypotf(fromfbits((ha << 0) | 0x00000000u),
			    fromfbits(hb), "ha-grid");
			check_hypotf(fromfbits((ha << 0) | 0x80000000u),
			    fromfbits(hb | 0x80000000u), "ha-grid-neg");
		}
	}

	for (unsigned d = 0; d <= 0x20u; d++) {
		std::uint32_t ha = 0x58800001u + d;
		std::uint32_t hb = 0x26800000u - d;
		check_hypotf(fromfbits(ha), fromfbits(hb), "boundary-walk");
		check_hypotf(fromfbits(hb), fromfbits(ha), "boundary-walk-swap");
	}

	for (long long n = 0; n < RANDOM_ITERS; n++)
		check_hypotf(fromfbits(rand_fbits()), fromfbits(rand_fbits()),
		    "rand");
}

static void run_fminimum_magf_tests(void)
{
	unsigned i, j;

	for (i = 0; i < NFVEC; i++) {
		for (j = 0; j < NFVEC; j++)
			check_fminimum_magf(fromfbits(fvec[i]),
			    fromfbits(fvec[j]), "cross");
	}

	check_fminimum_magf(0.0f, -0.0f, "signed-zero");
	check_fminimum_magf(-0.0f, 0.0f, "signed-zero");
	check_fminimum_magf(2.0f, -3.0f, "mag-order");
	check_fminimum_magf(-3.0f, 2.0f, "mag-order");
	check_fminimum_magf(2.0f, -2.0f, "mag-tie");
	check_fminimum_magf(-2.0f, 2.0f, "mag-tie");
	check_fminimum_magf(2.0f, 2.0f, "mag-equal");
	check_fminimum_magf(-2.0f, -2.0f, "mag-equal");

	for (i = 0; i <= 255u; i++) {
		std::uint32_t infp = (i << 23) | 0x80000000u;
		std::uint32_t infn = (i << 23);
		std::uint32_t nanp = infn | 1u;
		std::uint32_t nann = infp | 1u;
		std::uint32_t normal = 0x3f800000u;

		check_fminimum_magf(fromfbits(normal), fromfbits(nanp),
		    "nan-boundary");
		check_fminimum_magf(fromfbits(nanp), fromfbits(normal),
		    "nan-boundary");
		check_fminimum_magf(fromfbits(normal), fromfbits(nann),
		    "nan-boundary");
		check_fminimum_magf(fromfbits(nann), fromfbits(normal),
		    "nan-boundary");
	}

	for (i = 0; i < NFVEC; i++) {
		float x = fromfbits(fvec[i]);

		check_fminimum_magf(x, x + 1.0f, "cmp-order");
		check_fminimum_magf(x, x - 1.0f, "cmp-order");
		check_fminimum_magf(x, -x, "cmp-neg");
		check_fminimum_magf(x, x, "cmp-tie");
	}

	for (unsigned bit = 0; bit < 32; bit++) {
		std::uint32_t m = 1u << bit;
		check_fminimum_magf(fromfbits(m), fromfbits(~m), "bit");
	}

	for (long long n = 0; n < RANDOM_ITERS; n++)
		check_fminimum_magf(fromfbits(rand_fbits()),
		    fromfbits(rand_fbits()), "rand");
}

static void run_isnan_tests(void)
{
	unsigned i, j;

	for (i = 0; i < NDVEC; i++)
		check_isnan(fromdbits(dvec[i]), "dvec");

	for (i = 0; i < NFVEC; i++)
		check_isnanf(fromfbits(fvec[i]), "fvec");

	for (unsigned bit = 0; bit < 64; bit++) {
		std::uint64_t m = 1ull << bit;
		check_isnan(fromdbits(m), "bit");
		check_isnan(fromdbits(~m), "bit-not");
	}

	for (unsigned bit = 0; bit < 32; bit++) {
		std::uint32_t m = 1u << bit;
		check_isnanf(fromfbits(m), "bit");
		check_isnanf(fromfbits(~m), "bit-not");
	}

	for (i = 0; i < NLDVEC; i++)
		check_isnanl(mkld(ldvec[i].se, ldvec[i].m), "ldvec");

	for (i = 0; i <= 2047u; i++) {
		std::uint64_t infp = ((std::uint64_t)i << 52) | 0x8000000000000000ull;
		std::uint64_t infn = ((std::uint64_t)i << 52);
		std::uint64_t nanp = infn | 1ull;
		std::uint64_t nann = infp | 1ull;
		std::uint64_t normal = 0x3ff0000000000000ull;

		check_isnan(fromdbits(normal), "exp-boundary");
		check_isnan(fromdbits(infp), "exp-boundary");
		check_isnan(fromdbits(infn), "exp-boundary");
		check_isnan(fromdbits(nanp), "exp-boundary");
		check_isnan(fromdbits(nann), "exp-boundary");
	}

	for (i = 0; i <= 255u; i++) {
		std::uint32_t infp = (i << 23) | 0x80000000u;
		std::uint32_t infn = (i << 23);
		std::uint32_t nanp = infn | 1u;
		std::uint32_t nann = infp | 1u;
		std::uint32_t normal = 0x3f800000u;

		check_isnanf(fromfbits(normal), "exp-boundary");
		check_isnanf(fromfbits(infp), "exp-boundary");
		check_isnanf(fromfbits(infn), "exp-boundary");
		check_isnanf(fromfbits(nanp), "exp-boundary");
		check_isnanf(fromfbits(nann), "exp-boundary");
	}

	for (i = 0; i <= 32767u; i++) {
		std::uint16_t infse = (std::uint16_t)i;
		std::uint16_t nanse = (std::uint16_t)(i | 0x8000u);
		std::uint64_t infm = 0x8000000000000000ull;
		std::uint64_t nanm = 0x8000000000000001ull;
		long double normal = 1.0L;

		check_isnanl(normal, "exp-boundary");
		check_isnanl(mkld(infse, infm), "exp-boundary");
		check_isnanl(mkld(nanse, nanm), "exp-boundary");
		check_isnanl(mkld(infse, nanm), "exp-boundary");
	}

	for (long long n = 0; n < RANDOM_ITERS; n++)
		check_isnan(fromdbits(rand_dbits()), "rand");
	for (long long n = 0; n < RANDOM_ITERS; n++)
		check_isnanf(fromfbits(rand_fbits()), "rand");
	for (long long n = 0; n < RANDOM_ITERS; n++)
		check_isnanl(rand_ld(), "rand");
}

int main(void)
{
	run_fmaximum_magl_tests();
	run_hypotf_tests();
	run_fminimum_magf_tests();
	run_isnan_tests();

	Stat *all[] = {
		&st_fmaximum_magl, &st_hypotf, &st_fminimum_magf,
		&st_isnan, &st_isnanf, &st_isnanl,
	};
	const unsigned nall = (unsigned)(sizeof all / sizeof all[0]);
	long long total_cases = 0;
	long long total_fails = 0;

	std::printf("\n%-16s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-16s %12s %12s\n", "----------------", "------------",
	    "------------");
	for (unsigned k = 0; k < nall; k++) {
		std::printf("%-16s %12lld %12lld\n", all[k]->name,
		    all[k]->cases, all[k]->fails);
		total_cases += all[k]->cases;
		total_fails += all[k]->fails;
	}
	std::printf("%-16s %12s %12s\n", "----------------", "------------",
	    "------------");
	std::printf("%-16s %12lld %12lld\n", "TOTAL", total_cases,
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
