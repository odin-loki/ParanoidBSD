/*
 * harness.cpp -- differential test for PBSD batch b0299.
 *
 * lockf is a libc fcntl interposition wrapper; brk, _brk and sbrk are
 * __sys_break wrappers with file-static break state.  The harness installs
 * instrumented mocks for the fcntl interposition slot and for __sys_break,
 * then compares mock-visible dispatch tags, every forwarded argument, flock
 * content hashes, the entire guard-filled buffers (including padding), errno
 * after failures, and return values.  Break-related tests run as one shared
 * deterministic sequence on both sides so paired static state stays aligned.
 */

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef F_ULOCK
#define	F_ULOCK		0
#define	F_LOCK		1
#define	F_TLOCK		2
#define	F_TEST		3
#endif

#ifndef F_UNLCK
#define	F_UNLCK		2
#define	F_WRLCK		1
#define	F_GETLK		5
#define	F_SETLK		6
#define	F_SETLKW	7
#endif

#ifndef SEEK_CUR
#define	SEEK_CUR	1
#endif

struct flock {
	short	l_type;
	short	l_whence;
	off_t	l_start;
	off_t	l_len;
	pid_t	l_pid;
	long	l_sysid;
};

extern "C" {

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_accept,
	INTERPOS_accept4,
	INTERPOS_aio_suspend,
	INTERPOS_close,
	INTERPOS_connect,
	INTERPOS_fcntl,
	INTERPOS_fsync,
	INTERPOS_fork,
	INTERPOS_msync,
	INTERPOS_nanosleep,
	INTERPOS_openat,
	INTERPOS_poll,
	INTERPOS_pselect,
	INTERPOS_recvfrom,
	INTERPOS_recvmsg,
	INTERPOS_select,
	INTERPOS_sendmsg,
	INTERPOS_sendto,
	INTERPOS_setcontext,
	INTERPOS_sigaction,
	INTERPOS_sigprocmask,
	INTERPOS_sigsuspend,
	INTERPOS_sigwait,
	INTERPOS_sigtimedwait,
	INTERPOS_sigwaitinfo,
	INTERPOS_swapcontext,
	INTERPOS_system,
	INTERPOS_tcdrain,
	INTERPOS_read,
	INTERPOS_readv,
	INTERPOS_wait4,
	INTERPOS_write,
	INTERPOS_writev,
	INTERPOS__pthread_mutex_init_calloc_cb,
	INTERPOS_spinlock,
	INTERPOS_spinunlock,
	INTERPOS_kevent,
	INTERPOS_wait6,
	INTERPOS_ppoll,
	INTERPOS_map_stacks_exec,
	INTERPOS_fdatasync,
	INTERPOS_clock_nanosleep,
	INTERPOS__reserved0,
	INTERPOS_pdfork,
	INTERPOS_uexterr_gettext,
	INTERPOS_pdwait,
	INTERPOS_MAX
};

extern interpos_func_t ref___libc_interposing[INTERPOS_MAX];

int ref_lockf(int filedes, int function, off_t size);
int ref_brk(const void *addr);
int ref__brk(const void *addr);
void *ref_sbrk(intptr_t incr);

}

import pbsd.lib.libc.sys.b0299;

namespace port = pbsd::lib_libc_sys::b0299;

#define	GUARD		0x7f

#define	FL_PAD		64
#define	FL_TOTAL	(sizeof(struct flock) + 2 * FL_PAD)

#define	OFF_NULL	(-4242424242LL)

#define	TAG_NONE	0
#define	TAG_FCNTL	1
#define	TAG_BREAK	2

#define	NSCALAR		8
#define	NPTR		4

struct Snap {
	unsigned long long	ncalls;
	int			tag;
	long long		sc[NSCALAR];
	long long		po[NPTR];
	unsigned		hs[2];
};

static Snap mock;

static int mock_fcntl_ret;
static short mock_getlk_type;
static int mock_getlk_sysid;
static pid_t mock_getlk_pid;

static void *mock_break_init;
static int mock_break_init_fail;
static int mock_break_set_fail;

static const unsigned char *base_fl;

static void
mock_reset(void)
{
	memset(&mock, 0, sizeof(mock));
}

