/*
 * harness.cpp -- differential test for PBSD batch b0143.
 *
 * Every function of the batch is driven through the port and through the
 * ref_ oracle in oracle.c and the two are compared on: return value,
 * errno, the complete call log recorded by the shared stdio-internals
 * environment (which includes a snapshot of the FILE handed to
 * __vfwscanf()/__vfwprintf() and the bytes it was pointed at), and, for
 * the functions that write through a caller-supplied pointer, every byte
 * of a guard-filled arena that extends well past the nominal write
 * window.  Pointers are only ever compared as classifications or as
 * offsets from the arena base, never as raw addresses.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

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
#include <locale.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

import pbsd.lib.libc.stdio.b0143;

namespace P = pbsd::lib_libc_stdio::b0143;

/* ------------------------------------------------------------------ */
/* Mirror of the layouts defined in oracle.c.                          */
/* ------------------------------------------------------------------ */

typedef long ref_fpos_t;

struct ref_sbuf {
	unsigned char *_base;
	int _size;
};

struct ref_FILE {
	unsigned char *_p;
	int _r;
	int _w;
	short _flags;
	short _file;
	struct ref_sbuf _bf;
	int _lbfsize;
	void *_cookie;
	int (*_close)(void *);
	int (*_read)(void *, char *, int);
	ref_fpos_t (*_seek)(void *, ref_fpos_t, int);
	int (*_write)(void *, const char *, int);
	struct ref_sbuf _ub;
	unsigned char *_up;
	int _ur;
	unsigned char _ubuf[3];
	unsigned char _nbuf[1];
	struct ref_sbuf _lb;
	int _blksize;
	ref_fpos_t _offset;
	void *_fl_mutex;
	void *_fl_owner;
	int _fl_count;
	int _orientation;
	mbstate_t _mbstate;
	int _flags2;
};

#define B0143_DATA_MAX	288
#define B0143_PATH_MAX	64
#define B0143_MODE_MAX	40

struct Log {
	int n_sflags;
	int n_sfp;
	int n_open;
	int n_close;
	int n_sseek;
	int n_vfwscanf;
	int n_vfwprintf;
	int n_getlocale;
	int n_fixlocale;
	int n_wcsrtombs;
	int n_mbsrtowcs;

	char sflags_mode[B0143_MODE_MAX];
	int sflags_ret;
	int sflags_oflags;

	char open_path[B0143_PATH_MAX];
	int open_oflags;
	int open_mode;
	int open_ret;

	int close_fd;

	long sseek_off;
	int sseek_whence;
	int sseek_fp_is_target;

	int f_flags;
	int f_r;
	int f_w;
	int f_file;
	int f_lbfsize;
	int f_flags2;
	int f_bf_size;
	int f_base_is_null;
	long f_p_off;
	int f_cookie_is_null;
	int f_cookie_is_self;
	int f_read_is_null;
	int f_read_ret;
	int f_write_is_null;
	int f_seek_is_null;
	int f_close_is_null;
	int f_ub_size;
	int f_lb_size;
	int f_ur;
	int f_blksize;
	int f_orientation;
	long f_offset;
	int f_locale_is_null;
	int f_locale_is_global;
	int f_locale_is_c;
	int f_fmt_len;
	unsigned int f_fmt0;
	int f_datalen;
	unsigned char f_data[B0143_DATA_MAX];

	int wp_written;
	int wp_ret;
	int scanf_ret;
};

extern "C" {
void	b0143_reset(void);
void	b0143_get_log(void *);
void	b0143_cfg_sfp(void *);
void	b0143_cfg_open(int, int);
void	b0143_cfg_vfwprintf(const char *, int, int);
void	*b0143_fnptr(int);
void	*b0143_locale(void);
size_t	b0143_log_size(void);

int		ref_vswscanf_l(const wchar_t * __restrict, locale_t,
		    const wchar_t * __restrict, va_list);
int		ref_vswscanf(const wchar_t * __restrict,
		    const wchar_t * __restrict, va_list);
struct ref_FILE	*ref_fopen(const char * __restrict, const char * __restrict);
int		ref_vswprintf_l(wchar_t * __restrict, size_t, locale_t,
		    const wchar_t * __restrict, va_list);
int		ref_vswprintf(wchar_t * __restrict, size_t,
		    const wchar_t * __restrict, va_list);
}

/* ------------------------------------------------------------------ */
/* Bookkeeping.                                                        */
/* ------------------------------------------------------------------ */

