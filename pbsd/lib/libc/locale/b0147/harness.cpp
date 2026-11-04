/*
 * Differential harness for batch b0147.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>

import pbsd.lib.libc.locale.b0147;

namespace P = pbsd::lib_libc_locale::b0147;

extern "C" {
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} ref_mbstate_t;

typedef int ref_rune_t;

typedef struct {
	ref_rune_t	__min;
	ref_rune_t	__max;
	ref_rune_t	__map;
	unsigned long	*__types;
} ref_rune_entry;

typedef struct {
	int		__nranges;
	ref_rune_entry	*__ranges;
} ref_rune_range;

typedef struct {
	char		__magic[8];
	char		__encoding[32];
	ref_rune_t	(*__sgetrune)(const char *, size_t, char const **);
	int		(*__sputrune)(ref_rune_t, char *, size_t, char **);
	ref_rune_t	__invalid_rune;
	unsigned long	__runetype[256];
	ref_rune_t	__maplower[256];
	ref_rune_t	__mapupper[256];
	ref_rune_range	__runetype_ext;
	ref_rune_range	__maplower_ext;
	ref_rune_range	__mapupper_ext;
	void		*__variable;
	int		__variable_len;
} ref_rune_locale;

struct ref_xlocale_ctype {
	size_t		(*__mbrtowc)(wchar_t *, const char *, size_t,
		    ref_mbstate_t *);
	int		(*__mbsinit)(const ref_mbstate_t *);
	size_t		(*__mbsnrtowcs)(wchar_t *, const char **, size_t, size_t,
		    ref_mbstate_t *);
	size_t		(*__wcrtomb)(char *, wchar_t, ref_mbstate_t *);
	size_t		(*__wcsnrtombs)(char *, const wchar_t **, size_t, size_t,
		    ref_mbstate_t *);
	ref_rune_locale	*runes;
	int		__mb_cur_max;
	int		__mb_sb_limit;
};

int	ref__BIG5_init(ref_xlocale_ctype *, ref_rune_locale *);
int	ref__BIG5_mbsinit(const ref_mbstate_t *);
int	ref__big5_check(unsigned int);
size_t	ref__BIG5_mbrtowc(wchar_t *, const char *, size_t, ref_mbstate_t *);
size_t	ref__BIG5_wcrtomb(char *, wchar_t, ref_mbstate_t *);
size_t	ref__BIG5_mbsnrtowcs(wchar_t *, const char **, size_t, size_t,
	    ref_mbstate_t *);
size_t	ref__BIG5_wcsnrtombs(char *, const wchar_t **, size_t, size_t,
	    ref_mbstate_t *);

int	ref__GBK_init(ref_xlocale_ctype *, ref_rune_locale *);
int	ref__GBK_mbsinit(const ref_mbstate_t *);
int	ref__gbk_check(unsigned int);
size_t	ref__GBK_mbrtowc(wchar_t *, const char *, size_t, ref_mbstate_t *);
size_t	ref__GBK_wcrtomb(char *, wchar_t, ref_mbstate_t *);
size_t	ref__GBK_mbsnrtowcs(wchar_t *, const char **, size_t, size_t,
	    ref_mbstate_t *);
size_t	ref__GBK_wcsnrtombs(char *, const wchar_t **, size_t, size_t,
	    ref_mbstate_t *);

int	ref__ascii_init(ref_xlocale_ctype *, ref_rune_locale *);
int	ref__ascii_mbsinit(const ref_mbstate_t *);
size_t	ref__ascii_mbrtowc(wchar_t *, const char *, size_t, ref_mbstate_t *);
size_t	ref__ascii_wcrtomb(char *, wchar_t, ref_mbstate_t *);
size_t	ref__ascii_mbsnrtowcs(wchar_t *, const char **, size_t, size_t,
	    ref_mbstate_t *);
size_t	ref__ascii_wcsnrtombs(char *, const wchar_t **, size_t, size_t,
	    ref_mbstate_t *);

int	ref__MSKanji_init(ref_xlocale_ctype *, ref_rune_locale *);
int	ref__MSKanji_mbsinit(const ref_mbstate_t *);
size_t	ref__MSKanji_mbrtowc(wchar_t *, const char *, size_t, ref_mbstate_t *);
size_t	ref__MSKanji_wcrtomb(char *, wchar_t, ref_mbstate_t *);
size_t	ref__MSKanji_mbsnrtowcs(wchar_t *, const char **, size_t, size_t,
	    ref_mbstate_t *);
size_t	ref__MSKanji_wcsnrtombs(char *, const wchar_t **, size_t, size_t,
	    ref_mbstate_t *);
}

enum {
	F_BIG5_INIT, F_BIG5_MBSINIT, F_BIG5_CHECK, F_BIG5_MBRTOWC,
	F_BIG5_WCRTOMB, F_BIG5_MBSNRTOWCS, F_BIG5_WCSNRTOMBS,
	F_GBK_INIT, F_GBK_MBSINIT, F_GBK_CHECK, F_GBK_MBRTOWC,
	F_GBK_WCRTOMB, F_GBK_MBSNRTOWCS, F_GBK_WCSNRTOMBS,
	F_ASCII_INIT, F_ASCII_MBSINIT, F_ASCII_MBRTOWC,
	F_ASCII_WCRTOMB, F_ASCII_MBSNRTOWCS, F_ASCII_WCSNRTOMBS,
	F_MSK_INIT, F_MSK_MBSINIT, F_MSK_MBRTOWC,
	F_MSK_WCRTOMB, F_MSK_MBSNRTOWCS, F_MSK_WCSNRTOMBS,
	F_COUNT
};

static const char *const fname[F_COUNT] = {
	"BIG5_init", "BIG5_mbsinit", "big5_check", "BIG5_mbrtowc",
	"BIG5_wcrtomb", "BIG5_mbsnrtowcs", "BIG5_wcsnrtombs",
	"GBK_init", "GBK_mbsinit", "gbk_check", "GBK_mbrtowc",
	"GBK_wcrtomb", "GBK_mbsnrtowcs", "GBK_wcsnrtombs",
	"ascii_init", "ascii_mbsinit", "ascii_mbrtowc",
	"ascii_wcrtomb", "ascii_mbsnrtowcs", "ascii_wcsnrtombs",
	"MSKanji_init", "MSKanji_mbsinit", "MSKanji_mbrtowc",
	"MSKanji_wcrtomb", "MSKanji_mbsnrtowcs", "MSKanji_wcsnrtombs"
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int nprinted[F_COUNT];

static constexpr unsigned char GUARD = 0x7f;
static constexpr wchar_t WGUARD = (wchar_t)0x7f7f;
static constexpr long long SWEEP = 200000;

static uint64_t rng = 0xB0147001ULL;

static uint64_t
rnd(void)
{
	rng ^= rng << 13;
	rng ^= rng >> 7;
	rng ^= rng << 17;
	return (rng);
}

static uint32_t
u32(uint32_t m)
{
	return ((uint32_t)(rnd() % m));
}

static void
report(int f, const char *why)
{
	nfail[f]++;
	if (nprinted[f]++ < 8)
		std::printf("  FAIL %-22s : %s\n", fname[f], why);
}

static void
mb_copy(const P::mbstate_t &s, ref_mbstate_t &d)
{
	std::memset(&d, 0, sizeof(d));
	std::memcpy(&d, &s, sizeof(ref_mbstate_t));
}

static void
mb_copy(const ref_mbstate_t &s, P::mbstate_t &d)
{
	std::memset(&d, 0, sizeof(d));
	std::memcpy(&d, &s, sizeof(ref_mbstate_t));
}

static bool
mb_eq(const P::mbstate_t &a, const ref_mbstate_t &b)
{
	return (std::memcmp(&a, &b, sizeof(ref_mbstate_t)) == 0);
}

static void
fill_guard(unsigned char *b, size_t n)
{
	std::memset(b, GUARD, n);
}

static void
fill_wguard(wchar_t *b, size_t n)
{
	for (size_t i = 0; i < n; i++)
		b[i] = WGUARD;
}

static bool
chk_ret(int f, size_t pv, size_t rv, int perrno, int rerrno)
{
	ncase[f]++;
	if (pv != rv || perrno != rerrno) {
		report(f, "ret/errno");
		return (false);
	}
	return (true);
}

static bool
chk_int(int f, int pv, int rv)
{
	ncase[f]++;
	if (pv != rv) {
		report(f, "return");
		return (false);
	}
	return (true);
}

static void
set_state_ch(P::mbstate_t &s, wchar_t ch)
{
	std::memcpy(&s, &ch, sizeof(ch));
}

static void
set_state_ch(ref_mbstate_t &s, wchar_t ch)
{
	std::memcpy(&s, &ch, sizeof(ch));
}

template<typename InitFn, typename RefInitFn>
static void
test_init(int f, InitFn init, RefInitFn ref_init, int mbmax, int sblimit)
{
	P::xlocale_ctype pl{};
	ref_xlocale_ctype rl{};
	P::_RuneLocale prl{};
	ref_rune_locale rrl{};

	int pr = init(&pl, &prl);
	int rr = ref_init(&rl, &rrl);
	if (!chk_int(f, pr, rr))
		return;
	if (pl.__mb_cur_max != rl.__mb_cur_max ||
	    pl.__mb_sb_limit != rl.__mb_sb_limit ||
	    pl.runes != &prl || rl.runes != &rrl) {
		report(f, "fields");
		return;
	}
	if (pl.__mb_cur_max != mbmax || pl.__mb_sb_limit != sblimit)
		report(f, "limits");
}

template<typename MbsinitFn, typename RefMbsinitFn>
static void
test_mbsinit(int f, MbsinitFn fn, RefMbsinitFn ref_fn)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};

	chk_int(f, fn(nullptr), ref_fn(nullptr));
	set_state_ch(ps, 0);
	set_state_ch(rs, 0);
	chk_int(f, fn(&ps), ref_fn(&rs));
	set_state_ch(ps, 1);
	set_state_ch(rs, 1);
	chk_int(f, fn(&ps), ref_fn(&rs));
	set_state_ch(ps, 0x123);
	set_state_ch(rs, 0x123);
	chk_int(f, fn(&ps), ref_fn(&rs));
}

template<typename CheckFn, typename RefCheckFn>
static void
edge_check(int f, CheckFn fn, RefCheckFn ref_fn)
{
	static const unsigned int vals[] = {
		0, 0x7f, 0x80, 0xa0, 0xa1, 0xfe, 0xff, 0x100, 0x181, 0x1fe
	};
	for (unsigned int v : vals) {
		chk_int(f, fn(v), ref_fn(v));
		chk_int(f, fn(v | 0xffffff00U), ref_fn(v | 0xffffff00U));
	}
}

template<typename CheckFn, typename RefCheckFn>
static void
sweep_check(int f, CheckFn fn, RefCheckFn ref_fn)
{
	for (long long i = 0; i < SWEEP; i++) {
		unsigned int v = (unsigned int)rnd();
		chk_int(f, fn(v), ref_fn(v));
	}
}

template<typename MbrFn, typename RefMbrFn>
static bool
run_mbr(int f, MbrFn fn, RefMbrFn ref_fn, const char *s, size_t n,
    P::mbstate_t &ps, ref_mbstate_t &rs, bool null_pwc)
{
	wchar_t pw{}, rw{};
	int pe, re;

	errno = 0;
	size_t pr = fn(null_pwc ? nullptr : &pw, s, n, &ps);
	pe = errno;
	errno = 0;
	size_t rr = ref_fn(null_pwc ? nullptr : &rw, s, n, &rs);
	re = errno;
	if (!chk_ret(f, pr, rr, pe, re))
		return (false);
	if (!null_pwc && pr != (size_t)-1 && pr != (size_t)-2 && pw != rw) {
		report(f, "wchar");
		return (false);
	}
	if (!mb_eq(ps, rs)) {
		report(f, "state");
		return (false);
	}
	return (true);
}

template<typename MbrFn, typename RefMbrFn>
static void
edge_mbr(int f, MbrFn fn, RefMbrFn ref_fn)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	char buf[8];

	/* s == NULL */
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	run_mbr(f, fn, ref_fn, nullptr, 0, ps, rs, true);
	run_mbr(f, fn, ref_fn, nullptr, 0, ps, rs, false);

	/* n == 0 */
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = 'a';
	run_mbr(f, fn, ref_fn, buf, 0, ps, rs, false);

	/* bad state */
	set_state_ch(ps, 0x100);
	set_state_ch(rs, 0x100);
	buf[0] = 'a'; buf[1] = '\0';
	run_mbr(f, fn, ref_fn, buf, 2, ps, rs, false);

	/* single ascii */
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = 'Z'; buf[1] = '\0';
	run_mbr(f, fn, ref_fn, buf, 2, ps, rs, false);

	/* NUL */
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = '\0';
	run_mbr(f, fn, ref_fn, buf, 1, ps, rs, false);

	/* high-bit lead incomplete */
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0xa1; buf[1] = (char)0xb2;
	run_mbr(f, fn, ref_fn, buf, 1, ps, rs, false);
	run_mbr(f, fn, ref_fn, buf, 2, ps, rs, false);

	/* continuation with NUL */
	set_state_ch(ps, 0xa1);
	set_state_ch(rs, 0xa1);
	buf[0] = '\0';
	run_mbr(f, fn, ref_fn, buf, 1, ps, rs, false);

	/* continuation success */
	set_state_ch(ps, 0xa1);
	set_state_ch(rs, 0xa1);
	buf[0] = (char)0xb2; buf[1] = 'x';
	run_mbr(f, fn, ref_fn, buf, 2, ps, rs, false);

	/* embedded NUL in 2-byte */
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0xa1; buf[1] = '\0';
	run_mbr(f, fn, ref_fn, buf, 2, ps, rs, false);
}

