/*
 * Differential harness for batch b0194s3: hash.c routines.
 *
 * Compares ref_* oracle functions against pbsd::lib_libc_db_hash::b0194s3.
 */

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef BYTE_ORDER
#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN 4321
#define BYTE_ORDER LITTLE_ENDIAN
#endif

import pbsd.lib.libc.db.hash.b0194s3;

namespace P = pbsd::lib_libc_db_hash::b0194s3;

typedef unsigned char u_char;
typedef unsigned int u_int;
typedef uint32_t u_int32_t;
typedef uint16_t u_int16_t;
typedef int32_t int32_t;

typedef struct {
	void *data;
	size_t size;
} DBT;

typedef enum {
	HASH_GET,
	HASH_PUT,
	HASH_PUTNEW,
	HASH_DELETE,
	HASH_FIRST,
	HASH_NEXT
} ACTION;

typedef struct _bufhead BUFHEAD;

struct _bufhead {
	BUFHEAD *prev;
	BUFHEAD *next;
	BUFHEAD *ovfl;
	u_int32_t addr;
	char *page;
	char flags;
};

typedef BUFHEAD **SEGMENT;

typedef struct hashhdr {
	int32_t magic;
	int32_t version;
	u_int32_t lorder;
	int32_t bsize;
	int32_t bshift;
	int32_t dsize;
	int32_t ssize;
	int32_t sshift;
	int32_t ovfl_point;
	int32_t last_freed;
	u_int32_t max_bucket;
	u_int32_t high_mask;
	u_int32_t low_mask;
	u_int32_t ffactor;
	int32_t nkeys;
	int32_t hdrpages;
	int32_t h_charkey;
	int32_t spares[32];
	u_int16_t bitmaps[32];
} HASHHDR;

typedef struct htab {
	HASHHDR hdr;
	int nsegs;
	int exsegs;
	u_int32_t (*hash)(const void *, size_t);
	int flags;
	int fp;
	char *tmp_buf;
	char *tmp_key;
	BUFHEAD *cpage;
	int cbucket;
	int cndx;
	int error;
	int new_file;
	int save_file;
	u_int32_t *mapp[32];
	int nmaps;
	int nbufs;
	BUFHEAD bufhead;
	SEGMENT *dir;
} HTAB;

typedef enum { DB_BTREE, DB_HASH, DB_RECNO } DBTYPE;

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

typedef struct {
	unsigned int bsize;
	unsigned int ffactor;
	unsigned int nelem;
	unsigned int cachesize;
	u_int32_t (*hash)(const void *, size_t);
	int lorder;
} HASHINFO;

#define R_CURSOR 1
#define R_FIRST 3
#define R_NEXT 7
#define R_NOOVERWRITE 8
#define HASHMAGIC 0x061561
#define HASHVERSION 2
#define CHARKEY "%$sniglet^&"
#define DEF_FFACTOR 65536
#define OVFLPAGE 0
#define PARTIAL_KEY 1
#define FULL_KEY 2
#define FULL_KEY_DATA 3
#define REAL_KEY 4
#define BUF_MOD 0x0001
#define BUF_PIN 0x0008

extern "C" {
u_int32_t ref___call_hash(HTAB *, char *, int);
int ref_alloc_segs(HTAB *, int);
void *ref_hash_realloc(SEGMENT **, int, int);
int ref_init_htab(HTAB *, int);
void ref_swap_header(HTAB *);
void ref_swap_header_copy(HASHHDR *, HASHHDR *);
int ref___expand_table(HTAB *);
int ref_hash_access(HTAB *, ACTION, DBT *, DBT *);
int ref_hash_close(DB *);
int ref_hash_delete(const DB *, const DBT *, u_int32_t);
int ref_hash_fd(const DB *);
int ref_hash_get(const DB *, const DBT *, DBT *, u_int32_t);
int ref_hash_put(const DB *, DBT *, const DBT *, u_int32_t);
int ref_hash_seq(const DB *, DBT *, DBT *, u_int32_t);
int ref_hash_sync(const DB *, u_int32_t);
int ref_hdestroy(HTAB *);
int ref_flush_meta(HTAB *);
HTAB *ref_init_hash(HTAB *, const char *, const HASHINFO *);
DB *ref___hash_open(const char *, int, int, const HASHINFO *, int);

void hash_mock_reset(void);
void hash_mock_bind_htab(HTAB *);
void hash_mock_register(BUFHEAD *);
int hash_mock_nbufs(void);
void hash_mock_snapshot_page(int idx, char *dst, int sz);
void hash_mock_set_get_fail_cnt(int v);
void hash_mock_set_find_bigpair_ret(int v);
void hash_mock_set_find_last_page_ret(u_int16_t v);
void hash_mock_set_addel_fail(int v);
void hash_mock_set_delpair_fail(int v);
void hash_mock_set_split_page_fail(int v);
void hash_mock_set_ibitmap_fail(int v);
void hash_mock_set_buf_free_fail(int v);
void hash_mock_set_put_page_fail(int v);
void hash_mock_set_big_return_fail(int v);
void hash_mock_set_big_keydata_fail(int v);
}

namespace {

constexpr unsigned char GUARD = 0x7f;
constexpr int PAGE_BSIZE = 256;
constexpr int MAX_KV = 4096;
constexpr unsigned long SWEEP_ITERS = 200000;

enum Fn {
	F_CALL_HASH,
	F_ALLOC_SEGS,
	F_HASH_REALLOC,
	F_INIT_HTAB,
	F_SWAP_HEADER,
	F_SWAP_HEADER_COPY,
	F_EXPAND_TABLE,
	F_HASH_ACCESS,
	F_HASH_GET,
	F_HASH_PUT,
	F_HASH_DELETE,
	F_HASH_SEQ,
	F_HDESTROY,
	F_FLUSH_META,
	F_HASH_SYNC,
	F_INIT_HASH,
	F_HASH_OPEN,
	F_HASH_CLOSE,
	F_HASH_FD,
	F_COUNT
};

const char *fn_name[F_COUNT] = {
	"__call_hash", "alloc_segs", "hash_realloc", "init_htab",
	"swap_header", "swap_header_copy", "__expand_table", "hash_access",
	"hash_get", "hash_put", "hash_delete", "hash_seq",
	"hdestroy", "flush_meta", "hash_sync", "init_hash",
	"__hash_open", "hash_close", "hash_fd"
};

unsigned long n_cases[F_COUNT];
unsigned long n_fails[F_COUNT];
unsigned shown[F_COUNT];

struct Rng {
	std::uint64_t s;

