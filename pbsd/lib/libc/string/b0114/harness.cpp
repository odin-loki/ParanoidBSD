/*
 * harness.cpp -- differential test for PBSD batch b0114.
 *
 * Every case is executed twice: once against the C++ port (module
 * pbsd.lib.libc.string.b0114) and once against the unmodified C reference in
 * oracle.c.  Return values, pointer *offsets* (never raw addresses) and the
 * complete contents of every buffer -- including the guard bytes past the
 * nominal write window -- are compared after each call.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>

import pbsd.lib.libc.string.b0114;

namespace port = pbsd::lib_libc_string::b0114;

extern "C" wchar_t *ref_wmemcpy(wchar_t * __restrict d,
    const wchar_t * __restrict s, std::size_t n);
extern "C" wchar_t *ref_wcpcpy(wchar_t * __restrict to,
    const wchar_t * __restrict from);

/* ------------------------------------------------------------------------ */
/* infrastructure                                                            */
/* ------------------------------------------------------------------------ */

static const unsigned char GUARD = 0x7f;
static const int MAX_REPORT = 8;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat st_wmemcpy = { "wmemcpy", 0, 0, 0 };
static Stat st_wcpcpy = { "wcpcpy", 0, 0, 0 };

static std::uint64_t rng_state;

static inline std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static inline std::size_t
rnd_mod(std::size_t m)
{
	return (std::size_t)(rnd() % (std::uint64_t)m);
}

static long
wptr_off(const wchar_t *base, const wchar_t *p)
{
	return (long)(p - base);
}

static void
dump_wide(const char *label, const wchar_t *p, std::size_t n)
{
	std::printf("      %s:", label);
	for (std::size_t i = 0; i < n; i++)
		std::printf(" %08lx", (unsigned long)(std::uint32_t)p[i]);
	std::printf("\n");
}

/* ------------------------------------------------------------------------ */
/* wmemcpy                                                                   */
/* ------------------------------------------------------------------------ */

static const std::size_t WBUF = 96;
static const std::size_t WSRC = 64;

static void
wmemcpy_run(const wchar_t *src, std::size_t n, const char *tag)
{
	wchar_t da[WBUF], db[WBUF];
	wchar_t sa[WSRC], sb[WSRC];

	if (n > WSRC) {
		std::fprintf(stderr,
		    "harness bug: %s would overflow src (n=%zu)\n", tag, n);
		std::exit(2);
	}

	std::memset(da, GUARD, sizeof(da));
	std::memset(db, GUARD, sizeof(db));
	std::memset(sa, GUARD, sizeof(sa));
	std::memset(sb, GUARD, sizeof(sb));
	std::memcpy(sa, src, n * sizeof(wchar_t));
	std::memcpy(sb, src, n * sizeof(wchar_t));

	wchar_t *ra = port::wmemcpy(da, sa, n);
	wchar_t *rb = ref_wmemcpy(db, sb, n);

	long off_a = wptr_off(da, ra);
	long off_b = wptr_off(db, rb);

	int bad_ret = (off_a != off_b);
	int bad_d = (std::memcmp(da, db, sizeof(da)) != 0);
	int bad_s = (std::memcmp(sa, sb, sizeof(sa)) != 0);

	st_wmemcpy.cases++;
	if (bad_ret || bad_d || bad_s) {
		st_wmemcpy.fails++;
		if (st_wmemcpy.reported < MAX_REPORT) {
			st_wmemcpy.reported++;
			std::printf("  FAIL wmemcpy [%s] n=%zu%s%s%s\n", tag, n,
			    bad_ret ? " ret" : "", bad_d ? " dst" : "",
			    bad_s ? " src" : "");
			std::printf("      port_off=%ld ref_off=%ld\n", off_a,
			    off_b);
			dump_wide("port_dst", da, WBUF);
			dump_wide("ref_dst ", db, WBUF);
			dump_wide("port_src", sa, WSRC);
			dump_wide("ref_src ", sb, WSRC);
		}
	}
}

static const wchar_t wcpy_alpha[] = {
	(wchar_t)0x0001, (wchar_t)0x007f, (wchar_t)0x0080, (wchar_t)0x00ff,
	(wchar_t)0x0100, (wchar_t)0x1234, (wchar_t)0x7fff, (wchar_t)0x8000,
	(wchar_t)0xffff, (wchar_t)0x41, (wchar_t)0x61, L'\0'
};

static void
wmemcpy_fill(wchar_t *buf, std::size_t n, int pattern)
{
	std::size_t na = sizeof(wcpy_alpha) / sizeof(wcpy_alpha[0]);

	for (std::size_t i = 0; i < n; i++) {
		switch (pattern) {
		case 0:
			buf[i] = wcpy_alpha[i % (na - 1)];
			break;
		case 1:
			buf[i] = (wchar_t)(0x80 + (i & 0x7f));
			break;
		case 2:
			buf[i] = (wchar_t)(0xff00 | (i & 0xff));
			break;
		default:
			buf[i] = (wchar_t)(i * 0x1111 + 0x80);
			break;
		}
	}
}

