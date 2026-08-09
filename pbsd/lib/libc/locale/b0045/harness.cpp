/*
 * Differential harness for batch b0045 (wcstol, wcstoull, wcstoimax, wcstoll
 * and their locale_t variants).
 *
 * Every case runs the C++23 port and the C oracle on two independent
 * guard-filled wchar buffers, then compares return values, errno, endptr
 * offsets (never raw addresses) and the entire buffer contents.
 */

#include <climits>
#include <cerrno>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <locale.h>

import pbsd.lib.libc.locale.b0045;

namespace P = pbsd::lib_libc_locale::b0045;

extern "C" {
long ref_wcstol(const wchar_t *, wchar_t **, int);
long ref_wcstol_l(const wchar_t *, wchar_t **, int, locale_t);
unsigned long long ref_wcstoull(const wchar_t *, wchar_t **, int);
unsigned long long ref_wcstoull_l(const wchar_t *, wchar_t **, int,
    locale_t);
intmax_t ref_wcstoimax(const wchar_t *, wchar_t **, int);
intmax_t ref_wcstoimax_l(const wchar_t *, wchar_t **, int, locale_t);
long long ref_wcstoll(const wchar_t *, wchar_t **, int);
long long ref_wcstoll_l(const wchar_t *, wchar_t **, int, locale_t);
}

enum {
	F_WCSTOL, F_WCSTOL_L,
	F_WCSTOULL, F_WCSTOULL_L,
	F_WCSTOIMAX, F_WCSTOIMAX_L,
	F_WCSTOLL, F_WCSTOLL_L,
	F_COUNT
};

static const char *const fn_name[F_COUNT] = {
	"wcstol", "wcstol_l",
	"wcstoull", "wcstoull_l",
	"wcstoimax", "wcstoimax_l",
	"wcstoll", "wcstoll_l"
};

static unsigned long long n_cases[F_COUNT];
static unsigned long long n_fails[F_COUNT];

static constexpr size_t BUFSZ = 160;
static constexpr wchar_t GUARD = (wchar_t)0x7f;
static constexpr unsigned long long RANDOM_ITERS = 200000;

static locale_t loc_c;
static locale_t loc_posix;
static unsigned long long case_seq;

/* ------------------------------------------------------------------ rng */

static uint64_t rng_state = 0xB00045ULL;

