// Differential test for PBSD batch b0287 (remquo, log2, atan2, fmodl).

import pbsd.lib.msun.src.b0287;

#include <cfloat>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace port = pbsd::lib_msun_src::b0287;

extern "C" {
double ref_remquo(double x, double y, int *quo);
double ref_log2(double x);
double ref_atan2(double y, double x);
long double ref_fmodl(long double x, long double y);
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

static Stat st_remquo = { "remquo", 0, 0, 0 };
static Stat st_log2 = { "log2", 0, 0, 0 };
static Stat st_atan2 = { "atan2", 0, 0, 0 };
static Stat st_fmodl = { "fmodl", 0, 0, 0 };

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

static std::uint64_t rng_state = 0xb0287a5a5a5a5a5aull;

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

static void check_remquo(double x, double y, const char *tag)
{
	unsigned char buf_p[GUARD_BUF], buf_o[GUARD_BUF];
	int *pq, *oq;
	double rp, ro;
	std::uint64_t pb, ob;

	std::memset(buf_p, GUARD, sizeof(buf_p));
	std::memset(buf_o, GUARD, sizeof(buf_o));
	pq = reinterpret_cast<int *>(buf_p + GUARD_OFF);
	oq = reinterpret_cast<int *>(buf_o + GUARD_OFF);
	*pq = 0x55555555;
	*oq = 0x55555555;

	st_remquo.cases++;

	rp = port::remquo(x, y, pq);
	ro = ref_remquo(x, y, oq);
	pb = dbits(rp);
	ob = dbits(ro);

	if (pb == ob && *pq == *oq &&
	    std::memcmp(buf_p, buf_o, sizeof(buf_p)) == 0)
		return;

	st_remquo.fails++;
	if (should_print(st_remquo))
		std::printf("remquo FAIL [%s] x=%#018llx y=%#018llx "
		    "port=%#018llx quo=%d ref=%#018llx quo=%d\n",
		    tag, (unsigned long long)dbits(x),
		    (unsigned long long)dbits(y),
		    (unsigned long long)pb, *pq,
		    (unsigned long long)ob, *oq);
}

static void check_log2(double x, const char *tag)
{
	std::uint64_t p, o;

	st_log2.cases++;

	p = dbits(port::log2(x));
	o = dbits(ref_log2(x));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_log2.fails++;
	if (should_print(st_log2))
		std::printf("log2 FAIL [%s] x=%#018llx port=%#018llx ref=%#018llx\n",
		    tag, (unsigned long long)dbits(x),
		    (unsigned long long)p, (unsigned long long)o);
}

static void check_atan2(double y, double x, const char *tag)
{
	std::uint64_t p, o;

	st_atan2.cases++;

	p = dbits(port::atan2(y, x));
	o = dbits(ref_atan2(y, x));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_atan2.fails++;
	if (should_print(st_atan2))
		std::printf("atan2 FAIL [%s] y=%#018llx x=%#018llx "
		    "port=%#018llx ref=%#018llx\n",
		    tag, (unsigned long long)dbits(y),
		    (unsigned long long)dbits(x),
		    (unsigned long long)p, (unsigned long long)o);
}

static void check_fmodl(long double x, long double y, const char *tag)
{
	ldrep p, o;

	st_fmodl.cases++;

	p = ldbits(port::fmodl(x, y));
	o = ldbits(ref_fmodl(x, y));

	if (guarded_equal(p.b, o.b, sizeof(p.b)))
		return;

	st_fmodl.fails++;
	if (should_print(st_fmodl)) {
		std::size_t i;
		std::printf("fmodl FAIL [%s] port=", tag);
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", p.b[i]);
		std::printf(" ref=");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", o.b[i]);
		std::printf("\n");
	}
}

static const std::uint64_t kEdgeD[] = {
	0x0000000000000000ULL, 0x8000000000000000ULL,
	0x0000000000000001ULL, 0x8000000000000001ULL,
	0x000fffffffffffffULL, 0x800fffffffffffffULL,
	0x0010000000000000ULL, 0x8010000000000000ULL,
	0x3fe0000000000000ULL, 0xbfe0000000000000ULL,
	0x3ff0000000000000ULL, 0xbff0000000000000ULL,
	0x3ff0000000000001ULL, 0xbff0000000000001ULL,
	0x3fe921fb54442d18ULL, 0xbfe921fb54442d18ULL,
	0x3fe921fb00000000ULL, 0xbfe921fb00000000ULL,
	0x3fe921fa80000000ULL, 0xbfe921fa80000000ULL,
	0x3ff71547652b82feULL, 0xbff71547652b82feULL,
	0x3e4fffff00000000ULL, 0xbe4fffff00000000ULL,
	0x3e50000000000000ULL, 0xbe50000000000000ULL,
	0x3e50000100000000ULL, 0xbe50000100000000ULL,
	0x3c90000000000000ULL, 0xbc90000000000000ULL,
	0x3cafffffffffffffULL, 0xbcafffffffffffffULL,
	0x3cb0000000000000ULL, 0xbcb0000000000000ULL,
	0x7ff0000000000000ULL, 0xfff0000000000000ULL,
	0x7ff8000000000000ULL, 0xfff8000000000000ULL,
	0x7ff0000000000001ULL, 0xfff0000000000001ULL,
	0x7fffffffffffffffULL, 0xffffffffffffffffULL,
};

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
};

