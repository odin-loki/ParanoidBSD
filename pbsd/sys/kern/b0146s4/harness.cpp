/*
 * PBSD batch b0146s4 differential harness.
 *
 * Drives the C++23 port in pbsd::sys_kern::b0146s4 and the ref_ oracle built
 * from hbsd/src/sys/kern/kern_sema.c side by side over the same semaphore
 * states and the same scripted condition variable wakeups, and compares:
 *
 *   - the return value of every call;
 *   - the whole modelled environment afterwards (mutex and condvar state,
 *     sema_value, sema_waiters, script position, total event count);
 *   - the complete event trace, entry by entry, so the order and the arguments
 *     of every mtx/cv/KTR/KASSERT/bzero operation are checked;
 *   - guard bytes on both sides of each semaphore object, so a write past the
 *     object is caught even when it does not change the result.
 *
 * Each semaphore object lives at a fixed offset inside a buffer pre-filled
 * with the guard byte 0x7f, which makes the bzero() in sema_init() and any
 * write outside the object observable.
 */

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <csetjmp>

import pbsd.sys.kern.b0146s4;

namespace P = pbsd::sys_kern::b0146s4;

struct sema;

extern "C" {
void		 ref_sema_init(struct sema *, int, const char *);
void		 ref_sema_destroy(struct sema *);
void		 ref__sema_post(struct sema *, const char *, int);
void		 ref__sema_wait(struct sema *, const char *, int);
int		 ref__sema_timedwait(struct sema *, int, const char *, int);
int		 ref__sema_trywait(struct sema *, const char *, int);
int		 ref_sema_value(struct sema *);
std::size_t	 ref_obj_size(void);
std::size_t	 ref_obj_align(void);
void		 ref_env_reset(void *, const void *, const void *, const int *,
		     const int *, int);
void		*ref_jmpbuf(void);
int		 ref_runaway_flag(void);
void		 ref_set_state(void *, int, int);
long		 ref_trace_count(void);
void		 ref_trace_get(long, int *, long *);
void		 ref_snapshot(const void *, long *);
}

#define	SNAP_N		18
#define	EVARGS		8
#define	MAXEV		4096
#define	SCRMAX		8
#define	GUARD		32
#define	BUFSZ		512
#define	MAXREPORT	25

enum {
	OP_INIT = 0,
	OP_POST,
	OP_WAIT,
	OP_TIMEDWAIT,
	OP_TRYWAIT,
	OP_VALUE,
	OP_DESTROY,
	OP_N
};

static const char *const opnames[OP_N] = {
	"sema_init",
	"_sema_post",
	"_sema_wait",
	"_sema_timedwait",
	"_sema_trywait",
	"sema_value",
	"sema_destroy"
};

static const char *const evnames[17] = {
	"?", "KASSERT", "bzero", "mtx_init", "cv_init", "mtx_lock",
	"mtx_unlock", "mtx_destroy", "cv_destroy", "cv_signal", "cv_wmesg",
	"cv_wait", "cv_timedwait", "CTR3", "CTR4", "CTR5", "CTR6"
};

struct Case {
	int	op;
	int	initval;
	int	poke;
	int	value;
	int	waiters;
	int	timo;
	int	line;
	int	nacts;
	int	deltas[SCRMAX];
	int	errs[SCRMAX];
	int	di;
	int	fi;
};

struct Result {
	long	ret;
	int	runaway;
	long	snap[SNAP_N];
	long	nev;
	int	ops[MAXEV];
	long	args[MAXEV][EVARGS];
};

static const char *const descs[] = {
	"semaphore",
	"",
	"z",
	"a rather long semaphore description used as the wait message"
};
static const int ndescs = (int)(sizeof(descs) / sizeof(descs[0]));

static const char *const files[] = {
	"kern_sema.c",
	"",
	"/usr/src/sys/kern/kern_sema.c"
};
static const int nfiles = (int)(sizeof(files) / sizeof(files[0]));

alignas(16) static unsigned char bufA[BUFSZ];
alignas(16) static unsigned char bufB[BUFSZ];

static Result resA;
static Result resB;

static std::size_t objsz;

static long cases_run[OP_N];
static long cases_fail[OP_N];
static long reported;

