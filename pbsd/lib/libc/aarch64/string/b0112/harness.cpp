/*
 * Differential test harness for PBSD batch b0112.
 *
 * Compares bzero, bcopy, strcat and strncat in
 * pbsd.lib.libc.aarch64.string.b0112 against the ref_ oracle in oracle.c.
 */

import pbsd.lib.libc.aarch64.string.b0112;

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::lib_libc_aarch64_string::b0112;

extern "C" {
void ref_bzero(void *, std::size_t);
void ref_bcopy(const void *, void *, std::size_t);
char *ref_strcat(char *__restrict, const char *__restrict);
char *ref_strncat(char *, const char *, std::size_t);
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
Stat st_strcat{"strcat", 0, 0, 0};
Stat st_strncat{"strncat", 0, 0, 0};

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
/* strcat                                                                    */
/* ------------------------------------------------------------------------ */

constexpr std::size_t SC_BUF = 160;
constexpr std::size_t SC_STR = 72;

void
sc_run(const char *dst, const char *append, const char *tag)
{
	unsigned char a[SC_BUF], b[SC_BUF];
	char aa[SC_STR], ab[SC_STR];
	std::size_t dlen = std::strlen(dst);
	std::size_t alen = std::strlen(append);

	if (dlen + alen + 1 > SC_BUF || alen + 1 > SC_STR) {
		std::fprintf(stderr, "harness bug: strcat %s\n", tag);
		std::exit(2);
	}

	std::size_t base = (SC_BUF - (dlen + alen + 1)) / 2;

	std::memset(a, GUARD, sizeof(a));
	std::memset(b, GUARD, sizeof(b));
	std::memset(aa, GUARD, sizeof(aa));
	std::memset(ab, GUARD, sizeof(ab));
	std::memcpy(a + base, dst, dlen + 1);
	std::memcpy(b + base, dst, dlen + 1);
	std::memcpy(aa, append, alen + 1);
	std::memcpy(ab, append, alen + 1);

	char *pa = reinterpret_cast<char *>(a + base);
	char *pb = reinterpret_cast<char *>(b + base);

	char *ra = port::strcat(pa, aa);
	char *rb = ref_strcat(pb, ab);

	std::ptrdiff_t oa = ra - pa;
	std::ptrdiff_t ob = rb - pb;
	int bad_ret = (oa != ob);
	int bad_buf = (std::memcmp(a, b, sizeof(a)) != 0);
	int bad_app = (std::memcmp(aa, ab, sizeof(aa)) != 0);

	st_strcat.cases++;
	if (bad_ret || bad_buf || bad_app) {
		st_strcat.fails++;
		if (st_strcat.reported < MAX_REPORT) {
			st_strcat.reported++;
			std::printf("  FAIL strcat [%s] dlen=%zu alen=%zu%s%s%s\n",
			    tag, dlen, alen, bad_ret ? " ret" : "",
			    bad_buf ? " buf" : "", bad_app ? " app" : "");
			std::printf("      port_off=%td ref_off=%td\n", oa, ob);
			dump_bytes("port_buf", a, SC_BUF);
			dump_bytes("ref_buf ", b, SC_BUF);
		}
	}
}

void
sc_edge()
{
	sc_run("", "", "both-empty");
	sc_run("", "a", "empty-dst");
	sc_run("a", "", "empty-app");
	sc_run("a", "b", "1+1");
	sc_run("ab", "cd", "2+2");
	sc_run("\x80", "\xff", "hi-bit");
	sc_run("prefix", "suffix", "words");
	sc_run("x", "yyyyyyyy", "short+long");
	{
		char d[48], a[48];

		std::memset(d, 'A', sizeof(d) - 1);
		d[sizeof(d) - 1] = '\0';
		std::memset(a, 'B', sizeof(a) - 1);
		a[sizeof(a) - 1] = '\0';
		sc_run(d, a, "long-both");
	}
}

const unsigned char sc_alpha[] = {
	'a', 'b', 0x80, 0xff, 0x7f, 0x01, 'z', 0xc3,
};

void
sc_random(long iters)
{
	char dst[SC_STR], app[SC_STR];
	std::size_t na = sizeof(sc_alpha) / sizeof(sc_alpha[0]);

	for (long it = 0; it < iters; it++) {
		std::size_t dl, al;
		int wide = (rnd_mod(4) == 0);

		if (rnd_mod(20) == 0)
			dl = 40 + rnd_mod(24);
		else
			dl = rnd_mod(20);
		al = rnd_mod(24);

		for (std::size_t i = 0; i < dl; i++)
			dst[i] = wide ? static_cast<char>(1 + rnd_mod(255))
				      : static_cast<char>(sc_alpha[rnd_mod(na)]);
		dst[dl] = '\0';
		for (std::size_t i = 0; i < al; i++)
			app[i] = wide ? static_cast<char>(1 + rnd_mod(255))
				      : static_cast<char>(sc_alpha[rnd_mod(na)]);
		app[al] = '\0';

		sc_run(dst, app, "rand");
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

void
print_table()
{
	const Stat *all[] = {&st_bzero, &st_bcopy, &st_strcat, &st_strncat};

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
	rng_state = 0xB0112ULL;
	bz_edge();
	bz_random(RANDOM_ITERS);

	rng_state = 0x112B0112ULL;
	bc_edge();
	bc_random(RANDOM_ITERS);

	rng_state = 0xCA7B0112ULL;
	sc_edge();
	sc_random(RANDOM_ITERS);

	rng_state = 0x5ACA7112ULL;
	sn_edge();
	sn_random(RANDOM_ITERS);

	print_table();
	return (st_bzero.fails == 0 && st_bcopy.fails == 0 &&
	    st_strcat.fails == 0 && st_strncat.fails == 0) ? 0 : 1;
}
