/*
 * harness.cpp -- differential test: PBSD port (batch b0098s2) vs ref_ oracle.
 *
 * Function under test:
 *   vwprintf(const wchar_t *, va_list)  -- writes to stdout
 *
 * stdout is redirected to a temporary file; the report goes to a dup() of fd 1.
 * Every case runs BOTH implementations against byte-identical stream state and
 * compares return value, errno, error flags, ftell(), the raw fd offset, the
 * bytes on disk, and the ENTIRE guard-filled argument slab (0x7f) including
 * bytes past the nominal write window.
 */

#include <cerrno>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

import pbsd.lib.libc.stdio.b0098s2;

namespace P = pbsd::lib_libc_stdio::b0098s2;

extern "C" int ref_vwprintf(const wchar_t *__restrict fmt, va_list ap);

static FILE *g_rep;

static void die(const char *what)
{
	fprintf(stderr, "harness: fatal: %s: %s\n", what, strerror(errno));
	exit(2);
}

struct Tally {
	const char *name;
	long cases;
	long fails;
	long shown;
};

static Tally t_printf = { "vwprintf", 0, 0, 0 };

static const long SHOW_MAX = 8;

struct Rng {
	unsigned long long s;
	explicit Rng(unsigned long long seed) : s(seed) {}
	unsigned long long next()
	{
		unsigned long long z = (s += 0x9E3779B97F4A7C15ULL);
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
		return z ^ (z >> 31);
	}
	unsigned u32() { return (unsigned)(next() >> 32); }
	unsigned below(unsigned n) { return n ? (u32() % n) : 0u; }
	bool chance(unsigned pct) { return below(100) < pct; }
};

static char g_out_path[] = "/tmp/pbsd_b0098s2_out_XXXXXX";
static int g_out_rfd = -1;

static void init_streams()
{
	int fd = dup(1);
	if (fd < 0)
		die("dup");
	g_rep = fdopen(fd, "w");
	if (!g_rep)
		die("fdopen");

	int outfd = mkstemp(g_out_path);
	if (outfd < 0)
		die("mkstemp");
	g_out_rfd = outfd;

	if (!freopen(g_out_path, "w+", stdout))
		die("freopen stdout");
}

static void cleanup_streams()
{
	fflush(g_rep);
	unlink(g_out_path);
}

static void reset_stdout(bool fresh)
{
	if (fresh) {
		if (!freopen(g_out_path, "w+", stdout))
			die("freopen stdout");
	} else {
		rewind(stdout);
		if (ftruncate(fileno(stdout), 0) != 0)
			die("ftruncate stdout");
		clearerr(stdout);
	}
}

static const size_t SLAB = 512;

struct Slab {
	alignas(16) unsigned char b[SLAB];
};

static inline int *s_i3(Slab &s) { return (int *)(s.b + 56); }

static void slab_guard(Slab &s) { memset(s.b, 0x7f, SLAB); }

static void dump_slab_diff(const Slab &a, const Slab &b)
{
	for (size_t i = 0; i < SLAB; i++)
		if (a.b[i] != b.b[i])
			fprintf(g_rep, "      slab[%3zu]: ref=%02x port=%02x\n",
			    i, a.b[i], b.b[i]);
}

static unsigned char pick_byte(Rng &r)
{
	static const char digits[] = "0123456789";
	static const char spaces[] = " \t\n\v\f\r";
	static const char numish[] = "+-.eExXaAfFnN";
	static const char punct[]  = ",:;[]%*(){}/\\\"'";
	unsigned k = r.below(100);
	if (k < 24)
		return (unsigned char)digits[r.below(10)];
	if (k < 36)
		return (unsigned char)spaces[r.below(6)];
	if (k < 48)
		return (unsigned char)numish[r.below(13)];
	if (k < 58)
		return (unsigned char)('a' + r.below(26));
	if (k < 65)
		return (unsigned char)('A' + r.below(26));
	if (k < 72)
		return (unsigned char)punct[r.below(15)];
	if (k < 80)
		return 0;
	if (k < 84)
		return 0x80;
	if (k < 88)
		return 0xff;
	return (unsigned char)(0x80 + r.below(0x80));
}

