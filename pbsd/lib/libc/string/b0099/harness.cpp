/*
 * harness.cpp -- differential test for PBSD batch b0099.
 *
 * Every case is executed twice: once against the C++ port (module
 * pbsd.lib.libc.string.b0099) and once against the unmodified C reference in
 * oracle.c.  Return values, pointer *offsets* (never raw addresses) and the
 * complete contents of every buffer -- including the guard bytes past the
 * nominal write window -- are compared after each call.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.string.b0099;

namespace port = pbsd::lib_libc_string::b0099;

extern "C" wchar_t *ref_wcsncat(wchar_t *__restrict s1,
    const wchar_t *__restrict s2, std::size_t n);
extern "C" char *ref_strpbrk(const char *s1, const char *s2);

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

static Stat st_wcsncat = { "wcsncat", 0, 0, 0 };
static Stat st_strpbrk = { "strpbrk", 0, 0, 0 };

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
/* wcsncat                                                                   */
/* ------------------------------------------------------------------------ */

static const std::size_t WDST = 96;	/* elements */
static const std::size_t WSRC = 64;	/* elements */

/*
 * dst/src are NUL-terminated wide strings of length dlen/slen.  Both
 * destination buffers are filled with the guard pattern, then the identical
 * prefix (content + terminator) is copied into each; everything after that
 * must be untouched by a correct implementation except for the bytes the
 * algorithm legitimately writes.
 */
