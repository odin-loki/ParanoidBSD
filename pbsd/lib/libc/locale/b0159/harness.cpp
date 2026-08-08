/*
 * Differential harness for batch b0159.
 */

#include <climits>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>

import pbsd.lib.libc.locale.b0159;

namespace P = pbsd::lib_libc_locale::b0159;

extern "C" {
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} ref_mbstate_t;

typedef struct {
	char		__encoding[32];
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

struct ref_xlocale {
	void		*components[8];
	char		*csym;
};

typedef struct ref_xlocale *ref_locale_t;

typedef struct {
	const char	*encoding;
	int		read_fail;
	int		asprintf_fail;
} pbsd_rune_hook_t;

typedef struct {
	char	p_cs_precedes;
	char	n_cs_precedes;
} pbsd_lconv;

typedef struct {
	struct {
		const char	*c_fmt;
		const char	*x_fmt;
		const char	*X_fmt;
		const char	*weekday[7];
		const char	*md_order;
	} time;
	struct {
		const char	*decimal_point;
	} numeric;
	struct {
		const char	*yesexpr;
	} messages;
	struct {
		const char	*currency_symbol;
		const char	*mon_decimal_point;
	} monetary;
	pbsd_lconv	conv;
	char		encoding[32];
} pbsd_nl_hook_t;

extern pbsd_rune_hook_t	pbsd_rune_hook;
extern pbsd_nl_hook_t	pbsd_nl_hook;
extern ref_xlocale	__xlocale_global_locale;
extern ref_xlocale_ctype __xlocale_global_ctype;
extern ref_rune_locale	_DefaultRuneLocale;
extern int		__mb_cur_max;
extern int		__mb_sb_limit;

void	pbsd_reset_hooks(void);
void	pbsd_oracle_init(void);

int	ref__none_init(ref_xlocale_ctype *, ref_rune_locale *);
int	ref__none_mbsinit(const ref_mbstate_t *);
size_t	ref__none_mbrtowc(wchar_t *, const char *, size_t, ref_mbstate_t *);
size_t	ref__none_wcrtomb(char *, wchar_t, ref_mbstate_t *);
size_t	ref__none_mbsnrtowcs(wchar_t *, const char **, size_t, size_t,
	    ref_mbstate_t *);
size_t	ref__none_wcsnrtombs(char *, const wchar_t **, size_t, size_t,
	    ref_mbstate_t *);

char	*ref_nl_langinfo_l(int item, ref_locale_t);
char	*ref_nl_langinfo(int item);

int	ref__GB18030_init(ref_xlocale_ctype *, ref_rune_locale *);
int	ref__GB18030_mbsinit(const ref_mbstate_t *);
size_t	ref__GB18030_mbrtowc(wchar_t *, const char *, size_t,
	    ref_mbstate_t *);
size_t	ref__GB18030_wcrtomb(char *, wchar_t, ref_mbstate_t *);
size_t	ref__GB18030_mbsnrtowcs(wchar_t *, const char **, size_t, size_t,
	    ref_mbstate_t *);
size_t	ref__GB18030_wcsnrtombs(char *, const wchar_t **, size_t, size_t,
	    ref_mbstate_t *);

const ref_rune_locale *ref___getCurrentRuneLocale(void);
int	ref___wrap_setrunelocale(const char *);
void	ref___set_thread_rune_locale(ref_locale_t);
void	*ref___ctype_load(const char *, ref_locale_t);
}

enum {
	F_NONE_INIT, F_NONE_MBSINIT, F_NONE_MBRTOWC, F_NONE_WCRTOMB,
	F_NONE_MBSNRTOWCS, F_NONE_WCSNRTOMBS,
	F_NL_LANGINFO_L, F_NL_LANGINFO,
	F_GB_INIT, F_GB_MBSINIT, F_GB_MBRTOWC, F_GB_WCRTOMB,
	F_GB_MBSNRTOWCS, F_GB_WCSNRTOMBS,
	F_GET_RUNE, F_WRAP_SETRUNE, F_SET_THREAD, F_CTYPE_LOAD,
	F_COUNT
};