	explicit Rng(std::uint64_t seed) : s(seed) {}

	std::uint64_t next()
	{
		s += 0x9E3779B97F4A7C15ull;
		std::uint64_t z = s;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}

	std::uint32_t below(std::uint32_t n) { return (std::uint32_t)(next() % n); }
};

static P::HTAB *ph(HTAB *p) { return reinterpret_cast<P::HTAB *>(p); }
static P::BUFHEAD *pb(BUFHEAD *p) { return reinterpret_cast<P::BUFHEAD *>(p); }
static const P::DBT *pd(const DBT *p) { return reinterpret_cast<const P::DBT *>(p); }
static P::DBT *pd(DBT *p) { return reinterpret_cast<P::DBT *>(p); }
static P::DB *pdb(DB *p) { return reinterpret_cast<P::DB *>(p); }
static P::HASHHDR *phh(HASHHDR *p) { return reinterpret_cast<P::HASHHDR *>(p); }
static P::HASHINFO *phi(HASHINFO *p) { return reinterpret_cast<P::HASHINFO *>(p); }

static u_int32_t test_hash(const void *k, size_t len)
{
	const auto *p = (const unsigned char *)k;
	u_int32_t h = 0;
	for (size_t i = 0; i < len; i++)
		h = h * 131u + p[i];
	return h;
}

static void guard_fill(void *p, size_t n) { std::memset(p, GUARD, n); }

static void page_init(char *p, int bsize)
{
	auto *sp = (u_int16_t *)p;
	sp[0] = 0;
	sp[1] = (u_int16_t)(bsize - 3 * (int)sizeof(u_int16_t));
	sp[2] = (u_int16_t)bsize;
}

static void putpair(char *p, const DBT *key, const DBT *val)
{
	u_int16_t *bp = (u_int16_t *)p;
	u_int16_t n = bp[0];
	u_int16_t off = bp[2] - (u_int16_t)key->size;
	std::memmove(p + off, key->data, key->size);
	bp[++n] = off;
	off -= (u_int16_t)val->size;
	std::memmove(p + off, val->data, val->size);
	bp[++n] = off;
	bp[0] = n;
	bp[n + 1] = off - ((n + 3) * sizeof(u_int16_t));
	bp[n + 2] = off;
}

static void init_htab_base(HTAB *hp, int bsize = PAGE_BSIZE)
{
	std::memset(hp, 0, sizeof(*hp));
	hp->hdr.bsize = bsize;
	hp->hdr.bshift = 8;
	hp->hdr.ssize = 256;
	hp->hdr.sshift = 8;
	hp->hdr.dsize = 256;
	hp->hdr.ffactor = DEF_FFACTOR;
	hp->hdr.high_mask = 1;
	hp->hdr.low_mask = 0;
	hp->hdr.max_bucket = 1;
	hp->hash = test_hash;
	hp->fp = -1;
	hp->new_file = 1;
	hp->cbucket = -1;
}

static void init_buf(BUFHEAD *bp, char *page, u_int32_t addr, int bsize)
{
	std::memset(bp, 0, sizeof(*bp));
	bp->page = page;
	bp->addr = addr;
	guard_fill(page, (size_t)bsize);
	page_init(page, bsize);
}

static void fail(Fn fn, const char *msg)
{
	n_fails[fn]++;
	if (shown[fn] < 10) {
		shown[fn]++;
		std::fprintf(stderr, "FAIL %s: %s\n", fn_name[fn], msg);
	}
}

static void check(Fn fn, bool ok, const char *msg)
{
	n_cases[fn]++;
	if (!ok)
		fail(fn, msg);
}

static bool buf_eq(const char *a, const char *b, size_t n)
{
	return std::memcmp(a, b, n) == 0;
}

static u_int32_t bucket_for(HTAB *hp, const char *key, size_t klen)
{
	return ref___call_hash(hp, (char *)key, (int)klen);
}

static void free_htab_dir(HTAB *hp)
{
	if (hp->dir) {
		if (hp->nsegs > 0 && hp->dir[0])
			std::free(hp->dir[0]);
		std::free(hp->dir);
		hp->dir = nullptr;
	}
	for (int i = 0; i < 32; i++) {
		if (hp->mapp[i]) {
			std::free(hp->mapp[i]);
			hp->mapp[i] = nullptr;
		}
	}
}

struct AccessEnv {
	HTAB htab;
	BUFHEAD buf;
	char page[PAGE_BSIZE];
	char keybuf[MAX_KV];
	char valbuf[MAX_KV];
	char outbuf_r[MAX_KV];
	char outbuf_p[MAX_KV];
};

static void setup_access_env(AccessEnv &e, const unsigned char *kb, size_t klen,
    const unsigned char *vb, size_t vlen, bool insert, bool reset_mock = true,
    bool init_htab = true)
{
	if (init_htab)
		init_htab_base(&e.htab);
	guard_fill(e.keybuf, MAX_KV);
	guard_fill(e.valbuf, MAX_KV);
	for (size_t i = 0; i < klen; i++)
		e.keybuf[i] = (char)kb[i];
	for (size_t i = 0; i < vlen; i++)
		e.valbuf[i] = (char)vb[i];
	u_int32_t bkt = bucket_for(&e.htab, e.keybuf, klen);
	init_buf(&e.buf, e.page, bkt, PAGE_BSIZE);
	if (reset_mock)
		hash_mock_reset();
	hash_mock_bind_htab(&e.htab);
	hash_mock_register(&e.buf);
	if (insert) {
		DBT key, val;
		key.data = e.keybuf;
		key.size = klen;
		val.data = e.valbuf;
		val.size = vlen;
		putpair(e.page, &key, &val);
	}
}

static void test_call_hash_edge()
{
	HTAB hr, hp;
	char k[8] = {0, (char)0x80, (char)0xff, 1, 0, 0, 0, 0};
	init_htab_base(&hr);
	init_htab_base(&hp);
	for (int len = 0; len <= 8; len++) {
		u_int32_t rr = ref___call_hash(&hr, k, len);
		u_int32_t rp = P::__call_hash(ph(&hp), k, len);
		check(F_CALL_HASH, rr == rp, "edge len");
	}
	hr.hdr.high_mask = 7;
	hr.hdr.low_mask = 3;
	hr.hdr.max_bucket = 5;
	hp.hdr = hr.hdr;
	u_int32_t rr = ref___call_hash(&hr, k, 4);
	u_int32_t rp = P::__call_hash(ph(&hp), k, 4);
	check(F_CALL_HASH, rr == rp, "mask branch");
	hr.hdr.high_mask = 0;
	hr.hdr.low_mask = 0;
	hr.hdr.max_bucket = 0;
	hp.hdr = hr.hdr;
	rr = ref___call_hash(&hr, k, 1);
	rp = P::__call_hash(ph(&hp), k, 1);
	check(F_CALL_HASH, rr == rp, "zero masks");
}

static void test_call_hash_sweep(Rng &rng)
{
	char keybuf[64];
	HTAB hp_r, hp_p;
	init_htab_base(&hp_r);
	init_htab_base(&hp_p);
	for (unsigned long i = 0; i < SWEEP_ITERS; i++) {
		size_t len = rng.below(33);
		for (size_t j = 0; j < len; j++)
			keybuf[j] = (char)(rng.next() & 0xff);
		hp_r.hdr.high_mask = rng.below(16) | 1;
		hp_r.hdr.low_mask = rng.below(hp_r.hdr.high_mask + 1);
		hp_r.hdr.max_bucket = rng.below(hp_r.hdr.high_mask + 1);
		hp_p.hdr = hp_r.hdr;
		u_int32_t rr = ref___call_hash(&hp_r, keybuf, (int)len);
		u_int32_t rp = P::__call_hash(ph(&hp_p), keybuf, (int)len);
		check(F_CALL_HASH, rr == rp, "sweep mismatch");
	}
}

static void test_alloc_segs()
{
	HTAB hr, hp;
	init_htab_base(&hr);
	init_htab_base(&hp);
	hash_mock_reset();
	int rr = ref_alloc_segs(&hr, 2);
	int rp = P::alloc_segs(ph(&hp), 2);
	check(F_ALLOC_SEGS, rr == rp, "ret");
	check(F_ALLOC_SEGS, hr.nsegs == hp.nsegs, "nsegs");
	check(F_ALLOC_SEGS, hr.dir != nullptr && hp.dir != nullptr, "dir");
	free_htab_dir(&hr);
	free_htab_dir(&hp);
	hash_mock_reset();
	rr = ref_alloc_segs(&hr, 0);
	rp = P::alloc_segs(ph(&hp), 0);
	check(F_ALLOC_SEGS, rr == rp && rr == 0, "zero segs");
	free_htab_dir(&hr);
	free_htab_dir(&hp);
}

static void test_hash_realloc()
{
	char oldmem_r[32];
	char oldmem_p[32];
	guard_fill(oldmem_r, 32);
	guard_fill(oldmem_p, 32);
	std::memcpy(oldmem_r, "seed-data-for-realloc!!", 23);
	std::memcpy(oldmem_p, "seed-data-for-realloc!!", 23);
	HTAB hr, hp;
	init_htab_base(&hr);
	init_htab_base(&hp);
	hr.dir = (SEGMENT *)std::malloc(32);
	hp.dir = (SEGMENT *)std::malloc(32);
	std::memcpy(hr.dir, oldmem_r, 32);
	std::memcpy(hp.dir, oldmem_p, 32);
	void *vr = ref_hash_realloc(&hr.dir, 32, 64);
	void *vp = P::hash_realloc(reinterpret_cast<P::SEGMENT **>(&hp.dir), 32, 64);
	check(F_HASH_REALLOC, (vr != nullptr) == (vp != nullptr), "null");
	if (vr && vp) {
		check(F_HASH_REALLOC,
		    std::memcmp((char *)hr.dir, "seed-data-for-realloc!!", 23) == 0,
		    "ref data");
		check(F_HASH_REALLOC,
		    std::memcmp((char *)hp.dir, "seed-data-for-realloc!!", 23) == 0,
		    "port data");
	}
	if (vr)
		std::free(hr.dir);
	if (vp)
		std::free(hp.dir);
}

static void test_init_htab()
{
	HTAB hr, hp;
	init_htab_base(&hr);
	init_htab_base(&hp);
	hash_mock_reset();
	int rr = ref_init_htab(&hr, 16);
	int rp = P::init_htab(ph(&hp), 16);
	check(F_INIT_HTAB, rr == rp, "ret");
	check(F_INIT_HTAB, hr.hdr.max_bucket == hp.hdr.max_bucket, "max_bucket");
	check(F_INIT_HTAB, hr.hdr.high_mask == hp.hdr.high_mask, "high_mask");
	free_htab_dir(&hr);
	free_htab_dir(&hp);
	hash_mock_reset();
	hash_mock_set_ibitmap_fail(1);
	rr = ref_init_htab(&hr, 8);
	rp = P::init_htab(ph(&hp), 8);
	check(F_INIT_HTAB, rr == rp && rr != 0, "ibitmap fail");
	free_htab_dir(&hr);
	free_htab_dir(&hp);
	hash_mock_reset();
	rr = ref_init_htab(&hr, 1);
	rp = P::init_htab(ph(&hp), 1);
	check(F_INIT_HTAB, rr == rp, "nelem 1");
	free_htab_dir(&hr);
	free_htab_dir(&hp);
}

static void test_swap_header()
{
#if BYTE_ORDER == LITTLE_ENDIAN
	HASHHDR sr, sp, dr, dp;
	std::memset(&sr, 0x55, sizeof(sr));
	std::memset(&sp, 0x55, sizeof(sp));
	sr.magic = 0x01020304;
	sr.version = 0x05060708;
	sr.lorder = 0x090a0b0c;
	sr.bitmaps[0] = 0x0d0e;
	sr.spares[3] = 0x11223344;
	sp = sr;
	HTAB hr, hp;
	init_htab_base(&hr);
	init_htab_base(&hp);
	hr.hdr = sr;
	hp.hdr = sp;
	ref_swap_header(&hr);
	P::swap_header(ph(&hp));
	check(F_SWAP_HEADER, hr.hdr.magic == hp.hdr.magic, "magic");
	check(F_SWAP_HEADER, hr.hdr.version == hp.hdr.version, "version");
	check(F_SWAP_HEADER, hr.hdr.bitmaps[0] == hp.hdr.bitmaps[0], "bitmap");
	check(F_SWAP_HEADER, hr.hdr.spares[3] == hp.hdr.spares[3], "spares");
	ref_swap_header_copy(&sr, &dr);
	P::swap_header_copy(phh(&sp), phh(&dp));
	check(F_SWAP_HEADER_COPY,
	    std::memcmp(&dr, &dp, sizeof(HASHHDR)) == 0, "copy");
#endif
}

static void test_expand_table()
{
	HTAB hr, hp;
	init_htab_base(&hr);
	init_htab_base(&hp);
	hash_mock_reset();
	ref_alloc_segs(&hr, 1);
	P::alloc_segs(ph(&hp), 1);
	hr.hdr.max_bucket = 1;
	hr.hdr.low_mask = 0;
	hr.hdr.high_mask = 1;
	hr.hdr.ovfl_point = 1;
	hr.hdr.spares[1] = 2;
	hr.hdr.spares[2] = 2;
	hr.nsegs = 1;
	hp.hdr = hr.hdr;
	hp.nsegs = hr.nsegs;
	hash_mock_set_split_page_fail(0);
	int rr = ref___expand_table(&hr);
	int rp = P::__expand_table(ph(&hp));
	check(F_EXPAND_TABLE, rr == rp, "ok");
	check(F_EXPAND_TABLE, hr.hdr.max_bucket == hp.hdr.max_bucket, "bucket");
	free_htab_dir(&hr);
	free_htab_dir(&hp);
	hash_mock_reset();
	init_htab_base(&hr);
	init_htab_base(&hp);
	ref_alloc_segs(&hr, 1);
	P::alloc_segs(ph(&hp), 1);
	hr.hdr.max_bucket = 1;
	hr.hdr.low_mask = 0;
	hr.hdr.high_mask = 1;
	hr.hdr.ovfl_point = 1;
	hr.hdr.spares[1] = 2;
	hr.hdr.spares[2] = 2;
	hr.nsegs = 1;
	hp.hdr = hr.hdr;
	hp.nsegs = hr.nsegs;
	hash_mock_set_split_page_fail(1);
	rr = ref___expand_table(&hr);
	rp = P::__expand_table(ph(&hp));
	check(F_EXPAND_TABLE, rr == rp && rr != 0, "split fail");
	free_htab_dir(&hr);
	free_htab_dir(&hp);
}

static void test_hash_access()
{
	AccessEnv er, ep;
	unsigned char kb[] = {0, 'a', (unsigned char)0xff, 'b'};
	unsigned char vb[] = {'v', (unsigned char)0x80};
	DBT keyr, valr, keyp, valp;

	setup_access_env(er, kb, 4, vb, 2, true);
	setup_access_env(ep, kb, 4, vb, 2, true, false);
	keyr.data = er.keybuf;
	keyr.size = 4;
	keyp.data = ep.keybuf;
	keyp.size = 4;
	guard_fill(er.outbuf_r, MAX_KV);
	guard_fill(ep.outbuf_p, MAX_KV);
	valr.data = er.outbuf_r;
	valr.size = 0;
	valp.data = ep.outbuf_p;
	valp.size = 0;
	int rr = ref_hash_access(&er.htab, HASH_GET, &keyr, &valr);
	int rp = P::hash_access(ph(&ep.htab), (P::ACTION)HASH_GET, pd(&keyp),
	    pd(&valp));
	check(F_HASH_ACCESS, rr == rp, "get ret");
	check(F_HASH_ACCESS, valr.size == valp.size, "get size");
	if (rr == 0 && rp == 0) {
		check(F_HASH_ACCESS, valr.size == 2, "get size val");
		check(F_HASH_ACCESS, std::memcmp(valr.data, vb, 2) == 0, "get data");
	}
	check(F_HASH_ACCESS, buf_eq(er.outbuf_r, ep.outbuf_p, MAX_KV), "get guard");

	setup_access_env(er, kb, 4, vb, 2, false);
	setup_access_env(ep, kb, 4, vb, 2, false, false);
	rr = ref_hash_access(&er.htab, HASH_GET, &keyr, &valr);
	rp = P::hash_access(ph(&ep.htab), (P::ACTION)HASH_GET, pd(&keyp),
	    pd(&valp));
	check(F_HASH_ACCESS, rr == rp && rr == 1, "not found");

	hash_mock_reset();
	hash_mock_set_get_fail_cnt(1);
	setup_access_env(er, kb, 1, vb, 1, true);
	setup_access_env(ep, kb, 1, vb, 1, true, false);
	rr = ref_hash_access(&er.htab, HASH_GET, &keyr, &valr);
	rp = P::hash_access(ph(&ep.htab), (P::ACTION)HASH_GET, pd(&keyp),
	    pd(&valp));
	check(F_HASH_ACCESS, rr == rp && rr == -1, "get_buf fail");

	hash_mock_reset();
	setup_access_env(er, kb, 2, vb, 2, false);
	setup_access_env(ep, kb, 2, vb, 2, false, false);
	valr.data = er.valbuf;
	valr.size = 2;
	valp.data = ep.valbuf;
	valp.size = 2;
	std::memcpy(er.valbuf, vb, 2);
	std::memcpy(ep.valbuf, vb, 2);
	rr = ref_hash_access(&er.htab, HASH_PUT, &keyr, &valr);
	rp = P::hash_access(ph(&ep.htab), (P::ACTION)HASH_PUT, pd(&keyp),
	    pd(&valp));
	check(F_HASH_ACCESS, rr == rp, "put");
	check(F_HASH_ACCESS, buf_eq(er.page, ep.page, PAGE_BSIZE), "put page");

	hash_mock_reset();
	hash_mock_set_addel_fail(1);
	setup_access_env(er, kb, 2, vb, 2, false, false);
	setup_access_env(ep, kb, 2, vb, 2, false, false);
	rr = ref_hash_access(&er.htab, HASH_PUT, &keyr, &valr);
	rp = P::hash_access(ph(&ep.htab), (P::ACTION)HASH_PUT, pd(&keyp),
	    pd(&valp));
	check(F_HASH_ACCESS, rr == rp && rr == -1, "addel fail");

	hash_mock_reset();
	setup_access_env(er, kb, 4, vb, 2, true);
	setup_access_env(ep, kb, 4, vb, 2, true, false);
	rr = ref_hash_access(&er.htab, HASH_DELETE, &keyr, nullptr);
	rp = P::hash_access(ph(&ep.htab), (P::ACTION)HASH_DELETE, pd(&keyp),
	    nullptr);
	check(F_HASH_ACCESS, rr == rp, "delete");
	check(F_HASH_ACCESS, buf_eq(er.page, ep.page, PAGE_BSIZE), "page after del");

	hash_mock_reset();
	hash_mock_set_delpair_fail(1);
	setup_access_env(er, kb, 4, vb, 2, true, false);
	setup_access_env(ep, kb, 4, vb, 2, true, false);
	rr = ref_hash_access(&er.htab, HASH_DELETE, &keyr, nullptr);
	rp = P::hash_access(ph(&ep.htab), (P::ACTION)HASH_DELETE, pd(&keyp),
	    nullptr);
	check(F_HASH_ACCESS, rr == rp && rr == -1, "delpair fail");

	hash_mock_reset();
	setup_access_env(er, kb, 4, vb, 2, true);
	setup_access_env(ep, kb, 4, vb, 2, true, false);
	rr = ref_hash_access(&er.htab, HASH_PUTNEW, &keyr, &valr);
	rp = P::hash_access(ph(&ep.htab), (P::ACTION)HASH_PUTNEW, pd(&keyp),
	    pd(&valp));
	check(F_HASH_ACCESS, rr == rp && rr == 1, "putnew exists");

	hash_mock_reset();
	setup_access_env(er, kb, 1, vb, 1, false);
	setup_access_env(ep, kb, 1, vb, 1, false, false);
	keyr.size = 1;
	keyp.size = 1;
	auto *sp = (u_int16_t *)er.page;
	sp[0] = 2;
	sp[1] = 0;
	sp[2] = PARTIAL_KEY;
	sp[4] = PARTIAL_KEY;
	std::memcpy(ep.page, er.page, PAGE_BSIZE);
	hash_mock_set_find_bigpair_ret(3);
	rr = ref_hash_access(&er.htab, HASH_GET, &keyr, &valr);
	rp = P::hash_access(ph(&ep.htab), (P::ACTION)HASH_GET, pd(&keyp),
	    pd(&valp));
	check(F_HASH_ACCESS, rr == rp && rr == 0, "bigpair found");

	hash_mock_reset();
	setup_access_env(er, kb, 1, vb, 1, false);
	setup_access_env(ep, kb, 1, vb, 1, false, false);
	keyr.size = 1;
	keyp.size = 1;
	sp = (u_int16_t *)er.page;
	sp[0] = 2;
	sp[1] = 0;
	sp[2] = PARTIAL_KEY;
	std::memcpy(ep.page, er.page, PAGE_BSIZE);
	hash_mock_set_find_bigpair_ret(-2);
	hash_mock_set_find_last_page_ret(0);
	rr = ref_hash_access(&er.htab, HASH_GET, &keyr, &valr);
	rp = P::hash_access(ph(&ep.htab), (P::ACTION)HASH_GET, pd(&keyp),
	    pd(&valp));
	check(F_HASH_ACCESS, rr == rp && rr == 1, "bigpair -2 no page");

	hash_mock_reset();
	hash_mock_set_big_return_fail(1);
	setup_access_env(er, kb, 4, vb, 2, true);
	setup_access_env(ep, kb, 4, vb, 2, true, false);
	sp = (u_int16_t *)er.page;
	sp[2] = PARTIAL_KEY;
	std::memcpy(ep.page, er.page, PAGE_BSIZE);
	rr = ref_hash_access(&er.htab, HASH_GET, &keyr, &valr);
	rp = P::hash_access(ph(&ep.htab), (P::ACTION)HASH_GET, pd(&keyp),
	    pd(&valp));
	check(F_HASH_ACCESS, rr == rp && rr == -1, "big_return fail");

	hash_mock_reset();
	setup_access_env(er, kb, 2, vb, 2, true);
	setup_access_env(ep, kb, 2, vb, 2, true, false);
	u_int32_t bkt = bucket_for(&er.htab, er.keybuf, 2);
	BUFHEAD ovfl_r, ovfl_p;
	char ovfl_page_r[PAGE_BSIZE];
	char ovfl_page_p[PAGE_BSIZE];
	init_buf(&ovfl_r, ovfl_page_r, 99, PAGE_BSIZE);
	init_buf(&ovfl_p, ovfl_page_p, 99, PAGE_BSIZE);
	er.buf.ovfl = &ovfl_r;
	ep.buf.ovfl = &ovfl_p;
	sp = (u_int16_t *)er.page;
	sp[0] = 2;
	sp[1] = OVFLPAGE;
	sp[2] = 99;
	std::memcpy(ep.page, er.page, PAGE_BSIZE);
	hash_mock_register(&ovfl_r);
	hash_mock_register(&ovfl_p);
	keyr.size = 2;
	keyp.size = 2;
	rr = ref_hash_access(&er.htab, HASH_GET, &keyr, &valr);
	rp = P::hash_access(ph(&ep.htab), (P::ACTION)HASH_GET, pd(&keyp),
	    pd(&valp));
	check(F_HASH_ACCESS, rr == rp, "ovfl chain miss");
	(void)bkt;
}

static DB *make_db(HTAB *hp, bool ref_side)
{
	auto *db = (DB *)std::malloc(sizeof(DB));
	std::memset(db, 0, sizeof(*db));
	db->internal = hp;
	db->type = DB_HASH;
	if (ref_side) {
		db->get = ref_hash_get;
		db->put = (int (*)(DB *, DBT *, const DBT *, unsigned int))ref_hash_put;
		db->del = ref_hash_delete;
		db->seq = ref_hash_seq;
		db->sync = ref_hash_sync;
		db->close = ref_hash_close;
		db->fd = ref_hash_fd;
	}
	return db;
}

static void test_db_wrappers()
{
	AccessEnv er, ep;
	unsigned char kb[] = {'k', 'e', 'y'};
	unsigned char vb[] = {'v', 'a', 'l'};
	DBT keyr, valr, keyp, valp;

	setup_access_env(er, kb, 3, vb, 3, true);
	setup_access_env(ep, kb, 3, vb, 3, true, false);
	er.htab.flags = O_RDWR;
	ep.htab.flags = O_RDWR;
	DB *dbr = make_db(&er.htab, true);
	DB *dbp = make_db(&ep.htab, false);
	keyr.data = er.keybuf;
	keyr.size = 3;
	keyp.data = ep.keybuf;
	keyp.size = 3;
	guard_fill(er.outbuf_r, MAX_KV);
	guard_fill(ep.outbuf_p, MAX_KV);
	valr.data = er.outbuf_r;
	valr.size = 0;
	valp.data = ep.outbuf_p;
	valp.size = 0;
	int rr = ref_hash_get(dbr, &keyr, &valr, 0);
	int rp = P::hash_get(pdb(dbp), pd(&keyp), pd(&valp), 0);
	check(F_HASH_GET, rr == rp, "get");
	check(F_HASH_GET, buf_eq(er.outbuf_r, ep.outbuf_p, MAX_KV), "get guard");

	rr = ref_hash_get(dbr, &keyr, &valr, R_FIRST);
	rp = P::hash_get(pdb(dbp), pd(&keyp), pd(&valp), R_FIRST);
	check(F_HASH_GET, rr == rp && rr == -1, "get bad flag");

	er.htab.flags = O_WRONLY;
	ep.htab.flags = O_WRONLY;
	rr = ref_hash_get(dbr, &keyr, &valr, 0);
	rp = P::hash_get(pdb(dbp), pd(&keyp), pd(&valp), 0);
	check(F_HASH_GET, rr == rp && rr == -1, "get wronly");
	er.htab.flags = O_RDWR;
	ep.htab.flags = O_RDWR;

	rr = ref_hash_put(dbr, &keyr, &valr, R_NOOVERWRITE);
	rp = P::hash_put(pdb(dbp), pd(&keyr), pd(&valr), R_NOOVERWRITE);
	check(F_HASH_PUT, rr == rp, "put nooverwrite");

	unsigned char kb2[] = {'n', 'e', 'w'};
	keyr.data = kb2;
	keyr.size = 3;
	keyp.data = kb2;
	keyp.size = 3;
	valr.data = er.valbuf;
	valr.size = 3;
	valp.data = ep.valbuf;
	valp.size = 3;
	std::memcpy(er.valbuf, vb, 3);
	std::memcpy(ep.valbuf, vb, 3);
	rr = ref_hash_put(dbr, &keyr, &valr, 0);
	rp = P::hash_put(pdb(dbp), pd(&keyr), pd(&valr), 0);
	check(F_HASH_PUT, rr == rp, "put new");

	er.htab.flags = O_RDONLY;
	ep.htab.flags = O_RDONLY;
	rr = ref_hash_put(dbr, &keyr, &valr, 0);
	rp = P::hash_put(pdb(dbp), pd(&keyr), pd(&valr), 0);
	check(F_HASH_PUT, rr == rp && rr == -1, "put rdonly");
	er.htab.flags = O_RDWR;
	ep.htab.flags = O_RDWR;

	rr = ref_hash_put(dbr, &keyr, &valr, R_FIRST);
	rp = P::hash_put(pdb(dbp), pd(&keyr), pd(&valr), R_FIRST);
	check(F_HASH_PUT, rr == rp && rr == -1, "put bad flag");

	keyr.data = er.keybuf;
	keyr.size = 3;
	keyp.data = ep.keybuf;
	keyp.size = 3;
	rr = ref_hash_delete(dbr, &keyr, R_CURSOR);
	rp = P::hash_delete(pdb(dbp), pd(&keyr), R_CURSOR);
	check(F_HASH_DELETE, rr == rp, "delete");

	rr = ref_hash_delete(dbr, &keyr, R_FIRST);
	rp = P::hash_delete(pdb(dbp), pd(&keyr), R_FIRST);
	check(F_HASH_DELETE, rr == rp && rr == -1, "delete bad flag");

	rr = ref_hash_fd(dbr);
	rp = P::hash_fd(pdb(dbp));
	check(F_HASH_FD, rr == rp, "fd closed");

	er.htab.fp = 42;
	ep.htab.fp = 42;
	rr = ref_hash_fd(dbr);
	rp = P::hash_fd(pdb(dbp));
	check(F_HASH_FD, rr == rp && rr == 42, "fd open");

	er.htab.save_file = 0;
	ep.htab.save_file = 0;
	rr = ref_hash_sync(dbr, 0);
	rp = P::hash_sync(pdb(dbp), 0);
	check(F_HASH_SYNC, rr == rp, "sync no save");

	er.htab.save_file = 1;
	ep.htab.save_file = 1;
	rr = ref_hash_sync(dbr, 0);
	rp = P::hash_sync(pdb(dbp), 0);
	check(F_HASH_SYNC, rr == rp, "sync");

	rr = ref_hash_sync(dbr, 1);
	rp = P::hash_sync(pdb(dbp), 1);
	check(F_HASH_SYNC, rr == rp && rr == -1, "sync bad flag");

	rr = ref_hash_close(nullptr);
	rp = P::hash_close(nullptr);
	check(F_HASH_CLOSE, rr == rp && rr == -1, "close null");

	std::free(dbr);
	std::free(dbp);
}

static void test_hash_seq()
{
	AccessEnv er, ep;
	unsigned char kb[] = {'a'};
	unsigned char vb[] = {'b'};
	DBT keyr, valr, keyp, valp;

	setup_access_env(er, kb, 1, vb, 1, true);
	setup_access_env(ep, kb, 1, vb, 1, true, false);
	er.htab.hdr.max_bucket = 0;
	er.htab.hdr.high_mask = 0;
	ep.htab.hdr.max_bucket = 0;
	ep.htab.hdr.high_mask = 0;
	DB *dbr = make_db(&er.htab, true);
	DB *dbp = make_db(&ep.htab, false);
	guard_fill(er.keybuf, MAX_KV);
	guard_fill(ep.keybuf, MAX_KV);
	guard_fill(er.valbuf, MAX_KV);
	guard_fill(ep.valbuf, MAX_KV);
	keyr.data = er.keybuf;
	valr.data = er.valbuf;
	keyp.data = ep.keybuf;
	valp.data = ep.valbuf;

	int rr = ref_hash_seq(dbr, &keyr, &valr, R_FIRST);
	int rp = P::hash_seq(pdb(dbp), pd(&keyp), pd(&valp), R_FIRST);
	check(F_HASH_SEQ, rr == rp, "first");
	check(F_HASH_SEQ, keyr.size == keyp.size, "key size");
	check(F_HASH_SEQ, buf_eq(er.keybuf, ep.keybuf, MAX_KV), "key guard");
	check(F_HASH_SEQ, buf_eq(er.valbuf, ep.valbuf, MAX_KV), "val guard");

	rr = ref_hash_seq(dbr, &keyr, &valr, R_NEXT);
	rp = P::hash_seq(pdb(dbp), pd(&keyp), pd(&valp), R_NEXT);
	check(F_HASH_SEQ, rr == rp && rr == 1, "next end");

	er.htab.cbucket = -1;
	ep.htab.cbucket = -1;
	rr = ref_hash_seq(dbr, &keyr, &valr, R_NEXT);
	rp = P::hash_seq(pdb(dbp), pd(&keyp), pd(&valp), R_NEXT);
	check(F_HASH_SEQ, rr == rp && rr == 1, "next without first");

	rr = ref_hash_seq(dbr, &keyr, &valr, R_CURSOR);
	rp = P::hash_seq(pdb(dbp), pd(&keyp), pd(&valp), R_CURSOR);
	check(F_HASH_SEQ, rr == rp && rr == -1, "seq bad flag");

	hash_mock_reset();
	hash_mock_set_big_keydata_fail(1);
	setup_access_env(er, kb, 1, vb, 1, true, false);
	setup_access_env(ep, kb, 1, vb, 1, true, false);
	auto *sp = (u_int16_t *)er.page;
	sp[2] = PARTIAL_KEY;
	std::memcpy(ep.page, er.page, PAGE_BSIZE);
	dbr->internal = &er.htab;
	dbp->internal = &ep.htab;
	rr = ref_hash_seq(dbr, &keyr, &valr, R_FIRST);
	rp = P::hash_seq(pdb(dbp), pd(&keyp), pd(&valp), R_FIRST);
	check(F_HASH_SEQ, rr == rp && rr == -1, "big_keydata fail");

	std::free(dbr);
	std::free(dbp);
}

static void test_hdestroy_flush()
{
	HTAB hr, hp;
	HTAB *hrp = (HTAB *)std::calloc(1, sizeof(HTAB));
	HTAB *hpp = (HTAB *)std::calloc(1, sizeof(HTAB));
	init_htab_base(hrp);
	init_htab_base(hpp);
	hrp->save_file = 0;
	hpp->save_file = 0;
	hash_mock_reset();
	int rr = ref_hdestroy(hrp);
	int rp = P::hdestroy(ph(hpp));
	check(F_HDESTROY, rr == rp, "hdestroy empty");

	init_htab_base(hrp);
	init_htab_base(hpp);
	hash_mock_reset();
	ref_alloc_segs(hrp, 1);
	P::alloc_segs(ph(hpp), 1);
	hrp->save_file = 0;
	hpp->save_file = 0;
	rr = ref_hdestroy(hrp);
	rp = P::hdestroy(ph(hpp));
	check(F_HDESTROY, rr == rp, "hdestroy with dir");

	init_htab_base(hrp);
	init_htab_base(hpp);
	hash_mock_reset();
	hash_mock_set_buf_free_fail(1);
	hrp->save_file = 1;
	hpp->save_file = 1;
	rr = ref_hdestroy(hrp);
	rp = P::hdestroy(ph(hpp));
	check(F_HDESTROY, rr == rp && rr == -1, "hdestroy buf_free fail");

	init_htab_base(&hr);
	init_htab_base(&hp);
	hr.save_file = 1;
	hp.save_file = 1;
	hr.fp = -1;
	hp.fp = -1;
	hash_mock_reset();
	rr = ref_flush_meta(&hr);
	rp = P::flush_meta(ph(&hp));
	check(F_FLUSH_META, rr == rp, "flush no fp");

	hash_mock_reset();
	hash_mock_set_put_page_fail(1);
	hr.save_file = 1;
	hp.save_file = 1;
	hr.fp = 3;
	hp.fp = 3;
	hr.mapp[0] = (u_int32_t *)std::calloc(1, sizeof(u_int32_t));
	hp.mapp[0] = (u_int32_t *)std::calloc(1, sizeof(u_int32_t));
	rr = ref_flush_meta(&hr);
	rp = P::flush_meta(ph(&hp));
	check(F_FLUSH_META, rr == rp && rr != 0, "put_page fail");
	std::free(hr.mapp[0]);
	std::free(hp.mapp[0]);
}

static void test_init_hash()
{
	HTAB hr, hp;
	std::memset(&hr, 0, sizeof(hr));
	std::memset(&hp, 0, sizeof(hp));
	hr.fp = -1;
	hp.fp = -1;
	hash_mock_reset();
	HASHINFO info{};
	info.nelem = 32;
	info.bsize = PAGE_BSIZE;
	HTAB *tr = ref_init_hash(&hr, nullptr, &info);
	HTAB th;
	std::memset(&th, 0, sizeof(th));
	th.fp = -1;
	HTAB *tp = reinterpret_cast<HTAB *>(P::init_hash(ph(&th), nullptr, phi(&info)));
	check(F_INIT_HASH, (tr != nullptr) == (tp != nullptr), "init null file");
	if (tr && tp) {
		check(F_INIT_HASH, tr->hdr.bsize == tp->hdr.bsize, "bsize");
		free_htab_dir(&hr);
		free_htab_dir(reinterpret_cast<HTAB *>(tp));
	}

	hash_mock_reset();
	info.lorder = 9999;
	tr = ref_init_hash(&hr, nullptr, &info);
	tp = reinterpret_cast<HTAB *>(P::init_hash(ph(&th), nullptr, phi(&info)));
	check(F_INIT_HASH, (tr == nullptr) && (tp == nullptr), "bad lorder");

	char tmpl[] = "/tmp/pbsd-hash-XXXXXX";
	int fd = mkstemp(tmpl);
	ftruncate(fd, 0);
	close(fd);
	hash_mock_reset();
	info.lorder = 0;
	info.bsize = 0;
	tr = ref_init_hash(&hr, tmpl, &info);
	tp = reinterpret_cast<HTAB *>(P::init_hash(ph(&th), tmpl, phi(&info)));
	check(F_INIT_HASH, (tr != nullptr) == (tp != nullptr), "file stat");
	if (tr)
		free_htab_dir(&hr);
	if (tp)
		free_htab_dir(reinterpret_cast<HTAB *>(tp));
	unlink(tmpl);
}

static void test_hash_open()
{
	hash_mock_reset();
	DB *dr = ref___hash_open(nullptr, O_RDWR, 0, nullptr, 0);
	P::DB *dp = P::__hash_open(nullptr, O_RDWR, 0, nullptr, 0);
	check(F_HASH_OPEN, (dr != nullptr) == (dp != nullptr), "mem open");
	if (dr)
		check(F_HASH_CLOSE, ref_hash_close(dr) == 0, "close ref mem");
	if (dp)
		check(F_HASH_CLOSE, P::hash_close(pdb((DB *)dp)) == 0, "close port mem");

	char tmpl[] = "/tmp/pbsd-hopen-XXXXXX";
	int fd = mkstemp(tmpl);
	close(fd);
	hash_mock_reset();
	dr = ref___hash_open(tmpl, O_RDWR | O_CREAT, 0600, nullptr, 0);
	dp = P::__hash_open(tmpl, O_RDWR | O_CREAT, 0600, nullptr, 0);
	check(F_HASH_OPEN, (dr != nullptr) == (dp != nullptr), "new file");
	if (dr)
		check(F_HASH_CLOSE, ref_hash_close(dr) == 0, "close ref file");
	if (dp)
		check(F_HASH_CLOSE, P::hash_close(pdb((DB *)dp)) == 0, "close port file");
	unlink(tmpl);

	hash_mock_reset();
	char tmpl2[] = "/tmp/pbsd-hopen2-XXXXXX";
	fd = mkstemp(tmpl2);
	dr = ref___hash_open(tmpl2, O_RDWR | O_CREAT, 0600, nullptr, 0);
	if (dr)
		ref_hash_close(dr);
	hash_mock_reset();
	dp = P::__hash_open(tmpl2, O_RDWR, 0600, nullptr, 0);
	dr = ref___hash_open(tmpl2, O_RDWR, 0600, nullptr, 0);
	check(F_HASH_OPEN, (dr != nullptr) == (dp != nullptr), "existing");
	if (dr)
		ref_hash_close(dr);
	if (dp)
		P::hash_close(pdb((DB *)dp));
	unlink(tmpl2);

	hash_mock_reset();
	dr = ref___hash_open(nullptr, O_WRONLY, 0, nullptr, 0);
	dp = P::__hash_open(nullptr, O_WRONLY, 0, nullptr, 0);
	check(F_HASH_OPEN, (dr != nullptr) == (dp != nullptr), "owronly");
	if (dr)
		ref_hash_close(dr);
	if (dp)
		P::hash_close(pdb((DB *)dp));

	hash_mock_reset();
	dr = ref___hash_open("/nonexistent/pbsd-hash-test", O_RDONLY, 0, nullptr, 0);
	dp = P::__hash_open("/nonexistent/pbsd-hash-test", O_RDONLY, 0, nullptr, 0);
	check(F_HASH_OPEN, (dr == nullptr) && (dp == nullptr), "missing file");
}

static void test_access_sweep(Rng &rng)
{
	unsigned char kb[32], vb[32];
	for (unsigned long i = 0; i < SWEEP_ITERS; i++) {
		AccessEnv er, ep;
		size_t klen = rng.below(17);
		size_t vlen = rng.below(17);
		for (size_t j = 0; j < klen; j++)
			kb[j] = (unsigned char)(rng.next() & 0xff);
		for (size_t j = 0; j < vlen; j++)
			vb[j] = (unsigned char)(rng.next() & 0xff);
		bool ins = (rng.next() & 1) != 0;
		setup_access_env(er, kb, klen, vb, vlen, ins);
		setup_access_env(ep, kb, klen, vb, vlen, ins, false);
		er.htab.hdr.high_mask = rng.below(8) | 1;
		er.htab.hdr.low_mask = rng.below(er.htab.hdr.high_mask + 1);
		er.htab.hdr.max_bucket = rng.below(er.htab.hdr.high_mask + 1);
		ep.htab.hdr = er.htab.hdr;
		u_int32_t bkt = bucket_for(&er.htab, er.keybuf, klen);
		er.buf.addr = bkt;
		ep.buf.addr = bkt;
		hash_mock_reset();
		hash_mock_bind_htab(&er.htab);
		hash_mock_register(&er.buf);
		hash_mock_bind_htab(&ep.htab);
		hash_mock_register(&ep.buf);
		DBT keyr, valr, keyp, valp;
		keyr.data = er.keybuf;
		keyr.size = klen;
		keyp.data = ep.keybuf;
		keyp.size = klen;
		guard_fill(er.outbuf_r, MAX_KV);
		guard_fill(ep.outbuf_p, MAX_KV);
		valr.data = er.outbuf_r;
		valr.size = 0;
		valp.data = ep.outbuf_p;
		valp.size = 0;
		int action = (int)(rng.next() % 3);
		int rr, rp;
		if (action == 0) {
			rr = ref_hash_access(&er.htab, HASH_GET, &keyr, &valr);
			rp = P::hash_access(ph(&ep.htab), (P::ACTION)HASH_GET,
			    pd(&keyp), pd(&valp));
		} else if (action == 1) {
			valr.data = er.valbuf;
			valr.size = vlen;
			valp.data = ep.valbuf;
			valp.size = vlen;
			std::memcpy(er.valbuf, vb, vlen);
			std::memcpy(ep.valbuf, vb, vlen);
			rr = ref_hash_access(&er.htab, HASH_PUT, &keyr, &valr);
			rp = P::hash_access(ph(&ep.htab), (P::ACTION)HASH_PUT,
			    pd(&keyp), pd(&valp));
		} else {
			rr = ref_hash_access(&er.htab, HASH_DELETE, &keyr, nullptr);
			rp = P::hash_access(ph(&ep.htab), (P::ACTION)HASH_DELETE,
			    pd(&keyp), nullptr);
		}
		check(F_HASH_ACCESS, rr == rp, "sweep");
		if (action == 0 && rr == 0)
			check(F_HASH_ACCESS, buf_eq(er.outbuf_r, ep.outbuf_p, MAX_KV),
			    "sweep guard");
		if (action != 0)
			check(F_HASH_ACCESS, buf_eq(er.page, ep.page, PAGE_BSIZE),
			    "sweep page");
	}
}

static void print_table()
{
	std::printf("\n%-20s %12s %12s\n", "function", "cases", "failures");
	std::printf("%-20s %12s %12s\n", "--------", "-----", "--------");
	unsigned long total_c = 0;
	unsigned long total_f = 0;
	for (int i = 0; i < F_COUNT; i++) {
		std::printf("%-20s %12lu %12lu\n", fn_name[i], n_cases[i],
		    n_fails[i]);
		total_c += n_cases[i];
		total_f += n_fails[i];
	}
	std::printf("%-20s %12lu %12lu\n", "TOTAL", total_c, total_f);
}

} // namespace

int main()
{
	Rng rng(0xB0194303ull);
	test_call_hash_edge();
	test_call_hash_sweep(rng);
	test_alloc_segs();
	test_hash_realloc();
	test_init_htab();
	test_swap_header();
	test_expand_table();
	test_hash_access();
	test_db_wrappers();
	test_hash_seq();
	test_hdestroy_flush();
	test_init_hash();
	test_hash_open();
	test_access_sweep(rng);
	print_table();
	unsigned long fails = 0;
	for (int i = 0; i < F_COUNT; i++)
		fails += n_fails[i];
	return fails ? 1 : 0;
}
