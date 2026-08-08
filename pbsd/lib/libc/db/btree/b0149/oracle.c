/*
 * Oracle for batch b0149: original HardenedBSD bt_split.c with ref_ prefix.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
typedef unsigned int	u_int;
typedef unsigned char	u_char;
typedef uint32_t	u_int32_t;
typedef uint16_t	u_int16_t;
typedef uint8_t		u_int8_t;
typedef char		* caddr_t;

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

typedef struct MPOOL MPOOL;

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

#define MAX_POOL 128

typedef struct {
	unsigned get_calls;
	unsigned put_calls;
	unsigned new_calls;
	unsigned panic_calls;
	int get_force_null;
	int new_force_null;
	int calloc_fail;
	pgno_t get_last_pgno;
	unsigned get_last_flags;
	void *last_put_page;
	unsigned last_put_flags;
	pgno_t next_pgno;
	int nreg;
	pgno_t reg_pgno[MAX_POOL];
	void *reg_page[MAX_POOL];
	int new_fail_after;
} test_mock_state;

test_mock_state test_mock;

void test_mock_reset(void)
{
	memset(&test_mock, 0, sizeof(test_mock));
	test_mock.next_pgno = 100;
}

void test_mock_register(pgno_t pgno, void *page)
{
	if (test_mock.nreg < MAX_POOL) {
		test_mock.reg_pgno[test_mock.nreg] = pgno;
		test_mock.reg_page[test_mock.nreg] = page;
		test_mock.nreg++;
	}
}

void *mpool_get(MPOOL *mp, pgno_t pgno, unsigned int flags)
{
	int i;
	(void)mp;
	test_mock.get_calls++;
	test_mock.get_last_pgno = pgno;
	test_mock.get_last_flags = flags;
	if (test_mock.get_force_null)
		return NULL;
	for (i = 0; i < test_mock.nreg; i++)
		if (test_mock.reg_pgno[i] == pgno)
			return test_mock.reg_page[i];
	return NULL;
}

int mpool_put(MPOOL *mp, void *page, unsigned int flags)
{
	(void)mp;
	test_mock.put_calls++;
	test_mock.last_put_page = page;
	test_mock.last_put_flags = flags;
	return RET_SUCCESS;
}

PAGE *__bt_new(BTREE *t, pgno_t *npg)
{
	PAGE *h;
	test_mock.new_calls++;
	if (test_mock.new_force_null)
		return NULL;
	if (test_mock.new_fail_after > 0) {
		test_mock.new_fail_after--;
		return NULL;
	}
	if (test_mock.calloc_fail)
		return NULL;
	h = (PAGE *)calloc(1, t->bt_psize);
	if (h == NULL)
		return NULL;
	if (npg)
		*npg = test_mock.next_pgno++;
	return h;
}

void __dbpanic(DB *dbp)
{
	(void)dbp;
	test_mock.panic_calls++;
}

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
 * __BT_SPLIT -- Split the tree.
 *
 * Parameters:
 *	t:	tree
 *	sp:	page to split
 *	key:	key to insert
 *	data:	data to insert
 *	flags:	BIGKEY/BIGDATA flags
 *	ilen:	insert length
 *	skip:	index to leave open
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS
 */
