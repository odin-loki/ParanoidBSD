/*
 * harness.cpp -- differential test for PBSD batch b0098.
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <unistd.h>
#include <fcntl.h>
#include <wchar.h>
#include <xlocale.h>

import pbsd.lib.libc.stdio.b0098;

namespace port = pbsd::lib_libc_stdio::b0098;

extern "C" {
int ref_vwscanf(const wchar_t * __restrict fmt, va_list ap);
int ref_vwscanf_l(locale_t locale, const wchar_t * __restrict fmt, va_list ap);
int ref_vwprintf(const wchar_t * __restrict fmt, va_list ap);
int ref_vwprintf_l(locale_t locale, const wchar_t * __restrict fmt, va_list ap);
void ref_setbuf(FILE * __restrict fp, char * __restrict buf);
}

static const unsigned char GUARD = 0x7f;
static const int MAX_REPORT = 8;
static const std::size_t GUARD_PAD = 32;
static const std::size_t WSCAN_CAP = 64;
static const std::size_t FILE_CAP = 4096;

struct Stat {
	const char *name;
	long cases;
	long fails;
	int reported;
};

static Stat st_vwscanf = { "vwscanf", 0, 0, 0 };
static Stat st_vwscanf_l = { "vwscanf_l", 0, 0, 0 };
static Stat st_vwprintf = { "vwprintf", 0, 0, 0 };
static Stat st_vwprintf_l = { "vwprintf_l", 0, 0, 0 };
static Stat st_setbuf = { "setbuf", 0, 0, 0 };

static std::uint64_t rng_state = 0x98b0098cafebabeULL;

static std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

static std::size_t
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

static void
fill_guard(unsigned char *p, std::size_t n)
{
	std::memset(p, GUARD, n);
}

static int
mk_input(const unsigned char *data, std::size_t len, char *path, std::size_t psz)
{
	int fd;

	std::snprintf(path, psz, "/tmp/pbsd_b0098_in_XXXXXX");
	fd = mkstemp(path);
	if (fd < 0)
		return -1;
	if (len > 0 && write(fd, data, len) != (ssize_t)len) {
		close(fd);
		unlink(path);
		return -1;
	}
	close(fd);
	return 0;
}

static int
mk_output(char *path, std::size_t psz)
{
	int fd;

	std::snprintf(path, psz, "/tmp/pbsd_b0098_out_XXXXXX");
	fd = mkstemp(path);
	if (fd < 0)
		return -1;
	close(fd);
	return 0;
}

/* ------------------------------------------------------------------------ */
/* vwscanf / vwscanf_l                                                       */
/* ------------------------------------------------------------------------ */

struct ScanObs {
	int ret;
	int i1;
	int i2;
	unsigned u1;
	long long ll1;
	wchar_t w1;
	unsigned char tail[64];
};

static int
do_port_scan(locale_t loc, const wchar_t *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	if (loc == nullptr)
		r = port::vwscanf(fmt, ap);
	else
		r = port::vwscanf_l(loc, fmt, ap);
	va_end(ap);
	return r;
}

static int
do_ref_scan(locale_t loc, const wchar_t *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	if (loc == nullptr)
		r = ref_vwscanf(fmt, ap);
	else
		r = ref_vwscanf_l(loc, fmt, ap);
	va_end(ap);
	return r;
}

static void
scan_case(Stat *st, locale_t loc, const unsigned char *data, std::size_t len,
    const wchar_t *fmt, const char *tag, ...)
{
	char path[64];
	ScanObs a{}, b{};
	va_list ap_a, ap_b;
	int bad;

	if (mk_input(data, len, path, sizeof(path)) != 0)
		std::exit(2);

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	va_start(ap_a, tag);
	if (loc == nullptr)
		a.ret = port::vwscanf(fmt, ap_a);
	else
		a.ret = port::vwscanf_l(loc, fmt, ap_a);
	va_end(ap_a);
	(void)std::fread(a.tail, 1, sizeof(a.tail), stdin);

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	va_start(ap_b, tag);
	if (loc == nullptr)
		b.ret = ref_vwscanf(fmt, ap_b);
	else
		b.ret = ref_vwscanf_l(loc, fmt, ap_b);
	va_end(ap_b);
	(void)std::fread(b.tail, 1, sizeof(b.tail), stdin);

	st->cases++;
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (std::memcmp(a.tail, b.tail, sizeof(a.tail)) != 0)
		bad = 1;
	if (bad)
		stat_fail(st, tag, "ret/tail");
	unlink(path);
}

