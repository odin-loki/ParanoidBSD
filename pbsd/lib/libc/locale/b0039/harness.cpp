/*
 * Differential harness for batch b0039.
 */

import pbsd.lib.libc.locale.b0039;

#include <climits>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::lib_libc_locale::b0039;

typedef union {
	char		__bytes[128];
	long long	__align;
} ref_mbstate_t;

extern "C" {
typedef struct xlocale *ref_locale_t;

extern struct xlocale ref_global_locale;
extern struct xlocale_ctype ref_global_ctype;

size_t ref_mbsrtowcs_l(wchar_t *, const char **, size_t, ref_mbstate_t *,
    ref_locale_t);
size_t ref_mbsrtowcs(wchar_t *, const char **, size_t, ref_mbstate_t *);
size_t ref_wcsrtombs_l(char *, const wchar_t **, size_t, ref_mbstate_t *,
    ref_locale_t);
size_t ref_wcsrtombs(char *, const wchar_t **, size_t, ref_mbstate_t *);
int ref_mbtowc_l(wchar_t *, const char *, size_t, ref_locale_t);
int ref_mbtowc(wchar_t *, const char *, size_t);
int ref_wcwidth(wchar_t);
int ref_wcwidth_l(wchar_t, ref_locale_t);
}

struct xlocale_ctype {
	ref_mbstate_t	mbsrtowcs;
	ref_mbstate_t	wcsrtombs;
	ref_mbstate_t	mbtowc;
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
static constexpr size_t WBUF_CAP = 64;
static constexpr size_t MBUF_CAP = 128;
static constexpr unsigned long long RANDOM_ITERS = 200000;

static Stats st_mbsrtowcs_l = { "mbsrtowcs_l", 0, 0 };
static Stats st_mbsrtowcs = { "mbsrtowcs", 0, 0 };
static Stats st_wcsrtombs_l = { "wcsrtombs_l", 0, 0 };
static Stats st_wcsrtombs = { "wcsrtombs", 0, 0 };
static Stats st_mbtowc_l = { "mbtowc_l", 0, 0 };
static Stats st_mbtowc = { "mbtowc", 0, 0 };
static Stats st_wcwidth = { "wcwidth", 0, 0 };
static Stats st_wcwidth_l = { "wcwidth_l", 0, 0 };

static uint32_t rng = 0xB0039002u;

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
zero_state(port::mbstate_t *ps)
{
	memset(ps, 0, sizeof(*ps));
}

static void
zero_state(ref_mbstate_t *ps)
{
	memset(ps, 0, sizeof(*ps));
}

static void
fill_guard(unsigned char *buf, size_t n)
{
	memset(buf, GUARD, n);
}

static size_t
utf8_encode(uint32_t cp, unsigned char *out, size_t cap)
{
	if (cp < 0x80 && cap >= 1) {
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

static void
report_fail(Stats &st, const char *tag)
{
	st.fails++;
	if (st.fails <= 5)
		std::printf("FAIL %s [%s]\n", st.name, tag);
}

static bool
compare_mbsrtowcs(Stats &st, const unsigned char *in, size_t inlen, size_t len,
    bool use_l, bool null_ps, bool null_dst)
{
	unsigned char pmb[MBUF_CAP], rmb[MBUF_CAP];
	unsigned char pwd[WBUF_CAP * sizeof(wchar_t)],
	    rwd[WBUF_CAP * sizeof(wchar_t)];
	const char *psrc, *rsrc;
	port::mbstate_t pstate;
	ref_mbstate_t rstate;
	size_t pr, rr;
	wchar_t *pdst, *rdst;

	st.cases++;
	zero_state(&pstate);
	zero_state(&rstate);
	fill_guard(pmb, sizeof(pmb));
	fill_guard(rmb, sizeof(rmb));
	fill_guard(pwd, sizeof(pwd));
	fill_guard(rwd, sizeof(rwd));
	memcpy(pmb + 8, in, inlen);
	pmb[8 + inlen] = '\0';
	memcpy(rmb + 8, in, inlen);
	rmb[8 + inlen] = '\0';
	psrc = (const char *)(pmb + 8);
	rsrc = (const char *)(rmb + 8);
	pdst = null_dst ? nullptr : (wchar_t *)(pwd + 8);
	rdst = null_dst ? nullptr : (wchar_t *)(rwd + 8);

	errno = 0;
	if (use_l) {
		pr = port::mbsrtowcs_l(pdst, &psrc, len,
		    null_ps ? nullptr : &pstate, port::global_locale());
		rr = ref_mbsrtowcs_l(rdst, &rsrc, len,
		    null_ps ? nullptr : &rstate, &ref_global_locale);
	} else {
		pr = port::mbsrtowcs(pdst, &psrc, len,
		    null_ps ? nullptr : &pstate);
		rr = ref_mbsrtowcs(rdst, &rsrc, len,
		    null_ps ? nullptr : &rstate);
	}

	if (pr != rr)
		return (report_fail(st, "ret"), false);
	if (psrc - (const char *)(pmb + 8) != rsrc - (const char *)(rmb + 8))
		return (report_fail(st, "srcoff"), false);
	if (memcmp(pmb, rmb, sizeof(pmb)) != 0)
		return (report_fail(st, "mbuf"), false);
	if (memcmp(pwd, rwd, sizeof(pwd)) != 0)
		return (report_fail(st, "wbuf"), false);
	return (true);
}

static bool
compare_wcsrtombs(Stats &st, const wchar_t *in, size_t inlen, size_t len,
    bool use_l, bool null_ps, bool null_dst)
{
	unsigned char pwd[WBUF_CAP * sizeof(wchar_t)],
	    rwd[WBUF_CAP * sizeof(wchar_t)];
	unsigned char pmb[MBUF_CAP], rmb[MBUF_CAP];
	const wchar_t *psrc, *rsrc;
	port::mbstate_t pstate;
	ref_mbstate_t rstate;
	size_t pr, rr;
	char *pdst, *rdst;

	st.cases++;
	zero_state(&pstate);
	zero_state(&rstate);
	fill_guard(pwd, sizeof(pwd));
	fill_guard(rwd, sizeof(rwd));
	fill_guard(pmb, sizeof(pmb));
	fill_guard(rmb, sizeof(rmb));
	memcpy(pwd + 8, in, inlen * sizeof(wchar_t));
	memcpy(rwd + 8, in, inlen * sizeof(wchar_t));
	psrc = (const wchar_t *)(pwd + 8);
	rsrc = (const wchar_t *)(rwd + 8);
	pdst = null_dst ? nullptr : (char *)(pmb + 8);
	rdst = null_dst ? nullptr : (char *)(rmb + 8);

	errno = 0;
	if (use_l) {
		pr = port::wcsrtombs_l(pdst, &psrc, len,
		    null_ps ? nullptr : &pstate, port::global_locale());
		rr = ref_wcsrtombs_l(rdst, &rsrc, len,
		    null_ps ? nullptr : &rstate, &ref_global_locale);
	} else {
		pr = port::wcsrtombs(pdst, &psrc, len,
		    null_ps ? nullptr : &pstate);
		rr = ref_wcsrtombs(rdst, &rsrc, len,
		    null_ps ? nullptr : &rstate);
	}

	if (pr != rr)
		return (report_fail(st, "ret"), false);
	if (psrc - (const wchar_t *)(pwd + 8) != rsrc - (const wchar_t *)(rwd + 8))
		return (report_fail(st, "srcoff"), false);
	if (memcmp(pmb, rmb, sizeof(pmb)) != 0)
		return (report_fail(st, "mbuf"), false);
	if (memcmp(pwd, rwd, sizeof(pwd)) != 0)
		return (report_fail(st, "wbuf"), false);
	return (true);
}

static bool
compare_mbtowc(Stats &st, const unsigned char *in, size_t n, bool use_l,
    bool null_pwc, bool null_s)
{
	unsigned char pin[64], rin[64];
	unsigned char pwcbuf[16], rwcbuf[16];
	port::mbstate_t pstate;
	ref_mbstate_t rstate;
	wchar_t pwc, rwc;
	int pr, rr, perrno, rerrno;

	st.cases++;
	zero_state(&pstate);
	zero_state(&rstate);
	fill_guard(pin, sizeof(pin));
	fill_guard(rin, sizeof(rin));
	fill_guard(pwcbuf, sizeof(pwcbuf));
	fill_guard(rwcbuf, sizeof(rwcbuf));
	if (!null_s) {
		memcpy(pin + 8, in, n);
		memcpy(rin + 8, in, n);
	}

	errno = 0;
	if (use_l) {
		pr = port::mbtowc_l(null_pwc ? nullptr : (wchar_t *)(pwcbuf + 4),
		    null_s ? nullptr : (const char *)(pin + 8), n,
		    port::global_locale());
		perrno = errno;
		errno = 0;
		rr = ref_mbtowc_l(null_pwc ? nullptr : (wchar_t *)(rwcbuf + 4),
		    null_s ? nullptr : (const char *)(rin + 8), n,
		    &ref_global_locale);
		rerrno = errno;
	} else {
		pr = port::mbtowc(null_pwc ? nullptr : (wchar_t *)(pwcbuf + 4),
		    null_s ? nullptr : (const char *)(pin + 8), n);
		perrno = errno;
		errno = 0;
		rr = ref_mbtowc(null_pwc ? nullptr : (wchar_t *)(rwcbuf + 4),
		    null_s ? nullptr : (const char *)(rin + 8), n);
		rerrno = errno;
	}

	if (pr != rr || perrno != rerrno)
		return (report_fail(st, "ret"), false);
	if (memcmp(pwcbuf, rwcbuf, sizeof(pwcbuf)) != 0)
		return (report_fail(st, "wbuf"), false);
	if (!null_pwc) {
		memcpy(&pwc, pwcbuf + 4, sizeof(pwc));
		memcpy(&rwc, rwcbuf + 4, sizeof(rwc));
		if (pwc != rwc)
			return (report_fail(st, "wchar"), false);
	}
	return (true);
}

static bool
compare_wcwidth_fn(Stats &st, wchar_t wc, bool use_l)
{
	int pr, rr;

	st.cases++;
	errno = 0;
	if (use_l) {
		pr = port::wcwidth_l(wc, port::global_locale());
		rr = ref_wcwidth_l(wc, &ref_global_locale);
	} else {
		pr = port::wcwidth(wc);
		rr = ref_wcwidth(wc);
	}
	if (pr != rr)
		return (report_fail(st, "ret"), false);
	return (true);
}

static void
hand_mbsrtowcs(Stats &st, bool use_l)
{
	unsigned char seq[32];
	size_t n;

	seq[0] = '\0';
	compare_mbsrtowcs(st, seq, 0, 0, use_l, false, true);
	compare_mbsrtowcs(st, seq, 0, 1, use_l, false, false);
	compare_mbsrtowcs(st, seq, 0, 0, use_l, true, false);
	seq[0] = 'A';
	compare_mbsrtowcs(st, seq, 1, 1, use_l, false, false);
	seq[0] = '\0';
	seq[1] = 'B';
	compare_mbsrtowcs(st, seq, 2, 4, use_l, false, false);
	n = utf8_encode(0x80, seq, sizeof(seq));
	compare_mbsrtowcs(st, seq, n, 2, use_l, false, false);
	n = utf8_encode(0x7FF, seq, sizeof(seq));
	compare_mbsrtowcs(st, seq, n, 2, use_l, false, false);
	n = utf8_encode(0x10000, seq, sizeof(seq));
	compare_mbsrtowcs(st, seq, n, 2, use_l, false, false);
	seq[0] = 0xFF;
	compare_mbsrtowcs(st, seq, 1, 4, use_l, false, false);
	seq[0] = 0xC0;
	seq[1] = 0x80;
	compare_mbsrtowcs(st, seq, 2, 4, use_l, false, false);
	seq[0] = 0xF0;
	seq[1] = 0x90;
	seq[2] = 0x80;
	seq[3] = 0x80;
	compare_mbsrtowcs(st, seq, 4, 4, use_l, false, false);
	compare_mbsrtowcs(st, seq, 4, 1, use_l, false, false);
	compare_mbsrtowcs(st, seq, 4, 0, use_l, false, false);
}

static void
hand_wcsrtombs(Stats &st, bool use_l)
{
	wchar_t seq[16];

	seq[0] = L'\0';
	compare_wcsrtombs(st, seq, 0, 0, use_l, false, true);
	compare_wcsrtombs(st, seq, 0, 1, use_l, false, false);
	compare_wcsrtombs(st, seq, 0, 0, use_l, true, false);
	seq[0] = L'A';
	compare_wcsrtombs(st, seq, 1, 1, use_l, false, false);
	seq[0] = L'\0';
	seq[1] = L'B';
	compare_wcsrtombs(st, seq, 2, 8, use_l, false, false);
	seq[0] = 0x80;
	compare_wcsrtombs(st, seq, 1, 8, use_l, false, false);
	seq[0] = 0x7FF;
	compare_wcsrtombs(st, seq, 1, 8, use_l, false, false);
	seq[0] = 0x10000;
	compare_wcsrtombs(st, seq, 1, 8, use_l, false, false);
	seq[0] = (wchar_t)0x10FFFF;
	compare_wcsrtombs(st, seq, 1, 8, use_l, false, false);
	seq[0] = (wchar_t)0x110000;
	compare_wcsrtombs(st, seq, 1, 8, use_l, false, false);
	compare_wcsrtombs(st, seq, 1, 1, use_l, false, false);
	compare_wcsrtombs(st, seq, 1, 0, use_l, false, false);
}

static void
hand_mbtowc(Stats &st, bool use_l)
{
	unsigned char seq[8];
	size_t n;

	compare_mbtowc(st, nullptr, 0, use_l, false, true);
	seq[0] = '\0';
	compare_mbtowc(st, seq, 1, use_l, false, false);
	seq[0] = 'Z';
	compare_mbtowc(st, seq, 1, use_l, false, false);
	n = utf8_encode(0x80, seq, sizeof(seq));
	compare_mbtowc(st, seq, n, use_l, false, false);
	n = utf8_encode(0x10000, seq, sizeof(seq));
	compare_mbtowc(st, seq, n, use_l, false, false);
	compare_mbtowc(st, seq, 1, use_l, false, false);
	seq[0] = 0xFF;
	compare_mbtowc(st, seq, 1, use_l, false, false);
	seq[0] = 0xC0;
	seq[1] = 0x80;
	compare_mbtowc(st, seq, 2, use_l, false, false);
	seq[0] = 0xF0;
	seq[1] = 0x90;
	compare_mbtowc(st, seq, 2, use_l, false, false);
	compare_mbtowc(st, seq, 0, use_l, false, false);
	compare_mbtowc(st, seq, 1, use_l, true, false);
}

static void
hand_wcwidth(Stats &st, bool use_l)
{
	compare_wcwidth_fn(st, 0, use_l);
	compare_wcwidth_fn(st, L'A', use_l);
	compare_wcwidth_fn(st, L'\t', use_l);
	compare_wcwidth_fn(st, 0x7f, use_l);
	compare_wcwidth_fn(st, 0x80, use_l);
	compare_wcwidth_fn(st, 0x7FF, use_l);
	compare_wcwidth_fn(st, 0x1100, use_l);
	compare_wcwidth_fn(st, 0xAC00, use_l);
	compare_wcwidth_fn(st, 0xD7A3, use_l);
	compare_wcwidth_fn(st, 0x2000, use_l);
}

static void
random_mbsrtowcs(Stats &st, bool use_l)
{
	unsigned char seq[MB_LEN_MAX * 4];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		size_t inlen = (size_t)(xorshift32() % (sizeof(seq) + 1));

		for (size_t j = 0; j < inlen; j++)
			seq[j] = (unsigned char)(xorshift32() & 0xFFu);
		if (!compare_mbsrtowcs(st, seq, inlen,
		    (size_t)(xorshift32() % (WBUF_CAP + 1)),
		    use_l, (xorshift32() & 7u) == 0,
		    (xorshift32() & 15u) == 0))
			return;
	}
}

static void
random_wcsrtombs(Stats &st, bool use_l)
{
	wchar_t seq[32];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		size_t inlen = (size_t)(xorshift32() % (sizeof(seq) / sizeof(seq[0]) + 1));

		for (size_t j = 0; j < inlen; j++) {
			if ((xorshift32() & 3u) == 0)
				seq[j] = (wchar_t)(xorshift32() & 0xFFFFu);
			else
				seq[j] = (wchar_t)xorshift32();
		}
		if (!compare_wcsrtombs(st, seq, inlen,
		    (size_t)(xorshift32() % (MBUF_CAP + 1)),
		    use_l, (xorshift32() & 7u) == 0,
		    (xorshift32() & 15u) == 0))
			return;
	}
}

static void
random_mbtowc(Stats &st, bool use_l)
{
	unsigned char seq[MB_LEN_MAX];

	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		size_t n = (size_t)(xorshift32() % (MB_LEN_MAX + 1));

		for (size_t j = 0; j < n; j++)
			seq[j] = (unsigned char)(xorshift32() & 0xFFu);
		if (!compare_mbtowc(st, seq, n, use_l,
		    (xorshift32() & 7u) == 0, false))
			return;
		if ((xorshift32() & 0x3FFu) == 0)
			compare_mbtowc(st, nullptr, 0, use_l, false, true);
	}
}

