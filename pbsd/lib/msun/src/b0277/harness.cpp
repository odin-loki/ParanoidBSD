// Differential test for PBSD batch b0277 (fmaximum_magf, acosl, sin).

import pbsd.lib.msun.src.b0277;

#include <cfloat>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace port = pbsd::lib_msun_src::b0277;

extern "C" {
float ref_fmaximum_magf(float x, float y);
long double ref_acosl(long double x);
double ref_sin(double x);
}

static const std::size_t ITERS = 200000;
static const unsigned char GUARD = 0x7f;
static const std::size_t GUARD_BUF = 64;
static const std::size_t GUARD_OFF = 16;

#if LDBL_MANT_DIG == 64
static const std::size_t LD_SIG = 10;
#else
static const std::size_t LD_SIG = sizeof(long double);
#endif

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned long long printed;
};

static Stat st_fmaximum_magf = { "fmaximum_magf", 0, 0, 0 };
static Stat st_acosl = { "acosl", 0, 0, 0 };
static Stat st_sin = { "sin", 0, 0, 0 };

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

static std::uint64_t rng_state = 0xb0277a5a5a5a5a5aull;

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

static void check_fmaximum_magf(float x, float y, const char *tag)
{
	std::uint32_t p, o;

	st_fmaximum_magf.cases++;

	p = fbits(port::fmaximum_magf(x, y));
	o = fbits(ref_fmaximum_magf(x, y));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_fmaximum_magf.fails++;
	if (should_print(st_fmaximum_magf))
		std::printf("fmaximum_magf FAIL [%s] x=%#010x y=%#010x port=%#010x ref=%#010x\n",
		    tag, fbits(x), fbits(y), p, o);
}

static void check_acosl(long double x, const char *tag)
{
	ldrep p, o;

	st_acosl.cases++;

	p = ldbits(port::acosl(x));
	o = ldbits(ref_acosl(x));

	if (guarded_equal(p.b, o.b, sizeof(p.b)))
		return;

	st_acosl.fails++;
	if (should_print(st_acosl)) {
		std::size_t i;
		std::printf("acosl FAIL [%s] port=", tag);
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", p.b[i]);
		std::printf(" ref=");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", o.b[i]);
		std::printf("\n");
	}
}

static void check_sin(double x, const char *tag)
{
	std::uint64_t p, o;

	st_sin.cases++;

	p = dbits(port::sin(x));
	o = dbits(ref_sin(x));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_sin.fails++;
	if (should_print(st_sin))
		std::printf("sin FAIL [%s] x=%#018llx port=%#018llx ref=%#018llx\n",
		    tag, (unsigned long long)dbits(x),
		    (unsigned long long)p, (unsigned long long)o);
}

static const std::uint32_t kEdgeF[] = {
	0x00000000u, 0x80000000u,
	0x00000001u, 0x80000001u,
	0x7f800000u, 0xff800000u,
	0x7fc00000u, 0xffc00000u,
	0x7f800001u, 0xff800001u,
	0x3f800000u, 0xbf800000u,
	0x40000000u, 0xc0000000u,
	0x3f000000u, 0xbf000000u,
	0x40800000u, 0xc0800000u,
	0x40400000u, 0xc0400000u,
	0x3f7fffffu, 0xbf7fffffu,
	0x3f800001u, 0xbf800001u,
	0x7f7fffffu, 0xff7fffffu,
};

static const std::uint64_t kEdgeD[] = {
	0x0000000000000000ULL, 0x8000000000000000ULL,
	0x0000000000000001ULL, 0x8000000000000001ULL,
	0x3e4fffff00000000ULL, 0xbe4fffff00000000ULL,
	0x3e50000000000000ULL, 0xbe50000000000000ULL,
	0x3e50000100000000ULL, 0xbe50000100000000ULL,
	0x3fe921fb00000000ULL, 0xbfe921fb00000000ULL,
	0x3fe921fa80000000ULL, 0xbfe921fa80000000ULL,
	0x3fe921fb54442d18ULL, 0xbfe921fb54442d18ULL,
	0x3ff921fb00000000ULL, 0xbff921fb00000000ULL,
	0x4002d97c00000000ULL, 0xc002d97c00000000ULL,
	0x400921fb00000000ULL, 0xc00921fb00000000ULL,
	0x400f6a7a00000000ULL, 0xc00f6a7a00000000ULL,
	0x4012d97c00000000ULL, 0xc012d97c00000000ULL,
	0x401921fb00000000ULL, 0xc01921fb00000000ULL,
	0x413921fb00000000ULL, 0xc13921fb00000000ULL,
	0x7ff0000000000000ULL, 0xfff0000000000000ULL,
	0x7ff8000000000000ULL, 0xfff8000000000000ULL,
	0x7fffffffffffffffULL, 0xffffffffffffffffULL,
};

struct ldvec {
	std::uint16_t se;
	std::uint64_t m;
};

