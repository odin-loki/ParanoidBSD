/*
 * b0066s2 differential test: pbsd::lib_libc_gdtoa::b0066s2::__hldtoa vs.
 * ref___hldtoa from oracle.c.
 *
 * Each case resets two guard-filled arenas (0x7f), runs the port on side 0 and
 * the oracle on side 1, then compares return strings, decpt, sign, rve offsets,
 * and the ENTIRE arena including slack past the nominal write window.
 */

#include <cfloat>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstring>

import pbsd.lib.libc.gdtoa.b0066s2;

namespace port = pbsd::lib_libc_gdtoa::b0066s2;

extern "C" {
char *ref___hldtoa(long double e, const char *xdigs, int ndigits, int *decpt,
    int *sign, char **rve);
void pbsd_case_begin(int side);
void __freedtoa(char *);
extern unsigned char pbsd_arena[2][512];
}

enum { PBSD_ARENA_SIZE = 512 };

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

static Stats g_stat = { "__hldtoa", 0, 0, 0 };

static const int MAXPRINT = 8;
static const char XDIGS_UP[] = "0123456789ABCDEF";
static const char XDIGS_LO[] = "0123456789abcdef";
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
fail(const char *label)
{
	g_stat.fails++;
	if (g_stat.printed++ < MAXPRINT)
		std::printf("  FAIL %-14s %s\n", g_stat.name, label);
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
chk_hldtoa(const char *label, long double ld, const char *xdigs, int ndigits,
    bool use_rve)
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

	pbsd_case_begin(0);
	a_ret = port::__hldtoa(ld, xdigs, ndigits, &a_decpt, &a_sign, a_rvep);

	pbsd_case_begin(1);
	b_ret = ref___hldtoa(ld, xdigs, ndigits, &b_decpt, &b_sign, b_rvep);

	bufsize = hldtoa_bufsize(ndigits);
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
	ok = ok && std::memcmp(pbsd_arena[0], pbsd_arena[1], PBSD_ARENA_SIZE) == 0;

	g_stat.cases++;
	if (!ok)
		fail(label);
}

static long double
make_ld_from_bytes(const unsigned char *b, std::size_t n)
{
	long double ld;

	std::memset(&ld, 0, sizeof(ld));
	std::memcpy(&ld, b, n < sizeof(ld) ? n : sizeof(ld));
	return ld;
}

