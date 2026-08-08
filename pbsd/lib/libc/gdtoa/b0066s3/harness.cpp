/*
 * b0066s3 differential test: pbsd::lib_libc_gdtoa::b0066s3 vs. the ref_ oracle.
 *
 * __hdtoa: compare return-string offsets, decpt, sign, rve offsets, and the
 * full instrumented rv_alloc arena (guard byte 0x7f everywhere else).
 */

#include <cfloat>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <limits>

import pbsd.lib.libc.gdtoa.b0066s3;

namespace port = pbsd::lib_libc_gdtoa::b0066s3;

extern "C" {
char *ref___hdtoa(double d, const char *xdigs, int ndigits, int *decpt, int *sign,
    char **rve);
void pbsd_case_begin(int side);
extern unsigned char pbsd_arena[][512];
}

enum { PBSD_ARENA_SIZE = 512, FN_HDTOA, NFUNC };

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

static Stats g_stat[NFUNC] = {
	{ "__hdtoa", 0, 0, 0 },
};

static const int MAXPRINT = 8;
static const char XDIGS_UP[] = "0123456789ABCDEF";
static const char XDIGS_LO[] = "0123456789abcdef";
static const int DBL_SIGFIGS = (DBL_MANT_DIG + 3) / 4 + 1;

static std::uint64_t rng_state = 0x0123456789abcdefULL;

