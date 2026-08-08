// Differential test harness for PBSD batch b0137.
//
// Every ported function is exercised against the unmodified reference
// implementation in oracle.c.  Pointer results are reduced to an offset from
// the arena base before comparison.  wcsdup compares the full allocated copy
// against the reference heap copy byte-for-byte.

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

import pbsd.lib.libc.string.b0137;

namespace P = pbsd::lib_libc_string::b0137;

extern "C" {
wchar_t *ref_wcsrchr(const wchar_t *s, wchar_t c);
wchar_t *ref_wcsdup(const wchar_t *s);
int ref_wcscasecmp(const wchar_t *s1, const wchar_t *s2);
}

// ---------------------------------------------------------------------------
// bookkeeping
// ---------------------------------------------------------------------------

struct Stats {
	const char *name;
	unsigned long long cases;
	unsigned long long failures;
	unsigned reported;
};

static Stats st_wcsrchr    = { "wcsrchr",    0, 0, 0 };
static Stats st_wcsdup       = { "wcsdup",     0, 0, 0 };
static Stats st_wcscasecmp   = { "wcscasecmp", 0, 0, 0 };

static const unsigned REPORT_LIMIT = 8;

static bool
should_report(Stats &s)
{
	if (s.reported >= REPORT_LIMIT)
		return false;
	s.reported++;
	return true;
}

// ---------------------------------------------------------------------------
// deterministic PRNG (splitmix64, fixed seed)
// ---------------------------------------------------------------------------

static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{
	rng_state = s;
}

static uint64_t
rng_next(void)
{
	uint64_t z = (rng_state += 0x9E3779B97F4A7C15ull);
	z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
	z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
	return z ^ (z >> 31);
}

static size_t
rng_below(size_t bound)
{
	if (bound == 0)
		return 0;
	return (size_t)(rng_next() % (uint64_t)bound);
}

// ---------------------------------------------------------------------------
// arena helpers
// ---------------------------------------------------------------------------

static const unsigned char GUARD = 0x7f;

static void
dump_first_diff(const void *a, const void *b, size_t nbytes)
{
	const unsigned char *pa = (const unsigned char *)a;
	const unsigned char *pb = (const unsigned char *)b;
	for (size_t i = 0; i < nbytes; i++) {
		if (pa[i] != pb[i]) {
			printf("      first byte diff at %zu: port=0x%02x "
			    "ref=0x%02x\n", i, pa[i], pb[i]);
			return;
		}
	}
	printf("      (arenas identical)\n");
}

static ptrdiff_t
ptr_offset(const wchar_t *base, const wchar_t *p)
{
	if (p == NULL)
		return -1;
	return p - base;
}

// ---------------------------------------------------------------------------
// wcsrchr
// ---------------------------------------------------------------------------

static const size_t WR_N      = 128;
static const size_t WR_LEAD   = 32;
static const size_t WR_MAXOFF = 32;
static const size_t WR_MAXLEN = 64;

static void
build_wstr(wchar_t *dst, size_t slen, unsigned pat, wchar_t tail)
{
	for (size_t i = 0; i < slen; i++) {
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
		dst[i] = v;
	}
	if (slen < WR_N - WR_LEAD)
		dst[slen] = L'\0';
	for (size_t i = slen + 1; i < WR_N - WR_LEAD; i++)
		dst[i] = tail;
}

static void
check_wcsrchr(const wchar_t *hay, size_t hay_base_off, wchar_t needle)
{
	wchar_t arena_a[WR_N], arena_b[WR_N];

	memset(arena_a, GUARD, sizeof(arena_a));
	memset(arena_b, GUARD, sizeof(arena_b));

	const wchar_t *sa = arena_a + WR_LEAD + hay_base_off;
	const wchar_t *sb = arena_b + WR_LEAD + hay_base_off;
	wchar_t *da = arena_a + WR_LEAD + hay_base_off;
	wchar_t *db = arena_b + WR_LEAD + hay_base_off;

	size_t copy_len = wcslen(hay) + 1;
	wmemcpy(da, hay, copy_len);
	wmemcpy(db, hay, copy_len);

	wchar_t *ra = P::wcsrchr(sa, needle);
	wchar_t *rb = ref_wcsrchr(sb, needle);

	ptrdiff_t oa = ptr_offset(arena_a, ra);
	ptrdiff_t ob = ptr_offset(arena_b, rb);

	st_wcsrchr.cases++;
	bool bad = false;

	if (oa != ob)
		bad = true;
	if (memcmp(arena_a, arena_b, sizeof(arena_a)) != 0)
		bad = true;

	if (bad) {
		st_wcsrchr.failures++;
		if (should_report(st_wcsrchr)) {
			printf("  FAIL wcsrchr(needle=0x%08x): ret offset "
			    "port=%td ref=%td\n",
			    (unsigned)(uint32_t)needle, oa, ob);
			dump_first_diff(arena_a, arena_b, sizeof(arena_a));
		}
	}
}

