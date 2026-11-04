/*
 * harness.cpp -- differential test for the b0155s1 batch.
 *
 * The C++23 port and the C oracle are driven side by side over two identically
 * initialised environments.  bt_put.c reaches out to mpool and to the rest of
 * the btree access method; those entry points are provided here once, with C
 * linkage, so both implementations exercise the very same environment and every
 * call they make is recorded.  After each call the two environments are
 * compared in full: return value, errno, every page buffer including the guard
 * bytes on both sides of the page, the key and data buffers, the whole BTREE
 * (pointers reduced to identities), and the recorded call trace.
 */

#include <sys/types.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

import pbsd.lib.libc.db.btree.b0155s1;

namespace port = pbsd::lib_libc_db_btree::b0155s1;

/*
 * Macros are not module artefacts, so the ones bt_put.c relies on are restated
 * here.  They must agree with port.cppm and oracle.c.
 */
#define	RET_ERROR	-1
#define	RET_SUCCESS	 0
#define	RET_SPECIAL	 1

#define	R_CURSOR	 1
#define	R_NOOVERWRITE	 8
#define	R_SETCURSOR	10

#define	P_INVALID	 0xffffffff

#define	P_BLEAF		0x02

#define	LALIGN(n)	(((n) + sizeof(pgno_t) - 1) & ~(sizeof(pgno_t) - 1))
#define	NOVFLSIZE	(sizeof(pgno_t) + sizeof(u_int32_t))

#define	BTDATAOFF							\
	(sizeof(pgno_t) + sizeof(pgno_t) + sizeof(pgno_t) +		\
	    sizeof(u_int32_t) + sizeof(indx_t) + sizeof(indx_t))
#define	NEXTINDEX(p)	(((p)->lower - BTDATAOFF) / sizeof(indx_t))

#define	P_BIGDATA	0x01
#define	P_BIGKEY	0x02

#define	NBLEAFDBT(ksize, dsize)						\
	LALIGN(sizeof(u_int32_t) + sizeof(u_int32_t) + sizeof(u_char) +	\
	    (ksize) + (dsize))

#define	CURS_ACQUIRE	0x01
#define	CURS_AFTER	0x02
#define	CURS_BEFORE	0x04
#define	CURS_INIT	0x08

#define	B_MODIFIED	0x00004
#define	B_RDONLY	0x00010
#define	B_NODUPS	0x08000

#define	F_ISSET(p, f)	((p)->flags & (f))
#define	F_SET(p, f)	((p)->flags |= (f))
#define	F_CLR(p, f)	((p)->flags &= ~(f))

#define	MPOOL_DIRTY	0x01

extern "C" int ref___bt_put(const DB *, DBT *, const DBT *, u_int);
extern "C" EPG *ref_bt_fast(BTREE *, const DBT *, const DBT *, int *);

/* ------------------------------------------------------------------------- */

#define	PSIZE		512
#define	NPG		5
#define	GUARD		32
#define	KDMAX		320
#define	GUARDBYTE	0x7f

struct alignas(16) PageBuf {
	unsigned char raw[GUARD + PSIZE + GUARD];
};

struct Log {
	static const size_t CAP = 1u << 15;
	char b[CAP];
	size_t n;
	size_t total;

	void reset() { n = 0; total = 0; b[0] = '\0'; }
	void ch(char c)
	{
		total++;
		if (n + 1 < CAP) {
			b[n++] = c;
			b[n] = '\0';
		}
	}
	void str(const char *s) { while (*s != '\0') ch(*s++); }
	void num(unsigned long long v)
	{
		char t[24];
		int i = 0;

		if (v == 0) {
			ch('0');
			return;
		}
		while (v != 0) {
			t[i++] = (char)('0' + (int)(v % 10));
			v /= 10;
		}
		while (i != 0)
			ch(t[--i]);
	}
	void snum(long long v)
	{
		if (v < 0) {
			ch('-');
			num((unsigned long long)(-v));
		} else
			num((unsigned long long)v);
	}
	void hex(const void *p, size_t len)
	{
		static const char d[] = "0123456789abcdef";
		const unsigned char *q = (const unsigned char *)p;

		for (size_t i = 0; i < len; i++) {
			ch(d[q[i] >> 4]);
			ch(d[q[i] & 0xf]);
		}
	}
};

struct Env {
	/* environment configuration -- identical in both copies */
	int npages_valid;
	int ovfl_ret;
	pgno_t ovfl_pgno;
	int ovfl_calls;
	int search_null;
	int search_page;
	unsigned search_index;
	int search_exact;
	int dleaf_ret;
	int dleaf_grow;
	int split_ret;
	int setcur_calls;

	Log log;
	DB db;
	BTREE t;
	DBT key;
	DBT data;
	unsigned char kbuf[GUARD + KDMAX + GUARD];
	unsigned char dbuf[GUARD + KDMAX + GUARD];
	PageBuf pages[NPG];
};

static Env EA;
static Env EB;
static Env ESCRATCH;

static inline PAGE *
pg_ptr(Env *e, int i)
{
	return (PAGE *)(void *)(e->pages[i].raw + GUARD);
}

static int
pg_id(Env *e, const void *p)
{
	if (p == NULL)
		return (-1);
	for (int i = 0; i < NPG; i++)
		if ((const void *)pg_ptr(e, i) == p)
			return (i);
	return (-2);
}

static unsigned
rnd32(unsigned *s)
{
	unsigned x = *s;

	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	*s = x;
	return (x);
}

/*
 * Byte generator: NULs, 0xff, 0x80 and the rest of the high-bit range all turn
 * up alongside plain ASCII.
 */
