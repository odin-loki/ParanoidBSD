/*
 * harness.cpp -- differential test for PBSD batch b0193s2 (bin/ed/io.c).
 *
 * Every ported function is driven with hand-written edge cases and with a
 * fixed-seed randomised sweep of 200000 iterations, and the C++ port is
 * compared against the untouched C oracle on:
 *   - return values;
 *   - the ENTIRE contents of every buffer the function can write into
 *     (guard-filled with 0x7f beforehand, compared past the nominal write
 *     window);
 *   - pointer results, as offsets from the owning buffer base;
 *   - every global the functions touch (isbinary, newline_added, lineno,
 *     ibufp, ibufsz, errmsg, mutex);
 *   - the exact bytes written to stdout and stderr.
 * Stateful readers are driven to exhaustion with a comparison after every
 * single iteration.
 */

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

import pbsd.bin.ed.b0193s2;

namespace P = pbsd::bin_ed::b0193s2;

extern "C" {
int    ref_get_stream_line(FILE *fp);
int    ref_put_stream_line(FILE *fp, const char *s, int len);
char  *ref_get_extended_line(int *sizep, int nonl);
int    ref_get_tty_line(void);
int    ref_put_tty_line(const char *s, int l, long n, int gflag);
char **ref_sbuf_addr(void);
int   *ref_sbufsz_addr(void);

extern const char *errmsg;
extern int scripted;
extern int isbinary;
extern int isglobal;
extern int lineno;
extern int cols;
extern int rows;
extern int ibufsz;
extern int newline_added;
extern int mutex;
extern char *ibuf;
extern char *ibufp;
}

#define GLB 001
#define GPR 002
#define GLS 004
#define GNP 010
#define GSG 020
#define ERR (-2)

/* ------------------------------------------------------------------ */
/* plumbing                                                            */
/* ------------------------------------------------------------------ */

static FILE *g_out;			/* the real stdout */
static FILE *g_err;			/* the real stderr */
static FILE *g_in;			/* the real stdin  */

#define ERRCAP 512
static char errb_p[ERRCAP], errb_r[ERRCAP];
static FILE *errs_p, *errs_r;

#define OUTCAP 3072
static char outb_p[OUTCAP], outb_r[OUTCAP];
static FILE *outs_p, *outs_r;

static void cap_begin(FILE *s, char *b, int n)
{
	fflush(s);
	rewind(s);
	memset(b, 0x7f, (size_t)n);
}

static long cap_end(FILE *s)
{
	fflush(s);
	return ftell(s);
}

static FILE *ro_stream(const char *data, size_t len)
{
	FILE *f = fmemopen((void *)data, len, "r");
	if (f == NULL) {
		f = tmpfile();
		if (f == NULL) {
			fprintf(g_err, "tmpfile() failed\n");
			exit(2);
		}
		if (len)
			fwrite(data, 1, len, f);
		rewind(f);
	}
	return f;
}

struct Checker {
	bool ok;
	char first[256];

	Checker() : ok(true) { first[0] = '\0'; }

	void note(const char *fmt, long long a, long long b, const char *what)
	{
		if (ok) {
			ok = false;
			snprintf(first, sizeof first, fmt, what, a, b);
		}
		ok = false;
	}
	void eqi(const char *what, long long a, long long b)
	{
		if (a != b)
			note("%s: port=%lld ref=%lld", a, b, what);
	}
	void eqmem(const char *what, const void *a, const void *b, size_t n)
	{
		if (n == 0 || memcmp(a, b, n) == 0)
			return;
		const unsigned char *x = (const unsigned char *)a;
		const unsigned char *y = (const unsigned char *)b;
		size_t i = 0;
		while (i < n && x[i] == y[i])
			i++;
		if (ok) {
			ok = false;
			snprintf(first, sizeof first,
			    "%s[%zu of %zu]: port=0x%02x ref=0x%02x",
			    what, i, n, x[i], y[i]);
		}
		ok = false;
	}
	void eqstr(const char *what, const char *a, const char *b)
	{
		if (a == NULL || b == NULL) {
			eqi(what, a != NULL, b != NULL);
			return;
		}
		if (strcmp(a, b) != 0) {
			if (ok) {
				ok = false;
				snprintf(first, sizeof first,
				    "%s: port=\"%.60s\" ref=\"%.60s\"", what, a, b);
			}
			ok = false;
		}
	}
};

