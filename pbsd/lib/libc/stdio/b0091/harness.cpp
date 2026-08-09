// b0091 differential test: port.cppm vs the ref_ oracle in oracle.c.
//
// Every function in the batch touches a FILE stream (fsetpos seeks one,
// getwchar reads stdin, putwchar writes stdout), so "compare the buffer" here
// means: run each side against its own private temp file, then compare the
// whole file contents through guard-filled buffers, plus the per-call return
// value, errno, stream position and the eof/error indicators.
//
// Reporting goes to g_out, a dup() of the original stdout taken before stdout
// is ever freopen()ed at a temp file.

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wchar.h>

#include <vector>

import pbsd.lib.libc.stdio.b0091;

namespace port = pbsd::lib_libc_stdio::b0091;

extern "C" {
int ref_fsetpos(FILE *iop, const off_t *pos);
wint_t ref_getwchar(void);
wint_t ref_getwchar_l(locale_t locale);
wint_t ref_putwchar_l(wchar_t wc, locale_t locale);
wint_t ref_putwchar(wchar_t wc);
}

/* ---------------------------------------------------------------- */
/* plumbing								*/
/* ---------------------------------------------------------------- */

static FILE *g_out;

struct Stat {
	const char *name;
	long long cases;
	long long fails;
	int shown;
};

static Stat st_fsetpos = { "fsetpos", 0, 0, 0 };
static Stat st_getwchar = { "getwchar", 0, 0, 0 };
static Stat st_getwchar_l = { "getwchar_l", 0, 0, 0 };
static Stat st_putwchar = { "putwchar", 0, 0, 0 };
static Stat st_putwchar_l = { "putwchar_l", 0, 0, 0 };

static const int MAXSHOW = 8;

static void
note_fail(Stat &s, const char *fmt, ...)
{
	s.fails++;
	if (s.shown < MAXSHOW) {
		s.shown++;
		va_list ap;
		fprintf(g_out, "  FAIL %s: ", s.name);
		va_start(ap, fmt);
		vfprintf(g_out, fmt, ap);
		va_end(ap);
		fputc('\n', g_out);
		fflush(g_out);
	}
}

static void
die(const char *what)
{
	fprintf(g_out, "harness setup failure: %s: %s\n", what,
	    strerror(errno));
	fflush(g_out);
	exit(2);
}

static uint64_t g_rs;

static void
seed(uint64_t s)
{
	g_rs = s;
}

static uint64_t
rnd(void)
{
	g_rs ^= g_rs << 13;
	g_rs ^= g_rs >> 7;
	g_rs ^= g_rs << 17;
	return g_rs;
}

static char pathA[] = "/tmp/pbsd_b0091_a_XXXXXX";
static char pathB[] = "/tmp/pbsd_b0091_b_XXXXXX";
static char pathC[] = "/tmp/pbsd_b0091_c_XXXXXX";
static char pathD[] = "/tmp/pbsd_b0091_d_XXXXXX";

static void
mktmp(char *tmpl)
{
	int fd = mkstemp(tmpl);
	if (fd < 0)
		die("mkstemp");
	close(fd);
}

static void
write_file(const char *path, const void *buf, size_t n)
{
	FILE *f = fopen(path, "wb");
	if (f == NULL)
		die("fopen for write");
	if (n != 0 && fwrite(buf, 1, n, f) != n)
		die("fwrite");
	if (fclose(f) != 0)
		die("fclose");
}

/*
 * Fill buf (cap bytes) with the guard byte 0x7f, then read the file into the
 * front of it.  Bytes the file does not cover keep the guard, so comparing the
 * full cap bytes of two such buffers also compares everything past the window
 * either side was supposed to write.
 */
static size_t
read_guarded(const char *path, unsigned char *buf, size_t cap)
{
	memset(buf, 0x7f, cap);
	FILE *f = fopen(path, "rb");
	if (f == NULL)
		die("fopen for read");
	size_t n = 0, r;
	while (n < cap && (r = fread(buf + n, 1, cap - n, f)) > 0)
		n += r;
	if (ferror(f))
		die("fread");
	fclose(f);
	return n;
}

/*
 * Compare the two read-back buffers in full, including the guard region.
 * `nominal' is the largest byte count the calls under test could legitimately
 * have produced; anything beyond it must still be guard bytes on both sides.
 */
