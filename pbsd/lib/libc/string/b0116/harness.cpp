/*
 * harness.cpp -- differential test for PBSD batch b0116.
 *
 * Every case is executed twice: once against the C++ port (module
 * pbsd.lib.libc.string.b0116) and once against the unmodified C reference in
 * oracle.c.  Return values, pointer *offsets* (never raw addresses) and the
 * complete contents of every buffer -- including the guard bytes past the
 * nominal write window -- are compared after each call.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.string.b0116;

namespace port = pbsd::lib_libc_string::b0116;

extern "C" char *ref_strdup(const char *str);
extern "C" wchar_t *ref_wcscat(wchar_t *__restrict s1,
    const wchar_t *__restrict s2);

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

static Stat st_strdup = { "strdup", 0, 0, 0 };
static Stat st_wcscat = { "wcscat", 0, 0, 0 };

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

static void
dump_wide(const char *label, const wchar_t *p, std::size_t n)
{
	std::printf("      %s:", label);
	for (std::size_t i = 0; i < n; i++)
		std::printf(" %08lx", (unsigned long)(std::uint32_t)p[i]);
	std::printf("\n");
}

static void
dump_bytes(const char *label, const char *p, std::size_t n)
{
	std::printf("      %s:", label);
	for (std::size_t i = 0; i < n; i++)
		std::printf(" %02x", (unsigned)(unsigned char)p[i]);
	std::printf("\n");
}

/* ------------------------------------------------------------------------ */
/* strdup                                                                    */
/* ------------------------------------------------------------------------ */

static void
sd_run(const char *str, std::size_t slen, const char *tag)
{
	char *pa = port::strdup(str);
	char *pb = ref_strdup(str);

	int bad_null = 0;
	int bad_data = 0;

	if (pa == nullptr || pb == nullptr) {
		if (pa != pb)
			bad_null = 1;
	} else {
		if (std::memcmp(pa, pb, slen + 1) != 0)
			bad_data = 1;
	}

	st_strdup.cases++;
	if (bad_null || bad_data) {
		st_strdup.fails++;
		if (st_strdup.reported < MAX_REPORT) {
			st_strdup.reported++;
			std::printf("  FAIL strdup [%s] slen=%zu%s%s\n", tag, slen,
			    bad_null ? " null" : "",
			    bad_data ? " data" : "");
			if (pa != nullptr && pb != nullptr) {
				dump_bytes("port", pa, slen + 1);
				dump_bytes("ref ", pb, slen + 1);
			} else {
				std::printf("      port=%p ref=%p\n",
				    (void *)pa, (void *)pb);
			}
		}
	}

	std::free(pa);
	std::free(pb);
}

static const char *const sd_strs[] = {
	"",
	"a",
	"b",
	"z",
	"ab",
	"abc",
	"abcdefg",
	"\x80",
	"\xff",
	"\x7f",
	"\x01",
	"\x80\xff\x7f",
	"a\x80" "b",
	"\xff" "a\xff",
	"aaaaaaaaaaq",
	"qaaaaaaaaaa",
	"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaZ",
	"Zaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
	"\x80\x80\x80\x80",
	"\xfe\xfd\xfc\xfb",
	"MNOP",
	"\x00",		/* embedded NUL still copied by strlen+1 semantics */
};

static void
sd_edge(void)
{
	std::size_t n = sizeof(sd_strs) / sizeof(sd_strs[0]);

	for (std::size_t i = 0; i < n; i++)
		sd_run(sd_strs[i], std::strlen(sd_strs[i]), "grid");

	/* every single byte value as a one-character string */
	{
		char s[2];

		s[1] = '\0';
		for (int v = 0; v < 256; v++) {
			s[0] = (char)v;
			sd_run(s, std::strlen(s), "byte");
		}
	}

	/* bytes past the first NUL are not part of the strdup input */
	{
		char buf[4] = { 'a', '\0', 'b', '\0' };
		sd_run(buf, std::strlen(buf), "past-nul");
	}

	/* boundary lengths up against a fixed buffer */
	{
		char buf[80];

		for (std::size_t len = 0; len < sizeof(buf); len++) {
			for (std::size_t i = 0; i < len; i++)
				buf[i] = (char)(0x80 + (i % 0x7f));
			buf[len] = '\0';
			sd_run(buf, len, "long");
		}
	}
}

