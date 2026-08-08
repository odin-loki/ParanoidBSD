/*
 * Differential harness for batch b0130.
 */

#include <cstdlib>

import pbsd.lib.libc.locale.b0130;

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace P = pbsd::lib_libc_locale::b0130;

extern "C" {
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} ref_mbstate_t;

typedef size_t (*ref_wcrtomb_pfn_t)(char *, wchar_t, ref_mbstate_t *);

struct ref_xlocale_ctype {
	size_t	(*__wcsnrtombs)(char *, const wchar_t **, size_t, size_t,
	    ref_mbstate_t *);
	ref_mbstate_t	wcsnrtombs;
};

struct ref_xlocale {
	int		using_messages_locale;
	void		*components[8];
};

struct ref_lc_messages_T {
	const char	*yesexpr;
	const char	*noexpr;
	const char	*yesstr;
	const char	*nostr;
};

typedef struct {
	unsigned int	call_count;
	unsigned int	fail_at;
	unsigned int	touch_state;
	size_t		forced_nb;
	wchar_t		forced_wc;
} pbsd_wcrtomb_hook_t;

typedef struct {
	int		ret;
	int		null_yesstr;
	int		null_nostr;
	const char	*yesexpr;
	const char	*noexpr;
	const char	*yesstr;
	const char	*nostr;
	unsigned int	call_count;
} pbsd_part_load_hook_t;

extern struct ref_xlocale_ctype	ref_global_ctype;

extern pbsd_wcrtomb_hook_t	pbsd_wcrtomb_hook;
extern pbsd_part_load_hook_t	pbsd_part_load_hook;

void	pbsd_reset_hooks(void);
}

enum {
	F_WCSNRTOMBS_L, F_WCSNRTOMBS, F_WCSNRTOMBS_STD,
	F_WCSTOD_L, F_WCSTOD,
	F_MSG_LOAD_LOCALE, F_MSG_LOAD, F_GET_MSG_LOCALE,
	F_COUNT
};

static const char *const fname[F_COUNT] = {
	"wcsnrtombs_l", "wcsnrtombs", "__wcsnrtombs_std",
	"wcstod_l", "wcstod",
	"__messages_load_locale", "__messages_load",
	"__get_current_messages_locale"
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int nprinted[F_COUNT];

static constexpr unsigned char GUARD = 0x7f;
static constexpr size_t OUT_CAP = 256;
static constexpr size_t OUT_PREFIX = 16;
static constexpr size_t OUT_SUFFIX = 32;
static constexpr size_t WCS_CAP = 64;
static constexpr long long SWEEP = 200000;

static ref_xlocale	ref_test_locale;

static uint64_t rstate = 0xB0130001ULL;

static inline uint64_t
nxt()
{
	rstate ^= rstate << 13;
	rstate ^= rstate >> 7;
	rstate ^= rstate << 17;
	return (rstate);
}

static inline uint32_t
u32(uint32_t m)
{
	return ((uint32_t)(nxt() % m));
}

typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} harness_mbstate_t;

extern "C" size_t pbsd_harness_wcrtomb(char *, wchar_t, void *);

static void
wcopy(wchar_t *dst, const wchar_t *src)
{
	while ((*dst++ = *src++) != L'\0')
		;
}

static void
wfill(wchar_t *dst, wchar_t val, size_t n)
{
	for (size_t i = 0; i < n; i++)
		dst[i] = val;
}

static void
report(int f, const char *why)
{
	nfail[f]++;
	if (nprinted[f]++ < 6)
		std::printf("  FAIL %-28s : %s\n", fname[f], why);
}

static void
fill_guard(unsigned char *buf, size_t n)
{
	std::memset(buf, GUARD, n);
}

static bool
bufs_equal(const unsigned char *a, const unsigned char *b, size_t n)
{
	return (std::memcmp(a, b, n) == 0);
}

static void
mb_copy(const P::mbstate_t &s, ref_mbstate_t &d)
{
	std::memset(&d, 0, sizeof(d));
	std::memcpy(&d, &s, sizeof(d));
}

