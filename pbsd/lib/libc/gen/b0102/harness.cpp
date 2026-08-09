/*
 * harness.cpp -- differential test for batch b0102.
 *
 * FreeBSD-private libc internals are exercised through recording test doubles
 * defined here and linked into both the port and the oracle.  Every observable
 * (return value, errno, buffer contents including guard bytes, and mock
 * records) is compared side by side.
 */

import pbsd.lib.libc.gen.b0102;

#include <csetjmp>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

namespace port = pbsd::lib_libc_gen::b0102;

#define	GETENTROPY_MAX	256
#define	LIO_WRITE	2
#define	LIO_FOFFSET	0x10
#define	LIO_VECTORED	0x20
#define	LIO_NOWAIT	1
#define	AIO_OP2_FOFFSET	0x0001
#define	AIO_OP2_VECTORED	0x0002

#define	GUARD	0x7f
#define	BUFSZ	512u
#define	SWEEP_ITERS	200000L
#define	MAX_PRINT	12
#define	ENT_RAISED	(-1000)

struct ora_aiocb {
	int	aio_lio_opcode;
};

struct ora_dir {
	pthread_mutex_t	dd_lock;
};

extern "C" {
int ref_aio_write2(ora_aiocb *, int);
void ref_seekdir(ora_dir *, long);
sig_t ref_signal(int, sig_t);
int ref_getentropy(void *, size_t);
extern sigset_t _sigintr;
}

/* ------------------------------------------------------------------ shared */

int __isthreaded;

static unsigned char *bufa;
static unsigned char *bufb;

struct stat_row {
	const char *name;
	long		cases;
	long		failures;
	long		printed;
};

static stat_row rows[] = {
	{ "aio_write2", 0, 0, 0 },
	{ "seekdir", 0, 0, 0 },
	{ "signal", 0, 0, 0 },
	{ "getentropy", 0, 0, 0 },
};

#define	R_AIO	0
#define	R_SEEK	1
#define	R_SIG	2
#define	R_ENT	3

static uint64_t rng_state = 0x00b0102feedULL;

static inline uint64_t
rnd64(void)
{
	uint64_t z = (rng_state += 0x9e3779b97f4a7c15ULL);

	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return (z ^ (z >> 31));
}

static inline uint32_t
rnd32(void)
{
	return ((uint32_t)(rnd64() >> 32));
}

static inline int
rnd_i(void)
{
	return ((int)rnd32());
}

static void
fail_row(int row, const char *label, const char *detail)
{
	stat_row &r = rows[row];

	r.failures++;
	if (r.printed < MAX_PRINT) {
		r.printed++;
		std::printf("  FAIL %-14s %-24s %s\n", r.name, label, detail);
	}
}

/* ------------------------------------------------------------- aio doubles */

enum aio_script {
	AIO_OK = 0,
	AIO_FAIL_ENOENT,
	AIO_FAIL_EIO_AERR_NEG,
	AIO_FAIL_EIO_AERR_ZERO,
	AIO_FAIL_EIO_AERR_EBUSY,
	AIO_FAIL_EIO_AERR_EAGAIN,
	AIO_SCRIPT_COUNT
};

struct aio_mock {
	int	script;
	long	lio_calls;
	long	aio_error_calls;
	int	last_mode;
	int	last_nent;
	int	last_opcode;
};

static aio_mock g_aio;

static void
aio_mock_reset(int script)
{
	std::memset(&g_aio, 0, sizeof(g_aio));
	g_aio.script = script;
}

extern "C" int
lio_listio(int mode, aiocb *const list[], int nent, void *sig)
{
	(void)sig;

	g_aio.lio_calls++;
	g_aio.last_mode = mode;
	g_aio.last_nent = nent;
	if (list != nullptr && nent > 0 && list[0] != nullptr)
		g_aio.last_opcode = list[0]->aio_lio_opcode;

	switch (g_aio.script) {
	case AIO_OK:
		errno = 0;
		return (0);
	case AIO_FAIL_ENOENT:
		errno = ENOENT;
		return (-1);
	case AIO_FAIL_EIO_AERR_NEG:
	case AIO_FAIL_EIO_AERR_ZERO:
	case AIO_FAIL_EIO_AERR_EBUSY:
	case AIO_FAIL_EIO_AERR_EAGAIN:
		errno = EIO;
		return (-1);
	default:
		errno = EINVAL;
		return (-1);
	}
}

