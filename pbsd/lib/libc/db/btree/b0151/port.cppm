module;

#include <sys/types.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

export module pbsd.lib.libc.db.btree.b0151;

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define RET_ERROR	-1
#define RET_SUCCESS	 0

#define	MAX_PAGE_OFFSET	65535

#define DEBUG
#define STATISTICS

typedef uint32_t	pgno_t;
typedef uint16_t	indx_t;
typedef uint32_t	recno_t;

typedef struct {
	void	*data;
	size_t	 size;
} DBT;

typedef struct __db {
	int (*close)(struct __db *);
	void *internal;
} DB;

#define	F_SET(p, f)	(p)->flags |= (f)
#define	F_CLR(p, f)	(p)->flags &= ~(f)
#define	F_ISSET(p, f)	((p)->flags & (f))

#define	MPOOL_DIRTY	0x01
#define	MPOOL_PAGE_NEXT	0x02
#define	MPOOL_IGNOREPIN	0x04

struct MPOOL {
	int opaque;
};

typedef struct _page {
	pgno_t	pgno;
	pgno_t	prevpg;
	pgno_t	nextpg;
#define	P_BINTERNAL	0x01
#define	P_BLEAF		0x02
#define	P_OVERFLOW	0x04
#define	P_RINTERNAL	0x08
#define	P_RLEAF		0x10
#define P_TYPE		0x1f
#define	P_PRESERVE	0x20
	u_int32_t flags;
	indx_t	lower;
	indx_t	upper;
	indx_t	linp[1];
} PAGE;

#define	P_INVALID	 0
#define	P_META		 0
#define	P_ROOT		 1

#define	BTDATAOFF							\
	(sizeof(pgno_t) + sizeof(pgno_t) + sizeof(pgno_t) +		\
	    sizeof(u_int32_t) + sizeof(indx_t) + sizeof(indx_t))
#define	NEXTINDEX(p)	(((p)->lower - BTDATAOFF) / sizeof(indx_t))
#define	LALIGN(n)	(((n) + sizeof(pgno_t) - 1) & ~(sizeof(pgno_t) - 1))

typedef struct _binternal {
	u_int32_t ksize;
	pgno_t	pgno;
#define	P_BIGDATA	0x01
#define	P_BIGKEY	0x02
	u_char	flags;
	char	bytes[1];
} BINTERNAL;

#define	GETBINTERNAL(pg, indx)						\
	((BINTERNAL *)((char *)(pg) + (pg)->linp[indx]))

typedef struct _rinternal {
	recno_t	nrecs;
	pgno_t	pgno;
} RINTERNAL;

#define	GETRINTERNAL(pg, indx)						\
	((RINTERNAL *)((char *)(pg) + (pg)->linp[indx]))

typedef struct _bleaf {
	u_int32_t	ksize;
	u_int32_t	dsize;
	u_char	flags;
	char	bytes[1];
} BLEAF;

#define	GETBLEAF(pg, indx)						\
	((BLEAF *)((char *)(pg) + (pg)->linp[indx]))

typedef struct _rleaf {
	u_int32_t	dsize;
	u_char	flags;
	char	bytes[1];
} RLEAF;

#define	GETRLEAF(pg, indx)						\
	((RLEAF *)((char *)(pg) + (pg)->linp[indx]))

typedef struct _epgno {
	pgno_t	pgno;
	indx_t	index;
} EPGNO;

typedef struct _epg {
	PAGE	*page;
	indx_t	 index;
} EPG;

typedef struct _cursor {
	EPGNO	 pg;
	DBT	 key;
	recno_t	 rcursor;
#define	CURS_ACQUIRE	0x01
#define	CURS_AFTER	0x02
#define	CURS_BEFORE	0x04
#define	CURS_INIT	0x08
	u_int8_t flags;
} CURSOR;