enum {
	FN_VSWSCANF_L = 0,
	FN_VSWSCANF,
	FN_FOPEN,
	FN_VSWPRINTF_L,
	FN_VSWPRINTF,
	FN_COUNT
};

static const char *fn_name[FN_COUNT] = {
	"vswscanf_l",
	"vswscanf",
	"fopen",
	"vswprintf_l",
	"vswprintf",
};

static long fn_cases[FN_COUNT];
static long fn_fail[FN_COUNT];
static int fn_reported[FN_COUNT];

static void
record(int fn, int ok, const char *what, const char *detail)
{

	fn_cases[fn]++;
	if (ok)
		return;
	fn_fail[fn]++;
	if (fn_reported[fn] < 5) {
		fn_reported[fn]++;
		fprintf(stderr, "FAIL %s: %s [%s]\n", fn_name[fn], what,
		    detail == NULL ? "" : detail);
	}
}

/* ------------------------------------------------------------------ */
/* Deterministic PRNG.                                                 */
/* ------------------------------------------------------------------ */

static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{

	rng_state = s;
}

static uint64_t
rng_next(void)
{
	uint64_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return (x * 0x2545F4914F6CDD1DULL);
}

static unsigned
rng_below(unsigned n)
{

	return ((unsigned)(rng_next() >> 11) % n);
}

/* ------------------------------------------------------------------ */
/* Locale selection shared by both sides.                              */
/* ------------------------------------------------------------------ */

static locale_t
sel_locale(unsigned sel)
{

	switch (sel % 3u) {
	case 0:
		return ((locale_t)0);
	case 1:
		return (LC_GLOBAL_LOCALE);
	default:
		return ((locale_t)b0143_locale());
	}
}

/* ------------------------------------------------------------------ */
/* vswscanf_l / vswscanf.                                              */
/* ------------------------------------------------------------------ */

static int
p_vswscanf_l(const wchar_t *str, locale_t loc, const wchar_t *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = P::vswscanf_l(str, loc, fmt, ap);
	va_end(ap);
	return (r);
}

static int
r_vswscanf_l(const wchar_t *str, locale_t loc, const wchar_t *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = ref_vswscanf_l(str, loc, fmt, ap);
	va_end(ap);
	return (r);
}

static int
p_vswscanf(const wchar_t *str, const wchar_t *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = P::vswscanf(str, fmt, ap);
	va_end(ap);
	return (r);
}

static int
r_vswscanf(const wchar_t *str, const wchar_t *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = ref_vswscanf(str, fmt, ap);
	va_end(ap);
	return (r);
}

static void
report_log_diff(const char *who, const Log *a, const Log *b)
{
	size_t i;

	for (i = 0; i < sizeof(Log); i++)
		if (((const unsigned char *)a)[i] !=
		    ((const unsigned char *)b)[i]) {
			fprintf(stderr, "  %s: log byte %zu: %02x vs %02x\n",
			    who, i, ((const unsigned char *)a)[i],
			    ((const unsigned char *)b)[i]);
			return;
		}
}

static void
case_vswscanf_l(const wchar_t *str, unsigned locsel, const wchar_t *fmt,
    const char *tag)
{
	Log plog, rlog;
	locale_t loc;
	int pr, rr, pe, re, ok;

	loc = sel_locale(locsel);
	b0143_reset();
	errno = 0;
	pr = p_vswscanf_l(str, loc, fmt, 12345);
	pe = errno;
	b0143_get_log(&plog);

	loc = sel_locale(locsel);
	b0143_reset();
	errno = 0;
	rr = r_vswscanf_l(str, loc, fmt, 12345);
	re = errno;
	b0143_get_log(&rlog);

	ok = (pr == rr) && (pe == re) &&
	    (memcmp(&plog, &rlog, sizeof(Log)) == 0);
	if (!ok && fn_reported[FN_VSWSCANF_L] < 5)
		fprintf(stderr, "  ret %d/%d errno %d/%d\n", pr, rr, pe, re);
	if (!ok && fn_reported[FN_VSWSCANF_L] < 5)
		report_log_diff("vswscanf_l", &plog, &rlog);
	record(FN_VSWSCANF_L, ok, tag, NULL);
}

