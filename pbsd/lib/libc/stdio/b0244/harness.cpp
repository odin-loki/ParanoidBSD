/*
 * harness.cpp -- differential test for PBSD batch b0244.
 *
 * Every ported function is exercised with hand-written edge cases and a
 * fixed-seed randomised sweep of at least 200000 iterations.  For each case
 * both the C++ port and the ref_ oracle are called and everything observable
 * is compared: return values, mock-call side effects, FILE orientation
 * state, and the entire guard-filled buffer for functions that write through
 * caller pointers.
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>

import pbsd.lib.libc.stdio.b0244;

namespace P = pbsd::lib_libc_stdio::b0244;

/* ------------------------------------------------------------------ */
/* Mirror of oracle.c layouts and mock state.                          */
/* ------------------------------------------------------------------ */

struct RefFILE {
	unsigned char _pbsd_guard_lo[8];
	int _orientation;
	int _pbsd_lockdepth;
	int _pbsd_lockseq;
	unsigned char _pbsd_guard_hi[8];
};

extern "C" {
extern int __isthreaded;

extern int mock_fwalk_calls;
extern void *mock_fwalk_fn;
extern int mock_fclose_calls;

extern ssize_t mock_getdelim_ret;
extern int mock_getdelim_last_delim;
extern char **mock_getdelim_last_linep;
extern size_t *mock_getdelim_last_linecapp;
extern RefFILE *mock_getdelim_last_fp;
extern size_t mock_getdelim_set_cap;
extern size_t mock_getdelim_write_len;
extern unsigned char mock_getdelim_write_buf[256];

extern int mock_vdprintf_ret;
extern int mock_vdprintf_last_fd;
extern const char *mock_vdprintf_last_fmt;
extern int mock_vdprintf_last_arg;

extern int mock_flock_calls;
extern int mock_funlock_calls;

void ref___fcloseall(void);
ssize_t ref_getline(char **, size_t *, RefFILE *);
int ref_dprintf(int, const char *, ...);
int ref_fwide(RefFILE *, int);
}

/* ------------------------------------------------------------------ */
/* Bookkeeping                                                         */
/* ------------------------------------------------------------------ */

enum Fn {
	FN_FCLOSEALL = 0,
	FN_GETLINE,
	FN_DPRINTF,
	FN_FWIDE,
	FN_COUNT
};

static const char *fn_name[FN_COUNT] = {
	"__fcloseall",
	"getline",
	"dprintf",
	"fwide",
};

static long fn_cases[FN_COUNT];
static long fn_fail[FN_COUNT];
static int fn_reported[FN_COUNT];

static void
record_fail(int fn, const char *msg)
{
	fn_fail[fn]++;
	if (fn_reported[fn] < 8) {
		fn_reported[fn]++;
		dprintf(STDERR_FILENO, "FAIL %s: %s\n", fn_name[fn], msg);
	}
}

static void
record_case(int fn)
{
	fn_cases[fn]++;
}

/* splitmix64 */
static uint64_t rng_state;

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

static void
mock_reset(void)
{
	__isthreaded = 0;
	mock_fwalk_calls = 0;
	mock_fwalk_fn = nullptr;
	mock_fclose_calls = 0;
	mock_getdelim_ret = 0;
	mock_getdelim_last_delim = 0;
	mock_getdelim_last_linep = nullptr;
	mock_getdelim_last_linecapp = nullptr;
	mock_getdelim_last_fp = nullptr;
	mock_getdelim_set_cap = 0;
	mock_getdelim_write_len = 0;
	memset(mock_getdelim_write_buf, 0, sizeof(mock_getdelim_write_buf));
	mock_vdprintf_ret = 0;
	mock_vdprintf_last_fd = -1;
	mock_vdprintf_last_fmt = nullptr;
	mock_vdprintf_last_arg = 0;
	mock_flock_calls = 0;
	mock_funlock_calls = 0;
}

