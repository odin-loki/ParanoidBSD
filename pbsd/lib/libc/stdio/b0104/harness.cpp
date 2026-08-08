/*
 * harness.cpp -- differential test for PBSD batch b0104.
 *
 * Every case executes the C++ port and the ref_ oracle on independent but
 * identically-prepared state.  Return values and the full contents of every
 * caller buffer (guard bytes included) are compared.
 */

#define _GNU_SOURCE
#include <cerrno>
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

import pbsd.lib.libc.stdio.b0104;

namespace P = pbsd::lib_libc_stdio::b0104;

extern "C" {
int	ref_fprintf(FILE * __restrict, const char * __restrict, ...);
int	ref_fprintf_l(FILE * __restrict, locale_t, const char * __restrict, ...);
int	ref_sscanf(const char * __restrict, char const * __restrict, ...);
int	ref_sscanf_l(const char * __restrict, locale_t, char const * __restrict,
	    ...);
int	ref_vscanf(const char * __restrict, __va_list);
int	ref_vscanf_l(locale_t, const char * __restrict, __va_list);
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

static Stat st_fprintf = { "fprintf", 0, 0, 0 };
static Stat st_fprintf_l = { "fprintf_l", 0, 0, 0 };
static Stat st_sscanf = { "sscanf", 0, 0, 0 };
static Stat st_sscanf_l = { "sscanf_l", 0, 0, 0 };
static Stat st_vscanf = { "vscanf", 0, 0, 0 };
static Stat st_vscanf_l = { "vscanf_l", 0, 0, 0 };

static std::uint64_t rng_state = 0xb0104c0ffeeULL;

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
/* fprintf / fprintf_l                                                       */
/* ------------------------------------------------------------------------ */

struct FprintObs {
	int ret;
	unsigned char out[512];
	unsigned char tail[32];
};

static FILE *
make_out_file(void)
{
	char path[] = "/tmp/pbsd_b0104_out_XXXXXX";
	int fd;
	FILE *fp;

	fd = mkstemp(path);
	if (fd < 0)
		return nullptr;
	unlink(path);
	fp = fdopen(fd, "w+");
	return fp;
}

static void
read_out(FILE *fp, unsigned char *out, std::size_t outsz,
    unsigned char *tail, std::size_t tailsz)
{
	std::size_t n;

	std::memset(out, GUARD, outsz);
	std::memset(tail, GUARD, tailsz);
	std::rewind(fp);
	n = fread(out, 1, outsz, fp);
	(void)n;
	(void)fread(tail, 1, tailsz, fp);
}

template<typename... Args>
static void
fprint_case(Stat *st, locale_t loc, int use_l, const char *fmt, Args... args)
{
	FILE *pa = make_out_file();
	FILE *pb = make_out_file();
	FprintObs a, b;
	int bad;

	if (pa == nullptr || pb == nullptr) {
		std::fprintf(stderr, "harness bug: temp file for fprintf\n");
		std::exit(2);
	}

	if (use_l)
		a.ret = P::fprintf_l(pa, loc, fmt, args...);
	else
		a.ret = P::fprintf(pa, fmt, args...);
	std::fflush(pa);
	read_out(pa, a.out, sizeof(a.out), a.tail, sizeof(a.tail));

	if (use_l)
		b.ret = ref_fprintf_l(pb, loc, fmt, args...);
	else
		b.ret = ref_fprintf(pb, fmt, args...);
	std::fflush(pb);
	read_out(pb, b.out, sizeof(b.out), b.tail, sizeof(b.tail));

	st->cases++;
	bad = 0;
	if (a.ret != b.ret)
		bad = 1;
	if (std::memcmp(a.out, b.out, sizeof(a.out)) != 0)
		bad = 1;
	if (std::memcmp(a.tail, b.tail, sizeof(a.tail)) != 0)
		bad = 1;
	if (bad) {
		char detail[160];
		std::snprintf(detail, sizeof(detail),
		    "fmt='%s' ret %d/%d", fmt, a.ret, b.ret);
		stat_fail(st, "case", detail);
	}

	fclose(pa);
	fclose(pb);
}

static void
fprintf_edges(locale_t loc)
{
	fprint_case(&st_fprintf, loc, 0, "");
	fprint_case(&st_fprintf, loc, 0, "%%");
	fprint_case(&st_fprintf, loc, 0, "%d", 0);
	fprint_case(&st_fprintf, loc, 0, "%d", -1);
	fprint_case(&st_fprintf, loc, 0, "%d", 42);
	fprint_case(&st_fprintf, loc, 0, "%u", 255U);
	fprint_case(&st_fprintf, loc, 0, "%x", 0xdeadbeefU);
	fprint_case(&st_fprintf, loc, 0, "%c", 'A');
	fprint_case(&st_fprintf, loc, 0, "%c", (int)(char)0x80);
	fprint_case(&st_fprintf, loc, 0, "%c", (int)(char)0xff);
	fprint_case(&st_fprintf, loc, 0, "%s", "");
	fprint_case(&st_fprintf, loc, 0, "%s", "hi");
	fprint_case(&st_fprintf, loc, 0, "%s", "\x80\xff");
	fprint_case(&st_fprintf, loc, 0, "% d", 7);
	fprint_case(&st_fprintf, loc, 0, "%+d", -9);
	fprint_case(&st_fprintf, loc, 0, "%05d", 3);
	fprint_case(&st_fprintf, loc, 0, "%s%c%d", "x", 'y', 1);

	fprint_case(&st_fprintf_l, loc, 1, "");
	fprint_case(&st_fprintf_l, loc, 1, "%%");
	fprint_case(&st_fprintf_l, loc, 1, "%d", 0);
	fprint_case(&st_fprintf_l, loc, 1, "%d", -1);
	fprint_case(&st_fprintf_l, loc, 1, "%d", 42);
	fprint_case(&st_fprintf_l, loc, 1, "%u", 255U);
	fprint_case(&st_fprintf_l, loc, 1, "%x", 0xdeadbeefU);
	fprint_case(&st_fprintf_l, loc, 1, "%c", 'A');
	fprint_case(&st_fprintf_l, loc, 1, "%c", (int)(char)0x80);
	fprint_case(&st_fprintf_l, loc, 1, "%c", (int)(char)0xff);
	fprint_case(&st_fprintf_l, loc, 1, "%s", "");
	fprint_case(&st_fprintf_l, loc, 1, "%s", "hi");
	fprint_case(&st_fprintf_l, loc, 1, "%s", "\x80\xff");
	fprint_case(&st_fprintf_l, loc, 1, "% d", 7);
	fprint_case(&st_fprintf_l, loc, 1, "%+d", -9);
	fprint_case(&st_fprintf_l, loc, 1, "%05d", 3);
	fprint_case(&st_fprintf_l, loc, 1, "%s%c%d", "x", 'y', 1);

	fprint_case(&st_fprintf_l, (locale_t)0, 1, "%d", 1);
	fprint_case(&st_fprintf_l, LC_GLOBAL_LOCALE, 1, "%d", 2);
}

static const char *const fprint_fmts[] = {
	"%d", "%u", "%x", "%c", "%s", "%%", "% d", "%+d", "%05d", "%hd",
	"%s%c%d", "%i", "%o", "%p", "%.2f", "%lld", "%*d", "%3s", "%02x",
	"%d %u %x", "%c%s", "%+08d", ""
};

static void
fprintf_random(locale_t loc, long n)
{
	char sbuf[32];

	for (long t = 0; t < n; t++) {
		const char *fmt = fprint_fmts[rnd_mod(sizeof(fprint_fmts) /
		    sizeof(fprint_fmts[0]))];
		int a = (int)rnd();
		int b = (int)rnd();
		unsigned u = (unsigned)rnd();
		long long ll = (long long)rnd();
		short h = (short)rnd();
		char c = (char)(rnd() & 0xff);
		double d = (double)(int)rnd() / 100.0;
		void *p = (void *)(uintptr_t)rnd();
		int width = (int)(rnd_mod(8));
		std::size_t slen = rnd_mod(sizeof(sbuf));

		for (std::size_t i = 0; i < slen; i++)
			sbuf[i] = (char)(rnd() & 0xff);
		sbuf[slen] = '\0';

		fprint_case(&st_fprintf, loc, 0, fmt, a, b, u, ll, h, c, d, p,
		    width, sbuf);
		fprint_case(&st_fprintf_l, loc, 1, fmt, a, b, u, ll, h, c, d, p,
		    width, sbuf);
		if ((t & 0xff) == 0) {
			fprint_case(&st_fprintf_l, (locale_t)0, 1, fmt, a);
			fprint_case(&st_fprintf_l, LC_GLOBAL_LOCALE, 1, fmt, a);
		}
	}
}

/* ------------------------------------------------------------------------ */
/* sscanf / sscanf_l                                                         */
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

static void
sscan_case(Stat *st, locale_t loc, int use_l, const char *str,
    const char *fmt, const char *tag)
{
	ScanSlots a, b;
	int ra, rb;
	int bad;

	scan_slots_init(&a);
	scan_slots_init(&b);

	if (use_l) {
		ra = P::sscanf_l(str, loc, fmt, &a.i, &a.u, &a.l, a.s, &a.c);
		rb = ref_sscanf_l(str, loc, fmt, &b.i, &b.u, &b.l, b.s, &b.c);
	} else {
		ra = P::sscanf(str, fmt, &a.i, &a.u, &a.l, a.s, &a.c);
		rb = ref_sscanf(str, fmt, &b.i, &b.u, &b.l, b.s, &b.c);
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
		    "[%s] str='%s' fmt='%s' ret %d/%d", tag, str, fmt, ra, rb);
		stat_fail(st, "case", detail);
	}
}

