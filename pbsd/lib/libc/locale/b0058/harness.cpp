/*
 * Differential harness for batch b0058.
 */

import pbsd.lib.libc.locale.b0058;

#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>

namespace port = pbsd::lib_libc_locale::b0058;

typedef union {
	char		__bytes[128];
	long long	__align;
} ref_mbstate_t;

extern "C" {
typedef struct xlocale *ref_locale_t;

extern struct xlocale ref_global_locale;
extern struct xlocale_ctype ref_global_ctype;

size_t ref_mbrtoc32_l(char32_t *, const char *, size_t, ref_mbstate_t *,
    ref_locale_t);
size_t ref_mbrtoc32(char32_t *, const char *, size_t, ref_mbstate_t *);
size_t ref_wcrtomb_l(char *, wchar_t, ref_mbstate_t *, ref_locale_t);
size_t ref_wcrtomb(char *, wchar_t, ref_mbstate_t *);
int ref_wctob_l(unsigned int, ref_locale_t);
int ref_wctob(unsigned int);
size_t ref_mbrlen_l(const char *, size_t, ref_mbstate_t *, ref_locale_t);
size_t ref_mbrlen(const char *, size_t, ref_mbstate_t *);
}

struct xlocale_ctype {
	ref_mbstate_t	mbrtoc32;
	ref_mbstate_t	mbrlen;
	ref_mbstate_t	wcrtomb;
	ref_mbstate_t	mbrtowc;
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
static constexpr unsigned long long RANDOM_ITERS = 200000;

static Stats st_mbrtoc32_l = { "mbrtoc32_l", 0, 0 };
static Stats st_mbrtoc32 = { "mbrtoc32", 0, 0 };
static Stats st_wcrtomb_l = { "wcrtomb_l", 0, 0 };
static Stats st_wcrtomb = { "wcrtomb", 0, 0 };
static Stats st_wctob_l = { "wctob_l", 0, 0 };
static Stats st_wctob = { "wctob", 0, 0 };
static Stats st_mbrlen_l = { "mbrlen_l", 0, 0 };
static Stats st_mbrlen = { "mbrlen", 0, 0 };

static uint32_t rng = 0xB0058001u;

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

static void
zero_state(port::mbstate_t *ps)
{
	memset(ps, 0, sizeof(*ps));
}

static void
zero_state(ref_mbstate_t *rs)
{
	memset(rs, 0, sizeof(*rs));
}

static bool
state_eq(const port::mbstate_t &a, const ref_mbstate_t &b)
{
	return (memcmp(&a, &b, sizeof(a)) == 0);
}

static bool
locale_ctype_eq()
{
	port::mbstate_t pmbrtoc32, pmbrlen, pwcrtomb, pmbrtowc;
	const xlocale_ctype *rc;

	port::snapshot_locale_states(&pmbrtoc32, &pmbrlen, &pwcrtomb, &pmbrtowc);
	rc = reinterpret_cast<const xlocale_ctype *>(ref_global_locale.components[1]);
	if (rc == nullptr)
		return (false);
	return (state_eq(pmbrtoc32, rc->mbrtoc32) &&
	    state_eq(pmbrlen, rc->mbrlen) &&
	    state_eq(pwcrtomb, rc->wcrtomb) &&
	    state_eq(pmbrtowc, rc->mbrtowc));
}

static void
report_fail(Stats &st, const char *tag)
{
	st.fails++;
	if (st.fails <= 5)
		std::printf("FAIL %s [%s]\n", st.name, tag);
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
compare_mbrtoc32_l(Stats &st, const unsigned char *in, size_t n, bool use_l,
    bool null_ps, bool null_pc, bool null_locale)
{
	unsigned char pin[64], rin[64];
	unsigned char pwcbuf[16], rwcbuf[16];
	port::mbstate_t pstate;
	ref_mbstate_t rstate;
	char32_t p32, r32;
	size_t pr, rr;
	int perrno, rerrno;

	st.cases++;
	zero_state(&pstate);
	zero_state(&rstate);
	fill_guard(pin, sizeof(pin));
	fill_guard(rin, sizeof(rin));
	fill_guard(pwcbuf, sizeof(pwcbuf));
	fill_guard(rwcbuf, sizeof(rwcbuf));
	memcpy(pin + 8, in, n);
	memcpy(rin + 8, in, n);

	errno = 0;
	if (use_l) {
		pr = port::mbrtoc32_l(null_pc ? nullptr : (char32_t *)(pwcbuf + 4),
		    (const char *)(pin + 8), n,
		    null_ps ? nullptr : &pstate,
		    null_locale ? nullptr : port::global_locale());
		perrno = errno;
		errno = 0;
		rr = ref_mbrtoc32_l(null_pc ? nullptr : (char32_t *)(rwcbuf + 4),
		    (const char *)(rin + 8), n,
		    null_ps ? nullptr : &rstate,
		    null_locale ? nullptr : &ref_global_locale);
		rerrno = errno;
	} else {
		pr = port::mbrtoc32(null_pc ? nullptr : (char32_t *)(pwcbuf + 4),
		    (const char *)(pin + 8), n,
		    null_ps ? nullptr : &pstate);
		perrno = errno;
		errno = 0;
		rr = ref_mbrtoc32(null_pc ? nullptr : (char32_t *)(rwcbuf + 4),
		    (const char *)(rin + 8), n,
		    null_ps ? nullptr : &rstate);
		rerrno = errno;
	}

	memcpy(&p32, pwcbuf + 4, sizeof(p32));
	memcpy(&r32, rwcbuf + 4, sizeof(r32));

	if (pr != rr || perrno != rerrno) {
		report_fail(st, "ret");
		return (false);
	}
	if (memcmp(pin, rin, sizeof(pin)) != 0) {
		report_fail(st, "inbuf");
		return (false);
	}
	if (memcmp(pwcbuf, rwcbuf, sizeof(pwcbuf)) != 0) {
		report_fail(st, "wcharbuf");
		return (false);
	}
	if (p32 != r32) {
		report_fail(st, "wchar");
		return (false);
	}
	if (!null_ps && !state_eq(pstate, rstate)) {
		report_fail(st, "state");
		return (false);
	}
	if (null_ps && !locale_ctype_eq()) {
		report_fail(st, "locstate");
		return (false);
	}
	return (true);
}

static bool
compare_wcrtomb_l(Stats &st, wchar_t wc, bool use_l, bool null_ps,
    bool null_s, bool null_locale)
{
	unsigned char pout[OUT_CAP], rout[OUT_CAP];
	port::mbstate_t pstate;
	ref_mbstate_t rstate;
	size_t pr, rr;
	int perrno, rerrno;

	st.cases++;
	zero_state(&pstate);
	zero_state(&rstate);
	if (!null_s) {
		fill_guard(pout, sizeof(pout));
		fill_guard(rout, sizeof(rout));
	}

	errno = 0;
	if (use_l) {
		pr = port::wcrtomb_l(null_s ? nullptr : (char *)(pout + 4), wc,
		    null_ps ? nullptr : &pstate,
		    null_locale ? nullptr : port::global_locale());
		perrno = errno;
		errno = 0;
		rr = ref_wcrtomb_l(null_s ? nullptr : (char *)(rout + 4), wc,
		    null_ps ? nullptr : &rstate,
		    null_locale ? nullptr : &ref_global_locale);
		rerrno = errno;
	} else {
		pr = port::wcrtomb(null_s ? nullptr : (char *)(pout + 4), wc,
		    null_ps ? nullptr : &pstate);
		perrno = errno;
		errno = 0;
		rr = ref_wcrtomb(null_s ? nullptr : (char *)(rout + 4), wc,
		    null_ps ? nullptr : &rstate);
		rerrno = errno;
	}

	if (pr != rr || perrno != rerrno) {
		report_fail(st, "ret");
		return (false);
	}
	if (null_s)
		return (true);
	if (memcmp(pout, rout, sizeof(pout)) != 0) {
		report_fail(st, "buf");
		return (false);
	}
	if (!null_ps && !state_eq(pstate, rstate)) {
		report_fail(st, "state");
		return (false);
	}
	if (null_ps && !locale_ctype_eq()) {
		report_fail(st, "locstate");
		return (false);
	}
	return (true);
}

static bool
compare_wctob_l(Stats &st, unsigned int c, bool use_l, bool null_locale)
{
	int pr, rr;

	st.cases++;

	if (use_l) {
		pr = port::wctob_l(c, null_locale ? nullptr : port::global_locale());
		rr = ref_wctob_l(c, null_locale ? nullptr : &ref_global_locale);
	} else {
		pr = port::wctob(c);
		rr = ref_wctob(c);
	}

	if (pr != rr) {
		report_fail(st, "ret");
		return (false);
	}
	return (true);
}

static bool
compare_mbrlen_l(Stats &st, const unsigned char *in, size_t n, bool use_l,
    bool null_ps, bool null_locale)
{
	unsigned char pin[64], rin[64];
	port::mbstate_t pstate;
	ref_mbstate_t rstate;
	size_t pr, rr;
	int perrno, rerrno;

	st.cases++;
	zero_state(&pstate);
	zero_state(&rstate);
	fill_guard(pin, sizeof(pin));
	fill_guard(rin, sizeof(rin));
	if (in != nullptr) {
		memcpy(pin + 8, in, n);
		memcpy(rin + 8, in, n);
	}

	errno = 0;
	if (use_l) {
		pr = port::mbrlen_l(in != nullptr ? (const char *)(pin + 8) : nullptr,
		    n, null_ps ? nullptr : &pstate,
		    null_locale ? nullptr : port::global_locale());
		perrno = errno;
		errno = 0;
		rr = ref_mbrlen_l(in != nullptr ? (const char *)(rin + 8) : nullptr,
		    n, null_ps ? nullptr : &rstate,
		    null_locale ? nullptr : &ref_global_locale);
		rerrno = errno;
	} else {
		pr = port::mbrlen(in != nullptr ? (const char *)(pin + 8) : nullptr,
		    n, null_ps ? nullptr : &pstate);
		perrno = errno;
		errno = 0;
		rr = ref_mbrlen(in != nullptr ? (const char *)(rin + 8) : nullptr,
		    n, null_ps ? nullptr : &rstate);
		rerrno = errno;
	}

	if (pr != rr || perrno != rerrno) {
		report_fail(st, "ret");
		return (false);
	}
	if (in != nullptr && memcmp(pin, rin, sizeof(pin)) != 0) {
		report_fail(st, "inbuf");
		return (false);
	}
	if (!null_ps && !state_eq(pstate, rstate)) {
		report_fail(st, "state");
		return (false);
	}
	if (null_ps && !locale_ctype_eq()) {
		report_fail(st, "locstate");
		return (false);
	}
	return (true);
}

static void
hand_mbrtoc32(Stats &st, bool use_l)
{
	unsigned char seq[8];
	size_t n;

	seq[0] = '\0';
	compare_mbrtoc32_l(st, seq, 0, use_l, false, false, false);
	compare_mbrtoc32_l(st, seq, 1, use_l, false, false, false);
	seq[0] = 'A';
	compare_mbrtoc32_l(st, seq, 0, use_l, false, false, false);
	compare_mbrtoc32_l(st, seq, 1, use_l, false, false, false);
	compare_mbrtoc32_l(st, seq, 2, use_l, false, false, false);
	n = utf8_encode(0x80, seq, sizeof(seq));
	compare_mbrtoc32_l(st, seq, n, use_l, false, false, false);
	compare_mbrtoc32_l(st, seq, 1, use_l, false, false, false);
	n = utf8_encode(0x7FF, seq, sizeof(seq));
	compare_mbrtoc32_l(st, seq, n, use_l, false, false, false);
	n = utf8_encode(0x20AC, seq, sizeof(seq));
	compare_mbrtoc32_l(st, seq, n, use_l, false, false, false);
	n = utf8_encode(0x10000, seq, sizeof(seq));
	compare_mbrtoc32_l(st, seq, n, use_l, false, false, false);
	compare_mbrtoc32_l(st, seq, 1, use_l, false, false, false);
	seq[0] = 0xFF;
	compare_mbrtoc32_l(st, seq, 1, use_l, false, false, false);
	seq[0] = 0xC0;
	seq[1] = 0x80;
	compare_mbrtoc32_l(st, seq, 2, use_l, false, false, false);
	compare_mbrtoc32_l(st, seq, 1, use_l, false, true, false);
	compare_mbrtoc32_l(st, seq, 1, use_l, true, false, false);
	compare_mbrtoc32_l(st, seq, 1, use_l, false, false, true);
	compare_mbrtoc32_l(st, seq, 2, use_l, true, true, true);
}

static void
hand_wcrtomb(Stats &st, bool use_l)
{
	compare_wcrtomb_l(st, L'\0', use_l, false, false, false);
	compare_wcrtomb_l(st, L'a', use_l, false, false, false);
	compare_wcrtomb_l(st, L'\x7f', use_l, false, false, false);
	compare_wcrtomb_l(st, (wchar_t)0x80, use_l, false, false, false);
	compare_wcrtomb_l(st, (wchar_t)0x7ff, use_l, false, false, false);
	compare_wcrtomb_l(st, (wchar_t)0x800, use_l, false, false, false);
	compare_wcrtomb_l(st, (wchar_t)0xffff, use_l, false, false, false);
	compare_wcrtomb_l(st, (wchar_t)0x10000, use_l, false, false, false);
	compare_wcrtomb_l(st, (wchar_t)0x10ffff, use_l, false, false, false);
	compare_wcrtomb_l(st, (wchar_t)0xd800, use_l, false, false, false);
	compare_wcrtomb_l(st, (wchar_t)0xdfff, use_l, false, false, false);
	compare_wcrtomb_l(st, (wchar_t)-1, use_l, false, false, false);
	compare_wcrtomb_l(st, L'x', use_l, false, true, false);
	compare_wcrtomb_l(st, L'q', use_l, true, false, false);
	compare_wcrtomb_l(st, L'z', use_l, false, false, true);
	compare_wcrtomb_l(st, L'w', use_l, true, true, true);
}

static void
hand_wctob(Stats &st, bool use_l)
{
	compare_wctob_l(st, (unsigned int)WEOF, use_l, false);
	compare_wctob_l(st, 0, use_l, false);
	compare_wctob_l(st, 'a', use_l, false);
	compare_wctob_l(st, 0x7f, use_l, false);
	compare_wctob_l(st, 0x80, use_l, false);
	compare_wctob_l(st, 0x7ff, use_l, false);
	compare_wctob_l(st, 0x800, use_l, false);
	compare_wctob_l(st, 0x10000, use_l, false);
	compare_wctob_l(st, 0x10ffff, use_l, false);
	compare_wctob_l(st, 0xd800, use_l, false);
	compare_wctob_l(st, 0xdfff, use_l, false);
	compare_wctob_l(st, (unsigned int)-1, use_l, false);
	compare_wctob_l(st, 256, use_l, false);
	compare_wctob_l(st, (unsigned int)WEOF, use_l, true);
	compare_wctob_l(st, 'Z', use_l, true);
}

static void
hand_mbrlen(Stats &st, bool use_l)
{
	static const unsigned char empty[] = "";
	static const unsigned char one[] = "a";
	static const unsigned char two[] = "ab";
	static const unsigned char euro[] = { 0xe2, 0x82, 0xac };
	static const unsigned char plane[] = { 0xf0, 0x90, 0x8d, 0x88 };
	static const unsigned char bad[] = { 0xff };
	static const unsigned char overlong[] = { 0xc0, 0x80 };

	compare_mbrlen_l(st, nullptr, 0, use_l, false, false);
	compare_mbrlen_l(st, empty, 0, use_l, false, false);
	compare_mbrlen_l(st, empty, 1, use_l, false, false);
	compare_mbrlen_l(st, one, 0, use_l, false, false);
	compare_mbrlen_l(st, one, 1, use_l, false, false);
	compare_mbrlen_l(st, one, 2, use_l, false, false);
	compare_mbrlen_l(st, two, 1, use_l, false, false);
	compare_mbrlen_l(st, two, 2, use_l, false, false);
	compare_mbrlen_l(st, euro, 1, use_l, false, false);
	compare_mbrlen_l(st, euro, 2, use_l, false, false);
	compare_mbrlen_l(st, euro, 3, use_l, false, false);
	compare_mbrlen_l(st, euro, 4, use_l, false, false);
	compare_mbrlen_l(st, plane, 1, use_l, false, false);
	compare_mbrlen_l(st, plane, 4, use_l, false, false);
	compare_mbrlen_l(st, bad, 1, use_l, false, false);
	compare_mbrlen_l(st, overlong, 1, use_l, false, false);
	compare_mbrlen_l(st, overlong, 2, use_l, false, false);
	compare_mbrlen_l(st, one, 1, use_l, true, false);
	compare_mbrlen_l(st, one, 1, use_l, false, true);
	compare_mbrlen_l(st, nullptr, 5, use_l, true, true);
}

static void
random_mbrtoc32(Stats &st, bool use_l)
{
	unsigned char seq[16];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		size_t n = (size_t)(xorshift32() % (sizeof(seq) + 1));

		for (size_t j = 0; j < n; j++)
			seq[j] = (unsigned char)(xorshift32() & 0xffu);
		if (!compare_mbrtoc32_l(st, seq, n, use_l,
		    (xorshift32() & 7u) == 0, (xorshift32() & 7u) == 0,
		    (xorshift32() & 0x3ffu) == 0))
			return;
	}
}

static void
random_wcrtomb(Stats &st, bool use_l)
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		wchar_t wc = (wchar_t)(xorshift32() & 0x1fffffu);

