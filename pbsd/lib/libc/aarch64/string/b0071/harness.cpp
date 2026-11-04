/*
 * Differential test harness for PBSD batch b0071.
 *
 * Compares strlcat and strpbrk in pbsd.lib.libc.aarch64.string.b0071 against
 * ref_strlcat and ref_strpbrk in oracle.c.
 */

import pbsd.lib.libc.aarch64.string.b0071;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::lib_libc_aarch64_string::b0071;

extern "C" {
std::size_t ref_strlcat(char *__restrict, const char *__restrict, std::size_t);
char *ref_strpbrk(const char *, const char *);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr int MAX_REPORT = 8;
constexpr long RANDOM_ITERS = 200000;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

Stat st_strlcat{"strlcat", 0, 0, 0};
Stat st_strpbrk{"strpbrk", 0, 0, 0};

std::uint64_t rng_state;

std::uint64_t
rnd()
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

std::size_t
rnd_mod(std::size_t m)
{
	return static_cast<std::size_t>(rnd() % static_cast<std::uint64_t>(m));
}

void
dump_bytes(const char *label, const unsigned char *p, std::size_t n)
{
	std::printf("      %s:", label);
	for (std::size_t i = 0; i < n; i++)
		std::printf(" %02x", static_cast<unsigned>(p[i]));
	std::printf("\n");
}

/* ------------------------------------------------------------------------ */
/* strlcat                                                                   */
/* ------------------------------------------------------------------------ */

constexpr std::size_t DST_BUF = 96;
constexpr std::size_t SRC_BUF = 64;

/*
 * Copy dlen bytes of dst prefix (may omit a terminator) and a NUL-terminated
 * src into guarded buffers, then compare return values and the entire dst
 * buffer after the call.
 */
void
sl_run(const unsigned char *dst, std::size_t dlen, std::size_t dstsize,
    const char *src, std::size_t slen, const char *tag)
{
	unsigned char a[DST_BUF], b[DST_BUF];
	char sa[SRC_BUF], sb[SRC_BUF];

	if (dlen > DST_BUF || slen + 1 > SRC_BUF) {
		std::fprintf(stderr,
		    "harness bug: %s would overflow (dlen=%zu slen=%zu)\n",
		    tag, dlen, slen);
		std::exit(2);
	}

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	std::memset(sa, GUARD, sizeof(sa));
	std::memset(sb, GUARD, sizeof(sb));
	std::memcpy(a, dst, dlen);
	std::memcpy(b, dst, dlen);
	std::memcpy(sa, src, slen + 1);
	std::memcpy(sb, src, slen + 1);

	std::size_t ra = port::strlcat(reinterpret_cast<char *>(a), sa, dstsize);
	std::size_t rb = ref_strlcat(reinterpret_cast<char *>(b), sb, dstsize);

	int bad_ret = (ra != rb);
	int bad_dst = (std::memcmp(a, b, sizeof(a)) != 0);
	int bad_src = (std::memcmp(sa, sb, sizeof(sa)) != 0);

	st_strlcat.cases++;
	if (bad_ret || bad_dst || bad_src) {
		st_strlcat.fails++;
		if (st_strlcat.reported < MAX_REPORT) {
			st_strlcat.reported++;
			std::printf("  FAIL strlcat [%s] dlen=%zu dstsize=%zu "
			    "slen=%zu%s%s%s\n",
			    tag, dlen, dstsize, slen, bad_ret ? " ret" : "",
			    bad_dst ? " dst" : "", bad_src ? " src" : "");
			std::printf("      port_ret=%zu ref_ret=%zu\n", ra, rb);
			dump_bytes("port_dst", a, DST_BUF);
			dump_bytes("ref_dst ", b, DST_BUF);
			if (bad_src) {
				dump_bytes("port_src",
				    reinterpret_cast<const unsigned char *>(sa),
				    SRC_BUF);
				dump_bytes("ref_src ",
				    reinterpret_cast<const unsigned char *>(sb),
				    SRC_BUF);
			}
		}
	}
}

void
sl_run_cstr(const char *dst, std::size_t dstsize, const char *src,
    const char *tag)
{
	std::size_t dlen = std::strlen(dst);
	std::size_t slen = std::strlen(src);

	sl_run(reinterpret_cast<const unsigned char *>(dst), dlen + 1, dstsize,
	    src, slen, tag);
}

void
sl_run_raw(const unsigned char *dst, std::size_t dlen, std::size_t dstsize,
    const char *src, const char *tag)
{
	sl_run(dst, dlen, dstsize, src, std::strlen(src), tag);
}

const char *const sl_dsts[] = {
	"",
	"a",
	"ab",
	"abc",
	"abcdefgh",
	"\x80",
	"\xff",
	"\x7f",
	"a\x80" "b",
	"\x80\xff\x7f",
	"0123456789",
};

const char *const sl_srcs[] = {
	"",
	"x",
	"xy",
	"xyz",
	"abcdefghijklmnop",
	"\x80",
	"\xff",
	"\x7f\x01",
	"\x80\xff",
	"qrstuvw",
};

const std::size_t sl_sizes[] = {
	0, 1, 2, 3, 4, 5, 8, 16, 32, 64, DST_BUF,
};

void
sl_edge()
{
	std::size_t nd = sizeof(sl_dsts) / sizeof(sl_dsts[0]);
	std::size_t ns = sizeof(sl_srcs) / sizeof(sl_srcs[0]);
	std::size_t nz = sizeof(sl_sizes) / sizeof(sl_sizes[0]);

	for (std::size_t i = 0; i < nd; i++) {
		std::size_t dlen = std::strlen(sl_dsts[i]);
		for (std::size_t j = 0; j < ns; j++) {
			for (std::size_t k = 0; k < nz; k++) {
				std::size_t dstsize = sl_sizes[k];

				sl_run_cstr(sl_dsts[i], dstsize, sl_srcs[j],
				    "grid");
				if (dstsize > 0 && dstsize <= dlen + 1)
					sl_run_cstr(sl_dsts[i], dstsize - 1,
					    sl_srcs[j], "size-1");
				sl_run_cstr(sl_dsts[i], dstsize + 1,
				    sl_srcs[j], "size+1");
			}
		}
	}

	/* dstsize == 0: loc == NULL branch, dst untouched */
	sl_run_cstr("", 0, "", "dstsize0-empty");
	sl_run_cstr("abc", 0, "def", "dstsize0-nz");
	sl_run_cstr("abc", 0, "\xff\x80", "dstsize0-hibit");

	/* unterminated dst: no NUL in first dstsize bytes */
	{
		unsigned char raw[DST_BUF];

		for (std::size_t n = 1; n <= 32; n++) {
			for (std::size_t i = 0; i < n; i++)
				raw[i] = static_cast<unsigned char>('a' +
				    (i % 26));
			sl_run_raw(raw, n, n, "", "unterm-empty-src");
			sl_run_raw(raw, n, n, "z", "unterm-src");
			sl_run_raw(raw, n, n, "longsource", "unterm-long-src");
			if (n > 1) {
				raw[n - 1] = 0x80;
				sl_run_raw(raw, n, n, "q", "unterm-hibit");
			}
		}
	}

	/* NUL-heavy and high-bit bytes throughout dst/src */
	{
		unsigned char dst[DST_BUF];
		char src[SRC_BUF];

		for (int v = 0; v < 256; v++) {
			dst[0] = static_cast<unsigned char>(v);
			dst[1] = '\0';
			src[0] = static_cast<char>(v);
			src[1] = '\0';
			sl_run_raw(dst, 2, 4, src, "byte-dst");
			sl_run_raw(dst, 1, 1, src, "byte-dstsize1");
		}
	}

	/* exact-fill: dst consumes all of dstsize without a terminator */
	{
		unsigned char dst[DST_BUF];

		for (std::size_t n = 1; n <= 24; n++) {
			for (std::size_t i = 0; i < n; i++)
				dst[i] = static_cast<unsigned char>(0x80 +
				    (i & 0x3f));
			for (std::size_t dstsize = n; dstsize <= n + 2;
			    dstsize++) {
				sl_run_raw(dst, n, dstsize, "tail", "fill");
				sl_run_raw(dst, n, dstsize, "", "fill-empty");
			}
		}
	}

	/* boundary around first NUL position */
	{
		char buf[32];

		for (std::size_t pos = 0; pos < 16; pos++) {
			for (std::size_t i = 0; i < pos; i++)
				buf[i] = static_cast<char>('A' + i);
			buf[pos] = '\0';
			sl_run_cstr(buf, pos, "x", "nul-at-pos");
			sl_run_cstr(buf, pos + 1, "x", "nul-after-pos");
			if (pos > 0)
				sl_run_cstr(buf, pos - 1, "x", "nul-before-pos");
		}
	}
}

const unsigned char sl_alpha[] = {
	'a', 'b', 'c', 'z', 0x01, 0x7f, 0x80, 0xff,
};

void
sl_random(long iters)
{
	unsigned char dst[DST_BUF];
	char src[SRC_BUF];
	std::size_t na = sizeof(sl_alpha) / sizeof(sl_alpha[0]);

	for (long it = 0; it < iters; it++) {
		std::size_t slen = rnd_mod(20);
		for (std::size_t i = 0; i < slen; i++)
			src[i] = static_cast<char>(sl_alpha[rnd_mod(na)]);
		src[slen] = '\0';

		std::size_t dstsize = rnd_mod(DST_BUF);
		int unterm = (rnd_mod(7) == 0);

		std::size_t dlen;
		if (unterm) {
			dlen = dstsize == 0 ? 0 : dstsize;
			for (std::size_t i = 0; i < dlen; i++)
				dst[i] = static_cast<unsigned char>(
				    1 + rnd_mod(255));
		} else {
			dlen = rnd_mod(24);
			for (std::size_t i = 0; i < dlen; i++)
				dst[i] = sl_alpha[rnd_mod(na)];
			dst[dlen] = '\0';
			dlen++;
		}

		sl_run(dst, dlen, dstsize, src, slen, "rand");
	}
}

/* ------------------------------------------------------------------------ */
/* strpbrk                                                                   */
/* ------------------------------------------------------------------------ */

constexpr std::size_t SBUF = 80;

void
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

	long oa = (ra == nullptr) ? -1L : static_cast<long>(ra - a1);
	long ob = (rb == nullptr) ? -1L : static_cast<long>(rb - b1);

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
			dump_bytes("s1",
			    reinterpret_cast<const unsigned char *>(a1),
			    l1 + 1);
			dump_bytes("s2",
			    reinterpret_cast<const unsigned char *>(a2),
			    l2 + 1);
		}
	}
}

