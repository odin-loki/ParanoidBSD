// Differential test harness for PBSD batch b0113.
//
// Every ported function is exercised against the unmodified reference
// implementation in oracle.c.  For the buffer-writing functions two separate
// arenas are pre-filled with the guard byte 0x7f, the identical input is fed to
// both implementations, and the WHOLE arena is compared afterwards -- including
// the bytes before and after the nominal write window -- so that an off-by-one
// or a stray store is caught, not just a wrong return value.  Pointer results
// are always reduced to an offset from the arena base before comparison.

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

import pbsd.lib.libc.string.b0113;

namespace P = pbsd::lib_libc_string::b0113;

extern "C" {
void *ref_mempcpy(void *__restrict dst, const void *__restrict src, size_t len);
wchar_t *ref_wcpncpy(wchar_t *__restrict dst, const wchar_t *__restrict src,
    size_t n);
wchar_t *ref_wmempcpy(wchar_t *__restrict dst, const wchar_t *__restrict src,
    size_t len);
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

static Stats st_mempcpy  = { "mempcpy",  0, 0, 0 };
static Stats st_wcpncpy  = { "wcpncpy",  0, 0, 0 };
static Stats st_wmempcpy = { "wmempcpy", 0, 0, 0 };

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
	return (size_t)(rng_next() % (uint64_t)bound);
}

// ---------------------------------------------------------------------------
// arena helpers
// ---------------------------------------------------------------------------

static const unsigned char GUARD = 0x7f;

// Print the first differing byte of two arenas, for diagnostics.
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

// ---------------------------------------------------------------------------
// mempcpy
// ---------------------------------------------------------------------------
//
// Arena is MP_N bytes.  The destination starts at MP_LEAD + off so that a
// mutated `- len` return value still lands inside the arena and the leading
// guard region is checked for stray writes as well.

static const size_t MP_N      = 256;
static const size_t MP_LEAD   = 64;
static const size_t MP_MAXOFF = 64;
static const size_t MP_MAXLEN = 64;

static void
check_mempcpy(const unsigned char *src, size_t len, size_t off)
{
	unsigned char a[MP_N], b[MP_N];

	memset(a, GUARD, MP_N);
	memset(b, GUARD, MP_N);

	unsigned char *da = a + MP_LEAD + off;
	unsigned char *db = b + MP_LEAD + off;

	void *ra = P::mempcpy(da, src, len);
	void *rb = ref_mempcpy(db, src, len);

	ptrdiff_t oa = (unsigned char *)ra - a;
	ptrdiff_t ob = (unsigned char *)rb - b;

	st_mempcpy.cases++;
	bool bad = false;

	if (oa != ob)
		bad = true;
	if (memcmp(a, b, MP_N) != 0)
		bad = true;

	if (bad) {
		st_mempcpy.failures++;
		if (should_report(st_mempcpy)) {
			printf("  FAIL mempcpy(off=%zu, len=%zu): "
			    "ret offset port=%td ref=%td\n",
			    off, len, oa, ob);
			dump_first_diff(a, b, MP_N);
		}
	}
}

static void
mempcpy_edge_cases(void)
{
	static const size_t offs[] = { 0, 1, 2, 3, 7, 8, 9, 15, 16, 31, 32,
	    63, 64 };
	static const size_t lens[] = { 0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17,
	    31, 32, 33, 63, 64 };

	unsigned char src[MP_MAXLEN + 1];

	for (unsigned pat = 0; pat < 6; pat++) {
		for (size_t i = 0; i <= MP_MAXLEN; i++) {
			switch (pat) {
			case 0: src[i] = 0x00; break;
			case 1: src[i] = 0xff; break;
			case 2: src[i] = 0x80; break;
			case 3: src[i] = (unsigned char)(0x80 + (i & 0x7f));
				break;
			case 4: src[i] = (unsigned char)((i & 1) ? 0x00 : 0xfe);
				break;
			default: src[i] = (unsigned char)(i * 7 + 1); break;
			}
		}
		for (size_t oi = 0; oi < sizeof(offs) / sizeof(offs[0]); oi++) {
			for (size_t li = 0;
			    li < sizeof(lens) / sizeof(lens[0]); li++) {
				if (MP_LEAD + offs[oi] + lens[li] > MP_N)
					continue;
				check_mempcpy(src, lens[li], offs[oi]);
			}
		}
	}
}