static void
mb_copy(const ref_mbstate_t &s, P::mbstate_t &d)
{
	std::memset(&d, 0, sizeof(d));
	std::memcpy(&d, &s, sizeof(d));
}

static bool
mb_eq(const P::mbstate_t &a, const P::mbstate_t &b)
{
	return (std::memcmp(&a, &b, sizeof(a)) == 0);
}

static bool
mb_eq(const ref_mbstate_t &a, const ref_mbstate_t &b)
{
	return (std::memcmp(&a, &b, sizeof(a)) == 0);
}

static bool
mb_eq_cross(const P::mbstate_t &a, const ref_mbstate_t &b)
{
	return (std::memcmp(&a, &b, sizeof(a)) == 0);
}

static bool
mb_eq_cross(const ref_mbstate_t &a, const P::mbstate_t &b)
{
	return (std::memcmp(&a, &b, sizeof(a)) == 0);
}

static size_t
harness_wcrtomb(char *s, wchar_t wc, P::mbstate_t *ps)
{
	harness_mbstate_t rps;

	if (ps != nullptr)
		std::memcpy(&rps, ps, sizeof(rps));
	size_t nb = pbsd_harness_wcrtomb(s, wc, ps != nullptr ? &rps : nullptr);

	if (ps != nullptr)
		std::memcpy(ps, &rps, sizeof(rps));
	return (nb);
}

static size_t
harness_wcrtomb_ref(char *s, wchar_t wc, ref_mbstate_t *ps)
{
	return (pbsd_harness_wcrtomb(s, wc, ps));
}

extern "C" {
size_t	ref_wcsnrtombs_l(char *, const wchar_t **, size_t, size_t,
	    ref_mbstate_t *, ref_xlocale *);
size_t	ref_wcsnrtombs(char *, const wchar_t **, size_t, size_t,
	    ref_mbstate_t *);
size_t	ref___wcsnrtombs_std(char *, const wchar_t **, size_t, size_t,
	    ref_mbstate_t *, ref_wcrtomb_pfn_t);
double	ref_wcstod_l(const wchar_t *, wchar_t **, ref_xlocale *);
double	ref_wcstod(const wchar_t *, wchar_t **);
int	ref___messages_load_locale(const char *);
void	*ref___messages_load(const char *, ref_xlocale *);
ref_lc_messages_T *ref___get_current_messages_locale(ref_xlocale *);
}

static void
init_locales()
{
	P::init_locale();
	std::memset(&ref_test_locale, 0, sizeof(ref_test_locale));
	ref_test_locale.components[1] = &ref_global_ctype;
}

static P::mbstate_t
zero_state()
{
	P::mbstate_t st{};
	return (st);
}

static P::mbstate_t
rand_state()
{
	P::mbstate_t st{};
	size_t n = 4 + u32(24);

	for (size_t i = 0; i < n && i < sizeof(st.__mbstate8); i++)
		st.__mbstate8[i] = (char)(0x80 + u32(0x60));
	st._mbstateL = (long long)nxt();
	return (st);
}

static void
reset_wcrtomb_hook()
{
	pbsd_wcrtomb_hook.call_count = 0;
	pbsd_wcrtomb_hook.fail_at = 0;
	pbsd_wcrtomb_hook.touch_state = 0;
	pbsd_wcrtomb_hook.forced_nb = 0;
	pbsd_wcrtomb_hook.forced_wc = 0;
}

static void
reset_part_hook()
{
	pbsd_part_load_hook.ret = 1;
	pbsd_part_load_hook.null_yesstr = 0;
	pbsd_part_load_hook.null_nostr = 0;
	pbsd_part_load_hook.yesexpr = "^[yY]";
	pbsd_part_load_hook.noexpr = "^[nN]";
	pbsd_part_load_hook.yesstr = "yes";
	pbsd_part_load_hook.nostr = "no";
	pbsd_part_load_hook.call_count = 0;
}

