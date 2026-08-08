/*
 * harness.cpp -- differential test for PBSD batch b0148s2 (undo.c).
 *
 * Every case runs the ported function and the ref_ oracle over identical
 * state and compares, after each operation, the full observable state:
 * the return value (as an offset into the undo stack, never a raw
 * address), the undo stack itself (u_p, usize and every entry's type and
 * head/tail node identity), the doubly linked line buffer walked both
 * forwards and backwards, all globals the routines touch, and the text of
 * the buffered lines.
 */

import pbsd.bin.ed.b0148s2;

#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace port = pbsd::bin_ed::b0148s2;

extern "C" {
extern long addr_last;
extern long current_addr;
extern long u_addr_last;
extern long u_current_addr;
extern int isglobal;
extern int modified;
extern int mutex;
extern int sigflags;
extern const char *errmsg;
extern int oracle_malloc_fail_at;
extern int oracle_malloc_calls;
extern int oracle_realloc_fail_at;
extern int oracle_realloc_calls;
extern int oracle_hup_calls;
extern int oracle_int_calls;
void oracle_reset_batch(void);
void oracle_inject_orphan_udel(long addr);
void *ref_push_undo_stack(int, long, long);
int ref_pop_undo_stack(void);
void ref_clear_undo_stack(void);
void *ref_get_addressed_line_node(long);
const char *ref_put_sbuf_line(const char *);
char *ref_get_sbuf_line(void *);
int ref_open_sbuf(void);
int ref_close_sbuf(void);
long oracle_u_p(void);
long oracle_usize(void);
long oracle_ustack_off(void *);
int oracle_stack_entry(long, int *, void **, void **);
void *oracle_buffer_head(void);
void *oracle_node_forw(void *);
void *oracle_node_back(void *);
}

