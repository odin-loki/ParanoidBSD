/*
 * b0066 differential test: pbsd::lib_libc_gdtoa::b0066 vs. the ref_ oracle.
 *
 * __ldtoa, __hdtoa, __hldtoa: compare return strings, decpt, sign, rve offsets,
 * and the full rv_alloc window (bufsize bytes from each return pointer base).
 */

#include <cfloat>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstring>

import pbsd.lib.libc.gdtoa.b0066;

namespace port = pbsd::lib_libc_gdtoa::b0066;

extern "C" {
char *ref___ldtoa(long double *ld, int mode, int ndigits, int *decpt, int *sign,
    char **rve);
char *ref___hdtoa(double d, const char *xdigs, int ndigits, int *decpt, int *sign,
    char **rve);
char *ref___hldtoa(long double e, const char *xdigs, int ndigits, int *decpt,
    int *sign, char **rve);
void freedtoa(char *);
}

enum { FN_LDTOA, FN_HDTOA, FN_HLDTOA, NFUNC };

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

static Stats g_stat[NFUNC] = {
	{ "__ldtoa",  0, 0, 0 },
	{ "__hdtoa",  0, 0, 0 },
	{ "__hldtoa", 0, 0, 0 },
};

static const int MAXPRINT = 8;
static const char XDIGS_UP[] = "0123456789ABCDEF";
static const char XDIGS_LO[] = "0123456789abcdef";

static const int DBL_SIGFIGS = (DBL_MANT_DIG + 3) / 4 + 1;
static const int LDBL_SIGFIGS = (LDBL_MANT_DIG + 3) / 4 + 1;

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

static int
hdtoa_bufsize(int ndigits)
{
	if (ndigits == 0)
		return 1;
	if (ndigits > 0)
		return ndigits;
	return DBL_SIGFIGS;
}