template<typename MbrFn, typename RefMbrFn>
static void
sweep_mbr(int f, MbrFn fn, RefMbrFn ref_fn)
{
	char in[16];

	for (long long i = 0; i < SWEEP; i++) {
		P::mbstate_t ps{};
		ref_mbstate_t rs{};
		size_t len = u32(8) + 1;
		bool null_pwc = (u32(2) == 0);
		size_t n = u32((uint32_t)len + 2);

		if (u32(4) == 0)
			std::memset(&ps, 0, sizeof(ps));
		else if (u32(3) == 0) {
			wchar_t ch = (wchar_t)(0x80 + u32(0x7f));
			if (u32(2) == 0)
				ch = (wchar_t)(ch | 0x100);
			set_state_ch(ps, ch);
		} else
			std::memset(&ps, 0x55, sizeof(ps));
		mb_copy(ps, rs);

		for (size_t j = 0; j < len; j++) {
			switch (u32(6)) {
			case 0: in[j] = (char)u32(256); break;
			case 1: in[j] = (char)(0x80 + u32(128)); break;
			case 2: in[j] = (char)(0xa0 + u32(96)); break;
			default: in[j] = (char)(32 + u32(96)); break;
			}
		}
		in[len] = '\0';
		if (u32(5) == 0)
			in[u32((uint32_t)len + 1)] = '\0';
		run_mbr(f, fn, ref_fn, in, n, ps, rs, null_pwc);
	}
}

