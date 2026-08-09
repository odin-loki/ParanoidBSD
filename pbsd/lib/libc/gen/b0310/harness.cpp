/*
 * harness.cpp -- differential test for PBSD batch b0310.
 *
 * Every function of the batch is driven twice per case: once through the
 * C++23 module port and once through the ref_ oracle compiled from the
 * original C.  The two sides are given separate, identically initialised
 * memory (filled with the guard byte 0x7f) so that any write outside the
 * nominal window shows up, and every externally observable effect --
 * return value, interposition-table dispatch, mutex operations, the
 * complete contents of the DIR block at the last instant it is still alive,
 * and the fd handed to _close() -- is recorded as an event and compared.
 *
 * The harness also supplies the libc environment both sides link against:
 * __libc_interposing[], __isthreaded, _reclaim_telldir(), the _pthread_*
 * wrappers and _close().
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <new>
#include <utility>

#include <pthread.h>
#include <sys/types.h>

import pbsd.lib.libc.gen.b0310;

namespace P = pbsd::lib_libc_gen::b0310;

/* ------------------------------------------------------------------ */
/* the oracle                                                          */
/* ------------------------------------------------------------------ */

extern "C" {
long ref__atomic_lock_stub(volatile long *);
long ref__atomic_lock(volatile long *);
void ref__spinlock(void *);
void ref__spinunlock(void *);
void ref___libc_spinlock_stub(void *);
void ref___libc_spinunlock_stub(void *);
int ref_fdclosedir(void *);
int ref_closedir(void *);
}

/* ------------------------------------------------------------------ */
/* recorded events                                                     */
/* ------------------------------------------------------------------ */

enum EvKind {
	EV_LOCK = 1,
	EV_UNLOCK,
	EV_DESTROY,
	EV_RECLAIM,
	EV_CLOSE,
	EV_SLOT
};

static constexpr std::size_t DIRSZ = sizeof(P::DIR);
static constexpr std::size_t GUARD = 32;
static constexpr std::size_t BLKSZ = DIRSZ + GUARD;
static constexpr std::size_t SNAPMAX = 256;
static_assert(BLKSZ <= SNAPMAX, "DIR block does not fit in a snapshot");

struct Ev {
	int kind;
	long a;
	long b;
	int snaplen;
	unsigned char snap[SNAPMAX];
};

struct Run {
	long rv;
	int nev;
	int overflow;
	Ev ev[16];
};

static Run g_cur;

static void
push_ev(int kind, long a, long b)
{
	if (g_cur.nev >= (int)(sizeof(g_cur.ev) / sizeof(g_cur.ev[0]))) {
		g_cur.overflow = 1;
		return;
	}
	Ev &e = g_cur.ev[g_cur.nev++];
	std::memset(&e, 0, sizeof(e));
	e.kind = kind;
	e.a = a;
	e.b = b;
}

/* ------------------------------------------------------------------ */
/* pointer symbolisation: raw addresses differ between the two sides,  */
/* so every pointer is reported as an offset from a known base         */
/* ------------------------------------------------------------------ */

static const unsigned char *g_blk;
static std::size_t g_blk_sz;
static const void *g_buf;
static const void *g_compat;
static const void *g_td;
static const unsigned char *g_spin;
static std::size_t g_spin_sz;

static long
symref(const void *p)
{
	const unsigned char *q = static_cast<const unsigned char *>(p);

	if (p == nullptr)
		return (0);
	if (g_blk != nullptr && q >= g_blk && q < g_blk + g_blk_sz)
		return (1000 + (q - g_blk));
	if (g_spin != nullptr && q >= g_spin && q < g_spin + g_spin_sz)
		return (5000 + (q - g_spin));
	if (p == g_buf)
		return (2000);
	if (p == g_compat)
		return (3000);
	if (p == g_td)
		return (4000);
	return (-1);
}

static void
poke_sym(unsigned char *snap, std::size_t off)
{
	void *p;
	long v;

	std::memcpy(&p, snap + off, sizeof(p));
	v = symref(p);
	std::memcpy(snap + off, &v, sizeof(v));
}

/* ------------------------------------------------------------------ */
/* the libc environment shared by port and oracle                      */
/* ------------------------------------------------------------------ */