static void
case_vswscanf(const wchar_t *str, const wchar_t *fmt, const char *tag)
{
	Log plog, rlog;
	int pr, rr, pe, re, ok;

	b0143_reset();
	errno = 0;
	pr = p_vswscanf(str, fmt, 12345);
	pe = errno;
	b0143_get_log(&plog);

	b0143_reset();
	errno = 0;
	rr = r_vswscanf(str, fmt, 12345);
	re = errno;
	b0143_get_log(&rlog);

	ok = (pr == rr) && (pe == re) &&
	    (memcmp(&plog, &rlog, sizeof(Log)) == 0);
	if (!ok && fn_reported[FN_VSWSCANF] < 5)
		report_log_diff("vswscanf", &plog, &rlog);
	record(FN_VSWSCANF, ok, tag, NULL);
}

/* ------------------------------------------------------------------ */
/* fopen.                                                              */
/* ------------------------------------------------------------------ */

#define FGUARD	64

struct FArena {
	alignas(16) unsigned char raw[FGUARD + sizeof(struct ref_FILE) +
	    FGUARD];
};

struct Canon {
	int r, w, flags, file, bf_size, lbfsize, ub_size, lb_size;
	int ur, blksize, fl_count, orientation, flags2;
	long offset;
	int p_kind, bf_base_kind, ub_base_kind, up_kind, lb_base_kind;
	int cookie_kind, mutex_kind, owner_kind;
	int close_kind, read_kind, write_kind, seek_kind;
	long p_off, bf_base_off, cookie_off;
	unsigned char ubuf[3];
	unsigned char nbuf[1];
	unsigned char mbst[sizeof(mbstate_t)];
};

static int
all_bytes(const void *p, unsigned char v)
{
	unsigned char buf[sizeof(void *)];
	size_t i;

	memcpy(buf, p, sizeof(buf));
	for (i = 0; i < sizeof(buf); i++)
		if (buf[i] != v)
			return (0);
	return (1);
}

static int
ptr_kind(const void *slot, const void *self, long *off)
{
	void *p;

	memcpy(&p, slot, sizeof(p));
	*off = 0;
	if (all_bytes(slot, 0x7f))
		return (2);
	if (p == NULL)
		return (0);
	if (p == self)
		return (1);
	*off = (long)((const unsigned char *)p - (const unsigned char *)self);
	return (3);
}

static int
fn_kind(const void *slot)
{
	void *p;
	int i;

	memcpy(&p, slot, sizeof(p));
	if (all_bytes(slot, 0x7f))
		return (5);
	if (p == NULL)
		return (0);
	for (i = 0; i < 4; i++)
		if (p == b0143_fnptr(i))
			return (i + 1);
	return (6);
}

static void
canon_file(const struct ref_FILE *fp, struct Canon *c)
{
	const void *self = (const void *)fp;
	long dummy;

	memset(c, 0, sizeof(*c));
	c->r = fp->_r;
	c->w = fp->_w;
	c->flags = (int)fp->_flags;
	c->file = (int)fp->_file;
	c->bf_size = fp->_bf._size;
	c->lbfsize = fp->_lbfsize;
	c->ub_size = fp->_ub._size;
	c->lb_size = fp->_lb._size;
	c->ur = fp->_ur;
	c->blksize = fp->_blksize;
	c->fl_count = fp->_fl_count;
	c->orientation = fp->_orientation;
	c->flags2 = fp->_flags2;
	c->offset = (long)fp->_offset;

	c->p_kind = ptr_kind(&fp->_p, self, &c->p_off);
	c->bf_base_kind = ptr_kind(&fp->_bf._base, self, &c->bf_base_off);
	c->ub_base_kind = ptr_kind(&fp->_ub._base, self, &dummy);
	c->up_kind = ptr_kind(&fp->_up, self, &dummy);
	c->lb_base_kind = ptr_kind(&fp->_lb._base, self, &dummy);
	c->mutex_kind = ptr_kind(&fp->_fl_mutex, self, &dummy);
	c->owner_kind = ptr_kind(&fp->_fl_owner, self, &dummy);
	c->cookie_kind = ptr_kind(&fp->_cookie, self, &c->cookie_off);
	c->close_kind = fn_kind(&fp->_close);
	c->read_kind = fn_kind(&fp->_read);
	c->write_kind = fn_kind(&fp->_write);
	c->seek_kind = fn_kind(&fp->_seek);

	memcpy(c->ubuf, fp->_ubuf, sizeof(c->ubuf));
	memcpy(c->nbuf, fp->_nbuf, sizeof(c->nbuf));
	memcpy(c->mbst, &fp->_mbstate, sizeof(c->mbst));
}