static void
wc_run(const wchar_t *dst, std::size_t dlen, const wchar_t *src,
    std::size_t slen, std::size_t n, const char *tag)
{
	wchar_t a[WDST], b[WDST];
	wchar_t sa[WSRC], sb[WSRC];
	std::size_t eff = (n < slen) ? n : slen;

	if (dlen + eff + 1 > WDST || slen + 1 > WSRC) {
		std::fprintf(stderr,
		    "harness bug: %s would overflow (dlen=%zu slen=%zu n=%zu)\n",
		    tag, dlen, slen, n);
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

	wchar_t *ra = port::wcsncat(a, sa, n);
	wchar_t *rb = ref_wcsncat(b, sb, n);

	long oa = (long)(ra - a);
	long ob = (long)(rb - b);

	int bad_ret = (oa != ob);
	int bad_dst = (std::memcmp(a, b, sizeof(a)) != 0);
	int bad_src = (std::memcmp(sa, sb, sizeof(sa)) != 0);

	st_wcsncat.cases++;
	if (bad_ret || bad_dst || bad_src) {
		st_wcsncat.fails++;
		if (st_wcsncat.reported < MAX_REPORT) {
			st_wcsncat.reported++;
			std::printf("  FAIL wcsncat [%s] dlen=%zu slen=%zu "
			    "n=%zu%s%s%s\n", tag, dlen, slen, n,
			    bad_ret ? " ret" : "", bad_dst ? " dst" : "",
			    bad_src ? " src" : "");
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

static const std::size_t wc_ns[] = {
	0, 1, 2, 3, 4, 7, 8, 9, 10, 15, 32, (std::size_t)-1,
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
	std::size_t nn = sizeof(wc_ns) / sizeof(wc_ns[0]);

	for (std::size_t i = 0; i < nd; i++) {
		std::size_t dlen = wlen(wc_dsts[i]);
		for (std::size_t j = 0; j < ns; j++) {
			std::size_t slen = wlen(wc_srcs[j]);

			for (std::size_t k = 0; k < nn; k++)
				wc_run(wc_dsts[i], dlen, wc_srcs[j], slen,
				    wc_ns[k], "grid");

			/* both sides of the n == slen boundary */
			if (slen > 0)
				wc_run(wc_dsts[i], dlen, wc_srcs[j], slen,
				    slen - 1, "n=slen-1");
			wc_run(wc_dsts[i], dlen, wc_srcs[j], slen, slen,
			    "n=slen");
			wc_run(wc_dsts[i], dlen, wc_srcs[j], slen, slen + 1,
			    "n=slen+1");
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

			for (std::size_t k = 0; k <= slen + 2; k++) {
				std::size_t eff = (k < slen) ? k : slen;
				std::size_t dlen = WDST - 1 - eff;

				for (std::size_t i = 0; i < dlen; i++)
					big[i] = (wchar_t)('A' + (i % 26));
				big[dlen] = L'\0';
				wc_run(big, dlen, src, slen, k, "exact-fill");

				if (dlen >= 1) {
					std::size_t d2 = dlen - 1;
					for (std::size_t i = 0; i < d2; i++)
						big[i] = (wchar_t)('a' +
						    (i % 26));
					big[d2] = L'\0';
					wc_run(big, d2, src, slen, k,
					    "near-fill");
				}
			}
		}
	}
}

static const wchar_t wc_alpha[] = {
	(wchar_t)0x00000001, (wchar_t)0x00000020, (wchar_t)0x00000041,
	(wchar_t)0x0000007f, (wchar_t)0x00000080, (wchar_t)0x000000ff,
	(wchar_t)0x00000100, (wchar_t)0x000007ff, (wchar_t)0x0000ffff,
	(wchar_t)0x0010ffff, (wchar_t)0x7ffffffe, (wchar_t)0x00000061,
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

		std::size_t n;
		switch ((int)rnd_mod(10)) {
		case 0: case 1: case 2: case 3:
			n = rnd_mod(16);
			break;
		case 4: case 5:
			n = slen;
			break;
		case 6:
			n = slen ? slen - 1 : 0;
			break;
		case 7:
			n = slen + 1;
			break;
		case 8:
			n = 0;
			break;
		default:
			n = (std::size_t)-1;
			break;
		}

		std::size_t eff = (n < slen) ? n : slen;
		std::size_t dlen;
		std::size_t mode = rnd_mod(10);

		if (mode < 7)
			dlen = rnd_mod(13);
		else if (mode < 9)
			dlen = 55 + rnd_mod(21);
		else
			dlen = WDST - 1 - eff;

		if (dlen + eff + 1 > WDST)
			dlen = WDST - 1 - eff;

		for (std::size_t i = 0; i < dlen; i++)
			dst[i] = wc_alpha[rnd_mod(na)];
		dst[dlen] = L'\0';

		wc_run(dst, dlen, src, slen, n, "rand");
	}
}

/* ------------------------------------------------------------------------ */
/* strpbrk                                                                   */
/* ------------------------------------------------------------------------ */

static const std::size_t SBUF = 80;

static void
sp_run(const char *s1, std::size_t l1, const char *s2, std::size_t l2,
    const char *tag)
{
	char a1[SBUF], a2[SBUF], b1[SBUF], b2[SBUF];

	if (l1 + 1 > SBUF || l2 + 1 > SBUF) {
		std::fprintf(stderr,
		    "harness bug: %s would overflow (l1=%zu l2=%zu)\n",
		    tag, l1, l2);
		std::exit(2);
	}

	std::memset(a1, GUARD, sizeof(a1));
	std::memset(a2, GUARD, sizeof(a2));
	std::memset(b1, GUARD, sizeof(b1));
	std::memset(b2, GUARD, sizeof(b2));
	std::memcpy(a1, s1, l1 + 1);
	std::memcpy(b1, s1, l1 + 1);
	std::memcpy(a2, s2, l2 + 1);
	std::memcpy(b2, s2, l2 + 1);

	char *ra = port::strpbrk(a1, a2);
	char *rb = ref_strpbrk(b1, b2);

	long oa = (ra == nullptr) ? -1L : (long)(ra - a1);
	long ob = (rb == nullptr) ? -1L : (long)(rb - b1);

	int bad_ret = (oa != ob);
	int bad_1 = (std::memcmp(a1, b1, sizeof(a1)) != 0);
	int bad_2 = (std::memcmp(a2, b2, sizeof(a2)) != 0);

	st_strpbrk.cases++;
	if (bad_ret || bad_1 || bad_2) {
		st_strpbrk.fails++;
		if (st_strpbrk.reported < MAX_REPORT) {
			st_strpbrk.reported++;
			std::printf("  FAIL strpbrk [%s] l1=%zu l2=%zu%s%s%s\n",
			    tag, l1, l2, bad_ret ? " ret" : "",
			    bad_1 ? " s1buf" : "", bad_2 ? " s2buf" : "");
			std::printf("      port_off=%ld ref_off=%ld\n", oa, ob);
			dump_bytes("s1", a1, l1 + 1);
			dump_bytes("s2", a2, l2 + 1);
		}
	}
}

static const char *const sp_s1[] = {
	"",
	"a",
	"b",
	"z",
	"ab",
	"ba",
	"abc",
	"cba",
	"abcdefghij",
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
};

static const char *const sp_s2[] = {
	"",
	"a",
	"b",
	"c",
	"z",
	"q",
	"Z",
	"\x80",
	"\xff",
	"\x7f",
	"\x01",
	"ac",
	"ca",
	"\x80\xff",
	"\xff\x80",
	"abcdefghij",
	"XYZ",
	"\xfb\xfc",
	"MNOP",
	"\x7f\x01",
};

static void
sp_edge(void)
{
	std::size_t n1 = sizeof(sp_s1) / sizeof(sp_s1[0]);
	std::size_t n2 = sizeof(sp_s2) / sizeof(sp_s2[0]);

	for (std::size_t i = 0; i < n1; i++)
		for (std::size_t j = 0; j < n2; j++)
			sp_run(sp_s1[i], std::strlen(sp_s1[i]), sp_s2[j],
			    std::strlen(sp_s2[j]), "grid");

	/*
	 * Every match position in a string that ends flush against the buffer
	 * guard, plus the corresponding no-match run of the same length.
	 */
	{
		char s1[SBUF];
		char s2[2];

		s2[1] = '\0';
		for (std::size_t len = 1; len + 1 <= SBUF; len++) {
			for (std::size_t i = 0; i < len; i++)
				s1[i] = 'a';
			s1[len] = '\0';

			for (std::size_t pos = 0; pos < len; pos++) {
				s1[pos] = 'Q';
				s2[0] = 'Q';
				sp_run(s1, len, s2, 1, "hit");
				s2[0] = 'R';
				sp_run(s1, len, s2, 1, "miss");
				s1[pos] = 'a';
			}
		}
	}

	/* every single byte value on both sides, matching and not */
	{
		char s1[2], s2[2];

		s1[1] = '\0';
		s2[1] = '\0';
		for (int v = 1; v < 256; v++) {
			s1[0] = (char)v;
			s2[0] = (char)v;
			sp_run(s1, 1, s2, 1, "byte-hit");
			s2[0] = (char)(v == 1 ? 2 : 1);
			sp_run(s1, 1, s2, 1, "byte-miss");
			sp_run(s1, 1, "", 0, "byte-empty-set");
			sp_run("", 0, s1, 1, "empty-str");
		}
	}
}

static const unsigned char sp_alpha[] = {
	'a', 'b', 0x80, 0xff, 0x7f, 0x01, 'z', 0xc3
};

static void
sp_random(long iters)
{
	char s1[SBUF], s2[SBUF];
	std::size_t na = sizeof(sp_alpha) / sizeof(sp_alpha[0]);

	for (long it = 0; it < iters; it++) {
		std::size_t l1, l2;
		int wide = (rnd_mod(4) == 0);

		if (rnd_mod(20) == 0)
			l1 = 40 + rnd_mod(39);	/* up to SBUF - 1 */
		else
			l1 = rnd_mod(14);
		l2 = rnd_mod(8);

		for (std::size_t i = 0; i < l1; i++)
			s1[i] = wide ? (char)(1 + rnd_mod(255))
			    : (char)sp_alpha[rnd_mod(na)];
		s1[l1] = '\0';
		for (std::size_t i = 0; i < l2; i++)
			s2[i] = wide ? (char)(1 + rnd_mod(255))
			    : (char)sp_alpha[rnd_mod(na)];
		s2[l2] = '\0';

		sp_run(s1, l1, s2, l2, "rand");
	}
}

/* ------------------------------------------------------------------------ */

int
main(void)
{
	rng_state = 0x0099b0099ULL;
	wc_edge();
	wc_random(220000);

	rng_state = 0xb0099ULL;
	sp_edge();
	sp_random(220000);

	std::printf("\n%-12s %12s %12s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-12s %12s %12s   %s\n", "------------", "------------",
	    "------------", "------");

	const Stat *all[] = { &st_wcsncat, &st_strpbrk };
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