template<typename WctFn, typename RefWctFn>
static bool
run_wct(int f, WctFn fn, RefWctFn ref_fn, char *out, size_t cap,
    wchar_t wc, P::mbstate_t &ps, ref_mbstate_t &rs, bool null_out)
{
	int pe, re;
	unsigned char ob[16], rb[16];

	if (!null_out) {
		fill_guard(ob, sizeof(ob));
		fill_guard(rb, sizeof(rb));
		std::memcpy(ob + 4, out, cap);
		std::memcpy(rb + 4, out, cap);
	}
	errno = 0;
	size_t pr = fn(null_out ? nullptr : (char *)(ob + 4), wc, &ps);
	pe = errno;
	errno = 0;
	size_t rr = ref_fn(null_out ? nullptr : (char *)(rb + 4), wc, &rs);
	re = errno;
	if (!chk_ret(f, pr, rr, pe, re))
		return (false);
	if (!null_out) {
		if (std::memcmp(ob, rb, sizeof(ob)) != 0) {
			report(f, "outbuf");
			return (false);
		}
	}
	if (!mb_eq(ps, rs)) {
		report(f, "state");
		return (false);
	}
	return (true);
}

template<typename WctFn, typename RefWctFn>
static void
edge_wct(int f, WctFn fn, RefWctFn ref_fn)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	char scratch[8];

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	run_wct(f, fn, ref_fn, scratch, 8, L'a', ps, rs, true);
	run_wct(f, fn, ref_fn, scratch, 8, L'a', ps, rs, false);
	run_wct(f, fn, ref_fn, scratch, 8, (wchar_t)0x8122, ps, rs, false);
	run_wct(f, fn, ref_fn, scratch, 8, L'\0', ps, rs, false);

	set_state_ch(ps, 1);
	set_state_ch(rs, 1);
	run_wct(f, fn, ref_fn, scratch, 8, L'x', ps, rs, false);
}