static void
fillbuf(unsigned char *p, unsigned n, unsigned seed)
{
	unsigned s = seed | 1u;

	for (unsigned i = 0; i < n; i++) {
		unsigned v = rnd32(&s);

		switch ((v >> 24) % 6u) {
		case 0:
			p[i] = 0x00;
			break;
		case 1:
			p[i] = 0xff;
			break;
		case 2:
			p[i] = (unsigned char)(0x80u | (v & 0x7fu));
			break;
		case 3:
			p[i] = (unsigned char)(v & 0x7fu);
			break;
		case 4:
			p[i] = 0x80;
			break;
		default:
			p[i] = (unsigned char)v;
			break;
		}
	}
}

/* ------------------------- shared environment ---------------------------- */

static void
logdbt(Env *e, const DBT *d)
{
	size_t n = d->size;

	e->log.ch('[');
	e->log.num(d->size);
	e->log.ch(':');
	if (n > 512)
		n = 512;
	e->log.hex(d->data, n);
	e->log.ch(']');
}

static size_t
safe_nextindex(PAGE *h)
{
	size_t ni;

	if (h == NULL || h->lower < (indx_t)BTDATAOFF)
		return (0);
	ni = NEXTINDEX(h);
	if (ni > 200)
		return (0);
	return (ni);
}

extern "C" void *
mpool_get(MPOOL *mp, pgno_t pgno, u_int flags)
{
	Env *e = (Env *)(void *)mp;
	void *r = NULL;

	e->log.str("mpool_get ");
	e->log.num(pgno);
	e->log.ch(' ');
	e->log.num(flags);
	if (pgno < (pgno_t)e->npages_valid)
		r = (void *)pg_ptr(e, (int)pgno);
	e->log.str(" -> ");
	e->log.snum(r != NULL ? (long long)pgno : -1);
	e->log.ch('\n');
	return (r);
}

extern "C" int
mpool_put(MPOOL *mp, void *page, u_int flags)
{
	Env *e = (Env *)(void *)mp;

	e->log.str("mpool_put ");
	e->log.snum(pg_id(e, page));
	e->log.ch(' ');
	e->log.num(flags);
	e->log.ch('\n');
	return (RET_SUCCESS);
}

extern "C" int
__ovfl_put(BTREE *t, const DBT *dbt, pgno_t *pg)
{
	Env *e = (Env *)(void *)t->bt_mp;

	e->log.str("ovfl_put ");
	logdbt(e, dbt);
	if (e->ovfl_ret == RET_ERROR) {
		e->log.str(" -> ERR\n");
		return (RET_ERROR);
	}
	*pg = e->ovfl_pgno + (pgno_t)(e->ovfl_calls * 7 + 1);
	e->ovfl_calls++;
	e->log.str(" -> ");
	e->log.num(*pg);
	e->log.ch('\n');
	return (RET_SUCCESS);
}

extern "C" EPG *
__bt_search(BTREE *t, const DBT *key, int *exactp)
{
	Env *e = (Env *)(void *)t->bt_mp;

	e->log.str("search ");
	logdbt(e, key);
	if (e->search_null) {
		e->log.str(" -> NULL\n");
		return (NULL);
	}
	t->bt_cur.page = pg_ptr(e, e->search_page);
	t->bt_cur.index = (indx_t)e->search_index;
	*exactp = e->search_exact;
	e->log.str(" -> ");
	e->log.num((unsigned)e->search_page);
	e->log.ch('/');
	e->log.num(e->search_index);
	e->log.ch('/');
	e->log.num((unsigned)e->search_exact);
	e->log.ch('\n');
	return (&t->bt_cur);
}

extern "C" int
__bt_dleaf(BTREE *t, const DBT *key, PAGE *h, u_int idx)
{
	Env *e = (Env *)(void *)t->bt_mp;
	size_t ni;

	e->log.str("dleaf ");
	e->log.snum(pg_id(e, h));
	e->log.ch(' ');
	e->log.num(idx);
	e->log.ch(' ');
	logdbt(e, key);
	if (e->dleaf_ret == RET_ERROR) {
		e->log.str(" -> ERR\n");
		return (RET_ERROR);
	}
	ni = safe_nextindex(h);
	if ((size_t)idx < ni) {
		memmove(h->linp + idx, h->linp + idx + 1,
		    (ni - idx - 1) * sizeof(indx_t));
		h->lower = (indx_t)(h->lower - sizeof(indx_t));
		if (e->dleaf_grow != 0 &&
		    (int)h->upper + e->dleaf_grow <= PSIZE)
			h->upper = (indx_t)((int)h->upper + e->dleaf_grow);
		e->log.str(" -> DEL\n");
	} else
		e->log.str(" -> NOP\n");
	return (RET_SUCCESS);
}

extern "C" int
__bt_split(BTREE *t, PAGE *h, const DBT *key, const DBT *data, int flags,
    size_t ilen, u_int32_t skip)
{
	Env *e = (Env *)(void *)t->bt_mp;

	e->log.str("split ");
	e->log.snum(pg_id(e, h));
	e->log.ch(' ');
	e->log.num((unsigned)flags);
	e->log.ch(' ');
	e->log.num(ilen);
	e->log.ch(' ');
	e->log.num(skip);
	e->log.ch(' ');
	logdbt(e, key);
	logdbt(e, data);
	e->log.str(" -> ");
	e->log.snum(e->split_ret);
	e->log.ch('\n');
	return (e->split_ret);
}

extern "C" int
__bt_setcur(BTREE *t, pgno_t pgno, u_int idx)
{
	Env *e = (Env *)(void *)t->bt_mp;

	e->log.str("setcur ");
	e->log.num(pgno);
	e->log.ch(' ');
	e->log.num(idx);
	e->log.ch('\n');
	e->setcur_calls++;
	if (t->bt_cursor.key.data != NULL) {
		t->bt_cursor.key.size = 0;
		t->bt_cursor.key.data = NULL;
	}
	F_CLR(&t->bt_cursor, CURS_ACQUIRE | CURS_AFTER | CURS_BEFORE);
	t->bt_cursor.pg.pgno = pgno;
	t->bt_cursor.pg.index = (indx_t)idx;
	F_SET(&t->bt_cursor, CURS_INIT);
	return (RET_SUCCESS);
}