static void
mempcpy_random_sweep(unsigned long long iters)
{
	unsigned char src[MP_MAXLEN + 1];

	for (unsigned long long it = 0; it < iters; it++) {
		size_t len = rng_below(MP_MAXLEN + 1);
		size_t off = rng_below(MP_MAXOFF + 1);

		// Mix of fully random bytes and high-bit-only bytes.
		if ((it & 3) == 0) {
			for (size_t i = 0; i <= MP_MAXLEN; i++)
				src[i] = (unsigned char)(0x80 |
				    (rng_next() & 0x7f));
		} else if ((it & 3) == 1) {
			for (size_t i = 0; i <= MP_MAXLEN; i++)
				src[i] = (unsigned char)((rng_next() & 1) ?
				    0x00 : 0xff);
		} else {
			for (size_t i = 0; i <= MP_MAXLEN; i++)
				src[i] = (unsigned char)rng_next();
		}

		check_mempcpy(src, len, off);
	}
}

// ---------------------------------------------------------------------------
// wmempcpy
// ---------------------------------------------------------------------------

static const size_t WM_N      = 128;	// wchar_t elements
static const size_t WM_LEAD   = 32;
static const size_t WM_MAXOFF = 32;
static const size_t WM_MAXLEN = 48;

static void
check_wmempcpy(const wchar_t *src, size_t len, size_t off)
{
	wchar_t a[WM_N], b[WM_N];

	memset(a, GUARD, sizeof(a));
	memset(b, GUARD, sizeof(b));

	wchar_t *da = a + WM_LEAD + off;
	wchar_t *db = b + WM_LEAD + off;

	wchar_t *ra = P::wmempcpy(da, src, len);
	wchar_t *rb = ref_wmempcpy(db, src, len);

	ptrdiff_t oa = ra - a;
	ptrdiff_t ob = rb - b;

	st_wmempcpy.cases++;
	bool bad = false;

	if (oa != ob)
		bad = true;
	if (memcmp(a, b, sizeof(a)) != 0)
		bad = true;

	if (bad) {
		st_wmempcpy.failures++;
		if (should_report(st_wmempcpy)) {
			printf("  FAIL wmempcpy(off=%zu, len=%zu): "
			    "ret offset port=%td ref=%td\n",
			    off, len, oa, ob);
			dump_first_diff(a, b, sizeof(a));
		}
	}
}

static void
wmempcpy_edge_cases(void)
{
	static const size_t offs[] = { 0, 1, 2, 3, 7, 8, 15, 16, 31, 32 };
	static const size_t lens[] = { 0, 1, 2, 3, 4, 5, 7, 8, 15, 16, 17,
	    31, 32, 47, 48 };

	wchar_t src[WM_MAXLEN + 1];

	for (unsigned pat = 0; pat < 6; pat++) {
		for (size_t i = 0; i <= WM_MAXLEN; i++) {
			switch (pat) {
			case 0: src[i] = L'\0'; break;
			case 1: src[i] = (wchar_t)0xffffffffu; break;
			case 2: src[i] = (wchar_t)0x80000000u; break;
			case 3: src[i] = (wchar_t)(0x80 + i); break;
			case 4: src[i] = (wchar_t)((i & 1) ? 0 : 0x7fffffff);
				break;
			default: src[i] = (wchar_t)(i * 0x01010101u + 1);
				break;
			}
		}
		for (size_t oi = 0; oi < sizeof(offs) / sizeof(offs[0]); oi++) {
			for (size_t li = 0;
			    li < sizeof(lens) / sizeof(lens[0]); li++) {
				if (WM_LEAD + offs[oi] + lens[li] > WM_N)
					continue;
				check_wmempcpy(src, lens[li], offs[oi]);
			}
		}
	}
}

static void
wmempcpy_random_sweep(unsigned long long iters)
{
	wchar_t src[WM_MAXLEN + 1];

	for (unsigned long long it = 0; it < iters; it++) {
		size_t len = rng_below(WM_MAXLEN + 1);
		size_t off = rng_below(WM_MAXOFF + 1);

		for (size_t i = 0; i <= WM_MAXLEN; i++) {
			uint64_t r = rng_next();
			switch (it & 3) {
			case 0:
				src[i] = (wchar_t)(uint32_t)r;
				break;
			case 1:
				src[i] = (wchar_t)(uint32_t)(r | 0x80000000u);
				break;
			case 2:
				src[i] = (wchar_t)((r & 1) ? 0 :
				    (wchar_t)0xffffffffu);
				break;
			default:
				src[i] = (wchar_t)(uint32_t)(r & 0xffu);
				break;
			}
		}

		check_wmempcpy(src, len, off);
	}
}

