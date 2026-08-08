/*
 * Differential harness for batch b0151: btree search/overflow/utils/debug.
 */

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

import pbsd.lib.libc.db.btree.b0151;

namespace P = pbsd::lib_libc_db_btree::b0151;

extern "C" {
typedef uint32_t pgno_t;
typedef uint16_t indx_t;
typedef uint32_t recno_t;
typedef unsigned int u_int;
typedef unsigned char u_char;
typedef uint32_t u_int32_t;
typedef unsigned long u_long;

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

typedef struct _epg {
	PAGE *page;
	indx_t index;
} EPG;

typedef struct _btree {
	MPOOL *bt_mp;
	DB *bt_dbp;
	EPG bt_cur;
	PAGE *bt_pinned;
	char bt_cursor[64];
	EPGNO bt_stack[50];
	EPGNO *bt_sp;
	DBT bt_rkey;
	DBT bt_rdata;
	char bt_pad[256];
	u_int32_t bt_psize;
	u_int32_t flags;
	int (*bt_cmp)(const DBT *, const DBT *);
} BTREE;

typedef struct _epgno {
	pgno_t pgno;
	indx_t index;
} EPGNO;

typedef struct {
	unsigned get_calls;
	unsigned put_calls;
	unsigned new_calls;
	unsigned free_calls;
	int get_force_null;
	int new_force_null;
	int calloc_fail;
	int realloc_fail;
	pgno_t get_last_pgno;
	unsigned get_last_flags;
	void *last_put_page;
	unsigned last_put_flags;
	void *last_free_page;
	pgno_t next_pgno;
	int nreg;
	pgno_t reg_pgno[256];
	void *reg_page[256];
	int new_fail_after;
	int seq_scan;
	pgno_t seq_next;
} test_mock_state;

extern test_mock_state test_mock;
extern u_long bt_cache_hit, bt_cache_miss, bt_pfxsaved, bt_rootsplit;
extern u_long bt_sortsplit, bt_split;

void test_mock_reset(void);
void test_mock_register(pgno_t pgno, void *page);
void test_mock_seq_begin(pgno_t start);

int ref___bt_defcmp(const DBT *, const DBT *);
size_t ref___bt_defpfx(const DBT *, const DBT *);
int ref___bt_cmp(BTREE *, const DBT *, EPG *);
int ref___bt_ret(BTREE *, EPG *, DBT *, DBT *, DBT *, DBT *, int);
int ref___ovfl_get(BTREE *, void *, size_t *, void **, size_t *);
int ref___ovfl_put(BTREE *, const DBT *, pgno_t *);
int ref___ovfl_delete(BTREE *, void *);
EPG *ref___bt_search(BTREE *, const DBT *, int *);
void ref___bt_dump(DB *);
void ref___bt_dmpage(PAGE *);
void ref___bt_dnpage(DB *, pgno_t);
void ref___bt_dpage(PAGE *);
void ref___bt_stat(DB *);

int harness_cmp(const DBT *a, const DBT *b)
{
	return ref___bt_defcmp(a, b);
}
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr size_t PAGE_SZ = 512;
constexpr size_t BIG_BUF = 4096;
constexpr unsigned SWEEP_ITERS = 200000;

#define P_INVALID 0
#define P_ROOT 1
#define P_BINTERNAL 0x01
#define P_BLEAF 0x02
#define P_OVERFLOW 0x04
#define P_RINTERNAL 0x08
#define P_RLEAF 0x10
#define P_TYPE 0x1f
#define P_BIGKEY 0x02
#define P_BIGDATA 0x01
#define P_PRESERVE 0x20
#define B_NODUPS 0x00020
#define B_DB_LOCK 0x04000
#define B_INMEM 0x00001
#define R_RECNO 0x00080
#define R_FIXLEN 0x00200
#define B_RDONLY 0x00010
#define B_METADIRTY 0x00002
#define RET_ERROR -1
#define RET_SUCCESS 0
#define MPOOL_DIRTY 0x01
#define MPOOL_IGNOREPIN 0x04

#define BTDATAOFF \
	(sizeof(pgno_t) + sizeof(pgno_t) + sizeof(pgno_t) + \
	    sizeof(u_int32_t) + sizeof(indx_t) + sizeof(indx_t))

enum Fn {
	F_DEFCMP,
	F_DEFPFX,
	F_BT_CMP,
	F_BT_RET,
	F_OVFL_GET,
	F_OVFL_PUT,
	F_OVFL_DELETE,
	F_BT_SEARCH,
	F_BT_DUMP,
	F_BT_DMPAGE,
	F_BT_DNPAGE,
	F_BT_DPAGE,
	F_BT_STAT,
	F_COUNT
};

const char *fn_name[F_COUNT] = {
	"__bt_defcmp",
	"__bt_defpfx",
	"__bt_cmp",
	"__bt_ret",
	"__ovfl_get",
	"__ovfl_put",
	"__ovfl_delete",
	"__bt_search",
	"__bt_dump",
	"__bt_dmpage",
	"__bt_dnpage",
	"__bt_dpage",
	"__bt_stat",
};

unsigned long n_cases[F_COUNT];
unsigned long n_fails[F_COUNT];
unsigned reported[F_COUNT];

uint64_t rng = 0xb0151b0151b0151ULL;

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

struct StderrCap {
	int pipefd[2];
	int saved;
	char buf[65536];
	size_t len;

	void start(void)
	{
		if (pipe(pipefd) != 0)
			return;
		saved = dup(STDERR_FILENO);
		dup2(pipefd[1], STDERR_FILENO);
		len = 0;
		buf[0] = '\0';
	}

	void stop(void)
	{
		fflush(stderr);
		dup2(saved, STDERR_FILENO);
		close(saved);
		close(pipefd[1]);
		len = (size_t)read(pipefd[0], buf, sizeof(buf) - 1);
		buf[len] = '\0';
		close(pipefd[0]);
	}
};

void init_tree(P::BTREE &tp, P::MPOOL &mp, P::DB &db, u_int32_t flags,
    u_int32_t psize)
{
	std::memset(&tp, 0, sizeof(tp));
	tp.bt_mp = &mp;
	tp.bt_dbp = &db;
	tp.bt_psize = psize;
	tp.bt_sp = tp.bt_stack;
	tp.flags = flags;
	tp.bt_cmp = harness_cmp;
	db.internal = &tp;
}

void init_tree_ref(BTREE &tr, MPOOL &mp, DB &db, u_int32_t flags,
    u_int32_t psize)
{
	std::memset(&tr, 0, sizeof(tr));
	tr.bt_mp = &mp;
	tr.bt_dbp = &db;
	tr.bt_psize = psize;
	tr.bt_sp = tr.bt_stack;
	tr.flags = flags;
	tr.bt_cmp = harness_cmp;
	db.internal = &tr;
}

void build_bleaf_page(PAGE *pg, size_t psize, int nents,
    const u_int32_t *ksizes, const u_int32_t *dsizes, const u_char *eflags,
    const u_char *keydata, const u_char *datadata)
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
		for (size_t k = 0; k < ksz; k++)
			e[9 + k] = keydata ? keydata[k + i] : (u_char)(0x80 + k + i);
		for (size_t d = 0; d < dsz; d++)
			e[9 + ksz + d] = datadata ? datadata[d + i] :
			    (u_char)(0xc0 + d + i);
		pg->linp[i] = (indx_t)off;
	}
	pg->lower = (indx_t)(BTDATAOFF + nents * sizeof(indx_t));
	pg->upper = (indx_t)off;
}

