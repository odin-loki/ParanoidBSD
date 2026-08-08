/*
 * harness.cpp -- differential test: PBSD port (batch b0098) vs ref_ oracle.
 *
 * Functions under test:
 *   vwscanf(const wchar_t *, va_list)   -- reads from stdin
 *   vwprintf(const wchar_t *, va_list)  -- writes to stdout
 *   setbuf(FILE *, char *)              -- installs a caller buffer
 *
 * Because all three touch the standard streams, stdin/stdout are redirected to
 * temporary files and the report is written to a dup() of the original fd 1.
 *
 * Every case runs BOTH implementations against byte-identical stream state and
 * compares every observable: return value, errno, error/eof flags, ftell(),
 * the raw fd offset, the bytes on disk, and the ENTIRE argument slab / caller
 * buffer (guard-filled with 0x7f) including bytes past the nominal write
 * window.
 */

#include <cerrno>
#include <cfloat>
#include <climits>
#include <clocale>
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

import pbsd.lib.libc.stdio.b0098;

namespace P = pbsd::lib_libc_stdio::b0098;

extern "C" int ref_vwscanf(const wchar_t *__restrict fmt, va_list ap);
extern "C" int ref_vwprintf(const wchar_t *__restrict fmt, va_list ap);
extern "C" void ref_setbuf(FILE *__restrict fp, char *__restrict buf);

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

static Tally t_scanf  = { "vwscanf", 0, 0, 0 };
static Tally t_printf = { "vwprintf", 0, 0, 0 };
static Tally t_setbuf = { "setbuf", 0, 0, 0 };

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

static char g_out_path[] = "/tmp/pbsd_b0098_out_XXXXXX";
static char g_in_path[]  = "/tmp/pbsd_b0098_in_XXXXXX";
static char g_sb_a[]     = "/tmp/pbsd_b0098_sba_XXXXXX";
static char g_sb_b[]     = "/tmp/pbsd_b0098_sbb_XXXXXX";

static int g_out_rfd = -1; /* independent read fd on g_out_path */
static int g_in_wfd  = -1; /* independent write fd on g_in_path  */

static void make_tmp(char *tmpl, int *keep_fd)
{
	int fd = mkstemp(tmpl);
	if (fd < 0)
		die("mkstemp");
	if (keep_fd)
		*keep_fd = fd;
	else
		close(fd);
}

static void init_streams()
{
	int fd = dup(1);
	if (fd < 0)
		die("dup");
	g_rep = fdopen(fd, "w");
	if (!g_rep)
		die("fdopen");

	make_tmp(g_out_path, &g_out_rfd);
	make_tmp(g_in_path, &g_in_wfd);
	make_tmp(g_sb_a, nullptr);
	make_tmp(g_sb_b, nullptr);

	if (!freopen(g_out_path, "w+", stdout))
		die("freopen stdout");
	if (!freopen(g_in_path, "r", stdin))
		die("freopen stdin");
}