extern "C" int
aio_error(const aiocb *iocb)
{
	(void)iocb;

	g_aio.aio_error_calls++;
	switch (g_aio.script) {
	case AIO_FAIL_EIO_AERR_NEG:
		return (-1);
	case AIO_FAIL_EIO_AERR_ZERO:
		return (0);
	case AIO_FAIL_EIO_AERR_EBUSY:
		return (EBUSY);
	case AIO_FAIL_EIO_AERR_EAGAIN:
		return (EAGAIN);
	default:
		return (0);
	}
}

struct guarded_aiocb {
	unsigned char	pre[32];
	aiocb		cb;
	unsigned char	post[32];
};

struct aio_capture {
	int	rv;
	int	errno_copy;
	int	opcode;
	long	lio_calls;
	long	aio_error_calls;
	int	last_mode;
	int	last_nent;
	int	last_opcode;
	unsigned char	pre[32];
	unsigned char	post[32];
};

static aio_capture
snap_aio(int rv, const guarded_aiocb &g)
{
	aio_capture c;

	c.rv = rv;
	c.errno_copy = errno;
	c.opcode = g.cb.aio_lio_opcode;
	c.lio_calls = g_aio.lio_calls;
	c.aio_error_calls = g_aio.aio_error_calls;
	c.last_mode = g_aio.last_mode;
	c.last_nent = g_aio.last_nent;
	c.last_opcode = g_aio.last_opcode;
	std::memcpy(c.pre, g.pre, sizeof(c.pre));
	std::memcpy(c.post, g.post, sizeof(c.post));
	return (c);
}

static bool
aio_cap_eq(const aio_capture &a, const aio_capture &b)
{
	return (a.rv == b.rv && a.errno_copy == b.errno_copy &&
	    a.opcode == b.opcode && a.lio_calls == b.lio_calls &&
	    a.aio_error_calls == b.aio_error_calls &&
	    a.last_mode == b.last_mode && a.last_nent == b.last_nent &&
	    a.last_opcode == b.last_opcode &&
	    std::memcmp(a.pre, b.pre, sizeof(a.pre)) == 0 &&
	    std::memcmp(a.post, b.post, sizeof(b.post)) == 0);
}

static void
init_guarded_aiocb(guarded_aiocb &g, int seed_opcode)
{
	std::memset(&g, GUARD, sizeof(g));
	g.cb.aio_lio_opcode = seed_opcode;
}

static void
do_aio_write2(int flags, int script, int seed_opcode, const char *label)
{
	guarded_aiocb ga, gb;
	int rva, rvb;
	aio_capture ca, cb;

	rows[R_AIO].cases++;

	init_guarded_aiocb(ga, seed_opcode);
	init_guarded_aiocb(gb, seed_opcode);

	aio_mock_reset(script);
	errno = 0;
	rva = port::aio_write2(&ga.cb, flags);
	ca = snap_aio(rva, ga);

	aio_mock_reset(script);
	errno = 0;
	rvb = ref_aio_write2(reinterpret_cast<ora_aiocb *>(&gb.cb), flags);
	cb = snap_aio(rvb, gb);

	if (!aio_cap_eq(ca, cb))
		fail_row(R_AIO, label, "mismatch");
}

/* ------------------------------------------------------------ seek doubles */

struct test_dir {
	unsigned char	pre[32];
	DIR		dir;
	long		recorded_loc;
	unsigned char	post[32];
};

struct seek_mock {
	long	lock_calls;
	long	unlock_calls;
	long	seekdir_calls;
	long	last_loc;
};

static seek_mock g_seek;

static void
seek_mock_reset(void)
{
	std::memset(&g_seek, 0, sizeof(g_seek));
}

extern "C" void
_pthread_mutex_lock(pthread_mutex_t *m)
{
	(void)m;
	g_seek.lock_calls++;
}

extern "C" void
_pthread_mutex_unlock(pthread_mutex_t *m)
{
	(void)m;
	g_seek.unlock_calls++;
}

extern "C" void
_seekdir(DIR *dirp, long loc)
{
	test_dir *td;

	g_seek.seekdir_calls++;
	g_seek.last_loc = loc;
	td = reinterpret_cast<test_dir *>(reinterpret_cast<char *>(dirp) -
	    offsetof(test_dir, dir));
	td->recorded_loc = loc;
}