int
ref___bt_split(BTREE *t, PAGE *sp, const DBT *key, const DBT *data, int flags,
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

	/*
	 * Split the page into two pages, l and r.  The split routines return
	 * a pointer to the page into which the key should be inserted and with
	 * skip set to the offset which should be used.  Additionally, l and r
	 * are pinned.
	 */
	skip = argskip;
	h = sp->pgno == P_ROOT ?
	    ref_bt_root(t, sp, &l, &r, &skip, ilen) :
	    ref_bt_page(t, sp, &l, &r, &skip, ilen);
	if (h == NULL)
		return (RET_ERROR);

	/*
	 * Insert the new key/data pair into the leaf page.  (Key inserts
	 * always cause a leaf page to split first.)
	 */
	h->linp[skip] = h->upper -= ilen;
	dest = (char *)h + h->upper;
	if (F_ISSET(t, R_RECNO))
		WR_RLEAF(dest, data, flags)
	else
		WR_BLEAF(dest, key, data, flags)

	/* If the root page was split, make it look right. */
	if (sp->pgno == P_ROOT &&
	    (F_ISSET(t, R_RECNO) ?
	    ref_bt_rroot(t, sp, l, r) : ref_bt_broot(t, sp, l, r)) == RET_ERROR)
		goto err2;

	/*
	 * Now we walk the parent page stack -- a LIFO stack of the pages that
	 * were traversed when we searched for the page that split.  Each stack
	 * entry is a page number and a page index offset.  The offset is for
	 * the page traversed on the search.  We've just split a page, so we
	 * have to insert a new key into the parent page.
	 *
	 * If the insert into the parent page causes it to split, may have to
	 * continue splitting all the way up the tree.  We stop if the root
	 * splits or the page inserted into didn't have to split to hold the
	 * new key.  Some algorithms replace the key for the old page as well
	 * as the new page.  We don't, as there's no reason to believe that the
	 * first key on the old page is any better than the key we have, and,
	 * in the case of a key being placed at index 0 causing the split, the
	 * key is unavailable.
	 *
	 * There are a maximum of 5 pages pinned at any time.  We keep the left
	 * and right pages pinned while working on the parent.   The 5 are the
	 * two children, left parent and right parent (when the parent splits)
	 * and the root page or the overflow key page when calling bt_preserve.
	 * This code must make sure that all pins are released other than the
	 * root page or overflow page which is unlocked elsewhere.
	 */
	while ((parent = BT_POP(t)) != NULL) {
		lchild = l;
		rchild = r;

		/* Get the parent page. */
		if ((h = mpool_get(t->bt_mp, parent->pgno, 0)) == NULL)
			goto err2;

		/*
		 * The new key goes ONE AFTER the index, because the split
		 * was to the right.
		 */
		skip = parent->index + 1;

		/*
		 * Calculate the space needed on the parent page.
		 *
		 * Prefix trees: space hack when inserting into BINTERNAL
		 * pages.  Retain only what's needed to distinguish between
		 * the new entry and the LAST entry on the page to its left.
		 * If the keys compare equal, retain the entire key.  Note,
		 * we don't touch overflow keys, and the entire key must be
		 * retained for the next-to-left most key on the leftmost
		 * page of each level, or the search will fail.  Applicable
		 * ONLY to internal pages that have leaf pages as children.
		 * Further reduction of the key between pairs of internal
		 * pages loses too much information.
		 */
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

		/* Split the parent page if necessary or shift the indices. */
		if ((u_int32_t)(h->upper - h->lower) < nbytes + sizeof(indx_t)) {
			sp = h;
			h = h->pgno == P_ROOT ?
			    ref_bt_root(t, h, &l, &r, &skip, nbytes) :
			    ref_bt_page(t, h, &l, &r, &skip, nbytes);
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

		/* Insert the key into the parent page. */
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
				if (ref_bt_preserve(t, pgno) == RET_ERROR)
					goto err1;
			}
			break;
		case P_RINTERNAL:
			/*
			 * Update the left page count.  If split
			 * added at index 0, fix the correct page.
			 */
			if (skip > 0)
				dest = (char *)h + h->linp[skip - 1];
			else
				dest = (char *)l + l->linp[NEXTINDEX(l) - 1];
			((RINTERNAL *)dest)->nrecs = ref_rec_total(lchild);
			((RINTERNAL *)dest)->pgno = lchild->pgno;

			/* Update the right page count. */
			h->linp[skip] = h->upper -= nbytes;
			dest = (char *)h + h->linp[skip];
			((RINTERNAL *)dest)->nrecs = ref_rec_total(rchild);
			((RINTERNAL *)dest)->pgno = rchild->pgno;
			break;
		case P_RLEAF:
			/*
			 * Update the left page count.  If split
			 * added at index 0, fix the correct page.
			 */
			if (skip > 0)
				dest = (char *)h + h->linp[skip - 1];
			else
				dest = (char *)l + l->linp[NEXTINDEX(l) - 1];
			((RINTERNAL *)dest)->nrecs = NEXTINDEX(lchild);
			((RINTERNAL *)dest)->pgno = lchild->pgno;

			/* Update the right page count. */
			h->linp[skip] = h->upper -= nbytes;
			dest = (char *)h + h->linp[skip];
			((RINTERNAL *)dest)->nrecs = NEXTINDEX(rchild);
			((RINTERNAL *)dest)->pgno = rchild->pgno;
			break;
		default:
			abort();
		}

		/* Unpin the held pages. */
		if (!parentsplit) {
			mpool_put(t->bt_mp, h, MPOOL_DIRTY);
			break;
		}

		/* If the root page was split, make it look right. */
		if (sp->pgno == P_ROOT &&
		    (F_ISSET(t, R_RECNO) ?
		    ref_bt_rroot(t, sp, l, r) : ref_bt_broot(t, sp, l, r)) == RET_ERROR)
			goto err1;

		mpool_put(t->bt_mp, lchild, MPOOL_DIRTY);
		mpool_put(t->bt_mp, rchild, MPOOL_DIRTY);
	}

	/* Unpin the held pages. */
	mpool_put(t->bt_mp, l, MPOOL_DIRTY);
	mpool_put(t->bt_mp, r, MPOOL_DIRTY);

	/* Clear any pages left on the stack. */
	return (RET_SUCCESS);

	/*
	 * If something fails in the above loop we were already walking back
	 * up the tree and the tree is now inconsistent.  Nothing much we can
	 * do about it but release any memory we're holding.
	 */
