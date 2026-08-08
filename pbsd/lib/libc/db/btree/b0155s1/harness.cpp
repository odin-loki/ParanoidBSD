/*
 * Differential harness for batch b0155s1: btree bt_put.c routines only.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.db.btree.b0155s1;

namespace P = pbsd::lib_libc_db_btree::b0155s1;

extern "C" {
typedef uint32_t pgno_t;
typedef uint16_t indx_t;
typedef uint32_t recno_t;
typedef unsigned int u_int;
typedef unsigned char u_char;
typedef uint32_t u_int32_t;
typedef char *caddr_t;

typedef enum { DB_BTREE, DB_HASH, DB_RECNO } DBTYPE;

typedef struct {
	void *data;
	size_t size;
} DBT;

typedef struct __db {
	DBTYPE type;
	int (*close)(struct __db *);
	int (*del)(const struct __db *, const DBT *, unsigned int);
	int (*get)(const struct __db *, const DBT *, DBT *, unsigned int);
	int (*put)(const struct __db *, DBT *, const DBT *, unsigned int);
	int (*seq)(const struct __db *, DBT *, DBT *, unsigned int);
	int (*sync)(const struct __db *, unsigned int);
	void *internal;
	int (*fd)(const struct __db *);
} DB;

typedef struct MPOOL {
	char dummy;
} MPOOL;

typedef struct _page {
	pgno_t pgno;
	pgno_t prevpg;
	pgno_t nextpg;
	u_int32_t flags;
	indx_t lower;
	indx_t upper;
	indx_t linp[1];
} PAGE;

typedef struct _epgno {
	pgno_t pgno;
	indx_t index;
} EPGNO;

typedef struct _epg {
	PAGE *page;
	indx_t index;
} EPG;

typedef struct _cursor {
	EPGNO pg;
	DBT key;
	recno_t rcursor;
	u_int8_t flags;
} CURSOR;

typedef struct _btree {
	MPOOL *bt_mp;
	DB *bt_dbp;
	EPG bt_cur;
	PAGE *bt_pinned;
	CURSOR bt_cursor;
	EPGNO bt_stack[50];
	EPGNO *bt_sp;
	DBT bt_rkey;
	DBT bt_rdata;
	int bt_fd;
	pgno_t bt_free;
	u_int32_t bt_psize;
	indx_t bt_ovflsize;
	int bt_lorder;
	int bt_order;
	EPGNO bt_last;
	int (*bt_cmp)(const DBT *, const DBT *);
	size_t (*bt_pfx)(const DBT *, const DBT *);
	int (*bt_irec)(struct _btree *, recno_t);
	FILE *bt_rfp;
	int bt_rfd;
	caddr_t bt_cmap;
	caddr_t bt_smap;
	caddr_t bt_emap;
	size_t bt_msize;
	recno_t bt_nrecs;
	size_t bt_reclen;
	u_char bt_bval;
	u_int32_t flags;
} BTREE;

typedef struct {
	unsigned get_calls, put_calls, new_calls, delete_calls;
	unsigned search_calls, split_calls, ovfl_put_calls, ovfl_del_calls;
	unsigned cmp_calls, dleaf_calls, setcur_calls;
	int get_force_null, search_force_null;
	int split_ret, ovfl_put_ret, dleaf_ret;
	int search_exact, cmp_ret;
	int split_force_error;
	pgno_t get_last_pgno, ovfl_pgno;
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

EPG *ref_bt_fast(BTREE *, const DBT *, const DBT *, int *);
int ref___bt_put(const DB *, DBT *, const DBT *, u_int);

int harness_cmp(const DBT *a, const DBT *b)
{
	size_t min = a->size < b->size ? a->size : b->size;
	if (min > 0) {
		int c = std::memcmp(a->data, b->data, min);
		if (c != 0)
			return c;
	}
	if (a->size < b->size)
		return -1;
	if (a->size > b->size)
		return 1;
	return 0;
}

struct MockSnap {
	test_mock_state mock;
};

MockSnap snap_mock(void)
{
	MockSnap s;
	s.mock = test_mock;
	return s;
}

void restore_mock(const MockSnap &s)
{
	test_mock = s.mock;
}
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr size_t PAGE_SZ = 512;
constexpr unsigned SWEEP_ITERS = 200000;

enum { NOT = 0, BACK = 1, FORWARD = 2 };

#define P_INVALID 0xffffffffu
#define P_ROOT 1
#define P_BLEAF 0x02
#define P_BIGKEY 0x02
#define P_BIGDATA 0x01
#define B_MODIFIED 0x00004
#define B_RDONLY 0x00010
#define B_NODUPS 0x00020
#define CURS_ACQUIRE 0x01
#define CURS_AFTER 0x02
#define CURS_BEFORE 0x04
#define CURS_INIT 0x08
#define R_CURSOR 1
#define R_NOOVERWRITE 8
#define R_SETCURSOR 10
#define RET_ERROR -1
#define RET_SUCCESS 0
#define RET_SPECIAL 1
#define MPOOL_DIRTY 0x01

#define BTDATAOFF \
	(sizeof(pgno_t) + sizeof(pgno_t) + sizeof(pgno_t) + \
	    sizeof(u_int32_t) + sizeof(indx_t) + sizeof(indx_t))
#define NEXTINDEX(p) (((p)->lower - BTDATAOFF) / sizeof(indx_t))
#define LALIGN(n) (((n) + sizeof(pgno_t) - 1) & ~(sizeof(pgno_t) - 1))
#define NBLEAFDBT(ksize, dsize) \
	LALIGN(sizeof(u_int32_t) + sizeof(u_int32_t) + sizeof(u_char) + \
	    (ksize) + (dsize))

enum Fn { F_BT_FAST, F_BT_PUT, F_COUNT };

const char *fn_name[F_COUNT] = { "bt_fast", "__bt_put" };

unsigned long n_cases[F_COUNT];
unsigned long n_fails[F_COUNT];
unsigned reported[F_COUNT];

uint64_t rng = 0xb0155b0155b0155ULL;

uint64_t nextr(void)
{
	rng ^= rng << 13;
	rng ^= rng >> 7;
	rng ^= rng << 17;
	return rng;
}

void fail(int fn, const char *msg)
{
	n_fails[fn]++;
	if (reported[fn] < 10) {
		reported[fn]++;
		std::fprintf(stderr, "FAIL %s: %s\n", fn_name[fn], msg);
	}
}

void check_eq(int fn, bool ok, const char *msg)
{
	n_cases[fn]++;
	if (!ok)
		fail(fn, msg);
}

void guard_fill(void *p, size_t n)
{
	std::memset(p, GUARD, n);
}

static int port_cmp(const P::DBT *a, const P::DBT *b)
{
	return harness_cmp((const DBT *)a, (const DBT *)b);
}

indx_t calc_ovflsize(u_int32_t psize, int minkeypage)
{
	indx_t ov = (indx_t)((psize - BTDATAOFF) / (unsigned)minkeypage -
	    (sizeof(indx_t) + NBLEAFDBT(0, 0)));
	if (ov < (indx_t)(NBLEAFDBT(8, 8) + sizeof(indx_t)))
		ov = (indx_t)(NBLEAFDBT(8, 8) + sizeof(indx_t));
	return ov;
}

void init_tree_port(P::BTREE &t, P::MPOOL &mp, P::DB &db, u_int32_t flags)
{
	std::memset(&t, 0, sizeof(t));
	t.bt_mp = &mp;
	t.bt_dbp = &db;
	t.bt_psize = (u_int32_t)PAGE_SZ;
	t.bt_ovflsize = calc_ovflsize((u_int32_t)PAGE_SZ, 2);
	t.bt_sp = t.bt_stack;
	t.bt_order = NOT;
	t.bt_fd = -1;
	t.bt_cmp = port_cmp;
	t.flags = flags;
	db.internal = &t;
}

void init_tree_ref(BTREE &t, MPOOL &mp, DB &db, u_int32_t flags)
{
	std::memset(&t, 0, sizeof(t));
	t.bt_mp = &mp;
	t.bt_dbp = &db;
	t.bt_psize = (u_int32_t)PAGE_SZ;
	t.bt_ovflsize = calc_ovflsize((u_int32_t)PAGE_SZ, 2);
	t.bt_sp = t.bt_stack;
	t.bt_order = NOT;
	t.bt_fd = -1;
	t.bt_cmp = harness_cmp;
	t.flags = flags;
	db.internal = &t;
}

struct MockDelta {
	unsigned get, put, search, split, ovfl_put, cmp, dleaf, setcur;
};

MockDelta mock_delta(const test_mock_state &before, const test_mock_state &after)
{
	MockDelta d{};
	d.get = after.get_calls - before.get_calls;
	d.put = after.put_calls - before.put_calls;
	d.search = after.search_calls - before.search_calls;
	d.split = after.split_calls - before.split_calls;
	d.ovfl_put = after.ovfl_put_calls - before.ovfl_put_calls;
	d.cmp = after.cmp_calls - before.cmp_calls;
	d.dleaf = after.dleaf_calls - before.dleaf_calls;
	d.setcur = after.setcur_calls - before.setcur_calls;
	return d;
}

bool mock_delta_eq(const MockDelta &a, const MockDelta &b)
{
	return a.get == b.get && a.put == b.put && a.search == b.search &&
	    a.split == b.split && a.ovfl_put == b.ovfl_put && a.cmp == b.cmp &&
	    a.dleaf == b.dleaf && a.setcur == b.setcur;
}

bool harness_cmp_btree(const P::BTREE &tp, const BTREE &tr)
{
	return tp.flags == tr.flags && tp.bt_order == tr.bt_order &&
	    tp.bt_free == tr.bt_free && tp.bt_last.pgno == tr.bt_last.pgno &&
	    tp.bt_last.index == tr.bt_last.index &&
	    tp.bt_cursor.pg.pgno == tr.bt_cursor.pg.pgno &&
	    tp.bt_cursor.pg.index == tr.bt_cursor.pg.index &&
	    tp.bt_cursor.flags == tr.bt_cursor.flags &&
	    (tp.bt_pinned == nullptr) == (tr.bt_pinned == nullptr);
}

void build_bleaf_page(PAGE *pg, size_t psize, int nents,
    const u_int32_t *ksizes, const u_int32_t *dsizes, const u_char *eflags)
{
	size_t off = psize;
	std::memset(pg, 0, psize);
	pg->flags = P_BLEAF;
	while (nents > 0) {
		size_t trial = psize;
		for (int i = nents - 1; i >= 0; i--) {
			trial -= NBLEAFDBT(ksizes[i], dsizes[i]);
			if (trial < BTDATAOFF + (size_t)nents * sizeof(indx_t))
				goto shrink;
		}
		break;
shrink:
		nents--;
	}
	off = psize;
	for (int i = nents - 1; i >= 0; i--) {
		size_t ksz = ksizes[i];
		size_t dsz = dsizes[i];
		size_t esz = NBLEAFDBT(ksz, dsz);
		off -= esz;
		unsigned char *e = (unsigned char *)pg + off;
		*(u_int32_t *)e = (u_int32_t)ksz;
		*(u_int32_t *)(e + 4) = (u_int32_t)dsz;
		e[8] = eflags[i];
		for (size_t k = 0; k < ksz + dsz; k++)
			e[9 + k] = (u_char)(0x80 + k + (unsigned)i);
		*(indx_t *)((unsigned char *)pg + BTDATAOFF +
		    (size_t)i * sizeof(indx_t)) = (indx_t)off;
	}
	pg->lower = (indx_t)(BTDATAOFF + nents * sizeof(indx_t));
	pg->upper = (indx_t)off;
}

void restore_mock_bt_put_params(int search_null, int exact, int split, int ovfl,
    int dleaf_fail, int split_fail, int ovfl_fail)
{
	test_mock.search_force_null = search_null;
	test_mock.search_exact = exact;
	test_mock.split_force_error = split_fail ? 1 : 0;
	test_mock.split_ret = split_fail ? RET_ERROR : RET_SUCCESS;
	test_mock.ovfl_put_ret = ovfl_fail ? RET_ERROR : RET_SUCCESS;
	test_mock.dleaf_ret = dleaf_fail ? RET_ERROR : RET_SUCCESS;
	if (split && !split_fail)
		test_mock.split_ret = RET_SUCCESS;
	if (ovfl && !ovfl_fail)
		test_mock.ovfl_put_ret = RET_SUCCESS;
}

void check_bt_fast(int order, int nents, indx_t last_idx, pgno_t last_pg,
    int cmp_ret, int get_null, size_t ksz, size_t dsz, int no_room)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	static unsigned char leaf_p[PAGE_SZ];
	static unsigned char keybuf[32];
	static unsigned char databuf[32];
	DBT key, data;
	u_int32_t ksizes[8] = { 4, 6, 8, 5, 7, 3, 9, 4 };
	u_int32_t dsizes[8] = { 3, 4, 2, 6, 5, 4, 1, 3 };
	u_char eflags[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	test_mock_reset();
	test_mock.cmp_ret = cmp_ret;
	test_mock.get_force_null = get_null;
	init_tree_port(tp, mp_p, db_p, 0);
	init_tree_ref(tr, mp_r, db_r, 0);
	tp.bt_order = order;
	tr.bt_order = order;
	tp.bt_last.pgno = last_pg;
	tp.bt_last.index = last_idx;
	tr.bt_last.pgno = last_pg;
	tr.bt_last.index = last_idx;
	guard_fill(leaf_p, PAGE_SZ);
	build_bleaf_page((PAGE *)leaf_p, PAGE_SZ, nents, ksizes, dsizes, eflags);
	((PAGE *)leaf_p)->pgno = last_pg;
	((PAGE *)leaf_p)->prevpg = order == BACK ? P_INVALID : 5;
	((PAGE *)leaf_p)->nextpg = order == FORWARD ? P_INVALID : 6;
	if (no_room)
		((PAGE *)leaf_p)->upper = (indx_t)(((PAGE *)leaf_p)->lower + 8);
	static unsigned char init_leaf[PAGE_SZ];
	std::memcpy(init_leaf, leaf_p, PAGE_SZ);
	test_mock_register(last_pg, leaf_p);
	keybuf[0] = 0x90;
	key.data = keybuf;
	key.size = ksz ? ksz : 4;
	data.data = databuf;
	data.size = dsz ? dsz : 4;

	int exact_p = -1, exact_r = -1;
	MockSnap snap = snap_mock();
	P::EPG *ep_p = P::bt_fast(&tp, (P::DBT *)&key, (P::DBT *)&data, &exact_p);
	u_int32_t order_p = tp.bt_order;
	MockDelta dp = mock_delta(snap.mock, test_mock);
	std::memcpy(leaf_p, init_leaf, PAGE_SZ);
	restore_mock(snap);
	test_mock.cmp_ret = cmp_ret;
	test_mock.get_force_null = get_null;
	EPG *ep_r = ref_bt_fast(&tr, &key, &data, &exact_r);
	MockDelta dr = mock_delta(snap.mock, test_mock);
	char msg[160];
	std::snprintf(msg, sizeof(msg),
	    "null port=%d ref=%d exact port=%d ref=%d order=%d cmp=%d room=%d",
	    ep_p == nullptr, ep_r == nullptr, exact_p, exact_r, order, cmp_ret,
	    !no_room);
	check_eq(F_BT_FAST, (ep_p == nullptr) == (ep_r == nullptr), msg);
	check_eq(F_BT_FAST, exact_p == exact_r, "exact");
	check_eq(F_BT_FAST, mock_delta_eq(dp, dr), "mock delta");
	if (ep_p && ep_r)
		check_eq(F_BT_FAST, ep_p->index == ep_r->index, "index");
	check_eq(F_BT_FAST, order_p == tr.bt_order, "bt_order");
}

void check_bt_put(u_int flags, u_int32_t tflags, int search_null, int exact,
    int split, int ovfl, int room, int curs_init, int curs_acquire,
    int dleaf_fail, int split_fail, int ovfl_fail, int pinned,
    int use_fast_path)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	static unsigned char leaf_p[PAGE_SZ];
	static unsigned char keybuf[64];
	static unsigned char databuf[64];
	DBT key, data;
	u_int32_t ksizes[2] = { 4, 5 };
	u_int32_t dsizes[2] = { 3, 4 };
	u_char eflags[2] = { 0, 0 };

	test_mock_reset();
	test_mock.search_force_null = search_null;
	test_mock.search_exact = exact;
	test_mock.search_epg.page = (PAGE *)leaf_p;
	test_mock.search_epg.index = 0;
	restore_mock_bt_put_params(search_null, exact, split, ovfl, dleaf_fail,
	    split_fail, ovfl_fail);
	init_tree_port(tp, mp_p, db_p, tflags);
	init_tree_ref(tr, mp_r, db_r, tflags);
	if (use_fast_path) {
		tp.bt_order = FORWARD;
		tr.bt_order = FORWARD;
		tp.bt_last.pgno = 8;
		tp.bt_last.index = 1;
		tr.bt_last.pgno = 8;
		tr.bt_last.index = 1;
	}
	guard_fill(leaf_p, PAGE_SZ);
	build_bleaf_page((PAGE *)leaf_p, PAGE_SZ, 2, ksizes, dsizes, eflags);
	((PAGE *)leaf_p)->pgno = 8;
	if (!room)
		((PAGE *)leaf_p)->upper = (indx_t)(((PAGE *)leaf_p)->lower + 8);
	test_mock.search_epg.page = (PAGE *)leaf_p;
	key.data = keybuf;
	key.size = ovfl ? tp.bt_ovflsize + 1 : 4;
	data.data = databuf;
	data.size = ovfl ? tp.bt_ovflsize + 1 : 4;
	std::memset(keybuf, 0xab, sizeof(keybuf));
	std::memset(databuf, 0xcd, sizeof(databuf));
	if (pinned) {
		tp.bt_pinned = (P::PAGE *)leaf_p;
		tr.bt_pinned = (PAGE *)leaf_p;
	}
	if (curs_init || flags == R_CURSOR) {
		tp.bt_cursor.flags = CURS_INIT |
		    (curs_acquire ? CURS_ACQUIRE : 0);
		tp.bt_cursor.pg.pgno = 8;
		tp.bt_cursor.pg.index = 0;
		tr.bt_cursor.flags = tp.bt_cursor.flags;
		tr.bt_cursor.pg.pgno = 8;
		tr.bt_cursor.pg.index = 0;
	}
	static unsigned char init_leaf[PAGE_SZ];
	std::memcpy(init_leaf, leaf_p, PAGE_SZ);
	test_mock_register(8, leaf_p);

	MockSnap snap = snap_mock();
	errno = 0;
	int rp = P::__bt_put(&db_p, (P::DBT *)&key, (P::DBT *)&data, flags);
	int ep = errno;
	u_int32_t flags_p = tp.flags;
	int order_p = tp.bt_order;
	MockDelta dp = mock_delta(snap.mock, test_mock);
	std::memcpy(leaf_p, init_leaf, PAGE_SZ);
	restore_mock(snap);
	errno = 0;
	restore_mock_bt_put_params(search_null, exact, split, ovfl, dleaf_fail,
	    split_fail, ovfl_fail);
	test_mock.search_epg.page = (PAGE *)leaf_p;
	test_mock.search_epg.index = 0;
	if (pinned)
		tr.bt_pinned = (PAGE *)leaf_p;
	if (curs_init || flags == R_CURSOR) {
		tr.bt_cursor.flags = CURS_INIT |
		    (curs_acquire ? CURS_ACQUIRE : 0);
		tr.bt_cursor.pg.pgno = 8;
		tr.bt_cursor.pg.index = 0;
	}
	if (use_fast_path) {
		tr.bt_order = FORWARD;
		tr.bt_last.pgno = 8;
		tr.bt_last.index = 1;
	}
	int rr = ref___bt_put(&db_r, &key, &data, flags);
	int er = errno;
	MockDelta dr = mock_delta(snap.mock, test_mock);
	char msg[240];
	std::snprintf(msg, sizeof(msg),
	    "ret port=%d ref=%d fl=%u exact=%d split=%d ovfl=%d room=%d "
	    "curs=%d fast=%d errno_p=%d errno_r=%d",
	    rp, rr, flags, exact, split, ovfl, room, curs_init, use_fast_path,
	    ep, er);
	check_eq(F_BT_PUT, rp == rr && ep == er, msg);
	check_eq(F_BT_PUT, mock_delta_eq(dp, dr), "mock delta");
	if (rp == rr && rp == RET_SUCCESS) {
		check_eq(F_BT_PUT, flags_p == tr.flags, "flags");
		check_eq(F_BT_PUT, order_p == tr.bt_order, "bt_order");
	}
}

void test_bt_fast_edges(void)
{
	check_bt_fast(FORWARD, 4, 3, 8, 0, 0, 4, 4, 0);
	check_bt_fast(FORWARD, 4, 3, 8, 1, 0, 4, 4, 0);
	check_bt_fast(FORWARD, 4, 3, 8, -1, 0, 4, 4, 0);
	check_bt_fast(BACK, 4, 0, 8, 0, 0, 4, 4, 0);
	check_bt_fast(BACK, 4, 0, 8, 1, 0, 4, 4, 0);
	check_bt_fast(BACK, 4, 0, 8, -1, 0, 4, 4, 0);
	check_bt_fast(FORWARD, 4, 3, 8, 0, 1, 4, 4, 0);
	check_bt_fast(FORWARD, 2, 1, 8, 0, 0, 200, 4, 0);
	check_bt_fast(FORWARD, 4, 3, 8, 0, 0, 4, 4, 1);
	check_bt_fast(NOT, 4, 3, 8, 0, 0, 4, 4, 0);
	check_bt_fast(FORWARD, 1, 0, 8, 0, 0, 4, 4, 0);
	check_bt_fast(BACK, 1, 0, 8, 0, 0, 4, 4, 0);
}

void test_bt_put_edges(void)
{
	check_bt_put(0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0);
	check_bt_put(R_NOOVERWRITE, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0);
	check_bt_put(0, B_RDONLY, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0);
	check_bt_put(99, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0);
	check_bt_put(0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0);
	check_bt_put(0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	check_bt_put(0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0);
	check_bt_put(R_SETCURSOR, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0);
	check_bt_put(0, B_NODUPS, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0);
	check_bt_put(R_CURSOR, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0);
	check_bt_put(R_CURSOR, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0);
	check_bt_put(R_CURSOR, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0);
	check_bt_put(0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0);
	check_bt_put(0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0);
	check_bt_put(0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0);
	check_bt_put(0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1);
}

void sweep_bt_fast(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++) {
		int order = (int)((nextr() % 4u) == 0 ? NOT :
		    (nextr() % 3u) ? FORWARD : BACK);
		int nents = (int)(nextr() % 6u) + 1;
		indx_t last_idx = order == BACK ? 0 : (indx_t)(nents - 1);
		check_bt_fast(order, nents, last_idx, 8,
		    (int)(nextr() % 3u) - 1, (int)(nextr() % 37u == 0),
		    (size_t)(nextr() % 16u) + 1, (size_t)(nextr() % 16u) + 1,
		    (int)(nextr() % 41u == 0));
	}
}

void sweep_bt_put(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++) {
		u_int fl = (nextr() % 6u == 0) ? R_NOOVERWRITE :
		    (nextr() % 7u == 0) ? R_SETCURSOR :
		    (nextr() % 8u == 0) ? R_CURSOR :
		    (nextr() % 9u == 0) ? 99u : 0u;
		check_bt_put(fl, (nextr() & 16u) ? B_RDONLY :
		    (nextr() & 32u) ? B_NODUPS : 0,
		    (int)(nextr() % 37u == 0), (int)(nextr() & 1u),
		    (int)(nextr() % 5u == 0), (int)(nextr() % 11u == 0),
		    (int)(nextr() & 1u), (int)(fl == R_CURSOR && (nextr() & 2u)),
		    (int)(fl == R_CURSOR && (nextr() & 4u)),
		    (int)(nextr() % 43u == 0), (int)(nextr() % 47u == 0),
		    (int)(nextr() % 53u == 0), (int)(nextr() % 59u == 0),
		    (int)(nextr() % 61u == 0));
	}
}

} // namespace

int main(void)
{
	test_bt_fast_edges();
	test_bt_put_edges();

	sweep_bt_fast();
	sweep_bt_put();

	std::printf("\n%-14s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < F_COUNT; i++)
		std::printf("%-14s %12lu %12lu\n", fn_name[i], n_cases[i],
		    n_fails[i]);

	unsigned long total_fails = 0;
	for (int i = 0; i < F_COUNT; i++)
		total_fails += n_fails[i];
	return total_fails == 0 ? 0 : 1;
}