typedef struct _btmeta {
	u_int32_t	magic;
	u_int32_t	version;
	u_int32_t	psize;
	u_int32_t	free;
	u_int32_t	nrecs;
#define	SAVEMETA	(B_NODUPS | R_RECNO)
	u_int32_t	flags;
} BTMETA;

typedef struct _btree {
	MPOOL	 *bt_mp;
	DB	 *bt_dbp;
	EPG	  bt_cur;
	PAGE	 *bt_pinned;
	CURSOR	  bt_cursor;
#define	BT_PUSH(t, p, i) {						\
	t->bt_sp->pgno = p;						\
	t->bt_sp->index = i;						\
	++t->bt_sp;							\
}
#define	BT_POP(t)	(t->bt_sp == t->bt_stack ? NULL : --t->bt_sp)
#define	BT_CLR(t)	(t->bt_sp = t->bt_stack)
	EPGNO	  bt_stack[50];
	EPGNO	 *bt_sp;
	DBT	  bt_rkey;
	DBT	  bt_rdata;
	int	  bt_fd;
	pgno_t	  bt_free;
	u_int32_t bt_psize;
	indx_t	  bt_ovflsize;
	int	  bt_lorder;
	enum { NOT, BACK, FORWARD } bt_order;
	EPGNO	  bt_last;
	int	(*bt_cmp)(const DBT *, const DBT *);
	size_t	(*bt_pfx)(const DBT *, const DBT *);
	int	(*bt_irec)(struct _btree *, recno_t);
	FILE	 *bt_rfp;
	int	  bt_rfd;
	caddr_t	  bt_cmap;
	caddr_t	  bt_smap;
	caddr_t   bt_emap;
	size_t	  bt_msize;
	recno_t	  bt_nrecs;
	size_t	  bt_reclen;
	u_char	  bt_bval;
#define	B_INMEM		0x00001
#define	B_METADIRTY	0x00002
#define	B_MODIFIED	0x00004
#define	B_NEEDSWAP	0x00008
#define	B_RDONLY	0x00010
#define	B_NODUPS	0x00020
#define	R_RECNO		0x00080
#define	R_CLOSEFP	0x00040
#define	R_EOF		0x00100
#define	R_FIXLEN	0x00200
#define	R_MEMMAPPED	0x00400
#define	R_INMEM		0x00800
#define	R_MODIFIED	0x01000
#define	R_RDONLY	0x02000
#define	B_DB_LOCK	0x04000
#define	B_DB_SHMEM	0x08000
#define	B_DB_TXN	0x10000
	u_int32_t flags;
} BTREE;

extern "C" {
void *mpool_get(MPOOL *, pgno_t, unsigned int);
int mpool_put(MPOOL *, void *, unsigned int);
PAGE *__bt_new(BTREE *, pgno_t *);
void __bt_free(BTREE *, PAGE *);
void *reallocf(void *, size_t);
void test_mock_reset(void);
void test_mock_register(pgno_t, void *);
void test_mock_seq_begin(pgno_t);
extern unsigned long bt_cache_hit, bt_cache_miss, bt_pfxsaved, bt_rootsplit;
extern unsigned long bt_sortsplit, bt_split;
}

