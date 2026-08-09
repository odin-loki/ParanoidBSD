/*
 * Differential harness for batch b0159.
 */

#include <cstdlib>
#include <cerrno>
#include <climits>
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

struct ref_pbsd_lconv {
	char	p_cs_precedes;
	char	n_cs_precedes;
};

struct ref_xlocale {
	void			*components[8];
	char			*csym;
	struct ref_pbsd_lconv	conv;
};

typedef struct ref_xlocale *ref_locale_t;

typedef struct {
	const char	*encoding;
	int		read_fail;
	int		asprintf_fail;
} pbsd_rune_hook_t;

typedef struct {
	struct {
		const char	*c_fmt;
		const char	*x_fmt;
		const char	*X_fmt;
		const char	*ampm_fmt;
		const char	*am;
		const char	*pm;
		const char	*weekday[7];
		const char	*wday[7];
		const char	*month[12];
		const char	*mon[12];
		const char	*alt_month[12];
		const char	*md_order;
	} time;
	struct {
		const char	*decimal_point;
		const char	*thousands_sep;
	} numeric;
	struct {
		const char	*yesexpr;
		const char	*noexpr;
		const char	*yesstr;
		const char	*nostr;
	} messages;
	struct {
		const char	*currency_symbol;
		const char	*mon_decimal_point;
	} monetary;
	struct ref_pbsd_lconv	conv;
	char			encoding[32];
} pbsd_nl_hook_t;

void			*pbsd_harness_global_locale(void);
pbsd_nl_hook_t		*pbsd_harness_nl_hook(void);
pbsd_rune_hook_t	*pbsd_harness_rune_hook(void);
ref_rune_locale		*pbsd_harness_default_rune(void);
void			pbsd_reset_hooks(void);
void			pbsd_oracle_init(void);

extern int		__mb_cur_max;

int	ref__none_mbsinit(const ref_mbstate_t *);
size_t	ref__none_mbrtowc(wchar_t *, const char *, size_t, ref_mbstate_t *);
size_t	ref__none_wcrtomb(char *, wchar_t, ref_mbstate_t *);
size_t	ref__none_mbsnrtowcs(wchar_t *, const char **, size_t, size_t,
	    ref_mbstate_t *);
size_t	ref__none_wcsnrtombs(char *, const wchar_t **, size_t, size_t,
	    ref_mbstate_t *);

char	*ref_nl_langinfo_l(int item, ref_locale_t);
char	*ref_nl_langinfo(int item);

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
	F_NONE_MBSINIT, F_NONE_MBRTOWC, F_NONE_WCRTOMB,
	F_NONE_MBSNRTOWCS, F_NONE_WCSNRTOMBS,
	F_NL_LANGINFO_L, F_NL_LANGINFO,
	F_GB_MBSINIT, F_GB_MBRTOWC, F_GB_WCRTOMB,
	F_GB_MBSNRTOWCS, F_GB_WCSNRTOMBS,
	F_GET_RUNE, F_WRAP_SETRUNE, F_SET_THREAD, F_CTYPE_LOAD,
	F_COUNT
};