static void
random_wcwidth(Stats &st, bool use_l)
{
	for (unsigned long long i = 0; i < RANDOM_ITERS; i++) {
		wchar_t wc;

		if ((xorshift32() & 3u) == 0)
			wc = (wchar_t)(xorshift32() & 0xFFFFu);
		else
			wc = (wchar_t)xorshift32();
		if (!compare_wcwidth_fn(st, wc, use_l))
			return;
	}
}

int
main()
{
	init_locales();

	hand_mbsrtowcs(st_mbsrtowcs_l, true);
	hand_mbsrtowcs(st_mbsrtowcs, false);
	hand_wcsrtombs(st_wcsrtombs_l, true);
	hand_wcsrtombs(st_wcsrtombs, false);
	hand_mbtowc(st_mbtowc_l, true);
	hand_mbtowc(st_mbtowc, false);
	hand_wcwidth(st_wcwidth, false);
	hand_wcwidth(st_wcwidth_l, true);

	random_mbsrtowcs(st_mbsrtowcs_l, true);
	random_mbsrtowcs(st_mbsrtowcs, false);
	random_wcsrtombs(st_wcsrtombs_l, true);
	random_wcsrtombs(st_wcsrtombs, false);
	random_mbtowc(st_mbtowc_l, true);
	random_mbtowc(st_mbtowc, false);
	random_wcwidth(st_wcwidth, false);
	random_wcwidth(st_wcwidth_l, true);

	std::printf("\n%-16s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-16s %12s %12s\n", "--------", "-----", "--------");
	std::printf("%-16s %12llu %12llu\n",
	    st_mbsrtowcs_l.name, st_mbsrtowcs_l.cases, st_mbsrtowcs_l.fails);
	std::printf("%-16s %12llu %12llu\n",
	    st_mbsrtowcs.name, st_mbsrtowcs.cases, st_mbsrtowcs.fails);
	std::printf("%-16s %12llu %12llu\n",
	    st_wcsrtombs_l.name, st_wcsrtombs_l.cases, st_wcsrtombs_l.fails);
	std::printf("%-16s %12llu %12llu\n",
	    st_wcsrtombs.name, st_wcsrtombs.cases, st_wcsrtombs.fails);
	std::printf("%-16s %12llu %12llu\n",
	    st_mbtowc_l.name, st_mbtowc_l.cases, st_mbtowc_l.fails);
	std::printf("%-16s %12llu %12llu\n",
	    st_mbtowc.name, st_mbtowc.cases, st_mbtowc.fails);
	std::printf("%-16s %12llu %12llu\n",
	    st_wcwidth.name, st_wcwidth.cases, st_wcwidth.fails);
	std::printf("%-16s %12llu %12llu\n",
	    st_wcwidth_l.name, st_wcwidth_l.cases, st_wcwidth_l.fails);

	if (st_mbsrtowcs_l.fails != 0 || st_mbsrtowcs.fails != 0 ||
	    st_wcsrtombs_l.fails != 0 || st_wcsrtombs.fails != 0 ||
	    st_mbtowc_l.fails != 0 || st_mbtowc.fails != 0 ||
	    st_wcwidth.fails != 0 || st_wcwidth_l.fails != 0)
		return (1);
	return (0);
}