// ---------------------------------------------------------------------------
// wcpncpy
// ---------------------------------------------------------------------------

static const size_t WC_N      = 128;	// wchar_t elements
static const size_t WC_LEAD   = 32;
static const size_t WC_MAXOFF = 32;
static const size_t WC_MAXN   = 48;
static const size_t WC_SRC_N  = 96;	// always fully initialised

static void
check_wcpncpy(const wchar_t *src, size_t n, size_t off)
{
	wchar_t a[WC_N], b[WC_N];

	memset(a, GUARD, sizeof(a));
	memset(b, GUARD, sizeof(b));

	wchar_t *da = a + WC_LEAD + off;
	wchar_t *db = b + WC_LEAD + off;

	wchar_t *ra = P::wcpncpy(da, src, n);
	wchar_t *rb = ref_wcpncpy(db, src, n);

	ptrdiff_t oa = ra - a;
	ptrdiff_t ob = rb - b;

	st_wcpncpy.cases++;
	bool bad = false;

	if (oa != ob)
		bad = true;
	if (memcmp(a, b, sizeof(a)) != 0)
		bad = true;

	if (bad) {
		st_wcpncpy.failures++;
		if (should_report(st_wcpncpy)) {
			printf("  FAIL wcpncpy(off=%zu, n=%zu): "
			    "ret offset port=%td ref=%td\n",
			    off, n, oa, ob);
			dump_first_diff(a, b, sizeof(a));
		}
	}
}

// Build a source string of `slen` non-NUL wide characters followed by NUL,
// padded out to WC_SRC_N with `tail`, so the whole array is initialised no
// matter how far the implementation reads.
static void
build_wsrc(wchar_t *dst, size_t slen, unsigned pat, wchar_t tail)
{
	for (size_t i = 0; i < slen; i++) {
		wchar_t v;
		switch (pat) {
		case 0: v = (wchar_t)(L'a' + (i % 26)); break;
		case 1: v = (wchar_t)0xffffffffu; break;
		case 2: v = (wchar_t)0x80000000u; break;
		case 3: v = (wchar_t)(0x80 + (i % 0x80)); break;
		case 4: v = (wchar_t)0x7fffffff; break;
		default: v = (wchar_t)(i * 0x01010101u + 1); break;
		}
		if (v == 0)
			v = 1;
		dst[i] = v;
	}
	if (slen < WC_SRC_N)
		dst[slen] = L'\0';
	for (size_t i = slen + 1; i < WC_SRC_N; i++)
		dst[i] = tail;
}

static void
wcpncpy_edge_cases(void)
{
	wchar_t src[WC_SRC_N];

	static const size_t offs[] = { 0, 1, 2, 3, 7, 8, 15, 16, 31, 32 };

	// Exhaustive small grid: every (n, strlen) pair up to WC_MAXN, which
	// covers strlen < n-1, strlen == n-1, strlen == n and strlen > n --
	// i.e. both sides of every boundary in the copy loop.
	for (unsigned pat = 0; pat < 6; pat++) {
		for (unsigned t = 0; t < 2; t++) {
			wchar_t tail = t ? L'\0' : (wchar_t)0xdeadbeefu;
			for (size_t n = 0; n <= WC_MAXN; n++) {
				for (size_t slen = 0;
				    slen <= WC_MAXN + 2 && slen < WC_SRC_N;
				    slen++) {
					build_wsrc(src, slen, pat, tail);
					check_wcpncpy(src, n, 0);
					check_wcpncpy(src, n, 1);
				}
			}
		}
	}

	// Same shapes across every destination alignment/offset, including the
	// offset that puts the last written element flush against the end of
	// the nominal window.
	for (size_t oi = 0; oi < sizeof(offs) / sizeof(offs[0]); oi++) {
		for (size_t n = 0; n <= WC_MAXN; n++) {
			if (WC_LEAD + offs[oi] + n > WC_N)
				continue;
			for (size_t slen = 0; slen <= WC_MAXN + 2; slen++) {
				build_wsrc(src, slen, slen % 6,
				    (wchar_t)0xcafebabeu);
				check_wcpncpy(src, n, offs[oi]);
			}
		}
	}

	// NUL-heavy sources: a NUL at every single position, with non-NUL data
	// both before and after it.
	for (size_t zpos = 0; zpos < WC_MAXN + 2; zpos++) {
		for (size_t i = 0; i < WC_SRC_N; i++)
			src[i] = (wchar_t)(0x80 + (i % 0x70));
		src[zpos] = L'\0';
		for (size_t n = 0; n <= WC_MAXN; n++)
			check_wcpncpy(src, n, 0);
	}

	// Alternating NUL / high-bit source, both phases.
	for (unsigned phase = 0; phase < 2; phase++) {
		for (size_t i = 0; i < WC_SRC_N; i++)
			src[i] = ((i + phase) & 1) ? L'\0' :
			    (wchar_t)0xffffffffu;
		for (size_t n = 0; n <= WC_MAXN; n++)
			for (size_t oi = 0;
			    oi < sizeof(offs) / sizeof(offs[0]); oi++) {
				if (WC_LEAD + offs[oi] + n > WC_N)
					continue;
				check_wcpncpy(src, n, offs[oi]);
			}
	}
}

