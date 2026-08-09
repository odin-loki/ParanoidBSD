/*
 * harness.cpp -- differential test for PBSD batch b0326.
 */

#ifndef EOF
#define EOF (-1)
#endif

#include <climits>
#include <cerrno>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#ifndef OFF_MAX
#define OFF_MAX LLONG_MAX
#endif

import pbsd.lib.libc.stdio.b0326;

namespace P = pbsd::lib_libc_stdio::b0326;

using PortFILE = __b0326_FILE;
using PrintfInfo = printf_info;
using PrintfIo = __printf_io;

typedef long long ref_fpos_t;

struct RefSbuf {
	unsigned char *_base;
	int _size;
};

struct RefFILE {
	unsigned char *_p;
	int _r;
	int _w;
	short _flags;
	short _file;
	struct RefSbuf _bf;
	int _lbfsize;
	void *_cookie;
	int (*_close)(void *);
	int (*_read)(void *, char *, int);
	ref_fpos_t (*_seek)(void *, ref_fpos_t, int);
	int (*_write)(void *, const char *, int);
	struct RefSbuf _ub;
	unsigned char *_up;
	int _ur;
	unsigned char _ubuf[3];
	unsigned char _nbuf[1];
	struct RefSbuf _lb;
	int _blksize;
	ref_fpos_t _offset;
	void *_fl_mutex;
	void *_fl_owner;
	int _fl_count;
	int _orientation;
	unsigned char _mbstate_pad[8];
	int _flags2;
};

#define	__SLBF	0x0001
#define	__SNBF	0x0002
#define	__SRD	0x0004
#define	__SWR	0x0008
#define	__SMBF	0x0080
#define	__SAPP	0x0100
#define	__SOPT	0x0400
#define	__SNPT	0x0800
#define	__SOFF	0x1000
#define	__SERR	0x0040
#define	__S2OAP	0x0001

#define MINUTE 60
#define HOUR	(60 * MINUTE)
#define DAY	(24 * HOUR)
#define YEAR	(365 * DAY)

extern "C" {
extern int mock_flock_calls;
extern int mock_funlock_calls;
extern int mock_stdio_lock_calls;
extern int mock_stdio_unlock_calls;
extern int mock_fstat_ret;
extern struct stat mock_fstat_st;
extern int mock_malloc_fail;
extern int mock_malloc_calls;
extern int mock_isatty_ret;
extern int mock_sflush_ret;
extern int mock_sflush_calls;
extern int mock_close_ret;
extern int mock_close_calls;
extern ref_fpos_t mock_sseek_ret;
extern int mock_sseek_fail;
extern int mock_printf_out_ret;
extern int mock_printf_out_calls;
extern const char *mock_printf_out_last_buf;
extern int mock_printf_out_last_len;
extern int mock_printf_flush_calls;
extern void (*__cleanup)(void);
extern int mock_cleanup_set;
extern unsigned char mock_malloc_arena[262144];
void b0326_reset(void);
ref_fpos_t __sseek(void *, ref_fpos_t, int);
int __sclose(void *);

void ref___smakebuf(RefFILE *);
int ref___swhatbuf(RefFILE *, size_t *, int *);
int ref___printf_arginfo_time(const PrintfInfo *, size_t, int *);
int ref___printf_render_time(PrintfIo *, const PrintfInfo *,
    const void *const *);
int ref_fdclose(RefFILE *, int *);
int ref_fclose(RefFILE *);
long ref_ftell(RefFILE *);
off_t ref_ftello(RefFILE *);
int ref__ftello(RefFILE *, ref_fpos_t *);
}

enum Fn {
	FN_SMAKEBUF = 0,
	FN_SWHATBUF,
	FN_ARGINFO_TIME,
	FN_RENDER_TIME,
	FN_FDCLOSE,
	FN_FCLOSE,
	FN_FTELL,
	FN_FTELLO,
	FN_FTELLO_U,
	FN_COUNT
};

static const char *fn_name[FN_COUNT] = {
	"__smakebuf",
	"__swhatbuf",
	"__printf_arginfo_time",
	"__printf_render_time",
	"fdclose",
	"fclose",
	"ftell",
	"ftello",
	"_ftello",
};

static long fn_cases[FN_COUNT];
static long fn_fail[FN_COUNT];
static int fn_reported[FN_COUNT];
static uint64_t rng_state;

static ref_fpos_t
other_seek(void *cookie, ref_fpos_t pos, int whence)
{
	(void)cookie;
	(void)pos;
	(void)whence;
	return (0);
}

static int
bad_close(void *cookie)
{
	(void)cookie;
	return (mock_close_ret);
}

static void
rng_seed(uint64_t s)
{
	rng_state = s;
}

