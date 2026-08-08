/*
 * Differential harness for batch b0155: btree put/open/seq/delete routines.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

import pbsd.lib.libc.db.btree.b0155;

namespace P = pbsd::lib_libc_db_btree::b0155;

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

typedef struct {
	unsigned long flags;
	unsigned int cachesize;
	int maxkeypage;
	int minkeypage;
	unsigned int psize;
	int (*compare)(const DBT *, const DBT *);
	size_t (*prefix)(const DBT *, const DBT *);
	int lorder;
} BTREEINFO;

typedef struct _btmeta {
	u_int32_t magic;
	u_int32_t version;
	u_int32_t psize;
	u_int32_t free;
	u_int32_t nrecs;
	u_int32_t flags;
} BTMETA;

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
	unsigned open_calls, filter_calls;
	unsigned search_calls, split_calls, ovfl_put_calls, ovfl_del_calls;
	unsigned ret_calls, free_calls, cmp_calls;
	unsigned open_fd_calls, fstat_calls, read_calls, close_calls;
	unsigned mkostemp_calls, getenv_calls, sigmask_calls, unlink_calls;
	unsigned calloc_calls;
	int get_force_null, new_force_null, search_force_null;
	int split_ret, ovfl_put_ret, ovfl_del_ret, ret_status, free_ret;
	int open_ret, fstat_ret, read_ret, close_ret, mkostemp_ret;
	int calloc_fail_after;
	int search_exact, cmp_ret;
	int split_force_error;
	pgno_t get_last_pgno, new_pgno_seq;
	unsigned get_last_flags, last_put_flags;
	void *last_put_page;
	int nreg;
	pgno_t reg_pgno[64];
	void *reg_page[64];
	EPG search_epg;
	pgno_t ovfl_pgno;
	int new_fail_after;
	int delete_ret;
	char *getenv_val;
	struct stat fstat_sb;
	unsigned char read_buf[512];
	ssize_t read_ret_val;
	int read_errno_val;
	MPOOL *mpool_open_ret;
} test_mock_state;

extern test_mock_state test_mock;

void test_mock_reset(void);
void test_mock_register(pgno_t pgno, void *page);

int ref_byteorder(void);
int ref_tmp(void);
int ref_nroot(BTREE *);
int ref___bt_fd(const DB *);
void ref___bt_setcur(BTREE *, pgno_t, u_int);
int ref___bt_relink(BTREE *, PAGE *);
EPG *ref_bt_fast(BTREE *, const DBT *, const DBT *, int *);
int ref___bt_dleaf(BTREE *, const DBT *, PAGE *, u_int);
int ref___bt_curdel(BTREE *, const DBT *, PAGE *, u_int);
int ref___bt_pdelete(BTREE *, PAGE *);
int ref___bt_bdelete(BTREE *, const DBT *);
int ref___bt_stkacq(BTREE *, PAGE **, CURSOR *);
int ref___bt_delete(const DB *, const DBT *, u_int);
int ref___bt_put(const DB *, DBT *, const DBT *, u_int);
int ref___bt_seqset(BTREE *, EPG *, DBT *, int);
int ref___bt_seqadv(BTREE *, EPG *, int);
int ref___bt_first(BTREE *, const DBT *, EPG *, int *);
int ref___bt_seq(const DB *, DBT *, DBT *, u_int);
DB *ref___bt_open(const char *, int, int, const BTREEINFO *, int);

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

#define P_INVALID 0
#define P_ROOT 1
#define P_BINTERNAL 0x01
#define P_BLEAF 0x02
#define P_OVERFLOW 0x04
#define P_RINTERNAL 0x08
#define P_RLEAF 0x10
#define P_BIGKEY 0x02
#define P_BIGDATA 0x01
#define B_INMEM 0x00001
#define B_METADIRTY 0x00002
#define B_MODIFIED 0x00004
#define B_NEEDSWAP 0x00008
#define B_RDONLY 0x00010
#define B_NODUPS 0x00020
#define R_RECNO 0x00080
#define B_DB_LOCK 0x04000
#define CURS_ACQUIRE 0x01
#define CURS_AFTER 0x02
#define CURS_BEFORE 0x04
#define CURS_INIT 0x08
#define R_CURSOR 1
#define R_FIRST 3
#define R_LAST 6
#define R_NEXT 7
#define R_NOOVERWRITE 8
#define R_PREV 9
#define R_SETCURSOR 10
#define R_DUP 0x01
#define RET_ERROR -1
#define RET_SUCCESS 0
#define RET_SPECIAL 1
#define MPOOL_DIRTY 0x01
#define MPOOL_PAGE_NEXT 0x02
#define BTREEMAGIC 0x053162
#define BTREEVERSION 3
#define BIG_ENDIAN 4321
#define LITTLE_ENDIAN 1234
#define DB_LOCK 0x20000000

#define BTDATAOFF \
	(sizeof(pgno_t) + sizeof(pgno_t) + sizeof(pgno_t) + \
	    sizeof(u_int32_t) + sizeof(indx_t) + sizeof(indx_t))
#define NEXTINDEX(p) (((p)->lower - BTDATAOFF) / sizeof(indx_t))
#define LALIGN(n) (((n) + sizeof(pgno_t) - 1) & ~(sizeof(pgno_t) - 1))
#define NBLEAFDBT(ksize, dsize) \
	LALIGN(sizeof(u_int32_t) + sizeof(u_int32_t) + sizeof(u_char) + \
	    (ksize) + (dsize))

enum Fn {
	F_BYTEORDER,
	F_TMP,
	F_NROOT,
	F_BT_FD,
	F_BT_SETCUR,
	F_BT_RELINK,
	F_BT_FAST,
	F_BT_DLEAF,
	F_BT_CURDEL,
	F_BT_PDELETE,
	F_BT_BDELETE,
	F_BT_STKACQ,
	F_BT_DELETE,
	F_BT_PUT,
	F_BT_SEQSET,
	F_BT_SEQADV,
	F_BT_FIRST,
	F_BT_SEQ,
	F_BT_OPEN,
	F_COUNT
};

const char *fn_name[F_COUNT] = {
	"byteorder",
	"tmp",
	"nroot",
	"__bt_fd",
	"__bt_setcur",
	"__bt_relink",
	"bt_fast",
	"__bt_dleaf",
	"__bt_curdel",
	"__bt_pdelete",
	"__bt_bdelete",
	"__bt_stkacq",
	"__bt_delete",
	"__bt_put",
	"__bt_seqset",
	"__bt_seqadv",
	"__bt_first",
	"__bt_seq",
	"__bt_open",
};

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

u_char rnd_byte(void)
{
	return (u_char)(nextr() & 0xffu);
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

bool bufs_eq(const unsigned char *a, const unsigned char *b, size_t n)
{
	return std::memcmp(a, b, n) == 0;
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
	unsigned get, put, new_c, search, split, ovfl_put, ovfl_del, free_c, cmp;
	unsigned ret, delete_c, open_fd, fstat, read, close, mkostemp, calloc;
	unsigned getenv, sigmask;
};

MockDelta mock_delta(const test_mock_state &before, const test_mock_state &after)
{
	MockDelta d{};
	d.get = after.get_calls - before.get_calls;
	d.put = after.put_calls - before.put_calls;
	d.new_c = after.new_calls - before.new_calls;
	d.search = after.search_calls - before.search_calls;
	d.split = after.split_calls - before.split_calls;
	d.ovfl_put = after.ovfl_put_calls - before.ovfl_put_calls;
	d.ovfl_del = after.ovfl_del_calls - before.ovfl_del_calls;
	d.free_c = after.free_calls - before.free_calls;
	d.cmp = after.cmp_calls - before.cmp_calls;
	d.ret = after.ret_calls - before.ret_calls;
	d.delete_c = after.delete_calls - before.delete_calls;
	d.open_fd = after.open_fd_calls - before.open_fd_calls;
	d.fstat = after.fstat_calls - before.fstat_calls;
	d.read = after.read_calls - before.read_calls;
	d.close = after.close_calls - before.close_calls;
	d.mkostemp = after.mkostemp_calls - before.mkostemp_calls;
	d.calloc = after.calloc_calls - before.calloc_calls;
	d.getenv = after.getenv_calls - before.getenv_calls;
	d.sigmask = after.sigmask_calls - before.sigmask_calls;
	return d;
}

bool mock_delta_eq(const MockDelta &a, const MockDelta &b)
{
	return a.get == b.get && a.put == b.put && a.new_c == b.new_c &&
	    a.search == b.search && a.split == b.split &&
	    a.ovfl_put == b.ovfl_put && a.ovfl_del == b.ovfl_del &&
	    a.free_c == b.free_c && a.cmp == b.cmp && a.ret == b.ret &&
	    a.delete_c == b.delete_c && a.open_fd == b.open_fd &&
	    a.fstat == b.fstat && a.read == b.read && a.close == b.close &&
	    a.mkostemp == b.mkostemp && a.calloc == b.calloc &&
	    a.getenv == b.getenv && a.sigmask == b.sigmask;
}

bool open_mock_delta_eq(const MockDelta &a, const MockDelta &b)
{
	MockDelta ap = a;
	MockDelta bp = b;
	ap.calloc = bp.calloc = 0;
	return mock_delta_eq(ap, bp);
}

bool harness_cmp_btree(const P::BTREE &tp, const BTREE &tr)
{
	return tp.flags == tr.flags && tp.bt_order == tr.bt_order &&
	    tp.bt_free == tr.bt_free && tp.bt_last.pgno == tr.bt_last.pgno &&
	    tp.bt_last.index == tr.bt_last.index &&
	    tp.bt_cursor.pg.pgno == tr.bt_cursor.pg.pgno &&
	    tp.bt_cursor.pg.index == tr.bt_cursor.pg.index &&
	    tp.bt_cursor.flags == tr.bt_cursor.flags &&
	    (tp.bt_pinned == nullptr) == (tr.bt_pinned == nullptr) &&
	    (tp.bt_cursor.key.data == nullptr) ==
		(tr.bt_cursor.key.data == nullptr);
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
		size_t esz = LALIGN(sizeof(u_int32_t) + sizeof(pgno_t) +
		    sizeof(u_char) + ksz);
		off -= esz;
		unsigned char *e = (unsigned char *)pg + off;
		*(u_int32_t *)e = (u_int32_t)ksz;
		*(pgno_t *)(e + 4) = pgnos[i];
		e[8] = eflags[i];
		for (size_t k = 0; k < ksz; k++)
			e[9 + k] = (u_char)(0xa0 + k + (unsigned)i);
		pg->linp[i] = (indx_t)off;
	}
	pg->lower = (indx_t)(BTDATAOFF + nents * sizeof(indx_t));
	pg->upper = (indx_t)off;
}

void copy_cursor_ref(BTREE &tr, const P::BTREE &tp)
{
	tr.bt_cursor.pg.pgno = tp.bt_cursor.pg.pgno;
	tr.bt_cursor.pg.index = tp.bt_cursor.pg.index;
	tr.bt_cursor.flags = tp.bt_cursor.flags;
	tr.bt_cursor.rcursor = tp.bt_cursor.rcursor;
	tr.bt_cursor.key.data = tp.bt_cursor.key.data;
	tr.bt_cursor.key.size = tp.bt_cursor.key.size;
}

void setup_open_mocks_empty(int open_ret, int mkostemp_ret)
{
	test_mock_reset();
	test_mock.open_ret = open_ret;
	test_mock.mkostemp_ret = mkostemp_ret;
	test_mock.fstat_sb.st_blksize = 4096;
	test_mock.mpool_open_ret = (MPOOL *)0x1;
}

void setup_open_mocks_meta(u_int32_t meta_flags)
{
	BTMETA m{};
	m.magic = BTREEMAGIC;
	m.version = BTREEVERSION;
	m.psize = (u_int32_t)PAGE_SZ;
	m.free = 7;
	m.nrecs = 42;
	m.flags = meta_flags;
	test_mock.fstat_sb.st_size = sizeof(BTMETA);
	test_mock.read_ret_val = (ssize_t)sizeof(BTMETA);
	std::memcpy(test_mock.read_buf, &m, sizeof(m));
}

void free_open_db(P::DB *dbp)
{
	if (dbp == nullptr)
		return;
	P::BTREE *t = (P::BTREE *)dbp->internal;
	if (t != nullptr) {
		if (t->bt_cursor.key.data != nullptr)
			std::free(t->bt_cursor.key.data);
		std::free(t);
	}
	std::free(dbp);
}

void free_open_db_ref(DB *dbp)
{
	if (dbp == nullptr)
		return;
	BTREE *t = (BTREE *)dbp->internal;
	if (t != nullptr) {
		if (t->bt_cursor.key.data != nullptr)
			std::free(t->bt_cursor.key.data);
		std::free(t);
	}
	std::free(dbp);
}

void check_byteorder(void)
{
	int rp = P::byteorder();
	int rr = ref_byteorder();
	char msg[64];
	std::snprintf(msg, sizeof(msg), "ret port=%d ref=%d", rp, rr);
	check_eq(F_BYTEORDER, rp == rr, msg);
}

void check_tmp(int mkostemp_ret, const char *tmpdir)
{
	test_mock_reset();
	test_mock.mkostemp_ret = mkostemp_ret;
	test_mock.getenv_val = tmpdir ? (char *)tmpdir : nullptr;
	MockSnap snap = snap_mock();
	int rp = P::tmp();
	int re_p = errno;
	MockDelta dp = mock_delta(snap.mock, test_mock);
	restore_mock(snap);
	int rr = ref_tmp();
	int re_r = errno;
	MockDelta dr = mock_delta(snap.mock, test_mock);
	char msg[128];
	std::snprintf(msg, sizeof(msg), "ret port=%d ref=%d mk=%d env=%s errno_p=%d errno_r=%d",
	    rp, rr, mkostemp_ret, tmpdir ? tmpdir : "null", re_p, re_r);
	check_eq(F_TMP, rp == rr && re_p == re_r, msg);
	check_eq(F_TMP, dp.mkostemp == dr.mkostemp, "mkostemp delta");
	check_eq(F_TMP, dp.getenv == dr.getenv, "getenv delta");
}

void check_nroot(int existing_valid, int existing_invalid, int get_null,
    int new_meta_null, int new_root_null, pgno_t new_seq)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	unsigned char root_p[PAGE_SZ];
	unsigned char root_r[PAGE_SZ];

	test_mock_reset();
	test_mock.get_force_null = get_null;
	test_mock.new_force_null = new_meta_null;
	test_mock.new_pgno_seq = new_seq;
	init_tree_port(tp, mp_p, db_p, 0);
	init_tree_ref(tr, mp_r, db_r, 0);

	if (existing_valid || existing_invalid) {
		guard_fill(root_p, PAGE_SZ);
		guard_fill(root_r, PAGE_SZ);
		((PAGE *)root_p)->pgno = existing_invalid ? 0 : P_ROOT;
		((PAGE *)root_p)->lower = existing_invalid ? 0 :
		    (indx_t)(BTDATAOFF + sizeof(indx_t));
		((PAGE *)root_r)->pgno = ((PAGE *)root_p)->pgno;
		((PAGE *)root_r)->lower = ((PAGE *)root_p)->lower;
		if (existing_invalid)
			((PAGE *)root_p)->linp[0] = 0;
		std::memcpy(root_r, root_p, PAGE_SZ);
		test_mock_register(P_ROOT, root_p);
	}
	if (new_root_null)
		test_mock.new_fail_after = 1;

	unsigned char init_root[PAGE_SZ];
	if (existing_valid || existing_invalid)
		std::memcpy(init_root, root_p, PAGE_SZ);

	MockSnap before = snap_mock();
	int rp = P::nroot(&tp);
	int ep = errno;
	MockDelta dp = mock_delta(before.mock, test_mock);
	u_int32_t flags_p = tp.flags;
	if (existing_valid || existing_invalid)
		std::memcpy(root_p, init_root, PAGE_SZ);
	restore_mock(before);
	if (new_root_null)
		test_mock.new_fail_after = 1;
	int rr = ref_nroot(&tr);
	int er = errno;
	MockDelta dr = mock_delta(before.mock, test_mock);
	char msg[160];
	std::snprintf(msg, sizeof(msg),
	    "ret port=%d ref=%d valid=%d inv=%d null=%d errno_p=%d errno_r=%d",
	    rp, rr, existing_valid, existing_invalid, get_null, ep, er);
	check_eq(F_NROOT, rp == rr, msg);
	check_eq(F_NROOT, mock_delta_eq(dp, dr), "mock delta");
	if (rp == rr && rp == RET_SUCCESS)
		check_eq(F_NROOT, flags_p == tr.flags, "flags");
}

void check_bt_fd(int inmem, int pinned)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	unsigned char pin_p[PAGE_SZ];
	unsigned char pin_r[PAGE_SZ];

	test_mock_reset();
	init_tree_port(tp, mp_p, db_p, inmem ? B_INMEM : 0);
	init_tree_ref(tr, mp_r, db_r, inmem ? B_INMEM : 0);
	tp.bt_fd = 9;
	tr.bt_fd = 9;
	if (pinned) {
		tp.bt_pinned = (P::PAGE *)pin_p;
		tr.bt_pinned = (PAGE *)pin_r;
	}

	MockSnap snap = snap_mock();
	errno = 0;
	int rp = P::__bt_fd(&db_p);
	int ep = errno;
	unsigned put_p = test_mock.put_calls;
	restore_mock(snap);
	errno = 0;
	int rr = ref___bt_fd(&db_r);
	int er = errno;
	char msg[128];
	std::snprintf(msg, sizeof(msg), "ret port=%d ref=%d inmem=%d pinned=%d errno_p=%d errno_r=%d",
	    rp, rr, inmem, pinned, ep, er);
	check_eq(F_BT_FD, rp == rr && ep == er, msg);
	check_eq(F_BT_FD, test_mock.put_calls == put_p, "put on pinned");
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

void check_bt_relink(pgno_t pgno, pgno_t prevpg, pgno_t nextpg, int null_neighbor)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	unsigned char hp[PAGE_SZ];
	unsigned char hr[PAGE_SZ];
	unsigned char pp[PAGE_SZ];
	unsigned char pr[PAGE_SZ];
	unsigned char np[PAGE_SZ];
	unsigned char nr[PAGE_SZ];

	test_mock_reset();
	init_tree_port(tp, mp_p, db_p, 0);
	init_tree_ref(tr, mp_r, db_r, 0);
	guard_fill(hp, PAGE_SZ);
	guard_fill(hr, PAGE_SZ);
	guard_fill(pp, PAGE_SZ);
	guard_fill(pr, PAGE_SZ);
	guard_fill(np, PAGE_SZ);
	guard_fill(nr, PAGE_SZ);
	((PAGE *)hp)->pgno = pgno;
	((PAGE *)hr)->pgno = pgno;
	((PAGE *)hp)->prevpg = prevpg;
	((PAGE *)hr)->prevpg = prevpg;
	((PAGE *)hp)->nextpg = nextpg;
	((PAGE *)hr)->nextpg = nextpg;
	if (prevpg != P_INVALID) {
		((PAGE *)pp)->pgno = prevpg;
		test_mock_register(prevpg, pp);
	}
	if (nextpg != P_INVALID) {
		((PAGE *)np)->pgno = nextpg;
		test_mock_register(nextpg, np);
	}
	if (null_neighbor)
		test_mock.get_force_null = 1;

	unsigned char init_pp[PAGE_SZ];
	unsigned char init_np[PAGE_SZ];
	if (prevpg != P_INVALID)
		std::memcpy(init_pp, pp, PAGE_SZ);
	if (nextpg != P_INVALID)
		std::memcpy(init_np, np, PAGE_SZ);

	MockSnap snap = snap_mock();
	int rp = P::__bt_relink(&tp, (P::PAGE *)hp);
	MockDelta dp = mock_delta(snap.mock, test_mock);
	unsigned char res_pp[PAGE_SZ];
	unsigned char res_np[PAGE_SZ];
	if (prevpg != P_INVALID)
		std::memcpy(res_pp, pp, PAGE_SZ);
	if (nextpg != P_INVALID)
		std::memcpy(res_np, np, PAGE_SZ);
	if (prevpg != P_INVALID)
		std::memcpy(pp, init_pp, PAGE_SZ);
	if (nextpg != P_INVALID)
		std::memcpy(np, init_np, PAGE_SZ);
	restore_mock(snap);
	int rr = ref___bt_relink(&tr, (PAGE *)hr);
	MockDelta dr = mock_delta(snap.mock, test_mock);
	char msg[128];
	std::snprintf(msg, sizeof(msg), "ret port=%d ref=%d pg=%u", rp, rr,
	    (unsigned)pgno);
	check_eq(F_BT_RELINK, rp == rr, msg);
	check_eq(F_BT_RELINK, mock_delta_eq(dp, dr), "mock delta");
	if (rp == rr && rp == RET_SUCCESS && prevpg != P_INVALID && !null_neighbor)
		check_eq(F_BT_RELINK, bufs_eq(res_pp, pp, PAGE_SZ), "prev page");
	if (rp == rr && rp == RET_SUCCESS && nextpg != P_INVALID && !null_neighbor)
		check_eq(F_BT_RELINK, bufs_eq(res_np, np, PAGE_SZ), "next page");
}

void check_bt_fast(int order, int nents, indx_t last_idx, pgno_t last_pg,
    int cmp_ret, int get_null, size_t ksz, size_t dsz)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	unsigned char leaf_p[PAGE_SZ];
	unsigned char leaf_r[PAGE_SZ];
	unsigned char keybuf[32];
	unsigned char databuf[32];
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
	guard_fill(leaf_r, PAGE_SZ);
	build_bleaf_page((PAGE *)leaf_p, PAGE_SZ, nents, ksizes, dsizes, eflags);
	std::memcpy(leaf_r, leaf_p, PAGE_SZ);
	((PAGE *)leaf_p)->pgno = last_pg;
	((PAGE *)leaf_r)->pgno = last_pg;
	((PAGE *)leaf_p)->prevpg = order == BACK ? P_INVALID : 5;
	((PAGE *)leaf_r)->prevpg = ((PAGE *)leaf_p)->prevpg;
	((PAGE *)leaf_p)->nextpg = order == FORWARD ? P_INVALID : 6;
	((PAGE *)leaf_r)->nextpg = ((PAGE *)leaf_p)->nextpg;
	unsigned char init_leaf[PAGE_SZ];
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
	    "null port=%d ref=%d exact port=%d ref=%d order=%d cmp=%d",
	    ep_p == nullptr, ep_r == nullptr, exact_p, exact_r, order, cmp_ret);
	check_eq(F_BT_FAST, (ep_p == nullptr) == (ep_r == nullptr), msg);
	check_eq(F_BT_FAST, exact_p == exact_r, "exact");
	check_eq(F_BT_FAST, mock_delta_eq(dp, dr), "mock delta");
	if (ep_p && ep_r)
		check_eq(F_BT_FAST, ep_p->index == ep_r->index, "index");
	check_eq(F_BT_FAST, order_p == tr.bt_order, "bt_order");
}

void check_bt_dleaf(int nents, u_int idx, int cursor_hit, int ovfl_key,
    int ovfl_data, u_int32_t tflags)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	unsigned char leaf_p[PAGE_SZ];
	unsigned char leaf_r[PAGE_SZ];
	u_int32_t ksizes[4] = { 4, 5, 6, 7 };
	u_int32_t dsizes[4] = { 3, 4, 5, 6 };
	u_char eflags[4] = { 0, 0, 0, 0 };
	DBT key;

	test_mock_reset();
	init_tree_port(tp, mp_p, db_p, tflags);
	init_tree_ref(tr, mp_r, db_r, tflags);
	guard_fill(leaf_p, PAGE_SZ);
	guard_fill(leaf_r, PAGE_SZ);
	if (ovfl_key)
		eflags[idx < 4 ? idx : 0] = P_BIGKEY;
	if (ovfl_data)
		eflags[idx < 4 ? idx : 0] |= P_BIGDATA;
	build_bleaf_page((PAGE *)leaf_p, PAGE_SZ, nents, ksizes, dsizes, eflags);
	std::memcpy(leaf_r, leaf_p, PAGE_SZ);
	((PAGE *)leaf_p)->pgno = 8;
	((PAGE *)leaf_r)->pgno = 8;
	if (cursor_hit) {
		tp.bt_cursor.flags = CURS_INIT;
		tp.bt_cursor.pg.pgno = 8;
		tp.bt_cursor.pg.index = idx;
		tr.bt_cursor.flags = CURS_INIT;
		tr.bt_cursor.pg.pgno = 8;
		tr.bt_cursor.pg.index = idx;
	}
	key.data = (void *)"abcd";
	key.size = 4;

	MockSnap snap = snap_mock();
	int rp = P::__bt_dleaf(&tp, (P::DBT *)&key, (P::PAGE *)leaf_p, idx);
	P::BTREE tp_after = tp;
	unsigned char res_p[PAGE_SZ];
	std::memcpy(res_p, leaf_p, PAGE_SZ);
	MockDelta dp = mock_delta(snap.mock, test_mock);
	restore_mock(snap);
	int rr = ref___bt_dleaf(&tr, &key, (PAGE *)leaf_r, idx);
	MockDelta dr = mock_delta(snap.mock, test_mock);
	char msg[160];
	std::snprintf(msg, sizeof(msg), "ret port=%d ref=%d idx=%u nents=%d ovfl=%d/%d",
	    rp, rr, idx, nents, ovfl_key, ovfl_data);
	check_eq(F_BT_DLEAF, rp == rr, msg);
	check_eq(F_BT_DLEAF, mock_delta_eq(dp, dr), "mock delta");
	check_eq(F_BT_DLEAF, bufs_eq(res_p, leaf_r, PAGE_SZ), "pagebuf");
	check_eq(F_BT_DLEAF, harness_cmp_btree(tp_after, tr), "btree");
}

void check_bt_curdel(int nents, u_int idx, u_int32_t tflags, int cmp_same_prev,
    int cmp_same_next, int ret_fail)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	unsigned char leaf_p[PAGE_SZ];
	unsigned char leaf_r[PAGE_SZ];
	unsigned char prev_p[PAGE_SZ];
	unsigned char prev_r[PAGE_SZ];
	unsigned char next_p[PAGE_SZ];
	unsigned char next_r[PAGE_SZ];
	u_int32_t ksizes[3] = { 4, 4, 4 };
	u_int32_t dsizes[3] = { 2, 2, 2 };
	u_char eflags[3] = { 0, 0, 0 };
	DBT key;

	test_mock_reset();
	test_mock.ret_status = ret_fail ? RET_ERROR : RET_SUCCESS;
	init_tree_port(tp, mp_p, db_p, tflags);
	init_tree_ref(tr, mp_r, db_r, tflags);
	guard_fill(leaf_p, PAGE_SZ);
	guard_fill(leaf_r, PAGE_SZ);
	build_bleaf_page((PAGE *)leaf_p, PAGE_SZ, nents, ksizes, dsizes, eflags);
	std::memcpy(leaf_r, leaf_p, PAGE_SZ);
	((PAGE *)leaf_p)->pgno = 12;
	((PAGE *)leaf_r)->pgno = 12;
	if (idx == 0 || idx == (u_int)(nents - 1)) {
		guard_fill(prev_p, PAGE_SZ);
		guard_fill(prev_r, PAGE_SZ);
		guard_fill(next_p, PAGE_SZ);
		guard_fill(next_r, PAGE_SZ);
		build_bleaf_page((PAGE *)prev_p, PAGE_SZ, 1, ksizes, dsizes, eflags);
		std::memcpy(prev_r, prev_p, PAGE_SZ);
		build_bleaf_page((PAGE *)next_p, PAGE_SZ, 1, ksizes, dsizes, eflags);
		std::memcpy(next_r, next_p, PAGE_SZ);
		((PAGE *)prev_p)->pgno = 11;
		((PAGE *)prev_r)->pgno = 11;
		((PAGE *)next_p)->pgno = 13;
		((PAGE *)next_r)->pgno = 13;
		((PAGE *)leaf_p)->prevpg = 11;
		((PAGE *)leaf_r)->prevpg = 11;
		((PAGE *)leaf_p)->nextpg = 13;
		((PAGE *)leaf_r)->nextpg = 13;
		test_mock_register(11, prev_p);
		test_mock_register(11, prev_r);
		test_mock_register(13, next_p);
		test_mock_register(13, next_r);
	}
	if (cmp_same_prev)
		test_mock.cmp_ret = 0;
	else if (cmp_same_next)
		test_mock.cmp_ret = 0;
	else
		test_mock.cmp_ret = 1;
	key.data = nullptr;
	key.size = 0;

	MockSnap snap = snap_mock();
	int rp = P::__bt_curdel(&tp, nullptr, (P::PAGE *)leaf_p, idx);
	P::BTREE tp_after = tp;
	MockDelta dp = mock_delta(snap.mock, test_mock);
	restore_mock(snap);
	if (cmp_same_prev)
		test_mock.cmp_ret = 0;
	else if (cmp_same_next)
		test_mock.cmp_ret = 0;
	else
		test_mock.cmp_ret = 1;
	test_mock.ret_status = ret_fail ? RET_ERROR : RET_SUCCESS;
	int rr = ref___bt_curdel(&tr, nullptr, (PAGE *)leaf_r, idx);
	MockDelta dr = mock_delta(snap.mock, test_mock);
	char msg[160];
	std::snprintf(msg, sizeof(msg), "ret port=%d ref=%d idx=%u nodups=%d fail=%d",
	    rp, rr, idx, (tflags & B_NODUPS) != 0, ret_fail);
	check_eq(F_BT_CURDEL, rp == rr, msg);
	check_eq(F_BT_CURDEL, mock_delta_eq(dp, dr), "mock delta");
	check_eq(F_BT_CURDEL, harness_cmp_btree(tp_after, tr), "btree");
}

void check_bt_pdelete(int nents, int stack_depth, int root_page, int ovfl_parent)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	unsigned char leaf_p[PAGE_SZ];
	unsigned char leaf_r[PAGE_SZ];
	unsigned char par_p[PAGE_SZ];
	unsigned char par_r[PAGE_SZ];
	u_int32_t ksizes[4] = { 4, 5, 6, 7 };
	u_char eflags[4] = { 0, 0, 0, 0 };
	pgno_t pgnos[4] = { 20, 21, 22, 23 };

	test_mock_reset();
	init_tree_port(tp, mp_p, db_p, 0);
	init_tree_ref(tr, mp_r, db_r, 0);
	tp.bt_sp = tp.bt_stack;
	tr.bt_sp = tr.bt_stack;
	guard_fill(leaf_p, PAGE_SZ);
	guard_fill(leaf_r, PAGE_SZ);
	guard_fill(par_p, PAGE_SZ);
	guard_fill(par_r, PAGE_SZ);
	if (root_page) {
		build_bleaf_page((PAGE *)leaf_p, PAGE_SZ, 1, ksizes, ksizes, eflags);
		((PAGE *)leaf_p)->pgno = P_ROOT;
	} else {
		build_bleaf_page((PAGE *)leaf_p, PAGE_SZ, nents, ksizes, ksizes, eflags);
		((PAGE *)leaf_p)->pgno = 30;
	}
	std::memcpy(leaf_r, leaf_p, PAGE_SZ);
	if (stack_depth > 0) {
		if (ovfl_parent)
			eflags[0] = P_BIGKEY;
		build_binternal_page((PAGE *)par_p, PAGE_SZ, stack_depth > 1 ? 2 : 1,
		    ksizes, eflags, pgnos);
		std::memcpy(par_r, par_p, PAGE_SZ);
		((PAGE *)par_p)->pgno = 3;
		((PAGE *)par_r)->pgno = 3;
		test_mock_register(3, par_p);
		test_mock_register(3, par_r);
		tp.bt_sp->pgno = 3;
		tp.bt_sp->index = 0;
		++tp.bt_sp;
		tr.bt_sp->pgno = 3;
		tr.bt_sp->index = 0;
		++tr.bt_sp;
	}

	MockSnap snap = snap_mock();
	int rp = P::__bt_pdelete(&tp, (P::PAGE *)leaf_p);
	unsigned char res_leaf_p[PAGE_SZ];
	std::memcpy(res_leaf_p, leaf_p, PAGE_SZ);
	MockDelta dp = mock_delta(snap.mock, test_mock);
	restore_mock(snap);
	tr.bt_sp = tr.bt_stack;
	if (stack_depth > 0) {
		tr.bt_sp->pgno = 3;
		tr.bt_sp->index = 0;
		++tr.bt_sp;
	}
	int rr = ref___bt_pdelete(&tr, (PAGE *)leaf_r);
	MockDelta dr = mock_delta(snap.mock, test_mock);
	char msg[128];
	std::snprintf(msg, sizeof(msg), "ret port=%d ref=%d root=%d depth=%d",
	    rp, rr, root_page, stack_depth);
	check_eq(F_BT_PDELETE, rp == rr, msg);
	check_eq(F_BT_PDELETE, mock_delta_eq(dp, dr), "mock delta");
	if (root_page && rp == rr && rp == RET_SUCCESS)
		check_eq(F_BT_PDELETE, bufs_eq(res_leaf_p, leaf_r, PAGE_SZ), "root leaf");
}

void check_bt_bdelete(int exact, u_int32_t tflags, int search_null, int dleaf_fail)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	unsigned char leaf_p[PAGE_SZ];
	unsigned char leaf_r[PAGE_SZ];
	DBT key;
	u_int32_t ksizes[3] = { 4, 5, 6 };
	u_int32_t dsizes[3] = { 2, 3, 4 };
	u_char eflags[3] = { 0, 0, 0 };

	test_mock_reset();
	test_mock.search_force_null = search_null;
	test_mock.search_exact = exact;
	test_mock.search_epg.page = (PAGE *)leaf_p;
	test_mock.search_epg.index = 1;
	test_mock.cmp_ret = 0;
	init_tree_port(tp, mp_p, db_p, tflags);
	init_tree_ref(tr, mp_r, db_r, tflags);
	guard_fill(leaf_p, PAGE_SZ);
	guard_fill(leaf_r, PAGE_SZ);
	build_bleaf_page((PAGE *)leaf_p, PAGE_SZ, 3, ksizes, dsizes, eflags);
	std::memcpy(leaf_r, leaf_p, PAGE_SZ);
	((PAGE *)leaf_p)->pgno = 8;
	((PAGE *)leaf_r)->pgno = 8;
	test_mock.search_epg.page = (PAGE *)leaf_p;
	key.data = (void *)"key";
	key.size = 3;
	if (dleaf_fail)
		test_mock.ovfl_del_ret = RET_ERROR;

	MockSnap snap = snap_mock();
	int rp = P::__bt_bdelete(&tp, (P::DBT *)&key);
	u_int32_t flags_p = tp.flags;
	MockDelta dp = mock_delta(snap.mock, test_mock);
	restore_mock(snap);
	test_mock.search_epg.page = (PAGE *)leaf_r;
	if (dleaf_fail)
		test_mock.ovfl_del_ret = RET_ERROR;
	int rr = ref___bt_bdelete(&tr, &key);
	MockDelta dr = mock_delta(snap.mock, test_mock);
	char msg[160];
	std::snprintf(msg, sizeof(msg),
	    "ret port=%d ref=%d exact=%d null=%d nodups=%d fail=%d",
	    rp, rr, exact, search_null, (tflags & B_NODUPS) != 0, dleaf_fail);
	check_eq(F_BT_BDELETE, rp == rr, msg);
	check_eq(F_BT_BDELETE, mock_delta_eq(dp, dr), "mock delta");
	if (rp == rr && rp == RET_SUCCESS)
		check_eq(F_BT_BDELETE, flags_p == tr.flags, "flags");
}

void check_bt_stkacq(int target_pg, int search_null)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	unsigned char leaf_p[PAGE_SZ];
	unsigned char leaf_r[PAGE_SZ];
	unsigned char srch_p[PAGE_SZ];
	unsigned char srch_r[PAGE_SZ];
	char keybuf[16];

	test_mock_reset();
	test_mock.search_force_null = search_null;
	test_mock.search_exact = 1;
	init_tree_port(tp, mp_p, db_p, 0);
	init_tree_ref(tr, mp_r, db_r, 0);
	guard_fill(leaf_p, PAGE_SZ);
	guard_fill(leaf_r, PAGE_SZ);
	guard_fill(srch_p, PAGE_SZ);
	guard_fill(srch_r, PAGE_SZ);
	u_int32_t ks[1] = { 4 };
	u_int32_t ds[1] = { 2 };
	u_char ef[1] = { 0 };
	build_bleaf_page((PAGE *)leaf_p, PAGE_SZ, 1, ks, ds, ef);
	std::memcpy(leaf_r, leaf_p, PAGE_SZ);
	build_bleaf_page((PAGE *)srch_p, PAGE_SZ, 1, ks, ds, ef);
	std::memcpy(srch_r, srch_p, PAGE_SZ);
	((PAGE *)leaf_p)->pgno = (pgno_t)target_pg;
	((PAGE *)leaf_r)->pgno = (pgno_t)target_pg;
	((PAGE *)srch_p)->pgno = (pgno_t)target_pg;
	((PAGE *)srch_r)->pgno = (pgno_t)target_pg;
	test_mock.search_epg.page = (PAGE *)srch_p;
	test_mock.search_epg.index = 0;
	test_mock_register((pgno_t)target_pg, leaf_p);
	tp.bt_cursor.flags = CURS_INIT;
	tp.bt_cursor.pg.pgno = (pgno_t)target_pg;
	tp.bt_cursor.pg.index = 0;
	tp.bt_cursor.key.data = keybuf;
	tp.bt_cursor.key.size = 4;
	tr.bt_cursor.flags = CURS_INIT;
	tr.bt_cursor.pg.pgno = (pgno_t)target_pg;
	tr.bt_cursor.pg.index = 0;
	tr.bt_cursor.key.data = keybuf;
	tr.bt_cursor.key.size = 4;

	P::PAGE *hp_p = (P::PAGE *)leaf_p;
	PAGE *hp_r = (PAGE *)leaf_r;
	MockSnap snap = snap_mock();
	int rp = P::__bt_stkacq(&tp, &hp_p, &tp.bt_cursor);
	MockDelta dp = mock_delta(snap.mock, test_mock);
	restore_mock(snap);
	hp_r = (PAGE *)leaf_r;
	int rr = ref___bt_stkacq(&tr, &hp_r, &tr.bt_cursor);
	MockDelta dr = mock_delta(snap.mock, test_mock);
	char msg[128];
	std::snprintf(msg, sizeof(msg), "ret port=%d ref=%d pg=%d null=%d",
	    rp, rr, target_pg, search_null);
	check_eq(F_BT_STKACQ, rp == rr, msg);
	check_eq(F_BT_STKACQ, mock_delta_eq(dp, dr), "mock delta");
}

void check_bt_delete(u_int flags, u_int32_t tflags, int curs_init, int curs_acquire,
    int search_null, int one_entry)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	unsigned char leaf_p[PAGE_SZ];
	unsigned char leaf_r[PAGE_SZ];
	DBT key;
	u_int32_t ksizes[2] = { 4, 5 };
	u_int32_t dsizes[2] = { 2, 3 };
	u_char eflags[2] = { 0, 0 };

	test_mock_reset();
	test_mock.search_force_null = search_null;
	test_mock.search_exact = 1;
	init_tree_port(tp, mp_p, db_p, tflags);
	init_tree_ref(tr, mp_r, db_r, tflags);
	guard_fill(leaf_p, PAGE_SZ);
	guard_fill(leaf_r, PAGE_SZ);
	build_bleaf_page((PAGE *)leaf_p, PAGE_SZ, one_entry ? 1 : 2, ksizes, dsizes,
	    eflags);
	std::memcpy(leaf_r, leaf_p, PAGE_SZ);
	((PAGE *)leaf_p)->pgno = 8;
	((PAGE *)leaf_r)->pgno = 8;
	unsigned char init_leaf[PAGE_SZ];
	std::memcpy(init_leaf, leaf_p, PAGE_SZ);
	test_mock.search_epg.page = (PAGE *)leaf_p;
	test_mock.search_epg.index = 0;
	test_mock_register(8, leaf_p);
	if (curs_init) {
		unsigned char ckey[8] = { 0x80, 0x81, 0x82, 0x83 };
		tp.bt_cursor.flags = CURS_INIT |
		    (curs_acquire ? CURS_ACQUIRE : 0);
		tp.bt_cursor.pg.pgno = 8;
		tp.bt_cursor.pg.index = 0;
		tp.bt_cursor.key.data = ckey;
		tp.bt_cursor.key.size = 4;
		tr.bt_cursor.flags = tp.bt_cursor.flags;
		tr.bt_cursor.pg.pgno = 8;
		tr.bt_cursor.pg.index = 0;
		tr.bt_cursor.key.data = ckey;
		tr.bt_cursor.key.size = 4;
	}
	key.data = (void *)"k";
	key.size = 1;

	MockSnap snap = snap_mock();
	errno = 0;
	int rp = P::__bt_delete(&db_p, (P::DBT *)&key, flags);
	int ep = errno;
	u_int32_t flags_p = tp.flags;
	MockDelta dp = mock_delta(snap.mock, test_mock);
	std::memcpy(leaf_p, init_leaf, PAGE_SZ);
	if (curs_init) {
		tp.bt_cursor.flags = CURS_INIT |
		    (curs_acquire ? CURS_ACQUIRE : 0);
		tp.bt_cursor.pg.pgno = 8;
		tp.bt_cursor.pg.index = 0;
	}
	restore_mock(snap);
	errno = 0;
	int rr = ref___bt_delete(&db_r, &key, flags);
	int er = errno;
	MockDelta dr = mock_delta(snap.mock, test_mock);
	char msg[160];
	std::snprintf(msg, sizeof(msg),
	    "ret port=%d ref=%d fl=%u rdonly=%d curs=%d errno_p=%d errno_r=%d",
	    rp, rr, flags, (tflags & B_RDONLY) != 0, curs_init, ep, er);
	check_eq(F_BT_DELETE, rp == rr && ep == er, msg);
	check_eq(F_BT_DELETE, mock_delta_eq(dp, dr), "mock delta");
	if (rp == rr && rp == RET_SUCCESS)
		check_eq(F_BT_DELETE, flags_p == tr.flags, "B_MODIFIED");
}

void check_bt_put(u_int flags, u_int32_t tflags, int search_null, int exact,
    int split, int ovfl, int room)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	unsigned char leaf_p[PAGE_SZ];
	unsigned char leaf_r[PAGE_SZ];
	unsigned char keybuf[64];
	unsigned char databuf[64];
	DBT key, data;
	u_int32_t ksizes[2] = { 4, 5 };
	u_int32_t dsizes[2] = { 3, 4 };
	u_char eflags[2] = { 0, 0 };

	test_mock_reset();
	test_mock.search_force_null = search_null;
	test_mock.search_exact = exact;
	test_mock.search_epg.page = (PAGE *)leaf_p;
	test_mock.search_epg.index = 0;
	if (split)
		test_mock.split_ret = RET_SUCCESS;
	if (ovfl)
		test_mock.ovfl_put_ret = RET_SUCCESS;
	init_tree_port(tp, mp_p, db_p, tflags);
	init_tree_ref(tr, mp_r, db_r, tflags);
	guard_fill(leaf_p, PAGE_SZ);
	guard_fill(leaf_r, PAGE_SZ);
	build_bleaf_page((PAGE *)leaf_p, PAGE_SZ, 2, ksizes, dsizes, eflags);
	std::memcpy(leaf_r, leaf_p, PAGE_SZ);
	((PAGE *)leaf_p)->pgno = 8;
	((PAGE *)leaf_r)->pgno = 8;
	if (!room)
		((PAGE *)leaf_p)->upper = (indx_t)(((PAGE *)leaf_p)->lower + 8);
	if (!room)
		((PAGE *)leaf_r)->upper = (indx_t)(((PAGE *)leaf_r)->lower + 8);
	test_mock.search_epg.page = (PAGE *)leaf_p;
	key.data = keybuf;
	key.size = ovfl ? tp.bt_ovflsize + 1 : 4;
	data.data = databuf;
	data.size = ovfl ? tp.bt_ovflsize + 1 : 4;
	std::memset(keybuf, 0xab, sizeof(keybuf));
	std::memset(databuf, 0xcd, sizeof(databuf));

	MockSnap snap = snap_mock();
	errno = 0;
	int rp = P::__bt_put(&db_p, (P::DBT *)&key, (P::DBT *)&data, flags);
	int ep = errno;
	u_int32_t flags_p = tp.flags;
	MockDelta dp = mock_delta(snap.mock, test_mock);
	restore_mock(snap);
	errno = 0;
	test_mock.search_epg.page = (PAGE *)leaf_r;
	int rr = ref___bt_put(&db_r, &key, &data, flags);
	int er = errno;
	MockDelta dr = mock_delta(snap.mock, test_mock);
	char msg[200];
	std::snprintf(msg, sizeof(msg),
	    "ret port=%d ref=%d fl=%u exact=%d split=%d ovfl=%d room=%d errno_p=%d errno_r=%d",
	    rp, rr, flags, exact, split, ovfl, room, ep, er);
	check_eq(F_BT_PUT, rp == rr && ep == er, msg);
	check_eq(F_BT_PUT, mock_delta_eq(dp, dr), "mock delta");
	if (rp == rr && rp == RET_SUCCESS)
		check_eq(F_BT_PUT, flags_p == tr.flags, "flags");
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
	unsigned char root_p[PAGE_SZ];
	unsigned char root_r[PAGE_SZ];
	unsigned char leaf_p[PAGE_SZ];
	unsigned char leaf_r[PAGE_SZ];
	P::EPG ep_p;
	EPG ep_r;
	DBT key;
	unsigned char keybuf[8];

	test_mock_reset();
	test_mock.get_force_null = get_null;
	init_tree_port(tp, mp_p, db_p, 0);
	init_tree_ref(tr, mp_r, db_r, 0);
	setup_tree_for_seq(root_p, leaf_p, empty, internal);
	std::memcpy(root_r, root_p, PAGE_SZ);
	std::memcpy(leaf_r, leaf_p, PAGE_SZ);
	unsigned char init_root[PAGE_SZ];
	unsigned char init_leaf[PAGE_SZ];
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
	unsigned char leaf_p[PAGE_SZ];
	unsigned char leaf_r[PAGE_SZ];
	unsigned char next_p[PAGE_SZ];
	unsigned char next_r[PAGE_SZ];
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
	((PAGE *)leaf_p)->nextpg = nextpg;
	((PAGE *)leaf_r)->nextpg = nextpg;
	unsigned char init_leaf[PAGE_SZ];
	unsigned char init_next[PAGE_SZ];
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
	unsigned char ckey[8] = { 0x80, 0x81, 0x82, 0x83 };
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
	copy_cursor_ref(tr, tp);
	if (curs_acquire) {
		tr.bt_cursor.key.data = ckey;
		tr.bt_cursor.key.size = 4;
	}
	test_mock.search_epg.page = (PAGE *)leaf_p;
	test_mock.search_epg.index = 0;
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
	unsigned char leaf_p[PAGE_SZ];
	unsigned char leaf_r[PAGE_SZ];
	P::EPG ep_p;
	EPG ep_r;
	DBT key;
	unsigned char keybuf[8];
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
    int ret_fail)
{
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	unsigned char root_p[PAGE_SZ];
	unsigned char root_r[PAGE_SZ];
	unsigned char keybuf[32];
	unsigned char databuf[32];
	DBT key, data;

	test_mock_reset();
	test_mock.get_force_null = get_null;
	test_mock.ret_status = ret_fail ? RET_ERROR : RET_SUCCESS;
	init_tree_port(tp, mp_p, db_p, 0);
	init_tree_ref(tr, mp_r, db_r, 0);
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
	    "ret port=%d ref=%d fl=%u init=%d empty=%d fail=%d errno_p=%d errno_r=%d",
	    rp, rr, flags, curs_init, empty, ret_fail, ep, er);
	check_eq(F_BT_SEQ, rp == rr && ep == er, msg);
	check_eq(F_BT_SEQ, mock_delta_eq(dp, dr), "mock delta");
	check_eq(F_BT_SEQ, harness_cmp_btree(tp, tr), "btree");
}

void check_bt_open(int inmem, int invalid_psize, int existing_meta,
    int open_fail, int calloc_fail, int mpool_null)
{
	P::BTREEINFO pinfo{};
	BTREEINFO rinfo{};
	test_mock_reset();
	test_mock.open_ret = open_fail ? -1 : 3;
	test_mock.mkostemp_ret = open_fail ? -1 : 4;
	test_mock.calloc_fail_after = calloc_fail;
	test_mock.mpool_open_ret = mpool_null ? nullptr : (MPOOL *)0x1;
	test_mock.fstat_sb.st_blksize = 4096;
	if (existing_meta)
		setup_open_mocks_meta(B_NODUPS);
	if (invalid_psize) {
		pinfo.psize = 511;
		pinfo.lorder = LITTLE_ENDIAN;
		rinfo.psize = 511;
		rinfo.lorder = LITTLE_ENDIAN;
	}
	if (existing_meta) {
		pinfo.lorder = LITTLE_ENDIAN;
		rinfo.lorder = LITTLE_ENDIAN;
	}
	if (!inmem && !invalid_psize && !existing_meta)
		test_mock.fstat_sb.st_size = 0;

	MockSnap snap = snap_mock();
	errno = 0;
	P::DB *dp_p = P::__bt_open(inmem ? nullptr : "/tmp/x.db",
	    O_RDWR, 0644,
	    invalid_psize || existing_meta ? &pinfo : nullptr,
	    inmem ? 0 : DB_LOCK);
	int ep = errno;
	MockDelta dport = mock_delta(snap.mock, test_mock);
	P::BTREE *tp = dp_p ? (P::BTREE *)dp_p->internal : nullptr;
	u_int32_t flags_p = tp ? tp->flags : 0;
	u_int32_t psize_p = tp ? tp->bt_psize : 0;
	int fd_p = tp ? tp->bt_fd : -99;
	restore_mock(snap);

	test_mock.open_ret = open_fail ? -1 : 3;
	test_mock.mkostemp_ret = open_fail ? -1 : 4;
	test_mock.calloc_fail_after = calloc_fail;
	test_mock.mpool_open_ret = mpool_null ? nullptr : (MPOOL *)0x1;
	test_mock.fstat_sb.st_blksize = 4096;
	if (existing_meta)
		setup_open_mocks_meta(B_NODUPS);
	if (!inmem && !invalid_psize && !existing_meta)
		test_mock.fstat_sb.st_size = 0;

	errno = 0;
	DB *dp_r = ref___bt_open(inmem ? nullptr : "/tmp/x.db", O_RDWR, 0644,
	    invalid_psize || existing_meta ? &rinfo : nullptr,
	    inmem ? 0 : DB_LOCK);
	int er = errno;
	MockDelta dref = mock_delta(snap.mock, test_mock);
	BTREE *tr = dp_r ? (BTREE *)dp_r->internal : nullptr;

	char msg[200];
	std::snprintf(msg, sizeof(msg),
	    "null port=%d ref=%d inmem=%d inv=%d meta=%d fail=%d errno_p=%d errno_r=%d",
	    dp_p == nullptr, dp_r == nullptr, inmem, invalid_psize,
	    existing_meta, open_fail || calloc_fail || mpool_null, ep, er);
	check_eq(F_BT_OPEN, (dp_p == nullptr) == (dp_r == nullptr) && ep == er, msg);
	check_eq(F_BT_OPEN, mock_delta_eq(dport, dref), "mock delta");
	if (dp_p && dp_r) {
		check_eq(F_BT_OPEN, (int)dp_p->type == (int)dp_r->type, "type");
		check_eq(F_BT_OPEN, flags_p == tr->flags, "flags");
		check_eq(F_BT_OPEN, psize_p == tr->bt_psize, "psize");
		check_eq(F_BT_OPEN, (fd_p < 0) == (tr->bt_fd < 0), "fd sign");
	}
	free_open_db(dp_p);
	free_open_db_ref(dp_r);
}

void test_byteorder_edges(void) { check_byteorder(); }

void test_tmp_edges(void)
{
	check_tmp(4, nullptr);
	check_tmp(4, "/tmp");
	check_tmp(-1, "/tmp");
}

void test_nroot_edges(void)
{
	check_nroot(1, 0, 0, 0, 0, P_ROOT);
	check_nroot(0, 1, 0, 0, 0, P_ROOT);
	check_nroot(0, 0, 1, 0, 0, P_ROOT);
	check_nroot(0, 0, 0, 1, 0, P_ROOT);
	check_nroot(0, 0, 0, 0, 1, 99);
}

void test_bt_fd_edges(void)
{
	check_bt_fd(0, 0);
	check_bt_fd(1, 0);
	check_bt_fd(0, 1);
}

void test_bt_setcur_edges(void)
{
	check_bt_setcur(0, 5, 0);
	check_bt_setcur(1, 7, 2);
	check_bt_setcur(0, P_ROOT, 1);
}

void test_bt_relink_edges(void)
{
	check_bt_relink(20, 19, 21, 0);
	check_bt_relink(20, P_INVALID, 21, 0);
	check_bt_relink(20, 19, P_INVALID, 0);
	check_bt_relink(20, 19, 21, 1);
}

void test_bt_fast_edges(void)
{
	check_bt_fast(FORWARD, 4, 3, 8, 0, 0, 4, 4);
	check_bt_fast(FORWARD, 4, 3, 8, 1, 0, 4, 4);
	check_bt_fast(BACK, 4, 0, 8, 0, 0, 4, 4);
	check_bt_fast(BACK, 4, 0, 8, -1, 0, 4, 4);
	check_bt_fast(FORWARD, 4, 3, 8, 0, 1, 4, 4);
	check_bt_fast(FORWARD, 2, 1, 8, 0, 0, 200, 4);
}

void test_bt_dleaf_edges(void)
{
	check_bt_dleaf(4, 1, 0, 0, 0, 0);
	check_bt_dleaf(4, 1, 1, 0, 0, 0);
	check_bt_dleaf(3, 0, 0, 1, 0, 0);
	check_bt_dleaf(3, 2, 0, 0, 1, 0);
}

void test_bt_curdel_edges(void)
{
	check_bt_curdel(3, 1, 0, 0, 0, 0);
	check_bt_curdel(3, 1, B_NODUPS, 0, 0, 0);
	check_bt_curdel(3, 0, 0, 1, 0, 0);
	check_bt_curdel(1, 0, 0, 0, 1, 0);
	check_bt_curdel(2, 1, 0, 0, 0, 1);
}

void test_bt_pdelete_edges(void)
{
	check_bt_pdelete(3, 1, 0, 0);
	check_bt_pdelete(1, 1, 1, 0);
	check_bt_pdelete(2, 0, 0, 0);
	check_bt_pdelete(2, 1, 0, 1);
}

void test_bt_bdelete_edges(void)
{
	check_bt_bdelete(1, B_NODUPS, 0, 0);
	check_bt_bdelete(0, 0, 0, 0);
	check_bt_bdelete(1, 0, 1, 0);
	check_bt_bdelete(1, 0, 0, 1);
}

void test_bt_stkacq_edges(void)
{
	check_bt_stkacq(8, 0);
	check_bt_stkacq(50, 0);
	check_bt_stkacq(8, 1);
}

void test_bt_delete_edges(void)
{
	check_bt_delete(0, 0, 0, 0, 0, 0);
	check_bt_delete(0, B_RDONLY, 0, 0, 0, 0);
	check_bt_delete(R_CURSOR, 0, 1, 0, 0, 0);
	check_bt_delete(R_CURSOR, 0, 1, 1, 0, 0);
	check_bt_delete(99, 0, 0, 0, 0, 0);
	check_bt_delete(R_CURSOR, 0, 1, 0, 0, 1);
}

void test_bt_put_edges(void)
{
	check_bt_put(0, 0, 0, 0, 0, 0, 1);
	check_bt_put(R_NOOVERWRITE, 0, 0, 1, 0, 0, 1);
	check_bt_put(0, B_RDONLY, 0, 0, 0, 0, 1);
	check_bt_put(99, 0, 0, 0, 0, 0, 1);
	check_bt_put(0, 0, 0, 0, 1, 0, 0);
	check_bt_put(0, 0, 0, 0, 0, 1, 1);
	check_bt_put(R_SETCURSOR, 0, 0, 0, 0, 0, 1);
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
	check_bt_seq(R_FIRST, 0, 0, 0, 0);
	check_bt_seq(R_NEXT, 0, 1, 0, 0);
	check_bt_seq(R_LAST, 0, 0, 0, 0);
	check_bt_seq(R_PREV, 0, 1, 0, 0);
	check_bt_seq(R_FIRST, 1, 0, 0, 0);
	check_bt_seq(99, 0, 0, 0, 0);
	check_bt_seq(R_FIRST, 0, 0, 0, 1);
}

void test_bt_open_edges(void)
{
	check_bt_open(1, 0, 0, 0, 0, 0);
	check_bt_open(0, 0, 0, 0, 0, 0);
	check_bt_open(0, 0, 1, 0, 0, 0);
	check_bt_open(0, 1, 0, 0, 0, 0);
	check_bt_open(1, 0, 0, 1, 0, 0);
	check_bt_open(1, 0, 0, 0, 1, 0);
	check_bt_open(1, 0, 0, 0, 0, 1);
}

void sweep_byteorder(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++)
		check_byteorder();
}

void sweep_tmp(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++)
		check_tmp((int)(nextr() & 1u) ? 4 : -1,
		    (int)(nextr() & 3u) == 0 ? "/tmp" : nullptr);
}

void sweep_nroot(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++)
		check_nroot((int)(nextr() & 1u), (int)(nextr() & 3u) == 0,
		    (int)(nextr() % 17u == 0), (int)(nextr() % 23u == 0),
		    (int)(nextr() % 29u == 0), (pgno_t)(100 + (nextr() % 50u)));
}

void sweep_bt_fd(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++)
		check_bt_fd((int)(nextr() & 1u), (int)(nextr() & 3u) == 0);
}

void sweep_bt_setcur(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++)
		check_bt_setcur((int)(nextr() & 1u),
		    (pgno_t)(2 + (nextr() % 20u)), (u_int)(nextr() % 4u));
}

void sweep_bt_relink(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++)
		check_bt_relink((pgno_t)(10 + (nextr() % 30u)),
		    (nextr() & 1u) ? (pgno_t)(5 + (nextr() % 5u)) : P_INVALID,
		    (nextr() & 2u) ? (pgno_t)(20 + (nextr() % 5u)) : P_INVALID,
		    (int)(nextr() % 31u == 0));
}

void sweep_bt_fast(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++) {
		int order = (int)((nextr() % 3u) ? FORWARD : BACK);
		int nents = (int)(nextr() % 6u) + 2;
		check_bt_fast(order, nents, (indx_t)(nents - 1), 8,
		    (int)(nextr() % 3u) - 1, (int)(nextr() % 37u == 0),
		    (size_t)(nextr() % 16u) + 1, (size_t)(nextr() % 16u) + 1);
	}
}

void sweep_bt_dleaf(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++) {
		int nents = (int)(nextr() % 4u) + 2;
		u_int idx = (u_int)(nextr() % (unsigned)nents);
		check_bt_dleaf(nents, idx, (int)(nextr() & 1u),
		    (int)(nextr() % 11u == 0), (int)(nextr() % 13u == 0),
		    (nextr() & 1u) ? B_NODUPS : 0);
	}
}

void sweep_bt_curdel(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++) {
		int nents = (int)(nextr() % 3u) + 2;
		check_bt_curdel(nents, (u_int)(nextr() % (unsigned)nents),
		    (nextr() & 1u) ? B_NODUPS : 0, (int)(nextr() % 7u == 0),
		    (int)(nextr() % 9u == 0), (int)(nextr() % 41u == 0));
	}
}

void sweep_bt_pdelete(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++)
		check_bt_pdelete((int)(nextr() % 4u) + 1, (int)(nextr() & 1u),
		    (int)(nextr() % 5u == 0), (int)(nextr() % 17u == 0));
}

void sweep_bt_bdelete(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++)
		check_bt_bdelete((int)(nextr() & 1u),
		    (nextr() & 1u) ? B_NODUPS : 0, (int)(nextr() % 29u == 0),
		    (int)(nextr() % 43u == 0));
}

void sweep_bt_stkacq(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++)
		check_bt_stkacq((int)(8 + (nextr() % 40u)),
		    (int)(nextr() % 37u == 0));
}

void sweep_bt_delete(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++) {
		u_int fl = (nextr() % 5u == 0) ? R_CURSOR :
		    (nextr() % 7u == 0) ? 99u : 0u;
		check_bt_delete(fl, (nextr() & 8u) ? B_RDONLY : 0,
		    fl == R_CURSOR, (int)(nextr() % 3u == 0),
		    (int)(nextr() % 31u == 0), (int)(nextr() & 1u));
	}
}

void sweep_bt_put(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++) {
		u_int fl = (nextr() % 6u == 0) ? R_NOOVERWRITE :
		    (nextr() % 7u == 0) ? R_SETCURSOR :
		    (nextr() % 8u == 0) ? 99u : 0u;
		check_bt_put(fl, (nextr() & 16u) ? B_RDONLY : 0,
		    (int)(nextr() % 37u == 0), (int)(nextr() & 1u),
		    (int)(nextr() % 5u == 0), (int)(nextr() % 11u == 0),
		    (int)(nextr() & 1u));
	}
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
		    (int)(nextr() % 29u == 0), (int)(nextr() % 47u == 0));
	}
}

void sweep_bt_open(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++) {
		int mode = (int)(nextr() % 7u);
		check_bt_open(mode == 0 || mode == 1, mode == 2, mode == 3,
		    mode == 4, mode == 5, mode == 6);
	}
}

} // namespace

int main(void)
{
	test_byteorder_edges();
	test_tmp_edges();
	test_nroot_edges();
	test_bt_fd_edges();
	test_bt_setcur_edges();
	test_bt_relink_edges();
	test_bt_fast_edges();
	test_bt_dleaf_edges();
	test_bt_curdel_edges();
	test_bt_pdelete_edges();
	test_bt_bdelete_edges();
	test_bt_stkacq_edges();
	test_bt_delete_edges();
	test_bt_put_edges();
	test_bt_seqset_edges();
	test_bt_seqadv_edges();
	test_bt_first_edges();
	test_bt_seq_edges();
	test_bt_open_edges();

	sweep_byteorder();
	sweep_tmp();
	sweep_nroot();
	sweep_bt_fd();
	sweep_bt_setcur();
	sweep_bt_relink();
	sweep_bt_fast();
	sweep_bt_dleaf();
	sweep_bt_curdel();
	sweep_bt_pdelete();
	sweep_bt_bdelete();
	sweep_bt_stkacq();
	sweep_bt_delete();
	sweep_bt_put();
	sweep_bt_seqset();
	sweep_bt_seqadv();
	sweep_bt_first();
	sweep_bt_seq();
	sweep_bt_open();

	std::printf("\n%-14s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < F_COUNT; i++)
		std::printf("%-14s %12lu %12lu\n", fn_name[i], n_cases[i],
		    n_fails[i]);

	unsigned long total_fails = 0;
	for (int i = 0; i < F_COUNT; i++)
		total_fails += n_fails[i];
	return total_fails == 0 ? 0 : 1;
}