static wchar_t pick_wchar(Rng &r)
{
	unsigned k = r.below(100);
	if (k < 42)
		return (wchar_t)(0x20 + r.below(0x5f));
	if (k < 56)
		return (wchar_t)L"\t\n\r "[r.below(4)];
	if (k < 62)
		return (wchar_t)(1 + r.below(0x1f));
	if (k < 70)
		return (wchar_t)0x80;
	if (k < 82)
		return (wchar_t)(0x80 + r.below(0x80));
	if (k < 92)
		return (wchar_t)(0x100 + r.below(0xf00));
	return (wchar_t)(0x10000 + r.below(0x1000));
}

enum PKind {
	P_NONE, P_INT, P_INT2, P_UINT, P_UINT3, P_WSTR, P_WSTR2, P_STR,
	P_WC, P_DBL, P_DBL3, P_PTR, P_N, P_LL, P_LONG, P_SIZE, P_IWD,
	P_WID_I, P_PREC_D, P_WID_PREC_D
};

struct PArgs {
	int i1, i2;
	int wid, prec;
	unsigned u1, u2, u3;
	long l1;
	long long ll;
	size_t z;
	double d1, d2, d3;
	void *p;
	wchar_t ws1[24], ws2[24];
	char cs1[24];
};

static int cp(bool port, const wchar_t *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int r = port ? P::vwprintf(fmt, ap) : ref_vwprintf(fmt, ap);
	va_end(ap);
	return r;
}

static int dispatch_printf(bool port, const wchar_t *fmt, int k, PArgs &a,
    Slab &s)
{
	switch (k) {
	case P_NONE:	return cp(port, fmt);
	case P_INT:	return cp(port, fmt, a.i1);
	case P_INT2:	return cp(port, fmt, a.i1, a.i2);
	case P_UINT:	return cp(port, fmt, a.u1);
	case P_UINT3:	return cp(port, fmt, a.u1, a.u2, a.u3);
	case P_WSTR:	return cp(port, fmt, a.ws1);
	case P_WSTR2:	return cp(port, fmt, a.ws1, a.ws2);
	case P_STR:	return cp(port, fmt, a.cs1);
	case P_WC:	return cp(port, fmt, (wint_t)a.u1);
	case P_DBL:	return cp(port, fmt, a.d1);
	case P_DBL3:	return cp(port, fmt, a.d1, a.d2, a.d3);
	case P_WID_I:	return cp(port, fmt, a.wid, a.i1);
	case P_PREC_D:	return cp(port, fmt, a.prec, a.d1);
	case P_WID_PREC_D: return cp(port, fmt, a.wid, a.prec, a.d1);
	case P_PTR:	return cp(port, fmt, a.p);
	case P_N:	return cp(port, fmt, s_i3(s));
	case P_LL:	return cp(port, fmt, a.ll);
	case P_LONG:	return cp(port, fmt, a.l1);
	case P_SIZE:	return cp(port, fmt, a.z);
	case P_IWD:	return cp(port, fmt, a.i1, a.ws1, a.d1);
	}
	abort();
}

static const size_t OUTMAX = 8192;

struct PCap {
	int rv;
	int en;
	int err;
	int err2;
	int flushrv;
	long tell;
	size_t n;
	Slab s;
	unsigned char out[OUTMAX];
};

static PCap pa, pb;

static void run_printf(bool port, const wchar_t *fmt, int kind, PArgs &args,
    bool fresh, PCap &c)
{
	slab_guard(c.s);
	memset(c.out, 0, sizeof c.out);
	reset_stdout(fresh);
	errno = 0;
	int r = dispatch_printf(port, fmt, kind, args, c.s);
	c.en = errno;
	c.rv = r;
	c.err = ferror(stdout) ? 1 : 0;
	c.tell = (long)ftell(stdout);
	c.flushrv = fflush(stdout);
	c.err2 = ferror(stdout) ? 1 : 0;
	ssize_t g = pread(g_out_rfd, c.out, OUTMAX, 0);
	if (g < 0)
		die("pread stdout");
	if ((size_t)g == OUTMAX)
		die("stdout capture overflow: widen OUTMAX");
	c.n = (size_t)g;
}