static void
wcsrchr_edge_cases(void)
{
	wchar_t src[WR_MAXLEN + 2];

	static const wchar_t needles[] = {
		L'\0', L'a', L'A', L'z', L'Z', (wchar_t)0x80,
		(wchar_t)0xff, (wchar_t)0x80000000u, (wchar_t)0xffffffffu,
		L'x'
	};

	// empty string
	src[0] = L'\0';
	for (size_t ni = 0; ni < sizeof(needles) / sizeof(needles[0]); ni++)
		check_wcsrchr(src, 0, needles[ni]);

	// single-character strings
	for (unsigned pat = 0; pat < 6; pat++) {
		for (size_t ch = 0; ch < 6; ch++) {
			wchar_t c;
			switch (ch) {
			case 0: c = L'a'; break;
			case 1: c = L'A'; break;
			case 2: c = (wchar_t)0x80; break;
			case 3: c = (wchar_t)0xff; break;
			case 4: c = (wchar_t)0x80000000u; break;
			default: c = L'x'; break;
			}
			src[0] = c;
			src[1] = L'\0';
			for (size_t ni = 0;
			    ni < sizeof(needles) / sizeof(needles[0]); ni++)
				check_wcsrchr(src, 0, needles[ni]);
		}
	}

	// fixed shapes: char at every position, duplicate needles, no match
	for (unsigned pat = 0; pat < 6; pat++) {
		for (size_t slen = 1; slen <= WR_MAXLEN; slen++) {
			build_wstr(src, slen, pat, (wchar_t)0xdeadbeefu);
			for (size_t pos = 0; pos <= slen; pos++) {
				wchar_t pin = (wchar_t)(L'm' + (pos % 5));
				if (pos < slen)
					src[pos] = pin;
				for (size_t ni = 0;
				    ni < sizeof(needles) / sizeof(needles[0]);
				    ni++)
					check_wcsrchr(src, 0, needles[ni]);
				check_wcsrchr(src, 0, pin);
				if (pos < slen)
					build_wstr(src, slen, pat,
					    (wchar_t)0xdeadbeefu);
			}
		}
	}

	// duplicate target: last occurrence must win
	for (size_t slen = 3; slen <= 16; slen++) {
		for (size_t i = 0; i < slen; i++)
			src[i] = (wchar_t)(L'b' + (i % 3));
		src[0] = L'q';
		src[slen / 2] = L'q';
		src[slen - 1] = L'q';
		src[slen] = L'\0';
		check_wcsrchr(src, 0, L'q');
	}

	// NUL-heavy haystacks
	for (size_t zpos = 0; zpos < WR_MAXLEN; zpos++) {
		for (size_t i = 0; i < WR_MAXLEN; i++)
			src[i] = (wchar_t)(0x80 + (i % 0x60));
		src[zpos] = L'\0';
		for (size_t ni = 0;
		    ni < sizeof(needles) / sizeof(needles[0]); ni++)
			check_wcsrchr(src, 0, needles[ni]);
	}

	// alignment offsets inside the arena
	for (size_t off = 0; off <= WR_MAXOFF; off++) {
		build_wstr(src, 8, 3, (wchar_t)0xcafebabeu);
		src[3] = L'X';
		check_wcsrchr(src, off, L'X');
		check_wcsrchr(src, off, L'\0');
		check_wcsrchr(src, off, (wchar_t)0x9999u);
	}
}