static void
mock_enter(int tag)
{
	mock.ncalls++;
	mock.tag = tag;
}

static long long
poff(const void *p, const unsigned char *base)
{
	if (p == nullptr)
		return (OFF_NULL);
	return ((long long)((const unsigned char *)p - base));
}

static unsigned
hash_bytes(const void *p, size_t n)
{
	const unsigned char *b = (const unsigned char *)p;
	unsigned h = 2166136261u;

	for (size_t i = 0; i < n; i++) {
		h ^= b[i];
		h *= 16777619u;
	}
	return (h);
}

static int
mock_fcntl(int fd, int cmd, intptr_t arg)
{
	struct flock *fl;

	mock_enter(TAG_FCNTL);
	mock.sc[0] = (long long)fd;
	mock.sc[1] = (long long)cmd;
	mock.sc[2] = arg;
	mock.po[0] = poff((const void *)arg, base_fl);

	if (cmd == F_GETLK || cmd == F_SETLK || cmd == F_SETLKW) {
		fl = (struct flock *)(intptr_t)arg;
		if (fl != nullptr) {
			mock.hs[0] = hash_bytes(fl, sizeof(struct flock));
			mock.sc[3] = (long long)fl->l_start;
			mock.sc[4] = (long long)fl->l_len;
			mock.sc[5] = (long long)fl->l_whence;
			mock.sc[6] = (long long)fl->l_type;
			if (cmd == F_GETLK) {
				fl->l_type = mock_getlk_type;
				fl->l_sysid = mock_getlk_sysid;
				fl->l_pid = mock_getlk_pid;
				mock.hs[1] = hash_bytes(fl, sizeof(struct flock));
			}
		}
	}
	return (mock_fcntl_ret);
}

extern "C" void *
__sys_break(char *nsize)
{
	mock_enter(TAG_BREAK);
	mock.sc[0] = (long long)(nsize != nullptr);
	if (nsize != nullptr)
		mock.sc[1] = (long long)(uintptr_t)nsize;

	if (nsize == nullptr) {
		if (mock_break_init_fail)
			return ((void *)-1);
		return (mock_break_init);
	}
	if (mock_break_set_fail)
		return ((void *)-1);
	return ((void *)(uintptr_t)nsize);
}

static void
install_fcntl_mock(interpos_func_t *table)
{
	table[INTERPOS_fcntl] = (interpos_func_t)mock_fcntl;
}

enum {
	FN_LOCKF,
	FN_BRK,
	FN__BRK,
	FN_SBRK,
	FN_COUNT
};

static const char *const fn_name[FN_COUNT] = {
	"lockf",
	"brk",
	"_brk",
	"sbrk",
};

static unsigned long long fn_cases[FN_COUNT];
static unsigned long long fn_fails[FN_COUNT];
static int fn_reported[FN_COUNT];

#define	MAX_REPORTS	8

static void
fail(int fn, const char *what, const char *detail)
{
	fn_fails[fn]++;
	if (fn_reported[fn] < MAX_REPORTS) {
		fn_reported[fn]++;
		printf("  FAIL %-8s %-8s %s\n", fn_name[fn], what, detail);
	} else if (fn_reported[fn] == MAX_REPORTS) {
		fn_reported[fn]++;
		printf("  FAIL %-8s ... further failures suppressed\n",
		    fn_name[fn]);
	}
}

static void
snap_str(char *out, size_t n, const Snap &s)
{
	snprintf(out, n,
	    "{nc=%llu tag=%d sc=[%lld,%lld,%lld,%lld,%lld,%lld,%lld,%lld] "
	    "po=[%lld,%lld,%lld,%lld] hs=[%u,%u]}",
	    s.ncalls, s.tag, s.sc[0], s.sc[1], s.sc[2], s.sc[3], s.sc[4],
	    s.sc[5], s.sc[6], s.sc[7], s.po[0], s.po[1], s.po[2], s.po[3],
	    s.hs[0], s.hs[1]);
}

