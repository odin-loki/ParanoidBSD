// Differential harness for PBSD batch b0304.

import pbsd.lib.libc.gen.b0304;

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <signal.h>
#include <unistd.h>

struct pthread_mutex;

namespace P = pbsd::lib_libc_gen::b0304;

static const unsigned char GUARD = 0x7f;
static const long SWEEP = 50000L;

#define __DTF_READALL	0x0008
#define __DTF_SKIPREAD	0x0010

extern "C" {
extern sigset_t _sigintr;
int ref_siginterrupt(int sig, int flag);
void ref_rewinddir(P::DIR *dirp);
}

/* ------------------------------------------------------------------ mocks */

int __isthreaded;

struct sigact_state {
	long	calls;
	int	fail_get_sig;
	int	fail_set_sig;
	int	get_ret;
	int	set_ret;
	int	initial_flags;
	int	last_get_sig;
	int	last_set_sig;
	int	last_set_flags;
};

static sigact_state g_sigact;

static void
sigact_reset(int fail_get, int fail_set, int get_ret, int set_ret, int init_flags)
{
	std::memset(&g_sigact, 0, sizeof(g_sigact));
	g_sigact.fail_get_sig = fail_get;
	g_sigact.fail_set_sig = fail_set;
	g_sigact.get_ret = get_ret;
	g_sigact.set_ret = set_ret;
	g_sigact.initial_flags = init_flags;
}

extern "C" int
__libc_sigaction(int signum, const struct sigaction *act,
    struct sigaction *oldact)
{
	g_sigact.calls++;
	if (act == nullptr) {
		g_sigact.last_get_sig = signum;
		if (oldact != nullptr) {
			oldact->sa_handler = SIG_DFL;
			sigemptyset(&oldact->sa_mask);
			oldact->sa_flags = g_sigact.initial_flags;
		}
		if (signum == g_sigact.fail_get_sig) {
			errno = EINVAL;
			return (g_sigact.get_ret);
		}
		return (0);
	}
	g_sigact.last_set_sig = signum;
	g_sigact.last_set_flags = act->sa_flags;
	if (signum == g_sigact.fail_set_sig) {
		errno = EINVAL;
		return (g_sigact.set_ret);
	}
	return (0);
}

static long g_lock_calls;
static long g_unlock_calls;

extern "C" void
_pthread_mutex_lock(struct pthread_mutex **m)
{
	(void)m;
	g_lock_calls++;
}

extern "C" void
_pthread_mutex_unlock(struct pthread_mutex **m)
{
	(void)m;
	g_unlock_calls++;
}

struct lseek_state {
	long	calls;
	int	last_fd;
	off_t	last_off;
	int	last_whence;
};

static lseek_state g_lseek;

extern "C" off_t
test_lseek(int fd, off_t offset, int whence)
{
	g_lseek.calls++;
	g_lseek.last_fd = fd;
	g_lseek.last_off = offset;
	g_lseek.last_whence = whence;
	return (0);
}

static long g_filldir_calls;
static int g_filldir_arg;

extern "C" bool
_filldir(P::DIR *dirp, bool use_current_pos)
{
	(void)dirp;
	g_filldir_calls++;
	g_filldir_arg = use_current_pos ? 1 : 0;
	return (true);
}

static long g_reclaim_calls;

extern "C" void
_reclaim_telldir(P::DIR *dirp)
{
	(void)dirp;
	g_reclaim_calls++;
}

/* ------------------------------------------------------------------ stats */

struct Stat {
	const char *name;
	unsigned long long cases;
	unsigned long long fails;
};

static Stat stats[] = {
	{ "siginterrupt", 0, 0 },
	{ "rewinddir", 0, 0 },
};
static const int NSTAT = (int)(sizeof(stats) / sizeof(stats[0]));

static Stat &
S(const char *n)
{
	for (int i = 0; i < NSTAT; i++)
		if (std::strcmp(stats[i].name, n) == 0)
			return stats[i];
	std::fprintf(stderr, "bad stat %s\n", n);
	std::exit(2);
}

static void
fail(Stat &st, const char *msg)
{
	st.fails++;
	if (st.fails <= 8)
		std::printf("  FAIL %s: %s\n", st.name, msg);
}

static void
ok(Stat &st)
{
	st.cases++;
}

/* ------------------------------------------------------------------ rng */

struct Rng {
	std::uint64_t s;
	explicit Rng(std::uint64_t seed) : s(seed) {}
	std::uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}
	std::uint32_t u32() { return (std::uint32_t)next(); }
	int coin() { return (int)(next() & 1); }
	int range(int lo, int hi)
	{
		if (hi <= lo)
			return lo;
		return lo + (int)(u32() % (std::uint32_t)(hi - lo + 1));
	}
};

