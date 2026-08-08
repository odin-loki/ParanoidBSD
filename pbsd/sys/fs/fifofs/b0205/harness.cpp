/*
 * PBSD batch b0205 -- differential test for sys/fs/fifofs/fifo_vnops.c.
 *
 * Every function is driven twice per case: once through the ref_ oracle
 * (oracle.c, unmodified bodies) and once through the C++23 port.  The two
 * sides get their own private object pool, and everything they can possibly
 * observe or mutate is compared:
 *
 *   - the return value;
 *   - every field of every kernel object afterwards (fifoinfo counters and
 *     generations, pipe_state, pipe_wgen, f_flag, f_pipegen, f_ops) -- pointer
 *     valued fields are canonicalised to (region, offset) pairs so that the
 *     two sides' distinct addresses stay comparable and a wrong-object write
 *     (&fi_readers vs &fi_writers) still shows up as a different offset;
 *   - the full ordered trace of kernel primitive calls (locks, wakeups,
 *     msleep, malloc/free, pipe ctor/dtor, finit, ...) with canonicalised
 *     arguments, so a lost PIPE_UNLOCK or a swapped wakeup channel fails;
 *   - the exact bytes printf() produced;
 *   - the guard bytes.  Each object is carved out of a pool pre-filled with
 *     the guard byte 0x7f and separated by 32 byte gaps; after the call every
 *     gap plus the pool tail must still be all 0x7f on both sides, so a write
 *     past the end of any object is caught even when the return value and the
 *     nominal fields match.
 */

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <climits>

import pbsd.sys.fs.fifofs.b0205;

namespace P = pbsd::sys_fs_fifofs::b0205;

/* ------------------------------------------------------------------ */
/* Mirror of the model types oracle.c declares, for the ref side.      */
/* ------------------------------------------------------------------ */
namespace refm {

using u_int = unsigned int;

struct fifoinfo;

struct pipe {
	u_int	pipe_state;
	int	pipe_wgen;
};

struct file {
	u_int		f_flag;
	int		f_pipegen;
	const void	*f_ops;
};

struct vnode {
	struct fifoinfo	*v_fifoinfo;
};

struct thread {
	int	td_tid;
};

struct flock {
	long	l_start;
	long	l_len;
	int	l_pid;
	short	l_type;
	short	l_whence;
};

struct vop_open_args {
	struct vnode	*a_vp;
	int		a_mode;
	struct thread	*a_td;
	struct file	*a_fp;
};

struct vop_close_args {
	struct vnode	*a_vp;
	int		a_fflag;
	struct thread	*a_td;
};

struct vop_print_args {
	struct vnode	*a_vp;
};

struct vop_advlock_args {
	struct vnode	*a_vp;
	void		*a_id;
	int		a_op;
	struct flock	*a_fl;
	int		a_flags;
};

struct fifoinfo {
	struct pipe *fi_pipe;
	long	fi_readers;
	long	fi_writers;
	u_int	fi_rgen;
	u_int	fi_wgen;
};

} /* namespace refm */

extern "C" {
void	ref_fifo_cleanup(refm::vnode *vp);
int	ref_fifo_open(refm::vop_open_args *ap);
int	ref_fifo_close(refm::vop_close_args *ap);
int	ref_fifo_printinfo(refm::vnode *vp);
int	ref_fifo_print(refm::vop_print_args *ap);
int	ref_fifo_advlock(refm::vop_advlock_args *ap);
}

/* The two models must be laid out identically for the pools to match. */
static_assert(sizeof(refm::fifoinfo) == sizeof(P::fifoinfo));
static_assert(sizeof(refm::pipe) == sizeof(P::pipe));
static_assert(sizeof(refm::file) == sizeof(P::file));
static_assert(sizeof(refm::vnode) == sizeof(P::vnode));
static_assert(sizeof(refm::vop_open_args) == sizeof(P::vop_open_args));
static_assert(offsetof(refm::fifoinfo, fi_pipe) == offsetof(P::fifoinfo, fi_pipe));
static_assert(offsetof(refm::fifoinfo, fi_readers) == offsetof(P::fifoinfo, fi_readers));
static_assert(offsetof(refm::fifoinfo, fi_writers) == offsetof(P::fifoinfo, fi_writers));
static_assert(offsetof(refm::fifoinfo, fi_rgen) == offsetof(P::fifoinfo, fi_rgen));
static_assert(offsetof(refm::fifoinfo, fi_wgen) == offsetof(P::fifoinfo, fi_wgen));
static_assert(offsetof(refm::pipe, pipe_wgen) == offsetof(P::pipe, pipe_wgen));
static_assert(offsetof(refm::file, f_pipegen) == offsetof(P::file, f_pipegen));
static_assert(offsetof(refm::file, f_ops) == offsetof(P::file, f_ops));

/* Layout twin used by the hooks, which are type agnostic. */
struct TwinFifo {
	void		*fi_pipe;
	long		fi_readers;
	long		fi_writers;
	unsigned int	fi_rgen;
	unsigned int	fi_wgen;
};
struct TwinPipe {
	unsigned int	pipe_state;
	int		pipe_wgen;
};
static_assert(sizeof(TwinFifo) == sizeof(refm::fifoinfo));
static_assert(offsetof(TwinFifo, fi_writers) == offsetof(refm::fifoinfo, fi_writers));
static_assert(offsetof(TwinFifo, fi_wgen) == offsetof(refm::fifoinfo, fi_wgen));
static_assert(sizeof(TwinPipe) == sizeof(refm::pipe));

/* ------------------------------------------------------------------ */
/* Kernel constants (same values as oracle.c and port.cppm).           */
/* ------------------------------------------------------------------ */
enum : int {
	K_EINVAL = 22,
	K_ENXIO = 6,
	K_EINTR = 4,
	K_FREAD = 0x0001,
	K_FWRITE = 0x0002,
	K_O_NONBLOCK = 0x0004,
	K_FEXEC = 0x00040000,
	K_F_FLOCK = 0x020,
	K_PIPE_ASYNC = 0x004,
	K_PIPE_WANTR = 0x008,
	K_PIPE_WANTW = 0x010,
	K_PIPE_WANT = 0x020,
	K_PIPE_SEL = 0x040,
	K_PIPE_EOF = 0x080,
	K_PIPE_DIRECTW = 0x400
};