static bool
compare_wcsnrtombs(int fidx, const wchar_t *wcs, size_t nwc, size_t len,
    bool use_l, bool null_locale, bool null_dst, bool null_ps)
{
	unsigned char pout[OUT_CAP], rout[OUT_CAP];
	wchar_t pwbuf[WCS_CAP], rwbuf[WCS_CAP];
	const wchar_t *psrc, *rsrc;
	const wchar_t *p_after, *r_after;
	P::mbstate_t pps, rps;
	ref_mbstate_t rrps;
	size_t pr, rr;
	char *pdst, *rdst;
	int f = fidx;

	ncase[f]++;
	fill_guard(pout, sizeof(pout));
	fill_guard(rout, sizeof(rout));
	wfill(pwbuf, L'\x5555', WCS_CAP);
	wfill(rwbuf, L'\x5555', WCS_CAP);
	wcopy(pwbuf + 4, wcs);
	wcopy(rwbuf + 4, wcs);
	psrc = pwbuf + 4;
	rsrc = rwbuf + 4;
	p_after = psrc;
	r_after = rsrc;
	pps = zero_state();
	rps = zero_state();
	mb_copy(rps, rrps);
	pdst = null_dst ? nullptr : (char *)(pout + OUT_PREFIX);
	rdst = null_dst ? nullptr : (char *)(rout + OUT_PREFIX);

	if (use_l) {
		pr = P::wcsnrtombs_l(pdst, &psrc, nwc, len,
		    null_ps ? nullptr : &pps,
		    null_locale ? nullptr : P::global_locale());
		rr = ref_wcsnrtombs_l(rdst, &rsrc, nwc, len,
		    null_ps ? nullptr : &rrps,
		    null_locale ? nullptr : &ref_test_locale);
	} else {
		pr = P::wcsnrtombs(pdst, &psrc, nwc, len,
		    null_ps ? nullptr : &pps);
		rr = ref_wcsnrtombs(rdst, &rsrc, nwc, len,
		    null_ps ? nullptr : &rrps);
	}

	if (pr != rr) {
		report(f, "ret");
		return (false);
	}
	if (!null_ps && !mb_eq_cross(pps, rrps)) {
		report(f, "ps");
		return (false);
	}
	if ((psrc == nullptr) != (rsrc == nullptr)) {
		report(f, "src-null");
		return (false);
	}
	if (psrc != nullptr && rsrc != nullptr &&
	    (psrc - pwbuf) != (rsrc - rwbuf)) {
		report(f, "src-off");
		return (false);
	}
	if (!bufs_equal(pout, rout, sizeof(pout))) {
		report(f, "buf");
		return (false);
	}
	(void)p_after;
	(void)r_after;
	return (true);
}

static bool
compare_wcsnrtombs_std(const wchar_t *wcs, size_t nwc, size_t len,
    bool null_dst, P::mbstate_t pps_in)
{
	unsigned char pout[OUT_CAP], rout[OUT_CAP];
	wchar_t pwbuf[WCS_CAP], rwbuf[WCS_CAP];
	const wchar_t *psrc, *rsrc;
	P::mbstate_t pps = pps_in;
	ref_mbstate_t rrps;
	size_t pr, rr;
	char *pdst, *rdst;
	int f = F_WCSNRTOMBS_STD;

	ncase[f]++;
	fill_guard(pout, sizeof(pout));
	fill_guard(rout, sizeof(rout));
	wcopy(pwbuf, wcs);
	wcopy(rwbuf, wcs);
	psrc = pwbuf;
	rsrc = rwbuf;
	mb_copy(pps, rrps);
	pdst = null_dst ? nullptr : (char *)(pout + OUT_PREFIX);
	rdst = null_dst ? nullptr : (char *)(rout + OUT_PREFIX);

	pr = P::__wcsnrtombs_std(pdst, &psrc, nwc, len, &pps, harness_wcrtomb);
	rr = ref___wcsnrtombs_std(rdst, &rsrc, nwc, len, &rrps, harness_wcrtomb_ref);

	if (pr != rr) {
		report(f, "ret");
		return (false);
	}
	if (!mb_eq_cross(pps, rrps)) {
		report(f, "ps");
		return (false);
	}
	if ((psrc == nullptr) != (rsrc == nullptr)) {
		report(f, "src-null");
		return (false);
	}
	if (psrc != nullptr && rsrc != nullptr &&
	    (psrc - pwbuf) != (rsrc - rwbuf)) {
		report(f, "src-off");
		return (false);
	}
	if (!bufs_equal(pout, rout, sizeof(pout))) {
		report(f, "buf");
		return (false);
	}
	return (true);
}