extern "C" {

P::interpos_func_t __libc_interposing[16];
int __isthreaded = 0;

void
_reclaim_telldir(P::DIR *dirp)
{
	push_ev(EV_RECLAIM, symref(dirp), 0);
	if (g_cur.overflow || g_cur.nev == 0)
		return;
	Ev &e = g_cur.ev[g_cur.nev - 1];
	if (dirp != nullptr &&
	    reinterpret_cast<const unsigned char *>(dirp) == g_blk) {
		e.snaplen = (int)g_blk_sz;
		std::memcpy(e.snap, dirp, g_blk_sz);
		poke_sym(e.snap, offsetof(P::DIR, dd_buf));
		poke_sym(e.snap, offsetof(P::DIR, dd_td));
		poke_sym(e.snap, offsetof(P::DIR, dd_compat_de));
	}
}

int
_pthread_mutex_lock(pthread_mutex_t *m)
{
	push_ev(EV_LOCK, symref(m), 0);
	return (pthread_mutex_lock(m));
}

int
_pthread_mutex_unlock(pthread_mutex_t *m)
{
	push_ev(EV_UNLOCK, symref(m), 0);
	return (pthread_mutex_unlock(m));
}

int
_pthread_mutex_destroy(pthread_mutex_t *m)
{
	push_ev(EV_DESTROY, symref(m), 0);
	return (pthread_mutex_destroy(m));
}

/*
 * Stand-in for libc's close(2) wrapper.  Deterministic and injective in fd
 * so that any corruption of the descriptor closedir() forwards is visible
 * both in the event log and in the return value.
 */
int
_close(int fd)
{
	push_ev(EV_CLOSE, fd, 0);
	return ((int)((static_cast<unsigned>(fd) * 2654435761u) >> 8));
}

} /* extern "C" */

/*
 * Every slot of the interposition table gets its own recorder, so that a
 * dispatch through the wrong index is caught instead of crashing.
 */
static void
install_interposing(void)
{
	[]<int... I>(std::integer_sequence<int, I...>) {
		((__libc_interposing[I] = reinterpret_cast<P::interpos_func_t>(
		    +[](void *p) { push_ev(EV_SLOT, I, symref(p)); })), ...);
	}(std::make_integer_sequence<int, 16>{});
}

/* ------------------------------------------------------------------ */
/* bookkeeping                                                         */
/* ------------------------------------------------------------------ */

struct Result {
	const char *name;
	long cases;
	long failures;
};

enum {
	F_ATOMIC_LOCK_STUB,
	F_ATOMIC_LOCK,
	F_SPINLOCK,
	F_SPINUNLOCK,
	F_LIBC_SPINLOCK_STUB,
	F_LIBC_SPINUNLOCK_STUB,
	F_FDCLOSEDIR,
	F_CLOSEDIR,
	F_COUNT
};

static Result g_res[F_COUNT] = {
	{ "_atomic_lock_stub", 0, 0 },
	{ "_atomic_lock", 0, 0 },
	{ "_spinlock", 0, 0 },
	{ "_spinunlock", 0, 0 },
	{ "__libc_spinlock_stub", 0, 0 },
	{ "__libc_spinunlock_stub", 0, 0 },
	{ "fdclosedir", 0, 0 },
	{ "closedir", 0, 0 },
};

static void
fail(int fn, const char *what)
{
	g_res[fn].failures++;
	if (g_res[fn].failures <= 5)
		std::printf("FAIL %-24s case %ld: %s\n", g_res[fn].name,
		    g_res[fn].cases, what);
}

static bool
runs_equal(const Run &a, const Run &b, const char *&why)
{
	if (a.rv != b.rv) {
		why = "return value differs";
		return (false);
	}
	if (a.overflow != b.overflow) {
		why = "event overflow differs";
		return (false);
	}
	if (a.nev != b.nev) {
		why = "number of recorded calls differs";
		return (false);
	}
	for (int i = 0; i < a.nev; i++) {
		const Ev &x = a.ev[i];
		const Ev &y = b.ev[i];
		if (x.kind != y.kind) {
			why = "recorded call kind differs";
			return (false);
		}
		if (x.a != y.a || x.b != y.b) {
			why = "recorded call argument differs";
			return (false);
		}
		if (x.snaplen != y.snaplen) {
			why = "snapshot length differs";
			return (false);
		}
		if (x.snaplen != 0 &&
		    std::memcmp(x.snap, y.snap, (std::size_t)x.snaplen) != 0) {
			why = "DIR block contents differ";
			return (false);
		}
	}
	return (true);
}

/* ------------------------------------------------------------------ */
/* fixed-seed PRNG                                                     */
/* ------------------------------------------------------------------ */

static std::uint64_t g_rs = 0x0123456789abcdefULL;

