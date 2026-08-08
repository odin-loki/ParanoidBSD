module;

#include <sys/types.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

export module pbsd.lib.libc.db.btree.b0149;

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#ifndef __PAST_END
#define __PAST_END(array, offset) (((__typeof__(*(array)) *)(array))[offset])
#endif

#define RET_ERROR	-1
#define RET_SUCCESS	 0

#define	MAX_PAGE_OFFSET	65535

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

#define NBINTERNAL(len)							\
	LALIGN(sizeof(u_int32_t) + sizeof(pgno_t) + sizeof(u_char) + (len))

#define	WR_BINTERNAL(p, size, pgno, flags) {				\
	*(u_int32_t *)p = size;						\
	p += sizeof(u_int32_t);						\
	*(pgno_t *)p = pgno;						\
	p += sizeof(pgno_t);						\
	*(u_char *)p = flags;						\
	p += sizeof(u_char);						\
}

typedef struct _rinternal {
	recno_t	nrecs;
	pgno_t	pgno;
} RINTERNAL;

#define	GETRINTERNAL(pg, indx)						\
	((RINTERNAL *)((char *)(pg) + (pg)->linp[indx]))

#define NRINTERNAL							\
	LALIGN(sizeof(recno_t) + sizeof(pgno_t))

#define	WR_RINTERNAL(p, nrecs, pgno) {					\
	*(recno_t *)p = nrecs;						\
	p += sizeof(recno_t);						\
	*(pgno_t *)p = pgno;						\
}

typedef struct _bleaf {
	u_int32_t	ksize;
	u_int32_t	dsize;
	u_char	flags;
	char	bytes[1];
} BLEAF;

#define	GETBLEAF(pg, indx)						\
	((BLEAF *)((char *)(pg) + (pg)->linp[indx]))

#define NBLEAF(p)	NBLEAFDBT((p)->ksize, (p)->dsize)

#define NBLEAFDBT(ksize, dsize)						\
	LALIGN(sizeof(u_int32_t) + sizeof(u_int32_t) + sizeof(u_char) +	\
	    (ksize) + (dsize))

#define	WR_BLEAF(p, key, data, flags) {					\
	*(u_int32_t *)p = key->size;					\
	p += sizeof(u_int32_t);						\
	*(u_int32_t *)p = data->size;					\
	p += sizeof(u_int32_t);						\
	*(u_char *)p = flags;						\
	p += sizeof(u_char);						\
	memmove(p, key->data, key->size);				\
	p += key->size;							\
	memmove(p, data->data, data->size);				\
}

typedef struct _rleaf {
	u_int32_t	dsize;
	u_char	flags;
	char	bytes[1];
} RLEAF;

#define	GETRLEAF(pg, indx)						\
	((RLEAF *)((char *)(pg) + (pg)->linp[indx]))

#define NRLEAF(p)	NRLEAFDBT((p)->dsize)

#define	NRLEAFDBT(dsize)						\
	LALIGN(sizeof(u_int32_t) + sizeof(u_char) + (dsize))

#define	WR_RLEAF(p, data, flags) {					\
	*(u_int32_t *)p = data->size;					\
	p += sizeof(u_int32_t);						\
	*(u_char *)p = flags;						\
	p += sizeof(u_char);						\
	memmove(p, data->data, data->size);				\
}

typedef struct _epgno {
	pgno_t	pgno;
	indx_t	index;
} EPGNO;

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

typedef struct _btree {
	MPOOL	 *bt_mp;
	DB	 *bt_dbp;
	struct {
		PAGE	*page;
		indx_t	 index;
	} bt_cur;
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
void __dbpanic(DB *);
}