struct Stat {
	const char *name;
	long cases;
	long fails;
	long shown;
};

static Stat stats[8];
static int nstats;

static Stat *mkstat(const char *name)
{
	Stat *s = &stats[nstats++];
	s->name = name;
	s->cases = s->fails = s->shown = 0;
	return s;
}

static void account(Stat *st, const Checker &c, const char *tag)
{
	st->cases++;
	if (!c.ok) {
		st->fails++;
		if (st->shown < 6) {
			st->shown++;
			fprintf(g_out, "  FAIL %s [%s] case %ld: %s\n",
			    st->name, tag, st->cases, c.first);
		}
	}
}

/* fixed-seed PRNG */
static uint64_t rs;
static void seed(uint64_t s) { rs = s; }
static uint32_t rnd(void)
{
	rs ^= rs << 13;
	rs ^= rs >> 7;
	rs ^= rs << 17;
	return (uint32_t)(rs >> 32);
}
static uint32_t rndn(uint32_t n) { return n ? rnd() % n : 0; }

static unsigned char rbyte(void)
{
	uint32_t r = rnd();
	switch (r & 15) {
	case 0: return 0x00;
	case 1: return (unsigned char)'\n';
	case 2: return (unsigned char)'\\';
	case 3: return 0x80;
	case 4: return 0xff;
	case 5: return 0x7f;
	case 6: return 0x1f;		/* just below the printable window */
	case 7: return 0x20;		/* just inside it                 */
	case 8: return 0x7e;		/* just below 127                 */
	case 9: return 0x07;		/* in ESCAPES                     */
	case 10: return 0x0b;		/* in ESCAPES                     */
	case 11: return 0x81;
	default: return (unsigned char)(r >> 9);
	}
}

/* ------------------------------------------------------------------ */
/* shared state handling                                               */
/* ------------------------------------------------------------------ */

static long off_of(const char *p, const char *base, int sz)
{
	if (p == NULL)
		return -1;
	if (base != NULL && p >= base && p <= base + sz)
		return p - base;
	return -1000000;
}

static void reset_state(int isbin, int nla, int lno)
{
	P::isbinary = isbinary = isbin;
	P::newline_added = newline_added = nla;
	P::lineno = lineno = lno;
	P::errmsg = errmsg = NULL;
	P::mutex = mutex = 0;
	P::ibufp = P::ibuf;
	ibufp = ibuf;
	if (P::ibuf)
		memset(P::ibuf, 0x7f, (size_t)P::ibufsz);
	if (ibuf)
		memset(ibuf, 0x7f, (size_t)ibufsz);
	errno = 0;
}

static int sb_old_p, sb_old_r;

static void sbuf_begin(void)
{
	char **pb = P::sbuf_addr();
	int *pn = P::sbufsz_addr();
	char **rb = ref_sbuf_addr();
	int *rn = ref_sbufsz_addr();

	sb_old_p = *pn;
	sb_old_r = *rn;
	if (*pb)
		memset(*pb, 0x7f, (size_t)*pn);
	if (*rb)
		memset(*rb, 0x7f, (size_t)*rn);
}

static void sbuf_cmp(Checker &c, long written)
{
	char **pb = P::sbuf_addr();
	int *pn = P::sbufsz_addr();
	char **rb = ref_sbuf_addr();
	int *rn = ref_sbufsz_addr();

	c.eqi("sbufsz", *pn, *rn);
	c.eqi("sbuf_alloc", *pb != NULL, *rb != NULL);
	if (!*pb || !*rb)
		return;
	size_t n = (size_t)(*pn < *rn ? *pn : *rn);
	bool grew = (*pn != sb_old_p || *rn != sb_old_r);
	if (grew) {
		/* the buffer just grew: the tail past the old (guard-filled)
		 * region is fresh malloc garbage, so limit the window to the
		 * old region plus whatever this call wrote. */
		size_t old = (size_t)(sb_old_p < sb_old_r ? sb_old_p : sb_old_r);
		size_t w = (size_t)(written < 0 ? 0 : written + 3);
		size_t lim = old > w ? old : w;
		if (lim < n)
			n = lim;
	}
	c.eqmem("sbuf", *pb, *rb, n);
	if (grew) {
		/* re-establish the guard over the fresh tail (dead space, no
		 * live line data lives there) so the next call can again be
		 * compared over the whole buffer. */
		if ((size_t)*pn > n)
			memset(*pb + n, 0x7f, (size_t)*pn - n);
		if ((size_t)*rn > n)
			memset(*rb + n, 0x7f, (size_t)*rn - n);
	}
}