static std::uint64_t
rnd(void)
{
	std::uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static void
fail(int fn, const char *label)
{
	g_stat[fn].fails++;
	if (g_stat[fn].printed++ < MAXPRINT)
		std::printf("  FAIL %-14s %s\n", g_stat[fn].name, label);
}

static double
bits_to_double(std::uint64_t u)
{
	double d;

	std::memcpy(&d, &u, sizeof(d));
	return d;
}

static bool
str_same(const char *a, const char *b)
{
	if (a == b)
		return true;
	if (a == nullptr || b == nullptr)
		return false;
	return std::strcmp(a, b) == 0;
}

static void
chk_hdtoa(int fn, const char *label, double d, const char *xdigs, int ndigits,
    bool use_rve)
{
	unsigned char a_snap[PBSD_ARENA_SIZE];
	unsigned char b_snap[PBSD_ARENA_SIZE];
	int a_decpt, b_decpt, a_sign, b_sign;
	char *a_rve = nullptr;
	char *b_rve = nullptr;
	char **a_rvep = use_rve ? &a_rve : nullptr;
	char **b_rvep = use_rve ? &b_rve : nullptr;
	char *a_ret;
	char *b_ret;
	std::ptrdiff_t a_ret_off;
	std::ptrdiff_t b_ret_off;
	std::ptrdiff_t a_rve_off;
	std::ptrdiff_t b_rve_off;
	bool ok;

	pbsd_case_begin(0);
	a_ret = port::__hdtoa(d, xdigs, ndigits, &a_decpt, &a_sign, a_rvep);
	std::memcpy(a_snap, pbsd_arena[0], PBSD_ARENA_SIZE);

	pbsd_case_begin(1);
	b_ret = ref___hdtoa(d, xdigs, ndigits, &b_decpt, &b_sign, b_rvep);
	std::memcpy(b_snap, pbsd_arena[1], PBSD_ARENA_SIZE);

	ok = a_decpt == b_decpt && a_sign == b_sign;
	ok = ok && str_same(a_ret, b_ret);

	if (a_ret != nullptr && b_ret != nullptr) {
		a_ret_off = a_ret - reinterpret_cast<char *>(pbsd_arena[0]);
		b_ret_off = b_ret - reinterpret_cast<char *>(pbsd_arena[1]);
		ok = ok && a_ret_off == b_ret_off;
	} else {
		ok = ok && a_ret == b_ret;
	}

	if (use_rve && a_ret != nullptr && b_ret != nullptr) {
		a_rve_off = a_rve - a_ret;
		b_rve_off = b_rve - b_ret;
		ok = ok && a_rve_off == b_rve_off;
	}

	ok = ok && std::memcmp(a_snap, b_snap, PBSD_ARENA_SIZE) == 0;

	g_stat[fn].cases++;
	if (!ok)
		fail(fn, label);
}

static void
test_hdtoa_edges(void)
{
	static const double vals[] = {
		0.0,
		-0.0,
		1.0,
		-1.0,
		0.5,
		-0.5,
		3.14159265358979323846,
		-3.14159265358979323846,
		DBL_MIN,
		-DBL_MIN,
		DBL_MAX,
		-DBL_MAX,
		0x1p-1022,
		-0x1p-1022,
		0x1p1023,
		0x1.1p-1074,
		0x1.fffffffffffffp1023,
		0x1p-1074,
		-0x1p-1074,
		0x1.8p1,
		-0x1.8p1,
		0x1.0p0,
		0x1.0p-1,
		0x1.0p1,
	};

	static const int ndigs[] = {
		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 20,
	};

	for (unsigned i = 0; i < sizeof(vals) / sizeof(vals[0]); i++) {
		for (unsigned j = 0; j < sizeof(ndigs) / sizeof(ndigs[0]); j++) {
			char label[64];
			std::snprintf(label, sizeof(label), "up-%u-%d-f", i, ndigs[j]);
			chk_hdtoa(FN_HDTOA, label, vals[i], XDIGS_UP, ndigs[j], false);
			std::snprintf(label, sizeof(label), "up-%u-%d-t", i, ndigs[j]);
			chk_hdtoa(FN_HDTOA, label, vals[i], XDIGS_UP, ndigs[j], true);
			std::snprintf(label, sizeof(label), "lo-%u-%d-f", i, ndigs[j]);
			chk_hdtoa(FN_HDTOA, label, vals[i], XDIGS_LO, ndigs[j], false);
			std::snprintf(label, sizeof(label), "lo-%u-%d-t", i, ndigs[j]);
			chk_hdtoa(FN_HDTOA, label, vals[i], XDIGS_LO, ndigs[j], true);
		}
	}

	{
		double pinf = std::numeric_limits<double>::infinity();
		double ninf = -pinf;
		double nanv = std::numeric_limits<double>::quiet_NaN();
		double snan = std::numeric_limits<double>::signaling_NaN();
		chk_hdtoa(FN_HDTOA, "inf-f", pinf, XDIGS_UP, 1, false);
		chk_hdtoa(FN_HDTOA, "ninf-f", ninf, XDIGS_UP, 1, false);
		chk_hdtoa(FN_HDTOA, "nan-f", nanv, XDIGS_UP, 1, false);
		chk_hdtoa(FN_HDTOA, "snan-f", snan, XDIGS_UP, 1, false);
		chk_hdtoa(FN_HDTOA, "inf-t", pinf, XDIGS_LO, -1, true);
		chk_hdtoa(FN_HDTOA, "nan-t", nanv, XDIGS_LO, 0, true);
		chk_hdtoa(FN_HDTOA, "zero-t", 0.0, XDIGS_UP, -1, true);
	}

	/* Rounding branch: SIGFIGS > ndigits && ndigits > 0 */
	for (int nd = 1; nd < DBL_SIGFIGS; nd++) {
		static const double round_vals[] = {
			0x1.23456789abcdep0,
			-0x1.23456789abcdep0,
			0x1.fffffffffffffp0,
			-0x1.fffffffffffffp0,
			0x1.0000000000001p0,
			0x1.8p10,
			-0x1.8p10,
		};
		for (unsigned k = 0; k < sizeof(round_vals) / sizeof(round_vals[0]);
		    k++) {
			char label[64];
			std::snprintf(label, sizeof(label), "round-%d-%u-f", nd, k);
			chk_hdtoa(FN_HDTOA, label, round_vals[k], XDIGS_UP, nd, false);
			std::snprintf(label, sizeof(label), "round-%d-%u-t", nd, k);
			chk_hdtoa(FN_HDTOA, label, round_vals[k], XDIGS_LO, nd, true);
		}
	}

	/* ndigits < 0 auto-size: values with trailing zero hex digits */
	{
		static const double trim_vals[] = {
			0x1p0,
			0x1p4,
			0x1p8,
			0x1p12,
			0x1.0p16,
			0x1.0p20,
			-0x1p4,
		};
		for (unsigned k = 0; k < sizeof(trim_vals) / sizeof(trim_vals[0]); k++) {
			char label[64];
			std::snprintf(label, sizeof(label), "trim-%u-f", k);
			chk_hdtoa(FN_HDTOA, label, trim_vals[k], XDIGS_UP, -1, false);
			std::snprintf(label, sizeof(label), "trim-%u-t", k);
			chk_hdtoa(FN_HDTOA, label, trim_vals[k], XDIGS_LO, -1, true);
		}
	}

	/* Boundary ndigits around SIGFIGS */
	for (int nd = DBL_SIGFIGS - 1; nd <= DBL_SIGFIGS + 2; nd++) {
		char label[64];
		std::snprintf(label, sizeof(label), "sigfig-%d", nd);
		chk_hdtoa(FN_HDTOA, label, 0x1.23456789abcdep0, XDIGS_UP, nd, true);
	}

	/* Explicit bit patterns: subnormal, high exponent, sign combinations */
	static const std::uint64_t patterns[] = {
		0x0000000000000000ULL,
		0x8000000000000000ULL,
		0x0000000000000001ULL,
		0x8000000000000001ULL,
		0x000fffffffffffffULL,
		0x800fffffffffffffULL,
		0x0010000000000000ULL,
		0x7fe0000000000000ULL,
		0xffe0000000000000ULL,
		0x7ff0000000000000ULL,
		0xfff0000000000000ULL,
		0x7ff8000000000000ULL,
		0xfff8000000000000ULL,
		0x7ff0000000000001ULL,
		0x7fefffffffffffffULL,
		0xffefffffffffffffULL,
	};
	for (unsigned i = 0; i < sizeof(patterns) / sizeof(patterns[0]); i++) {
		double d = bits_to_double(patterns[i]);
		for (int nd : { -1, 0, 1, DBL_SIGFIGS }) {
			char label[64];
			std::snprintf(label, sizeof(label), "pat-%u-%d", i, nd);
			chk_hdtoa(FN_HDTOA, label, d, XDIGS_UP, nd, (i & 1) != 0);
		}
	}
}

static double
rnd_double(void)
{
	std::uint64_t u = rnd();
	switch (rnd() % 20) {
	case 0: u = 0; break;
	case 1: u = 0x8000000000000000ULL; break;
	case 2: u = 0x7ff0000000000000ULL; break;
	case 3: u = 0xfff0000000000000ULL; break;
	case 4: u = 0x7ff8000000000000ULL; break;
	case 5: u = 0xfff8000000000000ULL; break;
	case 6: u = 0x0000000000000001ULL; break;
	case 7: u = 0x8000000000000001ULL; break;
	case 8: u = 0x7fefffffffffffffULL; break;
	case 9: u = 0xffefffffffffffffULL; break;
	case 10: u = 0x000fffffffffffffULL; break;
	case 11: u = 0x800fffffffffffffULL; break;
	case 12: u = 0x0010000000000000ULL; break;
	case 13: u = 0x7fe0000000000000ULL; break;
	case 14: u = (rnd() & 0x000fffffffffffffULL) | 0x0010000000000000ULL; break;
	case 15: u = (rnd() & 0x000fffffffffffffULL) | 0x7fe0000000000000ULL; break;
	default: break;
	}
	return bits_to_double(u);
}

static void
test_hdtoa_random(long iters)
{
	char label[64];

	for (long i = 0; i < iters; i++) {
		double v = rnd_double();
		const char *xdigs = (rnd() & 1) ? XDIGS_UP : XDIGS_LO;
		int ndigits;
		bool use_rve = (rnd() & 1) != 0;

		switch (rnd() % 10) {
		case 0: ndigits = -1; break;
		case 1: ndigits = 0; break;
		case 2: ndigits = 1; break;
		case 3: ndigits = (int)(rnd() % (DBL_SIGFIGS - 1)) + 1; break;
		case 4: ndigits = DBL_SIGFIGS - 1; break;
		case 5: ndigits = DBL_SIGFIGS; break;
		case 6: ndigits = DBL_SIGFIGS + (int)(rnd() % 5) + 1; break;
		case 7: ndigits = (int)(rnd() % 20) + 1; break;
		case 8: ndigits = -(int)(rnd() % 3) - 1; break;
		default: ndigits = (int)(rnd() % 40) - 5; break;
		}

		std::snprintf(label, sizeof(label), "random-%ld", i);
		chk_hdtoa(FN_HDTOA, label, v, xdigs, ndigits, use_rve);
	}
}

static void
report(void)
{
	long long cases = 0;
	long long fails = 0;

	std::printf("\n%-18s %12s %12s\n", "function", "cases", "failures");
	std::printf("--------------------------------------------\n");
	for (int i = 0; i < NFUNC; i++) {
		std::printf("%-18s %12lld %12lld\n", g_stat[i].name,
		    g_stat[i].cases, g_stat[i].fails);
		cases += g_stat[i].cases;
		fails += g_stat[i].fails;
	}
	std::printf("--------------------------------------------\n");
	std::printf("%-18s %12lld %12lld\n", "TOTAL", cases, fails);
	std::printf("\n%s\n", fails == 0 ? "PASS" : "FAIL");
	std::fflush(stdout);
}

int
main(void)
{
	test_hdtoa_edges();
	test_hdtoa_random(200000);
	report();
	return g_stat[FN_HDTOA].fails == 0 ? 0 : 1;
}