err1:	mpool_put(t->bt_mp, lchild, MPOOL_DIRTY);
	mpool_put(t->bt_mp, rchild, MPOOL_DIRTY);

err2:	mpool_put(t->bt_mp, l, 0);
	mpool_put(t->bt_mp, r, 0);
	__dbpanic(t->bt_dbp);
	return (RET_ERROR);
}

/*
 * BT_PAGE -- Split a non-root page of a btree.
 *
 * Parameters:
 *	t:	tree
 *	h:	root page
 *	lp:	pointer to left page pointer
 *	rp:	pointer to right page pointer
 *	skip:	pointer to index to leave open
 *	ilen:	insert length
 *
 * Returns:
 *	Pointer to page in which to insert or NULL on error.
 */
static PAGE *
ref_bt_page(BTREE *t, PAGE *h, PAGE **lp, PAGE **rp, indx_t *skip, size_t ilen)
{
	PAGE *l, *r, *tp;
	pgno_t npg;

	/* Put the new right page for the split into place. */
	if ((r = __bt_new(t, &npg)) == NULL)
		return (NULL);
	r->pgno = npg;
	r->lower = BTDATAOFF;
	r->upper = t->bt_psize;
	r->nextpg = h->nextpg;
	r->prevpg = h->pgno;
	r->flags = h->flags & P_TYPE;

	/*
	 * If we're splitting the last page on a level because we're appending
	 * a key to it (skip is NEXTINDEX()), it's likely that the data is
	 * sorted.  Adding an empty page on the side of the level is less work
	 * and can push the fill factor much higher than normal.  If we're
	 * wrong it's no big deal, we'll just do the split the right way next
	 * time.  It may look like it's equally easy to do a similar hack for
	 * reverse sorted data, that is, split the tree left, but it's not.
	 * Don't even try.
	 */
	if (h->nextpg == P_INVALID && *skip == NEXTINDEX(h)) {
		h->nextpg = r->pgno;
		r->lower = BTDATAOFF + sizeof(indx_t);
		*skip = 0;
		*lp = h;
		*rp = r;
		return (r);
	}

	/* Put the new left page for the split into place. */
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

	/* Fix up the previous pointer of the page after the split page. */
	if (h->nextpg != P_INVALID) {
		if ((tp = mpool_get(t->bt_mp, h->nextpg, 0)) == NULL) {
			free(l);
			/* XXX mpool_free(t->bt_mp, r->pgno); */
			return (NULL);
		}
		tp->prevpg = r->pgno;
		mpool_put(t->bt_mp, tp, MPOOL_DIRTY);
	}

	/*
	 * Split right.  The key/data pairs aren't sorted in the btree page so
	 * it's simpler to copy the data from the split page onto two new pages
	 * instead of copying half the data to the right page and compacting
	 * the left page in place.  Since the left page can't change, we have
	 * to swap the original and the allocated left page after the split.
	 */
	tp = ref_bt_psplit(t, h, l, r, skip, ilen);

	/* Move the new left page onto the old left page. */
	memmove(h, l, t->bt_psize);
	if (tp == l)
		tp = h;
	free(l);

	*lp = h;
	*rp = r;
	return (tp);
}