static void show_bytes(const unsigned char *d, size_t n)
{
	fputc('"', g_rep);
	for (size_t i = 0; i < n && i < 64; i++) {
		unsigned char ch = d[i];
		if (ch >= 0x20 && ch < 0x7f && ch != '"' && ch != '\\')
			fputc(ch, g_rep);
		else
			fprintf(g_rep, "\\x%02x", ch);
	}
	fprintf(g_rep, "\"%s", n > 64 ? "..." : "");
}

static void show_wfmt(const wchar_t *f)
{
	fputc('"', g_rep);
	for (size_t i = 0; f[i] && i < 64; i++) {
		unsigned long v = (unsigned long)f[i];
		if (v >= 0x20 && v < 0x7f)
			fputc((int)v, g_rep);
		else
			fprintf(g_rep, "\\u%04lx", v);
	}
	fputc('"', g_rep);
}

static void check_printf(const wchar_t *fmt, int kind, PArgs &args, bool fresh,
    const char *tag)
{
	run_printf(false, fmt, kind, args, fresh, pa);
	run_printf(true, fmt, kind, args, fresh, pb);
	t_printf.cases++;
	bool ok = pa.rv == pb.rv && pa.en == pb.en && pa.err == pb.err &&
	    pa.err2 == pb.err2 && pa.flushrv == pb.flushrv &&
	    pa.tell == pb.tell && pa.n == pb.n &&
	    memcmp(pa.out, pb.out, OUTMAX) == 0 &&
	    memcmp(pa.s.b, pb.s.b, SLAB) == 0;
	if (ok)
		return;
	t_printf.fails++;
	if (t_printf.shown++ >= SHOW_MAX)
		return;
	fprintf(g_rep, "  FAIL vwprintf [%s] kind=%d fmt=", tag, kind);
	show_wfmt(fmt);
	fprintf(g_rep, "\n    ref : rv=%d errno=%d err=%d/%d flush=%d tell=%ld n=%zu out=",
	    pa.rv, pa.en, pa.err, pa.err2, pa.flushrv, pa.tell, pa.n);
	show_bytes(pa.out, pa.n);
	fprintf(g_rep, "\n    port: rv=%d errno=%d err=%d/%d flush=%d tell=%ld n=%zu out=",
	    pb.rv, pb.en, pb.err, pb.err2, pb.flushrv, pb.tell, pb.n);
	show_bytes(pb.out, pb.n);
	fputc('\n', g_rep);
	dump_slab_diff(pa.s, pb.s);
}

struct PFmt {
	const wchar_t *fmt;
	int kind;
};

static const PFmt p_fmts[] = {
	{ L"", P_NONE },
	{ L"hello", P_NONE },
	{ L"%%", P_NONE },
	{ L"a%%b", P_NONE },
	{ L"%d", P_INT },
	{ L"%5d|%-5d", P_INT2 },
	{ L"%+05d", P_INT },
	{ L"% d", P_INT },
	{ L"%*d", P_WID_I },
	{ L"%.0d", P_INT },
	{ L"%c", P_INT },
	{ L"%u", P_UINT },
	{ L"%#x %#o %u", P_UINT3 },
	{ L"%X", P_UINT },
	{ L"%ls", P_WSTR },
	{ L"[%10ls][%-10.3ls]", P_WSTR2 },
	{ L"%.0ls", P_WSTR },
	{ L"%s", P_STR },
	{ L"%10.4s", P_STR },
	{ L"%lc", P_WC },
	{ L"%5lc", P_WC },
	{ L"%f %e %g", P_DBL3 },
	{ L"%.0f", P_DBL },
	{ L"%a", P_DBL },
	{ L"%.*f", P_PREC_D },
	{ L"%*.*f", P_WID_PREC_D },
	{ L"%20.10e", P_DBL },
	{ L"%G", P_DBL },
	{ L"%p", P_PTR },
	{ L"ab%ncd", P_N },
	{ L"%lld", P_LL },
	{ L"%ld", P_LONG },
	{ L"%zu", P_SIZE },
	{ L"%d[%ls]%.3f", P_IWD },
	{ L"%hhd %hd", P_INT2 },
	{ L"\x80\xff", P_NONE },
	{ L"%d\x80%ls", P_IWD },
};
static const size_t P_NFMT = sizeof(p_fmts) / sizeof(p_fmts[0]);

