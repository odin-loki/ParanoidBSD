// Differential test harness for PBSD batch b0257.
//
// Every ported function is exercised against the unmodified reference
// implementation in oracle.c.

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <cwctype>

import pbsd.lib.libc.string.b0257;

namespace P = pbsd::lib_libc_string::b0257;

extern "C" {
int ref_wcsncasecmp(const wchar_t *s1, const wchar_t *s2, size_t n);
int ref___timingsafe_memcmp(const void *b1, const void *b2, size_t len);
}

/* ------------------------------------------------------------------ */
/* bookkeeping                                                        */
/* ------------------------------------------------------------------ */

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
	int shown;
};

static Stat st_wcsncasecmp = { "wcsncasecmp", 0, 0, 0 };
static Stat st_timingsafe_memcmp = { "__timingsafe_memcmp", 0, 0, 0 };

static const int MAX_SHOW = 8;
static const unsigned long long RANDOM_ITERS = 200000ull;

static bool
begin_fail(Stat &st, const char *what)
{
	st.fails++;
	if (st.shown >= MAX_SHOW)
		return false;
	st.shown++;
	std::printf("FAIL %s: %s\n", st.name, what);
	return true;
}

/* ------------------------------------------------------------------ */
/* deterministic RNG (splitmix64, fixed seed)                         */
/* ------------------------------------------------------------------ */

struct Rng {
	std::uint64_t s;

	explicit Rng(std::uint64_t seed) : s(seed) {}

	std::uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}

	std::uint32_t below(std::uint32_t n) { return (std::uint32_t)(next() % n); }
};

/* ------------------------------------------------------------------ */
/* wcsncasecmp                                                        */
/* ------------------------------------------------------------------ */

static const size_t WCS_MAXLEN = 96;

static void
check_wcsncasecmp(const wchar_t *s1, const wchar_t *s2, size_t n)
{
	int ra = P::wcsncasecmp(s1, s2, n);
	int rb = ref_wcsncasecmp(s1, s2, n);

	st_wcsncasecmp.cases++;
	if (ra != rb) {
		if (begin_fail(st_wcsncasecmp, "return mismatch"))
			std::printf("    n=%zu port=%d ref=%d\n", n, ra, rb);
	}
}

static void
fill_wcs_pair(wchar_t *s1, wchar_t *s2, size_t len1, size_t len2,
    unsigned pat, int skew)
{
	for (size_t i = 0; i < len1; i++) {
		wchar_t v;
		switch (pat) {
		case 0: v = (wchar_t)(L'a' + (i % 26)); break;
		case 1: v = (wchar_t)0xffffffffu; break;
		case 2: v = (wchar_t)0x80000000u; break;
		case 3: v = (wchar_t)(0x80 + (i % 0x70)); break;
		case 4: v = (wchar_t)((i & 1) ? L'A' : L'a'); break;
		default: v = (wchar_t)(i * 0x01010101u + 1); break;
		}
		if (v == 0)
			v = 1;
		s1[i] = v;
	}
	s1[len1] = L'\0';

	for (size_t i = 0; i < len2; i++) {
		wchar_t v = s1[i < len1 ? i : 0];
		if (skew == 0)
			v = (wchar_t)towlower(v);
		else if (skew == 1 && i < len2)
			v = (wchar_t)towupper(v);
		else if (skew == 2 && i == len2 / 2)
			v = (wchar_t)(v + 1);
		else if (skew == 3 && i < len1 && i < len2)
			v = s1[i];
		s2[i] = v;
	}
	s2[len2] = L'\0';
}

