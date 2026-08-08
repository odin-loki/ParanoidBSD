/*
 * Differential harness for batch b0041.
 */

import pbsd.lib.libc.locale.b0041;

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::lib_libc_locale::b0041;

#ifndef EOF
#define EOF	(-1)
#endif

using wint_t = decltype(port::btowc(0));

typedef union {
	char		__bytes[128];
	long long	__align;
} ref_mbstate_t;

extern "C" {
typedef struct xlocale *ref_locale_t;

extern struct xlocale ref_global_locale;
extern struct xlocale_ctype ref_global_ctype;

size_t ref_wcstombs_l(char *, const wchar_t *, size_t, ref_locale_t);
size_t ref_wcstombs(char *, const wchar_t *, size_t);
int ref_wctomb_l(char *, wchar_t, ref_locale_t);
int ref_wctomb(char *, wchar_t);
wint_t ref_btowc_l(int, ref_locale_t);
wint_t ref_btowc(int);
int ref_mblen_l(const char *, size_t, ref_locale_t);
int ref_mblen(const char *, size_t);
}

struct xlocale_ctype {
	ref_mbstate_t	mblen;
	ref_mbstate_t	wctomb;
};

struct xlocale {
	void		*components[6];
};

struct Stats {
	const char		*name;
	unsigned long long	cases;
	unsigned long long	fails;
};

static constexpr unsigned char GUARD = 0x7f;
static constexpr size_t OUT_CAP = 64;
static constexpr size_t WCS_CAP = 32;
static constexpr unsigned long long RANDOM_ITERS = 200000;

static Stats st_wcstombs_l = { "wcstombs_l", 0, 0 };
static Stats st_wcstombs = { "wcstombs", 0, 0 };
static Stats st_wctomb_l = { "wctomb_l", 0, 0 };
static Stats st_wctomb = { "wctomb", 0, 0 };
static Stats st_btowc_l = { "btowc_l", 0, 0 };
static Stats st_btowc = { "btowc", 0, 0 };
static Stats st_mblen_l = { "mblen_l", 0, 0 };
static Stats st_mblen = { "mblen", 0, 0 };

static uint32_t rng = 0xB0004001u;

static uint32_t
xorshift32()
{
	uint32_t x = rng;

	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	rng = x;
	return (x);
}

static void
init_locales()
{
	port::init_locale();
	ref_global_locale.components[1] = &ref_global_ctype;
}

static void
fill_guard(unsigned char *buf, size_t n)
{
	memset(buf, GUARD, n);
}

static bool
guards_intact(const unsigned char *buf, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		if (buf[i] != GUARD)
			return (false);
	}
	return (true);
}

static bool
bufs_equal(const unsigned char *a, const unsigned char *b, size_t n)
{
	return (memcmp(a, b, n) == 0);
}

static void
report_fail(Stats &st, const char *tag)
{
	st.fails++;
	if (st.fails <= 5)
		std::printf("FAIL %s [%s]\n", st.name, tag);
}

static bool
compare_wcstombs_l(Stats &st, const wchar_t *pwcs, size_t n, bool use_l,
    bool null_locale)
{
	unsigned char pout[OUT_CAP], rout[OUT_CAP];
	size_t pr, rr;

	st.cases++;
	fill_guard(pout, sizeof(pout));
	fill_guard(rout, sizeof(rout));

	if (use_l) {
		pr = port::wcstombs_l((char *)(pout + 8), pwcs, n,
		    null_locale ? nullptr : port::global_locale());
		rr = ref_wcstombs_l((char *)(rout + 8), pwcs, n,
		    null_locale ? nullptr : &ref_global_locale);
	} else {
		pr = port::wcstombs((char *)(pout + 8), pwcs, n);
		rr = ref_wcstombs((char *)(rout + 8), pwcs, n);
	}

	if (pr != rr) {
		report_fail(st, "ret");
		return (false);
	}
	if (!guards_intact(pout, 8) || !guards_intact(pout + 8 + OUT_CAP - 16,
	    OUT_CAP - 16 - 8)) {
		report_fail(st, "port guard");
		return (false);
	}
	if (!guards_intact(rout, 8) || !guards_intact(rout + 8 + OUT_CAP - 16,
	    OUT_CAP - 16 - 8)) {
		report_fail(st, "ref guard");
		return (false);
	}
	if (!bufs_equal(pout, rout, sizeof(pout))) {
		report_fail(st, "buf");
		return (false);
	}
	return (true);
}