static void set_ws(wchar_t *dst, size_t cap, const wchar_t *src)
{
	size_t i = 0;
	for (; src[i] && i + 1 < cap; i++)
		dst[i] = src[i];
	dst[i] = L'\0';
}

static void hand_printf()
{
	static const wchar_t *wstrs[] = {
		L"", L"a", L"abc", L"\x7f", L"\x80", L"\xff",
		L"a\x80" L"b", L"\x100", L"\x10000",
		L"0123456789012345678901"
	};
	static const char *cstrs[] = {
		"", "a", "abc", "\x7f", "\x80", "\xff", "a\x80" "b",
		"0123456789012345678901"
	};
	static const int ints[] = { 0, 1, -1, 2, -2, 7, -7, INT_MAX, INT_MIN,
		255, -255, 1000000 };
	static const double dbls[] = { 0.0, -0.0, 1.0, -1.0, 0.5, -0.5,
		3.14159265358979, 1e300, 1e-300, DBL_MAX, DBL_MIN,
		4.9406564584124654e-324, 1.0 / 0.0, -1.0 / 0.0, 0.0 / 0.0 };
	static const size_t NW = sizeof(wstrs) / sizeof(wstrs[0]);
	static const size_t NC = sizeof(cstrs) / sizeof(cstrs[0]);
	static const size_t NI = sizeof(ints) / sizeof(ints[0]);
	static const size_t ND = sizeof(dbls) / sizeof(dbls[0]);
	size_t bundles = NW;
	if (NC > bundles) bundles = NC;
	if (NI > bundles) bundles = NI;
	if (ND > bundles) bundles = ND;

	PArgs a;
	for (size_t b = 0; b < bundles; b++) {
		memset(&a, 0, sizeof a);
		a.i1 = ints[b % NI];
		a.i2 = ints[(b + 3) % NI];
		a.wid = (int)(b % 9) - 4;
		a.prec = (int)(b % 7) - 1;
		a.u1 = (unsigned)ints[b % NI];
		a.u2 = (unsigned)ints[(b + 1) % NI];
		a.u3 = (unsigned)ints[(b + 2) % NI];
		a.l1 = (long)ints[b % NI];
		a.ll = (long long)ints[b % NI] * 1000000007LL;
		a.z = (size_t)(unsigned)ints[b % NI];
		a.d1 = dbls[b % ND];
		a.d2 = dbls[(b + 1) % ND];
		a.d3 = dbls[(b + 2) % ND];
		a.p = (void *)(uintptr_t)0x123456789abcdefULL;
		set_ws(a.ws1, 24, wstrs[b % NW]);
		set_ws(a.ws2, 24, wstrs[(b + 1) % NW]);
		strncpy(a.cs1, cstrs[b % NC], sizeof a.cs1 - 1);
		a.cs1[sizeof a.cs1 - 1] = '\0';
		for (size_t f = 0; f < P_NFMT; f++)
			check_printf(p_fmts[f].fmt, p_fmts[f].kind, a, true,
			    "hand");
	}
	static const int widths[] = { -40, -2, -1, 0, 1, 2, 39, 40 };
	static const int precs[] = { -1, 0, 1, 2, 17, 25 };
	for (size_t w = 0; w < sizeof(widths) / sizeof(widths[0]); w++)
		for (size_t p = 0; p < sizeof(precs) / sizeof(precs[0]); p++) {
			memset(&a, 0, sizeof a);
			a.wid = widths[w];
			a.prec = precs[p];
			a.i1 = -12345;
			a.d1 = -1.0 / 3.0;
			set_ws(a.ws1, 24, L"wz");
			check_printf(L"%*d", P_WID_I, a, true, "hand");
			check_printf(L"%.*f", P_PREC_D, a, true, "hand");
			check_printf(L"%*.*f", P_WID_PREC_D, a, true, "hand");
			check_printf(L"%-*.*e", P_WID_PREC_D, a, true, "hand");
		}
}

