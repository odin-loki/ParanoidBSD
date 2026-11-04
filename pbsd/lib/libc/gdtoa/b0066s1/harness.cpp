/*
 * b0066s1 differential test: pbsd::lib_libc_gdtoa::b0066s1 vs. the ref_ oracle.
 */

#include <cfloat>
#include <cfenv>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <limits>

import pbsd.lib.libc.gdtoa.b0066s1;

namespace port = pbsd::lib_libc_gdtoa::b0066s1;

extern "C" {
char *ref___ldtoa(long double *ld, int mode, int ndigits, int *decpt, int *sign,
    char **rve);
void pbsd_case_begin(int side);

#define	PBSD_ARENA_SIZE	512

struct pbsd_log {
	int alloc_calls;
	int alloc_n[2];
	long alloc_off[2];
	long alloc_usable[2];
	int nrv_calls;
	int nrv_n[2];
	char nrv_s[2][96];
	int gdtoa_calls;
	int fpi_nbits;
	int fpi_emin;
	int fpi_emax;
	int fpi_rounding;
	int fpi_sudden_underflow;
	int g_be;
	int g_kind;
	int g_mode;
	int g_ndigits;
	unsigned int g_bits[2];
};

extern unsigned char pbsd_arena[2][PBSD_ARENA_SIZE];
extern struct pbsd_log pbsd_logs[2];
extern int pbsd_gdtoa_decpt;
}

enum { FN_LDTOA, NFUNC };

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

static Stats g_stat[NFUNC] = { { "__ldtoa", 0, 0, 0 } };
static const int MAXPRINT = 8;
static std::uint64_t rng_state = 0x0123456789abcdefULL;

