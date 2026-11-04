/*
 * Differential harness for PBSD batch b0155s4 (bt_delete.c).
 *
 * Every function of the batch is exercised against the ref_ oracle in
 * oracle.c.  Both sides run against the *same* mocked btree environment
 * (mpool, __bt_search, __bt_cmp, __bt_ret, __ovfl_delete, __bt_free), each
 * on its own private page buffer.  Both buffers are pre-filled with the
 * guard byte 0x7f and the entire buffer -- including the bytes beyond the
 * last page -- is compared afterwards, together with the BTREE state, the
 * cursor, the parent stack, errno, the mpool pin counts and a hash of the
 * full environment call trace.  Comparing only return values would let a
 * broken port through.
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

import pbsd.lib.libc.db.btree.b0155s4;

namespace P = pbsd::lib_libc_db_btree::b0155s4;

typedef P::PAGE PAGE;
typedef P::BTREE BTREE;
typedef P::CURSOR CURSOR;
typedef P::EPG EPG;
typedef P::EPGNO EPGNO;
typedef P::DBT DBT;
typedef P::DB DB;

enum {
	RET_ERROR = -1,
	RET_SUCCESS = 0,
	RET_SPECIAL = 1
};

enum {
	CURS_ACQUIRE = 0x01,
	CURS_AFTER = 0x02,
	CURS_BEFORE = 0x04,
	CURS_INIT = 0x08
};

enum {
	B_MODIFIED = 0x00004,
	B_RDONLY = 0x00010,
	B_NODUPS = 0x00020
};

enum { P_BIGDATA = 0x01, P_BIGKEY = 0x02 };

/* ------------------------------------------------------------------ */
/* Geometry of the mocked page pool.                                   */
/* ------------------------------------------------------------------ */

enum {
	NPAGE = 6,		/* pages 1..5 usable, page 0 is P_META    */
	PGSZ = 256,
	MEMSZ = NPAGE * PGSZ,
	/*
	 * A generous tail so that a linp[] slot picked up from outside the
	 * live index range (which __bt_stkacq can do) still dereferences
	 * inside the allocation.  Nothing ever writes there; if a mutated
	 * port does, the buffers diverge and the case fails.
	 */
	GUARD = 40960,
	BUFSZ = MEMSZ + GUARD,
	CMPZONE = MEMSZ + 64,	/* bytes compared byte-for-byte per case  */
	NSLOT = 10,		/* linp[] slots materialised on every page */
	NIDXMAX = 8,		/* largest NEXTINDEX() a page may carry    */
	STKMAX = 6,
	NSCRIPT = 4,
	BTDATAOFF = 20
};

enum {
	FN_DELETE = 0,
	FN_STKACQ,
	FN_BDELETE,
	FN_PDELETE,
	FN_DLEAF,
	FN_CURDEL,
	FN_RELINK,
	NFN
};

static const char *const FNNAME[NFN] = {
	"__bt_delete", "__bt_stkacq", "__bt_bdelete", "__bt_pdelete",
	"__bt_dleaf", "__bt_curdel", "__bt_relink"
};

/* Pages 1 and 2 hold BLEAF records, pages 0/3/4/5 hold BINTERNAL ones. */
static const int LEAFPG[2] = { 1, 2 };
static const int POOL_ALL[4] = { 0, 3, 4, 5 };
static const int POOL_A[2] = { 0, 3 };
static const int POOL_B[2] = { 4, 5 };
/* Values a BINTERNAL's pgno field may take (6 makes mpool_get fail). */
static const uint32_t OKPG[5] = { 0, 3, 4, 5, 6 };

/* ------------------------------------------------------------------ */
/* The oracle.                                                         */
/* ------------------------------------------------------------------ */

extern "C" {
int ref___bt_delete(const DB *, const DBT *, unsigned int);
int ref___bt_stkacq(BTREE *, PAGE **, CURSOR *);
int ref___bt_bdelete(BTREE *, const DBT *);
int ref___bt_pdelete(BTREE *, PAGE *);
int ref___bt_dleaf(BTREE *, const DBT *, PAGE *, unsigned int);
int ref___bt_curdel(BTREE *, const DBT *, PAGE *, unsigned int);
int ref___bt_relink(BTREE *, PAGE *);
}

/* ------------------------------------------------------------------ */
/* Case description.                                                   */
/* ------------------------------------------------------------------ */

struct PageCfg {
	uint32_t pgno, prevpg, nextpg, pflags;
	uint16_t nent;
	uint16_t upper;
	uint16_t linp[NSLOT];
	uint32_t f0[NSLOT];		/* ksize                          */
	uint32_t f4[NSLOT];		/* dsize (leaf) / pgno (internal) */
	uint8_t f8[NSLOT];		/* record flags                   */
};

struct Script {
	int fail;
	uint32_t pgno;
	uint16_t index;
	int exact;
	int stackn;
	uint32_t stkpg[STKMAX];
	uint16_t stkidx[STKMAX];
};

struct Env {
	PageCfg pg[NPAGE];
	uint32_t getfail;		/* bitmask: mpool_get returns NULL */
	uint32_t tflags;
	uint32_t psize;
	uint8_t cflags;
	uint32_t cpgno;
	uint16_t cindex;
	uint32_t ckeysize;
	int pinned;
	int stackn;
	uint32_t stkpg[STKMAX];
	uint16_t stkidx[STKMAX];
	Script sc[NSCRIPT];
	signed char cmp[NPAGE][NSLOT + 2];
	int ret_status, ovfl_status, free_status, fuel;
	uint32_t dbflags;
	uint32_t apgno, aidx;
	int key_null;
	uint32_t keysize;
};

/* ------------------------------------------------------------------ */
/* Mock environment state.                                             */
/* ------------------------------------------------------------------ */