static void
init_test_dir(test_dir &td)
{
	std::memset(&td, GUARD, sizeof(td));
	pthread_mutex_init(&td.dir.dd_lock, nullptr);
	td.recorded_loc = -9999L;
}

struct seek_capture {
	long	lock_calls;
	long	unlock_calls;
	long	seekdir_calls;
	long	last_loc;
	long	recorded_loc;
	unsigned char	pre[32];
	unsigned char	post[32];
};

static seek_capture
snap_seek(const test_dir &td)
{
	seek_capture c;

	c.lock_calls = g_seek.lock_calls;
	c.unlock_calls = g_seek.unlock_calls;
	c.seekdir_calls = g_seek.seekdir_calls;
	c.last_loc = g_seek.last_loc;
	c.recorded_loc = td.recorded_loc;
	std::memcpy(c.pre, td.pre, sizeof(c.pre));
	std::memcpy(c.post, td.post, sizeof(c.post));
	return (c);
}

static bool
seek_cap_eq(const seek_capture &a, const seek_capture &b)
{
	return (a.lock_calls == b.lock_calls &&
	    a.unlock_calls == b.unlock_calls &&
	    a.seekdir_calls == b.seekdir_calls && a.last_loc == b.last_loc &&
	    a.recorded_loc == b.recorded_loc &&
	    std::memcmp(a.pre, b.pre, sizeof(a.pre)) == 0 &&
	    std::memcmp(a.post, b.post, sizeof(b.post)) == 0);
}

static void
do_seekdir(long loc, int threaded, const char *label)
{
	test_dir da, db;
	seek_capture ca, cb;

	rows[R_SEEK].cases++;

	init_test_dir(da);
	init_test_dir(db);

	__isthreaded = threaded;

	seek_mock_reset();
	port::seekdir(&da.dir, loc);
	ca = snap_seek(da);

	seek_mock_reset();
	ref_seekdir(reinterpret_cast<ora_dir *>(&db.dir), loc);
	cb = snap_seek(db);

	if (!seek_cap_eq(ca, cb))
		fail_row(R_SEEK, label, "mismatch");
}

/* ------------------------------------------------------------ signal mocks */

struct sigact_mock {
	long		calls;
	int		last_signum;
	void	       *last_handler;
	int		last_flags;
	int		fail_signum;
	void	       *old_handler;
};

static sigact_mock g_sigact;

static void
sigact_mock_reset(int fail_signum, void (*old_handler)(int))
{
	std::memset(&g_sigact, 0, sizeof(g_sigact));
	g_sigact.fail_signum = fail_signum;
	g_sigact.old_handler = reinterpret_cast<void *>(old_handler);
}

extern "C" int
__libc_sigaction(int signum, const struct sigaction *act,
    struct sigaction *oldact)
{
	g_sigact.calls++;
	g_sigact.last_signum = signum;
	if (act != nullptr) {
		g_sigact.last_handler = reinterpret_cast<void *>(act->sa_handler);
		g_sigact.last_flags = act->sa_flags;
	}
	if (oldact != nullptr) {
		oldact->sa_handler =
		    reinterpret_cast<void (*)(int)>(g_sigact.old_handler);
		sigemptyset(&oldact->sa_mask);
		oldact->sa_flags = 0;
	}
	if (signum == g_sigact.fail_signum) {
		errno = EINVAL;
		return (-1);
	}
	return (0);
}

static void
sync_sigintr(const sigset_t *mask)
{
	port::_sigintr = *mask;
	_sigintr = *mask;
}

struct sig_capture {
	sig_t		rv;
	long		calls;
	int		last_signum;
	void	       *last_handler;
	int		last_flags;
};

static sig_capture
snap_sig(sig_t rv)
{
	sig_capture c;

	c.rv = rv;
	c.calls = g_sigact.calls;
	c.last_signum = g_sigact.last_signum;
	c.last_handler = g_sigact.last_handler;
	c.last_flags = g_sigact.last_flags;
	return (c);
}

static bool
sig_cap_eq(const sig_capture &a, const sig_capture &b)
{
	return (a.rv == b.rv && a.calls == b.calls &&
	    a.last_signum == b.last_signum && a.last_handler == b.last_handler &&
	    a.last_flags == b.last_flags);
}