template<typename WctFn, typename RefWctFn>
static void
sweep_wct(int f, WctFn fn, RefWctFn ref_fn)
{
	char scratch[8];

	for (long long i = 0; i < SWEEP; i++) {
		P::mbstate_t ps{};
		ref_mbstate_t rs{};
		wchar_t wc = (wchar_t)(rnd() & 0xffff);
		bool null_out = (u32(5) == 0);

		if (u32(3) == 0)
			set_state_ch(ps, (wchar_t)(u32(0x200)));
		else
			std::memset(&ps, 0, sizeof(ps));
		mb_copy(ps, rs);
		run_wct(f, fn, ref_fn, scratch, 8, wc, ps, rs, null_out);
	}
}

template<typename MbsFn, typename RefMbsFn>
static bool
run_mbs(int f, MbsFn fn, RefMbsFn ref_fn, const char *bytes, size_t nms,
    size_t len, bool dst_null)
{
	char in_p[64], in_r[64];
	wchar_t out_p[32], out_r[32];
	const char *sp, *sr;
	P::mbstate_t ps_p{}, ps_r{};
	ref_mbstate_t rs_p{}, rs_r{};
	int pe, re;

	fill_guard((unsigned char *)in_p, sizeof(in_p));
	fill_guard((unsigned char *)in_r, sizeof(in_r));
	std::memcpy(in_p + 8, bytes, std::strlen(bytes) + 1);
	std::memcpy(in_r + 8, bytes, std::strlen(bytes) + 1);
	fill_wguard(out_p, sizeof(out_p) / sizeof(out_p[0]));
	fill_wguard(out_r, sizeof(out_r) / sizeof(out_r[0]));
	sp = in_p + 8;
	sr = in_r + 8;
	errno = 0;
	size_t pv = fn(dst_null ? nullptr : out_p, &sp, nms, len, &ps_p);
	pe = errno;
	errno = 0;
	size_t rv = ref_fn(dst_null ? nullptr : out_r, &sr, nms, len, &rs_p);
	re = errno;
	if (!chk_ret(f, pv, rv, pe, re))
		return (false);
	if (!dst_null && std::memcmp(out_p, out_r, sizeof(out_p)) != 0) {
		report(f, "wbuf");
		return (false);
	}
	if ((sp == nullptr) != (sr == nullptr) ||
	    (sp != nullptr && (sp - in_p) != (sr - in_r))) {
		report(f, "src");
		return (false);
	}
	if (std::memcmp(in_p, in_r, sizeof(in_p)) != 0) {
		report(f, "inbuf");
		return (false);
	}
	if (!mb_eq(ps_p, rs_p)) {
		report(f, "state");
		return (false);
	}
	return (true);
}