static int ib_old_p, ib_old_r;

static void ibuf_begin(void)
{
	ib_old_p = P::ibufsz;
	ib_old_r = ibufsz;
}

static void ibuf_cmp(Checker &c, long written)
{
	c.eqi("ibufsz", P::ibufsz, ibufsz);
	c.eqi("ibuf_alloc", P::ibuf != NULL, ibuf != NULL);
	if (!P::ibuf || !ibuf)
		return;
	size_t n = (size_t)(P::ibufsz < ibufsz ? P::ibufsz : ibufsz);
	bool grew = (P::ibufsz != ib_old_p || ibufsz != ib_old_r);
	if (grew) {
		size_t old = (size_t)(ib_old_p < ib_old_r ? ib_old_p : ib_old_r);
		size_t w = (size_t)(written < 0 ? 0 : written + 3);
		size_t lim = old > w ? old : w;
		if (lim < n)
			n = lim;
	}
	c.eqmem("ibuf", P::ibuf, ibuf, n);
	if (grew) {
		if ((size_t)P::ibufsz > n)
			memset(P::ibuf + n, 0x7f, (size_t)P::ibufsz - n);
		if ((size_t)ibufsz > n)
			memset(ibuf + n, 0x7f, (size_t)ibufsz - n);
		ib_old_p = P::ibufsz;
		ib_old_r = ibufsz;
	}
}

static void cmp_globals(Checker &c)
{
	c.eqi("isbinary", P::isbinary, isbinary);
	c.eqi("newline_added", P::newline_added, newline_added);
	c.eqi("lineno", P::lineno, lineno);
	c.eqi("mutex", P::mutex, mutex);
	c.eqstr("errmsg", P::errmsg, errmsg);
	c.eqi("ibufp_off", off_of(P::ibufp, P::ibuf, P::ibufsz),
	    off_of(ibufp, ibuf, ibufsz));
}

static void cmp_stderr(Checker &c)
{
	c.eqmem("stderr", errb_p, errb_r, ERRCAP);
}

/* ------------------------------------------------------------------ */
/* get_stream_line                                                     */
/* ------------------------------------------------------------------ */

static Stat *st_gsl;

static void run_gsl(const char *data, size_t len, int isbin, int nla, int iters,
    const char *tag)
{
	Checker c;
	FILE *fp = ro_stream(data, len);
	FILE *fr = ro_stream(data, len);

	reset_state(isbin, nla, 0);

	for (int k = 0; k < iters; k++) {
		sbuf_begin();

		cap_begin(errs_p, errb_p, ERRCAP);
		stderr = errs_p;
		errno = 0;
		int rp = P::get_stream_line(fp);
		cap_end(errs_p);

		cap_begin(errs_r, errb_r, ERRCAP);
		stderr = errs_r;
		errno = 0;
		int rr = ref_get_stream_line(fr);
		cap_end(errs_r);

		stderr = g_err;

		c.eqi("ret", rp, rr);
		c.eqi("feof", feof(fp) != 0, feof(fr) != 0);
		cmp_globals(c);
		sbuf_cmp(c, rp > rr ? rp : rr);
		cmp_stderr(c);
		if (!c.ok)
			break;
	}
	fclose(fp);
	fclose(fr);
	account(st_gsl, c, tag);
}