static void
case_fopen(const char *file, const char *mode, int open_ret, int open_err,
    int sfp_null, unsigned char fill, const char *tag)
{
	struct FArena pa, ra;
	struct ref_FILE *pfp, *rfp;
	struct Canon pc, rc;
	Log plog, rlog;
	P::FILE *pret;
	struct ref_FILE *rret;
	int pe, re, pk, rk, ok;

	memset(pa.raw, fill, sizeof(pa.raw));
	memset(ra.raw, fill, sizeof(ra.raw));
	pfp = (struct ref_FILE *)(void *)(pa.raw + FGUARD);
	rfp = (struct ref_FILE *)(void *)(ra.raw + FGUARD);

	b0143_reset();
	b0143_cfg_sfp(sfp_null ? NULL : (void *)pfp);
	b0143_cfg_open(open_ret, open_err);
	errno = 0;
	pret = P::fopen(file, mode);
	pe = errno;
	b0143_get_log(&plog);

	b0143_reset();
	b0143_cfg_sfp(sfp_null ? NULL : (void *)rfp);
	b0143_cfg_open(open_ret, open_err);
	errno = 0;
	rret = ref_fopen(file, mode);
	re = errno;
	b0143_get_log(&rlog);

	pk = pret == NULL ? 0 : ((void *)pret == (void *)pfp ? 1 : 2);
	rk = rret == NULL ? 0 : ((void *)rret == (void *)rfp ? 1 : 2);

	canon_file(pfp, &pc);
	canon_file(rfp, &rc);

	ok = (pk == rk) && (pe == re) &&
	    (memcmp(&plog, &rlog, sizeof(Log)) == 0) &&
	    (memcmp(&pc, &rc, sizeof(struct Canon)) == 0) &&
	    (memcmp(pa.raw, ra.raw, FGUARD) == 0) &&
	    (memcmp(pa.raw + FGUARD + sizeof(struct ref_FILE),
		ra.raw + FGUARD + sizeof(struct ref_FILE), FGUARD) == 0);
	if (!ok && fn_reported[FN_FOPEN] < 5)
		report_log_diff("fopen", &plog, &rlog);
	record(FN_FOPEN, ok, tag, mode);
}

/* ------------------------------------------------------------------ */
/* vswprintf_l / vswprintf.                                            */
/* ------------------------------------------------------------------ */

#define WSLOTS	256
#define WLEAD	32

struct WArena {
	alignas(16) unsigned char raw[WSLOTS * sizeof(wchar_t)];
};

static wchar_t *
warena_s(struct WArena *a)
{

	return ((wchar_t *)(void *)(a->raw + WLEAD * sizeof(wchar_t)));
}

static int
p_vswprintf_l(wchar_t *s, size_t n, locale_t loc, const wchar_t *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = P::vswprintf_l(s, n, loc, fmt, ap);
	va_end(ap);
	return (r);
}

static int
r_vswprintf_l(wchar_t *s, size_t n, locale_t loc, const wchar_t *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = ref_vswprintf_l(s, n, loc, fmt, ap);
	va_end(ap);
	return (r);
}

static int
p_vswprintf(wchar_t *s, size_t n, const wchar_t *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = P::vswprintf(s, n, fmt, ap);
	va_end(ap);
	return (r);
}

static int
r_vswprintf(wchar_t *s, size_t n, const wchar_t *fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = ref_vswprintf(s, n, fmt, ap);
	va_end(ap);
	return (r);
}

static void
case_vswprintf_l(size_t n, unsigned locsel, const wchar_t *fmt,
    const char *out, int outlen, int outret, unsigned char fill,
    const char *tag)
{
	struct WArena pa, ra;
	Log plog, rlog;
	locale_t loc;
	int pr, rr, pe, re, ok;

	memset(pa.raw, fill, sizeof(pa.raw));
	memset(ra.raw, fill, sizeof(ra.raw));

	loc = sel_locale(locsel);
	b0143_reset();
	b0143_cfg_vfwprintf(out, outlen, outret);
	errno = 0;
	pr = p_vswprintf_l(warena_s(&pa), n, loc, fmt, 12345);
	pe = errno;
	b0143_get_log(&plog);

	loc = sel_locale(locsel);
	b0143_reset();
	b0143_cfg_vfwprintf(out, outlen, outret);
	errno = 0;
	rr = r_vswprintf_l(warena_s(&ra), n, loc, fmt, 12345);
	re = errno;
	b0143_get_log(&rlog);

	ok = (pr == rr) && (pe == re) &&
	    (memcmp(&plog, &rlog, sizeof(Log)) == 0) &&
	    (memcmp(pa.raw, ra.raw, sizeof(pa.raw)) == 0);
	if (!ok && fn_reported[FN_VSWPRINTF_L] < 5) {
		fprintf(stderr, "  ret %d/%d errno %d/%d n=%zu\n", pr, rr, pe,
		    re, n);
		report_log_diff("vswprintf_l", &plog, &rlog);
	}
	record(FN_VSWPRINTF_L, ok, tag, NULL);
}