struct MockSnap {
	int fwalk_calls;
	void *fwalk_fn;
	int getdelim_delim;
	char **getdelim_linep;
	size_t *getdelim_linecapp;
	RefFILE *getdelim_fp;
	int vdprintf_fd;
	const char *vdprintf_fmt;
	int vdprintf_arg;
	int flock_calls;
	int funlock_calls;
};

static void
mock_snap(MockSnap *s)
{
	s->fwalk_calls = mock_fwalk_calls;
	s->fwalk_fn = mock_fwalk_fn;
	s->getdelim_delim = mock_getdelim_last_delim;
	s->getdelim_linep = mock_getdelim_last_linep;
	s->getdelim_linecapp = mock_getdelim_last_linecapp;
	s->getdelim_fp = mock_getdelim_last_fp;
	s->vdprintf_fd = mock_vdprintf_last_fd;
	s->vdprintf_fmt = mock_vdprintf_last_fmt;
	s->vdprintf_arg = mock_vdprintf_last_arg;
	s->flock_calls = mock_flock_calls;
	s->funlock_calls = mock_funlock_calls;
}

static int
mock_eq(const MockSnap *a, const MockSnap *b)
{
	return (a->fwalk_calls == b->fwalk_calls &&
	    a->fwalk_fn == b->fwalk_fn &&
	    a->getdelim_delim == b->getdelim_delim &&
	    a->getdelim_linep == b->getdelim_linep &&
	    a->getdelim_linecapp == b->getdelim_linecapp &&
	    a->getdelim_fp == b->getdelim_fp &&
	    a->vdprintf_fd == b->vdprintf_fd &&
	    a->vdprintf_fmt == b->vdprintf_fmt &&
	    a->vdprintf_arg == b->vdprintf_arg &&
	    a->flock_calls == b->flock_calls &&
	    a->funlock_calls == b->funlock_calls);
}

static void
fill_guard(void *buf, size_t n)
{
	memset(buf, 0x7f, n);
}

/* ------------------------------------------------------------------ */
/* __fcloseall                                                         */
/* ------------------------------------------------------------------ */

static void
test_fcloseall_once(int fn)
{
	MockSnap snap_p, snap_r;

	record_case(fn);

	mock_reset();
	P::__fcloseall();
	mock_snap(&snap_p);

	mock_reset();
	ref___fcloseall();
	mock_snap(&snap_r);

	if (snap_p.fwalk_calls != 1) {
		record_fail(fn, "port: fwalk not called once");
	}
	if (snap_r.fwalk_calls != 1) {
		record_fail(fn, "ref: fwalk not called once");
	}
	if (snap_p.fwalk_fn != snap_r.fwalk_fn) {
		record_fail(fn, "fwalk fn pointer mismatch");
	}
	if (!mock_eq(&snap_p, &snap_r)) {
		record_fail(fn, "mock state mismatch");
	}
}

static void
test_fcloseall(void)
{
	const int fn = FN_FCLOSEALL;
	int i;

	for (i = 0; i < 200000; i++)
		test_fcloseall_once(fn);
}

/* ------------------------------------------------------------------ */
/* getline                                                             */
/* ------------------------------------------------------------------ */

struct LineArena {
	unsigned char before[32];
	char line[256];
	unsigned char after[32];
};

struct CapArena {
	unsigned char before[32];
	size_t cap;
	unsigned char after[32];
};