static void
sscanf_edges(locale_t loc)
{
	sscan_case(&st_sscanf, loc, 0, "", "%d", "empty");
	sscan_case(&st_sscanf, loc, 0, "42", "%d", "int");
	sscan_case(&st_sscanf, loc, 0, "-7", "%d", "neg");
	sscan_case(&st_sscanf, loc, 0, "0", "%d", "zero");
	sscan_case(&st_sscanf, loc, 0, "ff", "%x", "hex");
	sscan_case(&st_sscanf, loc, 0, "z", "%c", "char");
	sscan_case(&st_sscanf, loc, 0, "\x80", "%c", "hi");
	sscan_case(&st_sscanf, loc, 0, "\xff", "%c", "ff");
	sscan_case(&st_sscanf, loc, 0, "abc", "%s", "str");
	sscan_case(&st_sscanf, loc, 0, "  9", "%d", "ws");
	sscan_case(&st_sscanf, loc, 0, "no", "%d", "nomatch");
	sscan_case(&st_sscanf, loc, 0, "12x", "%d", "partial");
	sscan_case(&st_sscanf, loc, 0, "1 2", "%d%d", "pair");
	sscan_case(&st_sscanf, loc, 0, "%%", "%%", "pct");
	sscan_case(&st_sscanf, loc, 0, "3", "%*d%d", "suppress");
	sscan_case(&st_sscanf, loc, 0, "\x00\x01", "%c%c", "nul");

	sscan_case(&st_sscanf_l, loc, 1, "", "%d", "empty");
	sscan_case(&st_sscanf_l, loc, 1, "42", "%d", "int");
	sscan_case(&st_sscanf_l, loc, 1, "-7", "%d", "neg");
	sscan_case(&st_sscanf_l, loc, 1, "0", "%d", "zero");
	sscan_case(&st_sscanf_l, loc, 1, "ff", "%x", "hex");
	sscan_case(&st_sscanf_l, loc, 1, "z", "%c", "char");
	sscan_case(&st_sscanf_l, loc, 1, "\x80", "%c", "hi");
	sscan_case(&st_sscanf_l, loc, 1, "\xff", "%c", "ff");
	sscan_case(&st_sscanf_l, loc, 1, "abc", "%s", "str");
	sscan_case(&st_sscanf_l, loc, 1, "  9", "%d", "ws");
	sscan_case(&st_sscanf_l, loc, 1, "no", "%d", "nomatch");
	sscan_case(&st_sscanf_l, loc, 1, "12x", "%d", "partial");
	sscan_case(&st_sscanf_l, loc, 1, "1 2", "%d%d", "pair");
	sscan_case(&st_sscanf_l, loc, 1, "%%", "%%", "pct");
	sscan_case(&st_sscanf_l, loc, 1, "3", "%*d%d", "suppress");
	sscan_case(&st_sscanf_l, loc, 1, "\x00\x01", "%c%c", "nul");

	sscan_case(&st_sscanf_l, (locale_t)0, 1, "5", "%d", "null-loc");
	sscan_case(&st_sscanf_l, LC_GLOBAL_LOCALE, 1, "6", "%d", "global-loc");
}