static const char *const fname[F_COUNT] = {
	"_none_mbsinit", "_none_mbrtowc", "_none_wcrtomb",
	"_none_mbsnrtowcs", "_none_wcsnrtombs",
	"nl_langinfo_l", "nl_langinfo",
	"_GB18030_mbsinit", "_GB18030_mbrtowc", "_GB18030_wcrtomb",
	"_GB18030_mbsnrtowcs", "_GB18030_wcsnrtombs",
	"__getCurrentRuneLocale", "__wrap_setrunelocale",
	"__set_thread_rune_locale", "__ctype_load"
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int nprinted[F_COUNT];

static constexpr unsigned char GUARD = 0x7f;
static constexpr wchar_t WGUARD = (wchar_t)0x7f7f;
static constexpr long long SWEEP = 200000;

#ifndef CODESET
#define CODESET		0
#define RADIXCHAR	62
#define CRNCYSTR	68
#endif

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
set_state_byte(P::mbstate_t &s, unsigned char b)
{
	ref_mbstate_t rs{};

	rs.__mbstate8[0] = (char)b;
	mb_copy(rs, s);
}

static void
set_state_bytes(P::mbstate_t &s, const ref_mbstate_t &rs)
{
	mb_copy(rs, s);
}

static ref_mbstate_t
make_state_byte(unsigned char b)
{
	ref_mbstate_t rs{};

	rs.__mbstate8[0] = (char)b;
	return (rs);
}

static ref_xlocale_ctype *
ref_ctype(void)
{
	ref_xlocale *xl = (ref_xlocale *)pbsd_harness_global_locale();

	return ((ref_xlocale_ctype *)xl->components[1]);
}

static void
sync_encoding(const char *enc)
{
	pbsd_nl_hook_t *nl = pbsd_harness_nl_hook();
	ref_rune_locale *def = pbsd_harness_default_rune();
	ref_xlocale_ctype *ct = ref_ctype();

	std::strncpy(nl->encoding, enc, sizeof(nl->encoding) - 1);
	nl->encoding[sizeof(nl->encoding) - 1] = '\0';
	std::strncpy(def->__encoding, enc, sizeof(def->__encoding) - 1);
	def->__encoding[sizeof(def->__encoding) - 1] = '\0';
	if (ct != nullptr && ct->runes != nullptr) {
		std::strncpy(ct->runes->__encoding, enc, 31);
		ct->runes->__encoding[31] = '\0';
	}
}

template<typename MbsinitFn, typename RefMbsinitFn>
static void
test_mbsinit(int f, MbsinitFn fn, RefMbsinitFn ref_fn)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};

	chk_int(f, fn(nullptr), ref_fn(nullptr));
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	chk_int(f, fn(&ps), ref_fn(&rs));
	set_state_byte(ps, 1);
	rs.__mbstate8[0] = 1;
	chk_int(f, fn(&ps), ref_fn(&rs));
	std::memset(&ps, 0x55, sizeof(ps));
	std::memset(&rs, 0x55, sizeof(rs));
	chk_int(f, fn(&ps), ref_fn(&rs));
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
edge_none_mbr(int f, MbrFn fn, RefMbrFn ref_fn)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	char buf[8];

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	run_mbr(f, fn, ref_fn, nullptr, 0, ps, rs, true);
	run_mbr(f, fn, ref_fn, nullptr, 0, ps, rs, false);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = 'a';
	run_mbr(f, fn, ref_fn, buf, 0, ps, rs, false);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = 'Z'; buf[1] = '\0';
	run_mbr(f, fn, ref_fn, buf, 2, ps, rs, false);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = '\0';
	run_mbr(f, fn, ref_fn, buf, 1, ps, rs, false);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0x80; buf[1] = (char)0xff;
	run_mbr(f, fn, ref_fn, buf, 1, ps, rs, false);
	run_mbr(f, fn, ref_fn, buf, 2, ps, rs, false);
}

template<typename MbrFn, typename RefMbrFn>
static void
edge_gb_mbr(int f, MbrFn fn, RefMbrFn ref_fn)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	unsigned char buf[8];

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	run_mbr(f, fn, ref_fn, nullptr, 0, ps, rs, true);

	buf[0] = 'A';
	run_mbr(f, fn, ref_fn, (const char *)buf, 1, ps, rs, false);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = 0x81; buf[1] = 0x40;
	run_mbr(f, fn, ref_fn, (const char *)buf, 1, ps, rs, false);
	run_mbr(f, fn, ref_fn, (const char *)buf, 2, ps, rs, false);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = 0x81; buf[1] = 0x30; buf[2] = 0x81; buf[3] = 0x30;
	run_mbr(f, fn, ref_fn, (const char *)buf, 4, ps, rs, false);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = 0x80;
	run_mbr(f, fn, ref_fn, (const char *)buf, 1, ps, rs, false);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = 0x81; buf[1] = 0x20;
	run_mbr(f, fn, ref_fn, (const char *)buf, 2, ps, rs, false);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	set_state_bytes(ps, make_state_byte(99));
	rs.__mbstate8[0] = 99;
	buf[0] = 0x81; buf[1] = 0x40;
	run_mbr(f, fn, ref_fn, (const char *)buf, 2, ps, rs, false);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = 0x81; buf[1] = 0x30; buf[2] = 0x7f;
	run_mbr(f, fn, ref_fn, (const char *)buf, 3, ps, rs, false);
}

