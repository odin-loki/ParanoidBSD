/*
 * PBSD batch b0153s2 -- differential test.
 *
 * Every case is executed against both the C++23 port and the ref_ oracle in
 * oracle.c, and the results are compared exhaustively: return value, errno,
 * and the complete destination buffer (head guard, nominal write window and
 * tail guard, all pre-filled with 0x7f).
 *
 * Source under test: hbsd/src/lib/libc/locale/wcsftime.c
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cwchar>

#include <locale.h>
#include <sys/resource.h>

import pbsd.lib.libc.locale.b0153s2;

namespace P = pbsd::lib_libc_locale::b0153s2;

extern "C" {
size_t	ref_wcsftime_l(wchar_t * __restrict, size_t,
	    const wchar_t * __restrict, const struct tm * __restrict,
	    locale_t);
size_t	ref_wcsftime(wchar_t * __restrict, size_t,
	    const wchar_t * __restrict, const struct tm * __restrict);
}

#define SIZE_T_MAX_LOCAL	((size_t)-1)

enum { F_WCSFTIME_L, F_WCSFTIME, F_COUNT };

static const char *const fname[F_COUNT] = { "wcsftime_l", "wcsftime" };

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int nprinted[F_COUNT];
static bool warming;

/*
 * Destination buffer layout.  wcs points WHEAD elements into the array so
 * that under-runs as well as over-runs of the nominal window are caught; the
 * whole array, guards included, is memcmp()d after every call.
 */
static const size_t WHEAD = 8;
static const size_t WCAP = 64;
static const size_t WTAIL = 16;
static const size_t WTOT = WHEAD + WCAP + WTAIL;
static const unsigned char GUARD = 0x7f;

static const long long SWEEP = 200000;

static locale_t lc_utf8;
static locale_t lc_c;
static locale_t lc_alt;

/* ------------------------------------------------------------------ */

static const uint64_t RNG_SEED = 0xb0153502c0ffee01ULL;
static uint64_t rng_state = RNG_SEED;