/*
 * Compare against the record actually stored on the page, so the answer depends
 * both on the key bytes and on the index bt_fast picked.
 */
extern "C" int
__bt_cmp(BTREE *t, const DBT *k, EPG *ep)
{
	Env *e = (Env *)(void *)t->bt_mp;
	PAGE *h = ep->page;
	size_t ni = safe_nextindex(h);
	int r;

	if (h == NULL)
		r = 1;
	else if ((size_t)ep->index >= ni)
		r = -3 - (int)ep->index;
	else {
		indx_t off = h->linp[ep->index];

		if (off < (indx_t)BTDATAOFF || (size_t)off + 9 > (size_t)PSIZE)
			r = 5;
		else {
			BLEAF *bl = (BLEAF *)(void *)((char *)h + off);
			size_t avail = (size_t)PSIZE - (size_t)off - 9;
			size_t ks = bl->ksize;
			size_t n;

			if (ks > avail)
				ks = avail;
			n = k->size < ks ? k->size : ks;
			r = memcmp(k->data, bl->bytes, n);
			if (r == 0)
				r = k->size < ks ? -1 : (k->size > ks ? 1 : 0);
		}
	}
	e->log.str("cmp ");
	e->log.snum(pg_id(e, h));
	e->log.ch(' ');
	e->log.num(ep->index);
	e->log.ch(' ');
	logdbt(e, k);
	e->log.str(" -> ");
	e->log.snum(r);
	e->log.ch('\n');
	return (r);
}

/* ----------------------------- fixtures ---------------------------------- */

struct Cfg {
	u_int put_flags;
	unsigned tflags;
	unsigned cursflags;
	int pinned;			/* -1: none, else page id */
	unsigned ovflsize;
	int ovfl_mode;
	int order;			/* 0 NOT, 1 BACK, 2 FORWARD */
	int last_page;			/* -1: P_INVALID */
	unsigned last_index;
	int curs_page;			/* -1: P_INVALID */
	unsigned curs_index;
	int npages_valid;
	int nrec[NPG];
	int pad[NPG];
	int prevpg[NPG];		/* -1: P_INVALID */
	int nextpg[NPG];
	unsigned pgseed[NPG];
	int ovfl_ret;
	unsigned ovfl_pgno;
	int search_null;
	int search_page;
	unsigned search_index;
	int search_exact;
	int dleaf_ret;
	int dleaf_grow;
	int split_ret;
	unsigned ksize;
	unsigned dsize;
	unsigned kseed;
	unsigned dseed;
	int key_page;			/* copy a stored key into the key buf */
	int key_rec;			/* -1: use generated bytes */
	int key_adj;			/* 0 equal, 1 longer, 2 shorter */
};

static Cfg
base_cfg(void)
{
	Cfg c;

	memset(&c, 0, sizeof(c));
	c.put_flags = 0;
	c.tflags = 0;
	c.cursflags = 0;
	c.pinned = -1;
	c.ovflsize = 200;
	c.ovfl_mode = 0;
	c.order = 0;
	c.last_page = 0;
	c.last_index = 0;
	c.curs_page = 0;
	c.curs_index = 0;
	c.npages_valid = NPG;
	for (int i = 0; i < NPG; i++) {
		c.nrec[i] = 3;
		c.pad[i] = 0;
		c.prevpg[i] = -1;
		c.nextpg[i] = -1;
		c.pgseed[i] = 0x1234u + (unsigned)i;
	}
	c.ovfl_ret = RET_SUCCESS;
	c.ovfl_pgno = 40;
	c.search_null = 0;
	c.search_page = 0;
	c.search_index = 3;
	c.search_exact = 0;
	c.dleaf_ret = RET_SUCCESS;
	c.dleaf_grow = 0;
	c.split_ret = RET_SUCCESS;
	c.ksize = 5;
	c.dsize = 7;
	c.kseed = 0xabcdu;
	c.dseed = 0xbeefu;
	c.key_page = 0;
	c.key_rec = -1;
	c.key_adj = 0;
	return (c);
}

/*
 * bt_ovflsize is never allowed below 2 * NOVFLSIZE.  At or above that value the
 * `goto storekey' retry in __bt_put settles; below it the original spins
 * forever, so no input can be built there.
 */
static unsigned
ovflsize_of(const Cfg *c)
{
	long v;

	switch (c->ovfl_mode) {
	case 1:
		v = (long)c->ksize + (long)c->dsize;
		break;
	case 2:
		v = (long)c->ksize + (long)c->dsize - 1;
		break;
	case 3:
		v = (long)c->ksize + (long)c->dsize + 1;
		break;
	case 4:
		v = (long)c->ksize;
		break;
	case 5:
		v = (long)c->ksize - 1;
		break;
	case 6:
		v = (long)c->ksize + 1;
		break;
	default:
		v = (long)c->ovflsize;
		break;
	}
	if (v < 16)
		v = 16;
	if (v > 65535)
		v = 65535;
	return ((unsigned)v);
}

