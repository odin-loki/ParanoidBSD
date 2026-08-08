/* harness.cpp: differential test of the b0148s3 port against the C oracle.
 *
 * Every operation is applied to BOTH implementations and, after each one, the
 * complete observable state is compared: return value, active_last,
 * active_size, active_ptr, active_ndx, the critical-section counter, errmsg,
 * and the whole active_list array up to the high-water mark of everything ever
 * written to it (not merely the [0, active_last) window).  Node pools live
 * inside 0x7f guard bands that are re-verified after every call, and every
 * pointer is compared as an index into its own pool, never as a raw address.
 */

#include <sys/types.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

import pbsd.bin.ed.b0148s3;

namespace P = pbsd::bin_ed::b0148s3;

extern "C" {

struct line {
	struct line	*q_forw;
	struct line	*q_back;
	off_t		seek;
	int		len;
};
typedef struct line ref_line_t;

int ref_set_active_node(ref_line_t *lp);
void ref_unset_active_nodes(ref_line_t *np, ref_line_t *mp);
ref_line_t *ref_next_active_node(void);
void ref_clear_active_list(void);

long ref_get_active_last(void);
long ref_get_active_size(void);
long ref_get_active_ptr(void);
long ref_get_active_ndx(void);
ref_line_t **ref_get_active_list(void);
int ref_get_mutex(void);
char *ref_get_errmsg(void);
void ref_reset_errmsg(void);

}

/* ------------------------------------------------------------------ pools */

#define NPOOL	64
#define GUARD	64
#define GBYTE	0x7f

struct PortPool {
	unsigned char lead[GUARD];
	P::line_t nodes[NPOOL];
	unsigned char trail[GUARD];
};

struct RefPool {
	unsigned char lead[GUARD];
	ref_line_t nodes[NPOOL];
	unsigned char trail[GUARD];
};

static PortPool ppool;
static RefPool rpool;

static off_t node_seek(long i) { return (off_t)(i * 7 + 1); }
static int node_len(long i) { return (int)(i * 3 + 2); }

static void
init_pools(void)
{
	memset(&ppool, GBYTE, sizeof(ppool));
	memset(&rpool, GBYTE, sizeof(rpool));
	for (long i = 0; i < NPOOL; i++) {
		long f = (i + 1) % NPOOL;
		long b = (i + NPOOL - 1) % NPOOL;
		ppool.nodes[i].q_forw = &ppool.nodes[f];
		ppool.nodes[i].q_back = &ppool.nodes[b];
		ppool.nodes[i].seek = node_seek(i);
		ppool.nodes[i].len = node_len(i);
		rpool.nodes[i].q_forw = &rpool.nodes[f];
		rpool.nodes[i].q_back = &rpool.nodes[b];
		rpool.nodes[i].seek = node_seek(i);
		rpool.nodes[i].len = node_len(i);
	}
}

/*
 * Pointer -> index.  Pointers outside the pool (which only a broken port can
 * produce) map to a large negative marker derived from the raw distance so the
 * two sides still compare unequal.
 */
static long
pidx(P::line_t *p)
{
	if (p == NULL)
		return -1;
	long d = (long)(p - ppool.nodes);
	if (d < 0 || d >= NPOOL)
		return -1000000 - d;
	return d;
}

static long
ridx(ref_line_t *p)
{
	if (p == NULL)
		return -1;
	long d = (long)(p - rpool.nodes);
	if (d < 0 || d >= NPOOL)
		return -1000000 - d;
	return d;
}

