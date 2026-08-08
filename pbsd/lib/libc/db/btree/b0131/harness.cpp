/*
 * Differential harness for batch b0131: btree page/get/close/conv routines.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

import pbsd.lib.libc.db.btree.b0131;

namespace P = pbsd::lib_libc_db_btree::b0131;

extern "C" {
typedef uint32_t pgno_t;
typedef uint16_t indx_t;
typedef unsigned int u_int;
typedef unsigned char u_char;

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

typedef struct _cursor {
	struct {
		pgno_t pgno;
		indx_t index;
	} pg;
	DBT key;
	uint32_t rcursor;
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
	struct {
		pgno_t pgno;
		indx_t index;
	} bt_stack[50];
	struct {
		pgno_t pgno;
		indx_t index;
	} *bt_sp;
	DBT bt_rkey;
	DBT bt_rdata;
	int bt_fd;
	pgno_t bt_free;
	uint32_t bt_psize;
	uint16_t bt_ovflsize;
	int bt_lorder;
	int bt_order;
	struct {
		pgno_t pgno;
		indx_t index;
	} bt_last;
	void *bt_cmp;
	void *bt_pfx;
	void *bt_irec;
	void *bt_rfp;
	int bt_rfd;
	char *bt_cmap;
	char *bt_smap;
	char *bt_emap;
	size_t bt_msize;
	uint32_t bt_nrecs;
	size_t bt_reclen;
	uint8_t bt_bval;
	uint32_t flags;
} BTREE;

typedef struct {
	int put_ret;
	unsigned put_calls;
	void *last_put_page;
	unsigned last_put_flags;
	int get_force_null;
	unsigned get_calls;
	pgno_t get_last_pgno;
	unsigned get_last_flags;
	int new_force_null;
	unsigned new_calls;
	pgno_t new_pgno;
	void *new_page;
	int sync_ret;
	unsigned sync_calls;
	int close_ret;
	unsigned close_calls;
	int search_force_null;
	unsigned search_calls;
	int search_exact;
	int ret_status;
	unsigned ret_calls;
	unsigned char ret_data[256];
	size_t ret_data_size;
	int close_fd_ret;
	unsigned close_fd_calls;
	int close_fd_arg;
} test_mock_state;

extern test_mock_state test_mock;

void test_mock_reset(void);

int ref___bt_free(BTREE *, PAGE *);
PAGE *ref___bt_new(BTREE *, pgno_t *);
int ref___bt_get(const DB *, const DBT *, DBT *, u_int);
int ref___bt_close(DB *);
int ref___bt_sync(const DB *, u_int);
void ref___bt_pgin(void *, pgno_t, void *);
void ref___bt_pgout(void *, pgno_t, void *);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr size_t PAGE_SZ = 8192;
constexpr unsigned SWEEP_ITERS = 200000;

#define P_INVALID 0
#define P_META 0
#define P_BINTERNAL 0x01
#define P_BLEAF 0x02
#define P_TYPE 0x1f
#define P_BIGKEY 0x02
#define P_BIGDATA 0x01
#define B_METADIRTY 0x00002
#define B_MODIFIED 0x00004
#define B_NEEDSWAP 0x00008
#define B_RDONLY 0x00010
#define B_INMEM 0x00001
#define B_DB_LOCK 0x04000
#define B_NODUPS 0x00020
#define R_RECNO 0x00080
#define RET_ERROR -1
#define RET_SUCCESS 0
#define RET_SPECIAL 1
#define MPOOL_DIRTY 0x01

#define BTDATAOFF \
	(sizeof(pgno_t) + sizeof(pgno_t) + sizeof(pgno_t) + \
	    sizeof(u_int32_t) + sizeof(indx_t) + sizeof(indx_t))

enum Fn {
	F_BT_FREE,
	F_BT_NEW,
	F_BT_GET,
	F_BT_CLOSE,
	F_BT_SYNC,
	F_BT_PGIN,
	F_BT_PGOUT,
	F_COUNT
};

const char *fn_name[F_COUNT] = {
	"__bt_free",
	"__bt_new",
	"__bt_get",
	"__bt_close",
	"__bt_sync",
	"__bt_pgin",
	"__bt_pgout",
};

unsigned long n_cases[F_COUNT];
unsigned long n_fails[F_COUNT];
unsigned reported[F_COUNT];

uint64_t rng = 0xb0131b0131b0131ULL;

uint64_t
nextr(void)
{
	rng ^= rng << 13;
	rng ^= rng >> 7;
	rng ^= rng << 17;
	return rng;
}

void
fail(int fn, const char *msg)
{
	n_fails[fn]++;
	if (reported[fn] < 10) {
		reported[fn]++;
		std::fprintf(stderr, "FAIL %s: %s\n", fn_name[fn], msg);
	}
}

void
case_ok(int fn)
{
	n_cases[fn]++;
}

void
check_eq(int fn, bool ok, const char *msg)
{
	n_cases[fn]++;
	if (!ok)
		fail(fn, msg);
}

void
guard_fill(void *p, size_t n)
{
	std::memset(p, GUARD, n);
}

bool
bufs_eq(const unsigned char *a, const unsigned char *b, size_t n)
{
	return std::memcmp(a, b, n) == 0;
}

void
write_u32_be(unsigned char *p, uint32_t v)
{
	p[0] = (unsigned char)(v >> 24);
	p[1] = (unsigned char)(v >> 16);
	p[2] = (unsigned char)(v >> 8);
	p[3] = (unsigned char)v;
}

void
write_u16_be(unsigned char *p, uint16_t v)
{
	p[0] = (unsigned char)(v >> 8);
	p[1] = (unsigned char)v;
}

uint32_t
rnd_u32(void)
{
	return (uint32_t)nextr();
}

u_char
rnd_byte(void)
{
	return (u_char)(nextr() & 0xffu);
}

size_t
align_pg(size_t n)
{
	return (n + sizeof(pgno_t) - 1) & ~(sizeof(pgno_t) - 1);
}

/*
 * Build a btree internal page in big-endian (disk) layout with nents entries.
 * flags[i] is the BINTERNAL flags byte for entry i (may include P_BIGKEY).
 */