static void
wmemcpy_edge(void)
{
	wchar_t src[WSRC];

	/* n == 0: no bytes copied, buffers unchanged */
	wmemcpy_fill(src, 8, 0);
	wmemcpy_run(src, 0, "n0");

	/* single wchar */
	src[0] = L'a';
	wmemcpy_run(src, 1, "n1");

	/* empty source content but n > 0 (no NUL required) */
	src[0] = (wchar_t)0x0080;
	wmemcpy_run(src, 1, "hi-bit-1");

	/* two-unit boundary around loop body */
	src[0] = L'x';
	src[1] = L'y';
	wmemcpy_run(src, 1, "n1-of-2");
	wmemcpy_run(src, 2, "n2");

	/* high-bit wchar values */
	for (int v = 0x80; v <= 0xff; v += 0x11) {
		src[0] = (wchar_t)v;
		src[1] = (wchar_t)(v ^ 0xff);
		src[2] = (wchar_t)0x7f;
		wmemcpy_run(src, 3, "hi-byte");
	}

	/* NUL-heavy: copy includes embedded NULs */
	src[0] = L'a';
	src[1] = L'\0';
	src[2] = L'b';
	src[3] = L'\0';
	src[4] = L'c';
	wmemcpy_run(src, 5, "nul-heavy");

	/* exact-fill lengths */
	for (std::size_t len = 0; len <= 16; len++) {
		wmemcpy_fill(src, len, 0);
		wmemcpy_run(src, len, "exact-len");
	}

	/* boundary lengths around common sizes */
	static const std::size_t bounds[] = {
		0, 1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64
	};
	for (std::size_t bi = 0; bi < sizeof(bounds) / sizeof(bounds[0]);
	    bi++) {
		std::size_t n = bounds[bi];
		if (n > WSRC)
			continue;
		wmemcpy_fill(src, n, 1);
		wmemcpy_run(src, n, "bound");
	}

	/* n * sizeof(wchar_t) sensitivity: all-0xFF pattern */
	for (std::size_t n = 1; n <= 8; n++) {
		for (std::size_t i = 0; i < n; i++)
			src[i] = (wchar_t)0xffff;
		wmemcpy_run(src, n, "all-ffff");
	}
}

static void
wmemcpy_random(long iters)
{
	wchar_t src[WSRC];

	for (long it = 0; it < iters; it++) {
		std::size_t n;
		int pat = (int)rnd_mod(4);

		switch ((int)rnd_mod(12)) {
		case 0:
			n = 0;
			break;
		case 1:
			n = 1;
			break;
		case 2:
		case 3:
			n = rnd_mod(8);
			break;
		case 4:
		case 5:
			n = rnd_mod(32);
			break;
		default:
			n = rnd_mod(WSRC);
			break;
		}

		wmemcpy_fill(src, n, pat);
		wmemcpy_run(src, n, "rand");
	}
}

/* ------------------------------------------------------------------------ */
/* wcpcpy                                                                    */
/* ------------------------------------------------------------------------ */

static void
wcpcpy_run(const wchar_t *from, std::size_t from_cap, const char *tag)
{
	wchar_t da[WBUF], db[WBUF];
	wchar_t fa[WSRC], fb[WSRC];
	std::size_t flen = std::wcslen(from);

	if (flen + 1 > WBUF || flen + 1 > from_cap) {
		std::fprintf(stderr,
		    "harness bug: %s would overflow (flen=%zu cap=%zu)\n",
		    tag, flen, from_cap);
		std::exit(2);
	}

	std::memset(da, GUARD, sizeof(da));
	std::memset(db, GUARD, sizeof(db));
	std::memset(fa, GUARD, sizeof(fa));
	std::memset(fb, GUARD, sizeof(fb));
	std::memcpy(fa, from, (flen + 1) * sizeof(wchar_t));
	std::memcpy(fb, from, (flen + 1) * sizeof(wchar_t));

	wchar_t *ra = port::wcpcpy(da, fa);
	wchar_t *rb = ref_wcpcpy(db, fb);

	long off_a = wptr_off(da, ra);
	long off_b = wptr_off(db, rb);
	long expect = (long)flen;

	int bad_ret = (off_a != off_b) || (off_a != expect);
	int bad_d = (std::memcmp(da, db, sizeof(da)) != 0);
	int bad_f = (std::memcmp(fa, fb, sizeof(fa)) != 0);

	st_wcpcpy.cases++;
	if (bad_ret || bad_d || bad_f) {
		st_wcpcpy.fails++;
		if (st_wcpcpy.reported < MAX_REPORT) {
			st_wcpcpy.reported++;
			std::printf("  FAIL wcpcpy [%s] flen=%zu%s%s%s\n", tag,
			    flen, bad_ret ? " ret" : "", bad_d ? " dst" : "",
			    bad_f ? " from" : "");
			std::printf("      port_off=%ld ref_off=%ld expect=%ld\n",
			    off_a, off_b, expect);
			dump_wide("port_dst", da, WBUF);
			dump_wide("ref_dst ", db, WBUF);
		}
	}
}