static void
build_page(Env *e, int i, const Cfg *c)
{
	PAGE *h;
	unsigned s;
	int up, lo;

	memset(e->pages[i].raw, GUARDBYTE, sizeof(e->pages[i].raw));
	h = pg_ptr(e, i);
	h->pgno = (pgno_t)i;
	h->prevpg = c->prevpg[i] < 0 ? (pgno_t)P_INVALID : (pgno_t)c->prevpg[i];
	h->nextpg = c->nextpg[i] < 0 ? (pgno_t)P_INVALID : (pgno_t)c->nextpg[i];
	h->flags = P_BLEAF;
	h->lower = (indx_t)BTDATAOFF;
	h->upper = (indx_t)PSIZE;
	s = c->pgseed[i] | 1u;
	for (int r = 0; r < c->nrec[i]; r++) {
		unsigned v = rnd32(&s);
		unsigned ks = 1u + (v & 7u);
		unsigned ds = (v >> 3) & 7u;
		size_t nb = NBLEAFDBT(ks, ds);
		char *p;

		if ((size_t)h->upper < (size_t)h->lower + nb + sizeof(indx_t))
			break;
		h->upper = (indx_t)((size_t)h->upper - nb);
		h->linp[r] = h->upper;
		p = (char *)h + h->upper;
		*(u_int32_t *)(void *)p = ks;
		p += sizeof(u_int32_t);
		*(u_int32_t *)(void *)p = ds;
		p += sizeof(u_int32_t);
		*(u_char *)p = 0;
		p += sizeof(u_char);
		fillbuf((unsigned char *)p, ks, s + 77u);
		p += ks;
		fillbuf((unsigned char *)p, ds, s + 99u);
		h->lower = (indx_t)(h->lower + sizeof(indx_t));
	}
	/*
	 * pad squeezes the free space so both sides of the
	 * `upper - lower < nbytes + sizeof(indx_t)' test are reachable; the
	 * clamp lets upper drop just below lower, which is where the cast to
	 * u_int32_t turns the difference into a huge value.
	 */
	up = (int)h->upper - c->pad[i];
	lo = (int)h->lower;
	if (up < lo - 8)
		up = (lo - 8) & ~3;
	if (up < 0)
		up = 0;
	h->upper = (indx_t)up;
}

static void
set_kd(Env *e, unsigned ks, unsigned ds, unsigned kseed, unsigned dseed)
{
	memset(e->kbuf, GUARDBYTE, sizeof(e->kbuf));
	memset(e->dbuf, GUARDBYTE, sizeof(e->dbuf));
	fillbuf(e->kbuf + GUARD, ks, kseed);
	fillbuf(e->dbuf + GUARD, ds, dseed);
	e->key.data = e->kbuf + GUARD;
	e->key.size = ks;
	e->data.data = e->dbuf + GUARD;
	e->data.size = ds;
}

/*
 * Copy a key that is actually on the page into the key buffer, so __bt_cmp can
 * return exactly zero -- otherwise `cmp == 0' and the `cmp ?' arm of bt_fast are
 * never taken.  key_adj lengthens or shortens it to force a strict sign.
 */
static void
plant_key(Env *e, const Cfg *c)
{
	PAGE *h = pg_ptr(e, c->key_page);
	size_t ni = safe_nextindex(h);
	indx_t off;
	BLEAF *bl;
	size_t ks;

	if ((size_t)c->key_rec >= ni)
		return;
	off = h->linp[c->key_rec];
	if (off < (indx_t)BTDATAOFF || (size_t)off + 9 > (size_t)PSIZE)
		return;
	bl = (BLEAF *)(void *)((char *)h + off);
	ks = bl->ksize;
	if (ks + 1 > KDMAX)
		return;
	memset(e->kbuf, GUARDBYTE, sizeof(e->kbuf));
	memcpy(e->kbuf + GUARD, bl->bytes, ks);
	if (c->key_adj == 1) {
		e->kbuf[GUARD + ks] = 0x41;
		ks++;
	} else if (c->key_adj == 2 && ks > 0)
		ks--;
	e->key.data = e->kbuf + GUARD;
	e->key.size = ks;
}

static void
setup(Env *e, const Cfg *c)
{
	e->npages_valid = c->npages_valid;
	e->ovfl_ret = c->ovfl_ret;
	e->ovfl_pgno = (pgno_t)c->ovfl_pgno;
	e->ovfl_calls = 0;
	e->search_null = c->search_null;
	e->search_page = c->search_page;
	e->search_index = c->search_index;
	e->search_exact = c->search_exact;
	e->dleaf_ret = c->dleaf_ret;
	e->dleaf_grow = c->dleaf_grow;
	e->split_ret = c->split_ret;
	e->setcur_calls = 0;
	e->log.reset();

	memset(&e->db, 0, sizeof(e->db));
	memset(&e->t, 0, sizeof(e->t));

	for (int i = 0; i < NPG; i++)
		build_page(e, i, c);

	set_kd(e, c->ksize, c->dsize, c->kseed, c->dseed);
	if (c->key_rec >= 0)
		plant_key(e, c);

	e->db.type = DB_BTREE;
	e->db.internal = &e->t;

	e->t.bt_mp = (MPOOL *)(void *)e;
	e->t.bt_dbp = &e->db;
	e->t.bt_cur.page = NULL;
	e->t.bt_cur.index = 0;
	e->t.bt_pinned = c->pinned < 0 ? NULL : pg_ptr(e, c->pinned);
	e->t.bt_cursor.pg.pgno =
	    c->curs_page < 0 ? (pgno_t)P_INVALID : (pgno_t)c->curs_page;
	e->t.bt_cursor.pg.index = (indx_t)c->curs_index;
	e->t.bt_cursor.flags = (u_int8_t)c->cursflags;
	e->t.bt_sp = e->t.bt_stack;
	e->t.bt_fd = -1;
	e->t.bt_free = (pgno_t)P_INVALID;
	e->t.bt_psize = PSIZE;
	e->t.bt_ovflsize = (indx_t)ovflsize_of(c);
	e->t.bt_lorder = 4321;
	e->t.bt_order = c->order == 0 ? NOT : (c->order == 1 ? BACK : FORWARD);
	e->t.bt_last.pgno =
	    c->last_page < 0 ? (pgno_t)P_INVALID : (pgno_t)c->last_page;
	e->t.bt_last.index = (indx_t)c->last_index;
	e->t.flags = c->tflags;
}