void build_binternal_page(PAGE *pg, size_t psize, int nents,
    const u_int32_t *ksizes, const u_char *eflags, const pgno_t *pgnos,
    const u_char *keydata)
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
			e[9 + k] = keydata ? keydata[k + i * 4] :
			    (u_char)(0xa0 + k + i);
		pg->linp[i] = (indx_t)off;
	}
	pg->lower = (indx_t)(BTDATAOFF + nents * sizeof(indx_t));
	pg->upper = (indx_t)off;
}

void build_rinternal_page(PAGE *pg, size_t psize, int nents,
    const recno_t *nrecs, const pgno_t *pgnos)
{
	size_t off = psize;
	pg->flags = P_RINTERNAL;
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
			e[5 + k] = (u_char)(0xd0 + k + i);
		pg->linp[i] = (indx_t)off;
	}
	pg->lower = (indx_t)(BTDATAOFF + nents * sizeof(indx_t));
	pg->upper = (indx_t)off;
}

PAGE *make_overflow_page(pgno_t pgno, pgno_t nextpg, const void *data,
    size_t dlen, size_t psize)
{
	unsigned char *buf = (unsigned char *)std::calloc(1, psize);
	PAGE *pg = (PAGE *)buf;
	pg->pgno = pgno;
	pg->nextpg = nextpg;
	pg->prevpg = P_INVALID;
	pg->flags = P_OVERFLOW;
	if (data && dlen)
		std::memcpy((char *)pg + BTDATAOFF, data, dlen);
	return pg;
}

void check_defcmp(const u_char *a, size_t asz, const u_char *b, size_t bsz)
{
	DBT da, db;
	da.data = (void *)a;
	da.size = asz;
	db.data = (void *)b;
	db.size = bsz;
	int rp = P::__bt_defcmp(&da, &db);
	int rr = ref___bt_defcmp(&da, &db);
	char msg[128];
	std::snprintf(msg, sizeof(msg), "ret port=%d ref=%d asz=%zu bsz=%zu",
	    rp, rr, asz, bsz);
	check_eq(F_DEFCMP, rp == rr, msg);
}

void check_defpfx(const u_char *a, size_t asz, const u_char *b, size_t bsz)
{
	DBT da, db;
	da.data = (void *)a;
	da.size = asz;
	db.data = (void *)b;
	db.size = bsz;
	size_t rp = P::__bt_defpfx(&da, &db);
	size_t rr = ref___bt_defpfx(&da, &db);
	char msg[128];
	std::snprintf(msg, sizeof(msg), "ret port=%zu ref=%zu asz=%zu bsz=%zu",
	    rp, rr, asz, bsz);
	check_eq(F_DEFPFX, rp == rr, msg);
}