static void
scan_case_d(Stat *st, locale_t loc, const unsigned char *data, std::size_t len,
    const wchar_t *fmt, const char *tag)
{
	char path[64];
	ScanObs a{}, b{};
	int bad;

	if (mk_input(data, len, path, sizeof(path)) != 0)
		std::exit(2);

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	a.ret = do_port_scan(loc, fmt, &a.i1);

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	b.ret = do_ref_scan(loc, fmt, &b.i1);

	st->cases++;
	bad = (a.ret != b.ret || a.i1 != b.i1);
	if (bad)
		stat_fail(st, tag, "d");
	unlink(path);
}

static void
scan_case_d2(Stat *st, locale_t loc, const unsigned char *data, std::size_t len,
    const char *tag)
{
	char path[64];
	ScanObs a{}, b{};
	int bad;

	if (mk_input(data, len, path, sizeof(path)) != 0)
		std::exit(2);

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	a.ret = do_port_scan(loc, L"%d %d", &a.i1, &a.i2);

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	b.ret = do_ref_scan(loc, L"%d %d", &b.i1, &b.i2);

	st->cases++;
	bad = (a.ret != b.ret || a.i1 != b.i1 || a.i2 != b.i2);
	if (bad)
		stat_fail(st, tag, "d2");
	unlink(path);
}

static void
scan_case_u(Stat *st, locale_t loc, const unsigned char *data, std::size_t len,
    const char *tag)
{
	char path[64];
	ScanObs a{}, b{};
	int bad;

	if (mk_input(data, len, path, sizeof(path)) != 0)
		std::exit(2);

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	a.ret = do_port_scan(loc, L"%u", &a.u1);

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	b.ret = do_ref_scan(loc, L"%u", &b.u1);

	st->cases++;
	bad = (a.ret != b.ret || a.u1 != b.u1);
	if (bad)
		stat_fail(st, tag, "u");
	unlink(path);
}

static void
scan_case_ll(Stat *st, locale_t loc, const unsigned char *data, std::size_t len,
    const char *tag)
{
	char path[64];
	ScanObs a{}, b{};
	int bad;

	if (mk_input(data, len, path, sizeof(path)) != 0)
		std::exit(2);

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	a.ret = port_vwscanf_l(loc, L"%lld", &a.ll1);

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	b.ret = ref_vwscanf_l(loc, L"%lld", &b.ll1);

	st->cases++;
	bad = (a.ret != b.ret || a.ll1 != b.ll1);
	if (bad)
		stat_fail(st, tag, "ll");
	unlink(path);
}

static void
scan_case_c(Stat *st, locale_t loc, const unsigned char *data, std::size_t len,
    const char *tag)
{
	char path[64];
	ScanObs a{}, b{};
	int bad;

	if (mk_input(data, len, path, sizeof(path)) != 0)
		std::exit(2);

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	a.ret = port_vwscanf_l(loc, L"%c", &a.w1);

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	b.ret = ref_vwscanf_l(loc, L"%c", &b.w1);

	st->cases++;
	bad = (a.ret != b.ret || a.w1 != b.w1);
	if (bad)
		stat_fail(st, tag, "c");
	unlink(path);
}