alignas(16) static unsigned char g_bufA[BUFSZ];
alignas(16) static unsigned char g_bufB[BUFSZ];
static unsigned char *g_mem;
static const Env *g_env;
static int g_fuel;
static int g_searchcalls;
static uint64_t g_loghash;
static int g_logn;
static int g_pin[NPAGE];
static EPG g_epg;
static char g_retbuf[64];
static char g_keybuf[64];
static long g_mpool_cookie;

static inline void
lc(unsigned char c)
{
	g_loghash = (g_loghash ^ c) * 1099511628211ULL;
	++g_logn;
}

static inline void
ls(const char *s)
{
	while (*s)
		lc((unsigned char)*s++);
}

static void
lu(unsigned long long v)
{
	char b[24];
	int n = 0;

	if (v == 0)
		b[n++] = '0';
	while (v) {
		b[n++] = (char)('0' + (int)(v % 10));
		v /= 10;
	}
	while (n)
		lc((unsigned char)b[--n]);
}

static void
li(long long v)
{
	if (v < 0) {
		lc('-');
		lu((unsigned long long)(-(v + 1)) + 1ULL);
	} else
		lu((unsigned long long)v);
}

static inline int
fuel(void)
{
	if (g_fuel <= 0) {
		lc('!');
		return 0;
	}
	--g_fuel;
	return 1;
}

static inline long
memoff(const void *p)
{
	return (long)((const unsigned char *)p - g_mem);
}

static inline unsigned
pidx(const void *p)
{
	long d = memoff(p);

	if (d < 0 || d >= MEMSZ)
		return 0xffffu;
	return (unsigned)(d / PGSZ);
}

extern "C" void *
mpool_get(P::MPOOL *, uint32_t pgno, unsigned int flags)
{
	ls("g");
	lu(pgno);
	lc(',');
	lu(flags);
	if (!fuel()) {
		ls("=X");
		return nullptr;
	}
	if (pgno >= (uint32_t)NPAGE || ((g_env->getfail >> pgno) & 1u)) {
		ls("=X");
		return nullptr;
	}
	++g_pin[pgno];
	ls("=ok");
	return g_mem + (size_t)pgno * PGSZ;
}

extern "C" int
mpool_put(P::MPOOL *, void *p, unsigned int flags)
{
	long d = memoff(p);

	ls("p");
	li(d);
	lc(',');
	lu(flags);
	if (d >= 0 && d < MEMSZ)
		--g_pin[(unsigned)(d / PGSZ)];
	return 0;
}

extern "C" EPG *
__bt_search(BTREE *t, const DBT *key, int *exactp)
{
	int i;

	i = g_searchcalls < NSCRIPT ? g_searchcalls : NSCRIPT - 1;
	++g_searchcalls;
	ls("S");
	lu((unsigned)i);
	lc(',');
	lu((unsigned long long)(key != nullptr ? key->size : (size_t)999));
	if (!fuel()) {
		ls("=X");
		return nullptr;
	}
	const Script &s = g_env->sc[i];
	if (s.fail) {
		ls("=X");
		return nullptr;
	}
	t->bt_sp = t->bt_stack;
	for (int k = 0; k < s.stackn; ++k) {
		t->bt_sp->pgno = s.stkpg[k];
		t->bt_sp->index = s.stkidx[k];
		++t->bt_sp;
	}
	g_epg.page = (PAGE *)(g_mem + (size_t)s.pgno * PGSZ);
	g_epg.index = s.index;
	++g_pin[s.pgno];
	*exactp = s.exact;
	ls("=");
	lu(s.pgno);
	lc(',');
	lu(s.index);
	lc(',');
	li(s.exact);
	return &g_epg;
}

extern "C" int
__bt_cmp(BTREE *, const DBT *key, EPG *e)
{
	unsigned p = pidx(e->page);
	int r;

	ls("c");
	lu(p);
	lc(',');
	lu(e->index);
	lc(',');
	lu((unsigned long long)(key != nullptr ? key->size : (size_t)999));
	if (!fuel()) {
		ls("=1");
		return 1;
	}
	if (p < (unsigned)NPAGE && e->index < (unsigned)(NSLOT + 2))
		r = g_env->cmp[p][e->index];
	else
		r = 1;
	ls("=");
	li(r);
	return r;
}

extern "C" int
__bt_ret(BTREE *, EPG *e, DBT *key, DBT *data, DBT *rkey, DBT *rdata, int copy)
{
	int st;

	ls("r");
	lu(pidx(e->page));
	lc(',');
	lu(e->index);
	lc(',');
	lu((unsigned)(key != nullptr));
	lu((unsigned)(data != nullptr));
	lu((unsigned)(rkey != nullptr));
	lu((unsigned)(rdata != nullptr));
	lc(',');
	li(copy);
	if (!fuel()) {
		ls("=X");
		return RET_ERROR;
	}
	st = g_env->ret_status;
	if (st == RET_SUCCESS) {
		if (key != nullptr) {
			key->data = g_retbuf;
			key->size = (size_t)(1 + (e->index & 7));
		}
		if (data != nullptr) {
			data->data = g_retbuf;
			data->size = (size_t)(2 + (e->index & 7));
		}
	}
	ls("=");
	li(st);
	return st;
}

extern "C" int
__ovfl_delete(BTREE *, void *p)
{
	ls("o");
	li(memoff(p));
	if (!fuel()) {
		ls("=X");
		return RET_ERROR;
	}
	ls("=");
	li(g_env->ovfl_status);
	return g_env->ovfl_status;
}

extern "C" int
__bt_free(BTREE *, PAGE *h)
{
	ls("f");
	lu(pidx(h));
	if (!fuel()) {
		ls("=X");
		return RET_ERROR;
	}
	ls("=");
	li(g_env->free_status);
	return g_env->free_status;
}

/* ------------------------------------------------------------------ */
/* Random generator (fixed seed, splitmix64).                          */
/* ------------------------------------------------------------------ */

struct Rng {
	uint64_t s;

	explicit Rng(uint64_t seed) : s(seed) {}

	uint64_t next(void)
	{
		uint64_t z = (s += 0x9E3779B97F4A7C15ULL);
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
		return z ^ (z >> 31);
	}