static double pick_double(Rng &r)
{
	unsigned k = r.below(100);
	if (k < 10)
		return 0.0;
	if (k < 14)
		return -0.0;
	if (k < 18)
		return 1.0 / 0.0;
	if (k < 22)
		return -1.0 / 0.0;
	if (k < 26)
		return 0.0 / 0.0;
	if (k < 34)
		return (double)(int)r.u32();
	if (k < 45) {
		double m = (double)(int)r.u32();
		return m / 4096.0;
	}
	if (k < 55)
		return DBL_MAX;
	if (k < 62)
		return DBL_MIN;
	if (k < 70)
		return 4.9406564584124654e-324;
	uint64_t bits = ((uint64_t)r.u32() << 32) | r.u32();
	double d;
	memcpy(&d, &bits, sizeof d);
	if (std::isnan(d))
		d = 1.5;
	return d;
}

static int gen_printf_fmt(Rng &r, wchar_t *out, size_t cap)
{
	struct Conv {
		const wchar_t *c;
		int k;
		int numeric;
	};
	static const Conv tbl[] = {
		{ L"d", P_INT, 1 }, { L"i", P_INT, 1 }, { L"c", P_INT, 0 },
		{ L"u", P_UINT, 1 }, { L"x", P_UINT, 1 }, { L"X", P_UINT, 1 },
		{ L"o", P_UINT, 1 }, { L"ls", P_WSTR, 0 }, { L"s", P_STR, 0 },
		{ L"f", P_DBL, 1 }, { L"e", P_DBL, 1 }, { L"g", P_DBL, 1 },
		{ L"E", P_DBL, 1 }, { L"G", P_DBL, 1 }, { L"a", P_DBL, 1 },
		{ L"lc", P_WC, 0 }, { L"lld", P_LL, 1 }, { L"ld", P_LONG, 1 },
		{ L"zu", P_SIZE, 1 }, { L"p", P_PTR, 0 }, { L"hhd", P_INT, 1 },
		{ L"hd", P_INT, 1 },
	};
	static const size_t NT = sizeof(tbl) / sizeof(tbl[0]);
	size_t o = 0;
	int kind = P_NONE;

	unsigned pre = r.below(4);
	for (unsigned i = 0; i < pre && o + 3 < cap; i++) {
		wchar_t ch = pick_wchar(r);
		if (ch == L'%') {
			out[o++] = L'%';
			out[o++] = L'%';
		} else if (ch) {
			out[o++] = ch;
		}
	}
	if (r.chance(85) && o + 32 < cap) {
		const Conv &cv = tbl[r.below((unsigned)NT)];
		out[o++] = L'%';
		unsigned nflag = r.below(3);
		for (unsigned i = 0; i < nflag; i++) {
			static const wchar_t fl[] = { L'-', L'+', L' ', L'0' };
			out[o++] = fl[r.below(4)];
		}
		if (cv.numeric && r.chance(20))
			out[o++] = L'#';
		if (r.chance(50)) {
			unsigned w = r.below(41);
			wchar_t tmp[8];
			int tn = 0;
			if (!w)
				tmp[tn++] = L'0';
			while (w) {
				tmp[tn++] = (wchar_t)(L'0' + (w % 10));
				w /= 10;
			}
			while (tn)
				out[o++] = tmp[--tn];
		}
		if (r.chance(40)) {
			out[o++] = L'.';
			unsigned pr = r.below(21);
			wchar_t tmp[8];
			int tn = 0;
			if (!pr)
				tmp[tn++] = L'0';
			while (pr) {
				tmp[tn++] = (wchar_t)(L'0' + (pr % 10));
				pr /= 10;
			}
			while (tn)
				out[o++] = tmp[--tn];
		}
		for (size_t i = 0; cv.c[i]; i++)
			out[o++] = cv.c[i];
		kind = cv.k;
	}
	unsigned post = r.below(4);
	for (unsigned i = 0; i < post && o + 3 < cap; i++) {
		wchar_t ch = pick_wchar(r);
		if (ch == L'%') {
			out[o++] = L'%';
			out[o++] = L'%';
		} else if (ch) {
			out[o++] = ch;
		}
	}
	out[o] = L'\0';
	return kind;
}

