/*
 * harness.cpp -- differential test: PBSD port (batch b0098s1) vs ref_ oracle.
 *
 * Function under test:
 *   vwscanf(const wchar_t *, va_list)   -- reads from stdin
 */

#include <cerrno>
#include <climits>
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

import pbsd.lib.libc.stdio.b0098s1;

namespace P = pbsd::lib_libc_stdio::b0098s1;

extern "C" int ref_vwscanf(const wchar_t *__restrict fmt, va_list ap);

/* ------------------------------------------------------------------ misc -- */

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

static Tally t_scanf = { "vwscanf", 0, 0, 0 };

static const long SHOW_MAX = 8;

/* splitmix64: fixed seed, fully reproducible. */
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

/* --------------------------------------------------------------- streams -- */

static char g_in_path[] = "/tmp/pbsd_b0098s1_in_XXXXXX";

static int g_in_wfd = -1;

static void init_streams()
{
	int fd = dup(1);
	if (fd < 0)
		die("dup");
	g_rep = fdopen(fd, "w");
	if (!g_rep)
		die("fdopen");

	g_in_wfd = mkstemp(g_in_path);
	if (g_in_wfd < 0)
		die("mkstemp");

	if (!freopen(g_in_path, "r", stdin))
		die("freopen stdin");
}

static void cleanup_streams()
{
	fflush(g_rep);
	if (g_in_wfd >= 0)
		close(g_in_wfd);
	unlink(g_in_path);
}

static void set_stdin(const unsigned char *d, size_t n, bool /*fresh*/)
{
	if (ftruncate(g_in_wfd, 0) != 0)
		die("ftruncate stdin");
	if (n && pwrite(g_in_wfd, d, n, 0) != (ssize_t)n)
		die("pwrite stdin");
	if (!freopen(g_in_path, "r", stdin))
		die("freopen stdin");
}

/* ------------------------------------------------------------------ slab -- */
/*
 * One guard-filled blob holds every scanf/printf output object.  Comparing the
 * whole blob catches writes outside the nominal window as well as inside it.
 */
static const size_t SLAB = 512;

struct Slab {
	alignas(16) unsigned char b[SLAB];
};

static inline int *s_i1(Slab &s) { return (int *)(s.b + 32); }
static inline int *s_i2(Slab &s) { return (int *)(s.b + 44); }
static inline int *s_i3(Slab &s) { return (int *)(s.b + 56); }
static inline wchar_t *s_w1(Slab &s) { return (wchar_t *)(s.b + 64); }  /* 32 wchar_t */
static inline wchar_t *s_w2(Slab &s) { return (wchar_t *)(s.b + 200); } /* 32 wchar_t */
static inline double *s_d1(Slab &s) { return (double *)(s.b + 336); }
static inline double *s_d2(Slab &s) { return (double *)(s.b + 352); }
static inline long long *s_ll(Slab &s) { return (long long *)(s.b + 368); }
static inline char *s_c(Slab &s) { return (char *)(s.b + 400); }
static inline unsigned *s_u(Slab &s) { return (unsigned *)(s.b + 408); }

static void slab_guard(Slab &s) { memset(s.b, 0x7f, SLAB); }

static void dump_slab_diff(const Slab &a, const Slab &b)
{
	for (size_t i = 0; i < SLAB; i++)
		if (a.b[i] != b.b[i])
			fprintf(g_rep, "      slab[%3zu]: ref=%02x port=%02x\n",
			    i, a.b[i], b.b[i]);
}

/* ----------------------------------------------------------- generators -- */

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
		return 0; /* NUL-heavy */
	if (k < 84)
		return 0x80;
	if (k < 88)
		return 0xff;
	return (unsigned char)(0x80 + r.below(0x80)); /* high bit */
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

/* ---------------------------------------------------------- vwscanf test -- */

enum SKind {
	S_NONE, S_INT, S_INT2, S_UINT, S_DBL, S_DBL2, S_WS, S_WS2,
	S_LC, S_LL, S_N, S_CHAR, S_INT_WS, S_INT_N
};

static int cs(bool port, const wchar_t *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int r = port ? P::vwscanf(fmt, ap) : ref_vwscanf(fmt, ap);
	va_end(ap);
	return r;
}