template<typename MbsFn, typename RefMbsFn>
static void
edge_mbs(int f, MbsFn fn, RefMbsFn ref_fn)
{
	static const char *cases[] = {
		"", "a", "ab", "\x80", "\xa1\xb2", "a\0b",
		"\xa1", "\x81\x82", "\xff", "\xc2\xa9"
	};
	for (const char *c : cases) {
		run_mbs(f, fn, ref_fn, c, 16, 8, false);
		run_mbs(f, fn, ref_fn, c, 16, 0, true);
		run_mbs(f, fn, ref_fn, c, 1, 8, false);
		run_mbs(f, fn, ref_fn, c, 0, 8, false);
	}
}

template<typename MbsFn, typename RefMbsFn>
static void
sweep_mbs(int f, MbsFn fn, RefMbsFn ref_fn)
{
	char in[20];

	for (long long i = 0; i < SWEEP; i++) {
		size_t blen = u32(12) + 1;
		for (size_t j = 0; j < blen; j++)
			in[j] = (char)(u32(256));
		in[blen] = '\0';
		if (u32(4) == 0)
			in[u32((uint32_t)blen + 1)] = '\0';
		run_mbs(f, fn, ref_fn, in, u32(16), u32(10),
		    u32(3) == 0);
	}
}

template<typename WcsFn, typename RefWcsFn>
static bool
run_wcs(int f, WcsFn fn, RefWcsFn ref_fn, const wchar_t *wcs, size_t nwc,
    size_t len, bool dst_null)
{
	unsigned char out_p[64], out_r[64];
	wchar_t in_p[32], in_r[32];
	const wchar_t *sp, *sr;
	P::mbstate_t ps_p{}, ps_r{};
	ref_mbstate_t rs_p{}, rs_r{};
	int pe, re;
	size_t wlen = 0;

	while (wcs[wlen] != L'\0')
		wlen++;
	fill_guard(out_p, sizeof(out_p));
	fill_guard(out_r, sizeof(out_r));
	fill_wguard(in_p, sizeof(in_p) / sizeof(in_p[0]));
	fill_wguard(in_r, sizeof(in_r) / sizeof(in_r[0]));
	std::memcpy(in_p + 4, wcs, (wlen + 1) * sizeof(wchar_t));
	std::memcpy(in_r + 4, wcs, (wlen + 1) * sizeof(wchar_t));
	sp = in_p + 4;
	sr = in_r + 4;
	errno = 0;
	size_t pv = fn(dst_null ? nullptr : (char *)(out_p + 8), &sp, nwc, len,
	    &ps_p);
	pe = errno;
	errno = 0;
	size_t rv = ref_fn(dst_null ? nullptr : (char *)(out_r + 8), &sr, nwc,
	    len, &rs_p);
	re = errno;
	if (!chk_ret(f, pv, rv, pe, re))
		return (false);
	if (!dst_null && std::memcmp(out_p, out_r, sizeof(out_p)) != 0) {
		report(f, "mbuf");
		return (false);
	}
	if ((sp == nullptr) != (sr == nullptr) ||
	    (sp != nullptr && (sp - in_p) != (sr - in_r))) {
		report(f, "wsrc");
		return (false);
	}
	if (std::memcmp(in_p, in_r, sizeof(in_p)) != 0) {
		report(f, "winbuf");
		return (false);
	}
	if (!mb_eq(ps_p, rs_p)) {
		report(f, "state");
		return (false);
	}
	return (true);
}

