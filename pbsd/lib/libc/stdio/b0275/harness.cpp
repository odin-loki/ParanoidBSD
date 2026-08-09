/*
 * harness.cpp -- differential test for PBSD batch b0275.
 */

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

import pbsd.lib.libc.stdio.b0275;

namespace P = pbsd::lib_libc_stdio::b0275;

struct RefFILE {
	unsigned char _pbsd_guard_lo[8];
	int _orientation;
	int _pbsd_lockdepth;
	int _pbsd_lockseq;
	unsigned char _pbsd_guard_hi[8];
};

using PrintfInfo = P::printf_info;
using PrintfIo = P::__printf_io;

extern "C" {
extern int __isthreaded;
extern RefFILE mock_stdin_storage;
extern RefFILE *pbsd_stdin;
#define stdin pbsd_stdin

extern int mock_flock_calls;
extern int mock_funlock_calls;

extern int mock_sgetc_calls;
extern int mock_sgetc_vals[4096];
extern int mock_sgetc_len;

extern ssize_t mock_write_ret;
extern int mock_write_calls;
extern int mock_write_last_fd;
extern size_t mock_write_last_n;
extern unsigned char mock_write_last_buf[256];

extern int mock_vsprintf_ret;
extern char *mock_vsprintf_last_str;
extern const char *mock_vsprintf_last_fmt;
extern size_t mock_vsprintf_write_len;
extern unsigned char mock_vsprintf_write_buf[512];

extern int mock_vsprintf_l_ret;
extern char *mock_vsprintf_l_last_str;
extern void *mock_vsprintf_l_last_locale;
extern const char *mock_vsprintf_l_last_fmt;
extern size_t mock_vsprintf_l_write_len;
extern unsigned char mock_vsprintf_l_write_buf[512];

extern void *mock_default_locale;

extern int mock_malloc_fail;

extern int mock_strvisx_ret;
extern char *mock_strvisx_last_dst;
extern const char *mock_strvisx_last_src;
extern unsigned mock_strvisx_last_len;
extern int mock_strvisx_last_flags;
extern int mock_strvisx_calls;

extern int mock_printf_out_ret;
extern int mock_printf_out_calls;
extern PrintfIo *mock_printf_out_last_io;
extern const PrintfInfo *mock_printf_out_last_pi;
extern const char *mock_printf_out_last_buf;
extern int mock_printf_out_last_len;

extern int mock_printf_flush_calls;
extern PrintfIo *mock_printf_flush_last_io;

char *ref___gets_unsafe(char *);
int ref_sprintf(char * __restrict, char const * __restrict, ...);
int ref_sprintf_l(char * __restrict, void *, char const * __restrict, ...);
int ref___printf_arginfo_vis(const PrintfInfo *, size_t, int *);
int ref___printf_render_vis(PrintfIo *, const PrintfInfo *, const void *const *);
}

enum Fn {
	FN_GETS = 0,
	FN_SPRINTF,
	FN_SPRINTF_L,
	FN_ARGINFO_VIS,
	FN_RENDER_VIS,
	FN_COUNT
};