static void test_get_stream_line(void)
{
	st_gsl = mkstat("get_stream_line");

	static const struct { const char *d; size_t n; } fixed[] = {
		{ "", 0 },
		{ "\n", 1 },
		{ "a", 1 },
		{ "a\n", 2 },
		{ "\0", 1 },
		{ "\0\n", 2 },
		{ "\0\0\0\n", 4 },
		{ "ab\0cd\n", 6 },
		{ "\n\n\n", 3 },
		{ "\xff", 1 },
		{ "\xff\n", 2 },
		{ "\x80\x81\xfe\xff\n", 5 },
		{ "\x7f\x7f\x7f", 3 },
		{ "abc", 3 },
		{ "abc\ndef", 7 },
		{ "abc\ndef\n", 8 },
		{ "\0a\0b\0\n\0c\n", 9 },
		{ "x\n\0\n", 4 },
	};

	for (unsigned i = 0; i < sizeof fixed / sizeof fixed[0]; i++)
		for (int b = 0; b < 2; b++)
			for (int nl = 0; nl < 2; nl++)
				run_gsl(fixed[i].d, fixed[i].n, b, nl, 6, "fixed");

	static char buf[64];
	for (long it = 0; it < 200000; it++) {
		size_t len = rndn(41);
		for (size_t i = 0; i < len; i++)
			buf[i] = (char)rbyte();
		int nl = 0;
		for (size_t i = 0; i < len; i++)
			if (buf[i] == '\n')
				nl++;
		int iters = nl + 2;
		if (iters > 16)
			iters = 16;
		run_gsl(buf, len, (int)(rnd() & 1), (int)(rnd() & 1), iters, "rand");
	}

	/* Buffer-growth cases last: they enlarge sbuf permanently.  Force
	 * REALLOC to fire, the buffer to move, and then straddle the exact
	 * "i + 2 == sbufsz" boundary in both directions. */
	{
		static char big[4096];
		static char edge[8192];

		for (size_t i = 0; i < sizeof big; i++)
			big[i] = (char)(i % 251);
		big[600] = '\n';
		big[1200] = '\n';
		run_gsl(big, 2500, 0, 0, 5, "grow");
		run_gsl(big, 2500, 1, 1, 5, "grow");

		int sz = *ref_sbufsz_addr();
		for (int d = -2; d <= 2; d++) {
			int n = sz - 2 + d;
			if (n < 1 || n + 1 >= (int)sizeof edge)
				continue;
			memset(edge, 'q', (size_t)n);
			edge[n] = '\n';
			run_gsl(edge, (size_t)n + 1, 0, 0, 3, "edge");
			run_gsl(edge, (size_t)n, 0, 0, 3, "edge");
			sz = *ref_sbufsz_addr();
		}
	}
}

/* ------------------------------------------------------------------ */
/* put_stream_line                                                     */
/* ------------------------------------------------------------------ */

static Stat *st_psl;

#define PSLBUF 160

static void run_psl(const char *s, int len, int cap, int readonly, const char *tag)
{
	Checker c;
	static char bp[PSLBUF], br[PSLBUF];

	if (cap < 1)
		cap = 1;
	if (cap > PSLBUF)
		cap = PSLBUF;

	reset_state(0, 0, 0);

	memset(bp, 0x7f, PSLBUF);
	memset(br, 0x7f, PSLBUF);

	FILE *fp = fmemopen(bp, (size_t)cap, readonly ? "r" : "w");
	FILE *fr = fmemopen(br, (size_t)cap, readonly ? "r" : "w");
	if (!fp || !fr) {
		fprintf(g_err, "fmemopen failed\n");
		exit(2);
	}

	cap_begin(errs_p, errb_p, ERRCAP);
	stderr = errs_p;
	errno = 0;
	int rp = P::put_stream_line(fp, s, len);
	fflush(fp);
	long pp = ftell(fp);
	cap_end(errs_p);

	cap_begin(errs_r, errb_r, ERRCAP);
	stderr = errs_r;
	errno = 0;
	int rr = ref_put_stream_line(fr, s, len);
	fflush(fr);
	long pr = ftell(fr);
	cap_end(errs_r);

	stderr = g_err;
	fclose(fp);
	fclose(fr);

	c.eqi("ret", rp, rr);
	c.eqi("pos", pp, pr);
	c.eqmem("outbuf", bp, br, PSLBUF);	/* whole buffer, guards included */
	cmp_globals(c);
	cmp_stderr(c);
	account(st_psl, c, tag);
}

