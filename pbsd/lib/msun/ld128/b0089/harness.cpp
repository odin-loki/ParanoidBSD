/*
 * Differential test harness for PBSD batch b0089.
 */

#include <cfloat>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.ld128.b0089;

namespace port = pbsd::lib_msun_ld128::b0089;

extern "C" {
long double ref_powl(long double x, long double y);
long double ref_exp2l(long double x);
long double ref_erfl(long double x);
long double ref_erfcl(long double x);
long double ref_logl(long double x);
long double ref_log1pl(long double x);
long double ref_log10l(long double x);
long double ref_log2l(long double x);
}

union IEEEl2bits {
	long double e;
	struct {
		unsigned long manl : 64;
		unsigned long manh : 48;
		unsigned int exp : 15;
		unsigned int sign : 1;
	} bits;
	struct {
		unsigned long manl : 64;
		unsigned long manh : 48;
		unsigned int expsign : 16;
	} xbits;
};

static const std::size_t LD_SIG = sizeof(long double);
static const unsigned char GUARD = 0x7f;
static const std::size_t GUARD_BUF = 64;
static const std::size_t GUARD_OFF = 16;

struct ldrep {
	unsigned char b[sizeof(long double)];
};

static bool
guarded_equal(const void *pa, const void *pb, std::size_t n)
{
	unsigned char a[GUARD_BUF], b[GUARD_BUF];
	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	std::memcpy(a + GUARD_OFF, pa, n);
	std::memcpy(b + GUARD_OFF, pb, n);
	return std::memcmp(a, b, sizeof(a)) == 0;
}

static ldrep
ldbits(long double x)
{
	ldrep r;
	std::memset(r.b, 0, sizeof(r.b));
	std::memcpy(r.b, &x, LD_SIG);
	return r;
}

static long double
mkld(std::uint16_t expsign, std::uint64_t manh, std::uint64_t manl)
{
	IEEEl2bits u;
	u.xbits.expsign = expsign;
	u.xbits.manh = manh;
	u.xbits.manl = manl;
	return u.e;
}

static void
ldhex(const ldrep &r)
{
	for (std::size_t i = LD_SIG; i-- > 0;)
		std::printf("%02x", r.b[i]);
}

struct stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	unsigned reported;
};

static stat st_powl = { "powl", 0, 0, 0 };
static stat st_exp2l = { "exp2l", 0, 0, 0 };
static stat st_erfl = { "erfl", 0, 0, 0 };
static stat st_erfcl = { "erfcl", 0, 0, 0 };
static stat st_logl = { "logl", 0, 0, 0 };
static stat st_log1pl = { "log1pl", 0, 0, 0 };
static stat st_log10l = { "log10l", 0, 0, 0 };
static stat st_log2l = { "log2l", 0, 0, 0 };
static const unsigned MAX_REPORT = 8;

template <typename Fn>
static void
check_unary(stat &st, Fn port_fn, long double (*ref_fn)(long double),
    long double x, const char *tag)
{
	ldrep p = ldbits(port_fn(x));
	ldrep o = ldbits(ref_fn(x));
	st.cases++;
	if (guarded_equal(p.b, o.b, sizeof(p.b)))
		return;
	st.fails++;
	if (st.reported < MAX_REPORT) {
		st.reported++;
		std::printf("  %s FAIL [%s] x=", st.name, tag);
		ldhex(ldbits(x));
		std::printf(" port="); ldhex(p);
		std::printf(" ref="); ldhex(o);
		std::printf("\n");
	}
}

static void
check_powl(long double x, long double y, const char *tag)
{
	ldrep p = ldbits(port::powl(x, y));
	ldrep o = ldbits(ref_powl(x, y));
	st_powl.cases++;
	if (guarded_equal(p.b, o.b, sizeof(p.b)))
		return;
	st_powl.fails++;
	if (st_powl.reported < MAX_REPORT) {
		st_powl.reported++;
		std::printf("  powl FAIL [%s] x=", tag); ldhex(ldbits(x));
		std::printf(" y="); ldhex(ldbits(y));
		std::printf(" port="); ldhex(p);
		std::printf(" ref="); ldhex(o);
		std::printf("\n");
	}
}

struct ldcase {
	std::uint16_t se;
	std::uint64_t mh;
	std::uint64_t ml;
};