static bool
check_pools(char *d, size_t n)
{
	for (long i = 0; i < GUARD; i++) {
		if (ppool.lead[i] != GBYTE || ppool.trail[i] != GBYTE) {
			snprintf(d, n, "port pool guard byte %ld clobbered", i);
			return false;
		}
		if (rpool.lead[i] != GBYTE || rpool.trail[i] != GBYTE) {
			snprintf(d, n, "ref pool guard byte %ld clobbered", i);
			return false;
		}
	}
	for (long i = 0; i < NPOOL; i++) {
		long f = (i + 1) % NPOOL;
		long b = (i + NPOOL - 1) % NPOOL;
		if (ppool.nodes[i].q_forw != &ppool.nodes[f] ||
		    ppool.nodes[i].q_back != &ppool.nodes[b] ||
		    ppool.nodes[i].seek != node_seek(i) ||
		    ppool.nodes[i].len != node_len(i)) {
			snprintf(d, n, "port pool node %ld modified", i);
			return false;
		}
		if (rpool.nodes[i].q_forw != &rpool.nodes[f] ||
		    rpool.nodes[i].q_back != &rpool.nodes[b] ||
		    rpool.nodes[i].seek != node_seek(i) ||
		    rpool.nodes[i].len != node_len(i)) {
			snprintf(d, n, "ref pool node %ld modified", i);
			return false;
		}
	}
	return true;
}

/* ------------------------------------------------------------ bookkeeping */

enum { F_SET = 0, F_UNSET, F_NEXT, F_CLEAR, NFUNC };

static const char *fname[NFUNC] = {
	"set_active_node",
	"unset_active_nodes",
	"next_active_node",
	"clear_active_list",
};

static long long ncase[NFUNC];
static long long nfail[NFUNC];
static int nprinted;

/*
 * High-water mark of active_list slots ever written since the last clear.
 * Everything below it has been stored to by both implementations, so the whole
 * region is comparable -- including the slots past the current active_last.
 */
static long hw;

static void
record(int fn, const char *ctx, const char *diag)
{
	nfail[fn]++;
	if (nprinted < 25) {
		printf("FAIL %-20s %-28s %s\n", fname[fn], ctx, diag);
		nprinted++;
	} else if (nprinted == 25) {
		printf("... further failures suppressed\n");
		nprinted++;
	}
}

static bool
check_state(char *d, size_t n)
{
	long pv, rv;
	int pi, ri;

	pv = P::get_active_last();
	rv = ref_get_active_last();
	if (pv != rv) {
		snprintf(d, n, "active_last port=%ld ref=%ld", pv, rv);
		return false;
	}
	long last = pv;

	pv = P::get_active_size();
	rv = ref_get_active_size();
	if (pv != rv) {
		snprintf(d, n, "active_size port=%ld ref=%ld", pv, rv);
		return false;
	}
	pv = P::get_active_ptr();
	rv = ref_get_active_ptr();
	if (pv != rv) {
		snprintf(d, n, "active_ptr port=%ld ref=%ld", pv, rv);
		return false;
	}
	pv = P::get_active_ndx();
	rv = ref_get_active_ndx();
	if (pv != rv) {
		snprintf(d, n, "active_ndx port=%ld ref=%ld", pv, rv);
		return false;
	}
	pi = P::get_mutex();
	ri = ref_get_mutex();
	if (pi != ri) {
		snprintf(d, n, "mutex port=%d ref=%d", pi, ri);
		return false;
	}

	char *pe = P::get_errmsg();
	char *re = ref_get_errmsg();
	if ((pe == NULL) != (re == NULL) ||
	    (pe != NULL && re != NULL && strcmp(pe, re) != 0)) {
		snprintf(d, n, "errmsg port=%s ref=%s",
		    pe ? pe : "(null)", re ? re : "(null)");
		return false;
	}

	P::line_t **plist = P::get_active_list();
	ref_line_t **rlist = ref_get_active_list();
	if ((plist == NULL) != (rlist == NULL)) {
		snprintf(d, n, "active_list nullness port=%d ref=%d",
		    plist == NULL, rlist == NULL);
		return false;
	}

	if (last > hw)
		hw = last;
	if (plist != NULL && rlist != NULL) {
		for (long i = 0; i < hw; i++) {
			long a = pidx(plist[i]);
			long b = ridx(rlist[i]);
			if (a != b) {
				snprintf(d, n,
				    "active_list[%ld] port=%ld ref=%ld "
				    "(last=%ld hw=%ld)", i, a, b, last, hw);
				return false;
			}
		}
	}

	return check_pools(d, n);
}

/* ------------------------------------------------------------- operations */