static void test_put_stream_line(void)
{
	st_psl = mkstat("put_stream_line");

	static char s[PSLBUF];
	for (int i = 0; i < PSLBUF; i++)
		s[i] = (char)(i * 7 + 1);

	static const char nulls[16] = { 0 };
	static const char highs[8] = { (char)0x80, (char)0xff, (char)0x7f,
	    (char)0x00, (char)0x81, (char)0xfe, (char)0x01, (char)0x7e };

	/* len = 0 must write nothing at all, on both a writable and a
	 * read-only stream. */
	run_psl(s, 0, 16, 0, "len0");
	run_psl(s, 0, 16, 1, "len0ro");
	run_psl(s, 1, 16, 0, "len1");
	run_psl(s, 1, 1, 0, "exact1");
	run_psl(s, 2, 1, 0, "over1");
	run_psl(nulls, 16, 32, 0, "nuls");
	run_psl(highs, 8, 32, 0, "high");
	run_psl(highs, 8, 4, 0, "highover");
	run_psl(s, 1, 1, 1, "ro1");
	run_psl(s, 64, 64, 1, "ro64");

	/* straddle every capacity boundary */
	for (int capv = 1; capv <= 24; capv++)
		for (int len = 0; len <= 26; len++) {
			run_psl(s, len, capv, 0, "grid");
			run_psl(nulls, len > 16 ? 16 : len, capv, 0, "gridnul");
		}

	for (long it = 0; it < 200000; it++) {
		int len = (int)rndn(97);
		for (int i = 0; i < len; i++)
			s[i] = (char)rbyte();
		int capv = 1 + (int)rndn(112);
		int ro = (rnd() & 7) == 0;
		run_psl(s, len, capv, ro, "rand");
	}
}

/* ------------------------------------------------------------------ */
/* get_tty_line                                                        */
/* ------------------------------------------------------------------ */

static Stat *st_gtl;

static void run_gtl(const char *data, size_t len, int isbin, int iters,
    const char *tag)
{
	Checker c;
	FILE *fp = ro_stream(data, len);
	FILE *fr = ro_stream(data, len);

	reset_state(isbin, 0, 3);

	for (int k = 0; k < iters; k++) {
		ibuf_begin();

		cap_begin(errs_p, errb_p, ERRCAP);
		stderr = errs_p;
		stdin = fp;
		errno = 0;
		int rp = P::get_tty_line();
		cap_end(errs_p);

		cap_begin(errs_r, errb_r, ERRCAP);
		stderr = errs_r;
		stdin = fr;
		errno = 0;
		int rr = ref_get_tty_line();
		cap_end(errs_r);

		stdin = g_in;
		stderr = g_err;

		c.eqi("ret", rp, rr);
		cmp_globals(c);
		ibuf_cmp(c, rp > rr ? rp : rr);
		cmp_stderr(c);
		if (!c.ok)
			break;
	}
	fclose(fp);
	fclose(fr);
	account(st_gtl, c, tag);
}

static void test_get_tty_line(void)
{
	st_gtl = mkstat("get_tty_line");

	static const struct { const char *d; size_t n; } fixed[] = {
		{ "", 0 },
		{ "\n", 1 },
		{ "a", 1 },
		{ "a\n", 2 },
		{ "\0", 1 },
		{ "\0\n", 2 },
		{ "\0\0\n", 3 },
		{ "\n\n\n\n", 4 },
		{ "ab\ncd", 5 },
		{ "ab\ncd\n", 6 },
		{ "\xff\n", 2 },
		{ "\x80\x00\xff\n", 4 },
		{ "\x7f", 1 },
		{ "no newline at all", 17 },
	};

	for (unsigned i = 0; i < sizeof fixed / sizeof fixed[0]; i++)
		for (int b = 0; b < 2; b++)
			run_gtl(fixed[i].d, fixed[i].n, b, 8, "fixed");

	static char buf[64];
	for (long it = 0; it < 200000; it++) {
		size_t len = rndn(41);
		for (size_t i = 0; i < len; i++)
			buf[i] = (char)rbyte();
		int nl = 0;
		for (size_t i = 0; i < len; i++)
			if (buf[i] == '\n')
				nl++;
		int iters = nl + 3;
		if (iters > 16)
			iters = 16;
		run_gtl(buf, len, (int)(rnd() & 1), iters, "rand");
	}

	/* ibuf growth last: straddle the "i + 2 == ibufsz" boundary. */
	{
		static char big[4096];
		static char edge[8192];

		for (size_t i = 0; i < sizeof big; i++)
			big[i] = (char)('A' + (i % 26));
		big[1500] = '\n';
		run_gtl(big, 2000, 0, 4, "grow");

		int sz = ibufsz;
		for (int d = -3; d <= 3; d++) {
			int n = sz - 2 + d;
			if (n < 1 || n + 1 >= (int)sizeof edge)
				continue;
			memset(edge, 'z', (size_t)n);
			edge[n] = '\n';
			run_gtl(edge, (size_t)n + 1, 0, 3, "edge");
			run_gtl(edge, (size_t)n, 0, 3, "edge");
			sz = ibufsz;
		}
	}
}

/* ------------------------------------------------------------------ */
/* get_extended_line                                                   */
/* ------------------------------------------------------------------ */