static void
wcsrchr_random_sweep(unsigned long long iters)
{
	wchar_t src[WR_MAXLEN + 2];

	for (unsigned long long it = 0; it < iters; it++) {
		size_t slen = rng_below(WR_MAXLEN + 1);
		size_t off = rng_below(WR_MAXOFF + 1);
		unsigned mode = (unsigned)(rng_next() & 3);

		for (size_t i = 0; i < slen; i++) {
			uint64_t r = rng_next();
			switch (mode) {
			case 0:
				src[i] = (wchar_t)(uint32_t)r;
				break;
			case 1:
				src[i] = (wchar_t)(uint32_t)(r | 0x80000000u);
				break;
			case 2:
				src[i] = (wchar_t)(uint32_t)(r & 0xffu);
				break;
			default:
				src[i] = (r & 1) ? L'\0' :
				    (wchar_t)(L'a' + (r % 26));
				break;
			}
			if (src[i] == 0 && i < slen)
				src[i] = (wchar_t)(0x80 + i);
		}
		src[slen] = L'\0';

		wchar_t needle;
		switch (rng_next() % 6u) {
		case 0:
			needle = slen > 0 ? src[rng_below(slen)] : L'\0';
			break;
		case 1:
			needle = L'\0';
			break;
		case 2:
			needle = (wchar_t)(uint32_t)rng_next();
			break;
		case 3:
			needle = (wchar_t)(0x80 | (rng_next() & 0x7fu));
			break;
		case 4:
			needle = (wchar_t)(L'A' + (rng_next() % 26));
			break;
		default:
			needle = (wchar_t)(0xdead0000u | (rng_next() & 0xffffu));
			break;
		}

		check_wcsrchr(src, off, needle);
	}
}

// ---------------------------------------------------------------------------
// wcsdup
// ---------------------------------------------------------------------------

static const size_t WD_MAXLEN = 96;
static const size_t WD_PAD    = 8;	// guard wchar_t past logical end

static void
check_wcsdup(const wchar_t *src)
{
	size_t len = wcslen(src) + 1;

	wchar_t *pa = P::wcsdup(src);
	wchar_t *pb = ref_wcsdup(src);

	st_wcsdup.cases++;
	bool bad = false;

	if ((pa == NULL) != (pb == NULL))
		bad = true;

	if (pa != NULL && pb != NULL) {
		size_t nbytes = len * sizeof(wchar_t);
		if (memcmp(pa, pb, nbytes) != 0)
			bad = true;
		if (wcscmp(pa, pb) != 0)
			bad = true;
	}

	if (bad) {
		st_wcsdup.failures++;
		if (should_report(st_wcsdup)) {
			printf("  FAIL wcsdup(len=%zu): port=%p ref=%p\n",
			    len - 1, (void *)pa, (void *)pb);
			if (pa != NULL && pb != NULL)
				dump_first_diff(pa, pb, len * sizeof(wchar_t));
		}
	}

	free(pa);
	free(pb);
}

static void
wcsdup_edge_cases(void)
{
	wchar_t src[WD_MAXLEN + WD_PAD + 1];

	// empty
	src[0] = L'\0';
	check_wcsdup(src);

	// single wchar_t
	static const wchar_t singles[] = {
		L'a', L'A', L'\0', (wchar_t)0x80, (wchar_t)0xff,
		(wchar_t)0x7fff, (wchar_t)0x8000, (wchar_t)0xffff,
		(wchar_t)0x80000000u, (wchar_t)0xffffffffu
	};
	for (size_t i = 0; i < sizeof(singles) / sizeof(singles[0]); i++) {
		if (singles[i] == L'\0')
			continue;
		src[0] = singles[i];
		src[1] = L'\0';
		check_wcsdup(src);
	}

	for (unsigned pat = 0; pat < 6; pat++) {
		for (size_t slen = 0; slen <= WD_MAXLEN; slen++) {
			for (size_t i = 0; i < slen; i++) {
				wchar_t v;
				switch (pat) {
				case 0:
					v = (wchar_t)(L'a' + (i % 26));
					break;
				case 1:
					v = (wchar_t)0xffffffffu;
					break;
				case 2:
					v = (wchar_t)0x80000000u;
					break;
				case 3:
					v = (wchar_t)(0x80 + (i % 0x70));
					break;
				case 4:
					v = (wchar_t)((i & 1) ? L'A' : L'a');
					break;
				default:
					v = (wchar_t)(i * 0x01010101u + 1);
					break;
				}
				if (v == 0)
					v = 1;
				src[i] = v;
			}
			src[slen] = L'\0';
			check_wcsdup(src);
		}
	}

	// NUL-heavy interior (only prefix is copied; wcslen stops at first NUL)
	for (size_t zpos = 1; zpos < WD_MAXLEN; zpos++) {
		for (size_t i = 0; i < WD_MAXLEN; i++)
			src[i] = (wchar_t)(0x80 + (i % 0x60));
		src[zpos] = L'\0';
		for (size_t i = zpos + 1; i < WD_MAXLEN; i++)
			src[i] = (wchar_t)0xbeef0000u;
		check_wcsdup(src);
	}

	// boundary lengths
	for (size_t slen = 1; slen <= WD_MAXLEN; slen += (slen < 16 ? 1 :
	    (slen < 48 ? 4 : 8))) {
		for (size_t i = 0; i < slen; i++)
			src[i] = (wchar_t)((i & 1) ? 0x80 : 0xff);
		src[slen] = L'\0';
		check_wcsdup(src);
	}
}