static void
test_getline_case(int fn, ssize_t mock_ret, size_t set_cap, size_t write_len,
    const unsigned char *write_src, LineArena *la_p, LineArena *la_r,
    CapArena *ca_p, CapArena *ca_r, P::FILE *fp_p, RefFILE *fp_r,
    char **linep_p, char **linep_r, int use_null_linep, int use_null_cap)
{
	MockSnap snap_p, snap_r;
	ssize_t ret_p, ret_r;
	size_t wi;

	record_case(fn);

	mock_reset();
	mock_getdelim_ret = mock_ret;
	mock_getdelim_set_cap = set_cap;
	mock_getdelim_write_len = write_len;
	for (wi = 0; wi < write_len && wi < sizeof(mock_getdelim_write_buf); wi++)
		mock_getdelim_write_buf[wi] = write_src[wi];

	fill_guard(la_p->before, sizeof(la_p->before));
	fill_guard(la_p->line, sizeof(la_p->line));
	fill_guard(la_p->after, sizeof(la_p->after));
	fill_guard(ca_p->before, sizeof(ca_p->before));
	fill_guard(&ca_p->cap, sizeof(ca_p->cap));
	fill_guard(ca_p->after, sizeof(ca_p->after));
	fill_guard(la_r->before, sizeof(la_r->before));
	fill_guard(la_r->line, sizeof(la_r->line));
	fill_guard(la_r->after, sizeof(la_r->after));
	fill_guard(ca_r->before, sizeof(ca_r->before));
	fill_guard(&ca_r->cap, sizeof(ca_r->cap));
	fill_guard(ca_r->after, sizeof(ca_r->after));

	*linep_p = use_null_linep ? nullptr : la_p->line;
	*linep_r = use_null_linep ? nullptr : la_r->line;
	ca_p->cap = 0xdeadbeefUL;
	ca_r->cap = 0xdeadbeefUL;

	ret_p = P::getline(use_null_linep ? nullptr : linep_p,
	    use_null_cap ? nullptr : &ca_p->cap, fp_p);
	mock_snap(&snap_p);

	mock_reset();
	mock_getdelim_ret = mock_ret;
	mock_getdelim_set_cap = set_cap;
	mock_getdelim_write_len = write_len;
	for (wi = 0; wi < write_len && wi < sizeof(mock_getdelim_write_buf); wi++)
		mock_getdelim_write_buf[wi] = write_src[wi];

	fill_guard(la_r->before, sizeof(la_r->before));
	fill_guard(la_r->line, sizeof(la_r->line));
	fill_guard(la_r->after, sizeof(la_r->after));
	fill_guard(ca_r->before, sizeof(ca_r->before));
	fill_guard(&ca_r->cap, sizeof(ca_r->cap));
	fill_guard(ca_r->after, sizeof(ca_r->after));

	*linep_r = use_null_linep ? nullptr : la_r->line;
	ca_r->cap = 0xdeadbeefUL;

	ret_r = ref_getline(use_null_linep ? nullptr : linep_r,
	    use_null_cap ? nullptr : &ca_r->cap, fp_r);
	mock_snap(&snap_r);

	if (ret_p != ret_r) {
		record_fail(fn, "return mismatch");
		return;
	}
	if (snap_p.getdelim_delim != '\n' || snap_r.getdelim_delim != '\n') {
		record_fail(fn, "delim is not newline");
		return;
	}
	if (!mock_eq(&snap_p, &snap_r)) {
		record_fail(fn, "mock state mismatch");
		return;
	}
	if (!use_null_cap) {
		if (ca_p->cap != ca_r->cap) {
			record_fail(fn, "linecapp value mismatch");
			return;
		}
		if (memcmp(ca_p, ca_r, sizeof(*ca_p)) != 0) {
			record_fail(fn, "linecapp arena mismatch");
			return;
		}
	}
	if (!use_null_linep) {
		if (memcmp(la_p, la_r, sizeof(*la_p)) != 0) {
			record_fail(fn, "line buffer arena mismatch");
			return;
		}
	}
}

