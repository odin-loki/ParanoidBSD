/*
 * Differential test harness for PBSD batch b0081.
 *
 * Compares strcpy, strnlen, strpbrk and strncpy in
 * pbsd.lib.libc.amd64.string.b0081 against the ref_ oracle in oracle.c.
 */

import pbsd.lib.libc.amd64.string.b0081;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::lib_libc_amd64_string::b0081;

extern "C" {
char *ref_strcpy(char *__restrict, const char *__restrict);
std::size_t ref_strnlen(const char *, std::size_t);
char *ref_strpbrk(const char *, const char *);
char *ref_strncpy(char *__restrict, const char *__restrict, std::size_t);
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

Stat st_strcpy{"strcpy", 0, 0, 0};
Stat st_strnlen{"strnlen", 0, 0, 0};
Stat st_strpbrk{"strpbrk", 0, 0, 0};
Stat st_strncpy{"strncpy", 0, 0, 0};

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
/* strcpy                                                                    */
/* ------------------------------------------------------------------------ */

constexpr std::size_t SC_DST = 96;
constexpr std::size_t SC_SRC = 64;

void
sc_run(const char *src, std::size_t slen, const char *tag)
{
	unsigned char a[SC_DST], b[SC_DST];
	char sa[SC_SRC], sb[SC_SRC];

	if (slen + 1 > SC_SRC) {
		std::fprintf(stderr,
		    "harness bug: %s would overflow src (slen=%zu)\n", tag,
		    slen);
		std::exit(2);
	}
	if (slen + 1 > SC_DST) {
		std::fprintf(stderr,
		    "harness bug: %s would overflow dst (slen=%zu)\n", tag,
		    slen);
		std::exit(2);
	}

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	std::memset(sa, GUARD, sizeof(sa));
	std::memset(sb, GUARD, sizeof(sb));
	std::memcpy(sa, src, slen + 1);
	std::memcpy(sb, src, slen + 1);

	char *ra = port::strcpy(reinterpret_cast<char *>(a), sa);
	char *rb = ref_strcpy(reinterpret_cast<char *>(b), sb);

	long oa = static_cast<long>(ra - reinterpret_cast<char *>(a));
	long ob = static_cast<long>(rb - reinterpret_cast<char *>(b));

	int bad_ret = (oa != ob);
	int bad_dst = (std::memcmp(a, b, sizeof(a)) != 0);
	int bad_src = (std::memcmp(sa, sb, sizeof(sa)) != 0);

	st_strcpy.cases++;
	if (bad_ret || bad_dst || bad_src) {
		st_strcpy.fails++;
		if (st_strcpy.reported < MAX_REPORT) {
			st_strcpy.reported++;
			std::printf("  FAIL strcpy [%s] slen=%zu%s%s%s\n", tag,
			    slen, bad_ret ? " ret" : "", bad_dst ? " dst" : "",
			    bad_src ? " src" : "");
			std::printf("      port_off=%ld ref_off=%ld\n", oa, ob);
			dump_bytes("port_dst", a, SC_DST);
			dump_bytes("ref_dst ", b, SC_DST);
			if (bad_src) {
				dump_bytes("port_src",
				    reinterpret_cast<const unsigned char *>(sa),
				    SC_SRC);
				dump_bytes("ref_src ",
				    reinterpret_cast<const unsigned char *>(sb),
				    SC_SRC);
			}
		}
	}
}

const char *const sc_srcs[] = {
	"",
	"a",
	"b",
	"z",
	"ab",
	"abc",
	"abcdefgh",
	"abcdefghijklmnop",
	"\x80",
	"\xff",
	"\x7f",
	"\x01",
	"a\x80" "b",
	"\x80\xff\x7f",
	"\xff" "a\xff",
	"0123456789",
	"MNOPQRSTUVWXYZ",
};

void
sc_edge()
{
	std::size_t ns = sizeof(sc_srcs) / sizeof(sc_srcs[0]);

	for (std::size_t i = 0; i < ns; i++)
		sc_run(sc_srcs[i], std::strlen(sc_srcs[i]), "grid");

	{
		char src[SC_SRC];

		for (int v = 0; v < 256; v++) {
			src[0] = static_cast<char>(v);
			src[1] = '\0';
			sc_run(src, 1, "single-byte");
		}
	}

	{
		char src[SC_SRC];

		for (std::size_t n = 1; n <= 32; n++) {
			for (std::size_t i = 0; i < n; i++)
				src[i] = static_cast<char>(0x80 + (i & 0x3f));
			src[n] = '\0';
			sc_run(src, n, "hibit-run");
		}
	}

	{
		char src[SC_SRC];

		for (std::size_t n = 0; n < 16; n++) {
			for (std::size_t i = 0; i < n; i++)
				src[i] = '\0';
			src[n] = '\0';
			sc_run(src, n, "nul-heavy");
		}
	}
}

const unsigned char sc_alpha[] = {
	'a', 'b', 'c', 'z', 0x01, 0x7f, 0x80, 0xff,
};

void
sc_random(long iters)
{
	char src[SC_SRC];
	std::size_t na = sizeof(sc_alpha) / sizeof(sc_alpha[0]);

	for (long it = 0; it < iters; it++) {
		std::size_t slen;
		int wide = (rnd_mod(5) == 0);

		if (rnd_mod(20) == 0)
			slen = 40 + rnd_mod(23);
		else
			slen = rnd_mod(24);

		for (std::size_t i = 0; i < slen; i++) {
			src[i] = wide ? static_cast<char>(1 + rnd_mod(255))
				      : static_cast<char>(sc_alpha[rnd_mod(na)]);
		}
		src[slen] = '\0';
		sc_run(src, slen, "rand");
	}
}

/* ------------------------------------------------------------------------ */
/* strnlen                                                                   */
/* ------------------------------------------------------------------------ */

constexpr std::size_t SN_BUF = 80;

void
sn_run(const unsigned char *s, std::size_t slen, std::size_t maxlen,
    const char *tag)
{
	unsigned char a[SN_BUF], b[SN_BUF];

	if (slen + 1 > SN_BUF) {
		std::fprintf(stderr,
		    "harness bug: %s would overflow (slen=%zu)\n", tag, slen);
		std::exit(2);
	}

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	std::memcpy(a, s, slen + 1);
	std::memcpy(b, s, slen + 1);

	std::size_t ra = port::strnlen(reinterpret_cast<const char *>(a),
	    maxlen);
	std::size_t rb = ref_strnlen(reinterpret_cast<const char *>(b),
	    maxlen);

	int bad_ret = (ra != rb);
	int bad_buf = (std::memcmp(a, b, sizeof(a)) != 0);

	st_strnlen.cases++;
	if (bad_ret || bad_buf) {
		st_strnlen.fails++;
		if (st_strnlen.reported < MAX_REPORT) {
			st_strnlen.reported++;
			std::printf("  FAIL strnlen [%s] slen=%zu maxlen=%zu%s%s\n",
			    tag, slen, maxlen, bad_ret ? " ret" : "",
			    bad_buf ? " buf" : "");
			std::printf("      port_ret=%zu ref_ret=%zu\n", ra, rb);
			dump_bytes("port_buf", a, slen + 1);
			dump_bytes("ref_buf ", b, slen + 1);
		}
	}
}

void
sn_run_cstr(const char *s, std::size_t maxlen, const char *tag)
{
	sn_run(reinterpret_cast<const unsigned char *>(s), std::strlen(s),
	    maxlen, tag);
}

const char *const sn_strs[] = {
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

const std::size_t sn_maxlens[] = {
	0, 1, 2, 3, 4, 5, 8, 16, 32, 64, SN_BUF, (std::size_t)-1,
};

void
sn_edge()
{
	std::size_t ns = sizeof(sn_strs) / sizeof(sn_strs[0]);
	std::size_t nm = sizeof(sn_maxlens) / sizeof(sn_maxlens[0]);

	for (std::size_t i = 0; i < ns; i++) {
		std::size_t slen = std::strlen(sn_strs[i]);
		for (std::size_t j = 0; j < nm; j++) {
			std::size_t maxlen = sn_maxlens[j];

			sn_run_cstr(sn_strs[i], maxlen, "grid");
			if (maxlen > 0)
				sn_run_cstr(sn_strs[i], maxlen - 1, "max-1");
			sn_run_cstr(sn_strs[i], maxlen + 1, "max+1");
			if (slen > 0) {
				sn_run_cstr(sn_strs[i], slen, "at-nul");
				if (slen > 1)
					sn_run_cstr(sn_strs[i], slen - 1,
					    "before-nul");
				sn_run_cstr(sn_strs[i], slen + 1, "after-nul");
			}
		}
	}

	{
		unsigned char buf[SN_BUF];

		for (std::size_t n = 1; n <= 32; n++) {
			for (std::size_t i = 0; i < n; i++)
				buf[i] = static_cast<unsigned char>('a' +
				    (i % 26));
			buf[n] = '\0';
			sn_run(buf, n, n, "exact-nul");
			sn_run(buf, n, n - 1, "short-max");
			sn_run(buf, n, n + 1, "long-max");
		}
	}

	{
		unsigned char buf[SN_BUF];

		for (std::size_t n = 1; n <= 24; n++) {
			for (std::size_t i = 0; i < n; i++)
				buf[i] = static_cast<unsigned char>(0x80 +
				    (i & 0x3f));
			for (std::size_t maxlen = 1; maxlen <= n + 2; maxlen++)
				sn_run(buf, n, maxlen, "no-nul");
		}
	}

	{
		unsigned char buf[2];

		for (int v = 0; v < 256; v++) {
			buf[0] = static_cast<unsigned char>(v);
			buf[1] = '\0';
			sn_run(buf, 1, 0, "byte-max0");
			sn_run(buf, 1, 1, "byte-max1");
			sn_run(buf, 1, 2, "byte-max2");
		}
	}
}

void
sn_random(long iters)
{
	unsigned char buf[SN_BUF];

	for (long it = 0; it < iters; it++) {
		std::size_t slen;
		int unterm = (rnd_mod(6) == 0);

		if (rnd_mod(20) == 0)
			slen = 40 + rnd_mod(39);
		else
			slen = rnd_mod(20);

		if (unterm) {
			for (std::size_t i = 0; i < slen; i++)
				buf[i] = static_cast<unsigned char>(
				    1 + rnd_mod(255));
			sn_run(buf, slen, rnd_mod(SN_BUF), "rand-unterm");
		} else {
			for (std::size_t i = 0; i < slen; i++)
				buf[i] = static_cast<unsigned char>(
				    'a' + rnd_mod(26));
			buf[slen] = '\0';
			sn_run(buf, slen, rnd_mod(SN_BUF + 1), "rand");
		}
	}
}

/* ------------------------------------------------------------------------ */
/* strpbrk                                                                   */
/* ------------------------------------------------------------------------ */

constexpr std::size_t SP_BUF = 80;

void
sp_run(const char *s1, std::size_t l1, const char *s2, std::size_t l2,
    const char *tag)
{
	char a1[SP_BUF], a2[SP_BUF], b1[SP_BUF], b2[SP_BUF];

	if (l1 + 1 > SP_BUF || l2 + 1 > SP_BUF) {
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
		char s1[SP_BUF];
		char s2[2];

		s2[1] = '\0';
		for (std::size_t len = 1; len + 1 <= SP_BUF; len++) {
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
	char s1[SP_BUF], s2[SP_BUF];
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

/* ------------------------------------------------------------------------ */
/* strncpy                                                                   */
/* ------------------------------------------------------------------------ */

constexpr std::size_t NC_DST = 96;
constexpr std::size_t NC_SRC = 64;

void
nc_run(const unsigned char *dst, std::size_t dlen, const char *src,
    std::size_t slen, std::size_t len, const char *tag)
{
	unsigned char a[NC_DST], b[NC_DST];
	char sa[NC_SRC], sb[NC_SRC];

	if (dlen > NC_DST || slen + 1 > NC_SRC) {
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

	char *ra = port::strncpy(reinterpret_cast<char *>(a), sa, len);
	char *rb = ref_strncpy(reinterpret_cast<char *>(b), sb, len);

	long oa = static_cast<long>(ra - reinterpret_cast<char *>(a));
	long ob = static_cast<long>(rb - reinterpret_cast<char *>(b));

	int bad_ret = (oa != ob);
	int bad_dst = (std::memcmp(a, b, sizeof(a)) != 0);
	int bad_src = (std::memcmp(sa, sb, sizeof(sa)) != 0);

	st_strncpy.cases++;
	if (bad_ret || bad_dst || bad_src) {
		st_strncpy.fails++;
		if (st_strncpy.reported < MAX_REPORT) {
			st_strncpy.reported++;
			std::printf("  FAIL strncpy [%s] dlen=%zu slen=%zu "
			    "len=%zu%s%s%s\n",
			    tag, dlen, slen, len, bad_ret ? " ret" : "",
			    bad_dst ? " dst" : "", bad_src ? " src" : "");
			std::printf("      port_off=%ld ref_off=%ld\n", oa, ob);
			dump_bytes("port_dst", a, NC_DST);
			dump_bytes("ref_dst ", b, NC_DST);
			if (bad_src) {
				dump_bytes("port_src",
				    reinterpret_cast<const unsigned char *>(sa),
				    NC_SRC);
				dump_bytes("ref_src ",
				    reinterpret_cast<const unsigned char *>(sb),
				    NC_SRC);
			}
		}
	}
}

void
nc_run_cstr(const unsigned char *dst, std::size_t dlen, const char *src,
    std::size_t len, const char *tag)
{
	nc_run(dst, dlen, src, std::strlen(src), len, tag);
}

const char *const nc_dsts[] = {
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

const char *const nc_srcs[] = {
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

const std::size_t nc_lens[] = {
	0, 1, 2, 3, 4, 5, 8, 16, 32, 64, NC_DST,
};

void
nc_edge()
{
	std::size_t nd = sizeof(nc_dsts) / sizeof(nc_dsts[0]);
	std::size_t ns = sizeof(nc_srcs) / sizeof(nc_srcs[0]);
	std::size_t nl = sizeof(nc_lens) / sizeof(nc_lens[0]);

	for (std::size_t i = 0; i < nd; i++) {
		std::size_t dlen = std::strlen(nc_dsts[i]);
		for (std::size_t j = 0; j < ns; j++) {
			std::size_t slen = std::strlen(nc_srcs[j]);
			for (std::size_t k = 0; k < nl; k++) {
				std::size_t len = nc_lens[k];

				nc_run_cstr(
				    reinterpret_cast<const unsigned char *>(
					nc_dsts[i]),
				    dlen + 1, nc_srcs[j], len, "grid");
				if (len > 0)
					nc_run_cstr(
					    reinterpret_cast<
						const unsigned char *>(
						nc_dsts[i]),
					    dlen + 1, nc_srcs[j], len - 1,
					    "len-1");
				nc_run_cstr(
				    reinterpret_cast<const unsigned char *>(
					nc_dsts[i]),
				    dlen + 1, nc_srcs[j], len + 1, "len+1");
				if (slen > 0) {
					nc_run_cstr(
					    reinterpret_cast<
						const unsigned char *>(
						nc_dsts[i]),
					    dlen + 1, nc_srcs[j], slen,
					    "at-nul");
					nc_run_cstr(
					    reinterpret_cast<
						const unsigned char *>(
						nc_dsts[i]),
					    dlen + 1, nc_srcs[j], slen + 1,
					    "past-nul");
					if (slen > 1)
						nc_run_cstr(
						    reinterpret_cast<
							const unsigned char *>(
							nc_dsts[i]),
						    dlen + 1, nc_srcs[j],
						    slen - 1, "before-nul");
				}
			}
		}
	}

	{
		unsigned char dst[NC_DST];

		for (std::size_t n = 1; n <= 32; n++) {
			for (std::size_t i = 0; i < n; i++)
				dst[i] = static_cast<unsigned char>('A' +
				    (i % 26));
			dst[n] = '\0';
			nc_run_cstr(dst, n + 1, "", 0, "empty-src-len0");
			nc_run_cstr(dst, n + 1, "z", 0, "nz-src-len0");
			nc_run_cstr(dst, n + 1, "longsource", n, "exact-fill");
			nc_run_cstr(dst, n + 1, "longsource", n + 1, "over-fill");
		}
	}

	{
		unsigned char dst[NC_DST];

		for (std::size_t n = 1; n <= 24; n++) {
			for (std::size_t i = 0; i < n; i++)
				dst[i] = static_cast<unsigned char>(0x80 +
				    (i & 0x3f));
			for (std::size_t len = 1; len <= n + 2; len++) {
				nc_run(dst, n, "tail", 4, len, "hibit-dst");
				nc_run(dst, n, "", 0, len, "hibit-empty");
			}
		}
	}

	{
		char src[NC_SRC];

		for (int v = 0; v < 256; v++) {
			unsigned char dst[4] = {0x7f, 0x7f, 0x7f, 0x7f};

			src[0] = static_cast<char>(v);
			src[1] = '\0';
			nc_run(dst, 4, src, 1, 0, "byte-len0");
			nc_run(dst, 4, src, 1, 1, "byte-len1");
			nc_run(dst, 4, src, 1, 2, "byte-len2");
		}
	}
}

const unsigned char nc_alpha[] = {
	'a', 'b', 'c', 'z', 0x01, 0x7f, 0x80, 0xff,
};

void
nc_random(long iters)
{
	unsigned char dst[NC_DST];
	char src[NC_SRC];
	std::size_t na = sizeof(nc_alpha) / sizeof(nc_alpha[0]);

	for (long it = 0; it < iters; it++) {
		std::size_t slen = rnd_mod(20);
		for (std::size_t i = 0; i < slen; i++)
			src[i] = static_cast<char>(nc_alpha[rnd_mod(na)]);
		src[slen] = '\0';

		std::size_t dlen = rnd_mod(24);
		for (std::size_t i = 0; i < dlen; i++)
			dst[i] = static_cast<unsigned char>(1 + rnd_mod(255));
		dst[dlen] = '\0';
		dlen++;

		std::size_t len = rnd_mod(NC_DST);
		nc_run(dst, dlen, src, slen, len, "rand");
	}
}

void
print_table()
{
	const Stat *all[] = {&st_strcpy, &st_strnlen, &st_strpbrk, &st_strncpy};

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
	rng_state = 0xB0081ULL;
	sc_edge();
	sc_random(RANDOM_ITERS);

	rng_state = 0x81B0081ULL;
	sn_edge();
	sn_random(RANDOM_ITERS);

	rng_state = 0x81B0082ULL;
	sp_edge();
	sp_random(RANDOM_ITERS);

	rng_state = 0x81B0083ULL;
	nc_edge();
	nc_random(RANDOM_ITERS);

	print_table();
	return (st_strcpy.fails == 0 && st_strnlen.fails == 0 &&
	    st_strpbrk.fails == 0 && st_strncpy.fails == 0) ? 0 : 1;
}