void
build_binternal_be(unsigned char *buf, size_t bufsz, int nents,
    const u_char *entry_flags, const uint32_t *ksizes)
{
	PAGE *pg;
	size_t off;
	int i;

	guard_fill(buf, bufsz);
	pg = (PAGE *)buf;
	write_u32_be((unsigned char *)&pg->pgno, rnd_u32());
	write_u32_be((unsigned char *)&pg->prevpg, rnd_u32());
	write_u32_be((unsigned char *)&pg->nextpg, rnd_u32());
	write_u32_be((unsigned char *)&pg->flags, P_BINTERNAL);
	off = bufsz;
	for (i = nents - 1; i >= 0; i--) {
		size_t ksz, pos;
		unsigned char *e;

		ksz = ksizes ? ksizes[i] : (size_t)(rnd_u32() % 16u);
		off -= align_pg(sizeof(uint32_t) + sizeof(pgno_t) + sizeof(u_char) +
		    ((entry_flags[i] & P_BIGKEY) ?
			(sizeof(uint32_t) + sizeof(pgno_t)) : 0) + ksz);
		e = buf + off;
		write_u32_be(e, (uint32_t)ksz);
		write_u32_be(e + 4, rnd_u32());
		e[8] = entry_flags[i];
		pos = 9;
		if (entry_flags[i] & P_BIGKEY) {
			write_u32_be(e + pos, rnd_u32());
			pos += 4;
			write_u32_be(e + pos, rnd_u32());
			pos += 4;
		}
		for (size_t k = 0; k < ksz; k++)
			e[pos + k] = rnd_byte();
		write_u16_be((unsigned char *)&pg->linp[i], (uint16_t)off);
	}
	write_u16_be((unsigned char *)&pg->lower, (uint16_t)(BTDATAOFF + nents * sizeof(indx_t)));
	write_u16_be((unsigned char *)&pg->upper, (uint16_t)off);
}