static std::uint64_t
nextr(void)
{
	g_rs ^= g_rs << 13;
	g_rs ^= g_rs >> 7;
	g_rs ^= g_rs << 17;
	return (g_rs);
}

/* ------------------------------------------------------------------ */
/* _atomic_lock_stub / _atomic_lock                                    */
/* ------------------------------------------------------------------ */

static constexpr std::size_t LBUF = 64;

struct LockRun {
	long rv;
	unsigned char buf[LBUF];
};

static void
run_atomic(bool port, bool alias, long value, int off, bool use_null,
    LockRun &r)
{
	std::memset(r.buf, 0x7f, sizeof(r.buf));
	std::memcpy(r.buf + off, &value, sizeof(value));

	volatile long *arg = use_null ? nullptr :
	    reinterpret_cast<volatile long *>(r.buf + off);

	if (alias)
		r.rv = port ? P::_atomic_lock(arg) : ref__atomic_lock(arg);
	else
		r.rv = port ? P::_atomic_lock_stub(arg) :
		    ref__atomic_lock_stub(arg);
}

static void
one_atomic(bool alias, long value, int off, bool use_null)
{
	LockRun a, b;
	int fn = alias ? F_ATOMIC_LOCK : F_ATOMIC_LOCK_STUB;

	run_atomic(true, alias, value, off, use_null, a);
	run_atomic(false, alias, value, off, use_null, b);

	if (a.rv != b.rv)
		fail(fn, "return value differs");
	else if (std::memcmp(a.buf, b.buf, sizeof(a.buf)) != 0)
		fail(fn, "buffer differs");
	g_res[fn].cases++;
}

static void
test_atomic(void)
{
	static const long vals[] = {
		0L, 1L, -1L, 2L, -2L, 0x7fL, (long)0x80L, (long)0xffL,
		(long)0x8080808080808080ULL, (long)0x7f7f7f7f7f7f7f7fULL,
		LONG_MIN, LONG_MAX, LONG_MIN + 1, LONG_MAX - 1
	};
	const int nvals = (int)(sizeof(vals) / sizeof(vals[0]));

	for (int alias = 0; alias < 2; alias++) {
		for (int i = 0; i < nvals; i++)
			for (int off = 0; off + 8 <= (int)LBUF; off += 8)
				one_atomic(alias != 0, vals[i], off, false);
		one_atomic(alias != 0, 0L, 0, true);
		one_atomic(alias != 0, -1L, 0, true);

		for (long i = 0; i < 50000; i++) {
			std::uint64_t r = nextr();
			long v = (long)(r ^ (nextr() << 1));
			if ((r & 7) == 0)
				v = vals[(r >> 3) % nvals];
			int off = (int)((r >> 8) % (LBUF / 8)) * 8;
			bool nul = ((r >> 20) & 63) == 0;
			one_atomic(alias != 0, v, off, nul);
		}
	}
}

/* ------------------------------------------------------------------ */
/* _spinlock / _spinunlock / __libc_spinlock_stub /                    */
/* __libc_spinunlock_stub                                              */
/* ------------------------------------------------------------------ */

static constexpr std::size_t SBUF = 128;

enum SpinFn {
	SF_SPINLOCK,
	SF_SPINUNLOCK,
	SF_LOCKSTUB,
	SF_UNLOCKSTUB
};

static const int spin_fn_res[] = {
	F_SPINLOCK, F_SPINUNLOCK, F_LIBC_SPINLOCK_STUB, F_LIBC_SPINUNLOCK_STUB
};

struct SpinRun {
	unsigned char buf[SBUF];
	Run run;
};

static void
run_spin(bool port, SpinFn f, int off, bool use_null, const unsigned char *fill,
    SpinRun &r)
{
	std::memset(r.buf, 0x7f, sizeof(r.buf));
	std::memcpy(r.buf, fill, sizeof(r.buf) / 2);

	g_spin = r.buf;
	g_spin_sz = sizeof(r.buf);
	std::memset(&g_cur, 0, sizeof(g_cur));

	P::spinlock_t *arg = use_null ? nullptr :
	    reinterpret_cast<P::spinlock_t *>(r.buf + off);
	void *varg = static_cast<void *>(arg);

	switch (f) {
	case SF_SPINLOCK:
		if (port)
			P::_spinlock(arg);
		else
			ref__spinlock(varg);
		break;
	case SF_SPINUNLOCK:
		if (port)
			P::_spinunlock(arg);
		else
			ref__spinunlock(varg);
		break;
	case SF_LOCKSTUB:
		if (port)
			P::__libc_spinlock_stub(arg);
		else
			ref___libc_spinlock_stub(varg);
		break;
	case SF_UNLOCKSTUB:
		if (port)
			P::__libc_spinunlock_stub(arg);
		else
			ref___libc_spinunlock_stub(varg);
		break;
	}

	r.run = g_cur;
	g_spin = nullptr;
	g_spin_sz = 0;
}

