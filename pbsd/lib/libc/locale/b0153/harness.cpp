/*
 * Differential harness for batch b0153.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cwchar>

import pbsd.lib.libc.locale.b0153;

namespace P = pbsd::lib_libc_locale::b0153;

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

struct ref_xlocale_component {
	long		retain_count;
	void		(*destructor)(void *);
	char		locale[32];
	char		version[12];
};

struct ref_xlocale {
	long		retain_count;
	void		(*destructor)(void *);
	struct ref_xlocale_component *components[6];
	int		monetary_locale_changed;
	int		using_monetary_locale;
	int		numeric_locale_changed;
	int		using_numeric_locale;
	int		using_time_locale;
	int		using_messages_locale;
	struct lconv	lconv;
	char		*csym;
};

struct port_xlocale {
	long		retain_count;
	void		(*destructor)(void *);
	struct ref_xlocale_component *components[6];
	int		monetary_locale_changed;
	int		using_monetary_locale;
	int		numeric_locale_changed;
	int		using_numeric_locale;
	int		using_time_locale;
	int		using_messages_locale;
	struct lconv	lconv;
	char		*csym;
};

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

typedef struct {
	int			monetary_changed;
	int			numeric_changed;
} pbsd_localeconv_hook_flags_t;

typedef struct {
	int			open_fail;
	int			open_errno;
	int			fstat_fail;
	int			fstat_errno;
	off_t			file_size;
	const char		*file_content;
	int			read_fail;
	int			read_errno;
	int			malloc_fail;
	int			close_count;
} pbsd_ldpart_hook_t;

typedef struct {
	int			wcsrtombs_fail;
	int			fail_malloc_at;
	int			malloc_calls;
	int			strftime_zero;
	size_t			strftime_len;
	int			mbsrtowcs_fail;
	int			mbsrtowcs_incomplete;
	int			mbsrtowcs_dstp_left;
	size_t			mbsrtowcs_count;
} pbsd_wcsftime_hook_t;

extern struct ref_xlocale	ref_test_locale;
extern struct port_xlocale	port_test_locale;
extern pbsd_ldpart_hook_t	pbsd_ldpart_hook;
extern pbsd_wcsftime_hook_t	pbsd_wcsftime_hook;

void	pbsd_reset_hooks(void);

struct lconv	*ref_localeconv_l(ref_xlocale *);
struct lconv	*ref_localeconv(void);
size_t	ref_wcsftime_l(wchar_t *, size_t, const wchar_t *, const struct tm *,
	    ref_xlocale *);
size_t	ref_wcsftime(wchar_t *, size_t, const wchar_t *, const struct tm *);
int	ref___part_load_locale(const char *, int *, char **, const char *,
	    int, int, const char **);
int	ref__GB2312_init(ref_xlocale_ctype *, ref_rune_locale *);
int	ref__GB2312_mbsinit(const ref_mbstate_t *);
int	ref__GB2312_check(const char *, size_t);
size_t	ref__GB2312_mbrtowc(wchar_t *, const char *, size_t, ref_mbstate_t *);
size_t	ref__GB2312_wcrtomb(char *, wchar_t, ref_mbstate_t *);
size_t	ref__GB2312_mbsnrtowcs(wchar_t *, const char **, size_t, size_t,
	    ref_mbstate_t *);
size_t	ref__GB2312_wcsnrtombs(char *, const wchar_t **, size_t, size_t,
	    ref_mbstate_t *);
}

enum {
	F_LOCALECONV_L, F_LOCALECONV, F_WCSFTIME_L, F_WCSFTIME,
	F_PART_LOAD, F_GB2312_INIT, F_GB2312_MBSINIT, F_GB2312_CHECK,
	F_GB2312_MBRTOWC, F_GB2312_WCRTOMB, F_GB2312_MBSNRTOWCS,
	F_GB2312_WCSNRTOMBS, F_COUNT
};

static const char *const fname[F_COUNT] = {
	"localeconv_l", "localeconv", "wcsftime_l", "wcsftime",
	"__part_load_locale", "GB2312_init", "GB2312_mbsinit", "GB2312_check",
	"GB2312_mbrtowc", "GB2312_wcrtomb", "GB2312_mbsnrtowcs",
	"GB2312_wcsnrtombs"
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int nprinted[F_COUNT];

static constexpr unsigned char GUARD = 0x7f;
static constexpr wchar_t WGUARD = (wchar_t)0x7f7f;
static constexpr long long SWEEP = 200000;

static uint64_t rng = 0xB0153001ULL;

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

static void
set_gb_state(P::mbstate_t &s, int count, unsigned char b0 = 0, unsigned char b1 = 0)
{
	std::memset(&s, 0, sizeof(s));
	s.__mbstate8[0] = (char)count;
	s.__mbstate8[1] = (char)b0;
	s.__mbstate8[2] = (char)b1;
}

static void
set_gb_state(ref_mbstate_t &s, int count, unsigned char b0 = 0, unsigned char b1 = 0)
{
	std::memset(&s, 0, sizeof(s));
	s.__mbstate8[0] = (char)count;
	s.__mbstate8[1] = (char)b0;
	s.__mbstate8[2] = (char)b1;
}

static bool
lconv_eq(const struct lconv *a, const struct lconv *b)
{
#define CMPF(F) (a->F == b->F || (a->F != nullptr && b->F != nullptr && \
    std::strcmp(a->F, b->F) == 0))
	return (CMPF(decimal_point) && CMPF(thousands_sep) && CMPF(grouping) &&
	    CMPF(int_curr_symbol) && CMPF(currency_symbol) &&
	    CMPF(mon_decimal_point) && CMPF(mon_thousands_sep) &&
	    CMPF(mon_grouping) && CMPF(positive_sign) && CMPF(negative_sign) &&
	    a->int_frac_digits == b->int_frac_digits &&
	    a->frac_digits == b->frac_digits &&
	    a->p_cs_precedes == b->p_cs_precedes &&
	    a->p_sep_by_space == b->p_sep_by_space &&
	    a->n_cs_precedes == b->n_cs_precedes &&
	    a->n_sep_by_space == b->n_sep_by_space &&
	    a->p_sign_posn == b->p_sign_posn &&
	    a->n_sign_posn == b->n_sign_posn &&
	    a->int_p_cs_precedes == b->int_p_cs_precedes &&
	    a->int_n_cs_precedes == b->int_n_cs_precedes &&
	    a->int_p_sep_by_space == b->int_p_sep_by_space &&
	    a->int_n_sep_by_space == b->int_n_sep_by_space &&
	    a->int_p_sign_posn == b->int_p_sign_posn &&
	    a->int_n_sign_posn == b->int_n_sign_posn);
#undef CMPF
}

static void
prep_localeconv(ref_xlocale &rl, P::xlocale &pl, int mon, int num)
{
	std::memset(&rl, 0, sizeof(rl));
	std::memset(&pl, 0, sizeof(pl));
	rl.monetary_locale_changed = mon;
	rl.numeric_locale_changed = num;
	pl.monetary_locale_changed = mon;
	pl.numeric_locale_changed = num;
}

static bool
run_localeconv(int f, bool use_l, ref_xlocale &rl, P::xlocale &pl)
{
	struct lconv *pr, *rr;

	ncase[f]++;
	if (use_l) {
		pr = P::localeconv_l(reinterpret_cast<P::locale_t>(&pl));
		rr = ref_localeconv_l(&rl);
	} else {
		pr = P::localeconv();
		rr = ref_localeconv();
	}
	if (pr == nullptr || rr == nullptr) {
		report(f, "null");
		return (false);
	}
	if (!lconv_eq(pr, rr)) {
		report(f, "fields");
		return (false);
	}
	if (rl.monetary_locale_changed != pl.monetary_locale_changed ||
	    rl.numeric_locale_changed != pl.numeric_locale_changed) {
		report(f, "flags");
		return (false);
	}
	if (!use_l) {
		if (ref_test_locale.monetary_locale_changed !=
		    port_test_locale.monetary_locale_changed ||
		    ref_test_locale.numeric_locale_changed !=
		    port_test_locale.numeric_locale_changed) {
			report(f, "gflags");
			return (false);
		}
	}
	return (true);
}

static void
edge_localeconv(int f, bool use_l)
{
	ref_xlocale rl{};
	P::xlocale pl{};

	prep_localeconv(rl, pl, 1, 1);
	run_localeconv(f, use_l, rl, pl);
	prep_localeconv(rl, pl, 1, 0);
	run_localeconv(f, use_l, rl, pl);
	prep_localeconv(rl, pl, 0, 1);
	run_localeconv(f, use_l, rl, pl);
	prep_localeconv(rl, pl, 0, 0);
	run_localeconv(f, use_l, rl, pl);
	if (!use_l) {
		ref_test_locale.monetary_locale_changed = 1;
		ref_test_locale.numeric_locale_changed = 1;
		port_test_locale.monetary_locale_changed = 1;
		port_test_locale.numeric_locale_changed = 1;
		run_localeconv(f, false, rl, pl);
	}
}

static void
sweep_localeconv(int f, bool use_l)
{
	ref_xlocale rl{};
	P::xlocale pl{};

	for (long long i = 0; i < SWEEP; i++) {
		int mon = (int)(u32(2));
		int num = (int)(u32(2));
		prep_localeconv(rl, pl, mon, num);
		if (!use_l) {
		ref_test_locale.monetary_locale_changed = mon;
		ref_test_locale.numeric_locale_changed = num;
		port_test_locale.monetary_locale_changed = mon;
		port_test_locale.numeric_locale_changed = num;
		}
		run_localeconv(f, use_l, rl, pl);
	}
}

static bool
run_wcsftime(int f, bool use_l, const wchar_t *fmt, size_t maxsize,
    const struct tm *tm, bool null_wcs)
{
	wchar_t pw[64], rw[64];
	size_t pr, rr;
	int pe, re;

	fill_wguard(pw, sizeof(pw) / sizeof(pw[0]));
	fill_wguard(rw, sizeof(rw) / sizeof(rw[0]));
	errno = 0;
	if (use_l) {
		pr = P::wcsftime_l(null_wcs ? nullptr : pw, maxsize, fmt, tm,
		    reinterpret_cast<P::locale_t>(&ref_test_locale));
		pe = errno;
		errno = 0;
		rr = ref_wcsftime_l(null_wcs ? nullptr : rw, maxsize, fmt, tm,
		    &ref_test_locale);
		re = errno;
	} else {
		pr = P::wcsftime(null_wcs ? nullptr : pw, maxsize, fmt, tm);
		pe = errno;
		errno = 0;
		rr = ref_wcsftime(null_wcs ? nullptr : rw, maxsize, fmt, tm);
		re = errno;
	}
	if (!chk_ret(f, pr, rr, pe, re))
		return (false);
	if (!null_wcs && pr > 0 && std::memcmp(pw, rw, (pr + 1) * sizeof(wchar_t)) != 0) {
		report(f, "wbuf");
		return (false);
	}
	return (true);
}

static void
edge_wcsftime(int f, bool use_l)
{
	struct tm tm{};

	tm.tm_year = 100;
	tm.tm_mon = 0;
	tm.tm_mday = 1;
	run_wcsftime(f, use_l, L"%Y", 32, &tm, false);
	run_wcsftime(f, use_l, L"ab", 16, &tm, false);
	run_wcsftime(f, use_l, L"", 8, &tm, false);

	pbsd_wcsftime_hook.wcsrtombs_fail = 1;
	run_wcsftime(f, use_l, L"x", 8, &tm, false);
	pbsd_wcsftime_hook.wcsrtombs_fail = 0;

	pbsd_wcsftime_hook.fail_malloc_at = 1;
	pbsd_wcsftime_hook.malloc_calls = 0;
	run_wcsftime(f, use_l, L"x", 8, &tm, false);
	pbsd_wcsftime_hook.fail_malloc_at = 0;

	pbsd_wcsftime_hook.fail_malloc_at = 2;
	pbsd_wcsftime_hook.malloc_calls = 0;
	run_wcsftime(f, use_l, L"x", 8, &tm, false);
	pbsd_wcsftime_hook.fail_malloc_at = 0;

	pbsd_wcsftime_hook.strftime_zero = 1;
	run_wcsftime(f, use_l, L"x", 8, &tm, false);
	pbsd_wcsftime_hook.strftime_zero = 0;

	run_wcsftime(f, use_l, L"x", SIZE_MAX / MB_CUR_MAX, &tm, false);

	pbsd_wcsftime_hook.mbsrtowcs_fail = 1;
	run_wcsftime(f, use_l, L"x", 16, &tm, false);
	pbsd_wcsftime_hook.mbsrtowcs_fail = 0;

	pbsd_wcsftime_hook.mbsrtowcs_incomplete = 1;
	run_wcsftime(f, use_l, L"x", 16, &tm, false);
	pbsd_wcsftime_hook.mbsrtowcs_incomplete = 0;

	pbsd_wcsftime_hook.mbsrtowcs_dstp_left = 1;
	run_wcsftime(f, use_l, L"x", 16, &tm, false);
	pbsd_wcsftime_hook.mbsrtowcs_dstp_left = 0;
}

static void
sweep_wcsftime(int f, bool use_l)
{
	struct tm tm{};
	wchar_t fmt[16];

	for (long long i = 0; i < SWEEP; i++) {
		size_t n = u32(8);
		for (size_t j = 0; j < n; j++)
			fmt[j] = (wchar_t)(L'a' + (j % 26));
		fmt[n] = L'\0';
		tm.tm_sec = (int)u32(60);
		tm.tm_min = (int)u32(60);
		tm.tm_hour = (int)u32(24);
		if (u32(8) == 0)
			pbsd_wcsftime_hook.wcsrtombs_fail = 1;
		else
			pbsd_wcsftime_hook.wcsrtombs_fail = 0;
		if (u32(16) == 0) {
			pbsd_wcsftime_hook.fail_malloc_at = 1 + (int)u32(3);
			pbsd_wcsftime_hook.malloc_calls = 0;
		} else
			pbsd_wcsftime_hook.fail_malloc_at = 0;
		if (u32(32) == 0)
			pbsd_wcsftime_hook.strftime_zero = 1;
		else
			pbsd_wcsftime_hook.strftime_zero = 0;
		run_wcsftime(f, use_l, fmt, u32(32) + 1, &tm, u32(4) == 0);
	}
}

static bool
run_part_load(const char *name, int maxl, int minl, int expect, int *using_out)
{
	const char *dst_p[8]{};
	const char *dst_r[8]{};
	char *buf_p = nullptr;
	char *buf_r = nullptr;
	int up = 0, ur = 0;
	int pr, rr;
	int f = F_PART_LOAD;

	ncase[f]++;
	pr = P::__part_load_locale(name, &up, &buf_p, "LC_FAKE", maxl, minl,
	    dst_p);
	rr = ref___part_load_locale(name, &ur, &buf_r, "LC_FAKE", maxl, minl,
	    dst_r);
	if (!chk_int(f, pr, rr) || pr != expect) {
		report(f, "code");
		return (false);
	}
	if (up != ur) {
		report(f, "using");
		return (false);
	}
	if (using_out != nullptr)
		*using_out = up;
	if (pr == 0) {
		for (int i = 0; i < maxl; i++) {
			if ((dst_p[i] == nullptr) != (dst_r[i] == nullptr) ||
			    (dst_p[i] != nullptr && std::strcmp(dst_p[i],
			    dst_r[i]) != 0)) {
				report(f, "dst");
				return (false);
			}
		}
	}
	std::free(buf_p);
	std::free(buf_r);
	return (true);
}

static void
edge_part_load()
{
	static const char good[] = "line1\nline2\nline3\n";

	pbsd_ldpart_hook.file_content = good;
	pbsd_ldpart_hook.file_size = (off_t)(sizeof(good) - 1);
	run_part_load("C", 3, 2, 1, nullptr);
	run_part_load("POSIX", 3, 2, 1, nullptr);
	run_part_load("C.UTF-8", 3, 2, 1, nullptr);

	run_part_load("en_US", 3, 2, 0, nullptr);

	pbsd_ldpart_hook.open_fail = 1;
	pbsd_ldpart_hook.open_errno = ENOENT;
	run_part_load("xx", 3, 2, -1, nullptr);
	pbsd_ldpart_hook.open_fail = 0;

	pbsd_ldpart_hook.fstat_fail = 1;
	pbsd_ldpart_hook.fstat_errno = EIO;
	run_part_load("xx", 3, 2, -1, nullptr);
	pbsd_ldpart_hook.fstat_fail = 0;

	pbsd_ldpart_hook.file_size = 0;
	run_part_load("xx", 3, 2, -1, nullptr);
	pbsd_ldpart_hook.file_size = (off_t)(sizeof(good) - 1);

	pbsd_ldpart_hook.malloc_fail = 1;
	run_part_load("xx", 3, 2, -1, nullptr);
	pbsd_ldpart_hook.malloc_fail = 0;

	static const char bad_nl[] = "noeol";
	pbsd_ldpart_hook.file_content = bad_nl;
	pbsd_ldpart_hook.file_size = (off_t)(sizeof(bad_nl) - 1);
	run_part_load("xx", 3, 2, -1, nullptr);

	static const char one_line[] = "only\n";
	pbsd_ldpart_hook.file_content = one_line;
	pbsd_ldpart_hook.file_size = (off_t)(sizeof(one_line) - 1);
	run_part_load("xx", 3, 3, -1, nullptr);

	pbsd_ldpart_hook.file_content = good;
	pbsd_ldpart_hook.file_size = (off_t)(sizeof(good) - 1);

	pbsd_ldpart_hook.read_fail = 1;
	pbsd_ldpart_hook.read_errno = EIO;
	run_part_load("xx", 3, 2, -1, nullptr);
	pbsd_ldpart_hook.read_fail = 0;
}

static void
sweep_part_load()
{
	static const char *names[] = { "C", "POSIX", "C.foo", "en", "xx", "a" };
	char content[64];

	for (long long i = 0; i < SWEEP; i++) {
		int lines = 2 + (int)u32(4);
		size_t pos = 0;
		for (int l = 0; l < lines; l++) {
			content[pos++] = (char)('a' + (l % 26));
			content[pos++] = '\n';
		}
		content[pos] = '\0';
		pbsd_ldpart_hook.file_content = content;
		pbsd_ldpart_hook.file_size = (off_t)pos;
		pbsd_ldpart_hook.open_fail = (u32(20) == 0);
		pbsd_ldpart_hook.open_errno = ENOENT;
		pbsd_ldpart_hook.fstat_fail = (u32(25) == 0);
		pbsd_ldpart_hook.malloc_fail = (u32(30) == 0);
		pbsd_ldpart_hook.read_fail = (u32(35) == 0);
		run_part_load(names[u32(6)], 3, 2,
		    pbsd_ldpart_hook.open_fail || pbsd_ldpart_hook.fstat_fail ||
		    pbsd_ldpart_hook.malloc_fail || pbsd_ldpart_hook.read_fail ?
		    -1 : (names[u32(6)][0] == 'C' ? 1 : 0), nullptr);
	}
}

static void
test_gb2312_init(int f)
{
	P::xlocale_ctype pl{};
	ref_xlocale_ctype rl{};
	P::_RuneLocale prl{};
	ref_rune_locale rrl{};

	int pr = P::GB2312_init(&pl, &prl);
	int rr = ref__GB2312_init(&rl, &rrl);
	if (!chk_int(f, pr, rr))
		return;
	if (pl.__mb_cur_max != rl.__mb_cur_max ||
	    pl.__mb_sb_limit != rl.__mb_sb_limit ||
	    pl.runes != &prl || rl.runes != &rrl)
		report(f, "fields");
}

static void
test_gb2312_mbsinit(int f)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};

	chk_int(f, P::GB2312_mbsinit(nullptr), ref__GB2312_mbsinit(nullptr));
	set_gb_state(ps, 0);
	set_gb_state(rs, 0);
	chk_int(f, P::GB2312_mbsinit(&ps), ref__GB2312_mbsinit(&rs));
	set_gb_state(ps, 1, 0xa1);
	set_gb_state(rs, 1, 0xa1);
	chk_int(f, P::GB2312_mbsinit(&ps), ref__GB2312_mbsinit(&rs));
}

static bool
run_gb_check(int f, const char *s, size_t n)
{
	int pr = P::GB2312_check(s, n);
	int rr = ref__GB2312_check(s, n);
	return (chk_int(f, pr, rr));
}

static void
edge_gb_check(int f)
{
	char buf[4];
	static const unsigned char vals[] = {
		0, 0x7f, 0x80, 0xa0, 0xa1, 0xfe, 0xff
	};
	for (unsigned char v : vals) {
		buf[0] = (char)v;
		run_gb_check(f, buf, 0);
		run_gb_check(f, buf, 1);
		buf[1] = (char)0xa1;
		run_gb_check(f, buf, 2);
		buf[1] = (char)0xff;
		run_gb_check(f, buf, 2);
	}
	buf[0] = (char)0xa1; buf[1] = (char)0xb2;
	run_gb_check(f, buf, 2);
}

static void
sweep_gb_check(int f)
{
	char buf[4];
	for (long long i = 0; i < SWEEP; i++) {
		buf[0] = (char)u32(256);
		buf[1] = (char)u32(256);
		run_gb_check(f, buf, u32(3));
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

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	run_mbr(f, fn, ref_fn, nullptr, 0, ps, rs, true);
	run_mbr(f, fn, ref_fn, nullptr, 0, ps, rs, false);

	buf[0] = 'a';
	run_mbr(f, fn, ref_fn, buf, 0, ps, rs, false);

	set_gb_state(ps, 99);
	set_gb_state(rs, 99);
	run_mbr(f, fn, ref_fn, buf, 2, ps, rs, false);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = 'Z';
	run_mbr(f, fn, ref_fn, buf, 2, ps, rs, false);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = '\0';
	run_mbr(f, fn, ref_fn, buf, 1, ps, rs, false);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0xa1; buf[1] = (char)0xb2;
	run_mbr(f, fn, ref_fn, buf, 1, ps, rs, false);
	run_mbr(f, fn, ref_fn, buf, 2, ps, rs, false);

	std::memset(&ps, 0, sizeof(ps));
	std::memset(&rs, 0, sizeof(rs));
	buf[0] = (char)0x80;
	run_mbr(f, fn, ref_fn, buf, 1, ps, rs, false);
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
		else if (u32(3) == 0)
			set_gb_state(ps, (int)u32(3), (unsigned char)u32(256),
			    (unsigned char)u32(256));
		else
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
		run_mbr(f, fn, ref_fn, in, n, ps, rs, null_pwc);
	}
}

template<typename WctFn, typename RefWctFn>
static bool
run_wct(int f, WctFn fn, RefWctFn ref_fn, wchar_t wc, P::mbstate_t &ps,
    ref_mbstate_t &rs, bool null_out)
{
	int pe, re;
	unsigned char ob[16], rb[16];

	fill_guard(ob, sizeof(ob));
	fill_guard(rb, sizeof(rb));
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
edge_wct(int f, WctFn fn, RefWctFn ref_fn)
{
	P::mbstate_t ps{};
	ref_mbstate_t rs{};

	run_wct(f, fn, ref_fn, L'a', ps, rs, true);
	run_wct(f, fn, ref_fn, L'a', ps, rs, false);
	run_wct(f, fn, ref_fn, (wchar_t)0x8122, ps, rs, false);
	run_wct(f, fn, ref_fn, L'\0', ps, rs, false);
	set_gb_state(ps, 1);
	set_gb_state(rs, 1);
	run_wct(f, fn, ref_fn, L'x', ps, rs, false);
}

template<typename WctFn, typename RefWctFn>
static void
sweep_wct(int f, WctFn fn, RefWctFn ref_fn)
{
	for (long long i = 0; i < SWEEP; i++) {
		P::mbstate_t ps{};
		ref_mbstate_t rs{};
		wchar_t wc = (wchar_t)(rnd() & 0xffff);
		bool null_out = (u32(5) == 0);
		if (u32(3) == 0)
			set_gb_state(ps, (int)u32(4));
		else
			std::memset(&ps, 0, sizeof(ps));
		mb_copy(ps, rs);
		run_wct(f, fn, ref_fn, wc, ps, rs, null_out);
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
		"", "a", "ab", "\x80", "\xa1\xb2", "a\0b", "\xa1", "\xff"
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
	if (!dst_null && std::memcmp(out_p, out_r, sizeof(out_r)) != 0) {
		report(f, "mbuf");
		return (false);
	}
	if ((sp == nullptr) != (sr == nullptr) ||
	    (sp != nullptr && (sp - in_p) != (sr - in_r))) {
		report(f, "wsrc");
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

int
main()
{
	pbsd_reset_hooks();

	edge_localeconv(F_LOCALECONV_L, true);
	sweep_localeconv(F_LOCALECONV_L, true);
	edge_localeconv(F_LOCALECONV, false);
	sweep_localeconv(F_LOCALECONV, false);

	edge_wcsftime(F_WCSFTIME_L, true);
	sweep_wcsftime(F_WCSFTIME_L, true);
	edge_wcsftime(F_WCSFTIME, false);
	sweep_wcsftime(F_WCSFTIME, false);

	edge_part_load();
	sweep_part_load();

	test_gb2312_init(F_GB2312_INIT);
	test_gb2312_mbsinit(F_GB2312_MBSINIT);
	edge_gb_check(F_GB2312_CHECK);
	sweep_gb_check(F_GB2312_CHECK);

	edge_mbr(F_GB2312_MBRTOWC, P::GB2312_mbrtowc, ref__GB2312_mbrtowc);
	sweep_mbr(F_GB2312_MBRTOWC, P::GB2312_mbrtowc, ref__GB2312_mbrtowc);
	edge_wct(F_GB2312_WCRTOMB, P::GB2312_wcrtomb, ref__GB2312_wcrtomb);
	sweep_wct(F_GB2312_WCRTOMB, P::GB2312_wcrtomb, ref__GB2312_wcrtomb);
	edge_mbs(F_GB2312_MBSNRTOWCS, P::GB2312_mbsnrtowcs, ref__GB2312_mbsnrtowcs);
	sweep_mbs(F_GB2312_MBSNRTOWCS, P::GB2312_mbsnrtowcs, ref__GB2312_mbsnrtowcs);
	edge_wcs(F_GB2312_WCSNRTOMBS, P::GB2312_wcsnrtombs, ref__GB2312_wcsnrtombs);
	sweep_wcs(F_GB2312_WCSNRTOMBS, P::GB2312_wcsnrtombs, ref__GB2312_wcsnrtombs);

	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	long long total_fail = 0;
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-22s %12lld %12lld\n", fname[i], ncase[i],
		    nfail[i]);
		total_fail += nfail[i];
	}
	return (total_fail == 0 ? 0 : 1);
}