void
build_bleaf_be(unsigned char *buf, size_t bufsz, int nents,
    const u_char *entry_flags, const uint32_t *ksizes, const uint32_t *dsizes)
{
	PAGE *pg;
	size_t off;
	int i;

	guard_fill(buf, bufsz);
	pg = (PAGE *)buf;
	write_u32_be((unsigned char *)&pg->pgno, rnd_u32());
	write_u32_be((unsigned char *)&pg->prevpg, rnd_u32());
	write_u32_be((unsigned char *)&pg->nextpg, rnd_u32());
	write_u32_be((unsigned char *)&pg->flags, P_BLEAF);
	off = bufsz;
	for (i = nents - 1; i >= 0; i--) {
		size_t ksz, dsz, pos, esz;
		unsigned char *e;
		u_char fl;

		ksz = ksizes ? ksizes[i] : (size_t)(rnd_u32() % 12u);
		dsz = dsizes ? dsizes[i] : (size_t)(rnd_u32() % 12u);
		fl = entry_flags[i];
		esz = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(u_char);
		if (fl & (P_BIGKEY | P_BIGDATA)) {
			if (fl & P_BIGKEY)
				esz += sizeof(uint32_t) + sizeof(pgno_t);
			if (fl & P_BIGDATA)
				esz += sizeof(uint32_t) + sizeof(pgno_t);
		}
		esz += ksz + dsz;
		off -= align_pg(esz);
		e = buf + off;
		write_u32_be(e, (uint32_t)ksz);
		write_u32_be(e + 4, (uint32_t)dsz);
		e[8] = fl;
		pos = 9;
		if (fl & (P_BIGKEY | P_BIGDATA)) {
			if (fl & P_BIGKEY) {
				write_u32_be(e + pos, rnd_u32());
				pos += 4;
				write_u32_be(e + pos, rnd_u32());
				pos += 4;
			}
			if (fl & P_BIGDATA) {
				write_u32_be(e + pos, rnd_u32());
				pos += 4;
				write_u32_be(e + pos, rnd_u32());
				pos += 4;
			}
		}
		for (size_t k = 0; k < ksz + dsz; k++)
			e[pos + k] = rnd_byte();
		write_u16_be((unsigned char *)&pg->linp[i], (uint16_t)off);
	}
	write_u16_be((unsigned char *)&pg->lower, (uint16_t)(BTDATAOFF + nents * sizeof(indx_t)));
	write_u16_be((unsigned char *)&pg->upper, (uint16_t)off);
}

void
build_meta_be(unsigned char *buf, size_t bufsz)
{
	guard_fill(buf, bufsz);
	write_u32_be(buf + 0, 0x053162);
	write_u32_be(buf + 4, 3);
	write_u32_be(buf + 8, 4096);
	write_u32_be(buf + 12, 7);
	write_u32_be(buf + 16, 99);
	write_u32_be(buf + 20, B_NODUPS);
}

struct TreePort {
	P::BTREE t;
	MPOOL mp;
	P::DB db;
	unsigned char pagebuf[PAGE_SZ];
};

struct TreeRef {
	BTREE t;
	MPOOL mp;
	DB db;
	unsigned char pagebuf[PAGE_SZ];
};

void
init_tree_port(TreePort &c, u_int32_t flags)
{
	std::memset(&c, 0, sizeof(c));
	c.t.bt_mp = &c.mp;
	c.t.flags = flags;
	c.db.internal = &c.t;
}

void
init_tree_ref(TreeRef &c, u_int32_t flags)
{
	std::memset(&c, 0, sizeof(c));
	c.t.bt_mp = &c.mp;
	c.t.flags = flags;
	c.db.internal = &c.t;
}

void
check_bt_free(int put_ret)
{
	TreePort pc;
	TreeRef rc;
	P::PAGE *ph = (P::PAGE *)pc.pagebuf;
	PAGE *rh = (PAGE *)rc.pagebuf;

	test_mock_reset();
	test_mock.put_ret = put_ret;
	init_tree_port(pc, 0);
	init_tree_ref(rc, 0);
	guard_fill(pc.pagebuf, sizeof(pc.pagebuf));
	guard_fill(rc.pagebuf, sizeof(rc.pagebuf));
	ph->pgno = 42;
	ph->prevpg = 11;
	ph->nextpg = 22;
	rh->pgno = 42;
	rh->prevpg = 11;
	rh->nextpg = 22;
	pc.t.bt_free = 7;
	rc.t.bt_free = 7;

	int rp = P::__bt_free(&pc.t, ph);
	int rr = ref___bt_free(&rc.t, rh);

	char msg[256];
	std::snprintf(msg, sizeof(msg), "ret port=%d ref=%d", rp, rr);
	check_eq(F_BT_FREE, rp == rr, msg);
	check_eq(F_BT_FREE, ph->prevpg == rh->prevpg && ph->nextpg == rh->nextpg,
	    "page links");
	check_eq(F_BT_FREE, pc.t.bt_free == rc.t.bt_free, "bt_free");
	check_eq(F_BT_FREE, (pc.t.flags & B_METADIRTY) == (rc.t.flags & B_METADIRTY),
	    "B_METADIRTY");
	check_eq(F_BT_FREE, bufs_eq(pc.pagebuf, rc.pagebuf, PAGE_SZ), "pagebuf");
}