static void
do_signal(int signum, void (*handler)(int), bool intr_member, int fail_signum,
    void (*old_handler)(int), const char *label)
{
	sigset_t mask;
	sig_capture ca, cb;
	sig_t rva, rvb;

	rows[R_SIG].cases++;

	sigemptyset(&mask);
	if (intr_member)
		sigaddset(&mask, signum);
	sync_sigintr(&mask);

	sigact_mock_reset(fail_signum, old_handler);
	rva = port::signal(signum, handler);
	ca = snap_sig(rva);

	sigact_mock_reset(fail_signum, old_handler);
	rvb = ref_signal(signum, handler);
	cb = snap_sig(rvb);

	if (!sig_cap_eq(ca, cb))
		fail_row(R_SIG, label, "mismatch");
}

/* ---------------------------------------------------------- entropy mocks */

enum ent_script {
	ENT_OK_FULL = 0,
	ENT_OK_ZERO,
	ENT_PARTIAL_TWO,
	ENT_PARTIAL_MANY,
	ENT_EINTR_ONCE,
	ENT_EINTR_TWICE,
	ENT_EFAULT,
	ENT_FATAL_ERRNO,
	ENT_FATAL_ZERO,
	ENT_SCRIPT_COUNT
};

struct ent_step {
	ssize_t	rv;
	int	err;
};

struct ent_mock {
	int		script;
	long		calls;
	size_t		last_len;
	unsigned int	last_flags;
	ent_step	steps[16];
	int		nsteps;
	int		phase;
};

static ent_mock g_ent;
static jmp_buf g_ent_jmp;
static volatile int g_ent_jmp_active;
static int g_raise_sig;

static void
ent_mock_reset(int script)
{
	std::memset(&g_ent, 0, sizeof(g_ent));
	g_ent.script = script;
	g_ent.nsteps = 0;

	switch (script) {
	case ENT_OK_FULL:
		g_ent.steps[0] = { 4096, 0 };
		g_ent.nsteps = 1;
		break;
	case ENT_OK_ZERO:
		g_ent.nsteps = 0;
		break;
	case ENT_PARTIAL_TWO:
		g_ent.steps[0] = { 7, 0 };
		g_ent.steps[1] = { 4096, 0 };
		g_ent.nsteps = 2;
		break;
	case ENT_PARTIAL_MANY:
		g_ent.steps[0] = { 1, 0 };
		g_ent.steps[1] = { 3, 0 };
		g_ent.steps[2] = { 5, 0 };
		g_ent.steps[3] = { 4096, 0 };
		g_ent.nsteps = 4;
		break;
	case ENT_EINTR_ONCE:
		g_ent.steps[0] = { -1, EINTR };
		g_ent.steps[1] = { 4096, 0 };
		g_ent.nsteps = 2;
		break;
	case ENT_EINTR_TWICE:
		g_ent.steps[0] = { -1, EINTR };
		g_ent.steps[1] = { -1, EINTR };
		g_ent.steps[2] = { 4096, 0 };
		g_ent.nsteps = 3;
		break;
	case ENT_EFAULT:
		g_ent.steps[0] = { -1, EFAULT };
		g_ent.nsteps = 1;
		break;
	case ENT_FATAL_ERRNO:
		g_ent.steps[0] = { -1, ENOENT };
		g_ent.nsteps = 1;
		break;
	case ENT_FATAL_ZERO:
		g_ent.steps[0] = { 0, 0 };
		g_ent.nsteps = 1;
		break;
	default:
		break;
	}
}

extern "C" ssize_t
getrandom(void *buf, size_t buflen, unsigned int flags)
{
	ent_step st;
	ssize_t rd;

	g_ent.calls++;
	g_ent.last_len = buflen;
	g_ent.last_flags = flags;

	if (g_ent.phase >= g_ent.nsteps) {
		rd = (ssize_t)buflen;
		if (buf != nullptr && rd > 0)
			std::memset(buf, (int)(0xa0 + (g_ent.calls & 0x3f)),
			    (size_t)rd);
		return (rd);
	}

	st = g_ent.steps[g_ent.phase++];
	if (st.rv < 0) {
		errno = st.err;
		return (-1);
	}
	if (st.rv == 0)
		return (0);
	rd = st.rv;
	if ((size_t)rd > buflen)
		rd = (ssize_t)buflen;
	if (buf != nullptr && rd > 0) {
		for (ssize_t i = 0; i < rd; i++)
			((unsigned char *)buf)[i] =
			    (unsigned char)(0x40 + g_ent.calls + (size_t)i);
	}
	return (rd);
}