static int
hldtoa_bufsize(int ndigits)
{
	if (ndigits == 0)
		return 1;
	if (ndigits > 0)
		return ndigits;
	return LDBL_SIGFIGS;
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
chk_ldtoa(int fn, const char *label, long double *ldp, int mode, int ndigits,
    bool use_rve)
{
	int a_decpt, b_decpt, a_sign, b_sign;
	char *a_rve = nullptr;
	char *b_rve = nullptr;
	char **a_rvep = use_rve ? &a_rve : nullptr;
	char **b_rvep = use_rve ? &b_rve : nullptr;
	char *a_ret;
	char *b_ret;
	std::ptrdiff_t aoff;
	std::ptrdiff_t boff;
	bool ok;

	a_ret = port::__ldtoa(ldp, mode, ndigits, &a_decpt, &a_sign, a_rvep);
	b_ret = ref___ldtoa(ldp, mode, ndigits, &b_decpt, &b_sign, b_rvep);

	ok = a_decpt == b_decpt && a_sign == b_sign;
	ok = ok && str_same(a_ret, b_ret);
	if (use_rve && a_ret != nullptr && b_ret != nullptr) {
		aoff = a_rve - a_ret;
		boff = b_rve - b_ret;
		ok = ok && aoff == boff;
	}

	g_stat[fn].cases++;
	if (!ok)
		fail(fn, label);

	if (a_ret != nullptr)
		freedtoa(a_ret);
	if (b_ret != nullptr)
		freedtoa(b_ret);
}

static void
chk_hextoa(int fn, const char *label, double d, long double ld,
    const char *xdigs, int ndigits, bool use_rve, bool is_ld)
{
	int a_decpt, b_decpt, a_sign, b_sign;
	char *a_rve = nullptr;
	char *b_rve = nullptr;
	char **a_rvep = use_rve ? &a_rve : nullptr;
	char **b_rvep = use_rve ? &b_rve : nullptr;
	char *a_ret;
	char *b_ret;
	int bufsize;
	std::ptrdiff_t aoff;
	std::ptrdiff_t boff;
	bool ok;
	bool special;

	if (is_ld) {
		a_ret = port::__hldtoa(ld, xdigs, ndigits, &a_decpt, &a_sign, a_rvep);
		b_ret = ref___hldtoa(ld, xdigs, ndigits, &b_decpt, &b_sign, b_rvep);
		bufsize = hldtoa_bufsize(ndigits);
	} else {
		a_ret = port::__hdtoa(d, xdigs, ndigits, &a_decpt, &a_sign, a_rvep);
		b_ret = ref___hdtoa(d, xdigs, ndigits, &b_decpt, &b_sign, b_rvep);
		bufsize = hdtoa_bufsize(ndigits);
	}

	special = (a_decpt == INT_MAX) || (a_decpt == 1 && a_ret != nullptr &&
	    (std::strcmp(a_ret, "0") == 0 || std::strcmp(a_ret, "Infinity") == 0 ||
	    std::strcmp(a_ret, "NaN") == 0));

	ok = a_decpt == b_decpt && a_sign == b_sign;
	ok = ok && str_same(a_ret, b_ret);
	if (use_rve && a_ret != nullptr && b_ret != nullptr) {
		aoff = a_rve - a_ret;
		boff = b_rve - b_ret;
		ok = ok && aoff == boff;
	}
	if (!special && a_ret != nullptr && b_ret != nullptr && bufsize > 0)
		ok = ok && std::memcmp(a_ret, b_ret, (std::size_t)bufsize) == 0;

	g_stat[fn].cases++;
	if (!ok)
		fail(fn, label);

	if (a_ret != nullptr)
		freedtoa(a_ret);
	if (b_ret != nullptr)
		freedtoa(b_ret);
}

static void
test_ldtoa_edges(void)
{
	static const long double vals[] = {
		0.0L,
		-0.0L,
		1.0L,
		-1.0L,
		0.5L,
		-0.5L,
		3.14159265358979323846L,
		-3.14159265358979323846L,
		LDBL_MIN,
		-LDBL_MIN,
		LDBL_MAX,
		-LDBL_MAX,
		0x1p-16382L,
		-0x1p-16382L,
		0x1p16383L,
		0x1.1p-16445L,
		0x1.fffffffffffffp16383L,
	};

	for (unsigned i = 0; i < sizeof(vals) / sizeof(vals[0]); i++) {
		long double v = vals[i];
		for (int mode = 0; mode <= 5; mode++) {
			static const int ndigs[] = { -1, 0, 1, 2, 5, 15, 30 };
			for (unsigned j = 0; j < sizeof(ndigs) / sizeof(ndigs[0]); j++) {
				char label[64];
				std::snprintf(label, sizeof(label), "edge-%u-m%d-n%d-f",
				    i, mode, ndigs[j]);
				chk_ldtoa(FN_LDTOA, label, &v, mode, ndigs[j], false);
				std::snprintf(label, sizeof(label), "edge-%u-m%d-n%d-t",
				    i, mode, ndigs[j]);
				chk_ldtoa(FN_LDTOA, label, &v, mode, ndigs[j], true);
			}
		}
	}

	{
		long double pinf = std::numeric_limits<long double>::infinity();
		long double ninf = -pinf;
		long double nanv = std::numeric_limits<long double>::quiet_NaN();
		long double nanp = std::numeric_limits<long double>::signaling_NaN();
		chk_ldtoa(FN_LDTOA, "inf-f", &pinf, 0, 0, false);
		chk_ldtoa(FN_LDTOA, "ninf-f", &ninf, 0, 0, false);
		chk_ldtoa(FN_LDTOA, "nan-f", &nanv, 0, 0, false);
		chk_ldtoa(FN_LDTOA, "snan-f", &nanp, 0, 0, false);
		chk_ldtoa(FN_LDTOA, "inf-t", &pinf, 2, 5, true);
		chk_ldtoa(FN_LDTOA, "nan-t", &nanv, 3, -1, true);
	}
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
	};

	static const int ndigs[] = { -1, 0, 1, 2, 5, 14, 20 };

	for (unsigned i = 0; i < sizeof(vals) / sizeof(vals[0]); i++) {
		for (unsigned j = 0; j < sizeof(ndigs) / sizeof(ndigs[0]); j++) {
			char label[64];
			std::snprintf(label, sizeof(label), "up-%u-%d-f", i, ndigs[j]);
			chk_hextoa(FN_HDTOA, label, vals[i], 0.0L, XDIGS_UP, ndigs[j],
			    false, false);
			std::snprintf(label, sizeof(label), "up-%u-%d-t", i, ndigs[j]);
			chk_hextoa(FN_HDTOA, label, vals[i], 0.0L, XDIGS_UP, ndigs[j],
			    true, false);
			std::snprintf(label, sizeof(label), "lo-%u-%d-f", i, ndigs[j]);
			chk_hextoa(FN_HDTOA, label, vals[i], 0.0L, XDIGS_LO, ndigs[j],
			    false, false);
			std::snprintf(label, sizeof(label), "lo-%u-%d-t", i, ndigs[j]);
			chk_hextoa(FN_HDTOA, label, vals[i], 0.0L, XDIGS_LO, ndigs[j],
			    true, false);
		}
	}

	{
		double pinf = std::numeric_limits<double>::infinity();
		double ninf = -pinf;
		double nanv = std::numeric_limits<double>::quiet_NaN();
		chk_hextoa(FN_HDTOA, "inf-f", pinf, 0.0L, XDIGS_UP, 1, false, false);
		chk_hextoa(FN_HDTOA, "ninf-f", ninf, 0.0L, XDIGS_UP, 1, false, false);
		chk_hextoa(FN_HDTOA, "nan-f", nanv, 0.0L, XDIGS_UP, 1, false, false);
		chk_hextoa(FN_HDTOA, "inf-t", pinf, 0.0L, XDIGS_LO, -1, true, false);
	}
}