static void
case_vswprintf(size_t n, const wchar_t *fmt, const char *out, int outlen,
    int outret, unsigned char fill, const char *tag)
{
	struct WArena pa, ra;
	Log plog, rlog;
	int pr, rr, pe, re, ok;

	memset(pa.raw, fill, sizeof(pa.raw));
	memset(ra.raw, fill, sizeof(ra.raw));

	b0143_reset();
	b0143_cfg_vfwprintf(out, outlen, outret);
	errno = 0;
	pr = p_vswprintf(warena_s(&pa), n, fmt, 12345);
	pe = errno;
	b0143_get_log(&plog);

	b0143_reset();
	b0143_cfg_vfwprintf(out, outlen, outret);
	errno = 0;
	rr = r_vswprintf(warena_s(&ra), n, fmt, 12345);
	re = errno;
	b0143_get_log(&rlog);

	ok = (pr == rr) && (pe == re) &&
	    (memcmp(&plog, &rlog, sizeof(Log)) == 0) &&
	    (memcmp(pa.raw, ra.raw, sizeof(pa.raw)) == 0);
	if (!ok && fn_reported[FN_VSWPRINTF] < 5)
		report_log_diff("vswprintf", &plog, &rlog);
	record(FN_VSWPRINTF, ok, tag, NULL);
}

/* ------------------------------------------------------------------ */
/* Hand-written edge cases.                                            */
/* ------------------------------------------------------------------ */

static const wchar_t *const edge_wstr[] = {
	L"",
	L"a",
	L"\x01",
	L"\x7f",
	L"\x80",
	L"\xff",
	L"\x80\x81\xfe\xff",
	L"\x7f\x80",
	L"ab",
	L"abc def",
	L"%d",
	L"\x100",
	L"\x7ff",
	L"\x800",
	L"\xd7ff",
	L"\xe000",
	L"\xffff",
	L"\x10000",
	L"\x10ffff",
	L"\xd800",			/* unencodable: lone surrogate */
	L"\xdfff",
	L"\x110000",			/* unencodable: out of range */
	L"a\xd800",
	L"\x110000" L"a",
	L"\x7f\x80\x7ff\x800\xffff\x10000",
	L"0123456789abcdef0123456789abcdef",
	L"%d %s %c",
	L" \t\n",
};
#define N_EDGE_WSTR	(sizeof(edge_wstr) / sizeof(edge_wstr[0]))

static const wchar_t *const edge_wfmt[] = {
	L"",
	L"%d",
	L"%s",
	L"%lc",
	L"a",
	L"\x80",
	L"\x10ffff",
	L"%d%d%d%d",
};
#define N_EDGE_WFMT	(sizeof(edge_wfmt) / sizeof(edge_wfmt[0]))

static const char *const edge_mode[] = {
	"", "r", "w", "a", "b", "z", "+", "x", "e",
	"r+", "w+", "a+", "rb", "wb", "ab", "rx", "wx", "ax",
	"re", "we", "ae", "r+b", "w+b", "a+b", "rbx", "wbx", "abx",
	"a+x", "wbx+e", "r+x", "arw", "war", "raw", "zz", "0", "\x80",
	"rrrr", "wwww", "aaaa", "abe+x", "a\x80", "r\xff",
};
#define N_EDGE_MODE	(sizeof(edge_mode) / sizeof(edge_mode[0]))

static const char *const edge_path[] = {
	"", "f", "/tmp/b0143", "a/b/c", "\x80\xff",
};
#define N_EDGE_PATH	(sizeof(edge_path) / sizeof(edge_path[0]))

static const int edge_fd[] = {
	-1, -2, 0, 1, 2, 3, 100,
	SHRT_MAX - 2, SHRT_MAX - 1, SHRT_MAX, SHRT_MAX + 1, SHRT_MAX + 2,
	40000, 65535, 65536, 100000, INT_MAX,
};
#define N_EDGE_FD	(sizeof(edge_fd) / sizeof(edge_fd[0]))

struct WpOut {
	const char *bytes;
	int len;
};

