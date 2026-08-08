/*
 * harness.cpp -- differential test for PBSD batch b0091.
 *
 * Every case executes the C++ port and the ref_ oracle on independent but
 * identically-prepared state.  Return values, post-call stream offsets, and
 * the full contents of every caller buffer (guard bytes included) are compared.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <locale.h>
#include <unistd.h>
#include <wchar.h>

import pbsd.lib.libc.stdio.b0091;

namespace port = pbsd::lib_libc_stdio::b0091;

extern "C" {
int ref_fsetpos(FILE *iop, const fpos_t *pos);
wint_t ref_getwchar(void);
wint_t ref_getwchar_l(locale_t locale);
wint_t ref_putwchar(wchar_t wc);
wint_t ref_putwchar_l(wchar_t wc, locale_t locale);
wint_t fgetwc_l(FILE *fp, locale_t loc);
wint_t fputwc_l(wchar_t wc, FILE *fp, locale_t loc);
wint_t putwchar_l(wchar_t wc, locale_t locale);
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

extern "C" wint_t
putwchar_l(wchar_t wc, locale_t locale)
{
	locale_t old = uselocale(locale);
	wint_t w = putwchar(wc);

	uselocale(old);
	return w;
}

/* ------------------------------------------------------------------------ */

static const unsigned char GUARD = 0x7f;
static const int MAX_REPORT = 8;
static const std::size_t FILEBUF = 4096;
static const std::size_t READBUF = 256;
static const std::size_t OUTBUF = 256;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat st_fsetpos = { "fsetpos", 0, 0, 0 };
static Stat st_getwchar = { "getwchar", 0, 0, 0 };
static Stat st_getwchar_l = { "getwchar_l", 0, 0, 0 };
static Stat st_putwchar = { "putwchar", 0, 0, 0 };
static Stat st_putwchar_l = { "putwchar_l", 0, 0, 0 };

static int g_stdout_fd = -1;

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
/* fsetpos                                                                   */
/* ------------------------------------------------------------------------ */

struct FsetposObs {
	int ret;
	off_t pos;
	unsigned char readbuf[READBUF];
};

static FILE *
make_temp_copy(const unsigned char *data, std::size_t len)
{
	char path[] = "/tmp/pbsd_b0091_XXXXXX";
	int fd;
	FILE *fp;

	fd = mkstemp(path);
	if (fd < 0)
		return nullptr;
	if (write(fd, data, len) != (ssize_t)len) {
		close(fd);
		unlink(path);
		return nullptr;
	}
	close(fd);
	fp = fopen(path, "r+b");
	unlink(path);
	return fp;
}

static void
sync_stream(FILE *fp, off_t offset)
{
	std::rewind(fp);
	if (offset > 0)
		(void)fseeko(fp, offset, SEEK_SET);
}

static FsetposObs
run_fsetpos(FILE *fp, fpos_t target, off_t advance_after_save)
{
	FsetposObs obs{};
	fpos_t saved = target;

	std::memset(obs.readbuf, GUARD, sizeof(obs.readbuf));
	sync_stream(fp, (off_t)target);
	if (fgetpos(fp, &saved) != 0) {
		obs.ret = -2;
		return obs;
	}
	if (advance_after_save != 0)
		(void)fseeko(fp, advance_after_save, SEEK_CUR);
	obs.ret = port::fsetpos(fp, &saved);
	obs.pos = ftello(fp);
	if (obs.pos >= 0) {
		std::size_t n = fread(obs.readbuf, 1, sizeof(obs.readbuf), fp);
		(void)n;
	}
	return obs;
}

static FsetposObs
run_ref_fsetpos(FILE *fp, fpos_t target, off_t advance_after_save)
{
	FsetposObs obs{};
	fpos_t saved = target;

	std::memset(obs.readbuf, GUARD, sizeof(obs.readbuf));
	sync_stream(fp, (off_t)target);
	if (fgetpos(fp, &saved) != 0) {
		obs.ret = -2;
		return obs;
	}
	if (advance_after_save != 0)
		(void)fseeko(fp, advance_after_save, SEEK_CUR);
	obs.ret = ref_fsetpos(fp, &saved);
	obs.pos = ftello(fp);
	if (obs.pos >= 0) {
		std::size_t n = fread(obs.readbuf, 1, sizeof(obs.readbuf), fp);
		(void)n;
	}
	return obs;
}

