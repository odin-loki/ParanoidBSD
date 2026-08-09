/*
 * Differential harness for batch b0128.
 */

import pbsd.lib.libc.locale.b0128;

#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::lib_libc_locale::b0128;

typedef union {
	char		__bytes[128];
	long long	__align;
} ref_mbstate_t;

extern "C" {
typedef struct xlocale *ref_locale_t;

extern struct xlocale ref_global_locale;
extern struct xlocale_ctype ref_global_ctype;

float ref_wcstof_l(const wchar_t *, wchar_t **, ref_locale_t);
float ref_wcstof(const wchar_t *, wchar_t **);
long double ref_wcstold_l(const wchar_t *, wchar_t **, ref_locale_t);
long double ref_wcstold(const wchar_t *, wchar_t **);
unsigned int ref_towctrans_l(unsigned int, int, ref_locale_t);
unsigned int ref_towctrans(unsigned int, int);
int ref_wctrans_l(const char *, ref_locale_t);
int ref_wctrans(const char *);
size_t ref_mbrtoc16_l(uint16_t *, const char *, size_t, ref_mbstate_t *,
    ref_locale_t);
size_t ref_mbrtoc16(uint16_t *, const char *, size_t, ref_mbstate_t *);
void ref_reset_malloc(void);
void ref_set_malloc_fail_at(int at);
void ref_reset_locale_states(void);
}

