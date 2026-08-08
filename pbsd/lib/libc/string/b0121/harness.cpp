/*
 * harness.cpp -- differential test for PBSD batch b0121.
 *
 * Every case is executed twice: once against the C++ port (module
 * pbsd.lib.libc.string.b0121) and once against the unmodified C reference in
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

import pbsd.lib.libc.string.b0121;

namespace port = pbsd::lib_libc_string::b0121;

extern "C" wchar_t *ref_wmemmove(wchar_t *d, const wchar_t *s, std::size_t n);
extern "C" wchar_t *ref_wcscpy(wchar_t *__restrict s1,
    const wchar_t *__restrict s2);
extern "C" wchar_t *ref_wmemset(wchar_t *s, wchar_t c, std::size_t n);

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

static Stat st_wmemmove = { "wmemmove", 0, 0, 0 };
static Stat st_wcscpy = { "wcscpy", 0, 0, 0 };
static Stat st_wmemset = { "wmemset", 0, 0, 0 };

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

static const wchar_t w_alpha[] = {
	(wchar_t)0x0001, (wchar_t)0x007f, (wchar_t)0x0080, (wchar_t)0x00ff,
	(wchar_t)0x0100, (wchar_t)0x1234, (wchar_t)0x7fff, (wchar_t)0x8000,
	(wchar_t)0xffff, (wchar_t)0x41, (wchar_t)0x61, L'\0'
};

static void
wfill(wchar_t *buf, std::size_t n, int pattern)
{
	std::size_t na = sizeof(w_alpha) / sizeof(w_alpha[0]);

	for (std::size_t i = 0; i < n; i++) {
		switch (pattern) {
		case 0:
			buf[i] = w_alpha[i % (na - 1)];
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

/* ------------------------------------------------------------------------ */
/* wmemmove                                                                  */
/* ------------------------------------------------------------------------ */

static const std::size_t WBUF = 128;

static void
wmemmove_run(std::size_t src_off, std::size_t dst_off, std::size_t n,
    int pattern, const char *tag)
{
	wchar_t a[WBUF], b[WBUF];

	if (src_off + n > WBUF || dst_off + n > WBUF) {
		std::fprintf(stderr,
		    "harness bug: %s would overflow (src=%zu dst=%zu n=%zu)\n",
		    tag, src_off, dst_off, n);
		std::exit(2);
	}

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	wfill(a, WBUF, pattern);
	wfill(b, WBUF, pattern);

	wchar_t *ra = port::wmemmove(a + dst_off, a + src_off, n);
	wchar_t *rb = ref_wmemmove(b + dst_off, b + src_off, n);

	long off_a = wptr_off(a, ra);
	long off_b = wptr_off(b, rb);
	long expect = (long)dst_off;

	int bad_ret = (off_a != off_b) || (off_a != expect);
	int bad_buf = (std::memcmp(a, b, sizeof(a)) != 0);

	st_wmemmove.cases++;
	if (bad_ret || bad_buf) {
		st_wmemmove.fails++;
		if (st_wmemmove.reported < MAX_REPORT) {
			st_wmemmove.reported++;
			std::printf(
			    "  FAIL wmemmove [%s] src=%zu dst=%zu n=%zu%s%s\n",
			    tag, src_off, dst_off, n, bad_ret ? " ret" : "",
			    bad_buf ? " buf" : "");
			std::printf(
			    "      port_off=%ld ref_off=%ld expect=%ld\n",
			    off_a, off_b, expect);
			dump_wide("port", a, WBUF);
			dump_wide("ref ", b, WBUF);
		}
	}
}