/* ------------------------------------------------------------------ */
/* Per-side world: object pool, symbol regions, trace, print capture.  */
/* ------------------------------------------------------------------ */
enum {
	R_NONE = 0,
	R_VP,
	R_FIP0,
	R_FIP1,
	R_PIPE0,
	R_PIPE1,
	R_FP,
	R_TD,
	R_ARGS,
	R_MT,
	R_PIPEOPS,
	R_BADFILEOPS,
	R_MAX
};

static const char *const region_name[R_MAX] = {
	"null", "vp", "fip", "newfip", "pipe", "newpipe", "fp", "td", "args",
	"M_VNODE", "pipeops", "badfileops"
};

enum {
	E_ASSERT_ELOCKED = 1,
	E_PIPE_DTOR,
	E_FREE,
	E_CTOR,
	E_MALLOC,
	E_PIPE_LOCK,
	E_PIPE_UNLOCK,
	E_WAKEUP,
	E_SELWAKEUP,
	E_VOP_UNLOCK,
	E_VN_LOCK,
	E_SIGDEFERSTOP,
	E_SIGALLOWSTOP,
	E_MSLEEP,
	E_FINIT,
	E_PRINTF,
	E_STDADVLOCK,
	E_OP_MAX
};

static const char *const ev_name[E_OP_MAX] = {
	"?", "ASSERT_VOP_ELOCKED", "pipe_dtor", "free", "pipe_named_ctor",
	"malloc", "PIPE_LOCK", "PIPE_UNLOCK", "wakeup", "pipeselwakeup",
	"VOP_UNLOCK", "vn_lock", "sigdeferstop", "sigallowstop", "msleep",
	"finit", "printf", "vop_stdadvlock"
};

constexpr int MAXEV = 128;
constexpr int POOL = 768;
constexpr int OUTMAX = 512;
constexpr unsigned char GUARD = 0x7f;

struct Ev {
	int	op;
	long	a[5];
};

/* What the scripted kernel primitives should do for the case at hand. */
struct Script {
	int		ctor_err;
	unsigned int	ctor_pipe_state;
	int		ctor_pipe_wgen;
	int		sigdefer_ret;
	int		advlock_ret;
	int		msleep_err[2];
	long		d_readers[2];
	long		d_writers[2];
	int		d_rgen[2];
	int		d_wgen[2];
	unsigned int	or_state[2];
	unsigned int	clr_state[2];
	int		d_pwgen[2];
};

struct GRange {
	unsigned short off;
	unsigned short len;
};

struct Region {
	const unsigned char	*base;
	unsigned long		size;
};

struct World {
	alignas(16) unsigned char pool[POOL];
	size_t		used;
	size_t		last_end;
	GRange		gr[24];
	int		ngr;
	int		nalloc;
	Region		regs[R_MAX];
	int		nev;
	int		ev_overflow;
	Ev		ev[MAXEV];
	char		out[OUTMAX];
	int		outlen;
	int		out_overflow;
	const Script	*scr;
	int		msleep_n;
};

static World g_ref_world;
static World g_port_world;
static World *g_w;
static unsigned char guardref[POOL];

extern "C" {
int pbsd_M_VNODE_obj = 0;
int pbsd_pipeops_obj = 0;
int pbsd_badfileops_obj = 0;
}

static void
world_reset(World &w, const Script *scr)
{
	std::memset(w.pool, GUARD, POOL);
	w.used = 32;
	w.last_end = 0;
	w.ngr = 0;
	w.nalloc = 0;
	std::memset(w.regs, 0, sizeof(w.regs));
	w.nev = 0;
	w.ev_overflow = 0;
	w.outlen = 0;
	w.out_overflow = 0;
	std::memset(w.out, 0, sizeof(w.out));
	w.scr = scr;
	w.msleep_n = 0;
	w.regs[R_MT].base = reinterpret_cast<const unsigned char *>(&pbsd_M_VNODE_obj);
	w.regs[R_MT].size = sizeof(int);
	w.regs[R_PIPEOPS].base = reinterpret_cast<const unsigned char *>(&pbsd_pipeops_obj);
	w.regs[R_PIPEOPS].size = sizeof(int);
	w.regs[R_BADFILEOPS].base =
	    reinterpret_cast<const unsigned char *>(&pbsd_badfileops_obj);
	w.regs[R_BADFILEOPS].size = sizeof(int);
}

static void *
wnew(World &w, size_t sz, int rid)
{
	size_t start = (w.used + 15) & ~static_cast<size_t>(15);

	if (start + sz > POOL) {
		std::fprintf(stderr, "harness: object pool exhausted\n");
		std::exit(2);
	}
	if (start > w.last_end && w.ngr < 24) {
		w.gr[w.ngr].off = static_cast<unsigned short>(w.last_end);
		w.gr[w.ngr].len = static_cast<unsigned short>(start - w.last_end);
		w.ngr++;
	}
	w.regs[rid].base = w.pool + start;
	w.regs[rid].size = sz;
	w.last_end = start + sz;
	w.used = start + sz + 32;
	w.nalloc++;
	return w.pool + start;
}

static int
guards_ok(const World &w)
{
	for (int i = 0; i < w.ngr; i++) {
		if (std::memcmp(w.pool + w.gr[i].off, guardref, w.gr[i].len) != 0)
			return 0;
	}
	if (w.last_end < POOL &&
	    std::memcmp(w.pool + w.last_end, guardref, POOL - w.last_end) != 0)
		return 0;
	return 1;
}

static long
sym(const void *p)
{
	if (p == nullptr)
		return 0;
	const unsigned char *q = static_cast<const unsigned char *>(p);
	for (int i = 1; i < R_MAX; i++) {
		if (g_w->regs[i].base == nullptr)
			continue;
		if (q >= g_w->regs[i].base &&
		    q < g_w->regs[i].base + g_w->regs[i].size)
			return static_cast<long>(i) * 4096 +
			    (q - g_w->regs[i].base);
	}
	return -1;
}

static void
sym_str(long s, char *buf, size_t n)
{
	if (s == 0) {
		std::snprintf(buf, n, "NULL");
	} else if (s < 0) {
		std::snprintf(buf, n, "<foreign>");
	} else {
		long r = s / 4096, off = s % 4096;
		std::snprintf(buf, n, "%s+%ld",
		    (r > 0 && r < R_MAX) ? region_name[r] : "?", off);
	}
}