static void
cmp_snap(int fn, const Snap &a, const Snap &b, const char *ctx)
{
	bool same = a.ncalls == b.ncalls && a.tag == b.tag;

	for (int i = 0; i < NSCALAR; i++)
		same = same && a.sc[i] == b.sc[i];
	for (int i = 0; i < NPTR; i++)
		same = same && a.po[i] == b.po[i];
	for (int i = 0; i < 2; i++)
		same = same && a.hs[i] == b.hs[i];
	if (same)
		return;

	char sa[512], sb[512], msg[1200];

	snap_str(sa, sizeof(sa), a);
	snap_str(sb, sizeof(sb), b);
	snprintf(msg, sizeof(msg), "%s ref=%s port=%s", ctx, sa, sb);
	fail(fn, "callee", msg);
}

static void
cmp_ret_int(int fn, int ra, int rb, const char *ctx)
{
	if (ra != rb) {
		char msg[512];

		snprintf(msg, sizeof(msg), "%s ref=%d port=%d", ctx, ra, rb);
		fail(fn, "return", msg);
	}
}

static void
cmp_ret_ptr(int fn, const void *ra, const void *rb, const char *ctx)
{
	if (ra != rb) {
		char msg[512];

		snprintf(msg, sizeof(msg), "%s ref=%p port=%p", ctx,
		    ra, rb);
		fail(fn, "return", msg);
	}
}

static uint64_t rng_state;

static void
rng_seed(uint64_t seed)
{
	rng_state = seed;
}

static uint64_t
rng_u64(void)
{
	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 7;
	rng_state ^= rng_state << 17;
	return (rng_state);
}

static uint32_t
rng_u32(void)
{
	return ((uint32_t)(rng_u64() >> 11));
}

static int
rnd_int(void)
{
	return ((int)(int32_t)rng_u32());
}

static off_t
rnd_off(void)
{
	return ((off_t)(int64_t)rng_u64());
}

static intptr_t
rnd_ptrdiff(void)
{
	return ((intptr_t)(int64_t)rng_u64());
}

static void
case_lockf(int fd, int function, off_t size, int fcntl_ret,
    short getlk_type, int getlk_sysid, pid_t getlk_pid)
{
	unsigned char fl_a[FL_TOTAL], fl_b[FL_TOTAL];
	Snap snap_a, snap_b;
	int ra, rb, ea, eb;
	char ctx[256];

	fn_cases[FN_LOCKF]++;

	mock_fcntl_ret = fcntl_ret;
	mock_getlk_type = getlk_type;
	mock_getlk_sysid = getlk_sysid;
	mock_getlk_pid = getlk_pid;

	snprintf(ctx, sizeof(ctx),
	    "fd=%d fn=%d size=%lld fcntl_ret=%d type=%d sysid=%d pid=%d",
	    fd, function, (long long)size, fcntl_ret, (int)getlk_type,
	    getlk_sysid, (int)getlk_pid);

	memset(fl_a, GUARD, sizeof(fl_a));
	memset(fl_b, GUARD, sizeof(fl_b));

	base_fl = nullptr;
	install_fcntl_mock(ref___libc_interposing);
	mock_reset();
	errno = 0;
	ra = ref_lockf(fd, function, size);
	ea = errno;
	snap_a = mock;

	base_fl = nullptr;
	install_fcntl_mock(port::__libc_interposing);
	mock_reset();
	errno = 0;
	rb = port::lockf(fd, function, size);
	eb = errno;
	snap_b = mock;

	cmp_snap(FN_LOCKF, snap_a, snap_b, ctx);
	cmp_ret_int(FN_LOCKF, ra, rb, ctx);
	if ((ra == -1 || rb == -1) && ea != eb) {
		char msg[512];

		snprintf(msg, sizeof(msg), "%s ref_errno=%d port_errno=%d", ctx,
		    ea, eb);
		fail(FN_LOCKF, "errno", msg);
	}
}

static void
case_brk(const void *addr, int set_fail)
{
	Snap snap_a, snap_b;
	int ra, rb, ea, eb;
	char ctx[192];

	fn_cases[FN_BRK]++;

	mock_break_set_fail = set_fail;

	snprintf(ctx, sizeof(ctx), "addr=%p set_fail=%d", addr, set_fail);

	mock_reset();
	errno = 0;
	ra = ref_brk(addr);
	ea = errno;
	snap_a = mock;

	mock_reset();
	errno = 0;
	rb = port::brk(addr);
	eb = errno;
	snap_b = mock;

	cmp_snap(FN_BRK, snap_a, snap_b, ctx);
	cmp_ret_int(FN_BRK, ra, rb, ctx);
	if ((ra == -1 || rb == -1) && ea != eb) {
		char msg[512];

		snprintf(msg, sizeof(msg), "%s ref_errno=%d port_errno=%d", ctx,
		    ea, eb);
		fail(FN_BRK, "errno", msg);
	}
}