static void
cmp_buffers(Stat &s, const char *tag, const unsigned char *a, size_t na,
    const unsigned char *b, size_t nb, size_t cap, size_t nominal)
{
	s.cases++;
	if (na != nb) {
		note_fail(s, "%s: file length port=%zu ref=%zu", tag, na, nb);
		return;
	}
	for (size_t i = 0; i < cap; i++) {
		if (a[i] != b[i]) {
			note_fail(s, "%s: byte %zu port=%02x ref=%02x "
			    "(len %zu)", tag, i, (unsigned)a[i],
			    (unsigned)b[i], na);
			return;
		}
	}
	for (size_t i = nominal; i < cap; i++) {
		if (a[i] != 0x7f) {
			note_fail(s, "%s: guard clobbered at %zu (%02x) past "
			    "window %zu", tag, i, (unsigned)a[i], nominal);
			return;
		}
	}
}

static locale_t g_u8 = (locale_t)0;
static locale_t g_c = (locale_t)0;
static const char *g_u8name = "C.UTF-8";

static void
set_global(const char *name)
{
	if (setlocale(LC_ALL, name) == NULL)
		die("setlocale");
}

/* ---------------------------------------------------------------- */
/* fsetpos								*/
/* ---------------------------------------------------------------- */

#define FS_SIZE 64
static unsigned char fs_content[FS_SIZE];

struct FRes {
	int ret;
	int err;
	long long tell;
	int c;
	int eofb;
	int errb;
	long long tell2;
	long long pos_after;
};

static void
fs_run(int use_ref, FILE *fp, off_t start, off_t p, FRes &o)
{
	off_t pos = p;

	if (fseeko(fp, start, SEEK_SET) != 0)
		die("fseeko to start position");
	clearerr(fp);
	errno = 0;
	int r = use_ref ? ref_fsetpos(fp, &pos) : port::fsetpos(fp, &pos);
	o.ret = r;
	o.err = errno;
	o.tell = (long long)ftello(fp);
	o.c = fgetc(fp);
	o.eofb = feof(fp) ? 1 : 0;
	o.errb = ferror(fp) ? 1 : 0;
	o.tell2 = (long long)ftello(fp);
	o.pos_after = (long long)pos;
}

static void
fs_case(FILE *fa, FILE *fb, off_t start, off_t p)
{
	Stat &s = st_fsetpos;
	FRes a, b;

	s.cases++;
	fs_run(0, fa, start, p, a);
	fs_run(1, fb, start, p, b);

	if (a.ret != b.ret || a.err != b.err || a.tell != b.tell ||
	    a.c != b.c || a.eofb != b.eofb || a.errb != b.errb ||
	    a.tell2 != b.tell2 || a.pos_after != b.pos_after) {
		note_fail(s, "start=%lld pos=%lld  port{ret=%d errno=%d "
		    "tell=%lld c=%d eof=%d err=%d tell2=%lld *pos=%lld} "
		    "ref{ret=%d errno=%d tell=%lld c=%d eof=%d err=%d "
		    "tell2=%lld *pos=%lld}", (long long)start, (long long)p,
		    a.ret, a.err, a.tell, a.c, a.eofb, a.errb, a.tell2,
		    a.pos_after, b.ret, b.err, b.tell, b.c, b.eofb, b.errb,
		    b.tell2, b.pos_after);
	}
}