static uint64_t
rnd(void)
{
	uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static uint32_t
rnd_below(uint32_t n)
{
	return (uint32_t)(rnd() % (uint64_t)n);
}

/* ------------------------------------------------------------- reporting */

static unsigned long long reported;

static void
show_wchars(const wchar_t *w, size_t n)
{
	size_t i;

	std::fputc('L', stdout);
	std::fputc('"', stdout);
	for (i = 0; i < n; i++) {
		wchar_t c = w[i];

		if (c >= 0x20 && c < 0x7f && c != L'\\' && c != L'"')
			std::fputc((int)c, stdout);
		else
			std::printf("\\x%04x", (unsigned)(uint32_t)c);
	}
	std::fputc('"', stdout);
}

static void
report(int fn, const wchar_t *w, size_t n, int base, const char *what,
    const char *lname, unsigned long long pv, unsigned long long rv)
{
	if (reported >= 40u) {
		if (reported == 40u)
			std::printf("  ... further mismatches suppressed\n");
		reported++;
		return;
	}
	reported++;
	std::printf("  MISMATCH %s [%s] base=%d locale=%s input=",
	    fn_name[fn], what, base, lname);
	show_wchars(w, n);
	std::printf(" len=%zu port=0x%016llx ref=0x%016llx\n", n, pv, rv);
}

static void
bump_fail(int fn)
{
	n_fails[fn]++;
}

/* ----------------------------------------------------------- call helpers */

struct Ret {
	unsigned long long bits;
	int err;
};

static Ret
call_port(int fn, const wchar_t *s, wchar_t **ep, int base, locale_t loc)
{
	Ret r;

	errno = 0;
	switch (fn) {
	case F_WCSTOL:
		r.bits = (unsigned long long)(unsigned long)P::wcstol(s, ep, base);
		break;
	case F_WCSTOL_L:
		r.bits = (unsigned long long)(unsigned long)P::wcstol_l(s, ep,
		    base, loc);
		break;
	case F_WCSTOULL:
		r.bits = (unsigned long long)P::wcstoull(s, ep, base);
		break;
	case F_WCSTOULL_L:
		r.bits = (unsigned long long)P::wcstoull_l(s, ep, base, loc);
		break;
	case F_WCSTOIMAX:
		r.bits = (unsigned long long)(uintmax_t)P::wcstoimax(s, ep, base);
		break;
	case F_WCSTOIMAX_L:
		r.bits = (unsigned long long)(uintmax_t)P::wcstoimax_l(s, ep,
		    base, loc);
		break;
	case F_WCSTOLL:
		r.bits = (unsigned long long)(unsigned long long)P::wcstoll(s,
		    ep, base);
		break;
	case F_WCSTOLL_L:
		r.bits = (unsigned long long)(unsigned long long)P::wcstoll_l(s,
		    ep, base, loc);
		break;
	default:
		r.bits = 0;
		break;
	}
	r.err = errno;
	return r;
}

static Ret
call_ref(int fn, const wchar_t *s, wchar_t **ep, int base, locale_t loc)
{
	Ret r;

	errno = 0;
	switch (fn) {
	case F_WCSTOL:
		r.bits = (unsigned long long)(unsigned long)ref_wcstol(s, ep,
		    base);
		break;
	case F_WCSTOL_L:
		r.bits = (unsigned long long)(unsigned long)ref_wcstol_l(s, ep,
		    base, loc);
		break;
	case F_WCSTOULL:
		r.bits = (unsigned long long)ref_wcstoull(s, ep, base);
		break;
	case F_WCSTOULL_L:
		r.bits = (unsigned long long)ref_wcstoull_l(s, ep, base, loc);
		break;
	case F_WCSTOIMAX:
		r.bits = (unsigned long long)(uintmax_t)ref_wcstoimax(s, ep,
		    base);
		break;
	case F_WCSTOIMAX_L:
		r.bits = (unsigned long long)(uintmax_t)ref_wcstoimax_l(s, ep,
		    base, loc);
		break;
	case F_WCSTOLL:
		r.bits = (unsigned long long)(unsigned long long)ref_wcstoll(s,
		    ep, base);
		break;
	case F_WCSTOLL_L:
		r.bits = (unsigned long long)(unsigned long long)ref_wcstoll_l(s,
		    ep, base, loc);
		break;
	default:
		r.bits = 0;
		break;
	}
	r.err = errno;
	return r;
}

/* -------------------------------------------------------------- checking */

static bool
check_one(int fn, const wchar_t *in, size_t len, int base, bool use_endptr,
    locale_t loc, const char *lname)
{
	wchar_t bufp[BUFSZ], bufr[BUFSZ], pristine[BUFSZ];
	wchar_t *epp, *epr;
	Ret rp, rr;
	long offp, offr;
	bool ok = true;

	if (len > BUFSZ - 2) {
		std::printf("internal: input too long (%zu)\n", len);
		std::exit(2);
	}

	std::wmemset(bufp, GUARD, BUFSZ);
	std::wmemcpy(bufp, in, len);
	bufp[len] = L'\0';
	std::wmemcpy(bufr, bufp, BUFSZ);
	std::wmemcpy(pristine, bufp, BUFSZ);

	epp = bufp + BUFSZ + 13;
	epr = bufr + BUFSZ + 13;

	rp = call_port(fn, bufp, use_endptr ? &epp : (wchar_t **)0, base, loc);
	rr = call_ref(fn, bufr, use_endptr ? &epr : (wchar_t **)0, base, loc);

	n_cases[fn]++;

	if (rp.bits != rr.bits) {
		report(fn, in, len, base, "return", lname, rp.bits, rr.bits);
		ok = false;
	}
	if (rp.err != rr.err) {
		report(fn, in, len, base, "errno", lname,
		    (unsigned long long)(unsigned)rp.err,
		    (unsigned long long)(unsigned)rr.err);
		ok = false;
	}
	if (use_endptr) {
		offp = (long)(epp - bufp);
		offr = (long)(epr - bufr);
		if (offp != offr) {
			report(fn, in, len, base, "endptr", lname,
			    (unsigned long long)(unsigned long)offp,
			    (unsigned long long)(unsigned long)offr);
			ok = false;
		}
	}
	if (std::wmemcmp(bufp, bufr, BUFSZ) != 0) {
		report(fn, in, len, base, "buffers-differ", lname, 0, 0);
		ok = false;
	}
	if (std::wmemcmp(bufp, pristine, BUFSZ) != 0) {
		report(fn, in, len, base, "port-wrote-buffer", lname, 0, 0);
		ok = false;
	}
	if (std::wmemcmp(bufr, pristine, BUFSZ) != 0) {
		report(fn, in, len, base, "ref-wrote-buffer", lname, 0, 0);
		ok = false;
	}
	if (!ok)
		bump_fail(fn);
	return ok;
}

static void
check_all(const wchar_t *in, size_t len, int base)
{
	locale_t loc;
	const char *lname;
	int fn;

	if ((case_seq++ & 1u) != 0u) {
		loc = loc_c;
		lname = "C";
	} else {
		loc = loc_posix;
		lname = "POSIX";
	}

	for (fn = 0; fn < F_COUNT; fn++) {
		bool is_l = (fn == F_WCSTOL_L || fn == F_WCSTOULL_L ||
		    fn == F_WCSTOIMAX_L || fn == F_WCSTOLL_L);

		check_one(fn, in, len, base, true,
		    is_l ? loc : (locale_t)0, is_l ? lname : "n/a");
		check_one(fn, in, len, base, false,
		    is_l ? loc : (locale_t)0, is_l ? lname : "n/a");
		if (is_l) {
			check_one(fn, in, len, base, true, (locale_t)0,
			    "NULL");
			check_one(fn, in, len, base, false, (locale_t)0,
			    "NULL");
		}
	}
}

static void
check_wcs(const wchar_t *s, int base)
{
	check_all(s, std::wcslen(s), base);
}

static void
check_wcs_n(const wchar_t *w, size_t n, int base)
{
	check_all(w, n, base);
}

/* ---------------------------------------------------------- edge cases */

struct WLit {
	const wchar_t *w;
	size_t n;
};

#define WL(s) { L##s, (sizeof(L##s) / sizeof(wchar_t)) - 1 }

static const WLit edge_wlits[] = {
	/* empty and whitespace */
	WL(""), WL(" "), WL("\t"), WL("\n"), WL("\v"), WL("\f"), WL("\r"),
	WL("   "), WL("\t\n\v\f\r"),

	/* signs and lone characters */
	WL("-"), WL("+"), WL("0"), WL("1"), WL("9"), WL("a"), WL("A"),
	WL("z"), WL("Z"), WL("g"), WL("G"), WL("x"), WL("X"), WL("b"), WL("B"),

	/* plain decimal */
	WL("0"), WL("-0"), WL("+0"), WL("00"), WL("007"), WL("09"),
	WL("42"), WL("-42"), WL("+42"), WL("  42"), WL("\t\n 42"),
	WL("- 42"), WL("+ 42"), WL("--42"), WL("++42"), WL("42abc"),

	/* auto base / hex / octal / binary */
	WL("0x10"), WL("0X10"), WL("-0x10"), WL("0x"), WL("0X"), WL("0xg"),
	WL("010"), WL("-010"), WL("0b101"), WL("0B1"), WL("08"), WL("0777"),
	WL("0xff"), WL("0XFF"), WL("0x7fffffff"), WL("0xffffffff"),
	WL("0x7fffffffffffffff"), WL("0x8000000000000000"),
	WL("0xffffffffffffffff"),

	/* explicit-base forms */
	WL("z"), WL("Z"), WL("10"), WL("1010"), WL("deadbeef"),
	WL("DEADBEEF"), WL("deadBEEF"),

	/* long boundaries */
	WL("2147483646"), WL("2147483647"), WL("2147483648"), WL("2147483649"),
	WL("-2147483647"), WL("-2147483648"), WL("-2147483649"),
	WL("9223372036854775806"), WL("9223372036854775807"),
	WL("9223372036854775808"), WL("9223372036854775809"),
	WL("-9223372036854775807"), WL("-9223372036854775808"),
	WL("-9223372036854775809"),
	WL("18446744073709551614"), WL("18446744073709551615"),
	WL("18446744073709551616"), WL("18446744073709551617"),

	/* far out of range */
	WL("99999999999999999999999999999999"),
	WL("-99999999999999999999999999999999"),
	WL("100000000000000000000000000000000000000000"),

	/* invalid bases exercised via check_bases loop */
	WL("123"), WL("abc"), WL("0x"), WL("0b"), WL("0b2"),
};

static const int bases[] = {
	0, 2, 8, 10, 16,
	1, 37, -1, 100, 36, 35, 3, 24
};

static void
run_edge_cases(void)
{
	size_t i, bi;
	wchar_t buf[BUFSZ];
	unsigned v;

	for (bi = 0; bi < sizeof(bases) / sizeof(bases[0]); bi++) {
		for (i = 0; i < sizeof(edge_wlits) / sizeof(edge_wlits[0]); i++)
			check_all(edge_wlits[i].w, edge_wlits[i].n, bases[bi]);
	}

	/* every wchar value alone */
	for (v = 0; v <= 0xffu; v++) {
		buf[0] = (wchar_t)v;
		buf[1] = L'\0';
		check_wcs(buf, 0);
		check_wcs(buf, 2);
		check_wcs(buf, 8);
		check_wcs(buf, 10);
		check_wcs(buf, 16);
	}

	/* high-bit wchar adjacent to digit runs */
	for (v = 0x80; v <= 0xffu; v++) {
		buf[0] = (wchar_t)v;
		buf[1] = L'4';
		buf[2] = L'2';
		buf[3] = L'\0';
		check_wcs(buf, 0);

		buf[0] = L'4';
		buf[1] = (wchar_t)v;
		buf[2] = L'2';
		buf[3] = L'\0';
		check_wcs(buf, 0);

		buf[0] = L'-';
		buf[1] = (wchar_t)v;
		buf[2] = L'7';
		buf[3] = L'\0';
		check_wcs(buf, 0);

		buf[0] = L'0';
		buf[1] = L'x';
		buf[2] = (wchar_t)v;
		buf[3] = L'f';
		buf[4] = L'\0';
		check_wcs(buf, 0);
		check_wcs(buf, 16);
	}

	/* NUL-heavy (embedded NUL terminates parse) */
	{
		const wchar_t n0[] = { L'\0', 0 };
		const wchar_t n1[] = { L'\0', L'4', L'2', 0 };
		const wchar_t n2[] = { L'4', L'\0', L'2', 0 };
		const wchar_t n3[] = { L'4', L'2', L'\0', 0 };
		const wchar_t n4[] = { L' ', L'\0', L'4', L'2', 0 };
		const wchar_t n5[] = { L'-', L'\0', L'4', L'2', 0 };
		const wchar_t n6[] = { L'0', L'x', L'\0', L'1', L'0', 0 };
		const wchar_t n7[] = { (wchar_t)0x80, L'\0', L'1', 0 };

		check_wcs_n(n0, 1, 0);
		check_wcs_n(n1, 3, 0);
		check_wcs_n(n2, 3, 0);
		check_wcs_n(n3, 3, 0);
		check_wcs_n(n4, 4, 0);
		check_wcs_n(n5, 4, 0);
		check_wcs_n(n6, 5, 0);
		check_wcs_n(n6, 5, 16);
		check_wcs_n(n7, 3, 0);
	}

	/* boundary lengths: runs of digits and zeros */
	for (v = 0; v <= 48u; v++) {
		size_t k;

		for (k = 0; k < v; k++)
			buf[k] = L'9';
		buf[v] = L'\0';
		check_wcs(buf, 0);
		check_wcs(buf, 10);
		check_wcs(buf, 16);

		for (k = 0; k < v; k++)
			buf[k] = L'0';
		buf[v] = L'\0';
		check_wcs(buf, 0);
		check_wcs(buf, 8);

		buf[0] = L'1';
		for (k = 1; k <= v; k++)
			buf[k] = L'0';
		buf[v + 1] = L'\0';
		check_wcs(buf, 0);

		buf[0] = L'-';
		buf[1] = L'1';
		for (k = 2; k <= v + 1; k++)
			buf[k] = L'0';
		buf[v + 2] = L'\0';
		check_wcs(buf, 0);

		for (k = 0; k < v; k++)
			buf[k] = L' ';
		buf[v] = L'7';
		buf[v + 1] = L'\0';
		check_wcs(buf, 0);
	}

	/* letter cases for base 36 and wcstoimax 'a'/'z' branch */
	for (v = 0; v < 26u; v++) {
		buf[0] = (wchar_t)(L'a' + (wchar_t)v);
		buf[1] = L'\0';
		check_wcs(buf, 0);
		check_wcs(buf, 10);
		check_wcs(buf, 36);

		buf[0] = (wchar_t)(L'A' + (wchar_t)v);
		buf[1] = L'\0';
		check_wcs(buf, 0);
		check_wcs(buf, 36);
	}

	/* cutlim boundary probes for base 10 */
	{
		static const wchar_t *const lims[] = {
			L"922337203685477580",
			L"9223372036854775807",
			L"9223372036854775808",
			L"18446744073709551615",
			L"18446744073709551616",
			L"9223372036854775805",
			L"9223372036854775806",
			L"-9223372036854775808",
			L"-9223372036854775809",
		};
		for (i = 0; i < sizeof(lims) / sizeof(lims[0]); i++) {
			check_wcs(lims[i], 0);
			check_wcs(lims[i], 10);
			check_wcs(lims[i], 16);
		}
	}
}

/* -------------------------------------------------------- random sweep */

static const wchar_t soup_alpha[] =
	L"0123456789" L"0123456789" L"0123456789"
	L"-+" L"xXbB" L"abcdefABCDEF" L" \t\n\v\f\r" L" \t"
	L"0123456789";

static size_t
gen_soup(wchar_t *buf)
{
	size_t n = (size_t)rnd_below(28u);
	size_t i;

	for (i = 0; i < n; i++) {
		unsigned pick = rnd_below(16u);

		if (pick == 0u)
			buf[i] = (wchar_t)rnd_below(512u);
		else if (pick == 1u) {
			static const wchar_t hi[] = {
				0x00, 0x7f, 0x80, 0x81, 0x85, 0xa0, 0xff,
				0x100, 0x7fff, 0x8000, 0xffff
			};
			buf[i] = hi[rnd_below((unsigned)(sizeof(hi) /
			    sizeof(hi[0])))];
		} else
			buf[i] = soup_alpha[rnd_below(
			    (unsigned)(sizeof(soup_alpha) / sizeof(wchar_t) -
			    1u))];
	}
	return n;
}

static size_t
fmt_u128_w(wchar_t *out, unsigned __int128 v)
{
	wchar_t tmp[64];
	size_t n = 0, i;

	if (v == 0) {
		out[0] = L'0';
		return 1;
	}
	while (v != 0) {
		tmp[n++] = (wchar_t)(L'0' + (int)(unsigned)(v % 10u));
		v /= 10u;
	}
	for (i = 0; i < n; i++)
		out[i] = tmp[n - 1u - i];
	return n;
}

static const unsigned __int128 bounds[] = {
	0u, 1u, 9u, 10u, 127u, 128u, 255u,
	2147483647u, 2147483648u,
	4294967295u, 4294967296u,
	(unsigned __int128)9223372036854775807ULL,
	(unsigned __int128)9223372036854775807ULL + 1u,
	(unsigned __int128)18446744073709551615ULL,
	(unsigned __int128)18446744073709551615ULL + 1u,
};

static size_t
gen_boundary(wchar_t *buf)
{
	unsigned __int128 base = bounds[rnd_below(
	    (unsigned)(sizeof(bounds) / sizeof(bounds[0])))];
	int delta = (int)rnd_below(9u) - 4;
	size_t n = 0;
	unsigned k, zeros, ws;

	if (delta < 0) {
		unsigned __int128 mag = (unsigned __int128)(-delta);

		if (base >= mag)
			base -= mag;
		else
			base = 0u;
	} else
		base += (unsigned __int128)delta;

	ws = rnd_below(4u);
	for (k = 0; k < ws; k++) {
		static const wchar_t sp[] = L" \t\n\v\f\r";
		buf[n++] = sp[rnd_below((unsigned)(sizeof(sp) /
		    sizeof(wchar_t) - 1u))];
	}
	switch (rnd_below(4u)) {
	case 0:
		buf[n++] = L'-';
		break;
	case 1:
		buf[n++] = L'+';
		break;
	default:
		break;
	}
	zeros = rnd_below(4u);
	for (k = 0; k < zeros; k++)
		buf[n++] = L'0';
	n += fmt_u128_w(buf + n, base);
	switch (rnd_below(8u)) {
	case 0:
		buf[n++] = L'x';
		break;
	case 1:
		buf[n++] = L'b';
		break;
	case 2:
		buf[n++] = L' ';
		buf[n++] = L'9';
		break;
	case 3:
		buf[n++] = (wchar_t)(L'a' + (wchar_t)rnd_below(26u));
		break;
	default:
		break;
	}
	return n;
}

static size_t
gen_digits(wchar_t *buf)
{
	size_t n = 0;
	unsigned len = 1u + rnd_below(50u);
	unsigned k;
	unsigned base_pick = rnd_below(4u);

	if (rnd_below(3u) == 0u)
		buf[n++] = (rnd_below(2u) != 0u) ? L'-' : L'+';
	if (base_pick == 0u) {
		buf[n++] = L'0';
		buf[n++] = (rnd_below(2u) != 0u) ? L'x' : L'X';
	}
	if (base_pick == 1u) {
		buf[n++] = L'0';
		buf[n++] = (rnd_below(2u) != 0u) ? L'b' : L'B';
	}
	for (k = 0; k < len; k++) {
		unsigned d = rnd_below(16u);
		if (d < 10u)
			buf[n++] = (wchar_t)(L'0' + (wchar_t)d);
		else
			buf[n++] = (wchar_t)(L'a' + (wchar_t)(d - 10u));
	}
	return n;
}

static size_t
gen_nul_mix(wchar_t *buf)
{
	size_t n = 0;
	unsigned len = rnd_below(16u);
	unsigned k;

	for (k = 0; k < len; k++) {
		if (rnd_below(5u) == 0u)
			buf[n++] = L'\0';
		else
			buf[n++] = soup_alpha[rnd_below(
			    (unsigned)(sizeof(soup_alpha) / sizeof(wchar_t) -
			    1u))];
	}
	return n;
}

static void
run_random_sweep(void)
{
	wchar_t buf[BUFSZ];
	unsigned i;
	static const int rand_bases[] = {
		0, 2, 8, 10, 16, 1, 37, 36, 35, 24, 3, -5
	};

	for (i = 0; i < RANDOM_ITERS; i++) {
		size_t n;
		int base = rand_bases[rnd_below((unsigned)(sizeof(rand_bases) /
		    sizeof(rand_bases[0])))];

		switch (i % 5u) {
		case 0:
			n = gen_soup(buf);
			break;
		case 1:
			n = gen_boundary(buf);
			break;
		case 2:
			n = gen_digits(buf);
			break;
		case 3:
			n = gen_nul_mix(buf);
			break;
		default:
			n = gen_soup(buf);
			break;
		}
		check_all(buf, n, base);
	}
}

/* ----------------------------------------------------------------- main */

int
main(void)
{
	int fn;
	unsigned long long total_cases = 0, total_fails = 0;

	loc_c = newlocale(LC_ALL_MASK, "C", (locale_t)0);
	loc_posix = newlocale(LC_ALL_MASK, "POSIX", (locale_t)0);
	if (loc_c == (locale_t)0 || loc_posix == (locale_t)0) {
		std::printf("newlocale() failed\n");
		return 1;
	}

	run_edge_cases();
	run_random_sweep();

	std::printf("\nbatch b0045 differential results\n");
	std::printf("%-14s %12s %10s %s\n", "function", "cases", "failures",
	    "status");
	for (fn = 0; fn < F_COUNT; fn++) {
		std::printf("%-14s %12llu %10llu %s\n", fn_name[fn],
		    n_cases[fn], n_fails[fn],
		    n_fails[fn] == 0u ? "ok" : "FAIL");
		total_cases += n_cases[fn];
		total_fails += n_fails[fn];
	}
	std::printf("%-14s %12llu %10llu %s\n", "TOTAL", total_cases,
	    total_fails, total_fails == 0u ? "ok" : "FAIL");

	freelocale(loc_c);
	freelocale(loc_posix);

	return total_fails == 0u ? 0 : 1;
}