static void
op_set(long ni, const char *ctx)
{
	char d[256];

	ncase[F_SET]++;
	P::line_t *pl = (ni < 0) ? NULL : &ppool.nodes[ni];
	ref_line_t *rl = (ni < 0) ? NULL : &rpool.nodes[ni];

	int a = P::set_active_node(pl);
	int b = ref_set_active_node(rl);
	if (a != b) {
		snprintf(d, sizeof(d), "return port=%d ref=%d (node=%ld)",
		    a, b, ni);
		record(F_SET, ctx, d);
		return;
	}
	if (!check_state(d, sizeof(d)))
		record(F_SET, ctx, d);
}

static void
op_unset(long ai, long bi, const char *ctx)
{
	char d[256];

	ncase[F_UNSET]++;
	P::unset_active_nodes(&ppool.nodes[ai], &ppool.nodes[bi]);
	ref_unset_active_nodes(&rpool.nodes[ai], &rpool.nodes[bi]);
	if (!check_state(d, sizeof(d)))
		record(F_UNSET, ctx, d);
}

static void
op_next(const char *ctx)
{
	char d[256];

	ncase[F_NEXT]++;
	P::line_t *a = P::next_active_node();
	ref_line_t *b = ref_next_active_node();
	long ia = pidx(a), ib = ridx(b);
	if (ia != ib) {
		snprintf(d, sizeof(d), "returned offset port=%ld ref=%ld",
		    ia, ib);
		record(F_NEXT, ctx, d);
		return;
	}
	if (!check_state(d, sizeof(d)))
		record(F_NEXT, ctx, d);
}

static void
op_clear(const char *ctx)
{
	char d[256];

	ncase[F_CLEAR]++;
	P::clear_active_list();
	ref_clear_active_list();
	hw = 0;
	if (!check_state(d, sizeof(d)))
		record(F_CLEAR, ctx, d);
}

/* Drive the iterator to exhaustion, comparing after every step. */
static void
drain(const char *ctx, long limit)
{
	for (long i = 0; i < limit; i++) {
		bool done = (P::get_active_ptr() >= P::get_active_last());
		op_next(ctx);
		if (done)
			break;
	}
}

/* ------------------------------------------------------------- edge cases */