static Stat *st_gel;
static long cv_written;			/* bytes of cvbuf known to be equal */
static int gel_last_size;		/* *sizep from the last completed call */
static int gel_last_cls;		/* 0 = NULL, 1 = ibuf, 2 = cvbuf */

static void run_gel(const char *data, size_t len, int nonl, const char *tag)
{
	Checker c;
	FILE *fp = ro_stream(data, len);
	FILE *fr = ro_stream(data, len);

	reset_state(0, 0, 0);

	/* prime ibufp with the first line, exactly as ed's main loop does */
	ibuf_begin();
	stdin = fp;
	int rp0 = P::get_tty_line();
	stdin = fr;
	int rr0 = ref_get_tty_line();
	stdin = g_in;

	c.eqi("prime_ret", rp0, rr0);
	cmp_globals(c);
	ibuf_cmp(c, rp0 > rr0 ? rp0 : rr0);
	if (!c.ok || rr0 <= 0 || ibuf[rr0 - 1] != '\n') {
		fclose(fp);
		fclose(fr);
		if (!c.ok)
			account(st_gel, c, tag);
		return;
	}

	int szp = -12345, szr = -12345;

	ibuf_begin();

	cap_begin(errs_p, errb_p, ERRCAP);
	stderr = errs_p;
	stdin = fp;
	errno = 0;
	char *retp = P::get_extended_line(&szp, nonl);
	cap_end(errs_p);

	cap_begin(errs_r, errb_r, ERRCAP);
	stderr = errs_r;
	stdin = fr;
	errno = 0;
	char *retr = ref_get_extended_line(&szr, nonl);
	cap_end(errs_r);

	stdin = g_in;
	stderr = g_err;
	fclose(fp);
	fclose(fr);

	int clsp = retp == NULL ? 0 :
	    (retp >= P::ibuf && retp < P::ibuf + P::ibufsz ? 1 : 2);
	int clsr = retr == NULL ? 0 :
	    (retr >= ibuf && retr < ibuf + ibufsz ? 1 : 2);

	gel_last_cls = clsr;
	gel_last_size = szr;

	c.eqi("ret_class", clsp, clsr);
	c.eqi("sizep", szp, szr);
	if (clsp == 1 && clsr == 1)
		c.eqi("ret_off", retp - P::ibuf, retr - ibuf);
	if (clsp == 2 && clsr == 2 && szp == szr && szp >= 0) {
		size_t n = (size_t)szp + 1;
		if (cv_written > 0) {
			/* cvbuf has been primed with a known pattern, so we can
			 * look well past this call's own write window. */
			size_t wide = n + 64;
			if (wide > (size_t)cv_written)
				wide = (size_t)cv_written;
			if (wide > n)
				n = wide;
		}
		c.eqmem("cvbuf", retp, retr, n);
	}
	cmp_globals(c);
	ibuf_cmp(c, rr0);
	cmp_stderr(c);
	account(st_gel, c, tag);
}