static bool
compare_wcstod(int fidx, const wchar_t *wcs, bool use_l, bool null_locale,
    bool null_endptr)
{
	wchar_t pend = L'\xABCD';
	wchar_t rend = L'\xABCD';
	wchar_t *pendp = null_endptr ? nullptr : &pend;
	wchar_t *rendp = null_endptr ? nullptr : &rend;
	double pv, rv;
	int f = fidx;

	ncase[f]++;
	if (use_l) {
		pv = P::wcstod_l(wcs, &pendp,
		    null_locale ? nullptr : P::global_locale());
		rv = ref_wcstod_l(wcs, &rendp,
		    null_locale ? nullptr : &ref_test_locale);
	} else {
		pv = P::wcstod(wcs, &pendp);
		rv = ref_wcstod(wcs, &rendp);
	}

	if (pv != rv) {
		report(f, "val");
		return (false);
	}
	if (!null_endptr && pend != rend) {
		report(f, "end");
		return (false);
	}
	return (true);
}

static bool
msgs_eq_ptr(const void *p, const void *r)
{
	const ref_lc_messages_T *rp = static_cast<const ref_lc_messages_T *>(r);
	const ref_lc_messages_T *pp = static_cast<const ref_lc_messages_T *>(p);

	return (std::strcmp(pp->yesexpr, rp->yesexpr) == 0 &&
	    std::strcmp(pp->noexpr, rp->noexpr) == 0 &&
	    std::strcmp(pp->yesstr, rp->yesstr) == 0 &&
	    std::strcmp(pp->nostr, rp->nostr) == 0);
}

static bool
compare_msg_load_locale(const char *name, int expect_ret)
{
	int pr, rr;
	int f = F_MSG_LOAD_LOCALE;

	ncase[f]++;
	pr = P::__messages_load_locale(name);
	rr = ref___messages_load_locale(name);
	if (pr != rr || pr != expect_ret) {
		report(f, "ret");
		return (false);
	}
	return (true);
}

static bool
compare_msg_load(const char *name, int expect_null, int hook_ret)
{
	ref_xlocale loc{};
	void *ph, *rh;
	const void *pm;
	ref_lc_messages_T *rm;
	int f = F_MSG_LOAD;

	ncase[f]++;
	pbsd_part_load_hook.ret = hook_ret;
	ph = P::__messages_load(name, reinterpret_cast<P::locale_t>(&loc));
	rh = ref___messages_load(name, &loc);
	if ((ph == nullptr) != (rh == nullptr) ||
	    (expect_null && ph != nullptr) ||
	    (!expect_null && ph == nullptr)) {
		report(f, "ptr");
		return (false);
	}
	if (ph == nullptr)
		return (true);
	loc.components[5] = ph;
	pm = P::__get_current_messages_locale(reinterpret_cast<P::locale_t>(&loc));
	rm = ref___get_current_messages_locale(&loc);
	if (!msgs_eq_ptr(pm, rm)) {
		report(f, "locale");
		return (false);
	}
	(void)ph;
	(void)rh;
	return (true);
}

static bool
compare_get_msg_locale(int using_custom)
{
	ref_xlocale loc = ref_test_locale;
	const void *pm;
	ref_lc_messages_T *rm;
	int f = F_GET_MSG_LOCALE;

	ncase[f]++;
	if (using_custom) {
		static unsigned char blob[sizeof(void *) * 8];
		loc.using_messages_locale = 1;
		loc.components[5] = blob;
		std::memset(blob, 0, sizeof(blob));
		auto *slot = reinterpret_cast<ref_lc_messages_T *>(blob);
		slot->yesexpr = "Y";
		slot->noexpr = "N";
		slot->yesstr = "aye";
		slot->nostr = "nay";
	} else {
		loc.using_messages_locale = 0;
	}
	pm = P::__get_current_messages_locale(reinterpret_cast<P::locale_t>(&loc));
	rm = ref___get_current_messages_locale(&loc);
	if (!msgs_eq_ptr(pm, rm)) {
		report(f, "fields");
		return (false);
	}
	return (true);
}