static uint64_t
rnd(void)
{
	uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static uint32_t
u32(uint32_t m)
{

	return ((uint32_t)(rnd() % m));
}

/* ------------------------------------------------------------------ */

static void
report(int f, const char *tag, const char *why, size_t maxsize, size_t pv,
    size_t rv, int pe, int re)
{

	nfail[f]++;
	if (nprinted[f]++ < 20)
		std::printf("  FAIL %-10s %-22s %-7s maxsize=%zu "
		    "port=(%zu,%d) ref=(%zu,%d)\n", fname[f], tag, why,
		    maxsize, pv, pe, rv, re);
}

/*
 * Run one case through both implementations and compare everything.  When
 * use_l is false the locale argument is unused: wcsftime() picks the locale
 * up from the thread through __get_locale().
 */
static void
run_case(int f, bool use_l, const wchar_t *fmt, size_t maxsize,
    const struct tm *tm, locale_t loc, bool null_wcs, const char *tag)
{
	wchar_t pbuf[WTOT], rbuf[WTOT];
	size_t pv, rv;
	int pe, re;

	if (!null_wcs && maxsize > WCAP) {
		std::printf("  harness bug: maxsize %zu exceeds WCAP\n",
		    maxsize);
		std::abort();
	}

	std::memset(pbuf, GUARD, sizeof(pbuf));
	std::memset(rbuf, GUARD, sizeof(rbuf));

	errno = 0;
	if (use_l)
		pv = P::wcsftime_l(null_wcs ? nullptr : pbuf + WHEAD, maxsize,
		    fmt, tm, loc);
	else
		pv = P::wcsftime(null_wcs ? nullptr : pbuf + WHEAD, maxsize,
		    fmt, tm);
	pe = errno;

	errno = 0;
	if (use_l)
		rv = ref_wcsftime_l(null_wcs ? nullptr : rbuf + WHEAD, maxsize,
		    fmt, tm, loc);
	else
		rv = ref_wcsftime(null_wcs ? nullptr : rbuf + WHEAD, maxsize,
		    fmt, tm);
	re = errno;

	if (warming)
		return;

	ncase[f]++;
	if (pv != rv) {
		report(f, tag, "ret", maxsize, pv, rv, pe, re);
		return;
	}
	if (pe != re) {
		report(f, tag, "errno", maxsize, pv, rv, pe, re);
		return;
	}
	if (std::memcmp(pbuf, rbuf, sizeof(pbuf)) != 0) {
		report(f, tag, "buffer", maxsize, pv, rv, pe, re);
		return;
	}
}

/* ------------------------------------------------------------------ */

static struct tm
base_tm(void)
{
	struct tm tm;

	std::memset(&tm, 0, sizeof(tm));
	tm.tm_sec = 7;
	tm.tm_min = 8;
	tm.tm_hour = 9;
	tm.tm_mday = 10;
	tm.tm_mon = 10;
	tm.tm_year = 100;
	tm.tm_wday = 3;
	tm.tm_yday = 313;
	tm.tm_isdst = 0;
	tm.tm_gmtoff = 3600;
	tm.tm_zone = "UTC";
	return (tm);
}

/*
 * Hand written cases.  Each is chosen so that some comparison or arithmetic
 * expression in the port sits exactly on a boundary, and both sides of that
 * boundary are covered.
 */
static void
edge_cases(int f, bool use_l, locale_t loc, const char *lctag)
{
	struct tm tm = base_tm();
	struct tm tmz = base_tm();
	struct tm tmbad = base_tm();
	const size_t mbcm = (size_t)MB_CUR_MAX;
	const size_t thresh = SIZE_T_MAX_LOCAL / mbcm;
	char tagbuf[64];
	unsigned c;

	tmz.tm_zone = "\xc3\xa9";	/* valid UTF-8, invalid in C locale */
	tmbad.tm_zone = "\xff\xfe";	/* never a valid multibyte sequence */

#define TAG(s)								\
	(std::snprintf(tagbuf, sizeof(tagbuf), "%s/%s", lctag, (s)), tagbuf)
#define RUN(fmt, ms, tmp, nw, s)					\
	run_case(f, use_l, (fmt), (ms), (tmp), loc, (nw), TAG(s))

	/* Empty format: strftime_l() returns 0, so the error path is taken. */
	RUN(L"", 0, &tm, false, "empty-0");
	RUN(L"", 1, &tm, false, "empty-1");
	RUN(L"", 8, &tm, false, "empty-8");
	RUN(L"", 64, &tm, false, "empty-64");

	/* One character, straddling the strftime() size boundary. */
	RUN(L"x", 0, &tm, false, "x-0");
	RUN(L"x", 1, &tm, false, "x-1");	/* no room for NUL -> 0 */
	RUN(L"x", 2, &tm, false, "x-2");	/* exactly fits -> 1 */
	RUN(L"x", 3, &tm, false, "x-3");

	/* Two characters: same boundary, one element further out. */
	RUN(L"ab", 2, &tm, false, "ab-2");
	RUN(L"ab", 3, &tm, false, "ab-3");
	RUN(L"ab", 4, &tm, false, "ab-4");

	/* NUL-heavy formats. */
	RUN(L"\0", 8, &tm, false, "nul-8");
	RUN(L"a\0b", 8, &tm, false, "nul-embedded");
	RUN(L"\0", 1, &tm, false, "nul-1");
	RUN(L"\0", 0, &tm, false, "nul-0");

	/* High-bit values 0x80..0xff as wide characters. */
	for (c = 0x80; c <= 0xff; c++) {
		wchar_t hb[2];

		hb[0] = (wchar_t)c;
		hb[1] = L'\0';
		RUN(hb, 0, &tm, false, "hi-0");
		RUN(hb, 1, &tm, false, "hi-1");
		RUN(hb, 2, &tm, false, "hi-2");
		RUN(hb, 3, &tm, false, "hi-3");
		RUN(hb, 4, &tm, false, "hi-4");
		RUN(hb, 32, &tm, false, "hi-32");
		RUN(hb, 8, &tm, true, "hi-null");
	}

	/* Multi-byte wide characters: sflen exceeds the wide length. */
	RUN(L"\u00e9", 2, &tm, false, "u00e9-2");
	RUN(L"\u00e9", 3, &tm, false, "u00e9-3");
	RUN(L"\u07ff", 4, &tm, false, "u07ff-4");
	RUN(L"\u0800", 4, &tm, false, "u0800-4");
	RUN(L"\uffff", 4, &tm, false, "uffff-4");
	RUN(L"\U0010ffff", 5, &tm, false, "u10ffff-5");
	RUN(L"\u00e9\u00e9\u00e9", 8, &tm, false, "u00e9x3-8");
	RUN(L"\u00e9\u00e9\u00e9", 7, &tm, false, "u00e9x3-7");

	/* wcsrtombs_l() failure: sflen == (size_t)-1, error with EILSEQ. */
	{
		wchar_t bad[3];

		bad[0] = (wchar_t)0xd800;		/* lone surrogate */
		bad[1] = L'\0';
		RUN(bad, 16, &tm, false, "surrogate");
		bad[0] = L'a';
		bad[1] = (wchar_t)0x110000;		/* beyond Unicode */
		bad[2] = L'\0';
		RUN(bad, 16, &tm, false, "beyond-unicode");
		bad[0] = (wchar_t)0x7fffffff;
		bad[1] = L'\0';
		RUN(bad, 16, &tm, false, "wchar-max");
		RUN(bad, 0, &tm, false, "wchar-max-0");
	}

	/*
	 * mbsrtowcs_l() failure: n == (size_t)-1 even though strftime_l()
	 * succeeded, with wcs already partly written.
	 */
	RUN(L"%Z", 8, &tmbad, false, "badzone-8");
	RUN(L"%Z", 4, &tmbad, false, "badzone-4");
	RUN(L"%Z", 3, &tmbad, false, "badzone-3");
	RUN(L"ab%Zcd", 16, &tmbad, false, "badzone-mid");
	RUN(L"%Z", 8, &tmz, false, "utf8zone-8");
	RUN(L"ab%Zcd", 16, &tmz, false, "utf8zone-mid");

	/*
	 * wcs == NULL: mbsrtowcs_l() leaves dstp non-NULL, so the third
	 * disjunct of the error test fires with n perfectly valid.
	 */
	RUN(L"abc", 8, &tm, true, "nullwcs-abc");
	RUN(L"", 8, &tm, true, "nullwcs-empty");
	RUN(L"%Y", 8, &tm, true, "nullwcs-Y");
	RUN(L"%Z", 8, &tmbad, true, "nullwcs-badzone");
	RUN(L"x", 0, &tm, true, "nullwcs-0");
	RUN(L"x", 1, &tm, true, "nullwcs-1");
	RUN(L"x", 2, &tm, true, "nullwcs-2");

	/* Conversion specifiers with an exact-fit maxsize on either side. */
	RUN(L"%Y", 4, &tm, false, "Y-4");
	RUN(L"%Y", 5, &tm, false, "Y-5");
	RUN(L"%Y", 6, &tm, false, "Y-6");
	RUN(L"%H:%M:%S", 8, &tm, false, "hms-8");
	RUN(L"%H:%M:%S", 9, &tm, false, "hms-9");
	RUN(L"%H:%M:%S", 10, &tm, false, "hms-10");
	RUN(L"%%", 2, &tm, false, "pct-2");
	RUN(L"%%", 3, &tm, false, "pct-3");
	RUN(L"%n%t", 3, &tm, false, "nt-3");
	RUN(L"%c", 64, &tm, false, "c-64");
	RUN(L"%x %X", 64, &tm, false, "xX-64");
	RUN(L"%A %B", 64, &tm, false, "AB-64");
	RUN(L"%z", 8, &tm, false, "z-8");

	/* A long literal, so a short-by-one sformat shows up in the output. */
	RUN(L"abcdefghijklmnopqrstuvwxyz0123", 30, &tm, false, "long-30");
	RUN(L"abcdefghijklmnopqrstuvwxyz0123", 31, &tm, false, "long-31");
	RUN(L"abcdefghijklmnopqrstuvwxyz0123", 32, &tm, false, "long-32");
	RUN(L"abcdefghijklmnopqrstuvwxyz0123", 64, &tm, false, "long-64");

	/*
	 * SIZE_T_MAX / MB_CUR_MAX <= maxsize, both sides of the boundary.
	 * Below the threshold the multiplication is huge but well defined and
	 * malloc() fails with ENOMEM; at or above it the function short
	 * circuits with EINVAL.  The two are told apart by errno alone.
	 */
	RUN(L"x", thresh - 1, &tm, true, "thresh-1");
	RUN(L"x", thresh, &tm, true, "thresh");
	RUN(L"x", thresh + 1, &tm, true, "thresh+1");
	RUN(L"x", SIZE_T_MAX_LOCAL, &tm, true, "sizemax");
	RUN(L"x", SIZE_T_MAX_LOCAL - 1, &tm, true, "sizemax-1");
	RUN(L"x", SIZE_T_MAX_LOCAL / 2, &tm, true, "sizemax/2");
	RUN(L"x", thresh / 2, &tm, true, "thresh/2");
	RUN(L"", thresh, &tm, true, "empty-thresh");
	RUN(L"", thresh - 1, &tm, true, "empty-thresh-1");

	/* Largest maxsize that still fits the caller's buffer. */
	RUN(L"%c", WCAP, &tm, false, "cap");
	RUN(L"%c", WCAP - 1, &tm, false, "cap-1");

#undef RUN
#undef TAG
}

/*
 * A block run under a constrained address space: maxsize is picked so that
 * maxsize * MB_CUR_MAX exceeds the limit while maxsize itself does not, so
 * the multiplication cannot be confused with a smaller expression.
 */
static void
edge_cases_rlimit(int f, bool use_l, locale_t loc)
{
	struct tm tm = base_tm();
	struct rlimit oldl, newl;
	const size_t mbcm = (size_t)MB_CUR_MAX;
	size_t ms;

	if (mbcm < 2)
		return;
	if (getrlimit(RLIMIT_AS, &oldl) != 0)
		return;
	newl = oldl;
	newl.rlim_cur = (rlim_t)1024 * 1024 * 1024;
	if (oldl.rlim_max != RLIM_INFINITY && newl.rlim_cur > oldl.rlim_max)
		return;
	if (oldl.rlim_cur != RLIM_INFINITY && oldl.rlim_cur < newl.rlim_cur)
		return;
	if (setrlimit(RLIMIT_AS, &newl) != 0)
		return;

	for (ms = (size_t)192 * 1024 * 1024; ms <= (size_t)320 * 1024 * 1024;
	    ms += (size_t)64 * 1024 * 1024) {
		run_case(f, use_l, L"abc", ms, &tm, loc, true, "rlimit");
		run_case(f, use_l, L"", ms, &tm, loc, true, "rlimit-empty");
	}

	(void)setrlimit(RLIMIT_AS, &oldl);
}

/* ------------------------------------------------------------------ */

static const char specs[] = "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%";

static const char *const zones[] = {
	nullptr, "UTC", "GMT", "CET", "", "\xc3\xa9", "\xff\xfe", "\x80",
	"abcdefghijklmnop"
};

static wchar_t
rand_wchar(void)
{

	switch (u32(16)) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		return ((wchar_t)(0x20 + u32(0x5f)));	/* ASCII printable */
	case 5:
		return ((wchar_t)(0x01 + u32(0x1f)));	/* ASCII control */
	case 6:
	case 7:
		return ((wchar_t)(0x80 + u32(0x80)));	/* high bit 0x80-0xff */
	case 8:
		return ((wchar_t)(0x100 + u32(0x700)));
	case 9:
		return ((wchar_t)(0x800 + u32(0x800)));
	case 10:
		return ((wchar_t)(0xe000 + u32(0x1900)));
	case 11:
		return ((wchar_t)(0x10000 + u32(0x100)));
	case 12:
		return ((wchar_t)0x10ffff);
	case 13:
		return ((wchar_t)(0xd800 + u32(0x800)));	/* surrogate */
	case 14:
		return ((wchar_t)(0x110000 + u32(0x1000)));	/* invalid */
	default:
		return ((wchar_t)L'%');
	}
}