static void
test_fsetpos(void)
{
	for (size_t i = 0; i < FS_SIZE; i++)
		fs_content[i] = (unsigned char)((i * 37u + (i % 5u) * 91u) &
		    0xff);
	/* make sure 0x00, 0x7f, 0x80 and 0xff are all in there */
	fs_content[0] = 0x00;
	fs_content[1] = 0xff;
	fs_content[2] = 0x80;
	fs_content[3] = 0x7f;
	fs_content[FS_SIZE - 2] = 0x00;
	fs_content[FS_SIZE - 1] = 0x80;
	write_file(pathA, fs_content, FS_SIZE);
	write_file(pathB, fs_content, FS_SIZE);

	FILE *fa = fopen(pathA, "rb");
	FILE *fb = fopen(pathB, "rb");
	if (fa == NULL || fb == NULL)
		die("fopen fsetpos inputs");

	/*
	 * Hand-written grid.  The starting position is non-zero for most rows
	 * and the file is non-empty, so SEEK_SET, SEEK_CUR and SEEK_END all
	 * land somewhere different: a whence mutation shows up as a different
	 * ftello() and a different byte read back.
	 */
	static const off_t starts[] = { 0, 1, 2, 7, 31, 32, 33, 63, 64, 65,
		100 };
	static const off_t poss[] = {
		INT64_MIN, INT64_MIN + 1, -4097, -4096, -65, -64, -63, -33,
		-32, -31, -2, -1, 0, 1, 2, 3, 31, 32, 33, 62, 63, 64, 65, 66,
		127, 128, 4095, 4096, 65535, 65536, 2147483647LL,
		2147483648LL, INT64_MAX - 1, INT64_MAX
	};
	for (size_t i = 0; i < sizeof starts / sizeof starts[0]; i++)
		for (size_t j = 0; j < sizeof poss / sizeof poss[0]; j++)
			fs_case(fa, fb, starts[i], poss[j]);

	/* fixed-seed randomised sweep */
	seed(0x5eed0091f5e7f0a5ULL);
	for (long long it = 0; it < 200000; it++) {
		off_t start = (off_t)(rnd() % 81);
		off_t p;
		switch (rnd() % 5) {
		case 0:
			/* dense band straddling 0 and the file size */
			p = (off_t)(rnd() % 200) - 70;
			break;
		case 1:
			/* right on the size boundary */
			p = (off_t)FS_SIZE + (off_t)(rnd() % 5) - 2;
			break;
		case 2:
			/* powers of two, both signs */
			p = (off_t)1 << (rnd() % 63);
			if (rnd() & 1)
				p = -p;
			break;
		case 3:
			p = (off_t)rnd();
			break;
		default:
			p = -(off_t)(rnd() % 4096) - 1;
			break;
		}
		fs_case(fa, fb, start, p);
	}

	fclose(fa);
	fclose(fb);

	/* neither side may have written to its input file */
	unsigned char ba[FS_SIZE + 16], bb[FS_SIZE + 16];
	size_t na = read_guarded(pathA, ba, sizeof ba);
	size_t nb = read_guarded(pathB, bb, sizeof bb);
	cmp_buffers(st_fsetpos, "input file untouched", ba, na, bb, nb,
	    sizeof ba, FS_SIZE);
	st_fsetpos.cases++;
	if (na != FS_SIZE || memcmp(ba, fs_content, FS_SIZE) != 0)
		note_fail(st_fsetpos, "input file mutated (len %zu)", na);
}

/* ---------------------------------------------------------------- */
/* getwchar / getwchar_l						*/
/* ---------------------------------------------------------------- */

struct GRes {
	unsigned r;
	int err;
	long long tell;
	int eofb;
	int errb;
};

/* kind: 0 port getwchar, 1 ref getwchar, 2 port getwchar_l, 3 ref */
static void
g_seq(int kind, locale_t loc, const char *path, std::vector<GRes> &v,
    size_t maxcalls)
{
	if (freopen(path, "rb", stdin) == NULL)
		die("freopen stdin");
	v.clear();
	for (size_t i = 0; i < maxcalls; i++) {
		errno = 0;
		wint_t r;
		switch (kind) {
		case 0:
			r = port::getwchar();
			break;
		case 1:
			r = ref_getwchar();
			break;
		case 2:
			r = port::getwchar_l(loc);
			break;
		default:
			r = ref_getwchar_l(loc);
			break;
		}
		GRes g;
		g.r = (unsigned)r;
		g.err = errno;
		g.tell = (long long)ftello(stdin);
		g.eofb = feof(stdin) ? 1 : 0;
		g.errb = ferror(stdin) ? 1 : 0;
		v.push_back(g);
		/*
		 * A wide read that fails leaves the position untouched, so
		 * calling again returns the same failure forever; stop at the
		 * first WEOF, having recorded it.
		 */
		if (r == WEOF)
			break;
	}
}