static const char *const scan_fmts[] = {
	"%d", "%u", "%x", "%i", "%o", "%c", "%s", "%d%d", "%x %u",
	"%*d%d", "%2d", "%3s", "%%", "%hd", "%ld", "%c%c", "%s%d", ""
};

static void
make_scan_input(char *buf, std::size_t bufsz, std::size_t len)
{
	std::size_t i;

	if (len >= bufsz)
		len = bufsz - 1;
	for (i = 0; i < len; i++)
		buf[i] = (char)(rnd() & 0xff);
	buf[len] = '\0';
}

static void
sscanf_random(locale_t loc, long n)
{
	char inbuf[96];

	for (long t = 0; t < n; t++) {
		const char *fmt = scan_fmts[rnd_mod(sizeof(scan_fmts) /
		    sizeof(scan_fmts[0]))];
		std::size_t len = rnd_mod(sizeof(inbuf));

		make_scan_input(inbuf, sizeof(inbuf), len);
		sscan_case(&st_sscanf, loc, 0, inbuf, fmt, "random");
		sscan_case(&st_sscanf_l, loc, 1, inbuf, fmt, "random");
		if ((t & 0xff) == 0) {
			sscan_case(&st_sscanf_l, (locale_t)0, 1, inbuf, fmt,
			    "null-loc");
			sscan_case(&st_sscanf_l, LC_GLOBAL_LOCALE, 1, inbuf, fmt,
			    "global-loc");
		}
	}
}