static void
one_spin(SpinFn f, int off, bool use_null, const unsigned char *fill)
{
	static SpinRun a, b;
	int fn = spin_fn_res[f];
	const char *why = "";

	run_spin(true, f, off, use_null, fill, a);
	run_spin(false, f, off, use_null, fill, b);

	if (!runs_equal(a.run, b.run, why))
		fail(fn, why);
	else if (std::memcmp(a.buf, b.buf, sizeof(a.buf)) != 0)
		fail(fn, "spinlock buffer differs");
	g_res[fn].cases++;
}

static void
test_spin(void)
{
	unsigned char fill[SBUF];

	for (int f = 0; f < 4; f++) {
		for (unsigned i = 0; i < SBUF; i++)
			fill[i] = (unsigned char)(i * 7 + 0x80);
		for (int off = 0; off + (int)sizeof(P::spinlock_t) <= (int)SBUF;
		    off += 8)
			one_spin((SpinFn)f, off, false, fill);
		std::memset(fill, 0x00, sizeof(fill));
		one_spin((SpinFn)f, 0, false, fill);
		std::memset(fill, 0xff, sizeof(fill));
		one_spin((SpinFn)f, 0, false, fill);
		one_spin((SpinFn)f, 0, true, fill);

		for (long i = 0; i < 50000; i++) {
			std::uint64_t r = nextr();
			for (unsigned j = 0; j < SBUF; j++) {
				if ((j % 8) == 0)
					r = nextr();
				fill[j] = (unsigned char)(r >> ((j % 8) * 8));
			}
			int off = (int)((r >> 3) %
			    ((SBUF - sizeof(P::spinlock_t)) / 8 + 1)) * 8;
			bool nul = ((r >> 11) & 63) == 0;
			one_spin((SpinFn)f, off, nul, fill);
		}
	}
}

/* ------------------------------------------------------------------ */
/* fdclosedir / closedir                                               */
/* ------------------------------------------------------------------ */

struct DirCase {
	int fd;
	long loc;
	long size;
	int len;
	long long seek;
	int flags;
	int has_buf;
	int has_compat;
	int has_td;
	int threaded;
};

static const pthread_mutex_t g_mtx_init = PTHREAD_MUTEX_INITIALIZER;
static unsigned char g_td_storage[64];

static void
run_dir(bool port, const DirCase &c, bool via_closedir, Run &out)
{
	unsigned char *blk = static_cast<unsigned char *>(std::malloc(BLKSZ));
	void *buf = c.has_buf ? std::malloc(64) : nullptr;
	void *cde = c.has_compat ? std::malloc(48) : nullptr;

	std::memset(blk, 0x7f, BLKSZ);
	if (buf != nullptr)
		std::memset(buf, 0x7f, 64);
	if (cde != nullptr)
		std::memset(cde, 0x7f, 48);

	P::DIR *d = new (blk) P::DIR;
	d->dd_fd = c.fd;
	d->dd_loc = c.loc;
	d->dd_size = c.size;
	d->dd_buf = static_cast<char *>(buf);
	d->dd_len = c.len;
	d->dd_seek = (off_t)c.seek;
	d->dd_flags = c.flags;
	std::memcpy(static_cast<void *>(&d->dd_lock), &g_mtx_init,
	    sizeof(pthread_mutex_t));
	d->dd_td = c.has_td ?
	    reinterpret_cast<P::_telldir *>(g_td_storage) : nullptr;
	d->dd_compat_de = static_cast<P::freebsd11_dirent *>(cde);

	g_blk = blk;
	g_blk_sz = BLKSZ;
	g_buf = buf;
	g_compat = cde;
	g_td = g_td_storage;
	std::memset(&g_cur, 0, sizeof(g_cur));
	__isthreaded = c.threaded;

	long rv;
	if (via_closedir)
		rv = port ? P::closedir(d) :
		    ref_closedir(reinterpret_cast<P::DIR *>(blk));
	else
		rv = port ? P::fdclosedir(d) :
		    ref_fdclosedir(reinterpret_cast<P::DIR *>(blk));

	__isthreaded = 0;
	g_cur.rv = rv;
	out = g_cur;

	g_blk = nullptr;
	g_blk_sz = 0;
	g_buf = g_compat = g_td = nullptr;
}