static void
edge_cases(void)
{
	/* virgin state: clear with active_list == NULL, then empty queries */
	op_clear("edge/virgin-clear");
	op_clear("edge/double-clear");
	op_next("edge/next-empty");
	op_next("edge/next-empty2");

	/* unset on an empty active list: inner loop never runs */
	op_unset(0, 0, "edge/unset-empty-np==mp");
	op_unset(0, 1, "edge/unset-empty-1node");
	op_unset(0, 5, "edge/unset-empty-5nodes");
	op_unset(60, 3, "edge/unset-empty-wrap");

	/* single element */
	op_clear("edge/1-clear");
	op_set(0, "edge/1-set");
	op_next("edge/1-next");
	op_next("edge/1-next-exhausted");
	op_next("edge/1-next-exhausted2");

	/* np == mp must not iterate at all even with a matching entry */
	op_clear("edge/np==mp-clear");
	op_set(0, "edge/np==mp-set");
	op_unset(0, 0, "edge/np==mp-nomove");
	op_unset(0, 0, "edge/np==mp-nomove2");
	op_next("edge/np==mp-next");

	/* single element, matched: INC_MOD(0, 0) */
	op_clear("edge/1-match-clear");
	op_set(0, "edge/1-match-set");
	op_unset(0, 1, "edge/1-match-unset");
	op_next("edge/1-match-next");
	op_next("edge/1-match-next2");

	/* single element, unmatched: full cycle leaves ndx alone */
	op_clear("edge/1-nomatch-clear");
	op_set(0, "edge/1-nomatch-set");
	op_unset(9, 10, "edge/1-nomatch-unset");
	op_next("edge/1-nomatch-next");

	/* two elements: exercises INC_MOD with k == 1 on both sides */
	op_clear("edge/2-clear");
	op_set(0, "edge/2-setA");
	op_set(1, "edge/2-setB");
	op_unset(1, 2, "edge/2-unsetB");	/* miss then hit, wrap to 0 */
	op_unset(0, 1, "edge/2-unsetA");
	op_next("edge/2-next1");
	op_next("edge/2-next2");

	/* three elements: INC_MOD with k == 2, hits l+1 == k exactly */
	op_clear("edge/3-clear");
	op_set(0, "edge/3-setA");
	op_set(1, "edge/3-setB");
	op_set(2, "edge/3-setC");
	op_unset(0, 1, "edge/3-unsetA");	/* hit at ndx 0 -> ndx 1 */
	op_unset(2, 3, "edge/3-unsetC");	/* miss at 1, hit at 2 -> 0 */
	op_unset(9, 10, "edge/3-unset-absent");	/* full no-match cycle */
	drain("edge/3-drain", 8);

	/* four elements, no match: exact number of INC_MOD steps matters */
	op_clear("edge/4-clear");
	op_set(0, "edge/4-setA");
	op_set(1, "edge/4-setB");
	op_set(2, "edge/4-setC");
	op_set(3, "edge/4-setD");
	op_unset(20, 21, "edge/4-absent1");
	op_unset(20, 22, "edge/4-absent2");
	op_unset(20, 23, "edge/4-absent3");
	op_unset(1, 2, "edge/4-hitB");
	op_unset(3, 4, "edge/4-hitD");
	drain("edge/4-drain", 8);

	/* duplicates: only one entry per outer iteration is cleared (break) */
	op_clear("edge/dup-clear");
	op_set(1, "edge/dup-set1");
	op_set(1, "edge/dup-set2");
	op_set(1, "edge/dup-set3");
	op_unset(1, 2, "edge/dup-unset1");
	op_unset(1, 2, "edge/dup-unset2");
	op_unset(1, 2, "edge/dup-unset3");
	op_unset(1, 2, "edge/dup-unset4");
	drain("edge/dup-drain", 8);

	/* NULL entries stored explicitly */
	op_clear("edge/null-clear");
	op_set(-1, "edge/null-set1");
	op_set(-1, "edge/null-set2");
	op_set(3, "edge/null-set3");
	op_unset(3, 4, "edge/null-unset");
	op_set(4, "edge/null-set4");
	drain("edge/null-drain", 8);

	/* leading NULLs skipped by the iterator */
	op_clear("edge/skip-clear");
	op_set(-1, "edge/skip-setnull1");
	op_set(-1, "edge/skip-setnull2");
	op_set(5, "edge/skip-setnode");
	op_next("edge/skip-next1");
	op_next("edge/skip-next2");

	/* growth after exhaustion: active_ptr == active_last, then last grows */
	op_clear("edge/regrow-clear");
	op_set(7, "edge/regrow-set1");
	drain("edge/regrow-drain1", 4);
	op_set(8, "edge/regrow-set2");
	op_next("edge/regrow-next1");
	op_next("edge/regrow-next2");
	op_set(9, "edge/regrow-set3");
	op_set(-1, "edge/regrow-set4");
	op_next("edge/regrow-next3");
	op_next("edge/regrow-next4");
	op_next("edge/regrow-next5");

	/* multi-node range walking the ring forwards */
	op_clear("edge/range-clear");
	for (long i = 0; i < 8; i++)
		op_set(i, "edge/range-set");
	op_unset(0, 8, "edge/range-unset-8");
	drain("edge/range-drain", 12);

	/* range that wraps around the end of the pool ring */
	op_clear("edge/wrap-clear");
	op_set(62, "edge/wrap-set62");
	op_set(63, "edge/wrap-set63");
	op_set(0, "edge/wrap-set0");
	op_set(1, "edge/wrap-set1");
	op_unset(62, 2, "edge/wrap-unset");
	drain("edge/wrap-drain", 8);

	/*
	 * Buffer growth: active_size steps at 512, 1024, 1536, ...  Every
	 * single insertion is checked, so an off-by-one in the
	 * active_last + 1 > active_size test shows up immediately.
	 */
	op_clear("edge/grow-clear");
	for (long i = 0; i < 3000; i++) {
		char ctx[48];
		snprintf(ctx, sizeof(ctx), "edge/grow-set-%ld", i);
		op_set((i * 13) % NPOOL, ctx);
		if (i % 97 == 0)
			op_next("edge/grow-next");
		if (i % 211 == 0)
			op_unset(i % NPOOL, (i + 1) % NPOOL, "edge/grow-unset");
	}
	op_unset(0, 3, "edge/grow-unset-big");
	drain("edge/grow-drain", 4000);
	op_clear("edge/grow-final-clear");
	op_next("edge/grow-final-next");
}