static void
fsetpos_case(const unsigned char *data, std::size_t len, off_t save_at,
    off_t advance, const char *tag)
{
	FILE *pa = make_temp_copy(data, len);
	FILE *pb = make_temp_copy(data, len);
	FsetposObs a, b;
	int bad;

	if (pa == nullptr || pb == nullptr) {
		std::fprintf(stderr, "harness bug: temp file for fsetpos\n");
		std::exit(2);
	}

	a = run_fsetpos(pa, (fpos_t)save_at, advance);
	b = run_ref_fsetpos(pb, (fpos_t)save_at, advance);

	st_fsetpos.cases++;
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (a.pos != b.pos)
		bad = 1;
	if (std::memcmp(a.readbuf, b.readbuf, sizeof(a.readbuf)) != 0)
		bad = 1;
	if (bad) {
		char detail[160];
		std::snprintf(detail, sizeof(detail),
		    "len=%zu save=%lld adv=%lld ret %d/%d pos %lld/%lld",
		    len, (long long)save_at, (long long)advance,
		    a.ret, b.ret, (long long)a.pos, (long long)b.pos);
		stat_fail(&st_fsetpos, tag, detail);
	}

	fclose(pa);
	fclose(pb);
}

static void
fsetpos_direct(const unsigned char *data, std::size_t len, fpos_t pos,
    const char *tag)
{
	FILE *pa = make_temp_copy(data, len);
	FILE *pb = make_temp_copy(data, len);
	FsetposObs a, b;
	int bad;

	if (pa == nullptr || pb == nullptr) {
		std::fprintf(stderr, "harness bug: temp file for fsetpos_direct\n");
		std::exit(2);
	}

	std::memset(a.readbuf, GUARD, sizeof(a.readbuf));
	std::memset(b.readbuf, GUARD, sizeof(b.readbuf));
	sync_stream(pa, 0);
	sync_stream(pb, 0);
	(void)fseeko(pa, (off_t)len, SEEK_SET);
	(void)fseeko(pb, (off_t)len, SEEK_SET);

	a.ret = port::fsetpos(pa, &pos);
	a.pos = ftello(pa);
	if (a.pos >= 0)
		(void)fread(a.readbuf, 1, sizeof(a.readbuf), pa);

	b.ret = ref_fsetpos(pb, &pos);
	b.pos = ftello(pb);
	if (b.pos >= 0)
		(void)fread(b.readbuf, 1, sizeof(b.readbuf), pb);

	st_fsetpos.cases++;
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (a.pos != b.pos)
		bad = 1;
	if (std::memcmp(a.readbuf, b.readbuf, sizeof(a.readbuf)) != 0)
		bad = 1;
	if (bad) {
		char detail[128];
		std::snprintf(detail, sizeof(detail),
		    "direct pos=%lld ret %d/%d stream %lld/%lld",
		    (long long)pos, a.ret, b.ret,
		    (long long)a.pos, (long long)b.pos);
		stat_fail(&st_fsetpos, tag, detail);
	}

	fclose(pa);
	fclose(pb);
}

static void
fsetpos_edges(void)
{
	static const unsigned char empty[] = { "" };
	static const unsigned char one[] = { 'x' };
	static const unsigned char hi[] = {
		0x00, 0x7f, 0x80, 0xff, 0xfe, 0x01, 0x00
	};
	unsigned char buf[FILEBUF];
	std::size_t i;

	std::memset(buf, GUARD, sizeof(buf));
	for (i = 0; i < sizeof(buf); i++)
		buf[i] = (unsigned char)(0x80 + (i & 0x7f));

	fsetpos_case(empty, 0, 0, 0, "empty@0");
	fsetpos_case(one, 1, 0, 0, "one@0");
	fsetpos_case(one, 1, 0, 1, "one@0+adv1");
	fsetpos_case(one, 1, 0, -1, "one@0+adv-1");
	fsetpos_case(hi, sizeof(hi), 0, 0, "hi@0");
	fsetpos_case(hi, sizeof(hi), 3, 2, "hi@3+2");
	fsetpos_case(hi, sizeof(hi), 6, 0, "hi@end");
	fsetpos_case(buf, sizeof(buf), 0, 0, "big@0");
	fsetpos_case(buf, sizeof(buf), 127, 64, "big@127+64");
	fsetpos_case(buf, sizeof(buf), 2047, 1, "big@2047+1");
	fsetpos_case(buf, sizeof(buf), (off_t)sizeof(buf) - 1, 0, "big@last");

	fsetpos_direct(hi, sizeof(hi), (fpos_t)0, "direct0");
	fsetpos_direct(hi, sizeof(hi), (fpos_t)4, "direct4");
	fsetpos_direct(buf, sizeof(buf), (fpos_t)999, "direct999");
	fsetpos_direct(buf, sizeof(buf), (fpos_t)-1, "direct-1");
}

static void
fsetpos_random(long n)
{
	unsigned char data[FILEBUF];

	for (long t = 0; t < n; t++) {
		std::size_t len = 1 + rnd_mod(sizeof(data));
		off_t save_at;
		off_t advance;

		for (std::size_t i = 0; i < len; i++)
			data[i] = (unsigned char)rnd();
		save_at = (off_t)rnd_mod(len);
		advance = (off_t)((long)rnd_mod(17) - 8);
		fsetpos_case(data, len, save_at, advance, "random");
	}
}

