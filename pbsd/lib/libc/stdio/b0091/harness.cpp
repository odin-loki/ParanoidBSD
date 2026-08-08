/*
 * harness.cpp -- differential test for PBSD batch b0091.
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
wint_t ref_putwchar(wchar_t wc);
wint_t ref_putwchar_l(wchar_t wc, locale_t locale);
wint_t fgetwc_l(FILE *fp, locale_t loc);
wint_t fputwc_l(wchar_t wc, FILE *fp, locale_t loc);
}

extern "C" wint_t
fgetwc_l(FILE *fp, locale_t loc)
{
	locale_t old = uselocale(loc);
	wint_t w = fgetwc(fp);

	uselocale(old);
	return w;
}

extern "C" wint_t
fputwc_l(wchar_t wc, FILE *fp, locale_t loc)
{
	locale_t old = uselocale(loc);
	wint_t w = fputwc(wc, fp);

	uselocale(old);
	return w;
}

/* ------------------------------------------------------------------------ */

static const unsigned char GUARD = 0x7f;
static const int MAX_REPORT = 8;
static const std::size_t OUTBUF = 256;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat st_getwchar = { "getwchar", 0, 0, 0 };
static Stat st_getwchar_l = { "getwchar_l", 0, 0, 0 };
static Stat st_putwchar = { "putwchar", 0, 0, 0 };
static Stat st_putwchar_l = { "putwchar_l", 0, 0, 0 };

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
/* getwchar                                                                  */
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

