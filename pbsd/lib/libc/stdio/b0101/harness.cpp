/*
 * harness.cpp -- differential test for PBSD batch b0101.
 */

#include <climits>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <locale.h>

import pbsd.lib.libc.stdio.b0101;

namespace port = pbsd::lib_libc_stdio::b0101;

extern "C" {
int ref_getc(FILE *);
int ref_getc_unlocked(FILE *);
int ref_vprintf(const char *__restrict, va_list);
int ref_vprintf_l(locale_t, const char *__restrict, va_list);
int ref_putw(int, FILE *);
int ref_fputc(int, FILE *);
int ref_fputc_unlocked(int, FILE *);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr std::size_t PRE = 16;
constexpr std::size_t USER = 512;
constexpr std::size_t POST = 16;
constexpr std::size_t ARENA = PRE + USER + POST;
constexpr int MAXPRINT = 8;
constexpr long RAND_ITERS = 200000;

enum StatId {
	S_GETC,
	S_GETC_UNLOCKED,
	S_VPRINTF,
	S_VPRINTF_L,
	S_PUTW,
	S_FPUTC,
	S_FPUTC_UNLOCKED,
	NSTAT
};

struct Stats {
	const char *name;
	long cases;
	long fails;
	int printed;
};

Stats g_stat[NSTAT] = {
	{ "getc",           0, 0, 0 },
	{ "getc_unlocked",  0, 0, 0 },
	{ "vprintf",        0, 0, 0 },
	{ "vprintf_l",      0, 0, 0 },
	{ "putw",           0, 0, 0 },
	{ "fputc",          0, 0, 0 },
	{ "fputc_unlocked", 0, 0, 0 },
};

std::uint64_t rng_state = 0xb0101facefeedULL;

std::uint64_t
rnd_u64(void)
{
	std::uint64_t z;

	rng_state += 0x9e3779b97f4a7c15ULL;
	z = rng_state;
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

unsigned
rnd_u32(void)
{
	return (unsigned)(rnd_u64() & 0xffffffffu);
}

std::size_t
rnd_mod(std::size_t m)
{
	return m == 0 ? 0 : (std::size_t)(rnd_u64() % (std::uint64_t)m);
}

void
fail_msg(StatId id, const char *tag, const char *detail)
{
	g_stat[id].fails++;
	if (g_stat[id].printed++ < MAXPRINT)
		std::printf("  FAIL %-16s %-20s %s\n", g_stat[id].name, tag,
		    detail);
}

void
case_inc(StatId id)
{
	g_stat[id].cases++;
}

struct GuardArena {
	unsigned char bytes[ARENA];

	void fill(void)
	{
		std::memset(bytes, GUARD, ARENA);
	}

	bool eq(const GuardArena &o) const
	{
		return std::memcmp(bytes, o.bytes, ARENA) == 0;
	}
};

FILE *
temp_file_rw(const unsigned char *data, std::size_t len, char *path_out)
{
	char path[] = "/tmp/pbsd_b0101_XXXXXX";
	int fd;
	FILE *fp;

	fd = mkstemp(path);
	if (fd < 0)
		return nullptr;
	if (len > 0 && write(fd, data, len) != (ssize_t)len) {
		close(fd);
		unlink(path);
		return nullptr;
	}
	close(fd);
	fp = fopen(path, "r+b");
	if (fp == nullptr) {
		unlink(path);
		return nullptr;
	}
	std::strcpy(path_out, path);
	return fp;
}

void
rewind_stream(FILE *fp)
{
	std::rewind(fp);
	clearerr(fp);
}

struct GetcObs {
	int ret;
	long pos;
	unsigned char tail[64];
};

template <typename Fn>
GetcObs
run_getc(Fn fn, FILE *fp, int pre_reads)
{
	GetcObs obs{};

	std::memset(obs.tail, GUARD, sizeof(obs.tail));
	rewind_stream(fp);
	for (int i = 0; i < pre_reads; i++)
		(void)fn(fp);
	obs.ret = fn(fp);
	obs.pos = ftell(fp);
	if (obs.pos >= 0)
		(void)fread(obs.tail, 1, sizeof(obs.tail), fp);
	return obs;
}

static void
getc_case(StatId id, int use_unlocked, const unsigned char *data,
    std::size_t len, int pre_reads, const char *tag)
{
	char pa[64], pb[64];
	FILE *fa, *fb;
	GetcObs a, b;
	int bad;

	fa = temp_file_rw(data, len, pa);
	fb = temp_file_rw(data, len, pb);
	if (fa == nullptr || fb == nullptr) {
		std::fprintf(stderr, "harness bug: temp file getc\n");
		std::exit(2);
	}

	if (use_unlocked) {
		a = run_getc(ref_getc_unlocked, fa, pre_reads);
		b = run_getc(port::getc_unlocked, fb, pre_reads);
	} else {
		a = run_getc(ref_getc, fa, pre_reads);
		b = run_getc(port::getc, fb, pre_reads);
	}

	case_inc(id);
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (a.pos != b.pos)
		bad = 1;
	if (std::memcmp(a.tail, b.tail, sizeof(a.tail)) != 0)
		bad = 1;
	if (bad) {
		char detail[160];
		std::snprintf(detail, sizeof(detail),
		    "len=%zu pre=%d ret=%d/%d pos=%ld/%ld",
		    len, pre_reads, a.ret, b.ret, a.pos, b.pos);
		fail_msg(id, tag, detail);
	}

	fclose(fa);
	fclose(fb);
	unlink(pa);
	unlink(pb);
}

static void
getc_edges(void)
{
	static const unsigned char empty[] = { "" };
	static const unsigned char one_a[] = { 'a' };
	static const unsigned char one_nul[] = { 0x00 };
	static const unsigned char one_7f[] = { 0x7f };
	static const unsigned char one_80[] = { (unsigned char)0x80 };
	static const unsigned char one_ff[] = { (unsigned char)0xff };
	static const unsigned char hi[] = {
		0x00, 0x7f, (unsigned char)0x80, (unsigned char)0xff, 'Z'
	};
	unsigned char buf[USER];
	std::size_t i;

	for (i = 0; i < sizeof(buf); i++)
		buf[i] = (unsigned char)(0x80 + (i & 0x7f));

	getc_case(S_GETC, 0, empty, 0, 0, "empty@0");
	getc_case(S_GETC, 0, one_a, 1, 0, "a@0");
	getc_case(S_GETC, 0, one_a, 1, 1, "a@eof");
	getc_case(S_GETC, 0, one_nul, 1, 0, "nul@0");
	getc_case(S_GETC, 0, one_7f, 1, 0, "0x7f@0");
	getc_case(S_GETC, 0, one_80, 1, 0, "0x80@0");
	getc_case(S_GETC, 0, one_ff, 1, 0, "0xff@0");
	getc_case(S_GETC, 0, hi, sizeof(hi), 0, "hi@0");
	getc_case(S_GETC, 0, hi, sizeof(hi), 2, "hi@2");
	getc_case(S_GETC, 0, hi, sizeof(hi), 4, "hi@eof");
	getc_case(S_GETC, 0, buf, sizeof(buf), 0, "big@0");
	getc_case(S_GETC, 0, buf, sizeof(buf), 127, "big@127");
	getc_case(S_GETC, 0, buf, sizeof(buf), (int)sizeof(buf) - 1, "big@last");

	getc_case(S_GETC_UNLOCKED, 1, empty, 0, 0, "empty@0");
	getc_case(S_GETC_UNLOCKED, 1, one_a, 1, 0, "a@0");
	getc_case(S_GETC_UNLOCKED, 1, one_a, 1, 1, "a@eof");
	getc_case(S_GETC_UNLOCKED, 1, one_nul, 1, 0, "nul@0");
	getc_case(S_GETC_UNLOCKED, 1, one_7f, 1, 0, "0x7f@0");
	getc_case(S_GETC_UNLOCKED, 1, one_80, 1, 0, "0x80@0");
	getc_case(S_GETC_UNLOCKED, 1, one_ff, 1, 0, "0xff@0");
	getc_case(S_GETC_UNLOCKED, 1, hi, sizeof(hi), 0, "hi@0");
	getc_case(S_GETC_UNLOCKED, 1, hi, sizeof(hi), 2, "hi@2");
	getc_case(S_GETC_UNLOCKED, 1, hi, sizeof(hi), 4, "hi@eof");
	getc_case(S_GETC_UNLOCKED, 1, buf, sizeof(buf), 0, "big@0");
	getc_case(S_GETC_UNLOCKED, 1, buf, sizeof(buf), 127, "big@127");
	getc_case(S_GETC_UNLOCKED, 1, buf, sizeof(buf), (int)sizeof(buf) - 1,
	    "big@last");
}

static void
getc_random(StatId id, int use_unlocked, long n)
{
	unsigned char data[256];

	for (long t = 0; t < n; t++) {
		std::size_t len = rnd_mod(sizeof(data) + 1);
		int pre = (int)rnd_mod(len + 2);

		for (std::size_t i = 0; i < len; i++)
			data[i] = (unsigned char)rnd_u32();
		getc_case(id, use_unlocked, data, len, pre, "random");
	}
}

struct VprintfObs {
	int ret;
	GuardArena out;
};

static int
capture_stdout_vprintf(GuardArena &out,
    int (*fn)(const char *__restrict, va_list), const char *fmt,
    va_list ap_in)
{
	char path[] = "/tmp/pbsd_b0101_vp_XXXXXX";
	int fd, saved_stdout;
	va_list ap;
	int ret;
	FILE *cap;

	out.fill();
	fd = mkstemp(path);
	if (fd < 0)
		return -9999;
	close(fd);

	saved_stdout = dup(STDOUT_FILENO);
	if (saved_stdout < 0) {
		unlink(path);
		return -9998;
	}
	if (freopen(path, "w", stdout) == nullptr) {
		close(saved_stdout);
		unlink(path);
		return -9998;
	}
	va_copy(ap, ap_in);
	ret = fn(fmt, ap);
	va_end(ap);
	fflush(stdout);
	dup2(saved_stdout, STDOUT_FILENO);
	close(saved_stdout);

	cap = fopen(path, "rb");
	if (cap != nullptr) {
		(void)fread(out.bytes + PRE, 1, USER, cap);
		fclose(cap);
	}
	unlink(path);
	return ret;
}

static int
capture_stdout_vprintf_l(GuardArena &out,
    int (*fn)(locale_t, const char *__restrict, va_list), locale_t loc,
    const char *fmt, va_list ap_in)
{
	char path[] = "/tmp/pbsd_b0101_vpl_XXXXXX";
	int fd, saved_stdout;
	va_list ap;
	int ret;
	FILE *cap;

	out.fill();
	fd = mkstemp(path);
	if (fd < 0)
		return -9999;
	close(fd);

	saved_stdout = dup(STDOUT_FILENO);
	if (saved_stdout < 0) {
		unlink(path);
		return -9998;
	}
	if (freopen(path, "w", stdout) == nullptr) {
		close(saved_stdout);
		unlink(path);
		return -9998;
	}
	va_copy(ap, ap_in);
	ret = fn(loc, fmt, ap);
	va_end(ap);
	fflush(stdout);
	dup2(saved_stdout, STDOUT_FILENO);
	close(saved_stdout);

	cap = fopen(path, "rb");
	if (cap != nullptr) {
		(void)fread(out.bytes + PRE, 1, USER, cap);
		fclose(cap);
	}
	unlink(path);
	return ret;
}

static VprintfObs
run_vprintf_port(const char *fmt, va_list ap)
{
	VprintfObs obs{};

	obs.ret = capture_stdout_vprintf(obs.out,
	    [](const char *f, va_list a) -> int {
		    return port::vprintf(f, a);
	    },
	    fmt, ap);
	return obs;
}

static VprintfObs
run_vprintf_ref(const char *fmt, va_list ap)
{
	VprintfObs obs{};

	obs.ret = capture_stdout_vprintf(obs.out, ref_vprintf, fmt, ap);
	return obs;
}

static VprintfObs
run_vprintf_l_port(locale_t loc, const char *fmt, va_list ap)
{
	VprintfObs obs{};

	obs.ret = capture_stdout_vprintf_l(obs.out,
	    [](locale_t l, const char *f, va_list a) -> int {
		    return port::vprintf_l(l, f, a);
	    },
	    loc, fmt, ap);
	return obs;
}

static VprintfObs
run_vprintf_l_ref(locale_t loc, const char *fmt, va_list ap)
{
	VprintfObs obs{};

	obs.ret = capture_stdout_vprintf_l(obs.out, ref_vprintf_l, loc, fmt, ap);
	return obs;
}

static void
vprintf_compare(StatId id, VprintfObs a, VprintfObs b, const char *tag,
    const char *detail)
{
	int bad = 0;

	case_inc(id);
	if (a.ret != b.ret)
		bad = 1;
	if (!a.out.eq(b.out))
		bad = 1;
	if (bad)
		fail_msg(id, tag, detail);
}

static void
vprintf_case_va(StatId id, locale_t loc, int use_l, const char *fmt,
    va_list ap, const char *tag, const char *detail)
{
	va_list ap_a, ap_b;
	VprintfObs a, b;

	va_copy(ap_a, ap);
	va_copy(ap_b, ap);
	if (use_l) {
		a = run_vprintf_l_ref(loc, fmt, ap_a);
		b = run_vprintf_l_port(loc, fmt, ap_b);
	} else {
		a = run_vprintf_ref(fmt, ap_a);
		b = run_vprintf_port(fmt, ap_b);
	}
	va_end(ap_a);
	va_end(ap_b);
	vprintf_compare(id, a, b, tag, detail);
}

#define VPRINTF_CASE(id, loc, use_l, tag, fmt, ...)                          \
	do {                                                                   \
		va_list ap;                                                    \
		char detail[128];                                              \
		va_start(ap, fmt);                                             \
		std::snprintf(detail, sizeof(detail), tag);                    \
		vprintf_case_va(id, loc, use_l, fmt, ap, tag, detail);        \
		va_end(ap);                                                    \
	} while (0)

static void
vprintf_edges(locale_t loc)
{
	VPRINTF_CASE(S_VPRINTF, loc, 0, "empty", "");
	VPRINTF_CASE(S_VPRINTF, loc, 0, "pct", "%%");
	VPRINTF_CASE(S_VPRINTF, loc, 0, "d0", "%d", 0);
	VPRINTF_CASE(S_VPRINTF, loc, 0, "d1", "%d", 1);
	VPRINTF_CASE(S_VPRINTF, loc, 0, "d-1", "%d", -1);
	VPRINTF_CASE(S_VPRINTF, loc, 0, "dmax", "%d", INT_MAX);
	VPRINTF_CASE(S_VPRINTF, loc, 0, "dmin", "%d", INT_MIN);
	VPRINTF_CASE(S_VPRINTF, loc, 0, "u", "%u", 0xffffffffu);
	VPRINTF_CASE(S_VPRINTF, loc, 0, "x", "%x", 0xdeadbeefu);
	VPRINTF_CASE(S_VPRINTF, loc, 0, "c0", "%c", 0);
	VPRINTF_CASE(S_VPRINTF, loc, 0, "c7f", "%c", 0x7f);
	VPRINTF_CASE(S_VPRINTF, loc, 0, "c80", "%c", 0x80);
	VPRINTF_CASE(S_VPRINTF, loc, 0, "cff", "%c", 0xff);
	VPRINTF_CASE(S_VPRINTF, loc, 0, "s-empty", "%s", "");
	VPRINTF_CASE(S_VPRINTF, loc, 0, "s-a", "%s", "a");
	VPRINTF_CASE(S_VPRINTF, loc, 0, "s-hi", "%s", "\x80\xff");
	VPRINTF_CASE(S_VPRINTF, loc, 0, "mix", "%d %u %x %c", 42, 7u, 0xab, 'Q');

	VPRINTF_CASE(S_VPRINTF_L, loc, 1, "empty", "");
	VPRINTF_CASE(S_VPRINTF_L, loc, 1, "pct", "%%");
	VPRINTF_CASE(S_VPRINTF_L, loc, 1, "d0", "%d", 0);
	VPRINTF_CASE(S_VPRINTF_L, loc, 1, "d1", "%d", 1);
	VPRINTF_CASE(S_VPRINTF_L, loc, 1, "d-1", "%d", -1);
	VPRINTF_CASE(S_VPRINTF_L, loc, 1, "dmax", "%d", INT_MAX);
	VPRINTF_CASE(S_VPRINTF_L, loc, 1, "dmin", "%d", INT_MIN);
	VPRINTF_CASE(S_VPRINTF_L, loc, 1, "u", "%u", 0xffffffffu);
	VPRINTF_CASE(S_VPRINTF_L, loc, 1, "x", "%x", 0xdeadbeefu);
	VPRINTF_CASE(S_VPRINTF_L, loc, 1, "c0", "%c", 0);
	VPRINTF_CASE(S_VPRINTF_L, loc, 1, "c7f", "%c", 0x7f);
	VPRINTF_CASE(S_VPRINTF_L, loc, 1, "c80", "%c", 0x80);
	VPRINTF_CASE(S_VPRINTF_L, loc, 1, "cff", "%c", 0xff);
	VPRINTF_CASE(S_VPRINTF_L, loc, 1, "s-empty", "%s", "");
	VPRINTF_CASE(S_VPRINTF_L, loc, 1, "s-a", "%s", "a");
	VPRINTF_CASE(S_VPRINTF_L, loc, 1, "s-hi", "%s", "\x80\xff");
	VPRINTF_CASE(S_VPRINTF_L, loc, 1, "mix", "%d %u %x %c", 42, 7u, 0xab, 'Q');
}

static void
vprintf_random(locale_t loc, long n)
{
	char str[64];

	for (long t = 0; t < n; t++) {
		unsigned pick = rnd_u32() % 8u;

		switch (pick) {
		case 0:
			VPRINTF_CASE(S_VPRINTF, loc, 0, "random", "");
			VPRINTF_CASE(S_VPRINTF_L, loc, 1, "random", "");
			break;
		case 1:
			VPRINTF_CASE(S_VPRINTF, loc, 0, "random", "%d",
			    (int)((int)rnd_u32() ^ (int)(rnd_u32() & 1 ? 0 :
			    INT_MIN)));
			VPRINTF_CASE(S_VPRINTF_L, loc, 1, "random", "%d",
			    (int)((int)rnd_u32() ^ (int)(rnd_u32() & 1 ? 0 :
			    INT_MIN)));
			break;
		case 2:
			VPRINTF_CASE(S_VPRINTF, loc, 0, "random", "%u",
			    rnd_u32());
			VPRINTF_CASE(S_VPRINTF_L, loc, 1, "random", "%u",
			    rnd_u32());
			break;
		case 3:
			VPRINTF_CASE(S_VPRINTF, loc, 0, "random", "%x",
			    rnd_u32());
			VPRINTF_CASE(S_VPRINTF_L, loc, 1, "random", "%x",
			    rnd_u32());
			break;
		case 4:
			VPRINTF_CASE(S_VPRINTF, loc, 0, "random", "%c",
			    (int)(rnd_u32() & 0xff));
			VPRINTF_CASE(S_VPRINTF_L, loc, 1, "random", "%c",
			    (int)(rnd_u32() & 0xff));
			break;
		case 5: {
			std::size_t slen = rnd_mod(sizeof(str));
			for (std::size_t i = 0; i < slen; i++)
				str[i] = (char)(unsigned char)(rnd_u32() & 0xff);
			str[slen] = '\0';
			VPRINTF_CASE(S_VPRINTF, loc, 0, "random", "%s", str);
			VPRINTF_CASE(S_VPRINTF_L, loc, 1, "random", "%s", str);
			break;
		}
		default:
			VPRINTF_CASE(S_VPRINTF, loc, 0, "random", "%d %u %c",
			    (int)rnd_u32(), rnd_u32(),
			    (int)(rnd_u32() & 0xff));
			VPRINTF_CASE(S_VPRINTF_L, loc, 1, "random",
			    "%d %u %c", (int)rnd_u32(), rnd_u32(),
			    (int)(rnd_u32() & 0xff));
			break;
		}
	}
}

struct PutwObs {
	int ret;
	long pos;
	unsigned char filebuf[ARENA];
};

template <typename Fn>
PutwObs
run_putw(Fn fn, FILE *fp, int w)
{
	PutwObs obs{};

	std::memset(obs.filebuf, GUARD, sizeof(obs.filebuf));
	rewind_stream(fp);
	obs.ret = fn(w, fp);
	fflush(fp);
	obs.pos = ftell(fp);
	rewind_stream(fp);
	(void)fread(obs.filebuf + PRE, 1, USER, fp);
	return obs;
}

static void
putw_case(int w, const char *tag)
{
	char pa[64], pb[64];
	FILE *fa, *fb;
	PutwObs a, b;
	int bad;

	fa = temp_file_rw(nullptr, 0, pa);
	fb = temp_file_rw(nullptr, 0, pb);
	if (fa == nullptr || fb == nullptr) {
		std::fprintf(stderr, "harness bug: temp file putw\n");
		std::exit(2);
	}

	a = run_putw(ref_putw, fa, w);
	b = run_putw(port::putw, fb, w);

	case_inc(S_PUTW);
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (a.pos != b.pos)
		bad = 1;
	if (std::memcmp(a.filebuf, b.filebuf, sizeof(a.filebuf)) != 0)
		bad = 1;
	if (bad) {
		char detail[128];
		std::snprintf(detail, sizeof(detail),
		    "w=%d ret=%d/%d pos=%ld/%ld", w, a.ret, b.ret, a.pos,
		    b.pos);
		fail_msg(S_PUTW, tag, detail);
	}

	fclose(fa);
	fclose(fb);
	unlink(pa);
	unlink(pb);
}

static void
putw_edges(void)
{
	putw_case(0, "zero");
	putw_case(1, "one");
	putw_case(-1, "neg1");
	putw_case(INT_MAX, "max");
	putw_case(INT_MIN, "min");
	putw_case(0x7f7f7f7f, "7f");
	putw_case((int)0x80808080, "80");
	putw_case((int)0xffffffff, "ff");
}

static void
putw_random(long n)
{
	for (long t = 0; t < n; t++)
		putw_case((int)rnd_u32(), "random");
}

struct FputcObs {
	int ret;
	long pos;
	unsigned char filebuf[ARENA];
};

template <typename Fn>
FputcObs
run_fputc(Fn fn, FILE *fp, int c)
{
	FputcObs obs{};

	std::memset(obs.filebuf, GUARD, sizeof(obs.filebuf));
	rewind_stream(fp);
	obs.ret = fn(c, fp);
	fflush(fp);
	obs.pos = ftell(fp);
	rewind_stream(fp);
	(void)fread(obs.filebuf + PRE, 1, USER, fp);
	return obs;
}

static void
fputc_case(StatId id, int use_unlocked, int c, const char *tag)
{
	char pa[64], pb[64];
	FILE *fa, *fb;
	FputcObs a, b;
	int bad;

	fa = temp_file_rw(nullptr, 0, pa);
	fb = temp_file_rw(nullptr, 0, pb);
	if (fa == nullptr || fb == nullptr) {
		std::fprintf(stderr, "harness bug: temp file fputc\n");
		std::exit(2);
	}

	if (use_unlocked) {
		a = run_fputc(ref_fputc_unlocked, fa, c);
		b = run_fputc(port::fputc_unlocked, fb, c);
	} else {
		a = run_fputc(ref_fputc, fa, c);
		b = run_fputc(port::fputc, fb, c);
	}

	case_inc(id);
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (a.pos != b.pos)
		bad = 1;
	if (std::memcmp(a.filebuf, b.filebuf, sizeof(a.filebuf)) != 0)
		bad = 1;
	if (bad) {
		char detail[128];
		std::snprintf(detail, sizeof(detail),
		    "c=%d ret=%d/%d pos=%ld/%ld", c, a.ret, b.ret, a.pos,
		    b.pos);
		fail_msg(id, tag, detail);
	}

	fclose(fa);
	fclose(fb);
	unlink(pa);
	unlink(pb);
}

static void
fputc_edges(void)
{
	fputc_case(S_FPUTC, 0, 'a', "a");
	fputc_case(S_FPUTC, 0, 0, "nul");
	fputc_case(S_FPUTC, 0, 0x7f, "0x7f");
	fputc_case(S_FPUTC, 0, 0x80, "0x80");
	fputc_case(S_FPUTC, 0, 0xff, "0xff");
	fputc_case(S_FPUTC, 0, -1, "neg1");
	fputc_case(S_FPUTC, 0, EOF, "eof");

	fputc_case(S_FPUTC_UNLOCKED, 1, 'a', "a");
	fputc_case(S_FPUTC_UNLOCKED, 1, 0, "nul");
	fputc_case(S_FPUTC_UNLOCKED, 1, 0x7f, "0x7f");
	fputc_case(S_FPUTC_UNLOCKED, 1, 0x80, "0x80");
	fputc_case(S_FPUTC_UNLOCKED, 1, 0xff, "0xff");
	fputc_case(S_FPUTC_UNLOCKED, 1, -1, "neg1");
	fputc_case(S_FPUTC_UNLOCKED, 1, EOF, "eof");
}

static void
fputc_random(StatId id, int use_unlocked, long n)
{
	for (long t = 0; t < n; t++) {
		int c = (int)(rnd_u32() & 0xff);
		if (rnd_u32() & 1)
			c = (int)((signed char)c);
		fputc_case(id, use_unlocked, c, "random");
	}
}

} /* namespace */

int
main(void)
{
	locale_t loc;
	long total_fails = 0;
	int rc = 0;

	setlocale(LC_ALL, "C");
	loc = newlocale(LC_ALL_MASK, "C", nullptr);
	if (loc == nullptr)
		loc = (locale_t)0;

	getc_edges();
	getc_random(S_GETC, 0, RAND_ITERS);
	getc_random(S_GETC_UNLOCKED, 1, RAND_ITERS);

	vprintf_edges(loc);
	vprintf_random(loc, RAND_ITERS);

	putw_edges();
	putw_random(RAND_ITERS);

	fputc_edges();
	fputc_random(S_FPUTC, 0, RAND_ITERS);
	fputc_random(S_FPUTC_UNLOCKED, 1, RAND_ITERS);

	if (loc != nullptr && loc != (locale_t)0)
		freelocale(loc);

	for (int i = 0; i < NSTAT; i++)
		total_fails += g_stat[i].fails;

	std::printf("\n");
	std::printf("+------------------+----------+----------+\n");
	std::printf("| function         |     cases|    fails |\n");
	std::printf("+------------------+----------+----------+\n");
	for (int i = 0; i < NSTAT; i++)
		std::printf("| %-16s | %9ld| %9ld|\n", g_stat[i].name,
		    g_stat[i].cases, g_stat[i].fails);
	std::printf("+------------------+----------+----------+\n");

	if (total_fails != 0)
		rc = 1;
	return rc;
}