static const unsigned char sd_alpha[] = {
	'a', 'b', 0x80, 0xff, 0x7f, 0x01, 'z', 0xc3, '\0'
};

static void
sd_random(long iters)
{
	char s[80];
	std::size_t na = sizeof(sd_alpha) / sizeof(sd_alpha[0]);

	for (long it = 0; it < iters; it++) {
		std::size_t slen;
		int wide = (rnd_mod(4) == 0);

		if (rnd_mod(20) == 0)
			slen = 40 + rnd_mod(39);
		else
			slen = rnd_mod(16);

		for (std::size_t i = 0; i < slen; i++) {
			if (wide)
				s[i] = (char)(rnd_mod(256));
			else
				s[i] = (char)sd_alpha[rnd_mod(na)];
		}
		s[slen] = '\0';

		/* sprinkle interior NULs that strlen still skips */
		if (slen > 2 && rnd_mod(7) == 0)
			s[rnd_mod(slen)] = '\0';

		sd_run(s, std::strlen(s), "rand");
	}
}

/* ------------------------------------------------------------------------ */
/* wcscat                                                                    */
/* ------------------------------------------------------------------------ */

static const std::size_t WDST = 96;
static const std::size_t WSRC = 64;

static void
wc_run(const wchar_t *dst, std::size_t dlen, const wchar_t *src,
    std::size_t slen, const char *tag)
{
	wchar_t a[WDST], b[WDST];
	wchar_t sa[WSRC], sb[WSRC];

	if (dlen + slen + 1 > WDST || slen + 1 > WSRC) {
		std::fprintf(stderr,
		    "harness bug: %s would overflow (dlen=%zu slen=%zu)\n",
		    tag, dlen, slen);
		std::exit(2);
	}

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	std::memset(sa, GUARD, sizeof(sa));
	std::memset(sb, GUARD, sizeof(sb));
	std::memcpy(a, dst, (dlen + 1) * sizeof(wchar_t));
	std::memcpy(b, dst, (dlen + 1) * sizeof(wchar_t));
	std::memcpy(sa, src, (slen + 1) * sizeof(wchar_t));
	std::memcpy(sb, src, (slen + 1) * sizeof(wchar_t));

	wchar_t *ra = port::wcscat(a, sa);
	wchar_t *rb = ref_wcscat(b, sb);

	long oa = (long)(ra - a);
	long ob = (long)(rb - b);

	int bad_ret = (oa != ob);
	int bad_dst = (std::memcmp(a, b, sizeof(a)) != 0);
	int bad_src = (std::memcmp(sa, sb, sizeof(sa)) != 0);

	st_wcscat.cases++;
	if (bad_ret || bad_dst || bad_src) {
		st_wcscat.fails++;
		if (st_wcscat.reported < MAX_REPORT) {
			st_wcscat.reported++;
			std::printf("  FAIL wcscat [%s] dlen=%zu slen=%zu%s%s%s\n",
			    tag, dlen, slen, bad_ret ? " ret" : "",
			    bad_dst ? " dst" : "", bad_src ? " src" : "");
			std::printf("      port_off=%ld ref_off=%ld\n", oa, ob);
			dump_wide("port_dst", a, WDST);
			dump_wide("ref_dst ", b, WDST);
			if (bad_src) {
				dump_wide("port_src", sa, WSRC);
				dump_wide("ref_src ", sb, WSRC);
			}
		}
	}
}

static const wchar_t *const wc_dsts[] = {
	L"",
	L"a",
	L"ab",
	L"abc",
	L"abcdefg",
	L"\x80",
	L"\xff",
	L"\x7f",
	L"\x0100\x7fffffff",
	L"\x10ffff",
	L"\x0001",
	L"xyzw\xff\x0100",
};

static const wchar_t *const wc_srcs[] = {
	L"",
	L"x",
	L"xy",
	L"xyz",
	L"\x80\xff",
	L"\x7fffffff",
	L"pqrstuvw",
	L"\x0001\x0002\x0003",
	L"\x10ffff\x10fffe",
	L"MNOPQRSTU",
};

static std::size_t
wlen(const wchar_t *s)
{
	std::size_t n = 0;
	while (s[n] != L'\0')
		n++;
	return n;
}