void check_bt_cmp_leaf(int nents, indx_t idx, const u_char *key, size_t ksz,
    u_int32_t tflags)
{
	unsigned char pgbuf_p[PAGE_SZ];
	unsigned char pgbuf_r[PAGE_SZ];
	u_int32_t ksizes[8] = { 3, 4, 5, 6, 7, 8, 9, 10 };
	u_int32_t dsizes[8] = { 2, 3, 4, 5, 6, 7, 8, 9 };
	u_char eflags[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	DBT k;

	test_mock_reset();
	init_tree(tp, mp_p, db_p, tflags, PAGE_SZ);
	init_tree_ref(tr, mp_r, db_r, tflags, PAGE_SZ);
	guard_fill(pgbuf_p, PAGE_SZ);
	guard_fill(pgbuf_r, PAGE_SZ);
	build_bleaf_page((PAGE *)pgbuf_p, PAGE_SZ, nents, ksizes, dsizes, eflags,
	    nullptr, nullptr);
	std::memcpy(pgbuf_r, pgbuf_p, PAGE_SZ);
	((PAGE *)pgbuf_p)->pgno = 2;
	((PAGE *)pgbuf_r)->pgno = 2;

	P::EPG ep;
	ep.page = (P::PAGE *)pgbuf_p;
	ep.index = idx;
	EPG er;
	er.page = (PAGE *)pgbuf_r;
	er.index = idx;
	k.data = (void *)key;
	k.size = ksz;

	int rp = P::__bt_cmp(&tp, &k, &ep);
	int rr = ref___bt_cmp(&tr, &k, &er);
	char msg[160];
	std::snprintf(msg, sizeof(msg),
	    "ret port=%d ref=%d idx=%u nents=%d ksz=%zu", rp, rr,
	    (unsigned)idx, nents, ksz);
	check_eq(F_BT_CMP, rp == rr, msg);
}

void check_bt_cmp_internal0(void)
{
	unsigned char pgbuf_p[PAGE_SZ];
	unsigned char pgbuf_r[PAGE_SZ];
	u_int32_t ksizes[2] = { 4, 4 };
	u_char eflags[2] = { 0, 0 };
	pgno_t pgnos[2] = { 10, 11 };
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	DBT k;
	u_char key[4] = { 1, 2, 3, 4 };

	test_mock_reset();
	init_tree(tp, mp_p, db_p, 0, PAGE_SZ);
	init_tree_ref(tr, mp_r, db_r, 0, PAGE_SZ);
	guard_fill(pgbuf_p, PAGE_SZ);
	guard_fill(pgbuf_r, PAGE_SZ);
	build_binternal_page((PAGE *)pgbuf_p, PAGE_SZ, 2, ksizes, eflags, pgnos,
	    nullptr);
	std::memcpy(pgbuf_r, pgbuf_p, PAGE_SZ);
	((PAGE *)pgbuf_p)->prevpg = P_INVALID;
	((PAGE *)pgbuf_r)->prevpg = P_INVALID;
	((PAGE *)pgbuf_p)->pgno = 1;
	((PAGE *)pgbuf_r)->pgno = 1;

	P::EPG ep;
	ep.page = (P::PAGE *)pgbuf_p;
	ep.index = 0;
	EPG er;
	er.page = (PAGE *)pgbuf_r;
	er.index = 0;
	k.data = key;
	k.size = 4;

	int rp = P::__bt_cmp(&tp, &k, &ep);
	int rr = ref___bt_cmp(&tr, &k, &er);
	check_eq(F_BT_CMP, rp == rr && rp == 1, "internal index 0 sentinel");
}

void setup_bigkey_overflow(unsigned char *leafbuf, size_t psize,
    pgno_t ovpg, const u_char *keydata, size_t ksz, size_t dsz)
{
	u_int32_t ksizes[1] = { (u_int32_t)(sizeof(pgno_t) + sizeof(u_int32_t)) };
	u_int32_t dsizes[1] = { (u_int32_t)dsz };
	u_char eflags[1] = { P_BIGKEY };
	build_bleaf_page((PAGE *)leafbuf, psize, 1, ksizes, dsizes, eflags,
	    nullptr, nullptr);
	BLEAF *bl = (BLEAF *)((char *)leafbuf + ((PAGE *)leafbuf)->linp[0]);
	*(pgno_t *)bl->bytes = ovpg;
	*(u_int32_t *)(bl->bytes + sizeof(pgno_t)) = (u_int32_t)ksz;
	if (dsz)
		std::memset(bl->bytes + bl->ksize, 0xee, dsz);
	(void)keydata;
}

void check_bt_cmp_bigkey(const u_char *key, size_t ksz)
{
	unsigned char leaf_p[PAGE_SZ];
	unsigned char leaf_r[PAGE_SZ];
	unsigned char ov_p[PAGE_SZ];
	unsigned char ov_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	DBT k;
	pgno_t ovpg = 50;

	test_mock_reset();
	init_tree(tp, mp_p, db_p, 0, PAGE_SZ);
	init_tree_ref(tr, mp_r, db_r, 0, PAGE_SZ);
	guard_fill(leaf_p, PAGE_SZ);
	guard_fill(leaf_r, PAGE_SZ);
	setup_bigkey_overflow(leaf_p, PAGE_SZ, ovpg, key, ksz, 4);
	std::memcpy(leaf_r, leaf_p, PAGE_SZ);
	((PAGE *)leaf_p)->flags = P_BLEAF;
	((PAGE *)leaf_r)->flags = P_BLEAF;
	PAGE *op = make_overflow_page(ovpg, P_INVALID, key, ksz, PAGE_SZ);
	PAGE *or_ = make_overflow_page(ovpg, P_INVALID, key, ksz, PAGE_SZ);
	test_mock_register(ovpg, op);
	test_mock_register(ovpg, or_);

	P::EPG ep;
	ep.page = (P::PAGE *)leaf_p;
	ep.index = 0;
	EPG er;
	er.page = (PAGE *)leaf_r;
	er.index = 0;
	k.data = (void *)key;
	k.size = ksz;

	int rp = P::__bt_cmp(&tp, &k, &ep);
	int rr = ref___bt_cmp(&tr, &k, &er);
	char msg[128];
	std::snprintf(msg, sizeof(msg), "bigkey port=%d ref=%d ksz=%zu", rp, rr,
	    ksz);
	check_eq(F_BT_CMP, rp == rr, msg);
	free(op);
	free(or_);
}

void check_bt_ret(int copy, u_int32_t tflags, u_char bflags, u_int32_t ksz,
    u_int32_t dsz, int key_null, int data_null)
{
	unsigned char leaf_p[PAGE_SZ];
	unsigned char leaf_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	unsigned char rkeybuf_p[BIG_BUF];
	unsigned char rkeybuf_r[BIG_BUF];
	unsigned char rdbuf_p[BIG_BUF];
	unsigned char rdbuf_r[BIG_BUF];
	unsigned char keybuf_p[BIG_BUF];
	unsigned char keybuf_r[BIG_BUF];
	unsigned char databuf_p[BIG_BUF];
	unsigned char databuf_r[BIG_BUF];
	u_int32_t ksizes[1] = { ksz };
	u_int32_t dsizes[1] = { dsz };
	u_char eflags[1] = { bflags };
	DBT key_p, key_r, data_p, data_r, rkey_p, rkey_r, rdata_p, rdata_r;

	test_mock_reset();
	init_tree(tp, mp_p, db_p, tflags, PAGE_SZ);
	init_tree_ref(tr, mp_r, db_r, tflags, PAGE_SZ);
	guard_fill(leaf_p, PAGE_SZ);
	guard_fill(leaf_r, PAGE_SZ);
	guard_fill(rkeybuf_p, BIG_BUF);
	guard_fill(rkeybuf_r, BIG_BUF);
	guard_fill(rdbuf_p, BIG_BUF);
	guard_fill(rdbuf_r, BIG_BUF);
	guard_fill(keybuf_p, BIG_BUF);
	guard_fill(keybuf_r, BIG_BUF);
	guard_fill(databuf_p, BIG_BUF);
	guard_fill(databuf_r, BIG_BUF);

	if (bflags & P_BIGKEY) {
		pgno_t ovpg = 60;
		setup_bigkey_overflow(leaf_p, PAGE_SZ, ovpg, nullptr, ksz, dsz);
		std::memcpy(leaf_r, leaf_p, PAGE_SZ);
		u_char kdata[64];
		for (u_int32_t i = 0; i < ksz && i < sizeof(kdata); i++)
			kdata[i] = (u_char)(0x80 + i);
		PAGE *op = make_overflow_page(ovpg, P_INVALID, kdata, ksz, PAGE_SZ);
		PAGE *or_ = make_overflow_page(ovpg, P_INVALID, kdata, ksz, PAGE_SZ);
		test_mock_register(ovpg, op);
		test_mock_register(ovpg, or_);
	} else {
		build_bleaf_page((PAGE *)leaf_p, PAGE_SZ, 1, ksizes, dsizes, eflags,
		    nullptr, nullptr);
		std::memcpy(leaf_r, leaf_p, PAGE_SZ);
	}

	P::EPG ep;
	ep.page = (P::PAGE *)leaf_p;
	ep.index = 0;
	EPG er;
	er.page = (PAGE *)leaf_r;
	er.index = 0;

	rkey_p.data = rkeybuf_p + 64;
	rkey_p.size = 32;
	rkey_r.data = rkeybuf_r + 64;
	rkey_r.size = 32;
	rdata_p.data = rdbuf_p + 64;
	rdata_p.size = 32;
	rdata_r.data = rdbuf_r + 64;
	rdata_r.size = 32;

	int rp = P::__bt_ret(&tp, &ep, key_null ? nullptr : &key_p,
	    &rkey_p, data_null ? nullptr : &data_p, &rdata_p, copy);
	int rr = ref___bt_ret(&tr, &er, key_null ? nullptr : &key_r,
	    &rkey_r, data_null ? nullptr : &data_r, &rdata_r, copy);

	char msg[200];
	std::snprintf(msg, sizeof(msg),
	    "ret port=%d ref=%d copy=%d fl=0x%x ksz=%u dsz=%u", rp, rr, copy,
	    (unsigned)bflags, ksz, dsz);
	check_eq(F_BT_RET, rp == rr, msg);

	if (rp == rr && rp == RET_SUCCESS) {
		if (!key_null) {
			check_eq(F_BT_RET, key_p.size == key_r.size, "key.size");
			if (key_p.size > 0 && key_p.data && key_r.data)
				check_eq(F_BT_RET,
				    std::memcmp(key_p.data, key_r.data,
					key_p.size) == 0,
				    "key.data");
		}
		if (!data_null) {
			check_eq(F_BT_RET, data_p.size == data_r.size,
			    "data.size");
			if (data_p.size > 0 && data_p.data && data_r.data)
				check_eq(F_BT_RET,
				    std::memcmp(data_p.data, data_r.data,
					data_p.size) == 0,
				    "data.data");
		}
		check_eq(F_BT_RET, bufs_eq(rkeybuf_p, rkeybuf_r, BIG_BUF),
		    "rkey guard");
		check_eq(F_BT_RET, bufs_eq(rdbuf_p, rdbuf_r, BIG_BUF),
		    "rdata guard");
	}
}

void check_ovfl_get(pgno_t firstpg, size_t total, int force_null, int smallbuf)
{
	unsigned char desc_p[16];
	unsigned char desc_r[16];
	unsigned char buf_p[BIG_BUF];
	unsigned char buf_r[BIG_BUF];
	unsigned char pg1_p[PAGE_SZ];
	unsigned char pg1_r[PAGE_SZ];
	unsigned char pg2_p[PAGE_SZ];
	unsigned char pg2_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	size_t plen = PAGE_SZ - BTDATAOFF;
	size_t nb1 = total < plen ? total : plen;
	size_t nb2 = total > plen ? total - plen : 0;
	pgno_t pg2 = firstpg + 1;
	u_char payload[128];
	size_t i;

	for (i = 0; i < total && i < sizeof(payload); i++)
		payload[i] = (u_char)(0x80 + i);

	test_mock_reset();
	test_mock.get_force_null = force_null;
	init_tree(tp, mp_p, db_p, 0, PAGE_SZ);
	init_tree_ref(tr, mp_r, db_r, 0, PAGE_SZ);
	guard_fill(buf_p, BIG_BUF);
	guard_fill(buf_r, BIG_BUF);

	*(pgno_t *)desc_p = firstpg;
	*(u_int32_t *)(desc_p + sizeof(pgno_t)) = (u_int32_t)total;
	std::memcpy(desc_r, desc_p, sizeof(desc_p));

	PAGE *h1p = make_overflow_page(firstpg, nb2 ? pg2 : P_INVALID,
	    payload, nb1, PAGE_SZ);
	PAGE *h1r = make_overflow_page(firstpg, nb2 ? pg2 : P_INVALID,
	    payload, nb1, PAGE_SZ);
	test_mock_register(firstpg, h1p);
	test_mock_register(firstpg, h1r);
	if (nb2) {
		PAGE *h2p = make_overflow_page(pg2, P_INVALID, payload + nb1,
		    nb2, PAGE_SZ);
		PAGE *h2r = make_overflow_page(pg2, P_INVALID, payload + nb1,
		    nb2, PAGE_SZ);
		test_mock_register(pg2, h2p);
		test_mock_register(pg2, h2r);
	}

	void *bp = buf_p + 128;
	void *br = buf_r + 128;
	size_t bsz_p = smallbuf ? 8 : BIG_BUF - 256;
	size_t bsz_r = smallbuf ? 8 : BIG_BUF - 256;
	size_t ssz_p = 0;
	size_t ssz_r = 0;

	int rp = P::__ovfl_get(&tp, desc_p, &ssz_p, &bp, &bsz_p);
	int rr = ref___ovfl_get(&tr, desc_r, &ssz_r, &br, &bsz_r);
	char msg[160];
	std::snprintf(msg, sizeof(msg),
	    "ret port=%d ref=%d total=%zu null=%d small=%d", rp, rr, total,
	    force_null, smallbuf);
	check_eq(F_OVFL_GET, rp == rr, msg);
	if (rp == rr && rp == RET_SUCCESS) {
		check_eq(F_OVFL_GET, ssz_p == ssz_r && ssz_p == total, "ssz");
		check_eq(F_OVFL_GET,
		    std::memcmp(bp, br, total) == 0, "payload");
		check_eq(F_OVFL_GET, bufs_eq(buf_p, buf_r, BIG_BUF), "guard");
	}
}

void check_ovfl_put(size_t dlen, int new_null, int fail_after)
{
	unsigned char data_p[BIG_BUF];
	unsigned char data_r[BIG_BUF];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	DBT dbt_p, dbt_r;
	pgno_t pg_p = 0;
	pgno_t pg_r = 0;

	test_mock_reset();
	test_mock.new_force_null = new_null;
	test_mock.new_fail_after = fail_after;
	init_tree(tp, mp_p, db_p, 0, PAGE_SZ);
	init_tree_ref(tr, mp_r, db_r, 0, PAGE_SZ);
	for (size_t i = 0; i < dlen; i++) {
		data_p[i] = (u_char)(0x80 + (i & 0x7f));
		data_r[i] = data_p[i];
	}
	dbt_p.data = data_p;
	dbt_p.size = dlen;
	dbt_r.data = data_r;
	dbt_r.size = dlen;

	unsigned new_before_p = test_mock.new_calls;
	unsigned new_before_r = test_mock.new_calls;

	int rp = P::__ovfl_put(&tp, &dbt_p, &pg_p);
	int rr = ref___ovfl_put(&tr, &dbt_r, &pg_r);
	(void)new_before_p;
	(void)new_before_r;
	char msg[128];
	std::snprintf(msg, sizeof(msg), "ret port=%d ref=%d dlen=%zu", rp, rr,
	    dlen);
	check_eq(F_OVFL_PUT, rp == rr, msg);
	if (rp == rr && rp == RET_SUCCESS)
		check_eq(F_OVFL_PUT, pg_p == pg_r, "first pgno");
}

void check_ovfl_delete(pgno_t firstpg, size_t total, int preserve, int nullget)
{
	unsigned char desc_p[16];
	unsigned char desc_r[16];
	unsigned char pgbuf_p[PAGE_SZ];
	unsigned char pgbuf_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	size_t plen = PAGE_SZ - BTDATAOFF;

	test_mock_reset();
	test_mock.get_force_null = nullget;
	init_tree(tp, mp_p, db_p, 0, PAGE_SZ);
	init_tree_ref(tr, mp_r, db_r, 0, PAGE_SZ);
	*(pgno_t *)desc_p = firstpg;
	*(u_int32_t *)(desc_p + sizeof(pgno_t)) = (u_int32_t)total;
	std::memcpy(desc_r, desc_p, sizeof(desc_p));

	guard_fill(pgbuf_p, PAGE_SZ);
	guard_fill(pgbuf_r, PAGE_SZ);
	((PAGE *)pgbuf_p)->pgno = firstpg;
	((PAGE *)pgbuf_r)->pgno = firstpg;
	if (preserve) {
		((PAGE *)pgbuf_p)->flags = P_OVERFLOW | P_PRESERVE;
		((PAGE *)pgbuf_r)->flags = P_OVERFLOW | P_PRESERVE;
	} else {
		((PAGE *)pgbuf_p)->flags = P_OVERFLOW;
		((PAGE *)pgbuf_r)->flags = P_OVERFLOW;
	}
	if (total > plen) {
		((PAGE *)pgbuf_p)->nextpg = firstpg + 1;
		((PAGE *)pgbuf_r)->nextpg = firstpg + 1;
	}
	test_mock_register(firstpg, pgbuf_p);
	test_mock_register(firstpg, pgbuf_r);
	if (total > plen) {
		unsigned char pg2_p[PAGE_SZ];
		unsigned char pg2_r[PAGE_SZ];
		guard_fill(pg2_p, PAGE_SZ);
		guard_fill(pg2_r, PAGE_SZ);
		((PAGE *)pg2_p)->pgno = firstpg + 1;
		((PAGE *)pg2_r)->pgno = firstpg + 1;
		((PAGE *)pg2_p)->flags = P_OVERFLOW;
		((PAGE *)pg2_r)->flags = P_OVERFLOW;
		test_mock_register(firstpg + 1, pg2_p);
		test_mock_register(firstpg + 1, pg2_r);
	}

	unsigned free_before = test_mock.free_calls;
	int rp = P::__ovfl_delete(&tp, desc_p);
	int rr = ref___ovfl_delete(&tr, desc_r);
	char msg[160];
	std::snprintf(msg, sizeof(msg),
	    "ret port=%d ref=%d total=%zu preserve=%d null=%d free_delta=%u",
	    rp, rr, total, preserve, nullget,
	    test_mock.free_calls - free_before);
	check_eq(F_OVFL_DELETE, rp == rr, msg);
}

void check_bt_search_leaf(int nents, const u_char *key, size_t ksz,
    u_int32_t tflags, int force_null)
{
	unsigned char root_p[PAGE_SZ];
	unsigned char root_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	DBT k;
	int exact_p = -1;
	int exact_r = -1;
	u_int32_t ksizes[8] = { 2, 4, 6, 8, 10, 12, 14, 16 };
	u_int32_t dsizes[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	u_char eflags[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	test_mock_reset();
	test_mock.get_force_null = force_null;
	init_tree(tp, mp_p, db_p, tflags, PAGE_SZ);
	init_tree_ref(tr, mp_r, db_r, tflags, PAGE_SZ);
	guard_fill(root_p, PAGE_SZ);
	guard_fill(root_r, PAGE_SZ);
	build_bleaf_page((PAGE *)root_p, PAGE_SZ, nents, ksizes, dsizes, eflags,
	    nullptr, nullptr);
	std::memcpy(root_r, root_p, PAGE_SZ);
	((PAGE *)root_p)->pgno = P_ROOT;
	((PAGE *)root_r)->pgno = P_ROOT;
	((PAGE *)root_p)->prevpg = P_INVALID;
	((PAGE *)root_r)->prevpg = P_INVALID;
	((PAGE *)root_p)->nextpg = P_INVALID;
	((PAGE *)root_r)->nextpg = P_INVALID;
	test_mock_register(P_ROOT, root_p);
	test_mock_register(P_ROOT, root_r);

	k.data = (void *)key;
	k.size = ksz;

	P::EPG *rp = P::__bt_search(&tp, &k, &exact_p);
	EPG *rr = ref___bt_search(&tr, &k, &exact_r);
	char msg[200];
	std::snprintf(msg, sizeof(msg),
	    "null port=%p ref=%p exact port=%d ref=%d nents=%d ksz=%zu",
	    (void *)rp, (void *)rr, exact_p, exact_r, nents, ksz);
	bool ok = (rp == nullptr) == (rr == nullptr) && exact_p == exact_r;
	if (ok && rp && rr) {
		ptrdiff_t off_p = (char *)rp - (char *)&tp;
		ptrdiff_t off_r = (char *)rr - (char *)&tr;
		ok = off_p == off_r &&
		    rp->index == rr->index &&
		    rp->page->pgno == rr->page->pgno;
	}
	check_eq(F_BT_SEARCH, ok, msg);
}

void check_bt_search_internal(int nleaf, const u_char *key, size_t ksz)
{
	unsigned char root_p[PAGE_SZ];
	unsigned char root_r[PAGE_SZ];
	unsigned char leaf_p[PAGE_SZ];
	unsigned char leaf_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	DBT k;
	int exact_p = -1;
	int exact_r = -1;
	u_int32_t iksizes[3] = { 2, 8, 16 };
	u_char ieflags[3] = { 0, 0, 0 };
	pgno_t ipgnos[3] = { 10, 10, 10 };
	u_int32_t ksizes[8] = { 2, 4, 6, 8, 10, 12, 14, 16 };
	u_int32_t dsizes[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	u_char eflags[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	test_mock_reset();
	init_tree(tp, mp_p, db_p, 0, PAGE_SZ);
	init_tree_ref(tr, mp_r, db_r, 0, PAGE_SZ);
	guard_fill(root_p, PAGE_SZ);
	guard_fill(root_r, PAGE_SZ);
	guard_fill(leaf_p, PAGE_SZ);
	guard_fill(leaf_r, PAGE_SZ);
	build_binternal_page((PAGE *)root_p, PAGE_SZ, 1, iksizes, ieflags, ipgnos,
	    nullptr);
	std::memcpy(root_r, root_p, PAGE_SZ);
	((PAGE *)root_p)->pgno = P_ROOT;
	((PAGE *)root_r)->pgno = P_ROOT;
	((PAGE *)root_p)->prevpg = P_INVALID;
	((PAGE *)root_r)->prevpg = P_INVALID;
	build_bleaf_page((PAGE *)leaf_p, PAGE_SZ, nleaf, ksizes, dsizes, eflags,
	    nullptr, nullptr);
	std::memcpy(leaf_r, leaf_p, PAGE_SZ);
	((PAGE *)leaf_p)->pgno = 10;
	((PAGE *)leaf_r)->pgno = 10;
	((PAGE *)leaf_p)->prevpg = P_INVALID;
	((PAGE *)leaf_r)->prevpg = P_INVALID;
	((PAGE *)leaf_p)->nextpg = P_INVALID;
	((PAGE *)leaf_r)->nextpg = P_INVALID;
	test_mock_register(P_ROOT, root_p);
	test_mock_register(P_ROOT, root_r);
	test_mock_register(10, leaf_p);
	test_mock_register(10, leaf_r);

	k.data = (void *)key;
	k.size = ksz;

	P::EPG *rp = P::__bt_search(&tp, &k, &exact_p);
	EPG *rr = ref___bt_search(&tr, &k, &exact_r);
	bool ok = (rp == nullptr) == (rr == nullptr) && exact_p == exact_r;
	if (ok && rp && rr)
		ok = rp->index == rr->index;
	char msg[160];
	std::snprintf(msg, sizeof(msg), "internal exact port=%d ref=%d idx port=%u ref=%u",
	    exact_p, exact_r, rp ? (unsigned)rp->index : 0u,
	    rr ? (unsigned)rr->index : 0u);
	check_eq(F_BT_SEARCH, ok, msg);
}

void check_bt_search_snext(int match_next)
{
	unsigned char leaf1_p[PAGE_SZ];
	unsigned char leaf1_r[PAGE_SZ];
	unsigned char leaf2_p[PAGE_SZ];
	unsigned char leaf2_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	DBT k;
	int exact_p = 0;
	int exact_r = 0;
	u_char key0[4] = { 0xa0, 0xa1, 0xa2, 0xa3 };
	u_char key1[4] = { 0xb0, 0xb1, 0xb2, 0xb3 };
	u_int32_t ksizes[1] = { 4 };
	u_int32_t dsizes[1] = { 2 };
	u_char eflags[1] = { 0 };

	test_mock_reset();
	init_tree(tp, mp_p, db_p, 0, PAGE_SZ);
	init_tree_ref(tr, mp_r, db_r, 0, PAGE_SZ);
	guard_fill(leaf1_p, PAGE_SZ);
	guard_fill(leaf1_r, PAGE_SZ);
	guard_fill(leaf2_p, PAGE_SZ);
	guard_fill(leaf2_r, PAGE_SZ);
	build_bleaf_page((PAGE *)leaf1_p, PAGE_SZ, 1, ksizes, dsizes, eflags,
	    key0, nullptr);
	std::memcpy(leaf1_r, leaf1_p, PAGE_SZ);
	build_bleaf_page((PAGE *)leaf2_p, PAGE_SZ, 1, ksizes, dsizes, eflags,
	    match_next ? key1 : key0, nullptr);
	std::memcpy(leaf2_r, leaf2_p, PAGE_SZ);
	((PAGE *)leaf1_p)->pgno = P_ROOT;
	((PAGE *)leaf1_r)->pgno = P_ROOT;
	((PAGE *)leaf1_p)->nextpg = 11;
	((PAGE *)leaf1_r)->nextpg = 11;
	((PAGE *)leaf2_p)->pgno = 11;
	((PAGE *)leaf2_r)->pgno = 11;
	((PAGE *)leaf2_p)->prevpg = P_ROOT;
	((PAGE *)leaf2_r)->prevpg = P_ROOT;
	test_mock_register(P_ROOT, leaf1_p);
	test_mock_register(P_ROOT, leaf1_r);
	test_mock_register(11, leaf2_p);
	test_mock_register(11, leaf2_r);

	k.data = key1;
	k.size = 4;

	P::EPG *rp = P::__bt_search(&tp, &k, &exact_p);
	EPG *rr = ref___bt_search(&tr, &k, &exact_r);
	bool ok = exact_p == exact_r;
	if (match_next)
		ok = ok && exact_p == 1;
	char msg[128];
	std::snprintf(msg, sizeof(msg), "snext exact port=%d ref=%d match=%d",
	    exact_p, exact_r, match_next);
	check_eq(F_BT_SEARCH, ok, msg);
	(void)rp;
	(void)rr;
}

void check_bt_search_sprev(int match_prev)
{
	unsigned char leaf0_p[PAGE_SZ];
	unsigned char leaf0_r[PAGE_SZ];
	unsigned char leaf1_p[PAGE_SZ];
	unsigned char leaf1_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	DBT k;
	int exact_p = 0;
	int exact_r = 0;
	u_char key0[4] = { 0xa0, 0xa1, 0xa2, 0xa3 };
	u_char key1[4] = { 0xb0, 0xb1, 0xb2, 0xb3 };
	u_int32_t ksizes[1] = { 4 };
	u_int32_t dsizes[1] = { 2 };
	u_char eflags[1] = { 0 };

	test_mock_reset();
	init_tree(tp, mp_p, db_p, 0, PAGE_SZ);
	init_tree_ref(tr, mp_r, db_r, 0, PAGE_SZ);
	guard_fill(leaf0_p, PAGE_SZ);
	guard_fill(leaf0_r, PAGE_SZ);
	guard_fill(leaf1_p, PAGE_SZ);
	guard_fill(leaf1_r, PAGE_SZ);
	build_bleaf_page((PAGE *)leaf0_p, PAGE_SZ, 1, ksizes, dsizes, eflags,
	    match_prev ? key1 : key0, nullptr);
	std::memcpy(leaf0_r, leaf0_p, PAGE_SZ);
	build_bleaf_page((PAGE *)leaf1_p, PAGE_SZ, 1, ksizes, dsizes, eflags,
	    key0, nullptr);
	std::memcpy(leaf1_r, leaf1_p, PAGE_SZ);
	((PAGE *)leaf0_p)->pgno = 10;
	((PAGE *)leaf0_r)->pgno = 10;
	((PAGE *)leaf0_p)->nextpg = P_ROOT;
	((PAGE *)leaf0_r)->nextpg = P_ROOT;
	((PAGE *)leaf1_p)->pgno = P_ROOT;
	((PAGE *)leaf1_r)->pgno = P_ROOT;
	((PAGE *)leaf1_p)->prevpg = 10;
	((PAGE *)leaf1_r)->prevpg = 10;
	test_mock_register(10, leaf0_p);
	test_mock_register(10, leaf0_r);
	test_mock_register(P_ROOT, leaf1_p);
	test_mock_register(P_ROOT, leaf1_r);

	k.data = key1;
	k.size = 4;

	P::__bt_search(&tp, &k, &exact_p);
	ref___bt_search(&tr, &k, &exact_r);
	bool ok = exact_p == exact_r;
	if (match_prev)
		ok = ok && exact_p == 1;
	char msg[128];
	std::snprintf(msg, sizeof(msg), "sprev exact port=%d ref=%d match=%d",
	    exact_p, exact_r, match_prev);
	check_eq(F_BT_SEARCH, ok, msg);
}

void check_dpage(u_int32_t ptype, int nents)
{
	unsigned char pgbuf_p[PAGE_SZ];
	unsigned char pgbuf_r[PAGE_SZ];
	StderrCap cp;
	StderrCap cr;

	guard_fill(pgbuf_p, PAGE_SZ);
	guard_fill(pgbuf_r, PAGE_SZ);
	((PAGE *)pgbuf_p)->pgno = 7;
	((PAGE *)pgbuf_r)->pgno = 7;
	((PAGE *)pgbuf_p)->prevpg = 3;
	((PAGE *)pgbuf_r)->prevpg = 3;
	((PAGE *)pgbuf_p)->nextpg = 9;
	((PAGE *)pgbuf_r)->nextpg = 9;
	((PAGE *)pgbuf_p)->flags = ptype;
	((PAGE *)pgbuf_r)->flags = ptype;

	if (ptype == P_BLEAF) {
		u_int32_t ks[4] = { 3, 4, 5, 6 };
		u_int32_t ds[4] = { 2, 3, 4, 5 };
		u_char ef[4] = { 0, P_BIGKEY, 0, P_BIGDATA };
		build_bleaf_page((PAGE *)pgbuf_p, PAGE_SZ, nents, ks, ds, ef,
		    nullptr, nullptr);
		std::memcpy(pgbuf_r, pgbuf_p, PAGE_SZ);
	} else if (ptype == P_BINTERNAL) {
		u_int32_t ks[4] = { 4, 5, 6, 7 };
		u_char ef[4] = { 0, P_BIGKEY, 0, 0 };
		pgno_t pgs[4] = { 20, 21, 22, 23 };
		build_binternal_page((PAGE *)pgbuf_p, PAGE_SZ, nents, ks, ef, pgs,
		    nullptr);
		std::memcpy(pgbuf_r, pgbuf_p, PAGE_SZ);
	} else if (ptype == P_RINTERNAL) {
		recno_t nr[4] = { 10, 20, 30, 40 };
		pgno_t pgs[4] = { 30, 31, 32, 33 };
		build_rinternal_page((PAGE *)pgbuf_p, PAGE_SZ, nents, nr, pgs);
		std::memcpy(pgbuf_r, pgbuf_p, PAGE_SZ);
	} else if (ptype == P_RLEAF) {
		u_int32_t ds[4] = { 5, 6, 7, 8 };
		u_char ef[4] = { 0, P_BIGDATA, 0, 0 };
		build_rleaf_page((PAGE *)pgbuf_p, PAGE_SZ, nents, ds, ef);
		std::memcpy(pgbuf_r, pgbuf_p, PAGE_SZ);
	} else if (ptype == P_OVERFLOW) {
		((PAGE *)pgbuf_p)->flags = P_OVERFLOW;
		((PAGE *)pgbuf_r)->flags = P_OVERFLOW;
	}

	cp.start();
	P::__bt_dpage((P::PAGE *)pgbuf_p);
	cp.stop();
	cr.start();
	ref___bt_dpage((PAGE *)pgbuf_r);
	cr.stop();
	check_eq(F_BT_DPAGE, std::strcmp(cp.buf, cr.buf) == 0, "stderr output");
}

void check_dmpage(void)
{
	unsigned char pgbuf_p[PAGE_SZ];
	unsigned char pgbuf_r[PAGE_SZ];
	StderrCap cp;
	StderrCap cr;
	P::BTMETA *mp = (P::BTMETA *)pgbuf_p;
	P::BTMETA *mr = (P::BTMETA *)pgbuf_r;

	guard_fill(pgbuf_p, PAGE_SZ);
	guard_fill(pgbuf_r, PAGE_SZ);
	mp->magic = 0x053162;
	mp->version = 3;
	mp->psize = PAGE_SZ;
	mp->free = 5;
	mp->nrecs = 99;
	mp->flags = B_NODUPS | R_RECNO;
	*mr = *mp;

	cp.start();
	P::__bt_dmpage((P::PAGE *)pgbuf_p);
	cp.stop();
	cr.start();
	ref___bt_dmpage((PAGE *)pgbuf_r);
	cr.stop();
	check_eq(F_BT_DMPAGE, std::strcmp(cp.buf, cr.buf) == 0, "stderr output");
}

void check_dnpage(pgno_t pgno)
{
	unsigned char pgbuf_p[PAGE_SZ];
	unsigned char pgbuf_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	StderrCap cp;
	StderrCap cr;

	test_mock_reset();
	init_tree(tp, mp_p, db_p, 0, PAGE_SZ);
	init_tree_ref(tr, mp_r, db_r, 0, PAGE_SZ);
	guard_fill(pgbuf_p, PAGE_SZ);
	guard_fill(pgbuf_r, PAGE_SZ);
	((PAGE *)pgbuf_p)->pgno = pgno;
	((PAGE *)pgbuf_r)->pgno = pgno;
	((PAGE *)pgbuf_p)->flags = P_BLEAF;
	((PAGE *)pgbuf_r)->flags = P_BLEAF;
	u_int32_t ks[1] = { 3 };
	u_int32_t ds[1] = { 2 };
	u_char ef[1] = { 0 };
	build_bleaf_page((PAGE *)pgbuf_p, PAGE_SZ, 1, ks, ds, ef, nullptr,
	    nullptr);
	std::memcpy(pgbuf_r, pgbuf_p, PAGE_SZ);
	test_mock_register(pgno, pgbuf_p);
	test_mock_register(pgno, pgbuf_r);

	cp.start();
	P::__bt_dnpage(&db_p, pgno);
	cp.stop();
	cr.start();
	ref___bt_dnpage(&db_r, pgno);
	cr.stop();
	check_eq(F_BT_DNPAGE, std::strcmp(cp.buf, cr.buf) == 0, "stderr output");
}

void setup_stat_tree(unsigned char *leaf_p, unsigned char *root_p, int nkeys)
{
	u_int32_t ksizes[8] = { 2, 4, 6, 8, 10, 12, 14, 16 };
	u_int32_t dsizes[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	u_char eflags[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	u_int32_t iks[1] = { 2 };
	u_char ief[1] = { 0 };
	pgno_t ipg[1] = { 2 };
	build_bleaf_page((PAGE *)leaf_p, PAGE_SZ, nkeys, ksizes, dsizes, eflags,
	    nullptr, nullptr);
	((PAGE *)leaf_p)->pgno = 2;
	build_binternal_page((PAGE *)root_p, PAGE_SZ, 1, iks, ief, ipg, nullptr);
	((PAGE *)root_p)->pgno = P_ROOT;
}

void check_dump_stat(u_int32_t tflags)
{
	unsigned char leaf_p[PAGE_SZ];
	unsigned char leaf_r[PAGE_SZ];
	unsigned char root_p[PAGE_SZ];
	unsigned char root_r[PAGE_SZ];
	P::BTREE tp;
	P::MPOOL mp_p;
	P::DB db_p;
	BTREE tr;
	MPOOL mp_r;
	DB db_r;
	StderrCap cp;
	StderrCap cr;
	StderrCap csp;
	StderrCap csr;

	test_mock_reset();
	test_mock_seq_begin(P_ROOT);
	init_tree(tp, mp_p, db_p, tflags | B_INMEM, PAGE_SZ);
	init_tree_ref(tr, mp_r, db_r, tflags | B_INMEM, PAGE_SZ);
	tp.bt_nrecs = 42;
	tr.bt_nrecs = 42;
	guard_fill(leaf_p, PAGE_SZ);
	guard_fill(leaf_r, PAGE_SZ);
	guard_fill(root_p, PAGE_SZ);
	guard_fill(root_r, PAGE_SZ);
	setup_stat_tree(leaf_p, root_p, 4);
	std::memcpy(leaf_r, leaf_p, PAGE_SZ);
	std::memcpy(root_r, root_p, PAGE_SZ);
	test_mock_register(P_ROOT, root_p);
	test_mock_register(P_ROOT, root_r);
	test_mock_register(2, leaf_p);
	test_mock_register(2, leaf_r);

	bt_cache_hit = 11;
	bt_cache_miss = 22;
	bt_split = 33;
	bt_rootsplit = 44;
	bt_sortsplit = 55;
	bt_pfxsaved = 66;

	cp.start();
	P::__bt_dump(&db_p);
	cp.stop();
	cr.start();
	ref___bt_dump(&db_r);
	cr.stop();
	check_eq(F_BT_DUMP, std::strcmp(cp.buf, cr.buf) == 0, "dump stderr");

	test_mock_seq_begin(P_ROOT);
	csp.start();
	P::__bt_stat(&db_p);
	csp.stop();
	csr.start();
	ref___bt_stat(&db_r);
	csr.stop();
	check_eq(F_BT_STAT, std::strcmp(csp.buf, csr.buf) == 0, "stat stderr");
}

void test_defcmp_edges(void)
{
	u_char empty[1];
	u_char a1[1] = { 0 };
	u_char b1[1] = { 0xff };
	u_char eq[4] = { 0x80, 0x81, 0x82, 0x83 };
	u_char lt[4] = { 0x80, 0x81, 0x82, 0x00 };
	u_char gt[4] = { 0x80, 0x81, 0x82, 0xff };
	check_defcmp(empty, 0, empty, 0);
	check_defcmp(a1, 1, b1, 1);
	check_defcmp(eq, 4, eq, 4);
	check_defcmp(lt, 4, gt, 4);
	check_defcmp(eq, 3, eq, 4);
	check_defcmp(eq, 4, eq, 3);
}

void test_defpfx_edges(void)
{
	u_char a[4] = { 1, 2, 3, 4 };
	u_char b[4] = { 1, 2, 9, 4 };
	u_char c[2] = { 1, 2 };
	u_char d[6] = { 1, 2, 3, 4, 5, 6 };
	check_defpfx(a, 4, a, 4);
	check_defpfx(a, 4, b, 4);
	check_defpfx(a, 4, c, 2);
	check_defpfx(c, 2, d, 6);
	check_defpfx(empty, 0, empty, 0);
}

void test_bt_cmp_edges(void)
{
	u_char key[8] = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87 };
	check_bt_cmp_internal0();
	check_bt_cmp_leaf(4, 0, key, 8, 0);
	check_bt_cmp_leaf(4, 2, key, 4, 0);
	check_bt_cmp_leaf(6, 5, key, 1, 0);
	check_bt_cmp_bigkey(key, 16);
}

void test_bt_ret_edges(void)
{
	check_bt_ret(0, 0, 0, 4, 6, 0, 0);
	check_bt_ret(1, 0, 0, 4, 6, 0, 0);
	check_bt_ret(0, B_DB_LOCK, 0, 8, 0, 0, 0);
	check_bt_ret(0, 0, 0, 0, 4, 0, 0);
	check_bt_ret(0, 0, 0, 4, 0, 1, 0);
	check_bt_ret(0, 0, 0, 4, 6, 0, 1);
	check_bt_ret(0, 0, P_BIGKEY, 12, 4, 0, 0);
	check_bt_ret(1, 0, P_BIGDATA, 4, 10, 0, 0);
}

void test_ovfl_edges(void)
{
	check_ovfl_get(40, 1, 0, 0);
	check_ovfl_get(40, 10, 0, 0);
	check_ovfl_get(40, 200, 0, 0);
	check_ovfl_get(40, 200, 0, 1);
	check_ovfl_get(40, 50, 1, 0);
	check_ovfl_put(1, 0, 0);
	check_ovfl_put(100, 0, 0);
	check_ovfl_put(300, 0, 0);
	check_ovfl_put(50, 1, 0);
	check_ovfl_put(200, 0, 1);
	check_ovfl_delete(70, 10, 0, 0);
	check_ovfl_delete(70, 200, 0, 0);
	check_ovfl_delete(70, 50, 1, 0);
	check_ovfl_delete(70, 50, 0, 1);
}

void test_search_edges(void)
{
	u_char key[4] = { 0x80, 0x81, 0x82, 0x83 };
	check_bt_search_leaf(4, key, 4, 0, 0);
	check_bt_search_leaf(4, key, 4, B_NODUPS, 0);
	check_bt_search_leaf(4, key, 4, 0, 1);
	check_bt_search_internal(4, key, 4);
	check_bt_search_snext(1);
	check_bt_search_snext(0);
	check_bt_search_sprev(1);
	check_bt_search_sprev(0);
}

void test_debug_edges(void)
{
	check_dpage(P_BLEAF, 4);
	check_dpage(P_BINTERNAL, 3);
	check_dpage(P_RINTERNAL, 2);
	check_dpage(P_RLEAF, 3);
	check_dpage(P_OVERFLOW, 0);
	check_dmpage();
	check_dnpage(15);
	check_dump_stat(R_RECNO);
	check_dump_stat(0);
}

void test_random_sweep(void)
{
	unsigned char abuf[64];
	unsigned char bbuf[64];
	u_char key[16];

	for (unsigned i = 0; i < SWEEP_ITERS; i++) {
		size_t asz = (size_t)(nextr() % 65u);
		size_t bsz = (size_t)(nextr() % 65u);
		for (size_t j = 0; j < asz; j++)
			abuf[j] = rnd_byte();
		for (size_t j = 0; j < bsz; j++)
			bbuf[j] = rnd_byte();
		switch (i % 13u) {
		case 0:
			check_defcmp(abuf, asz, bbuf, bsz);
			break;
		case 1:
			check_defpfx(abuf, asz, bbuf, bsz);
			break;
		case 2:
			for (size_t j = 0; j < sizeof(key); j++)
				key[j] = rnd_byte();
			check_bt_cmp_leaf((int)(nextr() % 6u) + 2,
			    (indx_t)(nextr() % 6u), key,
			    (size_t)(nextr() % 9u) + 1, 0);
			break;
		case 3:
			check_bt_cmp_bigkey(abuf, asz > 0 ? asz : 1);
			break;
		case 4:
			check_bt_ret((int)(nextr() & 1u), 0, 0,
			    (u_int32_t)(nextr() % 16u),
			    (u_int32_t)(nextr() % 16u),
			    (int)(nextr() % 3u == 0),
			    (int)(nextr() % 5u == 0));
			break;
		case 5:
			check_ovfl_get((pgno_t)(40 + (nextr() % 20u)),
			    (size_t)(nextr() % 250u) + 1,
			    (int)(nextr() % 17u == 0),
			    (int)(nextr() & 1u));
			break;
		case 6:
			check_ovfl_put((size_t)(nextr() % 350u) + 1,
			    (int)(nextr() % 31u == 0),
			    (int)(nextr() % 23u == 0 ? 1 : 0));
			break;
		case 7:
			check_ovfl_delete((pgno_t)(70 + (nextr() % 10u)),
			    (size_t)(nextr() % 220u) + 1,
			    (int)(nextr() & 3u) == 0,
			    (int)(nextr() % 29u == 0));
			break;
		case 8:
			for (size_t j = 0; j < sizeof(key); j++)
				key[j] = rnd_byte();
			check_bt_search_leaf((int)(nextr() % 6u) + 2, key,
			    (size_t)(nextr() % 9u) + 1,
			    (nextr() & 1u) ? B_NODUPS : 0,
			    (int)(nextr() % 37u == 0));
			break;
		case 9:
			for (size_t j = 0; j < sizeof(key); j++)
				key[j] = rnd_byte();
			check_bt_search_internal((int)(nextr() % 6u) + 2, key,
			    (size_t)(nextr() % 9u) + 1);
			break;
		case 10:
			check_bt_search_snext((int)(nextr() & 1u));
			break;
		case 11:
			check_dpage((nextr() & 3u) == 0 ? P_BLEAF :
			    (nextr() & 3u) == 1 ? P_BINTERNAL :
			    (nextr() & 3u) == 2 ? P_RINTERNAL : P_RLEAF,
			    (int)(nextr() % 4u) + 1);
			break;
		case 12:
			if ((i & 0x3ffu) == 0)
				check_dump_stat((nextr() & 1u) ? R_RECNO : 0);
			else
				check_dnpage((pgno_t)(10 + (nextr() % 50u)));
			break;
		default:
			break;
		}
	}
}

} // namespace

int main(void)
{
	test_defcmp_edges();
	test_defpfx_edges();
	test_bt_cmp_edges();
	test_bt_ret_edges();
	test_ovfl_edges();
	test_search_edges();
	test_debug_edges();
	test_random_sweep();

	std::printf("\n%-14s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < F_COUNT; i++)
		std::printf("%-14s %12lu %12lu\n", fn_name[i], n_cases[i],
		    n_fails[i]);

	unsigned long total_fails = 0;
	for (int i = 0; i < F_COUNT; i++)
		total_fails += n_fails[i];
	return total_fails == 0 ? 0 : 1;
}
