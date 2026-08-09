// Differential test for PBSD batch b0305 (lrint, fmax, fmin, fminimum_magl).

#include <cfenv>
#include <cfloat>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

import pbsd.lib.msun.src.b0305;

namespace port = pbsd::lib_msun_src::b0305;

extern "C" {
long ref_lrint(double x);
double ref_fmax(double x, double y);
double ref_fmin(double x, double y);
long double ref_fminimum_magl(long double x, long double y);
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

static Stat st_lrint = { "lrint", 0, 0, 0 };
static Stat st_fmax = { "fmax", 0, 0, 0 };
static Stat st_fmin = { "fmin", 0, 0, 0 };
static Stat st_fminimum_magl = { "fminimum_magl", 0, 0, 0 };

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

static const int kRoundModes[4] = {
	FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO
};

static const int kPresets[5] = {
	0,
	FE_INEXACT,
	FE_INVALID,
	FE_INEXACT | FE_INVALID,
	FE_ALL_EXCEPT
};

struct Obs {
	long long ret;
	int raised;
	int round_after;
};

static bool operator!=(const Obs &a, const Obs &b)
{
	return a.ret != b.ret || a.raised != b.raised ||
	    a.round_after != b.round_after;
}

static void prepare(int mode, int preset)
{
	std::fesetround(mode);
	std::feclearexcept(FE_ALL_EXCEPT);
	if (preset != 0)
		std::feraiseexcept(preset);
}

template <typename T, typename R>
static Obs run(R (*fp)(T), T x, int mode, int preset)
{
	volatile T arg = x;
	Obs o;

	prepare(mode, preset);
	volatile R r = fp(arg);
	o.raised = std::fetestexcept(FE_ALL_EXCEPT);
	o.round_after = std::fegetround();
	o.ret = (long long)r;
	return o;
}

static void check_lrint(std::uint64_t xbits, int mode, int preset,
    const char *tag)
{
	double x = fromdbits(xbits);
	Obs p, r;

	st_lrint.cases++;

	p = run<double, long>(port::lrint, x, mode, preset);
	r = run<double, long>(ref_lrint, x, mode, preset);
	if (p != r) {

	st_lrint.fails++;
	if (should_print(st_lrint)) {
		std::printf("lrint FAIL [%s] x=%#018llx round=%d preset=0x%x\n",
		    tag, (unsigned long long)xbits, mode, (unsigned)preset);
		std::printf("  port: ret=%lld raised=0x%x round_after=%d\n",
		    p.ret, (unsigned)p.raised, p.round_after);
		std::printf("  ref : ret=%lld raised=0x%x round_after=%d\n",
		    r.ret, (unsigned)r.raised, r.round_after);
	}
}

static void check_fmax(double x, double y, const char *tag)
{
	std::uint64_t p, o;

	st_fmax.cases++;

	p = dbits(port::fmax(x, y));
	o = dbits(ref_fmax(x, y));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_fmax.fails++;
	if (should_print(st_fmax))
		std::printf("fmax FAIL [%s] x=%#018llx y=%#018llx port=%#018llx ref=%#018llx\n",
		    tag, (unsigned long long)dbits(x),
		    (unsigned long long)dbits(y), (unsigned long long)p,
		    (unsigned long long)o);
}

static void check_fmin(double x, double y, const char *tag)
{
	std::uint64_t p, o;

	st_fmin.cases++;

	p = dbits(port::fmin(x, y));
	o = dbits(ref_fmin(x, y));

	if (guarded_equal(&p, &o, sizeof(p)))
		return;

	st_fmin.fails++;
	if (should_print(st_fmin))
		std::printf("fmin FAIL [%s] x=%#018llx y=%#018llx port=%#018llx ref=%#018llx\n",
		    tag, (unsigned long long)dbits(x),
		    (unsigned long long)dbits(y), (unsigned long long)p,
		    (unsigned long long)o);
}

static void check_fminimum_magl(long double x, long double y, const char *tag)
{
	ldrep p, o;

	st_fminimum_magl.cases++;

	p = ldbits(port::fminimum_magl(x, y));
	o = ldbits(ref_fminimum_magl(x, y));

	if (guarded_equal(p.b, o.b, sizeof(p.b)))
		return;

	st_fminimum_magl.fails++;
	if (should_print(st_fminimum_magl)) {
		std::size_t i;

		std::printf("fminimum_magl FAIL [%s] port=", tag);
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", p.b[i]);
		std::printf(" ref=");
		for (i = LD_SIG; i-- > 0;)
			std::printf("%02x", o.b[i]);
		std::printf("\n");
	}
}

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
	0x000fffffffffffffull, 0x800fffffffffffffull,
	0x43e0000000000000ull, 0xc3e0000000000000ull,
	0x43efffffffffffffull, 0xc3efffffffffffffull,
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

static std::uint64_t rng_state = 0xb0305a5a5a5a5a5aull;

static std::uint64_t next_u64(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ull);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
	return z ^ (z >> 31);
}

static std::uint32_t next_mod(std::uint32_t m)
{
	return (std::uint32_t)(next_u64() % m);
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

	for (int mi = 0; mi < 4; mi++) {
		int mode = kRoundModes[mi];
		for (int pi = 0; pi < 5; pi++) {
			int preset = kPresets[pi];

			for (i = 0; i < NDVEC; i++)
				check_lrint(dvec[i], mode, preset, "edge");

			static const double lvals[] = {
				0.0, -0.0, 0.5, -0.5, 1.0, -1.0, 1.5, -1.5,
				2147483647.0, -2147483647.0,
				2147483648.0, -2147483648.0,
				9223372036854775808.0, -9223372036854775808.0,
				DBL_MAX, -DBL_MAX, DBL_MIN, -DBL_MIN,
				__builtin_inf(), -__builtin_inf(),
				__builtin_nan(""), -__builtin_nan("p"),
			};
			const unsigned n = (unsigned)(sizeof lvals / sizeof lvals[0]);

			for (i = 0; i < n; i++)
				check_lrint(dbits(lvals[i]), mode, preset,
				    "scalar");
		}
	}

	for (i = 0; i < NDVEC; i++) {
		for (j = 0; j < NDVEC; j++) {
			double x = fromdbits(dvec[i]);
			double y = fromdbits(dvec[j]);

			check_fmax(x, y, "cross");
			check_fmin(x, y, "cross");
		}
	}

	check_fmax(0.0, -0.0, "signed-zero");
	check_fmax(-0.0, 0.0, "signed-zero");
	check_fmin(0.0, -0.0, "signed-zero");
	check_fmin(-0.0, 0.0, "signed-zero");

	for (i = 0; i < NLDVEC; i++) {
		for (j = 0; j < NLDVEC; j++) {
			check_fminimum_magl(mkld(ldvec[i].se, ldvec[i].m),
			    mkld(ldvec[j].se, ldvec[j].m), "cross");
		}
	}

	check_fminimum_magl(0.0L, -0.0L, "signed-zero");
	check_fminimum_magl(-0.0L, 0.0L, "signed-zero");
	check_fminimum_magl(2.0L, -3.0L, "mag-order");
	check_fminimum_magl(-3.0L, 2.0L, "mag-order");
	check_fminimum_magl(2.0L, -2.0L, "mag-tie");
	check_fminimum_magl(-2.0L, 2.0L, "mag-tie");
	check_fminimum_magl(2.0L, 2.0L, "mag-equal");
	check_fminimum_magl(-2.0L, -2.0L, "mag-equal");

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
			{ 0x0000000000000000ull, 0x8000000000000000ull },
			{ 0x8000000000000000ull, 0x0000000000000000ull },
		};
		const unsigned n = (unsigned)(sizeof dpairs / sizeof dpairs[0]);

		for (i = 0; i < n; i++) {
			check_fmax(fromdbits(dpairs[i][0]),
			    fromdbits(dpairs[i][1]), "branch");
			check_fmin(fromdbits(dpairs[i][0]),
			    fromdbits(dpairs[i][1]), "branch");
		}
	}

	for (unsigned bit = 0; bit < 64; bit++) {
		std::uint64_t m = 1ull << bit;
		check_fmax(fromdbits(m), fromdbits(~m), "bit");
		check_fmin(fromdbits(m), fromdbits(~m), "bit");
	}

	for (i = 0; i <= 2047u; i++) {
		std::uint64_t infp = ((std::uint64_t)i << 52) | 0x8000000000000000ull;
		std::uint64_t infn = ((std::uint64_t)i << 52);
		std::uint64_t nanp = infn | 1ull;
		std::uint64_t nann = infp | 1ull;
		std::uint64_t normal = 0x3ff0000000000000ull;

		check_fmax(fromdbits(normal), fromdbits(nanp), "nan-boundary");
		check_fmax(fromdbits(nanp), fromdbits(normal), "nan-boundary");
		check_fmin(fromdbits(normal), fromdbits(nann), "nan-boundary");
		check_fmin(fromdbits(nann), fromdbits(normal), "nan-boundary");
	}

	for (i = 0; i <= 32767u; i++) {
		std::uint16_t infse = (std::uint16_t)i;
		std::uint16_t nanse = (std::uint16_t)(i | 0x8000u);
		std::uint64_t infm = 0x8000000000000000ull;
		std::uint64_t nanm = 0x8000000000000001ull;
		long double normal = 1.0L;

		check_fminimum_magl(normal, mkld(infse, infm), "ld-nan-boundary");
		check_fminimum_magl(mkld(infse, nanm), normal, "ld-nan-boundary");
		check_fminimum_magl(normal, mkld(nanse, nanm), "ld-nan-boundary");
		check_fminimum_magl(mkld(nanse, nanm), normal, "ld-nan-boundary");
	}

	for (i = 0; i < NLDVEC; i++) {
		long double x = mkld(ldvec[i].se, ldvec[i].m);

		check_fminimum_magl(x, x + 1.0L, "cmp-order");
		check_fminimum_magl(x, x - 1.0L, "cmp-order");
		check_fminimum_magl(x, -x, "cmp-neg");
		check_fminimum_magl(x, x, "cmp-tie");
	}

	for (long long n = 0; n < RANDOM_ITERS; n++) {
		int mode = kRoundModes[(unsigned)(n % 4)];
		int preset = kPresets[(unsigned)(n % 5)];
		std::uint64_t db = rand_dbits();

		check_lrint(db, mode, preset, "rand");
		check_fmax(fromdbits(rand_dbits()), fromdbits(rand_dbits()),
		    "rand");
		check_fmin(fromdbits(rand_dbits()), fromdbits(rand_dbits()),
		    "rand");
		check_fminimum_magl(rand_ld(), rand_ld(), "rand");
	}

	Stat *all[] = { &st_lrint, &st_fmax, &st_fmin, &st_fminimum_magl };
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

	std::printf("%s\n", bad ? "FAIL" : "PASS");
	return bad ? 1 : 0;
}
