/*
 * Differential harness for batch b0149: btree split routines.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.db.btree.b0149;

namespace P = pbsd::lib_libc_db_btree::b0149;

extern "C" {
typedef uint32_t pgno_t;
typedef uint16_t indx_t;
typedef uint32_t recno_t;
typedef unsigned int u_int;
typedef unsigned char u_char;
typedef uint32_t u_int32_t;

typedef struct {
	void *data;
	size_t size;
} DBT;

typedef struct __db {
	int (*close)(struct __db *);
	void *internal;
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

typedef struct _cursor {
	EPGNO pg;
	DBT key;
	recno_t rcursor;
	uint8_t flags;
} CURSOR;

typedef struct _btree {
	MPOOL *bt_mp;
	DB *bt_dbp;
	struct {
		PAGE *page;
		indx_t index;
	} bt_cur;
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
	void *bt_rfp;
	int bt_rfd;
	char *bt_cmap;
	char *bt_smap;
	char *bt_emap;
	size_t bt_msize;
	recno_t bt_nrecs;
	size_t bt_reclen;
	uint8_t bt_bval;
	u_int32_t flags;
} BTREE;

typedef struct {
	unsigned get_calls;
	unsigned put_calls;
	unsigned new_calls;
	unsigned panic_calls;
	int get_force_null;
	int new_force_null;
	int calloc_fail;
	pgno_t get_last_pgno;
	unsigned get_last_flags;
	void *last_put_page;
	unsigned last_put_flags;
	pgno_t next_pgno;
	int nreg;
	pgno_t reg_pgno[128];
	void *reg_page[128];
	int new_fail_after;
} test_mock_state;

extern test_mock_state test_mock;

void test_mock_reset(void);
void test_mock_register(pgno_t pgno, void *page);

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

int ref___bt_split(BTREE *, PAGE *, const DBT *, const DBT *, int, size_t,
    u_int32_t);
PAGE *ref_bt_page(BTREE *, PAGE *, PAGE **, PAGE **, indx_t *, size_t);
PAGE *ref_bt_root(BTREE *, PAGE *, PAGE **, PAGE **, indx_t *, size_t);
int ref_bt_rroot(BTREE *, PAGE *, PAGE *, PAGE *);
int ref_bt_broot(BTREE *, PAGE *, PAGE *, PAGE *);
PAGE *ref_bt_psplit(BTREE *, PAGE *, PAGE *, PAGE *, indx_t *, size_t);
int ref_bt_preserve(BTREE *, pgno_t);
recno_t ref_rec_total(PAGE *);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr size_t PAGE_SZ = 512;
constexpr unsigned SWEEP_ITERS = 200000;

#define P_INVALID 0
#define P_ROOT 1
#define P_BINTERNAL 0x01
#define P_BLEAF 0x02
#define P_RINTERNAL 0x08
#define P_RLEAF 0x10
#define P_TYPE 0x1f
#define P_BIGKEY 0x02
#define P_BIGDATA 0x01
#define P_PRESERVE 0x20
#define R_RECNO 0x00080
#define CURS_INIT 0x08
#define RET_ERROR -1
#define RET_SUCCESS 0
#define MPOOL_DIRTY 0x01

#define NEXTINDEX(p) (((p)->lower - BTDATAOFF) / sizeof(indx_t))

#define BTDATAOFF \
	(sizeof(pgno_t) + sizeof(pgno_t) + sizeof(pgno_t) + \
	    sizeof(u_int32_t) + sizeof(indx_t) + sizeof(indx_t))

enum Fn {
	F_REC_TOTAL,
	F_BT_PRESERVE,
	F_BT_PSPLIT,
	F_BT_RROOT,
	F_BT_BROOT,
	F_BT_PAGE,
	F_BT_ROOT,
	F_BT_SPLIT,
	F_COUNT
};

const char *fn_name[F_COUNT] = {
	"rec_total",
	"bt_preserve",
	"bt_psplit",
	"bt_rroot",
	"bt_broot",
	"bt_page",
	"bt_root",
	"__bt_split",
};

unsigned long n_cases[F_COUNT];
unsigned long n_fails[F_COUNT];
unsigned reported[F_COUNT];

uint64_t rng = 0xb0149b0149b0149ULL;

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

void case_ok(int fn)
{
	n_cases[fn]++;
}

void check_eq(int fn, bool ok, const char *msg)
{
	n_cases[fn]++;
	if (!ok)
		fail(fn, msg);
}

u_char rnd_byte(void)
{
	return (u_char)(nextr() & 0xffu);
}

size_t align_pg(size_t n)
{
	return (n + sizeof(pgno_t) - 1) & ~(sizeof(pgno_t) - 1);
}

void guard_fill(void *p, size_t n)
{
	std::memset(p, GUARD, n);
}

bool bufs_eq(const unsigned char *a, const unsigned char *b, size_t n)
{
	return std::memcmp(a, b, n) == 0;
}

extern "C" size_t harness_pfx(const DBT *a, const DBT *b)
{
	size_t m = a->size < b->size ? a->size : b->size;
	return m > 1 ? m / 2 : m;
}

static size_t port_pfx(const P::DBT *a, const P::DBT *b)
{
	return harness_pfx((const DBT *)a, (const DBT *)b);
}

void init_tree(P::BTREE &tp, P::MPOOL &mp, P::DB &db, u_int32_t flags,
    u_int32_t psize)
{
	std::memset(&tp, 0, sizeof(tp));
	tp.bt_mp = &mp;
	tp.bt_dbp = &db;
	tp.bt_psize = psize;
	tp.bt_sp = tp.bt_stack;
	tp.flags = flags;
	db.internal = &tp;
}

void init_tree(BTREE &tr, MPOOL &mp, DB &db, u_int32_t flags, u_int32_t psize)
{
	std::memset(&tr, 0, sizeof(tr));
	tr.bt_mp = &mp;
	tr.bt_dbp = &db;
	tr.bt_psize = psize;
	tr.bt_sp = tr.bt_stack;
	tr.flags = flags;
	db.internal = &tr;
}

PAGE *alloc_page_buf(unsigned char *buf, size_t sz)
{
	guard_fill(buf, sz);
	return (PAGE *)buf;
}

void build_rinternal_page(PAGE *pg, size_t psize, int nents,
    const recno_t *nrecs, const pgno_t *pgnos, u_int32_t pflags)
{
	size_t off = psize;
	pg->pgno = 2;
	pg->prevpg = P_INVALID;
	pg->nextpg = P_INVALID;
	pg->flags = pflags;
	for (int i = nents - 1; i >= 0; i--) {
		size_t esz = align_pg(sizeof(recno_t) + sizeof(pgno_t));
		off -= esz;
		unsigned char *e = (unsigned char *)pg + off;
		*(recno_t *)e = nrecs[i];
		*(pgno_t *)(e + sizeof(recno_t)) = pgnos[i];
		pg->linp[i] = (indx_t)off;
	}
	pg->lower = (indx_t)(BTDATAOFF + nents * sizeof(indx_t));
	pg->upper = (indx_t)off;
}

void build_bleaf_page(PAGE *pg, size_t psize, int nents,
    const u_int32_t *ksizes, const u_int32_t *dsizes, const u_char *eflags)
{
	size_t off = psize;
	pg->flags = P_BLEAF;
	for (int i = nents - 1; i >= 0; i--) {
		size_t ksz = ksizes[i];
		size_t dsz = dsizes[i];
		u_char fl = eflags[i];
		size_t esz = align_pg(sizeof(u_int32_t) + sizeof(u_int32_t) +
		    sizeof(u_char) + ksz + dsz);
		off -= esz;
		unsigned char *e = (unsigned char *)pg + off;
		*(u_int32_t *)e = (u_int32_t)ksz;
		*(u_int32_t *)(e + 4) = (u_int32_t)dsz;
		e[8] = fl;
		for (size_t k = 0; k < ksz + dsz; k++)
			e[9 + k] = (u_char)(0x80 + k + i);
		pg->linp[i] = (indx_t)off;
	}
	pg->lower = (indx_t)(BTDATAOFF + nents * sizeof(indx_t));
	pg->upper = (indx_t)off;
}

void build_binternal_page(PAGE *pg, size_t psize, int nents,
    const u_int32_t *ksizes, const u_char *eflags, const pgno_t *pgnos)
{
	size_t off = psize;
	pg->flags = P_BINTERNAL;
	for (int i = nents - 1; i >= 0; i--) {
		size_t ksz = ksizes[i];
		size_t esz = align_pg(sizeof(u_int32_t) + sizeof(pgno_t) +
		    sizeof(u_char) + ksz);
		off -= esz;
		unsigned char *e = (unsigned char *)pg + off;
		*(u_int32_t *)e = (u_int32_t)ksz;
		*(pgno_t *)(e + 4) = pgnos[i];
		e[8] = eflags[i];
		for (size_t k = 0; k < ksz; k++)
			e[9 + k] = (u_char)(0xa0 + k + i);
		pg->linp[i] = (indx_t)off;
	}
	pg->lower = (indx_t)(BTDATAOFF + nents * sizeof(indx_t));
	pg->upper = (indx_t)off;
}

void build_rleaf_page(PAGE *pg, size_t psize, int nents,
    const u_int32_t *dsizes, const u_char *eflags)
{
	size_t off = psize;
	pg->flags = P_RLEAF;
	for (int i = nents - 1; i >= 0; i--) {
		size_t dsz = dsizes[i];
		size_t esz = align_pg(sizeof(u_int32_t) + sizeof(u_char) + dsz);
		off -= esz;
		unsigned char *e = (unsigned char *)pg + off;
		*(u_int32_t *)e = (u_int32_t)dsz;
		e[4] = eflags[i];
		for (size_t k = 0; k < dsz; k++)
			e[5 + k] = (u_char)(0xc0 + k + i);
		pg->linp[i] = (indx_t)off;
	}
	pg->lower = (indx_t)(BTDATAOFF + nents * sizeof(indx_t));
	pg->upper = (indx_t)off;
}

void check_rec_total(int nents, const recno_t *nrecs)
{
	unsigned char buf_p[PAGE_SZ];
	unsigned char buf_r[PAGE_SZ];
	pgno_t pgs[] = { 10, 11, 12, 13 };

	build_rinternal_page((PAGE *)buf_p, PAGE_SZ, nents, nrecs, pgs, P_RINTERNAL);
	std::memcpy(buf_r, buf_p, PAGE_SZ);

	recno_t rp = P::rec_total((P::PAGE *)buf_p);
	recno_t rr = ref_rec_total((PAGE *)buf_r);
	char msg[128];
	std::snprintf(msg, sizeof(msg), "ret port=%u ref=%u nents=%d",
	    (unsigned)rp, (unsigned)rr, nents);
	check_eq(F_REC_TOTAL, rp == rr, msg);
}

void check_bt_preserve(int force_null, pgno_t pgno)
{
	unsigned char pgbuf_p[PAGE_SZ];
	unsigned char pgbuf_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;

	test_mock_reset();
	test_mock.get_force_null = force_null;
	init_tree(tp, mp_p, db_p, 0, PAGE_SZ);
	init_tree(tr, mp_r, db_r, 0, PAGE_SZ);
	alloc_page_buf(pgbuf_p, PAGE_SZ);
	alloc_page_buf(pgbuf_r, PAGE_SZ);
	((PAGE *)pgbuf_p)->pgno = pgno;
	((PAGE *)pgbuf_r)->pgno = pgno;
	test_mock_register(pgno, pgbuf_p);
	test_mock_register(pgno, pgbuf_r);

	int rp = P::bt_preserve(&tp, pgno);
	int rr = ref_bt_preserve(&tr, pgno);
	char msg[128];
	std::snprintf(msg, sizeof(msg), "ret port=%d ref=%d null=%d", rp, rr,
	    force_null);
	check_eq(F_BT_PRESERVE, rp == rr, msg);
	if (!force_null) {
		check_eq(F_BT_PRESERVE,
		    (((PAGE *)pgbuf_p)->flags & P_PRESERVE) ==
			(((PAGE *)pgbuf_r)->flags & P_PRESERVE),
		    "P_PRESERVE");
		check_eq(F_BT_PRESERVE, bufs_eq(pgbuf_p, pgbuf_r, PAGE_SZ),
		    "pagebuf");
	}
}

struct PsplitCtx {
	unsigned char src_p[PAGE_SZ];
	unsigned char src_r[PAGE_SZ];
	unsigned char lp_p[PAGE_SZ];
	unsigned char lp_r[PAGE_SZ];
	unsigned char rp_p[PAGE_SZ];
	unsigned char rp_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
};

void check_bt_psplit(u_int32_t ptype, int nents, indx_t skip, size_t ilen,
    int curs_init, indx_t curs_idx)
{
	PsplitCtx c;
	u_int32_t ksizes[8] = { 4, 8, 6, 10, 5, 7, 3, 9 };
	u_int32_t dsizes[8] = { 4, 3, 6, 2, 5, 4, 8, 1 };
	u_char eflags[8] = { 0, 0, P_BIGKEY, 0, 0, 0, 0, 0 };
	pgno_t pgnos[8] = { 20, 21, 22, 23, 24, 25, 26, 27 };
	recno_t nrecs[8] = { 10, 20, 30, 40, 50, 60, 70, 80 };
	u_int32_t rsizes[8] = { 6, 8, 4, 10, 3, 7, 5, 9 };

	test_mock_reset();
	init_tree(c.tp, c.mp_p, c.db_p, 0, PAGE_SZ);
	init_tree(c.tr, c.mp_r, c.db_r, 0, PAGE_SZ);
	alloc_page_buf(c.src_p, PAGE_SZ);
	alloc_page_buf(c.src_r, PAGE_SZ);
	alloc_page_buf(c.lp_p, PAGE_SZ);
	alloc_page_buf(c.lp_r, PAGE_SZ);
	alloc_page_buf(c.rp_p, PAGE_SZ);
	alloc_page_buf(c.rp_r, PAGE_SZ);

	PAGE *sp = (PAGE *)c.src_p;
	PAGE *sr = (PAGE *)c.src_r;
	sp->pgno = 5;
	sr->pgno = 5;
	if (ptype == P_BLEAF)
		build_bleaf_page(sp, PAGE_SZ, nents, ksizes, dsizes, eflags);
	else if (ptype == P_BINTERNAL)
		build_binternal_page(sp, PAGE_SZ, nents, ksizes, eflags, pgnos);
	else if (ptype == P_RINTERNAL)
		build_rinternal_page(sp, PAGE_SZ, nents, nrecs, pgnos, P_RINTERNAL);
	else
		build_rleaf_page(sp, PAGE_SZ, nents, rsizes, eflags);
	std::memcpy(c.src_r, c.src_p, PAGE_SZ);

	PAGE *lp_p = (PAGE *)c.lp_p;
	PAGE *lp_r = (PAGE *)c.lp_r;
	PAGE *rp_p = (PAGE *)c.rp_p;
	PAGE *rp_r = (PAGE *)c.rp_r;
	lp_p->pgno = 6;
	lp_r->pgno = 6;
	rp_p->pgno = 7;
	rp_r->pgno = 7;
	lp_p->lower = lp_p->upper = BTDATAOFF;
	lp_r->lower = lp_r->upper = BTDATAOFF;
	rp_p->lower = rp_p->upper = BTDATAOFF;
	rp_r->lower = rp_r->upper = BTDATAOFF;
	lp_p->flags = rp_p->flags = ptype;
	lp_r->flags = rp_r->flags = ptype;

	indx_t skip_p = skip;
	indx_t skip_r = skip;
	if (curs_init) {
		c.tp.bt_cursor.flags = CURS_INIT;
		c.tp.bt_cursor.pg.pgno = sp->pgno;
		c.tp.bt_cursor.pg.index = curs_idx;
		c.tr.bt_cursor.flags = CURS_INIT;
		c.tr.bt_cursor.pg.pgno = sr->pgno;
		c.tr.bt_cursor.pg.index = curs_idx;
	}

	unsigned char init_src[PAGE_SZ];
	std::memcpy(init_src, c.src_p, PAGE_SZ);
	MockSnap snap = snap_mock();
	P::PAGE *ret_p = P::bt_psplit(&c.tp, (P::PAGE *)sp, (P::PAGE *)lp_p,
	    (P::PAGE *)rp_p, &skip_p, ilen);
	unsigned char res_src_p[PAGE_SZ];
	unsigned char res_lp_p[PAGE_SZ];
	unsigned char res_rp_p[PAGE_SZ];
	P::CURSOR res_cur_p = c.tp.bt_cursor;
	indx_t res_skip_p = skip_p;
	std::memcpy(res_src_p, c.src_p, PAGE_SZ);
	std::memcpy(res_lp_p, c.lp_p, PAGE_SZ);
	std::memcpy(res_rp_p, c.rp_p, PAGE_SZ);

	std::memcpy(c.src_r, init_src, PAGE_SZ);
	guard_fill(c.lp_r, PAGE_SZ);
	guard_fill(c.rp_r, PAGE_SZ);
	lp_r = (PAGE *)c.lp_r;
	rp_r = (PAGE *)c.rp_r;
	lp_r->lower = lp_r->upper = BTDATAOFF;
	rp_r->lower = rp_r->upper = BTDATAOFF;
	lp_r->flags = rp_r->flags = ptype;
	if (curs_init) {
		c.tr.bt_cursor.flags = CURS_INIT;
		c.tr.bt_cursor.pg.pgno = sr->pgno;
		c.tr.bt_cursor.pg.index = curs_idx;
	}
	skip_r = skip;
	restore_mock(snap);

	PAGE *ret_r = ref_bt_psplit(&c.tr, sr, lp_r, rp_r, &skip_r, ilen);

	std::memcpy(c.src_p, res_src_p, PAGE_SZ);
	std::memcpy(c.lp_p, res_lp_p, PAGE_SZ);
	std::memcpy(c.rp_p, res_rp_p, PAGE_SZ);
	c.tp.bt_cursor.flags = res_cur_p.flags;
	c.tp.bt_cursor.pg.pgno = res_cur_p.pg.pgno;
	c.tp.bt_cursor.pg.index = res_cur_p.pg.index;
	skip_p = res_skip_p;

	char msg[256];
	if ((ret_p == nullptr) != (ret_r == nullptr)) {
		check_eq(F_BT_PSPLIT, false, "null ret mismatch");
		return;
	}
	std::ptrdiff_t off_p = ret_p ? (char *)ret_p - (char *)c.lp_p : -1;
	std::ptrdiff_t off_r = ret_r ? (char *)ret_r - (char *)c.lp_r : -1;
	bool rp_is_l = (ret_p == (P::PAGE *)c.lp_p);
	bool rr_is_l = (ret_r == lp_r);
	std::snprintf(msg, sizeof(msg),
	    "ret side port=%d ref=%d type=0x%x skip=%u ilen=%zu",
	    rp_is_l, rr_is_l, ptype, (unsigned)skip, ilen);
	check_eq(F_BT_PSPLIT, rp_is_l == rr_is_l, msg);
	check_eq(F_BT_PSPLIT, skip_p == skip_r, "skip");
	check_eq(F_BT_PSPLIT, c.tp.bt_cursor.pg.pgno == c.tr.bt_cursor.pg.pgno,
	    "cursor pgno");
	check_eq(F_BT_PSPLIT, c.tp.bt_cursor.pg.index == c.tr.bt_cursor.pg.index,
	    "cursor index");
	check_eq(F_BT_PSPLIT, bufs_eq(c.lp_p, c.lp_r, PAGE_SZ), "left page");
	check_eq(F_BT_PSPLIT, bufs_eq(c.rp_p, c.rp_r, PAGE_SZ), "right page");
}

void check_bt_rroot(int l_is_rleaf, int r_is_rleaf, int nents_l, int nents_r)
{
	unsigned char root_p[PAGE_SZ];
	unsigned char root_r[PAGE_SZ];
	unsigned char lp_p[PAGE_SZ];
	unsigned char lp_r[PAGE_SZ];
	unsigned char rp_p[PAGE_SZ];
	unsigned char rp_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	recno_t nrecs_l[] = { 5, 15 };
	recno_t nrecs_r[] = { 7, 21 };
	pgno_t pgs_l[] = { 30, 31 };
	pgno_t pgs_r[] = { 40, 41 };
	u_int32_t rsizes_l[] = { 4, 6, 3, 5 };
	u_int32_t rsizes_r[] = { 3, 8, 2, 4 };
	u_char zf[] = { 0, 0, 0, 0 };

	test_mock_reset();
	init_tree(tp, mp_p, db_p, R_RECNO, PAGE_SZ);
	init_tree(tr, mp_r, db_r, R_RECNO, PAGE_SZ);
	guard_fill(root_p, PAGE_SZ);
	guard_fill(root_r, PAGE_SZ);
	alloc_page_buf(lp_p, PAGE_SZ);
	alloc_page_buf(lp_r, PAGE_SZ);
	alloc_page_buf(rp_p, PAGE_SZ);
	alloc_page_buf(rp_r, PAGE_SZ);

	PAGE *hp = (PAGE *)root_p;
	PAGE *hr = (PAGE *)root_r;
	hp->pgno = P_ROOT;
	hr->pgno = P_ROOT;
	hp->flags = P_RLEAF;
	hr->flags = P_RLEAF;

	PAGE *lp = (PAGE *)lp_p;
	PAGE *lr = (PAGE *)lp_r;
	PAGE *rp = (PAGE *)rp_p;
	PAGE *rr = (PAGE *)rp_r;
	lp->pgno = 2;
	lr->pgno = 2;
	rp->pgno = 3;
	rr->pgno = 3;
	if (l_is_rleaf)
		build_rleaf_page(lp, PAGE_SZ, nents_l, rsizes_l, zf);
	else
		build_rinternal_page(lp, PAGE_SZ, nents_l, nrecs_l, pgs_l,
		    P_RINTERNAL);
	if (r_is_rleaf)
		build_rleaf_page(rp, PAGE_SZ, nents_r, rsizes_r, zf);
	else
		build_rinternal_page(rp, PAGE_SZ, nents_r, nrecs_r, pgs_r,
		    P_RINTERNAL);
	std::memcpy(lp_r, lp_p, PAGE_SZ);
	std::memcpy(rp_r, rp_p, PAGE_SZ);

	int ret_p = P::bt_rroot(&tp, (P::PAGE *)hp, (P::PAGE *)lp, (P::PAGE *)rp);
	int ret_r = ref_bt_rroot(&tr, hr, lr, rr);
	char msg[128];
	std::snprintf(msg, sizeof(msg), "ret port=%d ref=%d", ret_p, ret_r);
	check_eq(F_BT_RROOT, ret_p == ret_r, msg);
	check_eq(F_BT_RROOT, bufs_eq(root_p, root_r, PAGE_SZ), "root page");
}

void check_bt_broot(u_int32_t root_type, int bigkey)
{
	unsigned char root_p[PAGE_SZ];
	unsigned char root_r[PAGE_SZ];
	unsigned char lp_p[PAGE_SZ];
	unsigned char lp_r[PAGE_SZ];
	unsigned char rp_p[PAGE_SZ];
	unsigned char rp_r[PAGE_SZ];
	unsigned char ov_p[PAGE_SZ];
	unsigned char ov_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	u_int32_t ks[] = { 4 };
	u_int32_t ds[] = { 4 };
	u_char ef[] = { (u_char)(bigkey ? P_BIGKEY : 0) };
	pgno_t pgs[] = { 50 };
	pgno_t ovpg = 99;

	test_mock_reset();
	init_tree(tp, mp_p, db_p, 0, PAGE_SZ);
	init_tree(tr, mp_r, db_r, 0, PAGE_SZ);
	guard_fill(root_p, PAGE_SZ);
	guard_fill(root_r, PAGE_SZ);
	alloc_page_buf(lp_p, PAGE_SZ);
	alloc_page_buf(lp_r, PAGE_SZ);
	alloc_page_buf(rp_p, PAGE_SZ);
	alloc_page_buf(rp_r, PAGE_SZ);
	alloc_page_buf(ov_p, PAGE_SZ);
	alloc_page_buf(ov_r, PAGE_SZ);
	if (bigkey) {
		test_mock_register(ovpg, ov_p);
		test_mock_register(ovpg, ov_r);
	}

	PAGE *hp = (PAGE *)root_p;
	PAGE *hr = (PAGE *)root_r;
	hp->pgno = P_ROOT;
	hr->pgno = P_ROOT;
	hp->flags = root_type;
	hr->flags = root_type;

	PAGE *lp = (PAGE *)lp_p;
	PAGE *lr = (PAGE *)lp_r;
	PAGE *rp = (PAGE *)rp_p;
	PAGE *rr = (PAGE *)rp_r;
	lp->pgno = 2;
	lr->pgno = 2;
	rp->pgno = 3;
	rr->pgno = 3;
	lp->lower = lp->upper = BTDATAOFF;
	lr->lower = lr->upper = BTDATAOFF;

	if (root_type == P_BLEAF) {
		if (bigkey) {
			ef[0] = P_BIGKEY;
			build_bleaf_page(rp, PAGE_SZ, 1, ks, ds, ef);
			unsigned char *blb = (unsigned char *)rp + rp->linp[0];
			std::memcpy(blb + 9, &ovpg, sizeof(ovpg));
		} else {
			build_bleaf_page(rp, PAGE_SZ, 1, ks, ds, ef);
		}
		std::memcpy(rr, rp_p, PAGE_SZ);
	} else {
		build_binternal_page(rp, PAGE_SZ, 1, ks, ef, pgs);
		std::memcpy(rr, rp_p, PAGE_SZ);
	}

	int ret_p = P::bt_broot(&tp, (P::PAGE *)hp, (P::PAGE *)lp, (P::PAGE *)rp);
	int ret_r = ref_bt_broot(&tr, hr, lr, rr);
	char msg[128];
	std::snprintf(msg, sizeof(msg), "ret port=%d ref=%d type=0x%x big=%d",
	    ret_p, ret_r, root_type, bigkey);
	check_eq(F_BT_BROOT, ret_p == ret_r, msg);
	check_eq(F_BT_BROOT, bufs_eq(root_p, root_r, PAGE_SZ), "root page");
	if (bigkey && ret_p == RET_SUCCESS)
		check_eq(F_BT_BROOT,
		    (((PAGE *)ov_p)->flags & P_PRESERVE) ==
			(((PAGE *)ov_r)->flags & P_PRESERVE),
		    "overflow preserve");
}

void check_bt_page(int sortsplit, int nextpg_fail, int nents, indx_t skip,
    size_t ilen)
{
	unsigned char src_p[PAGE_SZ];
	unsigned char src_r[PAGE_SZ];
	unsigned char next_p[PAGE_SZ];
	unsigned char next_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	u_int32_t ks[] = { 4, 6, 8, 5 };
	u_int32_t ds[] = { 3, 4, 2, 6 };
	u_char ef[] = { 0, 0, 0, 0 };
	pgno_t nextpg = 8;

	test_mock_reset();
	test_mock.get_force_null = nextpg_fail;
	init_tree(tp, mp_p, db_p, 0, PAGE_SZ);
	init_tree(tr, mp_r, db_r, 0, PAGE_SZ);
	alloc_page_buf(src_p, PAGE_SZ);
	alloc_page_buf(src_r, PAGE_SZ);
	alloc_page_buf(next_p, PAGE_SZ);
	alloc_page_buf(next_r, PAGE_SZ);

	PAGE *sp = (PAGE *)src_p;
	PAGE *sr = (PAGE *)src_r;
	sp->pgno = 4;
	sr->pgno = 4;
	build_bleaf_page(sp, PAGE_SZ, nents, ks, ds, ef);
	std::memcpy(src_r, src_p, PAGE_SZ);
	if (sortsplit) {
		sp->nextpg = P_INVALID;
		sr->nextpg = P_INVALID;
		skip = (indx_t)NEXTINDEX(sp);
	} else {
		sp->nextpg = nextpg;
		sr->nextpg = nextpg;
		((PAGE *)next_p)->pgno = nextpg;
		((PAGE *)next_r)->pgno = nextpg;
		test_mock_register(nextpg, next_p);
		test_mock_register(nextpg, next_r);
	}
	if (nextpg_fail)
		test_mock.get_force_null = 1;

	unsigned char init_src[PAGE_SZ];
	std::memcpy(init_src, src_p, PAGE_SZ);
	P::PAGE *lout_p = nullptr;
	P::PAGE *rout_p = nullptr;
	indx_t skip_p = skip;
	indx_t skip_r = skip;
	MockSnap snap = snap_mock();
	P::PAGE *ret_p = P::bt_page(&tp, (P::PAGE *)sp, &lout_p, &rout_p, &skip_p,
	    ilen);
	unsigned char res_src_p[PAGE_SZ];
	std::memcpy(res_src_p, src_p, PAGE_SZ);
	bool ret_is_l_p = (ret_p == lout_p);
	indx_t res_skip_p = skip_p;

	std::memcpy(src_r, init_src, PAGE_SZ);
	if (sortsplit) {
		sr->nextpg = P_INVALID;
		skip_r = (indx_t)NEXTINDEX(sr);
	} else {
		sr->nextpg = nextpg;
	}
	skip_r = skip;
	restore_mock(snap);

	PAGE *lout_r = nullptr;
	PAGE *rout_r = nullptr;
	PAGE *ret_r = ref_bt_page(&tr, sr, &lout_r, &rout_r, &skip_r, ilen);

	std::memcpy(src_p, res_src_p, PAGE_SZ);
	skip_p = res_skip_p;

	char msg[256];
	if ((ret_p == nullptr) != (ret_r == nullptr)) {
		check_eq(F_BT_PAGE, false, "null ret");
		return;
	}
	check_eq(F_BT_PAGE, skip_p == skip_r, "skip");
	if (ret_p) {
		bool rp_is_l = ret_is_l_p;
		bool rr_is_l = (ret_r == lout_r);
		std::snprintf(msg, sizeof(msg), "ret side port=%d ref=%d", rp_is_l,
		    rr_is_l);
		check_eq(F_BT_PAGE, rp_is_l == rr_is_l, msg);
		check_eq(F_BT_PAGE, bufs_eq(src_p, src_r, PAGE_SZ), "src page");
	}
}

void check_bt_root(int nents, indx_t skip, size_t ilen, int fail_second)
{
	unsigned char src_p[PAGE_SZ];
	unsigned char src_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	u_int32_t ks[] = { 5, 7, 4, 6 };
	u_int32_t ds[] = { 3, 2, 5, 4 };
	u_char ef[] = { 0, 0, 0, 0 };

	test_mock_reset();
	if (fail_second)
		test_mock.new_fail_after = 1;
	init_tree(tp, mp_p, db_p, 0, PAGE_SZ);
	init_tree(tr, mp_r, db_r, 0, PAGE_SZ);
	alloc_page_buf(src_p, PAGE_SZ);
	alloc_page_buf(src_r, PAGE_SZ);
	PAGE *sp = (PAGE *)src_p;
	PAGE *sr = (PAGE *)src_r;
	sp->pgno = P_ROOT;
	sr->pgno = P_ROOT;
	build_bleaf_page(sp, PAGE_SZ, nents, ks, ds, ef);
	std::memcpy(src_r, src_p, PAGE_SZ);

	unsigned char init_src[PAGE_SZ];
	std::memcpy(init_src, src_p, PAGE_SZ);
	P::PAGE *lp_p = nullptr;
	P::PAGE *rp_p = nullptr;
	indx_t skip_p = skip;
	indx_t skip_r = skip;
	MockSnap snap = snap_mock();
	P::PAGE *ret_p = P::bt_root(&tp, (P::PAGE *)sp, &lp_p, &rp_p, &skip_p,
	    ilen);
	unsigned char res_src_p[PAGE_SZ];
	std::memcpy(res_src_p, src_p, PAGE_SZ);
	indx_t res_skip_p = skip_p;
	int had_null_p = (ret_p == nullptr);

	std::memcpy(src_r, init_src, PAGE_SZ);
	skip_r = skip;
	restore_mock(snap);

	PAGE *lp_r = nullptr;
	PAGE *rp_r = nullptr;
	PAGE *ret_r = ref_bt_root(&tr, sr, &lp_r, &rp_r, &skip_r, ilen);

	std::memcpy(src_p, res_src_p, PAGE_SZ);
	skip_p = res_skip_p;

	char msg[128];
	if (had_null_p != (ret_r == nullptr)) {
		check_eq(F_BT_ROOT, false, "null ret");
		return;
	}
	check_eq(F_BT_ROOT, skip_p == skip_r, "skip");
	std::snprintf(msg, sizeof(msg), "new_calls=%u", test_mock.new_calls);
	check_eq(F_BT_ROOT, true, msg);
}

void check_bt_split_leaf_root(int nents, u_int32_t argskip, int use_recno,
    int with_pfx)
{
	unsigned char sp_p[PAGE_SZ];
	unsigned char sp_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	unsigned char keybuf[16];
	unsigned char databuf[16];
	P::DBT key_p, data_p;
	DBT key_r, data_r;
	u_int32_t ks[] = { 4, 6, 5, 7, 8, 3, 9, 2 };
	u_int32_t ds[] = { 3, 4, 2, 5, 6, 1, 7, 3 };
	u_char ef[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	test_mock_reset();
	init_tree(tp, mp_p, db_p, use_recno ? R_RECNO : 0, PAGE_SZ);
	init_tree(tr, mp_r, db_r, use_recno ? R_RECNO : 0, PAGE_SZ);
	if (with_pfx) {
		tp.bt_pfx = port_pfx;
		tr.bt_pfx = harness_pfx;
	}
	alloc_page_buf(sp_p, PAGE_SZ);
	alloc_page_buf(sp_r, PAGE_SZ);
	PAGE *spp = (PAGE *)sp_p;
	PAGE *spr = (PAGE *)sp_r;
	spp->pgno = P_ROOT;
	spr->pgno = P_ROOT;
	if (use_recno)
		build_rleaf_page(spp, PAGE_SZ, nents, ds, ef);
	else
		build_bleaf_page(spp, PAGE_SZ, nents, ks, ds, ef);
	std::memcpy(sp_r, sp_p, PAGE_SZ);

	key_p.data = keybuf;
	key_p.size = 4;
	data_p.data = databuf;
	data_p.size = 4;
	key_r.data = keybuf;
	key_r.size = 4;
	data_r.data = databuf;
	data_r.size = 4;
	std::memset(keybuf, 0xab, sizeof(keybuf));
	std::memset(databuf, 0xcd, sizeof(databuf));

	size_t ilen = use_recno ?
	    align_pg(sizeof(u_int32_t) + sizeof(u_char) + data_p.size) :
	    align_pg(sizeof(u_int32_t) + sizeof(u_int32_t) + sizeof(u_char) +
		key_p.size + data_p.size);

	unsigned char init_sp[PAGE_SZ];
	std::memcpy(init_sp, sp_p, PAGE_SZ);
	tp.bt_sp = tp.bt_stack;
	tr.bt_sp = tr.bt_stack;
	MockSnap snap = snap_mock();
	int rp = P::__bt_split(&tp, (P::PAGE *)spp, (P::DBT *)&key_p,
	    (P::DBT *)&data_p, 0, ilen, argskip);
	unsigned char res_sp_p[PAGE_SZ];
	std::memcpy(res_sp_p, sp_p, PAGE_SZ);

	std::memcpy(sp_r, init_sp, PAGE_SZ);
	tp.bt_sp = tp.bt_stack;
	tr.bt_sp = tr.bt_stack;
	restore_mock(snap);
	int rr = ref___bt_split(&tr, spr, &key_r, &data_r, 0, ilen, argskip);
	std::memcpy(sp_p, res_sp_p, PAGE_SZ);

	char msg[256];
	std::snprintf(msg, sizeof(msg),
	    "ret port=%d ref=%d recno=%d skip=%u panic_p=%u panic_r=%u",
	    rp, rr, use_recno, argskip, 0u, test_mock.panic_calls);
	check_eq(F_BT_SPLIT, rp == rr, msg);
	check_eq(F_BT_SPLIT, bufs_eq(sp_p, sp_r, PAGE_SZ), "split page");
}

void check_bt_split_with_parent(int parent_room, int child_type)
{
	unsigned char sp_p[PAGE_SZ];
	unsigned char sp_r[PAGE_SZ];
	unsigned char par_p[PAGE_SZ];
	unsigned char par_r[PAGE_SZ];
	unsigned char lc_p[PAGE_SZ];
	unsigned char lc_r[PAGE_SZ];
	unsigned char rc_p[PAGE_SZ];
	unsigned char rc_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	unsigned char keybuf[8];
	unsigned char databuf[8];
	P::DBT key_p, data_p;
	DBT key_r, data_r;
	u_int32_t ks[] = { 4, 5 };
	u_int32_t ds[] = { 4, 3 };
	u_char ef[] = { 0, 0 };
	pgno_t pgs[] = { 60, 61, 62, 63, 64, 65, 66, 67 };

	test_mock_reset();
	init_tree(tp, mp_p, db_p, 0, PAGE_SZ);
	init_tree(tr, mp_r, db_r, 0, PAGE_SZ);
	tp.bt_pfx = port_pfx;
	tr.bt_pfx = harness_pfx;

	alloc_page_buf(sp_p, PAGE_SZ);
	alloc_page_buf(sp_r, PAGE_SZ);
	alloc_page_buf(par_p, PAGE_SZ);
	alloc_page_buf(par_r, PAGE_SZ);
	alloc_page_buf(lc_p, PAGE_SZ);
	alloc_page_buf(lc_r, PAGE_SZ);
	alloc_page_buf(rc_p, PAGE_SZ);
	alloc_page_buf(rc_r, PAGE_SZ);

	PAGE *spp = (PAGE *)sp_p;
	PAGE *spr = (PAGE *)sp_r;
	spp->pgno = 4;
	spr->pgno = 4;
	build_bleaf_page(spp, PAGE_SZ, 2, ks, ds, ef);
	std::memcpy(sp_r, sp_p, PAGE_SZ);

	PAGE *parp = (PAGE *)par_p;
	PAGE *parr = (PAGE *)par_r;
	parp->pgno = 3;
	parr->pgno = 3;
	parp->flags = P_BINTERNAL;
	parr->flags = P_BINTERNAL;
	if (parent_room) {
		parp->lower = BTDATAOFF;
		parp->upper = (indx_t)(PAGE_SZ - 64);
		parr->lower = BTDATAOFF;
		parr->upper = (indx_t)(PAGE_SZ - 64);
	} else {
		u_int32_t ks8[] = { 4, 5, 6, 7, 8, 9, 10, 11 };
		u_char ef8[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		pgno_t pgs8[] = { 60, 61, 62, 63, 64, 65, 66, 67 };
		build_binternal_page(parp, PAGE_SZ, 8, ks8, ef8, pgs8);
		std::memcpy(par_r, par_p, PAGE_SZ);
	}

	test_mock_register(3, par_p);
	test_mock_register(3, par_r);

	key_p.data = keybuf;
	key_p.size = 4;
	data_p.data = databuf;
	data_p.size = 4;
	key_r.data = keybuf;
	key_r.size = 4;
	data_r.data = databuf;
	data_r.size = 4;
	std::memset(keybuf, 0x11, sizeof(keybuf));
	std::memset(databuf, 0x22, sizeof(databuf));

	size_t ilen = align_pg(sizeof(u_int32_t) + sizeof(u_int32_t) +
	    sizeof(u_char) + key_p.size + data_p.size);

	unsigned char init_sp[PAGE_SZ];
	std::memcpy(init_sp, sp_p, PAGE_SZ);
	tp.bt_sp = tp.bt_stack;
	tr.bt_sp = tr.bt_stack;
	tp.bt_sp->pgno = 3;
	tp.bt_sp->index = 0;
	++tp.bt_sp;
	tr.bt_sp->pgno = 3;
	tr.bt_sp->index = 0;
	++tr.bt_sp;
	MockSnap snap = snap_mock();
	int rp = P::__bt_split(&tp, (P::PAGE *)spp, (P::DBT *)&key_p,
	    (P::DBT *)&data_p, 0, ilen, 1);
	unsigned char res_sp_p[PAGE_SZ];
	std::memcpy(res_sp_p, sp_p, PAGE_SZ);

	std::memcpy(sp_r, init_sp, PAGE_SZ);
	tp.bt_sp = tp.bt_stack;
	tr.bt_sp = tr.bt_stack;
	tp.bt_sp->pgno = 3;
	tp.bt_sp->index = 0;
	++tp.bt_sp;
	tr.bt_sp->pgno = 3;
	tr.bt_sp->index = 0;
	++tr.bt_sp;
	restore_mock(snap);
	int rr = ref___bt_split(&tr, spr, &key_r, &data_r, 0, ilen, 1);
	std::memcpy(sp_p, res_sp_p, PAGE_SZ);

	char msg[128];
	std::snprintf(msg, sizeof(msg), "ret port=%d ref=%d room=%d", rp, rr,
	    parent_room);
	check_eq(F_BT_SPLIT, rp == rr, msg);
}

void test_rec_total_edges(void)
{
	recno_t z[] = { 0 };
	recno_t a[] = { 100 };
	recno_t b[] = { 1, 255 };
	recno_t c[] = { 0x7fffffff, 1 };
	recno_t d[] = { 10, 20, 30, 40 };
	check_rec_total(0, z);
	check_rec_total(1, a);
	check_rec_total(2, b);
	check_rec_total(2, c);
	check_rec_total(4, d);
}

void test_bt_preserve_edges(void)
{
	check_bt_preserve(0, 77);
	check_bt_preserve(0, 1);
	check_bt_preserve(1, 88);
}

void test_bt_psplit_edges(void)
{
	check_bt_psplit(P_BLEAF, 4, 0, 20, 0, 0);
	check_bt_psplit(P_BLEAF, 4, 2, 24, 1, 1);
	check_bt_psplit(P_BLEAF, 6, 3, 16, 1, 5);
	check_bt_psplit(P_BINTERNAL, 5, 1, 12, 0, 0);
	check_bt_psplit(P_BINTERNAL, 4, 4, 28, 1, 2);
	check_bt_psplit(P_RINTERNAL, 4, 0, 0, 0, 0);
	check_bt_psplit(P_RINTERNAL, 3, 2, 0, 1, 1);
	check_bt_psplit(P_RLEAF, 5, 0, 10, 0, 0);
	check_bt_psplit(P_RLEAF, 4, 3, 8, 1, 0);
	check_bt_psplit(P_BLEAF, 2, 1, 200, 0, 0);
}

void test_bt_rroot_edges(void)
{
	check_bt_rroot(1, 1, 2, 2);
	check_bt_rroot(0, 0, 2, 2);
	check_bt_rroot(1, 0, 1, 2);
	check_bt_rroot(0, 1, 2, 1);
}

void test_bt_broot_edges(void)
{
	check_bt_broot(P_BLEAF, 0);
	check_bt_broot(P_BLEAF, 1);
	check_bt_broot(P_BINTERNAL, 0);
}

void test_bt_page_edges(void)
{
	check_bt_page(1, 0, 2, 0, 20);
	check_bt_page(0, 0, 3, 1, 24);
	check_bt_page(0, 1, 3, 0, 16);
	check_bt_page(0, 0, 4, 2, 12);
}

void test_bt_root_edges(void)
{
	check_bt_root(3, 1, 20, 0);
	check_bt_root(4, 0, 16, 0);
	check_bt_root(2, 2, 24, 1);
}

void test_bt_split_edges(void)
{
	check_bt_split_leaf_root(2, 1, 0, 0);
	check_bt_split_leaf_root(3, 0, 0, 1);
	check_bt_split_leaf_root(2, 2, 1, 0);
	check_bt_split_with_parent(1, P_BLEAF);
	check_bt_split_with_parent(0, P_BLEAF);
}

void test_random_sweep(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++) {
		switch (i % 8u) {
		case 0: {
			recno_t nr[4];
			int n = (int)(i % 4u) + 1;
			for (int j = 0; j < n; j++)
				nr[j] = (recno_t)(nextr() & 0xffffu);
			check_rec_total(n, nr);
			break;
		}
		case 1:
			check_bt_preserve((int)(nextr() & 1u),
			    (pgno_t)(2 + (nextr() % 200u)));
			break;
		case 2: {
			u_int32_t pt = (nextr() & 3u) == 0 ? P_BLEAF :
			    (nextr() & 3u) == 1 ? P_BINTERNAL :
			    (nextr() & 3u) == 2 ? P_RINTERNAL : P_RLEAF;
			int n = (int)(i % 6u) + 2;
			indx_t sk = (indx_t)(nextr() % (unsigned)(n + 1));
			size_t il = (size_t)(nextr() % 32u);
			check_bt_psplit(pt, n, sk, il, (int)(nextr() & 1u),
			    (indx_t)(nextr() % (unsigned)n));
			break;
		}
		case 3:
			check_bt_rroot((int)(nextr() & 1u), (int)(nextr() & 1u),
			    (int)(i % 2u) + 1, (int)(i % 3u) + 1);
			break;
		case 4:
			check_bt_broot((nextr() & 1u) ? P_BINTERNAL : P_BLEAF,
			    (int)(nextr() & 1u));
			break;
		case 5:
			check_bt_page((int)(nextr() & 1u), (int)(nextr() & 1u),
			    (int)(i % 4u) + 2, (indx_t)(nextr() % 4u),
			    (size_t)(nextr() % 28u));
			break;
		case 6:
			check_bt_root((int)(i % 4u) + 2,
			    (indx_t)(nextr() % 4u), (size_t)(nextr() % 24u),
			    (int)(nextr() & 1u));
			break;
		case 7:
			check_bt_split_leaf_root((int)(i % 3u) + 2,
			    (u_int32_t)(nextr() % 4u), (int)(nextr() & 1u),
			    (int)(nextr() & 1u));
			if ((i & 0xffu) == 0)
				check_bt_split_with_parent((int)(nextr() & 1u),
				    P_BLEAF);
			break;
		default:
			break;
		}
	}
}

} // namespace

int main(void)
{
	check_bt_psplit(P_BLEAF, 4, 0, 20, 0, 0);
	return 0;
}