static void edge_cases(void)
{
	std::size_t i, j;
	const std::size_t nd = sizeof(kEdgeD) / sizeof(kEdgeD[0]);
	const std::size_t nld = sizeof(ldvec) / sizeof(ldvec[0]);

	for (i = 0; i < nd; i++) {
		for (j = 0; j < nd; j++)
			check_remquo(fromdbits(kEdgeD[i]), fromdbits(kEdgeD[j]),
			    "edge-pair");
		check_remquo(fromdbits(kEdgeD[i]), fromdbits(kEdgeD[i]),
		    "edge-self");
	}

	/* remquo: tiny y branch (y < 0x1p-1021) vs half-y branch */
	check_remquo(3.0, 0x1p-1022, "tiny-y");
	check_remquo(-3.0, 0x1p-1022, "tiny-y-neg");
	check_remquo(0x1p-1022, 0x1p-1021, "tiny-y-boundary");
	check_remquo(1.5, 2.0, "half-y");
	check_remquo(1.0, 2.0, "exact-half-y");
	check_remquo(-1.5, 2.0, "half-y-neg");
	check_remquo(5.0, 3.0, "general");
	check_remquo(-5.0, 3.0, "general-neg");
	check_remquo(5.0, -3.0, "y-neg");
	check_remquo(0.0, 1.0, "zero-x");
	check_remquo(-0.0, 1.0, "negzero-x");

	for (i = 0; i < nd; i++)
		check_log2(fromdbits(kEdgeD[i]), "edge");

	check_log2(1.0, "one");
	check_log2(-1.0, "neg-one");
	check_log2(0.0, "zero");
	check_log2(-0.0, "neg-zero");
	check_log2(2.0, "two");
	check_log2(0.5, "half");
	check_log2(1.4142135623730951, "sqrt2");
	check_log2(0.7071067811865476, "inv-sqrt2");
	check_log2(1.414213562373095, "sqrt2-lo");
	check_log2(0.7071067811865475, "inv-sqrt2-lo");
	check_log2(0x1p-1074, "min-subnormal");
	check_log2(0x1p-1022, "min-normal");
	check_log2(0x1.fffffffffffffp1023, "max-finite");

	for (i = 0; i < nd; i++) {
		for (j = 0; j < nd; j++)
			check_atan2(fromdbits(kEdgeD[i]), fromdbits(kEdgeD[j]),
			    "edge-pair");
	}

	/* atan2 special branches */
	check_atan2(0.0, 1.0, "y0-posx");
	check_atan2(-0.0, 1.0, "ny0-posx");
	check_atan2(0.0, -1.0, "y0-negx");
	check_atan2(-0.0, -1.0, "ny0-negx");
	check_atan2(1.0, 0.0, "posy0");
	check_atan2(-1.0, 0.0, "negy0");
	check_atan2(1.0, 1.0, "x1");
	check_atan2(2.0, 1.0, "x1-general");
	check_atan2(__builtin_inf(), __builtin_inf(), "inf-inf");
	check_atan2(-__builtin_inf(), __builtin_inf(), "ninf-inf");
	check_atan2(__builtin_inf(), -__builtin_inf(), "inf-ninf");
	check_atan2(-__builtin_inf(), -__builtin_inf(), "ninf-ninf");
	check_atan2(1.0, __builtin_inf(), "finite-posinf");
	check_atan2(-1.0, __builtin_inf(), "nfinite-posinf");
	check_atan2(1.0, -__builtin_inf(), "finite-neginf");
	check_atan2(-1.0, -__builtin_inf(), "nfinite-neginf");
	check_atan2(__builtin_inf(), 1.0, "inf-finite");
	check_atan2(-__builtin_inf(), 1.0, "ninf-finite");
	check_atan2(1.0, -1.0, "q2");
	check_atan2(-1.0, -1.0, "q3");
	check_atan2(0x1p61, 1.0, "k-gt-60");
	check_atan2(1.0, -0x1p61, "k-lt-minus60");
	check_atan2(1.0, -1.0, "neg-x-normal");

	for (i = 0; i < nld; i++) {
		for (j = 0; j < nld; j++)
			check_fmodl(mkld(ldvec[i].se, ldvec[i].m),
			    mkld(ldvec[j].se, ldvec[j].m), "edge-pair");
	}

	check_fmodl(5.0L, 3.0L, "general");
	check_fmodl(-5.0L, 3.0L, "neg-x");
	check_fmodl(5.0L, -3.0L, "neg-y");
	check_fmodl(1.0L, 1.0L, "equal");
	check_fmodl(0.5L, 1.0L, "lt-y");
	check_fmodl(0x1p-5000L, 1.0L, "subnormal-x");
	check_fmodl(1.0L, 0x1p-5000L, "subnormal-y");
	check_fmodl(0x1p-5000L, 0x1p-4999L, "subnormal-both");
	check_fmodl(3.0L, 0.0L, "y-zero");
	check_fmodl(__builtin_infl(), 1.0L, "x-inf");
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
		    ((0x3ff0000000000000ULL + d) & 0x7fffffffffffffffULL);
	} else if (kind < 35) {
		std::uint64_t d = next_mod64(11u) - 5u;
		u = (u & 0x8000000000000000ULL) |
		    ((0x3e50000000000000ULL + d) & 0x7fffffffffffffffULL);
	} else if (kind < 45) {
		std::uint64_t d = next_mod64(11u) - 5u;
		u = (u & 0x8000000000000000ULL) |
		    ((0x3fe921fb00000000ULL + d) & 0x7fffffffffffffffULL);
	} else if (kind < 55) {
		std::uint64_t d = next_mod64(11u) - 5u;
		u = (u & 0x8000000000000000ULL) |
		    ((0x3cb0000000000000ULL + d) & 0x7fffffffffffffffULL);
	}

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