const char *const sp_s1[] = {
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
	"\x80\x80\x80\x80",
	"\xfe\xfd\xfc\xfb",
	"MNOP",
};

const char *const sp_s2[] = {
	"",
	"a",
	"b",
	"c",
	"z",
	"q",
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

void
sp_edge()
{
	std::size_t n1 = sizeof(sp_s1) / sizeof(sp_s1[0]);
	std::size_t n2 = sizeof(sp_s2) / sizeof(sp_s2[0]);

	for (std::size_t i = 0; i < n1; i++)
		for (std::size_t j = 0; j < n2; j++)
			sp_run(sp_s1[i], std::strlen(sp_s1[i]), sp_s2[j],
			    std::strlen(sp_s2[j]), "grid");

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

	{
		char s1[2], s2[2];

		s1[1] = '\0';
		s2[1] = '\0';
		for (int v = 1; v < 256; v++) {
			s1[0] = static_cast<char>(v);
			s2[0] = static_cast<char>(v);
			sp_run(s1, 1, s2, 1, "byte-hit");
			s2[0] = static_cast<char>(v == 1 ? 2 : 1);
			sp_run(s1, 1, s2, 1, "byte-miss");
			sp_run(s1, 1, "", 0, "byte-empty-set");
			sp_run("", 0, s1, 1, "empty-str");
		}
	}
}

const unsigned char sp_alpha[] = {
	'a', 'b', 0x80, 0xff, 0x7f, 0x01, 'z', 0xc3,
};

void
sp_random(long iters)
{
	char s1[SBUF], s2[SBUF];
	std::size_t na = sizeof(sp_alpha) / sizeof(sp_alpha[0]);

	for (long it = 0; it < iters; it++) {
		std::size_t l1, l2;
		int wide = (rnd_mod(4) == 0);

		if (rnd_mod(20) == 0)
			l1 = 40 + rnd_mod(39);
		else
			l1 = rnd_mod(14);
		l2 = rnd_mod(8);

		for (std::size_t i = 0; i < l1; i++)
			s1[i] = wide ? static_cast<char>(1 + rnd_mod(255))
				     : static_cast<char>(sp_alpha[rnd_mod(na)]);
		s1[l1] = '\0';
		for (std::size_t i = 0; i < l2; i++)
			s2[i] = wide ? static_cast<char>(1 + rnd_mod(255))
				     : static_cast<char>(sp_alpha[rnd_mod(na)]);
		s2[l2] = '\0';

		sp_run(s1, l1, s2, l2, "rand");
	}
}

void
print_table()
{
	const Stat *all[] = {&st_strlcat, &st_strpbrk};

	std::printf("\n%-12s %12s %12s   %s\n", "function", "cases",
	    "failures", "result");
	std::printf("%-12s %12s %12s   %s\n", "------------", "------------",
	    "------------", "------");

	long total_fail = 0;
	long total_case = 0;

	for (const Stat *s : all) {
		std::printf("%-12s %12ld %12ld   %s\n", s->name, s->cases,
		    s->fails, s->fails == 0 ? "PASS" : "FAIL");
		total_fail += s->fails;
		total_case += s->cases;
	}
	std::printf("%-12s %12s %12s   %s\n", "------------", "------------",
	    "------------", "------");
	std::printf("%-12s %12ld %12ld   %s\n", "TOTAL", total_case,
	    total_fail, total_fail == 0 ? "PASS" : "FAIL");
}

} /* namespace */

int
main()
{
	rng_state = 0xB0071ULL;
	sl_edge();
	sl_random(RANDOM_ITERS);

	rng_state = 0x71B0071ULL;
	sp_edge();
	sp_random(RANDOM_ITERS);

	print_table();
	return (st_strlcat.fails == 0 && st_strpbrk.fails == 0) ? 0 : 1;
}