static void
wmemmove_edge(void)
{
	/* n == 0: no wchar_t copied */
	wmemmove_run(0, 0, 0, 0, "n0");
	wmemmove_run(4, 8, 0, 0, "n0-offset");
	wmemmove_run(10, 2, 0, 1, "n0-overlap");

	/* single wchar, disjoint */
	wmemmove_run(0, 16, 1, 0, "n1-disjoint");
	wmemmove_run(16, 0, 1, 1, "n1-rev");

	/* forward overlap: dst > src */
	wmemmove_run(0, 1, 1, 0, "fwd-n1");
	wmemmove_run(0, 1, 2, 0, "fwd-n2");
	wmemmove_run(0, 1, 4, 0, "fwd-n4");
	wmemmove_run(0, 2, 8, 1, "fwd-n8");
	wmemmove_run(1, 3, 6, 2, "fwd-mid");
	wmemmove_run(0, 1, 16, 0, "fwd-n16");

	/* backward overlap: dst < src */
	wmemmove_run(1, 0, 1, 0, "bwd-n1");
	wmemmove_run(2, 0, 2, 0, "bwd-n2");
	wmemmove_run(4, 0, 4, 1, "bwd-n4");
	wmemmove_run(8, 0, 8, 2, "bwd-n8");
	wmemmove_run(3, 1, 6, 0, "bwd-mid");
	wmemmove_run(16, 0, 16, 1, "bwd-n16");

	/* exact overlap: src_off == dst_off */
	wmemmove_run(0, 0, 8, 0, "same-n8");
	wmemmove_run(4, 4, 12, 1, "same-n12");

	/* high-bit wchar patterns */
	for (int v = 0x80; v <= 0xff; v += 0x11) {
		wchar_t tmp[WBUF];

		std::memset(tmp, GUARD, sizeof(tmp));
		for (std::size_t i = 0; i < 8; i++)
			tmp[i] = (wchar_t)(v + i);
		std::memcpy(tmp + WBUF / 2, tmp, 8 * sizeof(wchar_t));
		wmemmove_run(0, WBUF / 2, 8, 3, "hi-byte");
		(void)tmp;
		wmemmove_run(0, 2, 6, 1, "hi-fwd");
		wmemmove_run(6, 0, 6, 2, "hi-bwd");
	}

	/* NUL-heavy content (not string semantics, but exercises bytes) */
	wmemmove_run(0, 4, 8, 0, "nul-heavy-fwd");
	wmemmove_run(8, 0, 8, 0, "nul-heavy-bwd");

	/* boundary lengths */
	static const std::size_t bounds[] = {
		0, 1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64
	};
	for (std::size_t bi = 0; bi < sizeof(bounds) / sizeof(bounds[0]);
	    bi++) {
		std::size_t n = bounds[bi];
		if (n + 8 > WBUF)
			continue;
		wmemmove_run(0, 8, n, 0, "bound-disjoint");
		wmemmove_run(0, 1, n, 1, "bound-fwd");
		wmemmove_run(n, 0, n, 2, "bound-bwd");
	}

	/* n * sizeof(wchar_t): all-0xFFFF runs */
	for (std::size_t n = 1; n <= 12; n++) {
		wmemmove_run(0, n, n, 2, "ffff-fwd");
		wmemmove_run(n, 0, n, 2, "ffff-bwd");
	}
}

static void
wmemmove_random(long iters)
{
	for (long it = 0; it < iters; it++) {
		std::size_t n, src_off, dst_off;
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
			n = rnd_mod(64);
			break;
		}

		if (n > WBUF - 1)
			n = WBUF - 1;

		std::size_t span = WBUF - n;
		src_off = rnd_mod(span);
		dst_off = rnd_mod(span);

		/* bias toward overlapping regions */
		if (rnd_mod(3) == 0) {
			if (rnd_mod(2) == 0)
				dst_off = src_off + rnd_mod(n > 0 ? n : 1);
			else if (src_off >= 1)
				dst_off = src_off - rnd_mod(src_off);
		}

		if (dst_off + n > WBUF)
			dst_off = WBUF - n;
		if (src_off + n > WBUF)
			src_off = WBUF - n;

		wmemmove_run(src_off, dst_off, n, pat, "rand");
	}
}

/* ------------------------------------------------------------------------ */
/* wcscpy                                                                    */
/* ------------------------------------------------------------------------ */

static const std::size_t WCPY_DST = 96;
static const std::size_t WCPY_SRC = 64;

static std::size_t
wlen(const wchar_t *s)
{
	std::size_t n = 0;
	while (s[n] != L'\0')
		n++;
	return n;
}

