// Differential test harness for PBSD batch b0313.
//
// Every ported function is exercised against the unmodified reference
// implementation in oracle.c.

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <locale.h>

import pbsd.lib.libc.string.b0313;

namespace P = pbsd::lib_libc_string::b0313;

extern "C" {
char *ref_strcasestr(const char *s, const char *find);
char *ref_strcasestr_l(const char *s, const char *find, locale_t locale);
int ref_wcswidth(const wchar_t *pwcs, size_t n);
int ref_wcswidth_l(const wchar_t *pwcs, size_t n, locale_t locale);
size_t ref_wcslcat(wchar_t *dst, const wchar_t *src, size_t siz);
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

static Stat st_strcasestr = { "strcasestr", 0, 0, 0 };
static Stat st_strcasestr_l = { "strcasestr_l", 0, 0, 0 };
static Stat st_wcswidth = { "wcswidth", 0, 0, 0 };
static Stat st_wcswidth_l = { "wcswidth_l", 0, 0, 0 };
static Stat st_wcslcat = { "wcslcat", 0, 0, 0 };

static const int MAX_SHOW = 8;
static const unsigned long long RANDOM_ITERS = 50000ull;

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

static void
dump_bytes(const char *label, const unsigned char *p, size_t n)
{
	std::printf("    %s[%zu] =", label, n);
	for (size_t i = 0; i < n; i++)
		std::printf(" %02x", (unsigned)p[i]);
	std::printf("\n");
}

static void
dump_wide(const char *label, const wchar_t *p, size_t n)
{
	std::printf("    %s[%zu] =", label, n);
	for (size_t i = 0; i < n; i++)
		std::printf(" %08lx", (unsigned long)(std::uint32_t)p[i]);
	std::printf("\n");
}

static long
first_diff(const void *a, const void *b, size_t n)
{
	const unsigned char *pa = (const unsigned char *)a;
	const unsigned char *pb = (const unsigned char *)b;

	for (size_t i = 0; i < n; i++)
		if (pa[i] != pb[i])
			return (long)i;
	return -1;
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
	bool pct(std::uint32_t p) { return below(100) < p; }
};

/* ------------------------------------------------------------------ */
/* shared buffers                                                     */
/* ------------------------------------------------------------------ */

static const unsigned char GUARD = 0x7f;

static const size_t SBUF = 128;
static const size_t MAXS = 48;
static const size_t WDCAP = 96;
static const size_t WSCAP = 64;
static const size_t MAXW = 40;

static void
fill_str(char *buf, size_t cap, const unsigned char *body, size_t len)
{
	std::memset(buf, GUARD, cap);
	for (size_t i = 0; i < len; i++)
		buf[i] = (char)body[i];
	buf[len] = '\0';
}

static void
fill_wdst(wchar_t *buf, size_t cap, const wchar_t *body, size_t len)
{
	std::memset(buf, GUARD, cap * sizeof(wchar_t));
	for (size_t i = 0; i < len; i++)
		buf[i] = body[i];
	buf[len] = L'\0';
}

static void
fill_wsrc(wchar_t *buf, size_t cap, const wchar_t *body, size_t len)
{
	std::memset(buf, GUARD, cap * sizeof(wchar_t));
	for (size_t i = 0; i < len; i++)
		buf[i] = body[i];
	buf[len] = L'\0';
}

static long
ptr_off(const char *base, const char *p)
{
	if (p == nullptr)
		return -1;
	return (long)(p - base);
}

/* ------------------------------------------------------------------ */
/* strcasestr / strcasestr_l                                          */
/* ------------------------------------------------------------------ */

static void
do_strcasestr(Stat &st, const unsigned char *hay, size_t hlen,
    const unsigned char *needle, size_t nlen, locale_t loc, bool use_l)
{
	char ha[SBUF], hb[SBUF], na[SBUF], nb[SBUF];

	fill_str(ha, SBUF, hay, hlen);
	fill_str(hb, SBUF, hay, hlen);
	fill_str(na, SBUF, needle, nlen);
	fill_str(nb, SBUF, needle, nlen);

	char *rp;
	char *ro;
	if (use_l) {
		rp = P::strcasestr_l(ha, na, loc);
		ro = ref_strcasestr_l(hb, nb, loc);
	} else {
		rp = P::strcasestr(ha, na);
		ro = ref_strcasestr(hb, nb);
	}

	st.cases++;
	long op = ptr_off(ha, rp);
	long oo = ptr_off(hb, ro);
	if (op != oo || std::memcmp(ha, hb, SBUF) != 0 ||
	    std::memcmp(na, nb, SBUF) != 0) {
		if (begin_fail(st, "mismatch")) {
			dump_bytes("hay", hay, hlen);
			dump_bytes("needle", needle, nlen);
			std::printf("    port_off=%ld ref_off=%ld hay_diff@%ld "
			    "needle_diff@%ld\n", op, oo,
			    first_diff(ha, hb, SBUF),
			    first_diff(na, nb, SBUF));
		}
	}
}

static void
check_strcasestr(const unsigned char *hay, size_t hlen,
    const unsigned char *needle, size_t nlen)
{
	do_strcasestr(st_strcasestr, hay, hlen, needle, nlen,
	    (locale_t)0, false);
}

static void
check_strcasestr_l(const unsigned char *hay, size_t hlen,
    const unsigned char *needle, size_t nlen, locale_t loc)
{
	do_strcasestr(st_strcasestr_l, hay, hlen, needle, nlen, loc, true);
}

#define BYTES(lit) (const unsigned char *)(lit), (sizeof(lit) - 1)

static void
strcasestr_edge_cases(void)
{
	locale_t loc = newlocale(LC_ALL_MASK, "C.UTF-8", (locale_t)0);
	if (loc == (locale_t)0)
		loc = newlocale(LC_ALL_MASK, "C", (locale_t)0);

	/* empty needle returns haystack start */
	check_strcasestr(BYTES("abc"), BYTES(""));
	check_strcasestr_l(BYTES("abc"), BYTES(""), loc);
	check_strcasestr_l(BYTES("abc"), BYTES(""), (locale_t)0);

	/* empty haystack, non-empty needle */
	check_strcasestr(BYTES(""), BYTES("a"));
	check_strcasestr(BYTES(""), BYTES("ab"));

	/* both empty */
	check_strcasestr(BYTES(""), BYTES(""));

	/* single char: found, not found, case fold */
	check_strcasestr(BYTES("abc"), BYTES("a"));
	check_strcasestr(BYTES("abc"), BYTES("A"));
	check_strcasestr(BYTES("abc"), BYTES("b"));
	check_strcasestr(BYTES("abc"), BYTES("d"));
	check_strcasestr(BYTES("Abc"), BYTES("a"));

	/* exact and partial matches */
	check_strcasestr(BYTES("foobar"), BYTES("bar"));
	check_strcasestr(BYTES("foobar"), BYTES("BAR"));
	check_strcasestr(BYTES("foobar"), BYTES("baz"));
	check_strcasestr(BYTES("foobar"), BYTES("foobar"));
	check_strcasestr(BYTES("foobar"), BYTES("FOOBAR"));

	/* repeated first-char scan */
	check_strcasestr(BYTES("aaaaab"), BYTES("b"));
	check_strcasestr(BYTES("aaaAab"), BYTES("ab"));
	check_strcasestr(BYTES("aaaAab"), BYTES("AB"));

	/* needle longer than haystack */
	check_strcasestr(BYTES("ab"), BYTES("abc"));
	check_strcasestr(BYTES("a"), BYTES("aa"));

	/* high-bit bytes */
	check_strcasestr(BYTES("\x80\x81\x82"), BYTES("\x80"));
	check_strcasestr(BYTES("\xff\xfe\xfd"), BYTES("\xff"));
	check_strcasestr(BYTES("a\x80""b"), BYTES("\x80"));
	check_strcasestr(BYTES("\x7f\x80"), BYTES("\x80"));

	/* embedded NUL: only prefix visible */
	check_strcasestr(BYTES("ab\0cd"), BYTES("cd"));
	check_strcasestr(BYTES("ab\0cd"), BYTES("ab"));

	/* case folding on high bytes */
	check_strcasestr(BYTES("ABCdef"), BYTES("def"));
	check_strcasestr(BYTES("abcDEF"), BYTES("DEF"));

	check_strcasestr_l(BYTES("FoObAr"), BYTES("oBa"), loc);
	check_strcasestr_l(BYTES("xyz"), BYTES("w"), loc);

	freelocale(loc);
}

static unsigned char
gen_byte(Rng &r, int mode)
{
	switch (mode) {
	case 0:
		return (unsigned char)(1 + r.below(255));
	case 1:
		return (unsigned char)('a' + r.below(26));
	case 2:
		return (unsigned char)(0x80 + r.below(0x80));
	default:
		return (unsigned char)(0x3f + r.below(0x41));
	}
}

static void
strcasestr_random_sweep(Rng &rng)
{
	unsigned char hay[MAXS + 1], needle[MAXS + 1];
	locale_t loc = newlocale(LC_ALL_MASK, "C.UTF-8", (locale_t)0);
	if (loc == (locale_t)0)
		loc = newlocale(LC_ALL_MASK, "C", (locale_t)0);

	for (unsigned long long it = 0; it < RANDOM_ITERS; it++) {
		int mode = (int)rng.below(4);
		size_t hlen = rng.below((std::uint32_t)MAXS + 1);
		size_t nlen = rng.below((std::uint32_t)MAXS + 1);

		for (size_t i = 0; i < hlen; i++)
			hay[i] = gen_byte(rng, mode);
		for (size_t i = 0; i < nlen; i++) {
			if (rng.pct(60) && hlen > 0)
				needle[i] = hay[rng.below((std::uint32_t)hlen)];
			else
				needle[i] = gen_byte(rng, mode);
		}

		check_strcasestr(hay, hlen, needle, nlen);
		if ((it & 1) == 0)
			check_strcasestr_l(hay, hlen, needle, nlen, loc);
		else
			check_strcasestr_l(hay, hlen, needle, nlen,
			    (locale_t)0);
	}

	freelocale(loc);
}

/* ------------------------------------------------------------------ */
/* wcswidth / wcswidth_l                                              */
/* ------------------------------------------------------------------ */

static void
do_wcswidth(Stat &st, const wchar_t *body, size_t wlen, size_t n,
    locale_t loc, bool use_l)
{
	wchar_t wa[WSCAP], wb[WSCAP];

	fill_wsrc(wa, WSCAP, body, wlen);
	fill_wsrc(wb, WSCAP, body, wlen);

	int rp, ro;
	if (use_l) {
		rp = P::wcswidth_l(wa, n, loc);
		ro = ref_wcswidth_l(wb, n, loc);
	} else {
		rp = P::wcswidth(wa, n);
		ro = ref_wcswidth(wb, n);
	}

	st.cases++;
	if (rp != ro || std::memcmp(wa, wb, sizeof wa) != 0) {
		if (begin_fail(st, "mismatch")) {
			dump_wide("wcs", body, wlen);
			std::printf("    n=%zu port=%d ref=%d buf_diff@%ld\n",
			    n, rp, ro, first_diff(wa, wb, sizeof wa));
		}
	}
}

static void
check_wcswidth(const wchar_t *body, size_t wlen, size_t n)
{
	do_wcswidth(st_wcswidth, body, wlen, n, (locale_t)0, false);
}

static void
check_wcswidth_l(const wchar_t *body, size_t wlen, size_t n, locale_t loc)
{
	do_wcswidth(st_wcswidth_l, body, wlen, n, loc, true);
}

static void
wcswidth_edge_cases(void)
{
	wchar_t buf[MAXW + 4];
	locale_t loc = newlocale(LC_ALL_MASK, "C.UTF-8", (locale_t)0);
	if (loc == (locale_t)0)
		loc = newlocale(LC_ALL_MASK, "C", (locale_t)0);

	/* empty string */
	buf[0] = L'\0';
	check_wcswidth(buf, 0, 0);
	check_wcswidth(buf, 0, 1);
	check_wcswidth(buf, 0, 10);
	check_wcswidth_l(buf, 0, 0, loc);
	check_wcswidth_l(buf, 0, 5, (locale_t)0);

	/* n == 0 with non-empty string */
	buf[0] = L'a';
	buf[1] = L'\0';
	check_wcswidth(buf, 1, 0);

	/* single printable wide chars */
	static const wchar_t singles[] = {
		L'a', L'A', L'0', L' ', L'\t', 0x7f, 0x80, 0xff,
		0x100, 0x7e7e, 0xfffd
	};
	for (size_t i = 0; i < sizeof singles / sizeof singles[0]; i++) {
		buf[0] = singles[i];
		buf[1] = L'\0';
		check_wcswidth(buf, 1, 1);
		check_wcswidth(buf, 1, 2);
		check_wcswidth_l(buf, 1, 1, loc);
	}

	/* short runs */
	for (size_t n = 1; n <= 8; n++) {
		for (size_t i = 0; i < n; i++)
			buf[i] = (wchar_t)(L'a' + (i % 26));
		check_wcswidth(buf, n, n);
		check_wcswidth(buf, n, n - 1);
		check_wcswidth(buf, n, n + 1);
		check_wcswidth(buf, n, n + 3);
	}

	/* n stops before NUL */
	buf[0] = L'a';
	buf[1] = L'b';
	buf[2] = L'c';
	buf[3] = L'\0';
	check_wcswidth(buf, 3, 1);
	check_wcswidth(buf, 3, 2);

	/* invalid wide chars: wcwidth returns -1 */
	static const wchar_t invalid[] = {
		(wchar_t)0xd800, (wchar_t)0xdfff, (wchar_t)0xffffffff
	};
	for (size_t i = 0; i < sizeof invalid / sizeof invalid[0]; i++) {
		buf[0] = invalid[i];
		buf[1] = L'\0';
		check_wcswidth(buf, 1, 1);
		check_wcswidth(buf, 1, 5);
		check_wcswidth_l(buf, 1, 1, loc);
	}

	/* invalid char after valid chars */
	buf[0] = L'a';
	buf[1] = (wchar_t)0xd800;
	buf[2] = L'\0';
	check_wcswidth(buf, 2, 2);
	check_wcswidth(buf, 2, 1);

	freelocale(loc);
}

static void
wcswidth_random_sweep(Rng &rng)
{
	wchar_t buf[MAXW + 1];
	locale_t loc = newlocale(LC_ALL_MASK, "C.UTF-8", (locale_t)0);
	if (loc == (locale_t)0)
		loc = newlocale(LC_ALL_MASK, "C", (locale_t)0);

	for (unsigned long long it = 0; it < RANDOM_ITERS; it++) {
		size_t wlen = rng.below((std::uint32_t)MAXW + 1);
		for (size_t i = 0; i < wlen; i++) {
			switch (rng.below(5)) {
			case 0:
				buf[i] = (wchar_t)(1 + rng.below(255));
				break;
			case 1:
				buf[i] = (wchar_t)(L'a' + rng.below(26));
				break;
			case 2:
				buf[i] = (wchar_t)(0x80 + rng.below(0x80));
				break;
			case 3:
				buf[i] = (wchar_t)(0xd800 + rng.below(0x800));
				break;
			default:
				buf[i] = (wchar_t)(1 + rng.below(0x10ffff));
				break;
			}
		}

		size_t n;
		switch (rng.below(6)) {
		case 0:
			n = 0;
			break;
		case 1:
			n = wlen;
			break;
		case 2:
			n = wlen + 1;
			break;
		case 3:
			n = (wlen == 0) ? 0 : wlen - 1;
			break;
		default:
			n = rng.below((std::uint32_t)MAXW + 5);
			break;
		}

		check_wcswidth(buf, wlen, n);
		if ((it & 1) == 0)
			check_wcswidth_l(buf, wlen, n, loc);
		else
			check_wcswidth_l(buf, wlen, n, (locale_t)0);
	}

	freelocale(loc);
}

/* ------------------------------------------------------------------ */
/* wcslcat                                                            */
/* ------------------------------------------------------------------ */

static void
t_wcslcat(const wchar_t *dbody, size_t dlen, const wchar_t *sbody,
    size_t slen, size_t siz)
{
	wchar_t da[WDCAP], db[WDCAP], sa[WSCAP], sb[WSCAP];

	fill_wdst(da, WDCAP, dbody, dlen);
	fill_wdst(db, WDCAP, dbody, dlen);
	fill_wsrc(sa, WSCAP, sbody, slen);
	fill_wsrc(sb, WSCAP, sbody, slen);

	size_t rp = P::wcslcat(da, sa, siz);
	size_t ro = ref_wcslcat(db, sb, siz);

	st_wcslcat.cases++;
	if (rp != ro || std::memcmp(da, db, sizeof da) != 0 ||
	    std::memcmp(sa, sb, sizeof sa) != 0) {
		if (begin_fail(st_wcslcat, "mismatch")) {
			dump_wide("dst", dbody, dlen);
			dump_wide("src", sbody, slen);
			std::printf("    siz=%zu port=%zu ref=%zu dst_diff@%ld "
			    "src_diff@%ld\n", siz, rp, ro,
			    first_diff(da, db, sizeof da),
			    first_diff(sa, sb, sizeof sa));
		}
	}
}

static void
t_wcslcat_sizes(const wchar_t *dbody, size_t dlen, const wchar_t *sbody,
    size_t slen)
{
	t_wcslcat(dbody, dlen, sbody, slen, 0);
	t_wcslcat(dbody, dlen, sbody, slen, 1);
	t_wcslcat(dbody, dlen, sbody, slen, 2);
	if (dlen + slen >= 1)
		t_wcslcat(dbody, dlen, sbody, slen, dlen + slen - 1);
	t_wcslcat(dbody, dlen, sbody, slen, dlen + slen);
	t_wcslcat(dbody, dlen, sbody, slen, dlen + slen + 1);
	t_wcslcat(dbody, dlen, sbody, slen, dlen + slen + 2);
	t_wcslcat(dbody, dlen, sbody, slen, MAXW + 8);
}

static void
wcslcat_edge_cases(void)
{
	wchar_t dst[MAXW + 1];
	wchar_t src[MAXW + 1];

	/* empty dst, empty src */
	dst[0] = L'\0';
	src[0] = L'\0';
	t_wcslcat_sizes(dst, 0, src, 0);

	/* empty dst, non-empty src */
	src[0] = L'a';
	src[1] = L'\0';
	t_wcslcat_sizes(dst, 0, src, 1);

	src[0] = L'x';
	src[1] = L'y';
	src[2] = L'z';
	src[3] = L'\0';
	t_wcslcat_sizes(dst, 0, src, 3);

	/* non-empty dst, empty src */
	dst[0] = L'a';
	dst[1] = L'\0';
	src[0] = L'\0';
	t_wcslcat_sizes(dst, 1, src, 0);

	/* append to short dst */
	dst[0] = L'h';
	dst[1] = L'i';
	dst[2] = L'\0';
	src[0] = L'!';
	src[1] = L'\0';
	t_wcslcat_sizes(dst, 2, src, 1);

	/* truncation boundary */
	dst[0] = L'a';
	dst[1] = L'b';
	dst[2] = L'\0';
	src[0] = L'c';
	src[1] = L'd';
	src[2] = L'e';
	src[3] = L'\0';
	for (size_t siz = 0; siz <= 8; siz++)
		t_wcslcat(dst, 2, src, 3, siz);

	/* dst fills entire buffer before append (n == 0 path) */
	for (size_t dlen = 1; dlen <= 6; dlen++) {
		for (size_t i = 0; i < dlen; i++)
			dst[i] = (wchar_t)(L'0' + i);
		for (size_t sl = 1; sl <= 4; sl++) {
			for (size_t i = 0; i < sl; i++)
				src[i] = (wchar_t)(L'A' + i);
			t_wcslcat(dst, dlen, src, sl, dlen);
			t_wcslcat(dst, dlen, src, sl, dlen + 1);
		}
	}

	/* high code points and guard-like values */
	static const wchar_t singles[] = {
		1, L'a', 0x7f, 0x80, 0xff, 0x100, 0xfffd, 0xffff,
		0x10ffff, 0x7f7f7f7f
	};
	for (size_t i = 0; i < sizeof singles / sizeof singles[0]; i++) {
		dst[0] = singles[i];
		dst[1] = L'\0';
		src[0] = L'z';
		src[1] = L'\0';
		t_wcslcat_sizes(dst, 1, src, 1);
	}

	/* long src append to empty dst */
	for (size_t i = 0; i < MAXW; i++)
		src[i] = (wchar_t)(1 + i * 7);
	for (size_t siz = 0; siz <= MAXW + 4; siz++) {
		dst[0] = L'\0';
		t_wcslcat(dst, 0, src, MAXW, siz);
	}
}

static void
wcslcat_random_sweep(Rng &rng)
{
	wchar_t dst[MAXW + 1], src[MAXW + 1];

	for (unsigned long long it = 0; it < RANDOM_ITERS; it++) {
		size_t dlen = rng.below((std::uint32_t)MAXW / 2 + 1);
		size_t slen = rng.below((std::uint32_t)MAXW + 1);

		for (size_t i = 0; i < dlen; i++) {
			switch (rng.below(4)) {
			case 0:
				dst[i] = (wchar_t)(L'a' + rng.below(26));
				break;
			case 1:
				dst[i] = (wchar_t)(0x80 + rng.below(0x80));
				break;
			case 2:
				dst[i] = (wchar_t)0x7f7f7f7f;
				break;
			default:
				dst[i] = (wchar_t)(1 + rng.below(0x10ffff));
				break;
			}
		}

		for (size_t i = 0; i < slen; i++) {
			if (rng.pct(30) && dlen > 0)
				src[i] = dst[rng.below((std::uint32_t)dlen)];
			else {
				switch (rng.below(4)) {
				case 0:
					src[i] = (wchar_t)(1 + rng.below(255));
					break;
				case 1:
					src[i] = (wchar_t)(0x80 +
					    rng.below(0x80));
					break;
				case 2:
					src[i] = (wchar_t)0x7f7f7f7f;
					break;
				default:
					src[i] = (wchar_t)(1 +
					    rng.below(0x10ffff));
					break;
				}
			}
		}

		size_t siz;
		switch (rng.below(7)) {
		case 0:
			siz = 0;
			break;
		case 1:
			siz = 1;
			break;
		case 2:
			siz = dlen;
			break;
		case 3:
			siz = dlen + slen;
			break;
		case 4:
			siz = dlen + slen + 1;
			break;
		case 5:
			siz = (dlen + slen == 0) ? 0 : dlen + slen - 1;
			break;
		default:
			siz = rng.below((std::uint32_t)MAXW + 8);
			break;
		}

		t_wcslcat(dst, dlen, src, slen, siz);
	}
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int
main()
{
	std::printf("b0313 differential test (wchar_t is %zu bytes)\n",
	    sizeof(wchar_t));

	Rng rng_css(0xB0313Aull);
	Rng rng_wsw(0xB0313Bull);
	Rng rng_wcl(0xB0313Cull);

	strcasestr_edge_cases();
	wcswidth_edge_cases();
	wcslcat_edge_cases();

	strcasestr_random_sweep(rng_css);
	wcswidth_random_sweep(rng_wsw);
	wcslcat_random_sweep(rng_wcl);

	Stat stats[] = {
		st_strcasestr,
		st_strcasestr_l,
		st_wcswidth,
		st_wcswidth_l,
		st_wcslcat,
	};
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