static void random_sweep(void)
{
	unsigned long long i;

	rng_state = 0xd1ce4e5b91234567ull;
	for (i = 0; i < ITERS; i++) {
		std::uint64_t ux = rng_d();
		std::uint64_t uy = rng_d();

		if ((i & 15) == 0)
			ux = 0x0000000000000000ULL;
		if ((i & 15) == 1)
			uy = 0x0000000000000000ULL;
		if ((i & 15) == 2)
			ux = 0x7ff0000000000000ULL;
		if ((i & 15) == 3)
			uy = 0x7ff8000000000000ULL;
		if ((i & 15) == 4)
			ux = 0x3ff0000000000000ULL;
		if ((i & 15) == 5)
			uy = 0x3ff0000000000001ULL;
		if ((i & 15) == 6)
			ux = 0x3cb0000000000000ULL;
		if ((i & 15) == 7)
			uy = 0x3cafffffffffffffULL;

		check_remquo(fromdbits(ux), fromdbits(uy), "random");
	}

	rng_state = 0xdecafbadc0ffee01ull;
	for (i = 0; i < ITERS; i++) {
		std::uint64_t ux = rng_d();

		if ((i & 15) == 0)
			ux = 0x0000000000000000ULL;
		if ((i & 15) == 1)
			ux = 0x8000000000000000ULL;
		if ((i & 15) == 2)
			ux = 0x3ff0000000000000ULL;
		if ((i & 15) == 3)
			ux = 0xbff0000000000000ULL;
		if ((i & 15) == 4)
			ux = 0x7ff0000000000000ULL;
		if ((i & 15) == 5)
			ux = 0x7ff8000000000000ULL;
		if ((i & 15) == 6)
			ux = 0x3fe921fb54442d18ULL;
		if ((i & 15) == 7)
			ux = 0x3e50000000000000ULL;

		check_log2(fromdbits(ux), "random");
	}

	rng_state = 0x243f6a8885a308d3ull;
	for (i = 0; i < ITERS; i++) {
		std::uint64_t uy = rng_d();
		std::uint64_t ux = rng_d();

		if ((i & 15) == 0)
			uy = 0x0000000000000000ULL;
		if ((i & 15) == 1)
			ux = 0x0000000000000000ULL;
		if ((i & 15) == 2)
			uy = 0x7ff0000000000000ULL;
		if ((i & 15) == 3)
			ux = 0xfff0000000000000ULL;
		if ((i & 15) == 4)
			ux = 0x3ff0000000000000ULL;
		if ((i & 15) == 5)
			uy = 0x8000000000000000ULL;
		if ((i & 15) == 6)
			ux = 0x8000000000000000ULL;
		if ((i & 15) == 7)
			uy = 0x7ff0000000000000ULL;

		check_atan2(fromdbits(uy), fromdbits(ux), "random");
	}

	rng_state = 0x3141592653589793ull;
	for (i = 0; i < ITERS; i++) {
		long double x = rng_ld();
		long double y = rng_ld();

		if ((i & 31) == 0)
			x = 0.0L;
		if ((i & 31) == 1)
			y = 0.0L;
		if ((i & 31) == 2)
			x = 1.0L;
		if ((i & 31) == 3)
			y = 1.0L;
		if ((i & 31) == 4)
			x = -1.0L;
		if ((i & 31) == 5)
			y = -1.0L;
		if ((i & 31) == 6)
			x = __builtin_nanl("");
		if ((i & 31) == 7)
			y = __builtin_infl();
		if ((i & 31) == 8)
			x = 0x1p-5000L;
		if ((i & 31) == 9)
			y = 0x1p-5000L;

		check_fmodl(x, y, "random");
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

	std::printf("PBSD batch b0287 differential test\n");
	std::printf("  function              cases     fails   status\n");

	edge_cases();
	random_sweep();

	row(st_remquo);
	row(st_log2);
	row(st_atan2);
	row(st_fmodl);

	fails = st_remquo.fails + st_log2.fails + st_atan2.fails + st_fmodl.fails;
	return fails == 0 ? 0 : 1;
}