export namespace pbsd::lib_libc_db_btree::b0151 {

using BTREE = ::BTREE;
using PAGE = ::PAGE;
using DB = ::DB;
using DBT = ::DBT;
using pgno_t = ::pgno_t;
using MPOOL = ::MPOOL;
using BINTERNAL = ::BINTERNAL;
using BLEAF = ::BLEAF;
using RINTERNAL = ::RINTERNAL;
using RLEAF = ::RLEAF;
using CURSOR = ::CURSOR;
using EPGNO = ::EPGNO;
using EPG = ::EPG;
using BTMETA = ::BTMETA;

static int __bt_snext(BTREE *, PAGE *, const DBT *, int *);
static int __bt_sprev(BTREE *, PAGE *, const DBT *, int *);
int __bt_cmp(BTREE *, const DBT *, EPG *);
void __bt_dpage(PAGE *);

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Olson.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * __bt_search --
 *	Search a btree for a key.
 */
EPG *
__bt_search(BTREE *t, const DBT *key, int *exactp)
{
	PAGE *h;
	indx_t base, idx, lim;
	pgno_t pg;
	int cmp;

	BT_CLR(t);
	for (pg = P_ROOT;;) {
		if ((h = (PAGE *)mpool_get(t->bt_mp, pg, 0)) == NULL)
			return (NULL);

		t->bt_cur.page = h;
		for (base = 0, lim = NEXTINDEX(h); lim; lim >>= 1) {
			t->bt_cur.index = idx = base + (lim >> 1);
			if ((cmp = __bt_cmp(t, key, &t->bt_cur)) == 0) {
				if (h->flags & P_BLEAF) {
					*exactp = 1;
					return (&t->bt_cur);
				}
				goto next;
			}
			if (cmp > 0) {
				base = idx + 1;
				--lim;
			}
		}

		if (h->flags & P_BLEAF) {
			if (!F_ISSET(t, B_NODUPS)) {
				if (base == 0 &&
				    h->prevpg != P_INVALID &&
				    __bt_sprev(t, h, key, exactp))
					return (&t->bt_cur);
				if (base == NEXTINDEX(h) &&
				    h->nextpg != P_INVALID &&
				    __bt_snext(t, h, key, exactp))
					return (&t->bt_cur);
			}
			*exactp = 0;
			t->bt_cur.index = base;
			return (&t->bt_cur);
		}

		idx = base ? base - 1 : base;

next:		BT_PUSH(t, h->pgno, idx);
		pg = GETBINTERNAL(h, idx)->pgno;
		mpool_put(t->bt_mp, h, 0);
	}
}

static int
__bt_snext(BTREE *t, PAGE *h, const DBT *key, int *exactp)
{
	EPG e;

	if ((e.page = (PAGE *)mpool_get(t->bt_mp, h->nextpg, 0)) == NULL)
		return (0);
	e.index = 0;
	if (__bt_cmp(t, key, &e) == 0) {
		mpool_put(t->bt_mp, h, 0);
		t->bt_cur = e;
		*exactp = 1;
		return (1);
	}
	mpool_put(t->bt_mp, e.page, 0);
	return (0);
}

static int
__bt_sprev(BTREE *t, PAGE *h, const DBT *key, int *exactp)
{
	EPG e;

	if ((e.page = (PAGE *)mpool_get(t->bt_mp, h->prevpg, 0)) == NULL)
		return (0);
	e.index = NEXTINDEX(e.page) - 1;
	if (__bt_cmp(t, key, &e) == 0) {
		mpool_put(t->bt_mp, h, 0);
		t->bt_cur = e;
		*exactp = 1;
		return (1);
	}
	mpool_put(t->bt_mp, e.page, 0);
	return (0);
}

int
__ovfl_get(BTREE *t, void *p, size_t *ssz, void **buf, size_t *bufsz)
{
	PAGE *h;
	pgno_t pg;
	size_t nb, plen;
	u_int32_t sz;

	memmove(&pg, p, sizeof(pgno_t));
	memmove(&sz, (char *)p + sizeof(pgno_t), sizeof(u_int32_t));
	*ssz = sz;

#ifdef DEBUG
	if (pg == P_INVALID || sz == 0)
		abort();
#endif
	if (*bufsz < sz) {
		*buf = reallocf(*buf, sz);
		if (*buf == NULL)
			return (RET_ERROR);
		*bufsz = sz;
	}

	plen = t->bt_psize - BTDATAOFF;
	for (p = *buf;; p = (char *)p + nb, pg = h->nextpg) {
		if ((h = (PAGE *)mpool_get(t->bt_mp, pg, 0)) == NULL)
			return (RET_ERROR);

		nb = MIN(sz, plen);
		memmove(p, (char *)h + BTDATAOFF, nb);
		mpool_put(t->bt_mp, h, 0);

		if ((sz -= nb) == 0)
			break;
	}
	return (RET_SUCCESS);
}

int
__ovfl_put(BTREE *t, const DBT *dbt, pgno_t *pg)
{
	PAGE *h, *last;
	void *p;
	pgno_t npg;
	size_t nb, plen;
	u_int32_t sz;

	plen = t->bt_psize - BTDATAOFF;
	for (last = NULL, p = dbt->data, sz = dbt->size;;
	    p = (char *)p + plen, last = h) {
		if ((h = __bt_new(t, &npg)) == NULL)
			return (RET_ERROR);

		h->pgno = npg;
		h->nextpg = h->prevpg = P_INVALID;
		h->flags = P_OVERFLOW;
		h->lower = h->upper = 0;

		nb = MIN(sz, plen);
		memmove((char *)h + BTDATAOFF, p, nb);

		if (last) {
			last->nextpg = h->pgno;
			mpool_put(t->bt_mp, last, MPOOL_DIRTY);
		} else
			*pg = h->pgno;

		if ((sz -= nb) == 0) {
			mpool_put(t->bt_mp, h, MPOOL_DIRTY);
			break;
		}
	}
	return (RET_SUCCESS);
}

int
__ovfl_delete(BTREE *t, void *p)
{
	PAGE *h;
	pgno_t pg;
	size_t plen;
	u_int32_t sz;

	memmove(&pg, p, sizeof(pgno_t));
	memmove(&sz, (char *)p + sizeof(pgno_t), sizeof(u_int32_t));

#ifdef DEBUG
	if (pg == P_INVALID || sz == 0)
		abort();
#endif
	if ((h = (PAGE *)mpool_get(t->bt_mp, pg, 0)) == NULL)
		return (RET_ERROR);

	if (h->flags & P_PRESERVE) {
		mpool_put(t->bt_mp, h, 0);
		return (RET_SUCCESS);
	}

	for (plen = t->bt_psize - BTDATAOFF;; sz -= plen) {
		pg = h->nextpg;
		__bt_free(t, h);
		if (sz <= plen)
			break;
		if ((h = (PAGE *)mpool_get(t->bt_mp, pg, 0)) == NULL)
			return (RET_ERROR);
	}
	return (RET_SUCCESS);
}

int
__bt_ret(BTREE *t, EPG *e, DBT *key, DBT *rkey, DBT *data, DBT *rdata, int copy)
{
	BLEAF *bl;
	void *p;

	bl = GETBLEAF(e->page, e->index);

	if (key == NULL)
		goto dataonly;

	if (bl->flags & P_BIGKEY) {
		if (__ovfl_get(t, bl->bytes,
		    &key->size, &rkey->data, &rkey->size))
			return (RET_ERROR);
		key->data = rkey->data;
	} else if (copy || F_ISSET(t, B_DB_LOCK)) {
		if (bl->ksize > rkey->size) {
			p = realloc(rkey->data, bl->ksize);
			if (p == NULL)
				return (RET_ERROR);
			rkey->data = p;
			rkey->size = bl->ksize;
		}
		memmove(rkey->data, bl->bytes, bl->ksize);
		key->size = bl->ksize;
		key->data = rkey->data;
	} else {
		key->size = bl->ksize;
		key->data = bl->bytes;
	}

dataonly:
	if (data == NULL)
		return (RET_SUCCESS);

	if (bl->flags & P_BIGDATA) {
		if (__ovfl_get(t, bl->bytes + bl->ksize,
		    &data->size, &rdata->data, &rdata->size))
			return (RET_ERROR);
		data->data = rdata->data;
	} else if (copy || F_ISSET(t, B_DB_LOCK)) {
		if (bl->dsize + 1 > rdata->size) {
			p = realloc(rdata->data, bl->dsize + 1);
			if (p == NULL)
				return (RET_ERROR);
			rdata->data = p;
			rdata->size = bl->dsize + 1;
		}
		memmove(rdata->data, bl->bytes + bl->ksize, bl->dsize);
		data->size = bl->dsize;
		data->data = rdata->data;
	} else {
		data->size = bl->dsize;
		data->data = bl->bytes + bl->ksize;
	}

	return (RET_SUCCESS);
}

int
__bt_cmp(BTREE *t, const DBT *k1, EPG *e)
{
	BINTERNAL *bi;
	BLEAF *bl;
	DBT k2;
	PAGE *h;
	void *bigkey;

	h = e->page;
	if (e->index == 0 && h->prevpg == P_INVALID && !(h->flags & P_BLEAF))
		return (1);

	bigkey = NULL;
	if (h->flags & P_BLEAF) {
		bl = GETBLEAF(h, e->index);
		if (bl->flags & P_BIGKEY)
			bigkey = bl->bytes;
		else {
			k2.data = bl->bytes;
			k2.size = bl->ksize;
		}
	} else {
		bi = GETBINTERNAL(h, e->index);
		if (bi->flags & P_BIGKEY)
			bigkey = bi->bytes;
		else {
			k2.data = bi->bytes;
			k2.size = bi->ksize;
		}
	}

	if (bigkey) {
		if (__ovfl_get(t, bigkey,
		    &k2.size, &t->bt_rdata.data, &t->bt_rdata.size))
			return (RET_ERROR);
		k2.data = t->bt_rdata.data;
	}
	return ((*t->bt_cmp)(k1, &k2));
}

int
__bt_defcmp(const DBT *a, const DBT *b)
{
	size_t len;
	u_char *p1, *p2;

	len = MIN(a->size, b->size);
	for (p1 = (u_char *)a->data, p2 = (u_char *)b->data; len--; ++p1, ++p2)
		if (*p1 != *p2)
			return ((int)*p1 - (int)*p2);
	return ((int)a->size - (int)b->size);
}

size_t
__bt_defpfx(const DBT *a, const DBT *b)
{
	u_char *p1, *p2;
	size_t cnt, len;

	cnt = 1;
	len = MIN(a->size, b->size);
	for (p1 = (u_char *)a->data, p2 = (u_char *)b->data; len--; ++p1, ++p2, ++cnt)
		if (*p1 != *p2)
			return (cnt);

	return (a->size < b->size ? a->size + 1 : a->size);
}

#ifdef DEBUG
void
__bt_dump(DB *dbp)
{
	BTREE *t;
	PAGE *h;
	pgno_t i;
	char *sep;

	t = (BTREE *)dbp->internal;
	(void)fprintf(stderr, "%s: pgsz %u",
	    F_ISSET(t, B_INMEM) ? "memory" : "disk", t->bt_psize);
	if (F_ISSET(t, R_RECNO))
		(void)fprintf(stderr, " keys %u", t->bt_nrecs);
#undef X
#define	X(flag, name) \
	if (F_ISSET(t, flag)) { \
		(void)fprintf(stderr, "%s%s", sep, name); \
		sep = ", "; \
	}
	if (t->flags != 0) {
		sep = " flags (";
		X(R_FIXLEN,	"FIXLEN");
		X(B_INMEM,	"INMEM");
		X(B_NODUPS,	"NODUPS");
		X(B_RDONLY,	"RDONLY");
		X(R_RECNO,	"RECNO");
		X(B_METADIRTY,"METADIRTY");
		(void)fprintf(stderr, ")\n");
	}
#undef X

	for (i = P_ROOT;
	    (h = (PAGE *)mpool_get(t->bt_mp, i, MPOOL_IGNOREPIN)) != NULL; ++i)
		__bt_dpage(h);
}

void
__bt_dmpage(PAGE *h)
{
	BTMETA *m;
	char *sep;

	m = (BTMETA *)h;
	(void)fprintf(stderr, "magic %x\n", m->magic);
	(void)fprintf(stderr, "version %u\n", m->version);
	(void)fprintf(stderr, "psize %u\n", m->psize);
	(void)fprintf(stderr, "free %u\n", m->free);
	(void)fprintf(stderr, "nrecs %u\n", m->nrecs);
	(void)fprintf(stderr, "flags %u", m->flags);
#undef X
#define	X(flag, name) \
	if (m->flags & flag) { \
		(void)fprintf(stderr, "%s%s", sep, name); \
		sep = ", "; \
	}
	if (m->flags) {
		sep = " (";
		X(B_NODUPS,	"NODUPS");
		X(R_RECNO,	"RECNO");
		(void)fprintf(stderr, ")");
	}
}

void
__bt_dnpage(DB *dbp, pgno_t pgno)
{
	BTREE *t;
	PAGE *h;

	t = (BTREE *)dbp->internal;
	if ((h = (PAGE *)mpool_get(t->bt_mp, pgno, MPOOL_IGNOREPIN)) != NULL)
		__bt_dpage(h);
}

void
__bt_dpage(PAGE *h)
{
	BINTERNAL *bi;
	BLEAF *bl;
	RINTERNAL *ri;
	RLEAF *rl;
	indx_t cur, top;
	char *sep;

	(void)fprintf(stderr, "    page %u: (", h->pgno);
#undef X
#define	X(flag, name) \
	if (h->flags & flag) { \
		(void)fprintf(stderr, "%s%s", sep, name); \
		sep = ", "; \
	}
	sep = "";
	X(P_BINTERNAL,	"BINTERNAL")
	X(P_BLEAF,	"BLEAF")
	X(P_RINTERNAL,	"RINTERNAL")
	X(P_RLEAF,	"RLEAF")
	X(P_OVERFLOW,	"OVERFLOW")
	X(P_PRESERVE,	"PRESERVE");
	(void)fprintf(stderr, ")\n");
#undef X

	(void)fprintf(stderr, "\tprev %2u next %2u", h->prevpg, h->nextpg);
	if (h->flags & P_OVERFLOW)
		return;

	top = NEXTINDEX(h);
	(void)fprintf(stderr, " lower %3d upper %3d nextind %d\n",
	    h->lower, h->upper, top);
	for (cur = 0; cur < top; cur++) {
		(void)fprintf(stderr, "\t[%03d] %4d ", cur, h->linp[cur]);
		switch (h->flags & P_TYPE) {
		case P_BINTERNAL:
			bi = GETBINTERNAL(h, cur);
			(void)fprintf(stderr,
			    "size %03d pgno %03d", bi->ksize, bi->pgno);
			if (bi->flags & P_BIGKEY)
				(void)fprintf(stderr, " (indirect)");
			else if (bi->ksize)
				(void)fprintf(stderr,
				    " {%.*s}", (int)bi->ksize, bi->bytes);
			break;
		case P_RINTERNAL:
			ri = GETRINTERNAL(h, cur);
			(void)fprintf(stderr, "entries %03d pgno %03d",
				ri->nrecs, ri->pgno);
			break;
		case P_BLEAF:
			bl = GETBLEAF(h, cur);
			if (bl->flags & P_BIGKEY)
				(void)fprintf(stderr,
				    "big key page %u size %u/",
				    *(pgno_t *)bl->bytes,
				    *(u_int32_t *)(bl->bytes + sizeof(pgno_t)));
			else if (bl->ksize)
				(void)fprintf(stderr, "%.*s/",
				    bl->ksize, bl->bytes);
			if (bl->flags & P_BIGDATA)
				(void)fprintf(stderr,
				    "big data page %u size %u",
				    *(pgno_t *)(bl->bytes + bl->ksize),
				    *(u_int32_t *)(bl->bytes + bl->ksize +
				    sizeof(pgno_t)));
			else if (bl->dsize)
				(void)fprintf(stderr, "%.*s",
				    (int)bl->dsize, bl->bytes + bl->ksize);
			break;
		case P_RLEAF:
			rl = GETRLEAF(h, cur);
			if (rl->flags & P_BIGDATA)
				(void)fprintf(stderr,
				    "big data page %u size %u",
				    *(pgno_t *)rl->bytes,
				    *(u_int32_t *)(rl->bytes + sizeof(pgno_t)));
			else if (rl->dsize)
				(void)fprintf(stderr,
				    "%.*s", (int)rl->dsize, rl->bytes);
			break;
		}
		(void)fprintf(stderr, "\n");
	}
}
#endif

#ifdef STATISTICS
void
__bt_stat(DB *dbp)
{
	BTREE *t;
	PAGE *h;
	pgno_t i, pcont, pinternal, pleaf;
	unsigned long ifree, lfree, nkeys;
	int levels;

	t = (BTREE *)dbp->internal;
	pcont = pinternal = pleaf = 0;
	nkeys = ifree = lfree = 0;
	for (i = P_ROOT;
	    (h = (PAGE *)mpool_get(t->bt_mp, i, MPOOL_IGNOREPIN)) != NULL; ++i)
		switch (h->flags & P_TYPE) {
		case P_BINTERNAL:
		case P_RINTERNAL:
			++pinternal;
			ifree += h->upper - h->lower;
			break;
		case P_BLEAF:
		case P_RLEAF:
			++pleaf;
			lfree += h->upper - h->lower;
			nkeys += NEXTINDEX(h);
			break;
		case P_OVERFLOW:
			++pcont;
			break;
		}

	for (i = P_ROOT, levels = 0 ;; ++levels) {
		h = (PAGE *)mpool_get(t->bt_mp, i, MPOOL_IGNOREPIN);
		if (h->flags & (P_BLEAF|P_RLEAF)) {
			if (levels == 0)
				levels = 1;
			break;
		}
		i = F_ISSET(t, R_RECNO) ?
		    GETRINTERNAL(h, 0)->pgno :
		    GETBINTERNAL(h, 0)->pgno;
	}

	(void)fprintf(stderr, "%d level%s with %lu keys",
	    levels, levels == 1 ? "" : "s", nkeys);
	if (F_ISSET(t, R_RECNO))
		(void)fprintf(stderr, " (%u header count)", t->bt_nrecs);
	(void)fprintf(stderr,
	    "\n%u pages (leaf %u, internal %u, overflow %u)\n",
	    pinternal + pleaf + pcont, pleaf, pinternal, pcont);
	(void)fprintf(stderr, "%lu cache hits, %lu cache misses\n",
	    bt_cache_hit, bt_cache_miss);
	(void)fprintf(stderr, "%lu splits (%lu root splits, %lu sort splits)\n",
	    bt_split, bt_rootsplit, bt_sortsplit);
	pleaf *= t->bt_psize - BTDATAOFF;
	if (pleaf)
		(void)fprintf(stderr,
		    "%.0f%% leaf fill (%lu bytes used, %lu bytes free)\n",
		    ((double)(pleaf - lfree) / pleaf) * 100,
		    pleaf - lfree, lfree);
	pinternal *= t->bt_psize - BTDATAOFF;
	if (pinternal)
		(void)fprintf(stderr,
		    "%.0f%% internal fill (%lu bytes used, %lu bytes free\n",
		    ((double)(pinternal - ifree) / pinternal) * 100,
		    pinternal - ifree, ifree);
	if (bt_pfxsaved)
		(void)fprintf(stderr, "prefix checking removed %lu bytes.\n",
		    bt_pfxsaved);
}
#endif

} // namespace pbsd::lib_libc_db_btree::b0151