static const ldvec kEdgeLD[] = {
	{ 0x0000, 0x8000000000000000ull },
	{ 0x8000, 0x8000000000000000ull },
	{ 0x3fff, 0x8000000000000000ull }, /* 1.0 */
	{ 0xbfff, 0x8000000000000000ull }, /* -1.0 */
	{ 0x3ffe, 0x8000000000000000ull }, /* 0.5 */
	{ 0xbffe, 0x8000000000000000ull }, /* -0.5 */
	{ 0x3ffe, 0x8000000000000001ull }, /* just above 0.5 */
	{ 0xbffe, 0x8000000000000001ull }, /* just below -0.5 */
	{ 0x3fff, 0x8000000000000001ull }, /* just above 1 */
	{ 0xbfff, 0x8000000000000001ull }, /* just below -1 */
	{ 0x4000, 0x8000000000000000ull }, /* 2.0 */
	{ 0xc000, 0x8000000000000000ull }, /* -2.0 */
	{ 0x7fff, 0x8000000000000000ull }, /* inf */
	{ 0xffff, 0x8000000000000000ull }, /* -inf */
	{ 0x7fff, 0x8000000000000001ull }, /* NaN */
	{ 0x3fff, 0x0000000000000000ull }, /* 1.0 without integer bit */
	{ 0x3ffc, 0x8000000000000000ull }, /* 0.25 */
	{ 0x3c00, 0x8000000000000000ull }, /* tiny ~2^-63 */
	{ 0x3bfe, 0x8000000000000000ull }, /* below ACOS_CONST */
	{ 0x3bff, 0x8000000000000000ull }, /* above ACOS_CONST */
};

static void edge_cases(void)
{
	std::size_t i, j;
	const std::size_t nf = sizeof(kEdgeF) / sizeof(kEdgeF[0]);
	const std::size_t nd = sizeof(kEdgeD) / sizeof(kEdgeD[0]);
	const std::size_t nld = sizeof(kEdgeLD) / sizeof(kEdgeLD[0]);

	for (i = 0; i < nf; i++) {
		for (j = 0; j < nf; j++)
			check_fmaximum_magf(fromfbits(kEdgeF[i]),
			    fromfbits(kEdgeF[j]), "edge-pair");
		check_fmaximum_magf(fromfbits(kEdgeF[i]), fromfbits(kEdgeF[i]),
		    "edge-self");
	}

	/* NaN boundary: exp==255 && man!=0 */
	for (i = 0; i < 256u; i++) {
		std::uint32_t nanx = 0x7f800000u | (i << 15);
		std::uint32_t nany = 0xff800000u | (i << 15);
		if ((nanx & 0x7fffffu) == 0)
			continue;
		check_fmaximum_magf(fromfbits(nanx), 1.0f, "nan-boundary");
		check_fmaximum_magf(1.0f, fromfbits(nany), "nan-boundary");
		check_fmaximum_magf(fromfbits(nanx), fromfbits(nany), "nan-boundary");
	}

	/* Magnitude order and equal-magnitude sign tie. */
	check_fmaximum_magf(2.0f, 1.0f, "mag-order");
	check_fmaximum_magf(-2.0f, 1.0f, "mag-order");
	check_fmaximum_magf(1.0f, 2.0f, "mag-order");
	check_fmaximum_magf(1.0f, -2.0f, "mag-order");
	check_fmaximum_magf(1.0f, 1.0f, "mag-tie");
	check_fmaximum_magf(-1.0f, 1.0f, "sign-tie");
	check_fmaximum_magf(1.0f, -1.0f, "sign-tie");
	check_fmaximum_magf(0.0f, -0.0f, "signed-zero");
	check_fmaximum_magf(-0.0f, 0.0f, "signed-zero");

	for (i = 0; i < nld; i++)
		check_acosl(mkld(kEdgeLD[i].se, kEdgeLD[i].m), "edge");

	/* Branch boundaries around |x|=1, 0.5, tiny. */
	check_acosl(1.0L, "unit");
	check_acosl(-1.0L, "unit");
	check_acosl(1.0L + 1e-18L, "gt-one");
	check_acosl(-1.0L - 1e-18L, "lt-minus-one");
	check_acosl(0.5L, "half");
	check_acosl(-0.5L, "neg-half");
	check_acosl(0.5L + 1e-18L, "gt-half");
	check_acosl(-0.5L - 1e-18L, "lt-neg-half");
	check_acosl(0.0L, "zero");
	check_acosl(-0.0L, "neg-zero");
	check_acosl(1e-70L, "tiny");
	check_acosl(-1e-70L, "neg-tiny");

	for (i = 0; i < nd; i++)
		check_sin(fromdbits(kEdgeD[i]), "edge");

	check_sin(0.0, "zero");
	check_sin(-0.0, "neg-zero");
	check_sin(1e-30, "tiny");
	check_sin(-1e-30, "neg-tiny");
	check_sin(0.7853981633974483, "pi/4");
	check_sin(-0.7853981633974483, "neg-pi/4");
	check_sin(1.5707963267948966, "pi/2");
	check_sin(3.141592653589793, "pi");
	check_sin(6.283185307179586, "2pi");
	check_sin(1e10, "large");
	check_sin(-1e10, "neg-large");
}