static void
test_hldtoa_edges(void)
{
	static const long double vals[] = {
		0.0L,
		-0.0L,
		1.0L,
		-1.0L,
		0.5L,
		-0.5L,
		3.14159265358979323846L,
		-3.14159265358979323846L,
		LDBL_MIN,
		-LDBL_MIN,
		LDBL_MAX,
		-LDBL_MAX,
		0x1p-16382L,
		-0x1p-16382L,
		0x1p16383L,
		0x1.1p-16445L,
		0x1.fffffffffffffp16383L,
	};

	static const int ndigs[] = { -1, 0, 1, 2, 5, 16, 25 };

	for (unsigned i = 0; i < sizeof(vals) / sizeof(vals[0]); i++) {
		for (unsigned j = 0; j < sizeof(ndigs) / sizeof(ndigs[0]); j++) {
			char label[64];
			std::snprintf(label, sizeof(label), "up-%u-%d-f", i, ndigs[j]);
			chk_hextoa(FN_HLDTOA, label, 0.0, vals[i], XDIGS_UP, ndigs[j],
			    false, true);
			std::snprintf(label, sizeof(label), "up-%u-%d-t", i, ndigs[j]);
			chk_hextoa(FN_HLDTOA, label, 0.0, vals[i], XDIGS_UP, ndigs[j],
			    true, true);
			std::snprintf(label, sizeof(label), "lo-%u-%d-f", i, ndigs[j]);
			chk_hextoa(FN_HLDTOA, label, 0.0, vals[i], XDIGS_LO, ndigs[j],
			    false, true);
			std::snprintf(label, sizeof(label), "lo-%u-%d-t", i, ndigs[j]);
			chk_hextoa(FN_HLDTOA, label, 0.0, vals[i], XDIGS_LO, ndigs[j],
			    true, true);
		}
	}

	{
		long double pinf = std::numeric_limits<long double>::infinity();
		long double ninf = -pinf;
		long double nanv = std::numeric_limits<long double>::quiet_NaN();
		chk_hextoa(FN_HLDTOA, "inf-f", 0.0, pinf, XDIGS_UP, 1, false, true);
		chk_hextoa(FN_HLDTOA, "ninf-f", 0.0, ninf, XDIGS_UP, 1, false, true);
		chk_hextoa(FN_HLDTOA, "nan-f", 0.0, nanv, XDIGS_UP, 1, false, true);
		chk_hextoa(FN_HLDTOA, "inf-t", 0.0, pinf, XDIGS_LO, -1, true, true);
	}
}