static void
sweep(int f, bool use_l)
{
	struct tm tm;
	wchar_t fmt[48];
	locale_t loc;
	size_t n, j, maxsize;
	long long i;

	rng_state = RNG_SEED;
	for (i = 0; i < SWEEP; i++) {
		/* Alternate the thread locale so MB_CUR_MAX varies. */
		if ((i & 0x3ff) == 0)
			uselocale((i & 0x400) ? lc_c : lc_utf8);

		switch (u32(4)) {
		case 0:
			loc = lc_c;
			break;
		case 1:
			loc = lc_alt;
			break;
		default:
			loc = lc_utf8;
			break;
		}

		n = u32(24);
		j = 0;
		while (j < n) {
			if (u32(3) == 0 && j + 1 < n) {
				fmt[j++] = L'%';
				fmt[j++] =
				    (wchar_t)specs[u32(sizeof(specs) - 1)];
			} else
				fmt[j++] = rand_wchar();
		}
		fmt[n] = L'\0';

		std::memset(&tm, 0, sizeof(tm));
		tm.tm_sec = (int)u32(61);
		tm.tm_min = (int)u32(60);
		tm.tm_hour = (int)u32(24);
		tm.tm_mday = 1 + (int)u32(31);
		tm.tm_mon = (int)u32(12);
		tm.tm_year = (int)u32(4000) - 1000;
		tm.tm_wday = (int)u32(7);
		tm.tm_yday = (int)u32(366);
		tm.tm_isdst = (int)u32(3) - 1;
		tm.tm_gmtoff = (long)u32(100000) - 50000;
		tm.tm_zone = zones[u32(sizeof(zones) / sizeof(zones[0]))];

		switch (u32(32)) {
		case 0:
			maxsize = SIZE_T_MAX_LOCAL / (size_t)MB_CUR_MAX -
			    (size_t)u32(2);
			break;
		case 1:
			maxsize = SIZE_T_MAX_LOCAL / (size_t)MB_CUR_MAX +
			    (size_t)u32(2);
			break;
		case 2:
			maxsize = SIZE_T_MAX_LOCAL - (size_t)u32(4);
			break;
		default:
			maxsize = (size_t)u32((uint32_t)WCAP + 1);
			break;
		}

		run_case(f, use_l, fmt, maxsize, &tm, loc,
		    maxsize > WCAP || u32(5) == 0, "sweep");
	}
	uselocale(lc_utf8);
}