static void
wcsncasecmp_edge_cases(void)
{
	wchar_t s1[WCS_MAXLEN + 4];
	wchar_t s2[WCS_MAXLEN + 4];

	/* n == 0 always returns 0 */
	s1[0] = L'x';
	s2[0] = L'y';
	s1[1] = s2[1] = L'\0';
	check_wcsncasecmp(s1, s2, 0);

	/* both empty */
	s1[0] = L'\0';
	s2[0] = L'\0';
	check_wcsncasecmp(s1, s2, 1);
	check_wcsncasecmp(s1, s2, 32);

	/* single char: equal, case-fold equal, different */
	s1[0] = L'a'; s1[1] = L'\0';
	s2[0] = L'a'; s2[1] = L'\0';
	check_wcsncasecmp(s1, s2, 1);
	s2[0] = L'A';
	check_wcsncasecmp(s1, s2, 1);
	s2[0] = L'b';
	check_wcsncasecmp(s1, s2, 1);
	s2[0] = L'\0';
	check_wcsncasecmp(s1, s2, 1);

	/* n limits comparison before later difference */
	for (size_t len = 2; len <= 24; len++) {
		for (size_t i = 0; i < len; i++)
			s1[i] = s2[i] = (wchar_t)(L'p' + (i % 5));
		s1[len] = s2[len] = L'\0';
		s2[len - 1] = (wchar_t)(s1[len - 1] + 1);
		for (size_t n = 1; n < len; n++)
			check_wcsncasecmp(s1, s2, n);
		check_wcsncasecmp(s1, s2, len);
	}

	/* first difference at every position within n */
	for (size_t len = 1; len <= 24; len++) {
		for (size_t diff_at = 0; diff_at < len; diff_at++) {
			for (size_t i = 0; i < len; i++)
				s1[i] = s2[i] = (wchar_t)(L'm' + (i % 7));
			s1[len] = s2[len] = L'\0';
			s2[diff_at] = (wchar_t)(s1[diff_at] + 1);
			for (size_t n = 1; n <= len; n++)
				check_wcsncasecmp(s1, s2, n);
			s2[diff_at] = (wchar_t)(s1[diff_at] - 1);
			for (size_t n = 1; n <= len; n++)
				check_wcsncasecmp(s1, s2, n);
		}
	}

	/* s1 shorter: return -(*s2) after NUL */
	for (size_t l1 = 0; l1 <= 16; l1++) {
		for (size_t l2 = 0; l2 <= 16; l2++) {
			for (size_t i = 0; i < l1; i++)
				s1[i] = (wchar_t)(L'q' + (i % 4));
			s1[l1] = L'\0';
			for (size_t i = 0; i < l2; i++)
				s2[i] = (wchar_t)(L'q' + (i % 4));
			s2[l2] = L'\0';
			for (size_t n = 0; n <= 32; n++)
				check_wcsncasecmp(s1, s2, n);
		}
	}

	/* identical / case-mixed strings for various n */
	for (size_t len = 1; len <= 32; len++) {
		for (size_t i = 0; i < len; i++)
			s1[i] = (wchar_t)(L'a' + (i % 26));
		s1[len] = L'\0';
		wmemcpy(s2, s1, len + 1);
		for (size_t n = 1; n <= len + 2; n++)
			check_wcsncasecmp(s1, s2, n);
		for (size_t i = 0; i < len; i++)
			s2[i] = (wchar_t)towupper(s1[i]);
		for (size_t n = 1; n <= len + 2; n++)
			check_wcsncasecmp(s1, s2, n);
	}

	/* high-bit wchar values */
	for (size_t len = 1; len <= 16; len++) {
		for (size_t i = 0; i < len; i++) {
			s1[i] = (wchar_t)(0x80 + i);
			s2[i] = (wchar_t)(0x80 + i);
		}
		s1[len] = s2[len] = L'\0';
		for (size_t n = 1; n <= len + 2; n++)
			check_wcsncasecmp(s1, s2, n);
		s2[len / 2] = (wchar_t)(0x80 + (len / 2) + 1);
		for (size_t n = 1; n <= len + 2; n++)
			check_wcsncasecmp(s1, s2, n);
	}

	/* NUL-heavy: embedded zero not possible in C strings, but short prefixes */
	s1[0] = L'\0';
	s2[0] = L'z';
	s2[1] = L'\0';
	for (size_t n = 0; n <= 4; n++)
		check_wcsncasecmp(s1, s2, n);

	/* difference visible only after towlower */
	s1[0] = L'A';
	s1[1] = L'\0';
	s2[0] = L'a';
	s2[1] = L'z';
	s2[2] = L'\0';
	for (size_t n = 1; n <= 4; n++)
		check_wcsncasecmp(s1, s2, n);

	/* length mismatch with pattern fills */
	for (size_t l1 = 0; l1 <= 24; l1++) {
		for (size_t l2 = 0; l2 <= 24; l2++) {
			for (unsigned pat = 0; pat < 6; pat++) {
				for (int skew = 0; skew < 4; skew++) {
					fill_wcs_pair(s1, s2, l1, l2, pat,
					    skew);
					for (size_t n = 0; n <= 28; n++)
						check_wcsncasecmp(s1, s2, n);
				}
			}
		}
	}

	/* boundary: --n == 0 with matching prefix */
	for (size_t len = 1; len <= 20; len++) {
		for (size_t i = 0; i < len; i++)
			s1[i] = s2[i] = (wchar_t)(L'k' + (i % 3));
		s1[len] = L'X';
		s2[len] = L'Y';
		s1[len + 1] = s2[len + 1] = L'\0';
		check_wcsncasecmp(s1, s2, len);
	}
}