	uint32_t n(uint32_t m) { return m ? (uint32_t)(next() % m) : 0u; }
};

/* ------------------------------------------------------------------ */
/* Page materialisation.                                               */
/* ------------------------------------------------------------------ */

static inline uint32_t
lalign(uint32_t v)
{
	return (v + 3u) & ~3u;
}

static void
layout(PageCfg &c, int isleaf, Rng &r)
{
	uint32_t nb[NSLOT];
	int perm[NSLOT];

	for (int s = 0; s < NSLOT; ++s) {
		uint32_t ks = r.n(12);
		if (isleaf) {
			uint32_t ds = r.n(12 - ks);
			c.f0[s] = ks;
			c.f4[s] = ds;
			nb[s] = lalign(9 + ks + ds);
		} else {
			c.f0[s] = ks;
			c.f4[s] = OKPG[r.n(5)];
			nb[s] = lalign(9 + ks);
		}
		c.f8[s] = (uint8_t)r.n(4);
		perm[s] = s;
	}
	for (int s = NSLOT - 1; s > 0; --s) {
		int j = (int)r.n((uint32_t)s + 1);
		int t = perm[s];
		perm[s] = perm[j];
		perm[j] = t;
	}

	uint32_t off = PGSZ;
	for (int k = 0; k < NSLOT; ++k) {
		int s = perm[k];
		off -= nb[s];
		c.linp[s] = (uint16_t)off;
	}
	/*
	 * upper is the lowest live record; everything live therefore sits
	 * at or above it and the records are packed, which is the invariant
	 * __bt_dleaf()/__bt_pdelete() rely on when they slide the page.
	 */
	uint32_t up = PGSZ;
	for (int s = 0; s < c.nent; ++s)
		if (c.linp[s] < up)
			up = c.linp[s];
	if (c.nent == 0)
		up = off;
	c.upper = (uint16_t)up;
}

static inline void
wr16(unsigned char *p, uint16_t v)
{
	memcpy(p, &v, 2);
}

static inline void
wr32(unsigned char *p, uint32_t v)
{
	memcpy(p, &v, 4);
}

static BTREE g_tree;
static DB g_db;

static void
build(const Env &E, unsigned char *buf)
{
	memset(buf, 0x7f, MEMSZ);
	for (int p = 0; p < NPAGE; ++p) {
		unsigned char *b = buf + (size_t)p * PGSZ;
		const PageCfg &c = E.pg[p];

		wr32(b + 0, c.pgno);
		wr32(b + 4, c.prevpg);
		wr32(b + 8, c.nextpg);
		wr32(b + 12, c.pflags);
		wr16(b + 16, (uint16_t)(BTDATAOFF + 2 * c.nent));
		wr16(b + 18, c.upper);
		for (int s = 0; s < NSLOT; ++s)
			wr16(b + 20 + 2 * s, c.linp[s]);
		for (int s = 0; s < NSLOT; ++s) {
			unsigned char *rec = b + c.linp[s];
			wr32(rec + 0, c.f0[s]);
			wr32(rec + 4, c.f4[s]);
			rec[8] = c.f8[s];
		}
	}

	memset(&g_tree, 0, sizeof g_tree);
	memset(&g_db, 0, sizeof g_db);
	g_tree.bt_mp = (P::MPOOL *)&g_mpool_cookie;
	g_tree.bt_dbp = &g_db;
	g_tree.bt_pinned = E.pinned < 0 ? nullptr
	    : (PAGE *)(buf + (size_t)E.pinned * PGSZ);
	g_tree.bt_cursor.pg.pgno = E.cpgno;
	g_tree.bt_cursor.pg.index = E.cindex;
	g_tree.bt_cursor.key.data = E.ckeysize ? g_keybuf : nullptr;
	g_tree.bt_cursor.key.size = E.ckeysize;
	g_tree.bt_cursor.flags = E.cflags;
	g_tree.bt_sp = g_tree.bt_stack;
	for (int k = 0; k < E.stackn; ++k) {
		g_tree.bt_sp->pgno = E.stkpg[k];
		g_tree.bt_sp->index = E.stkidx[k];
		++g_tree.bt_sp;
	}
	g_tree.bt_psize = E.psize;
	g_tree.flags = E.tflags;
	g_db.internal = &g_tree;
}

/* ------------------------------------------------------------------ */
/* Observable state after a call.                                      */
/* ------------------------------------------------------------------ */

struct Snap {
	int rc;
	int err;
	unsigned pinned;
	unsigned spdepth;
	unsigned hpout;
	uint32_t stkpg[50];
	uint16_t stkidx[50];
	uint32_t cpgno;
	uint16_t cindex;
	uint8_t cflags;
	uint32_t ckeysize;
	int ckeycode;
	uint32_t tflags;
	uint32_t psize;
	uint64_t loghash;
	int logn;
	int pin[NPAGE];
	unsigned char mem[CMPZONE];
};

static Snap g_sa, g_sb;

static void
snapshot(Snap &S, int rc, unsigned hpout, const unsigned char *buf)
{
	memset(&S, 0, sizeof S);
	S.rc = rc;
	S.err = errno;
	S.pinned = g_tree.bt_pinned == nullptr ? 0xffffu
	    : (unsigned)(((const unsigned char *)g_tree.bt_pinned - buf));
	S.spdepth = (unsigned)(g_tree.bt_sp - g_tree.bt_stack);
	S.hpout = hpout;
	for (int i = 0; i < 50; ++i) {
		S.stkpg[i] = g_tree.bt_stack[i].pgno;
		S.stkidx[i] = g_tree.bt_stack[i].index;
	}
	S.cpgno = g_tree.bt_cursor.pg.pgno;
	S.cindex = g_tree.bt_cursor.pg.index;
	S.cflags = g_tree.bt_cursor.flags;
	S.ckeysize = (uint32_t)g_tree.bt_cursor.key.size;
	S.ckeycode = g_tree.bt_cursor.key.data == nullptr ? 0
	    : (g_tree.bt_cursor.key.data == g_keybuf ? 1
	    : (g_tree.bt_cursor.key.data == g_retbuf ? 2 : 3));
	S.tflags = g_tree.flags;
	S.psize = g_tree.bt_psize;
	S.loghash = g_loghash;
	S.logn = g_logn;
	for (int i = 0; i < NPAGE; ++i)
		S.pin[i] = g_pin[i];
	memcpy(S.mem, buf, CMPZONE);
}