static const struct WpOut edge_out[] = {
	{ "", 0 },
	{ "a", 1 },
	{ "ab", 2 },
	{ "abc", 3 },
	{ "abcd", 4 },
	{ "abcdefghij", 10 },
	{ "abcdefghijklmnopqrstuvwxyz0123", 30 },
	{ "\x7f", 1 },
	{ "\xff", 1 },			/* invalid multibyte -> EILSEQ */
	{ "a\xff" "b", 3 },
	{ "\xc2\x80", 2 },		/* U+0080 */
	{ "\xdf\xbf", 2 },		/* U+07FF */
	{ "\xe0\xa0\x80", 3 },		/* U+0800 */
	{ "\xef\xbf\xbf", 3 },		/* U+FFFF */
	{ "\xf0\x90\x80\x80", 4 },	/* U+10000 */
	{ "\xf4\x8f\xbf\xbf", 4 },	/* U+10FFFF */
	{ "\xc2", 1 },			/* truncated -> EILSEQ */
	{ "\x80", 1 },			/* stray continuation -> EILSEQ */
	{ "a\0b", 3 },			/* embedded NUL stops conversion */
	{ "0123456789012345678901234567890123456789"
	  "0123456789012345678901234567890123456789"
	  "0123456789012345678901234567890123456789"
	  "0123456", 127 },		/* exactly fills the 127-byte window */
	{ "0123456789012345678901234567890123456789"
	  "0123456789012345678901234567890123456789"
	  "0123456789012345678901234567890123456789"
	  "01234567890", 131 },		/* over-long: mock truncates to 127 */
};
#define N_EDGE_OUT	(sizeof(edge_out) / sizeof(edge_out[0]))

static const size_t edge_n[] = {
	0, 1, 2, 3, 4, 5, 9, 10, 11, 29, 30, 31, 100,
	126, 127, 128,
	(size_t)INT_MAX - 1,
	(size_t)INT_MAX,
	(size_t)INT_MAX + 1,		/* n - 1 == INT_MAX: still allowed */
	(size_t)INT_MAX + 2,		/* n - 1 == INT_MAX + 1: EOVERFLOW */
	(size_t)INT_MAX + 3,
	SIZE_MAX - 1,
	SIZE_MAX,
};
#define N_EDGE_N	(sizeof(edge_n) / sizeof(edge_n[0]))

static const int edge_ret[] = { -32768, -7, -2, -1, 0, 1, 2, 127, 32767 };
#define N_EDGE_RET	(sizeof(edge_ret) / sizeof(edge_ret[0]))

