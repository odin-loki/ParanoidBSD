/*
 * harness.cpp -- differential test for PBSD batch b0127.
 *
 * Every case executes the C++ port and the ref_ oracle on independent but
 * identically-prepared state.  Return values and the full contents of every
 * caller buffer (guard bytes included) are compared.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <climits>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#if __has_include(<xlocale.h>)
#include <xlocale.h>
#else
#include <locale.h>
#endif

import pbsd.lib.libc.stdio.b0127;

namespace P = pbsd::lib_libc_stdio::b0127;

extern "C" {
int	ref_fscanf(FILE * __restrict, char const * __restrict, ...);
int	ref_fscanf_l(FILE * __restrict, locale_t, char const * __restrict, ...);
}

/* ------------------------------------------------------------------------ */

static constexpr unsigned char GUARD = 0x7f;
static constexpr int MAX_REPORT = 8;
static constexpr long SWEEP = 200000;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat st_fscanf = { "fscanf", 0, 0, 0 };
static Stat st_fscanf_l = { "fscanf_l", 0, 0, 0 };

static std::uint64_t rng_state = 0xb0127c0ffeeULL;

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
	if (m == 0)
		return 0;
	return (std::size_t)(rnd() % (std::uint64_t)m);
}

static void
stat_fail(Stat *st, const char *tag, const char *detail)
{
	st->fails++;
	if (st->reported < MAX_REPORT) {
		st->reported++;
		std::printf("  FAIL %s [%s] %s\n", st->name, tag, detail);
	}
}

/* ------------------------------------------------------------------------ */

struct ScanSlots {
	unsigned char pre[16];
	int i;
	unsigned char mid1[16];
	unsigned u;
	unsigned char mid2[16];
	long l;
	unsigned char mid3[16];
	char s[64];
	unsigned char mid4[16];
	char c;
	unsigned char post[16];
};

static void
scan_slots_init(ScanSlots *sl)
{
	std::memset(sl, GUARD, sizeof(*sl));
}

static int
slots_equal(const ScanSlots *a, const ScanSlots *b)
{
	return std::memcmp(a, b, sizeof(*a)) == 0;
}

static int
write_input_file(const char *path, const unsigned char *data, std::size_t len)
{
	FILE *fp = std::fopen(path, "wb");

	if (fp == nullptr)
		return -1;
	if (len > 0 && std::fwrite(data, 1, len, fp) != len) {
		std::fclose(fp);
		return -1;
	}
	return std::fclose(fp);
}

static int
call_port_fscanf(const char *path, const char *fmt, ScanSlots *sl)
{
	FILE *fp;
	int r;

	fp = std::fopen(path, "rb");
	if (fp == nullptr)
		return -2;
	setbuf(fp, nullptr);
	clearerr(fp);
	scan_slots_init(sl);
	r = P::fscanf(fp, fmt, &sl->i, &sl->u, &sl->l, sl->s, &sl->c);
	std::fclose(fp);
	return r;
}

static int
call_ref_fscanf(const char *path, const char *fmt, ScanSlots *sl)
{
	FILE *fp;
	int r;

	fp = std::fopen(path, "rb");
	if (fp == nullptr)
		return -2;
	setbuf(fp, nullptr);
	clearerr(fp);
	scan_slots_init(sl);
	r = ref_fscanf(fp, fmt, &sl->i, &sl->u, &sl->l, sl->s, &sl->c);
	std::fclose(fp);
	return r;
}

static int
call_port_fscanf_l(const char *path, locale_t loc, const char *fmt, ScanSlots *sl)
{
	FILE *fp;
	int r;

	fp = std::fopen(path, "rb");
	if (fp == nullptr)
		return -2;
	setbuf(fp, nullptr);
	clearerr(fp);
	scan_slots_init(sl);
	r = P::fscanf_l(fp, loc, fmt, &sl->i, &sl->u, &sl->l, sl->s, &sl->c);
	std::fclose(fp);
	return r;
}

static int
call_ref_fscanf_l(const char *path, locale_t loc, const char *fmt, ScanSlots *sl)
{
	FILE *fp;
	int r;

	fp = std::fopen(path, "rb");
	if (fp == nullptr)
		return -2;
	setbuf(fp, nullptr);
	clearerr(fp);
	scan_slots_init(sl);
	r = ref_fscanf_l(fp, loc, fmt, &sl->i, &sl->u, &sl->l, sl->s, &sl->c);
	std::fclose(fp);
	return r;
}

