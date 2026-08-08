/*
 * Differential harness for batch b0075.
 */

import pbsd.lib.libc.locale.b0075;

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::lib_libc_locale::b0075;

typedef union {
	char		__bytes[128];
	long long	__align;
} ref_mbstate_t;

extern "C" {
typedef struct xlocale *ref_locale_t;

extern struct xlocale ref_global_locale;
extern struct xlocale_ctype ref_global_ctype;

int ref___collate_range_cmp(char, char);
int ref___wcollate_range_cmp(wchar_t, wchar_t);
size_t ref_mbrtowc_l(wchar_t *, const char *, size_t, ref_mbstate_t *,
    ref_locale_t);
size_t ref_mbrtowc(wchar_t *, const char *, size_t, ref_mbstate_t *);
size_t ref_c32rtomb_l(char *, char32_t, ref_mbstate_t *, ref_locale_t);
size_t ref_c32rtomb(char *, char32_t, ref_mbstate_t *);
size_t ref_mbstowcs_l(wchar_t *, const char *, size_t, ref_locale_t);
size_t ref_mbstowcs(wchar_t *, const char *, size_t);
}

struct xlocale_ctype {
	ref_mbstate_t	mbrtowc;
	ref_mbstate_t	c32rtomb;
	ref_mbstate_t	wcrtomb;
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

static Stats st_collate = { "__collate_range_cmp", 0, 0 };
static Stats st_wcollate = { "__wcollate_range_cmp", 0, 0 };
static Stats st_mbrtowc_l = { "mbrtowc_l", 0, 0 };
static Stats st_mbrtowc = { "mbrtowc", 0, 0 };
static Stats st_c32rtomb_l = { "c32rtomb_l", 0, 0 };
static Stats st_c32rtomb = { "c32rtomb", 0, 0 };
static Stats st_mbstowcs_l = { "mbstowcs_l", 0, 0 };
static Stats st_mbstowcs = { "mbstowcs", 0, 0 };

static uint32_t rng = 0xB0075001u;

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
	const auto &pc = port::global_locale()->components[1];
	const auto &rc = ref_global_locale.components[1];
	const port::mbstate_t *pm;
	const ref_mbstate_t *rm;

	if (pc == nullptr || rc == nullptr)
		return (pc == rc);
	pm = &reinterpret_cast<const struct {
		port::mbstate_t mbrtowc;
		port::mbstate_t c32rtomb;
		port::mbstate_t wcrtomb;
	} *>(pc)->mbrtowc;
	rm = &reinterpret_cast<const struct xlocale_ctype *>(rc)->mbrtowc;
	if (!state_eq(pm[0], rm[0]) || !state_eq(pm[1], rm[1]) ||
	    !state_eq(pm[2], rm[2]))
		return (false);
	return (true);
}

static void
report_fail(Stats &st, const char *tag)
{
	st.fails++;
	if (st.fails <= 5)
		std::printf("FAIL %s [%s]\n", st.name, tag);
}

static bool
compare_collate(Stats &st, char c1, char c2)
{
	int pr, rr;

	st.cases++;
	pr = port::__collate_range_cmp(c1, c2);
	rr = ref___collate_range_cmp(c1, c2);
	if (pr != rr) {
		report_fail(st, "ret");
		return (false);
	}
	return (true);
}