static void
edge_cases(void)
{
	size_t i, j, k;
	unsigned sel;
	char buf[64];

	/* vswscanf_l / vswscanf */
	for (i = 0; i < N_EDGE_WSTR; i++) {
		for (j = 0; j < N_EDGE_WFMT; j++) {
			for (sel = 0; sel < 3; sel++) {
				snprintf(buf, sizeof(buf), "edge s%zu f%zu l%u",
				    i, j, sel);
				case_vswscanf_l(edge_wstr[i], sel,
				    edge_wfmt[j], buf);
			}
			snprintf(buf, sizeof(buf), "edge s%zu f%zu", i, j);
			case_vswscanf(edge_wstr[i], edge_wfmt[j], buf);
		}
	}

	/* A NUL-heavy wide buffer: wcslen() stops at the first L'\0'. */
	{
		static const wchar_t nulheavy[] = {
			0, 0, 0, 0, 0, 0, 0, 0
		};
		static const wchar_t nulmid[] = {
			L'a', L'b', 0, L'c', L'd', 0, 0
		};
		static const wchar_t hibytes[] = {
			0x80, 0x81, 0x9f, 0xa0, 0xfe, 0xff, 0
		};
		case_vswscanf_l(nulheavy, 0, L"%d", "edge nulheavy");
		case_vswscanf_l(nulmid, 1, L"%d", "edge nulmid");
		case_vswscanf_l(hibytes, 2, L"%d", "edge hibytes");
		case_vswscanf(nulheavy, L"%d", "edge nulheavy");
		case_vswscanf(nulmid, L"%d", "edge nulmid");
		case_vswscanf(hibytes, L"%d", "edge hibytes");
	}

	/* fopen */
	for (i = 0; i < N_EDGE_MODE; i++) {
		for (j = 0; j < N_EDGE_FD; j++) {
			snprintf(buf, sizeof(buf), "edge m%zu fd%d", i,
			    edge_fd[j]);
			case_fopen("/tmp/b0143", edge_mode[i], edge_fd[j],
			    ENOENT, 0, 0x7f, buf);
			case_fopen("/tmp/b0143", edge_mode[i], edge_fd[j],
			    ENOENT, 0, 0x00, buf);
		}
		snprintf(buf, sizeof(buf), "edge m%zu sfpnull", i);
		case_fopen("/tmp/b0143", edge_mode[i], 3, ENOENT, 1, 0x7f, buf);
		case_fopen("/tmp/b0143", edge_mode[i], 3, ENOENT, 1, 0x00, buf);
	}
	for (i = 0; i < N_EDGE_PATH; i++) {
		snprintf(buf, sizeof(buf), "edge p%zu", i);
		case_fopen(edge_path[i], "r", 3, ENOENT, 0, 0x7f, buf);
		case_fopen(edge_path[i], "a", 5, ENOENT, 0, 0x00, buf);
		case_fopen(edge_path[i], "w", -1, EACCES, 0, 0x7f, buf);
		case_fopen(edge_path[i], "a+", -1, EPERM, 0, 0x00, buf);
	}

	/* vswprintf_l / vswprintf */
	for (i = 0; i < N_EDGE_N; i++) {
		for (j = 0; j < N_EDGE_OUT; j++) {
			/*
			 * Keep the converted result inside the arena when n
			 * is enormous: the conversion stops at the NUL, so a
			 * short mock output is what bounds the write.
			 */
			if (edge_n[i] > (size_t)(WSLOTS - WLEAD - 8) &&
			    edge_out[j].len > 120)
				continue;
			for (k = 0; k < N_EDGE_RET; k++) {
				snprintf(buf, sizeof(buf),
				    "edge n%zu o%zu r%zu", i, j, k);
				case_vswprintf_l(edge_n[i], (unsigned)(i + j),
				    L"%ls", edge_out[j].bytes, edge_out[j].len,
				    edge_ret[k], 0x7f, buf);
				case_vswprintf_l(edge_n[i],
				    (unsigned)(i + j + 1), L"",
				    edge_out[j].bytes, edge_out[j].len,
				    edge_ret[k], 0x00, buf);
				case_vswprintf(edge_n[i], L"%ls",
				    edge_out[j].bytes, edge_out[j].len,
				    edge_ret[k], 0x7f, buf);
				case_vswprintf(edge_n[i], L"",
				    edge_out[j].bytes, edge_out[j].len,
				    edge_ret[k], 0x00, buf);
			}
		}
	}

	/*
	 * Straddle the nwc == n boundary exactly: an ASCII mock output of
	 * length L converts to L wide characters, so n == L truncates
	 * (EOVERFLOW, s[n - 1] cleared) and n == L + 1 does not.
	 */
	{
		static const char ascii[] =
		    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOP";
		int L;

		for (L = 0; L <= 40; L++) {
			size_t nn;

			for (nn = (size_t)(L > 2 ? L - 2 : 1);
			    nn <= (size_t)L + 2; nn++) {
				snprintf(buf, sizeof(buf), "straddle L%d n%zu",
				    L, nn);
				case_vswprintf_l(nn, (unsigned)L, L"%ls",
				    ascii, L, 0, 0x7f, buf);
				case_vswprintf_l(nn, (unsigned)(L + 1), L"%ls",
				    ascii, L, 3, 0x00, buf);
				case_vswprintf(nn, L"%ls", ascii, L, 0, 0x7f,
				    buf);
				case_vswprintf(nn, L"%ls", ascii, L, 3, 0x00,
				    buf);
			}
		}
	}
}

/* ------------------------------------------------------------------ */
/* Randomised sweeps.                                                  */
/* ------------------------------------------------------------------ */

static wchar_t
rand_wchar(void)
{

	switch (rng_below(10)) {
	case 0:
		return ((wchar_t)(1 + rng_below(0x7f)));
	case 1:
		return ((wchar_t)(0x80 + rng_below(0x80)));
	case 2:
		return ((wchar_t)(0x100 + rng_below(0x700)));
	case 3:
		return ((wchar_t)(0x800 + rng_below(0xd000)));
	case 4:
		return ((wchar_t)(0xe000 + rng_below(0x2000)));
	case 5:
		return ((wchar_t)(0x10000 + rng_below(0x100000)));
	case 6:
		return ((wchar_t)(0xd800 + rng_below(0x800)));
	case 7:
		return ((wchar_t)(0x110000 + rng_below(0x100000)));
	case 8:
		return ((wchar_t)0);
	default:
		return ((wchar_t)(0x20 + rng_below(0x60)));
	}
}

static void
rand_wstr(wchar_t *out, size_t cap)
{
	size_t len, i;

	len = rng_below((unsigned)cap - 1);
	for (i = 0; i < len; i++)
		out[i] = rand_wchar();
	out[len] = L'\0';
}