extern "C" int
raise(int sig)
{
	g_raise_sig = sig;
	if (sig == SIGKILL && g_ent_jmp_active)
		longjmp(g_ent_jmp, 1);
	return (-1);
}

struct ent_capture {
	int	rv;
	int	errno_copy;
	long	calls;
	int	raised;
	unsigned char buf[BUFSZ];
	unsigned char pre[32];
	unsigned char post[32];
};

static int
call_port_getentropy(void *buf, size_t len)
{
	int rv;

	g_ent_jmp_active = 1;
	if (setjmp(g_ent_jmp) != 0) {
		g_ent_jmp_active = 0;
		return (ENT_RAISED);
	}
	rv = port::getentropy(buf, len);
	g_ent_jmp_active = 0;
	return (rv);
}

static int
call_ref_getentropy(void *buf, size_t len)
{
	int rv;

	g_ent_jmp_active = 1;
	if (setjmp(g_ent_jmp) != 0) {
		g_ent_jmp_active = 0;
		return (ENT_RAISED);
	}
	rv = ref_getentropy(buf, len);
	g_ent_jmp_active = 0;
	return (rv);
}

struct guarded_buf {
	unsigned char pre[32];
	unsigned char data[BUFSZ];
	unsigned char post[32];
};

static ent_capture
snap_ent(int rv, const guarded_buf &g)
{
	ent_capture c;

	c.rv = rv;
	c.errno_copy = errno;
	c.calls = g_ent.calls;
	c.raised = (rv == ENT_RAISED) ? 1 : 0;
	std::memcpy(c.buf, g.data, sizeof(c.buf));
	std::memcpy(c.pre, g.pre, sizeof(c.pre));
	std::memcpy(c.post, g.post, sizeof(c.post));
	return (c);
}

static bool
ent_cap_eq(const ent_capture &a, const ent_capture &b)
{
	return (a.rv == b.rv && a.errno_copy == b.errno_copy &&
	    a.calls == b.calls && a.raised == b.raised &&
	    std::memcmp(a.buf, b.buf, sizeof(a.buf)) == 0 &&
	    std::memcmp(a.pre, b.pre, sizeof(a.pre)) == 0 &&
	    std::memcmp(a.post, b.post, sizeof(b.post)) == 0);
}

static void
init_guarded_buf(guarded_buf &g, const unsigned char *pat, size_t patlen)
{
	std::memset(&g, GUARD, sizeof(g));
	if (pat != nullptr && patlen > 0) {
		size_t n = patlen < BUFSZ ? patlen : BUFSZ;
		std::memcpy(g.data, pat, n);
	}
}

static void
do_getentropy(size_t len, int script, const unsigned char *pat, size_t patlen,
    const char *label)
{
	guarded_buf ga, gb;
	int rva, rvb;
	ent_capture ca, cb;

	rows[R_ENT].cases++;

	init_guarded_buf(ga, pat, patlen);
	init_guarded_buf(gb, pat, patlen);

	ent_mock_reset(script);
	g_raise_sig = 0;
	errno = 0;
	rva = call_port_getentropy(ga.data, len);
	ca = snap_ent(rva, ga);

	ent_mock_reset(script);
	g_raise_sig = 0;
	errno = 0;
	rvb = call_ref_getentropy(gb.data, len);
	cb = snap_ent(rvb, gb);

	if (!ent_cap_eq(ca, cb))
		fail_row(R_ENT, label, "mismatch");
}

/* ----------------------------------------------------------- edge matrices */