/* ---------------------------- comparison --------------------------------- */

struct Snap {
	BTREE t;
	int ovfl_calls;
	int setcur_calls;
};

static void
snap(Env *e, Snap *s)
{
	memset(s, 0, sizeof(*s));
	memcpy(&s->t, &e->t, sizeof(BTREE));
	s->t.bt_mp = (MPOOL *)(uintptr_t)1;
	s->t.bt_dbp = (DB *)(uintptr_t)2;
	s->t.bt_cur.page = (PAGE *)(uintptr_t)(pg_id(e, s->t.bt_cur.page) + 8);
	s->t.bt_pinned = (PAGE *)(uintptr_t)(pg_id(e, s->t.bt_pinned) + 8);
	s->t.bt_sp = (EPGNO *)(uintptr_t)(s->t.bt_sp != NULL ?
	    (uintptr_t)(s->t.bt_sp - e->t.bt_stack) + 1 : 0);
	s->t.bt_rkey.data = (void *)(uintptr_t)(s->t.bt_rkey.data != NULL);
	s->t.bt_rdata.data = (void *)(uintptr_t)(s->t.bt_rdata.data != NULL);
	s->t.bt_cursor.key.data =
	    (void *)(uintptr_t)(s->t.bt_cursor.key.data != NULL);
	s->t.bt_cmp = NULL;
	s->t.bt_pfx = NULL;
	s->t.bt_irec = NULL;
	s->t.bt_rfp = NULL;
	s->t.bt_cmap = NULL;
	s->t.bt_smap = NULL;
	s->t.bt_emap = NULL;
	s->ovfl_calls = e->ovfl_calls;
	s->setcur_calls = e->setcur_calls;
}

static int
cmp_state(Env *a, Env *b)
{
	Snap sa, sb;

	snap(a, &sa);
	snap(b, &sb);
	if (memcmp(&sa, &sb, sizeof(Snap)) != 0)
		return (1);
	if (memcmp(a->pages, b->pages, sizeof(a->pages)) != 0)
		return (2);
	if (memcmp(a->kbuf, b->kbuf, sizeof(a->kbuf)) != 0)
		return (3);
	if (memcmp(a->dbuf, b->dbuf, sizeof(a->dbuf)) != 0)
		return (4);
	if (a->key.size != b->key.size || a->data.size != b->data.size)
		return (5);
	if ((char *)a->key.data - (char *)a->kbuf !=
	    (char *)b->key.data - (char *)b->kbuf)
		return (6);
	if ((char *)a->data.data - (char *)a->dbuf !=
	    (char *)b->data.data - (char *)b->dbuf)
		return (7);
	if (a->log.n != b->log.n || a->log.total != b->log.total ||
	    memcmp(a->log.b, b->log.b, a->log.n) != 0)
		return (8);
	return (0);
}

struct Stats {
	const char *name;
	long cases;
	long fails;
};

static Stats st_put = { "__bt_put", 0, 0 };
static Stats st_fast = { "bt_fast", 0, 0 };
static int reported;

static void
fail(Stats *s, const char *tag, int code, long long ra, long long rb,
    int ea, int eb)
{
	s->fails++;
	if (reported < 25) {
		reported++;
		printf("FAIL %-8s %-10s code=%d ret=%lld/%lld aux=%d/%d\n",
		    s->name, tag, code, ra, rb, ea, eb);
		if (code == 8) {
			printf("  A trace:\n%s", EA.log.b);
			printf("  B trace:\n%s", EB.log.b);
		}
	}
}

/* ------------------------------ drivers ---------------------------------- */

static void
refill(Env *e, const Cfg *c, int k)
{
	unsigned h = c->kseed ^ (0x9e3779b9u * (unsigned)k);
	unsigned ks = (c->ksize + 7u * (unsigned)k + (h % 13u)) % (KDMAX - 8u);
	unsigned ds =
	    (c->dsize + 11u * (unsigned)k + ((h >> 8) % 17u)) % (KDMAX - 8u);

	set_kd(e, ks, ds, c->kseed + 1000u * (unsigned)k,
	    c->dseed + 2000u * (unsigned)k);
	e->log.reset();
}

static void
run_put(const Cfg *c, int steps, const char *tag)
{
	setup(&EA, c);
	setup(&EB, c);
	for (int k = 0; k < steps; k++) {
		int ra, rb, ea, eb, code;

		if (k != 0) {
			refill(&EA, c, k);
			refill(&EB, c, k);
		}
		errno = 0;
		ra = port::__bt_put(&EA.db, &EA.key, &EA.data, c->put_flags);
		ea = errno;
		errno = 0;
		rb = ref___bt_put(&EB.db, &EB.key, &EB.data, c->put_flags);
		eb = errno;
		st_put.cases++;
		if (ra != rb)
			code = 100;
		else if (ea != eb)
			code = 101;
		else
			code = cmp_state(&EA, &EB);
		if (code != 0)
			fail(&st_put, tag, code, ra, rb, ea, eb);
	}
}