static void
scan_case_ws(Stat *st, locale_t loc, const unsigned char *data, std::size_t len,
    const char *tag)
{
	char path[64];
	unsigned char abuf[WSCAN_CAP * sizeof(wchar_t) + 2 * GUARD_PAD];
	unsigned char bbuf[WSCAN_CAP * sizeof(wchar_t) + 2 * GUARD_PAD];
	wchar_t *aw = (wchar_t *)(abuf + GUARD_PAD);
	wchar_t *bw = (wchar_t *)(bbuf + GUARD_PAD);
	int ra, rb, bad;

	if (mk_input(data, len, path, sizeof(path)) != 0)
		std::exit(2);

	fill_guard(abuf, sizeof(abuf));
	fill_guard(bbuf, sizeof(bbuf));

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	ra = port_vwscanf_l(loc, L"%ls", aw);

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	rb = ref_vwscanf_l(loc, L"%ls", bw);

	st->cases++;
	bad = (ra != rb || std::memcmp(abuf, bbuf, sizeof(abuf)) != 0);
	if (bad)
		stat_fail(st, tag, "ws");
	unlink(path);
}

static void
scan_edges(locale_t loc)
{
	Stat *st = (loc == nullptr) ? &st_vwscanf : &st_vwscanf_l;

	scan_case(st, loc, (const unsigned char *)"", 0, L"", "empty");
	scan_case_d(st, loc, (const unsigned char *)"42\n", 3, L"%d", "int42");
	scan_case_d2(st, loc, (const unsigned char *)"-1 99\n", 6, "two_int");
	scan_case_d(st, loc, (const unsigned char *)"0\n", 2, L"%d", "zero");
	scan_case_d(st, loc, (const unsigned char *)"2147483647\n", 12, L"%d", "maxint");
	scan_case_u(st, loc, (const unsigned char *)"4294967295\n", 12, "maxu");
	scan_case_ll(st, loc, (const unsigned char *)"9223372036854775807\n", 21, "maxll");
	scan_case_c(st, loc, (const unsigned char *)"x", 1, "char_x");
	scan_case_c(st, loc, (const unsigned char *)"\xff", 1, "char_ff");
	scan_case_c(st, loc, (const unsigned char *)"\x80", 1, "char_80");
	scan_case_ws(st, loc, (const unsigned char *)"hello\n", 6, "str");
	scan_case_ws(st, loc, (const unsigned char *)"a\n", 2, "str1");
	scan_case_d(st, loc, (const unsigned char *)"", 0, L"%d", "eof_int");
	scan_case_d(st, loc, (const unsigned char *)"abc\n", 4, L"%d", "bad_int");
	scan_case_d(st, loc, (const unsigned char *)"  \t7\n", 5, L"%d", "ws7");
	scan_case_d(st, loc, (const unsigned char *)"0x2a\n", 5, L"%x", "hex");
}

static void
scan_random(locale_t loc, long n)
{
	Stat *st = (loc == nullptr) ? &st_vwscanf : &st_vwscanf_l;
	unsigned char data[256];

	for (long t = 0; t < n; t++) {
		std::size_t len = rnd_mod(sizeof(data));
		int pick = (int)rnd_mod(7);

		for (std::size_t i = 0; i < len; i++)
			data[i] = (unsigned char)rnd();

		switch (pick) {
		case 0:
			scan_case_d(st, loc, data, len, L"%d", "random");
			break;
		case 1:
			scan_case_u(st, loc, data, len, "random");
			break;
		case 2:
			scan_case_ll(st, loc, data, len, "random");
			break;
		case 3:
			scan_case_c(st, loc, data, len, "random");
			break;
		case 4:
			scan_case_ws(st, loc, data, len, "random");
			break;
		case 5:
			scan_case(st, loc, data, len, L"", "random");
			break;
		default:
			scan_case_d2(st, loc, data, len, "random");
			break;
		}
	}
}

/* ------------------------------------------------------------------------ */
/* vwprintf / vwprintf_l                                                   */
/* ------------------------------------------------------------------------ */

struct PrintObs {
	int ret;
	unsigned char out[FILE_CAP];
	std::size_t out_len;
};