static void
test_getline(void)
{
	const int fn = FN_GETLINE;
	LineArena la_p, la_r;
	CapArena ca_p, ca_r;
	char *linep_p = la_p.line;
	char *linep_r = la_r.line;
	P::FILE fp_p = {};
	RefFILE fp_r = {};
	unsigned char pat[256];
	int i, b;

	/* empty write, zero return */
	test_getline_case(fn, 0, 0, 0, pat, &la_p, &la_r, &ca_p, &ca_r,
	    &fp_p, &fp_r, &linep_p, &linep_r, 0, 0);

	/* single char */
	pat[0] = 'a';
	test_getline_case(fn, 1, 16, 1, pat, &la_p, &la_r, &ca_p, &ca_r,
	    &fp_p, &fp_r, &linep_p, &linep_r, 0, 0);

	/* NUL-heavy */
	memset(pat, 0, 8);
	test_getline_case(fn, 8, 32, 8, pat, &la_p, &la_r, &ca_p, &ca_r,
	    &fp_p, &fp_r, &linep_p, &linep_r, 0, 0);

	/* high-bit bytes */
	for (b = 0; b < 16; b++)
		pat[b] = (unsigned char)(0x80 + b);
	test_getline_case(fn, 16, 64, 16, pat, &la_p, &la_r, &ca_p, &ca_r,
	    &fp_p, &fp_r, &linep_p, &linep_r, 0, 0);

	/* boundary: -1 return (EOF) */
	test_getline_case(fn, -1, 0, 0, pat, &la_p, &la_r, &ca_p, &ca_r,
	    &fp_p, &fp_r, &linep_p, &linep_r, 0, 0);

	/* null linep */
	test_getline_case(fn, 5, 8, 4, pat, &la_p, &la_r, &ca_p, &ca_r,
	    &fp_p, &fp_r, &linep_p, &linep_r, 1, 0);

	/* null linecapp */
	test_getline_case(fn, 3, 0, 2, pat, &la_p, &la_r, &ca_p, &ca_r,
	    &fp_p, &fp_r, &linep_p, &linep_r, 0, 1);

	/* both null */
	test_getline_case(fn, 2, 0, 0, pat, &la_p, &la_r, &ca_p, &ca_r,
	    &fp_p, &fp_r, &linep_p, &linep_r, 1, 1);

	for (i = 0; i < 200000; i++) {
		ssize_t mock_ret = (ssize_t)(rng_i32() % 257) - 1;
		size_t set_cap = (size_t)(rng_next() & 0xffffUL);
		size_t write_len = (size_t)(rng_next() % 257);
		int j;

		for (j = 0; j < 256; j++)
			pat[j] = (unsigned char)(rng_next() & 0xff);
		test_getline_case(fn, mock_ret, set_cap, write_len, pat,
		    &la_p, &la_r, &ca_p, &ca_r, &fp_p, &fp_r, &linep_p,
		    &linep_r, (int)(rng_next() & 1), (int)(rng_next() & 1));
	}
}

/* ------------------------------------------------------------------ */
/* dprintf                                                             */
/* ------------------------------------------------------------------ */

static void
test_dprintf_case(int fn, int fd, const char *fmt, int arg)
{
	MockSnap snap_p, snap_r;
	int ret_p, ret_r;

	record_case(fn);

	mock_reset();
	mock_vdprintf_ret = arg + fd;
	ret_p = P::dprintf(fd, fmt, arg);
	mock_snap(&snap_p);

	mock_reset();
	mock_vdprintf_ret = arg + fd;
	ret_r = ref_dprintf(fd, fmt, arg);
	mock_snap(&snap_r);

	if (ret_p != ret_r) {
		record_fail(fn, "return mismatch");
		return;
	}
	if (!mock_eq(&snap_p, &snap_r)) {
		record_fail(fn, "mock state mismatch");
		return;
	}
	if (snap_p.vdprintf_fd != fd || snap_p.vdprintf_fmt != fmt ||
	    snap_p.vdprintf_arg != arg) {
		record_fail(fn, "port vdprintf args wrong");
	}
}

static void
test_dprintf(void)
{
	const int fn = FN_DPRINTF;
	static const char fmt_empty[] = "";
	static const char fmt_one[] = "%d";
	static const char fmt_hi[] = "\x80\xff";
	int i;

	test_dprintf_case(fn, 0, fmt_empty, 0);
	test_dprintf_case(fn, 1, fmt_one, 1);
	test_dprintf_case(fn, -1, fmt_one, -1);
	test_dprintf_case(fn, 255, fmt_hi, 0x7f);
	test_dprintf_case(fn, INT_MAX, fmt_one, INT_MIN);
	test_dprintf_case(fn, 0, fmt_one, 0);
	test_dprintf_case(fn, 0, fmt_one, 1);

	for (i = 0; i < 200000; i++) {
		int fd = rng_i32();
		int arg = rng_i32();
		const char *fmts[] = { "", "%d", "%x", "x\x80" };
		const char *fmt = fmts[rng_next() % 4];
		test_dprintf_case(fn, fd, fmt, arg);
	}
}

