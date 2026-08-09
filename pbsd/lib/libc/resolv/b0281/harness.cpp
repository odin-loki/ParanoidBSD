/*
 * harness.cpp -- differential test for PBSD batch b0281.
 */

import pbsd.lib.libc.resolv.b0281;

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

namespace P = pbsd::lib_libc_resolv::b0281;

using ResState = P::__res_state_layout;
using ResStatePtr = P::res_state;
using MtCtx = P::mtctxres_t;

/*
 * C oracle global _res (oracle.c).  Layout must match P::__res_state_layout.
 */
struct CResLayout {
	int res_h_errno;
	unsigned int options;
	struct {
		struct {
			P::__res_state_ext *ext;
		} _ext;
	} _u;
};

extern "C" {
extern CResLayout _res;
extern int h_errno;
extern const int h_nerr;

extern int mock_thr_main_ret;
extern int mock_thr_once_ret;
extern int mock_thr_keycreate_ret;
extern int mock_thr_setspecific_ret;
extern ResStatePtr mock_thr_getspecific_val;
extern int mock_calloc_fail;
extern int mock_clock_gettime_ret;
extern struct timespec mock_clock_now;
extern int mock_stat_ret;
extern struct stat mock_stat_sb;
extern void *mock_pthread_getspecific_val;
extern int mock_pthread_setspecific_ret;
extern int mock_pthread_key_create_ret;
extern int mock_malloc_fail;
extern int mock_writev_ret;
extern int mock_writev_calls;
extern int mock_writev_last_fd;
extern int mock_writev_last_count;
extern struct iovec mock_writev_last_iov[8];
extern int mock_res_ndestroy_calls;

int *ref___h_errno(void);
void ref___h_errno_set(ResStatePtr, int);
int ref___res_enable_mt(void);
int ref___res_disable_mt(void);
MtCtx *ref___mtctxres(void);
ResStatePtr ref___res_state(void);
void ref_herror(const char *);
const char *ref_hstrerror(int);
}

enum Fn {
	FN_H_ERRNO,
	FN_H_ERRNO_SET,
	FN_RES_ENABLE_MT,
	FN_RES_DISABLE_MT,
	FN_MTCTXRES,
	FN_RES_STATE,
	FN_HERROR,
	FN_HSTRERROR,
	FN_COUNT
};

static const char *fn_name[FN_COUNT] = {
	"__h_errno",
	"__h_errno_set",
	"__res_enable_mt",
	"__res_disable_mt",
	"___mtctxres",
	"__res_state",
	"herror",
	"hstrerror",
};

static unsigned long long fn_cases[FN_COUNT];
static unsigned long long fn_fail[FN_COUNT];
static int fn_reported[FN_COUNT];

static constexpr unsigned long SWEEP_ITERS = 200000UL;
static constexpr unsigned char GUARD = 0x7f;

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

static std::uint64_t rng_state = 0xb0281decafbad26ULL;