static long
strid(const char *s)
{
	if (s == nullptr)
		return 0;
	unsigned long h = 1469598103934665603UL;
	while (*s != '\0') {
		h ^= static_cast<unsigned char>(*s++);
		h *= 1099511628211UL;
	}
	return static_cast<long>(h & 0xffffffUL);
}

static void
push(int op, long a0 = 0, long a1 = 0, long a2 = 0, long a3 = 0, long a4 = 0)
{
	World &w = *g_w;

	if (w.nev >= MAXEV) {
		w.ev_overflow = 1;
		return;
	}
	w.ev[w.nev].op = op;
	w.ev[w.nev].a[0] = a0;
	w.ev[w.nev].a[1] = a1;
	w.ev[w.nev].a[2] = a2;
	w.ev[w.nev].a[3] = a3;
	w.ev[w.nev].a[4] = a4;
	w.nev++;
}

/* ------------------------------------------------------------------ */
/* The modelled kernel primitives, shared by oracle and port.          */
/* ------------------------------------------------------------------ */
extern "C" {

int
pbsd_kprintf(const char *fmt, ...)
{
	World &w = *g_w;
	char tmp[256];
	va_list ap;

	va_start(ap, fmt);
	int n = std::vsnprintf(tmp, sizeof(tmp), fmt, ap);
	va_end(ap);
	if (n < 0 || n >= static_cast<int>(sizeof(tmp)))
		w.out_overflow = 1;
	int len = (n < 0) ? 0 : n;
	if (len > static_cast<int>(sizeof(tmp)) - 1)
		len = static_cast<int>(sizeof(tmp)) - 1;
	if (w.outlen + len >= OUTMAX) {
		w.out_overflow = 1;
		len = OUTMAX - 1 - w.outlen;
		if (len < 0)
			len = 0;
	}
	std::memcpy(w.out + w.outlen, tmp, static_cast<size_t>(len));
	w.outlen += len;
	w.out[w.outlen] = '\0';
	push(E_PRINTF, n, strid(fmt));
	return n;
}

void *
pbsd_kmalloc(unsigned long size, void *mtp, int flags)
{
	World &w = *g_w;
	void *p = wnew(w, size, R_FIP1);

	if ((flags & 0x0100) != 0)		/* M_ZERO */
		std::memset(p, 0, size);
	push(E_MALLOC, static_cast<long>(size), sym(mtp), flags, sym(p));
	return p;
}

void
pbsd_kfree(void *addr, void *mtp)
{
	/*
	 * Deliberately does not release the storage: the final contents of a
	 * freed fifoinfo are still compared between the two sides.
	 */
	push(E_FREE, sym(addr), sym(mtp));
}

int
pbsd_pipe_named_ctor(void **pp, void *td)
{
	World &w = *g_w;
	TwinPipe *np = static_cast<TwinPipe *>(wnew(w, sizeof(TwinPipe), R_PIPE1));

	np->pipe_state = w.scr->ctor_pipe_state;
	np->pipe_wgen = w.scr->ctor_pipe_wgen;
	*pp = np;
	push(E_CTOR, sym(td), w.scr->ctor_err, sym(np));
	return w.scr->ctor_err;
}

void
pbsd_pipe_dtor(void *cpipe)
{
	push(E_PIPE_DTOR, sym(cpipe));
}

void
pbsd_pipe_lock(void *cpipe)
{
	push(E_PIPE_LOCK, sym(cpipe));
}

void
pbsd_pipe_unlock(void *cpipe)
{
	push(E_PIPE_UNLOCK, sym(cpipe));
}

void
pbsd_wakeup(void *chan)
{
	push(E_WAKEUP, sym(chan));
}

void
pbsd_pipeselwakeup(void *cpipe)
{
	push(E_SELWAKEUP, sym(cpipe));
}

void
pbsd_assert_vop_elocked(void *vp, const char *str)
{
	push(E_ASSERT_ELOCKED, sym(vp), strid(str));
}

void
pbsd_vop_unlock(void *vp)
{
	push(E_VOP_UNLOCK, sym(vp));
}

void
pbsd_vn_lock(void *vp, int flags)
{
	push(E_VN_LOCK, sym(vp), flags);
}

int
pbsd_sigdeferstop(int mode)
{
	push(E_SIGDEFERSTOP, mode);
	return g_w->scr->sigdefer_ret;
}

void
pbsd_sigallowstop(int prev)
{
	push(E_SIGALLOWSTOP, prev);
}

int
pbsd_msleep(void *chan, void *mtx, int pri, const char *wmesg, int timo)
{
	World &w = *g_w;
	int i = (w.msleep_n < 2) ? w.msleep_n : 1;

	w.msleep_n++;
	push(E_MSLEEP, sym(chan), sym(mtx), pri, strid(wmesg), timo);

	/*
	 * msleep() drops the vnode lock, so the fifo state may have been
	 * changed by another thread while we slept.  The scripted deltas model
	 * that; chan points into the fifoinfo, which is how the sleeping
	 * object is located.
	 */
	TwinFifo *f = nullptr;
	const unsigned char *q = static_cast<const unsigned char *>(chan);
	for (int rid = R_FIP0; rid <= R_FIP1; rid++) {
		if (w.regs[rid].base != nullptr && q >= w.regs[rid].base &&
		    q < w.regs[rid].base + w.regs[rid].size) {
			f = reinterpret_cast<TwinFifo *>(
			    const_cast<unsigned char *>(w.regs[rid].base));
		}
	}
	if (f != nullptr) {
		f->fi_readers += w.scr->d_readers[i];
		f->fi_writers += w.scr->d_writers[i];
		f->fi_rgen = f->fi_rgen +
		    static_cast<unsigned int>(w.scr->d_rgen[i]);
		f->fi_wgen = f->fi_wgen +
		    static_cast<unsigned int>(w.scr->d_wgen[i]);
		TwinPipe *tp = static_cast<TwinPipe *>(f->fi_pipe);
		if (tp != nullptr) {
			tp->pipe_state |= w.scr->or_state[i];
			tp->pipe_state &= ~w.scr->clr_state[i];
			tp->pipe_wgen += w.scr->d_pwgen[i];
		}
	}
	return w.scr->msleep_err[i];
}

void
pbsd_finit(void *fp, unsigned int flag, int type, void *data, const void *ops)
{
	push(E_FINIT, sym(fp), static_cast<long>(flag), type, sym(data),
	    sym(ops));
}

int
pbsd_vop_stdadvlock(void *ap)
{
	push(E_STDADVLOCK, sym(ap));
	return g_w->scr->advlock_ret;
}

} /* extern "C" */