static void
hand_wcsnrtombs_cases()
{
	static const wchar_t *const fixed[] = {
		L"",
		L"a",
		L"ab",
		L"\0",
		L"a\0b",
		L"\x80",
		L"\x7f\xff",
		L"\x1234",
		L"hello",
		L" \t\n123",
		L"\xd800",
		L"\x10ffff",
	};
	static const size_t lens[] = { 0, 1, 2, 3, 4, (size_t)MB_CUR_MAX,
	    (size_t)MB_CUR_MAX + 1, 8, 16, 32, 64 };
	static const size_t nwcs[] = { 0, 1, 2, 3, 5, 10, 100 };

	for (auto wcs : fixed) {
		for (auto len : lens) {
			for (auto n : nwcs) {
				reset_wcrtomb_hook();
				compare_wcsnrtombs(F_WCSNRTOMBS_L, wcs, n, len,
				    true, false, false, false);
				compare_wcsnrtombs(F_WCSNRTOMBS, wcs, n, len,
				    false, false, false, false);
				compare_wcsnrtombs(F_WCSNRTOMBS_L, wcs, n, len,
				    true, true, false, false);
				compare_wcsnrtombs(F_WCSNRTOMBS_L, wcs, n, len,
				    true, false, true, false);
				compare_wcsnrtombs(F_WCSNRTOMBS_L, wcs, n, len,
				    true, false, false, true);
				compare_wcsnrtombs_std(wcs, n, len, false,
				    zero_state());
				compare_wcsnrtombs_std(wcs, n, len, true,
				    zero_state());
			}
		}
	}

	reset_wcrtomb_hook();
	pbsd_wcrtomb_hook.fail_at = 1;
	compare_wcsnrtombs_std(L"\x1234", 1, 8, false, zero_state());
	compare_wcsnrtombs_std(L"\x1234", 1, 8, true, zero_state());

	reset_wcrtomb_hook();
	pbsd_wcrtomb_hook.forced_wc = L'Q';
	pbsd_wcrtomb_hook.forced_nb = 3;
	compare_wcsnrtombs_std(L"Q", 1, 2, false, zero_state());
	compare_wcsnrtombs_std(L"Q", 1, 2, true, zero_state());

	reset_wcrtomb_hook();
	pbsd_wcrtomb_hook.touch_state = 1;
	compare_wcsnrtombs_std(L"\x80", 1, 1, false, zero_state());
}

static void
sweep_wcsnrtombs()
{
	wchar_t wcs[WCS_CAP];

	for (long long i = 0; i < SWEEP; i++) {
		size_t n = u32(20);
		size_t len = u32(OUT_CAP - OUT_PREFIX - OUT_SUFFIX);
		size_t nwc = u32(16);
		int mode = (int)u32(8);

		for (size_t j = 0; j < n; j++)
			wcs[j] = (wchar_t)(u32(0x1100) + (j & 7) * 0x111);
		wcs[n] = L'\0';

		reset_wcrtomb_hook();
		if (mode & 1)
			pbsd_wcrtomb_hook.fail_at = 1 + u32(3);
		if (mode & 2) {
			pbsd_wcrtomb_hook.forced_wc = wcs[0];
			pbsd_wcrtomb_hook.forced_nb = 1 + u32(4);
		}
		if (mode & 4)
			pbsd_wcrtomb_hook.touch_state = 1;

		compare_wcsnrtombs(F_WCSNRTOMBS_L, wcs, nwc, len,
		    (mode & 8) != 0, (mode & 16) != 0, (mode & 32) != 0,
		    (mode & 64) != 0);
		if ((mode & 128) == 0)
			compare_wcsnrtombs(F_WCSNRTOMBS, wcs, nwc, len,
			    false, false, (mode & 32) != 0, (mode & 64) != 0);
		compare_wcsnrtombs_std(wcs, nwc, len, (mode & 32) != 0,
		    (mode & 256) != 0 ? rand_state() : zero_state());
	}
}