void
check_bt_new(int freelist, int get_null, int new_null)
{
	TreePort pc;
	TreeRef rc;
	P::PAGE *fp = (P::PAGE *)pc.pagebuf;
	PAGE *fr = (PAGE *)rc.pagebuf;
	pgno_t npg_p = 0, npg_r = 0;

	test_mock_reset();
	test_mock.new_page = fr;
	test_mock.new_pgno = 55;
	test_mock.get_force_null = get_null;
	test_mock.new_force_null = new_null;
	init_tree_port(pc, 0);
	init_tree_ref(rc, 0);
	fp->nextpg = 88;
	fr->nextpg = 88;
	pc.t.bt_free = freelist ? 5 : P_INVALID;
	rc.t.bt_free = freelist ? 5 : P_INVALID;

	P::PAGE *pp = P::__bt_new(&pc.t, &npg_p);
	PAGE *pr = ref___bt_new(&rc.t, &npg_r);

	char msg[256];
	if (pp == nullptr && pr == nullptr) {
		check_eq(F_BT_NEW, true, "both null");
	} else if (pp == nullptr || pr == nullptr) {
		check_eq(F_BT_NEW, false, "null mismatch");
	} else {
		std::snprintf(msg, sizeof(msg), "offset port=%td ref=%td",
		    (char *)pp - (char *)pc.pagebuf,
		    (char *)pr - (char *)rc.pagebuf);
		check_eq(F_BT_NEW,
		    (pp - (P::PAGE *)pc.pagebuf) == (pr - (PAGE *)rc.pagebuf),
		    msg);
	}
	std::snprintf(msg, sizeof(msg), "npg port=%u ref=%u", (unsigned)npg_p,
	    (unsigned)npg_r);
	check_eq(F_BT_NEW, npg_p == npg_r, msg);
	check_eq(F_BT_NEW, pc.t.bt_free == rc.t.bt_free, "bt_free");
	check_eq(F_BT_NEW, (pc.t.flags & B_METADIRTY) == (rc.t.flags & B_METADIRTY),
	    "B_METADIRTY");
}

void
check_bt_get(u_int flags, int had_pinned, int search_null, int exact,
    u_int32_t tflags, int ret_stat)
{
	TreePort pc;
	TreeRef rc;
	unsigned char pagebuf_r[PAGE_SZ];
	P::DBT key, data_p, data_r;
	unsigned char keybuf[8];
	int ep = 0, er = 0;

	test_mock_reset();
	test_mock.search_force_null = search_null;
	test_mock.search_exact = exact;
	test_mock.ret_status = ret_stat;
	test_mock.ret_data_size = 4;
	test_mock.ret_data[0] = 0xde;
	test_mock.ret_data[1] = 0xad;
	test_mock.ret_data[2] = 0xbe;
	test_mock.ret_data[3] = 0xef;
	init_tree_port(pc, tflags);
	init_tree_ref(rc, tflags);
	test_mock.new_page = pagebuf_r;
	key.data = keybuf;
	key.size = sizeof(keybuf);
	std::memset(keybuf, 0xa5, sizeof(keybuf));
	if (had_pinned) {
		pc.t.bt_pinned = (P::PAGE *)pc.pagebuf;
		rc.t.bt_pinned = (PAGE *)rc.pagebuf;
	}

	errno = 0;
	int rp = P::__bt_get(&pc.db, &key, &data_p, flags);
	int save_errno = errno;
	errno = 0;
	int rr = ref___bt_get(&rc.db, &key, &data_r, flags);
	char msg[256];
	std::snprintf(msg, sizeof(msg),
	    "ret port=%d ref=%d flags=%u exact=%d errno_p=%d errno_r=%d",
	    rp, rr, flags, exact, save_errno, errno);
	check_eq(F_BT_GET, rp == rr, msg);
	check_eq(F_BT_GET, pc.t.bt_pinned == rc.t.bt_pinned ||
	    ((pc.t.bt_pinned == nullptr) == (rc.t.bt_pinned == nullptr)),
	    "bt_pinned");
	if (rp == RET_SUCCESS) {
		check_eq(F_BT_GET, data_p.size == data_r.size, "data.size");
		if (data_p.size == data_r.size && data_p.size > 0)
			check_eq(F_BT_GET,
			    std::memcmp(data_p.data, data_r.data, data_p.size) == 0,
			    "data bytes");
	}
}