static bool
compare_wctomb_l(Stats &st, char *s, wchar_t wc, bool use_l, bool null_locale)
{
	unsigned char pout[OUT_CAP], rout[OUT_CAP];
	int pr, rr;

	st.cases++;
	if (s != nullptr) {
		fill_guard(pout, sizeof(pout));
		fill_guard(rout, sizeof(rout));
	}

	if (use_l) {
		pr = port::wctomb_l(s != nullptr ? (char *)(pout + 4) : nullptr,
		    wc, null_locale ? nullptr : port::global_locale());
		rr = ref_wctomb_l(s != nullptr ? (char *)(rout + 4) : nullptr,
		    wc, null_locale ? nullptr : &ref_global_locale);
	} else {
		pr = port::wctomb(s != nullptr ? (char *)(pout + 4) : nullptr, wc);
		rr = ref_wctomb(s != nullptr ? (char *)(rout + 4) : nullptr, wc);
	}

	if (pr != rr) {
		report_fail(st, "ret");
		return (false);
	}
	if (s == nullptr)
		return (true);
	if (!guards_intact(pout, 4) || !guards_intact(pout + 4 + 16, OUT_CAP - 20)) {
		report_fail(st, "port guard");
		return (false);
	}
	if (!guards_intact(rout, 4) || !guards_intact(rout + 4 + 16, OUT_CAP - 20)) {
		report_fail(st, "ref guard");
		return (false);
	}
	if (!bufs_equal(pout, rout, sizeof(pout))) {
		report_fail(st, "buf");
		return (false);
	}
	return (true);
}

static bool
compare_btowc_l(Stats &st, int c, bool use_l, bool null_locale)
{
	wint_t pw, rw;

	st.cases++;

	if (use_l) {
		pw = port::btowc_l(c, null_locale ? nullptr : port::global_locale());
		rw = ref_btowc_l(c, null_locale ? nullptr : &ref_global_locale);
	} else {
		pw = port::btowc(c);
		rw = ref_btowc(c);
	}

	if (pw != rw) {
		report_fail(st, "ret");
		return (false);
	}
	return (true);
}

static bool
compare_mblen_l(Stats &st, const char *s, size_t n, bool use_l,
    bool null_locale)
{
	int pr, rr;

	st.cases++;

	if (use_l) {
		pr = port::mblen_l(s, n, null_locale ? nullptr : port::global_locale());
		rr = ref_mblen_l(s, n, null_locale ? nullptr : &ref_global_locale);
	} else {
		pr = port::mblen(s, n);
		rr = ref_mblen(s, n);
	}

	if (pr != rr) {
		report_fail(st, "ret");
		return (false);
	}
	return (true);
}

static void
hand_wcstombs(Stats &st, bool use_l)
{
	static const wchar_t empty[] = { L'\0' };
	static const wchar_t one[] = { L'a', L'\0' };
	static const wchar_t two[] = { L'a', L'b', L'\0' };
	static const wchar_t nul_mid[] = { L'a', L'\0', L'b', L'\0' };
	static const wchar_t euro[] = { 0x20ac, L'\0' };
	static const wchar_t plane[] = { 0x10000, L'\0' };
	static const wchar_t bad[] = { 0xd800, L'\0' };
	static const wchar_t mix[] = { L'A', 0x00a2, 0x20ac, 0x10348, L'\0' };

	compare_wcstombs_l(st, empty, 0, use_l, false);
	compare_wcstombs_l(st, empty, 1, use_l, false);
	compare_wcstombs_l(st, empty, OUT_CAP, use_l, false);
	compare_wcstombs_l(st, one, 0, use_l, false);
	compare_wcstombs_l(st, one, 1, use_l, false);
	compare_wcstombs_l(st, one, 2, use_l, false);
	compare_wcstombs_l(st, two, 1, use_l, false);
	compare_wcstombs_l(st, two, 2, use_l, false);
	compare_wcstombs_l(st, two, 3, use_l, false);
	compare_wcstombs_l(st, nul_mid, 4, use_l, false);
	compare_wcstombs_l(st, euro, 2, use_l, false);
	compare_wcstombs_l(st, euro, 4, use_l, false);
	compare_wcstombs_l(st, plane, 5, use_l, false);
	compare_wcstombs_l(st, bad, 4, use_l, false);
	compare_wcstombs_l(st, mix, 16, use_l, false);
	compare_wcstombs_l(st, mix, 3, use_l, false);
	compare_wcstombs_l(st, one, 2, use_l, true);
}

