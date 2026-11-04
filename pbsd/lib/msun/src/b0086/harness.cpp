/*
 * Differential test for PBSD batch b0086.
 *
 * Every case is fed to both the C++23 port and the ref_ oracle built from the
 * untouched HardenedBSD C, and the results are compared bit for bit.
 */

import pbsd.lib.msun.src.b0086;

#include <bit>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>

extern "C" {
double ref_fabs(double);
int ref_finite(double);
long double ref_lgammal(long double);
extern int signgam;
}

namespace P = pbsd::lib_msun_src::b0086;

#if (defined(__i386__) || defined(__x86_64__)) && LDBL_MANT_DIG == 64
#define PBSD_LD80 1
#endif

static const int GUARD_BYTE = 0x7f;
static const int SIGNGAM_GUARD = 0x7f7f7f7f;
static const int MAX_REPORT = 8;

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
	unsigned reported;
};

static Stat st_fabs = { "fabs", 0, 0, 0 };
static Stat st_finite = { "finite", 0, 0, 0 };
static Stat st_lgammal = { "lgammal", 0, 0, 0 };

static unsigned long long cov_finite_one, cov_finite_zero;
static unsigned long long cov_finite_sub, cov_finite_maxfinite, cov_finite_inf;
static unsigned long long cov_fabs_neg, cov_fabs_pos, cov_fabs_lowbits;
static unsigned long long cov_lg_written, cov_lg_neg_sign, cov_lg_pos_sign;

static std::uint64_t rng_state;

static std::uint64_t
rnd64(void)
{
	std::uint64_t z = (rng_state += 0x9E3779B97F4A7C15ULL);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
	return z ^ (z >> 31);
}

static double
mkdouble(std::uint32_t hi, std::uint32_t lo)
{
	return std::bit_cast<double>(((std::uint64_t)hi << 32) | lo);
}

static std::uint64_t
dbits(double d)
{
	return std::bit_cast<std::uint64_t>(d);
}

static bool
ld_same(long double a, long double b)
{
	unsigned char ba[sizeof(long double)], bb[sizeof(long double)];

	std::memset(ba, 0, sizeof(ba));
	std::memset(bb, 0, sizeof(bb));
	std::memcpy(ba, &a, sizeof(a));
	std::memcpy(bb, &b, sizeof(b));
#if LDBL_MANT_DIG == 64
	return std::memcmp(ba, bb, 10) == 0;
#else
	return std::memcmp(ba, bb, sizeof(long double)) == 0;
#endif
}

static void
print_ld(const long double *v)
{
	unsigned char b[sizeof(long double)];
	std::size_t i, n;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, v, sizeof(*v));
#if LDBL_MANT_DIG == 64
	n = 10;
#else
	n = sizeof(long double);
#endif
	for (i = n; i-- > 0;)
		std::printf("%02x", b[i]);
}

/*
 * Both scalar entry points take their argument by value, but a stray write
 * through a miscomputed pointer would still be observable, so each call is
 * made from a guard-filled scratch array whose untouched cells are verified
 * afterwards.
 */
union GuardSlot {
	double d;
	unsigned char b[sizeof(double)];
};

static GuardSlot port_pad[8];
static GuardSlot ref_pad[8];
static const int PAD_SLOT = 3;

static void
pad_fill(GuardSlot *pad, double x)
{
	int i;

	for (i = 0; i < 8; i++)
		std::memset(pad[i].b, GUARD_BYTE, sizeof(pad[i].b));
	pad[PAD_SLOT].d = x;
}

static bool
pad_intact(const GuardSlot *pad, double x)
{
	unsigned char guard[sizeof(double)];
	int i;

	std::memset(guard, GUARD_BYTE, sizeof(guard));
	for (i = 0; i < 8; i++) {
		if (i == PAD_SLOT) {
			if (dbits(pad[i].d) != dbits(x))
				return false;
			continue;
		}
		if (std::memcmp(pad[i].b, guard, sizeof(guard)) != 0)
			return false;
	}
	return true;
}

static void
check_fabs(double x)
{
	double got, want;
	bool bad;

	st_fabs.cases++;

	pad_fill(port_pad, x);
	pad_fill(ref_pad, x);
	got = P::fabs(port_pad[PAD_SLOT].d);
	want = ref_fabs(ref_pad[PAD_SLOT].d);

	bad = dbits(got) != dbits(want);
	if (!pad_intact(port_pad, x) || !pad_intact(ref_pad, x))
		bad = true;
	if (std::memcmp(port_pad, ref_pad, sizeof(port_pad)) != 0)
		bad = true;

	if (dbits(x) >> 63)
		cov_fabs_neg++;
	else
		cov_fabs_pos++;
	if ((dbits(x) & 0xffffffffULL) != 0)
		cov_fabs_lowbits++;

	if (bad) {
		st_fabs.failures++;
		if (st_fabs.reported++ < MAX_REPORT)
			std::printf("  fabs   FAIL x=%016llx port=%016llx "
			    "ref=%016llx\n",
			    (unsigned long long)dbits(x),
			    (unsigned long long)dbits(got),
			    (unsigned long long)dbits(want));
	}
}