void
check_bt_sync(u_int flags, u_int32_t tflags, int meta_null, int sync_ret)
{
	TreePort pc;
	TreeRef rc;

	test_mock_reset();
	test_mock.get_force_null = meta_null;
	test_mock.sync_ret = sync_ret;
	init_tree_port(pc, tflags);
	init_tree_ref(rc, tflags);
	pc.t.bt_psize = 512;
	rc.t.bt_psize = 512;
	pc.t.bt_free = 3;
	rc.t.bt_free = 3;
	pc.t.bt_nrecs = 100;
	rc.t.bt_nrecs = 100;

	int rp = P::__bt_sync(&pc.db, flags);
	int rr = ref___bt_sync(&rc.db, flags);
	char msg[256];
	std::snprintf(msg, sizeof(msg), "ret port=%d ref=%d flags=%u tflags=0x%x",
	    rp, rr, flags, tflags);
	check_eq(F_BT_SYNC, rp == rr, msg);
	check_eq(F_BT_SYNC, (pc.t.flags & B_MODIFIED) == (rc.t.flags & B_MODIFIED),
	    "B_MODIFIED");
}

void
check_bt_close(int with_allocs, int close_fd_ret, int sync_meta_null)
{
	TreePort pc;
	TreeRef rc;
	char *cp, *cr;

	test_mock_reset();
	test_mock.get_force_null = sync_meta_null;
	test_mock.close_fd_ret = close_fd_ret;
	init_tree_port(pc, B_MODIFIED | B_METADIRTY);
	init_tree_ref(rc, B_MODIFIED | B_METADIRTY);
	pc.t.bt_fd = 9;
	rc.t.bt_fd = 9;
	if (with_allocs) {
		cp = (char *)std::malloc(32);
		cr = (char *)std::malloc(32);
		pc.t.bt_cursor.key.data = cp;
		pc.t.bt_cursor.key.size = 32;
		rc.t.bt_cursor.key.data = cr;
		rc.t.bt_cursor.key.size = 32;
		pc.t.bt_rkey.data = std::malloc(16);
		pc.t.bt_rkey.size = 16;
		rc.t.bt_rkey.data = std::malloc(16);
		rc.t.bt_rkey.size = 16;
		pc.t.bt_rdata.data = std::malloc(64);
		pc.t.bt_rdata.size = 64;
		rc.t.bt_rdata.data = std::malloc(64);
		rc.t.bt_rdata.size = 64;
	}

	P::DB *dbp_p = &pc.db;
	DB *dbp_r = &rc.db;
	int rp = P::__bt_close(dbp_p);
	int rr = ref___bt_close(dbp_r);
	char msg[256];
	std::snprintf(msg, sizeof(msg), "ret port=%d ref=%d close_fd=%d", rp, rr,
	    close_fd_ret);
	check_eq(F_BT_CLOSE, rp == rr, msg);
}