static void
hand_wctomb(Stats &st, bool use_l)
{
	char dummy = 0;

	compare_wctomb_l(st, nullptr, L'x', use_l, false);
	compare_wctomb_l(st, &dummy, L'\0', use_l, false);
	compare_wctomb_l(st, &dummy, L'a', use_l, false);
	compare_wctomb_l(st, &dummy, L'\x7f', use_l, false);
	compare_wctomb_l(st, &dummy, (wchar_t)0x80, use_l, false);
	compare_wctomb_l(st, &dummy, (wchar_t)0x7ff, use_l, false);
	compare_wctomb_l(st, &dummy, (wchar_t)0x800, use_l, false);
	compare_wctomb_l(st, &dummy, (wchar_t)0xffff, use_l, false);
	compare_wctomb_l(st, &dummy, (wchar_t)0x10000, use_l, false);
	compare_wctomb_l(st, &dummy, (wchar_t)0x10ffff, use_l, false);
	compare_wctomb_l(st, &dummy, (wchar_t)0xd800, use_l, false);
	compare_wctomb_l(st, &dummy, (wchar_t)0xdfff, use_l, false);
	compare_wctomb_l(st, &dummy, (wchar_t)-1, use_l, false);
	compare_wctomb_l(st, nullptr, L'z', use_l, true);
	compare_wctomb_l(st, &dummy, L'q', use_l, true);
}

static void
hand_btowc(Stats &st, bool use_l)
{
	compare_btowc_l(st, EOF, use_l, false);
	compare_btowc_l(st, 0, use_l, false);
	compare_btowc_l(st, 'a', use_l, false);
	compare_btowc_l(st, 0x7f, use_l, false);
	compare_btowc_l(st, 0x80, use_l, false);
	compare_btowc_l(st, 0xc2, use_l, false);
	compare_btowc_l(st, 0xff, use_l, false);
	compare_btowc_l(st, -1, use_l, false);
	compare_btowc_l(st, 256, use_l, false);
	compare_btowc_l(st, 0x100, use_l, false);
	compare_btowc_l(st, EOF, use_l, true);
	compare_btowc_l(st, 0xe2, use_l, false);
}

static void
hand_mblen(Stats &st, bool use_l)
{
	static const char empty[] = "";
	static const char one[] = "a";
	static const char two[] = "ab";
	static const char euro[] = "\xe2\x82\xac";
	static const char plane[] = "\xf0\x90\x8d\x88";
	static const char bad[] = "\xff";
	static const char overlong[] = "\xc0\x80";

	compare_mblen_l(st, nullptr, 0, use_l, false);
	compare_mblen_l(st, empty, 0, use_l, false);
	compare_mblen_l(st, empty, 1, use_l, false);
	compare_mblen_l(st, one, 0, use_l, false);
	compare_mblen_l(st, one, 1, use_l, false);
	compare_mblen_l(st, one, 2, use_l, false);
	compare_mblen_l(st, two, 1, use_l, false);
	compare_mblen_l(st, two, 2, use_l, false);
	compare_mblen_l(st, euro, 1, use_l, false);
	compare_mblen_l(st, euro, 2, use_l, false);
	compare_mblen_l(st, euro, 3, use_l, false);
	compare_mblen_l(st, euro, 4, use_l, false);
	compare_mblen_l(st, plane, 1, use_l, false);
	compare_mblen_l(st, plane, 4, use_l, false);
	compare_mblen_l(st, bad, 1, use_l, false);
	compare_mblen_l(st, overlong, 1, use_l, false);
	compare_mblen_l(st, overlong, 2, use_l, false);
	compare_mblen_l(st, nullptr, 5, use_l, true);
	compare_mblen_l(st, one, 1, use_l, true);
}