static void
wcscpy_run(const wchar_t *src, std::size_t slen, const char *tag)
{
	wchar_t da[WCPY_DST], db[WCPY_DST];
	wchar_t sa[WCPY_SRC], sb[WCPY_SRC];

	if (slen + 1 > WCPY_SRC) {
		std::fprintf(stderr,
		    "harness bug: %s would overflow src (slen=%zu)\n", tag,
		    slen);
		std::exit(2);
	}

	std::memset(da, GUARD, sizeof(da));
	std::memset(db, GUARD, sizeof(db));
	std::memset(sa, GUARD, sizeof(sa));
	std::memset(sb, GUARD, sizeof(sb));
	std::memcpy(sa, src, (slen + 1) * sizeof(wchar_t));
	std::memcpy(sb, src, (slen + 1) * sizeof(wchar_t));

	wchar_t *ra = port::wcscpy(da, sa);
	wchar_t *rb = ref_wcscpy(db, sb);

	long off_a = wptr_off(da, ra);
	long off_b = wptr_off(db, rb);

	int bad_ret = (off_a != off_b) || (off_a != 0);
	int bad_d = (std::memcmp(da, db, sizeof(da)) != 0);
	int bad_s = (std::memcmp(sa, sb, sizeof(sa)) != 0);

	st_wcscpy.cases++;
	if (bad_ret || bad_d || bad_s) {
		st_wcscpy.fails++;
		if (st_wcscpy.reported < MAX_REPORT) {
			st_wcscpy.reported++;
			std::printf("  FAIL wcscpy [%s] slen=%zu%s%s%s\n", tag,
			    slen, bad_ret ? " ret" : "", bad_d ? " dst" : "",
			    bad_s ? " src" : "");
			std::printf("      port_off=%ld ref_off=%ld expect=0\n",
			    off_a, off_b);
			dump_wide("port_dst", da, WCPY_DST);
			dump_wide("ref_dst ", db, WCPY_DST);
			if (bad_s) {
				dump_wide("port_src", sa, WCPY_SRC);
				dump_wide("ref_src ", sb, WCPY_SRC);
			}
		}
	}
}

static const wchar_t *const wc_srcs[] = {
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
	L"\x7fffffff",
	L"\x10ffff",
};

static void
wcscpy_edge(void)
{
	std::size_t ns = sizeof(wc_srcs) / sizeof(wc_srcs[0]);

	for (std::size_t i = 0; i < ns; i++)
		wcscpy_run(wc_srcs[i], wlen(wc_srcs[i]), "grid");

	/* every low-byte value as single-character string */
	{
		wchar_t s[2];

		s[1] = L'\0';
		for (int v = 0; v < 256; v++) {
			s[0] = (wchar_t)v;
			wcscpy_run(s, wlen(s), "byte-val");
		}
	}

	/* lengths 0..24 exercise loop entry, body, and exit */
	{
		wchar_t s[28];

		for (std::size_t len = 0; len <= 24; len++) {
			for (std::size_t i = 0; i < len; i++)
				s[i] = (wchar_t)(L'a' + (wchar_t)(i % 26));
			s[len] = L'\0';
			wcscpy_run(s, len, "len-scan");
		}
	}

	/* NUL-heavy source: only prefix before first NUL is copied */
	{
		wchar_t s[] = { L'x', L'\0', L'y', L'z', L'\0' };
		wcscpy_run(s, 1, "early-nul");
	}

	/* multiple leading NULs */
	{
		wchar_t s[] = { L'\0', L'\0', L'a', L'\0' };
		wcscpy_run(s, 0, "lead-nul");
	}

	/* high-bit wchar sequences */
	{
		wchar_t s[8];

		for (int i = 0; i < 6; i++)
			s[i] = (wchar_t)(0x8000 + i * 0x111);
		s[6] = L'\0';
		wcscpy_run(s, 6, "hi-seq");
	}

	/* boundary lengths right up against the destination buffer */
	{
		wchar_t s[WCPY_DST];

		for (std::size_t len = 0; len + 1 <= WCPY_SRC; len++) {
			for (std::size_t i = 0; i < len; i++)
				s[i] = (wchar_t)(0x100 + i);
			s[len] = L'\0';
			wcscpy_run(s, len, "exact-fill");
		}
	}
}

static void
wcscpy_random(long iters)
{
	wchar_t src[WCPY_SRC];
	std::size_t na = sizeof(w_alpha) / sizeof(w_alpha[0]);

	for (long it = 0; it < iters; it++) {
		std::size_t slen;
		int wide = (rnd_mod(5) == 0);

		if (rnd_mod(20) == 0)
			slen = 0;
		else if (rnd_mod(10) == 0)
			slen = 1;
		else
			slen = rnd_mod(28);

		for (std::size_t i = 0; i < slen; i++) {
			if (wide)
				src[i] = (wchar_t)(rnd_mod(0x10000));
			else
				src[i] = w_alpha[rnd_mod(na - 1)];
		}
		src[slen] = L'\0';

		if (slen > 2 && rnd_mod(7) == 0)
			src[rnd_mod(slen)] = L'\0';

		wcscpy_run(src, wlen(src), "rand");
	}
}

/* ------------------------------------------------------------------------ */
/* wmemset                                                                   */
/* ------------------------------------------------------------------------ */

static const std::size_t WSET_BUF = 96;

