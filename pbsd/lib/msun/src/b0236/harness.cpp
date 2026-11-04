/*
 * Differential harness for batch b0236 (w_dremf.c, w_cabsf.c, w_cabsl.c,
 * w_cabs.c).
 *
 * Every function is compared bit-for-bit against the unmodified C reference
 * in oracle.c.  Results are staged through 0x7f-guarded buffers so that any
 * stray write past the nominal result window is caught.
 */

#include <cfloat>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.src.b0236;

namespace port = pbsd::lib_msun_src::b0236;

extern "C" {
float ref_dremf(float x, float y);
float ref_cabsf(float _Complex z);
long double ref_cabsl(long double _Complex z);
double ref_cabs(double _Complex z);
}

#if LDBL_MANT_DIG == 64
static const std::size_t LD_SIG = 10;
#else
static const std::size_t LD_SIG = sizeof(long double);
#endif

static const unsigned char GUARD = 0x7f;
static const std::size_t GUARD_BUF = 64;
static const std::size_t GUARD_OFF = 16;

static const unsigned long long ITERS = 200000ull;
static const unsigned MAX_REPORT = 12;

static bool
guarded_equal(const void *pa, const void *pb, std::size_t n)
{
	unsigned char a[GUARD_BUF], b[GUARD_BUF];

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	std::memcpy(a + GUARD_OFF, pa, n);
	std::memcpy(b + GUARD_OFF, pb, n);
	return (std::memcmp(a, b, sizeof(a)) == 0);
}

static std::uint32_t
fbits(float f)
{
	std::uint32_t u;

	std::memcpy(&u, &f, sizeof(u));
	return u;
}

static float
fromfbits(std::uint32_t u)
{
	float f;

	std::memcpy(&f, &u, sizeof(f));
	return f;
}

static std::uint64_t
dbits(double d)
{
	std::uint64_t u;

	std::memcpy(&u, &d, sizeof(u));
	return u;
}

static double
fromdbits(std::uint64_t u)
{
	double d;

	std::memcpy(&d, &u, sizeof(d));
	return d;
}

struct ldrep {
	unsigned char b[sizeof(long double)];
};

static ldrep
ldbits(long double x)
{
	ldrep r;

	std::memset(r.b, 0, sizeof(r.b));
	std::memcpy(r.b, &x, LD_SIG);
	return r;
}

static long double
mkld(std::uint16_t se, std::uint64_t m)
{
	unsigned char b[sizeof(long double)];
	long double x;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &m, sizeof(m));
	std::memcpy(b + 8, &se, sizeof(se));
	std::memcpy(&x, b, sizeof(x));
	return x;
}

static float _Complex
mkfc(float re, float im)
{
	float _Complex z;

	__real__ z = re;
	__imag__ z = im;
	return z;
}

static double _Complex
mkdc(double re, double im)
{
	double _Complex z;

	__real__ z = re;
	__imag__ z = im;
	return z;
}

static long double _Complex
mkldc(long double re, long double im)
{
	long double _Complex z;

	__real__ z = re;
	__imag__ z = im;
	return z;
}

struct stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned reported;
};

static stat st_dremf = { "dremf", 0, 0, 0 };
static stat st_cabsf = { "cabsf", 0, 0, 0 };
static stat st_cabs = { "cabs", 0, 0, 0 };
static stat st_cabsl = { "cabsl", 0, 0, 0 };

static const std::uint32_t fvec[] = {
	0x00000000u, 0x80000000u,
	0x00000001u, 0x80000001u,
	0x00000080u, 0x00008000u,
	0x007fffffu, 0x807fffffu,
	0x00800000u, 0x80800000u,
	0x00800001u,
	0x3f000000u, 0xbf000000u,
	0x3f800000u, 0xbf800000u,
	0x40000000u, 0xc0000000u,
	0x40490fdbu, 0xc0490fdbu,
	0x4b000000u, 0xcb000000u,
	0x7f7fffffu, 0xff7fffffu,
	0x7f7ffffeu,
	0x7f800000u, 0xff800000u,
	0x7f800001u, 0xff800001u,
	0x7fa00000u, 0xffa00000u,
	0x7fbfffffu,
	0x7fc00000u, 0xffc00000u,
	0x7fc00001u, 0x7fffffffu, 0xffffffffu,
	0x80808080u, 0x7f7f7f7fu,
	0x00ff00ffu, 0xff00ff00u,
	0x8000ffffu, 0x0000ff80u,
	0x33333333u, 0xb3333333u,
	0x3f000001u, 0x3f7fffffu, 0x3f800001u,
};
static const std::size_t NFVEC = sizeof(fvec) / sizeof(fvec[0]);