struct xlocale_ctype {
	void		*__mbrtowc;
	void		*__wcsnrtombs;
	ref_mbstate_t	mbrtoc16;
	ref_mbstate_t	mbrtoc32;
	ref_mbstate_t	wcsrtombs;
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
static constexpr size_t WBUF_CAP = 128;
static constexpr size_t OUT_CAP = 64;
static constexpr unsigned long long RANDOM_ITERS = 200000;

static Stats st_wcstof_l = { "wcstof_l", 0, 0 };
static Stats st_wcstof = { "wcstof", 0, 0 };
static Stats st_wcstold_l = { "wcstold_l", 0, 0 };
static Stats st_wcstold = { "wcstold", 0, 0 };
static Stats st_towctrans_l = { "towctrans_l", 0, 0 };
static Stats st_towctrans = { "towctrans", 0, 0 };
static Stats st_wctrans_l = { "wctrans_l", 0, 0 };
static Stats st_wctrans = { "wctrans", 0, 0 };
static Stats st_mbrtoc16_l = { "mbrtoc16_l", 0, 0 };
static Stats st_mbrtoc16 = { "mbrtoc16", 0, 0 };

static uint32_t rng = 0xB0128001u;

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
reset_both()
{
	port::init_locale();
	port::reset_malloc();
	ref_reset_malloc();
	ref_reset_locale_states();
	ref_global_locale.components[1] = &ref_global_ctype;
}

static void
fill_guard(unsigned char *buf, size_t n)
{
	memset(buf, GUARD, n);
}

static void
report_fail(Stats &st, const char *tag)
{
	st.fails++;
	if (st.fails <= 5)
		std::printf("FAIL %s [%s]\n", st.name, tag);
}

static bool
float_bits_eq(float a, float b)
{
	uint32_t ua, ub;

	memcpy(&ua, &a, sizeof(ua));
	memcpy(&ub, &b, sizeof(ub));
	return (ua == ub);
}

static bool
ld_bits_eq(long double a, long double b)
{
	unsigned char ba[sizeof(long double)];
	unsigned char bb[sizeof(long double)];

	memcpy(ba, &a, sizeof(ba));
	memcpy(bb, &b, sizeof(bb));
	return (memcmp(ba, bb, sizeof(ba)) == 0);
}

static size_t
wcsnlen_local(const wchar_t *s, size_t maxlen)
{
	size_t n = 0;

	while (n < maxlen && s[n] != L'\0')
		n++;
	return (n);
}

static bool
state_eq(const port::mbstate_t &a, const ref_mbstate_t &b)
{
	return (memcmp(&a, &b, sizeof(a)) == 0);
}

static size_t
utf8_encode(uint32_t cp, unsigned char *out, size_t cap)
{
	if (cap < 1)
		return (0);
	if (cp < 0x80) {
		out[0] = (unsigned char)cp;
		return (1);
	}
	if (cp < 0x800 && cap >= 2) {
		out[0] = (unsigned char)(0xC0 | (cp >> 6));
		out[1] = (unsigned char)(0x80 | (cp & 0x3F));
		return (2);
	}
	if (cp < 0x10000 && cap >= 3) {
		out[0] = (unsigned char)(0xE0 | (cp >> 12));
		out[1] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
		out[2] = (unsigned char)(0x80 | (cp & 0x3F));
		return (3);
	}
	if (cp <= 0x10FFFF && cap >= 4) {
		out[0] = (unsigned char)(0xF0 | (cp >> 18));
		out[1] = (unsigned char)(0x80 | ((cp >> 12) & 0x3F));
		out[2] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
		out[3] = (unsigned char)(0x80 | (cp & 0x3F));
		return (4);
	}
	return (0);
}

static bool
compare_wcstof_l(Stats &st, const wchar_t *in, bool use_l, bool null_end,
    bool null_locale, bool malloc_fail)
{
	unsigned char pwbuf[WBUF_CAP], rwbuf[WBUF_CAP];
	wchar_t *pend, *rend;
	wchar_t poff, roff;
	float pf, rf;
	size_t plen, rlen;

	st.cases++;
	reset_both();
	if (malloc_fail) {
		port::set_malloc_fail_at(1);
		ref_set_malloc_fail_at(1);
	}
	fill_guard(pwbuf, sizeof(pwbuf));
	fill_guard(rwbuf, sizeof(rwbuf));
	plen = wcsnlen_local(in, (sizeof(pwbuf) / sizeof(wchar_t)) - 9);
	rlen = plen;
	memcpy(pwbuf + 8, in, (plen + 1) * sizeof(wchar_t));
	memcpy(rwbuf + 8, in, (rlen + 1) * sizeof(wchar_t));

	if (use_l) {
		pf = port::wcstof_l((const wchar_t *)(pwbuf + 8),
		    null_end ? nullptr : &pend,
		    null_locale ? nullptr : port::global_locale());
		rf = ref_wcstof_l((const wchar_t *)(rwbuf + 8),
		    null_end ? nullptr : &rend,
		    null_locale ? nullptr : &ref_global_locale);
	} else {
		pf = port::wcstof((const wchar_t *)(pwbuf + 8),
		    null_end ? nullptr : &pend);
		rf = ref_wcstof((const wchar_t *)(rwbuf + 8),
		    null_end ? nullptr : &rend);
	}

	if (!float_bits_eq(pf, rf)) {
		report_fail(st, "val");
		return (false);
	}
	if (memcmp(pwbuf, rwbuf, sizeof(pwbuf)) != 0) {
		report_fail(st, "wbuf");
		return (false);
	}
	if (!null_end) {
		poff = (wchar_t)(pend - (const wchar_t *)(pwbuf + 8));
		roff = (wchar_t)(rend - (const wchar_t *)(rwbuf + 8));
		if (poff != roff) {
			report_fail(st, "end");
			return (false);
		}
	}
	return (true);
}

static bool
compare_wcstold_l(Stats &st, const wchar_t *in, bool use_l, bool null_end,
    bool null_locale, bool malloc_fail)
{
	unsigned char pwbuf[WBUF_CAP], rwbuf[WBUF_CAP];
	wchar_t *pend, *rend;
	wchar_t poff, roff;
	long double pf, rf;
	size_t plen;

	st.cases++;
	reset_both();
	if (malloc_fail) {
		port::set_malloc_fail_at(1);
		ref_set_malloc_fail_at(1);
	}
	fill_guard(pwbuf, sizeof(pwbuf));
	fill_guard(rwbuf, sizeof(rwbuf));
	plen = wcsnlen_local(in, (sizeof(pwbuf) / sizeof(wchar_t)) - 9);
	memcpy(pwbuf + 8, in, (plen + 1) * sizeof(wchar_t));
	memcpy(rwbuf + 8, in, (plen + 1) * sizeof(wchar_t));

	if (use_l) {
		pf = port::wcstold_l((const wchar_t *)(pwbuf + 8),
		    null_end ? nullptr : &pend,
		    null_locale ? nullptr : port::global_locale());
		rf = ref_wcstold_l((const wchar_t *)(rwbuf + 8),
		    null_end ? nullptr : &rend,
		    null_locale ? nullptr : &ref_global_locale);
	} else {
		pf = port::wcstold((const wchar_t *)(pwbuf + 8),
		    null_end ? nullptr : &pend);
		rf = ref_wcstold((const wchar_t *)(rwbuf + 8),
		    null_end ? nullptr : &rend);
	}

	if (!ld_bits_eq(pf, rf)) {
		report_fail(st, "val");
		return (false);
	}
	if (memcmp(pwbuf, rwbuf, sizeof(pwbuf)) != 0) {
		report_fail(st, "wbuf");
		return (false);
	}
	if (!null_end) {
		poff = (wchar_t)(pend - (const wchar_t *)(pwbuf + 8));
		roff = (wchar_t)(rend - (const wchar_t *)(rwbuf + 8));
		if (poff != roff) {
			report_fail(st, "end");
			return (false);
		}
	}
	return (true);
}

static bool
compare_towctrans_l(Stats &st, unsigned int wc, int desc, bool use_l,
    bool null_locale)
{
	unsigned int pr, rr;
	int perrno, rerrno;

	st.cases++;

	errno = 0;
	if (use_l) {
		pr = port::towctrans_l(wc, desc,
		    null_locale ? nullptr : port::global_locale());
		perrno = errno;
		errno = 0;
		rr = ref_towctrans_l(wc, desc,
		    null_locale ? nullptr : &ref_global_locale);
		rerrno = errno;
	} else {
		pr = port::towctrans(wc, desc);
		perrno = errno;
		errno = 0;
		rr = ref_towctrans(wc, desc);
		rerrno = errno;
	}

	if (pr != rr || perrno != rerrno) {
		report_fail(st, "ret");
		return (false);
	}
	return (true);
}

static bool
compare_wctrans_l(Stats &st, const char *name, bool use_l, bool null_locale)
{
	int pr, rr;
	int perrno, rerrno;

	st.cases++;

	errno = 0;
	if (use_l) {
		pr = port::wctrans_l(name,
		    null_locale ? nullptr : port::global_locale());
		perrno = errno;
		errno = 0;
		rr = ref_wctrans_l(name,
		    null_locale ? nullptr : &ref_global_locale);
		rerrno = errno;
	} else {
		pr = port::wctrans(name);
		perrno = errno;
		errno = 0;
		rr = ref_wctrans(name);
		rerrno = errno;
	}

	if (pr != rr || perrno != rerrno) {
		report_fail(st, "ret");
		return (false);
	}
	return (true);
}

static bool
compare_mbrtoc16_l(Stats &st, const unsigned char *in, size_t n, bool use_l,
    bool null_ps, bool null_pc, bool null_locale)
{
	unsigned char pin[OUT_CAP], rin[OUT_CAP];
	unsigned char pwcbuf[16], rwcbuf[16];
	port::mbstate_t pstate;
	ref_mbstate_t rstate;
	uint16_t p16, r16;
	size_t pr, rr;
	int perrno, rerrno;

	st.cases++;
	if (null_ps)
		reset_both();
	else
		port::init_locale();
	memset(&pstate, 0, sizeof(pstate));
	memset(&rstate, 0, sizeof(rstate));
	fill_guard(pin, sizeof(pin));
	fill_guard(rin, sizeof(rin));
	fill_guard(pwcbuf, sizeof(pwcbuf));
	fill_guard(rwcbuf, sizeof(rwcbuf));
	if (in != nullptr) {
		memcpy(pin + 8, in, n);
		memcpy(rin + 8, in, n);
	}

	errno = 0;
	if (use_l) {
		pr = port::mbrtoc16_l(null_pc ? nullptr : (std::uint16_t *)(pwcbuf + 4),
		    in != nullptr ? (const char *)(pin + 8) : nullptr, n,
		    null_ps ? nullptr : &pstate,
		    null_locale ? nullptr : port::global_locale());
		perrno = errno;
		errno = 0;
		rr = ref_mbrtoc16_l(null_pc ? nullptr : (uint16_t *)(rwcbuf + 4),
		    in != nullptr ? (const char *)(rin + 8) : nullptr, n,
		    null_ps ? nullptr : &rstate,
		    null_locale ? nullptr : &ref_global_locale);
		rerrno = errno;
	} else {
		pr = port::mbrtoc16(null_pc ? nullptr : (std::uint16_t *)(pwcbuf + 4),
		    in != nullptr ? (const char *)(pin + 8) : nullptr, n,
		    null_ps ? nullptr : &pstate);
		perrno = errno;
		errno = 0;
		rr = ref_mbrtoc16(null_pc ? nullptr : (uint16_t *)(rwcbuf + 4),
		    in != nullptr ? (const char *)(rin + 8) : nullptr, n,
		    null_ps ? nullptr : &rstate);
		rerrno = errno;
	}

	memcpy(&p16, pwcbuf + 4, sizeof(p16));
	memcpy(&r16, rwcbuf + 4, sizeof(r16));

	if (pr != rr || perrno != rerrno) {
		report_fail(st, "ret");
		return (false);
	}
	if (in != nullptr && memcmp(pin, rin, sizeof(pin)) != 0) {
		report_fail(st, "inbuf");
		return (false);
	}
	if (memcmp(pwcbuf, rwcbuf, sizeof(pwcbuf)) != 0) {
		report_fail(st, "outbuf");
		return (false);
	}
	if (!null_pc && p16 != r16) {
		report_fail(st, "c16");
		return (false);
	}
	if (!null_ps && !state_eq(pstate, rstate)) {
		report_fail(st, "state");
		return (false);
	}
	return (true);
}

static bool
compare_mbrtoc16_pair(Stats &st, const unsigned char *in, size_t n, bool use_l)
{
	unsigned char seq[16];
	size_t i;

	if (!compare_mbrtoc16_l(st, in, n, use_l, false, false, false))
		return (false);
	memcpy(seq, in, n);
	for (i = 0; i < n; i++) {
		if (!compare_mbrtoc16_l(st, seq, i + 1, use_l, false, false, false))
			return (false);
	}
	return (true);
}

static void
hand_wcstof(Stats &st, bool use_l)
{
	compare_wcstof_l(st, L"", use_l, false, false, false);
	compare_wcstof_l(st, L"   ", use_l, false, false, false);
	compare_wcstof_l(st, L"  0", use_l, false, false, false);
	compare_wcstof_l(st, L"  42", use_l, false, false, false);
	compare_wcstof_l(st, L"123", use_l, false, false, false);
	compare_wcstof_l(st, L"-123.45", use_l, false, false, false);
	compare_wcstof_l(st, L"+3.14e2", use_l, false, false, false);
	compare_wcstof_l(st, L"0x1.8p3", use_l, false, false, false);
	compare_wcstof_l(st, L"abc", use_l, false, false, false);
	compare_wcstof_l(st, L"12abc", use_l, false, false, false);
	compare_wcstof_l(st, L" 12abc", use_l, false, false, false);
	compare_wcstof_l(st, L"\t\n-0.5", use_l, false, false, false);
	compare_wcstof_l(st, L"inf", use_l, false, false, false);
	compare_wcstof_l(st, L"nan", use_l, false, false, false);
	compare_wcstof_l(st, L".5", use_l, false, false, false);
	compare_wcstof_l(st, L"1.", use_l, false, false, false);
	compare_wcstof_l(st, L"1e", use_l, false, false, false);
	compare_wcstof_l(st, L"1e+3", use_l, false, false, false);
	compare_wcstof_l(st, L"\x80", use_l, false, false, false);
	compare_wcstof_l(st, L"\x7f", use_l, false, false, false);
	compare_wcstof_l(st, L"123", use_l, true, false, false);
	compare_wcstof_l(st, L"456", use_l, false, true, false);
	compare_wcstof_l(st, L"789", use_l, false, false, true);
	compare_wcstof_l(st, L"42", use_l, false, false, true);
}

static void
hand_wcstold(Stats &st, bool use_l)
{
	compare_wcstold_l(st, L"", use_l, false, false, false);
	compare_wcstold_l(st, L"   ", use_l, false, false, false);
	compare_wcstold_l(st, L"  1.25", use_l, false, false, false);
	compare_wcstold_l(st, L"-99.99", use_l, false, false, false);
	compare_wcstold_l(st, L"+1e40", use_l, false, false, false);
	compare_wcstold_l(st, L"xyz", use_l, false, false, false);
	compare_wcstold_l(st, L"3xyz", use_l, false, false, false);
	compare_wcstold_l(st, L" 3xyz", use_l, false, false, false);
	compare_wcstold_l(st, L"0", use_l, true, false, false);
	compare_wcstold_l(st, L"1", use_l, false, true, false);
	compare_wcstold_l(st, L"2", use_l, false, false, true);
}

static void
hand_towctrans(Stats &st, bool use_l)
{
	compare_towctrans_l(st, 'A', port::WCT_TOLOWER, use_l, false);
	compare_towctrans_l(st, 'z', port::WCT_TOUPPER, use_l, false);
	compare_towctrans_l(st, '5', port::WCT_TOLOWER, use_l, false);
	compare_towctrans_l(st, 0x80, port::WCT_TOUPPER, use_l, false);
	compare_towctrans_l(st, 0xff, port::WCT_TOLOWER, use_l, false);
	compare_towctrans_l(st, 'm', port::WCT_ERROR, use_l, false);
	compare_towctrans_l(st, 'n', 99, use_l, false);
	compare_towctrans_l(st, 'Q', port::WCT_TOLOWER, use_l, true);
	compare_towctrans_l(st, 'r', port::WCT_TOUPPER, use_l, true);
}

static void
hand_wctrans(Stats &st, bool use_l)
{
	compare_wctrans_l(st, "tolower", use_l, false);
	compare_wctrans_l(st, "toupper", use_l, false);
	compare_wctrans_l(st, "", use_l, false);
	compare_wctrans_l(st, "tolowerx", use_l, false);
	compare_wctrans_l(st, "TOLOWER", use_l, false);
	compare_wctrans_l(st, "toupper", use_l, true);
	compare_wctrans_l(st, "tolower", use_l, true);
}

static void
hand_mbrtoc16(Stats &st, bool use_l)
{
	unsigned char seq[8];
	size_t n;

	seq[0] = '\0';
	compare_mbrtoc16_l(st, seq, 0, use_l, false, false, false);
	compare_mbrtoc16_l(st, seq, 1, use_l, false, false, false);
	seq[0] = 'A';
	compare_mbrtoc16_l(st, seq, 0, use_l, false, false, false);
	compare_mbrtoc16_l(st, seq, 1, use_l, false, false, false);
	compare_mbrtoc16_l(st, seq, 2, use_l, false, false, false);
	n = utf8_encode(0x80, seq, sizeof(seq));
	compare_mbrtoc16_pair(st, seq, n, use_l);
	n = utf8_encode(0x7ff, seq, sizeof(seq));
	compare_mbrtoc16_pair(st, seq, n, use_l);
	n = utf8_encode(0x20ac, seq, sizeof(seq));
	compare_mbrtoc16_pair(st, seq, n, use_l);
	n = utf8_encode(0x10000, seq, sizeof(seq));
	compare_mbrtoc16_pair(st, seq, n, use_l);
	seq[0] = 0xff;
	compare_mbrtoc16_l(st, seq, 1, use_l, false, false, false);
	seq[0] = 0xc0;
	seq[1] = 0x80;
	compare_mbrtoc16_l(st, seq, 2, use_l, false, false, false);
	compare_mbrtoc16_l(st, seq, 1, use_l, false, true, false);
	compare_mbrtoc16_l(st, seq, 1, use_l, true, false, false);
	compare_mbrtoc16_l(st, nullptr, 0, use_l, false, true, false);
	compare_mbrtoc16_l(st, seq, 1, use_l, false, false, true);
}

static void
random_wcstof(Stats &st, bool use_l)
{
	wchar_t wbuf[64];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		size_t n = (size_t)(xorshift32() % 48);

		for (size_t j = 0; j < n; j++) {
			uint32_t r = xorshift32();
			if ((r & 0x3fu) == 0)
				wbuf[j] = (wchar_t)("0123456789.+-eEinfanx"[r % 18]);
			else if ((r & 0x100u) == 0)
				wbuf[j] = (wchar_t)L' ';
			else
				wbuf[j] = (wchar_t)(r & 0x7fu);
		}
		wbuf[n] = L'\0';
		if (!compare_wcstof_l(st, wbuf, use_l,
		    (xorshift32() & 7u) == 0, (xorshift32() & 0x3ffu) == 0,
		    (xorshift32() & 0xffffu) == 0))
			return;
	}
}