static bool
sigset_eq(const sigset_t &a, const sigset_t &b)
{
	return std::memcmp(&a, &b, sizeof(sigset_t)) == 0;
}

static void
sync_sigintr(const sigset_t *mask)
{
	if (mask != nullptr) {
		P::_sigintr = *mask;
		_sigintr = *mask;
	} else {
		sigemptyset(&P::_sigintr);
		sigemptyset(&_sigintr);
	}
}

struct sig_capture {
	int	rv;
	long	calls;
	int	last_get_sig;
	int	last_set_sig;
	int	last_set_flags;
	sigset_t mask;
	int	in_sig;
	int	was_member_before;
};

static sig_capture
run_siginterrupt(bool port_side, int sig, int flag, int fail_get, int fail_set,
    int get_ret, int set_ret, int init_flags, const sigset_t *pre_mask)
{
	sig_capture c;
	sigset_t before;

	sync_sigintr(pre_mask);
	before = port_side ? P::_sigintr : _sigintr;
	c.was_member_before = sigismember(&before, sig);
	c.in_sig = sig;

	sigact_reset(fail_get, fail_set, get_ret, set_ret, init_flags);
	g_lock_calls = g_unlock_calls = 0;

	if (port_side)
		c.rv = P::siginterrupt(sig, flag);
	else
		c.rv = ref_siginterrupt(sig, flag);

	c.calls = g_sigact.calls;
	c.last_get_sig = g_sigact.last_get_sig;
	c.last_set_sig = g_sigact.last_set_sig;
	c.last_set_flags = g_sigact.last_set_flags;
	c.mask = port_side ? P::_sigintr : _sigintr;
	return (c);
}

static bool
sig_cap_eq(const sig_capture &a, const sig_capture &b)
{
	return (a.rv == b.rv && a.calls == b.calls &&
	    a.last_get_sig == b.last_get_sig &&
	    a.last_set_sig == b.last_set_sig &&
	    a.last_set_flags == b.last_set_flags &&
	    sigset_eq(a.mask, b.mask));
}

static void
check_siginterrupt(int sig, int flag, int fail_get, int fail_set, int get_ret,
    int set_ret, int init_flags, const sigset_t *pre_mask, const char *label)
{
	Stat &st = S("siginterrupt");
	sig_capture pa, ra;

	st.cases++;
	pa = run_siginterrupt(true, sig, flag, fail_get, fail_set, get_ret,
	    set_ret, init_flags, pre_mask);
	ra = run_siginterrupt(false, sig, flag, fail_get, fail_set, get_ret,
	    set_ret, init_flags, pre_mask);
	if (!sig_cap_eq(pa, ra))
		fail(st, label);
}

static void
test_siginterrupt_edges(void)
{
	sigset_t m;

	sigemptyset(&m);
	check_siginterrupt(0, 0, -1, -1, 0, 0, 0, &m, "sig0 flag0 empty");
	check_siginterrupt(0, 1, -1, -1, 0, 0, SA_RESTART, &m, "sig0 flag1 restart");
	check_siginterrupt(0, 1, -1, -1, 0, 0, 0, &m, "sig0 flag1 norestart");
	check_siginterrupt(0, 0, -1, -1, 0, 0, SA_RESTART, &m, "sig0 flag0 clear restart");

	check_siginterrupt(1, 1, -1, -1, 0, 0, 0, &m, "sig1 flag1");
	check_siginterrupt(1, 0, -1, -1, 0, 0, 0, &m, "sig1 flag0");
	check_siginterrupt(SIGINT, 1, -1, -1, 0, 0, SA_RESTART, &m, "sigint flag1");
	check_siginterrupt(SIGINT, 0, -1, -1, 0, 0, 0, &m, "sigint flag0");

	sigaddset(&m, SIGINT);
	check_siginterrupt(SIGINT, 0, -1, -1, 0, 0, SA_RESTART, &m,
	    "member del restart");
	check_siginterrupt(SIGINT, 1, -1, -1, 0, 0, 0, &m, "member add norestart");

	sigemptyset(&m);
	sigaddset(&m, SIGUSR1);
	check_siginterrupt(SIGUSR2, 1, -1, -1, 0, 0, 0, &m, "other sig add");

	check_siginterrupt(5, 2, -1, -1, 0, 0, SA_RESTART, &m, "flag truthy 2");
	check_siginterrupt(5, -1, -1, -1, 0, 0, SA_RESTART, &m, "flag truthy -1");
	check_siginterrupt(5, 0x80, -1, -1, 0, 0, 0, &m, "flag high bit");

	check_siginterrupt(7, 1, 7, -1, -1, 0, SA_RESTART, &m, "get fail -1");
	check_siginterrupt(8, 1, 8, -1, -2, 0, 0, &m, "get fail -2");
	check_siginterrupt(9, 1, -1, 9, 0, -1, SA_RESTART, &m, "set fail -1");
	check_siginterrupt(10, 0, -1, 10, 0, -3, 0, &m, "set fail -3 flag0");

	sigemptyset(&m);
	for (int s = 0; s < 32; s++) {
		if (s == 9 || s == 32)
			continue;
		if (s & 1)
			sigaddset(&m, s);
		check_siginterrupt(s, s & 1, -1, -1, 0, 0,
		    (s & 2) ? SA_RESTART : 0, &m, "boundary sweep");
	}

	check_siginterrupt(31, 1, -1, -1, 0, 0, SA_RESTART, &m, "sig31");
	check_siginterrupt(32, 0, -1, -1, 0, 0, 0, &m, "sig32");
	check_siginterrupt(63, 1, -1, -1, 0, 0, 0, &m, "sig63");
	check_siginterrupt(64, 0, -1, -1, 0, 0, SA_RESTART, &m, "sig64");
}