static const wchar_t *const wcp_from[] = {
	L"",
	L"a",
	L"b",
	L"z",
	L"ab",
	L"ba",
	L"abc",
	L"hello",
	L"world",
	L"\x7f",
	L"\x80",
	L"\xff",
	L"\xffff",
	L"\x10000",
	L"a\x000" "b",
	L"aaaaaaaaaa",
	L"bbbbbbbbbb",
	L"\x80\x80\x80",
	L"\xff\xff",
	L"\x01\x02\x03",
};

static void
wcpcpy_edge(void)
{
	std::size_t nf = sizeof(wcp_from) / sizeof(wcp_from[0]);

	for (std::size_t i = 0; i < nf; i++)
		wcpcpy_run(wcp_from[i], WSRC, "grid");

	/* every low-byte value as single-character string */
	{
		wchar_t s[2];

		s[1] = L'\0';
		for (int v = 0; v < 256; v++) {
			s[0] = (wchar_t)v;
			wcpcpy_run(s, 2, "byte-val");
		}
	}

	/* lengths 0..20 exercise loop entry, body, and exit */
	{
		wchar_t s[24];

		for (std::size_t len = 0; len <= 20; len++) {
			for (std::size_t i = 0; i < len; i++)
				s[i] = (wchar_t)(L'a' + (wchar_t)(i % 26));
			s[len] = L'\0';
			wcpcpy_run(s, len + 1, "len-scan");
		}
	}

	/* NUL-heavy source: only prefix before first NUL is copied */
	{
		wchar_t s[] = { L'x', L'\0', L'y', L'z', L'\0' };
		wcpcpy_run(s, 5, "early-nul");
	}

	/* high-bit wchar sequences */
	{
		wchar_t s[8];

		for (int i = 0; i < 6; i++)
			s[i] = (wchar_t)(0x8000 + i * 0x111);
		s[6] = L'\0';
		wcpcpy_run(s, 8, "hi-seq");
	}
}

static void
wcpcpy_random(long iters)
{
	wchar_t from[WSRC];
	std::size_t na = sizeof(wcpy_alpha) / sizeof(wcpy_alpha[0]);

	for (long it = 0; it < iters; it++) {
		std::size_t flen;
		int wide = (rnd_mod(5) == 0);

		if (rnd_mod(20) == 0)
			flen = 0;
		else if (rnd_mod(10) == 0)
			flen = 1;
		else
			flen = rnd_mod(24);

		for (std::size_t i = 0; i < flen; i++) {
			if (wide)
				from[i] = (wchar_t)(rnd_mod(0x10000));
			else
				from[i] = wcpy_alpha[rnd_mod(na - 1)];
		}
		from[flen] = L'\0';

		/* occasional embedded NUL (stops copy early) */
		if (flen > 2 && rnd_mod(7) == 0)
			from[rnd_mod(flen)] = L'\0';

		wcpcpy_run(from, flen + 1, "rand");
	}
}

/* ------------------------------------------------------------------------ */

int
main(void)
{
	rng_state = 0x01140114ULL;
	wmemcpy_edge();
	wmemcpy_random(200000);

	rng_state = 0x01140214ULL;
	wcpcpy_edge();
	wcpcpy_random(200000);

	std::printf("\n%-12s %12s %12s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-12s %12s %12s   %s\n", "------------", "------------",
	    "------------", "------");

	const Stat *all[] = { &st_wmemcpy, &st_wcpcpy };
	long total_fail = 0;
	long total_case = 0;

	for (std::size_t i = 0; i < sizeof(all) / sizeof(all[0]); i++) {
		std::printf("%-12s %12ld %12ld   %s\n", all[i]->name,
		    all[i]->cases, all[i]->fails,
		    all[i]->fails == 0 ? "PASS" : "FAIL");
		total_fail += all[i]->fails;
		total_case += all[i]->cases;
	}
	std::printf("%-12s %12s %12s   %s\n", "------------", "------------",
	    "------------", "------");
	std::printf("%-12s %12ld %12ld   %s\n", "TOTAL", total_case,
	    total_fail, total_fail == 0 ? "PASS" : "FAIL");

	return total_fail == 0 ? 0 : 1;
}