namespace {

constexpr int UNI = 24;		/* known line nodes, index 0 == buffer_head */
constexpr int WCAP = 40;	/* bound on a buffer walk (lists can cycle) */
constexpr int SCAP = 128;	/* undo stack entries compared */
constexpr int NL = 12;		/* line texts compared */

constexpr long PUSH_ITERS = 70000;
constexpr long POP_ITERS = 70000;
constexpr long CLEAR_ITERS = 70000;

/* ------------------------------------------------------------------ */
/* side-neutral access to the two implementations                       */
/* ------------------------------------------------------------------ */

void *p_head() { return port::port_buffer_head(); }
void *p_forw(void *p) { return port::port_node_forw(p); }
void *p_back(void *p) { return port::port_node_back(p); }
long p_u_p() { return port::port_u_p(); }
long p_usize() { return port::port_usize(); }
long p_off(void *e) { return port::port_ustack_off(e); }
int p_ent(long i, int *ty, void **h, void **t)
{
	return port::port_stack_entry(i, ty, h, t);
}
void *p_gal(long n) { return port::get_addressed_line_node(n); }
char *p_gsl(void *p) { return port::get_sbuf_line((port::line_t *)p); }

void *r_head() { return oracle_buffer_head(); }
void *r_forw(void *p) { return oracle_node_forw(p); }
void *r_back(void *p) { return oracle_node_back(p); }
long r_u_p() { return oracle_u_p(); }
long r_usize() { return oracle_usize(); }
long r_off(void *e) { return oracle_ustack_off(e); }
int r_ent(long i, int *ty, void **h, void **t)
{
	return oracle_stack_entry(i, ty, h, t);
}
void *r_gal(long n) { return ref_get_addressed_line_node(n); }
char *r_gsl(void *p) { return ref_get_sbuf_line(p); }

struct Ops {
	void *(*head)();
	void *(*forw)(void *);
	void *(*back)(void *);
	long (*u_p)();
	long (*usize)();
	long (*off)(void *);
	int (*ent)(long, int *, void **, void **);
	void *(*gal)(long);
	char *(*gsl)(void *);
	long *addr_last;
	long *current_addr;
	long *u_current_addr;
	long *u_addr_last;
	int *modified;
	int *mutex;
	int *isglobal;
	int *sigflags;
	int *hup;
	int *intc;
	int *malloc_fail_at;
	int *realloc_fail_at;
	int *realloc_calls;
	const char **errmsg;
	void *uni[UNI];
	int uni_n;
};

Ops R, P;

void init_ops()
{
	R.head = r_head; R.forw = r_forw; R.back = r_back;
	R.u_p = r_u_p; R.usize = r_usize; R.off = r_off; R.ent = r_ent;
	R.gal = r_gal; R.gsl = r_gsl;
	R.addr_last = &addr_last;
	R.current_addr = &current_addr;
	R.u_current_addr = &u_current_addr;
	R.u_addr_last = &u_addr_last;
	R.modified = &modified;
	R.mutex = &mutex;
	R.isglobal = &isglobal;
	R.sigflags = &sigflags;
	R.hup = &oracle_hup_calls;
	R.intc = &oracle_int_calls;
	R.malloc_fail_at = &oracle_malloc_fail_at;
	R.realloc_fail_at = &oracle_realloc_fail_at;
	R.realloc_calls = &oracle_realloc_calls;
	R.errmsg = &errmsg;
	R.uni_n = 0;

	P.head = p_head; P.forw = p_forw; P.back = p_back;
	P.u_p = p_u_p; P.usize = p_usize; P.off = p_off; P.ent = p_ent;
	P.gal = p_gal; P.gsl = p_gsl;
	P.addr_last = &port::addr_last;
	P.current_addr = &port::current_addr;
	P.u_current_addr = &port::u_current_addr;
	P.u_addr_last = &port::u_addr_last;
	P.modified = &port::modified;
	P.mutex = &port::mutex;
	P.isglobal = &port::isglobal;
	P.sigflags = &port::sigflags;
	P.hup = &port::hup_calls;
	P.intc = &port::int_calls;
	P.malloc_fail_at = &port::malloc_fail_at;
	P.realloc_fail_at = &port::realloc_fail_at;
	P.realloc_calls = &port::realloc_calls;
	P.errmsg = &port::errmsg;
	P.uni_n = 0;
}

/* ------------------------------------------------------------------ */
/* observable state                                                     */
/* ------------------------------------------------------------------ */

bool deep_text;

struct Snap {
	long u_p, usize;
	long addr_last, current_addr, u_current_addr, u_addr_last;
	long ret_off;
	int ret_null;
	int modified, mutex, isglobal, hup, intc, realloc_calls;
	int n_ent;
	int ty[SCAP], h[SCAP], t[SCAP];
	int fwd_n, bwd_n;
	int fwd[WCAP], bwd[WCAP];
	int nl;
	char err[48];
	char text[NL][40];
};

/*
 * Line nodes live at different addresses on each side, so identity is
 * expressed as an index into the set of nodes captured at set-up time.
 */
void capture_uni(Ops &o, int nlines)
{
	o.uni_n = 0;
	for (int i = 0; i <= nlines && i < UNI; i++)
		o.uni[o.uni_n++] = o.gal(i);
}

int uid(const Ops &o, void *p)
{
	if (p == nullptr)
		return -3;
	for (int i = 0; i < o.uni_n; i++)
		if (o.uni[i] == p)
			return i;
	return -2;
}

void snap(Ops &o, Snap &s, void *ret)
{
	std::memset(&s, 0, sizeof s);

	s.ret_null = (ret == nullptr);
	s.ret_off = o.off(ret);
	s.u_p = o.u_p();
	s.usize = o.usize();
	s.addr_last = *o.addr_last;
	s.current_addr = *o.current_addr;
	s.u_current_addr = *o.u_current_addr;
	s.u_addr_last = *o.u_addr_last;
	s.modified = *o.modified;
	s.mutex = *o.mutex;
	s.isglobal = *o.isglobal;
	s.hup = *o.hup;
	s.intc = *o.intc;
	s.realloc_calls = *o.realloc_calls;
	std::snprintf(s.err, sizeof s.err, "%s", *o.errmsg);

	for (long i = 0; i < s.u_p && s.n_ent < SCAP; i++) {
		int ty;
		void *h, *t;
		if (!o.ent(i, &ty, &h, &t))
			break;
		s.ty[s.n_ent] = ty;
		s.h[s.n_ent] = uid(o, h);
		s.t[s.n_ent] = uid(o, t);
		s.n_ent++;
	}

	void *bh = o.head();
	for (void *p = o.forw(bh); p != bh && s.fwd_n < WCAP; p = o.forw(p))
		s.fwd[s.fwd_n++] = uid(o, p);
	for (void *p = o.back(bh); p != bh && s.bwd_n < WCAP; p = o.back(p))
		s.bwd[s.bwd_n++] = uid(o, p);

	if (deep_text) {
		long lim = s.addr_last < NL ? s.addr_last : NL;
		for (long i = 1; i <= lim; i++) {
			char *cs = o.gsl(o.gal(i));
			std::snprintf(s.text[s.nl], sizeof s.text[0], "%s",
			    cs ? cs : "<null>");
			s.nl++;
		}
	}
}

struct Stat { const char *name; long cases, fails; };
Stat stats[3];
int nstats;

Stat &reg(const char *n)
{
	stats[nstats++] = {n, 0, 0};
	return stats[nstats - 1];
}

/* Compare after one operation.  ru/pu are the raw returns, if any. */
void check(Stat &st, void *ru, void *pu)
{
	Snap a, b;

	snap(R, a, ru);
	snap(P, b, pu);
	st.cases++;
	if (std::memcmp(&a, &b, sizeof a) != 0)
		st.fails++;
}

/* As check(), for a function returning an int rather than a pointer. */
void check_int(Stat &st, int rr, int rp)
{
	Snap a, b;

	snap(R, a, nullptr);
	snap(P, b, nullptr);
	st.cases++;
	if (rr != rp || std::memcmp(&a, &b, sizeof a) != 0)
		st.fails++;
}

/* ------------------------------------------------------------------ */
/* fixtures                                                             */
/* ------------------------------------------------------------------ */

std::uint64_t rng;

void seed() { rng = 0xb014842facefeedULL; }

std::uint64_t rnd()
{
	std::uint64_t z = (rng += 0x9e3779b97f4a7c15ULL);
	z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
	z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
	return z ^ (z >> 31);
}

long rndl() { return (long)(rnd() & 0x7fff); }

void teardown()
{
	ref_close_sbuf();
	port::close_sbuf();
}

/*
 * Fresh scratch file, nlines lines, and a captured node universe.  ig and
 * sf drive the isglobal branch and the SPL0 signal-flag branches.
 */
bool prep(int nlines, int ig, int sf)
{
	oracle_reset_batch();
	port::reset_batch();
	if (ref_open_sbuf() != 0)
		return false;
	if (port::open_sbuf() != 0)
		return false;
	ref_get_addressed_line_node(0);
	port::get_addressed_line_node(0);
	for (int i = 0; i < nlines; i++) {
		char l[32];
		std::snprintf(l, sizeof l, "L%02d\n", i);
		ref_put_sbuf_line(l);
	}
	for (int i = 0; i < nlines; i++) {
		char l[32];
		std::snprintf(l, sizeof l, "L%02d\n", i);
		port::put_sbuf_line(l);
	}
	isglobal = ig;
	port::isglobal = ig;
	sigflags = sf;
	port::sigflags = sf;
	capture_uni(R, nlines);
	capture_uni(P, nlines);
	oracle_hup_calls = oracle_int_calls = 0;
	port::hup_calls = port::int_calls = 0;
	return true;
}

void set_undo(long uc, long ua)
{
	u_current_addr = uc;
	u_addr_last = ua;
	port::u_current_addr = uc;
	port::u_addr_last = ua;
}

/*
 * UMOV/VMOV entries are read in adjacent pairs by pop_undo_stack, which
 * indexes ustack[n - 1]; pushing them singly would make the oracle read
 * ustack[-1] and compare unrelated heap bytes.  Always push them paired.
 */
void push_both(int ty, long f, long t)
{
	ref_push_undo_stack(ty, f, t);
	port::push_undo_stack(ty, f, t);
	if (ty == 2 || ty == 3) {
		ref_push_undo_stack(ty, f, t);
		port::push_undo_stack(ty, f, t);
	}
}

/* Types that leave every line node owned by the buffer, so that an
 * internal clear_undo_stack cannot free a node still on the list. */
int safe_type(std::uint64_t r)
{
	static const int tab[4] = {0, 2, 3, 4};
	return tab[r & 3];
}

/* ------------------------------------------------------------------ */
/* push_undo_stack                                                      */
/* ------------------------------------------------------------------ */

struct PushCase {
	int nlines;
	int pre;		/* entries already on the stack */
	int type;
	long from, to;
	long albias;		/* perturb addr_last before the call */
	int ig, sf;
	int mfail;		/* malloc failure injection */
	int rfail;		/* 0, or realloc fails after this many more */
};

void run_push(Stat &st, const PushCase &c)
{
	if (!prep(c.nlines, c.ig, c.sf))
		return;
	set_undo(c.nlines / 2, c.nlines);
	for (int i = 0; i < c.pre; i++)
		push_both(safe_type(rnd()), 1 + (i % 3), 1 + (i % 4));

	addr_last += c.albias;
	port::addr_last += c.albias;
	oracle_malloc_fail_at = c.mfail;
	port::malloc_fail_at = c.mfail;
	if (c.rfail) {
		oracle_realloc_fail_at = oracle_realloc_calls + c.rfail;
		port::realloc_fail_at = port::realloc_calls + c.rfail;
	}

	void *ru = ref_push_undo_stack(c.type, c.from, c.to);
	void *pu = port::push_undo_stack(c.type, c.from, c.to);
	check(st, ru, pu);

	/* a second push observes usize/u_p growth and the post-OOM state */
	ru = ref_push_undo_stack(c.type, c.to, c.from);
	pu = port::push_undo_stack(c.type, c.to, c.from);
	check(st, ru, pu);

	oracle_realloc_fail_at = 0;
	port::realloc_fail_at = 0;
	oracle_malloc_fail_at = 0;
	port::malloc_fail_at = 0;
	teardown();
}

void test_push_undo_stack()
{
	Stat &st = reg("push_undo_stack");

	seed();
	deep_text = true;

	/* boundaries of the addressed-node lookups and of the type field */
	run_push(st, {5, 0, 0, 1, 2, 0, 0, 0, 0, 0});
	run_push(st, {5, 0, 1, 1, 3, 0, 0, 0, 0, 0});
	run_push(st, {5, 0, 2, 1, 3, 0, 0, 0, 0, 0});
	run_push(st, {5, 0, 3, 2, 4, 0, 0, 0, 0, 0});
	run_push(st, {5, 0, 4, 1, 1, 0, 0, 0, 0, 0});
	run_push(st, {5, 0, 0, 0, 0, 0, 0, 0, 0, 0});
	run_push(st, {5, 0, 0, 5, 5, 0, 0, 0, 0, 0});
	run_push(st, {5, 0, 0, 5, 0, 0, 0, 0, 0, 0});
	run_push(st, {5, 0, 0, 0, 5, 0, 0, 0, 0, 0});
	run_push(st, {1, 0, 0, 0, 1, 0, 0, 0, 0, 0});
	run_push(st, {1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
	run_push(st, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
	run_push(st, {2, 0, 0, 1, 2, 0, 1, 3, 0, 0});
	run_push(st, {5, 3, 0, 1, 2, 0, 0, 0, 0, 0});
	run_push(st, {5, 7, 2, 1, 4, 0, 0, 0, 0, 0});
	run_push(st, {5, 0, 1, 2, 4, 0, 0, 0, 1, 0});

	/*
	 * A lying addr_last makes get_addressed_line_node's resting position
	 * matter, so the order of the two lookups becomes observable.
	 */
	for (long k = 1; k <= 4; k++)
		for (long f = 0; f <= 6; f++)
			for (long t = 0; t <= 6; t++)
				run_push(st, {5, 0, 0, f, t, k, 0, 0, 0, 0});
	for (long k = 1; k <= 3; k++)
		run_push(st, {5, 2, 2, 4, 5, k, 0, 3, 0, 0});

	/* realloc failure: the out-of-memory tail of push_undo_stack */
	run_push(st, {5, 0, 0, 1, 2, 0, 0, 0, 0, 1});
	run_push(st, {5, 0, 0, 1, 2, 0, 0, 0, 0, 2});
	run_push(st, {5, 4, 0, 1, 2, 0, 0, 0, 0, 1});
	run_push(st, {5, 4, 2, 1, 2, 0, 1, 3, 0, 1});
	run_push(st, {5, 6, 3, 0, 5, 2, 0, 1, 0, 1});

	/* fill the first USIZE slots so the realloc growth path is retaken */
	for (int rf = 0; rf <= 2; rf++) {
		if (!prep(5, 0, 0))
			continue;
		set_undo(3, 5);
		for (int i = 0; i < 99; i++)
			push_both(0, 1 + (i % 4), 1 + (i % 5));
		if (rf) {
			oracle_realloc_fail_at = oracle_realloc_calls + rf;
			port::realloc_fail_at = port::realloc_calls + rf;
		}
		for (int i = 0; i < 6; i++) {
			void *ru = ref_push_undo_stack(i % 5, 1, 5);
			void *pu = port::push_undo_stack(i % 5, 1, 5);
			check(st, ru, pu);
		}
		oracle_realloc_fail_at = 0;
		port::realloc_fail_at = 0;
		teardown();
	}

	for (long i = 0; i < PUSH_ITERS; i++) {
		deep_text = (i % 64) == 0;
		PushCase c;
		c.nlines = (int)(rnd() % 7);
		c.pre = (int)(rnd() % 6);
		c.type = (int)(rnd() % 6);
		c.from = (long)(rnd() % 9);
		c.to = (long)(rnd() % 9);
		c.albias = (long)(rnd() % 5);
		c.ig = (int)(rnd() & 1);
		c.sf = (int)(rnd() % 4);
		c.mfail = (rnd() % 64 == 0) ? 1 : 0;
		c.rfail = (rnd() % 32 == 0) ? 1 + (int)(rnd() % 2) : 0;
		run_push(st, c);
	}
}

/* ------------------------------------------------------------------ */
/* pop_undo_stack                                                       */
/* ------------------------------------------------------------------ */

struct PopCase {
	int nlines;
	int pushes;
	long uc, ua;		/* u_current_addr, u_addr_last */
	long ca, al;		/* current_addr, addr_last before the call */
	int ig, sf;
	int types;		/* 0: mixed 0/2/3, 1: include UDEL, 2: only 0 */
	int twice;
};

void run_pop(Stat &st, const PopCase &c)
{
	if (!prep(c.nlines, c.ig, c.sf))
		return;
	set_undo(c.uc, c.ua);
	for (int i = 0; i < c.pushes; i++) {
		int ty;
		if (c.types == 1)
			ty = (int)(rnd() % 5);
		else if (c.types == 2)
			ty = 0;
		else
			ty = safe_type(rnd());
		push_both(ty, (long)(rnd() % (c.nlines + 1)),
		    (long)(rnd() % (c.nlines + 1)));
	}
	if (c.ca >= 0) {
		current_addr = c.ca;
		port::current_addr = c.ca;
	}
	if (c.al >= 0) {
		addr_last = c.al;
		port::addr_last = c.al;
	}

	int rr = ref_pop_undo_stack();
	int rp = port::pop_undo_stack();
	check_int(st, rr, rp);

	if (c.twice) {
		rr = ref_pop_undo_stack();
		rp = port::pop_undo_stack();
		check_int(st, rr, rp);
	}
	teardown();
}

void test_pop_undo_stack()
{
	Stat &st = reg("pop_undo_stack");

	seed();
	deep_text = true;

	/* both sides of each half of the "nothing to undo" test */
	run_pop(st, {4, 0, -1, -1, -1, -1, 0, 0, 0, 0});
	run_pop(st, {4, 0, -1, 4, -1, -1, 0, 0, 0, 0});
	run_pop(st, {4, 0, 2, -1, -1, -1, 0, 0, 0, 0});
	run_pop(st, {4, 0, 2, 4, -1, -1, 0, 0, 0, 0});
	run_pop(st, {4, 0, 0, 0, -1, -1, 0, 0, 0, 0});
	run_pop(st, {4, 2, -1, -1, -1, -1, 0, 0, 0, 0});
	run_pop(st, {4, 2, 0, -1, -1, -1, 0, 0, 0, 0});
	run_pop(st, {4, 2, -1, 0, -1, -1, 0, 0, 0, 0});
	run_pop(st, {4, 2, -2, -2, -1, -1, 0, 0, 0, 0});

	/* stack depths around the reversal boundary (u_p + 1) / 2 */
	for (int n = 0; n <= 9; n++) {
		run_pop(st, {4, n, 2, 4, -1, -1, 0, 0, 2, 1});
		run_pop(st, {4, n, 2, 4, -1, -1, 1, 0, 2, 1});
		run_pop(st, {4, n, 2, 4, -1, -1, 0, 3, 0, 1});
		run_pop(st, {4, n, 0, 0, -1, -1, 0, 0, 1, 0});
	}

	/* the undo/current address exchange */
	run_pop(st, {4, 1, 0, 4, 3, 4, 0, 0, 2, 0});
	run_pop(st, {4, 1, 4, 0, 0, 4, 0, 0, 2, 0});
	run_pop(st, {4, 1, 3, 3, 3, 3, 0, 0, 2, 0});
	run_pop(st, {4, 1, 1, 2, 3, 4, 0, 0, 2, 1});
	run_pop(st, {0, 0, 0, 0, 0, 0, 0, 0, 2, 1});
	run_pop(st, {1, 1, 1, 1, 1, 1, 0, 0, 2, 1});

	/* signal-flag combinations drive both SPL0 branches */
	for (int sf = 0; sf < 4; sf++)
		run_pop(st, {5, 3, 2, 5, -1, -1, sf & 1, sf, 0, 1});

	for (long i = 0; i < POP_ITERS; i++) {
		deep_text = (i % 64) == 0;
		PopCase c;
		c.nlines = (int)(rnd() % 7);
		c.pushes = (int)(rnd() % 10);
		std::uint64_t m = rnd() % 8;
		c.uc = (m & 1) ? -1 : (long)(rnd() % (c.nlines + 2));
		c.ua = (m & 2) ? -1 : (long)(rnd() % (c.nlines + 2));
		c.ca = (m & 4) ? -1 : (long)(rnd() % (c.nlines + 1));
		c.al = -1;
		c.ig = (int)(rnd() & 1);
		c.sf = (int)(rnd() % 4);
		c.types = (int)(rnd() % 3);
		c.twice = (int)(rnd() & 1);
		run_pop(st, c);
	}
}

/* ------------------------------------------------------------------ */
/* clear_undo_stack                                                     */
/* ------------------------------------------------------------------ */

struct ClearCase {
	int nlines;
	int pushes;
	int orphans;		/* UDEL entries over unlinked nodes */
	long ca, al;
	int ig, sf;
	int twice;
	int popfirst;
};

void run_clear(Stat &st, const ClearCase &c)
{
	if (!prep(c.nlines, c.ig, c.sf))
		return;
	set_undo(c.nlines / 2, c.nlines);
	for (int i = 0; i < c.pushes; i++)
		push_both(safe_type(rnd()), (long)(rnd() % (c.nlines + 1)),
		    (long)(rnd() % (c.nlines + 1)));
	for (int i = 0; i < c.orphans && addr_last > 1; i++) {
		long a = 1 + (long)(rnd() % addr_last);
		oracle_inject_orphan_udel(a);
		port::inject_orphan_udel(a);
	}
	if (c.popfirst) {
		ref_pop_undo_stack();
		port::pop_undo_stack();
	}
	if (c.ca >= 0) {
		current_addr = c.ca;
		port::current_addr = c.ca;
	}
	if (c.al >= 0) {
		addr_last = c.al;
		port::addr_last = c.al;
	}

	ref_clear_undo_stack();
	port::clear_undo_stack();
	check(st, nullptr, nullptr);

	if (c.twice) {
		ref_clear_undo_stack();
		port::clear_undo_stack();
		check(st, nullptr, nullptr);
	}
	teardown();
}

void test_clear_undo_stack()
{
	Stat &st = reg("clear_undo_stack");

	seed();
	deep_text = true;

	run_clear(st, {5, 0, 0, -1, -1, 0, 0, 1, 0});
	run_clear(st, {5, 1, 0, -1, -1, 0, 0, 1, 0});
	run_clear(st, {5, 2, 0, -1, -1, 0, 0, 0, 0});
	run_clear(st, {5, 0, 1, -1, -1, 0, 0, 1, 0});
	run_clear(st, {5, 0, 2, -1, -1, 0, 0, 1, 0});
	run_clear(st, {5, 3, 1, -1, -1, 0, 0, 1, 0});
	run_clear(st, {5, 3, 1, -1, -1, 0, 0, 0, 1});
	run_clear(st, {5, 6, 3, -1, -1, 1, 3, 1, 0});
	run_clear(st, {1, 0, 1, -1, -1, 0, 0, 1, 0});
	run_clear(st, {2, 1, 1, -1, -1, 0, 0, 1, 0});
	run_clear(st, {0, 0, 0, -1, -1, 0, 0, 1, 0});
	run_clear(st, {5, 0, 0, 0, 0, 0, 0, 1, 0});
	run_clear(st, {5, 0, 0, 4, 2, 0, 0, 1, 0});
	run_clear(st, {5, 0, 0, 2, 4, 0, 0, 1, 0});
	run_clear(st, {5, 12, 0, -1, -1, 0, 0, 1, 0});

	/*
	 * A multi-line UDEL run: the inner loop walks h forwards to t's
	 * successor, so it must free more than one node.
	 */
	for (int n = 2; n <= 4; n++) {
		if (!prep(6, 0, 0))
			continue;
		set_undo(3, 6);
		for (int i = 0; i < n; i++) {
			long a = 1 + (long)(rnd() % addr_last);
			oracle_inject_orphan_udel(a);
			port::inject_orphan_udel(a);
		}
		ref_clear_undo_stack();
		port::clear_undo_stack();
		check(st, nullptr, nullptr);
		teardown();
	}

	for (long i = 0; i < CLEAR_ITERS; i++) {
		deep_text = (i % 64) == 0;
		ClearCase c;
		c.nlines = (int)(rnd() % 7);
		c.pushes = (int)(rnd() % 8);
		c.orphans = (int)(rnd() % 3);
		c.ca = (rnd() & 1) ? -1 : (long)(rnd() % 6);
		c.al = (rnd() & 1) ? -1 : (long)(rnd() % 6);
		c.ig = (int)(rnd() & 1);
		c.sf = (int)(rnd() % 4);
		c.twice = (int)(rnd() & 1);
		c.popfirst = (int)(rnd() & 1);
		run_clear(st, c);
	}
}

} /* namespace */

int main()
{
	init_ops();

	test_push_undo_stack();
	test_pop_undo_stack();
	test_clear_undo_stack();

	oracle_reset_batch();
	port::reset_batch();

	std::printf("PBSD batch b0148s2 differential test\n\n");
	std::printf("%-28s %12s %12s %10s\n", "function", "cases", "failures",
	    "result");
	long tc = 0, tf = 0;
	for (int i = 0; i < nstats; i++) {
		tc += stats[i].cases;
		tf += stats[i].fails;
		std::printf("%-28s %12ld %12ld %10s\n", stats[i].name,
		    stats[i].cases, stats[i].fails,
		    stats[i].fails ? "FAIL" : "PASS");
	}
	std::printf("%-28s %12ld %12ld %10s\n", "TOTAL", tc, tf,
	    tf ? "FAIL" : "PASS");
	return tf ? 1 : 0;
}