static void
test_siginterrupt_random(Rng &rng)
{
	Stat &st = S("siginterrupt");
	sigset_t m;

	for (long i = 0; i < SWEEP; i++) {
		int sig = (int)(rng.u32() % 128);
		int flag = (int)rng.u32();
		int fail_get = (rng.coin() == 0) ? -1 : (int)(rng.u32() % 128);
		int fail_set = (rng.coin() == 0) ? -1 : (int)(rng.u32() % 128);
		int get_ret = (rng.coin() ? -1 : -2);
		int set_ret = (rng.coin() ? -1 : -3);
		int init_flags = (rng.coin() ? SA_RESTART : 0) |
		    (rng.coin() ? SA_ONSTACK : 0);

		sigemptyset(&m);
		for (int b = 0; b < 8; b++) {
			if (rng.coin())
				sigaddset(&m, rng.range(0, 127));
		}

		st.cases++;
		sig_capture pa = run_siginterrupt(true, sig, flag, fail_get, fail_set,
		    get_ret, set_ret, init_flags, &m);
		sig_capture ra = run_siginterrupt(false, sig, flag, fail_get, fail_set,
		    get_ret, set_ret, init_flags, &m);
		if (!sig_cap_eq(pa, ra))
			fail(st, "random");
	}
}

/* ---------------------------------------------------------------- rewind */

struct test_dir {
	unsigned char	pre[32];
	P::DIR		dir;
	unsigned char	post[32];
};

struct rewind_capture {
	long	lock_calls;
	long	unlock_calls;
	long	filldir_calls;
	int	filldir_arg;
	long	lseek_calls;
	int	lseek_fd;
	off_t	lseek_off;
	int	lseek_whence;
	long	reclaim_calls;
	int	dd_fd;
	std::size_t dd_loc;
	off_t	dd_seek;
	int	dd_flags;
	unsigned char pre[32];
	unsigned char post[32];
};

static void
init_test_dir(test_dir &td, int fd, std::size_t loc, off_t seek, int flags)
{
	std::memset(&td, GUARD, sizeof(td));
	td.dir.dd_fd = fd;
	td.dir.dd_loc = loc;
	td.dir.dd_seek = seek;
	td.dir.dd_flags = flags;
	td.dir.dd_lock = nullptr;
}

static void
rewind_mocks_reset(void)
{
	g_lock_calls = g_unlock_calls = 0;
	g_filldir_calls = 0;
	g_filldir_arg = -1;
	std::memset(&g_lseek, 0, sizeof(g_lseek));
	g_reclaim_calls = 0;
}

static rewind_capture
snap_rewind(const test_dir &td)
{
	rewind_capture c;

	c.lock_calls = g_lock_calls;
	c.unlock_calls = g_unlock_calls;
	c.filldir_calls = g_filldir_calls;
	c.filldir_arg = g_filldir_arg;
	c.lseek_calls = g_lseek.calls;
	c.lseek_fd = g_lseek.last_fd;
	c.lseek_off = g_lseek.last_off;
	c.lseek_whence = g_lseek.last_whence;
	c.reclaim_calls = g_reclaim_calls;
	c.dd_fd = td.dir.dd_fd;
	c.dd_loc = td.dir.dd_loc;
	c.dd_seek = td.dir.dd_seek;
	c.dd_flags = td.dir.dd_flags;
	std::memcpy(c.pre, td.pre, sizeof(c.pre));
	std::memcpy(c.post, td.post, sizeof(c.post));
	return (c);
}

