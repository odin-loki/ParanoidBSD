/*
 * Differential harness for batch b0155s3: btree sequential scan (bt_seq.c).
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.db.btree.b0155s3;

namespace P = pbsd::lib_libc_db_btree::b0155s3;

extern "C" {
typedef uint32_t pgno_t;
typedef uint16_t indx_t;
typedef uint32_t recno_t;
typedef unsigned int u_int;
typedef unsigned char u_char;
typedef uint32_t u_int32_t;
typedef char *caddr_t;

typedef enum { DB_BTREE, DB_HASH, DB_RECNO } DBTYPE;
typedef struct { void *data; size_t size; } DBT;

typedef struct __db {
	DBTYPE type;
	int (*close)(struct __db *);
	int (*del)(const struct __db *, const DBT *, unsigned int);
	int (*get)(const struct __db *, const DBT *, DBT *, unsigned int);
	int (*put)(struct __db *, DBT *, const DBT *, unsigned int);
	int (*seq)(const struct __db *, DBT *, DBT *, unsigned int);
	int (*sync)(const struct __db *, unsigned int);
	void *internal;
	int (*fd)(const struct __db *);
} DB;

typedef struct MPOOL { char dummy; } MPOOL;

typedef struct _page {
	pgno_t pgno, prevpg, nextpg;
	u_int32_t flags;
	indx_t lower, upper;
	indx_t linp[1];
} PAGE;

typedef struct _epgno { pgno_t pgno; indx_t index; } EPGNO;
typedef struct _epg { PAGE *page; indx_t index; } EPG;
typedef struct _cursor { EPGNO pg; DBT key; recno_t rcursor; u_int8_t flags; } CURSOR;

typedef struct _btree {
	MPOOL *bt_mp; DB *bt_dbp; EPG bt_cur; PAGE *bt_pinned;
	CURSOR bt_cursor; EPGNO bt_stack[50]; EPGNO *bt_sp;
	DBT bt_rkey, bt_rdata; int bt_fd; pgno_t bt_free;
	u_int32_t bt_psize; indx_t bt_ovflsize; int bt_lorder, bt_order;
	EPGNO bt_last;
	int (*bt_cmp)(const DBT *, const DBT *);
	size_t (*bt_pfx)(const DBT *, const DBT *);
	int (*bt_irec)(struct _btree *, recno_t);
	FILE *bt_rfp; int bt_rfd;
	caddr_t bt_cmap, bt_smap, bt_emap;
	size_t bt_msize; recno_t bt_nrecs; size_t bt_reclen;
	u_char bt_bval; u_int32_t flags;
} BTREE;

typedef struct {
	unsigned get_calls, put_calls;
	unsigned search_calls, cmp_calls, ret_calls;
	int get_force_null, search_force_null;
	int search_exact, cmp_ret, ret_status;
	pgno_t get_last_pgno;
	unsigned get_last_flags, last_put_flags;
	void *last_put_page;
	int nreg;
	pgno_t reg_pgno[64];
	void *reg_page[64];
	EPG search_epg;
} test_mock_state;

extern test_mock_state test_mock;
void test_mock_reset(void);
void test_mock_register(pgno_t pgno, void *page);

void ref___bt_setcur(BTREE *, pgno_t, u_int);
int ref___bt_seqset(BTREE *, EPG *, DBT *, int);
int ref___bt_seqadv(BTREE *, EPG *, int);
int ref___bt_first(BTREE *, const DBT *, EPG *, int *);
int ref___bt_seq(const DB *, DBT *, DBT *, u_int);

int harness_cmp(const DBT *a, const DBT *b)
{
	size_t min = a->size < b->size ? a->size : b->size;
	if (min > 0) {
		int c = std::memcmp(a->data, b->data, min);
		if (c != 0) return c;
	}
	if (a->size < b->size) return -1;
	if (a->size > b->size) return 1;
	return 0;
}
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr size_t PAGE_SZ = 512;
constexpr unsigned SWEEP_ITERS = 200000;

enum { NOT = 0, BACK = 1, FORWARD = 2 };
#define P_INVALID 0
#define P_ROOT 1
#define P_BINTERNAL 0x01
#define P_BLEAF 0x02
#define B_NODUPS 0x00020
#define B_DB_LOCK 0x04000
#define CURS_ACQUIRE 0x01
#define CURS_AFTER 0x02
#define CURS_BEFORE 0x04
#define CURS_INIT 0x08
#define R_CURSOR 1
#define R_FIRST 3
#define R_LAST 6
#define R_NEXT 7
#define R_PREV 9
#define RET_ERROR -1
#define RET_SUCCESS 0
#define RET_SPECIAL 1
#define BTDATAOFF (sizeof(pgno_t)+sizeof(pgno_t)+sizeof(pgno_t)+sizeof(u_int32_t)+sizeof(indx_t)+sizeof(indx_t))
#define NEXTINDEX(p) (((p)->lower - BTDATAOFF) / sizeof(indx_t))
#define LALIGN(n) (((n) + sizeof(pgno_t) - 1) & ~(sizeof(pgno_t) - 1))
#define NBLEAFDBT(ksize, dsize) LALIGN(sizeof(u_int32_t)+sizeof(u_int32_t)+sizeof(u_char)+(ksize)+(dsize))

enum Fn { F_BT_SETCUR, F_BT_SEQSET, F_BT_SEQADV, F_BT_FIRST, F_BT_SEQ, F_COUNT };
const char *fn_name[F_COUNT] = {"__bt_setcur","__bt_seqset","__bt_seqadv","__bt_first","__bt_seq"};
unsigned long n_cases[F_COUNT], n_fails[F_COUNT];
unsigned reported[F_COUNT];
uint64_t rng = 1263225679477498465ULL;

uint64_t nextr(void) { rng ^= rng << 13; rng ^= rng >> 7; rng ^= rng << 17; return rng; }
void fail(int fn, const char *msg) {
	n_fails[fn]++;
	if (reported[fn] < 10) { reported[fn]++; std::fprintf(stderr, "FAIL %s: %s\n", fn_name[fn], msg); }
}
void check_eq(int fn, bool ok, const char *msg) { n_cases[fn]++; if (!ok) fail(fn, msg); }
void guard_fill(void *p, size_t n) { std::memset(p, GUARD, n); }

struct MockSnap { test_mock_state mock; };
MockSnap snap_mock(void) { MockSnap s; s.mock = test_mock; return s; }
void restore_mock(const MockSnap &s) { test_mock = s.mock; }
struct MockDelta { unsigned get, put, search, cmp, ret; };
MockDelta mock_delta(const test_mock_state &b, const test_mock_state &a) {
	MockDelta d{};
	d.get = a.get_calls - b.get_calls;
	d.put = a.put_calls - b.put_calls;
	d.search = a.search_calls - b.search_calls;
	d.cmp = a.cmp_calls - b.cmp_calls;
	d.ret = a.ret_calls - b.ret_calls;
	return d;
}
bool mock_delta_eq(const MockDelta &a, const MockDelta &b) {
	return a.get==b.get && a.put==b.put && a.search==b.search && a.cmp==b.cmp && a.ret==b.ret;
}

static int port_cmp(const P::DBT *a, const P::DBT *b) { return harness_cmp((const DBT *)a, (const DBT *)b); }

indx_t calc_ovflsize(u_int32_t psize, int minkeypage) {
	indx_t ov = (indx_t)((psize - BTDATAOFF) / (unsigned)minkeypage - (sizeof(indx_t) + NBLEAFDBT(0, 0)));
	if (ov < (indx_t)(NBLEAFDBT(8, 8) + sizeof(indx_t))) ov = (indx_t)(NBLEAFDBT(8, 8) + sizeof(indx_t));
	return ov;
}

void init_tree_port(P::BTREE &t, P::MPOOL &mp, P::DB &db, u_int32_t flags) {
	std::memset(&t, 0, sizeof(t));
	t.bt_mp = &mp; t.bt_dbp = &db; t.bt_psize = (u_int32_t)PAGE_SZ;
	t.bt_ovflsize = calc_ovflsize((u_int32_t)PAGE_SZ, 2);
	t.bt_sp = t.bt_stack; t.bt_order = NOT; t.bt_fd = -1; t.bt_cmp = port_cmp;
	t.flags = flags; db.internal = &t;
}
void init_tree_ref(BTREE &t, MPOOL &mp, DB &db, u_int32_t flags) {
	std::memset(&t, 0, sizeof(t));
	t.bt_mp = &mp; t.bt_dbp = &db; t.bt_psize = (u_int32_t)PAGE_SZ;
	t.bt_ovflsize = calc_ovflsize((u_int32_t)PAGE_SZ, 2);
	t.bt_sp = t.bt_stack; t.bt_order = NOT; t.bt_fd = -1; t.bt_cmp = harness_cmp;
	t.flags = flags; db.internal = &t;
}
bool harness_cmp_btree(const P::BTREE &tp, const BTREE &tr) {
	return tp.flags == tr.flags && tp.bt_order == tr.bt_order &&
	    tp.bt_cursor.pg.pgno == tr.bt_cursor.pg.pgno &&
	    tp.bt_cursor.pg.index == tr.bt_cursor.pg.index &&
	    tp.bt_cursor.flags == tr.bt_cursor.flags &&
	    (tp.bt_pinned == nullptr) == (tr.bt_pinned == nullptr) &&
	    (tp.bt_cursor.key.data == nullptr) == (tr.bt_cursor.key.data == nullptr);
}

void build_bleaf_page(PAGE *pg, size_t psize, int nents, const u_int32_t *ksizes, const u_int32_t *dsizes, const u_char *eflags) {
	size_t off = psize; std::memset(pg, 0, psize); pg->flags = P_BLEAF;
	while (nents > 0) {
		size_t trial = psize;
		for (int i = nents - 1; i >= 0; i--) {
			trial -= NBLEAFDBT(ksizes[i], dsizes[i]);
			if (trial < BTDATAOFF + (size_t)nents * sizeof(indx_t)) goto shrink;
		}
		break;
shrink: nents--;
	}
	off = psize;
	for (int i = nents - 1; i >= 0; i--) {
		size_t ksz = ksizes[i], dsz = dsizes[i], esz = NBLEAFDBT(ksz, dsz);
		off -= esz;
		unsigned char *e = (unsigned char *)pg + off;
		*(u_int32_t *)e = (u_int32_t)ksz; *(u_int32_t *)(e + 4) = (u_int32_t)dsz;
		e[8] = eflags[i];
		for (size_t k = 0; k < ksz + dsz; k++) e[9 + k] = (u_char)(0x80 + k + (unsigned)i);
		*(indx_t *)((unsigned char *)pg + BTDATAOFF + (size_t)i * sizeof(indx_t)) = (indx_t)off;
	}
	pg->lower = (indx_t)(BTDATAOFF + nents * sizeof(indx_t)); pg->upper = (indx_t)off;
}
void build_binternal_page(PAGE *pg, size_t psize, int nents, const u_int32_t *ksizes, const u_char *eflags, const pgno_t *pgnos) {
	size_t off = psize; pg->flags = P_BINTERNAL;
	for (int i = nents - 1; i >= 0; i--) {
		size_t ksz = ksizes[i], esz = LALIGN(sizeof(u_int32_t)+sizeof(pgno_t)+sizeof(u_char)+ksz);
		off -= esz;
		unsigned char *e = (unsigned char *)pg + off;
		*(u_int32_t *)e = (u_int32_t)ksz; *(pgno_t *)(e + 4) = pgnos[i]; e[8] = eflags[i];
		for (size_t k = 0; k < ksz; k++) e[9 + k] = (u_char)(0xa0 + k + (unsigned)i);
		*(indx_t *)((unsigned char *)pg + BTDATAOFF + (size_t)i * sizeof(indx_t)) = (indx_t)off;
	}
	pg->lower = (indx_t)(BTDATAOFF + nents * sizeof(indx_t)); pg->upper = (indx_t)off;
}
void copy_cursor_ref(BTREE &tr, const P::BTREE &tp) {
	tr.bt_cursor.pg.pgno = tp.bt_cursor.pg.pgno;
	tr.bt_cursor.pg.index = tp.bt_cursor.pg.index;
	tr.bt_cursor.flags = tp.bt_cursor.flags;
	tr.bt_cursor.rcursor = tp.bt_cursor.rcursor;
	tr.bt_cursor.key.data = tp.bt_cursor.key.data;
	tr.bt_cursor.key.size = tp.bt_cursor.key.size;
}


void check_bt_setcur(int had_key, pgno_t pgno, u_int idx)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	char *kp = nullptr;
	char *kr = nullptr;

	test_mock_reset();
	init_tree_port(tp, mp_p, db_p, 0);
	init_tree_ref(tr, mp_r, db_r, 0);
	if (had_key) {
		kp = (char *)std::malloc(16);
		kr = (char *)std::malloc(16);
		tp.bt_cursor.key.data = kp;
		tp.bt_cursor.key.size = 16;
		tr.bt_cursor.key.data = kr;
		tr.bt_cursor.key.size = 16;
		tp.bt_cursor.flags = CURS_ACQUIRE | CURS_AFTER;
		tr.bt_cursor.flags = CURS_ACQUIRE | CURS_AFTER;
	}

	P::__bt_setcur(&tp, pgno, idx);
	ref___bt_setcur(&tr, pgno, idx);
	char msg[128];
	std::snprintf(msg, sizeof(msg), "pgno port=%u ref=%u idx port=%u ref=%u",
	    (unsigned)tp.bt_cursor.pg.pgno, (unsigned)tr.bt_cursor.pg.pgno,
	    (unsigned)tp.bt_cursor.pg.index, (unsigned)tr.bt_cursor.pg.index);
	check_eq(F_BT_SETCUR, harness_cmp_btree(tp, tr), msg);
	check_eq(F_BT_SETCUR,
	    (tp.bt_cursor.flags & CURS_INIT) == (tr.bt_cursor.flags & CURS_INIT),
	    "CURS_INIT");
	if (had_key)
		check_eq(F_BT_SETCUR, tp.bt_cursor.key.data == nullptr, "key freed");
}
void setup_tree_for_seq(unsigned char *root_p, unsigned char *leaf_p,
    int empty, int internal)
{
	guard_fill(root_p, PAGE_SZ);
	guard_fill(leaf_p, PAGE_SZ);
	if (empty) {
		((PAGE *)root_p)->pgno = P_ROOT;
		((PAGE *)root_p)->flags = P_BLEAF;
		((PAGE *)root_p)->lower = BTDATAOFF;
		((PAGE *)root_p)->upper = (indx_t)(PAGE_SZ / 2);
		return;
	}
	u_int32_t ks[1] = { 2 };
	u_char ef[1] = { 0 };
	pgno_t pg[1] = { 10 };
	u_int32_t ksizes[3] = { 4, 5, 6 };
	u_int32_t dsizes[3] = { 2, 3, 4 };
	u_char eflags[3] = { 0, 0, 0 };
	if (internal) {
		build_binternal_page((PAGE *)root_p, PAGE_SZ, 1, ks, ef, pg);
		((PAGE *)root_p)->pgno = P_ROOT;
		build_bleaf_page((PAGE *)leaf_p, PAGE_SZ, 3, ksizes, dsizes, eflags);
		((PAGE *)leaf_p)->pgno = 10;
		((PAGE *)leaf_p)->prevpg = P_INVALID;
		((PAGE *)leaf_p)->nextpg = P_INVALID;
	} else {
		build_bleaf_page((PAGE *)root_p, PAGE_SZ, 3, ksizes, dsizes, eflags);
		((PAGE *)root_p)->pgno = P_ROOT;
		((PAGE *)root_p)->prevpg = P_INVALID;
		((PAGE *)root_p)->nextpg = P_INVALID;
	}
}
void check_bt_seqset(int flags, int empty, int internal, int get_null,
    int first_null_key)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	static unsigned char root_p[PAGE_SZ];
	static unsigned char root_r[PAGE_SZ];
	static unsigned char leaf_p[PAGE_SZ];
	static unsigned char leaf_r[PAGE_SZ];
	P::EPG ep_p;
	EPG ep_r;
	DBT key;
	static unsigned char keybuf[8];

	test_mock_reset();
	test_mock.get_force_null = get_null;
	init_tree_port(tp, mp_p, db_p, 0);
	init_tree_ref(tr, mp_r, db_r, 0);
	setup_tree_for_seq(root_p, leaf_p, empty, internal);
	std::memcpy(root_r, root_p, PAGE_SZ);
	std::memcpy(leaf_r, leaf_p, PAGE_SZ);
	static unsigned char init_root[PAGE_SZ];
	static unsigned char init_leaf[PAGE_SZ];
	std::memcpy(init_root, root_p, PAGE_SZ);
	std::memcpy(init_leaf, leaf_p, PAGE_SZ);
	test_mock_register(P_ROOT, root_p);
	if (internal)
		test_mock_register(10, leaf_p);
	test_mock.search_epg.page = internal ? (PAGE *)leaf_p : (PAGE *)root_p;
	test_mock.search_epg.index = 0;
	test_mock.search_exact = 0;
	key.data = first_null_key ? nullptr : keybuf;
	key.size = first_null_key ? 0 : 4;
	keybuf[0] = 0x80;

	MockSnap snap = snap_mock();
	errno = 0;
	int rp = P::__bt_seqset(&tp, &ep_p, (P::DBT *)&key, flags);
	int ep = errno;
	MockDelta dp = mock_delta(snap.mock, test_mock);
	std::memcpy(root_p, init_root, PAGE_SZ);
	if (internal)
		std::memcpy(leaf_p, init_leaf, PAGE_SZ);
	restore_mock(snap);
	test_mock.search_epg.page = internal ? (PAGE *)leaf_p : (PAGE *)root_p;
	test_mock.search_epg.index = 0;
	errno = 0;
	int rr = ref___bt_seqset(&tr, &ep_r, &key, flags);
	int er = errno;
	MockDelta dr = mock_delta(snap.mock, test_mock);
	char msg[160];
	std::snprintf(msg, sizeof(msg),
	    "ret port=%d ref=%d fl=%d empty=%d int=%d null=%d errno_p=%d errno_r=%d",
	    rp, rr, flags, empty, internal, get_null, ep, er);
	check_eq(F_BT_SEQSET, rp == rr && ep == er, msg);
	check_eq(F_BT_SEQSET, mock_delta_eq(dp, dr), "mock delta");
	if (rp == rr && rp == RET_SUCCESS) {
		check_eq(F_BT_SEQSET, ep_p.index == ep_r.index, "index");
		check_eq(F_BT_SEQSET, ep_p.page->pgno == ep_r.page->pgno, "pgno");
	}
}
void check_bt_seqadv(int advflags, int curs_acquire, int curs_after,
    int curs_before, indx_t cidx, pgno_t cpg, pgno_t nextpg)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	static unsigned char leaf_p[PAGE_SZ];
	static unsigned char leaf_r[PAGE_SZ];
	static unsigned char next_p[PAGE_SZ];
	static unsigned char next_r[PAGE_SZ];
	P::EPG ep_p;
	EPG ep_r;
	u_int32_t ksizes[3] = { 4, 5, 6 };
	u_int32_t dsizes[3] = { 2, 3, 4 };
	u_char eflags[3] = { 0, 0, 0 };

	test_mock_reset();
	init_tree_port(tp, mp_p, db_p, 0);
	init_tree_ref(tr, mp_r, db_r, 0);
	guard_fill(leaf_p, PAGE_SZ);
	guard_fill(leaf_r, PAGE_SZ);
	build_bleaf_page((PAGE *)leaf_p, PAGE_SZ, 3, ksizes, dsizes, eflags);
	std::memcpy(leaf_r, leaf_p, PAGE_SZ);
	((PAGE *)leaf_p)->pgno = cpg;
	((PAGE *)leaf_r)->pgno = cpg;
	((PAGE *)leaf_p)->prevpg = P_INVALID;
	((PAGE *)leaf_r)->prevpg = P_INVALID;
	((PAGE *)leaf_p)->nextpg = nextpg;
	((PAGE *)leaf_r)->nextpg = nextpg;
	static unsigned char init_leaf[PAGE_SZ];
	static unsigned char init_next[PAGE_SZ];
	std::memcpy(init_leaf, leaf_p, PAGE_SZ);
	test_mock_register(cpg, leaf_p);
	if (nextpg != P_INVALID) {
		guard_fill(next_p, PAGE_SZ);
		guard_fill(next_r, PAGE_SZ);
		build_bleaf_page((PAGE *)next_p, PAGE_SZ, 2, ksizes, dsizes, eflags);
		std::memcpy(next_r, next_p, PAGE_SZ);
		((PAGE *)next_p)->pgno = nextpg;
		((PAGE *)next_r)->pgno = nextpg;
		std::memcpy(init_next, next_p, PAGE_SZ);
		test_mock_register(nextpg, next_p);
	}
	test_mock.search_epg.page = (PAGE *)leaf_p;
	test_mock.search_epg.index = 0;
	static unsigned char ckey[8] = { 0x80, 0x81, 0x82, 0x83 };
	tp.bt_cursor.flags = CURS_INIT | (curs_acquire ? CURS_ACQUIRE : 0) |
	    (curs_after ? CURS_AFTER : 0) | (curs_before ? CURS_BEFORE : 0);
	tp.bt_cursor.pg.pgno = cpg;
	tp.bt_cursor.pg.index = cidx;
	if (curs_acquire) {
		tp.bt_cursor.key.data = ckey;
		tp.bt_cursor.key.size = 4;
	}
	copy_cursor_ref(tr, tp);

	MockSnap snap = snap_mock();
	int rp = P::__bt_seqadv(&tp, &ep_p, advflags);
	MockDelta dp = mock_delta(snap.mock, test_mock);
	std::memcpy(leaf_p, init_leaf, PAGE_SZ);
	if (nextpg != P_INVALID)
		std::memcpy(next_p, init_next, PAGE_SZ);
	restore_mock(snap);
	tp.bt_cursor.flags = CURS_INIT | (curs_acquire ? CURS_ACQUIRE : 0) |
	    (curs_after ? CURS_AFTER : 0) | (curs_before ? CURS_BEFORE : 0);
	tp.bt_cursor.pg.pgno = cpg;
	tp.bt_cursor.pg.index = cidx;
	copy_cursor_ref(tr, tp);
	if (curs_acquire) {
		tr.bt_cursor.key.data = ckey;
		tr.bt_cursor.key.size = 4;
	}
	test_mock.search_epg.page = (PAGE *)leaf_p;
	test_mock.search_epg.index = 0;
	test_mock_register(cpg, leaf_p);
	if (nextpg != P_INVALID)
		test_mock_register(nextpg, next_p);
	int rr = ref___bt_seqadv(&tr, &ep_r, advflags);
	MockDelta dr = mock_delta(snap.mock, test_mock);
	char msg[160];
	std::snprintf(msg, sizeof(msg),
	    "ret port=%d ref=%d fl=%d cidx=%u next=%u", rp, rr, advflags,
	    (unsigned)cidx, (unsigned)nextpg);
	check_eq(F_BT_SEQADV, rp == rr, msg);
	check_eq(F_BT_SEQADV, mock_delta_eq(dp, dr), "mock delta");
	if (rp == rr && rp == RET_SUCCESS) {
		check_eq(F_BT_SEQADV, ep_p.index == ep_r.index, "index");
		check_eq(F_BT_SEQADV, ep_p.page->pgno == ep_r.page->pgno, "pgno");
	}
}
void check_bt_first(int exact, u_int32_t tflags, int search_null, int nodups)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	static unsigned char leaf_p[PAGE_SZ];
	static unsigned char leaf_r[PAGE_SZ];
	P::EPG ep_p;
	EPG ep_r;
	DBT key;
	static unsigned char keybuf[8];
	int exact_p = 0, exact_r = 0;

	if (exact && !nodups)
		nodups = 1;

	test_mock_reset();
	test_mock.search_force_null = search_null;
	test_mock.search_exact = exact;
	test_mock.cmp_ret = 0;
	init_tree_port(tp, mp_p, db_p, tflags | (nodups ? B_NODUPS : 0));
	init_tree_ref(tr, mp_r, db_r, tflags | (nodups ? B_NODUPS : 0));
	guard_fill(leaf_p, PAGE_SZ);
	guard_fill(leaf_r, PAGE_SZ);
	u_int32_t ksizes[3] = { 4, 5, 6 };
	u_int32_t dsizes[3] = { 2, 3, 4 };
	u_char eflags[3] = { 0, 0, 0 };
	build_bleaf_page((PAGE *)leaf_p, PAGE_SZ, 3, ksizes, dsizes, eflags);
	std::memcpy(leaf_r, leaf_p, PAGE_SZ);
	((PAGE *)leaf_p)->pgno = 8;
	((PAGE *)leaf_r)->pgno = 8;
	test_mock.search_epg.page = (PAGE *)leaf_p;
	test_mock.search_epg.index = 1;
	key.data = keybuf;
	key.size = 4;
	keybuf[0] = 0x80;

	MockSnap snap = snap_mock();
	int rp = P::__bt_first(&tp, (P::DBT *)&key, &ep_p, &exact_p);
	MockDelta dp = mock_delta(snap.mock, test_mock);
	restore_mock(snap);
	test_mock.search_epg.page = (PAGE *)leaf_r;
	int rr = ref___bt_first(&tr, &key, &ep_r, &exact_r);
	MockDelta dr = mock_delta(snap.mock, test_mock);
	char msg[160];
	std::snprintf(msg, sizeof(msg),
	    "ret port=%d ref=%d exact_p=%d exact_r=%d search=%d nodups=%d",
	    rp, rr, exact_p, exact_r, exact, nodups);
	check_eq(F_BT_FIRST, rp == rr, msg);
	check_eq(F_BT_FIRST, exact_p == exact_r, "exact flag");
	check_eq(F_BT_FIRST, mock_delta_eq(dp, dr), "mock delta");
}
void check_bt_seq(u_int flags, int empty, int curs_init, int get_null,
    int ret_fail, int db_lock)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	static unsigned char root_p[PAGE_SZ];
	static unsigned char root_r[PAGE_SZ];
	static unsigned char keybuf[32];
	static unsigned char databuf[32];
	DBT key, data;

	test_mock_reset();
	test_mock.get_force_null = get_null;
	test_mock.ret_status = ret_fail ? RET_ERROR : RET_SUCCESS;
	init_tree_port(tp, mp_p, db_p, db_lock ? B_DB_LOCK : 0);
	init_tree_ref(tr, mp_r, db_r, db_lock ? B_DB_LOCK : 0);
	setup_tree_for_seq(root_p, root_p, empty, 0);
	std::memcpy(root_r, root_p, PAGE_SZ);
	test_mock_register(P_ROOT, root_p);
	test_mock_register(P_ROOT, root_r);
	if (curs_init) {
		tp.bt_cursor.flags = CURS_INIT;
		tp.bt_cursor.pg.pgno = P_ROOT;
		tp.bt_cursor.pg.index = 0;
		tr.bt_cursor.flags = CURS_INIT;
		tr.bt_cursor.pg.pgno = P_ROOT;
		tr.bt_cursor.pg.index = 0;
	}
	key.data = keybuf;
	key.size = 4;
	data.data = databuf;
	data.size = 8;
	std::memset(keybuf, 0x11, sizeof(keybuf));
	std::memset(databuf, 0x22, sizeof(databuf));

	MockSnap snap = snap_mock();
	errno = 0;
	int rp = P::__bt_seq(&db_p, (P::DBT *)&key, (P::DBT *)&data, flags);
	int ep = errno;
	MockDelta dp = mock_delta(snap.mock, test_mock);
	restore_mock(snap);
	errno = 0;
	int rr = ref___bt_seq(&db_r, &key, &data, flags);
	int er = errno;
	MockDelta dr = mock_delta(snap.mock, test_mock);
	char msg[160];
	std::snprintf(msg, sizeof(msg),
	    "ret port=%d ref=%d fl=%u init=%d empty=%d fail=%d lock=%d errno_p=%d errno_r=%d",
	    rp, rr, flags, curs_init, empty, ret_fail, db_lock, ep, er);
	check_eq(F_BT_SEQ, rp == rr && ep == er, msg);
	check_eq(F_BT_SEQ, mock_delta_eq(dp, dr), "mock delta");
	check_eq(F_BT_SEQ, harness_cmp_btree(tp, tr), "btree");
}
void test_bt_setcur_edges(void)
{
	check_bt_setcur(0, 5, 0);
	check_bt_setcur(1, 7, 2);
	check_bt_setcur(0, P_ROOT, 1);
	check_bt_setcur(0, 0xff, 0xff);
}
void test_bt_seqset_edges(void)
{
	check_bt_seqset(R_FIRST, 0, 0, 0, 0);
	check_bt_seqset(R_LAST, 0, 0, 0, 0);
	check_bt_seqset(R_FIRST, 1, 0, 0, 0);
	check_bt_seqset(R_CURSOR, 0, 0, 0, 0);
	check_bt_seqset(R_CURSOR, 0, 0, 0, 1);
	check_bt_seqset(R_FIRST, 0, 1, 0, 0);
	check_bt_seqset(R_FIRST, 0, 0, 1, 0);
}
void test_bt_seqadv_edges(void)
{
	check_bt_seqadv(R_NEXT, 0, 0, 0, 0, 8, 9);
	check_bt_seqadv(R_NEXT, 0, 0, 0, 2, 8, P_INVALID);
	check_bt_seqadv(R_PREV, 0, 0, 0, 0, 8, P_INVALID);
	check_bt_seqadv(R_PREV, 0, 0, 1, 0, 8, P_INVALID);
	check_bt_seqadv(R_NEXT, 1, 0, 0, 0, 8, 9);
}
void test_bt_first_edges(void)
{
	check_bt_first(1, 0, 0, 1);
	check_bt_first(0, 0, 0, 0);
	check_bt_first(1, 0, 1, 0);
	check_bt_first(1, 0, 0, 0);
}
void test_bt_seq_edges(void)
{
	check_bt_seq(R_FIRST, 0, 0, 0, 0, 0);
	check_bt_seq(R_NEXT, 0, 1, 0, 0, 0);
	check_bt_seq(R_LAST, 0, 0, 0, 0, 0);
	check_bt_seq(R_PREV, 0, 1, 0, 0, 0);
	check_bt_seq(R_FIRST, 1, 0, 0, 0, 0);
	check_bt_seq(99, 0, 0, 0, 0, 0);
	check_bt_seq(R_FIRST, 0, 0, 0, 1, 0);
	check_bt_seq(R_FIRST, 0, 0, 0, 0, 1);
}
void sweep_bt_setcur(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++)
		check_bt_setcur((int)(nextr() & 1u),
		    (pgno_t)(2 + (nextr() % 20u)), (u_int)(nextr() % 4u));
}
void sweep_bt_seqset(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++) {
		int fl = (int)((nextr() % 4u) == 0 ? R_LAST :
		    (nextr() % 4u) == 1 ? R_CURSOR : R_FIRST);
		check_bt_seqset(fl, (int)(nextr() % 5u == 0),
		    (int)(nextr() % 3u == 0), (int)(nextr() % 29u == 0),
		    (int)(nextr() % 13u == 0));
	}
}
void sweep_bt_seqadv(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++)
		check_bt_seqadv((int)(nextr() & 1u) ? R_NEXT : R_PREV,
		    (int)(nextr() % 17u == 0), (int)(nextr() % 19u == 0),
		    (int)(nextr() % 23u == 0), (indx_t)(nextr() % 3u),
		    8, (nextr() & 2u) ? (pgno_t)9 : P_INVALID);
}
void sweep_bt_first(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++)
		check_bt_first((int)(nextr() & 1u), 0, (int)(nextr() % 31u == 0),
		    (int)(nextr() & 1u));
}
void sweep_bt_seq(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++) {
		u_int fl = (nextr() % 4u == 0) ? R_LAST :
		    (nextr() % 4u == 1) ? R_PREV :
		    (nextr() % 4u == 2) ? R_NEXT : R_FIRST;
		check_bt_seq(fl, (int)(nextr() % 6u == 0), (int)(nextr() & 1u),
		    (int)(nextr() % 29u == 0), (int)(nextr() % 47u == 0),
		    (int)(nextr() & 4u));
	}
}
} // namespace


int main(void) {
	test_bt_setcur_edges();
	test_bt_seqset_edges();
	test_bt_seqadv_edges();
	test_bt_first_edges();
	test_bt_seq_edges();
	sweep_bt_setcur();
	sweep_bt_seqset();
	sweep_bt_seqadv();
	sweep_bt_first();
	sweep_bt_seq();
	std::printf("\n%-14s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < F_COUNT; i++)
		std::printf("%-14s %12lu %12lu\n", fn_name[i], n_cases[i], n_fails[i]);
	unsigned long tf = 0;
	for (int i = 0; i < F_COUNT; i++) tf += n_fails[i];
	return tf == 0 ? 0 : 1;
}