static std::uint64_t
rnd64(void)
{
	std::uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static int
rnd_i32(void)
{
	return ((int)(rnd64() & 0x7fffffffu));
}

static void
mock_reset_base(void)
{
	mock_thr_main_ret = 1;
	mock_thr_once_ret = 0;
	mock_thr_keycreate_ret = 0;
	mock_thr_setspecific_ret = 0;
	mock_thr_getspecific_val = NULL;
	mock_calloc_fail = 0;
	mock_clock_gettime_ret = 0;
	mock_clock_now = {0, 0};
	mock_stat_ret = -1;
	memset(&mock_stat_sb, 0, sizeof(mock_stat_sb));
	mock_pthread_getspecific_val = NULL;
	mock_pthread_setspecific_ret = 0;
	mock_pthread_key_create_ret = 0;
	mock_malloc_fail = 0;
	mock_writev_ret = 0;
	mock_writev_calls = 0;
	mock_writev_last_fd = -1;
	mock_writev_last_count = 0;
	mock_res_ndestroy_calls = 0;
	h_errno = 0;
	memset(&_res, 0, sizeof(_res));
}

static void
fill_guard(void *buf, size_t n)
{
	memset(buf, GUARD, n);
}

static bool
bufs_equal(const void *a, const void *b, size_t n)
{
	return (memcmp(a, b, n) == 0);
}

static bool
check_hstrerror_case(int err, const char *ctx)
{
	const char *ref_s, *port_s;

	record_case(FN_HSTRERROR);
	ref_s = ref_hstrerror(err);
	port_s = P::hstrerror(err);
	if (strcmp(ref_s, port_s) != 0) {
		record_fail(FN_HSTRERROR, ctx);
		return (false);
	}
	return (true);
}

static void
test_hstrerror_handwritten(void)
{
	(void)check_hstrerror_case(INT_MIN, "INT_MIN");
	(void)check_hstrerror_case(-1, "neg1");
	(void)check_hstrerror_case(0, "zero");
	(void)check_hstrerror_case(1, "one");
	(void)check_hstrerror_case(2, "two");
	(void)check_hstrerror_case(3, "three");
	(void)check_hstrerror_case(4, "four");
	(void)check_hstrerror_case(5, "five");
	(void)check_hstrerror_case(h_nerr - 1, "h_nerr-1");
	(void)check_hstrerror_case(h_nerr, "h_nerr");
	(void)check_hstrerror_case(INT_MAX, "INT_MAX");
}

static void
test_hstrerror_random(void)
{
	unsigned long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		char ctx[48];
		int err;

		if ((rnd64() & 7u) == 0u)
			err = (int)(rnd64() & 0xffffffffu);
		else
			err = rnd_i32() % 16 - 4;
		snprintf(ctx, sizeof(ctx), "rand#%lu", i);
		(void)check_hstrerror_case(err, ctx);
	}
}

static bool
check_res_enable_mt(void)
{
	int ref_r, port_r;

	record_case(FN_RES_ENABLE_MT);
	ref_r = ref___res_enable_mt();
	port_r = P::__res_enable_mt();
	if (ref_r != port_r) {
		record_fail(FN_RES_ENABLE_MT, "return mismatch");
		return (false);
	}
	return (true);
}

static bool
check_res_disable_mt(void)
{
	int ref_r, port_r;

	record_case(FN_RES_DISABLE_MT);
	ref_r = ref___res_disable_mt();
	port_r = P::__res_disable_mt();
	if (ref_r != port_r) {
		record_fail(FN_RES_DISABLE_MT, "return mismatch");
		return (false);
	}
	return (true);
}

static bool
check_h_errno_ptr(const char *ctx)
{
	int *ref_p, *port_p;
	ResStatePtr ref_rs, port_rs;
	ptrdiff_t ref_off, port_off;

	record_case(FN_H_ERRNO);
	ref_rs = ref___res_state();
	port_rs = P::__res_state();
	ref_p = ref___h_errno();
	port_p = P::__h_errno();
	ref_off = ref_p - &ref_rs->res_h_errno;
	port_off = port_p - &port_rs->res_h_errno;
	if (ref_off != port_off || *ref_p != *port_p) {
		record_fail(FN_H_ERRNO, ctx);
		return (false);
	}
	return (true);
}

static void
test_h_errno_handwritten(void)
{
	mock_reset_base();
	_res.res_h_errno = 0;
	(void)check_h_errno_ptr("init0");
	_res.res_h_errno = 3;
	(void)check_h_errno_ptr("init3");
	_res.res_h_errno = -1;
	(void)check_h_errno_ptr("neg");
	_res.res_h_errno = 127;
	(void)check_h_errno_ptr("127");
}