static void
g_round(int use_l, locale_t loc, const unsigned char *data, size_t n,
    const char *tag)
{
	Stat &s = use_l ? st_getwchar_l : st_getwchar;
	std::vector<GRes> va, vb;

	write_file(pathA, data, n);
	write_file(pathB, data, n);

	g_seq(use_l ? 2 : 0, loc, pathA, va, n + 2);
	g_seq(use_l ? 3 : 1, loc, pathB, vb, n + 2);

	size_t m = va.size() < vb.size() ? va.size() : vb.size();
	for (size_t i = 0; i < m; i++) {
		s.cases++;
		if (va[i].r != vb[i].r || va[i].err != vb[i].err ||
		    va[i].tell != vb[i].tell || va[i].eofb != vb[i].eofb ||
		    va[i].errb != vb[i].errb) {
			note_fail(s, "%s: call %zu port{r=%08x errno=%d "
			    "tell=%lld eof=%d err=%d} ref{r=%08x errno=%d "
			    "tell=%lld eof=%d err=%d}", tag, i, va[i].r,
			    va[i].err, va[i].tell, va[i].eofb, va[i].errb,
			    vb[i].r, vb[i].err, vb[i].tell, vb[i].eofb,
			    vb[i].errb);
		}
	}
	s.cases++;
	if (va.size() != vb.size())
		note_fail(s, "%s: call count port=%zu ref=%zu", tag, va.size(),
		    vb.size());

	std::vector<unsigned char> ba(n + 16), bb(n + 16);
	size_t na = read_guarded(pathA, ba.data(), ba.size());
	size_t nb = read_guarded(pathB, bb.data(), bb.size());
	cmp_buffers(s, tag, ba.data(), na, bb.data(), nb, ba.size(), n);
	s.cases++;
	if (na != n || (n != 0 && memcmp(ba.data(), data, n) != 0))
		note_fail(s, "%s: input file mutated (len %zu, want %zu)", tag,
		    na, n);
}

static size_t
utf8_encode(unsigned cp, unsigned char *o)
{
	if (cp < 0x80) {
		o[0] = (unsigned char)cp;
		return 1;
	}
	if (cp < 0x800) {
		o[0] = (unsigned char)(0xc0 | (cp >> 6));
		o[1] = (unsigned char)(0x80 | (cp & 0x3f));
		return 2;
	}
	if (cp < 0x10000) {
		o[0] = (unsigned char)(0xe0 | (cp >> 12));
		o[1] = (unsigned char)(0x80 | ((cp >> 6) & 0x3f));
		o[2] = (unsigned char)(0x80 | (cp & 0x3f));
		return 3;
	}
	o[0] = (unsigned char)(0xf0 | (cp >> 18));
	o[1] = (unsigned char)(0x80 | ((cp >> 12) & 0x3f));
	o[2] = (unsigned char)(0x80 | ((cp >> 6) & 0x3f));
	o[3] = (unsigned char)(0x80 | (cp & 0x3f));
	return 4;
}

static void
edge_byte_strings(std::vector<std::vector<unsigned char> > &v)
{
	static const char *lits[] = {
		"",
		"A",
		"\x00",
		"\x00\x00\x00",
		"\x01",
		"\x7f",
		"\x80",
		"\x81",
		"\xbf",
		"\xc0\x80",
		"\xc1\xbf",
		"\xc2\x80",
		"\xc3\xa9",
		"\xc3",
		"\xdf\xbf",
		"\xe0\x80\x80",
		"\xe0\xa0\x80",
		"\xe0\xa0",
		"\xed\x9f\xbf",
		"\xed\xa0\x80",
		"\xed\xbf\xbf",
		"\xee\x80\x80",
		"\xef\xbf\xbd",
		"\xef\xbf\xbf",
		"\xf0\x8f\xbf\xbf",
		"\xf0\x90\x80\x80",
		"\xf4\x8f\xbf\xbf",
		"\xf4\x90\x80\x80",
		"\xf5\x80\x80\x80",
		"\xf8\x88\x80\x80\x80",
		"\xfe",
		"\xff",
		"\xff\xfe",
		"A\x80" "B",
		"A\xc3\xa9" "B",
		"\x7f\x80\x81\xfe\xff",
		"abc",
	};
	static const size_t lens[] = { 0, 1, 1, 3, 1, 1, 1, 1, 1, 2, 2, 2, 2,
		1, 2, 3, 3, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 1, 1, 2, 3,
		4, 5, 3 };

	for (size_t i = 0; i < sizeof lits / sizeof lits[0]; i++) {
		const unsigned char *p = (const unsigned char *)lits[i];
		v.push_back(std::vector<unsigned char>(p, p + lens[i]));
	}

	/* every single byte on its own */
	for (unsigned b = 0; b < 256; b++) {
		std::vector<unsigned char> one;
		one.push_back((unsigned char)b);
		v.push_back(one);
	}
	/* every trail byte behind a two-byte lead */
	for (unsigned b = 0; b < 256; b++) {
		std::vector<unsigned char> two;
		two.push_back(0xc3);
		two.push_back((unsigned char)b);
		v.push_back(two);
	}
	/* boundary lengths, each run ending in a high-bit byte */
	for (size_t len = 0; len <= 8; len++) {
		std::vector<unsigned char> run;
		for (size_t i = 0; i < len; i++)
			run.push_back((unsigned char)(i + 1 == len ? 0xff :
			    0x41 + i));
		v.push_back(run);
	}
}