static void
wcsncasecmp_random_sweep(Rng &rng)
{
	wchar_t s1[WCS_MAXLEN + 4];
	wchar_t s2[WCS_MAXLEN + 4];

	for (unsigned long long it = 0; it < RANDOM_ITERS; it++) {
		size_t l1 = rng.below((std::uint32_t)(WCS_MAXLEN + 1));
		size_t l2 = rng.below((std::uint32_t)(WCS_MAXLEN + 1));
		unsigned pat = (unsigned)(rng.next() % 6u);
		int skew = (int)(rng.next() % 4u);

		for (size_t i = 0; i < l1; i++) {
			std::uint64_t r = rng.next();
			wchar_t v;
			switch (pat) {
			case 0:
				v = (wchar_t)(L'a' + (r % 26));
				break;
			case 1:
				v = (wchar_t)(std::uint32_t)r;
				break;
			case 2:
				v = (wchar_t)(std::uint32_t)(r | 0x80000000u);
				break;
			case 3:
				v = (wchar_t)(0x80 | (r & 0x7fu));
				break;
			case 4:
				v = (r & 1) ? (wchar_t)towupper(
				    (wchar_t)(L'a' + (r % 26))) :
				    (wchar_t)(L'a' + (r % 26));
				break;
			default:
				v = (r & 3) ? (wchar_t)(std::uint32_t)r :
				    L'\0';
				break;
			}
			if (v == 0 && i < l1)
				v = (wchar_t)(0x80 + i);
			s1[i] = v;
		}
		s1[l1] = L'\0';

		fill_wcs_pair(s1, s2, l1, l2, pat, skew);

		if ((it & 63) == 0 && l1 > 0 && l2 > 0) {
			size_t pos = rng.below((std::uint32_t)(l1 < l2 ? l1 : l2));
			s2[pos] = (wchar_t)(s1[pos] ^ (1u << (rng.next() % 5u)));
		}

		size_t n = rng.below((std::uint32_t)(WCS_MAXLEN + 5));
		check_wcsncasecmp(s1, s2, n);
	}
}

/* ------------------------------------------------------------------ */
/* __timingsafe_memcmp                                                */
/* ------------------------------------------------------------------ */

static const size_t TS_MAX = 128;

static void
check_timingsafe_memcmp(const unsigned char *a, const unsigned char *b,
    size_t n)
{
	int ra = P::__timingsafe_memcmp(a, b, n);
	int rb = ref___timingsafe_memcmp(a, b, n);

	st_timingsafe_memcmp.cases++;
	if (ra != rb) {
		if (begin_fail(st_timingsafe_memcmp, "return mismatch"))
			std::printf("    n=%zu port=%d ref=%d\n", n, ra, rb);
	}
}