static int
port_vwprintf_l(locale_t loc, const wchar_t *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	if (loc == nullptr)
		r = port::vwprintf(fmt, ap);
	else
		r = port::vwprintf_l(loc, fmt, ap);
	va_end(ap);
	return r;
}

static int
ref_vwprintf_l(locale_t loc, const wchar_t *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	if (loc == nullptr)
		r = ref_vwprintf(fmt, ap);
	else
		r = ref_vwprintf_l(loc, fmt, ap);
	va_end(ap);
	return r;
}

static void
print_run(locale_t loc, const char *path, const wchar_t *fmt, PrintObs *obs, ...)
{
	va_list ap;

	fill_guard(obs->out, sizeof(obs->out));
	obs->out_len = 0;

	if (freopen(path, "w", stdout) == nullptr) {
		obs->ret = -9999;
		return;
	}
	va_start(ap, obs);
	if (loc == nullptr)
		obs->ret = port::vwprintf(fmt, ap);
	else
		obs->ret = port::vwprintf_l(loc, fmt, ap);
	va_end(ap);
	std::fflush(stdout);
	if (read_output_file(path, obs->out, sizeof(obs->out), &obs->out_len) != 0)
		obs->ret = -9998;
}

static void
print_run_ref(locale_t loc, const char *path, const wchar_t *fmt, PrintObs *obs,
    ...)
{
	va_list ap;

	fill_guard(obs->out, sizeof(obs->out));
	obs->out_len = 0;

	if (freopen(path, "w", stdout) == nullptr) {
		obs->ret = -9999;
		return;
	}
	va_start(ap, obs);
	if (loc == nullptr)
		obs->ret = ref_vwprintf(fmt, ap);
	else
		obs->ret = ref_vwprintf_l(loc, fmt, ap);
	va_end(ap);
	std::fflush(stdout);
	if (read_output_file(path, obs->out, sizeof(obs->out), &obs->out_len) != 0)
		obs->ret = -9998;
}

static int
read_output_file(const char *path, unsigned char *out, std::size_t cap,
    std::size_t *len)
{
	FILE *fp = std::fopen(path, "rb");
	std::size_t n;

	if (fp == nullptr)
		return -1;
	n = std::fread(out, 1, cap, fp);
	*len = n;
	return std::fclose(fp);
}

static void
print_case(Stat *st, locale_t loc, const wchar_t *fmt, const char *tag, ...)
{
	char path_a[64], path_b[64];
	PrintObs a{}, b{};
	va_list ap_a, ap_b;
	int bad;

	if (mk_output(path_a, sizeof(path_a)) != 0 ||
	    mk_output(path_b, sizeof(path_b)) != 0)
		std::exit(2);

	fill_guard(a.out, sizeof(a.out));
	fill_guard(b.out, sizeof(b.out));

	if (freopen(path_a, "w", stdout) == nullptr)
		std::exit(2);
	va_start(ap_a, tag);
	if (loc == nullptr)
		a.ret = port::vwprintf(fmt, ap_a);
	else
		a.ret = port::vwprintf_l(loc, fmt, ap_a);
	va_end(ap_a);
	std::fflush(stdout);
	read_output_file(path_a, a.out, sizeof(a.out), &a.out_len);

	if (freopen(path_b, "w", stdout) == nullptr)
		std::exit(2);
	va_start(ap_b, tag);
	if (loc == nullptr)
		b.ret = ref_vwprintf(fmt, ap_b);
	else
		b.ret = ref_vwprintf_l(loc, fmt, ap_b);
	va_end(ap_b);
	std::fflush(stdout);
	read_output_file(path_b, b.out, sizeof(b.out), &b.out_len);

	st->cases++;
	bad = 0;
	if (a.ret != b.ret || a.out_len != b.out_len)
		bad = 1;
	if (std::memcmp(a.out, b.out, sizeof(a.out)) != 0)
		bad = 1;
	if (bad)
		stat_fail(st, tag, "print");
	unlink(path_a);
	unlink(path_b);
}