/* ------------------------------------------------------------------ */

static void
run_ref(const Case &c, Result &r)
{
	void *obj;

	std::memset(bufA, 0x7f, sizeof(bufA));
	obj = bufA + GUARD;
	ref_env_reset(obj, descs[c.di], files[c.fi], c.deltas, c.errs,
	    c.nacts);
	r.ret = 0;
	r.runaway = 0;
	if (setjmp(*static_cast<std::jmp_buf *>(ref_jmpbuf())) == 0) {
		ref_sema_init(reinterpret_cast<struct sema *>(obj), c.initval,
		    descs[c.di]);
		if (c.poke)
			ref_set_state(obj, c.value, c.waiters);
		switch (c.op) {
		case OP_INIT:
			break;
		case OP_POST:
			ref__sema_post(reinterpret_cast<struct sema *>(obj),
			    files[c.fi], c.line);
			break;
		case OP_WAIT:
			ref__sema_wait(reinterpret_cast<struct sema *>(obj),
			    files[c.fi], c.line);
			break;
		case OP_TIMEDWAIT:
			r.ret = ref__sema_timedwait(
			    reinterpret_cast<struct sema *>(obj), c.timo,
			    files[c.fi], c.line);
			break;
		case OP_TRYWAIT:
			r.ret = ref__sema_trywait(
			    reinterpret_cast<struct sema *>(obj), files[c.fi],
			    c.line);
			break;
		case OP_VALUE:
			r.ret = ref_sema_value(
			    reinterpret_cast<struct sema *>(obj));
			break;
		case OP_DESTROY:
			ref_sema_destroy(reinterpret_cast<struct sema *>(obj));
			break;
		}
	} else {
		r.runaway = 1;
	}
	r.runaway |= ref_runaway_flag();
	ref_snapshot(obj, r.snap);
	r.nev = ref_trace_count();
	for (long i = 0; i < r.nev && i < MAXEV; i++)
		ref_trace_get(i, &r.ops[i], r.args[i]);
}

static void
run_port(const Case &c, Result &r)
{
	void *obj;

	std::memset(bufB, 0x7f, sizeof(bufB));
	obj = bufB + GUARD;
	P::env_reset(obj, descs[c.di], files[c.fi], c.deltas, c.errs, c.nacts);
	r.ret = 0;
	r.runaway = 0;
	if (setjmp(*static_cast<std::jmp_buf *>(P::jmpbuf())) == 0) {
		P::sema_init(static_cast<P::sema *>(obj), c.initval,
		    descs[c.di]);
		if (c.poke)
			P::set_state(obj, c.value, c.waiters);
		switch (c.op) {
		case OP_INIT:
			break;
		case OP_POST:
			P::_sema_post(static_cast<P::sema *>(obj), files[c.fi],
			    c.line);
			break;
		case OP_WAIT:
			P::_sema_wait(static_cast<P::sema *>(obj), files[c.fi],
			    c.line);
			break;
		case OP_TIMEDWAIT:
			r.ret = P::_sema_timedwait(static_cast<P::sema *>(obj),
			    c.timo, files[c.fi], c.line);
			break;
		case OP_TRYWAIT:
			r.ret = P::_sema_trywait(static_cast<P::sema *>(obj),
			    files[c.fi], c.line);
			break;
		case OP_VALUE:
			r.ret = P::sema_value(static_cast<P::sema *>(obj));
			break;
		case OP_DESTROY:
			P::sema_destroy(static_cast<P::sema *>(obj));
			break;
		}
	} else {
		r.runaway = 1;
	}
	r.runaway |= P::runaway_flag();
	P::snapshot(obj, r.snap);
	r.nev = P::trace_count();
	for (long i = 0; i < r.nev && i < MAXEV; i++)
		P::trace_get(i, &r.ops[i], r.args[i]);
}

static void
describe(const Case &c)
{

	std::printf("    case: op=%s initval=%d poke=%d value=%d waiters=%d "
	    "timo=%d line=%d desc=%d file=%d nacts=%d", opnames[c.op],
	    c.initval, c.poke, c.value, c.waiters, c.timo, c.line, c.di, c.fi,
	    c.nacts);
	for (int i = 0; i < c.nacts; i++)
		std::printf(" [%d:d=%d,e=%d]", i, c.deltas[i], c.errs[i]);
	std::printf("\n");
}