/* ------------------------------------------------------------------------ */
/* getwchar / getwchar_l                                                     */
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

	wchar_case(&st_getwchar_l, empty, 0, nullptr, 1, "null-loc");
	wchar_case(&st_getwchar_l, a, 1, nullptr, 1, "null-loc-a");
	wchar_case(&st_getwchar_l, utf8, 3, LC_GLOBAL_LOCALE, 1, "global-loc");
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
	locale_t pick;

	for (long t = 0; t < n; t++) {
		std::size_t len = rnd_mod(sizeof(data));

		for (std::size_t i = 0; i < len; i++)
			data[i] = (unsigned char)rnd();
		switch (rnd_mod(3)) {
		case 0:
			pick = nullptr;
			break;
		case 1:
			pick = LC_GLOBAL_LOCALE;
			break;
		default:
			pick = loc;
			break;
		}
		wchar_case(&st_getwchar_l, data, len, pick, 1, "random");
	}
}

/* ------------------------------------------------------------------------ */
/* putwchar / putwchar_l                                                     */
/* ------------------------------------------------------------------------ */

struct PutwcharObs {
	wint_t ret;
	unsigned char outbuf[OUTBUF];
};

static void
stdout_init(void)
{
	if (g_stdout_fd < 0)
		g_stdout_fd = dup(STDOUT_FILENO);
}

static FILE *
stdout_capture_begin(const char *path)
{
	FILE *cap;

	stdout_init();
	fflush(stdout);
	cap = fopen(path, "w+b");
	if (cap == nullptr)
		return nullptr;
	if (dup2(fileno(cap), STDOUT_FILENO) != 0) {
		fclose(cap);
		return nullptr;
	}
	setvbuf(stdout, nullptr, _IONBF, 0);
	clearerr(stdout);
	return cap;
}

static void
stdout_capture_end(FILE *cap, unsigned char *outbuf, std::size_t outlen)
{
	fflush(stdout);
	if (cap != nullptr) {
		rewind(cap);
		(void)fread(outbuf, 1, outlen, cap);
	}
	dup2(g_stdout_fd, STDOUT_FILENO);
	fflush(stdout);
	if (cap != nullptr)
		fclose(cap);
}

static PutwcharObs
call_putwchar(wchar_t wc)
{
	char path[] = "/tmp/pbsd_b0091_pw_XXXXXX";
	int fd;
	FILE *cap;
	PutwcharObs obs{};

	fd = mkstemp(path);
	if (fd < 0) {
		obs.ret = (wint_t)0xdead;
		return obs;
	}
	close(fd);

	std::memset(obs.outbuf, GUARD, sizeof(obs.outbuf));
	cap = stdout_capture_begin(path);
	if (cap == nullptr) {
		unlink(path);
		obs.ret = (wint_t)0xdead;
		return obs;
	}
	obs.ret = port::putwchar(wc);
	stdout_capture_end(cap, obs.outbuf, sizeof(obs.outbuf));
	unlink(path);
	return obs;
}

static PutwcharObs
call_ref_putwchar(wchar_t wc)
{
	char path[] = "/tmp/pbsd_b0091_pw_XXXXXX";
	int fd;
	FILE *cap;
	PutwcharObs obs{};

	fd = mkstemp(path);
	if (fd < 0) {
		obs.ret = (wint_t)0xdead;
		return obs;
	}
	close(fd);

	std::memset(obs.outbuf, GUARD, sizeof(obs.outbuf));
	cap = stdout_capture_begin(path);
	if (cap == nullptr) {
		unlink(path);
		obs.ret = (wint_t)0xdead;
		return obs;
	}
	obs.ret = ref_putwchar(wc);
	stdout_capture_end(cap, obs.outbuf, sizeof(obs.outbuf));
	unlink(path);
	return obs;
}

static PutwcharObs
call_putwchar_l(wchar_t wc, locale_t loc)
{
	char path[] = "/tmp/pbsd_b0091_pw_XXXXXX";
	int fd;
	FILE *cap;
	PutwcharObs obs{};

	fd = mkstemp(path);
	if (fd < 0) {
		obs.ret = (wint_t)0xdead;
		return obs;
	}
	close(fd);

	std::memset(obs.outbuf, GUARD, sizeof(obs.outbuf));
	cap = stdout_capture_begin(path);
	if (cap == nullptr) {
		unlink(path);
		obs.ret = (wint_t)0xdead;
		return obs;
	}
	obs.ret = port::putwchar_l(wc, loc);
	stdout_capture_end(cap, obs.outbuf, sizeof(obs.outbuf));
	unlink(path);
	return obs;
}