static void
wcpncpy_random_sweep(unsigned long long iters)
{
	wchar_t src[WC_SRC_N];

	for (unsigned long long it = 0; it < iters; it++) {
		size_t n = rng_below(WC_MAXN + 1);
		size_t off = rng_below(WC_MAXOFF + 1);
		if (WC_LEAD + off + n > WC_N)
			off = 0;

		unsigned mode = (unsigned)(rng_next() & 3);
		for (size_t i = 0; i < WC_SRC_N; i++) {
			uint64_t r = rng_next();
			switch (mode) {
			case 0:
				// Dense NULs: forces the terminator branch and
				// the zero-fill tail at every position.
				src[i] = (r & 3) ? (wchar_t)(uint32_t)r : L'\0';
				break;
			case 1:
				// Sparse NULs: mostly runs past n.
				src[i] = (r % 64u == 0) ? L'\0' :
				    (wchar_t)(uint32_t)(r | 0x80000000u);
				break;
			case 2:
				src[i] = (wchar_t)(uint32_t)(r & 0xffu);
				break;
			default:
				src[i] = (r & 1) ? L'\0' :
				    (wchar_t)0xffffffffu;
				break;
			}
		}

		// Occasionally pin the NUL exactly on a boundary of n so the
		// strlen == n-2 / n-1 / n / n+1 transitions get hit far more
		// often than they would by chance.
		if (n > 0) {
			uint64_t r = rng_next();
			size_t pin;
			switch (r % 5u) {
			case 0: pin = n - 1; break;
			case 1: pin = n; break;
			case 2: pin = n + 1 < WC_SRC_N ? n + 1 : n; break;
			case 3: pin = n > 1 ? n - 2 : 0; break;
			default: pin = WC_SRC_N; break;
			}
			if (pin < WC_SRC_N) {
				for (size_t i = 0; i < pin; i++)
					if (src[i] == L'\0')
						src[i] = (wchar_t)(0x80 + i);
				src[pin] = L'\0';
			}
		}

		check_wcpncpy(src, n, off);
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

	printf("b0113 differential test (wchar_t is %zu bytes)\n",
	    sizeof(wchar_t));

	mempcpy_edge_cases();
	rng_seed(0x0113C0FFEE000001ull);
	mempcpy_random_sweep(RANDOM_ITERS);

	wmempcpy_edge_cases();
	rng_seed(0x0113C0FFEE000002ull);
	wmempcpy_random_sweep(RANDOM_ITERS);

	wcpncpy_edge_cases();
	rng_seed(0x0113C0FFEE000003ull);
	wcpncpy_random_sweep(RANDOM_ITERS);

	unsigned long long total_cases = st_mempcpy.cases + st_wcpncpy.cases +
	    st_wmempcpy.cases;
	unsigned long long total_fail = st_mempcpy.failures +
	    st_wcpncpy.failures + st_wmempcpy.failures;

	printf("\n  %-12s %14s %14s   %s\n", "function", "cases", "failures",
	    "status");
	printf("  ------------ -------------- --------------   ------\n");
	row(st_mempcpy);
	row(st_wcpncpy);
	row(st_wmempcpy);
	printf("  ------------ -------------- --------------   ------\n");
	printf("  %-12s %14llu %14llu   %s\n", "TOTAL", total_cases, total_fail,
	    total_fail == 0 ? "ok" : "FAILED");

	if (total_cases == 0) {
		printf("\nno cases executed\n");
		return 1;
	}
	return total_fail == 0 ? 0 : 1;
}