static void
print_case_d(Stat *st, locale_t loc, const wchar_t *fmt, int v, const char *tag)
{
	char path_a[64], path_b[64];
	PrintObs a{}, b{};
	int bad;

	if (mk_output(path_a, sizeof(path_a)) != 0 ||
	    mk_output(path_b, sizeof(path_b)) != 0)
		std::exit(2);

	fill_guard(a.out, sizeof(a.out));
	fill_guard(b.out, sizeof(b.out));

	if (freopen(path_a, "w", stdout) == nullptr)
		std::exit(2);
	a.ret = port_vwprintf_l(loc, fmt, v);
	std::fflush(stdout);
	read_output_file(path_a, a.out, sizeof(a.out), &a.out_len);

	if (freopen(path_b, "w", stdout) == nullptr)
		std::exit(2);
	b.ret = ref_vwprintf_l(loc, fmt, v);
	std::fflush(stdout);
	read_output_file(path_b, b.out, sizeof(b.out), &b.out_len);

	st->cases++;
	bad = (a.ret != b.ret || a.out_len != b.out_len ||
	    std::memcmp(a.out, b.out, sizeof(a.out)) != 0);
	if (bad)
		stat_fail(st, tag, "print_d");
	unlink(path_a);
	unlink(path_b);
}

static void
print_case_d2(Stat *st, locale_t loc, int v1, int v2, const char *tag)
{
	char path_a[64], path_b[64];
	PrintObs a{}, b{};
	int bad;

	if (mk_output(path_a, sizeof(path_a)) != 0 ||
	    mk_output(path_b, sizeof(path_b)) != 0)
		std::exit(2);

	fill_guard(a.out, sizeof(a.out));
	fill_guard(b.out, sizeof(b.out));

	if (freopen(path_a, "w", stdout) == nullptr)
		std::exit(2);
	a.ret = port_vwprintf_l(loc, L"%d %d", v1, v2);
	std::fflush(stdout);
	read_output_file(path_a, a.out, sizeof(a.out), &a.out_len);

	if (freopen(path_b, "w", stdout) == nullptr)
		std::exit(2);
	b.ret = ref_vwprintf_l(loc, L"%d %d", v1, v2);
	std::fflush(stdout);
	read_output_file(path_b, b.out, sizeof(b.out), &b.out_len);

	st->cases++;
	bad = (a.ret != b.ret || a.out_len != b.out_len ||
	    std::memcmp(a.out, b.out, sizeof(a.out)) != 0);
	if (bad)
		stat_fail(st, tag, "print_d2");
	unlink(path_a);
	unlink(path_b);
}

static void
print_edges(locale_t loc)
{
	Stat *st = (loc == nullptr) ? &st_vwprintf : &st_vwprintf_l;

	print_case(st, loc, L"", "empty");
	print_case(st, loc, L"hello", "lit");
	print_case_d(st, loc, L"%d", 0, "zero");
	print_case_d(st, loc, L"%d", -1, "neg");
	print_case_d(st, loc, L"%d", 2147483647, "max");
	print_case_d(st, loc, L"%u", 4294967295U, "maxu");
	print_case_d(st, loc, L"%x", 0xdeadbeef, "hex");
	print_case_d2(st, loc, 3, 7, "pair");
	print_case(st, loc, L"\xff\xfe", "hibytes");
}

static void
print_random(locale_t loc, long n)
{
	Stat *st = (loc == nullptr) ? &st_vwprintf : &st_vwprintf_l;

	for (long t = 0; t < n; t++) {
		int pick = (int)rnd_mod(6);
		int v = (int)(rnd() & 0x7fffffffU);
		int v2 = (int)(rnd() & 0xffffU);

		switch (pick) {
		case 0:
			print_case_d(st, loc, L"%d", v, "random");
			break;
		case 1:
			print_case_d(st, loc, L"%u", (unsigned)v, "random");
			break;
		case 2:
			print_case_d(st, loc, L"%x", v, "random");
			break;
		case 3:
			print_case_d2(st, loc, v, v2, "random");
			break;
		case 4:
			print_case(st, loc, L"", "random");
			break;
		default:
			print_case(st, loc, L"txt", "random");
			break;
		}
	}
}