static void test_get_extended_line(void)
{
	st_gel = mkstat("get_extended_line");

	/* Priming call: grows cvbuf to its final size (512) and writes a
	 * known pattern across ~497 bytes of it in both builds, so later
	 * cases can be compared well past their own write window. */
	{
		static char prime[512];
		size_t k = 0;
		for (int line = 0; line < 5; line++) {
			for (int i = 0; i < 98; i++)
				prime[k++] = (char)('a' + (i % 26));
			prime[k++] = (line == 4) ? 'Z' : '\\';
			prime[k++] = '\n';
		}
		gel_last_cls = 0;
		gel_last_size = -1;
		run_gel(prime, k, 0, "prime");
		if (gel_last_cls != 2 || gel_last_size < 64) {
			fprintf(g_out, "  priming of cvbuf failed (cls=%d size=%d)\n",
			    gel_last_cls, gel_last_size);
			st_gel->fails++;
		} else
			cv_written = gel_last_size + 1;
	}

	static const struct { const char *d; size_t n; } fixed[] = {
		{ "\n", 1 },			/* l == 1, below the l < 2 gate */
		{ "a\n", 2 },			/* l == 2, no escape           */
		{ "\\\n", 2 },			/* l == 2, escape, then EOF    */
		{ "\\\nx\n", 4 },
		{ "\\\n\n", 3 },
		{ "\\\nab", 4 },		/* continuation without newline */
		{ "a\\\nb\n", 5 },
		{ "a\\\\\n", 4 },		/* even parity: not an escape  */
		{ "a\\\\\\\n", 5 },		/* odd parity: an escape       */
		{ "a\\\\\\\nq\n", 7 },
		{ "\\\n\\\n\\\nz\n", 8 },
		{ "\\\n\\\n\\\n", 6 },
		{ "ab\\\ncd\\\nef\n", 11 },
		{ "\0\\\n\0\n", 5 },
		{ "\xff\\\n\x80\n", 5 },
		{ "\\\n\\", 3 },
		{ "x\\\n", 3 },
		{ "\\\nx", 3 },
		{ "abc\\\n\n", 6 },
		{ "\\\\\n", 3 },
		{ "\\\\\\\n\n", 5 },
	};

	for (unsigned i = 0; i < sizeof fixed / sizeof fixed[0]; i++)
		for (int nonl = 0; nonl < 2; nonl++)
			run_gel(fixed[i].d, fixed[i].n, nonl, "fixed");

	static char buf[300];
	for (long it = 0; it < 200000; it++) {
		size_t k = 0;
		size_t limit = 40 + rndn(200);
		int first = 1;
		while (k < limit) {
			size_t seg = rndn(9);
			for (size_t i = 0; i < seg && k < sizeof buf - 4; i++) {
				unsigned char b;
				/* bias hard toward backslashes so both
				 * parities of has_trailing_escape are hit */
				if ((rnd() & 3) == 0)
					b = (unsigned char)'\\';
				else {
					b = rbyte();
					if (b == '\n')
						b = 'w';
				}
				buf[k++] = (char)b;
			}
			if (first || (rnd() & 15) != 0) {
				buf[k++] = '\n';
				first = 0;
			}
			if (k >= sizeof buf - 4)
				break;
		}
		run_gel(buf, k, (int)(rnd() & 1), "rand");
	}
}

/* ------------------------------------------------------------------ */
/* put_tty_line                                                        */
/* ------------------------------------------------------------------ */

static Stat *st_ptl;

static void run_ptl(const char *s, int l, long n, int gflag, int colsv, int rowsv,
    int scr, int isg, const char *sin, size_t sinlen, const char *tag)
{
	Checker c;
	FILE *fp = ro_stream(sin, sinlen);
	FILE *fr = ro_stream(sin, sinlen);

	reset_state(0, 0, 5);
	P::cols = cols = colsv;
	P::rows = rows = rowsv;
	P::scripted = scripted = scr;
	P::isglobal = isglobal = isg;

	ibuf_begin();

	cap_begin(outs_p, outb_p, OUTCAP);
	cap_begin(errs_p, errb_p, ERRCAP);
	stdout = outs_p;
	stderr = errs_p;
	stdin = fp;
	errno = 0;
	int rp = P::put_tty_line(s, l, n, gflag);
	long lp = cap_end(outs_p);
	cap_end(errs_p);

	cap_begin(outs_r, outb_r, OUTCAP);
	cap_begin(errs_r, errb_r, ERRCAP);
	stdout = outs_r;
	stderr = errs_r;
	stdin = fr;
	errno = 0;
	int rr = ref_put_tty_line(s, l, n, gflag);
	long lr = cap_end(outs_r);
	cap_end(errs_r);

	stdout = g_out;
	stderr = g_err;
	stdin = g_in;
	fclose(fp);
	fclose(fr);

	c.eqi("ret", rp, rr);
	c.eqi("outlen", lp, lr);
	c.eqmem("stdout", outb_p, outb_r, OUTCAP);	/* whole buffer */
	cmp_globals(c);
	ibuf_cmp(c, 64);
	cmp_stderr(c);
	account(st_ptl, c, tag);
}