static const char *fn_name[FN_COUNT] = {
	"__gets_unsafe",
	"sprintf",
	"sprintf_l",
	"__printf_arginfo_vis",
	"__printf_render_vis",
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
fill_guard(void *buf, size_t n)
{
	memset(buf, 0x7f, n);
}

struct GetsSnap {
	int orientation;
	int lockdepth;
	int lockseq;
	int flock_calls;
	int funlock_calls;
	int sgetc_calls;
	int write_calls;
	int write_fd;
	size_t write_n;
	unsigned char write_buf[256];
	unsigned char guard_lo[8];
	unsigned char guard_hi[8];
};

static void
mock_reset_gets(void)
{
	int i;

	__isthreaded = 0;
	mock_flock_calls = 0;
	mock_funlock_calls = 0;
	mock_sgetc_calls = 0;
	mock_sgetc_len = 0;
	for (i = 0; i < 4096; i++)
		mock_sgetc_vals[i] = EOF;
	mock_write_ret = (ssize_t)sizeof(
	    "warning: this program uses gets(), which is unsafe.\n") - 1;
	mock_write_calls = 0;
	mock_write_last_fd = -1;
	mock_write_last_n = 0;
	memset(mock_write_last_buf, 0, sizeof(mock_write_last_buf));
	memset(&mock_stdin_storage, 0, sizeof(mock_stdin_storage));
	fill_guard(mock_stdin_storage._pbsd_guard_lo,
	    sizeof(mock_stdin_storage._pbsd_guard_lo));
	fill_guard(mock_stdin_storage._pbsd_guard_hi,
	    sizeof(mock_stdin_storage._pbsd_guard_hi));
}

static void
gets_snap(GetsSnap *s)
{
	s->orientation = mock_stdin_storage._orientation;
	s->lockdepth = mock_stdin_storage._pbsd_lockdepth;
	s->lockseq = mock_stdin_storage._pbsd_lockseq;
	s->flock_calls = mock_flock_calls;
	s->funlock_calls = mock_funlock_calls;
	s->sgetc_calls = mock_sgetc_calls;
	s->write_calls = mock_write_calls;
	s->write_fd = mock_write_last_fd;
	s->write_n = mock_write_last_n;
	memcpy(s->write_buf, mock_write_last_buf, sizeof(s->write_buf));
	memcpy(s->guard_lo, mock_stdin_storage._pbsd_guard_lo, 8);
	memcpy(s->guard_hi, mock_stdin_storage._pbsd_guard_hi, 8);
}

static int
gets_snap_eq(const GetsSnap *a, const GetsSnap *b)
{
	return (memcmp(a, b, sizeof(*a)) == 0);
}

struct BufArena {
	unsigned char before[32];
	char data[512];
	unsigned char after[32];
};

static void
test_gets_case(int fn, const int *seq, int seq_len, int threaded)
{
	BufArena ap, ar;
	GetsSnap snap_p, snap_r;
	char *ret_p, *ret_r;
	ptrdiff_t off_p, off_r;
	int i;

	record_case(fn);

	mock_reset_gets();
	__isthreaded = threaded;
	mock_sgetc_len = seq_len;
	for (i = 0; i < seq_len; i++)
		mock_sgetc_vals[i] = seq[i];

	fill_guard(ap.before, sizeof(ap.before));
	fill_guard(ap.data, sizeof(ap.data));
	fill_guard(ap.after, sizeof(ap.after));
	fill_guard(ar.before, sizeof(ar.before));
	fill_guard(ar.data, sizeof(ar.data));
	fill_guard(ar.after, sizeof(ar.after));

	ret_p = P::__gets_unsafe(ap.data);
	gets_snap(&snap_p);

	mock_reset_gets();
	__isthreaded = threaded;
	mock_sgetc_len = seq_len;
	for (i = 0; i < seq_len; i++)
		mock_sgetc_vals[i] = seq[i];

	ret_r = ref___gets_unsafe(ar.data);
	gets_snap(&snap_r);

	if ((ret_p == nullptr) != (ret_r == nullptr)) {
		record_fail(fn, "null return mismatch");
		return;
	}
	off_p = ret_p ? (ret_p - ap.data) : 0;
	off_r = ret_r ? (ret_r - ar.data) : 0;
	if (off_p != off_r) {
		record_fail(fn, "return offset mismatch");
		return;
	}
	if (memcmp(&ap, &ar, sizeof(ap)) != 0) {
		record_fail(fn, "buffer arena mismatch");
		return;
	}
	if (!gets_snap_eq(&snap_p, &snap_r)) {
		record_fail(fn, "mock/stdin state mismatch");
		return;
	}
	if (snap_p.orientation != -1) {
		record_fail(fn, "stdin not oriented to -1");
		return;
	}
	if (threaded && (snap_p.flock_calls != 1 || snap_p.funlock_calls != 1)) {
		record_fail(fn, "threaded lock count");
		return;
	}
}

static void
test_gets(void)
{
	const int fn = FN_GETS;
	static const int empty_eof[] = { EOF };
	static const int newline[] = { '\n' };
	static const int one_a[] = { 'a', '\n' };
	static const int hi_bytes[] = { 0x80, 0xff, 0xfe, '\n' };
	static const int eof_after_data[] = { 'x', 'y', EOF };
	static const int nul_heavy[] = { 0, 0, 'a', 0, '\n' };
	int i, j, seq[64], n;

	test_gets_case(fn, empty_eof, 1, 0);
	test_gets_case(fn, newline, 1, 0);
	test_gets_case(fn, one_a, 2, 0);
	test_gets_case(fn, hi_bytes, 4, 0);
	test_gets_case(fn, eof_after_data, 3, 0);
	test_gets_case(fn, nul_heavy, 5, 0);
	test_gets_case(fn, one_a, 2, 1);

	for (i = 0; i < 200000; i++) {
		n = (int)(rng_next() % 64);
		for (j = 0; j < n; j++) {
			switch (rng_next() % 5) {
			case 0:
				seq[j] = EOF;
				break;
			case 1:
				seq[j] = '\n';
				break;
			case 2:
				seq[j] = 0;
				break;
			case 3:
				seq[j] = (int)(0x80 + (rng_next() & 0x7f));
				break;
			default:
				seq[j] = (int)(rng_next() & 0xff);
				break;
			}
		}
		test_gets_case(fn, seq, n, (int)(rng_next() & 1));
	}
}

struct SprintfSnap {
	int ret;
	char *last_str;
	const char *last_fmt;
	size_t write_len;
	unsigned char write_buf[512];
};

static void
mock_reset_sprintf(void)
{
	mock_vsprintf_ret = 0;
	mock_vsprintf_last_str = nullptr;
	mock_vsprintf_last_fmt = nullptr;
	mock_vsprintf_write_len = 0;
	memset(mock_vsprintf_write_buf, 0, sizeof(mock_vsprintf_write_buf));
}

static void
sprintf_snap(SprintfSnap *s)
{
	s->ret = mock_vsprintf_ret;
	s->last_str = mock_vsprintf_last_str;
	s->last_fmt = mock_vsprintf_last_fmt;
	s->write_len = mock_vsprintf_write_len;
	memcpy(s->write_buf, mock_vsprintf_write_buf, sizeof(s->write_buf));
}

static void
test_sprintf_case(int fn, int mock_ret, size_t write_len,
    const unsigned char *write_src, const char *fmt)
{
	BufArena ap, ar;
	SprintfSnap snap_p, snap_r;
	int ret_p, ret_r;

	record_case(fn);

	mock_reset_sprintf();
	mock_vsprintf_ret = mock_ret;
	mock_vsprintf_write_len = write_len;
	memcpy(mock_vsprintf_write_buf, write_src, write_len);

	fill_guard(ap.before, sizeof(ap.before));
	fill_guard(ap.data, sizeof(ap.data));
	fill_guard(ap.after, sizeof(ap.after));
	fill_guard(ar.before, sizeof(ar.before));
	fill_guard(ar.data, sizeof(ar.data));
	fill_guard(ar.after, sizeof(ar.after));

	ret_p = P::sprintf(ap.data, fmt, 42);
	sprintf_snap(&snap_p);
	snap_p.ret = ret_p;

	mock_reset_sprintf();
	mock_vsprintf_ret = mock_ret;
	mock_vsprintf_write_len = write_len;
	memcpy(mock_vsprintf_write_buf, write_src, write_len);

	ret_r = ref_sprintf(ar.data, fmt, 42);
	sprintf_snap(&snap_r);
	snap_r.ret = ret_r;

	if (snap_p.ret != snap_r.ret) {
		record_fail(fn, "return mismatch");
		return;
	}
	if (memcmp(&ap, &ar, sizeof(ap)) != 0) {
		record_fail(fn, "buffer arena mismatch");
		return;
	}
	if (snap_p.last_fmt != snap_r.last_fmt) {
		record_fail(fn, "fmt pointer mismatch");
		return;
	}
	if (snap_p.last_str != ap.data || snap_r.last_str != ar.data) {
		record_fail(fn, "str pointer mismatch");
		return;
	}
}

static void
test_sprintf(void)
{
	const int fn = FN_SPRINTF;
	unsigned char pat[256];
	int i, b;

	memset(pat, 0, sizeof(pat));
	test_sprintf_case(fn, 0, 0, pat, "");
	test_sprintf_case(fn, 1, 1, pat, "%c");
	pat[0] = 'a';
	test_sprintf_case(fn, 1, 1, pat, "x");
	for (b = 0; b < 16; b++)
		pat[b] = (unsigned char)(0x80 + b);
	test_sprintf_case(fn, 16, 16, pat, "%s");
	test_sprintf_case(fn, -1, 0, pat, "%d");
	test_sprintf_case(fn, INT_MAX, 8, pat, "%x");

	for (i = 0; i < 200000; i++) {
		int mock_ret = rng_i32();
		size_t write_len = (size_t)(rng_next() % 257);
		const char *fmts[] = { "", "%d", "%s", "\x80" };
		const char *fmt = fmts[rng_next() % 4];
		int j;

		for (j = 0; j < 256; j++)
			pat[j] = (unsigned char)(rng_next() & 0xff);
		test_sprintf_case(fn, mock_ret, write_len, pat, fmt);
	}
}

struct SprintfLSnap {
	int ret;
	char *last_str;
	void *last_locale;
	const char *last_fmt;
};

static void
mock_reset_sprintf_l(void)
{
	mock_vsprintf_l_ret = 0;
	mock_vsprintf_l_last_str = nullptr;
	mock_vsprintf_l_last_locale = nullptr;
	mock_vsprintf_l_last_fmt = nullptr;
	mock_vsprintf_l_write_len = 0;
	memset(mock_vsprintf_l_write_buf, 0, sizeof(mock_vsprintf_l_write_buf));
}

static void
test_sprintf_l_case(int fn, int mock_ret, size_t write_len,
    const unsigned char *write_src, void *locale, const char *fmt)
{
	BufArena ap, ar;
	SprintfLSnap snap_p, snap_r;
	int ret_p, ret_r;
	void *loc_p = locale;
	void *loc_r = locale;

	record_case(fn);

	mock_reset_sprintf_l();
	mock_vsprintf_l_ret = mock_ret;
	mock_vsprintf_l_write_len = write_len;
	memcpy(mock_vsprintf_l_write_buf, write_src, write_len);

	fill_guard(ap.before, sizeof(ap.before));
	fill_guard(ap.data, sizeof(ap.data));
	fill_guard(ap.after, sizeof(ap.after));
	fill_guard(ar.before, sizeof(ar.before));
	fill_guard(ar.data, sizeof(ar.data));
	fill_guard(ar.after, sizeof(ar.after));

	ret_p = P::sprintf_l(ap.data, loc_p, fmt, 7);
	snap_p.ret = ret_p;
	snap_p.last_str = mock_vsprintf_l_last_str;
	snap_p.last_locale = mock_vsprintf_l_last_locale;
	snap_p.last_fmt = mock_vsprintf_l_last_fmt;

	mock_reset_sprintf_l();
	mock_vsprintf_l_ret = mock_ret;
	mock_vsprintf_l_write_len = write_len;
	memcpy(mock_vsprintf_l_write_buf, write_src, write_len);

	ret_r = ref_sprintf_l(ar.data, loc_r, fmt, 7);
	snap_r.ret = ret_r;
	snap_r.last_str = mock_vsprintf_l_last_str;
	snap_r.last_locale = mock_vsprintf_l_last_locale;
	snap_r.last_fmt = mock_vsprintf_l_last_fmt;

	if (snap_p.ret != snap_r.ret) {
		record_fail(fn, "return mismatch");
		return;
	}
	if (memcmp(&ap, &ar, sizeof(ap)) != 0) {
		record_fail(fn, "buffer arena mismatch");
		return;
	}
	if (snap_p.last_fmt != snap_r.last_fmt) {
		record_fail(fn, "fmt mismatch");
		return;
	}
	if (snap_p.last_str != ap.data || snap_r.last_str != ar.data) {
		record_fail(fn, "str mismatch");
		return;
	}
	if (locale == nullptr) {
		if (snap_p.last_locale != mock_default_locale ||
		    snap_r.last_locale != mock_default_locale) {
			record_fail(fn, "FIX_LOCALE not applied");
			return;
		}
	} else if (snap_p.last_locale != snap_r.last_locale) {
		record_fail(fn, "locale mismatch");
		return;
	}
}

static void
test_sprintf_l(void)
{
	const int fn = FN_SPRINTF_L;
	unsigned char pat[256];
	void *loc = (void *)0xdeadbeefUL;
	int i;

	memset(pat, 0, sizeof(pat));
	test_sprintf_l_case(fn, 0, 0, pat, nullptr, "");
	test_sprintf_l_case(fn, 3, 3, pat, loc, "%d");
	pat[0] = 0xff;
	test_sprintf_l_case(fn, 1, 1, pat, loc, "h");
	test_sprintf_l_case(fn, -5, 0, pat, nullptr, "%x");

	for (i = 0; i < 200000; i++) {
		int mock_ret = rng_i32();
		size_t write_len = (size_t)(rng_next() % 257);
		void *locale = (rng_next() & 1) ? nullptr : (void *)(uintptr_t)rng_next();
		const char *fmts[] = { "", "%d", "%s" };
		const char *fmt = fmts[rng_next() % 3];
		int j;

		for (j = 0; j < 256; j++)
			pat[j] = (unsigned char)(rng_next() & 0xff);
		test_sprintf_l_case(fn, mock_ret, write_len, pat, locale, fmt);
	}
}

static void
test_arginfo_vis_case(int fn, int prec, int n)
{
	PrintfInfo pi = {};
	int argt_p[4], argt_r[4];
	int ret_p, ret_r;
	int i;

	for (i = 0; i < 4; i++) {
		argt_p[i] = 0x55;
		argt_r[i] = 0x55;
	}
	pi.prec = prec;

	record_case(fn);

	ret_p = P::__printf_arginfo_vis(&pi, (size_t)n, argt_p);
	ret_r = ref___printf_arginfo_vis(&pi, (size_t)n, argt_r);

	if (ret_p != ret_r) {
		record_fail(fn, "return mismatch");
		return;
	}
	if (argt_p[0] != argt_r[0]) {
		record_fail(fn, "argt[0] mismatch");
		return;
	}
	if (ret_p != 1 || argt_p[0] != 5) {
		record_fail(fn, "unexpected arginfo result");
		return;
	}
}

static void
test_arginfo_vis(void)
{
	const int fn = FN_ARGINFO_VIS;
	int i;

	test_arginfo_vis_case(fn, -1, 1);
	test_arginfo_vis_case(fn, 0, 1);
	test_arginfo_vis_case(fn, INT_MAX, 1);
	test_arginfo_vis_case(fn, INT_MIN, 2);

	for (i = 0; i < 200000; i++)
		test_arginfo_vis_case(fn, rng_i32(), 1 + (int)(rng_next() % 4));
}

struct RenderSnap {
	int ret;
	int strvisx_calls;
	int strvisx_flags;
	unsigned strvisx_len;
	int printf_out_calls;
	int printf_out_len;
	int printf_flush_calls;
};

static void
mock_reset_render(void)
{
	mock_malloc_fail = 0;
	mock_strvisx_ret = 0;
	mock_strvisx_calls = 0;
	mock_strvisx_last_flags = 0;
	mock_strvisx_last_len = 0;
	mock_printf_out_ret = 0;
	mock_printf_out_calls = 0;
	mock_printf_out_last_len = 0;
	mock_printf_flush_calls = 0;
}

static void
render_snap(RenderSnap *s)
{
	s->ret = 0;
	s->strvisx_calls = mock_strvisx_calls;
	s->strvisx_flags = mock_strvisx_last_flags;
	s->strvisx_len = mock_strvisx_last_len;
	s->printf_out_calls = mock_printf_out_calls;
	s->printf_out_len = mock_printf_out_last_len;
	s->printf_flush_calls = mock_printf_flush_calls;
}

static void
test_render_vis_case(int fn, const char *str, int prec, int showsign, int pad,
    int alt, int vis_ret, int out_ret, int malloc_fail)
{
	PrintfInfo pi = {};
	PrintfIo io = {};
	const void *arg_p[1];
	const void *arg_r[1];
	RenderSnap snap_p, snap_r;
	int ret_p, ret_r;

	pi.prec = prec;
	pi.showsign = showsign;
	pi.pad = pad;
	pi.alt = alt;

	record_case(fn);

	mock_reset_render();
	mock_malloc_fail = malloc_fail;
	mock_strvisx_ret = vis_ret;
	mock_printf_out_ret = out_ret;
	arg_p[0] = &str;
	ret_p = P::__printf_render_vis(&io, &pi, arg_p);
	render_snap(&snap_p);
	snap_p.ret = ret_p;

	mock_reset_render();
	mock_malloc_fail = malloc_fail;
	mock_strvisx_ret = vis_ret;
	mock_printf_out_ret = out_ret;
	arg_r[0] = &str;
	ret_r = ref___printf_render_vis(&io, &pi, arg_r);
	render_snap(&snap_r);
	snap_r.ret = ret_r;

	if (snap_p.ret != snap_r.ret) {
		record_fail(fn, "return mismatch");
		return;
	}
	if (snap_p.strvisx_calls != snap_r.strvisx_calls ||
	    snap_p.strvisx_flags != snap_r.strvisx_flags ||
	    snap_p.strvisx_len != snap_r.strvisx_len ||
	    snap_p.printf_out_calls != snap_r.printf_out_calls ||
	    snap_p.printf_out_len != snap_r.printf_out_len ||
	    snap_p.printf_flush_calls != snap_r.printf_flush_calls) {
		record_fail(fn, "mock state mismatch");
		return;
	}
}

static void
test_render_vis_null(int fn)
{
	PrintfInfo pi = {};
	PrintfIo io = {};
	const void *arg_p[1];
	const void *arg_r[1];
	char *nullp = nullptr;
	int ret_p, ret_r;

	record_case(fn);

	mock_reset_render();
	mock_printf_out_ret = 6;
	arg_p[0] = &nullp;
	ret_p = P::__printf_render_vis(&io, &pi, arg_p);

	mock_reset_render();
	mock_printf_out_ret = 6;
	arg_r[0] = &nullp;
	ret_r = ref___printf_render_vis(&io, &pi, arg_r);

	if (ret_p != ret_r) {
		record_fail(fn, "null return mismatch");
		return;
	}

	if (ret_p != ret_r) {
		record_fail(fn, "null return mismatch");
		return;
	}
	if (mock_printf_out_calls != 1 || mock_printf_out_last_len != 6) {
		record_fail(fn, "null path out args");
		return;
	}
}

static void
test_render_vis(void)
{
	const int fn = FN_RENDER_VIS;
	static const char empty[] = "";
	static const char one[] = "a";
	static const char hi[] = "\x80\xff\xfe";
	char randbuf[128];
	int i;

	test_render_vis_null(fn);

	test_render_vis_case(fn, empty, -1, 0, ' ', 0, 0, 0, 0);
	test_render_vis_case(fn, one, -1, 1, ' ', 0, 3, 2, 0);
	test_render_vis_case(fn, one, -1, 0, '0', 0, 2, 4, 0);
	test_render_vis_case(fn, hi, -1, 0, ' ', 1, 5, 1, 0);
	test_render_vis_case(fn, hi, 2, 0, 'x', 0, 2, 3, 0);
	test_render_vis_case(fn, one, 0, 0, ' ', 0, 0, 0, 0);
	test_render_vis_case(fn, one, -1, 0, ' ', 0, 0, 0, 1);

	for (i = 0; i < 200000; i++) {
		int slen = (int)(rng_next() % 32);
		int prec = (int)(rng_i32() % 40) - 5;
		int showsign = (int)(rng_next() & 1);
		int pad = (rng_next() & 1) ? '0' : ' ';
		int alt = (int)(rng_next() & 1);
		int vis_ret = rng_i32() % 64;
		int out_ret = rng_i32() % 64;
		int j;

		for (j = 0; j < slen; j++)
			randbuf[j] = (char)(rng_next() & 0xff);
		randbuf[slen] = '\0';
		test_render_vis_case(fn, randbuf, prec, showsign, pad, alt,
		    vis_ret, out_ret, 0);
	}
}

int
main(void)
{
	int i;
	int any_fail = 0;

	rng_seed(0xb0275ULL);

	test_gets();
	test_sprintf();
	test_sprintf_l();
	test_arginfo_vis();
	test_render_vis();

	dprintf(STDERR_FILENO, "\n%-22s %10s %10s\n", "function", "cases", "failures");
	for (i = 0; i < FN_COUNT; i++) {
		dprintf(STDERR_FILENO, "%-22s %10ld %10ld\n",
		    fn_name[i], fn_cases[i], fn_fail[i]);
		if (fn_fail[i] != 0)
			any_fail = 1;
	}

	return (any_fail ? 1 : 0);
}