static double
rnd_double(void)
{
	std::uint64_t u = rnd();
	switch (rnd() % 16) {
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
	default: break;
	}
	double d;
	std::memcpy(&d, &u, sizeof(d));
	return d;
}

static long double
rnd_long_double(void)
{
	unsigned char b[10];
	for (int i = 0; i < 10; i++)
		b[i] = (unsigned char)(rnd() & 0xff);
	switch (rnd() % 16) {
	case 0:
		std::memset(b, 0, 10);
		break;
	case 1:
		std::memset(b, 0, 10);
		b[9] = 0x80;
		break;
	case 2:
		std::memset(b, 0, 10);
		b[8] = 0x80;
		b[9] = 0x7f;
		break;
	case 3:
		std::memset(b, 0, 10);
		b[8] = 0x80;
		b[9] = 0xff;
		break;
	case 4:
		std::memset(b, 0, 10);
		b[0] = 0x01;
		break;
	case 5:
		std::memset(b, 0, 10);
		b[0] = 0x01;
		b[9] = 0x80;
		break;
	default:
		break;
	}
	long double ld;
	std::memcpy(&ld, b, sizeof(ld));
	return ld;
}

static void
test_ldtoa_random(long iters)
{
	char label[64];

	for (long i = 0; i < iters; i++) {
		long double v = rnd_long_double();
		int mode = (int)(rnd() % 6);
		int ndigits;
		bool use_rve = (rnd() & 1) != 0;

		switch (rnd() % 8) {
		case 0: ndigits = -1; break;
		case 1: ndigits = 0; break;
		case 2: ndigits = 1; break;
		case 3: ndigits = (int)(rnd() % 5) + 1; break;
		case 4: ndigits = (int)(rnd() % 15) + 1; break;
		case 5: ndigits = (int)(rnd() % 40) + 1; break;
		default: ndigits = (int)(rnd() % 80) + 1; break;
		}

		std::snprintf(label, sizeof(label), "random-%ld", i);
		chk_ldtoa(FN_LDTOA, label, &v, mode, ndigits, use_rve);
	}
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

		switch (rnd() % 8) {
		case 0: ndigits = -1; break;
		case 1: ndigits = 0; break;
		case 2: ndigits = 1; break;
		case 3: ndigits = (int)(rnd() % DBL_SIGFIGS) + 1; break;
		case 4: ndigits = DBL_SIGFIGS; break;
		case 5: ndigits = DBL_SIGFIGS + (int)(rnd() % 5); break;
		default: ndigits = (int)(rnd() % 20) + 1; break;
		}

		std::snprintf(label, sizeof(label), "random-%ld", i);
		chk_hextoa(FN_HDTOA, label, v, 0.0L, xdigs, ndigits, use_rve, false);
	}
}

static void
test_hldtoa_random(long iters)
{
	char label[64];

	for (long i = 0; i < iters; i++) {
		long double v = rnd_long_double();
		const char *xdigs = (rnd() & 1) ? XDIGS_UP : XDIGS_LO;
		int ndigits;
		bool use_rve = (rnd() & 1) != 0;

		switch (rnd() % 8) {
		case 0: ndigits = -1; break;
		case 1: ndigits = 0; break;
		case 2: ndigits = 1; break;
		case 3: ndigits = (int)(rnd() % LDBL_SIGFIGS) + 1; break;
		case 4: ndigits = LDBL_SIGFIGS; break;
		case 5: ndigits = LDBL_SIGFIGS + (int)(rnd() % 5); break;
		default: ndigits = (int)(rnd() % 25) + 1; break;
		}

		std::snprintf(label, sizeof(label), "random-%ld", i);
		chk_hextoa(FN_HLDTOA, label, 0.0, v, xdigs, ndigits, use_rve, true);
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
	test_ldtoa_edges();
	test_hdtoa_edges();
	test_hldtoa_edges();

	test_ldtoa_random(200000);
	test_hdtoa_random(200000);
	test_hldtoa_random(200000);

	report();
	return g_stat[FN_LDTOA].fails + g_stat[FN_HDTOA].fails +
	    g_stat[FN_HLDTOA].fails == 0 ? 0 : 1;
}
