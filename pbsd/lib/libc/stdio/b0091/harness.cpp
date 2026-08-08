/*
 * harness.cpp -- differential test for PBSD batch b0091.
 *
 * Every case executes the C++ port and the ref_ oracle on independent but
 * identically-prepared stdin streams.  Return values and the full tail-buffer
 * contents (guard bytes included) are compared after each call.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <locale.h>
#include <unistd.h>
#include <wchar.h>

import pbsd.lib.libc.stdio.b0091;

namespace port = pbsd::lib_libc_stdio::b0091;

extern "C" {
wint_t ref_getwchar(void);
wint_t ref_getwchar_l(locale_t locale);
wint_t fgetwc_l(FILE *fp, locale_t loc);
}

extern "C" wint_t
fgetwc_l(FILE *fp, locale_t loc)
{
	locale_t old = uselocale(loc);
	wint_t w = fgetwc(fp);

	uselocale(old);
	return w;
}

/* ------------------------------------------------------------------------ */

static const unsigned char GUARD = 0x7f;
static const int MAX_REPORT = 8;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat st_getwchar = { "getwchar", 0, 0, 0 };
static Stat st_getwchar_l = { "getwchar_l", 0, 0, 0 };

static std::uint64_t rng_state = 0x91b0091cafebabeULL;

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
stat_fail(Stat *st, const char *tag, const char *detail)
{
	st->fails++;
	if (st->reported < MAX_REPORT) {
		st->reported++;
		std::printf("  FAIL %s [%s] %s\n", st->name, tag, detail);
	}
}

/* ------------------------------------------------------------------------ */

struct WcharObs {
	wint_t ret;
	unsigned char tail[64];
};

static int
write_input_file(const char *path, const unsigned char *data, std::size_t len)
{
	FILE *fp = fopen(path, "wb");

	if (fp == nullptr)
		return -1;
	if (len > 0 && fwrite(data, 1, len, fp) != len) {
		fclose(fp);
		return -1;
	}
	return fclose(fp);
}

static WcharObs
call_getwchar_on(const char *path)
{
	WcharObs obs{};

	std::memset(obs.tail, GUARD, sizeof(obs.tail));
	if (freopen(path, "r", stdin) == nullptr) {
		obs.ret = (wint_t)0xdead;
		return obs;
	}
	clearerr(stdin);
	obs.ret = port::getwchar();
	(void)fread(obs.tail, 1, sizeof(obs.tail), stdin);
	return obs;
}

static WcharObs
call_ref_getwchar_on(const char *path)
{
	WcharObs obs{};

	std::memset(obs.tail, GUARD, sizeof(obs.tail));
	if (freopen(path, "r", stdin) == nullptr) {
		obs.ret = (wint_t)0xdead;
		return obs;
	}
	clearerr(stdin);
	obs.ret = ref_getwchar();
	(void)fread(obs.tail, 1, sizeof(obs.tail), stdin);
	return obs;
}

static WcharObs
call_getwchar_l_on(const char *path, locale_t loc)
{
	WcharObs obs{};

	std::memset(obs.tail, GUARD, sizeof(obs.tail));
	if (freopen(path, "r", stdin) == nullptr) {
		obs.ret = (wint_t)0xdead;
		return obs;
	}
	clearerr(stdin);
	obs.ret = port::getwchar_l(loc);
	(void)fread(obs.tail, 1, sizeof(obs.tail), stdin);
	return obs;
}

static WcharObs
call_ref_getwchar_l_on(const char *path, locale_t loc)
{
	WcharObs obs{};

	std::memset(obs.tail, GUARD, sizeof(obs.tail));
	if (freopen(path, "r", stdin) == nullptr) {
		obs.ret = (wint_t)0xdead;
		return obs;
	}
	clearerr(stdin);
	obs.ret = ref_getwchar_l(loc);
	(void)fread(obs.tail, 1, sizeof(obs.tail), stdin);
	return obs;
}

static void
wchar_case(Stat *st, const unsigned char *data, std::size_t len,
    locale_t loc, int use_l, const char *tag)
{
	char path[] = "/tmp/pbsd_b0091_wc_XXXXXX";
	int fd;
	WcharObs a, b;
	int bad;

	fd = mkstemp(path);
	if (fd < 0) {
		std::fprintf(stderr, "harness bug: mkstemp wchar\n");
		std::exit(2);
	}
	close(fd);
	if (write_input_file(path, data, len) != 0) {
		unlink(path);
		std::fprintf(stderr, "harness bug: write wchar input\n");
		std::exit(2);
	}

	if (use_l) {
		a = call_getwchar_l_on(path, loc);
		b = call_ref_getwchar_l_on(path, loc);
	} else {
		a = call_getwchar_on(path);
		b = call_ref_getwchar_on(path);
	}

	st->cases++;
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (std::memcmp(a.tail, b.tail, sizeof(a.tail)) != 0)
		bad = 1;
	if (bad) {
		char detail[128];
		std::snprintf(detail, sizeof(detail),
		    "len=%zu ret=%ld/%ld", len, (long)a.ret, (long)b.ret);
		stat_fail(st, tag, detail);
	}

	unlink(path);
}