static const std::uint64_t dvec[] = {
	0x0000000000000000ull, 0x8000000000000000ull,
	0x0000000000000001ull, 0x8000000000000001ull,
	0x0000000000000080ull, 0x0008000000000000ull,
	0x000fffffffffffffull, 0x800fffffffffffffull,
	0x0010000000000000ull, 0x8010000000000000ull,
	0x3fe0000000000000ull, 0xbfe0000000000000ull,
	0x3ff0000000000000ull, 0xbff0000000000000ull,
	0x4000000000000000ull, 0xc000000000000000ull,
	0x400921fb54442d18ull, 0xc00921fb54442d18ull,
	0x7fefffffffffffffull, 0xffefffffffffffffull,
	0x7ff0000000000000ull, 0xfff0000000000000ull,
	0x7ff0000000000001ull, 0xfff0000000000001ull,
	0x7ff8000000000000ull, 0xfff8000000000000ull,
	0x7fffffffffffffffull, 0xffffffffffffffffull,
	0x8080808080808080ull, 0x7f7f7f7f7f7f7f7full,
	0x3ff0000000000001ull, 0x3fefffffffffffffull,
};
static const std::size_t NDVEC = sizeof(dvec) / sizeof(dvec[0]);

static void
check_dremf(std::uint32_t xb, std::uint32_t yb, const char *tag)
{
	const float x = fromfbits(xb);
	const float y = fromfbits(yb);
	std::uint32_t got, want;

	st_dremf.cases++;

	got = fbits(port::dremf(x, y));
	want = fbits(ref_dremf(x, y));

	if (guarded_equal(&got, &want, sizeof(got)))
		return;

	st_dremf.fails++;
	if (st_dremf.reported < MAX_REPORT) {
		st_dremf.reported++;
		std::printf("  dremf FAIL [%s] x=%08x y=%08x port=%08x ref=%08x\n",
		    tag, xb, yb, got, want);
	}
}

static void
check_cabsf(std::uint32_t re, std::uint32_t im, const char *tag)
{
	const float _Complex z = mkfc(fromfbits(re), fromfbits(im));
	std::uint32_t got, want;

	st_cabsf.cases++;

	got = fbits(port::cabsf(z));
	want = fbits(ref_cabsf(z));

	if (guarded_equal(&got, &want, sizeof(got)))
		return;

	st_cabsf.fails++;
	if (st_cabsf.reported < MAX_REPORT) {
		st_cabsf.reported++;
		std::printf("  cabsf FAIL [%s] in=(%08x,%08x) port=%08x ref=%08x\n",
		    tag, re, im, got, want);
	}
}

static void
check_cabs(std::uint64_t re, std::uint64_t im, const char *tag)
{
	const double _Complex z = mkdc(fromdbits(re), fromdbits(im));
	std::uint64_t got, want;

	st_cabs.cases++;

	got = dbits(port::cabs(z));
	want = dbits(ref_cabs(z));

	if (guarded_equal(&got, &want, sizeof(got)))
		return;

	st_cabs.fails++;
	if (st_cabs.reported < MAX_REPORT) {
		st_cabs.reported++;
		std::printf("  cabs FAIL [%s] in=(%016llx,%016llx) "
		    "port=%016llx ref=%016llx\n",
		    tag, (unsigned long long)re, (unsigned long long)im,
		    (unsigned long long)got, (unsigned long long)want);
	}
}

static void
check_cabsl(long double re, long double im, const char *tag)
{
	const long double _Complex z = mkldc(re, im);
	ldrep got, want;

	st_cabsl.cases++;

	got = ldbits(port::cabsl(z));
	want = ldbits(ref_cabsl(z));

	if (guarded_equal(&got, &want, sizeof(got)))
		return;

	st_cabsl.fails++;
	if (st_cabsl.reported < MAX_REPORT) {
		std::size_t i;

		st_cabsl.reported++;
		std::printf("  cabsl FAIL [%s] port=", tag);
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", got.b[i]);
		std::printf(" ref=");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", want.b[i]);
		std::printf("\n");
	}
}