static int
guards_ok(const unsigned char *buf)
{

	for (std::size_t i = 0; i < (std::size_t)GUARD; i++) {
		if (buf[i] != 0x7f)
			return (0);
	}
	for (std::size_t i = GUARD + objsz; i < (std::size_t)BUFSZ; i++) {
		if (buf[i] != 0x7f)
			return (0);
	}
	return (1);
}

static int
compare(const Case &c)
{
	int bad = 0;

	if (resA.ret != resB.ret) {
		if (reported < MAXREPORT) {
			std::printf("MISMATCH return: oracle=%ld port=%ld\n",
			    resA.ret, resB.ret);
			describe(c);
		}
		bad = 1;
	}
	if (resA.runaway != resB.runaway) {
		if (reported < MAXREPORT) {
			std::printf("MISMATCH runaway: oracle=%d port=%d\n",
			    resA.runaway, resB.runaway);
			describe(c);
		}
		bad = 1;
	}
	for (int i = 0; i < SNAP_N; i++) {
		if (resA.snap[i] == resB.snap[i])
			continue;
		if (reported < MAXREPORT) {
			std::printf("MISMATCH state[%d]: oracle=%ld "
			    "port=%ld\n", i, resA.snap[i], resB.snap[i]);
			describe(c);
		}
		bad = 1;
	}
	if (resA.nev != resB.nev) {
		if (reported < MAXREPORT) {
			std::printf("MISMATCH trace length: oracle=%ld "
			    "port=%ld\n", resA.nev, resB.nev);
			describe(c);
		}
		bad = 1;
	}
	long n = resA.nev < resB.nev ? resA.nev : resB.nev;
	if (n > MAXEV)
		n = MAXEV;
	for (long i = 0; i < n; i++) {
		int diff = (resA.ops[i] != resB.ops[i]);
		for (int k = 0; k < EVARGS && !diff; k++)
			diff = (resA.args[i][k] != resB.args[i][k]);
		if (!diff)
			continue;
		if (reported < MAXREPORT) {
			std::printf("MISMATCH trace[%ld]: oracle=%s(", i,
			    (resA.ops[i] >= 0 && resA.ops[i] <= 16) ?
			    evnames[resA.ops[i]] : "?");
			for (int k = 0; k < EVARGS; k++)
				std::printf("%ld%s", resA.args[i][k],
				    k == EVARGS - 1 ? "" : ",");
			std::printf(") port=%s(",
			    (resB.ops[i] >= 0 && resB.ops[i] <= 16) ?
			    evnames[resB.ops[i]] : "?");
			for (int k = 0; k < EVARGS; k++)
				std::printf("%ld%s", resB.args[i][k],
				    k == EVARGS - 1 ? "" : ",");
			std::printf(")\n");
			describe(c);
		}
		bad = 1;
		break;
	}
	if (!guards_ok(bufA) || !guards_ok(bufB) ||
	    std::memcmp(bufA, bufB, GUARD) != 0 ||
	    std::memcmp(bufA + GUARD + objsz, bufB + GUARD + objsz,
	    BUFSZ - GUARD - objsz) != 0) {
		if (reported < MAXREPORT) {
			std::printf("MISMATCH guard bytes clobbered\n");
			describe(c);
		}
		bad = 1;
	}
	if (bad)
		reported++;
	return (bad);
}

static void
run_case(const Case &c)
{

	run_ref(c, resA);
	run_port(c, resB);
	cases_run[c.op]++;
	if (compare(c))
		cases_fail[c.op]++;
}

/* ------------------------------------------------------------------ */

struct Script {
	int	n;
	int	d[SCRMAX];
	int	e[SCRMAX];
};

/*
 * Wakeup scripts.  Between them these drive every exit from both wait loops:
 * the value becoming non-zero (positive and negative), the timed wait failing
 * with the value still zero, the timed wait failing after the value has
 * already moved, and running the script off its end.
 */