static void
case__brk(const void *addr, int set_fail)
{
	Snap snap_a, snap_b;
	int ra, rb, ea, eb;
	char ctx[192];

	fn_cases[FN__BRK]++;

	mock_break_set_fail = set_fail;

	snprintf(ctx, sizeof(ctx), "addr=%p set_fail=%d", addr, set_fail);

	mock_reset();
	errno = 0;
	ra = ref__brk(addr);
	ea = errno;
	snap_a = mock;

	mock_reset();
	errno = 0;
	rb = port::_brk(addr);
	eb = errno;
	snap_b = mock;

	cmp_snap(FN__BRK, snap_a, snap_b, ctx);
	cmp_ret_int(FN__BRK, ra, rb, ctx);
	if ((ra == -1 || rb == -1) && ea != eb) {
		char msg[512];

		snprintf(msg, sizeof(msg), "%s ref_errno=%d port_errno=%d", ctx,
		    ea, eb);
		fail(FN__BRK, "errno", msg);
	}
}

static void
case_sbrk(intptr_t incr, int set_fail)
{
	Snap snap_a, snap_b;
	void *ra, *rb;
	int ea, eb;
	char ctx[192];

	fn_cases[FN_SBRK]++;

	mock_break_set_fail = set_fail;

	snprintf(ctx, sizeof(ctx), "incr=%td set_fail=%d", incr, set_fail);

	mock_reset();
	errno = 0;
	ra = ref_sbrk(incr);
	ea = errno;
	snap_a = mock;

	mock_reset();
	errno = 0;
	rb = port::sbrk(incr);
	eb = errno;
	snap_b = mock;

	cmp_snap(FN_SBRK, snap_a, snap_b, ctx);
	cmp_ret_ptr(FN_SBRK, ra, rb, ctx);
	if ((ra == (void *)-1 || rb == (void *)-1) && ea != eb) {
		char msg[512];

		snprintf(msg, sizeof(msg), "%s ref_errno=%d port_errno=%d", ctx,
		    ea, eb);
		fail(FN_SBRK, "errno", msg);
	}
}

static void
configure_break_init(void *init, int init_fail)
{
	mock_break_init = init;
	mock_break_init_fail = init_fail;
	mock_break_set_fail = 0;
}

static void
test_lockf(void)
{
	static const int fds[] = { -1, 0, 1, 3, 0x7e, 0x7f, 0x80, INT_MAX, INT_MIN };
	static const int fns[] = {
		F_ULOCK, F_LOCK, F_TLOCK, F_TEST, 0, 1, -1, 0x7e, 0x7f, 0x80,
	};
	static const off_t sizes[] = {
		0, 1, -1, 0x7e, 0x7f, 0x80, (off_t)LLONG_MAX, (off_t)LLONG_MIN,
	};
	static const int rets[] = { 0, 1, -1, 0x7f, 0x80, INT_MIN, INT_MAX };
	pid_t me = getpid();

	for (size_t f = 0; f < sizeof(fds) / sizeof(fds[0]); f++)
		for (size_t fn = 0; fn < sizeof(fns) / sizeof(fns[0]); fn++)
			for (size_t s = 0; s < sizeof(sizes) / sizeof(sizes[0]); s++)
				for (size_t r = 0; r < sizeof(rets) / sizeof(rets[0]); r++)
					case_lockf(fds[f], fns[fn], sizes[s],
					    rets[r], F_WRLCK, 1, me + 1);

	case_lockf(7, F_TEST, 16, -1, F_WRLCK, 0, me);
	case_lockf(7, F_TEST, 16, 0, F_UNLCK, 99, me + 99);
	case_lockf(7, F_TEST, 16, 0, F_WRLCK, 0, me);
	case_lockf(7, F_TEST, 16, 0, F_WRLCK, 0, me + 1);
	case_lockf(7, F_TEST, 16, 0, F_WRLCK, 1, me);
	case_lockf(7, F_TEST, 16, 0, F_WRLCK, 0, 0);

	for (unsigned b = 0x00; b <= 0xff; b++)
		case_lockf((int)(int8_t)b, F_TLOCK, (off_t)(int8_t)b, (int)(int8_t)~b,
		    (short)(int8_t)b, (int)(int8_t)b, (pid_t)(int8_t)~b);

	rng_seed(0x6c6f636b66ULL);
	for (int n = 0; n < 50000; n++) {
		int fn = rnd_int() & 3;
		int pick = (int)(rng_u32() % 5u);

		switch (pick) {
		case 0:
			fn = F_ULOCK;
			break;
		case 1:
			fn = F_LOCK;
			break;
		case 2:
			fn = F_TLOCK;
			break;
		case 3:
			fn = F_TEST;
			break;
		default:
			fn = rnd_int();
			break;
		}
		case_lockf(rnd_int(), fn, rnd_off(), (int)(int32_t)rng_u32(),
		    (short)(rng_u32() & 3u), rnd_int(), (pid_t)rnd_int());
	}
}