static void
test_h_errno_random(void)
{
	unsigned long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		char ctx[48];

		mock_reset_base();
		_res.res_h_errno = rnd_i32();
		if ((rnd64() & 3u) == 0u)
			mock_thr_main_ret = 0;
		snprintf(ctx, sizeof(ctx), "rand#%lu", i);
		(void)check_h_errno_ptr(ctx);
	}
}

static bool
check_h_errno_set(ResStatePtr ref_s, ResStatePtr port_s, int err,
    const unsigned char *ref_guard, const unsigned char *port_guard,
    size_t guard_n, const char *ctx)
{
	int ref_h, port_h;

	record_case(FN_H_ERRNO_SET);
	ref___h_errno_set(ref_s, err);
	P::__h_errno_set(port_s, err);
	ref_h = h_errno;
	port_h = h_errno;
	if (ref_s->res_h_errno != err || port_s->res_h_errno != err ||
	    ref_h != err || port_h != err ||
	    !bufs_equal(ref_guard, ref_s, guard_n) ||
	    !bufs_equal(port_guard, port_s, guard_n)) {
		record_fail(FN_H_ERRNO_SET, ctx);
		return (false);
	}
	return (true);
}

static void
test_h_errno_set_handwritten(void)
{
	ResState ref_s, port_s;
	unsigned char ref_guard[sizeof(ResState) + 32];
	unsigned char port_guard[sizeof(ResState) + 32];

	mock_reset_base();
	fill_guard(ref_guard, sizeof(ref_guard));
	fill_guard(port_guard, sizeof(port_guard));
	memcpy(ref_guard + 16, &ref_s, sizeof(ref_s));
	memcpy(port_guard + 16, &port_s, sizeof(port_s));
	(void)check_h_errno_set(&ref_s, &port_s, 0, ref_guard, port_guard,
	    sizeof(ref_guard), "zero");
	(void)check_h_errno_set(&ref_s, &port_s, 4, ref_guard, port_guard,
	    sizeof(ref_guard), "four");
	(void)check_h_errno_set(&ref_s, &port_s, -1, ref_guard, port_guard,
	    sizeof(ref_guard), "neg");
	(void)check_h_errno_set(&ref_s, &port_s, 0x7f, ref_guard, port_guard,
	    sizeof(ref_guard), "0x7f");
	(void)check_h_errno_set(&ref_s, &port_s, 0x80, ref_guard, port_guard,
	    sizeof(ref_guard), "0x80");
}

static void
test_h_errno_set_random(void)
{
	unsigned long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		ResState ref_s, port_s;
		unsigned char ref_guard[sizeof(ResState) + 32];
		unsigned char port_guard[sizeof(ResState) + 32];
		char ctx[48];
		int err;

		mock_reset_base();
		fill_guard(ref_guard, sizeof(ref_guard));
		fill_guard(port_guard, sizeof(port_guard));
		memcpy(ref_guard + 16, &ref_s, sizeof(ref_s));
		memcpy(port_guard + 16, &port_s, sizeof(port_s));
		err = rnd_i32();
		snprintf(ctx, sizeof(ctx), "rand#%lu", i);
		(void)check_h_errno_set(&ref_s, &port_s, err, ref_guard, port_guard,
		    sizeof(ref_guard), ctx);
	}
}

struct WritevSnap {
	int calls;
	int fd;
	int count;
	struct iovec iov[8];
};

static WritevSnap
snap_writev(void)
{
	WritevSnap s;
	int i;

	s.calls = mock_writev_calls;
	s.fd = mock_writev_last_fd;
	s.count = mock_writev_last_count;
	for (i = 0; i < 8; i++)
		s.iov[i] = mock_writev_last_iov[i];
	return (s);
}

static bool
writev_snaps_equal(const WritevSnap &a, const WritevSnap &b)
{
	int i;

	if (a.calls != b.calls || a.fd != b.fd || a.count != b.count)
		return (false);
	for (i = 0; i < 8; i++) {
		if (a.iov[i].iov_len != b.iov[i].iov_len)
			return (false);
		if (a.iov[i].iov_len > 0 &&
		    memcmp(a.iov[i].iov_base, b.iov[i].iov_base,
		    a.iov[i].iov_len) != 0)
			return (false);
	}
	return (true);
}