static void
check_cabsl_bits(ldrep re, ldrep im, const char *tag)
{
	long double r, i;

	std::memcpy(&r, re.b, sizeof(r));
	std::memcpy(&i, im.b, sizeof(i));
	check_cabsl(r, i, tag);
}

static std::uint64_t rng_state;

static std::uint64_t
rng_next(void)
{
	std::uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ull;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
	return (z ^ (z >> 31));
}

static std::uint32_t
rand32(void)
{
	return (std::uint32_t)rng_next();
}

static std::uint64_t
rand64(void)
{
	return rng_next();
}

static void
dremf_edge_cases(void)
{
	std::size_t i, j;

	for (i = 0; i < NFVEC; i++)
		for (j = 0; j < NFVEC; j++)
			check_dremf(fvec[i], fvec[j], "cross");

	for (i = 0; i < NFVEC; i++) {
		check_dremf(fvec[i], 0u, "y-zero");
		check_dremf(0u, fvec[i], "x-zero");
		check_dremf(fvec[i], 0x80000000u, "y-negzero");
		check_dremf(0x80000000u, fvec[i], "x-negzero");
		check_dremf(fvec[i], fvec[i], "equal");
		check_dremf(fvec[i], fvec[i] ^ 0x80000000u, "opp-sign");
	}

	static const std::uint32_t walk[] = {
		0x00000000u, 0x80000000u, 0x00800000u, 0x007fffffu,
		0x7f800000u, 0x7f7fffffu, 0x3f800000u, 0x40000000u,
	};
	for (std::size_t k = 0; k < sizeof walk / sizeof walk[0]; k++) {
		for (int d = -32; d <= 32; d++) {
			std::uint32_t xb = walk[k] + (std::uint32_t)d;
			std::uint32_t yb = walk[k] + (std::uint32_t)d + 1u;

			check_dremf(xb, yb, "walk");
			check_dremf(yb, xb, "walk");
		}
	}
}

static void
cabsf_edge_cases(void)
{
	std::size_t i, j;

	for (i = 0; i < NFVEC; i++)
		for (j = 0; j < NFVEC; j++)
			check_cabsf(fvec[i], fvec[j], "cross");

	for (i = 0; i < NFVEC; i++) {
		check_cabsf(fvec[i], 0u, "real-only");
		check_cabsf(0u, fvec[i], "imag-only");
		check_cabsf(fvec[i], fvec[i], "diag");
		check_cabsf(fvec[i], fvec[i] ^ 0x80000000u, "diag");
		check_cabsf(fvec[i], 0x80000000u, "imag-negzero");
		check_cabsf(0x80000000u, fvec[i], "real-negzero");
	}

	check_cabsf(0u, 0u, "origin");
	check_cabsf(0x80000000u, 0x80000000u, "origin");
}

static void
cabs_edge_cases(void)
{
	std::size_t i, j;

	for (i = 0; i < NDVEC; i++)
		for (j = 0; j < NDVEC; j++)
			check_cabs(dvec[i], dvec[j], "cross");

	for (i = 0; i < NDVEC; i++) {
		check_cabs(dvec[i], 0ull, "real-only");
		check_cabs(0ull, dvec[i], "imag-only");
		check_cabs(dvec[i], dvec[i], "diag");
		check_cabs(dvec[i], dvec[i] ^ 0x8000000000000000ull, "diag");
		check_cabs(dvec[i], 0x8000000000000000ull, "imag-negzero");
		check_cabs(0x8000000000000000ull, dvec[i], "real-negzero");
	}
}