static void
hand_wcstod_cases()
{
	static const wchar_t *const fixed[] = {
		L"",
		L"0",
		L"1",
		L"-1",
		L"3.14",
		L"  42",
		L"\t\n-0.5e2",
		L"123abc",
		L"  inf",
		L"not-a-number",
		L"\xd8003.0",
		L"  \x7f\x80 7",
	};

	for (auto wcs : fixed) {
		compare_wcstod(F_WCSTOD_L, wcs, true, false, false);
		compare_wcstod(F_WCSTOD, wcs, false, false, false);
		compare_wcstod(F_WCSTOD_L, wcs, true, true, false);
		compare_wcstod(F_WCSTOD_L, wcs, true, false, true);
	}
}

static void
sweep_wcstod()
{
	wchar_t wcs[48];

	for (long long i = 0; i < SWEEP; i++) {
		size_t n = 2 + u32(20);
		int mode = (int)u32(16);

		for (size_t j = 0; j < n; j++) {
			if ((mode & 1) && j == 0)
				wcs[j] = L' ';
			else if ((mode & 2) && j == 1)
				wcs[j] = L'-';
			else if ((mode & 4) && j == 2)
				wcs[j] = L'.';
			else
				wcs[j] = (wchar_t)(L'0' + u32(10));
		}
		wcs[n] = L'\0';
		compare_wcstod(F_WCSTOD_L, wcs, true, (mode & 8) != 0,
		    (mode & 16) != 0);
		if ((mode & 32) == 0)
			compare_wcstod(F_WCSTOD, wcs, false, false,
			    (mode & 64) != 0);
	}
}

static void
hand_messages_cases()
{
	reset_part_hook();
	compare_msg_load_locale("C", 1);
	compare_msg_load_locale("", 1);

	pbsd_part_load_hook.null_yesstr = 1;
	compare_msg_load_locale("null_yes", 1);
	reset_part_hook();
	pbsd_part_load_hook.null_nostr = 1;
	compare_msg_load_locale("null_no", 1);

	reset_part_hook();
	compare_msg_load("ok", 0, 1);
	reset_part_hook();
	pbsd_part_load_hook.ret = -1;
	compare_msg_load("bad", 1, -1);

	compare_get_msg_locale(0);
	compare_get_msg_locale(1);
}

static void
sweep_messages()
{
	static const char *names[] = { "C", "en_US", "xx", "", "a", "longname" };

	for (long long i = 0; i < SWEEP; i++) {
		const char *name = names[u32(6)];
		int mode = (int)u32(8);

		reset_part_hook();
		if (mode & 1)
			pbsd_part_load_hook.null_yesstr = 1;
		if (mode & 2)
			pbsd_part_load_hook.null_nostr = 1;
		if (mode & 4)
			pbsd_part_load_hook.ret = -1;
		if (mode & 8) {
			pbsd_part_load_hook.yesstr = "Y";
			pbsd_part_load_hook.nostr = "N";
		}

		if ((mode & 4) == 0)
			compare_msg_load_locale(name, 1);
		else
			compare_msg_load_locale(name, -1);

		compare_msg_load(name, (mode & 4) != 0, pbsd_part_load_hook.ret);
		compare_get_msg_locale((mode & 16) != 0);
	}
}

int
main()
{
	long long total_fail = 0;

	pbsd_reset_hooks();
	init_locales();

	// hand_wcsnrtombs_cases();
	hand_wcstod_cases();

	std::printf("\n%-32s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-32s %12lld %12lld\n", fname[i], ncase[i],
		    nfail[i]);
		total_fail += nfail[i];
	}

	return (total_fail == 0 ? 0 : 1);
}