static const char *const fname[F_COUNT] = {
	"_none_init", "_none_mbsinit", "_none_mbrtowc", "_none_wcrtomb",
	"_none_mbsnrtowcs", "_none_wcsnrtombs",
	"nl_langinfo_l", "nl_langinfo",
	"_GB18030_init", "_GB18030_mbsinit", "_GB18030_mbrtowc",
	"_GB18030_wcrtomb", "_GB18030_mbsnrtowcs", "_GB18030_wcsnrtombs",
	"__getCurrentRuneLocale", "__wrap_setrunelocale",
	"__set_thread_rune_locale", "__ctype_load"
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int nprinted[F_COUNT];

static constexpr unsigned char GUARD = 0x7f;
static constexpr wchar_t WGUARD = (wchar_t)0x7f7f;
static constexpr long long SWEEP = 200000;
static constexpr size_t OUT_CAP = 256;
static constexpr size_t OUT_PAD = 32;

static uint64_t rng = 0xB0159001ULL;

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
		std::printf("  FAIL %-26s : %s\n", fname[f], why);
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

static bool
chk_ret(int f, size_t pv, size_t rv, int pe, int re)
{
	ncase[f]++;
	if (pv != rv || pe != re) {
		report(f, "ret/errno");
		return (false);
	}
	return (true);
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
chk_buf(int f, const unsigned char *p, const unsigned char *r, size_t n)
{
	if (std::memcmp(p, r, n) != 0) {
		report(f, "buffer");
		return (false);
	}
	return (true);
}

static ref_xlocale_ctype *
ref_ctype(void)
{
	return ((ref_xlocale_ctype *)__xlocale_global_locale.components[1]);
}

static void
sync_encoding(const char *enc)
{
	std::strncpy(pbsd_nl_hook.encoding, enc, sizeof(pbsd_nl_hook.encoding) - 1);
	std::strncpy(_DefaultRuneLocale.__encoding, enc,
	    sizeof(_DefaultRuneLocale.__encoding) - 1);
	if (ref_ctype() != nullptr && ref_ctype()->runes != nullptr)
		std::strncpy(ref_ctype()->runes->__encoding, enc, 31);
}

static void
test_none_init(void)
{
	const int f = F_NONE_INIT;
	ref_xlocale_ctype rl, rr;
	ref_rune_locale runep, runer;
	P::xlocale_ctype pl, pr;
	P::_RuneLocale runep2, runer2;

	std::memset(&rl, 0, sizeof(rl));
	std::memset(&rr, 0, sizeof(rr));
	std::memset(&pl, 0, sizeof(pl));
	std::memset(&pr, 0, sizeof(pr));
	std::memset(&runep, 0, sizeof(runep));
	std::memset(&runer, 0, sizeof(runer));
	std::memset(&runep2, 0, sizeof(runep2));
	std::memset(&runer2, 0, sizeof(runer2));

	int pv = P::_none_init(&pl, &runep2);
	int rv = ref__none_init(&rr, &runer);
	ncase[f]++;
	if (pv != rv || pl.__mb_cur_max != rr.__mb_cur_max ||
	    pl.__mb_sb_limit != rr.__mb_sb_limit ||
	    (pl.__mbrtowc == nullptr) != (rr.__mbrtowc == nullptr)) {
		report(f, "init fields");
	}
}

static void
test_none_mbsinit(void)
{
	const int f = F_NONE_MBSINIT;
	ref_mbstate_t rs;
	P::mbstate_t ps;
	std::memset(&rs, 0x80, sizeof(rs));
	std::memset(&ps, 0x80, sizeof(ps));
	int pv = P::_none_mbsinit(&ps);
	int rv = ref__none_mbsinit(&rs);
	chk_ret(f, (size_t)pv, (size_t)rv, 0, 0);
	pv = P::_none_mbsinit(nullptr);
	rv = ref__none_mbsinit(nullptr);
	chk_ret(f, (size_t)pv, (size_t)rv, 0, 0);
}

static void
test_none_mbrtowc_case(int f, const char *s, size_t n, bool use_pwc)
{
	unsigned char buf[OUT_CAP];
	wchar_t pw, rw;
	P::mbstate_t ps;
	ref_mbstate_t rs;
	int pe = 0, re = 0;

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	errno = 0;
	size_t pr = P::_none_mbrtowc(use_pwc ? &pw : nullptr,
	    s, n, &ps);
	pe = errno;
	errno = 0;
	size_t rr = ref__none_mbrtowc(use_pwc ? &rw : nullptr,
	    s, n, &rs);
	re = errno;
	if (!chk_ret(f, pr, rr, pe, re))
		return;
	if (use_pwc && pr != (size_t)-1 && pr != (size_t)-2 && pw != rw)
		report(f, "wchar");
}

static void
test_none_mbrtowc(void)
{
	const int f = F_NONE_MBRTOWC;
	test_none_mbrtowc_case(f, nullptr, 0, false);
	test_none_mbrtowc_case(f, "", 0, true);
	test_none_mbrtowc_case(f, "A", 0, true);
	test_none_mbrtowc_case(f, "A", 1, true);
	test_none_mbrtowc_case(f, "\0", 1, true);
	test_none_mbrtowc_case(f, "\x80", 1, true);
	test_none_mbrtowc_case(f, "\xff", 1, true);
}

static void
test_none_wcrtomb_case(int f, wchar_t wc, bool use_out)
{
	unsigned char po[OUT_CAP], ro[OUT_CAP];
	P::mbstate_t ps;
	ref_mbstate_t rs;
	int pe = 0, re = 0;

	fill_guard(po, sizeof(po));
	fill_guard(ro, sizeof(ro));
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	errno = 0;
	size_t pr = P::_none_wcrtomb(use_out ? (char *)po : nullptr, wc, &ps);
	pe = errno;
	errno = 0;
	size_t rr = ref__none_wcrtomb(use_out ? (char *)ro : nullptr, wc, &rs);
	re = errno;
	if (!chk_ret(f, pr, rr, pe, re))
		return;
	if (use_out && pr == 1)
		chk_buf(f, po, ro, sizeof(po));
}

static void
test_none_wcrtomb(void)
{
	const int f = F_NONE_WCRTOMB;
	test_none_wcrtomb_case(f, L'A', true);
	test_none_wcrtomb_case(f, L'\0', true);
	test_none_wcrtomb_case(f, (wchar_t)0xff, true);
	test_none_wcrtomb_case(f, (wchar_t)-1, true);
	test_none_wcrtomb_case(f, (wchar_t)(UCHAR_MAX + 1), true);
	test_none_wcrtomb_case(f, L'Z', false);
}

static void
test_none_mbsnrtowcs_one(int f, const char *in, size_t nms, size_t len,
    bool dst_null)
{
	unsigned char po[OUT_CAP], ro[OUT_CAP];
	wchar_t pw[OUT_CAP], rw[OUT_CAP];
	const char *sp = in;
	const char *sr = in;
	P::mbstate_t ps;
	ref_mbstate_t rs;
	int pe = 0, re = 0;

	fill_guard(po, sizeof(po));
	fill_guard(ro, sizeof(ro));
	fill_wguard(pw, OUT_CAP);
	fill_wguard(rw, OUT_CAP);
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	errno = 0;
	size_t pr = P::_none_mbsnrtowcs(dst_null ? nullptr : pw, &sp, nms, len, &ps);
	pe = errno;
	const char *sr_save = sr;
	errno = 0;
	size_t rr = ref__none_mbsnrtowcs(dst_null ? nullptr : rw, &sr, nms, len, &rs);
	re = errno;
	if (!chk_ret(f, pr, rr, pe, re))
		return;
	if (!dst_null)
		chk_buf(f, (unsigned char *)pw, (unsigned char *)rw, sizeof(pw));
	if ((sp == nullptr) != (sr == nullptr))
		report(f, "src ptr");
	else if (sp != nullptr && sp - in != sr - sr_save)
		report(f, "src off");
}

static void
test_none_mbsnrtowcs(void)
{
	const int f = F_NONE_MBSNRTOWCS;
	test_none_mbsnrtowcs_one(f, "", 0, 0, true);
	test_none_mbsnrtowcs_one(f, "hi", 2, 0, true);
	test_none_mbsnrtowcs_one(f, "a\0b", 3, 3, false);
	test_none_mbsnrtowcs_one(f, "\x80\xff", 2, 2, false);
}

static void
test_none_wcsnrtombs_one(int f, const wchar_t *in, size_t nwc, size_t len,
    bool dst_null)
{
	unsigned char po[OUT_CAP], ro[OUT_CAP];
	const wchar_t *sp = in;
	const wchar_t *sr = in;
	P::mbstate_t ps;
	ref_mbstate_t rs;
	int pe = 0, re = 0;

	fill_guard(po, sizeof(po));
	fill_guard(ro, sizeof(ro));
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	errno = 0;
	size_t pr = P::_none_wcsnrtombs(dst_null ? nullptr : (char *)po,
	    &sp, nwc, len, &ps);
	pe = errno;
	const wchar_t *sr_save = sr;
	errno = 0;
	size_t rr = ref__none_wcsnrtombs(dst_null ? nullptr : (char *)ro,
	    &sr, nwc, len, &rs);
	re = errno;
	if (!chk_ret(f, pr, rr, pe, re))
		return;
	if (!dst_null)
		chk_buf(f, po, ro, sizeof(po));
	if ((sp == nullptr) != (sr == nullptr))
		report(f, "src ptr");
	else if (sp != nullptr && sp - in != sr - sr_save)
		report(f, "src off");
}

static void
test_none_wcsnrtombs(void)
{
	const int f = F_NONE_WCSNRTOMBS;
	const wchar_t w0[] = { L'h', L'i', L'\0' };
	const wchar_t wb[] = { (wchar_t)0xff, L'\0' };
	const wchar_t winv[] = { (wchar_t)-1, L'a', L'\0' };
	test_none_wcsnrtombs_one(f, w0, 3, 3, false);
	test_none_wcsnrtombs_one(f, w0, 3, 0, true);
	test_none_wcsnrtombs_one(f, wb, 2, 2, false);
	test_none_wcsnrtombs_one(f, winv, 3, 3, false);
}

static void
test_nl_one(int f, int item, const char *enc)
{
	sync_encoding(enc);
	char *pp = P::nl_langinfo_l(item, nullptr);
	char *rp = ref_nl_langinfo_l(item, nullptr);
	ncase[f]++;
	if ((pp == nullptr) != (rp == nullptr)) {
		report(f, "null ptr");
		return;
	}
	if (pp != nullptr && std::strcmp(pp, rp) != 0)
		report(f, "string");
}

static void
test_nl_langinfo(void)
{
	test_nl_one(F_NL_LANGINFO_L, 0 /*CODESET*/, "EUC-JP");
	test_nl_one(F_NL_LANGINFO_L, 0, "EUC-CN");
	test_nl_one(F_NL_LANGINFO_L, 0, "BIG5");
	test_nl_one(F_NL_LANGINFO_L, 0, "MSKanji");
	test_nl_one(F_NL_LANGINFO_L, 0, "NONE");
	test_nl_one(F_NL_LANGINFO_L, 0, "NONE:US-ASCII");
	test_nl_one(F_NL_LANGINFO_L, 0, "UTF-8");
	test_nl_one(F_NL_LANGINFO_L, 62 /*RADIXCHAR*/, "UTF-8");
	test_nl_one(F_NL_LANGINFO_L, 7 /*DAY_1*/, "UTF-8");
	test_nl_one(F_NL_LANGINFO_L, 999, "UTF-8");

	pbsd_nl_hook.monetary.currency_symbol = "$";
	pbsd_nl_hook.conv.p_cs_precedes = 1;
	pbsd_nl_hook.conv.n_cs_precedes = 1;
	test_nl_one(F_NL_LANGINFO_L, 68 /*CRNCYSTR*/, "UTF-8");

	pbsd_nl_hook.conv.p_cs_precedes = (char)CHAR_MAX;
	pbsd_nl_hook.monetary.currency_symbol = ".";
	pbsd_nl_hook.monetary.mon_decimal_point = ".";
	test_nl_one(F_NL_LANGINFO_L, 68, "UTF-8");

	test_nl_one(F_NL_LANGINFO, 0, "EUC-KR");
	char *pp = P::nl_langinfo(0);
	char *rp = ref_nl_langinfo(0);
	ncase[F_NL_LANGINFO]++;
	if (pp == nullptr || rp == nullptr || std::strcmp(pp, rp) != 0)
		report(F_NL_LANGINFO, "nl_langinfo");
}

static void
test_gb_init(void)
{
	const int f = F_GB_INIT;
	ref_xlocale_ctype rr;
	P::xlocale_ctype pl;
	ref_rune_locale runer;
	P::_RuneLocale runep;
	std::memset(&rr, 0, sizeof(rr));
	std::memset(&pl, 0, sizeof(pl));
	std::memset(&runer, 0, sizeof(runer));
	std::memset(&runep, 0, sizeof(runep));
	int pv = P::_GB18030_init(&pl, &runep);
	int rv = ref__GB18030_init(&rr, &runer);
	ncase[f]++;
	if (pv != rv || pl.__mb_cur_max != rr.__mb_cur_max)
		report(f, "init");
}

static void
test_gb_mbsinit(void)
{
	const int f = F_GB_MBSINIT;
	ref_mbstate_t rs;
	P::mbstate_t ps;
	std::memset(&rs, 0, sizeof(rs));
	std::memset(&ps, 0, sizeof(ps));
	chk_ret(f, (size_t)P::_GB18030_mbsinit(&ps),
	    (size_t)ref__GB18030_mbsinit(&rs), 0, 0);
	rs.__mbstate8[0] = 1;
	ps.__mbstate8[0] = 1;
	chk_ret(f, (size_t)P::_GB18030_mbsinit(&ps),
	    (size_t)ref__GB18030_mbsinit(&rs), 0, 0);
	chk_ret(f, (size_t)P::_GB18030_mbsinit(nullptr),
	    (size_t)ref__GB18030_mbsinit(nullptr), 0, 0);
}

static void
test_gb_mbrtowc_bytes(int f, const unsigned char *seq, size_t n, size_t pre)
{
	wchar_t pw, rw;
	P::mbstate_t ps;
	ref_mbstate_t rs;
	int pe = 0, re = 0;

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	if (pre > 0 && pre < sizeof(ps.__mbstate8)) {
		ps.__mbstate8[0] = (char)pre;
		rs.__mbstate8[0] = (char)pre;
	}
	errno = 0;
	size_t pr = P::_GB18030_mbrtowc(&pw, (const char *)seq, n, &ps);
	pe = errno;
	errno = 0;
	size_t rr = ref__GB18030_mbrtowc(&rw, (const char *)seq, n, &rs);
	re = errno;
	if (!chk_ret(f, pr, rr, pe, re))
		return;
	if (pr != (size_t)-1 && pr != (size_t)-2 && pw != rw)
		report(f, "wchar");
	if (!mb_eq(ps, rs))
		report(f, "state");
}

static void
test_gb_mbrtowc(void)
{
	const int f = F_GB_MBRTOWC;
	const unsigned char s1[] = { 'A' };
	const unsigned char s2[] = { 0x81, 0x40 };
	const unsigned char s4[] = { 0x81, 0x30, 0x81, 0x30 };
	const unsigned char bad[] = { 0x80 };
	const unsigned char bad2[] = { 0x81, 0x20 };
	test_gb_mbrtowc_bytes(f, nullptr, 0, 0);
	test_gb_mbrtowc_bytes(f, s1, 1, 0);
	test_gb_mbrtowc_bytes(f, s2, 1, 0);
	test_gb_mbrtowc_bytes(f, s2, 2, 0);
	test_gb_mbrtowc_bytes(f, s4, 4, 0);
	test_gb_mbrtowc_bytes(f, bad, 1, 0);
	test_gb_mbrtowc_bytes(f, bad2, 2, 0);
	test_gb_mbrtowc_bytes(f, s2, 2, 99);
}

static void
test_gb_wcrtomb_one(int f, wchar_t wc)
{
	unsigned char po[OUT_CAP], ro[OUT_CAP];
	P::mbstate_t ps;
	ref_mbstate_t rs;
	int pe = 0, re = 0;

	fill_guard(po, sizeof(po));
	fill_guard(ro, sizeof(ro));
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	errno = 0;
	size_t pr = P::_GB18030_wcrtomb((char *)po, wc, &ps);
	pe = errno;
	errno = 0;
	size_t rr = ref__GB18030_wcrtomb((char *)ro, wc, &rs);
	re = errno;
	if (!chk_ret(f, pr, rr, pe, re))
		return;
	chk_buf(f, po, ro, sizeof(po));
}

static void
test_gb_wcrtomb(void)
{
	const int f = F_GB_WCRTOMB;
	test_gb_wcrtomb_one(f, L'A');
	test_gb_wcrtomb_one(f, (wchar_t)0x8140);
	test_gb_wcrtomb_one(f, (wchar_t)0x01308130);
	test_gb_wcrtomb_one(f, (wchar_t)0x00ff0000);
	test_gb_wcrtomb_one(f, (wchar_t)-1);
	P::mbstate_t ps;
	ref_mbstate_t rs;
	ps.__mbstate8[0] = 1;
	rs.__mbstate8[0] = 1;
	errno = 0;
	size_t pr = P::_GB18030_wcrtomb(nullptr, L'A', &ps);
	int pe = errno;
	errno = 0;
	size_t rr = ref__GB18030_wcrtomb(nullptr, L'A', &rs);
	chk_ret(f, pr, rr, pe, errno);
}

static void
test_gb_mbsnrtowcs(void)
{
	const int f = F_GB_MBSNRTOWCS;
	const char in[] = { (char)0x41, (char)0x81, (char)0x40, '\0' };
	const char *sp = in;
	const char *sr = in;
	wchar_t pw[16], rw[16];
	P::mbstate_t ps;
	ref_mbstate_t rs;
	fill_wguard(pw, 16);
	fill_wguard(rw, 16);
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	errno = 0;
	size_t pr = P::_GB18030_mbsnrtowcs(pw, &sp, 4, 4, &ps);
	int pe = errno;
	errno = 0;
	size_t rr = ref__GB18030_mbsnrtowcs(rw, &sr, 4, 4, &rs);
	chk_ret(f, pr, rr, pe, errno);
}

static void
test_gb_wcsnrtombs(void)
{
	const int f = F_GB_WCSNRTOMBS;
	const wchar_t in[] = { L'A', (wchar_t)0x8140, L'\0' };
	const wchar_t *sp = in;
	const wchar_t *sr = in;
	unsigned char po[32], ro[32];
	P::mbstate_t ps;
	ref_mbstate_t rs;
	fill_guard(po, sizeof(po));
	fill_guard(ro, sizeof(ro));
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	errno = 0;
	size_t pr = P::_GB18030_wcsnrtombs((char *)po, &sp, 3, 8, &ps);
	int pe = errno;
	errno = 0;
	size_t rr = ref__GB18030_wcsnrtombs((char *)ro, &sr, 3, 8, &rs);
	chk_ret(f, pr, rr, pe, errno);
	chk_buf(f, po, ro, sizeof(po));
}

static void
test_get_rune(void)
{
	const int f = F_GET_RUNE;
	const P::_RuneLocale *pp = P::__getCurrentRuneLocale();
	const ref_rune_locale *rp = ref___getCurrentRuneLocale();
	ncase[f]++;
	if ((pp == nullptr) != (rp == nullptr))
		report(f, "null");
	else if (pp != nullptr && std::strcmp(pp->__encoding, rp->__encoding) != 0)
		report(f, "encoding");
}

static void
test_wrap_setrunelocale(void)
{
	const int f = F_WRAP_SETRUNE;
	pbsd_rune_hook.read_fail = 0;
	pbsd_rune_hook.asprintf_fail = 0;

	pbsd_rune_hook.encoding = "C";
	int pv = P::__wrap_setrunelocale("C");
	int rv = ref___wrap_setrunelocale("C");
	ncase[f]++;
	if (pv != rv || __mb_cur_max != 1)
		report(f, "C locale");

	pbsd_rune_hook.encoding = "GB18030";
	pv = P::__wrap_setrunelocale("zh_CN.GB18030");
	rv = ref___wrap_setrunelocale("zh_CN.GB18030");
	ncase[f]++;
	if (pv != rv || __mb_cur_max != 4)
		report(f, "GB18030");

	pbsd_rune_hook.encoding = "BOGUS";
	pv = P::__wrap_setrunelocale("bogus");
	rv = ref___wrap_setrunelocale("bogus");
	ncase[f]++;
	if (pv != rv)
		report(f, "EFTYPE");

	pbsd_rune_hook.read_fail = 1;
	pv = P::__wrap_setrunelocale("x");
	rv = ref___wrap_setrunelocale("x");
	ncase[f]++;
	if (pv != rv)
		report(f, "read fail");
}

static void
test_set_thread(void)
{
	const int f = F_SET_THREAD;
	P::__set_thread_rune_locale(nullptr);
	ref___set_thread_rune_locale(nullptr);
	ncase[f]++;
	P::__set_thread_rune_locale((P::locale_t)-1);
	ref___set_thread_rune_locale((ref_locale_t)-1);
	ncase[f]++;
	P::__set_thread_rune_locale(&__xlocale_global_locale);
	ref___set_thread_rune_locale(&__xlocale_global_locale);
	ncase[f]++;
}

static void
test_ctype_load(void)
{
	const int f = F_CTYPE_LOAD;
	pbsd_rune_hook.read_fail = 0;
	pbsd_rune_hook.encoding = "NONE";
	void *pp = P::__ctype_load("NONE", nullptr);
	void *rp = ref___ctype_load("NONE", nullptr);
	ncase[f]++;
	if ((pp == nullptr) != (rp == nullptr))
		report(f, "load");
	if (pp != nullptr)
		free(pp);
	if (rp != nullptr)
		free(rp);

	pbsd_rune_hook.read_fail = 1;
	pp = P::__ctype_load("bad", nullptr);
	rp = ref___ctype_load("bad", nullptr);
	ncase[f]++;
	if ((pp == nullptr) != (rp == nullptr))
		report(f, "fail load");
}

static void
sweep_none(void)
{
	unsigned char seq[8];
	for (long long i = 0; i < SWEEP; i++) {
		size_t n = u32(8);
		for (size_t j = 0; j < n; j++)
			seq[j] = (unsigned char)(rnd() & 0xff);
		test_none_mbrtowc_case(F_NONE_MBRTOWC, (const char *)seq, n,
		    (rnd() & 1) != 0);
	}
	for (long long i = 0; i < SWEEP; i++) {
		test_none_wcrtomb_case(F_NONE_WCRTOMB,
		    (wchar_t)(rnd() & 0x1ff), (rnd() & 1) != 0);
	}
}

static void
sweep_gb(void)
{
	unsigned char seq[8];
	for (long long i = 0; i < SWEEP; i++) {
		size_t n = u32(8);
		for (size_t j = 0; j < n; j++)
			seq[j] = (unsigned char)(rnd() & 0xff);
		test_gb_mbrtowc_bytes(F_GB_MBRTOWC, seq, n, u32(5));
	}
	for (long long i = 0; i < SWEEP; i++) {
		test_gb_wcrtomb_one(F_GB_WCRTOMB, (wchar_t)(rnd()));
	}
}

static void
sweep_nl(void)
{
	static const char *encs[] = {
		"UTF-8", "EUC-JP", "NONE", "NONE:foo", "BIG5", "MSKanji"
	};
	for (long long i = 0; i < SWEEP; i++) {
		int item = (int)(rnd() % 80);
		const char *enc = encs[u32(6)];
		test_nl_one(F_NL_LANGINFO_L, item, enc);
	}
}

int
main(void)
{
	long long total_fail = 0;

	pbsd_oracle_init();
	pbsd_reset_hooks();

	test_none_init();
	test_none_mbsinit();
	test_none_mbrtowc();
	test_none_wcrtomb();
	test_none_mbsnrtowcs();
	test_none_wcsnrtombs();
	test_nl_langinfo();
	test_gb_init();
	test_gb_mbsinit();
	test_gb_mbrtowc();
	test_gb_wcrtomb();
	test_gb_mbsnrtowcs();
	test_gb_wcsnrtombs();
	test_get_rune();
	test_wrap_setrunelocale();
	test_set_thread();
	test_ctype_load();

	sweep_none();
	sweep_gb();
	sweep_nl();

	std::printf("b0159 differential harness\n");
	std::printf("%-28s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-28s %12lld %12lld\n", fname[i], ncase[i], nfail[i]);
		total_fail += nfail[i];
	}
	std::printf("total failures: %lld\n", total_fail);
	return (total_fail == 0 ? 0 : 1);
}
