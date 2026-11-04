/*
 * harness.cpp -- differential test for PBSD batch b0298.
 *
 * Every case runs against BOTH the C++23 port and the ref_ oracle.
 * insque: entire guarded region including pointer link fields encoded as
 * offsets from the region base.
 * tfind: return pointer as offset from the tree arena base (never an address).
 * twalk: full visit trace (node offset, VISIT, level) for every callback.
 */

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.stdlib.b0298;

namespace P = pbsd::lib_libc_stdlib::b0298;

extern "C" {
void ref_insque(void *element, void *pred);
P::posix_tnode *ref_tfind(const void *, P::posix_tnode * const *,
    int (*)(const void *, const void *));
void ref_twalk(const P::posix_tnode *,
    void (*)(const P::posix_tnode *, int, int));
}

namespace {

struct Stat {
	const char *name;
	unsigned long cases;
	unsigned long fails;
	unsigned reported;
};

static Stat st_insque = { "insque", 0, 0, 0 };
static Stat st_tfind = { "tfind", 0, 0, 0 };
static Stat st_twalk = { "twalk", 0, 0, 0 };

static const unsigned char GUARD = 0x7f;
static const unsigned SWEEP_ITERS = 200000u;

static void
check(Stat &s, bool ok, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

static void
check(Stat &s, bool ok, const char *fmt, ...)
{
	va_list ap;

	s.cases++;
	if (ok)
		return;
	s.fails++;
	if (s.reported < 15) {
		s.reported++;
		std::fprintf(stderr, "FAIL [%s] ", s.name);
		va_start(ap, fmt);
		std::vfprintf(stderr, fmt, ap);
		va_end(ap);
		std::fputc('\n', stderr);
	} else if (s.reported == 15) {
		s.reported++;
		std::fprintf(stderr, "FAIL [%s] (further failures suppressed)\n",
		    s.name);
	}
}

static std::uint64_t rng_state = 0x9e3779b97f4a7c15ull;

static std::uint64_t
rng_next(void)
{
	std::uint64_t x = rng_state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	rng_state = x;
	return (x * 0x2545f4914f6cdd1dull);
}

static unsigned
rng_below(unsigned n)
{
	return (n == 0 ? 0u : (unsigned)(rng_next() % n));
}

static int
icmp(const void *a, const void *b)
{
	int x = *(const int *)a;
	int y = *(const int *)b;

	return (x > y) - (x < y);
}

static int
icmp_scaled(const void *a, const void *b)
{
	int x = *(const int *)a;
	int y = *(const int *)b;
	int d = x - y;

	if (d == 0)
		return (0);
	return (d < 0 ? -3 : 4);
}

static int
icmp_highbit(const void *a, const void *b)
{
	unsigned char x = (unsigned char)*(const int *)a;
	unsigned char y = (unsigned char)*(const int *)b;

	return ((int)x > (int)y) - ((int)x < (int)y);
}

typedef int (*cmp_fn)(const void *, const void *);

static cmp_fn
pick_cmp(unsigned tag)
{
	switch (tag % 3u) {
	case 0:
		return (icmp);
	case 1:
		return (icmp_scaled);
	default:
		return (icmp_highbit);
	}
}

struct Elem {
	Elem *next;
	Elem *prev;
};

enum { NEL = 8, IPAD = 16 };

struct InsRegion {
	unsigned char pre[IPAD];
	Elem el[NEL];
	unsigned char post[IPAD];
};

static void
ins_build(InsRegion &r, const int *nx, const int *pv)
{
	std::memset(&r, GUARD, sizeof(r));
	for (int i = 0; i < NEL; i++) {
		r.el[i].next = (nx[i] < 0) ? nullptr : &r.el[nx[i]];
		r.el[i].prev = (pv[i] < 0) ? nullptr : &r.el[pv[i]];
	}
}

static long
ins_enc(const InsRegion &r, const void *p)
{
	if (p == nullptr)
		return (-1);
	const unsigned char *base =
	    reinterpret_cast<const unsigned char *>(&r);
	const unsigned char *q = reinterpret_cast<const unsigned char *>(p);
	if (q < base || q >= base + sizeof(InsRegion))
		return (-2);
	return (q - base);
}

static bool
ins_region_equal(const InsRegion &a, const InsRegion &b)
{
	if (std::memcmp(a.pre, b.pre, IPAD) != 0)
		return (false);
	if (std::memcmp(a.post, b.post, IPAD) != 0)
		return (false);
	for (int i = 0; i < NEL; i++) {
		if (ins_enc(a, a.el[i].next) != ins_enc(b, b.el[i].next))
			return (false);
		if (ins_enc(a, a.el[i].prev) != ins_enc(b, b.el[i].prev))
			return (false);
	}
	return (true);
}

static void
case_insque(const int *nx, const int *pv, int elem, int pred)
{
	InsRegion a, b;
	void *pa, *pb;

	ins_build(a, nx, pv);
	ins_build(b, nx, pv);

	if (pred < 0) {
		pa = nullptr;
		pb = nullptr;
	} else {
		pa = &a.el[pred];
		pb = &b.el[pred];
	}

	P::insque(&a.el[elem], pa);
	ref_insque(&b.el[elem], pb);

	check(st_insque, ins_region_equal(a, b),
	    "elem=%d pred=%d region differs", elem, pred);
}

static int
ins_pick_link(void)
{
	std::uint64_t r = rng_next();

	if ((r & 7u) == 0u)
		return (-1);
	return ((int)((r >> 3) % NEL));
}

static void
test_insque_edges(void)
{
	int nx[NEL], pv[NEL];

	for (int i = 0; i < NEL; i++) {
		for (int j = 0; j < NEL; j++) {
			nx[j] = -1;
			pv[j] = -1;
		}
		case_insque(nx, pv, i, -1);
	}

	for (int p = 0; p < NEL; p++) {
		for (int e = 0; e < NEL; e++) {
			for (int i = 0; i < NEL; i++) {
				nx[i] = -1;
				pv[i] = -1;
			}
			case_insque(nx, pv, e, p);
		}
	}

	for (int p = 0; p < NEL; p++) {
		for (int n = 0; n < NEL; n++) {
			if (n == p)
				continue;
			for (int e = 0; e < NEL; e++) {
				for (int i = 0; i < NEL; i++) {
					nx[i] = -1;
					pv[i] = -1;
				}
				nx[p] = n;
				pv[n] = p;
				case_insque(nx, pv, e, p);
			}
		}
	}

	for (int i = 0; i < NEL; i++) {
		nx[i] = (i == NEL - 1) ? -1 : i + 1;
		pv[i] = (i == 0) ? -1 : i - 1;
	}
	for (int p = 0; p < NEL; p++)
		for (int e = 0; e < NEL; e++)
			case_insque(nx, pv, e, p);

	for (int i = 0; i < NEL; i++) {
		nx[i] = (i + 1) % NEL;
		pv[i] = (i + NEL - 1) % NEL;
	}
	for (int p = 0; p < NEL; p++)
		for (int e = 0; e < NEL; e++)
			case_insque(nx, pv, e, p);

	for (int i = 0; i < NEL; i++) {
		nx[i] = -1;
		pv[i] = -1;
	}
	nx[1] = 2;
	pv[2] = 1;
	case_insque(nx, pv, 0, 1);
	case_insque(nx, pv, 3, 1);
}

static void
test_insque_random(unsigned iters)
{
	int nx[NEL], pv[NEL];

	for (unsigned t = 0; t < iters; t++) {
		for (int i = 0; i < NEL; i++) {
			nx[i] = ins_pick_link();
			pv[i] = ins_pick_link();
		}
		int elem = (int)rng_below(NEL);
		int pred = ((rng_next() & 15u) == 0u) ? -1 : (int)rng_below(NEL);
		case_insque(nx, pv, elem, pred);
	}
}

enum { MAXN = 32, TPAD = 16 };

struct TreeArena {
	unsigned char pre[TPAD];
	P::posix_tnode nodes[MAXN];
	int keys[MAXN];
	unsigned char post[TPAD];
};

static void
tree_clear(TreeArena &a)
{
	std::memset(&a, GUARD, sizeof(a));
	for (int i = 0; i < MAXN; i++) {
		a.nodes[i].key = &a.keys[i];
		a.nodes[i].llink = nullptr;
		a.nodes[i].rlink = nullptr;
		a.nodes[i].balance = 0;
		a.keys[i] = 0;
	}
}

static long
tree_enc(const TreeArena &a, const P::posix_tnode *p)
{
	if (p == nullptr)
		return (-1);
	const unsigned char *base =
	    reinterpret_cast<const unsigned char *>(&a);
	const unsigned char *q = reinterpret_cast<const unsigned char *>(p);
	if (q < base || q >= base + sizeof(TreeArena))
		return (-2);
	return (q - base);
}

static int
tree_slot(const TreeArena &a, const P::posix_tnode *p)
{
	long off = tree_enc(a, p);
	if (off < 0)
		return (-1);
	return (int)((off - (long)TPAD) / (long)sizeof(P::posix_tnode));
}

static P::posix_tnode *
tree_build_bst(TreeArena &a, const int *vals, int n, cmp_fn cmp)
{
	int used = 0;

	tree_clear(a);
	for (int i = 0; i < n; i++) {
		int v = vals[i];
		int parent;
		int side;

		if (used == 0) {
			a.keys[0] = v;
			used = 1;
			continue;
		}
		parent = 0;
		for (;;) {
			int r = cmp(&v, a.nodes[parent].key);

			if (r == 0)
				break;
			if (r < 0) {
				if (a.nodes[parent].llink == nullptr) {
					side = -1;
					break;
				}
				parent = tree_slot(a, a.nodes[parent].llink);
			} else {
				if (a.nodes[parent].rlink == nullptr) {
					side = 1;
					break;
				}
				parent = tree_slot(a, a.nodes[parent].rlink);
			}
		}
		if (cmp(&v, a.nodes[parent].key) == 0)
			continue;
		a.keys[used] = v;
		if (side < 0)
			a.nodes[parent].llink = &a.nodes[used];
		else
			a.nodes[parent].rlink = &a.nodes[used];
		used++;
	}
	return (used > 0 ? &a.nodes[0] : nullptr);
}

static void
case_tfind(TreeArena &pa, TreeArena &ra, const int *vals, int n, int seek,
    cmp_fn cmp, bool null_rootp)
{
	P::posix_tnode *proot;
	P::posix_tnode *rroot;
	P::posix_tnode *got;
	P::posix_tnode *want;
	int key = seek;

	proot = tree_build_bst(pa, vals, n, cmp);
	rroot = tree_build_bst(ra, vals, n, cmp);

	if (null_rootp) {
		got = P::tfind(&key, nullptr, cmp);
		want = ref_tfind(&key, nullptr, cmp);
	} else if (proot == nullptr) {
		got = P::tfind(&key, &proot, cmp);
		want = ref_tfind(&key, &rroot, cmp);
	} else {
		got = P::tfind(&key, &proot, cmp);
		want = ref_tfind(&key, &rroot, cmp);
	}

	bool ok = tree_enc(pa, got) == tree_enc(ra, want);
	check(st_tfind, ok,
	    "n=%d seek=%d null_rootp=%d off port=%ld ref=%ld",
	    n, seek, null_rootp ? 1 : 0, tree_enc(pa, got),
	    tree_enc(ra, want));
}

static void
test_tfind_edges(void)
{
	static const int empty[] = { };
	static const int one[] = { 42 };
	static const int three[] = { 50, 25, 75 };
	static const int left_chain[] = { 64, 32, 16, 8, 4, 2, 1 };
	static const int right_chain[] = { 1, 2, 4, 8, 16, 32, 64 };
	static const int zigzag[] = { 32, 16, 48, 8, 24, 40, 56, 0x80, 0xff };
	static const int dups[] = { 5, 5, 3, 3, 7, 7 };
	static const int highbit[] = { 0x7f, 0x80, 0xfe, 0xff, 1, 0 };
	TreeArena pa, ra;
	cmp_fn cmps[] = { icmp, icmp_scaled, icmp_highbit };
	int seeks[] = {
		-100, -1, 0, 1, 2, 3, 4, 7, 8, 15, 16, 24, 25, 31, 32, 42, 48,
		50, 63, 64, 75, 100, 0x7f, 0x80, 0xfe, 0xff, 127, 128, 255,
	};

	for (unsigned c = 0; c < sizeof(cmps) / sizeof(cmps[0]); c++) {
		cmp_fn cmp = cmps[c];

		case_tfind(pa, ra, empty, 0, 0, cmp, false);
		case_tfind(pa, ra, empty, 0, 0, cmp, true);
		case_tfind(pa, ra, one, 1, 42, cmp, false);
		case_tfind(pa, ra, one, 1, 41, cmp, false);
		case_tfind(pa, ra, one, 1, 43, cmp, false);
		case_tfind(pa, ra, three, 3, 25, cmp, false);
		case_tfind(pa, ra, three, 3, 24, cmp, false);
		case_tfind(pa, ra, three, 3, 26, cmp, false);
		case_tfind(pa, ra, three, 3, 75, cmp, false);
		case_tfind(pa, ra, three, 3, 50, cmp, false);
		case_tfind(pa, ra, left_chain,
		    (int)(sizeof(left_chain) / sizeof(left_chain[0])), 8, cmp,
		    false);
		case_tfind(pa, ra, right_chain,
		    (int)(sizeof(right_chain) / sizeof(right_chain[0])), 32,
		    cmp, false);
		case_tfind(pa, ra, zigzag,
		    (int)(sizeof(zigzag) / sizeof(zigzag[0])), 0x80, cmp,
		    false);
		case_tfind(pa, ra, dups,
		    (int)(sizeof(dups) / sizeof(dups[0])), 5, cmp, false);
		case_tfind(pa, ra, highbit,
		    (int)(sizeof(highbit) / sizeof(highbit[0])), 0x80, cmp,
		    false);

		for (unsigned i = 0; i < sizeof(seeks) / sizeof(seeks[0]); i++)
			case_tfind(pa, ra, zigzag,
			    (int)(sizeof(zigzag) / sizeof(zigzag[0])),
			    seeks[i], cmp, false);
	}
}

static void
test_tfind_random(unsigned iters)
{
	TreeArena pa, ra;
	int vals[MAXN];

	for (unsigned t = 0; t < iters; t++) {
		cmp_fn cmp = pick_cmp((unsigned)rng_next());
		int n = (int)(rng_below(MAXN - 1u) + 1u);
		bool null_rootp = ((rng_next() & 127u) == 0u);

		for (int i = 0; i < n; i++) {
			unsigned r = (unsigned)rng_next();
			if ((r & 7u) == 0u)
				vals[i] = (int)(0x80 + rng_below(0x80u));
			else
				vals[i] = (int)((int)r % 512) - 256;
		}
		int seek;
		switch (rng_below(5u)) {
		case 0:
			seek = vals[rng_below((unsigned)n)];
			break;
		case 1:
			seek = (int)((int)rng_next() % 1024) - 512;
			break;
		case 2:
			seek = (int)(0x80 + rng_below(0x80u));
			break;
		case 3:
			seek = vals[0];
			break;
		default:
			seek = vals[n - 1];
			break;
		}
		case_tfind(pa, ra, vals, n, seek, cmp, null_rootp);
	}
}

enum { MAXW = 4096 };

struct WalkTrace {
	long node_off[MAXW];
	int visit[MAXW];
	int level[MAXW];
	unsigned count;
};

static WalkTrace g_port_trace;
static WalkTrace g_ref_trace;
static const TreeArena *g_trace_arena;

static void
port_walk_cb(const P::posix_tnode *node, P::VISIT visit, int level)
{
	if (g_port_trace.count >= MAXW)
		return;
	g_port_trace.node_off[g_port_trace.count] =
	    tree_enc(*g_trace_arena, node);
	g_port_trace.visit[g_port_trace.count] = (int)visit;
	g_port_trace.level[g_port_trace.count] = level;
	g_port_trace.count++;
}

static void
ref_walk_cb(const P::posix_tnode *node, int visit, int level)
{
	if (g_ref_trace.count >= MAXW)
		return;
	g_ref_trace.node_off[g_ref_trace.count] =
	    tree_enc(*g_trace_arena, node);
	g_ref_trace.visit[g_ref_trace.count] = visit;
	g_ref_trace.level[g_ref_trace.count] = level;
	g_ref_trace.count++;
}

static bool
walk_trace_equal(const WalkTrace &a, const WalkTrace &b)
{
	if (a.count != b.count)
		return (false);
	for (unsigned i = 0; i < a.count; i++) {
		if (a.node_off[i] != b.node_off[i] ||
		    a.visit[i] != b.visit[i] || a.level[i] != b.level[i])
			return (false);
	}
	return (true);
}

static void
case_twalk(TreeArena &pa, TreeArena &ra, const int *vals, int n, cmp_fn cmp,
    bool null_root, bool null_action)
{
	P::posix_tnode *proot;
	P::posix_tnode *rroot;

	proot = tree_build_bst(pa, vals, n, cmp);
	rroot = tree_build_bst(ra, vals, n, cmp);

	g_port_trace.count = 0;
	g_ref_trace.count = 0;
	g_trace_arena = &pa;

	if (null_root) {
		P::twalk(nullptr, null_action ? nullptr : port_walk_cb);
		ref_twalk(nullptr, null_action ? nullptr : ref_walk_cb);
	} else if (null_action) {
		P::twalk(proot, nullptr);
		ref_twalk(rroot, nullptr);
	} else {
		P::twalk(proot, port_walk_cb);
		g_trace_arena = &ra;
		ref_twalk(rroot, ref_walk_cb);
	}

	bool ok = walk_trace_equal(g_port_trace, g_ref_trace);
	check(st_twalk, ok, "n=%d null_root=%d null_action=%d steps=%u",
	    n, null_root ? 1 : 0, null_action ? 1 : 0, g_port_trace.count);
}

static void
test_twalk_edges(void)
{
	static const int one[] = { 10 };
	static const int two_left[] = { 20, 10 };
	static const int two_right[] = { 10, 20 };
	static const int full3[] = { 20, 10, 30 };
	static const int deep[] = { 8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7 };
	static const int skew_l[] = { 7, 6, 5, 4, 3, 2, 1 };
	static const int skew_r[] = { 1, 2, 3, 4, 5, 6, 7 };
	static const int highbit[] = { 0x80, 0x40, 0xc0, 0x20, 0xff };
	TreeArena pa, ra;
	cmp_fn cmps[] = { icmp, icmp_scaled, icmp_highbit };
	const int *sets[] = {
		one, two_left, two_right, full3, deep, skew_l, skew_r, highbit,
	};
	const int setlens[] = {
		1, 2, 2, 3,
		(int)(sizeof(deep) / sizeof(deep[0])),
		(int)(sizeof(skew_l) / sizeof(skew_l[0])),
		(int)(sizeof(skew_r) / sizeof(skew_r[0])),
		(int)(sizeof(highbit) / sizeof(highbit[0])),
	};
	static const int empty[] = { };

	for (unsigned c = 0; c < sizeof(cmps) / sizeof(cmps[0]); c++) {
		cmp_fn cmp = cmps[c];
		case_twalk(pa, ra, empty, 0, cmp, false, false);
		case_twalk(pa, ra, empty, 0, cmp, true, false);
		case_twalk(pa, ra, empty, 0, cmp, false, true);
		case_twalk(pa, ra, empty, 0, cmp, true, true);
		for (unsigned s = 0; s < sizeof(sets) / sizeof(sets[0]); s++) {
			case_twalk(pa, ra, sets[s], setlens[s], cmp, false,
			    false);
			case_twalk(pa, ra, sets[s], setlens[s], cmp, true,
			    false);
			case_twalk(pa, ra, sets[s], setlens[s], cmp, false,
			    true);
			case_twalk(pa, ra, sets[s], setlens[s], cmp, true,
			    true);
		}
	}
}

static void
test_twalk_random(unsigned iters)
{
	TreeArena pa, ra;
	int vals[MAXN];

	for (unsigned t = 0; t < iters; t++) {
		cmp_fn cmp = pick_cmp((unsigned)rng_next());
		int n = (int)rng_below(MAXN + 1u);
		bool null_root = ((rng_next() & 63u) == 0u);
		bool null_action = ((rng_next() & 63u) == 0u);

		for (int i = 0; i < n; i++) {
			unsigned r = (unsigned)rng_next();
			if ((r & 7u) == 0u)
				vals[i] = (int)(0x80 + rng_below(0x80u));
			else
				vals[i] = (int)((int)r % 1024) - 512;
		}
		case_twalk(pa, ra, vals, n, cmp, null_root, null_action);
	}
}

static void
report(const Stat &s)
{
	std::printf("  %-10s %12lu %12lu  %s\n", s.name, s.cases, s.fails,
	    s.fails == 0 ? "ok" : "FAILED");
}

} /* anonymous namespace */

int
main(void)
{
	rng_state = 0x9e3779b97f4a7c15ull;

	test_insque_edges();
	test_insque_random(SWEEP_ITERS);

	test_tfind_edges();
	test_tfind_random(SWEEP_ITERS);

	test_twalk_edges();
	test_twalk_random(SWEEP_ITERS);

	unsigned long cases = st_insque.cases + st_tfind.cases + st_twalk.cases;
	unsigned long fails = st_insque.fails + st_tfind.fails + st_twalk.fails;

	std::printf("\nbatch b0298: port vs. oracle\n");
	std::printf("  %-10s %12s %12s\n", "function", "cases", "failures");
	std::printf("  ---------- ------------ ------------\n");
	report(st_insque);
	report(st_tfind);
	report(st_twalk);
	std::printf("  ---------- ------------ ------------\n");
	std::printf("  %-10s %12lu %12lu  %s\n", "TOTAL", cases, fails,
	    fails == 0 ? "PASS" : "FAIL");

	return (fails == 0 ? 0 : 1);
}