static void test_put_tty_line(void)
{
	st_ptl = mkstat("put_tty_line");

	static char tty[256];
	for (size_t i = 0; i < sizeof tty; i++)
		tty[i] = ((i % 3) == 2) ? '\n' : (char)('a' + (i % 26));

	static const char *fixed[] = {
		"", "a", "\\", "\n", "\t", "\a\b\f\r\v", "\x1f", "\x20",
		"\x7e", "\x7f", "\x80", "\xff", "ab\\cd", "a\0b",
		"\x00\x01\x02\x3f\x40\xc0\xf8\x07",
		"0123456789012345678901234567890123456789",
	};
	static const int fixedlen[] = {
		0, 1, 1, 1, 1, 5, 1, 1,
		1, 1, 1, 1, 5, 3,
		8,
		40,
	};
	static const long ns[] = { 0, 1, -1, 12345, LONG_MAX, LONG_MIN };
	static const int colsvs[] = { -1, 0, 1, 2, 3, 8, 9, 39, 40, 72 };
	static const int rowsvs[] = { -1, 0, 1, 2, 22 };

	for (unsigned i = 0; i < sizeof fixed / sizeof fixed[0]; i++)
		for (int gflag = 0; gflag < 32; gflag++)
			for (unsigned ci = 0; ci < sizeof colsvs / sizeof colsvs[0]; ci++)
				for (unsigned ri = 0; ri < sizeof rowsvs / sizeof rowsvs[0]; ri++)
					for (int sc = 0; sc < 2; sc++)
						for (int ig = 0; ig < 2; ig++)
							run_ptl(fixed[i], fixedlen[i],
							    ns[(i + gflag) % 6], gflag,
							    colsvs[ci], rowsvs[ri],
							    sc, ig, tty, sizeof tty,
							    "fixed");

	static char s[64];
	for (long it = 0; it < 200000; it++) {
		int len = (int)rndn(41);
		for (int i = 0; i < len; i++)
			s[i] = (char)rbyte();
		int l = (rnd() & 7) == 0 ? (int)rndn((uint32_t)len + 1) : len;
		long n = ns[rndn(6)];
		int gflag = (int)rndn(32);
		int colsv = colsvs[rndn((uint32_t)(sizeof colsvs / sizeof colsvs[0]))];
		int rowsv = rowsvs[rndn((uint32_t)(sizeof rowsvs / sizeof rowsvs[0]))];
		int sc = (int)(rnd() & 1);
		int ig = (int)(rnd() & 1);
		size_t sinlen = 8 + rndn(200);
		run_ptl(s, l, n, gflag, colsv, rowsv, sc, ig, tty, sinlen, "rand");
	}
}

/* ------------------------------------------------------------------ */

int main(void)
{
	g_out = stdout;
	g_err = stderr;
	g_in = stdin;

	errs_p = fmemopen(errb_p, ERRCAP, "w");
	errs_r = fmemopen(errb_r, ERRCAP, "w");
	outs_p = fmemopen(outb_p, OUTCAP, "w");
	outs_r = fmemopen(outb_r, OUTCAP, "w");
	if (!errs_p || !errs_r || !outs_p || !outs_r) {
		fprintf(g_err, "fmemopen setup failed\n");
		return 2;
	}

	/* warm-up: get both sbuf and ibuf allocated so that the guard fill
	 * below has something to fill. */
	{
		const char *warm = "warmup line\n";
		FILE *a = ro_stream(warm, strlen(warm));
		FILE *b = ro_stream(warm, strlen(warm));
		P::get_stream_line(a);
		ref_get_stream_line(b);
		fclose(a);
		fclose(b);

		a = ro_stream(warm, strlen(warm));
		b = ro_stream(warm, strlen(warm));
		stdin = a;
		P::get_tty_line();
		stdin = b;
		ref_get_tty_line();
		stdin = g_in;
		fclose(a);
		fclose(b);
	}

	seed(0x0123456789abcdefULL);

	/* get_tty_line last: its growth cases permanently enlarge ibuf, which
	 * only makes the other sweeps slower. */
	test_get_stream_line();
	test_put_stream_line();
	test_get_extended_line();
	test_put_tty_line();
	test_get_tty_line();

	long totc = 0, totf = 0;
	fprintf(g_out, "\n%-20s %12s %12s  %s\n", "function", "cases", "failures",
	    "result");
	fprintf(g_out, "%-20s %12s %12s  %s\n", "--------------------",
	    "------------", "------------", "------");
	for (int i = 0; i < nstats; i++) {
		fprintf(g_out, "%-20s %12ld %12ld  %s\n", stats[i].name,
		    stats[i].cases, stats[i].fails,
		    stats[i].fails ? "FAIL" : "ok");
		totc += stats[i].cases;
		totf += stats[i].fails;
	}
	fprintf(g_out, "%-20s %12ld %12ld  %s\n", "TOTAL", totc, totf,
	    totf ? "FAIL" : "ok");
	fflush(g_out);

	return totf ? 1 : 0;
}