static void
edge_aio(void)
{
	static const int flags[] = {
		0,
		AIO_OP2_FOFFSET,
		AIO_OP2_VECTORED,
		AIO_OP2_FOFFSET | AIO_OP2_VECTORED,
		0x0003,
		0x0004,
		0x0005,
		0x0007,
		0x00ff,
		0x7fff,
		0x8000,
		0xffff,
		INT_MIN,
		INT_MAX,
		-1,
	};
	static const int seeds[] = {
		0, 1, 2, 0x10, 0x20, 0x32, 0x7f, 0x80, 0xff, INT_MIN, INT_MAX,
	};

	for (unsigned f = 0; f < sizeof(flags) / sizeof(flags[0]); f++) {
		for (unsigned s = 0; s < sizeof(seeds) / sizeof(seeds[0]); s++) {
			for (int sc = 0; sc < AIO_SCRIPT_COUNT; sc++)
				do_aio_write2(flags[f], sc, seeds[s], "edge");
		}
	}

	for (unsigned v = 0; v < 256u; v++)
		do_aio_write2((int)v, AIO_OK, (int)(v ^ 0x55), "byte-flag");
}

static void
edge_seek(void)
{
	static const long locs[] = {
		0L, 1L, -1L, 2L, 127L, 128L, 255L, 256L, 1023L, 1024L,
		0x7ffffffeL, 0x7fffffffL, 0x80000000L, 0xffffffffL, LONG_MIN,
		LONG_MAX,
	};

	for (unsigned t = 0; t < 2u; t++) {
		int threaded = (int)t;
		for (unsigned i = 0; i < sizeof(locs) / sizeof(locs[0]); i++)
			do_seekdir(locs[i], threaded, "edge");
	}
}

static void
edge_signal(void)
{
	static void (*handlers[])(int) = {
		SIG_DFL, SIG_IGN, SIG_ERR,
		reinterpret_cast<void (*)(int)>(0x1000),
		reinterpret_cast<void (*)(int)>(0xdeadbeef),
	};
	static const int sigs[] = {
		SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGABRT, SIGFPE, SIGKILL,
		SIGBUS, SIGSEGV, SIGPIPE, SIGALRM, SIGTERM, SIGUSR1, SIGUSR2,
		SIGCHLD, SIGCONT, SIGSTOP, SIGTSTP, SIGTTIN, SIGTTOU,
		1, 31, 32, 63, 64, 127, 128, 255,
	};

	for (unsigned s = 0; s < sizeof(sigs) / sizeof(sigs[0]); s++) {
		for (unsigned h = 0; h < sizeof(handlers) / sizeof(handlers[0]);
		    h++) {
			do_signal(sigs[s], handlers[h], false, -1,
			    reinterpret_cast<void (*)(int)>(0x5555),
			    "edge-restart");
			do_signal(sigs[s], handlers[h], true, -1,
			    reinterpret_cast<void (*)(int)>(0xaaaa),
			    "edge-norestart");
			do_signal(sigs[s], handlers[h], false, sigs[s],
			    reinterpret_cast<void (*)(int)>(0xbbbb),
			    "edge-fail");
		}
	}
}

static void
edge_getentropy(void)
{
	static const size_t lens[] = {
		0, 1, 2, 3, 4, 7, 8, 15, 16, 17, 31, 32, 63, 64, 65, 127,
		128, 129, 200, 255, 256, 257, 300, 511, 512,
	};
	unsigned char pat[300];

	for (unsigned i = 0; i < sizeof(pat); i++)
		pat[i] = (unsigned char)(0x80u + (i & 0x7fu));

	for (unsigned i = 0; i < sizeof(lens) / sizeof(lens[0]); i++) {
		for (int sc = 0; sc < ENT_SCRIPT_COUNT; sc++)
			do_getentropy(lens[i], sc, pat, lens[i], "edge");
	}

	for (unsigned v = 0; v < 256u; v++) {
		unsigned char b[4] = { (unsigned char)v, 0x00, (unsigned char)~v,
			0x7f };
		do_getentropy(16, ENT_OK_FULL, b, sizeof(b), "byte-pattern");
	}

	{
		unsigned char nulheavy[] = {
			0x00, 0x80, 0xff, 0x00, 0x7f, 0x00, 0x01, 0xfe,
		};
		for (size_t n = 0; n <= sizeof(nulheavy); n++)
			do_getentropy(32, ENT_PARTIAL_TWO, nulheavy, n,
			    "nul-heavy");
	}
}

/* -------------------------------------------------------------- randomise */

static int
rnd_aio_script(void)
{
	return ((int)(rnd32() % (uint32_t)AIO_SCRIPT_COUNT));
}