static void
cabsl_edge_cases(void)
{
	static const struct {
		std::uint16_t se;
		std::uint64_t m;
	} ldtab[] = {
		{ 0x0000, 0x0000000000000000ull },
		{ 0x8000, 0x0000000000000000ull },
		{ 0x0000, 0x0000000000000001ull },
		{ 0x8000, 0x0000000000000001ull },
		{ 0x3fff, 0x8000000000000000ull },
		{ 0xbfff, 0x8000000000000000ull },
		{ 0x3fff, 0x8000000000000001ull },
		{ 0x4000, 0x8000000000000000ull },
		{ 0xc000, 0x8000000000000000ull },
		{ 0x4000, 0xc000000000000000ull },
		{ 0x7fff, 0x8000000000000000ull },
		{ 0xffff, 0x8000000000000000ull },
		{ 0x7fff, 0xc000000000000000ull },
		{ 0x7ffe, 0xffffffffffffffffull },
		{ 0x3ffe, 0xffffffffffffffffull },
	};
	const std::size_t nld = sizeof(ldtab) / sizeof(ldtab[0]);
	std::size_t i, j;

	for (i = 0; i < nld; i++)
		for (j = 0; j < nld; j++)
			check_cabsl(mkld(ldtab[i].se, ldtab[i].m),
			    mkld(ldtab[j].se, ldtab[j].m), "cross");

	for (i = 0; i < nld; i++) {
		long double re = mkld(ldtab[i].se, ldtab[i].m);
		long double z = 0.0L;

		check_cabsl(re, z, "real-only");
		check_cabsl(z, re, "imag-only");
		check_cabsl(re, re, "diag");
		check_cabsl(re, -re, "diag");
	}

	check_cabsl(0.0L, 0.0L, "origin");
	check_cabsl(-0.0L, -0.0L, "origin");
}

static void
dremf_random(void)
{
	unsigned long long i;

	for (i = 0; i < ITERS; i++) {
		std::uint32_t xb = rand32();
		std::uint32_t yb = rand32();

		if ((i & 7) == 0)
			yb = fvec[i % NFVEC];
		if ((i & 7) == 1)
			xb = fvec[i % NFVEC];
		if ((i & 15) == 2)
			yb = 0u;
		if ((i & 15) == 3)
			xb = 0u;

		check_dremf(xb, yb, "random");
	}
}

static void
cabsf_random(void)
{
	unsigned long long i;

	for (i = 0; i < ITERS; i++) {
		std::uint32_t re = rand32();
		std::uint32_t im = rand32();

		if ((i & 7) == 0)
			im = 0u;
		if ((i & 7) == 1)
			re = 0u;
		if ((i & 7) == 2)
			im = re;

		check_cabsf(re, im, "random");
	}
}

static void
cabs_random(void)
{
	unsigned long long i;

	for (i = 0; i < ITERS; i++) {
		std::uint64_t re = rand64();
		std::uint64_t im = rand64();

		if ((i & 7) == 0)
			im = 0ull;
		if ((i & 7) == 1)
			re = 0ull;
		if ((i & 7) == 2)
			im = re;

		check_cabs(re, im, "random");
	}
}

static void
cabsl_random(void)
{
	unsigned long long i;

	for (i = 0; i < ITERS; i++) {
		ldrep re, im;
		std::uint64_t r = rand64();

		std::memset(re.b, 0, sizeof(re.b));
		std::memset(im.b, 0, sizeof(im.b));
		std::memcpy(re.b, &r, sizeof(r));
		std::memcpy(im.b, (const unsigned char *)&r + 1, LD_SIG > 7 ? 7 : LD_SIG);

		if ((i & 7) == 0)
			std::memset(im.b, 0, LD_SIG);
		if ((i & 7) == 1)
			std::memset(re.b, 0, LD_SIG);
		if ((i & 31) == 2)
			std::memcpy(im.b, re.b, LD_SIG);

		check_cabsl_bits(re, im, "random");
	}
}

static void
row(const stat &s)
{
	std::printf("  %-14s %12llu %10llu   %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "PASS" : "FAIL");
}

int
main(void)
{
	unsigned long long fails;

	std::printf("pbsd batch b0236 differential test\n\n");

	rng_state = 0xd1b0236b97f4a7c1ull;

	dremf_edge_cases();
	cabsf_edge_cases();
	cabs_edge_cases();
	cabsl_edge_cases();

	dremf_random();
	cabsf_random();
	cabs_random();
	cabsl_random();

	std::printf("\n  %-14s %12s %10s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("  ----------------------------------------------------\n");
	row(st_dremf);
	row(st_cabsf);
	row(st_cabs);
	row(st_cabsl);

	fails = st_dremf.fails + st_cabsf.fails + st_cabs.fails +
	    st_cabsl.fails;
	std::printf("\n%s: %llu total failures\n",
	    fails == 0 ? "PASS" : "FAIL", fails);

	if (st_dremf.cases == 0 || st_cabsf.cases == 0 || st_cabs.cases == 0 ||
	    st_cabsl.cases == 0) {
		std::printf("not every function was exercised\n");
		return 1;
	}

	return (fails == 0 ? 0 : 1);
}