static std::uint32_t rng_f(void)
{
	std::uint32_t u = next_u32();
	unsigned kind = (unsigned)(next_u64() % 100u);

	if (kind < 5)
		u &= 0x80000000u;
	else if (kind < 10)
		u |= 0x7f800000u;
	else if (kind < 15)
		u &= 0x807fffffu;

	return u;
}

static long double rng_ld(void)
{
	std::uint64_t m = next_u64();
	std::uint64_t r = next_u64();
	std::uint16_t sign = (r & 1) ? 0x8000u : 0x0000u;
	unsigned kind = (unsigned)((r >> 1) % 100u);
	std::uint16_t e;

	if (kind < 5)
		e = 0x0000u;
	else if (kind < 10)
		e = 0x7fffu;
	else
		e = (std::uint16_t)(next_u64() & 0x7fffu);

	if (e == 0x0000u)
		m &= ~(std::uint64_t)1 << 63;
	else if (e < 0x7fffu)
		m |= (std::uint64_t)1 << 63;

	return mkld((std::uint16_t)(sign | e), m);
}

static std::uint64_t rng_d(void)
{
	std::uint64_t u = next_u64();
	unsigned kind = (unsigned)(next_u64() % 100u);

	if (kind < 5)
		u &= 0x8000000000000000ULL;
	else if (kind < 10)
		u |= 0x7ff0000000000000ULL;
	else if (kind < 15)
		u &= 0x800fffffffffffffULL;
	else if (kind < 25) {
		std::uint64_t d = next_mod64(11u) - 5u;
		u = (u & 0x8000000000000000ULL) |
		    ((0x3fe921fb00000000ULL + d) & 0x7fffffffffffffffULL);
	} else if (kind < 35) {
		std::uint64_t d = next_mod64(11u) - 5u;
		u = (u & 0x8000000000000000ULL) |
		    ((0x3e50000000000000ULL + d) & 0x7fffffffffffffffULL);
	}

	return u;
}

static void random_sweep(void)
{
	unsigned long long i;

	rng_state = 0xd1ce4e5b91234567ull;
	for (i = 0; i < ITERS; i++) {
		std::uint32_t ux = rng_f();
		std::uint32_t uy = rng_f();

		if ((i & 15) == 0)
			ux = 0x00000000u;
		if ((i & 15) == 1)
			uy = 0x80000000u;
		if ((i & 15) == 2)
			ux = 0x7fc00000u;
		if ((i & 15) == 3)
			uy = 0xffc00000u;
		if ((i & 15) == 4)
			ux = 0x7f800000u;
		if ((i & 15) == 5)
			uy = 0xff800000u;
		if ((i & 15) == 6)
			ux = 0x3f800000u;
		if ((i & 15) == 7)
			uy = 0xbf800000u;

		check_fmaximum_magf(fromfbits(ux), fromfbits(uy), "random");
	}

	rng_state = 0xdecafbadc0ffee01ull;
	for (i = 0; i < ITERS; i++) {
		long double x = rng_ld();

		if ((i & 31) == 0)
			x = 0.0L;
		if ((i & 31) == 1)
			x = -0.0L;
		if ((i & 31) == 2)
			x = 1.0L;
		if ((i & 31) == 3)
			x = -1.0L;
		if ((i & 31) == 4)
			x = 0.5L;
		if ((i & 31) == 5)
			x = -0.5L;
		if ((i & 31) == 6)
			x = __builtin_nanl("");
		if ((i & 31) == 7)
			x = __builtin_infl();

		check_acosl(x, "random");
	}

	rng_state = 0x243f6a8885a308d3ull;
	for (i = 0; i < ITERS; i++) {
		std::uint64_t ux = rng_d();

		if ((i & 15) == 0)
			ux = 0x0000000000000000ULL;
		if ((i & 15) == 1)
			ux = 0x8000000000000000ULL;
		if ((i & 15) == 2)
			ux = 0x3e4fffff00000000ULL;
		if ((i & 15) == 3)
			ux = 0x3e50000000000000ULL;
		if ((i & 15) == 4)
			ux = 0x7ff0000000000000ULL;
		if ((i & 15) == 5)
			ux = 0xfff0000000000000ULL;
		if ((i & 15) == 6)
			ux = 0x7ff8000000000000ULL;
		if ((i & 15) == 7)
			ux = 0x3fe921fb54442d18ULL;

		check_sin(fromdbits(ux), "random");
	}
}

static void row(const Stat &s)
{
	std::printf("  %-16s %12llu %10llu   %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "PASS" : "FAIL");
}

int main(void)
{
	unsigned long long fails;

	std::printf("PBSD batch b0277 differential test\n");
	std::printf("  function              cases     fails   status\n");

	edge_cases();
	random_sweep();

	row(st_fmaximum_magf);
	row(st_acosl);
	row(st_sin);

	fails = st_fmaximum_magf.fails + st_acosl.fails + st_sin.fails;
	return fails == 0 ? 0 : 1;
}