static void
wc_edge(void)
{
	std::size_t nd = sizeof(wc_dsts) / sizeof(wc_dsts[0]);
	std::size_t ns = sizeof(wc_srcs) / sizeof(wc_srcs[0]);

	for (std::size_t i = 0; i < nd; i++) {
		std::size_t dlen = wlen(wc_dsts[i]);
		for (std::size_t j = 0; j < ns; j++) {
			std::size_t slen = wlen(wc_srcs[j]);
			wc_run(wc_dsts[i], dlen, wc_srcs[j], slen, "grid");
		}
	}

	/*
	 * Destination lengths that push the write window right up against the
	 * end of the buffer, so that a one-past write lands outside it.
	 */
	{
		wchar_t big[WDST];
		wchar_t src[WSRC];

		for (std::size_t slen = 0; slen <= 12; slen++) {
			for (std::size_t i = 0; i < slen; i++)
				src[i] = (wchar_t)(0x100 + i);
			src[slen] = L'\0';

			for (std::size_t dlen = 0; dlen + slen + 1 <= WDST;
			    dlen++) {
				for (std::size_t i = 0; i < dlen; i++)
					big[i] = (wchar_t)('A' + (i % 26));
				big[dlen] = L'\0';
				wc_run(big, dlen, src, slen, "exact-fill");

				if (dlen >= 1) {
					std::size_t d2 = dlen - 1;
					for (std::size_t i = 0; i < d2; i++)
						big[i] = (wchar_t)('a' +
						    (i % 26));
					big[d2] = L'\0';
					wc_run(big, d2, src, slen, "near-fill");
				}
			}
		}
	}

	/* NUL-heavy destination: scan loop must walk every zero */
	{
		wchar_t dst[WDST];
		wchar_t src[4] = { (wchar_t)0x1234, L'\0' };

		for (std::size_t zlen = 0; zlen < 20; zlen++) {
			for (std::size_t i = 0; i < zlen; i++)
				dst[i] = L'\0';
			dst[zlen] = L'\0';
			wc_run(dst, zlen, src, 1, "nul-dst");
		}
	}
}

static const wchar_t wc_alpha[] = {
	(wchar_t)0x00000001, (wchar_t)0x00000020, (wchar_t)0x00000041,
	(wchar_t)0x0000007f, (wchar_t)0x00000080, (wchar_t)0x000000ff,
	(wchar_t)0x00000100, (wchar_t)0x000007ff, (wchar_t)0x0000ffff,
	(wchar_t)0x0010ffff, (wchar_t)0x7ffffffe, (wchar_t)0x00000061,
	L'\0',
};

static void
wc_random(long iters)
{
	wchar_t dst[WDST], src[WSRC];
	std::size_t na = sizeof(wc_alpha) / sizeof(wc_alpha[0]);

	for (long it = 0; it < iters; it++) {
		std::size_t slen = rnd_mod(13);
		for (std::size_t i = 0; i < slen; i++)
			src[i] = wc_alpha[rnd_mod(na)];
		src[slen] = L'\0';

		std::size_t dlen;
		std::size_t mode = rnd_mod(10);

		if (mode < 7)
			dlen = rnd_mod(13);
		else if (mode < 9)
			dlen = 55 + rnd_mod(21);
		else
			dlen = WDST - 1 - slen;

		if (dlen + slen + 1 > WDST)
			dlen = WDST - 1 - slen;

		for (std::size_t i = 0; i < dlen; i++)
			dst[i] = wc_alpha[rnd_mod(na)];
		dst[dlen] = L'\0';

		/* interior NULs in destination exercise the first while */
		if (dlen > 2 && rnd_mod(8) == 0)
			dst[rnd_mod(dlen)] = L'\0';

		wc_run(dst, wlen(dst), src, slen, "rand");
	}
}

/* ------------------------------------------------------------------------ */

int
main(void)
{
	rng_state = 0x0116b0116ULL;
	sd_edge();
	sd_random(110000);

	rng_state = 0xb0116ULL;
	wc_edge();
	wc_random(110000);

	std::printf("\n%-12s %12s %12s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-12s %12s %12s   %s\n", "------------", "------------",
	    "------------", "------");

	const Stat *all[] = { &st_strdup, &st_wcscat };
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