/* ------------------------------------------------------------------ */

static locale_t
open_locale(const char *const *names)
{
	locale_t l;

	for (; *names != nullptr; names++) {
		l = newlocale(LC_ALL_MASK, *names, (locale_t)0);
		if (l != (locale_t)0)
			return (l);
	}
	return ((locale_t)0);
}

/*
 * Force any one-shot lazy initialisation inside libc (timezone data, locale
 * tables, the first large malloc()) to happen before errno starts being
 * compared, so that the first of the two calls is not special.
 */
static void
warmup(void)
{
	struct tm tm = base_tm();
	struct tm tmbad = base_tm();
	int r, f;

	tmbad.tm_zone = "\xff\xfe";
	warming = true;
	for (r = 0; r < 3; r++) {
		for (f = 0; f < F_COUNT; f++) {
			bool ul = (f == F_WCSFTIME_L);

			uselocale(lc_utf8);
			run_case(f, ul, L"%c %x %X %Z %z %A %B", 64, &tm,
			    lc_utf8, false, "warm");
			run_case(f, ul, L"%c %x %X %Z %z %A %B", 64, &tm,
			    lc_c, false, "warm");
			run_case(f, ul, L"%Z", 8, &tmbad, lc_utf8, false,
			    "warm");
			run_case(f, ul, L"", 8, &tm, lc_utf8, false, "warm");
			run_case(f, ul, L"x", SIZE_T_MAX_LOCAL, &tm, lc_utf8,
			    true, "warm");
			run_case(f, ul, L"x",
			    SIZE_T_MAX_LOCAL / (size_t)MB_CUR_MAX - 1, &tm,
			    lc_utf8, true, "warm");
			uselocale(lc_c);
			run_case(f, ul, L"%c %Z %A", 64, &tm, lc_c, false,
			    "warm");
			run_case(f, ul, L"x",
			    SIZE_T_MAX_LOCAL / (size_t)MB_CUR_MAX - 1, &tm,
			    lc_c, true, "warm");
		}
	}
	uselocale(lc_utf8);
	warming = false;
}