/* ------------------------------------------------------------------ */
/* Case description and observed outcome.                              */
/* ------------------------------------------------------------------ */
struct Scenario {
	int		have_fip;
	long		readers;
	long		writers;
	unsigned int	rgen;
	unsigned int	wgen;
	unsigned int	pipe_state;
	int		pipe_wgen;
	int		fp_null;
	unsigned int	f_flag;
	int		mode;
	int		fflag;
	int		flags;
	int		aop;
	Script		scr;
};

struct Result {
	int	rv;
	long	sym_vfifo;
	int	fip_present[2];
	long	fip[2][5];
	int	pipe_present[2];
	long	pip[2][2];
	long	fpv[3];
	int	nalloc;
	int	nev;
	int	ev_overflow;
	int	out_overflow;
	int	guard_ok;
	int	outlen;
	char	out[OUTMAX];
	Ev	ev[MAXEV];
};

template <class T>
static void
collect(World &w, Result &r, void *vpv, void *fpv)
{
	g_w = &w;
	auto *vp = static_cast<typename T::vnode *>(vpv);

	r.sym_vfifo = sym(vp->v_fifoinfo);
	int fi_rid[2] = { R_FIP0, R_FIP1 };
	for (int k = 0; k < 2; k++) {
		if (w.regs[fi_rid[k]].base == nullptr) {
			r.fip_present[k] = 0;
			continue;
		}
		r.fip_present[k] = 1;
		auto *f = reinterpret_cast<const typename T::fifoinfo *>(
		    w.regs[fi_rid[k]].base);
		r.fip[k][0] = sym(f->fi_pipe);
		r.fip[k][1] = f->fi_readers;
		r.fip[k][2] = f->fi_writers;
		r.fip[k][3] = static_cast<long>(f->fi_rgen);
		r.fip[k][4] = static_cast<long>(f->fi_wgen);
	}
	int pi_rid[2] = { R_PIPE0, R_PIPE1 };
	for (int k = 0; k < 2; k++) {
		if (w.regs[pi_rid[k]].base == nullptr) {
			r.pipe_present[k] = 0;
			continue;
		}
		r.pipe_present[k] = 1;
		auto *p = reinterpret_cast<const typename T::pipe *>(
		    w.regs[pi_rid[k]].base);
		r.pip[k][0] = static_cast<long>(p->pipe_state);
		r.pip[k][1] = p->pipe_wgen;
	}
	if (fpv != nullptr) {
		auto *fp = static_cast<const typename T::file *>(fpv);
		r.fpv[0] = static_cast<long>(fp->f_flag);
		r.fpv[1] = fp->f_pipegen;
		r.fpv[2] = sym(fp->f_ops);
	}
	r.nalloc = w.nalloc;
	r.nev = w.nev;
	r.ev_overflow = w.ev_overflow;
	r.out_overflow = w.out_overflow;
	r.guard_ok = guards_ok(w);
	r.outlen = w.outlen;
	std::memcpy(r.out, w.out, sizeof(r.out));
	std::memcpy(r.ev, w.ev, sizeof(r.ev));
}

/* ------------------------------------------------------------------ */
/* Traits binding each side's types and entry points.                  */
/* ------------------------------------------------------------------ */
struct RefT {
	using vnode = refm::vnode;
	using fifoinfo = refm::fifoinfo;
	using pipe = refm::pipe;
	using file = refm::file;
	using thread = refm::thread;
	using open_args = refm::vop_open_args;
	using close_args = refm::vop_close_args;
	using print_args = refm::vop_print_args;
	using advlock_args = refm::vop_advlock_args;

	static void do_cleanup(vnode *vp) { ref_fifo_cleanup(vp); }
	static int do_open(open_args *a) { return ref_fifo_open(a); }
	static int do_close(close_args *a) { return ref_fifo_close(a); }
	static int do_printinfo(vnode *vp) { return ref_fifo_printinfo(vp); }
	static int do_print(print_args *a) { return ref_fifo_print(a); }
	static int do_advlock(advlock_args *a) { return ref_fifo_advlock(a); }
};

struct PortT {
	using vnode = P::vnode;
	using fifoinfo = P::fifoinfo;
	using pipe = P::pipe;
	using file = P::file;
	using thread = P::thread;
	using open_args = P::vop_open_args;
	using close_args = P::vop_close_args;
	using print_args = P::vop_print_args;
	using advlock_args = P::vop_advlock_args;

	static void do_cleanup(vnode *vp) { P::fifo_cleanup(vp); }
	static int do_open(open_args *a) { return P::fifo_open(a); }
	static int do_close(close_args *a) { return P::fifo_close(a); }
	static int do_printinfo(vnode *vp) { return P::fifo_printinfo(vp); }
	static int do_print(print_args *a) { return P::fifo_print(a); }
	static int do_advlock(advlock_args *a) { return P::fifo_advlock(a); }
};

/*
 * Common object construction.  The pool layout, and therefore every symbol,
 * is the same on both sides regardless of which function is exercised.
 */
template <class T>
struct Objs {
	typename T::vnode	*vp;
	typename T::pipe	*pipe;
	typename T::fifoinfo	*fip;
	typename T::file	*fp;
	typename T::thread	*td;
	void			*args;
};

template <class T>
static Objs<T>
build(const Scenario &s, World &w, size_t argsz)
{
	Objs<T> o;

	world_reset(w, &s.scr);
	g_w = &w;
	o.vp = static_cast<typename T::vnode *>(
	    wnew(w, sizeof(typename T::vnode), R_VP));
	o.pipe = static_cast<typename T::pipe *>(
	    wnew(w, sizeof(typename T::pipe), R_PIPE0));
	o.fip = static_cast<typename T::fifoinfo *>(
	    wnew(w, sizeof(typename T::fifoinfo), R_FIP0));
	o.fp = static_cast<typename T::file *>(
	    wnew(w, sizeof(typename T::file), R_FP));
	o.td = static_cast<typename T::thread *>(
	    wnew(w, sizeof(typename T::thread), R_TD));
	o.args = wnew(w, argsz, R_ARGS);

	o.pipe->pipe_state = s.pipe_state;
	o.pipe->pipe_wgen = s.pipe_wgen;
	o.fip->fi_pipe = o.pipe;
	o.fip->fi_readers = s.readers;
	o.fip->fi_writers = s.writers;
	o.fip->fi_rgen = s.rgen;
	o.fip->fi_wgen = s.wgen;
	o.fp->f_flag = s.f_flag;
	o.fp->f_pipegen = 0x5a5a5a;
	o.fp->f_ops = &pbsd_badfileops_obj;
	o.td->td_tid = 4242;
	o.vp->v_fifoinfo = s.have_fip ? o.fip : nullptr;
	return o;
}