template<typename WcsFn, typename RefWcsFn>
static void
edge_wcs(int f, WcsFn fn, RefWcsFn ref_fn)
{
	const wchar_t *cases[] = {
		L"", L"a", L"ab", L"\x80", (const wchar_t *)L"\x8122",
		L"a\x00b", (const wchar_t *)L"\x8140"
	};
	for (const wchar_t *c : cases) {
		run_wcs(f, fn, ref_fn, c, 16, 16, false);
		run_wcs(f, fn, ref_fn, c, 16, 0, true);
		run_wcs(f, fn, ref_fn, c, 1, 16, false);
		run_wcs(f, fn, ref_fn, c, 16, 1, false);
		run_wcs(f, fn, ref_fn, c, 16, 3, false);
	}
}

template<typename WcsFn, typename RefWcsFn>
static void
sweep_wcs(int f, WcsFn fn, RefWcsFn ref_fn)
{
	wchar_t in[16];

	for (long long i = 0; i < SWEEP; i++) {
		size_t n = u32(8) + 1;
		for (size_t j = 0; j < n; j++)
			in[j] = (wchar_t)(rnd() & 0xffff);
		in[n] = L'\0';
		run_wcs(f, fn, ref_fn, in, u32(12), u32(8) + 1, u32(4) == 0);
	}
}

static void
edge_ascii_mbr(int f)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	char buf[4];

	run_mbr(f, P::ascii_mbrtowc, ref__ascii_mbrtowc, nullptr, 0, ps, rs, true);
	buf[0] = 'a';
	run_mbr(f, P::ascii_mbrtowc, ref__ascii_mbrtowc, buf, 0, ps, rs, false);
	buf[0] = (char)0x80;
	run_mbr(f, P::ascii_mbrtowc, ref__ascii_mbrtowc, buf, 1, ps, rs, false);
	buf[0] = '\0';
	run_mbr(f, P::ascii_mbrtowc, ref__ascii_mbrtowc, buf, 1, ps, rs, false);
}

