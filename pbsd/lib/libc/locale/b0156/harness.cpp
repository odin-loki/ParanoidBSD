/*
 * Differential harness for batch b0156.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>

import pbsd.lib.libc.locale.b0156;

namespace P = pbsd::lib_libc_locale::b0156;

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
	size_t		(*__mbrtowc)(wchar_t *, const char *, size_t, ref_mbstate_t *);
	size_t		(*__wcrtomb)(char *, wchar_t, ref_mbstate_t *);
	int		(*__mbsinit)(const ref_mbstate_t *);
	size_t		(*__mbsnrtowcs)(wchar_t *, const char **, size_t, size_t,
		    ref_mbstate_t *);
	size_t		(*__wcsnrtombs)(char *, const wchar_t **, size_t, size_t,
		    ref_mbstate_t *);
	ref_rune_locale	*runes;
	int		__mb_cur_max;
	int		__mb_sb_limit;
};

struct ref_xlocale {
	void		*components[8];
};

typedef struct {
	int directive_count;
	int chain_count;
	int large_count;
	int subst_count[10];
	int directive[10];
	int undef_pri[10];
	uint32_t pri_count[10];
	int chain_max_len;
} ref_collate_info_t;

typedef struct { int pri[10]; } ref_collate_char_t;
typedef struct { wchar_t str[32]; int pri[10]; } ref_collate_chain_t;
typedef struct { wchar_t val; struct { int pri[10]; } pri; } ref_collate_large_t;
typedef struct { wchar_t key; int32_t pri[16]; } ref_collate_subst_t;

struct ref_xlocale_collate {
	struct { void (*destructor)(void *); char version[16]; char name[32]; } header;
	int __collate_load_error;
	char *map;
	size_t maplen;
	ref_collate_info_t *info;
	ref_collate_char_t *char_pri_table;
	ref_collate_subst_t *subst_table[10];
	ref_collate_chain_t *chain_pri_table;
	ref_collate_large_t *large_pri_table;
};

extern ref_xlocale_collate ref___xlocale_C_collate;
extern ref_rune_locale ref__DefaultRuneLocale;

extern "C" void pbsd_reset_collate_hooks(void);

ref_rune_locale *ref___runes_for_locale(ref_xlocale *, int *);
int ref__UTF8_init(ref_xlocale_ctype *, ref_rune_locale *);
int ref__UTF8_mbsinit(const ref_mbstate_t *);
size_t ref__UTF8_mbrtowc(wchar_t *, const char *, size_t, ref_mbstate_t *);
size_t ref__UTF8_mbsnrtowcs(wchar_t *, const char **, size_t, size_t, ref_mbstate_t *);
size_t ref__UTF8_wcrtomb(char *, wchar_t, ref_mbstate_t *);
size_t ref__UTF8_wcsnrtombs(char *, const wchar_t **, size_t, size_t, ref_mbstate_t *);

int ref__EUC_mbsinit(const ref_mbstate_t *);
int ref__EUC_CN_init(ref_xlocale_ctype *, ref_rune_locale *);
size_t ref__EUC_CN_mbrtowc(wchar_t *, const char *, size_t, ref_mbstate_t *);
size_t ref__EUC_CN_wcrtomb(char *, wchar_t, ref_mbstate_t *);
size_t ref__EUC_CN_mbsnrtowcs(wchar_t *, const char **, size_t, size_t, ref_mbstate_t *);
size_t ref__EUC_CN_wcsnrtombs(char *, const wchar_t **, size_t, size_t, ref_mbstate_t *);
int ref__EUC_KR_init(ref_xlocale_ctype *, ref_rune_locale *);
size_t ref__EUC_KR_mbrtowc(wchar_t *, const char *, size_t, ref_mbstate_t *);
size_t ref__EUC_KR_wcrtomb(char *, wchar_t, ref_mbstate_t *);
size_t ref__EUC_KR_mbsnrtowcs(wchar_t *, const char **, size_t, size_t, ref_mbstate_t *);
size_t ref__EUC_KR_wcsnrtombs(char *, const wchar_t **, size_t, size_t, ref_mbstate_t *);
int ref__EUC_JP_init(ref_xlocale_ctype *, ref_rune_locale *);
size_t ref__EUC_JP_mbrtowc(wchar_t *, const char *, size_t, ref_mbstate_t *);
size_t ref__EUC_JP_wcrtomb(char *, wchar_t, ref_mbstate_t *);
size_t ref__EUC_JP_mbsnrtowcs(wchar_t *, const char **, size_t, size_t, ref_mbstate_t *);
size_t ref__EUC_JP_wcsnrtombs(char *, const wchar_t **, size_t, size_t, ref_mbstate_t *);
int ref__EUC_TW_init(ref_xlocale_ctype *, ref_rune_locale *);
size_t ref__EUC_TW_mbrtowc(wchar_t *, const char *, size_t, ref_mbstate_t *);
size_t ref__EUC_TW_wcrtomb(char *, wchar_t, ref_mbstate_t *);
size_t ref__EUC_TW_mbsnrtowcs(wchar_t *, const char **, size_t, size_t, ref_mbstate_t *);
size_t ref__EUC_TW_wcsnrtombs(char *, const wchar_t **, size_t, size_t, ref_mbstate_t *);
size_t ref__EUC_mbrtowc_impl(wchar_t *, const char *, size_t, ref_mbstate_t *,
    uint8_t, uint8_t, uint8_t, uint8_t);
size_t ref__EUC_wcrtomb_impl(char *, wchar_t, ref_mbstate_t *, uint8_t, uint8_t,
    uint8_t, uint8_t);

void *ref___collate_load(const char *, ref_xlocale *);
int ref___collate_load_tables(const char *);
void ref__collate_lookup(ref_xlocale_collate *, const wchar_t *, int *, int *, int,
    const int **);
size_t ref__collate_wxfrm(ref_xlocale_collate *, const wchar_t *, wchar_t *, size_t);
size_t ref__collate_sxfrm(ref_xlocale_collate *, const wchar_t *, char *, size_t);
int ref___collate_equiv_value(ref_xlocale *, const wchar_t *, size_t);
size_t ref___collate_collating_symbol(wchar_t *, size_t, const char *, size_t,
    ref_mbstate_t *);
int ref___collate_equiv_class(const char *, size_t, ref_mbstate_t *);
size_t ref___collate_equiv_match(int, wchar_t *, size_t, wchar_t, const char *,
    size_t, ref_mbstate_t *, size_t *);
const int32_t *ref_substsearch(ref_xlocale_collate *, const wchar_t, int);
ref_collate_chain_t *ref_chainsearch(ref_xlocale_collate *, const wchar_t *, int *);
ref_collate_large_t *ref_largesearch(ref_xlocale_collate *, const wchar_t);
}

#define F_LIST \
	X(RUNES) \
	X(UTF8_INIT) X(UTF8_MBSINIT) X(UTF8_MBRTOWC) X(UTF8_MBSNRTOWCS) \
	X(UTF8_WCRTOMB) X(UTF8_WCSNRTOMBS) \
	X(EUC_MBSINIT) \
	X(EUC_CN_INIT) X(EUC_CN_MBRTOWC) X(EUC_CN_WCRTOMB) \
	X(EUC_CN_MBSNRTOWCS) X(EUC_CN_WCSNRTOMBS) \
	X(EUC_KR_INIT) X(EUC_KR_MBRTOWC) X(EUC_KR_WCRTOMB) \
	X(EUC_KR_MBSNRTOWCS) X(EUC_KR_WCSNRTOMBS) \
	X(EUC_JP_INIT) X(EUC_JP_MBRTOWC) X(EUC_JP_WCRTOMB) \
	X(EUC_JP_MBSNRTOWCS) X(EUC_JP_WCSNRTOMBS) \
	X(EUC_TW_INIT) X(EUC_TW_MBRTOWC) X(EUC_TW_WCRTOMB) \
	X(EUC_TW_MBSNRTOWCS) X(EUC_TW_WCSNRTOMBS) \
	X(EUC_MBRTOWC_IMPL) X(EUC_WCRTOMB_IMPL) \
	X(COLLATE_LOAD) X(COLLATE_LOAD_TABLES) X(COLLATE_LOOKUP) \
	X(COLLATE_WXFRM) X(COLLATE_SXFRM) X(COLLATE_EQUIV_VALUE) \
	X(COLLATE_COLLATING_SYMBOL) X(COLLATE_EQUIV_CLASS) \
	X(COLLATE_EQUIV_MATCH) X(SUBSTSEARCH) X(CHAINSEARCH) X(LARGESSEARCH)

enum {
#define X(n) F_##n,
	F_LIST
#undef X
	F_COUNT
};

static const char *const fname[F_COUNT] = {
#define X(n) #n,
	F_LIST
#undef X
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int nprinted[F_COUNT];

static constexpr unsigned char GUARD = 0x7f;
static constexpr wchar_t WGUARD = (wchar_t)0x7f7f;
static constexpr long long SWEEP = 200000;

static uint64_t rng = 0xB0156001ULL;

static uint64_t rnd(void)
{
	rng ^= rng << 13;
	rng ^= rng >> 7;
	rng ^= rng << 17;
	return (rng);
}

static uint32_t u32(uint32_t m)
{
	return ((uint32_t)(rnd() % m));
}

static void report(int f, const char *why)
{
	nfail[f]++;
	if (nprinted[f]++ < 8)
		std::printf("  FAIL %-28s : %s\n", fname[f], why);
}

static void mb_copy(const P::mbstate_t &s, ref_mbstate_t &d)
{
	std::memset(&d, 0, sizeof(d));
	std::memcpy(&d, &s, sizeof(ref_mbstate_t));
}

static void mb_copy(const ref_mbstate_t &s, P::mbstate_t &d)
{
	std::memset(&d, 0, sizeof(d));
	std::memcpy(&d, &s, sizeof(ref_mbstate_t));
}

static bool mb_eq(const P::mbstate_t &a, const ref_mbstate_t &b)
{
	return (std::memcmp(&a, &b, sizeof(ref_mbstate_t)) == 0);
}

static void fill_guard(unsigned char *b, size_t n)
{
	std::memset(b, GUARD, n);
}

static void fill_wguard(wchar_t *b, size_t n)
{
	for (size_t i = 0; i < n; i++)
		b[i] = WGUARD;
}

static bool chk_ret(int f, size_t pv, size_t rv, int perrno, int rerrno)
{
	ncase[f]++;
	if (pv != rv || perrno != rerrno) {
		report(f, "ret/errno");
		return (false);
	}
	return (true);
}

static bool chk_int(int f, int pv, int rv)
{
	ncase[f]++;
	if (pv != rv) {
		report(f, "return");
		return (false);
	}
	return (true);
}

static void set_state_ch(P::mbstate_t &s, wchar_t ch)
{
	std::memcpy(&s, &ch, sizeof(ch));
}

static void set_state_ch(ref_mbstate_t &s, wchar_t ch)
{
	std::memcpy(&s, &ch, sizeof(ch));
}

static void set_utf8_state(P::mbstate_t &s, int want, wchar_t ch, wchar_t lb)
{
	struct { wchar_t ch; int want; wchar_t lbound; } st;
	st.ch = ch; st.want = want; st.lbound = lb;
	std::memcpy(&s, &st, sizeof(st));
}

static void set_utf8_state(ref_mbstate_t &s, int want, wchar_t ch, wchar_t lb)
{
	struct { wchar_t ch; int want; wchar_t lbound; } st;
	st.ch = ch; st.want = want; st.lbound = lb;
	std::memcpy(&s, &st, sizeof(st));
}

static void set_euc_state(P::mbstate_t &s, int want, wchar_t ch)
{
	struct { wchar_t ch; int set; int want; } st;
	st.ch = ch; st.set = 0; st.want = want;
	std::memcpy(&s, &st, sizeof(st));
}

static void set_euc_state(ref_mbstate_t &s, int want, wchar_t ch)
{
	struct { wchar_t ch; int set; int want; } st;
	st.ch = ch; st.set = 0; st.want = want;
	std::memcpy(&s, &st, sizeof(st));
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



static void test_runes_for_locale(int f)
{
	P::xlocale_ctype pct{};
	ref_xlocale_ctype rct{};
	P::xlocale pl{};
	ref_xlocale rl{};

	pct.runes = const_cast<P::_RuneLocale *>(P::default_rune_locale());
	pct.__mb_sb_limit = 128;
	rct.runes = &ref__DefaultRuneLocale;
	rct.__mb_sb_limit = 128;
	pl.components[1] = &pct;
	rl.components[1] = &rct;

	int plim{}, rlim{};
	P::_RuneLocale *pr = P::__runes_for_locale(&pl, &plim);
	ref_rune_locale *rr = ref___runes_for_locale(&rl, &rlim);
	ncase[f]++;
	if (pr != P::default_rune_locale() || rr != &ref__DefaultRuneLocale ||
	    plim != 128 || rlim != 128)
		report(f, "values");
}

static void edge_utf8_mbr(int f)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	char buf[8];

	edge_mbr(f, P::UTF8_mbrtowc, ref__UTF8_mbrtowc);
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0xc0; buf[1] = (char)0x80;
	run_mbr(f, P::UTF8_mbrtowc, ref__UTF8_mbrtowc, buf, 2, ps, rs, false);
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0xc2; buf[1] = (char)0xa9;
	run_mbr(f, P::UTF8_mbrtowc, ref__UTF8_mbrtowc, buf, 2, ps, rs, false);
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0xed; buf[1] = (char)0xa0; buf[2] = (char)0x80;
	run_mbr(f, P::UTF8_mbrtowc, ref__UTF8_mbrtowc, buf, 3, ps, rs, false);
	set_utf8_state(ps, 2, 0, 0x80);
	set_utf8_state(rs, 2, 0, 0x80);
	buf[0] = (char)0x80;
	run_mbr(f, P::UTF8_mbrtowc, ref__UTF8_mbrtowc, buf, 1, ps, rs, false);
	set_utf8_state(ps, 7, 0, 0);
	set_utf8_state(rs, 7, 0, 0);
	buf[0] = 'a';
	run_mbr(f, P::UTF8_mbrtowc, ref__UTF8_mbrtowc, buf, 1, ps, rs, false);
}

static void edge_utf8_wct(int f)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	char scratch[8];

	edge_wct(f, P::UTF8_wcrtomb, ref__UTF8_wcrtomb);
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	run_wct(f, P::UTF8_wcrtomb, ref__UTF8_wcrtomb, scratch, 8, (wchar_t)0xd800,
	    ps, rs, false);
	run_wct(f, P::UTF8_wcrtomb, ref__UTF8_wcrtomb, scratch, 8, (wchar_t)0x10000,
	    ps, rs, false);
}

template<typename MbrFn, typename RefMbrFn>
static void edge_euc_mbr(int f, MbrFn fn, RefMbrFn ref_fn)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	char buf[8];

	edge_mbr(f, fn, ref_fn);
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0xa1; buf[1] = (char)0xb2;
	run_mbr(f, fn, ref_fn, buf, 1, ps, rs, false);
	run_mbr(f, fn, ref_fn, buf, 2, ps, rs, false);
	set_euc_state(ps, 1, 0);
	set_euc_state(rs, 1, 0);
	buf[0] = (char)0xb2;
	run_mbr(f, fn, ref_fn, buf, 1, ps, rs, false);
}

static void edge_euc_jp_mbr(int f)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	char buf[8];

	edge_euc_mbr(f, P::EUC_JP_mbrtowc, ref__EUC_JP_mbrtowc);
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0x8e; buf[1] = (char)0xa1; buf[2] = (char)0xb2;
	run_mbr(f, P::EUC_JP_mbrtowc, ref__EUC_JP_mbrtowc, buf, 3, ps, rs, false);
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0x8f; buf[1] = (char)0xa1; buf[2] = (char)0xb2;
	run_mbr(f, P::EUC_JP_mbrtowc, ref__EUC_JP_mbrtowc, buf, 3, ps, rs, false);
}

template<typename WctFn, typename RefWctFn>
static void edge_euc_wct(int f, WctFn fn, RefWctFn ref_fn)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	char scratch[8];

	edge_wct(f, fn, ref_fn);
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	run_wct(f, fn, ref_fn, scratch, 8, (wchar_t)0xa1b2, ps, rs, false);
}

static P::xlocale_collate *make_port_collate()
{
	auto *t = (P::xlocale_collate *)std::calloc(1, sizeof(P::xlocale_collate));
	auto *info = (P::collate_info_t *)std::calloc(1, sizeof(P::collate_info_t));
	auto *chars = (P::collate_char_t *)std::calloc(256, sizeof(P::collate_char_t));
	auto *chains = (P::collate_chain_t *)std::calloc(1, sizeof(P::collate_chain_t));
	auto *large = (P::collate_large_t *)std::calloc(1, sizeof(P::collate_large_t));
	auto *subst = (P::collate_subst_t *)std::calloc(1, sizeof(P::collate_subst_t));

	info->directive_count = 1;
	info->chain_count = 1;
	info->large_count = 1;
	info->subst_count[0] = 1;
	info->directive[0] = 0x01;
	info->undef_pri[0] = 5;
	info->pri_count[0] = 0x40;
	info->chain_max_len = 2;

	chars['a'].pri[0] = 10;
	chars['b'].pri[0] = 20;
	chars['a'].pri[0] |= 0x80000000;
	subst->key = chars['a'].pri[0];
	subst->pri[0] = 99;
	subst->pri[1] = 0;

	chains->str[0] = L'c'; chains->str[1] = L'h'; chains->str[2] = 0;
	chains->pri[0] = 30;
	large->val = (wchar_t)0x100;
	large->pri.pri[0] = 40;

	t->__collate_load_error = 0;
	t->info = info;
	t->char_pri_table = chars;
	t->subst_table[0] = subst;
	t->chain_pri_table = chains;
	t->large_pri_table = large;
	return (t);
}

static ref_xlocale_collate *make_ref_collate()
{
	auto *t = (ref_xlocale_collate *)std::calloc(1, sizeof(ref_xlocale_collate));
	auto *info = (ref_collate_info_t *)std::calloc(1, sizeof(ref_collate_info_t));
	auto *chars = (ref_collate_char_t *)std::calloc(256, sizeof(ref_collate_char_t));
	auto *chains = (ref_collate_chain_t *)std::calloc(1, sizeof(ref_collate_chain_t));
	auto *large = (ref_collate_large_t *)std::calloc(1, sizeof(ref_collate_large_t));
	auto *subst = (ref_collate_subst_t *)std::calloc(1, sizeof(ref_collate_subst_t));

	info->directive_count = 1;
	info->chain_count = 1;
	info->large_count = 1;
	info->subst_count[0] = 1;
	info->directive[0] = 0x01;
	info->undef_pri[0] = 5;
	info->pri_count[0] = 0x40;
	info->chain_max_len = 2;

	chars['a'].pri[0] = 10;
	chars['b'].pri[0] = 20;
	chars['a'].pri[0] |= 0x80000000;
	subst->key = chars['a'].pri[0];
	subst->pri[0] = 99;
	subst->pri[1] = 0;

	chains->str[0] = L'c'; chains->str[1] = L'h'; chains->str[2] = 0;
	chains->pri[0] = 30;
	large->val = (wchar_t)0x100;
	large->pri.pri[0] = 40;

	t->__collate_load_error = 0;
	t->info = info;
	t->char_pri_table = chars;
	t->subst_table[0] = subst;
	t->chain_pri_table = chains;
	t->large_pri_table = large;
	return (t);
}

static void test_collate_lookup(int f, P::xlocale_collate *pt, ref_xlocale_collate *rt)
{
	const wchar_t *srcs[] = { L"a", L"b", L"ch", L"\x100", L"z" };
	for (const wchar_t *src : srcs) {
		int plen{}, rlen{}, ppri{}, rpri{};
		const int *pstate = nullptr;
		const int *rstate = nullptr;
		P::_collate_lookup(pt, src, &plen, &ppri, 0, &pstate);
		ref__collate_lookup(rt, src, &rlen, &rpri, 0, &rstate);
		ncase[f]++;
		if (plen != rlen || ppri != rpri)
			report(f, "lookup");
	}
}

static void test_collate_xfrm(int fwx, int fsx, P::xlocale_collate *pt, ref_xlocale_collate *rt)
{
	wchar_t pw[64], rw[64];
	unsigned char ps[64], rs[64];
	fill_wguard(pw, 64);
	fill_wguard(rw, 64);
	fill_guard(ps, 64);
	fill_guard(rs, 64);
	errno = 0;
	size_t pn = P::_collate_wxfrm(pt, L"ab", pw, 32);
	int pe = errno;
	errno = 0;
	size_t rn = ref__collate_wxfrm(rt, L"ab", rw, 32);
	int re = errno;
	chk_ret(fwx, pn, rn, pe, re);
	if (std::memcmp(pw, rw, sizeof(pw)) != 0)
		report(fwx, "wxfrm buf");
	fill_guard(ps, 64);
	fill_guard(rs, 64);
	errno = 0;
	pn = P::_collate_sxfrm(pt, L"ab", (char *)ps, 32);
	pe = errno;
	errno = 0;
	rn = ref__collate_sxfrm(rt, L"ab", (char *)rs, 32);
	re = errno;
	chk_ret(fsx, pn, rn, pe, re);
	if (std::memcmp(ps, rs, sizeof(ps)) != 0)
		report(fsx, "sxfrm buf");
}

static void test_collate_equiv_value(int f)
{
	P::xlocale pl{};
	ref_xlocale rl{};
	pl.components[0] = P::collate_c_table();
	rl.components[0] = &ref___xlocale_C_collate;
	chk_int(f, P::__collate_equiv_value((P::locale_t)&pl, L"a", 1),
	    ref___collate_equiv_value(&rl, L"a", 1));
}

static void test_collate_collating_symbol(int f)
{
	const char *src = "a";
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	wchar_t pdst{}, rdst{};
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	size_t psz = P::__collate_collating_symbol(&pdst, 1, src, 1, &ps);
	size_t rsz = ref___collate_collating_symbol(&rdst, 1, src, 1, &rs);
	ncase[f]++;
	if (psz != rsz || (psz == 1 && pdst != rdst))
		report(f, "collating_symbol");
}

static void test_collate_equiv_class(int f)
{
	const char *src = "a";
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	chk_int(f, P::__collate_equiv_class(src, 1, &ps),
	    ref___collate_equiv_class(src, 1, &rs));
}

static void test_collate_equiv_match(int f)
{
	const char *src = "a";
	P::mbstate_t ps{};
	ref_mbstate_t rs{};
	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	size_t psz = P::__collate_equiv_match(10, nullptr, 0, 0, src, 1, &ps, nullptr);
	size_t rsz = ref___collate_equiv_match(10, nullptr, 0, 0, src, 1, &rs, nullptr);
	ncase[f]++;
	if (psz != rsz)
		report(f, "equiv_match");
}

static void test_collate_search(int fsub, int fch, int flg, P::xlocale_collate *pt,
    ref_xlocale_collate *rt)
{
	int pl{}, rl{};
	const int32_t *ps = P::collate_substsearch(pt, pt->char_pri_table['a'].pri[0], 0);
	const int32_t *rs = ref_substsearch(rt, rt->char_pri_table['a'].pri[0], 0);
	ncase[fsub]++;
	if ((ps == nullptr) != (rs == nullptr) || (ps && rs && *ps != *rs))
		report(fsub, "substsearch");
	P::collate_chainsearch(pt, L"ch", &pl);
	ref_chainsearch(rt, L"ch", &rl);
	ncase[fch]++;
	if (pl != rl)
		report(fch, "chainsearch");
	auto *plg = P::collate_largesearch(pt, (wchar_t)0x100);
	auto *rlg = ref_largesearch(rt, (wchar_t)0x100);
	ncase[flg]++;
	if ((plg == nullptr) != (rlg == nullptr) ||
	    (plg && rlg && plg->pri.pri[0] != rlg->pri.pri[0]))
		report(flg, "largesearch");
}

static void sweep_euc_impl(int f)
{
	for (long long i = 0; i < SWEEP; i++) {
		P::mbstate_t ps{};
		ref_mbstate_t rs{};
		char in[8];
		size_t n = u32(4) + 1;
		for (size_t j = 0; j < n; j++)
			in[j] = (char)(u32(256));
		in[n] = '\0';
		uint8_t cs2 = (uint8_t)(0x80 + u32(16));
		uint8_t cs3 = (uint8_t)(0x90 + u32(16));
		run_mbr(f,
		    [&](wchar_t *pwc, const char *s, size_t nb, P::mbstate_t *st) {
			return P::EUC_mbrtowc_impl(pwc, s, nb, st, cs2, 4, cs3, 3);
		    },
		    [&](wchar_t *pwc, const char *s, size_t nb, ref_mbstate_t *st) {
			return ref__EUC_mbrtowc_impl(pwc, s, nb, st, cs2, 4, cs3, 3);
		    },
		    in, u32(8), ps, rs, u32(2) == 0);
	}
}

int main()
{
	P::xlocale_collate *pc = make_port_collate();
	ref_xlocale_collate *rc = make_ref_collate();
	P::xlocale ploc{};

	test_runes_for_locale(F_RUNES);

	test_init(F_UTF8_INIT, P::_UTF8_init, ref__UTF8_init, 4, 128);
	test_mbsinit(F_UTF8_MBSINIT, P::UTF8_mbsinit, ref__UTF8_mbsinit);
	edge_utf8_mbr(F_UTF8_MBRTOWC);
	sweep_mbr(F_UTF8_MBRTOWC, P::UTF8_mbrtowc, ref__UTF8_mbrtowc);
	edge_wct(F_UTF8_WCRTOMB, P::UTF8_wcrtomb, ref__UTF8_wcrtomb);
	sweep_wct(F_UTF8_WCRTOMB, P::UTF8_wcrtomb, ref__UTF8_wcrtomb);
	edge_mbs(F_UTF8_MBSNRTOWCS, P::UTF8_mbsnrtowcs, ref__UTF8_mbsnrtowcs);
	sweep_mbs(F_UTF8_MBSNRTOWCS, P::UTF8_mbsnrtowcs, ref__UTF8_mbsnrtowcs);
	edge_wcs(F_UTF8_WCSNRTOMBS, P::UTF8_wcsnrtombs, ref__UTF8_wcsnrtombs);
	sweep_wcs(F_UTF8_WCSNRTOMBS, P::UTF8_wcsnrtombs, ref__UTF8_wcsnrtombs);

	test_mbsinit(F_EUC_MBSINIT, P::EUC_mbsinit, ref__EUC_mbsinit);

	test_init(F_EUC_CN_INIT, P::_EUC_CN_init, ref__EUC_CN_init, 4, 128);
	edge_euc_mbr(F_EUC_CN_MBRTOWC, P::EUC_CN_mbrtowc, ref__EUC_CN_mbrtowc);
	sweep_mbr(F_EUC_CN_MBRTOWC, P::EUC_CN_mbrtowc, ref__EUC_CN_mbrtowc);
	edge_euc_wct(F_EUC_CN_WCRTOMB, P::EUC_CN_wcrtomb, ref__EUC_CN_wcrtomb);
	sweep_wct(F_EUC_CN_WCRTOMB, P::EUC_CN_wcrtomb, ref__EUC_CN_wcrtomb);
	edge_mbs(F_EUC_CN_MBSNRTOWCS, P::EUC_CN_mbsnrtowcs, ref__EUC_CN_mbsnrtowcs);
	sweep_mbs(F_EUC_CN_MBSNRTOWCS, P::EUC_CN_mbsnrtowcs, ref__EUC_CN_mbsnrtowcs);
	edge_wcs(F_EUC_CN_WCSNRTOMBS, P::EUC_CN_wcsnrtombs, ref__EUC_CN_wcsnrtombs);
	sweep_wcs(F_EUC_CN_WCSNRTOMBS, P::EUC_CN_wcsnrtombs, ref__EUC_CN_wcsnrtombs);

	test_init(F_EUC_KR_INIT, P::_EUC_KR_init, ref__EUC_KR_init, 2, 128);
	edge_euc_mbr(F_EUC_KR_MBRTOWC, P::EUC_KR_mbrtowc, ref__EUC_KR_mbrtowc);
	sweep_mbr(F_EUC_KR_MBRTOWC, P::EUC_KR_mbrtowc, ref__EUC_KR_mbrtowc);
	edge_euc_wct(F_EUC_KR_WCRTOMB, P::EUC_KR_wcrtomb, ref__EUC_KR_wcrtomb);
	sweep_wct(F_EUC_KR_WCRTOMB, P::EUC_KR_wcrtomb, ref__EUC_KR_wcrtomb);
	edge_mbs(F_EUC_KR_MBSNRTOWCS, P::EUC_KR_mbsnrtowcs, ref__EUC_KR_mbsnrtowcs);
	sweep_mbs(F_EUC_KR_MBSNRTOWCS, P::EUC_KR_mbsnrtowcs, ref__EUC_KR_mbsnrtowcs);
	edge_wcs(F_EUC_KR_WCSNRTOMBS, P::EUC_KR_wcsnrtombs, ref__EUC_KR_wcsnrtombs);
	sweep_wcs(F_EUC_KR_WCSNRTOMBS, P::EUC_KR_wcsnrtombs, ref__EUC_KR_wcsnrtombs);

	test_init(F_EUC_JP_INIT, P::_EUC_JP_init, ref__EUC_JP_init, 3, 128);
	edge_euc_jp_mbr(F_EUC_JP_MBRTOWC);
	sweep_mbr(F_EUC_JP_MBRTOWC, P::EUC_JP_mbrtowc, ref__EUC_JP_mbrtowc);
	edge_euc_wct(F_EUC_JP_WCRTOMB, P::EUC_JP_wcrtomb, ref__EUC_JP_wcrtomb);
	sweep_wct(F_EUC_JP_WCRTOMB, P::EUC_JP_wcrtomb, ref__EUC_JP_wcrtomb);
	edge_mbs(F_EUC_JP_MBSNRTOWCS, P::EUC_JP_mbsnrtowcs, ref__EUC_JP_mbsnrtowcs);
	sweep_mbs(F_EUC_JP_MBSNRTOWCS, P::EUC_JP_mbsnrtowcs, ref__EUC_JP_mbsnrtowcs);
	edge_wcs(F_EUC_JP_WCSNRTOMBS, P::EUC_JP_wcsnrtombs, ref__EUC_JP_wcsnrtombs);
	sweep_wcs(F_EUC_JP_WCSNRTOMBS, P::EUC_JP_wcsnrtombs, ref__EUC_JP_wcsnrtombs);

	test_init(F_EUC_TW_INIT, P::_EUC_TW_init, ref__EUC_TW_init, 4, 128);
	edge_euc_mbr(F_EUC_TW_MBRTOWC, P::EUC_TW_mbrtowc, ref__EUC_TW_mbrtowc);
	sweep_mbr(F_EUC_TW_MBRTOWC, P::EUC_TW_mbrtowc, ref__EUC_TW_mbrtowc);
	edge_euc_wct(F_EUC_TW_WCRTOMB, P::EUC_TW_wcrtomb, ref__EUC_TW_wcrtomb);
	sweep_wct(F_EUC_TW_WCRTOMB, P::EUC_TW_wcrtomb, ref__EUC_TW_wcrtomb);
	edge_mbs(F_EUC_TW_MBSNRTOWCS, P::EUC_TW_mbsnrtowcs, ref__EUC_TW_mbsnrtowcs);
	sweep_mbs(F_EUC_TW_MBSNRTOWCS, P::EUC_TW_mbsnrtowcs, ref__EUC_TW_mbsnrtowcs);
	edge_wcs(F_EUC_TW_WCSNRTOMBS, P::EUC_TW_wcsnrtombs, ref__EUC_TW_wcsnrtombs);
	sweep_wcs(F_EUC_TW_WCSNRTOMBS, P::EUC_TW_wcsnrtombs, ref__EUC_TW_wcsnrtombs);

	sweep_euc_impl(F_EUC_MBRTOWC_IMPL);
	for (long long i = 0; i < SWEEP; i++) {
		P::mbstate_t ps{};
		ref_mbstate_t rs{};
		char scratch[8];
		wchar_t wc = (wchar_t)(0xa000 + u32(0x2000));
		uint8_t cs2 = 0x8e, cs3 = 0x8f;
		run_wct(F_EUC_WCRTOMB_IMPL,
		    [&](char *s, wchar_t w, P::mbstate_t *st) {
			return P::EUC_wcrtomb_impl(s, w, st, cs2, 2, cs3, 3);
		    },
		    [&](char *s, wchar_t w, ref_mbstate_t *st) {
			return ref__EUC_wcrtomb_impl(s, w, st, cs2, 2, cs3, 3);
		    },
		    scratch, 8, wc, ps, rs, u32(3) == 0);
	}

	pbsd_reset_collate_hooks();
	void *pload = P::__collate_load("C", (P::locale_t)&ploc);
	void *rload = ref___collate_load("C", nullptr);
	ncase[F_COLLATE_LOAD]++;
	if (pload == nullptr || rload == nullptr)
		report(F_COLLATE_LOAD, "null");
	chk_int(F_COLLATE_LOAD_TABLES, P::__collate_load_tables("C"),
	    ref___collate_load_tables("C"));
	chk_int(F_COLLATE_LOAD_TABLES, P::__collate_load_tables("POSIX"),
	    ref___collate_load_tables("POSIX"));

	test_collate_lookup(F_COLLATE_LOOKUP, pc, rc);
	test_collate_xfrm(F_COLLATE_WXFRM, F_COLLATE_SXFRM, pc, rc);
	test_collate_equiv_value(F_COLLATE_EQUIV_VALUE);
	test_collate_collating_symbol(F_COLLATE_COLLATING_SYMBOL);
	test_collate_equiv_class(F_COLLATE_EQUIV_CLASS);
	test_collate_equiv_match(F_COLLATE_EQUIV_MATCH);
	test_collate_search(F_SUBSTSEARCH, F_CHAINSEARCH, F_LARGESSEARCH, pc, rc);

	std::printf("\n%-28s %12s %12s\n", "function", "cases", "failures");
	long long total_fail = 0;
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-28s %12lld %12lld\n", fname[i], ncase[i], nfail[i]);
		total_fail += nfail[i];
	}
	std::free(pc);
	std::free(rc);
	return (total_fail == 0 ? 0 : 1);
}