template <class T>
static void
run_open(const Scenario &s, World &w, Result &r)
{
	std::memset(&r, 0, sizeof(r));
	Objs<T> o = build<T>(s, w, sizeof(typename T::open_args));
	auto *ap = static_cast<typename T::open_args *>(o.args);

	ap->a_vp = o.vp;
	ap->a_mode = s.mode;
	ap->a_td = o.td;
	ap->a_fp = s.fp_null ? nullptr : o.fp;
	r.rv = T::do_open(ap);
	collect<T>(w, r, o.vp, o.fp);
}

template <class T>
static void
run_close(const Scenario &s, World &w, Result &r)
{
	std::memset(&r, 0, sizeof(r));
	Objs<T> o = build<T>(s, w, sizeof(typename T::close_args));
	auto *ap = static_cast<typename T::close_args *>(o.args);

	ap->a_vp = o.vp;
	ap->a_fflag = s.fflag;
	ap->a_td = o.td;
	r.rv = T::do_close(ap);
	collect<T>(w, r, o.vp, o.fp);
}

template <class T>
static void
run_cleanup(const Scenario &s, World &w, Result &r)
{
	std::memset(&r, 0, sizeof(r));
	Objs<T> o = build<T>(s, w, sizeof(typename T::open_args));

	o.vp->v_fifoinfo = o.fip;	/* fifo_cleanup() dereferences it */
	T::do_cleanup(o.vp);
	r.rv = 0;
	collect<T>(w, r, o.vp, o.fp);
}

template <class T>
static void
run_printinfo(const Scenario &s, World &w, Result &r)
{
	std::memset(&r, 0, sizeof(r));
	Objs<T> o = build<T>(s, w, sizeof(typename T::open_args));

	r.rv = T::do_printinfo(o.vp);
	collect<T>(w, r, o.vp, o.fp);
}

template <class T>
static void
run_print(const Scenario &s, World &w, Result &r)
{
	std::memset(&r, 0, sizeof(r));
	Objs<T> o = build<T>(s, w, sizeof(typename T::print_args));
	auto *ap = static_cast<typename T::print_args *>(o.args);

	ap->a_vp = o.vp;
	r.rv = T::do_print(ap);
	collect<T>(w, r, o.vp, o.fp);
}

template <class T>
static void
run_advlock(const Scenario &s, World &w, Result &r)
{
	std::memset(&r, 0, sizeof(r));
	Objs<T> o = build<T>(s, w, sizeof(typename T::advlock_args));
	auto *ap = static_cast<typename T::advlock_args *>(o.args);

	ap->a_vp = o.vp;
	ap->a_id = o.td;
	ap->a_op = s.aop;
	ap->a_fl = nullptr;
	ap->a_flags = s.flags;
	r.rv = T::do_advlock(ap);
	collect<T>(w, r, o.vp, o.fp);
}

/* ------------------------------------------------------------------ */
/* Reporting.                                                          */
/* ------------------------------------------------------------------ */
static void
print_ev(const Ev *ev, int n)
{
	char b0[32], b1[32], b2[32];

	for (int i = 0; i < n; i++) {
		const char *nm = (ev[i].op > 0 && ev[i].op < E_OP_MAX) ?
		    ev_name[ev[i].op] : "?";
		sym_str(ev[i].a[0], b0, sizeof(b0));
		sym_str(ev[i].a[1], b1, sizeof(b1));
		sym_str(ev[i].a[3], b2, sizeof(b2));
		std::printf("      [%2d] %-20s a0=%s a1=%ld/%s a2=%ld a3=%ld/%s a4=%ld\n",
		    i, nm, b0, ev[i].a[1], b1, ev[i].a[2], ev[i].a[3], b2,
		    ev[i].a[4]);
	}
}

static void
print_scn(const Scenario &s)
{
	std::printf("    scenario: have_fip=%d readers=%ld writers=%ld "
	    "rgen=%u wgen=%u pipe_state=0x%x pipe_wgen=%d\n"
	    "              fp_null=%d f_flag=0x%x mode=0x%x fflag=0x%x "
	    "flags=0x%x aop=%d\n"
	    "              ctor_err=%d ctor_state=0x%x ctor_wgen=%d "
	    "sigdefer=%d advlock=%d\n"
	    "              msleep_err={%d,%d} dr={%ld,%ld} dw={%ld,%ld} "
	    "drg={%d,%d} dwg={%d,%d}\n"
	    "              or={0x%x,0x%x} clr={0x%x,0x%x} dpw={%d,%d}\n",
	    s.have_fip, s.readers, s.writers, s.rgen, s.wgen, s.pipe_state,
	    s.pipe_wgen, s.fp_null, s.f_flag, (unsigned)s.mode,
	    (unsigned)s.fflag, (unsigned)s.flags, s.aop, s.scr.ctor_err,
	    s.scr.ctor_pipe_state, s.scr.ctor_pipe_wgen, s.scr.sigdefer_ret,
	    s.scr.advlock_ret, s.scr.msleep_err[0], s.scr.msleep_err[1],
	    s.scr.d_readers[0], s.scr.d_readers[1], s.scr.d_writers[0],
	    s.scr.d_writers[1], s.scr.d_rgen[0], s.scr.d_rgen[1],
	    s.scr.d_wgen[0], s.scr.d_wgen[1], s.scr.or_state[0],
	    s.scr.or_state[1], s.scr.clr_state[0], s.scr.clr_state[1],
	    s.scr.d_pwgen[0], s.scr.d_pwgen[1]);
}