static bool
compare_wcollate(Stats &st, wchar_t c1, wchar_t c2)
{
	int pr, rr;

	st.cases++;
	pr = port::__wcollate_range_cmp(c1, c2);
	rr = ref___wcollate_range_cmp(c1, c2);
	if (pr != rr) {
		report_fail(st, "ret");
		return (false);
	}
	return (true);
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
compare_mbrtowc_l(Stats &st, const unsigned char *in, size_t n, bool use_l,
    bool null_ps, bool null_pwc, bool null_locale, bool null_s)
{
	unsigned char pin[64], rin[64];
	unsigned char pwcbuf[16], rwcbuf[16];
	port::mbstate_t pstate;
	ref_mbstate_t rstate;
	wchar_t pw, rw;
	size_t pr, rr;
	int perrno, rerrno;

	st.cases++;
	zero_state(&pstate);
	zero_state(&rstate);
	fill_guard(pin, sizeof(pin));
	fill_guard(rin, sizeof(rin));
	fill_guard(pwcbuf, sizeof(pwcbuf));
	fill_guard(rwcbuf, sizeof(rwcbuf));
	if (in != nullptr && !null_s) {
		memcpy(pin + 8, in, n);
		memcpy(rin + 8, in, n);
	}

	errno = 0;
	if (use_l) {
		pr = port::mbrtowc_l(null_pwc ? nullptr : (wchar_t *)(pwcbuf + 4),
		    null_s ? nullptr : (const char *)(pin + 8), n,
		    null_ps ? nullptr : &pstate,
		    null_locale ? nullptr : port::global_locale());
		perrno = errno;
		errno = 0;
		rr = ref_mbrtowc_l(null_pwc ? nullptr : (wchar_t *)(rwcbuf + 4),
		    null_s ? nullptr : (const char *)(rin + 8), n,
		    null_ps ? nullptr : &rstate,
		    null_locale ? nullptr : &ref_global_locale);
		rerrno = errno;
	} else {
		pr = port::mbrtowc(null_pwc ? nullptr : (wchar_t *)(pwcbuf + 4),
		    null_s ? nullptr : (const char *)(pin + 8), n,
		    null_ps ? nullptr : &pstate);
		perrno = errno;
		errno = 0;
		rr = ref_mbrtowc(null_pwc ? nullptr : (wchar_t *)(rwcbuf + 4),
		    null_s ? nullptr : (const char *)(rin + 8), n,
		    null_ps ? nullptr : &rstate);
		rerrno = errno;
	}

	memcpy(&pw, pwcbuf + 4, sizeof(pw));
	memcpy(&rw, rwcbuf + 4, sizeof(rw));

	if (pr != rr || perrno != rerrno) {
		report_fail(st, "ret");
		return (false);
	}
	if (in != nullptr && !null_s && memcmp(pin, rin, sizeof(pin)) != 0) {
		report_fail(st, "inbuf");
		return (false);
	}
	if (memcmp(pwcbuf, rwcbuf, sizeof(pwcbuf)) != 0) {
		report_fail(st, "wcharbuf");
		return (false);
	}
	if (pw != rw) {
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
compare_c32rtomb_l(Stats &st, char32_t c, bool use_l, bool null_ps,
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
		pr = port::c32rtomb_l(null_s ? nullptr : (char *)(pout + 4), c,
		    null_ps ? nullptr : &pstate,
		    null_locale ? nullptr : port::global_locale());
		perrno = errno;
		errno = 0;
		rr = ref_c32rtomb_l(null_s ? nullptr : (char *)(rout + 4), c,
		    null_ps ? nullptr : &rstate,
		    null_locale ? nullptr : &ref_global_locale);
		rerrno = errno;
	} else {
		pr = port::c32rtomb(null_s ? nullptr : (char *)(pout + 4), c,
		    null_ps ? nullptr : &pstate);
		perrno = errno;
		errno = 0;
		rr = ref_c32rtomb(null_s ? nullptr : (char *)(rout + 4), c,
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
compare_mbstowcs_l(Stats &st, const char *s, size_t n, bool use_l,
    bool null_pwcs, bool null_locale)
{
	unsigned char pout[WCS_CAP * sizeof(wchar_t)];
	unsigned char rout[WCS_CAP * sizeof(wchar_t)];
	size_t pr, rr;
	int perrno, rerrno;

	st.cases++;
	if (!null_pwcs) {
		fill_guard(pout, sizeof(pout));
		fill_guard(rout, sizeof(rout));
	}

	errno = 0;
	if (use_l) {
		pr = port::mbstowcs_l(null_pwcs ? nullptr :
		    (wchar_t *)(pout + 8), s, n,
		    null_locale ? nullptr : port::global_locale());
		perrno = errno;
		errno = 0;
		rr = ref_mbstowcs_l(null_pwcs ? nullptr :
		    (wchar_t *)(rout + 8), s, n,
		    null_locale ? nullptr : &ref_global_locale);
		rerrno = errno;
	} else {
		pr = port::mbstowcs(null_pwcs ? nullptr :
		    (wchar_t *)(pout + 8), s, n);
		perrno = errno;
		errno = 0;
		rr = ref_mbstowcs(null_pwcs ? nullptr :
		    (wchar_t *)(rout + 8), s, n);
		rerrno = errno;
	}

	if (pr != rr || perrno != rerrno) {
		report_fail(st, "ret");
		return (false);
	}
	if (null_pwcs)
		return (true);
	if (memcmp(pout, rout, sizeof(pout)) != 0) {
		report_fail(st, "buf");
		return (false);
	}
	return (true);
}

static void
hand_collate()
{
	compare_collate(st_collate, '\0', '\0');
	compare_collate(st_collate, 'a', 'a');
	compare_collate(st_collate, 'a', 'b');
	compare_collate(st_collate, 'z', 'a');
	compare_collate(st_collate, (char)0x7f, (char)0x7f);
	compare_collate(st_collate, (char)0x80, (char)0x80);
	compare_collate(st_collate, (char)0xff, (char)0xfe);
	compare_collate(st_collate, (char)0x01, (char)0xff);
	compare_collate(st_collate, 'A', 'a');
	compare_collate(st_collate, '0', '9');
}

static void
hand_wcollate()
{
	compare_wcollate(st_wcollate, L'\0', L'\0');
	compare_wcollate(st_wcollate, L'a', L'a');
	compare_wcollate(st_wcollate, L'a', L'b');
	compare_wcollate(st_wcollate, L'\x7f', L'\x7f');
	compare_wcollate(st_wcollate, (wchar_t)0x80, (wchar_t)0x80);
	compare_wcollate(st_wcollate, (wchar_t)0xffff, (wchar_t)0xfffe);
	compare_wcollate(st_wcollate, (wchar_t)0x20ac, (wchar_t)0x20ac);
	compare_wcollate(st_wcollate, L'A', L'a');
}

static void
hand_mbrtowc(Stats &st, bool use_l)
{
	unsigned char seq[8];
	size_t n;

	seq[0] = '\0';
	compare_mbrtowc_l(st, seq, 0, use_l, false, false, false, false);
	compare_mbrtowc_l(st, seq, 1, use_l, false, false, false, false);
	seq[0] = 'A';
	compare_mbrtowc_l(st, seq, 0, use_l, false, false, false, false);
	compare_mbrtowc_l(st, seq, 1, use_l, false, false, false, false);
	compare_mbrtowc_l(st, seq, 2, use_l, false, false, false, false);
	n = utf8_encode(0x80, seq, sizeof(seq));
	compare_mbrtowc_l(st, seq, n, use_l, false, false, false, false);
	compare_mbrtowc_l(st, seq, 1, use_l, false, false, false, false);
	n = utf8_encode(0x7FF, seq, sizeof(seq));
	compare_mbrtowc_l(st, seq, n, use_l, false, false, false, false);
	n = utf8_encode(0x20AC, seq, sizeof(seq));
	compare_mbrtowc_l(st, seq, n, use_l, false, false, false, false);
	n = utf8_encode(0x10000, seq, sizeof(seq));
	compare_mbrtowc_l(st, seq, n, use_l, false, false, false, false);
	compare_mbrtowc_l(st, seq, 1, use_l, false, false, false, false);
	seq[0] = 0xFF;
	compare_mbrtowc_l(st, seq, 1, use_l, false, false, false, false);
	seq[0] = 0xC0;
	seq[1] = 0x80;
	compare_mbrtowc_l(st, seq, 2, use_l, false, false, false, false);
	compare_mbrtowc_l(st, seq, 1, use_l, false, true, false, false);
	compare_mbrtowc_l(st, seq, 1, use_l, true, false, false, false);
	compare_mbrtowc_l(st, seq, 1, use_l, false, false, true, false);
	compare_mbrtowc_l(st, nullptr, 0, use_l, false, false, false, true);
	compare_mbrtowc_l(st, seq, 2, use_l, true, true, true);
}

static void
hand_c32rtomb(Stats &st, bool use_l)
{
	compare_c32rtomb_l(st, 0, use_l, false, false, false);
	compare_c32rtomb_l(st, 'a', use_l, false, false, false);
	compare_c32rtomb_l(st, (char32_t)0x7f, use_l, false, false, false);
	compare_c32rtomb_l(st, (char32_t)0x80, use_l, false, false, false);
	compare_c32rtomb_l(st, (char32_t)0x7ff, use_l, false, false, false);
	compare_c32rtomb_l(st, (char32_t)0x800, use_l, false, false, false);
	compare_c32rtomb_l(st, (char32_t)0xffff, use_l, false, false, false);
	compare_c32rtomb_l(st, (char32_t)0x10000, use_l, false, false, false);
	compare_c32rtomb_l(st, (char32_t)0x10ffff, use_l, false, false, false);
	compare_c32rtomb_l(st, (char32_t)0xd7ff, use_l, false, false, false);
	compare_c32rtomb_l(st, (char32_t)0xe000, use_l, false, false, false);
	compare_c32rtomb_l(st, (char32_t)0xd800, use_l, false, false, false);
	compare_c32rtomb_l(st, (char32_t)0xdfff, use_l, false, false, false);
	compare_c32rtomb_l(st, (char32_t)0x110000, use_l, false, false, false);
	compare_c32rtomb_l(st, (char32_t)0xFFFFFFFFu, use_l, false, false, false);
	compare_c32rtomb_l(st, 'x', use_l, false, true, false, false);
	compare_c32rtomb_l(st, 'q', use_l, true, false, false);
	compare_c32rtomb_l(st, 'z', use_l, false, false, true);
	compare_c32rtomb_l(st, 'w', use_l, true, true, true);
}

static void
hand_mbstowcs(Stats &st, bool use_l)
{
	static const char empty[] = "";
	static const char one[] = "a";
	static const char two[] = "ab";
	static const char euro[] = "\xe2\x82\xac";
	static const char plane[] = "\xf0\x90\x8d\x88";
	static const char bad[] = "\xff";
	static const char nulheavy[] = "\0\0a\0";

	compare_mbstowcs_l(st, empty, 0, use_l, false, false);
	compare_mbstowcs_l(st, empty, 1, use_l, false, false);
	compare_mbstowcs_l(st, one, 0, use_l, false, false);
	compare_mbstowcs_l(st, one, 1, use_l, false, false);
	compare_mbstowcs_l(st, one, 2, use_l, false, false);
	compare_mbstowcs_l(st, two, 1, use_l, false, false);
	compare_mbstowcs_l(st, two, 2, use_l, false, false);
	compare_mbstowcs_l(st, euro, 1, use_l, false, false);
	compare_mbstowcs_l(st, euro, 2, use_l, false, false);
	compare_mbstowcs_l(st, euro, 3, use_l, false, false);
	compare_mbstowcs_l(st, plane, 4, use_l, false, false);
	compare_mbstowcs_l(st, bad, 1, use_l, false, false);
	compare_mbstowcs_l(st, nulheavy, 4, use_l, false, false);
	compare_mbstowcs_l(st, one, 1, use_l, true, false);
	compare_mbstowcs_l(st, one, 1, use_l, false, true);
	compare_mbstowcs_l(st, two, 1, use_l, true, true);
}

static void
random_collate()
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		char c1 = (char)(xorshift32() & 0xffu);
		char c2 = (char)(xorshift32() & 0xffu);

		if (!compare_collate(st_collate, c1, c2))
			return;
	}
}

static void
random_wcollate()
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		wchar_t c1 = (wchar_t)(xorshift32() & 0xffffu);
		wchar_t c2 = (wchar_t)(xorshift32() & 0xffffu);

		if (!compare_wcollate(st_wcollate, c1, c2))
			return;
	}
}

static void
random_mbrtowc(Stats &st, bool use_l)
{
	unsigned char seq[16];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		size_t n = (size_t)(xorshift32() % (sizeof(seq) + 1));

		for (size_t j = 0; j < n; j++)
			seq[j] = (unsigned char)(xorshift32() & 0xffu);
		if (!compare_mbrtowc_l(st, seq, n, use_l,
		    (xorshift32() & 7u) == 0, (xorshift32() & 7u) == 0,
		    (xorshift32() & 0x3ffu) == 0,
		    (xorshift32() & 0x1ffu) == 0))
			return;
	}
}