static void
test_getwchar(void)
{
	std::vector<std::vector<unsigned char> > edges;
	edge_byte_strings(edges);

	/* --- hand-written edge cases, in both encodings and both APIs --- */
	set_global("C");
	for (size_t i = 0; i < edges.size(); i++) {
		const unsigned char *d = edges[i].empty() ?
		    (const unsigned char *)"" : &edges[i][0];
		size_t n = edges[i].size();
		g_round(0, (locale_t)0, d, n, "edge/global=C");
		g_round(1, g_u8, d, n, "edge/l=U8,global=C");
		g_round(1, g_c, d, n, "edge/l=C,global=C");
		g_round(1, (locale_t)0, d, n, "edge/l=NULL,global=C");
	}
	set_global(g_u8name);
	for (size_t i = 0; i < edges.size(); i++) {
		const unsigned char *d = edges[i].empty() ?
		    (const unsigned char *)"" : &edges[i][0];
		size_t n = edges[i].size();
		g_round(0, (locale_t)0, d, n, "edge/global=U8");
		g_round(1, g_c, d, n, "edge/l=C,global=U8");
		g_round(1, g_u8, d, n, "edge/l=U8,global=U8");
		g_round(1, (locale_t)0, d, n, "edge/l=NULL,global=U8");
	}
	set_global("C");

	/* --- fixed-seed randomised sweep --- */
	seed(0xc0ffee0091ULL);
	const size_t CHUNK = 8000;
	std::vector<unsigned char> buf;

	/* plain getwchar: ASCII under the C locale reads clean to EOF */
	for (int round = 0; round < 14; round++) {
		buf.clear();
		for (size_t i = 0; i < CHUNK; i++)
			buf.push_back((unsigned char)(rnd() % 0x80));
		g_round(0, (locale_t)0, &buf[0], buf.size(), "rand/ascii,C");
	}
	/* plain getwchar under UTF-8: multibyte, with junk near the tail */
	set_global(g_u8name);
	for (int round = 0; round < 14; round++) {
		buf.clear();
		size_t chars = 0;
		while (chars < CHUNK) {
			unsigned cp;
			switch (rnd() % 20) {
			case 0:
			case 1:
			case 2:
				cp = 0x80 + (unsigned)(rnd() % 0x780);
				break;
			case 3:
			case 4:
				cp = 0x800 + (unsigned)(rnd() % 0xd000);
				break;
			case 5:
				cp = 0x10000 + (unsigned)(rnd() % 0x100000);
				break;
			default:
				cp = (unsigned)(rnd() % 0x80);
				break;
			}
			unsigned char tmp[4];
			size_t k = utf8_encode(cp, tmp);
			for (size_t j = 0; j < k; j++)
				buf.push_back(tmp[j]);
			chars++;
			if (chars > CHUNK - CHUNK / 10 && (rnd() % 997) == 0)
				buf.push_back((unsigned char)(0x80 +
				    rnd() % 0x80));
		}
		g_round(0, (locale_t)0, &buf[0], buf.size(), "rand/utf8,U8");
	}
	set_global("C");

	/* getwchar_l: explicit locale, global locale deliberately the other */
	for (int round = 0; round < 14; round++) {
		buf.clear();
		size_t chars = 0;
		while (chars < CHUNK) {
			unsigned cp = (rnd() % 4) == 0 ?
			    0x80 + (unsigned)(rnd() % 0x1000) :
			    (unsigned)(rnd() % 0x80);
			unsigned char tmp[4];
			size_t k = utf8_encode(cp, tmp);
			for (size_t j = 0; j < k; j++)
				buf.push_back(tmp[j]);
			chars++;
		}
		g_round(1, g_u8, &buf[0], buf.size(), "rand/l=U8,global=C");
	}
	for (int round = 0; round < 5; round++) {
		buf.clear();
		for (size_t i = 0; i < CHUNK; i++)
			buf.push_back((unsigned char)(rnd() % 0x80));
		g_round(1, g_c, &buf[0], buf.size(), "rand/l=C,global=C");
		g_round(1, (locale_t)0, &buf[0], buf.size(),
		    "rand/l=NULL,global=C");
	}
	set_global(g_u8name);
	for (int round = 0; round < 3; round++) {
		buf.clear();
		size_t chars = 0;
		while (chars < CHUNK) {
			unsigned cp = (rnd() % 3) == 0 ?
			    0x80 + (unsigned)(rnd() % 0x1000) :
			    (unsigned)(rnd() % 0x80);
			unsigned char tmp[4];
			size_t k = utf8_encode(cp, tmp);
			for (size_t j = 0; j < k; j++)
				buf.push_back(tmp[j]);
			chars++;
		}
		g_round(1, (locale_t)0, &buf[0], buf.size(),
		    "rand/l=NULL,global=U8");
		g_round(1, g_u8, &buf[0], buf.size(), "rand/l=U8,global=U8");
	}
	set_global("C");
}