/* -------------------------------------------- exhaustive small-state sweep */

/*
 * Every list content pattern over {NULL, A, B, C, D} for lengths 0..4, crossed
 * with every unset range start and length, so that the (active_last,
 * active_ndx, contents) triple is driven through essentially all of its small
 * reachable states -- which is where the INC_MOD wrap boundary lives.
 */
static void
exhaustive_small(void)
{
	char ctx[64];

	for (long len = 0; len <= 4; len++) {
		long npat = 1;
		for (long i = 0; i < len; i++)
			npat *= 5;
		for (long pat = 0; pat < npat; pat++) {
			for (long u = 0; u < 5; u++) {
				for (long c = 0; c <= 3; c++) {
					snprintf(ctx, sizeof(ctx),
					    "small/%ld/%ld/%ld/%ld",
					    len, pat, u, c);
					op_clear(ctx);
					long p = pat;
					for (long i = 0; i < len; i++) {
						long e = p % 5;
						p /= 5;
						op_set(e - 1, ctx);
					}
					op_unset(u, (u + c) % NPOOL, ctx);
					op_unset((u + 1) % NPOOL,
					    (u + 1 + c) % NPOOL, ctx);
					op_unset(u, (u + c) % NPOOL, ctx);
					op_next(ctx);
					op_next(ctx);
				}
			}
		}
	}
}

/* ------------------------------------------------------------ random sweep */

static uint64_t rng_state;

static uint64_t
rnd(void)
{
	uint64_t x = rng_state;
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	rng_state = x;
	return x * 2685821657736338717ULL;
}

static long
rrange(long n)
{
	return (long)(rnd() % (uint64_t)n);
}

static long
random_phase(const char *name, uint64_t seed, long iters, long maxlen,
    long maxrange, long clear_den, long nodes_used)
{
	char ctx[64];

	rng_state = seed;
	for (long it = 0; it < iters; it++) {
		snprintf(ctx, sizeof(ctx), "%s#%ld", name, it);
		if (P::get_active_last() > maxlen) {
			op_clear(ctx);
			continue;
		}
		if (rrange(clear_den) == 0) {
			op_clear(ctx);
			continue;
		}
		long r = rrange(100);
		if (r < 46) {
			op_set(rrange(nodes_used + 1) - 1, ctx);
		} else if (r < 70) {
			long a = rrange(nodes_used);
			long c = rrange(maxrange + 1);
			op_unset(a, (a + c) % NPOOL, ctx);
		} else {
			op_next(ctx);
		}
	}
	return iters;
}

/* -------------------------------------------------------------------- main */

int
main(void)
{
	init_pools();

	edge_cases();
	exhaustive_small();

	long iters = 0;
	iters += random_phase("rnd-tiny", 0x0123456789abcdefULL, 90000,
	    6, 3, 12, 4);
	iters += random_phase("rnd-mid", 0x9e3779b97f4a7c15ULL, 70000,
	    40, 5, 60, 12);
	iters += random_phase("rnd-big", 0xdeadbeefcafebabeULL, 50000,
	    1300, 1, 4000, 40);

	op_clear("final-clear");

	long long tc = 0, tf = 0;
	printf("\n%-22s %12s %12s\n", "function", "cases", "failures");
	printf("%-22s %12s %12s\n", "----------------------",
	    "------------", "------------");
	for (int i = 0; i < NFUNC; i++) {
		printf("%-22s %12lld %12lld\n", fname[i], ncase[i], nfail[i]);
		tc += ncase[i];
		tf += nfail[i];
	}
	printf("%-22s %12s %12s\n", "----------------------",
	    "------------", "------------");
	printf("%-22s %12lld %12lld\n", "TOTAL", tc, tf);
	printf("\nrandom iterations: %ld\n", iters);
	printf("result: %s\n", tf == 0 ? "PASS" : "FAIL");

	return tf == 0 ? 0 : 1;
}
