/*
 * harness.cpp -- differential test for PBSD batch b0098.
 *
 * Every case executes the C++ port and the ref_ oracle on independent but
 * identically-prepared state.  Return values, stream output, scanf destinations,
 * and the full contents of every caller buffer (guard bytes included) are
 * compared.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <cstdarg>
#include <limits>
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

/* ------------------------------------------------------------------------ */

static const unsigned char GUARD = 0x7f;
static const int MAX_REPORT = 8;
static const std::size_t GUARD_PAD = 32;
static const std::size_t OUT_CAP = 128;
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

static void
fill_guard(unsigned char *p, std::size_t n)
{
	std::memset(p, GUARD, n);
}

static int
write_file(const char *path, const unsigned char *data, std::size_t len)
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
read_file(const char *path, unsigned char *out, std::size_t cap, std::size_t *len)
{
	FILE *fp = std::fopen(path, "rb");
	std::size_t n;

	if (fp == nullptr)
		return -1;
	n = std::fread(out, 1, cap, fp);
	*len = n;
	return std::fclose(fp);
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
	unsigned char wtail[OUT_CAP];
	unsigned char stream_tail[64];
};

static int
run_port_vwscanf(const char *inpath, const wchar_t *fmt, ScanObs *obs, int kind)
{
	FILE *fp;

	std::memset(obs->wtail, GUARD, sizeof(obs->wtail));
	std::memset(obs->stream_tail, GUARD, sizeof(obs->stream_tail));
	obs->i1 = 0x55555555;
	obs->i2 = 0x66666666;
	obs->u1 = 0x77777777U;
	obs->ll1 = 0x8888888888888888LL;
	obs->w1 = (wchar_t)0x9999;

	fp = std::freopen(inpath, "r", stdin);
	if (fp == nullptr) {
		obs->ret = -9999;
		return -1;
	}
	clearerr(stdin);

	{
		va_list ap;
		va_start(ap, kind);
		switch (kind) {
		case 0:
			obs->ret = port::vwscanf(fmt, ap);
			break;
		case 1:
			obs->ret = port::vwscanf(fmt, ap);
			break;
		case 2:
			obs->ret = port::vwscanf(fmt, ap);
			break;
		case 3:
			obs->ret = port::vwscanf(fmt, ap);
			break;
		case 4:
			obs->ret = port::vwscanf(fmt, ap);
			break;
		case 5:
			obs->ret = port::vwscanf(fmt, ap);
			break;
		default:
			obs->ret = port::vwscanf(fmt, ap);
			break;
		}
		va_end(ap);
	}

	if (std::fread(obs->stream_tail, 1, sizeof(obs->stream_tail), stdin) > 0) { }
	return 0;
}

static int
run_ref_vwscanf(const char *inpath, const wchar_t *fmt, ScanObs *obs, int kind)
{
	FILE *fp;

	std::memset(obs->wtail, GUARD, sizeof(obs->wtail));
	std::memset(obs->stream_tail, GUARD, sizeof(obs->stream_tail));
	obs->i1 = 0x55555555;
	obs->i2 = 0x66666666;
	obs->u1 = 0x77777777U;
	obs->ll1 = 0x8888888888888888LL;
	obs->w1 = (wchar_t)0x9999;

	fp = std::freopen(inpath, "r", stdin);
	if (fp == nullptr) {
		obs->ret = -9999;
		return -1;
	}
	clearerr(stdin);

	{
		va_list ap;
		va_start(ap, kind);
		switch (kind) {
		case 0:
			obs->ret = ref_vwscanf(fmt, ap);
			break;
		case 1:
			obs->ret = ref_vwscanf(fmt, ap);
			break;
		case 2:
			obs->ret = ref_vwscanf(fmt, ap);
			break;
		case 3:
			obs->ret = ref_vwscanf(fmt, ap);
			break;
		case 4:
			obs->ret = ref_vwscanf(fmt, ap);
			break;
		case 5:
			obs->ret = ref_vwscanf(fmt, ap);
			break;
		default:
			obs->ret = ref_vwscanf(fmt, ap);
			break;
		}
		va_end(ap);
	}

	if (std::fread(obs->stream_tail, 1, sizeof(obs->stream_tail), stdin) > 0) { }
	return 0;
}