/* ------------------------------------------------------------------------ */
/* setbuf                                                                    */
/* ------------------------------------------------------------------------ */

struct SetbufObs {
	unsigned char filebuf[FILE_CAP];
	std::size_t file_len;
	unsigned char blob[BUFSIZ + 2 * GUARD_PAD];
	int err;
	int eof;
	long pos;
};

static void
setbuf_run(FILE *fp, void (*fn)(FILE *, char *), char *userbuf,
    const unsigned char *writes, std::size_t nwrites, std::size_t chunksz,
    int do_fflush, SetbufObs *obs)
{
	std::size_t i;

	fill_guard(obs->blob, sizeof(obs->blob));
	if (userbuf != nullptr)
		std::memset(userbuf, 0xa5, BUFSIZ);
	fn(fp, userbuf);

	for (i = 0; i < nwrites; i += chunksz) {
		std::size_t n = chunksz;

		if (i + n > nwrites)
			n = nwrites - i;
		if (n == 0)
			break;
		(void)std::fwrite(writes + i, 1, n, fp);
	}
	if (do_fflush)
		(void)std::fflush(fp);

	obs->err = std::ferror(fp) ? 1 : 0;
	obs->eof = std::feof(fp) ? 1 : 0;
	obs->pos = std::ftell(fp);
	std::rewind(fp);
	obs->file_len = std::fread(obs->filebuf, 1, sizeof(obs->filebuf), fp);
}

static void
setbuf_case(const unsigned char *writes, std::size_t nwrites, std::size_t chunksz,
    int use_userbuf, int do_fflush, const char *tag)
{
	char path_a[64], path_b[64];
	FILE *fa, *fb;
	unsigned char blob_a[BUFSIZ + 2 * GUARD_PAD];
	unsigned char blob_b[BUFSIZ + 2 * GUARD_PAD];
	char *buf_a = use_userbuf ? (char *)(blob_a + GUARD_PAD) : nullptr;
	char *buf_b = use_userbuf ? (char *)(blob_b + GUARD_PAD) : nullptr;
	SetbufObs oa{}, ob{};
	int bad;

	std::snprintf(path_a, sizeof(path_a), "/tmp/pbsd_b0098_sb_a_XXXXXX");
	std::snprintf(path_b, sizeof(path_b), "/tmp/pbsd_b0098_sb_b_XXXXXX");
	if (mkstemp(path_a) < 0 || mkstemp(path_b) < 0)
		std::exit(2);

	fa = std::fopen(path_a, "w+b");
	fb = std::fopen(path_b, "w+b");
	if (fa == nullptr || fb == nullptr)
		std::exit(2);

	fill_guard(blob_a, sizeof(blob_a));
	fill_guard(blob_b, sizeof(blob_b));
	fill_guard(oa.filebuf, sizeof(oa.filebuf));
	fill_guard(ob.filebuf, sizeof(ob.filebuf));

	setbuf_run(fa, [](FILE *fp, char *buf) { port::setbuf(fp, buf); },
	    buf_a, writes, nwrites, chunksz, do_fflush, &oa);
	std::memcpy(oa.blob, blob_a, sizeof(oa.blob));

	setbuf_run(fb, [](FILE *fp, char *buf) { ref_setbuf(fp, buf); },
	    buf_b, writes, nwrites, chunksz, do_fflush, &ob);
	std::memcpy(ob.blob, blob_b, sizeof(ob.blob));

	st_setbuf.cases++;
	bad = 0;
	if (oa.file_len != ob.file_len)
		bad = 1;
	if (std::memcmp(oa.filebuf, ob.filebuf, sizeof(oa.filebuf)) != 0)
		bad = 1;
	if (std::memcmp(oa.blob, ob.blob, sizeof(oa.blob)) != 0)
		bad = 1;
	if (oa.err != ob.err || oa.eof != ob.eof || oa.pos != ob.pos)
		bad = 1;
	if (bad) {
		char detail[160];
		std::snprintf(detail, sizeof(detail),
		    "n=%zu chunk=%zu user=%d fflush=%d len %zu/%zu pos %ld/%ld",
		    nwrites, chunksz, use_userbuf, do_fflush,
		    oa.file_len, ob.file_len, oa.pos, ob.pos);
		stat_fail(&st_setbuf, tag, detail);
	}

	std::fclose(fa);
	std::fclose(fb);
	unlink(path_a);
	unlink(path_b);
}

