/*
 * Differential test harness for PBSD batch b0120.
 *
 * Compares bzero, bcopy, strncat and strlcat in
 * pbsd.lib.libc.amd64.string.b0120 against the ref_ oracle in oracle.c.
 */

import pbsd.lib.libc.amd64.string.b0120;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::lib_libc_amd64_string::b0120;

extern "C" {
void ref_bzero(void *, std::size_t);
void ref_bcopy(const void *, void *, std::size_t);
char *ref_strncat(char *, const char *, std::size_t);
std::size_t ref_strlcat(char *__restrict, const char *__restrict, std::size_t);
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

Stat st_bzero{"bzero", 0, 0, 0};
Stat st_bcopy{"bcopy", 0, 0, 0};
Stat st_strncat{"strncat", 0, 0, 0};
Stat st_strlcat{"strlcat", 0, 0, 0};

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
/* bzero                                                                     */
/* ------------------------------------------------------------------------ */

constexpr std::size_t BZ_BUF = 96;

void
bz_run(std::size_t start, std::size_t zlen, unsigned char fill, const char *tag)
{
	unsigned char a[BZ_BUF], b[BZ_BUF];

	if (start + zlen > BZ_BUF) {
		std::fprintf(stderr,
		    "harness bug: bzero %s start=%zu zlen=%zu\n", tag, start,
		    zlen);
		std::exit(2);
	}

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	for (std::size_t i = start; i < start + zlen; i++) {
		a[i] = fill;
		b[i] = fill;
	}

	port::bzero(a + start, zlen);
	ref_bzero(b + start, zlen);

	int bad = (std::memcmp(a, b, sizeof(a)) != 0);

	st_bzero.cases++;
	if (bad) {
		st_bzero.fails++;
		if (st_bzero.reported < MAX_REPORT) {
			st_bzero.reported++;
			std::printf("  FAIL bzero [%s] start=%zu zlen=%zu fill=%02x\n",
			    tag, start, zlen, static_cast<unsigned>(fill));
			dump_bytes("port", a, BZ_BUF);
			dump_bytes("ref ", b, BZ_BUF);
		}
	}
}

void
bz_edge()
{
	bz_run(0, 0, 0x00, "len0");
	bz_run(0, 0, 0xff, "len0-hi");
	bz_run(0, 1, 0xab, "len1");
	bz_run(8, 1, 0x80, "off1-hi");
	bz_run(0, BZ_BUF, 0x55, "full");
	bz_run(4, 32, 0xff, "mid32-hi");
	bz_run(BZ_BUF - 1, 1, 0x01, "tail1");
}

void
bz_random(long iters)
{
	for (long it = 0; it < iters; it++) {
		std::size_t start = rnd_mod(BZ_BUF);
		std::size_t zlen = rnd_mod(BZ_BUF - start + 1);
		unsigned char fill = static_cast<unsigned char>(rnd_mod(256));

		bz_run(start, zlen, fill, "rand");
	}
}

/* ------------------------------------------------------------------------ */
/* bcopy                                                                     */
/* ------------------------------------------------------------------------ */

constexpr std::size_t BC_BUF = 128;

void
bc_run(std::size_t dst_off, std::size_t src_off, std::size_t len,
    const char *tag)
{
	unsigned char a[BC_BUF], b[BC_BUF];

	if (dst_off >= BC_BUF || src_off >= BC_BUF ||
	    dst_off + len > BC_BUF || src_off + len > BC_BUF) {
		std::fprintf(stderr,
		    "harness bug: bcopy %s dst=%zu src=%zu len=%zu\n", tag,
		    dst_off, src_off, len);
		std::exit(2);
	}

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	for (std::size_t i = 0; i < BC_BUF; i++) {
		unsigned char v = static_cast<unsigned char>((i * 37 + 0x13) & 0xff);

		a[i] = v;
		b[i] = v;
	}

	port::bcopy(a + src_off, a + dst_off, len);
	ref_bcopy(b + src_off, b + dst_off, len);

	int bad = (std::memcmp(a, b, sizeof(a)) != 0);

	st_bcopy.cases++;
	if (bad) {
		st_bcopy.fails++;
		if (st_bcopy.reported < MAX_REPORT) {
			st_bcopy.reported++;
			std::printf("  FAIL bcopy [%s] dst=%zu src=%zu len=%zu\n",
			    tag, dst_off, src_off, len);
			dump_bytes("port", a, BC_BUF);
			dump_bytes("ref ", b, BC_BUF);
		}
	}
}

void
bc_edge()
{
	bc_run(10, 20, 0, "len0");
	bc_run(10, 20, 1, "len1");
	bc_run(10, 20, 30, "fwd-disjoint");
	bc_run(20, 10, 30, "bwd-disjoint");
	bc_run(15, 10, 20, "fwd-overlap");
	bc_run(10, 15, 20, "bwd-overlap");
	bc_run(40, 40, 16, "same");
	bc_run(0, 64, 64, "block");
	bc_run(1, 0, 80, "src0-dst1");
	bc_run(0, 1, 80, "src1-dst0");
}

void
bc_random(long iters)
{
	for (long it = 0; it < iters; it++) {
		std::size_t dst_off = rnd_mod(BC_BUF);
		std::size_t src_off = rnd_mod(BC_BUF);
		std::size_t max_len = BC_BUF - (dst_off > src_off ? dst_off : src_off);

		if (max_len == 0)
			max_len = 1;
		std::size_t len = rnd_mod(max_len);

		bc_run(dst_off, src_off, len, "rand");
	}
}

/* ------------------------------------------------------------------------ */
/* strncat                                                                   */
/* ------------------------------------------------------------------------ */

constexpr std::size_t SN_BUF = 192;
constexpr std::size_t SN_SRC = 80;

void
sn_run(const char *dest, const unsigned char *src, std::size_t slen,
    std::size_t n, const char *tag)
{
	unsigned char a[SN_BUF], b[SN_BUF];
	char sa[SN_SRC], sb[SN_SRC];
	std::size_t dlen = std::strlen(dest);

	if (dlen + n + 2 > SN_BUF || slen + 2 > SN_SRC) {
		std::fprintf(stderr, "harness bug: strncat %s\n", tag);
		std::exit(2);
	}

	std::size_t base = (SN_BUF - (dlen + n + 2)) / 2;

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	std::memset(sa, GUARD, sizeof(sa));
	std::memset(sb, GUARD, sizeof(sb));
	std::memcpy(a + base, dest, dlen + 1);
	std::memcpy(b + base, dest, dlen + 1);
	std::memcpy(sa, src, slen);
	std::memcpy(sb, src, slen);
	sa[slen] = '\0';
	sb[slen] = '\0';

	char *pa = reinterpret_cast<char *>(a + base);
	char *pb = reinterpret_cast<char *>(b + base);

	char *ra = port::strncat(pa, sa, n);
	char *rb = ref_strncat(pb, sb, n);

	std::ptrdiff_t oa = ra - pa;
	std::ptrdiff_t ob = rb - pb;
	int bad_ret = (oa != ob);
	int bad_buf = (std::memcmp(a, b, sizeof(a)) != 0);
	int bad_src = (std::memcmp(sa, sb, sizeof(sa)) != 0);

	st_strncat.cases++;
	if (bad_ret || bad_buf || bad_src) {
		st_strncat.fails++;
		if (st_strncat.reported < MAX_REPORT) {
			st_strncat.reported++;
			std::printf(
			    "  FAIL strncat [%s] dlen=%zu slen=%zu n=%zu%s%s%s\n",
			    tag, dlen, slen, n, bad_ret ? " ret" : "",
			    bad_buf ? " buf" : "", bad_src ? " src" : "");
			std::printf("      port_off=%td ref_off=%td\n", oa, ob);
			dump_bytes("port_buf", a, SN_BUF);
			dump_bytes("ref_buf ", b, SN_BUF);
		}
	}
}

void
sn_run_cstr(const char *dest, const char *src, std::size_t n, const char *tag)
{
	std::size_t slen = std::strlen(src);

	sn_run(dest, reinterpret_cast<const unsigned char *>(src), slen, n, tag);
}

void
sn_edge()
{
	sn_run_cstr("", "", 0, "empty-n0");
	sn_run_cstr("", "", 1, "empty-n1");
	sn_run_cstr("", "a", 0, "dst0-n0");
	sn_run_cstr("", "a", 1, "dst0-n1");
	sn_run_cstr("", "a", 2, "dst0-n2");
	sn_run_cstr("x", "", 0, "app0-n0");
	sn_run_cstr("x", "y", 0, "n0");
	sn_run_cstr("x", "y", 1, "n1-hit");
	sn_run_cstr("x", "yz", 2, "n2-miss");
	sn_run_cstr("", "\0ab", 5, "lead-nul");
	sn_run_cstr("ab", "cd", 1, "n1");
	sn_run_cstr("ab", "cd", 2, "n2");
	sn_run_cstr("ab", "cd", 3, "n3");
	sn_run_cstr("ab", "cd", 8, "n-big");
	sn_run_cstr("\x80", "\xff", 1, "hi-n1");
	sn_run_cstr("\x80", "\xff\xfe", 2, "hi-n2");
	{
		const unsigned char raw[] = {0x79, 0x7a, 0x7b, 0x7c};

		sn_run("", raw, sizeof(raw), 4, "raw4");
		sn_run("p", raw, sizeof(raw), 2, "p-raw-n2");
	}
}

const unsigned char sn_alpha[] = {
	'a', 'b', 0x80, 0xff, 0x7f, 0x01, 'z', 0xc3,
};

void
sn_random(long iters)
{
	char dest[SN_SRC];
	unsigned char src[SN_SRC];
	std::size_t na = sizeof(sn_alpha) / sizeof(sn_alpha[0]);

	for (long it = 0; it < iters; it++) {
		std::size_t dl, sl, n;
		int wide = (rnd_mod(4) == 0);
		int raw = (rnd_mod(5) == 0);

		if (rnd_mod(20) == 0)
			dl = 30 + rnd_mod(30);
		else
			dl = rnd_mod(18);
		sl = 1 + rnd_mod(32);
		n = rnd_mod(sl + 6);

		for (std::size_t i = 0; i < dl; i++)
			dest[i] = wide ? static_cast<char>(1 + rnd_mod(255))
				       : static_cast<char>(sn_alpha[rnd_mod(na)]);
		dest[dl] = '\0';

		if (raw) {
			for (std::size_t i = 0; i < sl; i++)
				src[i] = static_cast<unsigned char>(0x40 +
				    rnd_mod(0xc0));
		} else {
			for (std::size_t i = 0; i < sl; i++)
				src[i] = static_cast<unsigned char>(
				    wide ? (1 + rnd_mod(255))
					 : sn_alpha[rnd_mod(na)]);
		}

		sn_run(dest, src, sl, n, "rand");
	}
}

/* ------------------------------------------------------------------------ */
/* strlcat                                                                   */
/* ------------------------------------------------------------------------ */

constexpr std::size_t DST_BUF = 96;
constexpr std::size_t SRC_BUF = 64;

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

	sl_run_cstr("", 0, "", "dstsize0-empty");
	sl_run_cstr("abc", 0, "def", "dstsize0-nz");
	sl_run_cstr("abc", 0, "\xff\x80", "dstsize0-hibit");

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

void
print_table()
{
	const Stat *all[] = {&st_bzero, &st_bcopy, &st_strncat, &st_strlcat};

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
	rng_state = 0xB0120ULL;
	bz_edge();
	bz_random(RANDOM_ITERS);

	rng_state = 0x120B0120ULL;
	bc_edge();
	bc_random(RANDOM_ITERS);

	rng_state = 0x5ACA7120ULL;
	sn_edge();
	sn_random(RANDOM_ITERS);

	rng_state = 0xCA7B0120ULL;
	sl_edge();
	sl_random(RANDOM_ITERS);

	print_table();
	return (st_bzero.fails == 0 && st_bcopy.fails == 0 &&
	    st_strncat.fails == 0 && st_strlcat.fails == 0) ? 0 : 1;
}