static void
print_side(const char *tag, const Result &r)
{
	char b[32];

	sym_str(r.sym_vfifo, b, sizeof(b));
	std::printf("    %s: rv=%d v_fifoinfo=%s nalloc=%d guard_ok=%d "
	    "ev_ovf=%d out_ovf=%d\n", tag, r.rv, b, r.nalloc, r.guard_ok,
	    r.ev_overflow, r.out_overflow);
	for (int k = 0; k < 2; k++) {
		if (!r.fip_present[k])
			continue;
		sym_str(r.fip[k][0], b, sizeof(b));
		std::printf("      fifoinfo[%s]: fi_pipe=%s readers=%ld "
		    "writers=%ld rgen=%lu wgen=%lu\n",
		    k == 0 ? "pre" : "new", b, r.fip[k][1], r.fip[k][2],
		    (unsigned long)r.fip[k][3], (unsigned long)r.fip[k][4]);
	}
	for (int k = 0; k < 2; k++) {
		if (!r.pipe_present[k])
			continue;
		std::printf("      pipe[%s]: pipe_state=0x%lx pipe_wgen=%ld\n",
		    k == 0 ? "pre" : "new", (unsigned long)r.pip[k][0],
		    r.pip[k][1]);
	}
	sym_str(r.fpv[2], b, sizeof(b));
	std::printf("      file: f_flag=0x%lx f_pipegen=%ld f_ops=%s\n",
	    (unsigned long)r.fpv[0], r.fpv[1], b);
	std::printf("      printf(%d): \"%s\"\n", r.outlen, r.out);
	print_ev(r.ev, r.nev);
}

struct Stat {
	const char	*name;
	long		cases;
	long		fails;
	int		shown;
};

static void
report(Stat &st, const Scenario &s, const Result &a, const Result &b)
{
	st.fails++;
	if (st.shown >= 3)
		return;
	st.shown++;
	std::printf("\nMISMATCH in %s (case %ld):\n", st.name, st.cases);
	print_scn(s);
	print_side("oracle", a);
	print_side("port  ", b);
}

/* ------------------------------------------------------------------ */
/* Case drivers.                                                       */
/* ------------------------------------------------------------------ */
#define DEFINE_DRIVER(fn)						\
static void								\
case_##fn(const Scenario &s, Stat &st)					\
{									\
	Result a, b;							\
									\
	run_##fn<RefT>(s, g_ref_world, a);				\
	run_##fn<PortT>(s, g_port_world, b);				\
	st.cases++;							\
	if (std::memcmp(&a, &b, sizeof(a)) != 0) {			\
		report(st, s, a, b);					\
		return;							\
	}								\
	if (a.guard_ok == 0 || b.guard_ok == 0 || a.ev_overflow != 0 ||	\
	    b.ev_overflow != 0 || a.out_overflow != 0 ||			\
	    b.out_overflow != 0)					\
		report(st, s, a, b);					\
}

DEFINE_DRIVER(open)
DEFINE_DRIVER(close)
DEFINE_DRIVER(cleanup)
DEFINE_DRIVER(printinfo)
DEFINE_DRIVER(print)
DEFINE_DRIVER(advlock)

/* ------------------------------------------------------------------ */
/* Fixed-seed RNG.                                                     */
/* ------------------------------------------------------------------ */
static uint64_t rng_state;

static void
rng_seed(uint64_t s)
{
	rng_state = s ? s : 0x9e3779b97f4a7c15ULL;
}

static uint64_t
rnd(void)
{
	uint64_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return x;
}

static long
rnd_small(long lo, long hi)
{
	return lo + static_cast<long>(rnd() % static_cast<uint64_t>(hi - lo + 1));
}

static unsigned int
rnd_pipe_state(void)
{
	static const unsigned int bits[] = {
		K_PIPE_ASYNC, K_PIPE_WANTR, K_PIPE_WANTW, K_PIPE_WANT,
		K_PIPE_SEL, K_PIPE_EOF, K_PIPE_DIRECTW, 0x80000000u
	};
	unsigned int v = 0;

	for (unsigned int b : bits) {
		if ((rnd() & 1) != 0)
			v |= b;
	}
	return v;
}

static int
rnd_mode(void)
{
	int m = 0;

	if (rnd() & 1)
		m |= K_FREAD;
	if (rnd() & 1)
		m |= K_FWRITE;
	if (rnd() & 1)
		m |= K_O_NONBLOCK;
	if ((rnd() % 4) == 0)
		m |= K_FEXEC;
	if ((rnd() % 8) == 0)
		m |= 0x0008;		/* O_APPEND-ish noise */
	if ((rnd() % 8) == 0)
		m |= 0x10000;
	return m;
}

static void
rnd_script(Script &sc)
{
	static const int errs[] = { 0, K_EINTR, K_EINVAL, K_ENXIO };

	sc.ctor_err = ((rnd() % 3) == 0) ? errs[rnd() % 4] : 0;
	sc.ctor_pipe_state = rnd_pipe_state();
	sc.ctor_pipe_wgen = static_cast<int>(rnd_small(-3, 3));
	sc.sigdefer_ret = static_cast<int>(rnd_small(0, 1));
	sc.advlock_ret = errs[rnd() % 4];
	for (int i = 0; i < 2; i++) {
		sc.msleep_err[i] = ((rnd() & 1) != 0) ? 0 : errs[rnd() % 4];
		sc.d_readers[i] = rnd_small(-2, 2);
		sc.d_writers[i] = rnd_small(-2, 2);
		sc.d_rgen[i] = static_cast<int>(rnd_small(-1, 1));
		sc.d_wgen[i] = static_cast<int>(rnd_small(-1, 1));
		sc.or_state[i] = ((rnd() % 3) == 0) ? rnd_pipe_state() : 0u;
		sc.clr_state[i] = ((rnd() % 3) == 0) ? rnd_pipe_state() : 0u;
		sc.d_pwgen[i] = static_cast<int>(rnd_small(-1, 1));
	}
}

static long
rnd_count(int wide)
{
	if (wide && (rnd() % 16) == 0) {
		/* Extreme values: only where no ++/-- is applied to them. */
		static const long ext[] = { LONG_MIN, LONG_MAX, LONG_MIN + 1,
		    LONG_MAX - 1, -1L << 40, 1L << 40 };
		return ext[rnd() % 6];
	}
	if ((rnd() % 8) == 0)
		return rnd_small(-1L << 20, 1L << 20);
	return rnd_small(-4, 4);
}