static void
edge_mskanji_mbr(int f)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	char buf[8];

	edge_mbr(f, P::MSKanji_mbrtowc, ref__MSKanji_mbrtowc);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0x81; buf[1] = (char)0x40;
	run_mbr(f, P::MSKanji_mbrtowc, ref__MSKanji_mbrtowc, buf, 2, ps, rs, false);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0xa0;
	run_mbr(f, P::MSKanji_mbrtowc, ref__MSKanji_mbrtowc, buf, 2, ps, rs, false);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0xe0; buf[1] = (char)0x40;
	run_mbr(f, P::MSKanji_mbrtowc, ref__MSKanji_mbrtowc, buf, 2, ps, rs, false);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0xfd;
	run_mbr(f, P::MSKanji_mbrtowc, ref__MSKanji_mbrtowc, buf, 2, ps, rs, false);
}

static void
edge_mskanji_wct(int f)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	char scratch[8];

	edge_wct(f, P::MSKanji_wcrtomb, ref__MSKanji_wcrtomb);
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	run_wct(f, P::MSKanji_wcrtomb, ref__MSKanji_wcrtomb, scratch, 8,
	    (wchar_t)0x100, ps, rs, false);
	run_wct(f, P::MSKanji_wcrtomb, ref__MSKanji_wcrtomb, scratch, 8,
	    (wchar_t)0x101, ps, rs, false);
}

static void
edge_gbk_mbr(int f)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	char buf[4];

	edge_mbr(f, P::GBK_mbrtowc, ref__GBK_mbrtowc);
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0x80;
	run_mbr(f, P::GBK_mbrtowc, ref__GBK_mbrtowc, buf, 2, ps, rs, false);
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0x81; buf[1] = (char)0x40;
	run_mbr(f, P::GBK_mbrtowc, ref__GBK_mbrtowc, buf, 2, ps, rs, false);
}

static void
edge_big5_mbr(int f)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	char buf[4];

	edge_mbr(f, P::BIG5_mbrtowc, ref__BIG5_mbrtowc);
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0xa0;
	run_mbr(f, P::BIG5_mbrtowc, ref__BIG5_mbrtowc, buf, 2, ps, rs, false);
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0xa1; buf[1] = (char)0x40;
	run_mbr(f, P::BIG5_mbrtowc, ref__BIG5_mbrtowc, buf, 2, ps, rs, false);
}