static PutwcharObs
call_ref_putwchar_l(wchar_t wc, locale_t loc)
{
	char path[] = "/tmp/pbsd_b0091_pw_XXXXXX";
	int fd;
	FILE *cap;
	PutwcharObs obs{};

	fd = mkstemp(path);
	if (fd < 0) {
		obs.ret = (wint_t)0xdead;
		return obs;
	}
	close(fd);

	std::memset(obs.outbuf, GUARD, sizeof(obs.outbuf));
	cap = stdout_capture_begin(path);
	if (cap == nullptr) {
		unlink(path);
		obs.ret = (wint_t)0xdead;
		return obs;
	}
	obs.ret = ref_putwchar_l(wc, loc);
	stdout_capture_end(cap, obs.outbuf, sizeof(obs.outbuf));
	unlink(path);
	return obs;
}

static void
putwchar_case(Stat *st, wchar_t wc, locale_t loc, int use_l, const char *tag)
{
	PutwcharObs a, b;
	int bad;

	if (use_l) {
		a = call_putwchar_l(wc, loc);
		b = call_ref_putwchar_l(wc, loc);
	} else {
		a = call_putwchar(wc);
		b = call_ref_putwchar(wc);
	}

	st->cases++;
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (std::memcmp(a.outbuf, b.outbuf, sizeof(a.outbuf)) != 0)
		bad = 1;
	if (bad) {
		char detail[128];
		std::snprintf(detail, sizeof(detail),
		    "wc=%ld ret=%ld/%ld", (long)wc, (long)a.ret, (long)b.ret);
		stat_fail(st, tag, detail);
	}
}

static void
putwchar_edges(locale_t loc)
{
	putwchar_case(&st_putwchar, L'\0', loc, 0, "nul");
	putwchar_case(&st_putwchar, L'a', loc, 0, "a");
	putwchar_case(&st_putwchar, L'z', loc, 0, "z");
	putwchar_case(&st_putwchar, (wchar_t)0x80, loc, 0, "0x80");
	putwchar_case(&st_putwchar, (wchar_t)0xff, loc, 0, "0xff");
	putwchar_case(&st_putwchar, (wchar_t)0xe9, loc, 0, "eacute");
	putwchar_case(&st_putwchar, L'\n', loc, 0, "nl");

	putwchar_case(&st_putwchar_l, L'\0', loc, 1, "nul");
	putwchar_case(&st_putwchar_l, L'a', loc, 1, "a");
	putwchar_case(&st_putwchar_l, L'z', loc, 1, "z");
	putwchar_case(&st_putwchar_l, (wchar_t)0x80, loc, 1, "0x80");
	putwchar_case(&st_putwchar_l, (wchar_t)0xff, loc, 1, "0xff");
	putwchar_case(&st_putwchar_l, (wchar_t)0xe9, loc, 1, "eacute");
	putwchar_case(&st_putwchar_l, L'\n', loc, 1, "nl");

	putwchar_case(&st_putwchar_l, L'a', nullptr, 1, "null-loc");
	putwchar_case(&st_putwchar_l, (wchar_t)0xe9, nullptr, 1, "null-loc-e");
	putwchar_case(&st_putwchar_l, L'!', LC_GLOBAL_LOCALE, 1, "global-loc");
}

static void
putwchar_random(locale_t loc, long n)
{
	locale_t pick;

	for (long t = 0; t < n; t++) {
		wchar_t wc = (wchar_t)(rnd() & 0xffff);
		putwchar_case(&st_putwchar, wc, loc, 0, "random");
	}
}

static void
putwchar_l_random(locale_t loc, long n)
{
	locale_t pick;

	for (long t = 0; t < n; t++) {
		wchar_t wc = (wchar_t)(rnd() & 0xffff);
		switch (rnd_mod(3)) {
		case 0:
			pick = nullptr;
			break;
		case 1:
			pick = LC_GLOBAL_LOCALE;
			break;
		default:
			pick = loc;
			break;
		}
		putwchar_case(&st_putwchar_l, wc, pick, 1, "random");
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

	fsetpos_edges();
	fsetpos_random(200000);

	getwchar_edges(loc);
	getwchar_random(loc, 200000);
	getwchar_l_random(loc, 200000);

	putwchar_edges(loc);
	putwchar_random(loc, 200000);
	putwchar_l_random(loc, 200000);

	if (loc != nullptr && loc != LC_GLOBAL_LOCALE)
		freelocale(loc);

	total_fails = st_fsetpos.fails + st_getwchar.fails + st_getwchar_l.fails +
	    st_putwchar.fails + st_putwchar_l.fails;

	std::printf("\n");
	std::printf("+----------------+----------+----------+\n");
	std::printf("| function       |     cases|    fails |\n");
	std::printf("+----------------+----------+----------+\n");
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_fsetpos.name, st_fsetpos.cases, st_fsetpos.fails);
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