/* ---------------------------------------------------------------- */
/* putwchar / putwchar_l						*/
/* ---------------------------------------------------------------- */

struct PRes {
	unsigned r;
	int err;
	int errb;
	int eofb;
	long long tell;
};

/* mode: 0 putwchar(wc), 1 putwchar_l(wc, loc), 2 putwchar_l(wc, NULL) */
static void
p_seq(int use_ref, int mode, locale_t loc, const char *path, const wchar_t *w,
    size_t n, std::vector<PRes> &v)
{
	if (freopen(path, "wb", stdout) == NULL)
		die("freopen stdout");
	v.clear();
	for (size_t i = 0; i < n; i++) {
		errno = 0;
		wint_t r;
		switch (mode) {
		case 0:
			r = use_ref ? ref_putwchar(w[i]) :
			    port::putwchar(w[i]);
			break;
		case 1:
			r = use_ref ? ref_putwchar_l(w[i], loc) :
			    port::putwchar_l(w[i], loc);
			break;
		default:
			r = use_ref ? ref_putwchar_l(w[i], (locale_t)0) :
			    port::putwchar_l(w[i], (locale_t)0);
			break;
		}
		PRes p;
		p.r = (unsigned)r;
		p.err = errno;
		p.errb = ferror(stdout) ? 1 : 0;
		p.eofb = feof(stdout) ? 1 : 0;
		p.tell = (long long)ftello(stdout);
		v.push_back(p);
		/*
		 * Both sides are cleared identically once the indicators have
		 * been recorded, so one bad conversion cannot hide every later
		 * call behind a sticky error flag.
		 */
		clearerr(stdout);
	}
	fflush(stdout);
}

static void
p_round(int mode, locale_t loc, const wchar_t *w, size_t n, const char *tag)
{
	Stat &s = mode == 0 ? st_putwchar : st_putwchar_l;
	std::vector<PRes> va, vb;

	p_seq(0, mode, loc, pathC, w, n, va);
	p_seq(1, mode, loc, pathD, w, n, vb);

	for (size_t i = 0; i < n; i++) {
		s.cases++;
		if (va[i].r != vb[i].r || va[i].err != vb[i].err ||
		    va[i].errb != vb[i].errb || va[i].eofb != vb[i].eofb ||
		    va[i].tell != vb[i].tell) {
			note_fail(s, "%s: call %zu wc=%08x port{r=%08x "
			    "errno=%d err=%d eof=%d tell=%lld} ref{r=%08x "
			    "errno=%d err=%d eof=%d tell=%lld}", tag, i,
			    (unsigned)w[i], va[i].r, va[i].err, va[i].errb,
			    va[i].eofb, va[i].tell, vb[i].r, vb[i].err,
			    vb[i].errb, vb[i].eofb, vb[i].tell);
		}
	}

	size_t cap = n * 8 + 64;
	std::vector<unsigned char> ba(cap), bb(cap);
	size_t na = read_guarded(pathC, ba.data(), cap);
	size_t nb = read_guarded(pathD, bb.data(), cap);
	cmp_buffers(s, tag, ba.data(), na, bb.data(), nb, cap, n * 6 + 8);
}