static bool
rewind_cap_eq(const rewind_capture &a, const rewind_capture &b)
{
	return (a.lock_calls == b.lock_calls &&
	    a.unlock_calls == b.unlock_calls &&
	    a.filldir_calls == b.filldir_calls &&
	    a.filldir_arg == b.filldir_arg && a.lseek_calls == b.lseek_calls &&
	    a.lseek_fd == b.lseek_fd && a.lseek_off == b.lseek_off &&
	    a.lseek_whence == b.lseek_whence &&
	    a.reclaim_calls == b.reclaim_calls && a.dd_fd == b.dd_fd &&
	    a.dd_loc == b.dd_loc && a.dd_seek == b.dd_seek &&
	    a.dd_flags == b.dd_flags &&
	    std::memcmp(a.pre, b.pre, sizeof(a.pre)) == 0 &&
	    std::memcmp(a.post, b.post, sizeof(a.post)) == 0);
}

static void
check_rewinddir(int fd, std::size_t loc, off_t seek, int flags, int threaded,
    const char *label)
{
	Stat &st = S("rewinddir");
	test_dir da, db;
	rewind_capture ca, cb;

	st.cases++;
	init_test_dir(da, fd, loc, seek, flags);
	init_test_dir(db, fd, loc, seek, flags);
	__isthreaded = threaded;

	rewind_mocks_reset();
	P::rewinddir(&da.dir);
	ca = snap_rewind(da);

	rewind_mocks_reset();
	ref_rewinddir(reinterpret_cast<P::DIR *>(&db.dir));
	cb = snap_rewind(db);

	if (!rewind_cap_eq(ca, cb))
		fail(st, label);
}

static void
test_rewinddir_edges(void)
{
	check_rewinddir(0, 0, 0, 0, 0, "plain fd0");
	check_rewinddir(3, 0, 0, 0, 0, "fd3");
	check_rewinddir(-1, 99, 77, 0, 0, "fd neg loc seek");
	check_rewinddir(7, 1, 1, __DTF_SKIPREAD, 0, "skipread only");
	check_rewinddir(7, 1, 1, __DTF_READALL, 0, "readall lseek skip");
	check_rewinddir(7, 1, 1, __DTF_READALL | __DTF_SKIPREAD, 0,
	    "readall clears skipread");
	check_rewinddir(4, 1024, 2048, __DTF_READALL, 1, "threaded readall");
	check_rewinddir(4, 1024, 2048, 0, 1, "threaded lseek");
	check_rewinddir(4, 1024, 2048, __DTF_SKIPREAD, 1, "threaded skip");
	check_rewinddir(0x7f, 0x80, 0x7f, 0x00ff, 0, "high-bit fields");
	check_rewinddir(0xff, 0xffffffffu, (off_t)-1, 0xffff, 0,
	    "max-ish values");
	check_rewinddir(1, 0, 100, __DTF_READALL - 1, 0, "below readall");
	check_rewinddir(1, 0, 100, __DTF_READALL + 1, 0, "above readall");
	check_rewinddir(2, 0, 100, __DTF_READALL | 0x100, 0, "readall extra");
}

static void
test_rewinddir_random(Rng &rng)
{
	Stat &st = S("rewinddir");

	for (long i = 0; i < SWEEP; i++) {
		int fd = (int)rng.u32();
		if (fd == 0 && rng.coin())
			fd = -1;
		std::size_t loc = (std::size_t)rng.u32();
		off_t seek = (off_t)rng.u32();
		int flags = (int)rng.u32();
		if (rng.coin())
			flags |= __DTF_READALL;
		if (rng.coin())
			flags |= __DTF_SKIPREAD;
		int threaded = rng.coin();

		test_dir da, db;
		rewind_capture ca, cb;

		st.cases++;
		init_test_dir(da, fd, loc, seek, flags);
		init_test_dir(db, fd, loc, seek, flags);
		__isthreaded = threaded;

		rewind_mocks_reset();
		P::rewinddir(&da.dir);
		ca = snap_rewind(da);

		rewind_mocks_reset();
		ref_rewinddir(reinterpret_cast<P::DIR *>(&db.dir));
		cb = snap_rewind(db);

		if (!rewind_cap_eq(ca, cb))
			fail(st, "random");
	}
}

/* ------------------------------------------------------------------ main */

int
main()
{
	Rng rng(0xb0304u);

	test_siginterrupt_edges();
	test_siginterrupt_random(rng);

	test_rewinddir_edges();
	test_rewinddir_random(rng);

	std::printf("\n%-16s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < NSTAT; i++)
		std::printf("%-16s %12llu %12llu\n", stats[i].name,
		    stats[i].cases, stats[i].fails);

	unsigned long long total_fail = 0;
	for (int i = 0; i < NSTAT; i++)
		total_fail += stats[i].fails;

	return (total_fail == 0 ? 0 : 1);
}