int
main()
{
	test_init(F_BIG5_INIT, P::BIG5_init, ref__BIG5_init, 2, 128);
	test_init(F_GBK_INIT, P::GBK_init, ref__GBK_init, 2, 128);
	test_init(F_ASCII_INIT, P::ascii_init, ref__ascii_init, 1, 128);
	test_init(F_MSK_INIT, P::MSKanji_init, ref__MSKanji_init, 2, 224);

	test_mbsinit(F_BIG5_MBSINIT, P::BIG5_mbsinit, ref__BIG5_mbsinit);
	test_mbsinit(F_GBK_MBSINIT, P::GBK_mbsinit, ref__GBK_mbsinit);
	test_mbsinit(F_ASCII_MBSINIT, P::ascii_mbsinit, ref__ascii_mbsinit);
	test_mbsinit(F_MSK_MBSINIT, P::MSKanji_mbsinit, ref__MSKanji_mbsinit);

	edge_check(F_BIG5_CHECK, P::big5_check, ref__big5_check);
	sweep_check(F_BIG5_CHECK, P::big5_check, ref__big5_check);
	edge_check(F_GBK_CHECK, P::gbk_check, ref__gbk_check);
	sweep_check(F_GBK_CHECK, P::gbk_check, ref__gbk_check);

	edge_big5_mbr(F_BIG5_MBRTOWC);
	sweep_mbr(F_BIG5_MBRTOWC, P::BIG5_mbrtowc, ref__BIG5_mbrtowc);
	edge_gbk_mbr(F_GBK_MBRTOWC);
	sweep_mbr(F_GBK_MBRTOWC, P::GBK_mbrtowc, ref__GBK_mbrtowc);
	edge_ascii_mbr(F_ASCII_MBRTOWC);
	sweep_mbr(F_ASCII_MBRTOWC, P::ascii_mbrtowc, ref__ascii_mbrtowc);
	edge_mskanji_mbr(F_MSK_MBRTOWC);
	sweep_mbr(F_MSK_MBRTOWC, P::MSKanji_mbrtowc, ref__MSKanji_mbrtowc);

	edge_wct(F_BIG5_WCRTOMB, P::BIG5_wcrtomb, ref__BIG5_wcrtomb);
	sweep_wct(F_BIG5_WCRTOMB, P::BIG5_wcrtomb, ref__BIG5_wcrtomb);
	edge_wct(F_GBK_WCRTOMB, P::GBK_wcrtomb, ref__GBK_wcrtomb);
	sweep_wct(F_GBK_WCRTOMB, P::GBK_wcrtomb, ref__GBK_wcrtomb);
	edge_wct(F_ASCII_WCRTOMB, P::ascii_wcrtomb, ref__ascii_wcrtomb);
	sweep_wct(F_ASCII_WCRTOMB, P::ascii_wcrtomb, ref__ascii_wcrtomb);
	edge_mskanji_wct(F_MSK_WCRTOMB);
	sweep_wct(F_MSK_WCRTOMB, P::MSKanji_wcrtomb, ref__MSKanji_wcrtomb);

	edge_mbs(F_BIG5_MBSNRTOWCS, P::BIG5_mbsnrtowcs, ref__BIG5_mbsnrtowcs);
	sweep_mbs(F_BIG5_MBSNRTOWCS, P::BIG5_mbsnrtowcs, ref__BIG5_mbsnrtowcs);
	edge_mbs(F_GBK_MBSNRTOWCS, P::GBK_mbsnrtowcs, ref__GBK_mbsnrtowcs);
	sweep_mbs(F_GBK_MBSNRTOWCS, P::GBK_mbsnrtowcs, ref__GBK_mbsnrtowcs);
	edge_mbs(F_ASCII_MBSNRTOWCS, P::ascii_mbsnrtowcs, ref__ascii_mbsnrtowcs);
	sweep_mbs(F_ASCII_MBSNRTOWCS, P::ascii_mbsnrtowcs, ref__ascii_mbsnrtowcs);
	edge_mbs(F_MSK_MBSNRTOWCS, P::MSKanji_mbsnrtowcs, ref__MSKanji_mbsnrtowcs);
	sweep_mbs(F_MSK_MBSNRTOWCS, P::MSKanji_mbsnrtowcs, ref__MSKanji_mbsnrtowcs);

	edge_wcs(F_BIG5_WCSNRTOMBS, P::BIG5_wcsnrtombs, ref__BIG5_wcsnrtombs);
	sweep_wcs(F_BIG5_WCSNRTOMBS, P::BIG5_wcsnrtombs, ref__BIG5_wcsnrtombs);
	edge_wcs(F_GBK_WCSNRTOMBS, P::GBK_wcsnrtombs, ref__GBK_wcsnrtombs);
	sweep_wcs(F_GBK_WCSNRTOMBS, P::GBK_wcsnrtombs, ref__GBK_wcsnrtombs);
	edge_wcs(F_ASCII_WCSNRTOMBS, P::ascii_wcsnrtombs, ref__ascii_wcsnrtombs);
	sweep_wcs(F_ASCII_WCSNRTOMBS, P::ascii_wcsnrtombs, ref__ascii_wcsnrtombs);
	edge_wcs(F_MSK_WCSNRTOMBS, P::MSKanji_wcsnrtombs, ref__MSKanji_wcsnrtombs);
	sweep_wcs(F_MSK_WCSNRTOMBS, P::MSKanji_wcsnrtombs, ref__MSKanji_wcsnrtombs);

	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	long long total_fail = 0;
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-22s %12lld %12lld\n", fname[i], ncase[i],
		    nfail[i]);
		total_fail += nfail[i];
	}
	return (total_fail == 0 ? 0 : 1);
}