static void
random_wcstold(Stats &st, bool use_l)
{
	wchar_t wbuf[64];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		size_t n = (size_t)(xorshift32() % 48);

		for (size_t j = 0; j < n; j++) {
			uint32_t r = xorshift32();
			if ((r & 0x3fu) == 0)
				wbuf[j] = (wchar_t)("0123456789.+-eE"[r % 14]);
			else if ((r & 0x100u) == 0)
				wbuf[j] = (wchar_t)L' ';
			else
				wbuf[j] = (wchar_t)(r & 0xffu);
		}
		wbuf[n] = L'\0';
		if (!compare_wcstold_l(st, wbuf, use_l,
		    (xorshift32() & 7u) == 0, (xorshift32() & 0x3ffu) == 0,
		    (xorshift32() & 0xffffu) == 0))
			return;
	}
}

static void
random_towctrans(Stats &st, bool use_l)
{
	static const int descs[] = {
		port::WCT_TOLOWER, port::WCT_TOUPPER, port::WCT_ERROR, 3, -1
	};

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		unsigned int wc = xorshift32() & 0x1ffffu;
		int desc = descs[xorshift32() % (sizeof(descs) / sizeof(descs[0]))];

		if (!compare_towctrans_l(st, wc, desc, use_l,
		    (xorshift32() & 0x3ffu) == 0))
			return;
	}
}