static const Script scripts[] = {
	{ 0, { 0 }, { 0 } },
	{ 1, { 0 }, { 0 } },
	{ 1, { 1 }, { 0 } },
	{ 1, { -1 }, { 0 } },
	{ 1, { 0 }, { 35 } },
	{ 1, { 1 }, { 35 } },
	{ 1, { -1 }, { 35 } },
	{ 2, { 0, 0 }, { 0, 0 } },
	{ 2, { 0, 1 }, { 0, 0 } },
	{ 2, { -1, 0 }, { 0, 0 } },
	{ 2, { 0, 0 }, { 0, 4 } },
	{ 3, { 0, 0, -1 }, { 0, 0, 0 } },
	{ 3, { 0, 0, 0 }, { 0, 0, 4 } },
	{ 3, { 0, 2, 0 }, { 0, 0, 0 } },
	{ 4, { 0, 0, 0, 3 }, { 0, 0, 0, 0 } },
	{ 5, { 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0 } }
};
static const int nscripts = (int)(sizeof(scripts) / sizeof(scripts[0]));

static void
setscript(Case &c, const Script &s)
{

	c.nacts = s.n;
	for (int i = 0; i < SCRMAX; i++) {
		c.deltas[i] = i < s.n ? s.d[i] : 0;
		c.errs[i] = i < s.n ? s.e[i] : 0;
	}
}

static void
edge_cases(void)
{
	static const int initvals[] = { -1000000, -2, -1, 0, 1, 2, 1000000 };
	static const int values[] = { -1000, -2, -1, 0, 1, 2, 1000 };
	static const int waiterss[] = { -2, -1, 0, 1, 2 };
	static const int timos[] = { -1, 0, 1, 1000 };
	static const int initsub[] = { -1, 0, 5 };
	static const int plainops[] = { OP_POST, OP_TRYWAIT, OP_VALUE,
	    OP_DESTROY };
	Case c;

	std::memset(&c, 0, sizeof(c));
	c.line = 123;

	/* sema_init alone, over every description and file string. */
	for (int iv = 0; iv < 7; iv++) {
		for (int d = 0; d < ndescs; d++) {
			for (int f = 0; f < nfiles; f++) {
				setscript(c, scripts[0]);
				c.op = OP_INIT;
				c.initval = initvals[iv];
				c.poke = 0;
				c.value = 0;
				c.waiters = 0;
				c.timo = 0;
				c.di = d;
				c.fi = f;
				run_case(c);
				c.poke = 1;
				c.value = -1;
				c.waiters = 1;
				run_case(c);
			}
		}
	}

	/* The operations that never block, over every state combination. */
	for (int o = 0; o < 4; o++) {
		for (int iv = 0; iv < 7; iv++) {
			for (int v = 0; v < 7; v++) {
				for (int w = 0; w < 5; w++) {
					setscript(c, scripts[0]);
					c.op = plainops[o];
					c.initval = initvals[iv];
					c.poke = 1;
					c.value = values[v];
					c.waiters = waiterss[w];
					c.timo = 0;
					c.di = iv % ndescs;
					c.fi = v % nfiles;
					c.line = 1 + iv * 7 + v;
					run_case(c);
				}
			}
		}
	}

	/* _sema_wait: every state against every wakeup script. */
	for (int iv = 0; iv < 3; iv++) {
		for (int v = 0; v < 7; v++) {
			for (int w = 0; w < 5; w++) {
				for (int s = 0; s < nscripts; s++) {
					setscript(c, scripts[s]);
					c.op = OP_WAIT;
					c.initval = initsub[iv];
					c.poke = 1;
					c.value = values[v];
					c.waiters = waiterss[w];
					c.timo = 0;
					c.di = s % ndescs;
					c.fi = w % nfiles;
					c.line = 1000 + s;
					run_case(c);
				}
			}
		}
	}

	/* _sema_timedwait: the same, across timeouts. */
	for (int iv = 0; iv < 3; iv++) {
		for (int v = 0; v < 7; v++) {
			for (int w = 0; w < 5; w++) {
				for (int s = 0; s < nscripts; s++) {
					for (int t = 0; t < 4; t++) {
						setscript(c, scripts[s]);
						c.op = OP_TIMEDWAIT;
						c.initval = initsub[iv];
						c.poke = 1;
						c.value = values[v];
						c.waiters = waiterss[w];
						c.timo = timos[t];
						c.di = t % ndescs;
						c.fi = s % nfiles;
						c.line = 2000 + s * 4 + t;
						run_case(c);
					}
				}
			}
		}
	}

	/*
	 * Every comparison in the batch tests sema_value or sema_waiters
	 * against zero, so walk both of them across zero for every operation
	 * and every script: each boundary is exercised from both sides.
	 */
	for (int v = -2; v <= 2; v++) {
		for (int w = -1; w <= 1; w++) {
			for (int o = 0; o < OP_N; o++) {
				for (int s = 0; s < nscripts; s++) {
					setscript(c, scripts[s]);
					c.op = o;
					c.initval = v;
					c.poke = 1;
					c.value = v;
					c.waiters = w;
					c.timo = v;
					c.di = 0;
					c.fi = 0;
					c.line = 3000 + v;
					run_case(c);
				}
			}
		}
	}
}