static void
run(const Env &E, int fn, unsigned char *buf, Snap &S, int useport)
{
	int rc = 0;
	unsigned hpout = 0xffffu;

	g_mem = buf;
	g_env = &E;
	g_fuel = E.fuel;
	g_searchcalls = 0;
	g_loghash = 1469598103934665603ULL;
	g_logn = 0;
	memset(g_pin, 0, sizeof g_pin);
	memset(g_retbuf, 0x5a, sizeof g_retbuf);
	memset(g_keybuf, 0x33, sizeof g_keybuf);
	build(E, buf);

	PAGE *h = (PAGE *)(buf + (size_t)(E.apgno % NPAGE) * PGSZ);
	DBT key;
	key.data = g_keybuf;
	key.size = E.keysize;
	const DBT *kp = E.key_null ? nullptr : &key;

	errno = 0;
	switch (fn) {
	case FN_DELETE:
		rc = useport ? P::__bt_delete(&g_db, &key, E.dbflags)
		    : ref___bt_delete(&g_db, &key, E.dbflags);
		break;
	case FN_STKACQ: {
		PAGE *hp = h;
		++g_pin[E.apgno % NPAGE];	/* the caller's pin */
		rc = useport ? P::__bt_stkacq(&g_tree, &hp, &g_tree.bt_cursor)
		    : ref___bt_stkacq(&g_tree, &hp, &g_tree.bt_cursor);
		hpout = hp == nullptr ? 0xfffeu : pidx(hp);
		break;
	}
	case FN_BDELETE:
		rc = useport ? P::__bt_bdelete(&g_tree, &key)
		    : ref___bt_bdelete(&g_tree, &key);
		break;
	case FN_PDELETE:
		rc = useport ? P::__bt_pdelete(&g_tree, h)
		    : ref___bt_pdelete(&g_tree, h);
		break;
	case FN_DLEAF:
		rc = useport ? P::__bt_dleaf(&g_tree, kp, h, E.aidx)
		    : ref___bt_dleaf(&g_tree, kp, h, E.aidx);
		break;
	case FN_CURDEL:
		rc = useport ? P::__bt_curdel(&g_tree, kp, h, E.aidx)
		    : ref___bt_curdel(&g_tree, kp, h, E.aidx);
		break;
	case FN_RELINK:
		rc = useport ? P::__bt_relink(&g_tree, h)
		    : ref___bt_relink(&g_tree, h);
		break;
	}
	snapshot(S, rc, hpout, buf);
}

static int
runcase(const Env &E, int fn)
{
	run(E, fn, g_bufA, g_sa, 1);
	run(E, fn, g_bufB, g_sb, 0);
	return memcmp(&g_sa, &g_sb, sizeof(Snap)) == 0;
}

/* ------------------------------------------------------------------ */
/* Safety fixups: keep every generated case inside the well-formedness */
/* envelope the original code assumes, without weakening any check.    */
/* ------------------------------------------------------------------ */

static void
fixStack(Env &E, int n, uint32_t *pg, uint16_t *idx, const int *pool, int npool)
{
	for (int k = 0; k < n; ++k) {
		pg[k] = (uint32_t)pool[pg[k] % (uint32_t)npool];
		uint16_t ne = E.pg[pg[k]].nent;
		idx[k] = (uint16_t)(idx[k] % ne);	/* ne >= 1 always */
	}
}