static void
random_c32rtomb(Stats &st, bool use_l)
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		char32_t c = (char32_t)(xorshift32() & 0x1fffffu);

		if (!compare_c32rtomb_l(st, c, use_l,
		    (xorshift32() & 7u) == 0, (xorshift32() & 3u) != 0,
		    (xorshift32() & 0x7ffu) == 0))
			return;
	}
}

static void
random_mbstowcs(Stats &st, bool use_l)
{
	char seq[24];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		size_t len = (size_t)(xorshift32() % (sizeof(seq) - 1));
		size_t n = (size_t)(xorshift32() % (WCS_CAP + 1));

		for (size_t j = 0; j < len; j++)
			seq[j] = (char)(xorshift32() & 0xffu);
		seq[len] = '\0';
		if (!compare_mbstowcs_l(st, seq, n, use_l,
		    (xorshift32() & 3u) != 0, (xorshift32() & 0x3ffu) == 0))
			return;
	}
}

static void
print_row(const Stats &st)
{
	std::printf("%-24s %12llu %12llu\n", st.name, st.cases, st.fails);
}

int
main()
{
	init_locales();

	hand_collate();
	hand_wcollate();
	hand_mbrtowc(st_mbrtowc_l, true);
	hand_mbrtowc(st_mbrtowc, false);
	hand_c32rtomb(st_c32rtomb_l, true);
	hand_c32rtomb(st_c32rtomb, false);
	hand_mbstowcs(st_mbstowcs_l, true);
	hand_mbstowcs(st_mbstowcs, false);

	random_collate();
	random_wcollate();
	random_mbrtowc(st_mbrtowc_l, true);
	random_mbrtowc(st_mbrtowc, false);
	random_c32rtomb(st_c32rtomb_l, true);
	random_c32rtomb(st_c32rtomb, false);
	random_mbstowcs(st_mbstowcs_l, true);
	random_mbstowcs(st_mbstowcs, false);

	std::printf("\n%-24s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-24s %12s %12s\n", "--------", "-----", "--------");
	print_row(st_collate);
	print_row(st_wcollate);
	print_row(st_mbrtowc_l);
	print_row(st_mbrtowc);
	print_row(st_c32rtomb_l);
	print_row(st_c32rtomb);
	print_row(st_mbstowcs_l);
	print_row(st_mbstowcs);

	if (st_collate.fails != 0 || st_wcollate.fails != 0 ||
	    st_mbrtowc_l.fails != 0 || st_mbrtowc.fails != 0 ||
	    st_c32rtomb_l.fails != 0 || st_c32rtomb.fails != 0 ||
	    st_mbstowcs_l.fails != 0 || st_mbstowcs.fails != 0)
		return (1);
	return (0);
}