/* ------------------------------------------------------------------ */

static std::uint64_t rngstate;

static std::uint64_t
rnd(void)
{

	rngstate ^= rngstate << 13;
	rngstate ^= rngstate >> 7;
	rngstate ^= rngstate << 17;
	return (rngstate);
}

static int
rndrange(int lo, int hi)
{

	return (lo + (int)(rnd() % (std::uint64_t)(hi - lo + 1)));
}

static void
random_sweep(long iters)
{
	static const int errpool[] = { 0, 0, 0, 35, 4, -1, 60 };
	Case c;

	rngstate = 0x0123456789abcdefULL;
	for (long it = 0; it < iters; it++) {
		std::memset(&c, 0, sizeof(c));
		c.op = (int)(rnd() % OP_N);
		if (rnd() % 4 == 0)
			c.initval = rndrange(-1000000, 1000000);
		else
			c.initval = rndrange(-3, 4);
		c.poke = (c.op == OP_INIT) ? (int)(rnd() % 2) : 1;
		if (rnd() % 4 == 0)
			c.value = rndrange(-100000, 100000);
		else
			c.value = rndrange(-3, 3);
		if (rnd() % 4 == 0)
			c.waiters = rndrange(-100000, 100000);
		else
			c.waiters = rndrange(-2, 2);
		c.timo = (rnd() % 2) ? rndrange(-2, 3) :
		    rndrange(-100000, 100000);
		c.line = rndrange(0, 65535);
		c.di = (int)(rnd() % (std::uint64_t)ndescs);
		c.fi = (int)(rnd() % (std::uint64_t)nfiles);
		c.nacts = (int)(rnd() % 7);
		for (int i = 0; i < SCRMAX; i++) {
			if (i < c.nacts) {
				c.deltas[i] = (rnd() % 2) ? 0 :
				    rndrange(-2, 2);
				c.errs[i] = errpool[rnd() % 7];
			} else {
				c.deltas[i] = 0;
				c.errs[i] = 0;
			}
		}
		run_case(c);
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	long totfail = 0;
	long totrun = 0;

	if (ref_obj_size() != P::obj_size() ||
	    ref_obj_align() != P::obj_align()) {
		std::printf("FATAL: object layout differs: oracle %zu/%zu "
		    "port %zu/%zu\n", ref_obj_size(), ref_obj_align(),
		    P::obj_size(), P::obj_align());
		return (1);
	}
	objsz = ref_obj_size();
	if (objsz + 2 * GUARD > BUFSZ) {
		std::printf("FATAL: buffer too small\n");
		return (1);
	}

	edge_cases();
	random_sweep(200000);

	std::printf("\n%-20s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-20s %12s %12s\n", "--------------------",
	    "------------", "------------");
	for (int o = 0; o < OP_N; o++) {
		std::printf("%-20s %12ld %12ld\n", opnames[o], cases_run[o],
		    cases_fail[o]);
		totrun += cases_run[o];
		totfail += cases_fail[o];
	}
	std::printf("%-20s %12s %12s\n", "--------------------",
	    "------------", "------------");
	std::printf("%-20s %12ld %12ld\n", "TOTAL", totrun, totfail);
	std::printf("\n%s\n", totfail == 0 ? "PASS" : "FAIL");
	return (totfail == 0 ? 0 : 1);
}