static void
wcsdup_random_sweep(unsigned long long iters)
{
	wchar_t src[WD_MAXLEN + 1];

	for (unsigned long long it = 0; it < iters; it++) {
		size_t slen = rng_below(WD_MAXLEN + 1);
		unsigned mode = (unsigned)(rng_next() & 3);

		for (size_t i = 0; i < slen; i++) {
			uint64_t r = rng_next();
			switch (mode) {
			case 0:
				src[i] = (wchar_t)(uint32_t)r;
				break;
			case 1:
				src[i] = (wchar_t)(uint32_t)(r | 0x80000000u);
				break;
			case 2:
				src[i] = (wchar_t)(uint32_t)(r & 0xffu);
				break;
			default:
				src[i] = (r & 7) ? (wchar_t)(L'a' + (r % 26)) :
				    L'\0';
				break;
			}
			if (src[i] == 0 && i < slen)
				src[i] = (wchar_t)(0x80 + i);
		}
		src[slen] = L'\0';

		if ((it & 127) == 0 && slen > 0) {
			size_t pin = rng_below(slen);
			for (size_t i = 0; i < pin; i++)
				if (src[i] == L'\0')
					src[i] = (wchar_t)(0x80 + i);
			src[pin] = L'\0';
		}

		check_wcsdup(src);
	}
}

// ---------------------------------------------------------------------------
// wcscasecmp
// ---------------------------------------------------------------------------

static const size_t CS_MAXLEN = 96;