/* ------------------------------------------------------------------------ */
/* vscanf / vscanf_l                                                         */
/* ------------------------------------------------------------------------ */

static int
write_stdin_file(const char *path, const unsigned char *data, std::size_t len)
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
port_vscanf_wrap(const char *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = P::vscanf(fmt, ap);
	va_end(ap);
	return (r);
}

static int
ref_vscanf_wrap(const char *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = ref_vscanf(fmt, ap);
	va_end(ap);
	return (r);
}

static int
port_vscanf_l_wrap(locale_t loc, const char *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = P::vscanf_l(loc, fmt, ap);
	va_end(ap);
	return (r);
}

static int
ref_vscanf_l_wrap(locale_t loc, const char *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = ref_vscanf_l(loc, fmt, ap);
	va_end(ap);
	return (r);
}

static int
call_port_vscanf(const char *path, const char *fmt, ScanSlots *sl)
{
	int r;

	if (freopen(path, "r", stdin) == nullptr)
		return -2;
	setbuf(stdin, nullptr);
	clearerr(stdin);
	scan_slots_init(sl);
	r = port_vscanf_wrap(fmt, &sl->i, &sl->u, &sl->l, sl->s, &sl->c);
	return r;
}

static int
call_ref_vscanf(const char *path, const char *fmt, ScanSlots *sl)
{
	int r;

	if (freopen(path, "r", stdin) == nullptr)
		return -2;
	setbuf(stdin, nullptr);
	clearerr(stdin);
	scan_slots_init(sl);
	r = ref_vscanf_wrap(fmt, &sl->i, &sl->u, &sl->l, sl->s, &sl->c);
	return r;
}

static int
call_port_vscanf_l(const char *path, locale_t loc, const char *fmt,
    ScanSlots *sl)
{
	int r;

	if (freopen(path, "r", stdin) == nullptr)
		return -2;
	setbuf(stdin, nullptr);
	clearerr(stdin);
	scan_slots_init(sl);
	r = port_vscanf_l_wrap(loc, fmt, &sl->i, &sl->u, &sl->l, sl->s, &sl->c);
	return r;
}

static int
call_ref_vscanf_l(const char *path, locale_t loc, const char *fmt,
    ScanSlots *sl)
{
	int r;

	if (freopen(path, "r", stdin) == nullptr)
		return -2;
	setbuf(stdin, nullptr);
	clearerr(stdin);
	scan_slots_init(sl);
	r = ref_vscanf_l_wrap(loc, fmt, &sl->i, &sl->u, &sl->l, sl->s, &sl->c);
	return r;
}