static int dispatch_scanf(bool port, const wchar_t *fmt, int k, Slab &s)
{
	switch (k) {
	case S_NONE:	return cs(port, fmt);
	case S_INT:	return cs(port, fmt, s_i1(s));
	case S_INT2:	return cs(port, fmt, s_i1(s), s_i2(s));
	case S_UINT:	return cs(port, fmt, s_u(s));
	case S_DBL:	return cs(port, fmt, s_d1(s));
	case S_DBL2:	return cs(port, fmt, s_d1(s), s_d2(s));
	case S_WS:	return cs(port, fmt, s_w1(s));
	case S_WS2:	return cs(port, fmt, s_w1(s), s_w2(s));
	case S_LC:	return cs(port, fmt, s_w1(s));
	case S_LL:	return cs(port, fmt, s_ll(s));
	case S_N:	return cs(port, fmt, s_i3(s));
	case S_CHAR:	return cs(port, fmt, s_c(s));
	case S_INT_WS:	return cs(port, fmt, s_i1(s), s_w1(s));
	case S_INT_N:	return cs(port, fmt, s_i1(s), s_i3(s));
	}
	abort();
}

struct SCap {
	int rv;
	int en;
	int err;
	int eof;
	long tell;
	long lpos;
	Slab s;
};

static void run_scanf(bool port, const wchar_t *fmt, int kind,
    const unsigned char *in, size_t n, bool fresh, SCap &c)
{
	slab_guard(c.s);
	set_stdin(in, n, fresh);
	errno = 0;
	int r = dispatch_scanf(port, fmt, kind, c.s);
	c.en = errno;
	c.rv = r;
	c.err = ferror(stdin) ? 1 : 0;
	c.eof = feof(stdin) ? 1 : 0;
	c.tell = (long)ftell(stdin);
	c.lpos = (long)lseek(fileno(stdin), 0, SEEK_CUR);
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

static void check_scanf(const wchar_t *fmt, int kind, const unsigned char *in,
    size_t n, bool fresh, const char *tag)
{
	static SCap a, b;
	run_scanf(false, fmt, kind, in, n, fresh, a);
	run_scanf(true, fmt, kind, in, n, fresh, b);
	t_scanf.cases++;
	bool ok = a.rv == b.rv && a.en == b.en && a.err == b.err &&
	    a.eof == b.eof && a.tell == b.tell && a.lpos == b.lpos &&
	    memcmp(a.s.b, b.s.b, SLAB) == 0;
	if (ok)
		return;
	t_scanf.fails++;
	if (t_scanf.shown++ >= SHOW_MAX)
		return;
	fprintf(g_rep, "  FAIL vwscanf [%s] kind=%d fmt=", tag, kind);
	show_wfmt(fmt);
	fprintf(g_rep, " in=");
	show_bytes(in, n);
	fprintf(g_rep, "\n    ref : rv=%d errno=%d err=%d eof=%d tell=%ld lpos=%ld\n",
	    a.rv, a.en, a.err, a.eof, a.tell, a.lpos);
	fprintf(g_rep, "    port: rv=%d errno=%d err=%d eof=%d tell=%ld lpos=%ld\n",
	    b.rv, b.en, b.err, b.eof, b.tell, b.lpos);
	dump_slab_diff(a.s, b.s);
}

struct SFmt {
	const wchar_t *fmt;
	int kind;
};

static const SFmt s_fmts[] = {
	{ L"", S_NONE },
	{ L"abc", S_NONE },
	{ L" ", S_NONE },
	{ L"%%", S_NONE },
	{ L"%d", S_INT },
	{ L"%2d", S_INT },
	{ L"%1d", S_INT },
	{ L"%d %d", S_INT2 },
	{ L"%d,%d", S_INT2 },
	{ L"%*d%d", S_INT },
	{ L"%i", S_INT },
	{ L"%u", S_UINT },
	{ L"%x", S_UINT },
	{ L"%o", S_UINT },
	{ L"%lf", S_DBL },
	{ L"%lf%lf", S_DBL2 },
	{ L"%31ls", S_WS },
	{ L"%1ls", S_WS },
	{ L"%31ls%31ls", S_WS2 },
	{ L"%lc", S_LC },
	{ L"%3lc", S_LC },
	{ L"%31l[0-9]", S_WS },
	{ L"%31l[^0-9]", S_WS },
	{ L"%31l[]]", S_WS },
	{ L"%lld", S_LL },
	{ L"%n", S_N },
	{ L"%d%n", S_INT_N },
	{ L"%hhd", S_CHAR },
	{ L"%d%31ls", S_INT_WS },
	{ L"%d%*c%d", S_INT2 },
	{ L"x%d", S_INT },
	{ L"%*31ls", S_NONE },
};
static const size_t S_NFMT = sizeof(s_fmts) / sizeof(s_fmts[0]);

struct SIn {
	const char *d;
	int n; /* -1 => strlen */
};

static const SIn s_ins[] = {
	{ "", 0 },
	{ "\0", 1 },
	{ "\0\0\0\0", 4 },
	{ "0", -1 },
	{ "1", -1 },
	{ "9", -1 },
	{ " ", -1 },
	{ "  \t\n\v\f\r  ", -1 },
	{ "\n", -1 },
	{ "-", -1 },
	{ "+", -1 },
	{ "-0", -1 },
	{ "0x", -1 },
	{ "0x10", -1 },
	{ "0X7F", -1 },
	{ "007", -1 },
	{ "08", -1 },
	{ "2147483647", -1 },
	{ "-2147483648", -1 },
	{ "2147483648", -1 },
	{ "4294967295", -1 },
	{ "4294967296", -1 },
	{ "9223372036854775807", -1 },
	{ "-9223372036854775809", -1 },
	{ "99999999999999999999999999", -1 },
	{ "abc", -1 },
	{ "abc def", -1 },
	{ "]]]", -1 },
	{ "0.5", -1 },
	{ ".5", -1 },
	{ "5.", -1 },
	{ "1e-3", -1 },
	{ "1e999", -1 },
	{ "-1e-999", -1 },
	{ "nan", -1 },
	{ "nan(1)", -1 },
	{ "inf", -1 },
	{ "-INFINITY", -1 },
	{ "0x1p+3", -1 },
	{ "1\0002", 3 },
	{ "12\0" "34", 5 },
	{ "\x80", 1 },
	{ "\xff", 1 },
	{ "\x80\x80\x80", 3 },
	{ "\xff\xfe\xfd\xfc", 4 },
	{ "1\x80", 2 },
	{ "\x80" "1", 2 },
	{ "12 \xc3\xa9 34", -1 },
	{ "\x7f", 1 },
	{ "\x01\x02\x03", 3 },
	{ "1 2 3 4 5", -1 },
	{ "  12,34  ", -1 },
	{ "1234567890123456789012345678901234567890", -1 },
	{ "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", -1 },
	{ "0000000000000000000000000000000000000000", -1 },
};
static const size_t S_NIN = sizeof(s_ins) / sizeof(s_ins[0]);

static void hand_scanf()
{
	for (size_t f = 0; f < S_NFMT; f++)
		for (size_t i = 0; i < S_NIN; i++) {
			size_t n = s_ins[i].n < 0 ? strlen(s_ins[i].d)
						  : (size_t)s_ins[i].n;
			check_scanf(s_fmts[f].fmt, s_fmts[f].kind,
			    (const unsigned char *)s_ins[i].d, n, true, "hand");
		}
}

static int gen_scanf_fmt(Rng &r, wchar_t *out, size_t cap)
{
	static const wchar_t *sets[] = {
		L"0-9", L"^0-9", L"a-z", L"^ ", L" \t", L"]", L"^]", L"^\n",
		L"0-9a-fA-F", L"^abc"
	};
	size_t o = 0;
	int kind = S_NONE;

	unsigned pre = r.below(3);
	for (unsigned i = 0; i < pre && o + 3 < cap; i++) {
		wchar_t ch = pick_wchar(r);
		if (ch == L'%') {
			out[o++] = L'%';
			out[o++] = L'%';
		} else if (ch) {
			out[o++] = ch;
		}
	}

	if (r.chance(88) && o + 24 < cap) {
		out[o++] = L'%';
		bool suppress = r.chance(15);
		if (suppress)
			out[o++] = L'*';
		unsigned pick = r.below(12);
		const wchar_t *conv = nullptr;
		int k = S_NONE;
		switch (pick) {
		case 0: conv = L"d"; k = S_INT; break;
		case 1: conv = L"i"; k = S_INT; break;
		case 2: conv = L"u"; k = S_UINT; break;
		case 3: conv = L"x"; k = S_UINT; break;
		case 4: conv = L"o"; k = S_UINT; break;
		case 5: conv = L"lf"; k = S_DBL; break;
		case 6: conv = L"ls"; k = S_WS; break;
		case 7: conv = L"lc"; k = S_LC; break;
		case 8: conv = L"lld"; k = S_LL; break;
		case 9: conv = L"hhd"; k = S_CHAR; break;
		case 10: conv = nullptr; k = S_WS; break; /* scanset */
		default: conv = L"n"; k = S_N; break;
		}
		bool needw = (k == S_WS || k == S_LC);
		if (k != S_N && (needw || r.chance(45))) {
			unsigned w = needw ? 1 + r.below(31) : 1 + r.below(22);
			wchar_t tmp[8];
			int tn = 0;
			while (w) {
				tmp[tn++] = (wchar_t)(L'0' + (w % 10));
				w /= 10;
			}
			while (tn)
				out[o++] = tmp[--tn];
		}
		if (conv) {
			for (size_t i = 0; conv[i]; i++)
				out[o++] = conv[i];
		} else {
			const wchar_t *set = sets[r.below(10)];
			out[o++] = L'l';
			out[o++] = L'[';
			for (size_t i = 0; set[i]; i++)
				out[o++] = set[i];
			out[o++] = L']';
		}
		kind = suppress ? S_NONE : k;
	}

	unsigned post = r.below(3);
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

static void sweep_scanf(long iters)
{
	Rng r(0x0098C0DE5CA7F00DULL);
	wchar_t fmt[96];
	unsigned char in[80];
	for (long it = 0; it < iters; it++) {
		int kind;
		const wchar_t *use;
		if (r.chance(55)) {
			size_t pi = r.below((unsigned)S_NFMT);
			use = s_fmts[pi].fmt;
			kind = s_fmts[pi].kind;
		} else {
			kind = gen_scanf_fmt(r, fmt, 96);
			use = fmt;
		}
		size_t n;
		unsigned shape = r.below(100);
		if (shape < 8)
			n = 0;
		else if (shape < 16)
			n = 1;
		else if (shape < 24)
			n = 2;
		else
			n = r.below(sizeof(in) + 1);
		for (size_t i = 0; i < n; i++)
			in[i] = pick_byte(r);
		bool fresh = (it % 4096) == 0;
		check_scanf(use, kind, in, n, fresh, "sweep");
	}
}


static void warmup()
{
	static SCap sc;
	const unsigned char in[] = "12 34 ab 5.5 x";
	for (int pass = 0; pass < 2; pass++) {
		bool port = pass != 0;
		run_scanf(port, L"%d", S_INT, in, sizeof in - 1, true, sc);
		run_scanf(port, L"%31ls", S_WS, in, sizeof in - 1, true, sc);
		run_scanf(port, L"%lf", S_DBL, in, sizeof in - 1, true, sc);
		run_scanf(port, L"%lc", S_LC, in, sizeof in - 1, true, sc);
		run_scanf(port, L"%31l[0-9]", S_WS, in, sizeof in - 1, true, sc);
	}
	(void)sc;
}

int main()
{
	init_streams();
	warmup();

	const long SWEEP = 200000;

	hand_scanf();
	sweep_scanf(SWEEP);

	fprintf(g_rep, "\n%-12s %12s %12s  %s\n", "function", "cases",
	    "failures", "result");
	fprintf(g_rep, "------------------------------------------------------\n");
	fprintf(g_rep, "%-12s %12ld %12ld  %s\n", t_scanf.name,
	    t_scanf.cases, t_scanf.fails, t_scanf.fails ? "FAIL" : "ok");
	fprintf(g_rep, "------------------------------------------------------\n");
	fprintf(g_rep, "%-12s %12ld %12ld  %s\n", "TOTAL",
	    t_scanf.cases, t_scanf.fails, t_scanf.fails ? "FAIL" : "ok");

	cleanup_streams();
	return t_scanf.fails ? 1 : 0;
}