static void
fixupSafety(Env &E, int fn)
{
	int i, k;

	for (k = 0; k < 4; ++k) {
		int p = POOL_ALL[k];
		for (int s = 0; s < NSLOT; ++s)
			E.pg[p].f4[s] = OKPG[E.pg[p].f4[s] % 5];
	}

	E.apgno %= NPAGE;
	E.cpgno %= NPAGE;
	if (E.pinned >= NPAGE)
		E.pinned = NPAGE - 1;
	if (E.fuel < 4)
		E.fuel = 4;

	const int *pool0 = POOL_ALL, *pool1 = POOL_ALL;
	int np0 = 4, np1 = 4;
	if (fn == FN_DELETE || fn == FN_BDELETE) {
		/*
		 * __bt_bdelete() can run two search passes and therefore two
		 * __bt_pdelete() calls; give each pass its own parent pages
		 * so a page reset to an empty root by pass one is never
		 * re-parsed by pass two.
		 */
		pool0 = POOL_A;
		np0 = 2;
		pool1 = POOL_B;
		np1 = 2;
	}
	fixStack(E, E.stackn, E.stkpg, E.stkidx, POOL_ALL, 4);
	fixStack(E, E.sc[0].stackn, E.sc[0].stkpg, E.sc[0].stkidx, pool0, np0);
	for (i = 1; i < NSCRIPT; ++i)
		fixStack(E, E.sc[i].stackn, E.sc[i].stkpg, E.sc[i].stkidx,
		    pool1, np1);

	for (i = 0; i < NSCRIPT; ++i)
		E.sc[i].pgno = (uint32_t)LEAFPG[E.sc[i].pgno % 2];

	switch (fn) {
	case FN_DELETE:
	case FN_BDELETE:
		E.sc[1].pgno = E.sc[0].pgno == 1u ? 2u : 1u;
		E.sc[2].exact = 0;
		E.sc[3].exact = 0;
		for (i = 0; i < 2; ++i) {
			uint16_t ne = E.pg[E.sc[i].pgno].nent;
			if (ne == 0)
				E.sc[i].exact = 0;
			else
				E.sc[i].index = (uint16_t)(E.sc[i].index % ne);
		}
		if (fn == FN_DELETE) {
			E.cpgno = (uint32_t)LEAFPG[E.cpgno % 2];
			E.cindex = (uint16_t)(E.cindex %
			    E.pg[E.cpgno].nent);
			/*
			 * When the R_CURSOR path has to acquire a stack we
			 * make __bt_stkacq() either fail outright or find
			 * the cursor page in one shot; the walking form
			 * would leave a stack __bt_pdelete() cannot parse.
			 */
			if (!E.sc[0].fail)
				E.pg[E.sc[0].pgno].pgno = E.cpgno;
		}
		break;
	case FN_STKACQ:
		/*
		 * Guarantee a parent that satisfies the "move to the next
		 * index" test, otherwise __bt_stkacq() reads its
		 * uninitialised idx and the two runs would differ for
		 * reasons that have nothing to do with the port.
		 */
		for (i = 0; i < NSCRIPT; ++i) {
			if (E.sc[i].stackn <= 0)
				continue;
			E.sc[i].stkpg[0] = 3;
			E.sc[i].stkidx[0] = (uint16_t)(1 +
			    (E.sc[i].stkidx[0] % (uint16_t)(E.pg[3].nent - 2)));
		}
		/*
		 * ... and keep the sibling walk to a single stack dance per
		 * direction so a replaced bottom-of-stack entry can never be
		 * consulted a second time.
		 */
		if (E.sc[0].stackn > 0 || E.sc[1].stackn > 0 ||
		    E.sc[2].stackn > 0 || E.sc[3].stackn > 0) {
			E.pg[1].nextpg = (E.pg[1].nextpg & 1u) ? 2u : 0u;
			E.pg[1].prevpg = (E.pg[1].prevpg & 1u) ? 2u : 0u;
			E.pg[2].nextpg = 0;
			E.pg[2].prevpg = 0;
		}
		break;
	case FN_DLEAF:
		E.apgno = (uint32_t)LEAFPG[E.apgno % 2];
		E.aidx %= E.pg[E.apgno].nent;
		break;
	case FN_CURDEL:
		E.apgno = (uint32_t)LEAFPG[E.apgno % 2];
		E.aidx %= (uint32_t)(NIDXMAX + 2);
		break;
	default:
		break;
	}
}

static void
genEnv(Env &E, Rng &r, int fn, const int *nreq)
{
	memset(&E, 0, sizeof E);

	for (int p = 0; p < NPAGE; ++p) {
		int isleaf = (p == 1 || p == 2);
		int n;

		if (nreq != nullptr && nreq[p] >= 0)
			n = nreq[p];
		else
			n = (int)r.n(NIDXMAX + 1);
		if (!isleaf && n < 1)
			n = 1;
		if (isleaf && n < 1 &&
		    (fn == FN_DELETE || fn == FN_BDELETE || fn == FN_DLEAF))
			n = 1;
		if (fn == FN_STKACQ && p == 3 && n < 3)
			n = 3;
		if (n > NIDXMAX)
			n = NIDXMAX;
		E.pg[p].nent = (uint16_t)n;
		layout(E.pg[p], isleaf, r);

		E.pg[p].pgno = r.n(3) == 0 ? 1u : r.n(NPAGE);
		E.pg[p].prevpg = r.n(NPAGE + 1);
		E.pg[p].nextpg = r.n(NPAGE + 1);
		E.pg[p].pflags = r.n(2) ? 0x02u : 0x01u;
	}

	E.getfail = r.n(4) == 0 ? r.n(64) : 0u;
	E.tflags = (r.n(2) ? B_NODUPS : 0u) | (r.n(6) == 0 ? B_RDONLY : 0u);
	static const uint32_t PSZ[4] = { 256, 512, 65536 + 256, 40 };
	E.psize = PSZ[r.n(4)];
	E.cflags = (uint8_t)(r.n(2) ? (CURS_INIT | r.n(8)) : r.n(16));
	E.cpgno = r.n(NPAGE);
	E.cindex = (uint16_t)r.n(NIDXMAX + 2);
	E.ckeysize = r.n(8);
	E.pinned = (int)r.n(NPAGE + 1) - 1;
	E.stackn = (int)r.n(STKMAX + 1);
	for (int k = 0; k < STKMAX; ++k) {
		E.stkpg[k] = r.n(4);
		E.stkidx[k] = (uint16_t)r.n(NIDXMAX);
	}
	for (int i = 0; i < NSCRIPT; ++i) {
		E.sc[i].fail = r.n(6) == 0;
		E.sc[i].pgno = r.n(2);
		E.sc[i].index = (uint16_t)r.n(NIDXMAX);
		E.sc[i].exact = r.n(3) != 0;
		E.sc[i].stackn = (int)r.n(STKMAX + 1);
		for (int k = 0; k < STKMAX; ++k) {
			E.sc[i].stkpg[k] = r.n(4);
			E.sc[i].stkidx[k] = (uint16_t)r.n(NIDXMAX);
		}
	}
	for (int p = 0; p < NPAGE; ++p)
		for (int s = 0; s < NSLOT + 2; ++s)
			E.cmp[p][s] = (signed char)(r.n(3) == 0 ? 0
			    : (r.n(2) ? 1 : -1));
	static const int ST[3] = { RET_SUCCESS, RET_ERROR, RET_SPECIAL };
	E.ret_status = r.n(3) == 0 ? ST[r.n(3)] : RET_SUCCESS;
	E.ovfl_status = r.n(3) == 0 ? RET_ERROR : RET_SUCCESS;
	E.free_status = r.n(4) == 0 ? RET_ERROR : RET_SUCCESS;
	E.fuel = 12 + (int)r.n(50);
	static const uint32_t DF[6] = { 0, 1, 0, 1, 2, 7 };
	E.dbflags = DF[r.n(6)];
	E.apgno = r.n(NPAGE);
	E.aidx = r.n(NIDXMAX + 2);
	E.key_null = (int)r.n(2);
	E.keysize = r.n(8);

	fixupSafety(E, fn);

	/* Bias the cursor onto the page under test so the cursor-delete
	 * arms of __bt_dleaf()/__bt_curdel() are actually reached. */
	if (fn == FN_DLEAF || fn == FN_CURDEL) {
		if (r.n(2)) {
			E.cpgno = E.pg[E.apgno].pgno;
			E.cindex = (uint16_t)(E.aidx + r.n(3) - 1);
		}
	}
}