static const ldcase ldvec[] = {
	{ 0x0000, 0x0000000000000000ull, 0x0000000000000000ull },
	{ 0x8000, 0x0000000000000000ull, 0x0000000000000000ull },
	{ 0x0000, 0x0000000000000001ull, 0x0000000000000000ull },
	{ 0x8000, 0x0000000000000001ull, 0x0000000000000000ull },
	{ 0x0000, 0x0000000000000000ull, 0x0000000000000001ull },
	{ 0x0000, 0x0000800000000000ull, 0x0000000000000000ull },
	{ 0x0001, 0x0000000000000000ull, 0x0000000000000000ull },
	{ 0x8001, 0x0000000000000000ull, 0x0000000000000000ull },
	{ 0x3ffe, 0x8000000000000000ull, 0x0000000000000000ull },
	{ 0xbffe, 0x8000000000000000ull, 0x0000000000000000ull },
	{ 0x3fff, 0x8000000000000000ull, 0x0000000000000000ull },
	{ 0xbfff, 0x8000000000000000ull, 0x0000000000000000ull },
	{ 0x3fff, 0x8000000000000001ull, 0x0000000000000000ull },
	{ 0x4000, 0x8000000000000000ull, 0x0000000000000000ull },
	{ 0xc000, 0x8000000000000000ull, 0x0000000000000000ull },
	{ 0x4001, 0xc000000000000000ull, 0x0000000000000000ull },
	{ 0x4000, 0xc90fdaa22168c235ull, 0x0000000000000000ull },
	{ 0x7ffe, 0xffffffffffffffffull, 0xffffffffffffffffull },
	{ 0xfffe, 0xffffffffffffffffull, 0xffffffffffffffffull },
	{ 0x7fff, 0x8000000000000000ull, 0x0000000000000000ull },
	{ 0xffff, 0x8000000000000000ull, 0x0000000000000000ull },
	{ 0x7fff, 0xc000000000000000ull, 0x0000000000000000ull },
	{ 0xffff, 0xc000000000000000ull, 0x0000000000000000ull },
	{ 0x7fff, 0x8000000000000001ull, 0x0000000000000000ull },
	{ 0x7fff, 0xffffffffffffffffull, 0xffffffffffffffffull },
	{ 0x3fff, 0x8080808080808080ull, 0xfefefefefefefefeull },
	{ 0xbfff, 0xff00ff00ff00ff00ull, 0x0101010101010101ull },
};
static const std::size_t NLDVEC = sizeof(ldvec) / sizeof(ldvec[0]);

static void edge_cases(void)
{
	for (std::size_t i = 0; i < NLDVEC; i++) {
		long double x = mkld(ldvec[i].se, ldvec[i].mh, ldvec[i].ml);
		check_unary(st_exp2l, port::exp2l, ref_exp2l, x, "vec");
		check_unary(st_erfl, port::erfl, ref_erfl, x, "vec");
		check_unary(st_erfcl, port::erfcl, ref_erfcl, x, "vec");
		check_unary(st_logl, port::logl, ref_logl, x, "vec");
		check_unary(st_log1pl, port::log1pl, ref_log1pl, x, "vec");
		check_unary(st_log10l, port::log10l, ref_log10l, x, "vec");
		check_unary(st_log2l, port::log2l, ref_log2l, x, "vec");
	}
	for (std::size_t i = 0; i < NLDVEC; i++)
		for (std::size_t j = 0; j < NLDVEC; j++)
			check_powl(mkld(ldvec[i].se, ldvec[i].mh, ldvec[i].ml),
			    mkld(ldvec[j].se, ldvec[j].mh, ldvec[j].ml), "cross");

	static const long double xs[] = {
		0.0L, -0.0L, 1.0L, -1.0L, 0.5L, -0.5L, 2.0L, -2.0L,
		1.5L, -1.5L, 3.0L, -3.0L, 10.0L, -10.0L,
		1e-4000L, -1e-4000L, 1e4000L,
		__builtin_nanl(""), -__builtin_nanl(""),
		__builtin_infl(), -__builtin_infl(),
		LDBL_MIN, -LDBL_MIN, LDBL_MAX, -LDBL_MAX,
		0x1p-113L, -0x1p-113L, 0x1p-40L, -0x1p-40L,
		0x1p-16373L, 0x1p40L, 0x1p113L,
		0.84375L, 0.84374L, 1.25L, 1.249L,
		2.85715L, 2.85714L, 9.0L, 8.999L, 108.0L, 107.0L,
		16384.0L, -16495.0L, 16383.0L, -16494.0L,
		0x1.8p112L, -0x1.8p112L,
		1.0L + 0x1p-100L, 1.0L - 0x1p-100L,
		2.0L - 0x1p-120L, 0x1.000001p0L,
	};
	static const long double ys[] = {
		0.0L, 1.0L, -1.0L, 2.0L, 0.5L, -0.5L, 3.0L, -3.0L,
		1e9L, -1e9L, 1e-9L, -1e-9L,
		__builtin_nanl(""), __builtin_infl(), -__builtin_infl(),
	};
	for (std::size_t a = 0; a < sizeof(xs)/sizeof(xs[0]); a++) {
		check_unary(st_exp2l, port::exp2l, ref_exp2l, xs[a], "special");
		check_unary(st_erfl, port::erfl, ref_erfl, xs[a], "special");
		check_unary(st_erfcl, port::erfcl, ref_erfcl, xs[a], "special");
		check_unary(st_logl, port::logl, ref_logl, xs[a], "special");
		check_unary(st_log1pl, port::log1pl, ref_log1pl, xs[a], "special");
		check_unary(st_log10l, port::log10l, ref_log10l, xs[a], "special");
		check_unary(st_log2l, port::log2l, ref_log2l, xs[a], "special");
		for (std::size_t b = 0; b < sizeof(ys)/sizeof(ys[0]); b++)
			check_powl(xs[a], ys[b], "special");
	}
	static const long double log1p_xs[] = {
		-0.999999999999999999L, -1.0L, -1.1L,
		-0.5L, -0.25L, -0x1p-113L, -0x1p-200L,
		0.0L, -0.0L, 0x1p-113L, 0x1p200L, 0x1p226L,
		__builtin_infl(), __builtin_nanl(""),
	};
	for (std::size_t a = 0; a < sizeof(log1p_xs)/sizeof(log1p_xs[0]); a++)
		check_unary(st_log1pl, port::log1pl, ref_log1pl, log1p_xs[a], "log1p");
	static const long double pow_pairs[][2] = {
		{ 0.0L, 1.0L }, { -0.0L, 2.0L }, { -0.0L, 3.0L },
		{ -1.0L, 2.0L }, { -1.0L, 3.0L }, { -1.0L, 0.5L },
		{ -2.0L, 0.5L }, { 0.0L, -1.0L }, { -0.0L, -2.0L },
		{ 2.0L, __builtin_infl() }, { 0.5L, -__builtin_infl() },
		{ 1.5L, 1e10L }, { 0.999L, 1e10L }, { 1.001L, -1e10L },
		{ __builtin_infl(), 2.0L }, { -__builtin_infl(), 2.0L },
	};
	for (std::size_t a = 0; a < sizeof(pow_pairs)/sizeof(pow_pairs[0]); a++)
		check_powl(pow_pairs[a][0], pow_pairs[a][1], "pair");
}