static bool
check_herror(const char *s, int herr, const char *ctx)
{
	WritevSnap ref_snap, port_snap;

	record_case(FN_HERROR);
	mock_reset_base();
	_res.res_h_errno = herr;
	mock_writev_calls = 0;
	ref_herror(s);
	ref_snap = snap_writev();
	mock_writev_calls = 0;
	P::herror(s);
	port_snap = snap_writev();
	if (!writev_snaps_equal(ref_snap, port_snap)) {
		record_fail(FN_HERROR, ctx);
		return (false);
	}
	return (true);
}

static void
test_herror_handwritten(void)
{
	(void)check_herror(NULL, 0, "null");
	(void)check_herror("", 1, "empty");
	(void)check_herror("x", 2, "one-char");
	(void)check_herror("host", 3, "host");
	(void)check_herror("bad\xffname", 4, "high-bit");
	(void)check_herror("\x80\xff", -1, "neg-herr");
	(void)check_herror("long\x00hidden", 0, "embedded-nul");
}

static void
test_herror_random(void)
{
	unsigned long i;
	char msg[64];

	for (i = 0; i < SWEEP_ITERS; i++) {
		char ctx[48];
		size_t n, j;
		int herr;

		n = (size_t)(rnd64() % 32u);
		for (j = 0; j < n; j++)
			msg[j] = (char)((rnd64() & 0xffu));
		msg[n] = '\0';
		herr = rnd_i32() % 8 - 2;
		snprintf(ctx, sizeof(ctx), "rand#%lu", i);
		if ((rnd64() & 3u) == 0u)
			(void)check_herror(NULL, herr, ctx);
		else
			(void)check_herror(msg, herr, ctx);
	}
}

static bool
same_res_state_semantics(ResStatePtr ref_r, ResStatePtr port_r)
{
	auto global = reinterpret_cast<ResStatePtr>(&_res);

	if ((ref_r == global) != (port_r == global))
		return (false);
	if (ref_r->options != port_r->options ||
	    ref_r->res_h_errno != port_r->res_h_errno)
		return (false);
	return (true);
}

static bool
check_res_state(const char *ctx)
{
	ResStatePtr ref_r, port_r;

	record_case(FN_RES_STATE);
	ref_r = ref___res_state();
	port_r = P::__res_state();
	if (!same_res_state_semantics(ref_r, port_r)) {
		record_fail(FN_RES_STATE, ctx);
		return (false);
	}
	return (true);
}