static void
run_fast(const Cfg *c, int steps, const char *tag)
{
	setup(&EA, c);
	setup(&EB, c);
	for (int k = 0; k < steps; k++) {
		/*
		 * Same sentinel in both: bt_fast leaves *exactp untouched when
		 * it misses, so an implementation that writes it anyway has to
		 * show up as a difference.
		 */
		int exa = -12345, exb = -12345;
		int ea, eb, code;
		EPG *ra, *rb;
		long oa, ob;

		if (k != 0) {
			refill(&EA, c, k);
			refill(&EB, c, k);
		}
		errno = 0;
		ra = port::bt_fast(&EA.t, &EA.key, &EA.data, &exa);
		ea = errno;
		errno = 0;
		rb = ref_bt_fast(&EB.t, &EB.key, &EB.data, &exb);
		eb = errno;
		/* offsets from each environment base, never raw addresses */
		oa = ra != NULL ? (long)((char *)ra - (char *)&EA) : -1;
		ob = rb != NULL ? (long)((char *)rb - (char *)&EB) : -1;
		st_fast.cases++;
		if (oa != ob)
			code = 200;
		else if (exa != exb)
			code = 201;
		else if (ea != eb)
			code = 202;
		else
			code = cmp_state(&EA, &EB);
		if (code != 0)
			fail(&st_fast, tag, code, oa, ob, exa, exb);
	}
}

static void
run_both(const Cfg *c, int steps, const char *tag)
{
	run_put(c, steps, tag);
	run_fast(c, steps, tag);
}

/*
 * Pick a key/data size pair that lands the free-space test exactly on its
 * boundary for the given page.  mode 0: nbytes + sizeof(indx_t) equals the free
 * space, the largest pair that must NOT split; mode 1: the smallest pair that
 * must split; mode 2: the largest pair that still fits.
 */
static void
tune_fit(Cfg *c, int page, int mode)
{
	PAGE *h;
	long freesp, best = -1;

	setup(&ESCRATCH, c);
	h = pg_ptr(&ESCRATCH, page);
	freesp = (long)h->upper - (long)h->lower;
	for (long s = 0; s <= 400; s++) {
		long need = (long)NBLEAFDBT((size_t)s, (size_t)0) +
		    (long)sizeof(indx_t);

		if (mode == 0 && need == freesp) {
			best = s;
			break;
		}
		if (mode == 1 && need > freesp) {
			best = s;
			break;
		}
		if (mode == 2 && need <= freesp)
			best = s;
	}
	if (best < 0)
		return;
	c->ksize = (unsigned)(best / 2);
	c->dsize = (unsigned)(best - best / 2);
	c->ovfl_mode = 0;
	if (c->ovflsize < c->ksize + c->dsize)
		c->ovflsize = c->ksize + c->dsize + 8u;
}

/* ---------------------------- hand cases --------------------------------- */