/*
 * BT_ROOT -- Split the root page of a btree.
 *
 * Parameters:
 *	t:	tree
 *	h:	root page
 *	lp:	pointer to left page pointer
 *	rp:	pointer to right page pointer
 *	skip:	pointer to index to leave open
 *	ilen:	insert length
 *
 * Returns:
 *	Pointer to page in which to insert or NULL on error.
 */
static PAGE *
ref_bt_root(BTREE *t, PAGE *h, PAGE **lp, PAGE **rp, indx_t *skip, size_t ilen)
{
	PAGE *l, *r, *tp;
	pgno_t lnpg, rnpg;

	/* Put the new left and right pages for the split into place. */
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

	/* Split the root page. */
	tp = ref_bt_psplit(t, h, l, r, skip, ilen);

	*lp = l;
	*rp = r;
	return (tp);
}

/*
 * BT_RROOT -- Fix up the recno root page after it has been split.
 *
 * Parameters:
 *	t:	tree
 *	h:	root page
 *	l:	left page
 *	r:	right page
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS
 */
static int
ref_bt_rroot(BTREE *t, PAGE *h, PAGE *l, PAGE *r)
{
	char *dest;

	/* Insert the left and right keys, set the header information. */
	h->linp[0] = h->upper = t->bt_psize - NRINTERNAL;
	dest = (char *)h + h->upper;
	WR_RINTERNAL(dest,
	    l->flags & P_RLEAF ? NEXTINDEX(l) : ref_rec_total(l), l->pgno);

	__PAST_END(h->linp, 1) = h->upper -= NRINTERNAL;
	dest = (char *)h + h->upper;
	WR_RINTERNAL(dest,
	    r->flags & P_RLEAF ? NEXTINDEX(r) : ref_rec_total(r), r->pgno);

	h->lower = BTDATAOFF + 2 * sizeof(indx_t);

	/* Unpin the root page, set to recno internal page. */
	h->flags &= ~P_TYPE;
	h->flags |= P_RINTERNAL;
	mpool_put(t->bt_mp, h, MPOOL_DIRTY);

	return (RET_SUCCESS);
}

/*
 * BT_BROOT -- Fix up the btree root page after it has been split.
 *
 * Parameters:
 *	t:	tree
 *	h:	root page
 *	l:	left page
 *	r:	right page
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS
 */