static std::uint64_t rnd(void)
{
	std::uint64_t z;
	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static void fail(int fn, const char *label)
{
	g_stat[fn].fails++;
	if (g_stat[fn].printed++ < MAXPRINT)
		std::printf("  FAIL %-14s %s\n", g_stat[fn].name, label);
}

struct LdtoaObs {
	char *ret;
	int decpt;
	int sign;
	std::ptrdiff_t rve_off;
	unsigned char arena[PBSD_ARENA_SIZE];
	pbsd_log log;
};

static bool log_same(const pbsd_log &a, const pbsd_log &b)
{
	return a.gdtoa_calls == b.gdtoa_calls && a.fpi_nbits == b.fpi_nbits &&
	    a.fpi_emin == b.fpi_emin && a.fpi_emax == b.fpi_emax &&
	    a.fpi_rounding == b.fpi_rounding &&
	    a.fpi_sudden_underflow == b.fpi_sudden_underflow &&
	    a.g_be == b.g_be && a.g_kind == b.g_kind && a.g_mode == b.g_mode &&
	    a.g_ndigits == b.g_ndigits && a.g_bits[0] == b.g_bits[0] &&
	    a.g_bits[1] == b.g_bits[1] && a.alloc_calls == b.alloc_calls &&
	    a.nrv_calls == b.nrv_calls;
}

static LdtoaObs run_port(long double *ldp, int mode, int ndigits, bool use_rve,
    int gdtoa_decpt)
{
	LdtoaObs obs{};
	char *rve = nullptr;
	char **rvep = use_rve ? &rve : nullptr;
	pbsd_gdtoa_decpt = gdtoa_decpt;
	pbsd_case_begin(0);
	obs.ret = port::__ldtoa(ldp, mode, ndigits, &obs.decpt, &obs.sign, rvep);
	if (use_rve && obs.ret != nullptr)
		obs.rve_off = rve - obs.ret;
	std::memcpy(obs.arena, pbsd_arena[0], PBSD_ARENA_SIZE);
	obs.log = pbsd_logs[0];
	return obs;
}

static LdtoaObs run_ref(long double *ldp, int mode, int ndigits, bool use_rve,
    int gdtoa_decpt)
{
	LdtoaObs obs{};
	char *rve = nullptr;
	char **rvep = use_rve ? &rve : nullptr;
	pbsd_gdtoa_decpt = gdtoa_decpt;
	pbsd_case_begin(1);
	obs.ret = ref___ldtoa(ldp, mode, ndigits, &obs.decpt, &obs.sign, rvep);
	if (use_rve && obs.ret != nullptr)
		obs.rve_off = rve - obs.ret;
	std::memcpy(obs.arena, pbsd_arena[1], PBSD_ARENA_SIZE);
	obs.log = pbsd_logs[1];
	return obs;
}

static bool str_same(const char *a, const char *b)
{
	if (a == b) return true;
	if (a == nullptr || b == nullptr) return false;
	return std::strcmp(a, b) == 0;
}

static void chk_ldtoa(const char *label, long double *ldp, int mode, int ndigits,
    bool use_rve, int gdtoa_decpt)
{
	LdtoaObs a = run_port(ldp, mode, ndigits, use_rve, gdtoa_decpt);
	LdtoaObs b = run_ref(ldp, mode, ndigits, use_rve, gdtoa_decpt);
	bool ok = a.decpt == b.decpt && a.sign == b.sign && str_same(a.ret, b.ret);
	if (use_rve && a.ret != nullptr && b.ret != nullptr)
		ok = ok && a.rve_off == b.rve_off;
	ok = ok && std::memcmp(a.arena, b.arena, PBSD_ARENA_SIZE) == 0;
	ok = ok && log_same(a.log, b.log);
	g_stat[FN_LDTOA].cases++;
	if (!ok) fail(FN_LDTOA, label);
}

static long double ld_from_bytes(const unsigned char b[10])
{
	long double ld;
	std::memset(&ld, 0, sizeof(ld));
	std::memcpy(&ld, b, 10);
	return ld;
}

static void test_ldtoa_edges(void)
{
	static const long double vals[] = {
		0.0L, -0.0L, 1.0L, -1.0L, 0.5L, -0.5L,
		3.14159265358979323846L, -3.14159265358979323846L,
		LDBL_MIN, -LDBL_MIN, LDBL_MAX, -LDBL_MAX,
		0x1p-16382L, -0x1p-16382L, 0x1p16383L,
		0x1.1p-16445L, 0x1.fffffffffffffp16383L,
	};
	static const int gdtoa_decpts[] = {
		0, 1, -1, 42, -32768, -32767, -32769, INT_MAX, INT_MIN,
	};
	static const int modes[] = { 0, 1, 2, 3, 4, 5 };
	static const int ndigs[] = { -1, 0, 1, 2, 5, 15, 30, 80 };

	for (unsigned i = 0; i < sizeof(vals) / sizeof(vals[0]); i++) {
		long double v = vals[i];
		for (unsigned m = 0; m < sizeof(modes) / sizeof(modes[0]); m++) {
			for (unsigned n = 0; n < sizeof(ndigs) / sizeof(ndigs[0]); n++) {
				for (unsigned d = 0;
				    d < sizeof(gdtoa_decpts) / sizeof(gdtoa_decpts[0]); d++) {
					char label[80];
					std::snprintf(label, sizeof(label),
					    "edge-%u-m%d-n%d-d%d-f", i, modes[m],
					    ndigs[n], gdtoa_decpts[d]);
					chk_ldtoa(label, &v, modes[m], ndigs[n], false,
					    gdtoa_decpts[d]);
					std::snprintf(label, sizeof(label),
					    "edge-%u-m%d-n%d-d%d-t", i, modes[m],
					    ndigs[n], gdtoa_decpts[d]);
					chk_ldtoa(label, &v, modes[m], ndigs[n], true,
					    gdtoa_decpts[d]);
				}
			}
		}
	}

	{
		long double pinf = std::numeric_limits<long double>::infinity();
		long double ninf = -pinf;
		long double nanv = std::numeric_limits<long double>::quiet_NaN();
		long double nanp = std::numeric_limits<long double>::signaling_NaN();
		chk_ldtoa("inf-f", &pinf, 0, 0, false, 0);
		chk_ldtoa("ninf-f", &ninf, 0, 0, false, 0);
		chk_ldtoa("nan-f", &nanv, 0, 0, false, 0);
		chk_ldtoa("snan-f", &nanp, 0, 0, false, 0);
		chk_ldtoa("inf-t", &pinf, 2, 5, true, -32768);
		chk_ldtoa("nan-t", &nanv, 3, -1, true, 7);
	}

	{
		static const unsigned char raw_bits[][10] = {
			{0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0x80},
			{1,0,0,0,0,0,0,0,0,0}, {1,0,0,0,0,0,0,0,0,0x80},
			{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f},
			{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},
			{0,0,0,0,0,0,0,0x80,0xff,0x7f},
			{0,0,0,0,0,0,0,0x80,0xff,0xff},
			{0x80,0,0,0,0,0,0,0,0,0x40},
			{0x80,0,0,0,0,0,0,0,0,0xc0},
		};
		for (unsigned i = 0; i < sizeof(raw_bits) / sizeof(raw_bits[0]); i++) {
			long double v = ld_from_bytes(raw_bits[i]);
			char label[48];
			std::snprintf(label, sizeof(label), "raw-%u-f", i);
			chk_ldtoa(label, &v, 0, 0, false, 0);
			std::snprintf(label, sizeof(label), "raw-%u-t", i);
			chk_ldtoa(label, &v, 1, 3, true, -32768);
		}
	}

	{
		static const int rounds[] = {
			FE_TONEAREST, FE_TOWARDZERO, FE_UPWARD, FE_DOWNWARD,
		};
		long double pos = 1.25L;
		long double neg = -1.25L;
		for (unsigned r = 0; r < sizeof(rounds) / sizeof(rounds[0]); r++) {
			char label[48];
			std::fesetround(rounds[r]);
			std::snprintf(label, sizeof(label), "round%u-pos", r);
			chk_ldtoa(label, &pos, 0, 6, true, 3);
			std::snprintf(label, sizeof(label), "round%u-neg", r);
			chk_ldtoa(label, &neg, 0, 6, true, 3);
		}
		std::fesetround(FE_TONEAREST);
	}
}

static long double rnd_long_double(void)
{
	unsigned char b[10];
	for (int i = 0; i < 10; i++)
		b[i] = (unsigned char)(rnd() & 0xff);
	switch (rnd() % 20) {
	case 0: std::memset(b, 0, 10); break;
	case 1: std::memset(b, 0, 10); b[9] = 0x80; break;
	case 2: std::memset(b, 0, 10); b[8] = 0x80; b[9] = 0x7f; break;
	case 3: std::memset(b, 0, 10); b[8] = 0x80; b[9] = 0xff; break;
	case 4: std::memset(b, 0, 10); b[0] = 0x01; break;
	case 5: std::memset(b, 0, 10); b[0] = 0x01; b[9] = 0x80; break;
	case 6: b[0] = 0x80; b[9] = (unsigned char)(0x40 | (rnd() & 0x3f)); break;
	case 7: b[0] = 0x80; b[9] = (unsigned char)(0xc0 | (rnd() & 0x3f)); break;
	case 8: b[9] = 0x7f; break;
	case 9: b[9] = 0xff; break;
	default: break;
	}
	return ld_from_bytes(b);
}

static int rnd_gdtoa_decpt(void)
{
	switch (rnd() % 12) {
	case 0: return -32768;
	case 1: return -32767;
	case 2: return -32769;
	case 3: return INT_MAX;
	case 4: return INT_MIN;
	case 5: return 0;
	case 6: return 1;
	case 7: return -1;
	default: return (int)(rnd() % 20000) - 10000;
	}
}

static void test_ldtoa_random(long iters)
{
	char label[64];
	static const int rounds[] = {
		FE_TONEAREST, FE_TOWARDZERO, FE_UPWARD, FE_DOWNWARD,
	};
	for (long i = 0; i < iters; i++) {
		long double v = rnd_long_double();
		int mode = (int)(rnd() % 6);
		int ndigits;
		bool use_rve = (rnd() & 1) != 0;
		int gdtoa_decpt = rnd_gdtoa_decpt();
		if ((rnd() & 7) == 0)
			std::fesetround(rounds[rnd() % 4]);
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
		chk_ldtoa(label, &v, mode, ndigits, use_rve, gdtoa_decpt);
	}
	std::fesetround(FE_TONEAREST);
}

static void report(void)
{
	long long cases = 0, fails = 0;
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

int main(void)
{
	test_ldtoa_edges();
	test_ldtoa_random(200000);
	report();
	return g_stat[FN_LDTOA].fails == 0 ? 0 : 1;
}