static void
setbuf_edges(void)
{
	static const unsigned char empty[] = { "" };
	static const unsigned char one[] = { 'x' };
	static const unsigned char hi[] = { 0x00, 0x7f, 0x80, 0xff };
	unsigned char big[FILE_CAP];
	std::size_t i;

	for (i = 0; i < sizeof(big); i++)
		big[i] = (unsigned char)(0x80 + (i & 0x7f));

	setbuf_case(empty, 0, 1, 0, 0, "null_empty");
	setbuf_case(empty, 0, 1, 1, 0, "user_empty");
	setbuf_case(one, 1, 1, 0, 0, "null_one");
	setbuf_case(one, 1, 1, 1, 0, "user_one");
	setbuf_case(one, 1, 1, 0, 1, "null_one_ff");
	setbuf_case(one, 1, 1, 1, 1, "user_one_ff");
	setbuf_case(hi, sizeof(hi), 1, 0, 0, "null_hi");
	setbuf_case(hi, sizeof(hi), 2, 1, 0, "user_hi");
	setbuf_case(big, 64, 1, 0, 0, "null_64");
	setbuf_case(big, 64, 8, 1, 0, "user_64");
	setbuf_case(big, sizeof(big), 1, 0, 1, "null_big_ff");
	setbuf_case(big, sizeof(big), 16, 1, 1, "user_big_ff");
	setbuf_case(big, BUFSIZ + 64, 1, 0, 0, "null_over");
	setbuf_case(big, BUFSIZ + 64, 32, 1, 1, "user_over_ff");
}

static void
setbuf_random(long n)
{
	unsigned char data[FILE_CAP];

	for (long t = 0; t < n; t++) {
		std::size_t len = rnd_mod(sizeof(data));
		std::size_t chunk = 1 + rnd_mod(64);
		int user = (int)(rnd() & 1);
		int flush = (int)(rnd() & 1);

		for (std::size_t i = 0; i < len; i++)
			data[i] = (unsigned char)rnd();
		setbuf_case(data, len, chunk, user, flush, "random");
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

	scan_edges(nullptr);
	scan_random(nullptr, 200000);
	scan_edges(loc);
	scan_random(loc, 200000);

	print_edges(nullptr);
	print_random(nullptr, 200000);
	print_edges(loc);
	print_random(loc, 200000);

	setbuf_edges();
	setbuf_random(200000);

	if (loc != nullptr && loc != LC_GLOBAL_LOCALE)
		freelocale(loc);

	total_fails = st_vwscanf.fails + st_vwscanf_l.fails + st_vwprintf.fails +
	    st_vwprintf_l.fails + st_setbuf.fails;

	std::printf("\n");
	std::printf("+----------------+----------+----------+\n");
	std::printf("| function       |     cases|    fails |\n");
	std::printf("+----------------+----------+----------+\n");
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_vwscanf.name, st_vwscanf.cases, st_vwscanf.fails);
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_vwscanf_l.name, st_vwscanf_l.cases, st_vwscanf_l.fails);
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_vwprintf.name, st_vwprintf.cases, st_vwprintf.fails);
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_vwprintf_l.name, st_vwprintf_l.cases, st_vwprintf_l.fails);
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_setbuf.name, st_setbuf.cases, st_setbuf.fails);
	std::printf("+----------------+----------+----------+\n");

	if (total_fails != 0)
		rc = 1;
	return rc;
}