static void
random_wctrans(Stats &st, bool use_l)
{
	static const char *names[] = {
		"tolower", "toupper", "", "x", "tolowerx", "TOLOWER", "toup"
	};

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		const char *name = names[xorshift32() % 7];
		if (!compare_wctrans_l(st, name, use_l, (xorshift32() & 0x3ffu) == 0))
			return;
	}
}

static void
random_mbrtoc16(Stats &st, bool use_l)
{
	unsigned char seq[16];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		size_t n = (size_t)(xorshift32() % (sizeof(seq) + 1));

		for (size_t j = 0; j < n; j++)
			seq[j] = (unsigned char)(xorshift32() & 0xffu);
		if (!compare_mbrtoc16_l(st, seq, n, use_l,
		    (xorshift32() & 7u) == 0, (xorshift32() & 7u) == 0,
		    (xorshift32() & 0x3ffu) == 0))
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
	reset_both();

	hand_wcstof(st_wcstof_l, true);
	hand_wcstof(st_wcstof, false);
	hand_wcstold(st_wcstold_l, true);
	hand_wcstold(st_wcstold, false);
	hand_towctrans(st_towctrans_l, true);
	hand_towctrans(st_towctrans, false);
	hand_wctrans(st_wctrans_l, true);
	hand_wctrans(st_wctrans, false);
	hand_mbrtoc16(st_mbrtoc16_l, true);
	hand_mbrtoc16(st_mbrtoc16, false);

	compare_wcstof_l(st_wcstof_l, L"99", true, false, false, true);
	compare_wcstof_l(st_wcstof, L"88", false, false, false, true);
	compare_wcstold_l(st_wcstold_l, L"77", true, false, false, true);

	random_wcstof(st_wcstof_l, true);
	random_wcstof(st_wcstof, false);
	random_wcstold(st_wcstold_l, true);
	random_wcstold(st_wcstold, false);
	random_towctrans(st_towctrans_l, true);
	random_towctrans(st_towctrans, false);
	random_wctrans(st_wctrans_l, true);
	random_wctrans(st_wctrans, false);
	random_mbrtoc16(st_mbrtoc16_l, true);
	random_mbrtoc16(st_mbrtoc16, false);

	std::printf("\n%-16s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-16s %12s %12s\n", "--------", "-----", "--------");
	print_row(st_wcstof_l);
	print_row(st_wcstof);
	print_row(st_wcstold_l);
	print_row(st_wcstold);
	print_row(st_towctrans_l);
	print_row(st_towctrans);
	print_row(st_wctrans_l);
	print_row(st_wctrans);
	print_row(st_mbrtoc16_l);
	print_row(st_mbrtoc16);

	if (st_wcstof_l.fails != 0 || st_wcstof.fails != 0 ||
	    st_wcstold_l.fails != 0 || st_wcstold.fails != 0 ||
	    st_towctrans_l.fails != 0 || st_towctrans.fails != 0 ||
	    st_wctrans_l.fails != 0 || st_wctrans.fails != 0 ||
	    st_mbrtoc16_l.fails != 0 || st_mbrtoc16.fails != 0)
		return (1);
	return (0);
}
