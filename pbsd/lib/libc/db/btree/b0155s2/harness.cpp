/*
 * Differential harness for batch b0155s2: btree open routines (bt_open.c).
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

import pbsd.lib.libc.db.btree.b0155s2;

namespace P = pbsd::lib_libc_db_btree::b0155s2;

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
	int (*put)(struct __db *, DBT *, const DBT *, unsigned int);
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
	unsigned open_fd_calls, fstat_calls, read_calls, close_calls;
	unsigned mkostemp_calls, getenv_calls, sigmask_calls;
	unsigned calloc_calls;
	int get_force_null, new_force_null;
	int open_ret, fstat_ret, read_ret, close_ret, mkostemp_ret;
	int calloc_fail_after;
	pgno_t get_last_pgno, new_pgno_seq;
	unsigned get_last_flags;
	void *last_put_page;
	int nreg;
	pgno_t reg_pgno[64];
	void *reg_page[64];
	int new_fail_after;
	int delete_ret;
	char *getenv_val;
	struct stat fstat_sb;
	unsigned char read_buf[512];
	ssize_t read_ret_val;
	MPOOL *mpool_open_ret;
} test_mock_state;

extern test_mock_state test_mock;

void test_mock_reset(void);
void test_mock_register(pgno_t pgno, void *page);

int ref_byteorder(void);
int ref_tmp(void);
int ref_nroot(BTREE *);
int ref___bt_fd(const DB *);
DB *ref___bt_open(const char *, int, int, const BTREEINFO *, int);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr size_t PAGE_SZ = 512;
constexpr unsigned SWEEP_ITERS = 200000;

enum { NOT = 0, BACK = 1, FORWARD = 2 };

#define P_INVALID 0
#define P_ROOT 1
#define B_INMEM 0x00001
#define B_NODUPS 0x00020
#define B_DB_LOCK 0x04000
#define B_DB_SHMEM 0x08000
#define B_DB_TXN 0x10000
#define R_DUP 0x01
#define RET_ERROR -1
#define RET_SUCCESS 0
#define BTREEMAGIC 0x053162
#define BTREEVERSION 3
#define BIG_ENDIAN 4321
#define LITTLE_ENDIAN 1234
#define DB_LOCK 0x20000000
#define DB_SHMEM 0x40000000
#define DB_TXN 0x80000000

#define BTDATAOFF \
	(sizeof(pgno_t) + sizeof(pgno_t) + sizeof(pgno_t) + \
	    sizeof(u_int32_t) + sizeof(indx_t) + sizeof(indx_t))

enum Fn { F_BYTEORDER, F_TMP, F_NROOT, F_BT_FD, F_BT_OPEN, F_COUNT };

const char *fn_name[F_COUNT] = {
	"byteorder", "tmp", "nroot", "__bt_fd", "__bt_open",
};

unsigned long n_cases[F_COUNT];
unsigned long n_fails[F_COUNT];
unsigned reported[F_COUNT];

uint64_t rng = 0xb0155s2b0155s200ULL;

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

void guard_fill(void *p, size_t n) { std::memset(p, GUARD, n); }

struct MockSnap { test_mock_state mock; };
MockSnap snap_mock(void) { return {test_mock}; }
void restore_mock(const MockSnap &s) { test_mock = s.mock; }

struct MockDelta {
	unsigned get, put, new_c, delete_c, open, filter;
	unsigned open_fd, fstat, read, close, mkostemp, calloc, getenv, sigmask;
};

MockDelta mock_delta(const test_mock_state &b, const test_mock_state &a)
{
	return {
		a.get_calls - b.get_calls, a.put_calls - b.put_calls,
		a.new_calls - b.new_calls, a.delete_calls - b.delete_calls,
		a.open_calls - b.open_calls, a.filter_calls - b.filter_calls,
		a.open_fd_calls - b.open_fd_calls, a.fstat_calls - b.fstat_calls,
		a.read_calls - b.read_calls, a.close_calls - b.close_calls,
		a.mkostemp_calls - b.mkostemp_calls, a.calloc_calls - b.calloc_calls,
		a.getenv_calls - b.getenv_calls, a.sigmask_calls - b.sigmask_calls,
	};
}

bool mock_delta_eq(const MockDelta &a, const MockDelta &b)
{
	return a.get == b.get && a.put == b.put && a.new_c == b.new_c &&
	    a.delete_c == b.delete_c && a.open == b.open &&
	    a.filter == b.filter && a.open_fd == b.open_fd &&
	    a.fstat == b.fstat && a.read == b.read && a.close == b.close &&
	    a.mkostemp == b.mkostemp && a.calloc == b.calloc &&
	    a.getenv == b.getenv && a.sigmask == b.sigmask;
}

bool open_mock_delta_eq(const MockDelta &a, const MockDelta &b)
{
	MockDelta ap = a, bp = b;
	ap.calloc = bp.calloc = 0;
	return mock_delta_eq(ap, bp);
}

static int port_cmp(const P::DBT *a, const P::DBT *b)
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

indx_t calc_ovflsize(u_int32_t psize, int minkeypage)
{
	indx_t ov = (indx_t)((psize - BTDATAOFF) / (unsigned)minkeypage - 16);
	if (ov < 28)
		ov = 28;
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
	t.flags = flags;
	db.internal = &t;
}

void free_open_db(P::DB *dbp)
{
	if (!dbp)
		return;
	std::free(dbp->internal);
	std::free(dbp);
}

void free_open_db_ref(DB *dbp)
{
	if (!dbp)
		return;
	std::free(dbp->internal);
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
	std::snprintf(msg, sizeof(msg),
	    "ret port=%d ref=%d mk=%d env=%s errno_p=%d errno_r=%d",
	    rp, rr, mkostemp_ret, tmpdir ? tmpdir : "null", re_p, re_r);
	check_eq(F_TMP, rp == rr && re_p == re_r, msg);
	check_eq(F_TMP, dp.mkostemp == dr.mkostemp, "mkostemp delta");
	check_eq(F_TMP, dp.getenv == dr.getenv, "getenv delta");
	check_eq(F_TMP, dp.sigmask == dr.sigmask, "sigmask delta");
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
	static unsigned char root_p[PAGE_SZ];
	static unsigned char root_r[PAGE_SZ];

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

	static unsigned char init_root[PAGE_SZ];
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
	static unsigned char pin_p[PAGE_SZ];
	static unsigned char pin_r[PAGE_SZ];

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
	std::snprintf(msg, sizeof(msg),
	    "ret port=%d ref=%d inmem=%d pinned=%d errno_p=%d errno_r=%d",
	    rp, rr, inmem, pinned, ep, er);
	check_eq(F_BT_FD, rp == rr && ep == er, msg);
	if (pinned)
		check_eq(F_BT_FD, test_mock.put_calls == put_p, "put on pinned");
}

struct OpenParams {
	int inmem;
	int flags;
	int invalid_psize;
	int bad_lorder;
	int bad_flags;
	int minkeypage;
	int cachesize;
	int existing_meta;
	int swapped_meta;
	int bad_meta_magic;
	int bad_meta_version;
	int bad_meta_psize;
	int bad_meta_flags;
	int open_fail;
	int calloc_fail;
	int mpool_null;
	int fstat_fail;
	int read_fail;
	int short_read;
	int nroot_fail;
	int dflags;
	int blksize;
	int user_psize;
};

void apply_open_mocks(const OpenParams &p)
{
	test_mock_reset();
	test_mock.open_ret = p.open_fail ? -1 : 3;
	test_mock.mkostemp_ret = p.open_fail ? -1 : 4;
	test_mock.calloc_fail_after = p.calloc_fail;
	test_mock.mpool_open_ret = p.mpool_null ? nullptr : (MPOOL *)0x1;
	test_mock.fstat_ret = p.fstat_fail ? EACCES : 0;
	test_mock.read_ret = p.read_fail ? EIO : 0;
	test_mock.fstat_sb.st_blksize = p.blksize > 0 ? p.blksize : 4096;
	test_mock.fstat_sb.st_size = 0;

	if (p.existing_meta || p.swapped_meta || p.bad_meta_magic ||
	    p.bad_meta_version || p.bad_meta_psize || p.bad_meta_flags) {
		u_int32_t magic = BTREEMAGIC;
		u_int32_t version = BTREEVERSION;
		u_int32_t psize = (u_int32_t)PAGE_SZ;
		u_int32_t flags = B_NODUPS;
		if (p.bad_meta_magic)
			magic = 0xdead;
		if (p.bad_meta_version)
			version = 99;
		if (p.bad_meta_psize)
			psize = 511;
		if (p.bad_meta_flags)
			flags = 0x80000000u;
		if (p.swapped_meta) {
			auto swap32 = [](u_int32_t v) {
				return ((v & 0xffu) << 24) | ((v & 0xff00u) << 8) |
				    ((v & 0xff0000u) >> 8) | ((v & 0xff000000u) >> 24);
			};
			magic = swap32(magic);
			version = swap32(version);
			psize = swap32(psize);
			flags = swap32(flags);
		}
		struct {
			u_int32_t magic, version, psize, free, nrecs, flags;
		} m{magic, version, psize, 7, 42, flags};
		test_mock.fstat_sb.st_size = (off_t)sizeof(m);
		test_mock.read_ret_val = p.short_read ?
		    (ssize_t)(sizeof(m) - 1) : (ssize_t)sizeof(m);
		std::memcpy(test_mock.read_buf, &m, sizeof(m));
	}

	if (p.nroot_fail) {
		test_mock.get_force_null = 1;
		test_mock.new_fail_after = 1;
	}
}

P::BTREEINFO make_port_info(const OpenParams &p)
{
	P::BTREEINFO info{};
	if (p.invalid_psize)
		info.psize = 511;
	else if (p.user_psize)
		info.psize = (unsigned)p.user_psize;
	info.lorder = p.bad_lorder ? 9999 : LITTLE_ENDIAN;
	if (p.bad_flags)
		info.flags = 0x02;
	if (p.minkeypage)
		info.minkeypage = p.minkeypage;
	if (p.cachesize)
		info.cachesize = (unsigned)p.cachesize;
	return info;
}

BTREEINFO make_ref_info(const OpenParams &p)
{
	BTREEINFO info{};
	if (p.invalid_psize)
		info.psize = 511;
	else if (p.user_psize)
		info.psize = (unsigned)p.user_psize;
	info.lorder = p.bad_lorder ? 9999 : LITTLE_ENDIAN;
	if (p.bad_flags)
		info.flags = 0x02;
	if (p.minkeypage)
		info.minkeypage = p.minkeypage;
	if (p.cachesize)
		info.cachesize = (unsigned)p.cachesize;
	return info;
}

bool use_openinfo(const OpenParams &p)
{
	return p.invalid_psize || p.bad_lorder || p.bad_flags ||
	    p.minkeypage || p.cachesize || p.user_psize ||
	    p.existing_meta || p.swapped_meta;
}

void check_bt_open(const OpenParams &p)
{
	P::BTREEINFO pinfo = make_port_info(p);
	BTREEINFO rinfo = make_ref_info(p);
	bool has_info = use_openinfo(p);
	const char *fname = p.inmem ? nullptr : "/tmp/x.db";
	int oflags = p.flags ? p.flags : O_RDWR;

	apply_open_mocks(p);
	MockSnap snap = snap_mock();
	errno = 0;
	P::DB *dp_p = P::__bt_open(fname, oflags, 0644,
	    has_info ? &pinfo : nullptr, p.dflags);
	int ep = errno;
	MockDelta dport = mock_delta(snap.mock, test_mock);
	P::BTREE *tp = dp_p ? (P::BTREE *)dp_p->internal : nullptr;
	u_int32_t flags_p = tp ? tp->flags : 0;
	u_int32_t psize_p = tp ? tp->bt_psize : 0;
	indx_t ovfl_p = tp ? tp->bt_ovflsize : 0;
	pgno_t free_p = tp ? tp->bt_free : 0;
	recno_t nrecs_p = tp ? tp->bt_nrecs : 0;
	int fd_p = tp ? tp->bt_fd : -99;
	int lorder_p = tp ? tp->bt_lorder : 0;
	restore_mock(snap);

	apply_open_mocks(p);
	errno = 0;
	DB *dp_r = ref___bt_open(fname, oflags, 0644,
	    has_info ? &rinfo : nullptr, p.dflags);
	int er = errno;
	MockDelta dref = mock_delta(snap.mock, test_mock);
	BTREE *tr = dp_r ? (BTREE *)dp_r->internal : nullptr;

	char msg[256];
	std::snprintf(msg, sizeof(msg),
	    "null port=%d ref=%d inmem=%d errno_p=%d errno_r=%d",
	    dp_p == nullptr, dp_r == nullptr, p.inmem, ep, er);
	bool null_match = (dp_p == nullptr) == (dp_r == nullptr);
	bool errno_match = (dp_p == nullptr) || (ep == er);
	check_eq(F_BT_OPEN, null_match && errno_match, msg);
	check_eq(F_BT_OPEN, open_mock_delta_eq(dport, dref), "mock delta");
	if (dp_p && dp_r) {
		check_eq(F_BT_OPEN, (int)dp_p->type == (int)dp_r->type, "type");
		check_eq(F_BT_OPEN, flags_p == tr->flags, "flags");
		check_eq(F_BT_OPEN, psize_p == tr->bt_psize, "psize");
		check_eq(F_BT_OPEN, ovfl_p == tr->bt_ovflsize, "ovflsize");
		check_eq(F_BT_OPEN, free_p == tr->bt_free, "free");
		check_eq(F_BT_OPEN, nrecs_p == tr->bt_nrecs, "nrecs");
		check_eq(F_BT_OPEN, lorder_p == tr->bt_lorder, "lorder");
		check_eq(F_BT_OPEN, (fd_p < 0) == (tr->bt_fd < 0), "fd sign");
		if (!p.inmem)
			check_eq(F_BT_OPEN, dport.filter == dref.filter, "filter");
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
	check_nroot(0, 0, 0, 0, 0, P_ROOT);
}

void test_bt_fd_edges(void)
{
	check_bt_fd(0, 0);
	check_bt_fd(1, 0);
	check_bt_fd(0, 1);
}

void test_bt_open_edges(void)
{
	check_bt_open({.inmem = 1});
	check_bt_open({.inmem = 0});
	check_bt_open({.invalid_psize = 1});
	check_bt_open({.existing_meta = 1});
	check_bt_open({.open_fail = 1});
	check_bt_open({.calloc_fail = 1});
	check_bt_open({.mpool_null = 1});
	check_bt_open({.inmem = 1, .flags = O_RDONLY});
	check_bt_open({.inmem = 0, .flags = O_WRONLY});
	check_bt_open({.bad_lorder = 1});
	check_bt_open({.bad_flags = 1});
	check_bt_open({.minkeypage = 1});
	check_bt_open({.fstat_fail = 1});
	check_bt_open({.read_fail = 1});
	check_bt_open({.short_read = 1});
	check_bt_open({.bad_meta_magic = 1});
	check_bt_open({.bad_meta_version = 1});
	check_bt_open({.bad_meta_psize = 1});
	check_bt_open({.bad_meta_flags = 1});
	check_bt_open({.swapped_meta = 1});
	check_bt_open({.cachesize = 1000});
	check_bt_open({.cachesize = 513});
	check_bt_open({.blksize = 256});
	check_bt_open({.blksize = 70000});
	check_bt_open({.user_psize = 1024});
	check_bt_open({.dflags = (int)DB_LOCK});
	check_bt_open({.dflags = (int)(DB_LOCK | DB_SHMEM | DB_TXN)});
	check_bt_open({.nroot_fail = 1});
	check_bt_open({.calloc_fail = 2});
	check_bt_open({.inmem = 0, .existing_meta = 1});
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

void sweep_bt_open(void)
{
	for (unsigned i = 0; i < SWEEP_ITERS; i++) {
		OpenParams p{};
		unsigned mode = (unsigned)(nextr() % 37u);
		p.inmem = (int)(mode % 3u == 0);
		p.flags = (mode % 7u == 1) ? O_RDONLY :
		    (mode % 7u == 2) ? O_WRONLY : O_RDWR;
		p.invalid_psize = (int)(mode % 11u == 0);
		p.bad_lorder = (int)(mode % 13u == 0);
		p.bad_flags = (int)(mode % 17u == 0);
		p.minkeypage = (mode % 19u == 0) ? 1 :
		    (mode % 19u == 1) ? 3 : 0;
		p.cachesize = (mode % 5u == 0) ? (int)(100 + (nextr() % 2000u)) : 0;
		p.existing_meta = (int)(mode % 23u == 0);
		p.swapped_meta = (int)(mode % 29u == 0);
		p.bad_meta_magic = (int)(mode % 31u == 0);
		p.bad_meta_version = (int)(mode % 33u == 0);
		p.bad_meta_psize = (int)(mode % 35u == 0);
		p.open_fail = (int)(mode % 41u == 0);
		p.calloc_fail = (int)((nextr() % 4u) + 1);
		if (mode % 43u != 0)
			p.calloc_fail = 0;
		p.mpool_null = (int)(mode % 47u == 0);
		p.fstat_fail = (int)(mode % 53u == 0);
		p.read_fail = (int)(mode % 59u == 0);
		p.short_read = (int)(mode % 61u == 0);
		p.nroot_fail = (int)(mode % 67u == 0);
		p.blksize = (int)(256 + (nextr() % 70000u));
		p.user_psize = (mode % 71u == 0) ? (int)(512 + (nextr() % 4u) * 512) : 0;
		p.dflags = (int)((nextr() & 7u) ? DB_LOCK : 0) |
		    (int)((nextr() & 8u) ? DB_SHMEM : 0) |
		    (int)((nextr() & 16u) ? DB_TXN : 0);
		check_bt_open(p);
	}
}

} // namespace

int main(void)
{
	test_byteorder_edges();
	test_tmp_edges();
	test_nroot_edges();
	test_bt_fd_edges();
	test_bt_open_edges();

	sweep_byteorder();
	sweep_tmp();
	sweep_nroot();
	sweep_bt_fd();
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