static int
ref_bt_broot(BTREE *t, PAGE *h, PAGE *l, PAGE *r)
{
	BINTERNAL *bi;
	BLEAF *bl;
	u_int32_t nbytes;
	char *dest;

	/*
	 * If the root page was a leaf page, change it into an internal page.
	 * We copy the key we split on (but not the key's data, in the case of
	 * a leaf page) to the new root page.
	 *
	 * The btree comparison code guarantees that the left-most key on any
	 * level of the tree is never used, so it doesn't need to be filled in.
	 */
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

		/*
		 * If the key is on an overflow page, mark the overflow chain
		 * so it isn't deleted when the leaf copy of the key is deleted.
		 */
	if (bl->flags & P_BIGKEY) {
			pgno_t pgno;
			memcpy(&pgno, bl->bytes, sizeof(pgno));
			if (ref_bt_preserve(t, pgno) == RET_ERROR)
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

	/* There are two keys on the page. */
	h->lower = BTDATAOFF + 2 * sizeof(indx_t);

	/* Unpin the root page, set to btree internal page. */
	h->flags &= ~P_TYPE;
	h->flags |= P_BINTERNAL;
	mpool_put(t->bt_mp, h, MPOOL_DIRTY);

	return (RET_SUCCESS);
}

/*
 * BT_PSPLIT -- Do the real work of splitting the page.
 *
 * Parameters:
 *	t:	tree
 *	h:	page to be split
 *	l:	page to put lower half of data
 *	r:	page to put upper half of data
 *	pskip:	pointer to index to leave open
 *	ilen:	insert length
 *
 * Returns:
 *	Pointer to page in which to insert.
 */
static PAGE *
ref_bt_psplit(BTREE *t, PAGE *h, PAGE *l, PAGE *r, indx_t *pskip, size_t ilen)
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

	/*
	 * Split the data to the left and right pages.  Leave the skip index
	 * open.  Additionally, make some effort not to split on an overflow
	 * key.  This makes internal page processing faster and can save
	 * space as overflow keys used by internal pages are never deleted.
	 */
	bigkeycnt = 0;
	skip = *pskip;
	full = t->bt_psize - BTDATAOFF;
	half = full / 2;
	used = 0;
	for (nxt = off = 0, top = NEXTINDEX(h); nxt < top; ++off) {
		if (skip == off) {
			nbytes = ilen;
			isbigkey = 0;		/* XXX: not really known. */
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

		/*
		 * If the key/data pairs are substantial fractions of the max
		 * possible size for the page, it's possible to get situations
		 * where we decide to try and copy too much onto the left page.
		 * Make sure that doesn't happen.
		 */
		if ((skip <= off && used + nbytes + sizeof(indx_t) >= full) ||
		    nxt == top - 1) {
			--off;
			break;
		}

		/* Copy the key/data pair, if not the skipped index. */
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

	/*
	 * Off is the last offset that's valid for the left page.
	 * Nxt is the first offset to be placed on the right page.
	 */
	l->lower += (off + 1) * sizeof(indx_t);

	/*
	 * If splitting the page that the cursor was on, the cursor has to be
	 * adjusted to point to the same record as before the split.  If the
	 * cursor is at or past the skipped slot, the cursor is incremented by
	 * one.  If the cursor is on the right page, it is decremented by the
	 * number of records split to the left page.
	 */
	c = &t->bt_cursor;
	if (F_ISSET(c, CURS_INIT) && c->pg.pgno == h->pgno) {
		if (c->pg.index >= skip)
			++c->pg.index;
		if (c->pg.index < nxt)			/* Left page. */
			c->pg.pgno = l->pgno;
		else {					/* Right page. */
			c->pg.pgno = r->pgno;
			c->pg.index -= nxt;
		}
	}

	/*
	 * If the skipped index was on the left page, just return that page.
	 * Otherwise, adjust the skip index to reflect the new position on
	 * the right page.
	 */
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

	/* If the key is being appended to the page, adjust the index. */
	if (skip == top)
		r->lower += sizeof(indx_t);

	return (rval);
}

/*
 * BT_PRESERVE -- Mark a chain of pages as used by an internal node.
 *
 * Chains of indirect blocks pointed to by leaf nodes get reclaimed when the
 * record that references them gets deleted.  Chains pointed to by internal
 * pages never get deleted.  This routine marks a chain as pointed to by an
 * internal page.
 *
 * Parameters:
 *	t:	tree
 *	pg:	page number of first page in the chain.
 *
 * Returns:
 *	RET_SUCCESS, RET_ERROR.
 */
static int
ref_bt_preserve(BTREE *t, pgno_t pg)
{
	PAGE *h;

	if ((h = mpool_get(t->bt_mp, pg, 0)) == NULL)
		return (RET_ERROR);
	h->flags |= P_PRESERVE;
	mpool_put(t->bt_mp, h, MPOOL_DIRTY);
	return (RET_SUCCESS);
}

/*
 * REC_TOTAL -- Return the number of recno entries below a page.
 *
 * Parameters:
 *	h:	page
 *
 * Returns:
 *	The number of recno entries below a page.
 *
 * XXX
 * These values could be set by the bt_psplit routine.  The problem is that the
 * entry has to be popped off of the stack etc. or the values have to be passed
 * all the way back to bt_split/bt_rroot and it's not very clean.
 */
static recno_t
ref_rec_total(PAGE *h)
{
	recno_t recs;
	indx_t nxt, top;

	for (recs = 0, nxt = 0, top = NEXTINDEX(h); nxt < top; ++nxt)
		recs += GETRINTERNAL(h, nxt)->nrecs;
	return (recs);
}