static void
test_res_state_handwritten(void)
{
	P::__res_state_ext ext;

	mock_reset_base();
	mock_thr_main_ret = 1;
	(void)check_res_state("main-default");

	mock_reset_base();
	mock_thr_main_ret = 1;
	_res.options = 0;
	(void)check_res_state("main-no-init");

	mock_reset_base();
	mock_thr_main_ret = 1;
	_res.options = 1; /* RES_INIT */
	(void)check_res_state("main-init-no-ext");

	mock_reset_base();
	mock_thr_main_ret = 1;
	_res.options = 1;
	ext.reload_period = 0;
	_res._u._ext.ext = &ext;
	(void)check_res_state("main-reload0");

	mock_reset_base();
	mock_thr_main_ret = 1;
	_res.options = 1;
	ext.reload_period = 60;
	ext.conf_stat = 100;
	ext.conf_mtim.tv_sec = 1;
	ext.conf_mtim.tv_nsec = 2;
	_res._u._ext.ext = &ext;
	mock_clock_now.tv_sec = 200;
	mock_clock_now.tv_nsec = 0;
	(void)check_res_state("main-reload-elapsed");

	mock_reset_base();
	mock_thr_main_ret = 1;
	_res.options = 1;
	ext.reload_period = 60;
	ext.conf_stat = 100;
	ext.conf_mtim.tv_sec = 5;
	ext.conf_mtim.tv_nsec = 6;
	_res._u._ext.ext = &ext;
	mock_clock_now.tv_sec = 200;
	mock_stat_ret = 0;
	mock_stat_sb.st_mtim.tv_sec = 99;
	mock_stat_sb.st_mtim.tv_nsec = 99;
	(void)check_res_state("main-stat-mtim-diff");

	mock_reset_base();
	mock_thr_main_ret = 1;
	_res.options = 1;
	ext.reload_period = 60;
	ext.conf_stat = 100;
	ext.conf_mtim.tv_sec = 5;
	ext.conf_mtim.tv_nsec = 6;
	_res._u._ext.ext = &ext;
	mock_clock_now.tv_sec = 200;
	mock_stat_ret = 0;
	mock_stat_sb.st_mtim.tv_sec = 5;
	mock_stat_sb.st_mtim.tv_nsec = 6;
	(void)check_res_state("main-stat-mtim-same");

	mock_reset_base();
	mock_thr_main_ret = 1;
	_res.options = 1;
	ext.reload_period = 60;
	ext.conf_stat = 100;
	_res._u._ext.ext = &ext;
	mock_clock_gettime_ret = -1;
	(void)check_res_state("main-clock-fail");

	mock_reset_base();
	mock_thr_main_ret = 0;
	mock_thr_keycreate_ret = 0;
	(void)check_res_state("worker-key-ok");

	mock_reset_base();
	mock_thr_main_ret = 0;
	mock_thr_once_ret = -1;
	(void)check_res_state("worker-once-fail");

	mock_reset_base();
	mock_thr_main_ret = 0;
	mock_thr_keycreate_ret = -1;
	(void)check_res_state("worker-key-fail");

	mock_reset_base();
	mock_thr_main_ret = 0;
	mock_thr_getspecific_val = reinterpret_cast<ResStatePtr>(&_res);
	(void)check_res_state("worker-tls-hit");

	mock_reset_base();
	mock_thr_main_ret = 0;
	mock_calloc_fail = 1;
	(void)check_res_state("worker-calloc-fail");

	mock_reset_base();
	mock_thr_main_ret = 0;
	mock_thr_setspecific_ret = -1;
	(void)check_res_state("worker-setspecific-fail");
}

static void
test_res_state_random(void)
{
	P::__res_state_ext ext;
	unsigned long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		char ctx[48];

		mock_reset_base();
		if ((rnd64() & 1u) == 0u) {
			mock_thr_main_ret = 1;
			_res.options = (unsigned)(rnd64() & 3u);
			if ((rnd64() & 3u) == 0u) {
				ext.reload_period = (unsigned)(rnd64() % 1000u);
				ext.conf_stat = (time_t)(rnd64() % 10000u);
				ext.conf_mtim.tv_sec = (time_t)(rnd64() % 10000u);
				ext.conf_mtim.tv_nsec = (long)(rnd64() % 1000000u);
				_res._u._ext.ext = &ext;
				mock_clock_now.tv_sec = (time_t)(rnd64() % 20000u);
				mock_clock_gettime_ret =
				    (int)((rnd64() & 7u) == 0u ? -1 : 0);
				if ((rnd64() & 3u) == 0u) {
					mock_stat_ret = 0;
					mock_stat_sb.st_mtim.tv_sec =
					    (time_t)(rnd64() % 10000u);
					mock_stat_sb.st_mtim.tv_nsec =
					    (long)(rnd64() % 1000000u);
				}
			} else if ((rnd64() & 1u) == 0u)
				_res._u._ext.ext = &ext;
		} else {
			mock_thr_main_ret = 0;
			mock_thr_once_ret =
			    (int)((rnd64() & 7u) == 0u ? -1 : 0);
			mock_thr_keycreate_ret =
			    (int)((rnd64() & 7u) == 0u ? -1 : 0);
			mock_thr_getspecific_val =
			    ((rnd64() & 3u) == 0u ?
			    reinterpret_cast<ResStatePtr>(&_res) : NULL);
			mock_calloc_fail = (int)((rnd64() & 15u) == 0u);
			mock_thr_setspecific_ret =
			    (int)((rnd64() & 15u) == 0u ? -1 : 0);
		}
		snprintf(ctx, sizeof(ctx), "rand#%lu", i);
		(void)check_res_state(ctx);
	}
}