static void
one_dir(const DirCase &c, bool via_closedir)
{
	static Run a, b;
	int fn = via_closedir ? F_CLOSEDIR : F_FDCLOSEDIR;
	const char *why = "";

	run_dir(true, c, via_closedir, a);
	run_dir(false, c, via_closedir, b);

	if (!runs_equal(a, b, why))
		fail(fn, why);
	else if (a.nev == 0)
		fail(fn, "no observable effect recorded");
	g_res[fn].cases++;
}

static void
test_dir(bool via_closedir)
{
	static const int fds[] = {
		INT_MIN, INT_MIN + 1, -3, -2, -1, 0, 1, 2, 3, 255,
		INT_MAX - 1, INT_MAX
	};
	static const long locs[] = { LONG_MIN, -1L, 0L, 1L, LONG_MAX };
	const int nfds = (int)(sizeof(fds) / sizeof(fds[0]));
	const int nlocs = (int)(sizeof(locs) / sizeof(locs[0]));
	DirCase c;

	for (int i = 0; i < nfds; i++) {
		for (int j = 0; j < nlocs; j++) {
			for (int m = 0; m < 16; m++) {
				c.fd = fds[i];
				c.loc = locs[j];
				c.size = (long)0x7f7f7f7f7f7f7f7fULL;
				c.len = 0x0a0b0c0d;
				c.seek = (long long)0x8080808080808080ULL;
				c.flags = -1;
				c.has_buf = (m >> 0) & 1;
				c.has_compat = (m >> 1) & 1;
				c.has_td = (m >> 2) & 1;
				c.threaded = (m >> 3) & 1;
				one_dir(c, via_closedir);
			}
		}
	}

	for (long i = 0; i < 50000; i++) {
		std::uint64_t r = nextr();

		c.fd = ((r & 3) == 0) ? fds[(r >> 2) % nfds] :
		    (int)(std::uint32_t)nextr();
		c.loc = ((r & 12) == 0) ? locs[(r >> 4) % nlocs] :
		    (long)nextr();
		c.size = (long)nextr();
		c.len = (int)(std::uint32_t)nextr();
		c.seek = (long long)nextr();
		c.flags = (int)(std::uint32_t)nextr();
		c.has_buf = (int)((r >> 32) & 1);
		c.has_compat = (int)((r >> 33) & 1);
		c.has_td = (int)((r >> 34) & 1);
		c.threaded = (int)((r >> 35) & 1);
		one_dir(c, via_closedir);
	}
}

/* ------------------------------------------------------------------ */

/*
 * Guard against the slot recorders collapsing into one closure type: if they
 * did, a dispatch through the wrong interposition slot would go unnoticed.
 */
static int
check_interposing(void)
{
	for (int i = 0; i < 16; i++) {
		std::memset(&g_cur, 0, sizeof(g_cur));
		(reinterpret_cast<void (*)(void *)>(__libc_interposing[i]))(
		    nullptr);
		if (g_cur.nev != 1 || g_cur.ev[0].kind != EV_SLOT ||
		    g_cur.ev[0].a != i) {
			std::printf("harness self-test failed: interposition "
			    "slot %d is not distinctly recorded\n", i);
			return (1);
		}
	}
	std::memset(&g_cur, 0, sizeof(g_cur));
	return (0);
}

int
main(void)
{
	install_interposing();
	if (check_interposing() != 0)
		return (1);

	test_atomic();
	test_spin();
	test_dir(false);
	test_dir(true);

	long total_cases = 0, total_fail = 0;

	std::printf("\n%-24s %10s %10s\n", "function", "cases", "failures");
	std::printf("%-24s %10s %10s\n", "------------------------",
	    "----------", "----------");
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-24s %10ld %10ld\n", g_res[i].name,
		    g_res[i].cases, g_res[i].failures);
		total_cases += g_res[i].cases;
		total_fail += g_res[i].failures;
	}
	std::printf("%-24s %10s %10s\n", "------------------------",
	    "----------", "----------");
	std::printf("%-24s %10ld %10ld\n", "TOTAL", total_cases, total_fail);
	std::printf("\n%s\n", total_fail == 0 ? "PASS" : "FAIL");

	return (total_fail == 0 ? 0 : 1);
}