/* ------------------------------------------------------------------ */
/* fwide                                                               */
/* ------------------------------------------------------------------ */

static void
test_fwide_case(int fn, int init_orient, int mode, int threaded)
{
	P::FILE fp_p = {};
	RefFILE fp_r = {};
	MockSnap snap_p, snap_r;
	int ret_p, ret_r;

	record_case(fn);

	fill_guard(fp_p._pbsd_guard_lo, sizeof(fp_p._pbsd_guard_lo));
	fill_guard(fp_p._pbsd_guard_hi, sizeof(fp_p._pbsd_guard_hi));
	fill_guard(fp_r._pbsd_guard_lo, sizeof(fp_r._pbsd_guard_lo));
	fill_guard(fp_r._pbsd_guard_hi, sizeof(fp_r._pbsd_guard_hi));

	fp_p._orientation = init_orient;
	fp_p._pbsd_lockdepth = 0;
	fp_p._pbsd_lockseq = 0;
	fp_r._orientation = init_orient;
	fp_r._pbsd_lockdepth = 0;
	fp_r._pbsd_lockseq = 0;

	mock_reset();
	__isthreaded = threaded;
	ret_p = P::fwide(&fp_p, mode);
	mock_snap(&snap_p);

	mock_reset();
	__isthreaded = threaded;
	ret_r = ref_fwide(&fp_r, mode);
	mock_snap(&snap_r);

	if (ret_p != ret_r) {
		record_fail(fn, "return mismatch");
		return;
	}
	if (memcmp(&fp_p, &fp_r, sizeof(fp_p)) != 0) {
		record_fail(fn, "FILE state mismatch");
		return;
	}
	if (!mock_eq(&snap_p, &snap_r)) {
		record_fail(fn, "mock state mismatch");
		return;
	}
	if (threaded && (snap_p.flock_calls != 1 || snap_p.funlock_calls != 1)) {
		record_fail(fn, "threaded lock/unlock count");
	}
	if (!threaded && (snap_p.flock_calls != 0 || snap_p.funlock_calls != 0)) {
		record_fail(fn, "non-threaded lock/unlock count");
	}
}

static void
test_fwide(void)
{
	const int fn = FN_FWIDE;
	static const int inits[] = { 0, 1, -1, 2, -2 };
	static const int modes[] = { 0, 1, -1, 2, -2, INT_MAX, INT_MIN };
	int i, a, b;

	for (a = 0; a < 5; a++) {
		for (b = 0; b < 7; b++) {
			test_fwide_case(fn, inits[a], modes[b], 0);
			test_fwide_case(fn, inits[a], modes[b], 1);
		}
	}

	for (i = 0; i < 200000; i++) {
		int init = (int)(rng_next() % 5) - 2;
		int mode = (int)(rng_i32() % 7) - 3;
		int threaded = (int)(rng_next() & 1);
		test_fwide_case(fn, init, mode, threaded);
	}
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int
main(void)
{
	int i;
	int any_fail = 0;

	rng_seed(0xb0244ULL);

	test_fcloseall();
	test_getline();
	test_dprintf();
	test_fwide();

	dprintf(STDERR_FILENO, "\n%-14s %10s %10s\n", "function", "cases", "failures");
	for (i = 0; i < FN_COUNT; i++) {
		dprintf(STDERR_FILENO, "%-14s %10ld %10ld\n",
		    fn_name[i], fn_cases[i], fn_fail[i]);
		if (fn_fail[i] != 0)
			any_fail = 1;
	}

	return (any_fail ? 1 : 0);
}