static int
rnd_ent_script(void)
{
	return ((int)(rnd32() % (uint32_t)ENT_SCRIPT_COUNT));
}

static int
rnd_aio_flags(void)
{
	uint32_t v = rnd32();

	switch (rnd32() % 6u) {
	case 0:
		return ((int)(v & 0x0003u));
	case 1:
		return ((int)v);
	case 2:
		return ((int)(v & 0x00ffu));
	case 3:
		return ((int)(1u << (rnd32() % 16u)));
	case 4:
		return ((int)((rnd32() & 1u) ? AIO_OP2_FOFFSET : 0) |
		    (int)((rnd32() & 1u) ? AIO_OP2_VECTORED : 0) |
		    (int)(rnd32() & 0xfffcu));
	default:
		return ((int)(rnd32() % 512u));
	}
}

static size_t
rnd_ent_len(void)
{
	uint32_t r = rnd32();

	switch (rnd32() % 8u) {
	case 0:
		return (0);
	case 1:
		return (1);
	case 2:
		return (GETENTROPY_MAX);
	case 3:
		return ((size_t)GETENTROPY_MAX + 1u + (r % 64u));
	case 4:
		return ((size_t)GETENTROPY_MAX - 1u);
	case 5:
		return ((size_t)GETENTROPY_MAX + 1u);
	case 6:
		return ((size_t)(r % 400u));
	default:
		return ((size_t)(r % 600u));
	}
}

static void
fill_pat(unsigned char *p, size_t n, uint64_t seed)
{
	for (size_t i = 0; i < n; i++) {
		uint64_t v = seed + i * 0x9e3779b97f4a7c15ULL;
		unsigned char c = (unsigned char)((v ^ (v >> 8)) & 0xffu);
		if (c == 0 && i + 1 < n)
			c = 0x80;
		p[i] = c;
	}
}

static void
sweep(void)
{
	unsigned char pat[320];

	for (long it = 0; it < SWEEP_ITERS; it++) {
		switch (rnd32() % 4u) {
		case 0:
			do_aio_write2(rnd_aio_flags(), rnd_aio_script(),
			    rnd_i(), "random");
			break;
		case 1:
			do_seekdir((long)rnd_i(), (int)(rnd32() & 1u),
			    "random");
			break;
		case 2: {
			int sig = rnd_i() & 0xff;
			void (*h)(int) = reinterpret_cast<void (*)(int)>(
			    (uintptr_t)(rnd64() | 1u));
			do_signal(sig, h, (rnd32() & 1u) != 0,
			    (rnd32() % 16u) == 0u ? sig : -1,
			    reinterpret_cast<void (*)(int)>(rnd64() | 1u),
			    "random");
			break;
		}
		default: {
			size_t n = rnd_ent_len();
			size_t plen = (size_t)(rnd32() % 200u);
			fill_pat(pat, plen, rnd64());
			do_getentropy(n, rnd_ent_script(), pat, plen,
			    "random");
			break;
		}
		}
	}
}

/* ------------------------------------------------------------------- main */

int
main(void)
{
	bufa = (unsigned char *)std::malloc(BUFSZ);
	bufb = (unsigned char *)std::malloc(BUFSZ);
	if (bufa == nullptr || bufb == nullptr) {
		std::fprintf(stderr, "out of memory\n");
		return (1);
	}

	edge_aio();
	edge_seek();
	edge_signal();
	edge_getentropy();
	sweep();

	long total_cases = 0;
	long total_failures = 0;

	std::printf("\n%-14s %12s %12s\n", "function", "cases", "failures");
	std::printf("----------------------------------------------\n");
	for (unsigned i = 0; i < sizeof(rows) / sizeof(rows[0]); i++) {
		std::printf("%-14s %12ld %12ld\n", rows[i].name, rows[i].cases,
		    rows[i].failures);
		total_cases += rows[i].cases;
		total_failures += rows[i].failures;
	}
	std::printf("----------------------------------------------\n");
	std::printf("%-14s %12ld %12ld\n", "TOTAL", total_cases,
	    total_failures);

	if (total_failures != 0) {
		std::printf("\nRESULT: FAIL (%ld mismatching cases)\n",
		    total_failures);
		return (1);
	}
	std::printf("\nRESULT: PASS\n");
	return (0);
}
