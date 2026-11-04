/*
 * b0023 differential test: pbsd::lib_libc_gdtoa::b0023 vs. the ref_ oracle.
 *
 * __gdtoa_locks: compare the full raw initializer image (both mutex slots).
 * strtold_l: compare the long-double return value bitwise and, when sp is
 * non-null, the end-pointer offset from a guard buffer base plus the entire
 * guard buffer (so a write past the parsed string is caught).
 */

#include <clocale>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstring>

#include <pthread.h>

import pbsd.lib.libc.gdtoa.b0023;

namespace port = pbsd::lib_libc_gdtoa::b0023;

extern "C" {
extern pthread_mutex_t ref___gdtoa_locks[];
long double ref_strtold_l(const char * __restrict s, char ** __restrict sp,
    locale_t locale);
}

enum { FN_LOCKS, FN_STRTOLD, NFUNC };

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

static Stats g_stat[NFUNC] = {
	{ "__gdtoa_locks", 0, 0, 0 },
	{ "strtold_l",     0, 0, 0 },
};

static const int MAXPRINT = 8;
static const int LOCKS_COUNT = 2;

enum { LEAD = 16, STR_MAX = 128, TRAIL = 16,
    BUF_SIZE = LEAD + STR_MAX + TRAIL + 1 };

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

static bool
ld_same(long double a, long double b)
{
	unsigned char pa[sizeof(long double)];
	unsigned char pb[sizeof(long double)];

	std::memcpy(pa, &a, sizeof(a));
	std::memcpy(pb, &b, sizeof(b));
	return std::memcmp(pa, pb, sizeof(a)) == 0;
}

static void
chk_locks(const char *label)
{
	unsigned char pa[sizeof(pthread_mutex_t) * LOCKS_COUNT];
	unsigned char pb[sizeof(pthread_mutex_t) * LOCKS_COUNT];
	bool ok;

	g_stat[FN_LOCKS].cases++;

	std::memcpy(pa, port::__gdtoa_locks, sizeof(pa));
	std::memcpy(pb, ref___gdtoa_locks, sizeof(pb));
	ok = std::memcmp(pa, pb, sizeof(pa)) == 0;

	if (!ok)
		fail(FN_LOCKS, label);
}

static void
chk_strtold(const char *label, const unsigned char *raw, int len,
    bool use_sp, locale_t loc)
{
	unsigned char abuf[BUF_SIZE];
	unsigned char bbuf[BUF_SIZE];
	char *aend;
	char *bend;
	const char *as;
	const char *bs;
	char **asp;
	char **bsp;
	long double ar;
	long double br;
	std::ptrdiff_t aoff;
	std::ptrdiff_t boff;
	bool ok;

	if (len < 0)
		len = 0;
	if (len > STR_MAX)
		len = STR_MAX;

	std::memset(abuf, 0x7f, sizeof(abuf));
	std::memset(bbuf, 0x7f, sizeof(bbuf));
	std::memcpy(abuf + LEAD, raw, (std::size_t)len);
	std::memcpy(bbuf + LEAD, raw, (std::size_t)len);
	abuf[LEAD + len] = '\0';
	bbuf[LEAD + len] = '\0';

	as = reinterpret_cast<const char *>(abuf + LEAD);
	bs = reinterpret_cast<const char *>(bbuf + LEAD);

	if (use_sp) {
		aend = reinterpret_cast<char *>(abuf + LEAD);
		bend = reinterpret_cast<char *>(bbuf + LEAD);
		asp = &aend;
		bsp = &bend;
	} else {
		asp = nullptr;
		bsp = nullptr;
	}

	ar = port::strtold_l(as, asp, loc);
	br = ref_strtold_l(bs, bsp, loc);

	ok = ld_same(ar, br);
	if (use_sp) {
		aoff = aend - reinterpret_cast<char *>(abuf);
		boff = bend - reinterpret_cast<char *>(bbuf);
		ok = ok && aoff == boff;
	}
	ok = ok && std::memcmp(abuf, bbuf, sizeof(abuf)) == 0;

	g_stat[FN_STRTOLD].cases++;
	if (!ok)
		fail(FN_STRTOLD, label);
}

static void
chk_strtold_cstr(const char *label, const char *s, bool use_sp, locale_t loc)
{
	chk_strtold(label, reinterpret_cast<const unsigned char *>(s),
	    s ? (int)std::strlen(s) : 0, use_sp, loc);
}

static void
test_locks_edges(void)
{
	chk_locks("initial image");
}

static void
test_locks_random(long iters)
{
	char label[64];

	for (long i = 0; i < iters; i++) {
		std::snprintf(label, sizeof(label), "random-%ld", i);
		chk_locks(label);
	}
}

