/*
 * harness.cpp -- differential test for PBSD batch b0143.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cerrno>
#include <cstdarg>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <fcntl.h>
#include <unistd.h>

#ifndef LC_GLOBAL_LOCALE
#define LC_GLOBAL_LOCALE ((locale_t)-1)
#endif

import pbsd.lib.libc.stdio.b0143;

namespace P = pbsd::lib_libc_stdio::b0143;

extern "C" {

typedef struct {
	unsigned char *_p;
	int _r;
	int _w;
	short _flags;
	short _file;
	struct {
		unsigned char *_base;
		int _size;
	} _bf;
	int _lbfsize;
	void *_cookie;
	int (*_close)(void *);
	int (*_read)(void *, char *, int);
	fpos_t (*_seek)(void *, fpos_t, int);
	int (*_write)(void *, const char *, int);
	struct {
		unsigned char *_base;
		int _size;
	} _ub;
	unsigned char *_up;
	int _ur;
	unsigned char _ubuf[3];
	unsigned char _nbuf[1];
	struct {
		unsigned char *_base;
		int _size;
	} _lb;
	int _blksize;
	fpos_t _offset;
	void *_fl_mutex;
	void *_fl_owner;
	int _fl_count;
	int _orientation;
	mbstate_t _mbstate;
	int _flags2;
} ref_FILE;

int		ref_vswscanf_l(const wchar_t * __restrict, void *,
		    const wchar_t * __restrict, va_list);
int		ref_vswscanf(const wchar_t * __restrict,
		    const wchar_t * __restrict, va_list);
ref_FILE	*ref_fopen(const char * __restrict, const char * __restrict);
int		ref_vswprintf_l(wchar_t * __restrict, size_t, void *,
		    const wchar_t * __restrict, va_list);
int		ref_vswprintf(wchar_t * __restrict, size_t,
		    const wchar_t * __restrict, va_list);

void		b0143_set_sfp_target(ref_FILE *);
void		b0143_set_open_hook(int, int);
int		_close(int);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr long SWEEP = 200000;
constexpr int MAX_REPORT = 8;

#define	__SRD	0x0004
#define	__SWR	0x0008
#define	__SRW	0x0010
#define	__S2OAP	0x0001

enum StatId {
	S_VSWSCANF_L,
	S_VSWSCANF,
	S_FOPEN,
	S_VSWPRINTF_L,
	S_VSWPRINTF,
	NSTAT
};

struct Stats {
	const char *name;
	long long cases;
	long long fails;
	int printed;
};

Stats g_stat[NSTAT] = {
	{ "vswscanf_l", 0, 0, 0 },
	{ "vswscanf", 0, 0, 0 },
	{ "fopen", 0, 0, 0 },
	{ "vswprintf_l", 0, 0, 0 },
	{ "vswprintf", 0, 0, 0 },
};

std::uint64_t rng = 0xb0143feedfaceULL;

std::uint64_t
rnd(void)
{
	std::uint64_t z = (rng += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

std::size_t
rnd_mod(std::size_t m)
{
	return m == 0 ? 0 : (std::size_t)(rnd() % (std::uint64_t)m);
}

void
case_inc(StatId which)
{
	g_stat[which].cases++;
}

void
fail_msg(StatId which, const char *label, const char *detail)
{
	Stats &s = g_stat[which];

	s.fails++;
	if (s.printed < MAX_REPORT) {
		std::fprintf(stderr, "FAIL %s [%s]: %s\n", s.name, label, detail);
		s.printed++;
	}
}

void
fill_guard(unsigned char *p, std::size_t n)
{
	for (std::size_t i = 0; i < n; i++)
		p[i] = GUARD;
}

void
fill_guard_w(wchar_t *p, std::size_t n)
{
	for (std::size_t i = 0; i < n; i++)
		p[i] = (wchar_t)GUARD;
}

bool
guard_eq(const unsigned char *p, std::size_t n)
{
	for (std::size_t i = 0; i < n; i++)
		if (p[i] != GUARD)
			return false;
	return true;
}

bool
guard_eq_w(const wchar_t *p, std::size_t n)
{
	for (std::size_t i = 0; i < n; i++)
		if (p[i] != (wchar_t)GUARD)
			return false;
	return true;
}

bool
fp_fields_eq(const P::FILE &p, const ref_FILE &r)
{
	if (p._flags != r._flags)
		return false;
	if (p._flags2 != r._flags2)
		return false;
	if (p._file != r._file)
		return false;
	return true;
}

int
port_vswscanf_l_va(const wchar_t *str, const wchar_t *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = P::vswscanf_l(str, LC_GLOBAL_LOCALE, fmt, ap);
	va_end(ap);
	return r;
}

int
ref_vswscanf_l_va(const wchar_t *str, const wchar_t *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = ref_vswscanf_l(str, LC_GLOBAL_LOCALE, fmt, ap);
	va_end(ap);
	return r;
}

int
port_vswscanf_va(const wchar_t *str, const wchar_t *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = P::vswscanf(str, fmt, ap);
	va_end(ap);
	return r;
}

int
ref_vswscanf_va(const wchar_t *str, const wchar_t *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = ref_vswscanf(str, fmt, ap);
	va_end(ap);
	return r;
}

int
port_vswprintf_l_va(wchar_t *s, size_t n, const wchar_t *fmt, va_list ap)
{
	return P::vswprintf_l(s, n, LC_GLOBAL_LOCALE, fmt, ap);
}

int
ref_vswprintf_l_va(wchar_t *s, size_t n, const wchar_t *fmt, va_list ap)
{
	return ref_vswprintf_l(s, n, LC_GLOBAL_LOCALE, fmt, ap);
}

int
port_vswprintf_va(wchar_t *s, size_t n, const wchar_t *fmt, va_list ap)
{
	return P::vswprintf(s, n, fmt, ap);
}

int
ref_vswprintf_va(wchar_t *s, size_t n, const wchar_t *fmt, va_list ap)
{
	return ref_vswprintf(s, n, fmt, ap);
}

char *
make_temp_path(char *buf, std::size_t bufsz)
{
	const char *base = "/tmp/pbsd_b0143_";
	std::snprintf(buf, bufsz, "%s%016llx", base,
	    (unsigned long long)rnd());
	return buf;
}

bool
test_vswscanf_int(StatId which, const char *label, const wchar_t *str,
    const wchar_t *fmt)
{
	int pv = 0x7f7f7f7f, rv = 0x7f7f7f7f;
	int rp, rr;

	case_inc(which);

	if (which == S_VSWSCANF_L) {
		rp = port_vswscanf_l_va(str, fmt, &pv);
		rr = ref_vswscanf_l_va(str, fmt, &rv);
	} else {
		rp = port_vswscanf_va(str, fmt, &pv);
		rr = ref_vswscanf_va(str, fmt, &rv);
	}

	bool ok = (rp == rr) && (pv == rv);
	if (!ok)
		fail_msg(which, label, "int mismatch");
	return ok;
}

bool
test_vswprintf_one(StatId which, const char *label, size_t n,
    const wchar_t *fmt, ...)
{
	const std::size_t cap = n + 32;
	wchar_t *dp = (wchar_t *)std::malloc(cap * sizeof(wchar_t));
	wchar_t *dr = (wchar_t *)std::malloc(cap * sizeof(wchar_t));
	bool ok = true;
	int rp, rr, pe, re;
	va_list ap, ap2;

	if (dp == nullptr || dr == nullptr) {
		std::free(dp);
		std::free(dr);
		return true;
	}

	fill_guard_w(dp, cap);
	fill_guard_w(dr, cap);

	errno = 0;
	va_start(ap, fmt);
	va_copy(ap2, ap);
	if (which == S_VSWPRINTF_L)
		rp = port_vswprintf_l_va(dp, n, fmt, ap2);
	else
		rp = port_vswprintf_va(dp, n, fmt, ap2);
	va_end(ap2);
	pe = errno;
	va_end(ap);

	errno = 0;
	va_start(ap, fmt);
	va_copy(ap2, ap);
	if (which == S_VSWPRINTF_L)
		rr = ref_vswprintf_l_va(dr, n, fmt, ap2);
	else
		rr = ref_vswprintf_va(dr, n, fmt, ap2);
	va_end(ap2);
	re = errno;
	va_end(ap);

	case_inc(which);

	if (rp != rr)
		ok = false;
	if (pe != re)
		ok = false;
	for (std::size_t i = 0; i < cap; i++) {
		if (dp[i] != dr[i])
			ok = false;
	}

	if (!ok)
		fail_msg(which, label, "buffer or retval mismatch");
	std::free(dp);
	std::free(dr);
	return ok;
}

bool
test_fopen_one(const char *label, const char *path, const char *mode,
    bool use_sfp_hook, bool use_open_hook, int open_val)
{
	P::FILE pc{};
	ref_FILE rc{};
	ref_FILE *rr;
	P::FILE *rp;
	int pe, re;
	bool ok = true;

	case_inc(S_FOPEN);

	std::memset(&pc, 0, sizeof(pc));
	std::memset(&rc, 0, sizeof(rc));

	if (use_open_hook)
		b0143_set_open_hook(1, open_val);

	if (use_sfp_hook) {
		rc._flags = 1;
		errno = 0;
		b0143_set_sfp_target(&rc);
		rp = P::fopen(path, mode);
		pe = errno;
		b0143_set_sfp_target(nullptr);

		errno = 0;
		b0143_set_sfp_target(&rc);
		rr = ref_fopen(path, mode);
		re = errno;
		b0143_set_sfp_target(nullptr);
	} else {
		errno = 0;
		rp = P::fopen(path, mode);
		pe = errno;
		errno = 0;
		rr = ref_fopen(path, mode);
		re = errno;
		if (rp != nullptr)
			pc = *rp;
	}

	if ((rr == nullptr) != (rp == nullptr))
		ok = false;
	if (rr == nullptr && rp == nullptr && pe != re)
		ok = false;
	if (rr != nullptr && rp != nullptr && !fp_fields_eq(pc, *rr))
		ok = false;

	if (rr != nullptr) {
		_close(rr->_file);
		rr->_flags = 0;
	}
	if (rp != nullptr)
		_close(pc._file);

	if (!ok)
		fail_msg(S_FOPEN, label, "fopen mismatch");
	return ok;
}

void
run_vswscanf_edges(StatId which)
{
	test_vswscanf_int(which, "empty fmt", L"", L"");
	test_vswscanf_int(which, "int basic", L"42", L"%d");
	test_vswscanf_int(which, "int neg", L"-99", L"%d");
	test_vswscanf_int(which, "two ints", L"1 2", L"%d %d");
	test_vswscanf_int(which, "hex", L"0xff", L"%x");
	test_vswscanf_int(which, "nul in str", L"a\0b", L"%c");
	test_vswscanf_int(which, "high wchar", L"\u00e9 7", L"%lc %d");
	test_vswscanf_int(which, "no match", L"abc", L"%d");
	test_vswscanf_int(which, "percent", L"%", L"%%");
	test_vswscanf_int(which, "spaces", L"   5", L"%d");
	test_vswscanf_int(which, "width", L"12345", L"%2d");
	test_vswscanf_int(which, "eof empty", L"", L"%d");
}

void
run_vswscanf_random(StatId which)
{
	wchar_t str[64];
	wchar_t fmt[16];

	for (long i = 0; i < SWEEP; i++) {
		std::size_t slen = rnd_mod(40) + 1;
		int v1 = (int)(rnd() & 0x7fffu);
		int v2 = (int)(rnd() & 0xffffu);

		for (std::size_t j = 0; j < slen; j++) {
			unsigned x = (unsigned)rnd();
			if ((x & 15u) == 0)
				str[j] = (wchar_t)(0x80 + (x & 0x7fu));
			else if ((x & 15u) == 1)
				str[j] = L'\0';
			else if ((x & 15u) == 2)
				str[j] = (wchar_t)('0' + (x % 10u));
			else
				str[j] = (wchar_t)('a' + (x % 26u));
		}
		str[slen] = L'\0';

		switch (rnd_mod(6)) {
		case 0:
			std::swprintf(fmt, 16, L"%%d");
			break;
		case 1:
			std::swprintf(fmt, 16, L"%%d %%d");
			break;
		case 2:
			std::swprintf(fmt, 16, L"%%x");
			break;
		case 3:
			std::swprintf(fmt, 16, L"%%c");
			break;
		case 4:
			std::swprintf(fmt, 16, L"%%*d");
			break;
		default:
			std::swprintf(fmt, 16, L"%%2d");
			break;
		}

		char label[32];
		std::snprintf(label, sizeof(label), "rnd%ld", i);
		test_vswscanf_int(which, label, str, fmt);
		(void)v1;
		(void)v2;
	}
}

void
run_vswprintf_edges(StatId which)
{
	test_vswprintf_one(which, "n zero", 0, L"%d", 1);
	test_vswprintf_one(which, "n one", 1, L"%d", 42);
	test_vswprintf_one(which, "n two", 2, L"%d", 9);
	test_vswprintf_one(which, "n small", 4, L"%d", 123);
	test_vswprintf_one(which, "n overflow", 3, L"%d", 9999);
	test_vswprintf_one(which, "string", 16, L"%ls", 0);
	test_vswprintf_one(which, "hex", 8, L"%x", 255);
	test_vswprintf_one(which, "neg", 8, L"%d", -42);
	test_vswprintf_one(which, "empty fmt", 8, L"", 0);
	test_vswprintf_one(which, "percent", 4, L"%%", 0);
	test_vswprintf_one(which, "wchar", 8, L"%lc", 0xe9);
	test_vswprintf_one(which, "n huge", (size_t)INT_MAX + 2, L"%d", 1);
}

void
run_vswprintf_random(StatId which)
{
	wchar_t fmt[32];

	for (long i = 0; i < SWEEP; i++) {
		size_t n = rnd_mod(64) + 1;
		int val = (int)(rnd() & 0x7fffffffu);

		switch (rnd_mod(5)) {
		case 0:
			std::swprintf(fmt, 32, L"%%d");
			break;
		case 1:
			std::swprintf(fmt, 32, L"%%x");
			break;
		case 2:
			std::swprintf(fmt, 32, L"%%04d");
			break;
		case 3:
			std::swprintf(fmt, 32, L"%%c");
			break;
		default:
			std::swprintf(fmt, 32, L"%%d %%d");
			val = (int)(rnd() & 0xffffu);
			break;
		}

		char label[32];
		std::snprintf(label, sizeof(label), "rnd%ld", i);
		test_vswprintf_one(which, label, n, fmt, val);
	}
}

void
run_fopen_edges(void)
{
	char path[64];
	char buf[8];

	make_temp_path(path, sizeof(path));
	unlink(path);

	test_fopen_one("bad mode", path, "q", false, false, 0);
	test_fopen_one("rx invalid", path, "rx", false, false, 0);
	test_fopen_one("null path", nullptr, "r", false, false, 0);
	test_fopen_one("create w", path, "w", false, false, 0);
	test_fopen_one("create r", path, "r", false, false, 0);
	test_fopen_one("append a", path, "a", false, false, 0);
	test_fopen_one("read write", path, "r+", false, false, 0);
	test_fopen_one("write plus", path, "w+", false, false, 0);
	test_fopen_one("append plus", path, "a+", false, false, 0);
	test_fopen_one("binary rb", path, "rb", false, false, 0);
	test_fopen_one("exclusive wx", path, "wx", false, false, 0);
	test_fopen_one("sfp fail", path, "w", true, false, 0);

	{
		int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0666);
		if (fd >= 0) {
			std::strcpy(buf, "data");
			write(fd, buf, 4);
			close(fd);
		}
	}
	test_fopen_one("existing r", path, "r", false, false, 0);

	b0143_set_open_hook(1, SHRT_MAX + 1);
	test_fopen_one("fd gt shrt_max", path, "r", false, true, SHRT_MAX + 1);

	b0143_set_open_hook(1, -1);
	test_fopen_one("open fail", "/nonexistent_pbsd_b0143_xyz", "r",
	    false, true, -1);

	unlink(path);
}

void
run_fopen_random(void)
{
	char path[64];
	const char *modes[] = { "r", "w", "a", "rb", "wb", "ab", "r+", "w+", "a+" };

	for (long i = 0; i < SWEEP; i++) {
		make_temp_path(path, sizeof(path));
		const char *mode = modes[rnd_mod(9)];

		if (mode[0] == 'w' || mode[0] == 'a' || std::strchr(mode, '+'))
	unlink(path);

		char label[32];
		std::snprintf(label, sizeof(label), "rnd%ld", i);
		test_fopen_one(label, path, mode, false, false, 0);
	unlink(path);
	}
}

void
print_table(void)
{
	long long total_cases = 0;
	long long total_fails = 0;

	std::printf("b0143 differential test results\n");
	std::printf("%-16s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < NSTAT; i++) {
		std::printf("%-16s %12lld %12lld\n",
		    g_stat[i].name, g_stat[i].cases, g_stat[i].fails);
		total_cases += g_stat[i].cases;
		total_fails += g_stat[i].fails;
	}
	std::printf("%-16s %12lld %12lld\n", "TOTAL", total_cases, total_fails);
}

} /* namespace */

int
main(void)
{
	run_vswscanf_edges(S_VSWSCANF_L);
	run_vswscanf_edges(S_VSWSCANF);
	run_vswscanf_random(S_VSWSCANF_L);
	run_vswscanf_random(S_VSWSCANF);

	run_vswprintf_edges(S_VSWPRINTF_L);
	run_vswprintf_edges(S_VSWPRINTF);
	run_vswprintf_random(S_VSWPRINTF_L);
	run_vswprintf_random(S_VSWPRINTF);

	run_fopen_edges();
	run_fopen_random();

	print_table();
	return (g_stat[0].fails + g_stat[1].fails + g_stat[2].fails +
	    g_stat[3].fails + g_stat[4].fails) > 0 ? 1 : 0;
}