static void
fscan_case(Stat *st, locale_t loc, int use_l, const unsigned char *data,
    std::size_t len, const char *fmt, const char *tag)
{
	char path[] = "/tmp/pbsd_b0127_in_XXXXXX";
	int fd;
	ScanSlots a, b;
	int ra, rb;
	int bad;

	fd = mkstemp(path);
	if (fd < 0) {
		std::fprintf(stderr, "harness bug: mkstemp fscanf\n");
		std::exit(2);
	}
	close(fd);
	if (write_input_file(path, data, len) != 0) {
		unlink(path);
		std::fprintf(stderr, "harness bug: write fscanf input\n");
		std::exit(2);
	}

	if (use_l) {
		ra = call_port_fscanf_l(path, loc, fmt, &a);
		rb = call_ref_fscanf_l(path, loc, fmt, &b);
	} else {
		ra = call_port_fscanf(path, fmt, &a);
		rb = call_ref_fscanf(path, fmt, &b);
	}

	st->cases++;
	bad = 0;
	if (ra != rb)
		bad = 1;
	if (!slots_equal(&a, &b))
		bad = 1;
	if (bad) {
		char detail[192];
		std::snprintf(detail, sizeof(detail),
		    "[%s] len=%zu fmt='%s' ret %d/%d", tag, len, fmt, ra, rb);
		stat_fail(st, "case", detail);
	}

	unlink(path);
}

static void
fscanf_edges(locale_t loc)
{
	static const unsigned char empty[] = { "" };
	static const unsigned char one[] = { '7' };
	static const unsigned char zero[] = { '0' };
	static const unsigned char neg[] = { '-', '3' };
	static const unsigned char hi[] = { 0x80 };
	static const unsigned char ff[] = { 0xff };
	static const unsigned char pair[] = { '1', ' ', '2' };
	static const unsigned char str[] = { 'a', 'b', 'c' };
	static const unsigned char pct[] = { '%' };
	static const unsigned char ws[] = { ' ', ' ', '9' };
	static const unsigned char nomatch[] = { 'n', 'o' };
	static const unsigned char partial[] = { '1', '2', 'x' };
	static const unsigned char suppress[] = { '3', ' ', '4' };
	static const unsigned char nulpair[] = { 0x00, 0x01 };
	static const unsigned char longnum[] = { '1', '2', '3', '4', '5', '6', '7' };
	static const unsigned char hex[] = { 'f', 'f' };
	static const unsigned char oct[] = { '7', '7' };

	fscan_case(&st_fscanf, loc, 0, empty, 0, "%d", "empty");
	fscan_case(&st_fscanf, loc, 0, one, 1, "%d", "one");
	fscan_case(&st_fscanf, loc, 0, zero, 1, "%d", "zero");
	fscan_case(&st_fscanf, loc, 0, neg, 2, "%d", "neg");
	fscan_case(&st_fscanf, loc, 0, hi, 1, "%c", "hi");
	fscan_case(&st_fscanf, loc, 0, ff, 1, "%c", "ff");
	fscan_case(&st_fscanf, loc, 0, pair, 3, "%d%d", "pair");
	fscan_case(&st_fscanf, loc, 0, str, 3, "%s", "str");
	fscan_case(&st_fscanf, loc, 0, pct, 1, "%%", "pct");
	fscan_case(&st_fscanf, loc, 0, ws, 3, "%d", "ws");
	fscan_case(&st_fscanf, loc, 0, nomatch, 2, "%d", "nomatch");
	fscan_case(&st_fscanf, loc, 0, partial, 3, "%d", "partial");
	fscan_case(&st_fscanf, loc, 0, suppress, 3, "%*d%d", "suppress");
	fscan_case(&st_fscanf, loc, 0, nulpair, 2, "%c%c", "nul");
	fscan_case(&st_fscanf, loc, 0, longnum, 7, "%d", "longnum");
	fscan_case(&st_fscanf, loc, 0, hex, 2, "%x", "hex");
	fscan_case(&st_fscanf, loc, 0, oct, 2, "%o", "oct");
	fscan_case(&st_fscanf, loc, 0, one, 1, "%u", "unsigned");
	fscan_case(&st_fscanf, loc, 0, neg, 2, "%i", "i");
	fscan_case(&st_fscanf, loc, 0, pair, 3, "%d %d", "spaced");
	fscan_case(&st_fscanf, loc, 0, str, 3, "%2s", "width");

	fscan_case(&st_fscanf_l, loc, 1, empty, 0, "%d", "empty");
	fscan_case(&st_fscanf_l, loc, 1, one, 1, "%d", "one");
	fscan_case(&st_fscanf_l, loc, 1, zero, 1, "%d", "zero");
	fscan_case(&st_fscanf_l, loc, 1, neg, 2, "%d", "neg");
	fscan_case(&st_fscanf_l, loc, 1, hi, 1, "%c", "hi");
	fscan_case(&st_fscanf_l, loc, 1, ff, 1, "%c", "ff");
	fscan_case(&st_fscanf_l, loc, 1, pair, 3, "%d%d", "pair");
	fscan_case(&st_fscanf_l, loc, 1, str, 3, "%s", "str");
	fscan_case(&st_fscanf_l, loc, 1, pct, 1, "%%", "pct");
	fscan_case(&st_fscanf_l, loc, 1, ws, 3, "%d", "ws");
	fscan_case(&st_fscanf_l, loc, 1, nomatch, 2, "%d", "nomatch");
	fscan_case(&st_fscanf_l, loc, 1, partial, 3, "%d", "partial");
	fscan_case(&st_fscanf_l, loc, 1, suppress, 3, "%*d%d", "suppress");
	fscan_case(&st_fscanf_l, loc, 1, nulpair, 2, "%c%c", "nul");
	fscan_case(&st_fscanf_l, loc, 1, longnum, 7, "%d", "longnum");
	fscan_case(&st_fscanf_l, loc, 1, hex, 2, "%x", "hex");
	fscan_case(&st_fscanf_l, loc, 1, oct, 2, "%o", "oct");
	fscan_case(&st_fscanf_l, loc, 1, one, 1, "%u", "unsigned");
	fscan_case(&st_fscanf_l, loc, 1, neg, 2, "%i", "i");
	fscan_case(&st_fscanf_l, loc, 1, pair, 3, "%d %d", "spaced");
	fscan_case(&st_fscanf_l, loc, 1, str, 3, "%2s", "width");

	fscan_case(&st_fscanf_l, (locale_t)0, 1, one, 1, "%d", "null-loc");
	fscan_case(&st_fscanf_l, LC_GLOBAL_LOCALE, 1, one, 1, "%d", "global-loc");
}