static void
test_strtold_edges(locale_t loc)
{
	static const char *const strings[] = {
		"",
		" ",
		"\t",
		"\n",
		"\r\n",
		"0",
		"-0",
		"+0",
		"1",
		"-1",
		"+1",
		"00",
		"01",
		"0.",
		".",
		".0",
		".5",
		"0.5",
		"5.",
		"5.0",
		"-.5",
		"+.5",
		"1.",
		"1.0",
		"1e0",
		"1e+0",
		"1e-0",
		"1E0",
		"1e1",
		"1e-1",
		"1e+1",
		"1e308",
		"1e-308",
		"1e309",
		"1e-324",
		"1e-325",
		"2.2250738585072014e-308",
		"1.7976931348623157e+308",
		"nan",
		"NaN",
		"NAN",
		"nan()",
		"inf",
		"Inf",
		"INF",
		"infinity",
		"INFINITY",
		"+inf",
		"-inf",
		"+infinity",
		"-infinity",
		"0x0p0",
		"0x1p0",
		"0x1.8p1",
		"0x1.fffffffffffffp1023",
		"0x1p-1022",
		"0x0.0000000000001p-1022",
		"x",
		"X",
		"12x34",
		"1..2",
		"1e",
		"e1",
		".e1",
		"1e9999",
		"1e-9999",
		"++1",
		"--1",
		"+-1",
		"-+1",
		"  1.5",
		"1.5  ",
		"  1.5  ",
		"\x80",
		"\xff",
		"\x7f",
		"1\x80",
		"\x80""1",
		"0\x80",
		"\xff\xff",
		"12345678901234567890",
		"123456789012345678901234567890",
		"1.234567890123456789",
		"-1.234567890123456789e-200",
		"2147483647",
		"-2147483648",
		"9223372036854775807",
		"-9223372036854775808",
		"0x",
		"0x1",
		"0xp0",
		"0x1p",
		"0x1.0",
		"0x1.0p",
		"0x1.0p+",
		"0x1.0p-",
		"0x1.0p1024",
		"0x1.0p-1025",
		"1p0",
		"1p+0",
		"1p-0",
		"1.0p0",
		"1.0p+0",
		"1.0p-0",
		"1.0p1024",
		"1.0p-1025",
		"000.000",
		"0001.0000",
		".0001",
		"000.",
		"000",
		"0000",
		"0x0.0p0",
		"0x0.1p0",
		"0x1.0p-1074",
		"0x1.0p1074",
		"0x1p-1074",
		"0x1p1074",
		"1e4932",
		"1e-4933",
		"1e4933",
		"1e-4932",
	};

	for (unsigned i = 0; i < sizeof(strings) / sizeof(strings[0]); i++) {
		chk_strtold_cstr(strings[i], strings[i], false, loc);
		chk_strtold_cstr(strings[i], strings[i], true, loc);
	}

	{
		static const unsigned char nul5[] =
		    { '1', '\0', '2', '\0', '3' };
		static const unsigned char nul_only[] = { '\0', '\0', '\0' };
		static const unsigned char lead_nul[] = { '\0', '1', '2' };
		static const unsigned char hi_nul[] =
		    { 0xff, '\0', '1', 0x80, '\0' };

		chk_strtold("nul5-false", nul5, 5, false, loc);
		chk_strtold("nul5-true", nul5, 5, true, loc);
		chk_strtold("nul_only-false", nul_only, 3, false, loc);
		chk_strtold("nul_only-true", nul_only, 3, true, loc);
		chk_strtold("lead_nul-false", lead_nul, 3, false, loc);
		chk_strtold("lead_nul-true", lead_nul, 3, true, loc);
		chk_strtold("hi_nul-false", hi_nul, 5, false, loc);
		chk_strtold("hi_nul-true", hi_nul, 5, true, loc);
	}

	for (int b = 0x80; b <= 0xff; b++) {
		unsigned char one[1] = { (unsigned char)b };
		char label[32];

		std::snprintf(label, sizeof(label), "byte-%02x-false", b);
		chk_strtold(label, one, 1, false, loc);
		std::snprintf(label, sizeof(label), "byte-%02x-true", b);
		chk_strtold(label, one, 1, true, loc);
	}

	for (int len = STR_MAX - 2; len <= STR_MAX; len++) {
		unsigned char buf[STR_MAX + 1];
		char label[32];

		for (int i = 0; i < len; i++)
			buf[i] = (unsigned char)(0x30 + (i % 10));
		std::snprintf(label, sizeof(label), "len-%d-false", len);
		chk_strtold(label, buf, len, false, loc);
		std::snprintf(label, sizeof(label), "len-%d-true", len);
		chk_strtold(label, buf, len, true, loc);
	}
}

static void
test_strtold_random(long iters, locale_t loc)
{
	unsigned char raw[STR_MAX + 1];
	char label[64];

	for (long i = 0; i < iters; i++) {
		int len = (int)(rnd() % (STR_MAX + 1));
		bool use_sp = (rnd() & 1) != 0;

		for (int j = 0; j < len; j++) {
			switch (rnd() % 8) {
			case 0: raw[j] = (unsigned char)('0' + (rnd() % 10)); break;
			case 1: raw[j] = (unsigned char)('a' + (rnd() % 26)); break;
			case 2: raw[j] = (unsigned char)('A' + (rnd() % 26)); break;
			case 3: raw[j] = (unsigned char)(0x80 + (rnd() % 0x80)); break;
			case 4: raw[j] = (unsigned char)(rnd() & 0xff); break;
			case 5: raw[j] = '.'; break;
			case 6: raw[j] = 'e'; break;
			default: raw[j] = '\0'; break;
			}
		}

		std::snprintf(label, sizeof(label), "random-%ld", i);
		chk_strtold(label, raw, len, use_sp, loc);
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
	locale_t loc = newlocale(LC_ALL_MASK, "C", (locale_t)0);

	if (loc == (locale_t)0) {
		std::fprintf(stderr, "newlocale(C) failed\n");
		return 1;
	}

	test_locks_edges();
	test_strtold_edges(loc);

	test_locks_random(100000);
	test_strtold_random(200000, loc);

	freelocale(loc);
	report();
	return g_stat[FN_LOCKS].fails + g_stat[FN_STRTOLD].fails == 0 ? 0 : 1;
}