static void
timingsafe_memcmp_edge_cases(void)
{
	unsigned char a[TS_MAX], b[TS_MAX];

	for (size_t n = 0; n <= TS_MAX; n++) {
		std::memset(a, 0x00, TS_MAX);
		std::memset(b, 0x00, TS_MAX);
		check_timingsafe_memcmp(a, b, n);

		std::memset(a, 0xff, TS_MAX);
		std::memset(b, 0xff, TS_MAX);
		check_timingsafe_memcmp(a, b, n);

		std::memset(a, 0x80, TS_MAX);
		std::memset(b, 0x80, TS_MAX);
		check_timingsafe_memcmp(a, b, n);
	}

	for (size_t n = 1; n <= TS_MAX; n++) {
		std::memset(a, 0x55, TS_MAX);
		std::memset(b, 0x55, TS_MAX);

		a[0] ^= 0x01;
		check_timingsafe_memcmp(a, b, n);

		a[0] = b[0];
		a[n - 1] ^= 0x80;
		check_timingsafe_memcmp(a, b, n);

		a[n - 1] = b[n - 1];
		if (n > 1) {
			a[n / 2] ^= 0xff;
			check_timingsafe_memcmp(a, b, n);
		}
	}

	/* first differing byte at every position */
	for (size_t n = 1; n <= TS_MAX; n++) {
		std::memset(a, 0x42, TS_MAX);
		std::memset(b, 0x42, TS_MAX);
		for (size_t pos = 0; pos < n; pos++) {
			a[pos] = (unsigned char)(0x40 + pos);
			b[pos] = (unsigned char)(0x40 + pos);
			b[pos] ^= 0x01;
			check_timingsafe_memcmp(a, b, n);
			a[pos] = b[pos] ^ 0x01;
			check_timingsafe_memcmp(a, b, n);
			b[pos] = a[pos];
		}
	}

	/* single-byte sweep: lt/gt/cmp branches */
	for (unsigned v = 0; v < 256; v++) {
		for (unsigned w = 0; w < 256; w++) {
			a[0] = (unsigned char)v;
			b[0] = (unsigned char)w;
			check_timingsafe_memcmp(a, b, 1);
		}
	}

	/* later difference must not overwrite first res */
	std::memset(a, 0x11, TS_MAX);
	std::memset(b, 0x11, TS_MAX);
	a[0] = 0x10;
	b[0] = 0x20;
	a[TS_MAX / 2] = 0x99;
	b[TS_MAX / 2] = 0x00;
	check_timingsafe_memcmp(a, b, TS_MAX);

	/* NUL-heavy */
	std::memset(a, 0x00, TS_MAX);
	std::memset(b, 0x00, TS_MAX);
	b[0] = 0x01;
	check_timingsafe_memcmp(a, b, TS_MAX);
	check_timingsafe_memcmp(a, a, TS_MAX);

	a[0] = 0xff;
	b[0] = 0x7f;
	check_timingsafe_memcmp(a, b, 1);
}

static void
timingsafe_memcmp_random_sweep(Rng &rng)
{
	unsigned char a[TS_MAX], b[TS_MAX];

	for (unsigned long long it = 0; it < RANDOM_ITERS; it++) {
		size_t n = rng.below((std::uint32_t)(TS_MAX + 1));

		for (size_t i = 0; i < n; i++) {
			if ((it & 3) == 0) {
				a[i] = (unsigned char)(0x80 | (rng.next() & 0x7f));
				b[i] = (unsigned char)(0x80 | (rng.next() & 0x7f));
			} else {
				a[i] = (unsigned char)(rng.next() & 0xff);
				b[i] = (unsigned char)(rng.next() & 0xff);
			}
		}

		if ((it & 1) == 0)
			std::memcpy(b, a, n);
		else if (n > 0) {
			size_t pos = rng.below((std::uint32_t)n);
			b[pos] ^= (unsigned char)(1 + rng.below(255));
		}

		check_timingsafe_memcmp(a, b, n);
	}
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int
main()
{
	std::printf("b0257 differential test (wchar_t is %zu bytes)\n",
	    sizeof(wchar_t));

	Rng rng_wcs(0xB0257Aull);
	Rng rng_ts(0xB0257Bull);

	wcsncasecmp_edge_cases();
	timingsafe_memcmp_edge_cases();

	wcsncasecmp_random_sweep(rng_wcs);
	timingsafe_memcmp_random_sweep(rng_ts);

	Stat stats[] = { st_wcsncasecmp, st_timingsafe_memcmp };
	unsigned long long total_cases = 0;
	unsigned long long total_fails = 0;

	std::printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-22s %12s %12s\n", "--------", "-----", "--------");
	for (size_t i = 0; i < sizeof(stats) / sizeof(stats[0]); i++) {
		std::printf("%-22s %12llu %12llu\n", stats[i].name,
		    stats[i].cases, stats[i].fails);
		total_cases += stats[i].cases;
		total_fails += stats[i].fails;
	}
	std::printf("%-22s %12llu %12llu\n", "TOTAL", total_cases,
	    total_fails);

	return total_fails == 0 ? 0 : 1;
}