		if (!compare_wcrtomb_l(st, wc, use_l,
		    (xorshift32() & 7u) == 0, (xorshift32() & 3u) != 0,
		    (xorshift32() & 0x7ffu) == 0))
			return;
	}
}

static void
random_wctob(Stats &st, bool use_l)
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		unsigned int c;

		if ((xorshift32() & 0xffu) == 0)
			c = (unsigned int)WEOF;
		else
			c = xorshift32() & 0x1fffffu;
		if (!compare_wctob_l(st, c, use_l, (xorshift32() & 0x3ffu) == 0))
			return;
	}
}

static void
random_mbrlen(Stats &st, bool use_l)
{
	unsigned char seq[16];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		size_t len = (size_t)(xorshift32() % (sizeof(seq) + 1));
		size_t n = (size_t)(xorshift32() % (sizeof(seq) + 1));

		if ((xorshift32() & 0x1ffu) == 0) {
			if (!compare_mbrlen_l(st, nullptr, n, use_l,
			    (xorshift32() & 7u) == 0,
			    (xorshift32() & 0x3ffu) == 0))
				return;
			continue;
		}
		for (size_t j = 0; j < len; j++)
			seq[j] = (unsigned char)(xorshift32() & 0xffu);
		if (!compare_mbrlen_l(st, seq, n, use_l,
		    (xorshift32() & 7u) == 0, (xorshift32() & 0x7ffu) == 0))
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

	hand_mbrtoc32(st_mbrtoc32_l, true);
	hand_mbrtoc32(st_mbrtoc32, false);
	hand_wcrtomb(st_wcrtomb_l, true);
	hand_wcrtomb(st_wcrtomb, false);
	hand_wctob(st_wctob_l, true);
	hand_wctob(st_wctob, false);
	hand_mbrlen(st_mbrlen_l, true);
	hand_mbrlen(st_mbrlen, false);

	random_mbrtoc32(st_mbrtoc32_l, true);
	random_mbrtoc32(st_mbrtoc32, false);
	random_wcrtomb(st_wcrtomb_l, true);
	random_wcrtomb(st_wcrtomb, false);
	random_wctob(st_wctob_l, true);
	random_wctob(st_wctob, false);
	random_mbrlen(st_mbrlen_l, true);
	random_mbrlen(st_mbrlen, false);

	std::printf("\n%-16s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-16s %12s %12s\n", "--------", "-----", "--------");
	print_row(st_mbrtoc32_l);
	print_row(st_mbrtoc32);
	print_row(st_wcrtomb_l);
	print_row(st_wcrtomb);
	print_row(st_wctob_l);
	print_row(st_wctob);
	print_row(st_mbrlen_l);
	print_row(st_mbrlen);

	if (st_mbrtoc32_l.fails != 0 || st_mbrtoc32.fails != 0 ||
	    st_wcrtomb_l.fails != 0 || st_wcrtomb.fails != 0 ||
	    st_wctob_l.fails != 0 || st_wctob.fails != 0 ||
	    st_mbrlen_l.fails != 0 || st_mbrlen.fails != 0)
		return (1);
	return (0);
}