static void
check_wcscasecmp(const wchar_t *s1, const wchar_t *s2)
{
	int ra = P::wcscasecmp(s1, s2);
	int rb = ref_wcscasecmp(s1, s2);

	st_wcscasecmp.cases++;
	if (ra != rb) {
		st_wcscasecmp.failures++;
		if (should_report(st_wcscasecmp)) {
			printf("  FAIL wcscasecmp: ret port=%d ref=%d\n",
			    ra, rb);
		}
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
wcscasecmp_edge_cases(void)
{
	wchar_t s1[CS_MAXLEN + 2];
	wchar_t s2[CS_MAXLEN + 2];

	// both empty
	s1[0] = L'\0';
	s2[0] = L'\0';
	check_wcscasecmp(s1, s2);

	// one empty
	for (size_t i = 0; i < CS_MAXLEN; i++)
		s1[i] = (wchar_t)(L'x' + (i % 5));
	s1[CS_MAXLEN] = L'\0';
	s2[0] = L'\0';
	check_wcscasecmp(s1, s2);
	check_wcscasecmp(s2, s1);

	// identical lowercase / uppercase / mixed
	for (size_t len = 1; len <= 32; len++) {
		for (size_t i = 0; i < len; i++)
			s1[i] = (wchar_t)(L'a' + (i % 26));
		s1[len] = L'\0';
		wmemcpy(s2, s1, len + 1);
		check_wcscasecmp(s1, s2);
		for (size_t i = 0; i < len; i++)
			s2[i] = (wchar_t)towupper(s1[i]);
		check_wcscasecmp(s1, s2);
	}

	// first difference at every position
	for (size_t len = 1; len <= 24; len++) {
		for (size_t diff_at = 0; diff_at < len; diff_at++) {
			for (size_t i = 0; i < len; i++)
				s1[i] = s2[i] = (wchar_t)(L'n' + (i % 7));
			s1[len] = s2[len] = L'\0';
			s2[diff_at] = (wchar_t)(s1[diff_at] + 1);
			check_wcscasecmp(s1, s2);
			check_wcscasecmp(s2, s1);
			s2[diff_at] = (wchar_t)(s1[diff_at] - 1);
			check_wcscasecmp(s1, s2);
		}
	}

	// length mismatch: s1 shorter / longer
	for (size_t l1 = 0; l1 <= 32; l1++) {
		for (size_t l2 = 0; l2 <= 32; l2++) {
			for (unsigned pat = 0; pat < 6; pat++) {
				for (int skew = 0; skew < 4; skew++) {
					fill_wcs_pair(s1, s2, l1, l2, pat,
					    skew);
					check_wcscasecmp(s1, s2);
				}
			}
		}
	}

	// high-bit only
	for (size_t len = 1; len <= 16; len++) {
		for (size_t i = 0; i < len; i++) {
			s1[i] = (wchar_t)(0x80 + i);
			s2[i] = (wchar_t)(0x80 + i);
		}
		s1[len] = s2[len] = L'\0';
		check_wcscasecmp(s1, s2);
		s2[len / 2] = (wchar_t)(0x80 + (len / 2) + 1);
		check_wcscasecmp(s1, s2);
	}

	// case-only difference resolved by towlower
	s1[0] = L'A';
	s1[1] = L'b';
	s1[2] = L'\0';
	s2[0] = L'a';
	s2[1] = L'B';
	s2[2] = L'\0';
	check_wcscasecmp(s1, s2);

	// difference visible only after towlower
	s1[0] = L'A';
	s1[1] = L'\0';
	s2[0] = L'a';
	s2[1] = L'z';
	s2[2] = L'\0';
	check_wcscasecmp(s1, s2);
}

static void
wcscasecmp_random_sweep(unsigned long long iters)
{
	wchar_t s1[CS_MAXLEN + 2];
	wchar_t s2[CS_MAXLEN + 2];

	for (unsigned long long it = 0; it < iters; it++) {
		size_t l1 = rng_below(CS_MAXLEN + 1);
		size_t l2 = rng_below(CS_MAXLEN + 1);
		unsigned pat = (unsigned)(rng_next() % 6u);
		int skew = (int)(rng_next() % 4u);

		for (size_t i = 0; i < l1; i++) {
			uint64_t r = rng_next();
			wchar_t v;
			switch (pat) {
			case 0:
				v = (wchar_t)(L'a' + (r % 26));
				break;
			case 1:
				v = (wchar_t)(uint32_t)r;
				break;
			case 2:
				v = (wchar_t)(uint32_t)(r | 0x80000000u);
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
				v = (r & 3) ? (wchar_t)(uint32_t)r : L'\0';
				break;
			}
			if (v == 0 && i < l1)
				v = (wchar_t)(0x80 + i);
			s1[i] = v;
		}
		s1[l1] = L'\0';

		fill_wcs_pair(s1, s2, l1, l2, pat, skew);

		if ((it & 63) == 0 && l1 > 0 && l2 > 0) {
			size_t pos = rng_below(l1 < l2 ? l1 : l2);
			s2[pos] = (wchar_t)(s1[pos] ^ (1u << (rng_next() % 5u)));
		}

		check_wcscasecmp(s1, s2);
	}
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

static void
row(const Stats &s)
{
	printf("  %-12s %14llu %14llu   %s\n", s.name, s.cases, s.failures,
	    s.failures == 0 ? "ok" : "FAILED");
}

int
main(void)
{
	const unsigned long long RANDOM_ITERS = 200000;

	printf("b0137 differential test (wchar_t is %zu bytes)\n",
	    sizeof(wchar_t));

	wcsrchr_edge_cases();
	rng_seed(0x0137C0FFEE000001ull);
	wcsrchr_random_sweep(RANDOM_ITERS);

	wcsdup_edge_cases();
	rng_seed(0x0137C0FFEE000002ull);
	wcsdup_random_sweep(RANDOM_ITERS);

	wcscasecmp_edge_cases();
	rng_seed(0x0137C0FFEE000003ull);
	wcscasecmp_random_sweep(RANDOM_ITERS);

	unsigned long long total_cases = st_wcsrchr.cases + st_wcsdup.cases +
	    st_wcscasecmp.cases;
	unsigned long long total_fail = st_wcsrchr.failures + st_wcsdup.failures +
	    st_wcscasecmp.failures;

	printf("\n  %-12s %14s %14s   %s\n", "function", "cases", "failures",
	    "status");
	printf("  ------------ -------------- --------------   ------\n");
	row(st_wcsrchr);
	row(st_wcsdup);
	row(st_wcscasecmp);
	printf("  ------------ -------------- --------------   ------\n");
	printf("  %-12s %14llu %14llu   %s\n", "TOTAL", total_cases, total_fail,
	    total_fail == 0 ? "ok" : "FAILED");

	if (total_cases == 0) {
		printf("\nno cases executed\n");
		return 1;
	}
	return total_fail == 0 ? 0 : 1;
}