static int
read_file_bytes(const char *path, unsigned char *buf, std::size_t buflen,
    std::size_t *outlen)
{
	FILE *fp = fopen(path, "rb");
	std::size_t n;

	if (fp == nullptr)
		return -1;
	n = fread(buf, 1, buflen, fp);
	*outlen = n;
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
    locale_t loc, int which, const char *tag)
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

	switch (which) {
	case 0:
		a = call_getwchar_on(path);
		b = call_ref_getwchar_on(path);
		break;
	case 1:
		a = call_getwchar_l_on(path, loc);
		b = call_ref_getwchar_l_on(path, loc);
		break;
	default:
		unlink(path);
		return;
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

/* ------------------------------------------------------------------------ */
/* putwchar                                                                  */
/* ------------------------------------------------------------------------ */

struct PutObs {
	wint_t ret;
	unsigned char out[OUTBUF];
	std::size_t outlen;
};

static PutObs
call_putwchar_on(const char *path, wchar_t wc)
{
	PutObs obs{};

	std::memset(obs.out, GUARD, sizeof(obs.out));
	if (freopen(path, "w", stdout) == nullptr) {
		obs.ret = (wint_t)0xdead;
		return obs;
	}
	obs.ret = port::putwchar(wc);
	fflush(stdout);
	freopen("/dev/null", "w", stdout);
	(void)read_file_bytes(path, obs.out, sizeof(obs.out), &obs.outlen);
	return obs;
}

static PutObs
call_ref_putwchar_on(const char *path, wchar_t wc)
{
	PutObs obs{};

	std::memset(obs.out, GUARD, sizeof(obs.out));
	if (freopen(path, "w", stdout) == nullptr) {
		obs.ret = (wint_t)0xdead;
		return obs;
	}
	obs.ret = ref_putwchar(wc);
	fflush(stdout);
	freopen("/dev/null", "w", stdout);
	(void)read_file_bytes(path, obs.out, sizeof(obs.out), &obs.outlen);
	return obs;
}

static PutObs
call_putwchar_l_on(const char *path, wchar_t wc, locale_t loc)
{
	PutObs obs{};

	std::memset(obs.out, GUARD, sizeof(obs.out));
	if (freopen(path, "w", stdout) == nullptr) {
		obs.ret = (wint_t)0xdead;
		return obs;
	}
	obs.ret = port::putwchar_l(wc, loc);
	fflush(stdout);
	freopen("/dev/null", "w", stdout);
	(void)read_file_bytes(path, obs.out, sizeof(obs.out), &obs.outlen);
	return obs;
}

static PutObs
call_ref_putwchar_l_on(const char *path, wchar_t wc, locale_t loc)
{
	PutObs obs{};

	std::memset(obs.out, GUARD, sizeof(obs.out));
	if (freopen(path, "w", stdout) == nullptr) {
		obs.ret = (wint_t)0xdead;
		return obs;
	}
	obs.ret = ref_putwchar_l(wc, loc);
	fflush(stdout);
	freopen("/dev/null", "w", stdout);
	(void)read_file_bytes(path, obs.out, sizeof(obs.out), &obs.outlen);
	return obs;
}

static void
put_case(Stat *st, wchar_t wc, locale_t loc, int which, const char *tag)
{
	char path[] = "/tmp/pbsd_b0091_pw_XXXXXX";
	int fd;
	PutObs a, b;
	int bad;

	fd = mkstemp(path);
	if (fd < 0) {
		std::fprintf(stderr, "harness bug: mkstemp putwchar\n");
		std::exit(2);
	}
	close(fd);
	unlink(path);

	switch (which) {
	case 0:
		a = call_putwchar_on(path, wc);
		b = call_ref_putwchar_on(path, wc);
		break;
	case 1:
		a = call_putwchar_l_on(path, wc, loc);
		b = call_ref_putwchar_l_on(path, wc, loc);
		break;
	default:
		return;
	}

	st->cases++;
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (a.outlen != b.outlen)
		bad = 1;
	if (std::memcmp(a.out, b.out, sizeof(a.out)) != 0)
		bad = 1;
	if (bad) {
		char detail[128];
		std::snprintf(detail, sizeof(detail),
		    "wc=%ld ret=%ld/%ld len=%zu/%zu", (long)wc,
		    (long)a.ret, (long)b.ret, a.outlen, b.outlen);
		stat_fail(st, tag, detail);
	}

	unlink(path);
}

/* ------------------------------------------------------------------------ */

static void
getwchar_edges(locale_t loc, locale_t null_loc)
{
	static const unsigned char empty[] = { "" };
	static const unsigned char a[] = { 'a' };
	static const unsigned char hi[] = { 0x80 };
	static const unsigned char ff[] = { 0xff };
	static const unsigned char utf8[] = { 0xc3, 0xa9, '!' };
	static const unsigned char mix[] = {
		0x7f, 0x80, 0x81, 0xfe, 0xff, 'x', '\n', 0x00, 'y'
	};

	wchar_case(&st_getwchar, empty, 0, loc, 0, "empty");
	wchar_case(&st_getwchar, a, 1, loc, 0, "a");
	wchar_case(&st_getwchar, hi, 1, loc, 0, "0x80");
	wchar_case(&st_getwchar, ff, 1, loc, 0, "0xff");
	wchar_case(&st_getwchar, utf8, 3, loc, 0, "utf8");
	wchar_case(&st_getwchar, mix, sizeof(mix), loc, 0, "mix");

	wchar_case(&st_getwchar_l, empty, 0, loc, 1, "empty");
	wchar_case(&st_getwchar_l, a, 1, loc, 1, "a");
	wchar_case(&st_getwchar_l, hi, 1, loc, 1, "0x80");
	wchar_case(&st_getwchar_l, ff, 1, loc, 1, "0xff");
	wchar_case(&st_getwchar_l, utf8, 3, loc, 1, "utf8");
	wchar_case(&st_getwchar_l, mix, sizeof(mix), loc, 1, "mix");
	wchar_case(&st_getwchar_l, a, 1, null_loc, 1, "null_loc");
	wchar_case(&st_getwchar_l, a, 1, LC_GLOBAL_LOCALE, 1, "global_loc");
}

static void
putwchar_edges(locale_t loc, locale_t null_loc)
{
	put_case(&st_putwchar, L'a', loc, 0, "a");
	put_case(&st_putwchar, L'z', loc, 0, "z");
	put_case(&st_putwchar, L'\0', loc, 0, "nul");
	put_case(&st_putwchar, (wchar_t)0x80, loc, 0, "0x80");
	put_case(&st_putwchar, (wchar_t)0xff, loc, 0, "0xff");
	put_case(&st_putwchar, (wchar_t)0x7f, loc, 0, "0x7f");
	put_case(&st_putwchar, L'\n', loc, 0, "nl");
	put_case(&st_putwchar, (wchar_t)0xe9, loc, 0, "eacute");

	put_case(&st_putwchar_l, L'a', loc, 1, "a");
	put_case(&st_putwchar_l, L'z', loc, 1, "z");
	put_case(&st_putwchar_l, L'\0', loc, 1, "nul");
	put_case(&st_putwchar_l, (wchar_t)0x80, loc, 1, "0x80");
	put_case(&st_putwchar_l, (wchar_t)0xff, loc, 1, "0xff");
	put_case(&st_putwchar_l, (wchar_t)0x7f, loc, 1, "0x7f");
	put_case(&st_putwchar_l, L'\n', loc, 1, "nl");
	put_case(&st_putwchar_l, (wchar_t)0xe9, loc, 1, "eacute");
	put_case(&st_putwchar_l, L'a', null_loc, 1, "null_loc");
	put_case(&st_putwchar_l, L'a', LC_GLOBAL_LOCALE, 1, "global_loc");
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

static void
putwchar_random(locale_t loc, long n)
{
	for (long t = 0; t < n; t++) {
		wchar_t wc = (wchar_t)rnd();
		put_case(&st_putwchar, wc, loc, 0, "random");
	}
}

static void
putwchar_l_random(locale_t loc, long n)
{
	for (long t = 0; t < n; t++) {
		wchar_t wc = (wchar_t)rnd();
		locale_t use = (rnd() & 3u) == 0 ? (locale_t)0 :
		    (rnd() & 3u) == 1 ? LC_GLOBAL_LOCALE : loc;
		put_case(&st_putwchar_l, wc, use, 1, "random");
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

	getwchar_edges(loc, (locale_t)0);
	putwchar_edges(loc, (locale_t)0);
	getwchar_random(loc, 200000);
	getwchar_l_random(loc, 200000);
	putwchar_random(loc, 200000);
	putwchar_l_random(loc, 200000);

	if (loc != nullptr && loc != LC_GLOBAL_LOCALE)
		freelocale(loc);

	total_fails = st_getwchar.fails + st_getwchar_l.fails +
	    st_putwchar.fails + st_putwchar_l.fails;

	std::printf("\n");
	std::printf("+----------------+----------+----------+\n");
	std::printf("| function       |     cases|    fails |\n");
	std::printf("+----------------+----------+----------+\n");
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_getwchar.name, st_getwchar.cases, st_getwchar.fails);
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_getwchar_l.name, st_getwchar_l.cases, st_getwchar_l.fails);
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_putwchar.name, st_putwchar.cases, st_putwchar.fails);
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_putwchar_l.name, st_putwchar_l.cases, st_putwchar_l.fails);
	std::printf("+----------------+----------+----------+\n");

	if (total_fails != 0)
		rc = 1;
	return rc;
}