static void cleanup_streams()
{
	fflush(g_rep);
	unlink(g_out_path);
	unlink(g_in_path);
	unlink(g_sb_a);
	unlink(g_sb_b);
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

static void set_stdin(const unsigned char *d, size_t n, bool fresh)
{
	if (ftruncate(g_in_wfd, 0) != 0)
		die("ftruncate stdin");
	if (n && pwrite(g_in_wfd, d, n, 0) != (ssize_t)n)
		die("pwrite stdin");
	if (fresh) {
		if (!freopen(g_in_path, "r", stdin))
			die("freopen stdin");
	} else {
		rewind(stdin);
		clearerr(stdin);
	}
}

static long path_size(const char *p)
{
	struct stat st;
	if (stat(p, &st) != 0)
		return -1;
	return (long)st.st_size;
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

/* --------------------------------------------------------- vwprintf test -- */

enum PKind {
	P_NONE, P_INT, P_INT2, P_UINT, P_UINT3, P_WSTR, P_WSTR2, P_STR,
	P_WC, P_DBL, P_DBL3, P_ID, P_PTR, P_N, P_LL, P_LONG, P_SIZE, P_IWD
};

struct PArgs {
	int i1, i2;
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
	case P_ID:	return cp(port, fmt, a.i1, a.d1);
	case P_PTR:	return cp(port, fmt, a.p);
	case P_N:	return cp(port, fmt, s_i3(s));
	case P_LL:	return cp(port, fmt, a.ll);
	case P_LONG:	return cp(port, fmt, a.l1);
	case P_SIZE:	return cp(port, fmt, a.z);
	case P_IWD:	return cp(port, fmt, a.i1, a.ws1, a.d1);
	}
	abort();
}

static const size_t OUTMAX = 1 << 16;

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
	c.n = (size_t)g;
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
	{ L"%*d", P_INT2 },
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
	{ L"%.*f", P_ID },
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
		/* %*d width: exercise negative, zero and positive widths. */
		for (size_t f = 0; f < P_NFMT; f++)
			check_printf(p_fmts[f].fmt, p_fmts[f].kind, a, true,
			    "hand");
	}
	/* %*d and %.*f with boundary width/precision values. */
	static const int widths[] = { -30, -1, 0, 1, 2, 30, 40 };
	for (size_t w = 0; w < sizeof(widths) / sizeof(widths[0]); w++) {
		memset(&a, 0, sizeof a);
		a.i1 = widths[w];
		a.i2 = -12345;
		a.d1 = -1.0 / 3.0;
		set_ws(a.ws1, 24, L"wz");
		check_printf(L"%*d", P_INT2, a, true, "hand");
		check_printf(L"%.*f", P_ID, a, true, "hand");
		check_printf(L"%*.*f", P_ID, a, true, "hand");
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
	Rng r(0x0098BEEFCAFE1234ULL);
	wchar_t fmt[128];
	PArgs a;
	for (long it = 0; it < iters; it++) {
		memset(&a, 0, sizeof a);
		a.i1 = (int)r.u32();
		if (r.chance(30))
			a.i1 = (int)(r.below(80)) - 40;
		a.i2 = (int)(r.below(80)) - 40;
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

/* ---------------------------------------------------------- setbuf test -- */

static const size_t SB_BUFCAP = (size_t)BUFSIZ + 64;
static const size_t SB_DATAMAX = (size_t)BUFSIZ * 2 + 64;
static const size_t SB_FILECAP = (size_t)BUFSIZ * 2 + 512;

struct BCap {
	long fsize_before;
	long lpos;
	long tell;
	long fsize_after;
	int err;
	int eof;
	int en;
	int flush_mid;
	int flush_end;
	int closerv;
	int nrd;
	int rd[72];
	size_t fn;
	unsigned char fb[SB_FILECAP];
	unsigned char buf[SB_BUFCAP];
};

static BCap ba, bb;

struct SbSpec {
	int mode;    /* 0 = write test, 1 = read test */
	int use_buf; /* pass caller buffer or NULL */
	size_t n;    /* payload size */
	int method;
	size_t nread;
	int flush_mid;
};

static void run_setbuf(bool port, const char *path, const SbSpec &sp,
    const unsigned char *data, BCap &c)
{
	memset(&c, 0, sizeof c);
	memset(c.buf, 0x7f, SB_BUFCAP);
	c.flush_mid = -3;

	FILE *f;
	if (sp.mode == 1) {
		int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
		if (fd < 0)
			die("open");
		if (sp.n && write(fd, data, sp.n) != (ssize_t)sp.n)
			die("write");
		close(fd);
		f = fopen(path, "r");
	} else {
		f = fopen(path, "w+");
	}
	if (!f)
		die("fopen");

	if (port)
		P::setbuf(f, sp.use_buf ? (char *)c.buf : nullptr);
	else
		ref_setbuf(f, sp.use_buf ? (char *)c.buf : nullptr);

	errno = 0;
	if (sp.mode == 0) {
		switch (sp.method) {
		case 0:
			fwrite(data, 1, sp.n, f);
			break;
		case 1:
			for (size_t i = 0; i < sp.n; i++)
				fputc(data[i], f);
			break;
		default: {
			size_t i = 0;
			while (i < sp.n) {
				size_t k = sp.n - i;
				if (k > 7)
					k = 7;
				fwrite(data + i, 1, k, f);
				i += k;
			}
			break;
		}
		}
		if (sp.flush_mid)
			c.flush_mid = fflush(f);
	} else {
		size_t k = sp.nread;
		if (k > 72)
			k = 72;
		switch (sp.method) {
		case 0:
			c.nrd = (int)k;
			for (size_t i = 0; i < k; i++)
				c.rd[i] = fgetc(f);
			break;
		case 1: {
			unsigned char tmp[72];
			size_t got = fread(tmp, 1, k, f);
			c.nrd = (int)k;
			for (size_t i = 0; i < k; i++)
				c.rd[i] = (i < got) ? (int)tmp[i] : -2;
			break;
		}
		default: {
			int ch = fgetc(f);
			c.rd[0] = ch;
			if (ch != EOF)
				ungetc(ch, f);
			c.rd[1] = fgetc(f);
			c.nrd = 2;
			break;
		}
		}
	}
	c.en = errno;
	c.err = ferror(f) ? 1 : 0;
	c.eof = feof(f) ? 1 : 0;
	c.fsize_before = path_size(path);
	c.lpos = (long)lseek(fileno(f), 0, SEEK_CUR);
	c.tell = (long)ftell(f);
	c.flush_end = fflush(f);
	c.fsize_after = path_size(path);
	c.closerv = fclose(f);

	int fd = open(path, O_RDONLY);
	if (fd < 0)
		die("reopen");
	ssize_t g = read(fd, c.fb, SB_FILECAP);
	close(fd);
	c.fn = (g > 0) ? (size_t)g : 0;
}

static void check_setbuf(const SbSpec &sp, const unsigned char *data,
    const char *tag)
{
	run_setbuf(false, g_sb_a, sp, data, ba);
	run_setbuf(true, g_sb_b, sp, data, bb);
	t_setbuf.cases++;
	if (memcmp(&ba, &bb, sizeof ba) == 0)
		return;
	t_setbuf.fails++;
	if (t_setbuf.shown++ >= SHOW_MAX)
		return;
	fprintf(g_rep,
	    "  FAIL setbuf [%s] mode=%d use_buf=%d n=%zu method=%d nread=%zu flush_mid=%d\n",
	    tag, sp.mode, sp.use_buf, sp.n, sp.method, sp.nread, sp.flush_mid);
	fprintf(g_rep,
	    "    ref : size_before=%ld lpos=%ld tell=%ld size_after=%ld err=%d eof=%d errno=%d fm=%d fe=%d close=%d fn=%zu\n",
	    ba.fsize_before, ba.lpos, ba.tell, ba.fsize_after, ba.err, ba.eof,
	    ba.en, ba.flush_mid, ba.flush_end, ba.closerv, ba.fn);
	fprintf(g_rep,
	    "    port: size_before=%ld lpos=%ld tell=%ld size_after=%ld err=%d eof=%d errno=%d fm=%d fe=%d close=%d fn=%zu\n",
	    bb.fsize_before, bb.lpos, bb.tell, bb.fsize_after, bb.err, bb.eof,
	    bb.en, bb.flush_mid, bb.flush_end, bb.closerv, bb.fn);
	if (ba.nrd != bb.nrd)
		fprintf(g_rep, "    nrd differs: %d vs %d\n", ba.nrd, bb.nrd);
	else
		for (int i = 0; i < ba.nrd; i++)
			if (ba.rd[i] != bb.rd[i])
				fprintf(g_rep, "    rd[%d]: ref=%d port=%d\n",
				    i, ba.rd[i], bb.rd[i]);
	long shown = 0;
	for (size_t i = 0; i < SB_BUFCAP && shown < 8; i++)
		if (ba.buf[i] != bb.buf[i]) {
			fprintf(g_rep, "    buf[%zu]: ref=%02x port=%02x\n", i,
			    ba.buf[i], bb.buf[i]);
			shown++;
		}
	shown = 0;
	for (size_t i = 0; i < SB_FILECAP && shown < 8; i++)
		if (ba.fb[i] != bb.fb[i]) {
			fprintf(g_rep, "    file[%zu]: ref=%02x port=%02x\n", i,
			    ba.fb[i], bb.fb[i]);
			shown++;
		}
}

static unsigned char *g_data;

static void fill_pattern(unsigned char *d, size_t n, int which)
{
	for (size_t i = 0; i < n; i++) {
		switch (which) {
		case 0:
			d[i] = (unsigned char)('A' + (i % 26));
			break;
		case 1:
			d[i] = (i % 17 == 0) ? 0
					     : (unsigned char)(0x80 | (i & 0x7f));
			break;
		case 2:
			d[i] = 0;
			break;
		default:
			d[i] = (unsigned char)(i * 7 + 0x80);
			break;
		}
	}
}

static void hand_setbuf()
{
	const size_t bs = (size_t)BUFSIZ;
	size_t sizes[] = { 0, 1, 2, 7, 8, 63, 64, 127, bs - 2, bs - 1, bs,
		bs + 1, bs + 2, 2 * bs - 1, 2 * bs, 2 * bs + 7 };
	const size_t NS = sizeof(sizes) / sizeof(sizes[0]);
	size_t reads[] = { 1, 2, 3, 8, 72 };
	const size_t NR = sizeof(reads) / sizeof(reads[0]);

	for (int pat = 0; pat < 4; pat++) {
		fill_pattern(g_data, SB_DATAMAX, pat);
		for (size_t si = 0; si < NS; si++) {
			size_t n = sizes[si];
			if (n > SB_DATAMAX)
				continue;
			for (int ub = 0; ub < 2; ub++) {
				for (int m = 0; m < 3; m++) {
					SbSpec w = { 0, ub, n, m, 0, 0 };
					check_setbuf(w, g_data, "hand");
					SbSpec w2 = { 0, ub, n, m, 0, 1 };
					check_setbuf(w2, g_data, "hand");
					for (size_t ri = 0; ri < NR; ri++) {
						SbSpec rd = { 1, ub, n, m,
							reads[ri], 0 };
						check_setbuf(rd, g_data,
						    "hand");
					}
				}
			}
		}
	}
}

static void sweep_setbuf(long iters)
{
	Rng r(0x0098FEEDFACE5678ULL);
	const size_t bs = (size_t)BUFSIZ;
	for (long it = 0; it < iters; it++) {
		SbSpec sp;
		sp.mode = (int)r.below(2);
		sp.use_buf = (int)r.below(2);
		sp.method = (int)r.below(3);
		sp.flush_mid = (int)r.below(2);
		sp.nread = 1 + r.below(72);
		unsigned shape = r.below(1000);
		if (shape < 12)
			sp.n = bs - 1;
		else if (shape < 24)
			sp.n = bs;
		else if (shape < 36)
			sp.n = bs + 1;
		else if (shape < 44)
			sp.n = bs + 2;
		else if (shape < 50)
			sp.n = 2 * bs;
		else if (shape < 56)
			sp.n = 2 * bs + 7;
		else if (shape < 62)
			sp.n = bs - 2;
		else if (shape < 120)
			sp.n = 0;
		else if (shape < 180)
			sp.n = 1;
		else
			sp.n = r.below(97);
		size_t fill = sp.n < 96 ? 96 : sp.n;
		if (fill > SB_DATAMAX)
			fill = SB_DATAMAX;
		fill_pattern(g_data, fill, (int)r.below(4));
		if (r.chance(30))
			for (size_t i = 0; i < fill; i++)
				g_data[i] = pick_byte(r);
		check_setbuf(sp, g_data, "sweep");
	}
}

/* ----------------------------------------------------------------- main -- */

static void warmup()
{
	/*
	 * Touch every code path glibc lazily initialises (wide stream
	 * orientation, mbstate conversion tables, float formatting) on both
	 * sides before any comparison, so first-call effects cannot show up as
	 * a spurious divergence.
	 */
	static SCap sc;
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
	const unsigned char in[] = "12 34 ab 5.5 x";
	for (int pass = 0; pass < 2; pass++) {
		bool port = pass != 0;
		run_scanf(port, L"%d %d %31ls %lf", S_NONE, in, sizeof in - 1,
		    true, sc);
		run_scanf(port, L"%d", S_INT, in, sizeof in - 1, true, sc);
		run_scanf(port, L"%31ls", S_WS, in, sizeof in - 1, true, sc);
		run_scanf(port, L"%lf", S_DBL, in, sizeof in - 1, true, sc);
		run_scanf(port, L"%lc", S_LC, in, sizeof in - 1, true, sc);
		run_scanf(port, L"%31l[0-9]", S_WS, in, sizeof in - 1, true, sc);
		run_printf(port, L"%d", P_INT, a, true, pa);
		run_printf(port, L"%ls", P_WSTR, a, true, pa);
		run_printf(port, L"%s", P_STR, a, true, pa);
		run_printf(port, L"%f %e %g", P_DBL3, a, true, pa);
		run_printf(port, L"%a", P_DBL, a, true, pa);
		run_printf(port, L"%lc", P_WC, a, true, pa);
		run_printf(port, L"%p", P_PTR, a, true, pa);
		run_printf(port, L"%zu", P_SIZE, a, true, pa);
	}
	/* Reset scanf's warmup slab expectations: nothing to do, caps unused. */
	(void)sc;
}

int main()
{
	init_streams();

	g_data = (unsigned char *)malloc(SB_DATAMAX);
	if (!g_data)
		die("malloc");

	warmup();

	const long SWEEP = 200000;

	hand_scanf();
	sweep_scanf(SWEEP);

	hand_printf();
	sweep_printf(SWEEP);

	hand_setbuf();
	sweep_setbuf(SWEEP);

	const Tally *all[] = { &t_scanf, &t_printf, &t_setbuf };
	long total_fail = 0;
	fprintf(g_rep, "\n%-12s %12s %12s  %s\n", "function", "cases",
	    "failures", "result");
	fprintf(g_rep, "------------------------------------------------------\n");
	for (int i = 0; i < 3; i++) {
		fprintf(g_rep, "%-12s %12ld %12ld  %s\n", all[i]->name,
		    all[i]->cases, all[i]->fails,
		    all[i]->fails ? "FAIL" : "ok");
		total_fail += all[i]->fails;
	}
	fprintf(g_rep, "------------------------------------------------------\n");
	fprintf(g_rep, "%-12s %12ld %12ld  %s\n", "TOTAL",
	    t_scanf.cases + t_printf.cases + t_setbuf.cases, total_fail,
	    total_fail ? "FAIL" : "ok");

	free(g_data);
	cleanup_streams();
	return total_fail ? 1 : 0;
}