static void
check_finite(double x)
{
	std::uint32_t hi;
	int got, want;
	bool bad;

	st_finite.cases++;

	pad_fill(port_pad, x);
	pad_fill(ref_pad, x);
	got = P::finite(port_pad[PAD_SLOT].d);
	want = ref_finite(ref_pad[PAD_SLOT].d);

	bad = got != want;
	if (!pad_intact(port_pad, x) || !pad_intact(ref_pad, x))
		bad = true;

	hi = (std::uint32_t)(dbits(x) >> 32) & 0x7fffffffu;
	if (want)
		cov_finite_one++;
	else
		cov_finite_zero++;
	if (hi < 0x00100000u)
		cov_finite_sub++;
	if (hi == 0x7fefffffu)
		cov_finite_maxfinite++;
	if (hi == 0x7ff00000u)
		cov_finite_inf++;

	if (bad) {
		st_finite.failures++;
		if (st_finite.reported++ < MAX_REPORT)
			std::printf("  finite FAIL x=%016llx port=%d ref=%d\n",
			    (unsigned long long)dbits(x), got, want);
	}
}

static void
check_double(double x)
{
	check_fabs(x);
	check_finite(x);
}

static void
check_lgammal(long double x)
{
	long double got, want;
	int got_sg, want_sg;
	bool bad;

	st_lgammal.cases++;

	signgam = SIGNGAM_GUARD;
	got = P::lgammal(x);
	got_sg = signgam;

	signgam = SIGNGAM_GUARD;
	want = ref_lgammal(x);
	want_sg = signgam;

	bad = !ld_same(got, want) || got_sg != want_sg;

	if (want_sg != SIGNGAM_GUARD) {
		cov_lg_written++;
		if (want_sg < 0)
			cov_lg_neg_sign++;
		else
			cov_lg_pos_sign++;
	}

	if (bad) {
		st_lgammal.failures++;
		if (st_lgammal.reported++ < MAX_REPORT) {
			std::printf("  lgammal FAIL x=");
			print_ld(&x);
			std::printf(" port=");
			print_ld(&got);
			std::printf("/%d ref=", got_sg);
			print_ld(&want);
			std::printf("/%d\n", want_sg);
		}
	}
}

/* ---------------------------------------------------------------- */
/* hand written edge cases                                          */
/* ---------------------------------------------------------------- */

static const std::uint32_t hi_edges[] = {
	0x00000000u, 0x00000001u, 0x00000002u, 0x0000007fu, 0x00000080u,
	0x000000ffu, 0x00007fffu, 0x00008000u, 0x0007ffffu, 0x00080000u,
	0x000fffffu, 0x00100000u, 0x00100001u, 0x001fffffu, 0x00200000u,
	0x3fe00000u, 0x3ff00000u, 0x3ff00001u, 0x40000000u, 0x40080000u,
	0x7fdfffffu, 0x7fe00000u, 0x7feffffeu, 0x7fefffffu, 0x7ff00000u,
	0x7ff00001u, 0x7ff7ffffu, 0x7ff80000u, 0x7fffffffu,
	0x80000000u, 0x80000001u, 0x800fffffu, 0x80100000u, 0x80100001u,
	0xbfe00000u, 0xbff00000u, 0xc0000000u,
	0xffdfffffu, 0xffe00000u, 0xffefffffu, 0xfff00000u, 0xfff00001u,
	0xfff7ffffu, 0xfff80000u, 0xffffffffu,
};

static const std::uint32_t lo_edges[] = {
	0x00000000u, 0x00000001u, 0x0000007fu, 0x00000080u, 0x000000ffu,
	0x0000ffffu, 0x12345678u, 0x7fffffffu, 0x80000000u, 0xfffffffeu,
	0xffffffffu,
};