static void
random_wcstombs(Stats &st, bool use_l)
{
	wchar_t wcs[WCS_CAP];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		size_t len = (size_t)(xorshift32() % WCS_CAP);
		size_t n = (size_t)(xorshift32() % OUT_CAP);

		for (size_t j = 0; j < len; j++) {
			uint32_t r = xorshift32();
			if ((r & 0xfu) == 0)
				wcs[j] = L'\0';
			else if ((r & 0x20u) == 0)
				wcs[j] = (wchar_t)(r & 0x7fu);
			else
				wcs[j] = (wchar_t)(r & 0x10ffffu);
		}
		wcs[len - 1] = L'\0';
		if (!compare_wcstombs_l(st, wcs, n, use_l,
		    (xorshift32() & 0x3ffu) == 0))
			return;
	}
}

static void
random_wctomb(Stats &st, bool use_l)
{
	char dummy = 0;

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		wchar_t wc = (wchar_t)(xorshift32() & 0x1fffffu);
		bool reset = (xorshift32() & 0x1ffu) == 0;

		if (reset) {
			if (!compare_wctomb_l(st, nullptr, wc, use_l,
			    (xorshift32() & 0x3ffu) == 0))
				return;
		}
		if (!compare_wctomb_l(st, &dummy, wc, use_l,
		    (xorshift32() & 0x7ffu) == 0))
			return;
	}
}

static void
random_btowc(Stats &st, bool use_l)
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		int c;

		if ((xorshift32() & 0xffu) == 0)
			c = EOF;
		else
			c = (int)(xorshift32() & 0x1ffu) - 1;
		if (!compare_btowc_l(st, c, use_l, (xorshift32() & 0x3ffu) == 0))
			return;
	}
}

static void
random_mblen(Stats &st, bool use_l)
{
	unsigned char seq[16];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		size_t len = (size_t)(xorshift32() % (sizeof(seq) + 1));
		size_t n = (size_t)(xorshift32() % (sizeof(seq) + 1));

		if ((xorshift32() & 0x1ffu) == 0) {
			if (!compare_mblen_l(st, nullptr, n, use_l,
			    (xorshift32() & 0x3ffu) == 0))
				return;
			continue;
		}
		for (size_t j = 0; j < len; j++)
			seq[j] = (unsigned char)(xorshift32() & 0xffu);
		if (!compare_mblen_l(st, (const char *)seq, n, use_l,
		    (xorshift32() & 0x7ffu) == 0))
			return;
	}
}

static void
print_row(const Stats &st)
{
	std::printf("%-16s %12llu %12llu\n", st.name, st.cases, st.fails);
}

int
main()
{
	init_locales();

	hand_wcstombs(st_wcstombs_l, true);
	hand_wcstombs(st_wcstombs, false);
	hand_wctomb(st_wctomb_l, true);
	hand_wctomb(st_wctomb, false);
	hand_btowc(st_btowc_l, true);
	hand_btowc(st_btowc, false);
	hand_mblen(st_mblen_l, true);
	hand_mblen(st_mblen, false);

	random_wcstombs(st_wcstombs_l, true);
	random_wcstombs(st_wcstombs, false);
	random_wctomb(st_wctomb_l, true);
	random_wctomb(st_wctomb, false);
	random_btowc(st_btowc_l, true);
	random_btowc(st_btowc, false);
	random_mblen(st_mblen_l, true);
	random_mblen(st_mblen, false);

	std::printf("\n%-16s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-16s %12s %12s\n", "--------", "-----", "--------");
	print_row(st_wcstombs_l);
	print_row(st_wcstombs);
	print_row(st_wctomb_l);
	print_row(st_wctomb);
	print_row(st_btowc_l);
	print_row(st_btowc);
	print_row(st_mblen_l);
	print_row(st_mblen);

	if (st_wcstombs_l.fails != 0 || st_wcstombs.fails != 0 ||
	    st_wctomb_l.fails != 0 || st_wctomb.fails != 0 ||
	    st_btowc_l.fails != 0 || st_btowc.fails != 0 ||
	    st_mblen_l.fails != 0 || st_mblen.fails != 0)
		return (1);
	return (0);
}