/* ------------------------------------------------------------------ */
/* Result accounting.                                                  */
/* ------------------------------------------------------------------ */

static long g_cases[NFN];
static long g_fails[NFN];
static int g_shown;

static void
check(const Env &E, int fn, const char *what, long which)
{
	++g_cases[fn];
	if (runcase(E, fn))
		return;
	++g_fails[fn];
	if (g_shown < 12) {
		++g_shown;
		printf("  MISMATCH %-12s %s #%ld: rc %d/%d errno %d/%d "
		    "log %016llx/%016llx (%d/%d) mem %s\n",
		    FNNAME[fn], what, which, g_sa.rc, g_sb.rc,
		    g_sa.err, g_sb.err,
		    (unsigned long long)g_sa.loghash,
		    (unsigned long long)g_sb.loghash,
		    g_sa.logn, g_sb.logn,
		    memcmp(g_sa.mem, g_sb.mem, CMPZONE) ? "differs" : "same");
	}
}

/* ------------------------------------------------------------------ */
/* Hand-written edge suites.                                           */
/* ------------------------------------------------------------------ */

static void
edge_relink(void)
{
	Env E;
	long c = 0;

	for (int ap = 0; ap < NPAGE; ++ap)
	for (int nx = 0; nx <= NPAGE; ++nx)
	for (int pv = 0; pv <= NPAGE; ++pv)
	for (int gf = 0; gf < 4; ++gf) {
		Rng r(0x1000 + c);
		genEnv(E, r, FN_RELINK, nullptr);
		E.apgno = (uint32_t)ap;
		E.pg[ap].nextpg = (uint32_t)nx;
		E.pg[ap].prevpg = (uint32_t)pv;
		E.getfail = gf == 0 ? 0u
		    : gf == 1 ? (nx < NPAGE ? (1u << nx) : 0u)
		    : gf == 2 ? (pv < NPAGE ? (1u << pv) : 0u)
		    : 0x3fu;
		E.fuel = 64;
		fixupSafety(E, FN_RELINK);
		check(E, FN_RELINK, "edge", c++);
	}
}

static void
edge_dleaf(void)
{
	Env E;
	long c = 0;
	static const int CF[5] = { 0, CURS_INIT, CURS_INIT | CURS_ACQUIRE,
	    CURS_INIT | CURS_AFTER, CURS_ACQUIRE };

	for (int ne = 1; ne <= NIDXMAX; ++ne)
	for (int iw = 0; iw < 3; ++iw)
	for (int cf = 0; cf < 5; ++cf)
	for (int cd = 0; cd < 3; ++cd)
	for (int cm = 0; cm < 2; ++cm)
	for (int rf = 0; rf < 4; ++rf)
	for (int ov = 0; ov < 2; ++ov)
	for (int nd = 0; nd < 2; ++nd) {
		int nreq[NPAGE];
		for (int p = 0; p < NPAGE; ++p)
			nreq[p] = -1;
		nreq[1] = ne;
		nreq[2] = ne;

		Rng r(0x20000 + c);
		genEnv(E, r, FN_DLEAF, nreq);
		int idx = iw == 0 ? 0 : (iw == 1 ? ne / 2 : ne - 1);
		E.aidx = (uint32_t)idx;
		E.cflags = (uint8_t)CF[cf];
		E.cindex = (uint16_t)(idx + cd - 1);
		E.cpgno = cm ? E.pg[E.apgno % NPAGE].pgno
		    : E.pg[E.apgno % NPAGE].pgno + 1u;
		E.pg[E.apgno % NPAGE].f8[idx] = (uint8_t)rf;
		E.ovfl_status = ov ? RET_ERROR : RET_SUCCESS;
		E.tflags = nd ? B_NODUPS : 0u;
		E.ret_status = RET_SUCCESS;
		E.fuel = 64;
		fixupSafety(E, FN_DLEAF);
		E.pg[E.apgno].f8[E.aidx] = (uint8_t)rf;
		check(E, FN_DLEAF, "edge", c++);
	}
}

static void
edge_curdel(void)
{
	Env E;
	long c = 0;
	static const int NE[5] = { 0, 1, 2, 3, NIDXMAX };
	static const int RS[3] = { RET_SUCCESS, RET_ERROR, RET_SPECIAL };

	for (int nei = 0; nei < 5; ++nei)
	for (int iw = 0; iw < 3; ++iw)
	for (int kn = 0; kn < 2; ++kn)
	for (int nd = 0; nd < 2; ++nd)
	for (int rs = 0; rs < 3; ++rs)
	for (int cmask = 0; cmask < 16; ++cmask)
	for (int pv = 0; pv < 2; ++pv)
	for (int nx = 0; nx < 2; ++nx) {
		int ne = NE[nei];
		int nreq[NPAGE];
		for (int p = 0; p < NPAGE; ++p)
			nreq[p] = -1;
		nreq[1] = ne;
		nreq[2] = ne;

		Rng r(0x40000 + c);
		genEnv(E, r, FN_CURDEL, nreq);
		int idx = iw == 0 ? 0 : (iw == 1 ? 1 : (ne ? ne - 1 : 0));
		E.aidx = (uint32_t)idx;
		E.key_null = kn;
		E.tflags = nd ? B_NODUPS : 0u;
		E.ret_status = RS[rs];
		E.pg[E.apgno].prevpg = pv ? 3u : 0u;
		E.pg[E.apgno].nextpg = nx ? 4u : 0u;
		E.getfail = 0;
		E.fuel = 64;
		fixupSafety(E, FN_CURDEL);
		int a = E.apgno;
		if (idx > 0)
			E.cmp[a][idx - 1] = (signed char)((cmask & 1) ? 0 : 1);
		if (idx + 1 < NSLOT + 2)
			E.cmp[a][idx + 1] = (signed char)((cmask & 2) ? 0 : 1);
		if (E.pg[a].prevpg < NPAGE) {
			int q = E.pg[a].prevpg;
			int j = E.pg[q].nent ? E.pg[q].nent - 1 : 0;
			E.cmp[q][j] = (signed char)((cmask & 4) ? 0 : 1);
		}
		if (E.pg[a].nextpg < NPAGE)
			E.cmp[E.pg[a].nextpg][0] =
			    (signed char)((cmask & 8) ? 0 : 1);
		check(E, FN_CURDEL, "edge", c++);
	}
}