static const char *const scan_fmts[] = {
	"%d", "%u", "%x", "%i", "%o", "%c", "%s", "%d%d", "%x %u",
	"%*d%d", "%2d", "%3s", "%%", "%hd", "%ld", "%c%c", "%s%d",
	"%d %d", "%5d", "%*c%c", ""
};

static void
fscanf_random(locale_t loc, long n)
{
	unsigned char inbuf[96];

	for (long t = 0; t < n; t++) {
		const char *fmt = scan_fmts[rnd_mod(sizeof(scan_fmts) /
		    sizeof(scan_fmts[0]))];
		std::size_t len = rnd_mod(sizeof(inbuf));

		for (std::size_t i = 0; i < len; i++)
			inbuf[i] = (unsigned char)rnd();
		fscan_case(&st_fscanf, loc, 0, inbuf, len, fmt, "random");
		fscan_case(&st_fscanf_l, loc, 1, inbuf, len, fmt, "random");
		if ((t & 0xff) == 0) {
			fscan_case(&st_fscanf_l, (locale_t)0, 1, inbuf, len,
			    fmt, "null-loc");
			fscan_case(&st_fscanf_l, LC_GLOBAL_LOCALE, 1, inbuf, len,
			    fmt, "global-loc");
		}
	}
}

/* ------------------------------------------------------------------------ */

int
main(void)
{
	locale_t loc;
	long total_fails;
	int rc = 0;

	loc = newlocale(LC_ALL_MASK, "C", (locale_t)0);
	if (loc == nullptr)
		loc = LC_GLOBAL_LOCALE;

	fscanf_edges(loc);
	fscanf_random(loc, SWEEP);

	if (loc != nullptr && loc != LC_GLOBAL_LOCALE)
		freelocale(loc);

	total_fails = st_fscanf.fails + st_fscanf_l.fails;

	std::printf("\n");
	std::printf("+----------------+----------+----------+\n");
	std::printf("| function       |     cases|    fails |\n");
	std::printf("+----------------+----------+----------+\n");
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_fscanf.name, st_fscanf.cases, st_fscanf.fails);
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_fscanf_l.name, st_fscanf_l.cases, st_fscanf_l.fails);
	std::printf("+----------------+----------+----------+\n");

	if (total_fails != 0)
		rc = 1;
	return rc;
}