static void
run_double_edges(void)
{
	std::size_t i, j;
	std::uint32_t e, m, s;

	for (i = 0; i < sizeof(hi_edges) / sizeof(hi_edges[0]); i++)
		for (j = 0; j < sizeof(lo_edges) / sizeof(lo_edges[0]); j++)
			check_double(mkdouble(hi_edges[i], lo_edges[j]));

	static const std::uint32_t mants[] = { 0x00000u, 0x00001u, 0x80000u,
		0xfffffu };
	static const std::uint32_t los[] = { 0x00000000u, 0xffffffffu };

	for (e = 0; e <= 0x7ffu; e++)
		for (s = 0; s < 2; s++)
			for (i = 0; i < 4; i++)
				for (j = 0; j < 2; j++) {
					m = (s << 31) | (e << 20) | mants[i];
					check_double(mkdouble(m, los[j]));
				}

	check_double(0.0);
	check_double(-0.0);
	check_double(1.0);
	check_double(-1.0);
	check_double(DBL_MIN);
	check_double(-DBL_MIN);
	check_double(DBL_MAX);
	check_double(-DBL_MAX);
	check_double(DBL_TRUE_MIN);
	check_double(-DBL_TRUE_MIN);
	check_double(HUGE_VAL);
	check_double(-HUGE_VAL);
	check_double(std::nan(""));
	check_double(-std::nan(""));
}

static long double
ld_from_bits(std::uint64_t mant, unsigned exp15, unsigned sign)
{
	long double v;
	unsigned char b[sizeof(long double)];
	std::uint16_t se;

	std::memset(b, 0, sizeof(b));
	std::memcpy(b, &mant, sizeof(mant));
	se = (std::uint16_t)(((sign & 1u) << 15) | (exp15 & 0x7fffu));
	std::memcpy(b + 8, &se, sizeof(se));
	std::memcpy(&v, b, sizeof(v));
	return v;
}

static void
run_lgammal_edges(void)
{
	static const long double vals[] = {
		0.0L, -0.0L, 1.0L, -1.0L, 2.0L, 3.0L, -2.0L, -3.0L, -4.0L,
		0.5L, -0.5L, 1.5L, -1.5L, -2.5L, -3.5L, -4.5L, -0.25L,
		0.25L, 1e-30L, -1e-30L, 1e30L, -1e30L, 1e300L, -1e300L,
		1e4000L, -1e4000L, 170.0L, -170.5L, 1023.5L, -1023.5L,
		LDBL_MIN, -LDBL_MIN, LDBL_MAX, -LDBL_MAX, LDBL_TRUE_MIN,
		-LDBL_TRUE_MIN, LDBL_EPSILON, -LDBL_EPSILON,
	};
	std::size_t i;

	for (i = 0; i < sizeof(vals) / sizeof(vals[0]); i++)
		check_lgammal(vals[i]);

	check_lgammal((long double)HUGE_VAL);
	check_lgammal(-(long double)HUGE_VAL);
	check_lgammal(std::nanl(""));
	check_lgammal(-std::nanl(""));

	for (i = 1; i <= 40; i++) {
		check_lgammal((long double)i);
		check_lgammal(-(long double)i);
		check_lgammal(-(long double)i + 0.5L);
		check_lgammal(-(long double)i - 0.5L);
		check_lgammal((long double)i / 7.0L);
		check_lgammal(-(long double)i / 7.0L);
	}

#ifdef PBSD_LD80
	static const unsigned exps[] = { 0u, 1u, 2u, 0x3ffdu, 0x3ffeu,
		0x3fffu, 0x4000u, 0x7ffdu, 0x7ffeu, 0x7fffu };
	static const std::uint64_t mants[] = {
		0x0000000000000000ULL, 0x0000000000000001ULL,
		0x7fffffffffffffffULL, 0x8000000000000000ULL,
		0x8000000000000001ULL, 0xc000000000000000ULL,
		0xffffffffffffffffULL,
	};
	std::size_t j, s;

	for (i = 0; i < sizeof(exps) / sizeof(exps[0]); i++)
		for (j = 0; j < sizeof(mants) / sizeof(mants[0]); j++)
			for (s = 0; s < 2; s++)
				check_lgammal(ld_from_bits(mants[j],
				    exps[i], (unsigned)s));
#endif
}

/* ---------------------------------------------------------------- */
/* randomised sweeps                                                */
/* ---------------------------------------------------------------- */

static const unsigned long DOUBLE_ITERS = 400000UL;
static const unsigned long LGAMMAL_ITERS = 250000UL;