static void
edge_pdelete(void)
{
	Env E;
	long c = 0;

	for (int sn = 0; sn < 3; ++sn)
	for (int sp = 0; sp < 2; ++sp)
	for (int ne = 1; ne <= 3; ++ne)
	for (int iw = 0; iw < 2; ++iw)
	for (int rt = 0; rt < 2; ++rt)
	for (int rf = 0; rf < 2; ++rf)
	for (int ov = 0; ov < 2; ++ov)
	for (int fr = 0; fr < 2; ++fr)
	for (int hr = 0; hr < 2; ++hr)
	for (int hn = 0; hn < 2; ++hn)
	for (int gf = 0; gf < 2; ++gf) {
		int page = POOL_ALL[sp];
		int nreq[NPAGE];
		for (int p = 0; p < NPAGE; ++p)
			nreq[p] = -1;
		nreq[page] = ne;

		Rng r(0x80000 + c);
		genEnv(E, r, FN_PDELETE, nreq);
		E.stackn = sn;
		for (int k = 0; k < sn; ++k) {
			E.stkpg[k] = (uint32_t)page;
			E.stkidx[k] = (uint16_t)(iw ? ne - 1 : 0);
		}
		E.pg[page].pgno = rt ? 1u : 2u;
		E.pg[page].f8[iw ? ne - 1 : 0] = (uint8_t)(rf ? P_BIGKEY : 0);
		E.ovfl_status = ov ? RET_ERROR : RET_SUCCESS;
		E.free_status = fr ? RET_ERROR : RET_SUCCESS;
		E.apgno = 2;
		E.pg[2].pgno = hr ? 1u : 5u;
		E.pg[2].nextpg = hn ? 4u : 0u;
		E.pg[2].prevpg = hn ? 0u : 3u;
		E.getfail = gf ? (1u << page) : 0u;
		E.psize = 256;
		E.fuel = 64;
		fixupSafety(E, FN_PDELETE);
		E.pg[page].f8[E.stackn ? E.stkidx[0] : 0] =
		    (uint8_t)(rf ? P_BIGKEY : 0);
		check(E, FN_PDELETE, "edge", c++);
	}
}

static void
edge_stkacq(void)
{
	Env E;
	long c = 0;

	for (int cp = 0; cp < NPAGE; ++cp)
	for (int s0 = 0; s0 < 2; ++s0)
	for (int f0 = 0; f0 < 2; ++f0)
	for (int f1 = 0; f1 < 2; ++f1)
	for (int sn = 0; sn < 4; ++sn)
	for (int p1n = 0; p1n < 3; ++p1n)
	for (int p1p = 0; p1p < 3; ++p1p)
	for (int hit = 0; hit < 3; ++hit)
	for (int gf = 0; gf < 2; ++gf) {
		Rng r(0x100000 + c);
		genEnv(E, r, FN_STKACQ, nullptr);
		E.cpgno = (uint32_t)cp;
		E.sc[0].pgno = (uint32_t)LEAFPG[s0];
		E.sc[1].pgno = (uint32_t)LEAFPG[s0];
		E.sc[0].fail = f0;
		E.sc[1].fail = f1;
		E.sc[0].stackn = sn;
		E.sc[1].stackn = sn;
		for (int k = 0; k < sn; ++k) {
			E.sc[0].stkpg[k] = (uint32_t)POOL_ALL[k % 4];
			E.sc[0].stkidx[k] = (uint16_t)k;
			E.sc[1].stkpg[k] = (uint32_t)POOL_ALL[k % 4];
			E.sc[1].stkidx[k] = (uint16_t)k;
		}
		E.pg[1].nextpg = (uint32_t)(p1n == 0 ? 0 : (p1n == 1 ? 2 : 6));
		E.pg[1].prevpg = (uint32_t)(p1p == 0 ? 0 : (p1p == 1 ? 2 : 6));
		E.pg[2].nextpg = 0;
		E.pg[2].prevpg = 0;
		/* hit: page content number vs the cursor's page number */
		E.pg[1].pgno = (uint32_t)(hit == 0 ? cp : (cp + 1) % NPAGE);
		E.pg[2].pgno = (uint32_t)(hit == 1 ? cp : (cp + 2) % NPAGE);
		E.apgno = (uint32_t)(hit == 2 ? 1 : 2);
		E.getfail = gf ? (1u << ((cp + 1) % NPAGE)) : 0u;
		E.fuel = 64;
		fixupSafety(E, FN_STKACQ);
		check(E, FN_STKACQ, "edge", c++);
	}
}