static uint64_t
rng_next(void)
{
	uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static int
rng_i32(void)
{
	return (int)(rng_next() & 0x7fffffffu);
}

static int
rng_bit(void)
{
	return (int)(rng_next() & 1U);
}

static void
record_fail(int fn, const char *msg)
{
	fn_fail[fn]++;
	if (fn_reported[fn] < 8) {
		fn_reported[fn]++;
		fprintf(stderr, "FAIL %s: %s\n", fn_name[fn], msg);
	}
}

static void
record_case(int fn)
{
	fn_cases[fn]++;
}

static void
harness_dprintf(const char *fmt, ...)
{
	char buf[512];
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	if (n > 0)
		(void)write(STDERR_FILENO, buf, (size_t)n);
}

enum {
	PK_NULL = 0,
	PK_NBUF,
	PK_UBUF,
	PK_MALLOC,
	PK_FP,
	PK_EXT
};

struct PtrNorm {
	int kind;
	ptrdiff_t off;
};

static PtrNorm
norm_ptr_port(const void *p, const PortFILE *fp)
{
	PtrNorm n = { PK_NULL, 0 };

	if (p == NULL)
		return (n);
	if (p == (const void *)fp->_nbuf) {
		n.kind = PK_NBUF;
		return (n);
	}
	if (p == (const void *)fp->_ubuf) {
		n.kind = PK_UBUF;
		return (n);
	}
	if (p >= (const void *)mock_malloc_arena &&
	    p < (const void *)(mock_malloc_arena + sizeof(mock_malloc_arena))) {
		n.kind = PK_MALLOC;
		n.off = (const char *)p - (const char *)mock_malloc_arena;
		return (n);
	}
	if (p >= (const void *)fp &&
	    p < (const void *)((const char *)fp + sizeof(*fp))) {
		n.kind = PK_FP;
		n.off = (const char *)p - (const char *)fp;
		return (n);
	}
	n.kind = PK_EXT;
	n.off = (ptrdiff_t)(uintptr_t)p;
	return (n);
}

static PtrNorm
norm_ptr_ref(const void *p, const RefFILE *fp)
{
	PtrNorm n = { PK_NULL, 0 };

	if (p == NULL)
		return (n);
	if (p == (const void *)fp->_nbuf) {
		n.kind = PK_NBUF;
		return (n);
	}
	if (p == (const void *)fp->_ubuf) {
		n.kind = PK_UBUF;
		return (n);
	}
	if (p >= (const void *)mock_malloc_arena &&
	    p < (const void *)(mock_malloc_arena + sizeof(mock_malloc_arena))) {
		n.kind = PK_MALLOC;
		n.off = (const char *)p - (const char *)mock_malloc_arena;
		return (n);
	}
	if (p >= (const void *)fp &&
	    p < (const void *)((const char *)fp + sizeof(*fp))) {
		n.kind = PK_FP;
		n.off = (const char *)p - (const char *)fp;
		return (n);
	}
	n.kind = PK_EXT;
	n.off = (ptrdiff_t)(uintptr_t)p;
	return (n);
}

static int
ptr_norm_eq(PtrNorm a, PtrNorm b)
{
	return (a.kind == b.kind && a.off == b.off);
}

struct FpSnap {
	short flags;
	short file;
	int r;
	int w;
	int bf_size;
	int blksize;
	int flags2;
	int ur;
	PtrNorm p;
	PtrNorm bf_base;
	PtrNorm ub_base;
	PtrNorm lb_base;
	ref_fpos_t offset;
};

static FpSnap
snap_port(const PortFILE *fp)
{
	FpSnap s = {};

	s.flags = fp->_flags;
	s.file = fp->_file;
	s.r = fp->_r;
	s.w = fp->_w;
	s.bf_size = fp->_bf._size;
	s.blksize = fp->_blksize;
	s.flags2 = fp->_flags2;
	s.ur = fp->_ur;
	s.offset = fp->_offset;
	s.p = norm_ptr_port(fp->_p, fp);
	s.bf_base = norm_ptr_port(fp->_bf._base, fp);
	s.ub_base = norm_ptr_port(fp->_ub._base, fp);
	s.lb_base = norm_ptr_port(fp->_lb._base, fp);
	return (s);
}

static FpSnap
snap_ref(const RefFILE *fp)
{
	FpSnap s = {};

	s.flags = fp->_flags;
	s.file = fp->_file;
	s.r = fp->_r;
	s.w = fp->_w;
	s.bf_size = fp->_bf._size;
	s.blksize = fp->_blksize;
	s.flags2 = fp->_flags2;
	s.ur = fp->_ur;
	s.offset = fp->_offset;
	s.p = norm_ptr_ref(fp->_p, fp);
	s.bf_base = norm_ptr_ref(fp->_bf._base, fp);
	s.ub_base = norm_ptr_ref(fp->_ub._base, fp);
	s.lb_base = norm_ptr_ref(fp->_lb._base, fp);
	return (s);
}

static int
snap_eq(const FpSnap *a, const FpSnap *b)
{
	return (a->flags == b->flags && a->file == b->file && a->r == b->r &&
	    a->w == b->w && a->bf_size == b->bf_size &&
	    a->blksize == b->blksize && a->flags2 == b->flags2 &&
	    a->ur == b->ur && a->offset == b->offset &&
	    ptr_norm_eq(a->p, b->p) && ptr_norm_eq(a->bf_base, b->bf_base) &&
	    ptr_norm_eq(a->ub_base, b->ub_base) &&
	    ptr_norm_eq(a->lb_base, b->lb_base));
}

struct MockSnap {
	int flock;
	int funlock;
	int stdio_lock;
	int stdio_unlock;
	int malloc_calls;
	int sflush_calls;
	int close_calls;
	int cleanup_set;
	void *cleanup_fn;
};

static MockSnap
snap_mocks(void)
{
	MockSnap m;

	m.flock = mock_flock_calls;
	m.funlock = mock_funlock_calls;
	m.stdio_lock = mock_stdio_lock_calls;
	m.stdio_unlock = mock_stdio_unlock_calls;
	m.malloc_calls = mock_malloc_calls;
	m.sflush_calls = mock_sflush_calls;
	m.close_calls = mock_close_calls;
	m.cleanup_set = mock_cleanup_set;
	m.cleanup_fn = (void *)__cleanup;
	return (m);
}

static int
mock_eq(const MockSnap *a, const MockSnap *b)
{
	return (a->flock == b->flock && a->funlock == b->funlock &&
	    a->stdio_lock == b->stdio_lock &&
	    a->stdio_unlock == b->stdio_unlock &&
	    a->malloc_calls == b->malloc_calls &&
	    a->sflush_calls == b->sflush_calls &&
	    a->close_calls == b->close_calls &&
	    a->cleanup_set == b->cleanup_set &&
	    a->cleanup_fn == b->cleanup_fn);
}

static void
fp_basic_init(PortFILE *pf, RefFILE *rf, short flags, short file)
{
	memset(pf, 0, sizeof(*pf));
	memset(rf, 0, sizeof(*rf));
	pf->_flags = rf->_flags = flags;
	pf->_file = rf->_file = file;
	pf->_seek = (long long (*)(void *, long long, int))__sseek;
	rf->_seek = __sseek;
	pf->_close = (int (*)(void *))__sclose;
	rf->_close = __sclose;
}

static void
cfg_fstat(int ok, mode_t mode, blksize_t blksize)
{
	mock_fstat_ret = ok ? 0 : -1;
	memset(&mock_fstat_st, 0, sizeof(mock_fstat_st));
	mock_fstat_st.st_mode = mode;
	mock_fstat_st.st_blksize = blksize;
}

static int
cmp_swhatbuf(int fn, PortFILE *pf, RefFILE *rf, int pret, int rret,
    size_t psize, size_t rsize, int pcould, int rcould)
{
	if (pret != rret || psize != rsize || pcould != rcould) {
		record_fail(fn, "return/out-params mismatch");
		return (0);
	}
	{
		FpSnap sp = snap_port(pf);
		FpSnap sr = snap_ref(rf);

		if (!snap_eq(&sp, &sr)) {
			record_fail(fn, "FILE state mismatch");
			return (0);
		}
	}
	return (1);
}

static void
test_swhatbuf_edge(int fn, short file, int fstat_ok, mode_t mode,
    blksize_t blksize, int use_other_seek)
{
	PortFILE pf;
	RefFILE rf;
	size_t psz, rsz;
	int pcould, rcould;
	int pret, rret;

	record_case(fn);
	b0326_reset();
	cfg_fstat(fstat_ok, mode, blksize);
	fp_basic_init(&pf, &rf, __SRD, file);
	if (use_other_seek) {
		pf._seek = (long long (*)(void *, long long, int))other_seek;
		rf._seek = other_seek;
	}
	pret = P::__swhatbuf(&pf, &psz, &pcould);

	b0326_reset();
	cfg_fstat(fstat_ok, mode, blksize);
	fp_basic_init(&pf, &rf, __SRD, file);
	if (use_other_seek) {
		pf._seek = (long long (*)(void *, long long, int))other_seek;
		rf._seek = other_seek;
	}
	rret = ref___swhatbuf(&rf, &rsz, &rcould);

	(void)cmp_swhatbuf(fn, &pf, &rf, pret, rret, psz, rsz, pcould, rcould);
}

static void
test_smakebuf_edge(int fn, short init_flags, short file, int fstat_ok,
    mode_t mode, blksize_t blksize, int malloc_fail, int is_tty)
{
	PortFILE pf, pf0;
	RefFILE rf, rf0;
	MockSnap mm, mr;

	record_case(fn);

	b0326_reset();
	mock_malloc_fail = malloc_fail;
	mock_isatty_ret = is_tty;
	cfg_fstat(fstat_ok, mode, blksize);
	fp_basic_init(&pf, &rf, init_flags, file);
	P::__smakebuf(&pf);
	mm = snap_mocks();
	memcpy(&pf0, &pf, sizeof(pf0));

	b0326_reset();
	mock_malloc_fail = malloc_fail;
	mock_isatty_ret = is_tty;
	cfg_fstat(fstat_ok, mode, blksize);
	fp_basic_init(&pf, &rf, init_flags, file);
	ref___smakebuf(&rf);
	mr = snap_mocks();
	memcpy(&rf0, &rf, sizeof(rf0));

	{
		FpSnap sp = snap_port(&pf0);
		FpSnap sr = snap_ref(&rf0);

		if (!snap_eq(&sp, &sr))
			record_fail(fn, "FILE snap mismatch");
	}
	if (!mock_eq(&mm, &mr))
		record_fail(fn, "mock snap mismatch");
}

static void
test_arginfo_edge(int fn, size_t n)
{
	PrintfInfo pi = {};
	int argt_p[8];
	int argt_r[8];
	int pret, rret;

	memset(argt_p, 0x7f, sizeof(argt_p));
	memset(argt_r, 0x7f, sizeof(argt_r));
	record_case(fn);

	pret = P::__printf_arginfo_time(&pi, n, argt_p);
	rret = ref___printf_arginfo_time(&pi, n, argt_r);

	if (pret != rret || argt_p[0] != argt_r[0])
		record_fail(fn, "arginfo mismatch");
	if (memcmp(argt_p, argt_r, sizeof(argt_p)) != 0)
		record_fail(fn, "argt buffer mismatch");
}

struct TimeVals {
	time_t tt;
	struct timeval tv;
	struct timespec ts;
};

static void
test_render_edge(int fn, PrintfInfo pi, TimeVals *tv)
{
	PrintfIo io = {};
	const void *argp[1];
	const void *argr[1];
	int pret, rret;
	int out_calls_p, flush_p, len_p;
	const char *buf_p;

	if (pi.is_long)
		argp[0] = argr[0] = &tv->tv;
	else if (pi.is_long_double)
		argp[0] = argr[0] = &tv->ts;
	else
		argp[0] = argr[0] = &tv->tt;

	b0326_reset();
	mock_printf_out_ret = 42;
	record_case(fn);
	pret = P::__printf_render_time(&io, &pi, argp);
	out_calls_p = mock_printf_out_calls;
	flush_p = mock_printf_flush_calls;
	buf_p = mock_printf_out_last_buf;
	len_p = mock_printf_out_last_len;

	b0326_reset();
	mock_printf_out_ret = 42;
	rret = ref___printf_render_time(&io, &pi, argr);

	if (pret != rret)
		record_fail(fn, "return mismatch");
	if (mock_printf_out_calls != out_calls_p)
		record_fail(fn, "printf_out calls mismatch");
	if (mock_printf_flush_calls != flush_p)
		record_fail(fn, "printf_flush calls mismatch");
	if (mock_printf_out_last_len != len_p)
		record_fail(fn, "printf_out len mismatch");
	if (len_p > 0 && (buf_p == NULL || mock_printf_out_last_buf == NULL ||
	    memcmp(buf_p, mock_printf_out_last_buf, (size_t)len_p) != 0))
		record_fail(fn, "printf_out buf mismatch");
}

static void
setup_close_fp(PortFILE *pf, RefFILE *rf, short flags, short file,
    int smb_malloc, int has_ub_heap, int has_lb_heap, unsigned char *ub_heap,
    unsigned char *lb_heap, unsigned char *bf_heap, int close_fn)
{
	fp_basic_init(pf, rf, flags, file);
	if (smb_malloc) {
		unsigned char *bf_p = (unsigned char *)malloc(64);
		unsigned char *bf_r = (unsigned char *)malloc(64);

		pf->_flags |= __SMBF;
		rf->_flags |= __SMBF;
		pf->_bf._base = pf->_p = bf_p;
		rf->_bf._base = rf->_p = bf_r;
		pf->_bf._size = rf->_bf._size = 64;
		(void)bf_heap;
	} else {
		pf->_bf._base = pf->_p = pf->_nbuf;
		rf->_bf._base = rf->_p = rf->_nbuf;
		(void)bf_heap;
	}
	if (has_ub_heap) {
		pf->_ub._base = (unsigned char *)malloc(16);
		rf->_ub._base = (unsigned char *)malloc(16);
		(void)ub_heap;
	}
	if (has_lb_heap) {
		pf->_lb._base = (unsigned char *)malloc(16);
		rf->_lb._base = (unsigned char *)malloc(16);
		(void)lb_heap;
	}
	if (close_fn == 0) {
		pf->_close = (int (*)(void *))__sclose;
		rf->_close = __sclose;
	} else {
		pf->_close = bad_close;
		rf->_close = bad_close;
	}
	pf->_cookie = rf->_cookie = (void *)0xC0326UL;
}

static void
test_fclose_edge(int fn, int use_fdclose, int fdp_valid, short flags,
    short file, int smb, int use_ub_heap, int use_lb_heap, int close_fn,
    int sflush_ret, int close_ret)
{
	PortFILE pf;
	RefFILE rf;
	unsigned char ub_storage[32];
	unsigned char lb_storage[32];
	unsigned char bf_storage[64];
	int fdp_p = 0x7f7f7f7f;
	int fdp_r = 0x7f7f7f7f;
	int pret, rret;
	int err_p, err_r;
	MockSnap mp, mr;

	memset(ub_storage, 0x80, sizeof(ub_storage));
	memset(lb_storage, 0x81, sizeof(lb_storage));
	memset(bf_storage, 0x82, sizeof(bf_storage));
	record_case(fn);

	b0326_reset();
	mock_sflush_ret = sflush_ret;
	mock_close_ret = close_ret;
	setup_close_fp(&pf, &rf, flags, file, smb, use_ub_heap, use_lb_heap,
	    ub_storage, lb_storage, bf_storage, close_fn);
	errno = 0;
	if (use_fdclose)
		pret = P::fdclose(&pf, fdp_valid ? &fdp_p : NULL);
	else
		pret = P::fclose(&pf);
	err_p = errno;
	mp = snap_mocks();

	b0326_reset();
	mock_sflush_ret = sflush_ret;
	mock_close_ret = close_ret;
	setup_close_fp(&pf, &rf, flags, file, smb, use_ub_heap, use_lb_heap,
	    ub_storage, lb_storage, bf_storage, close_fn);
	errno = 0;
	if (use_fdclose)
		rret = ref_fdclose(&rf, fdp_valid ? &fdp_r : NULL);
	else
		rret = ref_fclose(&rf);
	err_r = errno;
	mr = snap_mocks();

	if (pret != rret)
		record_fail(fn, "return mismatch");
	if (err_p != err_r)
		record_fail(fn, "errno mismatch");
	if (fdp_valid && fdp_p != fdp_r)
		record_fail(fn, "fdp mismatch");
	{
		FpSnap sp = snap_port(&pf);
		FpSnap sr = snap_ref(&rf);

		if (!snap_eq(&sp, &sr))
			record_fail(fn, "FILE snap mismatch");
	}
	if (!mock_eq(&mp, &mr))
		record_fail(fn, "mock snap mismatch");
}

static void
setup_ftell_fp(PortFILE *pf, RefFILE *rf, short flags, int flags2,
    ref_fpos_t offset, int r, int ur, int has_ub, int p_off, int seek_null,
    int sseek_fail, ref_fpos_t sseek_ret)
{
	unsigned char buf[32];

	memset(buf, 0x55, sizeof(buf));
	fp_basic_init(pf, rf, flags, 3);
	pf->_flags2 = rf->_flags2 = (short)flags2;
	pf->_offset = rf->_offset = offset;
	pf->_r = rf->_r = r;
	pf->_ur = rf->_ur = ur;
	pf->_bf._base = rf->_bf._base = buf;
	pf->_p = rf->_p = buf + p_off;
	if (has_ub) {
		pf->_ub._base = pf->_ubuf;
		rf->_ub._base = rf->_ubuf;
	}
	if (seek_null) {
		pf->_seek = NULL;
		rf->_seek = NULL;
	}
	mock_sseek_fail = sseek_fail;
	mock_sseek_ret = sseek_ret;
}

static void
test_ftello_u_edge(int fn, short flags, int flags2, ref_fpos_t offset, int r,
    int ur, int has_ub, int p_off, int seek_null, int sseek_fail,
    ref_fpos_t sseek_ret)
{
	PortFILE pf;
	RefFILE rf;
	ref_fpos_t op = 0x7f7f7f7f7f7f7f7fLL;
	ref_fpos_t orv = 0x7f7f7f7f7f7f7f7fLL;
	int pret, rret;
	int err_p, err_r;

	record_case(fn);
	b0326_reset();
	setup_ftell_fp(&pf, &rf, flags, flags2, offset, r, ur, has_ub, p_off,
	    seek_null, sseek_fail, sseek_ret);
	errno = 0;
	pret = P::_ftello(&pf, &op);
	err_p = errno;

	b0326_reset();
	setup_ftell_fp(&pf, &rf, flags, flags2, offset, r, ur, has_ub, p_off,
	    seek_null, sseek_fail, sseek_ret);
	errno = 0;
	rret = ref__ftello(&rf, &orv);
	err_r = errno;

	if (pret != rret)
		record_fail(fn, "_ftello ret mismatch");
	if (err_p != err_r)
		record_fail(fn, "_ftello errno mismatch");
	if (op != orv)
		record_fail(fn, "_ftello offset mismatch");
	{
		FpSnap sp = snap_port(&pf);
		FpSnap sr = snap_ref(&rf);

		if (!snap_eq(&sp, &sr))
			record_fail(fn, "FILE snap mismatch");
	}
}

static void
test_ftello_edge(int fn, short flags, int flags2, ref_fpos_t offset, int r,
    int ur, int has_ub, int p_off, int sseek_fail, ref_fpos_t sseek_ret)
{
	PortFILE pf;
	RefFILE rf;
	off_t pv, rv;
	int err_p, err_r;

	record_case(fn);
	b0326_reset();
	setup_ftell_fp(&pf, &rf, flags, flags2, offset, r, ur, has_ub, p_off,
	    0, sseek_fail, sseek_ret);
	errno = 0;
	pv = P::ftello(&pf);
	err_p = errno;

	b0326_reset();
	setup_ftell_fp(&pf, &rf, flags, flags2, offset, r, ur, has_ub, p_off,
	    0, sseek_fail, sseek_ret);
	errno = 0;
	rv = ref_ftello(&rf);
	err_r = errno;

	if (pv != rv)
		record_fail(fn, "ftello rv mismatch");
	if (err_p != err_r)
		record_fail(fn, "ftello errno mismatch");
}

static void
test_ftell_edge(int fn, ref_fpos_t inner_ret)
{
	PortFILE pf;
	RefFILE rf;
	long pv, rv;
	int err_p, err_r;

	record_case(fn);
	b0326_reset();
	setup_ftell_fp(&pf, &rf, __SRD, 0, 0, 0, 0, 0, 0, 0, 0, inner_ret);
	errno = 0;
	pv = P::ftell(&pf);
	err_p = errno;

	b0326_reset();
	setup_ftell_fp(&pf, &rf, __SRD, 0, 0, 0, 0, 0, 0, 0, 0, inner_ret);
	errno = 0;
	rv = ref_ftell(&rf);
	err_r = errno;

	if (pv != rv)
		record_fail(fn, "ftell rv mismatch");
	if (err_p != err_r)
		record_fail(fn, "ftell errno mismatch");
}

static void
run_swhatbuf_edges(void)
{
	const int fn = FN_SWHATBUF;

	test_swhatbuf_edge(fn, -1, 0, 0, 0, 0);
	test_swhatbuf_edge(fn, 3, 0, 0, 0, 0);
	test_swhatbuf_edge(fn, 3, 1, S_IFCHR, 0, 0);
	test_swhatbuf_edge(fn, 3, 1, S_IFCHR, -1, 0);
	test_swhatbuf_edge(fn, 3, 1, S_IFREG, 4096, 0);
	test_swhatbuf_edge(fn, 3, 1, S_IFREG, 4096, 1);
	test_swhatbuf_edge(fn, 3, 1, S_IFIFO, 512, 0);
}

static void
run_smakebuf_edges(void)
{
	const int fn = FN_SMAKEBUF;

	test_smakebuf_edge(fn, __SNBF, 1, 1, S_IFREG, 4096, 0, 0);
	test_smakebuf_edge(fn, __SRD, 1, 1, S_IFREG, 4096, 1, 0);
	test_smakebuf_edge(fn, __SRD, 1, 1, S_IFREG, 4096, 0, 0);
	test_smakebuf_edge(fn, __SRD, 1, 1, S_IFCHR, 512, 0, 1);
	test_smakebuf_edge(fn, __SRD, 1, 0, 0, 0, 0, 0);
	test_smakebuf_edge(fn, __SRD, -1, 1, S_IFREG, 8192, 0, 0);
	test_smakebuf_edge(fn, __SRD, 1, 1, S_IFREG, 0, 0, 0);
}

static void
run_arginfo_edges(void)
{
	test_arginfo_edge(FN_ARGINFO_TIME, 1);
	test_arginfo_edge(FN_ARGINFO_TIME, 4);
}

static void
run_render_edges(void)
{
	PrintfInfo pi;
	TimeVals tv;

	memset(&pi, 0, sizeof(pi));
	tv.tt = 0;
	test_render_edge(FN_RENDER_TIME, pi, &tv);

	pi.is_long = 1;
	tv.tv.tv_sec = 0;
	tv.tv.tv_usec = 123456;
	test_render_edge(FN_RENDER_TIME, pi, &tv);

	memset(&pi, 0, sizeof(pi));
	pi.is_long_double = 1;
	pi.prec = 3;
	tv.ts.tv_sec = 5;
	tv.ts.tv_nsec = 987654321;
	test_render_edge(FN_RENDER_TIME, pi, &tv);

	memset(&pi, 0, sizeof(pi));
	pi.alt = 1;
	tv.tt = (time_t)YEAR + DAY + HOUR + MINUTE + 42;
	test_render_edge(FN_RENDER_TIME, pi, &tv);

	memset(&pi, 0, sizeof(pi));
	pi.alt = 1;
	pi.is_long = 1;
	pi.prec = 0;
	tv.tv.tv_sec = DAY;
	tv.tv.tv_usec = 999999;
	test_render_edge(FN_RENDER_TIME, pi, &tv);

	memset(&pi, 0, sizeof(pi));
	pi.is_long_double = 1;
	pi.prec = 9;
	tv.ts.tv_sec = 1;
	tv.ts.tv_nsec = 1;
	test_render_edge(FN_RENDER_TIME, pi, &tv);

	memset(&pi, 0, sizeof(pi));
	tv.tt = (time_t)-1;
	test_render_edge(FN_RENDER_TIME, pi, &tv);
}

static void
run_fclose_edges(void)
{
	test_fclose_edge(FN_FCLOSE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	test_fclose_edge(FN_FCLOSE, 0, 0, __SWR, 3, 0, 0, 0, 0, 0, 0);
	test_fclose_edge(FN_FCLOSE, 0, 0, __SWR, 3, 1, 0, 0, 0, EOF, 0);
	test_fclose_edge(FN_FCLOSE, 0, 0, __SWR | __SMBF, 3, 1, 0, 0, 0, 0, 0);
	test_fclose_edge(FN_FCLOSE, 0, 0, __SRD, 3, 0, 1, 1, 0, 0, 0);
	test_fclose_edge(FN_FCLOSE, 0, 0, __SWR, 3, 0, 0, 0, 0, 0, EOF);

	test_fclose_edge(FN_FDCLOSE, 1, 1, __SWR, 3, 0, 0, 0, 0, 0, 0);
	test_fclose_edge(FN_FDCLOSE, 1, 0, __SWR, 3, 0, 0, 0, 0, 0, 0);
	test_fclose_edge(FN_FDCLOSE, 1, 1, __SWR, 3, 0, 0, 0, 1, 0, 0);
	test_fclose_edge(FN_FDCLOSE, 1, 1, __SWR, -1, 0, 0, 0, 0, 0, 0);
	test_fclose_edge(FN_FDCLOSE, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void
run_ftell_edges(void)
{
	test_ftello_u_edge(FN_FTELLO_U, __SRD, 0, 0, 5, 0, 0, 0, 1, 0, 100);
	test_ftello_u_edge(FN_FTELLO_U, __SRD, 0, 0, 5, 3, 0, 0, 0, 0, 100);
	test_ftello_u_edge(FN_FTELLO_U, __SRD, 0, 0, 1, 1, 1, 0, 0, 0, 10);
	test_ftello_u_edge(FN_FTELLO_U, __SWR, 0, 0, 0, 0, 0, 4, 0, 0, 20);
	test_ftello_u_edge(FN_FTELLO_U, __SWR | __SAPP, 0, 0, 0, 0, 0, 3, 0, 0, 30);
	test_ftello_u_edge(FN_FTELLO_U, __SWR, __S2OAP, 0, 0, 0, 0, 2, 0, 0, 40);
	test_ftello_u_edge(FN_FTELLO_U, __SOFF, 0, 77, 0, 0, 0, 0, 0, 0, 0);
	test_ftello_u_edge(FN_FTELLO_U, __SWR, 0, 0, 0, 0, 0, 8, 0, 0,
	    (ref_fpos_t)OFF_MAX);

	test_ftello_edge(FN_FTELLO, __SRD, 0, 0, 2, 0, 0, 0, 0, 50);
	test_ftello_edge(FN_FTELLO, __SRD, 0, 0, 0, 1, 0, 0, 0, -5);

	test_ftell_edge(FN_FTELL, 100);
	test_ftell_edge(FN_FTELL, (ref_fpos_t)LONG_MAX);
	test_ftell_edge(FN_FTELL, (ref_fpos_t)LONG_MAX + 1);
}

static void
rand_swhatbuf(void)
{
	const int fn = FN_SWHATBUF;
	PortFILE pf;
	RefFILE rf;
	size_t psz, rsz;
	int pcould, rcould;
	int pret, rret;
	mode_t modes[] = { S_IFREG, S_IFCHR, S_IFIFO, S_IFBLK };

	for (int i = 0; i < 10000; i++) {
		short file = (short)(rng_i32() % 8 - 2);
		int ok = rng_bit();
		mode_t mode = modes[rng_i32() % 4];
		blksize_t blksize = (blksize_t)(rng_i32() % 8193 - 1);
		int other = rng_bit();

		record_case(fn);
		b0326_reset();
		cfg_fstat(ok, mode, blksize);
		fp_basic_init(&pf, &rf, __SRD, file);
		if (other) {
			pf._seek = (long long (*)(void *, long long,
			    int))other_seek;
			rf._seek = other_seek;
		}
		pret = P::__swhatbuf(&pf, &psz, &pcould);

		b0326_reset();
		cfg_fstat(ok, mode, blksize);
		fp_basic_init(&pf, &rf, __SRD, file);
		if (other) {
			pf._seek = (long long (*)(void *, long long,
			    int))other_seek;
			rf._seek = other_seek;
		}
		rret = ref___swhatbuf(&rf, &rsz, &rcould);

		(void)cmp_swhatbuf(fn, &pf, &rf, pret, rret, psz, rsz,
		    pcould, rcould);
	}
}

static void
rand_smakebuf(void)
{
	const int fn = FN_SMAKEBUF;

	for (int i = 0; i < 10000; i++) {
		short flags = (short)(rng_i32() & 0x7fff);
		if (rng_bit())
			flags |= __SNBF;
		short file = (short)(rng_i32() % 16 - 2);
		int ok = rng_bit();
		mode_t mode = (rng_bit() ? S_IFREG : S_IFCHR);
		blksize_t blksize = (blksize_t)(rng_i32() % 16384);
		int mfail = rng_bit();
		int tty = rng_bit();

		test_smakebuf_edge(fn, flags, file, ok, mode, blksize, mfail,
		    tty);
	}
}

static void
rand_arginfo(void)
{
	for (int i = 0; i < 10000; i++)
		test_arginfo_edge(FN_ARGINFO_TIME, (size_t)(1 + (rng_i32() % 8)));
}

static void
rand_render(void)
{
	for (int i = 0; i < 10000; i++) {
		PrintfInfo pi;
		TimeVals tv;

		memset(&pi, 0, sizeof(pi));
		memset(&tv, 0, sizeof(tv));
		if (rng_i32() % 3 == 0) {
			pi.is_long = 1;
			tv.tv.tv_sec = (time_t)(rng_i32() % 2000000 - 1000);
			tv.tv.tv_usec = rng_i32() % 1000000;
		} else if (rng_i32() % 3 == 1) {
			pi.is_long_double = 1;
			tv.ts.tv_sec = (time_t)(rng_i32() % 2000000);
			tv.ts.tv_nsec = rng_i32() % 1000000000;
		} else {
			tv.tt = (time_t)(rng_i32() % 4000000 - 2000000);
		}
		if (rng_bit())
			pi.alt = 1;
		if (rng_bit())
			pi.prec = rng_i32() % 11 - 1;
		test_render_edge(FN_RENDER_TIME, pi, &tv);
	}
}

static void
rand_fclose(void)
{
	for (int i = 0; i < 10000; i++) {
		int use_fd = rng_bit();
		int fdp_ok = rng_bit();
		short flags = (short)(rng_bit() ? (rng_i32() | __SWR) : 0);
		if (flags == 0 && rng_bit())
			flags = (short)__SRD;
		short file = (short)(rng_i32() % 8 - 2);
		int smb = rng_bit();
		int ub = rng_bit();
		int lb = rng_bit();
		int bad_close = rng_bit();
		int sflush = rng_bit() ? EOF : 0;
		int close = rng_bit() ? EOF : 0;

		test_fclose_edge(use_fd ? FN_FDCLOSE : FN_FCLOSE, use_fd,
		    fdp_ok, flags, file, smb, ub, lb, bad_close, sflush,
		    close);
	}
}

static void
rand_ftell(void)
{
	for (int i = 0; i < 10000; i++) {
		short flags = (short)(rng_i32() & 0x3fff);
		int flags2 = rng_bit() ? __S2OAP : 0;
		ref_fpos_t off = (ref_fpos_t)(rng_i32() % 2000);
		int r = rng_i32() % 16;
		int ur = rng_i32() % 8;
		int poff = rng_i32() % 12;
		int seek_null = rng_bit() && (rng_i32() % 5 == 0);
		int fail = rng_bit();
		ref_fpos_t sret = (ref_fpos_t)(rng_i32() % 100000);

		test_ftello_u_edge(FN_FTELLO_U, flags, flags2, off, r, ur,
		    rng_bit(), poff, seek_null, fail, sret);
		test_ftello_edge(FN_FTELLO, flags, flags2, off, r, ur,
		    rng_bit(), poff, fail, sret);
		test_ftell_edge(FN_FTELL, sret);
	}
}

int
main(void)
{
	rng_seed(0xb0326ULL);

	run_swhatbuf_edges();
	run_smakebuf_edges();
	run_arginfo_edges();
	run_render_edges();
	run_fclose_edges();
	run_ftell_edges();

	rand_swhatbuf();
	rand_smakebuf();
	rand_arginfo();
	rand_render();
	rand_fclose();
	rand_ftell();

	harness_dprintf("\n%-28s %10s %10s\n", "function", "cases", "failures");
	for (int i = 0; i < FN_COUNT; i++)
		harness_dprintf("%-28s %10ld %10ld\n", fn_name[i],
		    fn_cases[i], fn_fail[i]);

	for (int i = 0; i < FN_COUNT; i++) {
		if (fn_fail[i] != 0)
			return (1);
	}
	return (0);
}