template<typename Builder>
void
check_conv(const char *which, void (*port_fn)(void *, pgno_t, void *),
    void (*ref_fn)(void *, pgno_t, void *), u_int32_t tflags, pgno_t pg,
    Builder builder)
{
	unsigned char buf_p[PAGE_SZ];
	unsigned char buf_r[PAGE_SZ];
	P::BTREE tp;
	BTREE tr;

	std::memset(&tp, 0, sizeof(tp));
	std::memset(&tr, 0, sizeof(tr));
	tp.flags = tflags;
	tr.flags = tflags;
	builder(buf_p, PAGE_SZ);
	builder(buf_r, PAGE_SZ);

	port_fn(&tp, pg, buf_p);
	ref_fn(&tr, pg, buf_r);

	int fn = (which[0] == 'p' && which[4] == 'i') ? F_BT_PGIN : F_BT_PGOUT;
	if (!bufs_eq(buf_p, buf_r, PAGE_SZ)) {
		char msg[128];
		std::snprintf(msg, sizeof(msg), "%s pg=%u tflags=0x%x mismatch",
		    which, (unsigned)pg, tflags);
		check_eq(fn, false, msg);
	} else
		case_ok(fn);
}

void
test_bt_free_edges(void)
{
	check_bt_free(RET_SUCCESS);
	check_bt_free(RET_ERROR);
}

void
test_bt_new_edges(void)
{
	check_bt_new(1, 0, 0);
	check_bt_new(1, 1, 0);
	check_bt_new(0, 0, 0);
	check_bt_new(0, 0, 1);
	check_bt_new(1, 1, 1);
}

void
test_bt_get_edges(void)
{
	check_bt_get(1, 0, 0, 0, 0, RET_SUCCESS);
	check_bt_get(0, 0, 1, 0, 0, RET_SUCCESS);
	check_bt_get(0, 0, 0, 0, 0, RET_SUCCESS);
	check_bt_get(0, 0, 0, 1, 0, RET_SUCCESS);
	check_bt_get(0, 0, 0, 1, B_DB_LOCK, RET_SUCCESS);
	check_bt_get(0, 1, 0, 1, 0, RET_SUCCESS);
	check_bt_get(0, 0, 0, 1, 0, RET_ERROR);
}

void
test_bt_sync_edges(void)
{
	check_bt_sync(1, 0, 0, RET_SUCCESS);
	check_bt_sync(0, B_INMEM, 0, RET_SUCCESS);
	check_bt_sync(0, B_RDONLY, 0, RET_SUCCESS);
	check_bt_sync(0, 0, 0, RET_SUCCESS);
	check_bt_sync(0, B_MODIFIED, 0, RET_SUCCESS);
	check_bt_sync(0, B_METADIRTY, 0, RET_SUCCESS);
	check_bt_sync(0, B_MODIFIED | B_METADIRTY, 0, RET_SUCCESS);
	check_bt_sync(0, B_MODIFIED | B_METADIRTY, 1, RET_SUCCESS);
	check_bt_sync(0, B_MODIFIED | B_METADIRTY, 0, RET_ERROR);
}

void
test_bt_close_edges(void)
{
	check_bt_close(0, 0, 0);
	check_bt_close(1, 0, 0);
	check_bt_close(0, 1, 0);
	check_bt_close(1, 1, 1);
}

void
test_conv_edges(void)
{
	static u_char bif[] = { 0, P_BIGKEY, 0 };
	static u_char blf[] = { 0, P_BIGKEY | P_BIGDATA, P_BIGDATA };
	static uint32_t ks[] = { 0, 4, 8 };
	static uint32_t ds[] = { 0, 3, 6 };

	check_conv("__bt_pgin", P::__bt_pgin, ref___bt_pgin, 0, 2,
	    [](unsigned char *b, size_t n) {
		    build_binternal_be(b, PAGE_SZ, 1, bif, ks);
	    });
	check_conv("__bt_pgin", P::__bt_pgin, ref___bt_pgin, B_NEEDSWAP, P_META,
	    [](unsigned char *b, size_t n) { build_meta_be(b, n); });
	check_conv("__bt_pgin", P::__bt_pgin, ref___bt_pgin, B_NEEDSWAP, 3,
	    [](unsigned char *b, size_t n) {
		    build_binternal_be(b, n, 3, bif, ks);
	    });
	check_conv("__bt_pgin", P::__bt_pgin, ref___bt_pgin, B_NEEDSWAP, 4,
	    [](unsigned char *b, size_t n) {
		    build_bleaf_be(b, n, 3, blf, ks, ds);
	    });
	check_conv("__bt_pgout", P::__bt_pgout, ref___bt_pgout, B_NEEDSWAP, P_META,
	    [](unsigned char *b, size_t n) { build_meta_be(b, n); });
	check_conv("__bt_pgout", P::__bt_pgout, ref___bt_pgout, B_NEEDSWAP, 5,
	    [](unsigned char *b, size_t n) {
		    build_binternal_be(b, n, 2, bif, ks);
	    });
	check_conv("__bt_pgout", P::__bt_pgout, ref___bt_pgout, B_NEEDSWAP, 6,
	    [](unsigned char *b, size_t n) {
		    build_bleaf_be(b, n, 2, blf, ks, ds);
	    });
}