static void
edge_bdelete(void)
{
	Env E;
	long c = 0;

	for (int ne = 1; ne <= 4; ++ne)
	for (int iw = 0; iw < 3; ++iw)
	for (int e0 = 0; e0 < 2; ++e0)
	for (int f0 = 0; f0 < 2; ++f0)
	for (int e1 = 0; e1 < 2; ++e1)
	for (int nd = 0; nd < 2; ++nd)
	for (int cmode = 0; cmode < 4; ++cmode)
	for (int sn = 0; sn < 3; ++sn)
	for (int ov = 0; ov < 2; ++ov)
	for (int fu = 0; fu < 2; ++fu) {
		int nreq[NPAGE];
		for (int p = 0; p < NPAGE; ++p)
			nreq[p] = -1;
		nreq[1] = ne;
		nreq[2] = ne;

		Rng r(0x200000 + c);
		genEnv(E, r, FN_BDELETE, nreq);
		E.sc[0].pgno = 1;
		E.sc[0].exact = e0;
		E.sc[0].fail = f0;
		E.sc[0].index = (uint16_t)(iw == 0 ? 0
		    : (iw == 1 ? ne / 2 : ne - 1));
		E.sc[0].stackn = sn;
		E.sc[1].exact = e1;
		E.sc[1].fail = 0;
		E.sc[1].index = 0;
		E.sc[1].stackn = sn;
		E.sc[2].fail = 0;
		E.sc[3].fail = 0;
		E.tflags = nd ? B_NODUPS : 0u;
		E.ovfl_status = ov ? RET_ERROR : RET_SUCCESS;
		E.cflags = 0;
		E.getfail = 0;
		E.fuel = fu ? 64 : 14;
		E.psize = 256;
		fixupSafety(E, FN_BDELETE);
		for (int p = 0; p < NPAGE; ++p)
			for (int s = 0; s < NSLOT + 2; ++s)
				E.cmp[p][s] = (signed char)(cmode == 0 ? 0
				    : cmode == 1 ? 1
				    : ((s & 1) ? 0 : 1));
		if (cmode == 3)
			for (int s = 0; s < NSLOT + 2; ++s)
				E.cmp[1][s] = (signed char)(s < ne / 2 ? 1 : 0);
		check(E, FN_BDELETE, "edge", c++);
	}
}

static void
edge_delete(void)
{
	Env E;
	long c = 0;
	static const uint32_t DF[5] = { 0, 1, 2, 3, 0xffffffffu };
	static const int CF[6] = { 0, CURS_INIT, CURS_INIT | CURS_ACQUIRE,
	    CURS_INIT | CURS_AFTER, CURS_INIT | CURS_BEFORE,
	    CURS_ACQUIRE | CURS_AFTER };

	for (int df = 0; df < 5; ++df)
	for (int ro = 0; ro < 2; ++ro)
	for (int pn = -1; pn < 2; ++pn)
	for (int cf = 0; cf < 6; ++cf)
	for (int ne = 1; ne <= 3; ++ne)
	for (int iw = 0; iw < 2; ++iw)
	for (int f0 = 0; f0 < 2; ++f0)
	for (int e0 = 0; e0 < 2; ++e0)
	for (int nd = 0; nd < 2; ++nd)
	for (int sn = 0; sn < 3; ++sn)
	for (int gf = 0; gf < 2; ++gf) {
		int nreq[NPAGE];
		for (int p = 0; p < NPAGE; ++p)
			nreq[p] = -1;
		nreq[1] = ne;
		nreq[2] = ne;

		Rng r(0x400000 + c);
		genEnv(E, r, FN_DELETE, nreq);
		E.dbflags = DF[df];
		E.tflags = (ro ? B_RDONLY : 0u) | (nd ? B_NODUPS : 0u);
		E.pinned = pn < 0 ? -1 : (pn == 0 ? 0 : 3);
		E.cflags = (uint8_t)CF[cf];
		E.cpgno = 1;
		E.cindex = (uint16_t)(iw ? ne - 1 : 0);
		E.sc[0].fail = f0;
		E.sc[0].exact = e0;
		E.sc[0].pgno = 1;
		E.sc[0].index = 0;
		E.sc[0].stackn = sn;
		E.sc[1].stackn = sn;
		E.getfail = gf ? (1u << 1) : 0u;
		E.psize = 256;
		E.fuel = 64;
		fixupSafety(E, FN_DELETE);
		check(E, FN_DELETE, "edge", c++);
	}
}

/* ------------------------------------------------------------------ */

int
main(void)
{
	long sweep = 200000;
	int i;

	memset(g_bufA, 0x7f, sizeof g_bufA);
	memset(g_bufB, 0x7f, sizeof g_bufB);

	edge_delete();
	edge_stkacq();
	edge_bdelete();
	edge_pdelete();
	edge_dleaf();
	edge_curdel();
	edge_relink();

	long edgecases[NFN];
	for (i = 0; i < NFN; ++i)
		edgecases[i] = g_cases[i];

	Env E;
	for (int fn = 0; fn < NFN; ++fn) {
		Rng r(0xC0FFEE0000ULL + (uint64_t)fn * 0x9E3779B97F4A7C15ULL);
		for (long k = 0; k < sweep; ++k) {
			genEnv(E, r, fn, nullptr);
			check(E, fn, "rand", k);
		}
	}

	/* The far guard must be pristine in both buffers. */
	long guardbad = 0;
	for (long o = CMPZONE; o < BUFSZ; ++o)
		if (g_bufA[o] != 0x7f || g_bufB[o] != 0x7f)
			++guardbad;

	long tc = 0, tf = 0;
	printf("\n");
	printf("%-14s %10s %10s %10s %10s\n",
	    "function", "edge", "random", "cases", "failures");
	printf("-------------------------------------------------------------"
	    "-\n");
	for (i = 0; i < NFN; ++i) {
		printf("%-14s %10ld %10ld %10ld %10ld\n", FNNAME[i],
		    edgecases[i], g_cases[i] - edgecases[i], g_cases[i],
		    g_fails[i]);
		tc += g_cases[i];
		tf += g_fails[i];
	}
	printf("-------------------------------------------------------------"
	    "-\n");
	printf("%-14s %10s %10s %10ld %10ld\n", "TOTAL", "", "", tc, tf);
	printf("guard bytes clobbered: %ld\n", guardbad);
	printf("\n%s\n", (tf == 0 && guardbad == 0) ? "PASS" : "FAIL");
	return (tf == 0 && guardbad == 0) ? 0 : 1;
}