static void
test_putwchar(void)
{
	static const long edgev[] = {
		0, 1, 2, 0x0a, 0x1f, 0x20, 0x40, 0x41, 0x7e, 0x7f, 0x80, 0x81,
		0xa0, 0xfe, 0xff, 0x100, 0x7fe, 0x7ff, 0x800, 0x801, 0xd7ff,
		0xd800, 0xd801, 0xdbff, 0xdc00, 0xdfff, 0xe000, 0xfffd,
		0xfffe, 0xffff, 0x10000, 0x10001, 0x3fffe, 0xfffff, 0x100000,
		0x10fffe, 0x10ffff, 0x110000, 0x110001, 0x1fffff, 0x200000,
		0x7ffffffe, 0x7fffffff, -1, -2, -0x80, -0x100, -0x7fffffff,
		-0x80000000L
	};
	const size_t ne = sizeof edgev / sizeof edgev[0];
	std::vector<wchar_t> edge(ne);
	for (size_t i = 0; i < ne; i++)
		edge[i] = (wchar_t)edgev[i];

	const char *globals[2];
	globals[0] = "C";
	globals[1] = g_u8name;

	/*
	 * Each wc value alone (a one-call stream, so the conversion state is
	 * always fresh) and then all of them in a single stream.
	 */
	for (int gi = 0; gi < 2; gi++) {
		set_global(globals[gi]);
		for (size_t i = 0; i < ne; i++) {
			wchar_t one = edge[i];
			p_round(0, (locale_t)0, &one, 1, "edge1/plain");
			p_round(1, g_u8, &one, 1, "edge1/l=U8");
			p_round(1, g_c, &one, 1, "edge1/l=C");
			p_round(2, (locale_t)0, &one, 1, "edge1/l=NULL");
		}
		p_round(0, (locale_t)0, &edge[0], ne, "edgeall/plain");
		p_round(1, g_u8, &edge[0], ne, "edgeall/l=U8");
		p_round(1, g_c, &edge[0], ne, "edgeall/l=C");
		p_round(2, (locale_t)0, &edge[0], ne, "edgeall/l=NULL");
	}

	/* zero-length stream: nothing written, both files must stay empty */
	set_global("C");
	p_round(0, (locale_t)0, &edge[0], 0, "empty/plain");
	p_round(1, g_u8, &edge[0], 0, "empty/l=U8");

	/* --- fixed-seed randomised sweep --- */
	seed(0x9e3779b97f4a7c15ULL);
	const size_t CHUNK = 8000;
	std::vector<wchar_t> w(CHUNK);

	for (int round = 0; round < 25; round++) {
		for (size_t i = 0; i < CHUNK; i++) {
			unsigned long v;
			switch (rnd() % 8) {
			case 0:
				v = rnd() % 0x80;
				break;
			case 1:
				v = 0x80 + rnd() % 0x780;
				break;
			case 2:
				v = 0x800 + rnd() % 0xf800;
				break;
			case 3:
				v = 0x10000 + rnd() % 0x100000;
				break;
			case 4:
				v = 0xd800 + rnd() % 0x800;
				break;
			case 5:
				v = 0x110000 + rnd() % 0x100000;
				break;
			case 6:
				v = (unsigned long)(uint32_t)rnd();
				break;
			default:
				v = (unsigned long)(uint32_t)(-(int32_t)(rnd() %
				    0x10000) - 1);
				break;
			}
			w[i] = (wchar_t)(uint32_t)v;
		}
		set_global(round % 2 == 0 ? "C" : g_u8name);
		p_round(0, (locale_t)0, &w[0], CHUNK, "rand/plain");
	}

	for (int round = 0; round < 7; round++) {
		for (size_t i = 0; i < CHUNK; i++) {
			unsigned long v;
			switch (rnd() % 6) {
			case 0:
				v = rnd() % 0x80;
				break;
			case 1:
				v = 0x80 + rnd() % 0x780;
				break;
			case 2:
				v = 0x800 + rnd() % 0xf800;
				break;
			case 3:
				v = 0x10000 + rnd() % 0x100000;
				break;
			case 4:
				v = (unsigned long)(uint32_t)rnd();
				break;
			default:
				v = 0xd800 + rnd() % 0x800;
				break;
			}
			w[i] = (wchar_t)(uint32_t)v;
		}
		/*
		 * The global locale and the explicit locale are always
		 * opposites here: if FIX_LOCALE stopped honouring the caller's
		 * locale the bytes in the file would change.
		 */
		set_global("C");
		p_round(1, g_u8, &w[0], CHUNK, "rand/l=U8,global=C");
		p_round(2, (locale_t)0, &w[0], CHUNK, "rand/l=NULL,global=C");
		set_global(g_u8name);
		p_round(1, g_c, &w[0], CHUNK, "rand/l=C,global=U8");
		p_round(2, (locale_t)0, &w[0], CHUNK, "rand/l=NULL,global=U8");
	}
	set_global("C");
}