static void
sweep_vswscanf(long iters)
{
	wchar_t str[16], fmt[8];
	long i;

	for (i = 0; i < iters; i++) {
		rand_wstr(str, 13);
		rand_wstr(fmt, 7);
		case_vswscanf_l(str, rng_below(3), fmt, "rand");
		case_vswscanf(str, fmt, "rand");
	}
}

static void
sweep_fopen(long iters)
{
	static const char modechars[] = "rwab+xe z0\x80";
	char mode[8];
	long i;
	unsigned len, j;
	int fd;

	for (i = 0; i < iters; i++) {
		len = 1 + rng_below(5);
		for (j = 0; j < len; j++)
			mode[j] = modechars[rng_below(
			    (unsigned)sizeof(modechars) - 1)];
		mode[len] = '\0';

		switch (rng_below(6)) {
		case 0:
			fd = -1;
			break;
		case 1:
			fd = (int)rng_below(8);
			break;
		case 2:
			fd = SHRT_MAX - (int)rng_below(3);
			break;
		case 3:
			fd = SHRT_MAX + (int)rng_below(3);
			break;
		case 4:
			fd = (int)rng_below(200000);
			break;
		default:
			fd = -(int)(1 + rng_below(5));
			break;
		}

		case_fopen(edge_path[rng_below(N_EDGE_PATH)], mode, fd,
		    (int)(1 + rng_below(40)), (int)rng_below(8) == 0,
		    (rng_below(2) != 0) ? 0x7f : 0x00, "rand");
	}
}

static void
sweep_vswprintf(long iters)
{
	char out[48];
	wchar_t fmt[8];
	size_t n;
	int len, ret;
	unsigned j;
	long i;

	for (i = 0; i < iters; i++) {
		len = (int)rng_below(44);
		for (j = 0; j < (unsigned)len; j++) {
			switch (rng_below(8)) {
			case 0:
				out[j] = (char)(0x80 + rng_below(0x80));
				break;
			case 1:
				out[j] = (char)0xc2;
				break;
			case 2:
				out[j] = (char)0;
				break;
			default:
				out[j] = (char)(0x20 + rng_below(0x5f));
				break;
			}
		}

		switch (rng_below(8)) {
		case 0:
			n = 0;
			break;
		case 1:
			n = 1 + rng_below(4);
			break;
		case 2:
			n = (size_t)INT_MAX + rng_below(3);
			break;
		case 3:
			n = SIZE_MAX - rng_below(3);
			break;
		case 4:
			n = 120 + rng_below(60);
			break;
		default:
			n = 1 + rng_below(50);
			break;
		}

		ret = edge_ret[rng_below(N_EDGE_RET)];
		rand_wstr(fmt, 7);

		case_vswprintf_l(n, rng_below(3), fmt, out, len, ret,
		    (rng_below(2) != 0) ? 0x7f : 0x00, "rand");
		case_vswprintf(n, fmt, out, len, ret,
		    (rng_below(2) != 0) ? 0x7f : 0x00, "rand");
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	long total_cases = 0, total_fail = 0;
	int i, rc;

	setlocale(LC_ALL, "C.UTF-8");

	if (sizeof(Log) != b0143_log_size()) {
		fprintf(stderr,
		    "harness/oracle log layout mismatch: %zu vs %zu\n",
		    sizeof(Log), b0143_log_size());
		return (1);
	}

	edge_cases();

	rng_seed(0x0143C0FFEEULL);
	sweep_vswscanf(200000);		/* 200000 vswscanf_l + 200000 vswscanf */
	sweep_fopen(200000);
	sweep_vswprintf(200000);	/* 200000 vswprintf_l + 200000 vswprintf */

	printf("%-14s %10s %10s\n", "function", "cases", "failures");
	printf("%-14s %10s %10s\n", "--------------", "----------",
	    "----------");
	for (i = 0; i < FN_COUNT; i++) {
		printf("%-14s %10ld %10ld\n", fn_name[i], fn_cases[i],
		    fn_fail[i]);
		total_cases += fn_cases[i];
		total_fail += fn_fail[i];
	}
	printf("%-14s %10s %10s\n", "--------------", "----------",
	    "----------");
	printf("%-14s %10ld %10ld\n", "TOTAL", total_cases, total_fail);

	rc = total_fail == 0 ? 0 : 1;
	printf("%s\n", rc == 0 ? "PASS" : "FAIL");
	return (rc);
}