int
main(void)
{
	static const char *const utf8_names[] = {
		"C.utf8", "C.UTF-8", "en_US.UTF-8", "en_US.utf8", nullptr
	};
	static const char *const c_names[] = { "C", "POSIX", nullptr };
	static const char *const alt_names[] = {
		"en_US.ISO-8859-1", "en_US.iso88591", "C.utf8", "C.UTF-8", "C",
		nullptr
	};
	long long total_fail = 0;
	int i;

	tzset();

	lc_utf8 = open_locale(utf8_names);
	lc_c = open_locale(c_names);
	lc_alt = open_locale(alt_names);
	if (lc_utf8 == (locale_t)0 || lc_c == (locale_t)0) {
		std::printf("cannot construct the test locales\n");
		return (1);
	}
	if (lc_alt == (locale_t)0)
		lc_alt = lc_c;

	uselocale(lc_utf8);
	warmup();

	for (i = 0; i < F_COUNT; i++) {
		bool ul = (i == F_WCSFTIME_L);

		uselocale(lc_utf8);
		edge_cases(i, ul, lc_utf8, "utf8");
		edge_cases(i, ul, lc_c, "utf8:C");
		edge_cases_rlimit(i, ul, lc_utf8);

		uselocale(lc_c);
		edge_cases(i, ul, lc_c, "C");
		edge_cases(i, ul, lc_utf8, "C:utf8");

		uselocale(lc_utf8);
		sweep(i, ul);
	}

	std::printf("\n%-14s %14s %14s\n", "function", "cases", "failures");
	std::printf("%-14s %14s %14s\n", "--------------", "--------------",
	    "--------------");
	for (i = 0; i < F_COUNT; i++) {
		std::printf("%-14s %14lld %14lld\n", fname[i], ncase[i],
		    nfail[i]);
		total_fail += nfail[i];
	}
	std::printf("%-14s %14s %14lld\n", "TOTAL", "", total_fail);

	return (total_fail == 0 ? 0 : 1);
}