template<typename MbrFn, typename RefMbrFn>
static void
sweep_mbr(int f, MbrFn fn, RefMbrFn ref_fn, bool gb)
{
	char in[16];

	for (long long i = 0; i < SWEEP / 5; i++) {
		P::mbstate_t ps{};
		ref_mbstate_t rs{};
		size_t len = u32(8) + 1;
		bool null_pwc = (u32(2) == 0);
		size_t n = u32((uint32_t)len + 2);

		if (u32(4) == 0)
			std::memset(&ps, 0, sizeof(ps));
		else if (gb && u32(3) == 0)
			set_state_byte(ps, (unsigned char)u32(5));
		else if (!gb && u32(3) == 0)
			std::memcpy(&ps, &len, sizeof(len));
		else
			std::memset(&ps, 0x55, sizeof(ps));
		mb_copy(ps, rs);

		for (size_t j = 0; j < len; j++) {
			if (gb) {
				switch (u32(6)) {
				case 0: in[j] = (char)u32(256); break;
				case 1: in[j] = (char)(0x30 + u32(16)); break;
				case 2: in[j] = (char)(0x81 + u32(126)); break;
				default: in[j] = (char)(32 + u32(96)); break;
				}
			} else {
				in[j] = (char)(u32(256));
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
	if (!null_out && std::memcmp(ob, rb, sizeof(ob)) != 0) {
		report(f, "outbuf");
		return (false);
	}
	if (!mb_eq(ps, rs)) {
		report(f, "state");
		return (false);
	}
	return (true);
}

template<typename WctFn, typename RefWctFn>
static void
edge_none_wct(int f, WctFn fn, RefWctFn ref_fn)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	char scratch[8];

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	run_wct(f, fn, ref_fn, scratch, 8, L'a', ps, rs, true);
	run_wct(f, fn, ref_fn, scratch, 8, L'a', ps, rs, false);
	run_wct(f, fn, ref_fn, scratch, 8, L'\0', ps, rs, false);
	run_wct(f, fn, ref_fn, scratch, 8, (wchar_t)0xff, ps, rs, false);
	run_wct(f, fn, ref_fn, scratch, 8, (wchar_t)-1, ps, rs, false);
	run_wct(f, fn, ref_fn, scratch, 8, (wchar_t)(UCHAR_MAX + 1), ps, rs,
	    false);
}

template<typename WctFn, typename RefWctFn>
static void
edge_gb_wct(int f, WctFn fn, RefWctFn ref_fn)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	char scratch[8];

	edge_none_wct(f, fn, ref_fn);
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	run_wct(f, fn, ref_fn, scratch, 8, (wchar_t)0x8140, ps, rs, false);
	run_wct(f, fn, ref_fn, scratch, 8, (wchar_t)0x01308130, ps, rs, false);
	run_wct(f, fn, ref_fn, scratch, 8, (wchar_t)0x00ff0000, ps, rs, false);

	set_state_byte(ps, 1);
	rs.__mbstate8[0] = 1;
	run_wct(f, fn, ref_fn, scratch, 8, L'A', ps, rs, false);
}

template<typename WctFn, typename RefWctFn>
static void
sweep_wct(int f, WctFn fn, RefWctFn ref_fn)
{
	char scratch[8];

	for (long long i = 0; i < SWEEP / 5; i++) {
		P::mbstate_t ps{};
		ref_mbstate_t rs{};
		wchar_t wc = (wchar_t)(rnd() & 0xffffffff);
		bool null_out = (u32(5) == 0);

		if (u32(3) == 0)
			set_state_byte(ps, (unsigned char)u32(4));
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
		"", "a", "ab", "\x80", "\xff", "a\0b", "\x81\x40",
		"\x81\x30\x81\x30"
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

	for (long long i = 0; i < SWEEP / 10; i++) {
		size_t blen = u32(12) + 1;
		for (size_t j = 0; j < blen; j++)
			in[j] = (char)(u32(256));
		in[blen] = '\0';
		if (u32(4) == 0)
			in[u32((uint32_t)blen + 1)] = '\0';
		run_mbs(f, fn, ref_fn, in, u32(16), u32(10), u32(3) == 0);
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
		L"a\x00b", (const wchar_t *)L"\x8140",
		(const wchar_t *)L"\x01308130"
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

	for (long long i = 0; i < SWEEP / 10; i++) {
		size_t n = u32(8) + 1;
		for (size_t j = 0; j < n; j++)
			in[j] = (wchar_t)(rnd() & 0xffff);
		in[n] = L'\0';
		run_wcs(f, fn, ref_fn, in, u32(12), u32(8) + 1, u32(4) == 0);
	}
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
	if (pp != nullptr && rp != nullptr && std::strcmp(pp, rp) != 0)
		report(f, "string");
}

static void
test_nl_langinfo(void)
{
	static const struct {
		const char *enc;
		const char *expect;
	} codesets[] = {
		{ "EUC-CN", "eucCN" },
		{ "EUC-JP", "eucJP" },
		{ "EUC-KR", "eucKR" },
		{ "EUC-TW", "eucTW" },
		{ "BIG5", "Big5" },
		{ "MSKanji", "SJIS" },
		{ "NONE", "US-ASCII" },
		{ "NONE:US-ASCII", "US-ASCII" },
		{ "NONE:foo", "foo" },
		{ "UTF-8", "UTF-8" },
		{ "GB18030", "GB18030" },
	};

	for (const auto &cs : codesets)
		test_nl_one(F_NL_LANGINFO_L, CODESET, cs.enc);

	pbsd_nl_hook_t *nl = pbsd_harness_nl_hook();
	nl->monetary.currency_symbol = "$";
	nl->conv.p_cs_precedes = 1;
	nl->conv.n_cs_precedes = 1;
	test_nl_one(F_NL_LANGINFO_L, CRNCYSTR, "UTF-8");

	nl->conv.p_cs_precedes = (char)CHAR_MAX;
	nl->monetary.currency_symbol = ".";
	nl->monetary.mon_decimal_point = ".";
	test_nl_one(F_NL_LANGINFO_L, CRNCYSTR, "UTF-8");

	test_nl_one(F_NL_LANGINFO_L, RADIXCHAR, "UTF-8");
	test_nl_one(F_NL_LANGINFO_L, 7 /*DAY_1*/, "UTF-8");
	test_nl_one(F_NL_LANGINFO_L, 999, "UTF-8");

	sync_encoding("EUC-KR");
	char *pp = P::nl_langinfo(CODESET);
	char *rp = ref_nl_langinfo(CODESET);
	ncase[F_NL_LANGINFO]++;
	if (pp == nullptr || rp == nullptr || std::strcmp(pp, rp) != 0)
		report(F_NL_LANGINFO, "nl_langinfo");
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
	pbsd_rune_hook_t *rh = pbsd_harness_rune_hook();

	rh->read_fail = 0;
	rh->asprintf_fail = 0;

	rh->encoding = "C";
	int pv = P::__wrap_setrunelocale("C");
	int rv = ref___wrap_setrunelocale("C");
	ncase[f]++;
	if (pv != rv || __mb_cur_max != 1)
		report(f, "C locale");

	rh->encoding = "GB18030";
	pv = P::__wrap_setrunelocale("zh_CN.GB18030");
	rv = ref___wrap_setrunelocale("zh_CN.GB18030");
	ncase[f]++;
	if (pv != rv || __mb_cur_max != 4)
		report(f, "GB18030");

	test_get_rune();

	rh->encoding = "UTF-8";
	pv = P::__wrap_setrunelocale("en_US.UTF-8");
	rv = ref___wrap_setrunelocale("en_US.UTF-8");
	ncase[f]++;
	if (pv != rv || __mb_cur_max != 4)
		report(f, "UTF-8");

	rh->encoding = "BOGUS";
	pv = P::__wrap_setrunelocale("bogus");
	rv = ref___wrap_setrunelocale("bogus");
	ncase[f]++;
	if (pv != rv)
		report(f, "EFTYPE");

	rh->read_fail = 1;
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
	void *gl = pbsd_harness_global_locale();

	P::__set_thread_rune_locale(nullptr);
	ref___set_thread_rune_locale(nullptr);
	ncase[f]++;

	P::__set_thread_rune_locale((P::locale_t)-1);
	ref___set_thread_rune_locale((ref_locale_t)-1);
	ncase[f]++;

	P::__set_thread_rune_locale((P::locale_t)gl);
	ref___set_thread_rune_locale((ref_locale_t)gl);
	ncase[f]++;
}

static void
test_ctype_load(void)
{
	const int f = F_CTYPE_LOAD;
	pbsd_rune_hook_t *rh = pbsd_harness_rune_hook();

	rh->read_fail = 0;
	rh->encoding = "NONE";
	void *pp = P::__ctype_load("NONE", nullptr);
	void *rp = ref___ctype_load("NONE", nullptr);
	ncase[f]++;
	if ((pp == nullptr) != (rp == nullptr))
		report(f, "load");
	if (pp != nullptr)
		std::free(pp);
	if (rp != nullptr)
		std::free(rp);

	rh->encoding = "GB18030";
	pp = P::__ctype_load("zh_CN.GB18030", nullptr);
	rp = ref___ctype_load("zh_CN.GB18030", nullptr);
	ncase[f]++;
	if ((pp == nullptr) != (rp == nullptr))
		report(f, "GB load");
	if (pp != nullptr)
		std::free(pp);
	if (rp != nullptr)
		std::free(rp);

	rh->read_fail = 1;
	pp = P::__ctype_load("bad", nullptr);
	rp = ref___ctype_load("bad", nullptr);
	ncase[f]++;
	if ((pp == nullptr) != (rp == nullptr))
		report(f, "fail load");
}

static void
sweep_nl(void)
{
	static const char *encs[] = {
		"UTF-8", "EUC-JP", "NONE", "NONE:foo", "BIG5", "MSKanji", "GB18030"
	};

	for (long long i = 0; i < SWEEP / 10; i++) {
		int item = (int)(rnd() % 80);
		const char *enc = encs[u32(7)];
		test_nl_one(F_NL_LANGINFO_L, item, enc);
	}
}

int
main(void)
{
	long long total_fail = 0;

	pbsd_oracle_init();
	pbsd_reset_hooks();
	__mb_cur_max = 4;

	test_mbsinit(F_NONE_MBSINIT, P::_none_mbsinit, ref__none_mbsinit);
	edge_none_mbr(F_NONE_MBRTOWC, P::_none_mbrtowc, ref__none_mbrtowc);
	edge_none_wct(F_NONE_WCRTOMB, P::_none_wcrtomb, ref__none_wcrtomb);
	edge_mbs(F_NONE_MBSNRTOWCS, P::_none_mbsnrtowcs, ref__none_mbsnrtowcs);
	edge_wcs(F_NONE_WCSNRTOMBS, P::_none_wcsnrtombs, ref__none_wcsnrtombs);

	test_mbsinit(F_GB_MBSINIT, P::_GB18030_mbsinit, ref__GB18030_mbsinit);
	edge_gb_mbr(F_GB_MBRTOWC, P::_GB18030_mbrtowc, ref__GB18030_mbrtowc);
	edge_gb_wct(F_GB_WCRTOMB, P::_GB18030_wcrtomb, ref__GB18030_wcrtomb);
	edge_mbs(F_GB_MBSNRTOWCS, P::_GB18030_mbsnrtowcs,
	    ref__GB18030_mbsnrtowcs);
	edge_wcs(F_GB_WCSNRTOMBS, P::_GB18030_wcsnrtombs,
	    ref__GB18030_wcsnrtombs);

	test_nl_langinfo();
	test_wrap_setrunelocale();
	test_set_thread();
	test_ctype_load();

	__mb_cur_max = 4;

	sweep_mbr(F_NONE_MBRTOWC, P::_none_mbrtowc, ref__none_mbrtowc, false);
	sweep_wct(F_NONE_WCRTOMB, P::_none_wcrtomb, ref__none_wcrtomb);
	sweep_mbs(F_NONE_MBSNRTOWCS, P::_none_mbsnrtowcs, ref__none_mbsnrtowcs);
	sweep_wcs(F_NONE_WCSNRTOMBS, P::_none_wcsnrtombs, ref__none_wcsnrtombs);

	sweep_mbr(F_GB_MBRTOWC, P::_GB18030_mbrtowc, ref__GB18030_mbrtowc, true);
	sweep_wct(F_GB_WCRTOMB, P::_GB18030_wcrtomb, ref__GB18030_wcrtomb);
	sweep_mbs(F_GB_MBSNRTOWCS, P::_GB18030_mbsnrtowcs,
	    ref__GB18030_mbsnrtowcs);
	sweep_wcs(F_GB_WCSNRTOMBS, P::_GB18030_wcsnrtombs,
	    ref__GB18030_wcsnrtombs);

	sweep_nl();

	std::printf("b0159 differential harness\n");
	std::printf("%-28s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-28s %12lld %12lld\n", fname[i], ncase[i],
		    nfail[i]);
		total_fail += nfail[i];
	}
	return (total_fail == 0 ? 0 : 1);
}