static int
run_port_vwscanf_l(const char *inpath, locale_t loc, const wchar_t *fmt,
    ScanObs *obs, int kind)
{
	FILE *fp;

	std::memset(obs->wtail, GUARD, sizeof(obs->wtail));
	std::memset(obs->stream_tail, GUARD, sizeof(obs->stream_tail));
	obs->i1 = 0x55555555;
	obs->i2 = 0x66666666;
	obs->u1 = 0x77777777U;
	obs->ll1 = 0x8888888888888888LL;
	obs->w1 = (wchar_t)0x9999;

	fp = std::freopen(inpath, "r", stdin);
	if (fp == nullptr) {
		obs->ret = -9999;
		return -1;
	}
	clearerr(stdin);

	{
		va_list ap;
		va_start(ap, kind);
		switch (kind) {
		case 0:
			obs->ret = port::vwscanf_l(loc, fmt, ap);
			break;
		case 1:
			obs->ret = port::vwscanf_l(loc, fmt, ap);
			break;
		case 2:
			obs->ret = port::vwscanf_l(loc, fmt, ap);
			break;
		case 3:
			obs->ret = port::vwscanf_l(loc, fmt, ap);
			break;
		case 4:
			obs->ret = port::vwscanf_l(loc, fmt, ap);
			break;
		case 5:
			obs->ret = port::vwscanf_l(loc, fmt, ap);
			break;
		default:
			obs->ret = port::vwscanf_l(loc, fmt, ap);
			break;
		}
		va_end(ap);
	}

	if (std::fread(obs->stream_tail, 1, sizeof(obs->stream_tail), stdin) > 0) { }
	return 0;
}

static int
run_ref_vwscanf_l(const char *inpath, locale_t loc, const wchar_t *fmt,
    ScanObs *obs, int kind)
{
	FILE *fp;

	std::memset(obs->wtail, GUARD, sizeof(obs->wtail));
	std::memset(obs->stream_tail, GUARD, sizeof(obs->stream_tail));
	obs->i1 = 0x55555555;
	obs->i2 = 0x66666666;
	obs->u1 = 0x77777777U;
	obs->ll1 = 0x8888888888888888LL;
	obs->w1 = (wchar_t)0x9999;

	fp = std::freopen(inpath, "r", stdin);
	if (fp == nullptr) {
		obs->ret = -9999;
		return -1;
	}
	clearerr(stdin);

	{
		va_list ap;
		va_start(ap, kind);
		switch (kind) {
		case 0:
			obs->ret = ref_vwscanf_l(loc, fmt, ap);
			break;
		case 1:
			obs->ret = ref_vwscanf_l(loc, fmt, ap);
			break;
		case 2:
			obs->ret = ref_vwscanf_l(loc, fmt, ap);
			break;
		case 3:
			obs->ret = ref_vwscanf_l(loc, fmt, ap);
			break;
		case 4:
			obs->ret = ref_vwscanf_l(loc, fmt, ap);
			break;
		case 5:
			obs->ret = ref_vwscanf_l(loc, fmt, ap);
			break;
		default:
			obs->ret = ref_vwscanf_l(loc, fmt, ap);
			break;
		}
		va_end(ap);
	}

	if (std::fread(obs->stream_tail, 1, sizeof(obs->stream_tail), stdin) > 0) { }
	return 0;
}

static int
port_vwscanf_call(const wchar_t *fmt, ScanObs *obs, int kind, ...)
{
	va_list ap;

	std::memset(obs->wtail, GUARD, sizeof(obs->wtail));
	obs->i1 = 0x55555555;
	obs->i2 = 0x66666666;
	obs->u1 = 0x77777777U;
	obs->ll1 = 0x8888888888888888LL;
	obs->w1 = (wchar_t)0x9999;

	va_start(ap, kind);
	obs->ret = port::vwscanf(fmt, ap);
	va_end(ap);
	return obs->ret;
}

static int
ref_vwscanf_call(const wchar_t *fmt, ScanObs *obs, int kind, ...)
{
	va_list ap;

	std::memset(obs->wtail, GUARD, sizeof(obs->wtail));
	obs->i1 = 0x55555555;
	obs->i2 = 0x66666666;
	obs->u1 = 0x77777777U;
	obs->ll1 = 0x8888888888888888LL;
	obs->w1 = (wchar_t)0x9999;

	va_start(ap, kind);
	obs->ret = ref_vwscanf(fmt, ap);
	va_end(ap);
	return obs->ret;
}

static int
port_vwscanf_l_call(locale_t loc, const wchar_t *fmt, ScanObs *obs, int kind,
    ...)
{
	va_list ap;

	std::memset(obs->wtail, GUARD, sizeof(obs->wtail));
	obs->i1 = 0x55555555;
	obs->i2 = 0x66666666;
	obs->u1 = 0x77777777U;
	obs->ll1 = 0x8888888888888888LL;
	obs->w1 = (wchar_t)0x9999;

	va_start(ap, kind);
	obs->ret = port::vwscanf_l(loc, fmt, ap);
	va_end(ap);
	return obs->ret;
}

