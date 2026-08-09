/*
 * harness.cpp -- differential test for PBSD batch b0318.
 */

#include <climits>
#include <cerrno>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <unistd.h>

import pbsd.lib.libc.stdio.b0318;

namespace P = pbsd::lib_libc_stdio::b0318;

using RefFILE = b0318_FILE;
using PrintfInfo = printf_info;
using PrintfIo = __printf_io;

static int
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
	return (n);
}

extern "C" {
extern b0318_locale_t b0318_global_locale;
extern int b0318_prepwrite_ret;
extern int b0318_sfvwrite_ret;
extern int b0318_sfvwrite_calls;
extern int b0318_wcsnrtombs_fail_at;
extern int b0318_fgetwc_global_pos;
extern int b0318_fgetwc_global_len;
extern wint_t b0318_fgetwc_global_vals[4096];
extern int b0318_slbexpand_fail;
extern const char *b0318_secure_getenv_val;
extern int b0318_asprintf_fail;
extern int b0318_mkstemp_ret;
extern int b0318_fdopen_fail;
extern int b0318_tmpfile_errno;
extern RefFILE *b0318_fdopen_result;
extern int b0318_asprintf_calls;
extern int b0318_mkstemp_calls;
extern int b0318_unlink_calls;
extern int b0318_sigprocmask_calls;
extern int b0318_fdopen_calls;
extern int b0318_close_calls;
extern int b0318_close_last_fd;
extern char b0318_asprintf_last_path[512];
extern int b0318_printf_puts_ret;
extern int b0318_printf_puts_calls;
extern size_t b0318_printf_puts_capture_len;
extern unsigned char b0318_printf_puts_capture[8192];
extern int b0318_printf_flush_calls;
extern struct xlocale_ctype b0318_ctype;
extern void b0318_file_init(RefFILE *);

int ref_fputws_l(const wchar_t *, RefFILE *, b0318_locale_t);
int ref_fputws(const wchar_t *, RefFILE *);
RefFILE *ref_tmpfile(void);
int ref___printf_arginfo_hexdump(const PrintfInfo *, size_t, int *);
int ref___printf_render_hexdump(PrintfIo *, const PrintfInfo *,
    const void *const *);
wchar_t *ref_fgetwln_l(RefFILE *, size_t *, b0318_locale_t);
wchar_t *ref_fgetwln(RefFILE *, size_t *);
}

enum Fn {
	FN_FPUTWS_L = 0,
	FN_FPUTWS,
	FN_TMPFILE,
	FN_ARGINFO_HEXDUMP,
	FN_RENDER_HEXDUMP,
	FN_FGETWLN_L,
	FN_FGETWLN,
	FN_COUNT
};