static void
run_double_random(void)
{
	unsigned long i;
	std::uint64_t r;
	std::uint32_t hi, lo, e;

	static const std::uint32_t near_exp[] = {
		0x000u, 0x001u, 0x002u, 0x3fdu, 0x3feu, 0x3ffu, 0x400u,
		0x7fcu, 0x7fdu, 0x7feu, 0x7ffu,
	};

	for (i = 0; i < DOUBLE_ITERS; i++) {
		r = rnd64();
		switch (i & 3u) {
		case 0:
			check_double(std::bit_cast<double>(r));
			break;
		case 1:
			e = near_exp[(rnd64() >> 13) %
			    (sizeof(near_exp) / sizeof(near_exp[0]))];
			hi = (std::uint32_t)((r & 1u) << 31) | (e << 20) |
			    (std::uint32_t)((r >> 8) & 0xfffffu);
			lo = (std::uint32_t)(rnd64() & 0xffffffffu);
			check_double(mkdouble(hi, lo));
			break;
		case 2:
			hi = (std::uint32_t)((r & 1u) << 31) |
			    (std::uint32_t)((r >> 20) & 0x000fffffu);
			lo = (std::uint32_t)(rnd64() & 0xffffffffu);
			check_double(mkdouble(hi, lo));
			break;
		default:
			hi = (std::uint32_t)((r & 1u) << 31) |
			    (0x7ff00000u - (std::uint32_t)((r >> 4) & 0x7u) *
			    0x00100000u) |
			    (std::uint32_t)((r >> 24) & 0xfffffu);
			lo = (std::uint32_t)(rnd64() & 0xffffffffu);
			check_double(mkdouble(hi, lo));
			break;
		}
	}
}

static void
run_lgammal_random(void)
{
	unsigned long i;
	std::uint64_t r;

	for (i = 0; i < LGAMMAL_ITERS; i++) {
		r = rnd64();
		switch (i & 3u) {
		case 0:
			check_lgammal((long double)
			    ((double)(std::int64_t)r / 4294967296.0));
			break;
		case 1:
			check_lgammal((long double)
			    (std::bit_cast<double>(r)));
			break;
#ifdef PBSD_LD80
		case 2:
			check_lgammal(ld_from_bits(rnd64(),
			    (unsigned)(r >> 40) & 0x7fffu, (unsigned)(r & 1u)));
			break;
		default:
			check_lgammal(ld_from_bits(rnd64(),
			    0x3fffu + ((unsigned)(r >> 8) & 0x1fu) -
			    ((unsigned)(r >> 16) & 0x1fu),
			    (unsigned)(r & 1u)));
			break;
#else
		case 2:
			check_lgammal((long double)(std::int32_t)(r >> 32) /
			    3.0L);
			break;
		default:
			check_lgammal(-(long double)((r >> 40) & 0xffffu) /
			    16.0L);
			break;
#endif
		}
	}
}

/* ---------------------------------------------------------------- */

static void
coverage(Stat *s, const char *what, unsigned long long n)
{
	if (n != 0)
		return;
	s->failures++;
	std::printf("  %s COVERAGE GAP: %s never exercised\n", s->name, what);
}

int
main(void)
{
	rng_state = 0xC0FFEE123456789ULL;

	run_double_edges();
	run_lgammal_edges();
	run_double_random();
	run_lgammal_random();

	coverage(&st_finite, "finite result 1", cov_finite_one);
	coverage(&st_finite, "finite result 0", cov_finite_zero);
	coverage(&st_finite, "|hi| below 0x00100000", cov_finite_sub);
	coverage(&st_finite, "|hi| == 0x7fefffff", cov_finite_maxfinite);
	coverage(&st_finite, "|hi| == 0x7ff00000", cov_finite_inf);
	coverage(&st_fabs, "negative input", cov_fabs_neg);
	coverage(&st_fabs, "positive input", cov_fabs_pos);
	coverage(&st_fabs, "non-zero low word", cov_fabs_lowbits);
	coverage(&st_lgammal, "signgam store", cov_lg_written);
	coverage(&st_lgammal, "signgam < 0", cov_lg_neg_sign);
	coverage(&st_lgammal, "signgam >= 0", cov_lg_pos_sign);

	Stat *all[] = { &st_fabs, &st_finite, &st_lgammal };
	unsigned long long total_fail = 0;
	int i;

	std::printf("\n%-10s %14s %12s  %s\n", "function", "cases", "failures",
	    "result");
	std::printf("---------------------------------------------------\n");
	for (i = 0; i < 3; i++) {
		std::printf("%-10s %14llu %12llu  %s\n", all[i]->name,
		    all[i]->cases, all[i]->failures,
		    all[i]->failures == 0 ? "PASS" : "FAIL");
		total_fail += all[i]->failures;
	}
	std::printf("---------------------------------------------------\n");
	std::printf("%-10s %14llu %12llu  %s\n", "TOTAL",
	    st_fabs.cases + st_finite.cases + st_lgammal.cases, total_fail,
	    total_fail == 0 ? "PASS" : "FAIL");

	return total_fail == 0 ? 0 : 1;
}