static int
ref_vwscanf_l_call(locale_t loc, const wchar_t *fmt, ScanObs *obs, int kind,
    ...)
{
	va_list ap;

	std::memset(obs->wtail, GUARD, sizeof(obs->wtail));
	obs->i1 = 0x55555555;
	obs->i2 = 0x66666666;
	obs->u1 = 0x77777777U;
	obs->ll1 = 0x8888888888888888LL;
	obs->w1 = (wchar_t)0x9999;

	va_start(ap, kind);
	obs->ret = ref_vwscanf_l(loc, fmt, ap);
	va_end(ap);
	return obs->ret;
}

static int
scan_open_input(const unsigned char *data, std::size_t len, char *path,
    std::size_t pathsz)
{
	int fd;

	std::snprintf(path, pathsz, "/tmp/pbsd_b0098_in_XXXXXX");
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

typedef int (*scan_exec_fn)(const char *, const wchar_t *, ScanObs *, int, ...);

static void
scan_case_common(Stat *st, const unsigned char *data, std::size_t len,
    const wchar_t *fmt, int kind, scan_exec_fn port_fn, scan_exec_fn ref_fn,
    locale_t loc, const char *tag)
{
	char path[64];
	ScanObs a{}, b{};
	int bad;

	if (scan_open_input(data, len, path, sizeof(path)) != 0) {
		std::fprintf(stderr, "harness bug: scan input\n");
		std::exit(2);
	}

	std::memset(a.stream_tail, GUARD, sizeof(a.stream_tail));
	std::memset(b.stream_tail, GUARD, sizeof(b.stream_tail));

	if (std::freopen(path, "r", stdin) == nullptr) {
		unlink(path);
		std::fprintf(stderr, "harness bug: freopen stdin\n");
		std::exit(2);
	}
	clearerr(stdin);

	switch (kind) {
	case 0:
		port_vwscanf_call(fmt, &a, kind);
		break;
	case 1:
		{
			va_list ap;
			va_start(ap, kind);
			a.ret = (loc == nullptr) ? port::vwscanf(fmt, ap) :
			    port::vwscanf_l(loc, fmt, ap);
			va_end(ap);
		}
		break;
	case 2:
		{
			va_list ap;
			va_start(ap, kind);
			a.ret = (loc == nullptr) ? port::vwscanf(fmt, ap) :
			    port::vwscanf_l(loc, fmt, ap);
			va_end(ap);
		}
		break;
	case 3:
		{
			va_list ap;
			va_start(ap, kind);
			a.ret = (loc == nullptr) ? port::vwscanf(fmt, ap) :
			    port::vwscanf_l(loc, fmt, ap);
			va_end(ap);
		}
		break;
	case 4:
		{
			va_list ap;
			va_start(ap, kind);
			a.ret = (loc == nullptr) ? port::vwscanf(fmt, ap) :
			    port::vwscanf_l(loc, fmt, ap);
			va_end(ap);
		}
		break;
	case 5:
		{
			va_list ap;
			va_start(ap, kind);
			a.ret = (loc == nullptr) ? port::vwscanf(fmt, ap) :
			    port::vwscanf_l(loc, fmt, ap);
			va_end(ap);
		}
		break;
	default:
		port_vwscanf_call(fmt, &a, kind);
		break;
	}
	(void)fread(a.stream_tail, 1, sizeof(a.stream_tail), stdin);

	if (std::freopen(path, "r", stdin) == nullptr) {
		unlink(path);
		std::fprintf(stderr, "harness bug: freopen stdin ref\n");
		std::exit(2);
	}
	clearerr(stdin);

	switch (kind) {
	case 0:
		ref_vwscanf_call(fmt, &b, kind);
		break;
	case 1:
		{
			va_list ap;
			va_start(ap, kind);
			b.ret = (loc == nullptr) ? ref_vwscanf(fmt, ap) :
			    ref_vwscanf_l(loc, fmt, ap);
			va_end(ap);
		}
		break;
	case 2:
		{
			va_list ap;
			va_start(ap, kind);
			b.ret = (loc == nullptr) ? ref_vwscanf(fmt, ap) :
			    ref_vwscanf_l(loc, fmt, ap);
			va_end(ap);
		}
		break;
	case 3:
		{
			va_list ap;
			va_start(ap, kind);
			b.ret = (loc == nullptr) ? ref_vwscanf(fmt, ap) :
			    ref_vwscanf_l(loc, fmt, ap);
			va_end(ap);
		}
		break;
	case 4:
		{
			va_list ap;
			va_start(ap, kind);
			b.ret = (loc == nullptr) ? ref_vwscanf(fmt, ap) :
			    ref_vwscanf_l(loc, fmt, ap);
			va_end(ap);
		}
		break;
	case 5:
		{
			va_list ap;
			va_start(ap, kind);
			b.ret = (loc == nullptr) ? ref_vwscanf(fmt, ap) :
			    ref_vwscanf_l(loc, fmt, ap);
			va_end(ap);
		}
		break;
	default:
		ref_vwscanf_call(fmt, &b, kind);
		break;
	}
	(void)fread(b.stream_tail, 1, sizeof(b.stream_tail), stdin);

	st->cases++;
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (a.i1 != b.i1 || a.i2 != b.i2 || a.u1 != b.u1 || a.ll1 != b.ll1 ||
	    a.w1 != b.w1)
		bad = 1;
	if (std::memcmp(a.wtail, b.wtail, sizeof(a.wtail)) != 0)
		bad = 1;
	if (std::memcmp(a.stream_tail, b.stream_tail, sizeof(a.stream_tail)) != 0)
		bad = 1;
	if (bad) {
		char detail[160];
		std::snprintf(detail, sizeof(detail),
		    "len=%zu kind=%d ret %d/%d i1 %d/%d",
		    len, kind, a.ret, b.ret, a.i1, b.i1);
		stat_fail(st, tag, detail);
	}

	unlink(path);
	(void)port_fn;
	(void)ref_fn;
}

static void
scan_case_d(Stat *st, locale_t loc, const unsigned char *data, std::size_t len,
    const wchar_t *fmt, const char *tag)
{
	char path[64];
	ScanObs a{}, b{};
	int bad;

	if (scan_open_input(data, len, path, sizeof(path)) != 0) {
		std::fprintf(stderr, "harness bug: scan input\n");
		std::exit(2);
	}

	std::memset(a.stream_tail, GUARD, sizeof(a.stream_tail));
	std::memset(b.stream_tail, GUARD, sizeof(b.stream_tail));
	a.i1 = b.i1 = 0x12121212;
	a.i2 = b.i2 = 0x34343434;

	if (std::freopen(path, "r", stdin) == nullptr) {
		unlink(path);
		std::exit(2);
	}
	clearerr(stdin);
	{
		va_list ap;
		va_start(ap, fmt);
		if (loc == nullptr)
			a.ret = port::vwscanf(fmt, ap);
		else
			a.ret = port::vwscanf_l(loc, fmt, ap);
		va_end(ap);
	}
	(void)fread(a.stream_tail, 1, sizeof(a.stream_tail), stdin);

	if (std::freopen(path, "r", stdin) == nullptr) {
		unlink(path);
		std::exit(2);
	}
	clearerr(stdin);
	{
		va_list ap;
		va_start(ap, fmt);
		if (loc == nullptr)
			b.ret = ref_vwscanf(fmt, ap);
		else
			b.ret = ref_vwscanf_l(loc, fmt, ap);
		va_end(ap);
	}
	(void)fread(b.stream_tail, 1, sizeof(b.stream_tail), stdin);

	st->cases++;
	bad = 0;
	if (a.ret != b.ret || a.i1 != b.i1 || a.i2 != b.i2)
		bad = 1;
	if (std::memcmp(a.stream_tail, b.stream_tail, sizeof(a.stream_tail)) != 0)
		bad = 1;
	if (bad)
		stat_fail(st, tag, "scan %d");

	unlink(path);
}

static void
scan_case_u(Stat *st, locale_t loc, const unsigned char *data, std::size_t len,
    const wchar_t *fmt, const char *tag)
{
	char path[64];
	ScanObs a{}, b{};
	int bad;

	if (scan_open_input(data, len, path, sizeof(path)) != 0)
		std::exit(2);

	a.u1 = b.u1 = 0xababababU;

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	{
		va_list ap;
		va_start(ap, fmt);
		if (loc == nullptr)
			a.ret = port::vwscanf(fmt, ap);
		else
			a.ret = port::vwscanf_l(loc, fmt, ap);
		va_end(ap);
	}

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	{
		va_list ap;
		va_start(ap, fmt);
		if (loc == nullptr)
			b.ret = ref_vwscanf(fmt, ap);
		else
			b.ret = ref_vwscanf_l(loc, fmt, ap);
		va_end(ap);
	}

	st->cases++;
	bad = (a.ret != b.ret || a.u1 != b.u1);
	if (bad)
		stat_fail(st, tag, "scan u");
	unlink(path);
}

static void
scan_case_ll(Stat *st, locale_t loc, const unsigned char *data,
    std::size_t len, const wchar_t *fmt, const char *tag)
{
	char path[64];
	ScanObs a{}, b{};
	int bad;

	if (scan_open_input(data, len, path, sizeof(path)) != 0)
		std::exit(2);

	a.ll1 = b.ll1 = 0x1111222233334444LL;

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	{
		va_list ap;
		va_start(ap, fmt);
		if (loc == nullptr)
			a.ret = port::vwscanf(fmt, ap);
		else
			a.ret = port::vwscanf_l(loc, fmt, ap);
		va_end(ap);
	}

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	{
		va_list ap;
		va_start(ap, fmt);
		if (loc == nullptr)
			b.ret = ref_vwscanf(fmt, ap);
		else
			b.ret = ref_vwscanf_l(loc, fmt, ap);
		va_end(ap);
	}

	st->cases++;
	bad = (a.ret != b.ret || a.ll1 != b.ll1);
	if (bad)
		stat_fail(st, tag, "scan ll");
	unlink(path);
}

static void
scan_case_c(Stat *st, locale_t loc, const unsigned char *data, std::size_t len,
    const wchar_t *fmt, const char *tag)
{
	char path[64];
	ScanObs a{}, b{};
	int bad;

	if (scan_open_input(data, len, path, sizeof(path)) != 0)
		std::exit(2);

	a.w1 = b.w1 = (wchar_t)0xcdcd;

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	{
		va_list ap;
		va_start(ap, fmt);
		if (loc == nullptr)
			a.ret = port::vwscanf(fmt, ap);
		else
			a.ret = port::vwscanf_l(loc, fmt, ap);
		va_end(ap);
	}

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	{
		va_list ap;
		va_start(ap, fmt);
		if (loc == nullptr)
			b.ret = ref_vwscanf(fmt, ap);
		else
			b.ret = ref_vwscanf_l(loc, fmt, ap);
		va_end(ap);
	}

	st->cases++;
	bad = (a.ret != b.ret || a.w1 != b.w1);
	if (bad)
		stat_fail(st, tag, "scan c");
	unlink(path);
}

static void
scan_case_ws(Stat *st, locale_t loc, const unsigned char *data, std::size_t len,
    const wchar_t *fmt, const char *tag)
{
	char path[64];
	ScanObs a{}, b{};
	unsigned char abuf[OUT_CAP + 2 * GUARD_PAD];
	unsigned char bbuf[OUT_CAP + 2 * GUARD_PAD];
	wchar_t *aw = (wchar_t *)(abuf + GUARD_PAD);
	wchar_t *bw = (wchar_t *)(bbuf + GUARD_PAD);
	int bad;

	if (scan_open_input(data, len, path, sizeof(path)) != 0)
		std::exit(2);

	fill_guard(abuf, sizeof(abuf));
	fill_guard(bbuf, sizeof(bbuf));

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	{
		va_list ap;
		va_start(ap, fmt);
		if (loc == nullptr)
			a.ret = port::vwscanf(fmt, ap);
		else
			a.ret = port::vwscanf_l(loc, fmt, ap);
		va_end(ap);
	}

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	{
		va_list ap;
		va_start(ap, fmt);
		if (loc == nullptr)
			b.ret = ref_vwscanf(fmt, ap);
		else
			b.ret = ref_vwscanf_l(loc, fmt, ap);
		va_end(ap);
	}

	st->cases++;
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (std::memcmp(abuf, bbuf, sizeof(abuf)) != 0)
		bad = 1;
	if (bad)
		stat_fail(st, tag, "scan ws");
	unlink(path);
}

static void
scan_case_empty(Stat *st, locale_t loc, const char *tag)
{
	static const unsigned char empty[] = { "" };
	char path[64];
	ScanObs a{}, b{};
	int bad;

	if (scan_open_input(empty, 0, path, sizeof(path)) != 0)
		std::exit(2);

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	{
		va_list ap;
		va_start(ap, fmt);
		const wchar_t *fmt = L"";
		if (loc == nullptr)
			a.ret = port::vwscanf(fmt, ap);
		else
			a.ret = port::vwscanf_l(loc, fmt, ap);
		va_end(ap);
	}

	if (std::freopen(path, "r", stdin) == nullptr)
		std::exit(2);
	clearerr(stdin);
	{
		va_list ap;
		const wchar_t *fmt = L"";
		va_start(ap, fmt);
		if (loc == nullptr)
			b.ret = ref_vwscanf(fmt, ap);
		else
			b.ret = ref_vwscanf_l(loc, fmt, ap);
		va_end(ap);
	}

	st->cases++;
	bad = (a.ret != b.ret);
	if (bad)
		stat_fail(st, tag, "empty fmt");
	unlink(path);
}

static void
scan_edges(locale_t loc)
{
	Stat *st = (loc == nullptr) ? &st_vwscanf : &st_vwscanf_l;

	scan_case_empty(st, loc, "empty");
	scan_case_d(st, loc, (const unsigned char *)"42\n", 3, L"%d", "int42");
	scan_case_d(st, loc, (const unsigned char *)"-1 99\n", 6, L"%d %d", "two_int");
	scan_case_d(st, loc, (const unsigned char *)"0\n", 2, L"%d", "zero");
	scan_case_d(st, loc, (const unsigned char *)"2147483647\n", 12, L"%d", "maxint");
	scan_case_u(st, loc, (const unsigned char *)"4294967295\n", 12, L"%u", "maxu");
	scan_case_ll(st, loc, (const unsigned char *)"9223372036854775807\n", 21,
	    L"%lld", "maxll");
	scan_case_c(st, loc, (const unsigned char *)"x", 1, L"%c", "char_x");
	scan_case_c(st, loc, (const unsigned char *)"\xff", 1, L"%c", "char_ff");
	scan_case_c(st, loc, (const unsigned char *)"\x80", 1, L"%c", "char_80");
	scan_case_ws(st, loc, (const unsigned char *)"hello\n", 6, L"%ls", "str");
	scan_case_ws(st, loc, (const unsigned char *)"a\n", 2, L"%ls", "str1");
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
	static const wchar_t *fmts[] = {
		L"%d", L"%u", L"%lld", L"%c", L"%2d", L"%x", L" %d", L"%d %d"
	};
	const int nfmts = (int)(sizeof(fmts) / sizeof(fmts[0]));

	for (long t = 0; t < n; t++) {
		std::size_t len = rnd_mod(sizeof(data));
		const wchar_t *fmt = fmts[rnd_mod((std::size_t)nfmts)];
		int pick = (int)rnd_mod(8);

		for (std::size_t i = 0; i < len; i++)
			data[i] = (unsigned char)rnd();

		switch (pick) {
		case 0:
			scan_case_d(st, loc, data, len, fmt, "random");
			break;
		case 1:
			scan_case_u(st, loc, data, len, fmt, "random");
			break;
		case 2:
			scan_case_ll(st, loc, data, len, fmt, "random");
			break;
		case 3:
			scan_case_c(st, loc, data, len, L"%c", "random");
			break;
		case 4:
			scan_case_ws(st, loc, data, len, L"%ls", "random");
			break;
		case 5:
			scan_case_empty(st, loc, "random");
			break;
		default:
			scan_case_d(st, loc, data, len, L"%d %d", "random");
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
capture_stdout_begin(char *path, std::size_t pathsz)
{
	std::snprintf(path, pathsz, "/tmp/pbsd_b0098_out_XXXXXX");
	return mkstemp(path);
}

static PrintObs
run_port_vwprintf(const wchar_t *fmt, locale_t loc, int use_l, ...)
{
	PrintObs obs{};
	char path[64];
	int fd;
	va_list ap;

	fill_guard(obs.out, sizeof(obs.out));
	obs.out_len = 0;

	fd = capture_stdout_begin(path, sizeof(path));
	if (fd < 0) {
		obs.ret = -9999;
		return obs;
	}
	close(fd);
	if (freopen(path, "w", stdout) == nullptr) {
		unlink(path);
		obs.ret = -9998;
		return obs;
	}

	va_start(ap, use_l);
	if (use_l)
		obs.ret = port::vwprintf_l(loc, fmt, ap);
	else
		obs.ret = port::vwprintf(fmt, ap);
	va_end(ap);

	std::fflush(stdout);
	if (read_file(path, obs.out, sizeof(obs.out), &obs.out_len) != 0)
		obs.ret = -9997;
	unlink(path);
	return obs;
}

static PrintObs
run_ref_vwprintf(const wchar_t *fmt, locale_t loc, int use_l, ...)
{
	PrintObs obs{};
	char path[64];
	int fd;
	va_list ap;

	fill_guard(obs.out, sizeof(obs.out));
	obs.out_len = 0;

	fd = capture_stdout_begin(path, sizeof(path));
	if (fd < 0) {
		obs.ret = -9999;
		return obs;
	}
	close(fd);
	if (freopen(path, "w", stdout) == nullptr) {
		unlink(path);
		obs.ret = -9998;
		return obs;
	}

	va_start(ap, use_l);
	if (use_l)
		obs.ret = ref_vwprintf_l(loc, fmt, ap);
	else
		obs.ret = ref_vwprintf(fmt, ap);
	va_end(ap);

	std::fflush(stdout);
	if (read_file(path, obs.out, sizeof(obs.out), &obs.out_len) != 0)
		obs.ret = -9997;
	unlink(path);
	return obs;
}

static void
print_case(Stat *st, locale_t loc, int use_l, const wchar_t *fmt,
    const char *tag, ...)
{
	PrintObs a{}, b{};
	va_list ap1, ap2;
	char path_a[64], path_b[64];
	int fd, bad;

	fill_guard(a.out, sizeof(a.out));
	fill_guard(b.out, sizeof(b.out));

	fd = capture_stdout_begin(path_a, sizeof(path_a));
	if (fd < 0)
		std::exit(2);
	close(fd);
	fd = capture_stdout_begin(path_b, sizeof(path_b));
	if (fd < 0)
		std::exit(2);
	close(fd);

	if (freopen(path_a, "w", stdout) == nullptr)
		std::exit(2);
	va_start(ap1, tag);
	if (use_l)
		a.ret = port::vwprintf_l(loc, fmt, ap1);
	else
		a.ret = port::vwprintf(fmt, ap1);
	va_end(ap1);
	std::fflush(stdout);
	if (read_file(path_a, a.out, sizeof(a.out), &a.out_len) != 0)
		std::exit(2);

	if (freopen(path_b, "w", stdout) == nullptr)
		std::exit(2);
	va_start(ap2, tag);
	if (use_l)
		b.ret = ref_vwprintf_l(loc, fmt, ap2);
	else
		b.ret = ref_vwprintf(fmt, ap2);
	va_end(ap2);
	std::fflush(stdout);
	if (read_file(path_b, b.out, sizeof(b.out), &b.out_len) != 0)
		std::exit(2);

	st->cases++;
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (a.out_len != b.out_len)
		bad = 1;
	if (std::memcmp(a.out, b.out, sizeof(a.out)) != 0)
		bad = 1;
	if (bad) {
		char detail[128];
		std::snprintf(detail, sizeof(detail), "ret %d/%d len %zu/%zu",
		    a.ret, b.ret, a.out_len, b.out_len);
		stat_fail(st, tag, detail);
	}

	unlink(path_a);
	unlink(path_b);
}

static void
print_case_lit(Stat *st, locale_t loc, int use_l, const wchar_t *fmt,
    const char *tag)
{
	print_case(st, loc, use_l, fmt, tag);
}

static void
print_case_d(Stat *st, locale_t loc, int use_l, const wchar_t *fmt, int v,
    const char *tag)
{
	char path_a[64], path_b[64];
	PrintObs a{}, b{};
	int fd, bad;

	fill_guard(a.out, sizeof(a.out));
	fill_guard(b.out, sizeof(b.out));

	fd = capture_stdout_begin(path_a, sizeof(path_a));
	close(fd);
	fd = capture_stdout_begin(path_b, sizeof(path_b));
	close(fd);

	if (freopen(path_a, "w", stdout) == nullptr)
		std::exit(2);
	{
		va_list ap;
		va_start(ap, v);
		if (use_l)
			a.ret = port::vwprintf_l(loc, fmt, ap);
		else
			a.ret = port::vwprintf(fmt, ap);
		va_end(ap);
	}
	std::fflush(stdout);
	read_file(path_a, a.out, sizeof(a.out), &a.out_len);

	if (freopen(path_b, "w", stdout) == nullptr)
		std::exit(2);
	{
		va_list ap;
		va_start(ap, v);
		if (use_l)
			b.ret = ref_vwprintf_l(loc, fmt, ap);
		else
			b.ret = ref_vwprintf(fmt, ap);
		va_end(ap);
	}
	std::fflush(stdout);
	read_file(path_b, b.out, sizeof(b.out), &b.out_len);

	st->cases++;
	bad = (a.ret != b.ret || a.out_len != b.out_len ||
	    std::memcmp(a.out, b.out, sizeof(a.out)) != 0);
	if (bad)
		stat_fail(st, tag, "print d");
	unlink(path_a);
	unlink(path_b);
}

static void
print_edges(locale_t loc, int use_l)
{
	Stat *st = use_l ? &st_vwprintf_l : &st_vwprintf;

	print_case_lit(st, loc, use_l, L"", "empty");
	print_case_lit(st, loc, use_l, L"hello", "lit");
	print_case_d(st, loc, use_l, L"%d", 0, "zero");
	print_case_d(st, loc, use_l, L"%d", -1, "neg");
	print_case_d(st, loc, use_l, L"%d", 2147483647, "max");
	print_case_d(st, loc, use_l, L"%u", 4294967295U, "maxu");
	print_case_d(st, loc, use_l, L"%x", 0xdeadbeef, "hex");
	print_case_d(st, loc, use_l, L"%d %d", 3, "pair");
	print_case_lit(st, loc, use_l, L"\xff\xfe", "hibytes");
}

static void
print_random(locale_t loc, int use_l, long n)
{
	Stat *st = use_l ? &st_vwprintf_l : &st_vwprintf;
	static const wchar_t *fmts[] = {
		L"%d", L"%u", L"%x", L"%d %d", L"_%d_", L"", L"txt"
	};
	const int nfmts = (int)(sizeof(fmts) / sizeof(fmts[0]));

	for (long t = 0; t < n; t++) {
		const wchar_t *fmt = fmts[rnd_mod((std::size_t)nfmts)];
		int v = (int)(rnd() & 0x7fffffffU);
		int v2 = (int)(rnd() & 0xffffU);

		if (fmt[0] == L'\0') {
			print_case_lit(st, loc, use_l, fmt, "random");
			continue;
		}
		if (std::wcsstr(fmt, L"%d") != nullptr && std::wcsstr(fmt, L"%d %d") != nullptr) {
			char path_a[64], path_b[64];
			PrintObs a{}, b{};
			int fd;

			fill_guard(a.out, sizeof(a.out));
			fill_guard(b.out, sizeof(b.out));
			fd = capture_stdout_begin(path_a, sizeof(path_a));
			close(fd);
			fd = capture_stdout_begin(path_b, sizeof(path_b));
			close(fd);
			if (freopen(path_a, "w", stdout) == nullptr)
				std::exit(2);
			{
				va_list ap;
				va_start(ap, v2);
				if (use_l)
					a.ret = port::vwprintf_l(loc, fmt, ap);
				else
					a.ret = port::vwprintf(fmt, ap);
				va_end(ap);
			}
			fflush(stdout);
			read_file(path_a, a.out, sizeof(a.out), &a.out_len);
			if (freopen(path_b, "w", stdout) == nullptr)
				std::exit(2);
			{
				va_list ap;
				va_start(ap, v2);
				if (use_l)
					b.ret = ref_vwprintf_l(loc, fmt, ap);
				else
					b.ret = ref_vwprintf(fmt, ap);
				va_end(ap);
			}
			fflush(stdout);
			read_file(path_b, b.out, sizeof(b.out), &b.out_len);
			st->cases++;
			if (a.ret != b.ret || a.out_len != b.out_len ||
			    std::memcmp(a.out, b.out, sizeof(a.out)) != 0)
				stat_fail(st, "random", "pair");
			unlink(path_a);
			unlink(path_b);
		} else if (std::wcsstr(fmt, L"%d") != nullptr ||
		    std::wcsstr(fmt, L"%u") != nullptr || std::wcsstr(fmt, L"%x") != nullptr) {
			print_case_d(st, loc, use_l, fmt, v, "random");
		} else {
			print_case_lit(st, loc, use_l, fmt, "random");
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

static FILE *
open_write_pair(const char *pa, const char *pb)
{
	(void)pa;
	(void)pb;
	return nullptr;
}

static void
setbuf_run_one(FILE *fp, void (*fn)(FILE *, char *), char *userbuf,
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

	setbuf_run_one(fa, [](FILE *fp, char *buf) { port::setbuf(fp, buf); },
	    buf_a, writes, nwrites, chunksz, do_fflush, &oa);
	std::memcpy(oa.blob, blob_a, sizeof(oa.blob));

	setbuf_run_one(fb, [](FILE *fp, char *buf) { ref_setbuf(fp, buf); },
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

	print_edges(nullptr, 0);
	print_random(nullptr, 0, 200000);
	print_edges(loc, 1);
	print_random(loc, 1, 200000);

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