static Scenario
rnd_scenario(int wide_counts, int force_fip)
{
	Scenario s;

	std::memset(&s, 0, sizeof(s));
	s.have_fip = force_fip ? 1 : static_cast<int>(rnd() & 1);
	s.readers = rnd_count(wide_counts);
	s.writers = rnd_count(wide_counts);
	s.rgen = static_cast<unsigned int>(rnd_small(0, 6));
	s.wgen = static_cast<unsigned int>(rnd_small(0, 6));
	if ((rnd() % 16) == 0)
		s.rgen = 0xfffffffeu;
	if ((rnd() % 16) == 0)
		s.wgen = 0xffffffffu;
	s.pipe_state = rnd_pipe_state();
	s.pipe_wgen = static_cast<int>(rnd_small(-3, 3));
	s.fp_null = ((rnd() % 8) == 0) ? 1 : 0;
	s.f_flag = static_cast<unsigned int>(rnd() & 0xffffu);
	s.mode = rnd_mode();
	s.fflag = static_cast<int>(rnd() & 0xf) |
	    (((rnd() % 4) == 0) ? 0x10000 : 0);
	s.flags = static_cast<int>(rnd() & 0x3f);
	if ((rnd() & 1) != 0)
		s.flags |= K_F_FLOCK;
	s.aop = static_cast<int>(rnd_small(1, 4));
	rnd_script(s.scr);
	return s;
}

/* ------------------------------------------------------------------ */
/* Hand-written edge case enumerations.                                */
/* ------------------------------------------------------------------ */
static const long edge_counts[] = { -2, -1, 0, 1, 2 };
static const unsigned int edge_states[] = {
	0u,
	K_PIPE_EOF,
	K_PIPE_WANTW,
	K_PIPE_WANTR,
	static_cast<unsigned int>(K_PIPE_EOF | K_PIPE_WANTR | K_PIPE_WANTW),
	0xffffffffu
};

static void
base_scenario(Scenario &s)
{
	std::memset(&s, 0, sizeof(s));
	s.have_fip = 1;
	s.rgen = 3;
	s.wgen = 5;
	s.pipe_wgen = 2;
	s.f_flag = 0x1234;
	s.scr.ctor_pipe_state = K_PIPE_EOF | K_PIPE_WANTR;
	s.scr.ctor_pipe_wgen = 7;
	s.scr.sigdefer_ret = 1;
	s.scr.advlock_ret = 0;
	s.aop = 1;
}

/* msleep side-effect profiles: each drives a different post-sleep branch. */
static void
apply_profile(Script &sc, int p)
{
	sc.d_readers[0] = sc.d_readers[1] = 0;
	sc.d_writers[0] = sc.d_writers[1] = 0;
	sc.d_rgen[0] = sc.d_rgen[1] = 0;
	sc.d_wgen[0] = sc.d_wgen[1] = 0;
	sc.or_state[0] = sc.or_state[1] = 0;
	sc.clr_state[0] = sc.clr_state[1] = 0;
	sc.d_pwgen[0] = sc.d_pwgen[1] = 0;
	switch (p) {
	case 0:
		break;
	case 1:
		sc.d_wgen[0] = 1;
		break;
	case 2:
		sc.d_rgen[1] = 1;
		break;
	case 3:
		sc.d_writers[0] = 1;
		sc.d_writers[1] = 1;
		break;
	case 4:
		sc.d_readers[0] = 1;
		sc.d_readers[1] = 1;
		break;
	case 5:
		sc.d_readers[0] = -1;
		sc.d_writers[1] = -1;
		break;
	case 6:
		sc.or_state[0] = K_PIPE_WANTW | K_PIPE_WANTR;
		sc.or_state[1] = K_PIPE_WANTW | K_PIPE_WANTR;
		sc.d_pwgen[0] = 1;
		sc.d_pwgen[1] = 1;
		break;
	case 7:
		sc.clr_state[0] = K_PIPE_EOF | K_PIPE_WANTW | K_PIPE_WANTR;
		sc.clr_state[1] = K_PIPE_EOF | K_PIPE_WANTW | K_PIPE_WANTR;
		sc.d_rgen[1] = 1;
		sc.d_wgen[0] = 1;
		break;
	}
}

static void
edges_open(Stat &st)
{
	static const int modes[] = {
		0,
		K_FREAD,
		K_FWRITE,
		K_FREAD | K_FWRITE,
		K_O_NONBLOCK,
		K_FREAD | K_O_NONBLOCK,
		K_FWRITE | K_O_NONBLOCK,
		K_FREAD | K_FWRITE | K_O_NONBLOCK,
		K_FEXEC,
		K_FREAD | K_FEXEC,
		K_FWRITE | K_FEXEC,
		K_FREAD | K_FWRITE | K_O_NONBLOCK | K_FEXEC,
		K_FREAD | 0x10000,
		K_FWRITE | 0x10000,
		K_FREAD | K_FWRITE | 0x10000
	};
	static const int ctor_errs[] = { 0, K_ENXIO };
	static const int sleep_errs[] = { 0, K_EINTR };
	Scenario s;

	for (int mi = 0; mi < (int)(sizeof(modes) / sizeof(modes[0])); mi++) {
	for (int fpn = 0; fpn < 2; fpn++) {
	for (int hf = 0; hf < 2; hf++) {
	for (long r : edge_counts) {
	for (long wr : edge_counts) {
	for (int si = 0; si < 3; si++) {
	for (int ce = 0; ce < 2; ce++) {
	for (int se = 0; se < 2; se++) {
	for (int prof = 0; prof < 8; prof++) {
		base_scenario(s);
		s.mode = modes[mi];
		s.fp_null = fpn;
		s.have_fip = hf;
		s.readers = r;
		s.writers = wr;
		s.pipe_state = edge_states[si];
		s.scr.ctor_err = ctor_errs[ce];
		s.scr.msleep_err[0] = sleep_errs[se];
		s.scr.msleep_err[1] = sleep_errs[se];
		apply_profile(s.scr, prof);
		case_open(s, st);

		/* Same case with the second sleep failing only. */
		s.scr.msleep_err[0] = 0;
		s.scr.msleep_err[1] = sleep_errs[se];
		case_open(s, st);

		/* ... and only the first. */
		s.scr.msleep_err[0] = sleep_errs[se];
		s.scr.msleep_err[1] = 0;
		case_open(s, st);
	}
	}
	}
	}
	}
	}
	}
	}
	}

	/* u_int generation wraparound around the gen == fi_wgen test. */
	for (unsigned int g : { 0u, 1u, 0xfffffffeu, 0xffffffffu }) {
		for (int d = -1; d <= 1; d++) {
			base_scenario(s);
			s.mode = K_FREAD | K_FWRITE;
			s.have_fip = 1;
			s.readers = 0;
			s.writers = 0;
			s.rgen = g;
			s.wgen = g;
			s.scr.msleep_err[0] = K_EINTR;
			s.scr.msleep_err[1] = K_EINTR;
			apply_profile(s.scr, 0);
			s.scr.d_wgen[0] = d;
			s.scr.d_rgen[1] = d;
			case_open(s, st);
		}
	}
}