static void
test_break_sequence(void)
{
	uintptr_t base = (uintptr_t)0x100000;
	uintptr_t lo = base - 0x1000;
	void *p;

	configure_break_init((void *)-1, 1);
	case_sbrk(0, 0);
	case_brk((void *)base, 0);
	case__brk((void *)base, 0);

	configure_break_init((void *)base, 0);
	case_sbrk(0, 0);
	case_sbrk(0, 0);

	case_brk((void *)lo, 0);
	case__brk((void *)lo, 0);

	case_brk((void *)(base + 0x100), 0);
	case_sbrk(0x40, 0);
	case_sbrk((intptr_t)-0x20, 0);
	case_sbrk((intptr_t)0, 0);

	case_brk((void *)(base + 0x200), 1);
	case__brk((void *)(base + 0x300), 1);

	case_sbrk((intptr_t)1, 0);
	case_sbrk((intptr_t)-1, 0);
	case_sbrk((intptr_t)0x7e, 0);
	case_sbrk((intptr_t)0x80, 0);
	case_sbrk((intptr_t)-0x7e, 0);
	case_sbrk((intptr_t)-0x80, 0);

	p = (void *)(uintptr_t)-1;
	case_brk(p, 0);
	case__brk(p, 0);
	case_sbrk((intptr_t)INTPTR_MAX, 0);
	case_sbrk((intptr_t)INTPTR_MIN, 0);
	case_sbrk((intptr_t)-1, 0);

	for (unsigned b = 0x00; b <= 0xff; b++) {
		uintptr_t a = base + (uintptr_t)(int8_t)b;

		case_brk((void *)a, (int)(b & 1u));
		case__brk((void *)a, (int)(b & 2u));
		case_sbrk((intptr_t)(int8_t)b, (int)(b & 4u));
	}

	rng_seed(0x62726b7362726bULL);
	for (int n = 0; n < 50000; n++) {
		int kind = (int)(rng_u32() % 3u);
		uintptr_t addr = base + (uintptr_t)(rng_u64() & 0xffffu);
		int fail = (int)(rng_u32() & 1u);
		intptr_t incr = (intptr_t)(int32_t)rng_u32();

		switch (kind) {
		case 0:
			case_brk((void *)addr, fail);
			break;
		case 1:
			case__brk((void *)addr, fail);
			break;
		default:
			case_sbrk(incr, fail);
			break;
		}
	}
}

int
main(void)
{
	unsigned long long total_cases = 0, total_fails = 0;

	test_lockf();
	test_break_sequence();

	printf("\n%-8s %12s %12s\n", "function", "cases", "failures");
	printf("--------------------------------------\n");
	for (int i = 0; i < FN_COUNT; i++) {
		printf("%-8s %12llu %12llu\n", fn_name[i], fn_cases[i],
		    fn_fails[i]);
		total_cases += fn_cases[i];
		total_fails += fn_fails[i];
	}
	printf("--------------------------------------\n");
	printf("%-8s %12llu %12llu\n", "TOTAL", total_cases, total_fails);

	return (total_fails == 0 ? 0 : 1);
}