static void
hand_cases(void)
{
	/* every put flag against every cursor flag combination */
	{
		static const u_int fl[] = { 0u, 1u, 2u, 3u, 8u, 9u, 10u, 11u,
		    99u, 0xffffffffu };

		for (unsigned fi = 0; fi < sizeof(fl) / sizeof(fl[0]); fi++)
			for (unsigned cf = 0; cf < 16u; cf++) {
				Cfg c = base_cfg();

				c.put_flags = fl[fi];
				c.cursflags = cf;
				run_put(&c, 1, "flags");
			}
	}

	/* read-only tree, and a page pinned across calls */
	for (int i = 0; i < 2; i++)
		for (int p = -1; p < NPG; p++) {
			Cfg c = base_cfg();

			c.tflags = i != 0 ? (unsigned)B_RDONLY : 0u;
			c.pinned = p;
			run_put(&c, 2, "pinned");
		}

	/*
	 * Overflow thresholds: both sides of key+data > ovflsize and of
	 * key > ovflsize, including the retry that stores both.
	 */
	{
		static const unsigned ov[] = { 16u, 17u, 18u, 20u, 24u, 25u };

		for (unsigned oi = 0; oi < sizeof(ov) / sizeof(ov[0]); oi++)
			for (unsigned ks = 0; ks <= 26u; ks++)
				for (unsigned ds = 0; ds <= 26u; ds++) {
					Cfg c = base_cfg();

					c.ovflsize = ov[oi];
					c.ksize = ks;
					c.dsize = ds;
					run_both(&c, 1, "ovfl");
				}
	}

	/* __ovfl_put failing on the key store and on the data store */
	for (int ks = 0; ks < 2; ks++) {
		Cfg c = base_cfg();

		c.ovfl_ret = RET_ERROR;
		c.ovflsize = 16u;
		c.ksize = ks != 0 ? 40u : 4u;
		c.dsize = 40u;
		run_put(&c, 1, "ovflerr");
	}

	/* empty, single byte and NUL-heavy keys and data */
	for (unsigned ks = 0; ks < 4u; ks++)
		for (unsigned ds = 0; ds < 4u; ds++)
			for (unsigned sd = 0; sd < 3u; sd++) {
				Cfg c = base_cfg();

				c.ksize = ks;
				c.dsize = ds;
				c.kseed = 1u + sd;
				c.dseed = 7u + sd;
				run_both(&c, 3, "tiny");
			}

	/*
	 * exact / B_NODUPS / flag matrix -- both halves of
	 * `!exact || !F_ISSET(t, B_NODUPS)'.
	 */
	for (int ex = 0; ex < 2; ex++)
		for (int nd = 0; nd < 2; nd++)
			for (int fi = 0; fi < 3; fi++)
				for (int de = 0; de < 2; de++) {
					Cfg c = base_cfg();

					c.search_exact = ex;
					c.tflags = nd != 0 ?
					    (unsigned)B_NODUPS : 0u;
					c.put_flags = fi == 0 ? 0u :
					    (fi == 1 ? (u_int)R_NOOVERWRITE :
					    (u_int)R_SETCURSOR);
					c.dleaf_ret = de != 0 ? RET_ERROR :
					    RET_SUCCESS;
					run_put(&c, 1, "exact");
				}

	/* __bt_search failing */
	for (int o = 0; o < 3; o++) {
		Cfg c = base_cfg();

		c.search_null = 1;
		c.order = o;
		run_put(&c, 1, "searchnul");
	}

	/* R_CURSOR: reachable page, unreachable page, every index */
	for (int p = -1; p < NPG; p++)
		for (unsigned ci = 0; ci < 6u; ci++)
			for (int nv = 0; nv <= NPG; nv += 2) {
				Cfg c = base_cfg();

				c.put_flags = R_CURSOR;
				c.cursflags = CURS_INIT;
				c.curs_page = p;
				c.curs_index = ci;
				c.npages_valid = nv;
				run_put(&c, 2, "rcursor");
			}

	/* free-space boundary, tuned exactly, on both sides */
	for (int nr = 0; nr <= 8; nr++)
		for (int pd = 0; pd <= 24; pd += 4)
			for (int mode = 0; mode < 3; mode++)
				for (int sr = 0; sr < 3; sr++) {
					Cfg c = base_cfg();

					for (int i = 0; i < NPG; i++) {
						c.nrec[i] = nr;
						c.pad[i] = pd;
					}
					c.search_index = (unsigned)nr;
					c.split_ret = sr == 0 ?
					    RET_SUCCESS : (sr == 1 ?
					    RET_ERROR : RET_SPECIAL);
					tune_fit(&c, 0, mode);
					run_both(&c, 1, "fit");
					c.put_flags = R_SETCURSOR;
					run_put(&c, 1, "fitcur");
				}

	/* upper driven below lower: the cast to u_int32_t must stay huge */
	for (int pd = 0; pd <= 64; pd += 4)
		for (int gr = 0; gr <= 8; gr += 4) {
			Cfg c = base_cfg();

			for (int i = 0; i < NPG; i++) {
				c.nrec[i] = 5;
				c.pad[i] = 300 + pd;
			}
			c.dleaf_grow = gr;
			c.search_exact = 1;
			c.tflags = B_NODUPS;
			run_both(&c, 2, "underflow");
		}

	/* insert index below, at and past the end of the offset array */
	for (int nr = 0; nr <= 6; nr++)
		for (unsigned si = 0; si <= (unsigned)nr + 2u; si++)
			for (int de = 0; de < 2; de++) {
				Cfg c = base_cfg();

				for (int i = 0; i < NPG; i++)
					c.nrec[i] = nr;
				c.search_index = si;
				c.search_exact = de;
				c.tflags = de != 0 ? (unsigned)B_NODUPS : 0u;
				run_put(&c, 3, "idx");
			}

	/* cursor adjustment: each conjunct false in turn, and the >= boundary */
	for (int nr = 1; nr <= 5; nr++)
		for (unsigned si = 0; si <= (unsigned)nr; si++)
			for (int dci = -1; dci <= 1; dci++)
				for (unsigned cf = 0; cf < 16u; cf++)
					for (int cp = 0; cp < 2; cp++) {
						Cfg c = base_cfg();
						long ci;

						for (int i = 0; i < NPG; i++)
							c.nrec[i] = nr;
						c.search_index = si;
						c.search_page = 0;
						ci = (long)si + dci;
						if (ci < 0)
							ci = 0;
						c.curs_index = (unsigned)ci;
						c.curs_page = cp;
						c.cursflags = cf;
						run_put(&c, 1, "cursadj");
					}

	/*
	 * Sort-order detection: nextpg and prevpg P_INVALID or not, with idx at
	 * the first, the last and an interior slot.
	 */
	for (int nr = 1; nr <= 5; nr++)
		for (int np = 0; np < 2; np++)
			for (int pp = 0; pp < 2; pp++)
				for (unsigned si = 0; si <= (unsigned)nr; si++)
					for (int o = 0; o < 3; o++) {
						Cfg c = base_cfg();

						for (int i = 0; i < NPG; i++) {
							c.nrec[i] = nr;
							c.nextpg[i] =
							    np != 0 ? -1 : 2;
							c.prevpg[i] =
							    pp != 0 ? -1 : 3;
						}
						c.search_index = si;
						c.order = o;
						c.last_page = 0;
						c.last_index = si;
						run_put(&c, 2, "order");
					}

	/* bt_fast: every miss point */
	for (int o = 0; o < 3; o++)
		for (int nr = 0; nr <= 6; nr++)
			for (int np = 0; np < 2; np++)
				for (int pp = 0; pp < 2; pp++)
					for (int lp = -1; lp < NPG; lp++) {
						Cfg c = base_cfg();

						for (int i = 0; i < NPG; i++) {
							c.nrec[i] = nr;
							c.nextpg[i] =
							    np != 0 ? -1 : 2;
							c.prevpg[i] =
							    pp != 0 ? -1 : 3;
						}
						c.order = o;
						c.last_page = lp;
						c.last_index = nr > 0 ?
						    (unsigned)(nr - 1) : 0u;
						run_both(&c, 1, "fastmiss");
					}

	/* bt_fast: cmp < 0, cmp == 0 and cmp > 0, at and off the edge index */
	for (int o = 1; o < 3; o++)
		for (int nr = 1; nr <= 6; nr++)
			for (int adj = 0; adj < 3; adj++)
				for (int li = 0; li < 2; li++) {
					Cfg c = base_cfg();

					for (int i = 0; i < NPG; i++) {
						c.nrec[i] = nr;
						c.nextpg[i] = -1;
						c.prevpg[i] = -1;
					}
					c.order = o;
					c.last_page = 0;
					c.last_index = o == 2 ?
					    (li != 0 ? (unsigned)(nr - 1) : 0u) :
					    (li != 0 ? 0u : 1u);
					c.key_page = 0;
					c.key_rec = o == 2 ? nr - 1 : 0;
					c.key_adj = adj;
					run_both(&c, 1, "fastcmp");
					c.put_flags = R_SETCURSOR;
					run_put(&c, 1, "fastcmp2");
				}

	/* every mpool reachability, so mpool_get fails at each entry point */
	for (int nv = 0; nv <= NPG; nv++)
		for (int lp = -1; lp < NPG; lp++)
			for (int o = 0; o < 3; o++) {
				Cfg c = base_cfg();

				c.npages_valid = nv;
				c.last_page = lp;
				c.order = o;
				run_both(&c, 2, "mpool");
			}

	/* __bt_dleaf shrinking the page to empty, then inserting again */
	for (int nr = 1; nr <= 4; nr++)
		for (unsigned si = 0; si < (unsigned)nr; si++)
			for (int gr = 0; gr <= 8; gr += 4) {
				Cfg c = base_cfg();

				for (int i = 0; i < NPG; i++)
					c.nrec[i] = nr;
				c.search_index = si;
				c.search_exact = 1;
				c.tflags = B_NODUPS;
				c.dleaf_grow = gr;
				run_put(&c, 4, "dleaf");
			}
}