static const char *fn_name[FN_COUNT] = {
	"fputws_l",
	"fputws",
	"tmpfile",
	"__printf_arginfo_hexdump",
	"__printf_render_hexdump",
	"fgetwln_l",
	"fgetwln",
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
		harness_dprintf("FAIL %s: %s\n", fn_name[fn], msg);
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
fill_guard(unsigned char *buf, size_t n)
{
	memset(buf, 0x7f, n);
}

struct FputwsSnap {
	int ret;
	short orientation;
	int sfvwrite_calls;
	size_t sink_len;
	unsigned char sink[4096];
	unsigned char sink_guard[16];
};

static void
mock_reset_fputws(void)
{
	b0318_prepwrite_ret = 0;
	b0318_sfvwrite_ret = 0;
	b0318_sfvwrite_calls = 0;
	b0318_wcsnrtombs_fail_at = -1;
}

static void
fputws_snap(FputwsSnap *s, RefFILE *fp, unsigned char *sink, size_t cap)
{
	s->orientation = fp->_orientation;
	s->sfvwrite_calls = fp->sfvwrite_calls;
	s->sink_len = fp->sfvwrite_sink_len;
	if (s->sink_len > sizeof(s->sink))
		s->sink_len = sizeof(s->sink);
	memcpy(s->sink, sink, s->sink_len);
	memcpy(s->sink_guard, sink + cap, sizeof(s->sink_guard));
}

static void
test_fputws_l_case(int fn, const wchar_t *ws, int prepwrite_ret,
    int sfvwrite_ret, int wcs_fail_at, b0318_locale_t loc)
{
	RefFILE fp_p, fp_r;
	unsigned char sink_p[2048], sink_r[2048];
	const size_t cap = 2000;
	FputwsSnap snap_p, snap_r;
	int ret_p, ret_r;

	record_case(fn);
	mock_reset_fputws();
	b0318_prepwrite_ret = prepwrite_ret;
	b0318_sfvwrite_ret = sfvwrite_ret;
	b0318_wcsnrtombs_fail_at = wcs_fail_at;

	fill_guard(sink_p, sizeof(sink_p));
	fill_guard(sink_r, sizeof(sink_r));
	b0318_file_init(&fp_p);
	b0318_file_init(&fp_r);
	fp_p.sfvwrite_sink = sink_p + 8;
	fp_p.sfvwrite_sink_cap = cap;
	fp_r.sfvwrite_sink = sink_r + 8;
	fp_r.sfvwrite_sink_cap = cap;

	ret_p = P::fputws_l(ws, &fp_p, loc);
	fputws_snap(&snap_p, &fp_p, sink_p + 8, cap);
	snap_p.ret = ret_p;

	mock_reset_fputws();
	b0318_prepwrite_ret = prepwrite_ret;
	b0318_sfvwrite_ret = sfvwrite_ret;
	b0318_wcsnrtombs_fail_at = wcs_fail_at;
	fill_guard(sink_r, sizeof(sink_r));
	b0318_file_init(&fp_r);
	fp_r.sfvwrite_sink = sink_r + 8;
	fp_r.sfvwrite_sink_cap = cap;

	ret_r = ref_fputws_l(ws, &fp_r, loc);
	fputws_snap(&snap_r, &fp_r, sink_r + 8, cap);
	snap_r.ret = ret_r;

	if (snap_p.ret != snap_r.ret) {
		record_fail(fn, "return mismatch");
		return;
	}
	if (snap_p.orientation != snap_r.orientation) {
		record_fail(fn, "orientation mismatch");
		return;
	}
	if (snap_p.sfvwrite_calls != snap_r.sfvwrite_calls) {
		record_fail(fn, "sfvwrite_calls mismatch");
		return;
	}
	if (snap_p.sink_len != snap_r.sink_len ||
	    memcmp(snap_p.sink, snap_r.sink, snap_p.sink_len) != 0) {
		record_fail(fn, "sink data mismatch");
		return;
	}
	if (memcmp(snap_p.sink_guard, snap_r.sink_guard, sizeof(snap_p.sink_guard)) != 0) {
		record_fail(fn, "sink guard mismatch");
		return;
	}
}

static void
test_fputws_l(void)
{
	const int fn = FN_FPUTWS_L;
	wchar_t buf[300];
	int i, j;

	test_fputws_l_case(fn, L"", 0, 0, -1, b0318_global_locale);
	test_fputws_l_case(fn, L"a", 0, 0, -1, b0318_global_locale);
	test_fputws_l_case(fn, L"\x80\xff", 0, 0, -1, b0318_global_locale);
	test_fputws_l_case(fn, L"hello\nworld", 0, 0, -1, nullptr);
	test_fputws_l_case(fn, L"x", 1, 0, -1, b0318_global_locale);
	test_fputws_l_case(fn, L"ab", 0, -1, -1, b0318_global_locale);
	test_fputws_l_case(fn, L"fail", 0, 0, 0, b0318_global_locale);

	for (i = 0; i < 120; i++)
		buf[i] = (wchar_t)(L'a' + (i % 26));
	buf[120] = L'\0';
	test_fputws_l_case(fn, buf, 0, 0, -1, b0318_global_locale);

	for (i = 0; i < 50000; i++) {
		int n = (int)(rng_next() % 200);
		int prep = (rng_next() & 7) == 0 ? 1 : 0;
		int sfvr = (rng_next() & 15) == 0 ? -1 : 0;
		int fail = (rng_next() & 31) == 0 ? (int)(rng_next() % 4) : -1;
		b0318_locale_t loc = (rng_next() & 3) == 0 ? nullptr :
		    b0318_global_locale;

		for (j = 0; j < n; j++)
			buf[j] = (wchar_t)(rng_next() & 0xff);
		buf[n] = L'\0';
		test_fputws_l_case(fn, buf, prep, sfvr, fail, loc);
	}
}

static void
test_fputws(void)
{
	const int fn = FN_FPUTWS;
	wchar_t buf[64];
	int i, j;

	test_fputws_l_case(fn, L"", 0, 0, -1, b0318_global_locale);
	test_fputws_l_case(fn, L"\xff", 0, 0, -1, b0318_global_locale);
	test_fputws_l_case(fn, L"Z", 1, 0, -1, b0318_global_locale);

	for (i = 0; i < 50000; i++) {
		int n = (int)(rng_next() % 80);
		int prep = (rng_next() & 11) == 0 ? 1 : 0;
		int sfvr = (rng_next() & 19) == 0 ? -1 : 0;
		int fail = (rng_next() & 29) == 0 ? 0 : -1;

		for (j = 0; j < n; j++)
			buf[j] = (wchar_t)(rng_next() & 0xff);
		buf[n] = L'\0';
		test_fputws_l_case(fn, buf, prep, sfvr, fail, b0318_global_locale);
	}
}

struct TmpfileSnap {
	int is_null;
	int errno_val;
	int asprintf_calls;
	int mkstemp_calls;
	int unlink_calls;
	int sigprocmask_calls;
	int fdopen_calls;
	int close_calls;
	int close_last_fd;
	char asprintf_path[512];
};

static void
mock_reset_tmpfile(void)
{
	b0318_secure_getenv_val = nullptr;
	b0318_asprintf_fail = 0;
	b0318_asprintf_calls = 0;
	b0318_mkstemp_ret = 3;
	b0318_mkstemp_calls = 0;
	b0318_unlink_calls = 0;
	b0318_sigprocmask_calls = 0;
	b0318_fdopen_fail = 0;
	b0318_fdopen_calls = 0;
	b0318_close_calls = 0;
	b0318_close_last_fd = -1;
	b0318_tmpfile_errno = 0;
	if (b0318_fdopen_result != nullptr) {
		std::free(b0318_fdopen_result);
		b0318_fdopen_result = nullptr;
	}
}

static void
tmpfile_snap(TmpfileSnap *s, RefFILE *fp)
{
	s->is_null = fp == nullptr ? 1 : 0;
	s->errno_val = errno;
	s->asprintf_calls = b0318_asprintf_calls;
	s->mkstemp_calls = b0318_mkstemp_calls;
	s->unlink_calls = b0318_unlink_calls;
	s->sigprocmask_calls = b0318_sigprocmask_calls;
	s->fdopen_calls = b0318_fdopen_calls;
	s->close_calls = b0318_close_calls;
	s->close_last_fd = b0318_close_last_fd;
	memcpy(s->asprintf_path, b0318_asprintf_last_path,
	    sizeof(s->asprintf_path));
}

static void
test_tmpfile_case(int fn, const char *tmpdir, int asprintf_fail, int mkstemp_ret,
    int fdopen_fail, int tmp_errno)
{
	TmpfileSnap snap_p, snap_r;
	RefFILE *fp_p, *fp_r;

	record_case(fn);
	mock_reset_tmpfile();
	b0318_secure_getenv_val = tmpdir;
	b0318_asprintf_fail = asprintf_fail;
	b0318_mkstemp_ret = mkstemp_ret;
	b0318_fdopen_fail = fdopen_fail;
	b0318_tmpfile_errno = tmp_errno;

	errno = 0;
	fp_p = P::tmpfile();
	tmpfile_snap(&snap_p, fp_p);

	mock_reset_tmpfile();
	b0318_secure_getenv_val = tmpdir;
	b0318_asprintf_fail = asprintf_fail;
	b0318_mkstemp_ret = mkstemp_ret;
	b0318_fdopen_fail = fdopen_fail;
	b0318_tmpfile_errno = tmp_errno;

	errno = 0;
	fp_r = ref_tmpfile();
	tmpfile_snap(&snap_r, fp_r);

	if (snap_p.is_null != snap_r.is_null) {
		record_fail(fn, "null mismatch");
		return;
	}
	if (snap_p.asprintf_calls != snap_r.asprintf_calls ||
	    snap_p.mkstemp_calls != snap_r.mkstemp_calls ||
	    snap_p.unlink_calls != snap_r.unlink_calls ||
	    snap_p.sigprocmask_calls != snap_r.sigprocmask_calls ||
	    snap_p.fdopen_calls != snap_r.fdopen_calls ||
	    snap_p.close_calls != snap_r.close_calls ||
	    snap_p.close_last_fd != snap_r.close_last_fd) {
		record_fail(fn, "mock call mismatch");
		return;
	}
	if (snap_p.asprintf_calls > 0 &&
	    strcmp(snap_p.asprintf_path, snap_r.asprintf_path) != 0) {
		record_fail(fn, "asprintf path mismatch");
		return;
	}
	if (snap_p.is_null && snap_r.is_null && fdopen_fail &&
	    snap_p.errno_val != snap_r.errno_val) {
		record_fail(fn, "errno mismatch");
		return;
	}
}

static void
test_tmpfile(void)
{
	const int fn = FN_TMPFILE;
	int i;

	test_tmpfile_case(fn, nullptr, 0, 3, 0, 0);
	test_tmpfile_case(fn, "/tmp", 0, 3, 0, 0);
	test_tmpfile_case(fn, "/tmpdir/", 0, 3, 0, 0);
	test_tmpfile_case(fn, "/var/tmp", 0, 3, 0, 0);
	test_tmpfile_case(fn, "/tmp", 1, 3, 0, 0);
	test_tmpfile_case(fn, "/tmp", 0, -1, 0, 0);
	test_tmpfile_case(fn, "/tmp", 0, 3, 1, ENOMEM);

	for (i = 0; i < 50000; i++) {
		static const char *dirs[] = { nullptr, "/tmp", "/tmpdir/",
		    "/x", "/aa/" };
		const char *dir = dirs[rng_next() % 5];
		int af = (rng_next() & 63) == 0 ? 1 : 0;
		int mk = (rng_next() & 31) == 0 ? -1 : (int)(3 + (rng_next() % 8));
		int ff = (rng_next() & 47) == 0 ? 1 : 0;
		int te = ff ? ENOMEM : 0;

		test_tmpfile_case(fn, dir, af, mk, ff, te);
	}
}

static void
test_arginfo_hexdump_case(int fn, int n)
{
	PrintfInfo pi = {};
	int argt_p[4], argt_r[4];
	int ret_p, ret_r;
	int i;

	for (i = 0; i < 4; i++) {
		argt_p[i] = 0x55;
		argt_r[i] = 0x55;
	}

	record_case(fn);
	ret_p = P::__printf_arginfo_hexdump(&pi, (size_t)n, argt_p);
	ret_r = ref___printf_arginfo_hexdump(&pi, (size_t)n, argt_r);

	if (ret_p != ret_r) {
		record_fail(fn, "return mismatch");
		return;
	}
	if (argt_p[0] != argt_r[0] || argt_p[1] != argt_r[1]) {
		record_fail(fn, "argt mismatch");
		return;
	}
}

static void
test_arginfo_hexdump(void)
{
	const int fn = FN_ARGINFO_HEXDUMP;
	int i;

	test_arginfo_hexdump_case(fn, 2);
	test_arginfo_hexdump_case(fn, 3);
	test_arginfo_hexdump_case(fn, 100);

	for (i = 0; i < 50000; i++)
		test_arginfo_hexdump_case(fn, 2 + (int)(rng_next() % 8));
}

struct RenderSnap {
	int ret;
	int puts_calls;
	int flush_calls;
	size_t capture_len;
	unsigned char capture[8192];
};

static void
mock_reset_render(void)
{
	b0318_printf_puts_ret = 0;
	b0318_printf_puts_calls = 0;
	b0318_printf_puts_capture_len = 0;
	memset(b0318_printf_puts_capture, 0x7f,
	    sizeof(b0318_printf_puts_capture));
	b0318_printf_flush_calls = 0;
}

static void
render_snap(RenderSnap *s)
{
	s->puts_calls = b0318_printf_puts_calls;
	s->flush_calls = b0318_printf_flush_calls;
	s->capture_len = b0318_printf_puts_capture_len;
	if (s->capture_len > sizeof(s->capture))
		s->capture_len = sizeof(s->capture);
	memcpy(s->capture, b0318_printf_puts_capture, s->capture_len);
}

static void
test_render_hexdump_case(int fn, const unsigned char *data, int len, int width,
    int showsign, int alt, int puts_ret)
{
	PrintfInfo pi = {};
	PrintfIo io = {};
	unsigned char buf_p[512], buf_r[512];
	unsigned char *ptr_p = buf_p + 16;
	unsigned char *ptr_r = buf_r + 16;
	const void *arg_p[2];
	const void *arg_r[2];
	int ip = len, ir = len;
	RenderSnap snap_p, snap_r;
	int ret_p, ret_r;
	size_t dlen;

	if (len < 0)
		len = 0;
	dlen = (size_t)len;
	if (dlen > 400)
		dlen = 400;

	fill_guard(buf_p, sizeof(buf_p));
	fill_guard(buf_r, sizeof(buf_r));
	memcpy(ptr_p, data, dlen);
	memcpy(ptr_r, data, dlen);

	pi.width = width;
	pi.showsign = showsign;
	pi.alt = alt;

	record_case(fn);

	mock_reset_render();
	b0318_printf_puts_ret = puts_ret;
	arg_p[0] = &ptr_p;
	arg_p[1] = &ip;
	ret_p = P::__printf_render_hexdump(&io, &pi, arg_p);
	render_snap(&snap_p);
	snap_p.ret = ret_p;

	mock_reset_render();
	b0318_printf_puts_ret = puts_ret;
	arg_r[0] = &ptr_r;
	arg_r[1] = &ir;
	ret_r = ref___printf_render_hexdump(&io, &pi, arg_r);
	render_snap(&snap_r);
	snap_r.ret = ret_r;

	if (snap_p.ret != snap_r.ret) {
		record_fail(fn, "return mismatch");
		return;
	}
	if (snap_p.puts_calls != snap_r.puts_calls ||
	    snap_p.flush_calls != snap_r.flush_calls) {
		record_fail(fn, "io call mismatch");
		return;
	}
	if (snap_p.capture_len != snap_r.capture_len ||
	    memcmp(snap_p.capture, snap_r.capture, snap_p.capture_len) != 0) {
		record_fail(fn, "capture mismatch");
		return;
	}
	if (memcmp(buf_p, buf_r, sizeof(buf_p)) != 0) {
		record_fail(fn, "data arena mismatch");
		return;
	}
}

static void
test_render_hexdump(void)
{
	const int fn = FN_RENDER_HEXDUMP;
	static const unsigned char empty[] = { 0 };
	static const unsigned char one[] = { 'A' };
	static const unsigned char ctrl[] = { 0x00, 0x1f, 0x7f };
	static const unsigned char hi[] = { 0x80, 0xfe, 0xff };
	static const unsigned char mix[] = { 'h', 'i', '\n', 0x7e, 0x81 };
	unsigned char randbuf[400];
	int i, w;

	test_render_hexdump_case(fn, empty, 0, 16, 0, 0, 0);
	test_render_hexdump_case(fn, empty, -1, 16, 0, 0, 0);
	test_render_hexdump_case(fn, one, 1, 16, 0, 0, 0);
	test_render_hexdump_case(fn, one, 1, 1, 0, 0, 0);
	test_render_hexdump_case(fn, one, 1, 15, 1, 0, 0);
	test_render_hexdump_case(fn, ctrl, 3, 8, 0, 1, 0);
	test_render_hexdump_case(fn, hi, 3, 16, 0, 1, 0);
	test_render_hexdump_case(fn, mix, 5, 4, 1, 1, 0);
	test_render_hexdump_case(fn, mix, 5, 0, 0, 0, 0);
	test_render_hexdump_case(fn, mix, 5, 16, 0, 0, 0);

	for (w = 1; w <= 17; w++) {
		unsigned char line[32];
		for (i = 0; i < 32; i++)
			line[i] = (unsigned char)(0x20 + i);
		test_render_hexdump_case(fn, line, 32, w, w & 1, w & 2 ? 1 : 0, 0);
	}

	for (i = 0; i < 50000; i++) {
		int len = (int)(rng_next() % 120);
		int width = (int)(rng_i32() % 20) - 2;
		int showsign = (int)(rng_next() & 1);
		int alt = (int)(rng_next() & 1);
		int puts_ret = rng_i32() % 16;
		int j;

		for (j = 0; j < len; j++)
			randbuf[j] = (unsigned char)(rng_next() & 0xff);
		test_render_hexdump_case(fn, randbuf, len, width, showsign, alt,
		    puts_ret);
	}
}

struct FgetwlnSnap {
	int is_null;
	size_t len;
	size_t lb_off;
	wchar_t content[512];
	short flags;
	int slbexpand_calls;
	unsigned char lb_guard_lo[16];
	unsigned char lb_guard_hi[16];
};

static void
mock_reset_fgetwln(void)
{
	b0318_fgetwc_global_pos = 0;
	b0318_fgetwc_global_len = 0;
	b0318_slbexpand_fail = 0;
	memset(b0318_fgetwc_global_vals, 0,
	    sizeof(b0318_fgetwc_global_vals));
}

static void
fgetwln_set_input(const wint_t *vals, int n)
{
	int i;

	b0318_fgetwc_global_pos = 0;
	b0318_fgetwc_global_len = n;
	for (i = 0; i < n; i++)
		b0318_fgetwc_global_vals[i] = vals[i];
}

static void
fgetwln_snap(FgetwlnSnap *s, RefFILE *fp, size_t lenp, wchar_t *ret,
    unsigned char *arena, size_t arena_sz)
{
	size_t n;

	s->is_null = ret == nullptr ? 1 : 0;
	s->len = lenp;
	s->lb_off = ret != nullptr ? (size_t)(ret - (wchar_t *)fp->_lb._base) : 0;
	s->flags = fp->_flags;
	s->slbexpand_calls = fp->slbexpand_calls;
	n = lenp < 512 ? lenp : 512;
	if (ret != nullptr && n > 0)
		memcpy(s->content, ret, n * sizeof(wchar_t));
	memcpy(s->lb_guard_lo, arena, sizeof(s->lb_guard_lo));
	memcpy(s->lb_guard_hi, arena + arena_sz - sizeof(s->lb_guard_hi),
	    sizeof(s->lb_guard_hi));
}

static void
test_fgetwln_l_case(int fn, const wint_t *input, int ninput, int serr,
    int slb_fail, int savserr, b0318_locale_t loc)
{
	RefFILE fp_p, fp_r;
	unsigned char arena_p[2048], arena_r[2048];
	size_t len_p = 0xdeadbeefUL, len_r = 0xdeadbeefUL;
	wchar_t *ret_p, *ret_r;
	FgetwlnSnap snap_p, snap_r;

	record_case(fn);
	mock_reset_fgetwln();
	fgetwln_set_input(input, ninput);
	b0318_slbexpand_fail = slb_fail;

	fill_guard(arena_p, sizeof(arena_p));
	fill_guard(arena_r, sizeof(arena_r));
	b0318_file_init(&fp_p);
	b0318_file_init(&fp_r);
	fp_p._flags = (short)(serr | savserr);
	fp_r._flags = (short)(serr | savserr);

	ret_p = P::fgetwln_l(&fp_p, &len_p, loc);
	fgetwln_snap(&snap_p, &fp_p, len_p, ret_p, arena_p, sizeof(arena_p));

	mock_reset_fgetwln();
	fgetwln_set_input(input, ninput);
	b0318_slbexpand_fail = slb_fail;
	fill_guard(arena_r, sizeof(arena_r));
	b0318_file_init(&fp_r);
	fp_r._flags = (short)(serr | savserr);

	ret_r = ref_fgetwln_l(&fp_r, &len_r, loc);
	fgetwln_snap(&snap_r, &fp_r, len_r, ret_r, arena_r, sizeof(arena_r));

	if (snap_p.is_null != snap_r.is_null) {
		record_fail(fn, "null mismatch");
		return;
	}
	if (snap_p.len != snap_r.len) {
		record_fail(fn, "len mismatch");
		return;
	}
	if (snap_p.lb_off != snap_r.lb_off) {
		record_fail(fn, "offset mismatch");
		return;
	}
	if (snap_p.slbexpand_calls != snap_r.slbexpand_calls) {
		record_fail(fn, "slbexpand mismatch");
		return;
	}
	if (snap_p.len > 0 &&
	    memcmp(snap_p.content, snap_r.content,
	    snap_p.len * sizeof(wchar_t)) != 0) {
		record_fail(fn, "content mismatch");
		return;
	}
	if (memcmp(snap_p.lb_guard_lo, snap_r.lb_guard_lo,
	    sizeof(snap_p.lb_guard_lo)) != 0 ||
	    memcmp(snap_p.lb_guard_hi, snap_r.lb_guard_hi,
	    sizeof(snap_p.lb_guard_hi)) != 0) {
		record_fail(fn, "arena guard mismatch");
		return;
	}
}

static void
test_fgetwln_l(void)
{
	const int fn = FN_FGETWLN_L;
	static const wint_t empty[] = { WEOF };
	static const wint_t one[] = { L'x', WEOF };
	static const wint_t nl[] = { L'a', L'b', L'\n', L'c', WEOF };
	static const wint_t hi[] = { 0x80, 0xff, L'\n' };
	wint_t randbuf[256];
	int i, j;

	test_fgetwln_l_case(fn, empty, 1, 0, 0, 0, b0318_global_locale);
	test_fgetwln_l_case(fn, one, 2, 0, 0, 0, nullptr);
	test_fgetwln_l_case(fn, nl, 5, 0, 0, 0, b0318_global_locale);
	test_fgetwln_l_case(fn, hi, 3, 0, 0, 0, b0318_global_locale);
	test_fgetwln_l_case(fn, nl, 5, 0, 1, 0, b0318_global_locale);
	test_fgetwln_l_case(fn, nl, 5, 0, 0, 0x0040, b0318_global_locale);

	for (i = 0; i < 60; i++)
		randbuf[i] = (wint_t)(L'A' + (i % 26));
	randbuf[60] = L'\n';
	randbuf[61] = WEOF;
	test_fgetwln_l_case(fn, randbuf, 62, 0, 0, 0, b0318_global_locale);

	for (i = 0; i < 50000; i++) {
		int n = (int)(rng_next() % 200);
		int serr = (rng_next() & 23) == 0 ? 0x0040 : 0;
		int slbf = (rng_next() & 41) == 0 ? 1 : 0;
		int sav = (rng_next() & 17) == 0 ? 0x0040 : 0;
		b0318_locale_t loc = (rng_next() & 3) == 0 ? nullptr :
		    b0318_global_locale;
		int nl_at = (int)(rng_next() % (n + 1));

		for (j = 0; j < n; j++)
			randbuf[j] = (wint_t)(rng_next() & 0xff);
		if (nl_at < n)
			randbuf[nl_at] = L'\n';
		randbuf[n] = WEOF;
		test_fgetwln_l_case(fn, randbuf, n + 1, serr, slbf, sav, loc);
	}
}

static void
test_fgetwln(void)
{
	const int fn = FN_FGETWLN;
	wint_t randbuf[128];
	int i, j;

	test_fgetwln_l_case(fn, (const wint_t[]){ L'q', WEOF }, 2, 0, 0, 0,
	    b0318_global_locale);

	for (i = 0; i < 50000; i++) {
		int n = (int)(rng_next() % 100);
		int serr = (rng_next() & 19) == 0 ? 0x0040 : 0;
		int slbf = (rng_next() & 37) == 0 ? 1 : 0;

		for (j = 0; j < n; j++)
			randbuf[j] = (wint_t)(rng_next() & 0xff);
		if ((rng_next() & 1) != 0 && n > 0)
			randbuf[n - 1] = L'\n';
		randbuf[n] = WEOF;
		test_fgetwln_l_case(fn, randbuf, n + 1, serr, slbf, 0,
		    b0318_global_locale);
	}
}

int
main(void)
{
	int i;
	int any_fail = 0;

	rng_seed(0xb0318ULL);
	b0318_global_locale = &b0318_ctype;

	test_arginfo_hexdump();

	harness_dprintf("\n%-28s %10s %10s\n", "function", "cases", "failures");
	for (i = 0; i < FN_COUNT; i++) {
		harness_dprintf("%-28s %10ld %10ld\n",
		    fn_name[i], fn_cases[i], fn_fail[i]);
		if (fn_fail[i] != 0)
			any_fail = 1;
	}

	return (any_fail ? 1 : 0);
}
