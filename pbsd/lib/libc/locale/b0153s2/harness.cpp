/*
 * Differential harness for batch b0153s2 (wcsftime.c).
 */

import pbsd.lib.libc.locale.b0153s2;

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace P = pbsd::lib_libc_locale::b0153s2;

extern "C" {
struct ref_xlocale {
	long	dummy;
};

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

void			pbsd_reset_hooks(void);
pbsd_wcsftime_hook_t	*pbsd_get_wcsftime_hook(void);
struct ref_xlocale	*ref_get_test_locale(void);
size_t	ref_wcsftime(wchar_t *, size_t, const wchar_t *, const struct tm *);
}

enum { F_WCSFTIME_L, F_WCSFTIME, F_COUNT };

static const char *const fname[F_COUNT] = {
	"wcsftime_l", "wcsftime"
};

static long long ncase[F_COUNT];
static long long nfail[F_COUNT];
static int nprinted[F_COUNT];

static constexpr unsigned char GUARD = 0x7f;
static constexpr wchar_t WGUARD = (wchar_t)0x7f7f;
static constexpr size_t WBUFSZ = 64;
static constexpr long long SWEEP = 200000;

static pbsd_wcsftime_hook_t &
wchook()
{
	return (*pbsd_get_wcsftime_hook());
}

static uint64_t rng = 0xB0153202ULL;

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
fill_wguard(wchar_t *b, size_t n)
{
	for (size_t i = 0; i < n; i++)
		b[i] = WGUARD;
}

static bool
wbuf_eq(const wchar_t *a, const wchar_t *b, size_t n)
{
	return (std::memcmp(a, b, n * sizeof(wchar_t)) == 0);
}

static bool
run_wcsftime(int f, bool use_l, const wchar_t *fmt, size_t maxsize,
    const struct tm *tm, bool null_wcs)
{
	wchar_t pw[WBUFSZ], rw[WBUFSZ];
	size_t pr, rr;
	int pe, re;

	fill_wguard(pw, WBUFSZ);
	fill_wguard(rw, WBUFSZ);
	errno = 0;
	wchook().malloc_calls = 0;
	if (use_l) {
		pr = P::wcsftime_l(null_wcs ? nullptr : pw, maxsize, fmt, tm,
		    P::test_locale());
		pe = errno;
		errno = 0;
		wchook().malloc_calls = 0;
		rr = ref_wcsftime_l(null_wcs ? nullptr : rw, maxsize, fmt, tm,
		    ref_get_test_locale());
		re = errno;
	} else {
		pr = P::wcsftime(null_wcs ? nullptr : pw, maxsize, fmt, tm);
		pe = errno;
		errno = 0;
		wchook().malloc_calls = 0;
		rr = ref_wcsftime(null_wcs ? nullptr : rw, maxsize, fmt, tm);
		re = errno;
	}
	if (!chk_ret(f, pr, rr, pe, re))
		return (false);
	if (!null_wcs && !wbuf_eq(pw, rw, WBUFSZ)) {
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
	run_wcsftime(f, use_l, L"x", 0, &tm, false);
	run_wcsftime(f, use_l, L"x", 1, &tm, false);

	wchook().wcsrtombs_fail = 1;
	run_wcsftime(f, use_l, L"x", 8, &tm, false);
	wchook().wcsrtombs_fail = 0;

	wchook().fail_malloc_at = 1;
	wchook().malloc_calls = 0;
	run_wcsftime(f, use_l, L"x", 8, &tm, false);
	wchook().fail_malloc_at = 0;

	wchook().fail_malloc_at = 2;
	wchook().malloc_calls = 0;
	run_wcsftime(f, use_l, L"x", 8, &tm, false);
	wchook().fail_malloc_at = 0;

	wchook().strftime_zero = 1;
	run_wcsftime(f, use_l, L"x", 8, &tm, false);
	wchook().strftime_zero = 0;

	run_wcsftime(f, use_l, L"x", SIZE_MAX / MB_CUR_MAX, &tm, false);
	run_wcsftime(f, use_l, L"x", SIZE_MAX / MB_CUR_MAX + 1, &tm, false);

	wchook().mbsrtowcs_fail = 1;
	run_wcsftime(f, use_l, L"x", 16, &tm, false);
	wchook().mbsrtowcs_fail = 0;

	wchook().mbsrtowcs_incomplete = 1;
	run_wcsftime(f, use_l, L"x", 16, &tm, false);
	wchook().mbsrtowcs_incomplete = 0;

	wchook().mbsrtowcs_dstp_left = 1;
	run_wcsftime(f, use_l, L"x", 16, &tm, false);
	wchook().mbsrtowcs_dstp_left = 0;

	/* wchar > 0xff forces wcsrtombs_l failure */
	run_wcsftime(f, use_l, L"\u0100", 16, &tm, false);

	wchook().strftime_len = 8;
	run_wcsftime(f, use_l, L"zz", 16, &tm, false);
	wchook().strftime_len = 0;

	run_wcsftime(f, use_l, L"x", 8, &tm, true);
}

static void
sweep_wcsftime(int f, bool use_l)
{
	struct tm tm{};
	wchar_t fmt[16];

	for (long long i = 0; i < SWEEP; i++) {
		size_t n = u32(8);
		for (size_t j = 0; j < n; j++) {
			switch (u32(5)) {
			case 0:
				fmt[j] = (wchar_t)(L'a' + (j % 26));
				break;
			case 1:
				fmt[j] = (wchar_t)(0x80 + u32(128));
				break;
			case 2:
				fmt[j] = (wchar_t)(0xff);
				break;
			default:
				fmt[j] = (wchar_t)(32 + u32(96));
				break;
			}
		}
		fmt[n] = L'\0';
		tm.tm_sec = (int)u32(60);
		tm.tm_min = (int)u32(60);
		tm.tm_hour = (int)u32(24);
		wchook().wcsrtombs_fail = (u32(8) == 0);
		if (u32(16) == 0) {
			wchook().fail_malloc_at = 1 + (int)u32(3);
			wchook().malloc_calls = 0;
		} else
			wchook().fail_malloc_at = 0;
		wchook().strftime_zero = (u32(32) == 0);
		wchook().mbsrtowcs_fail = (u32(40) == 0);
		wchook().mbsrtowcs_incomplete = (u32(48) == 0);
		wchook().mbsrtowcs_dstp_left = (u32(56) == 0);
		wchook().strftime_len = (size_t)u32(12);
		run_wcsftime(f, use_l, fmt, u32(32) + 1, &tm, u32(4) == 0);
	}
}

int
main()
{
	pbsd_reset_hooks();

	edge_wcsftime(F_WCSFTIME_L, true);
	sweep_wcsftime(F_WCSFTIME_L, true);
	edge_wcsftime(F_WCSFTIME, false);
	sweep_wcsftime(F_WCSFTIME, false);

	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	long long total_fail = 0;
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-22s %12lld %12lld\n", fname[i], ncase[i],
		    nfail[i]);
		total_fail += nfail[i];
	}
	return (total_fail == 0 ? 0 : 1);
}