/* --------------------------- random sweep -------------------------------- */

static void
rnd_cfg(Cfg *c, unsigned *s)
{
	static const u_int fl[] = { 0u, 1u, 8u, 10u, 2u, 3u, 11u, 99u };
	unsigned v;

	*c = base_cfg();
	c->put_flags = fl[rnd32(s) % (sizeof(fl) / sizeof(fl[0]))];
	v = rnd32(s);
	c->tflags = ((v & 1u) != 0u ? (unsigned)B_NODUPS : 0u) |
	    ((v & 2u) != 0u ? (unsigned)B_MODIFIED : 0u) |
	    (((v >> 8) % 32u) == 0u ? (unsigned)B_RDONLY : 0u);
	c->cursflags = rnd32(s) % 16u;
	v = rnd32(s) % 16u;
	c->pinned = v < 12u ? -1 : (int)(v - 12u);
	c->npages_valid = (rnd32(s) % 8u) == 0u ?
	    (int)(rnd32(s) % (NPG + 1u)) : NPG;
	c->order = (int)(rnd32(s) % 3u);
	v = rnd32(s) % 8u;
	c->last_page = v < 2u ? -1 : (int)(v % NPG);
	c->last_index = rnd32(s) % 9u;
	v = rnd32(s) % 8u;
	c->curs_page = v < 1u ? -1 : (int)(v % NPG);
	c->curs_index = rnd32(s) % 9u;
	for (int i = 0; i < NPG; i++) {
		c->nrec[i] = (int)(rnd32(s) % 11u);
		c->pad[i] = (int)(4u * (rnd32(s) % 15u));
		v = rnd32(s) % 6u;
		c->prevpg[i] = v < 3u ? -1 : (int)(v % NPG);
		v = rnd32(s) % 6u;
		c->nextpg[i] = v < 3u ? -1 : (int)(v % NPG);
		c->pgseed[i] = rnd32(s);
	}
	c->ovfl_ret = (rnd32(s) % 10u) == 0u ? RET_ERROR : RET_SUCCESS;
	c->ovfl_pgno = rnd32(s) % 4096u;
	c->search_null = (int)((rnd32(s) % 12u) == 0u);
	c->search_page = (int)(rnd32(s) % NPG);
	c->search_index = rnd32(s) % ((unsigned)c->nrec[c->search_page] + 3u);
	c->search_exact = (int)(rnd32(s) % 2u);
	c->dleaf_ret = (rnd32(s) % 12u) == 0u ? RET_ERROR : RET_SUCCESS;
	c->dleaf_grow = (int)(4u * (rnd32(s) % 5u));
	v = rnd32(s) % 5u;
	c->split_ret = v < 3u ? RET_SUCCESS :
	    (v == 3u ? RET_ERROR : RET_SPECIAL);
	c->ksize = (rnd32(s) % 10u) < 8u ? rnd32(s) % 46u :
	    rnd32(s) % (KDMAX - 8u);
	c->dsize = (rnd32(s) % 10u) < 8u ? rnd32(s) % 46u :
	    rnd32(s) % (KDMAX - 8u);
	c->kseed = rnd32(s);
	c->dseed = rnd32(s);
	c->ovflsize = 16u + rnd32(s) % 400u;
	v = rnd32(s) % 12u;
	c->ovfl_mode = v <= 6u ? (int)v : 0;
	if ((rnd32(s) % 3u) == 0u) {
		c->key_page = (int)(rnd32(s) % NPG);
		c->key_rec = (int)(rnd32(s) % 8u);
		c->key_adj = (int)(rnd32(s) % 3u);
	}
	if (c->put_flags == (u_int)R_CURSOR) {
		int p = c->curs_page < 0 ? 0 : c->curs_page;

		c->curs_index = rnd32(s) % ((unsigned)c->nrec[p] + 3u);
	}
}

#define	SWEEP	200000

static void
sweep(void)
{
	unsigned s = 0x5eed1155u;
	Cfg c;

	for (long i = 0; i < SWEEP; i++) {
		int steps;

		rnd_cfg(&c, &s);
		steps = 1 + (int)(rnd32(&s) % 3u);
		run_put(&c, steps, "sweep");
	}

	s = 0x0155f00du;
	for (long i = 0; i < SWEEP; i++) {
		rnd_cfg(&c, &s);
		run_fast(&c, 1 + (int)(rnd32(&s) % 2u), "sweep");
	}
}

int
main(void)
{
	long tot;

	hand_cases();
	sweep();

	printf("\n%-12s %12s %12s\n", "function", "cases", "failures");
	printf("%-12s %12ld %12ld\n", st_put.name, st_put.cases, st_put.fails);
	printf("%-12s %12ld %12ld\n", st_fast.name, st_fast.cases,
	    st_fast.fails);
	tot = st_put.fails + st_fast.fails;
	printf("\n%s\n", tot == 0 ? "ALL CASES MATCH" : "DIVERGENCE DETECTED");
	return (tot == 0 ? 0 : 1);
}