static void
test_edges(void)
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

	static const int ndigs[] = {
		-1, 0, 1, 2, 5, 16, 25,
		LDBL_SIGFIGS - 1,
		LDBL_SIGFIGS,
		LDBL_SIGFIGS + 1,
	};

	for (unsigned i = 0; i < sizeof(vals) / sizeof(vals[0]); i++) {
		for (unsigned j = 0; j < sizeof(ndigs) / sizeof(ndigs[0]); j++) {
			char label[64];
			std::snprintf(label, sizeof(label), "up-%u-%d-f", i, ndigs[j]);
			chk_hldtoa(label, vals[i], XDIGS_UP, ndigs[j], false);
			std::snprintf(label, sizeof(label), "up-%u-%d-t", i, ndigs[j]);
			chk_hldtoa(label, vals[i], XDIGS_UP, ndigs[j], true);
			std::snprintf(label, sizeof(label), "lo-%u-%d-f", i, ndigs[j]);
			chk_hldtoa(label, vals[i], XDIGS_LO, ndigs[j], false);
			std::snprintf(label, sizeof(label), "lo-%u-%d-t", i, ndigs[j]);
			chk_hldtoa(label, vals[i], XDIGS_LO, ndigs[j], true);
		}
	}

	{
		long double pinf = std::numeric_limits<long double>::infinity();
		long double ninf = -pinf;
		long double nanv = std::numeric_limits<long double>::quiet_NaN();
		chk_hldtoa("inf-f", pinf, XDIGS_UP, 1, false);
		chk_hldtoa("ninf-f", ninf, XDIGS_UP, 1, false);
		chk_hldtoa("nan-f", nanv, XDIGS_UP, 1, false);
		chk_hldtoa("inf-t", pinf, XDIGS_LO, -1, true);
	}

	/* xdigs edge cases: empty-ish, single char, NUL-heavy, high-bit bytes. */
	{
		char emptyish[] = { '\0' };
		char single[] = { 'A' };
		char nulheavy[] = {
			'0', '\0', '1', '\0', '2', '\0', '3', '\0',
			'4', '\0', '5', '\0', '6', '\0', '7', '\0',
			'8', '\0', '9', '\0', 'A', '\0', 'B', '\0',
			'C', '\0', 'D', '\0', 'E', '\0', 'F', '\0',
		};
		char hibit[16];
		long double probe = 1.5L;

		for (int k = 0; k < 16; k++)
			hibit[k] = (char)(0x80 + k);
		chk_hldtoa("xd-empty", probe, emptyish, 1, false);
		chk_hldtoa("xd-single", probe, single, 1, false);
		chk_hldtoa("xd-nulheavy-f", probe, nulheavy, 5, false);
		chk_hldtoa("xd-nulheavy-t", probe, nulheavy, -1, true);
		chk_hldtoa("xd-hibit-f", probe, hibit, 8, false);
		chk_hldtoa("xd-hibit-t", probe, hibit, -1, true);
	}

	/* Raw byte patterns: high-bit bytes 0x80-0xFF in the long-double image. */
	{
		unsigned char pat[16];
		long double ld;

		for (int k = 0; k < 16; k++) {
			std::memset(pat, 0, sizeof(pat));
			pat[k] = (unsigned char)(0x80 + k);
			ld = make_ld_from_bytes(pat, sizeof(pat));
			char label[32];
			std::snprintf(label, sizeof(label), "hibyte-%02x", pat[k]);
			chk_hldtoa(label, ld, XDIGS_UP, -1, true);
		}
		for (int k = 0; k < 16; k++) {
			std::memset(pat, 0xff, sizeof(pat));
			pat[k] = (unsigned char)(0x80 + k);
			ld = make_ld_from_bytes(pat, sizeof(pat));
			char label[32];
			std::snprintf(label, sizeof(label), "hibyteff-%02x", pat[k]);
			chk_hldtoa(label, ld, XDIGS_LO, 3, false);
		}
	}

	/* Boundary ndigits around SIGFIGS with values that stress rounding. */
	{
		long double ld = 0x1.fffffffffffffp16382L;
		chk_hldtoa("sig-bnd-0", ld, XDIGS_UP, 0, false);
		chk_hldtoa("sig-bnd-1", ld, XDIGS_UP, 1, true);
		chk_hldtoa("sig-bnd-s", ld, XDIGS_UP, LDBL_SIGFIGS, false);
		chk_hldtoa("sig-bnd-s+1", ld, XDIGS_UP, LDBL_SIGFIGS + 1, true);
		chk_hldtoa("sig-bnd-neg", ld, XDIGS_LO, -1, false);
	}
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
	std::memset(&ld, 0, sizeof(ld));
	std::memcpy(&ld, b, 10);
	return ld;
}

static void
test_random(long iters)
{
	char label[64];
	char rnd_xdigs[16];

	for (long i = 0; i < iters; i++) {
		long double v = rnd_long_double();
		const char *xdigs;
		int ndigits;
		bool use_rve = (rnd() & 1) != 0;

		if ((rnd() & 3) == 0) {
			for (int k = 0; k < 16; k++)
				rnd_xdigs[k] = (char)(0x80 + (rnd() & 0x0f));
			xdigs = rnd_xdigs;
		} else {
			xdigs = (rnd() & 1) ? XDIGS_UP : XDIGS_LO;
		}

		switch (rnd() % 8) {
		case 0:
			ndigits = -1;
			break;
		case 1:
			ndigits = 0;
			break;
		case 2:
			ndigits = 1;
			break;
		case 3:
			ndigits = (int)(rnd() % LDBL_SIGFIGS) + 1;
			break;
		case 4:
			ndigits = LDBL_SIGFIGS;
			break;
		case 5:
			ndigits = LDBL_SIGFIGS + (int)(rnd() % 5);
			break;
		default:
			ndigits = (int)(rnd() % 25) + 1;
			break;
		}

		std::snprintf(label, sizeof(label), "random-%ld", i);
		chk_hldtoa(label, v, xdigs, ndigits, use_rve);
	}
}

static void
report(void)
{
	std::printf("\n%-18s %12s %12s\n", "function", "cases", "failures");
	std::printf("--------------------------------------------\n");
	std::printf("%-18s %12lld %12lld\n", g_stat.name, g_stat.cases,
	    g_stat.fails);
	std::printf("--------------------------------------------\n");
	std::printf("%-18s %12lld %12lld\n", "TOTAL", g_stat.cases,
	    g_stat.fails);
	std::printf("\n%s\n", g_stat.fails == 0 ? "PASS" : "FAIL");
	std::fflush(stdout);
}

int
main(void)
{
	test_edges();
	test_random(200000);
	report();
	return g_stat.fails == 0 ? 0 : 1;
}