export namespace pbsd::lib_libc_db_btree::b0149 {

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

int	 bt_broot(BTREE *, PAGE *, PAGE *, PAGE *);
PAGE	*bt_page(BTREE *, PAGE *, PAGE **, PAGE **, indx_t *, size_t);
int	 bt_preserve(BTREE *, pgno_t);
PAGE	*bt_psplit(BTREE *, PAGE *, PAGE *, PAGE *, indx_t *, size_t);
PAGE	*bt_root(BTREE *, PAGE *, PAGE **, PAGE **, indx_t *, size_t);
int	 bt_rroot(BTREE *, PAGE *, PAGE *, PAGE *);
recno_t	 rec_total(PAGE *);

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

int
__bt_split(BTREE *t, PAGE *sp, const DBT *key, const DBT *data, int flags,
    size_t ilen, u_int32_t argskip)
{
	BINTERNAL *bi;
	BLEAF *bl, *tbl;
	DBT a, b;
	EPGNO *parent;
	PAGE *h, *l, *r, *lchild, *rchild;
	indx_t nxtindex;
	u_int16_t skip;
	u_int32_t n, nbytes, nksize;
	int parentsplit;
	char *dest;

	skip = argskip;
	h = sp->pgno == P_ROOT ?
	    bt_root(t, sp, &l, &r, &skip, ilen) :
	    bt_page(t, sp, &l, &r, &skip, ilen);
	if (h == NULL)
		return (RET_ERROR);

	h->linp[skip] = h->upper -= ilen;
	dest = (char *)h + h->upper;
	if (F_ISSET(t, R_RECNO))
		WR_RLEAF(dest, data, flags)
	else
		WR_BLEAF(dest, key, data, flags)

	if (sp->pgno == P_ROOT &&
	    (F_ISSET(t, R_RECNO) ?
	    bt_rroot(t, sp, l, r) : bt_broot(t, sp, l, r)) == RET_ERROR)
		goto err2;

	while ((parent = BT_POP(t)) != NULL) {
		lchild = l;
		rchild = r;

		if ((h = (PAGE *)mpool_get(t->bt_mp, parent->pgno, 0)) == NULL)
			goto err2;

		skip = parent->index + 1;

		switch (rchild->flags & P_TYPE) {
		case P_BINTERNAL:
			bi = GETBINTERNAL(rchild, 0);
			nbytes = NBINTERNAL(bi->ksize);
			break;
		case P_BLEAF:
			bl = GETBLEAF(rchild, 0);
			nbytes = NBINTERNAL(bl->ksize);
			if (t->bt_pfx && !(bl->flags & P_BIGKEY) &&
			    (h->prevpg != P_INVALID || skip > 1)) {
				tbl = GETBLEAF(lchild, NEXTINDEX(lchild) - 1);
				a.size = tbl->ksize;
				a.data = tbl->bytes;
				b.size = bl->ksize;
				b.data = bl->bytes;
				nksize = t->bt_pfx(&a, &b);
				n = NBINTERNAL(nksize);
				if (n < nbytes) {
					nbytes = n;
				} else
					nksize = 0;
			} else
				nksize = 0;
			break;
		case P_RINTERNAL:
		case P_RLEAF:
			nbytes = NRINTERNAL;
			break;
		default:
			abort();
		}

		if ((u_int32_t)(h->upper - h->lower) < nbytes + sizeof(indx_t)) {
			sp = h;
			h = h->pgno == P_ROOT ?
			    bt_root(t, h, &l, &r, &skip, nbytes) :
			    bt_page(t, h, &l, &r, &skip, nbytes);
			if (h == NULL)
				goto err1;
			parentsplit = 1;
		} else {
			if (skip < (nxtindex = NEXTINDEX(h)))
				memmove(h->linp + skip + 1, h->linp + skip,
				    (nxtindex - skip) * sizeof(indx_t));
			h->lower += sizeof(indx_t);
			parentsplit = 0;
		}

		switch (rchild->flags & P_TYPE) {
		case P_BINTERNAL:
			h->linp[skip] = h->upper -= nbytes;
			dest = (char *)h + h->linp[skip];
			memmove(dest, bi, nbytes);
			((BINTERNAL *)dest)->pgno = rchild->pgno;
			break;
		case P_BLEAF:
			h->linp[skip] = h->upper -= nbytes;
			dest = (char *)h + h->linp[skip];
			WR_BINTERNAL(dest, nksize ? nksize : bl->ksize,
			    rchild->pgno, bl->flags & P_BIGKEY);
			memmove(dest, bl->bytes, nksize ? nksize : bl->ksize);
			if (bl->flags & P_BIGKEY) {
				pgno_t pgno;
				memcpy(&pgno, bl->bytes, sizeof(pgno));
				if (bt_preserve(t, pgno) == RET_ERROR)
					goto err1;
			}
			break;
		case P_RINTERNAL:
			if (skip > 0)
				dest = (char *)h + h->linp[skip - 1];
			else
				dest = (char *)l + l->linp[NEXTINDEX(l) - 1];
			((RINTERNAL *)dest)->nrecs = rec_total(lchild);
			((RINTERNAL *)dest)->pgno = lchild->pgno;

			h->linp[skip] = h->upper -= nbytes;
			dest = (char *)h + h->linp[skip];
			((RINTERNAL *)dest)->nrecs = rec_total(rchild);
			((RINTERNAL *)dest)->pgno = rchild->pgno;
			break;
		case P_RLEAF:
			if (skip > 0)
				dest = (char *)h + h->linp[skip - 1];
			else
				dest = (char *)l + l->linp[NEXTINDEX(l) - 1];
			((RINTERNAL *)dest)->nrecs = NEXTINDEX(lchild);
			((RINTERNAL *)dest)->pgno = lchild->pgno;

			h->linp[skip] = h->upper -= nbytes;
			dest = (char *)h + h->linp[skip];
			((RINTERNAL *)dest)->nrecs = NEXTINDEX(rchild);
			((RINTERNAL *)dest)->pgno = rchild->pgno;
			break;
		default:
			abort();
		}

		if (!parentsplit) {
			mpool_put(t->bt_mp, h, MPOOL_DIRTY);
			break;
		}

		if (sp->pgno == P_ROOT &&
		    (F_ISSET(t, R_RECNO) ?
		    bt_rroot(t, sp, l, r) : bt_broot(t, sp, l, r)) == RET_ERROR)
			goto err1;

		mpool_put(t->bt_mp, lchild, MPOOL_DIRTY);
		mpool_put(t->bt_mp, rchild, MPOOL_DIRTY);
	}

	mpool_put(t->bt_mp, l, MPOOL_DIRTY);
	mpool_put(t->bt_mp, r, MPOOL_DIRTY);

	return (RET_SUCCESS);

err1:	mpool_put(t->bt_mp, lchild, MPOOL_DIRTY);
	mpool_put(t->bt_mp, rchild, MPOOL_DIRTY);

err2:	mpool_put(t->bt_mp, l, 0);
	mpool_put(t->bt_mp, r, 0);
	__dbpanic(t->bt_dbp);
	return (RET_ERROR);
}

PAGE *
bt_page(BTREE *t, PAGE *h, PAGE **lp, PAGE **rp, indx_t *skip, size_t ilen)
{
	PAGE *l, *r, *tp;
	pgno_t npg;

	if ((r = __bt_new(t, &npg)) == NULL)
		return (NULL);
	r->pgno = npg;
	r->lower = BTDATAOFF;
	r->upper = t->bt_psize;
	r->nextpg = h->nextpg;
	r->prevpg = h->pgno;
	r->flags = h->flags & P_TYPE;

	if (h->nextpg == P_INVALID && *skip == NEXTINDEX(h)) {
		h->nextpg = r->pgno;
		r->lower = BTDATAOFF + sizeof(indx_t);
		*skip = 0;
		*lp = h;
		*rp = r;
		return (r);
	}

	if ((l = (PAGE *)calloc(1, t->bt_psize)) == NULL) {
		mpool_put(t->bt_mp, r, 0);
		return (NULL);
	}
	l->pgno = h->pgno;
	l->nextpg = r->pgno;
	l->prevpg = h->prevpg;
	l->lower = BTDATAOFF;
	l->upper = t->bt_psize;
	l->flags = h->flags & P_TYPE;

	if (h->nextpg != P_INVALID) {
		if ((tp = (PAGE *)mpool_get(t->bt_mp, h->nextpg, 0)) == NULL) {
			free(l);
			return (NULL);
		}
		tp->prevpg = r->pgno;
		mpool_put(t->bt_mp, tp, MPOOL_DIRTY);
	}

	tp = bt_psplit(t, h, l, r, skip, ilen);

	memmove(h, l, t->bt_psize);
	if (tp == l)
		tp = h;
	free(l);

	*lp = h;
	*rp = r;
	return (tp);
}

PAGE *
bt_root(BTREE *t, PAGE *h, PAGE **lp, PAGE **rp, indx_t *skip, size_t ilen)
{
	PAGE *l, *r, *tp;
	pgno_t lnpg, rnpg;

	if ((l = __bt_new(t, &lnpg)) == NULL ||
	    (r = __bt_new(t, &rnpg)) == NULL)
		return (NULL);
	l->pgno = lnpg;
	r->pgno = rnpg;
	l->nextpg = r->pgno;
	r->prevpg = l->pgno;
	l->prevpg = r->nextpg = P_INVALID;
	l->lower = r->lower = BTDATAOFF;
	l->upper = r->upper = t->bt_psize;
	l->flags = r->flags = h->flags & P_TYPE;

	tp = bt_psplit(t, h, l, r, skip, ilen);

	*lp = l;
	*rp = r;
	return (tp);
}

int
bt_rroot(BTREE *t, PAGE *h, PAGE *l, PAGE *r)
{
	char *dest;

	h->linp[0] = h->upper = t->bt_psize - NRINTERNAL;
	dest = (char *)h + h->upper;
	WR_RINTERNAL(dest,
	    l->flags & P_RLEAF ? NEXTINDEX(l) : rec_total(l), l->pgno);

	__PAST_END(h->linp, 1) = h->upper -= NRINTERNAL;
	dest = (char *)h + h->upper;
	WR_RINTERNAL(dest,
	    r->flags & P_RLEAF ? NEXTINDEX(r) : rec_total(r), r->pgno);

	h->lower = BTDATAOFF + 2 * sizeof(indx_t);

	h->flags &= ~P_TYPE;
	h->flags |= P_RINTERNAL;
	mpool_put(t->bt_mp, h, MPOOL_DIRTY);

	return (RET_SUCCESS);
}

int
bt_broot(BTREE *t, PAGE *h, PAGE *l, PAGE *r)
{
	BINTERNAL *bi;
	BLEAF *bl;
	u_int32_t nbytes;
	char *dest;

	nbytes = NBINTERNAL(0);
	h->linp[0] = h->upper = t->bt_psize - nbytes;
	dest = (char *)h + h->upper;
	WR_BINTERNAL(dest, 0, l->pgno, 0);

	switch (h->flags & P_TYPE) {
	case P_BLEAF:
		bl = GETBLEAF(r, 0);
		nbytes = NBINTERNAL(bl->ksize);
		__PAST_END(h->linp, 1) = h->upper -= nbytes;
		dest = (char *)h + h->upper;
		WR_BINTERNAL(dest, bl->ksize, r->pgno, 0);
		memmove(dest, bl->bytes, bl->ksize);

	if (bl->flags & P_BIGKEY) {
			pgno_t pgno;
			memcpy(&pgno, bl->bytes, sizeof(pgno));
			if (bt_preserve(t, pgno) == RET_ERROR)
				return (RET_ERROR);
		}
		break;
	case P_BINTERNAL:
		bi = GETBINTERNAL(r, 0);
		nbytes = NBINTERNAL(bi->ksize);
		__PAST_END(h->linp, 1) = h->upper -= nbytes;
		dest = (char *)h + h->upper;
		memmove(dest, bi, nbytes);
		((BINTERNAL *)dest)->pgno = r->pgno;
		break;
	default:
		abort();
	}

	h->lower = BTDATAOFF + 2 * sizeof(indx_t);

	h->flags &= ~P_TYPE;
	h->flags |= P_BINTERNAL;
	mpool_put(t->bt_mp, h, MPOOL_DIRTY);

	return (RET_SUCCESS);
}

PAGE *
bt_psplit(BTREE *t, PAGE *h, PAGE *l, PAGE *r, indx_t *pskip, size_t ilen)
{
	BINTERNAL *bi;
	BLEAF *bl;
	CURSOR *c;
	RLEAF *rl;
	PAGE *rval;
	void *src;
	indx_t full, half, nxt, off, skip, top, used;
	u_int32_t nbytes;
	int bigkeycnt, isbigkey;

	bigkeycnt = 0;
	skip = *pskip;
	full = t->bt_psize - BTDATAOFF;
	half = full / 2;
	used = 0;
	for (nxt = off = 0, top = NEXTINDEX(h); nxt < top; ++off) {
		if (skip == off) {
			nbytes = ilen;
			isbigkey = 0;
		} else
			switch (h->flags & P_TYPE) {
			case P_BINTERNAL:
				src = bi = GETBINTERNAL(h, nxt);
				nbytes = NBINTERNAL(bi->ksize);
				isbigkey = bi->flags & P_BIGKEY;
				break;
			case P_BLEAF:
				src = bl = GETBLEAF(h, nxt);
				nbytes = NBLEAF(bl);
				isbigkey = bl->flags & P_BIGKEY;
				break;
			case P_RINTERNAL:
				src = GETRINTERNAL(h, nxt);
				nbytes = NRINTERNAL;
				isbigkey = 0;
				break;
			case P_RLEAF:
				src = rl = GETRLEAF(h, nxt);
				nbytes = NRLEAF(rl);
				isbigkey = 0;
				break;
			default:
				abort();
			}

		if ((skip <= off && used + nbytes + sizeof(indx_t) >= full) ||
		    nxt == top - 1) {
			--off;
			break;
		}

		if (skip != off) {
			++nxt;

			l->linp[off] = l->upper -= nbytes;
			memmove((char *)l + l->upper, src, nbytes);
		}

		used += nbytes + sizeof(indx_t);
		if (used >= half) {
			if (!isbigkey || bigkeycnt == 3)
				break;
			else
				++bigkeycnt;
		}
	}

	l->lower += (off + 1) * sizeof(indx_t);

	c = &t->bt_cursor;
	if (F_ISSET(c, CURS_INIT) && c->pg.pgno == h->pgno) {
		if (c->pg.index >= skip)
			++c->pg.index;
		if (c->pg.index < nxt)
			c->pg.pgno = l->pgno;
		else {
			c->pg.pgno = r->pgno;
			c->pg.index -= nxt;
		}
	}

	if (skip <= off) {
		skip = MAX_PAGE_OFFSET;
		rval = l;
	} else {
		rval = r;
		*pskip -= nxt;
	}

	for (off = 0; nxt < top; ++off) {
		if (skip == nxt) {
			++off;
			skip = MAX_PAGE_OFFSET;
		}
		switch (h->flags & P_TYPE) {
		case P_BINTERNAL:
			src = bi = GETBINTERNAL(h, nxt);
			nbytes = NBINTERNAL(bi->ksize);
			break;
		case P_BLEAF:
			src = bl = GETBLEAF(h, nxt);
			nbytes = NBLEAF(bl);
			break;
		case P_RINTERNAL:
			src = GETRINTERNAL(h, nxt);
			nbytes = NRINTERNAL;
			break;
		case P_RLEAF:
			src = rl = GETRLEAF(h, nxt);
			nbytes = NRLEAF(rl);
			break;
		default:
			abort();
		}
		++nxt;
		r->linp[off] = r->upper -= nbytes;
		memmove((char *)r + r->upper, src, nbytes);
	}
	r->lower += off * sizeof(indx_t);

	if (skip == top)
		r->lower += sizeof(indx_t);

	return (rval);
}

int
bt_preserve(BTREE *t, pgno_t pg)
{
	PAGE *h;

	if ((h = (PAGE *)mpool_get(t->bt_mp, pg, 0)) == NULL)
		return (RET_ERROR);
	h->flags |= P_PRESERVE;
	mpool_put(t->bt_mp, h, MPOOL_DIRTY);
	return (RET_SUCCESS);
}

recno_t
rec_total(PAGE *h)
{
	recno_t recs;
	indx_t nxt, top;

	for (recs = 0, nxt = 0, top = NEXTINDEX(h); nxt < top; ++nxt)
		recs += GETRINTERNAL(h, nxt)->nrecs;
	return (recs);
}

} // namespace pbsd::lib_libc_db_btree::b0149