struct rng64 {
	std::uint64_t s;
	explicit rng64(std::uint64_t seed) : s(seed) {}
	std::uint64_t next() {
		s ^= s >> 12; s ^= s << 25; s ^= s >> 27;
		return s * 0x2545F4914F6CDD1Dull;
	}
};

static long double rand_ld(rng64 &r, int cls)
{
	std::uint64_t w = r.next();
	std::uint16_t se;
	std::uint64_t mh, ml;
	switch (cls % 8) {
	case 0: se = (std::uint16_t)(w & 0xffff); mh = r.next(); ml = r.next(); break;
	case 1: se = (std::uint16_t)((w & 0x7fff) + 0x3c00); mh = r.next() & 0x0000ffffffffffffull; ml = r.next(); break;
	case 2: se = (std::uint16_t)(w & 0x7fff); mh = r.next() & 0x0000ffffffffffffull; ml = r.next(); break;
	case 3: se = 0x7fff; mh = 0x8000000000000000ull | (r.next() & 0x7fffffffffffffffull); ml = r.next(); break;
	case 4: se = 0x7fff; mh = 0xc000000000000000ull | (r.next() & 0x3fffffffffffffffull); ml = r.next(); break;
	case 5: se = 0x7fff; mh = 0x8000000000000000ull; ml = 0; break;
	case 6: se = 0xffff; mh = 0x8000000000000000ull; ml = 0; break;
	default: se = (std::uint16_t)(0x3fff + (int)(w % 200) - 100); mh = 0x8000000000000000ull | (r.next() >> 1); ml = r.next(); break;
	}
	return mkld(se, mh, ml);
}

static void random_sweep(void)
{
	rng64 r(0xB0089ull);
	for (unsigned i = 0; i < 250000; i++) {
		long double x = rand_ld(r, (int)r.next());
		long double y = rand_ld(r, (int)(r.next() >> 8));
		check_unary(st_exp2l, port::exp2l, ref_exp2l, x, "rand");
		check_unary(st_erfl, port::erfl, ref_erfl, x, "rand");
		check_unary(st_erfcl, port::erfcl, ref_erfcl, x, "rand");
		check_unary(st_logl, port::logl, ref_logl, x, "rand");
		check_unary(st_log1pl, port::log1pl, ref_log1pl, x, "rand");
		check_unary(st_log10l, port::log10l, ref_log10l, x, "rand");
		check_unary(st_log2l, port::log2l, ref_log2l, x, "rand");
		check_powl(x, y, "rand");
	}
}

static void print_row(const stat &st)
{
	std::printf("%-8s %12llu %12llu\n", st.name, st.cases, st.fails);
}

int main()
{
	std::printf("PBSD batch b0089 differential harness\n");
	std::printf("LDBL_MANT_DIG=%d, comparing %zu bytes of long double\n\n",
	    (int)LDBL_MANT_DIG, LD_SIG);
	edge_cases();
	random_sweep();
	std::printf("%-8s %12s %12s\n", "function", "cases", "failures");
	print_row(st_powl); print_row(st_exp2l); print_row(st_erfl); print_row(st_erfcl);
	print_row(st_logl); print_row(st_log1pl); print_row(st_log10l); print_row(st_log2l);
	unsigned long long total = st_powl.fails + st_exp2l.fails + st_erfl.fails +
	    st_erfcl.fails + st_logl.fails + st_log1pl.fails + st_log10l.fails + st_log2l.fails;
	return total == 0 ? 0 : 1;
}