static void
edges_close(Stat &st)
{
	static const int fflags[] = {
		0, K_FREAD, K_FWRITE, K_FREAD | K_FWRITE,
		K_FREAD | K_FWRITE | 0x10000, 0x10000, ~0
	};
	Scenario s;

	for (int fi = 0; fi < (int)(sizeof(fflags) / sizeof(fflags[0])); fi++) {
	for (int hf = 0; hf < 2; hf++) {
	for (long r : edge_counts) {
	for (long wr : edge_counts) {
	for (unsigned int ps : edge_states) {
	for (int pw = -1; pw <= 1; pw++) {
		base_scenario(s);
		s.fflag = fflags[fi];
		s.have_fip = hf;
		s.readers = r;
		s.writers = wr;
		s.pipe_state = ps;
		s.pipe_wgen = pw;
		case_close(s, st);
	}
	}
	}
	}
	}
	}
}

static void
edges_cleanup(Stat &st)
{
	Scenario s;

	for (long r : edge_counts) {
	for (long wr : edge_counts) {
	for (unsigned int ps : edge_states) {
	for (int pw = -1; pw <= 1; pw++) {
		base_scenario(s);
		s.readers = r;
		s.writers = wr;
		s.pipe_state = ps;
		s.pipe_wgen = pw;
		case_cleanup(s, st);
	}
	}
	}
	}
}

static const long print_counts[] = {
	0, 1, -1, 2, -2, 7, LONG_MIN, LONG_MAX, LONG_MIN + 1, LONG_MAX - 1,
	255, 256, -255, 0x7f, 0x80, -0x80
};

static void
edges_printinfo(Stat &st)
{
	Scenario s;

	for (int hf = 0; hf < 2; hf++) {
	for (long r : print_counts) {
	for (long wr : print_counts) {
		base_scenario(s);
		s.have_fip = hf;
		s.readers = r;
		s.writers = wr;
		case_printinfo(s, st);
	}
	}
	}
}

static void
edges_print(Stat &st)
{
	Scenario s;

	for (int hf = 0; hf < 2; hf++) {
	for (long r : print_counts) {
	for (long wr : print_counts) {
		base_scenario(s);
		s.have_fip = hf;
		s.readers = r;
		s.writers = wr;
		case_print(s, st);
	}
	}
	}
}

static void
edges_advlock(Stat &st)
{
	static const int flagsv[] = {
		0, K_F_FLOCK, K_F_FLOCK - 1, K_F_FLOCK + 1,
		K_F_FLOCK | 0x1, ~K_F_FLOCK, ~0, 0x1f, 0x20, 0x21, 0x40
	};
	static const int rets[] = { 0, K_EINVAL, 13, -1 };
	Scenario s;

	for (int fi = 0; fi < (int)(sizeof(flagsv) / sizeof(flagsv[0])); fi++) {
	for (int ri = 0; ri < 4; ri++) {
	for (int op = 1; op <= 3; op++) {
		base_scenario(s);
		s.flags = flagsv[fi];
		s.scr.advlock_ret = rets[ri];
		s.aop = op;
		case_advlock(s, st);
	}
	}
	}
}

/* ------------------------------------------------------------------ */
int
main(void)
{
	constexpr long SWEEP = 200000;
	Stat st[6] = {
		{ "fifo_cleanup", 0, 0, 0 },
		{ "fifo_open", 0, 0, 0 },
		{ "fifo_close", 0, 0, 0 },
		{ "fifo_printinfo", 0, 0, 0 },
		{ "fifo_print", 0, 0, 0 },
		{ "fifo_advlock", 0, 0, 0 }
	};

	std::memset(guardref, GUARD, sizeof(guardref));

	edges_cleanup(st[0]);
	edges_open(st[1]);
	edges_close(st[2]);
	edges_printinfo(st[3]);
	edges_print(st[4]);
	edges_advlock(st[5]);

	rng_seed(0xb0205c0ffee1234ULL);
	for (long i = 0; i < SWEEP; i++)
		case_cleanup(rnd_scenario(0, 1), st[0]);
	rng_seed(0x1a2b3c4d5e6f7788ULL);
	for (long i = 0; i < SWEEP; i++)
		case_open(rnd_scenario(0, 0), st[1]);
	rng_seed(0x0f1e2d3c4b5a6978ULL);
	for (long i = 0; i < SWEEP; i++)
		case_close(rnd_scenario(0, 0), st[2]);
	rng_seed(0xdeadbeefcafe0205ULL);
	for (long i = 0; i < SWEEP; i++)
		case_printinfo(rnd_scenario(1, 0), st[3]);
	rng_seed(0x5555aaaa33330205ULL);
	for (long i = 0; i < SWEEP; i++)
		case_print(rnd_scenario(1, 0), st[4]);
	rng_seed(0x0205020502050205ULL);
	for (long i = 0; i < SWEEP; i++)
		case_advlock(rnd_scenario(0, 0), st[5]);

	long total_cases = 0, total_fails = 0;

	std::printf("\n%-18s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-18s %12s %12s\n", "------------------",
	    "------------", "------------");
	for (const Stat &s : st) {
		std::printf("%-18s %12ld %12ld\n", s.name, s.cases, s.fails);
		total_cases += s.cases;
		total_fails += s.fails;
	}
	std::printf("%-18s %12s %12s\n", "------------------",
	    "------------", "------------");
	std::printf("%-18s %12ld %12ld\n", "TOTAL", total_cases, total_fails);
	std::printf("\n%s\n", total_fails == 0 ? "PASS: port matches oracle "
	    "on every case" : "FAIL: port diverges from oracle");
	return total_fails == 0 ? 0 : 1;
}