void
test_random_sweep(void)
{
	u_char flbuf[8];
	uint32_t ks[8], ds[8];

	for (unsigned i = 0; i < SWEEP_ITERS; i++) {
		switch (i % 7u) {
		case 0:
			check_bt_free((nextr() & 1u) ? RET_SUCCESS : RET_ERROR);
			break;
		case 1:
			check_bt_new((int)(nextr() & 1u), (int)(nextr() & 1u),
			    (int)(nextr() & 1u));
			break;
		case 2:
			check_bt_get((u_int)(nextr() & 1u), (int)(nextr() & 1u),
			    (int)(nextr() & 1u), (int)(nextr() & 1u),
			    (nextr() & 1u) ? B_DB_LOCK : 0,
			    (nextr() & 1u) ? RET_ERROR : RET_SUCCESS);
			break;
		case 3:
			check_bt_sync((u_int)(nextr() & 1u),
			    (u_int32_t)(nextr() & (B_INMEM | B_RDONLY | B_MODIFIED |
				B_METADIRTY)),
			    (int)(nextr() & 1u),
			    (nextr() & 1u) ? RET_ERROR : RET_SUCCESS);
			break;
		case 4: {
			int alloc = (int)(nextr() & 1u);
			if (alloc)
				check_bt_close(alloc, (int)(nextr() & 1u),
				    (int)(nextr() & 1u));
			break;
		}
		case 5:
		case 6: {
			int nent = (int)(nextr() % 5u) + 1;
			int is_leaf = (int)(i & 1u);
			for (int e = 0; e < nent; e++) {
				flbuf[e] = (u_char)(nextr() & (P_BIGKEY | P_BIGDATA));
				ks[e] = (uint32_t)(nextr() % 20u);
				ds[e] = (uint32_t)(nextr() % 20u);
			}
			if (is_leaf) {
				check_conv("__bt_pgin", P::__bt_pgin, ref___bt_pgin,
				    B_NEEDSWAP, (pgno_t)(2 + nextr() % 100u),
				    [&](unsigned char *b, size_t n) {
					    build_bleaf_be(b, n, nent, flbuf, ks, ds);
				    });
				check_conv("__bt_pgout", P::__bt_pgout, ref___bt_pgout,
				    B_NEEDSWAP, (pgno_t)(2 + nextr() % 100u),
				    [&](unsigned char *b, size_t n) {
					    build_bleaf_be(b, n, nent, flbuf, ks, ds);
				    });
			} else {
				check_conv("__bt_pgin", P::__bt_pgin, ref___bt_pgin,
				    B_NEEDSWAP, (pgno_t)(2 + nextr() % 100u),
				    [&](unsigned char *b, size_t n) {
					    build_binternal_be(b, n, nent, flbuf, ks);
				    });
				check_conv("__bt_pgout", P::__bt_pgout, ref___bt_pgout,
				    B_NEEDSWAP, (pgno_t)(2 + nextr() % 100u),
				    [&](unsigned char *b, size_t n) {
					    build_binternal_be(b, n, nent, flbuf, ks);
				    });
			}
			break;
		}
		default:
			break;
		}
	}
}

} // namespace

int
main(void)
{
	test_bt_free_edges();
	test_bt_new_edges();
	test_bt_get_edges();
	test_bt_sync_edges();
	test_bt_close_edges();
	test_conv_edges();
	test_random_sweep();

	std::printf("\n%-12s %12s %12s\n", "function", "cases", "failures");
	for (int i = 0; i < F_COUNT; i++)
		std::printf("%-12s %12lu %12lu\n", fn_name[i], n_cases[i],
		    n_fails[i]);

	unsigned long total_fails = 0;
	for (int i = 0; i < F_COUNT; i++)
		total_fails += n_fails[i];
	return total_fails == 0 ? 0 : 1;
}