static void sweep_printf(long iters)
{
	Rng r(0x0098B2BEEFCAFE1234ULL);
	wchar_t fmt[128];
	PArgs a;
	for (long it = 0; it < iters; it++) {
		memset(&a, 0, sizeof a);
		a.i1 = (int)r.u32();
		if (r.chance(30))
			a.i1 = (int)(r.below(80)) - 40;
		a.i2 = (int)(r.below(80)) - 40;
		a.wid = (int)(r.below(84)) - 42;
		a.prec = (int)(r.below(28)) - 2;
		a.u1 = r.u32();
		if (r.chance(35))
			a.u1 = r.below(0x120);
		a.u2 = r.u32();
		a.u3 = r.below(9);
		a.l1 = (long)(int)r.u32();
		a.ll = (long long)((unsigned long long)r.u32() << 32 | r.u32());
		a.z = (size_t)r.u32();
		a.d1 = pick_double(r);
		a.d2 = pick_double(r);
		a.d3 = pick_double(r);
		a.p = r.chance(20) ? nullptr
				   : (void *)(uintptr_t)((unsigned long long)r.u32() << 4);
		unsigned wn = r.below(12);
		for (unsigned i = 0; i < wn; i++)
			a.ws1[i] = pick_wchar(r);
		a.ws1[wn] = L'\0';
		wn = r.below(12);
		for (unsigned i = 0; i < wn; i++)
			a.ws2[i] = pick_wchar(r);
		a.ws2[wn] = L'\0';
		unsigned cn = r.below(12);
		for (unsigned i = 0; i < cn; i++) {
			unsigned char ch = pick_byte(r);
			a.cs1[i] = (char)(ch ? ch : 'z');
		}
		a.cs1[cn] = '\0';

		int kind;
		const wchar_t *use;
		if (r.chance(50)) {
			size_t pi = r.below((unsigned)P_NFMT);
			use = p_fmts[pi].fmt;
			kind = p_fmts[pi].kind;
		} else {
			kind = gen_printf_fmt(r, fmt, 128);
			use = fmt;
		}
		bool fresh = (it % 4096) == 0;
		check_printf(use, kind, a, fresh, "sweep");
	}
}

static void warmup()
{
	static PArgs a;
	memset(&a, 0, sizeof a);
	a.d1 = a.d2 = a.d3 = 1.5;
	a.i1 = a.i2 = 1;
	a.u1 = 65;
	a.z = 1;
	a.p = (void *)(uintptr_t)1;
	set_ws(a.ws1, 24, L"aa");
	set_ws(a.ws2, 24, L"bb");
	strcpy(a.cs1, "aa");
	for (int pass = 0; pass < 2; pass++) {
		bool port = pass != 0;
		run_printf(port, L"%d", P_INT, a, true, pa);
		run_printf(port, L"%ls", P_WSTR, a, true, pa);
		run_printf(port, L"%s", P_STR, a, true, pa);
		run_printf(port, L"%f %e %g", P_DBL3, a, true, pa);
		run_printf(port, L"%a", P_DBL, a, true, pa);
		run_printf(port, L"%lc", P_WC, a, true, pa);
		run_printf(port, L"%p", P_PTR, a, true, pa);
		run_printf(port, L"%zu", P_SIZE, a, true, pa);
	}
}

int main()
{
	init_streams();
	warmup();

	const long SWEEP = 200000;

	hand_printf();
	sweep_printf(SWEEP);

	fprintf(g_rep, "\n%-12s %12s %12s  %s\n", "function", "cases",
	    "failures", "result");
	fprintf(g_rep, "------------------------------------------------------\n");
	fprintf(g_rep, "%-12s %12ld %12ld  %s\n", t_printf.name,
	    t_printf.cases, t_printf.fails,
	    t_printf.fails ? "FAIL" : "ok");
	fprintf(g_rep, "------------------------------------------------------\n");
	fprintf(g_rep, "%-12s %12ld %12ld  %s\n", "TOTAL",
	    t_printf.cases, t_printf.fails,
	    t_printf.fails ? "FAIL" : "ok");

	cleanup_streams();
	return t_printf.fails ? 1 : 0;
}