/*
 * Belt and braces: prove the byte-level comparison in p_round can actually see
 * the difference between the C locale and the UTF-8 locale, i.e. that a port
 * which ignored its locale argument would be caught rather than silently
 * agreeing with the oracle.
 */
static int
check_discrimination(void)
{
	wchar_t wc = (wchar_t)0x00e9;
	std::vector<PRes> v;
	unsigned char b1[64], b2[64];

	set_global("C");
	p_seq(0, 1, g_u8, pathC, &wc, 1, v);
	size_t n1 = read_guarded(pathC, b1, sizeof b1);
	p_seq(0, 1, g_c, pathC, &wc, 1, v);
	size_t n2 = read_guarded(pathC, b2, sizeof b2);
	set_global("C");

	if (n1 == n2 && memcmp(b1, b2, sizeof b1) == 0) {
		fprintf(g_out, "WARNING: the C and UTF-8 locales produce "
		    "identical output here, so locale sensitivity is not "
		    "being tested\n");
		return 0;
	}
	return 1;
}

/* ---------------------------------------------------------------- */

int
main(void)
{
	int fd = dup(1);
	if (fd < 0)
		return 2;
	g_out = fdopen(fd, "w");
	if (g_out == NULL)
		return 2;

	mktmp(pathA);
	mktmp(pathB);
	mktmp(pathC);
	mktmp(pathD);

	g_c = newlocale(LC_ALL_MASK, "C", (locale_t)0);
	g_u8 = newlocale(LC_ALL_MASK, "C.UTF-8", (locale_t)0);
	if (g_u8 == (locale_t)0) {
		g_u8 = newlocale(LC_ALL_MASK, "en_US.UTF-8", (locale_t)0);
		g_u8name = "en_US.UTF-8";
	}
	if (g_c == (locale_t)0)
		die("newlocale C");
	if (g_u8 == (locale_t)0) {
		fprintf(g_out, "WARNING: no UTF-8 locale available; running "
		    "single-byte only\n");
		g_u8 = g_c;
		g_u8name = "C";
	}
	set_global("C");

	int discriminates = check_discrimination();

	test_fsetpos();
	test_getwchar();
	test_putwchar();

	/* stop writing into a temp file before the process exits */
	if (freopen("/dev/null", "wb", stdout) == NULL)
		die("restore stdout");
	if (freopen("/dev/null", "rb", stdin) == NULL)
		die("restore stdin");
	unlink(pathA);
	unlink(pathB);
	unlink(pathC);
	unlink(pathD);

	Stat *all[] = { &st_fsetpos, &st_getwchar, &st_getwchar_l,
		&st_putwchar, &st_putwchar_l };
	long long tc = 0, tf = 0;

	fprintf(g_out, "\nb0091  port vs oracle  (utf8 locale: %s, "
	    "locale-sensitive: %s)\n", g_u8name, discriminates ? "yes" : "NO");
	fprintf(g_out, "%-14s %12s %10s  %s\n", "function", "cases",
	    "failures", "result");
	fprintf(g_out, "%-14s %12s %10s  %s\n", "--------------",
	    "------------", "----------", "------");
	for (size_t i = 0; i < sizeof all / sizeof all[0]; i++) {
		fprintf(g_out, "%-14s %12lld %10lld  %s\n", all[i]->name,
		    all[i]->cases, all[i]->fails,
		    all[i]->fails == 0 ? "ok" : "FAILED");
		tc += all[i]->cases;
		tf += all[i]->fails;
	}
	fprintf(g_out, "%-14s %12lld %10lld  %s\n", "TOTAL", tc, tf,
	    tf == 0 ? "ok" : "FAILED");
	fflush(g_out);

	return tf == 0 ? 0 : 1;
}