static bool
check_mtctxres(const char *ctx)
{
	MtCtx *ref1, *ref2, *port1, *port2;

	record_case(FN_MTCTXRES);
	ref1 = ref___mtctxres();
	port1 = P::___mtctxres();
	ref2 = ref___mtctxres();
	port2 = P::___mtctxres();
	if ((ref1 == ref2) != (port1 == port2)) {
		record_fail(FN_MTCTXRES, ctx);
		return (false);
	}
	return (true);
}

static void
test_mtctxres_handwritten(void)
{
	mock_reset_base();
	mock_pthread_key_create_ret = -1;
	(void)check_mtctxres("key-create-fail");

	mock_reset_base();
	mock_pthread_key_create_ret = 0;
	mock_pthread_getspecific_val = NULL;
	mock_malloc_fail = 0;
	(void)check_mtctxres("alloc-path");

	mock_reset_base();
	mock_pthread_key_create_ret = 0;
	mock_malloc_fail = 1;
	(void)check_mtctxres("malloc-fail");

	mock_reset_base();
	mock_pthread_key_create_ret = 0;
	mock_malloc_fail = 0;
	mock_pthread_setspecific_ret = EAGAIN;
	(void)check_mtctxres("setspecific-fail");

	mock_reset_base();
	mock_pthread_key_create_ret = 0;
	mock_malloc_fail = 0;
	mock_pthread_getspecific_val = (void *)0x1;
	(void)check_mtctxres("existing-ctx");
}

static void
test_mtctxres_random(void)
{
	unsigned long i;

	for (i = 0; i < SWEEP_ITERS; i++) {
		char ctx[48];

		mock_reset_base();
		mock_pthread_key_create_ret =
		    (int)((rnd64() & 7u) == 0u ? EAGAIN : 0);
		mock_malloc_fail = (int)((rnd64() & 15u) == 0u);
		mock_pthread_setspecific_ret =
		    (int)((rnd64() & 15u) == 0u ? EINVAL : 0);
		if ((rnd64() & 3u) == 0u)
			mock_pthread_getspecific_val = (void *)0x1;
		else
			mock_pthread_getspecific_val = NULL;
		snprintf(ctx, sizeof(ctx), "rand#%lu", i);
		(void)check_mtctxres(ctx);
	}
}

int
main(void)
{
	unsigned long long total_fail = 0;
	int f;

	(void)check_res_enable_mt();
	(void)check_res_disable_mt();

	test_mtctxres_handwritten();
	test_mtctxres_random();
	test_hstrerror_handwritten();
	test_hstrerror_random();
	test_h_errno_handwritten();
	test_h_errno_random();
	test_h_errno_set_handwritten();
	test_h_errno_set_random();
	test_herror_handwritten();
	test_herror_random();
	test_res_state_handwritten();
	test_res_state_random();

	for (f = 0; f < FN_COUNT; f++)
		total_fail += fn_fail[f];

	std::printf("\n%-18s %12s %12s\n", "function", "cases", "failures");
	for (f = 0; f < FN_COUNT; f++)
		std::printf("%-18s %12llu %12llu\n", fn_name[f],
		    fn_cases[f], fn_fail[f]);
	std::printf("%-18s %12s %12llu\n", "TOTAL", "",
	    total_fail);

	return (total_fail == 0 ? 0 : 1);
}