static void
vscan_case(Stat *st, locale_t loc, int use_l, const unsigned char *data,
    std::size_t len, const char *fmt, const char *tag)
{
	char path[] = "/tmp/pbsd_b0104_in_XXXXXX";
	int fd;
	ScanSlots a, b;
	int ra, rb;
	int bad;

	fd = mkstemp(path);
	if (fd < 0) {
		std::fprintf(stderr, "harness bug: mkstemp vscanf\n");
		std::exit(2);
	}
	close(fd);
	if (write_stdin_file(path, data, len) != 0) {
		unlink(path);
		std::fprintf(stderr, "harness bug: write vscanf input\n");
		std::exit(2);
	}

	if (use_l) {
		ra = call_port_vscanf_l(path, loc, fmt, &a);
		rb = call_ref_vscanf_l(path, loc, fmt, &b);
	} else {
		ra = call_port_vscanf(path, fmt, &a);
		rb = call_ref_vscanf(path, fmt, &b);
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
vscanf_edges(locale_t loc)
{
	static const unsigned char empty[] = { "" };
	static const unsigned char one[] = { '7' };
	static const unsigned char neg[] = { '-', '3' };
	static const unsigned char hi[] = { 0x80 };
	static const unsigned char ff[] = { 0xff };
	static const unsigned char pair[] = { '1', ' ', '2' };
	static const unsigned char str[] = { 'a', 'b', 'c' };
	static const unsigned char pct[] = { '%' };
	static const unsigned char nulpair[] = { 0x00, 0x01 };

	vscan_case(&st_vscanf, loc, 0, empty, 0, "%d", "empty");
	vscan_case(&st_vscanf, loc, 0, one, 1, "%d", "one");
	vscan_case(&st_vscanf, loc, 0, neg, 2, "%d", "neg");
	vscan_case(&st_vscanf, loc, 0, hi, 1, "%c", "hi");
	vscan_case(&st_vscanf, loc, 0, ff, 1, "%c", "ff");
	vscan_case(&st_vscanf, loc, 0, pair, 3, "%d%d", "pair");
	vscan_case(&st_vscanf, loc, 0, str, 3, "%s", "str");
	vscan_case(&st_vscanf, loc, 0, pct, 1, "%%", "pct");
	vscan_case(&st_vscanf, loc, 0, nulpair, 2, "%c%c", "nul");

	vscan_case(&st_vscanf_l, loc, 1, empty, 0, "%d", "empty");
	vscan_case(&st_vscanf_l, loc, 1, one, 1, "%d", "one");
	vscan_case(&st_vscanf_l, loc, 1, neg, 2, "%d", "neg");
	vscan_case(&st_vscanf_l, loc, 1, hi, 1, "%c", "hi");
	vscan_case(&st_vscanf_l, loc, 1, ff, 1, "%c", "ff");
	vscan_case(&st_vscanf_l, loc, 1, pair, 3, "%d%d", "pair");
	vscan_case(&st_vscanf_l, loc, 1, str, 3, "%s", "str");
	vscan_case(&st_vscanf_l, loc, 1, pct, 1, "%%", "pct");
	vscan_case(&st_vscanf_l, loc, 1, nulpair, 2, "%c%c", "nul");

	vscan_case(&st_vscanf_l, (locale_t)0, 1, one, 1, "%d", "null-loc");
	vscan_case(&st_vscanf_l, LC_GLOBAL_LOCALE, 1, one, 1, "%d", "global-loc");
}

static void
vscanf_random(locale_t loc, long n)
{
	unsigned char inbuf[96];

	for (long t = 0; t < n; t++) {
		const char *fmt = scan_fmts[rnd_mod(sizeof(scan_fmts) /
		    sizeof(scan_fmts[0]))];
		std::size_t len = rnd_mod(sizeof(inbuf));

		for (std::size_t i = 0; i < len; i++)
			inbuf[i] = (unsigned char)rnd();
		vscan_case(&st_vscanf, loc, 0, inbuf, len, fmt, "random");
		vscan_case(&st_vscanf_l, loc, 1, inbuf, len, fmt, "random");
		if ((t & 0xff) == 0) {
			vscan_case(&st_vscanf_l, (locale_t)0, 1, inbuf, len,
			    fmt, "null-loc");
			vscan_case(&st_vscanf_l, LC_GLOBAL_LOCALE, 1, inbuf, len,
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

	fprintf_edges(loc);
	fprintf_random(loc, SWEEP);

	sscanf_edges(loc);
	sscanf_random(loc, SWEEP);

	vscanf_edges(loc);
	vscanf_random(loc, SWEEP);

	if (loc != nullptr && loc != LC_GLOBAL_LOCALE)
		freelocale(loc);

	total_fails = st_fprintf.fails + st_fprintf_l.fails + st_sscanf.fails +
	    st_sscanf_l.fails + st_vscanf.fails + st_vscanf_l.fails;

	std::printf("\n");
	std::printf("+----------------+----------+----------+\n");
	std::printf("| function       |     cases|    fails |\n");
	std::printf("+----------------+----------+----------+\n");
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_fprintf.name, st_fprintf.cases, st_fprintf.fails);
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_fprintf_l.name, st_fprintf_l.cases, st_fprintf_l.fails);
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_sscanf.name, st_sscanf.cases, st_sscanf.fails);
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_sscanf_l.name, st_sscanf_l.cases, st_sscanf_l.fails);
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_vscanf.name, st_vscanf.cases, st_vscanf.fails);
	std::printf("| %-14s | %9ld| %9ld|\n",
	    st_vscanf_l.name, st_vscanf_l.cases, st_vscanf_l.fails);
	std::printf("+----------------+----------+----------+\n");

	if (total_fails != 0)
		rc = 1;
	return rc;
}