static void
wmemset_run(std::size_t off, wchar_t c, std::size_t n, const char *tag)
{
	wchar_t a[WSET_BUF], b[WSET_BUF];

	if (off + n > WSET_BUF) {
		std::fprintf(stderr,
		    "harness bug: %s would overflow (off=%zu n=%zu)\n", tag,
		    off, n);
		std::exit(2);
	}

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));

	wchar_t *ra = port::wmemset(a + off, c, n);
	wchar_t *rb = ref_wmemset(b + off, c, n);

	long off_a = wptr_off(a, ra);
	long off_b = wptr_off(b, rb);
	long expect = (long)off;

	int bad_ret = (off_a != off_b) || (off_a != expect);
	int bad_buf = (std::memcmp(a, b, sizeof(a)) != 0);

	st_wmemset.cases++;
	if (bad_ret || bad_buf) {
		st_wmemset.fails++;
		if (st_wmemset.reported < MAX_REPORT) {
			st_wmemset.reported++;
			std::printf(
			    "  FAIL wmemset [%s] off=%zu c=%08lx n=%zu%s%s\n",
			    tag, off, (unsigned long)(std::uint32_t)c, n,
			    bad_ret ? " ret" : "", bad_buf ? " buf" : "");
			std::printf(
			    "      port_off=%ld ref_off=%ld expect=%ld\n",
			    off_a, off_b, expect);
			dump_wide("port", a, WSET_BUF);
			dump_wide("ref ", b, WSET_BUF);
		}
	}
}

static void
wmemset_edge(void)
{
	/* n == 0 */
	wmemset_run(0, L'x', 0, "n0");
	wmemset_run(8, (wchar_t)0x80, 0, "n0-off");

	/* single wchar */
	wmemset_run(0, L'a', 1, "n1");
	wmemset_run(4, (wchar_t)0xff, 1, "n1-off");

	/* c == L'\0' */
	wmemset_run(0, L'\0', 8, "c-nul");
	wmemset_run(2, L'\0', 4, "c-nul-off");

	/* high-bit fill values */
	for (int v = 0x80; v <= 0xff; v += 0x11) {
		wmemset_run(0, (wchar_t)v, 4, "hi-c");
		wmemset_run(1, (wchar_t)(v | 0xff00), 6, "hi-c2");
	}

	static const wchar_t c_vals[] = {
		(wchar_t)0x0000, (wchar_t)0x0001, (wchar_t)0x007f,
		(wchar_t)0x0080, (wchar_t)0x00ff, (wchar_t)0x0100,
		(wchar_t)0x1234, (wchar_t)0x7fff, (wchar_t)0x8000,
		(wchar_t)0xffff, (wchar_t)0x10ffff,
	};
	for (std::size_t ci = 0; ci < sizeof(c_vals) / sizeof(c_vals[0]);
	    ci++) {
		wmemset_run(0, c_vals[ci], 3, "c-grid");
		wmemset_run(5, c_vals[ci], 7, "c-grid-off");
	}

	/* boundary lengths */
	static const std::size_t bounds[] = {
		0, 1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 63, 64
	};
	for (std::size_t bi = 0; bi < sizeof(bounds) / sizeof(bounds[0]);
	    bi++) {
		std::size_t n = bounds[bi];
		if (n + 4 > WSET_BUF)
			continue;
		wmemset_run(0, (wchar_t)0x4141, n, "bound");
		wmemset_run(4, (wchar_t)0x8080, n, "bound-off");
	}

	/* exact-fill to end of buffer */
	for (std::size_t n = 0; n <= WSET_BUF; n++)
		wmemset_run(0, (wchar_t)(0x100 + n), n, "exact-fill");
}

static void
wmemset_random(long iters)
{
	for (long it = 0; it < iters; it++) {
		std::size_t n, off;
		wchar_t c;

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
			n = rnd_mod(WSET_BUF);
			break;
		}

		if (n > WSET_BUF)
			n = WSET_BUF;
		off = rnd_mod(WSET_BUF - n + 1);

		if (rnd_mod(5) == 0)
			c = (wchar_t)(rnd_mod(0x10000));
		else
			c = w_alpha[rnd_mod(sizeof(w_alpha) / sizeof(w_alpha[0]) -
			    1)];

		wmemset_run(off, c, n, "rand");
	}
}

/* ------------------------------------------------------------------------ */

int
main(void)
{
	rng_state = 0x0121a121ULL;
	wmemmove_edge();
	wmemmove_random(200000);

	rng_state = 0x0121b121ULL;
	wcscpy_edge();
	wcscpy_random(200000);

	rng_state = 0x0121c121ULL;
	wmemset_edge();
	wmemset_random(200000);

	std::printf("\n%-12s %12s %12s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-12s %12s %12s   %s\n", "------------", "------------",
	    "------------", "------");

	const Stat *all[] = { &st_wmemmove, &st_wcscpy, &st_wmemset };
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