static void
getwchar_edges(locale_t loc)
{
	static const unsigned char empty[] = { "" };
	static const unsigned char a[] = { 'a' };
	static const unsigned char z[] = { 'z' };
	static const unsigned char hi[] = { 0x80 };
	static const unsigned char ff[] = { 0xff };
	static const unsigned char nul[] = { 0x00, 'b', 0x00 };
	static const unsigned char utf8[] = { 0xc3, 0xa9, '!' };
	static const unsigned char pair[] = { 'A', 'B', 'C' };
	static const unsigned char mix[] = {
		0x7f, 0x80, 0x81, 0xfe, 0xff, 'x', '\n', 0x00, 'y'
	};

	wchar_case(&st_getwchar, empty, 0, loc, 0, "empty");
	wchar_case(&st_getwchar, a, 1, loc, 0, "a");
	wchar_case(&st_getwchar, z, 1, loc, 0, "z");
	wchar_case(&st_getwchar, hi, 1, loc, 0, "0x80");
	wchar_case(&st_getwchar, ff, 1, loc, 0, "0xff");
	wchar_case(&st_getwchar, nul, 3, loc, 0, "nul");
	wchar_case(&st_getwchar, utf8, 3, loc, 0, "utf8");
	wchar_case(&st_getwchar, pair, 3, loc, 0, "abc");
	wchar_case(&st_getwchar, mix, sizeof(mix), loc, 0, "mix");

	wchar_case(&st_getwchar_l, empty, 0, loc, 1, "empty");
	wchar_case(&st_getwchar_l, a, 1, loc, 1, "a");
	wchar_case(&st_getwchar_l, z, 1, loc, 1, "z");
	wchar_case(&st_getwchar_l, hi, 1, loc, 1, "0x80");
	wchar_case(&st_getwchar_l, ff, 1, loc, 1, "0xff");
	wchar_case(&st_getwchar_l, nul, 3, loc, 1, "nul");
	wchar_case(&st_getwchar_l, utf8, 3, loc, 1, "utf8");
	wchar_case(&st_getwchar_l, pair, 3, loc, 1, "abc");
	wchar_case(&st_getwchar_l, mix, sizeof(mix), loc, 1, "mix");
}

static void
getwchar_random(locale_t loc, long n)
{
	unsigned char data[128];

	for (long t = 0; t < n; t++) {
		std::size_t len = rnd_mod(sizeof(data));

		for (std::size_t i = 0; i < len; i++)
			data[i] = (unsigned char)rnd();
		wchar_case(&st_getwchar, data, len, loc, 0, "random");
	}
}

static void
getwchar_l_random(locale_t loc, long n)
{
	unsigned char data[128];

	for (long t = 0; t < n; t++) {
		std::size_t len = rnd_mod(sizeof(data));

		for (std::size_t i = 0; i < len; i++)
			data[i] = (unsigned char)rnd();
		wchar_case(&st_getwchar_l, data, len, loc, 1, "random");
	}
}

/* ------------------------------------------------------------------------ */

int
main(void)
{
	locale_t loc;
	long total_fails;
	int rc = 0;

	setlocale(LC_ALL, "C.UTF-8");
	loc = newlocale(LC_ALL_MASK, "C.UTF-8", nullptr);
	if (loc == nullptr)
		loc = duplocale(LC_GLOBAL_LOCALE);

	getwchar_edges(loc);
	getwchar_random(loc, 200000);
	getwchar_l_random(loc, 200000);

	if (loc != nullptr && loc != LC_GLOBAL_LOCALE)
		freelocale(loc);

	total_fails = st_getwchar.fails + st_getwchar_l.fails;

	std::printf("\n");
	std::printf("+----------------+----------+----------+\n");
	std::printf("| function       |     cases|    fails |\n");
	std::printf("+----------------+----------+----------+\n");
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_getwchar.name, st_getwchar.cases, st_getwchar.fails);
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_getwchar_l.name, st_getwchar_l.cases, st_getwchar_l.fails);
	std::printf("+----------------+----------+----------+\n");

	if (total_fails != 0)
		rc = 1;
	return rc;
}
