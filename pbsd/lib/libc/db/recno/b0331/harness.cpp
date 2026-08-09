/*
 * Differential harness for PBSD batch b0331 (rec_delete, rec_open, rec_get,
 * rec_put).  Every batch function is exercised against the ref_ oracle.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

import pbsd.lib.libc.db.recno.b0331;

namespace P = pbsd::lib_libc_db_recno::b0331;

enum {
	RET_ERROR = -1,
	RET_SUCCESS = 0,
	RET_SPECIAL = 1
};

enum {
	R_CURSOR = 1,
	R_IAFTER = 4,
	R_IBEFORE = 5,
	R_NOOVERWRITE = 8,
	R_SETCURSOR = 10,
	R_FIXEDLEN = 0x01,
	R_NOKEY = 0x02,
	R_SNAPSHOT = 0x04,
	R_RECNO = 0x00080,
	R_CLOSEFP = 0x00040,
	R_EOF = 0x00100,
	R_FIXLEN = 0x00200,
	R_MEMMAPPED = 0x00400,
	R_INMEM = 0x00800,
	R_MODIFIED = 0x01000,
	R_RDONLY = 0x02000,
	B_MODIFIED = 0x00004,
	B_DB_LOCK = 0x04000,
	CURS_INIT = 0x08,
	P_BLEAF = 0x02,
	P_RLEAF = 0x10,
	P_BIGDATA = 0x01,
	P_ROOT = 1,
	MPOOL_DIRTY = 0x01,
	SDELETE = 0,
	SINSERT = 1,
	SEARCH = 2,
	PGSZ = 512,
	MAP_SZ = 4096,
	PIPE_SZ = 2048,
	RDATA_SZ = 512,
	GUARD = 0x7f,
	CMPZONE = PGSZ + 256,
	NFN = 11,
	FN_DELETE = 0,
	FN_DLEAF,
	FN_OPEN,
	FN_FD,
	FN_GET,
	FN_FPIPE,
	FN_VPIPE,
	FN_FMAP,
	FN_VMAP,
	FN_PUT,
	FN_IPUT
};

static const char *const FNNAME[NFN] = {
	"__rec_delete", "__rec_dleaf", "__rec_open", "__rec_fd", "__rec_get",
	"__rec_fpipe", "__rec_vpipe", "__rec_fmap", "__rec_vmap", "__rec_put",
	"__rec_iput"
};

typedef P::DBT DBT;
typedef P::PAGE PAGE;
typedef P::BTREE BTREE;
typedef P::DB DB;
typedef P::EPG EPG;
typedef P::RECNOINFO RECNOINFO;

extern "C" {
int ref___rec_delete(const DB *, const DBT *, unsigned int);
int ref___rec_dleaf(BTREE *, PAGE *, uint32_t);
DB *ref___rec_open(const char *, int, int, const RECNOINFO *, int);
int ref___rec_fd(const DB *);
int ref___rec_get(const DB *, const DBT *, DBT *, unsigned int);
int ref___rec_fpipe(BTREE *, uint32_t);
int ref___rec_vpipe(BTREE *, uint32_t);
int ref___rec_fmap(BTREE *, uint32_t);
int ref___rec_vmap(BTREE *, uint32_t);
int ref___rec_put(DB *, DBT *, const DBT *, unsigned int);
int ref___rec_iput(BTREE *, uint32_t, const DBT *, unsigned int);
int ref___rec_close(DB *);
int ref___rec_seq(const DB *, DBT *, DBT *, unsigned int);
int ref___rec_sync(const DB *, unsigned int);
}

struct Rng {
	uint64_t s;
	explicit Rng(uint64_t seed) : s(seed) {}
	uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ULL;
		uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
		return z ^ (z >> 31);
	}
	uint32_t n(uint32_t m) { return m ? (uint32_t)(next() % m) : 0u; }
};

static int g_irec_call;

struct Env {
	uint32_t flags;
	uint32_t bt_nrecs;
	uint8_t cflags;
	uint32_t rcursor;
	uint32_t keyrec;
	int key_zero;
	uint32_t put_flags;
	uint32_t get_flags;
	uint32_t del_flags;
	uint32_t dleaf_idx;
	uint32_t iput_nrec;
	uint32_t iput_flags;
	uint32_t iput_dsz;
	uint32_t irec_top;
	uint32_t ovflsize;
	int pinned;
	int fuel;
	int search_fail;
	int search_op;
	uint32_t search_nrec;
	uint32_t search_index;
	int ret_status;
	int irec_status;
	int ovfl_del_status;
	int ovfl_put_status;
	int split_status;
	int dleaf_fail;
	int iput_fail;
	int bt_open_null;
	int open_fail;
	int open_rfd;
	int lseek_espipe;
	int fstat_fail;
	int st_size_zero;
	int fdopen_fail;
	int mpool_root_fail;
	int root_bleaf;
	int snapshot_fail;
	int close_on_err;
	int realloc_fail;
	int malloc_fail;
	uint32_t reclen;
	uint8_t bval;
	size_t data_size;
	size_t map_len;
	size_t pipe_len;
	unsigned char map_data[MAP_SZ];
	unsigned char pipe_data[PIPE_SZ];
	unsigned char payload[64];
	char fname_buf[16];
	int use_fname;
	int use_openinfo;
	unsigned long oi_flags;
	size_t oi_reclen;
	uint8_t oi_bval;
	int open_flags;
	int open_mode;
};

static const Env *g_env;
static unsigned char *g_mem;
static int g_fuel;
static uint64_t g_loghash;
static int g_logn;
static int g_iput_calls;
static long g_mpool_cookie;
static DB g_pdb, g_odb;
static BTREE g_ptree, g_otree;
static PAGE *g_page;
static unsigned char g_bufA[CMPZONE + MAP_SZ + PIPE_SZ + RDATA_SZ + 128];
static unsigned char g_bufB[CMPZONE + MAP_SZ + PIPE_SZ + RDATA_SZ + 128];
static unsigned char g_dataA[128], g_dataB[128];
static unsigned char g_keyA[8], g_keyB[8];
static uint32_t g_keyval;
static FILE *g_pipeA, *g_pipeB;
static int g_open_rfd = 99;

static inline void lc(unsigned char c)
{
	g_loghash = (g_loghash ^ c) * 1099511628211ULL;
	++g_logn;
}

static inline void ls(const char *s)
{
	while (*s)
		lc((unsigned char)*s++);
}

static inline int fuel()
{
	if (g_fuel <= 0) {
		lc('!');
		return 0;
	}
	--g_fuel;
	return 1;
}

static inline uint32_t lalign(uint32_t n)
{
	return (n + 3u) & ~3u;
}

static inline uint32_t nrleafdbt(uint32_t dsz)
{
	return lalign(5u + dsz);
}

static void build_leaf_page(unsigned char *pg, int nent,
    const uint32_t *dsizes, const uint8_t *dflags, const unsigned char *bytes)
{
	memset(pg, GUARD, PGSZ);
	uint32_t upper = PGSZ;
	uint32_t lower = 20u + (uint32_t)nent * 2u;
	for (int i = nent - 1; i >= 0; --i) {
		uint32_t nb = nrleafdbt(dsizes[i]);
		upper -= nb;
		pg[upper + 0] = (unsigned char)(dsizes[i]);
		pg[upper + 1] = (unsigned char)(dsizes[i] >> 8);
		pg[upper + 2] = (unsigned char)(dsizes[i] >> 16);
		pg[upper + 3] = (unsigned char)(dsizes[i] >> 24);
		pg[upper + 4] = dflags[i];
		if (dsizes[i] > 0 && bytes != nullptr)
			memcpy(pg + upper + 5, bytes + i * 8, dsizes[i]);
		pg[20 + i * 2] = (unsigned char)upper;
		pg[20 + i * 2 + 1] = (unsigned char)(upper >> 8);
	}
	pg[12] = P_RLEAF;
	pg[16] = (unsigned char)lower;
	pg[17] = (unsigned char)(lower >> 8);
	pg[18] = (unsigned char)upper;
	pg[19] = (unsigned char)(upper >> 8);
}

static void setup_tree(BTREE &t, DB &db, unsigned char *buf, bool port)
{
	memset(&t, 0, sizeof t);
	memset(&db, 0, sizeof db);
	t.bt_mp = (P::MPOOL *)&g_mpool_cookie;
	t.bt_dbp = &db;
	t.bt_sp = t.bt_stack;
	t.flags = g_env->flags;
	t.bt_nrecs = g_env->bt_nrecs;
	t.bt_cursor.flags = g_env->cflags;
	t.bt_cursor.rcursor = g_env->rcursor;
	t.bt_ovflsize = (P::indx_t)g_env->ovflsize;
	t.bt_reclen = g_env->reclen;
	t.bt_bval = g_env->bval;
	t.bt_rfd = g_env->open_rfd;
	if (g_env->pinned >= 0)
		t.bt_pinned = (PAGE *)(buf + (size_t)g_env->pinned * PGSZ);
	t.bt_rdata.data = buf + CMPZONE + MAP_SZ + PIPE_SZ;
	t.bt_rdata.size = 16;
	t.bt_cmap = (char *)(buf + CMPZONE);
	t.bt_smap = (char *)(buf + CMPZONE);
	t.bt_emap = (char *)(buf + CMPZONE + g_env->map_len);
	t.bt_msize = g_env->map_len;
	memcpy(buf + CMPZONE, g_env->map_data, g_env->map_len > MAP_SZ ? MAP_SZ : g_env->map_len);
	db.internal = &t;
	(void)port;
}

extern "C" void *reallocf(void *p, size_t n)
{
	ls("rf");
	if (g_env && g_env->realloc_fail && n > 0) {
		ls("=fail");
		free(p);
		return nullptr;
	}
	void *q = realloc(p, n);
	if (!q && n) {
		free(p);
		ls("=null");
		return nullptr;
	}
	ls("=ok");
	return q;
}

extern "C" void *mpool_get(P::MPOOL *, uint32_t pgno, unsigned int fl)
{
	ls("mg");
	lc((unsigned char)('0' + (pgno % 10)));
	if (!fuel()) {
		ls("=X");
		return nullptr;
	}
	if (pgno == P_ROOT && g_env->mpool_root_fail) {
		ls("=fail");
		return nullptr;
	}
	if (pgno != P_ROOT) {
		ls("=bad");
		return nullptr;
	}
	ls("=ok");
	return g_mem;
}

extern "C" int mpool_put(P::MPOOL *, void *p, unsigned int fl)
{
	ls("mp");
	lc(fl ? 'd' : '0');
	return 0;
}

extern "C" EPG *__rec_search(BTREE *t, uint32_t nrec, int op)
{
	static EPG e;
	ls("sr");
	if (!fuel() || g_env->search_fail) {
		ls("=X");
		return nullptr;
	}
	e.page = g_page;
	e.index = (P::indx_t)g_env->search_index;
	ls("=ok");
	return &e;
}

extern "C" int __rec_ret(BTREE *, EPG *, uint32_t nrec, DBT *key, DBT *data)
{
	ls("rt");
	if (!fuel())
		return RET_ERROR;
	if (key) {
		g_keyval = nrec;
		key->data = &g_keyval;
		key->size = sizeof g_keyval;
	}
	if (data) {
		data->data = g_dataA;
		data->size = 4;
	}
	return g_env->ret_status;
}

extern "C" int __ovfl_delete(BTREE *, char *)
{
	ls("od");
	return g_env->ovfl_del_status;
}

extern "C" int __ovfl_put(BTREE *, const DBT *, uint32_t *pg)
{
	ls("op");
	if (!fuel() || g_env->ovfl_put_status != RET_SUCCESS)
		return RET_ERROR;
	*pg = 7;
	return RET_SUCCESS;
}

extern "C" int __bt_split(BTREE *, PAGE *, const DBT *, const DBT *, unsigned int,
    uint32_t, P::indx_t)
{
	ls("sp");
	return g_env->split_status;
}

extern "C" int mock_irec(BTREE *t, uint32_t top)
{
	ls("ir");
	++g_irec_call;
	if (!fuel())
		return RET_ERROR;
	if (g_env->irec_status == RET_SUCCESS)
		t->bt_nrecs = top - 1;
	return g_env->irec_status;
}

extern "C" DB *__bt_open(const char *, int, int, const void *, int)
{
	ls("bo");
	if (!fuel() || g_env->bt_open_null)
		return nullptr;
	g_pdb.internal = &g_ptree;
	g_odb.internal = &g_otree;
	return &g_pdb;
}

extern "C" int __bt_close(DB *)
{
	ls("bc");
	return 0;
}

extern "C" int __rec_close(DB *) { return 0; }
extern "C" int __rec_seq(const DB *, DBT *, DBT *, unsigned int) { return 0; }
extern "C" int __rec_sync(const DB *, unsigned int) { return 0; }
extern "C" int ref___rec_close(DB *) { return 0; }
extern "C" int ref___rec_seq(const DB *, DBT *, DBT *, unsigned int) { return 0; }
extern "C" int ref___rec_sync(const DB *, unsigned int) { return 0; }

extern "C" int _open(const char *fname, int flags, ...)
{
	ls("opn");
	if (!fuel() || g_env->open_fail)
		return -1;
	return g_env->open_rfd;
}

extern "C" int _close(int fd)
{
	ls("cls");
	(void)fd;
	return 0;
}

extern "C" int _fstat(int fd, struct stat *sb)
{
	ls("fst");
	(void)fd;
	if (g_env->fstat_fail)
		return -1;
	memset(sb, 0, sizeof *sb);
	sb->st_size = g_env->st_size_zero ? 0 : (off_t)g_env->map_len;
	return 0;
}

extern "C" off_t lseek(int fd, off_t off, int whence)
{
	ls("lsk");
	(void)fd;
	(void)off;
	(void)whence;
	if (g_env->lseek_espipe) {
		errno = ESPIPE;
		return (off_t)-1;
	}
	errno = 0;
	return 0;
}

extern "C" FILE *fdopen(int fd, const char *mode)
{
	ls("fdo");
	(void)fd;
	(void)mode;
	if (g_env->fdopen_fail)
		return nullptr;
	return g_pipeA;
}

struct Snap {
	int rc;
	int err;
	uint32_t flags;
	uint32_t bt_nrecs;
	uint32_t rcursor;
	uint8_t cflags;
	unsigned pinned_off;
	unsigned cmap_off;
	size_t rdata_size;
	void *rdata_ptr;
	uint64_t loghash;
	int logn;
	int iput_calls;
	unsigned char page[CMPZONE];
	unsigned char rdata[RDATA_SZ];
};

static Snap g_sa, g_sb;

static void snapshot(Snap &S, int rc, BTREE &t, unsigned char *buf)
{
	memset(&S, 0, sizeof S);
	S.rc = rc;
	S.err = errno;
	S.flags = t.flags;
	S.bt_nrecs = t.bt_nrecs;
	S.rcursor = t.bt_cursor.rcursor;
	S.cflags = t.bt_cursor.flags;
	S.pinned_off = t.bt_pinned ? (unsigned)((unsigned char *)t.bt_pinned - buf) : 0xffffu;
	S.cmap_off = t.bt_cmap ? (unsigned)((unsigned char *)t.bt_cmap - buf) : 0xffffu;
	S.rdata_size = t.bt_rdata.size;
	S.rdata_ptr = t.bt_rdata.data;
	S.loghash = g_loghash;
	S.logn = g_logn;
	S.iput_calls = g_iput_calls;
	memcpy(S.page, buf, CMPZONE);
	if (t.bt_rdata.data)
		memcpy(S.rdata, t.bt_rdata.data, RDATA_SZ);
}

static int run_one(int fn, bool port, Snap &S)
{
	g_loghash = 1469598103934665603ULL;
	g_logn = 0;
	g_iput_calls = 0;
	g_irec_call = 0;
	errno = 0;

	unsigned char *buf = port ? g_bufA : g_bufB;
	unsigned char *data = port ? g_dataA : g_dataB;
	unsigned char *keybuf = port ? g_keyA : g_keyB;
	memset(buf, GUARD, CMPZONE + MAP_SZ + PIPE_SZ + RDATA_SZ + 128);
	memset(data, GUARD, sizeof g_dataA);
	memcpy(data, g_env->payload, g_env->data_size < 64 ? g_env->data_size : 64);

	if (fn == FN_FPIPE || fn == FN_VPIPE) {
		if (g_pipeA) fclose(g_pipeA);
		if (g_pipeB) fclose(g_pipeB);
		g_pipeA = fmemopen(port ? g_bufB + CMPZONE + MAP_SZ : g_bufA + CMPZONE + MAP_SZ,
		    g_env->pipe_len, "r");
		g_pipeB = g_pipeA;
		memcpy(port ? g_bufB + CMPZONE + MAP_SZ : g_bufA + CMPZONE + MAP_SZ,
		    g_env->pipe_data, g_env->pipe_len);
	}

	g_mem = buf;
	build_leaf_page(buf, 3, nullptr, nullptr, nullptr);
	{
		uint32_t ds[3] = { 4, 8, 2 };
		uint8_t df[3] = { 0, (uint8_t)(g_env->ovfl_del_status == RET_ERROR ? 0 : P_BIGDATA), 0 };
		unsigned char bytes[24];
		memset(bytes, 0xa5, sizeof bytes);
		build_leaf_page(buf, 3, ds, df, bytes);
	}
	g_page = (PAGE *)buf;

	BTREE &tree = port ? g_ptree : g_otree;
	DB &db = port ? g_pdb : g_odb;
	setup_tree(tree, db, buf, port);
	tree.bt_irec = mock_irec;
	if (fn == FN_FPIPE || fn == FN_VPIPE)
		tree.bt_rfp = g_pipeA;

	DBT key, data_dbt;
	uint32_t krec = g_env->keyrec;
	key.data = g_env->key_zero ? nullptr : keybuf;
	if (!g_env->key_zero) {
		memcpy(keybuf, &krec, sizeof krec);
		key.size = sizeof krec;
	}
	data_dbt.data = data;
	data_dbt.size = g_env->data_size;

	int rc = RET_ERROR;

	switch (fn) {
	case FN_DELETE:
		rc = port ? P::__rec_delete(&db, &key, g_env->del_flags)
		    : ref___rec_delete(&db, &key, g_env->del_flags);
		break;
	case FN_DLEAF:
		rc = port ? P::__rec_dleaf(&tree, g_page, g_env->dleaf_idx)
		    : ref___rec_dleaf(&tree, g_page, g_env->dleaf_idx);
		break;
	case FN_OPEN: {
		RECNOINFO oi, *poi = nullptr;
		const char *fname = nullptr;
		if (g_env->use_fname) {
			char fname_local[16];
			memcpy(fname_local, "/tmp/x", 7);
			fname = fname_local;
		}
		if (g_env->use_openinfo) {
			memset(&oi, 0, sizeof oi);
			oi.flags = g_env->oi_flags;
			oi.reclen = g_env->oi_reclen;
			oi.bval = g_env->oi_bval;
			poi = &oi;
		}
		DB *dp = port ? P::__rec_open(fname, g_env->open_flags,
		    g_env->open_mode, poi, 0)
		    : ref___rec_open(fname, g_env->open_flags, g_env->open_mode,
		    poi, 0);
		rc = dp ? RET_SUCCESS : RET_ERROR;
		if (dp) {
			tree = *static_cast<BTREE *>(dp->internal);
		}
		break;
	}
	case FN_FD:
		rc = port ? P::__rec_fd(&db) : ref___rec_fd(&db);
		break;
	case FN_GET:
		rc = port ? P::__rec_get(&db, &key, &data_dbt, g_env->get_flags)
		    : ref___rec_get(&db, &key, &data_dbt, g_env->get_flags);
		break;
	case FN_FPIPE:
		rc = port ? P::__rec_fpipe(&tree, g_env->irec_top)
		    : ref___rec_fpipe(&tree, g_env->irec_top);
		break;
	case FN_VPIPE:
		rc = port ? P::__rec_vpipe(&tree, g_env->irec_top)
		    : ref___rec_vpipe(&tree, g_env->irec_top);
		break;
	case FN_FMAP:
		rc = port ? P::__rec_fmap(&tree, g_env->irec_top)
		    : ref___rec_fmap(&tree, g_env->irec_top);
		break;
	case FN_VMAP:
		rc = port ? P::__rec_vmap(&tree, g_env->irec_top)
		    : ref___rec_vmap(&tree, g_env->irec_top);
		break;
	case FN_PUT:
		rc = port ? P::__rec_put(&db, &key, &data_dbt, g_env->put_flags)
		    : ref___rec_put(&db, &key, &data_dbt, g_env->put_flags);
		break;
	case FN_IPUT:
		rc = port ? P::__rec_iput(&tree, g_env->iput_nrec, &data_dbt, g_env->iput_flags)
		    : ref___rec_iput(&tree, g_env->iput_nrec, &data_dbt, g_env->iput_flags);
		break;
	}
	snapshot(S, rc, tree, buf);
	return rc;
}

static long g_cases[NFN];
static long g_fails[NFN];

static void check_env(const Env &E, int fn, const char *tag, long id)
{
	++g_cases[fn];
	g_env = &E;
	g_fuel = E.fuel;
	run_one(fn, true, g_sa);
	g_fuel = E.fuel;
	run_one(fn, false, g_sb);
	if (memcmp(&g_sa, &g_sb, sizeof(Snap)) != 0) {
		++g_fails[fn];
		static int shown;
		if (shown < 12) {
			++shown;
			printf("  MISMATCH %-12s %s #%ld: rc %d/%d errno %d/%d "
			    "nrecs %u/%u flags %x/%x log %llx/%llx\n",
			    FNNAME[fn], tag, id, g_sa.rc, g_sb.rc, g_sa.err,
			    g_sb.err, g_sa.bt_nrecs, g_sb.bt_nrecs,
			    g_sa.flags, g_sb.flags,
			    (unsigned long long)g_sa.loghash,
			    (unsigned long long)g_sb.loghash);
		}
	}
}

static void gen_env(Env &E, Rng &r, int fn)
{
	memset(&E, 0, sizeof E);
	E.fuel = 32 + (int)r.n(64);
	E.flags = (r.n(2) ? R_MODIFIED : 0) | (r.n(2) ? R_INMEM : 0)
	    | (r.n(3) == 0 ? R_EOF : 0) | (r.n(3) == 0 ? R_FIXLEN : 0)
	    | (r.n(4) == 0 ? B_DB_LOCK : 0) | (r.n(4) == 0 ? R_CLOSEFP : 0);
	E.bt_nrecs = r.n(20);
	E.cflags = (uint8_t)((r.n(2) ? CURS_INIT : 0) | r.n(8));
	E.rcursor = 1 + r.n(15);
	E.keyrec = r.n(25);
	E.key_zero = r.n(16) == 0;
	E.del_flags = r.n(2) ? R_CURSOR : r.n(8);
	E.get_flags = r.n(4) == 0 ? r.n(16) : 0;
	static const unsigned putfl[] = {
		0, R_CURSOR, R_SETCURSOR, R_IAFTER, R_IBEFORE, R_NOOVERWRITE, 99
	};
	E.put_flags = putfl[r.n(7)];
	E.iput_flags = putfl[r.n(5)];
	E.dleaf_idx = r.n(3);
	E.iput_nrec = r.n(10);
	E.iput_dsz = 1 + r.n(32);
	E.irec_top = 1 + r.n(8);
	E.ovflsize = 8 + r.n(64);
	E.pinned = (int)r.n(3) - 1;
	E.search_fail = r.n(8) == 0;
	E.search_index = r.n(3);
	E.ret_status = r.n(4) == 0 ? RET_ERROR : RET_SUCCESS;
	E.irec_status = r.n(3) == 0 ? RET_ERROR : (r.n(2) ? RET_SPECIAL : RET_SUCCESS);
	E.ovfl_del_status = r.n(6) == 0 ? RET_ERROR : RET_SUCCESS;
	E.ovfl_put_status = r.n(6) == 0 ? RET_ERROR : RET_SUCCESS;
	E.split_status = r.n(5) == 0 ? RET_ERROR : RET_SUCCESS;
	E.reclen = 2 + r.n(16);
	E.bval = (uint8_t)(0x80 | r.n(128));
	E.data_size = r.n(33);
	E.map_len = 16 + r.n(MAP_SZ - 32);
	E.pipe_len = 8 + r.n(PIPE_SZ - 16);
	for (unsigned i = 0; i < MAP_SZ; ++i)
		E.map_data[i] = (unsigned char)(0x80 | r.n(128));
	for (unsigned i = 0; i < PIPE_SZ; ++i)
		E.pipe_data[i] = (unsigned char)r.n(256);
	for (unsigned i = 0; i < 64; ++i)
		E.payload[i] = (unsigned char)r.n(256);
	E.use_fname = r.n(2);
	E.use_openinfo = r.n(2);
	E.oi_flags = (r.n(2) ? R_FIXEDLEN : 0) | (r.n(3) == 0 ? R_SNAPSHOT : 0)
	    | (r.n(5) == 0 ? 0x10u : 0u);
	E.oi_reclen = 1 + r.n(32);
	E.oi_bval = (uint8_t)r.n(256);
	E.open_flags = (r.n(2) ? O_RDONLY : O_RDWR) | (r.n(3) == 0 ? O_WRONLY : 0);
	E.open_mode = 0644;
	E.open_fail = r.n(10) == 0;
	E.open_rfd = 10 + (int)r.n(20);
	E.lseek_espipe = r.n(2);
	E.fstat_fail = r.n(8) == 0;
	E.st_size_zero = r.n(2);
	E.fdopen_fail = r.n(10) == 0;
	E.mpool_root_fail = r.n(10) == 0;
	E.root_bleaf = r.n(2);
	E.snapshot_fail = r.n(6) == 0;
	E.realloc_fail = r.n(12) == 0;
	if (fn == FN_DLEAF)
		E.dleaf_idx = r.n(3);
	if (fn == FN_IPUT && E.iput_dsz > E.ovflsize)
		E.iput_dsz = E.ovflsize;
	if (fn == FN_PUT && (E.flags & R_FIXLEN)) {
		if (E.data_size > E.reclen)
			E.data_size = E.reclen;
	}
	E.data_size = E.iput_dsz;
	(void)fn;
}

static void edge_delete(void)
{
	long c = 0;
	for (int fl = 0; fl < 4; ++fl)
	for (uint32_t kr = 0; kr < 6; ++kr)
	for (uint32_t nr = 0; nr < 10; ++nr)
	for (int pin = -1; pin < 2; ++pin) {
		Env E{};
		E.fuel = 48;
		E.del_flags = fl == 0 ? 0 : fl == 1 ? R_CURSOR : fl == 2 ? 99 : R_CURSOR;
		E.keyrec = kr;
		E.bt_nrecs = nr;
		E.cflags = fl == 1 || fl == 3 ? CURS_INIT : 0;
		E.rcursor = 1 + (kr % 5);
		E.pinned = pin;
		E.search_fail = 0;
		E.ret_status = RET_SUCCESS;
		check_env(E, FN_DELETE, "edge", c++);
	}
}

static void edge_dleaf(void)
{
	long c = 0;
	for (uint32_t idx = 0; idx < 3; ++idx)
	for (int big = 0; big < 2; ++big)
	for (int od = 0; od < 2; ++od) {
		Env E{};
		E.fuel = 48;
		E.dleaf_idx = idx;
		E.bt_nrecs = 5;
		E.ovfl_del_status = od ? RET_ERROR : RET_SUCCESS;
		check_env(E, FN_DLEAF, "edge", c++);
	}
}

static void edge_open(void)
{
	long c = 0;
	for (int fnm = 0; fnm < 2; ++fnm)
	for (int oi = 0; oi < 2; ++oi)
	for (int pipe = 0; pipe < 2; ++pipe)
	for (int acc = 0; acc < 3; ++acc)
	for (int zst = 0; zst < 2; ++zst)
	for (int snap = 0; snap < 2; ++snap) {
		Env E{};
		E.fuel = 64;
		E.use_fname = fnm;
		E.use_openinfo = oi;
		E.lseek_espipe = pipe;
		E.open_flags = acc == 0 ? O_RDONLY : acc == 1 ? O_RDWR : O_WRONLY;
		E.st_size_zero = zst;
		E.oi_flags = (oi ? R_FIXEDLEN : 0) | (snap ? R_SNAPSHOT : 0);
		E.oi_reclen = 8;
		E.oi_bval = '\n';
		E.bt_open_null = 0;
		E.mpool_root_fail = 0;
		E.fdopen_fail = 0;
		E.irec_status = snap ? RET_ERROR : RET_SUCCESS;
		check_env(E, FN_OPEN, "edge", c++);
	}
}

static void edge_fd(void)
{
	long c = 0;
	for (int im = 0; im < 2; ++im)
	for (int pin = -1; pin < 2; ++pin) {
		Env E{};
		E.fuel = 32;
		E.flags = im ? R_INMEM : 0;
		E.pinned = pin;
		E.open_rfd = 42;
		check_env(E, FN_FD, "edge", c++);
	}
}

static void edge_get(void)
{
	long c = 0;
	for (int fl = 0; fl < 4; ++fl)
	for (uint32_t kr = 0; kr < 8; ++kr)
	for (uint32_t nr = 0; nr < 12; ++nr)
	for (int eof = 0; eof < 2; ++eof) {
		Env E{};
		E.fuel = 48;
		E.get_flags = fl == 0 ? 0 : fl == 1 ? 1 : fl == 2 ? 0 : 7;
		E.keyrec = kr;
		E.bt_nrecs = nr;
		E.flags = (eof ? R_EOF : 0) | (fl == 3 ? R_INMEM : 0);
		E.irec_status = RET_SUCCESS;
		E.search_fail = 0;
		E.ret_status = RET_SUCCESS;
		check_env(E, FN_GET, "edge", c++);
	}
}

static void edge_pipe_map(int fn)
{
	long c = 0;
	for (int fix = 0; fix < 2; ++fix)
	for (uint32_t top = 1; top < 6; ++top)
	for (uint32_t nr = 0; nr < 4; ++nr)
	for (uint8_t bv = 0; bv < 2; ++bv) {
		Env E{};
		E.fuel = 96;
		E.flags = fix ? R_FIXLEN : 0;
		E.reclen = 4;
		E.bval = bv ? (uint8_t)'\n' : (uint8_t)0xff;
		E.irec_top = top;
		E.bt_nrecs = nr;
		E.iput_fail = 0;
		E.search_fail = 0;
		E.split_status = RET_SUCCESS;
		E.map_len = 64;
		memset(E.map_data, 'A', 60);
		E.map_data[60] = E.bval;
		E.map_data[61] = 'X';
		memset(E.pipe_data, 'B', 40);
		E.pipe_data[40] = E.bval;
		E.pipe_len = 48;
		check_env(E, fn, "edge", c++);
	}
}

static void edge_put(void)
{
	long c = 0;
	static const unsigned fl[] = {
		0, R_CURSOR, R_SETCURSOR, R_IAFTER, R_IBEFORE, R_NOOVERWRITE, 99
	};
	for (int fi = 0; fi < 7; ++fi)
	for (int fix = 0; fix < 2; ++fix)
	for (uint32_t kr = 0; kr < 6; ++kr)
	for (uint32_t nr = 0; nr < 8; ++nr) {
		Env E{};
		E.fuel = 64;
		E.put_flags = fl[fi];
		E.flags = fix ? R_FIXLEN : 0;
		E.reclen = 8;
		E.keyrec = kr;
		E.bt_nrecs = nr;
		E.cflags = (fi == 1) ? CURS_INIT : 0;
		E.rcursor = 2;
		E.data_size = fix ? (kr % 9) : (1 + kr);
		E.irec_status = RET_SUCCESS;
		E.search_fail = 0;
		E.ret_status = RET_SUCCESS;
		check_env(E, FN_PUT, "edge", c++);
	}
}

static void edge_iput(void)
{
	long c = 0;
	static const unsigned fl[] = { 0, R_IAFTER, R_IBEFORE };
	for (int fi = 0; fi < 3; ++fi)
	for (uint32_t nr = 0; nr < 6; ++nr)
	for (uint32_t dsz = 1; dsz < 20; dsz += 3)
	for (int spl = 0; spl < 2; ++spl)
	for (int ov = 0; ov < 2; ++ov) {
		Env E{};
		E.fuel = 64;
		E.iput_flags = fl[fi];
		E.iput_nrec = nr;
		E.bt_nrecs = nr + 1;
		E.iput_dsz = dsz;
		E.data_size = dsz;
		E.ovflsize = ov ? 4 : 64;
		E.ovfl_put_status = RET_SUCCESS;
		E.split_status = spl ? RET_ERROR : RET_SUCCESS;
		E.search_fail = 0;
		check_env(E, FN_IPUT, "edge", c++);
	}
}

static void sweep(int fn, uint64_t seed, long n)
{
	for (long i = 0; i < n; ++i) {
		Env E;
		Rng r(seed + (uint64_t)i * 0x9E3779B97F4A7C15ULL);
		gen_env(E, r, fn);
		check_env(E, fn, "rand", i);
	}
}

int main(void)
{
	edge_delete();
	edge_dleaf();
	edge_open();
	edge_fd();
	edge_get();
	edge_pipe_map(FN_FPIPE);
	edge_pipe_map(FN_VPIPE);
	edge_pipe_map(FN_FMAP);
	edge_pipe_map(FN_VMAP);
	edge_put();
	edge_iput();

	for (int i = 0; i < NFN; ++i)
		sweep(i, 0x33100000ULL + (uint64_t)i * 0x1000000ULL, 30000);

	int fail = 0;
	printf("b0331 differential harness\n");
	printf("%-14s %10s %10s\n", "function", "cases", "failures");
	for (int i = 0; i < NFN; ++i) {
		printf("%-14s %10ld %10ld\n", FNNAME[i], g_cases[i], g_fails[i]);
		if (g_fails[i] != 0)
			fail = 1;
	}
	return fail ? 1 : 0;
}
