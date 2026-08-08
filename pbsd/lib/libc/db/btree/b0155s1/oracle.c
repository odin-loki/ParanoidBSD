/*
 * oracle.c -- reference build of the b0155s1 batch.
 *
 * Sources: hbsd/src/lib/libc/db/btree/bt_put.c
 *
 * Every function of the original source is present here with a ref_ prefix and
 * an otherwise byte-identical body.  bt_put.c consumes <db.h>, btree.h and
 * mpool.h, which are not part of the batch; the declarations it needs are
 * supplied below.  The external btree/mpool entry points bt_put.c calls are
 * declared, not defined -- the harness provides the shared environment that
 * both this oracle and the C++23 port run against.
 */

#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LONG_BIT
#define	LONG_BIT	(sizeof(long) * 8)
#endif

typedef struct MPOOL MPOOL;

typedef u_int32_t	pgno_t;
typedef u_int16_t	indx_t;
typedef u_int32_t	recno_t;

typedef enum __dbtype_e { DB_BTREE, DB_HASH, DB_RECNO } DBTYPE;

typedef struct __dbt_s {
	void	*data;			/* data */
	size_t	 size;			/* data length */
} DBT;

typedef struct __db {
	DBTYPE type;			/* underlying db type */
	int (*close)(struct __db *);
	int (*del)(const struct __db *, const DBT *, u_int);
	int (*get)(const struct __db *, const DBT *, DBT *, u_int);
	int (*put)(const struct __db *, DBT *, const DBT *, u_int);
	int (*seq)(const struct __db *, DBT *, DBT *, u_int);
	int (*sync)(const struct __db *, u_int);
	void *internal;			/* access method private */
	int (*fd)(const struct __db *);
} DB;

typedef struct _page {
	pgno_t	pgno;			/* this page's page number */
	pgno_t	prevpg;			/* left sibling */
	pgno_t	nextpg;			/* right sibling */

	u_int32_t flags;
	indx_t	lower;			/* lower bound of free space on page */
	indx_t	upper;			/* upper bound of free space on page */
	indx_t	linp[1];		/* indx_t-aligned VAR. LENGTH DATA */
} PAGE;

typedef struct _bleaf {
	u_int32_t	ksize;		/* size of key */
	u_int32_t	dsize;		/* size of data */
	u_char	flags;			/* P_BIGDATA, P_BIGKEY */
	char	bytes[1];		/* data */
} BLEAF;

typedef struct _epgno {
	pgno_t	pgno;			/* the page number */
	indx_t	index;			/* the index on the page */
} EPGNO;

typedef struct _epg {
	PAGE	*page;			/* the (pinned) page */
	indx_t	 index;			/* the index on the page */
} EPG;

typedef struct _cursor {
	EPGNO	 pg;			/* B: Saved tree reference. */
	DBT	 key;			/* B: Saved key, or key.data == NULL. */
	recno_t	 rcursor;		/* R: recno cursor (1-based) */

	u_int8_t flags;
} CURSOR;

enum bt_order_e { NOT, BACK, FORWARD };

typedef struct _btree {
	MPOOL	 *bt_mp;		/* memory pool cookie */

	DB	 *bt_dbp;		/* pointer to enclosing DB */

	EPG	  bt_cur;		/* current (pinned) page */
	PAGE	 *bt_pinned;		/* page pinned across calls */

	CURSOR	  bt_cursor;		/* cursor */

	EPGNO	  bt_stack[50];		/* stack of parent pages */
	EPGNO	 *bt_sp;		/* current stack pointer */

	DBT	  bt_rkey;		/* returned key */
	DBT	  bt_rdata;		/* returned data */

	int	  bt_fd;		/* tree file descriptor */

	pgno_t	  bt_free;		/* next free page */
	u_int32_t bt_psize;		/* page size */
	indx_t	  bt_ovflsize;		/* cut-off for key/data overflow */
	int	  bt_lorder;		/* byte order */
					/* sorted order */
	enum bt_order_e bt_order;
	EPGNO	  bt_last;		/* last insert */

					/* B: key comparison function */
	int	(*bt_cmp)(const DBT *, const DBT *);
					/* B: prefix comparison function */
	size_t	(*bt_pfx)(const DBT *, const DBT *);
					/* R: recno input function */
	int	(*bt_irec)(struct _btree *, recno_t);

	FILE	 *bt_rfp;		/* R: record FILE pointer */
	int	  bt_rfd;		/* R: record file descriptor */

	caddr_t	  bt_cmap;		/* R: current point in mapped space */
	caddr_t	  bt_smap;		/* R: start of mapped space */
	caddr_t   bt_emap;		/* R: end of mapped space */
	size_t	  bt_msize;		/* R: size of mapped region. */

	recno_t	  bt_nrecs;		/* R: number of records */
	size_t	  bt_reclen;		/* R: fixed record length */
	u_char	  bt_bval;		/* R: delimiting byte/pad character */

	u_int32_t flags;
} BTREE;

#define	RET_ERROR	-1
#define	RET_SUCCESS	 0
#define	RET_SPECIAL	 1

#define	R_CURSOR	 1
#define	__R_UNUSED	 2
#define	R_FIRST		 3
#define	R_IAFTER	 4
#define	R_IBEFORE	 5
#define	R_LAST		 6
#define	R_NEXT		 7
#define	R_NOOVERWRITE	 8
#define	R_PREV		 9
#define	R_SETCURSOR	10
#define	R_RECNOSYNC	11

#define	P_INVALID	 0xffffffff

#define	DEFMINKEYPAGE	 (2)
#define	MINCACHE	 (5)
#define	MINPSIZE	 (512)

#define	P_BINTERNAL	0x01		/* btree internal page */
#define	P_BLEAF		0x02		/* leaf page */
#define	P_OVERFLOW	0x04		/* overflow page */
#define	P_RINTERNAL	0x08		/* recno internal page */
#define	P_RLEAF		0x10		/* leaf page */
#define	P_TYPE		0x1f		/* type mask */
#define	P_PRESERVE	0x20		/* never delete this chain of pages */

#define	LALIGN(n)	(((n) + sizeof(pgno_t) - 1) & ~(sizeof(pgno_t) - 1))
#define	NOVFLSIZE	(sizeof(pgno_t) + sizeof(u_int32_t))

#define	BTDATAOFF							\
	(sizeof(pgno_t) + sizeof(pgno_t) + sizeof(pgno_t) +		\
	    sizeof(u_int32_t) + sizeof(indx_t) + sizeof(indx_t))
#define	NEXTINDEX(p)	(((p)->lower - BTDATAOFF) / sizeof(indx_t))

#define	P_BIGDATA	0x01		/* overflow data */
#define	P_BIGKEY	0x02		/* overflow key */

#define	NBLEAFDBT(ksize, dsize)						\
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

#define	CURS_ACQUIRE	0x01		/*  B: Cursor intialized. */
#define	CURS_AFTER	0x02		/*  B: Unreturned cursor after. */
#define	CURS_BEFORE	0x04		/*  B: Unreturned cursor before. */
#define	CURS_INIT	0x08		/*  B: Cursor initialized. */

#define	B_INMEM		0x00001		/* in-memory tree */
#define	B_METADIRTY	0x00002		/* need to write metadata */
#define	B_MODIFIED	0x00004		/* tree modified */
#define	B_NEEDSWAP	0x00008		/* if byte order requires swapping */
#define	B_RDONLY	0x00010		/* read-only tree */

#define	B_DB_LOCK	0x00020		/* DB_LOCK specified. */
#define	B_DB_SHMEM	0x00040		/* DB_SHMEM specified. */
#define	B_DB_TXN	0x00080		/* DB_TXN specified. */

#define	R_CLOSEFP	0x00100		/* opened a file pointer */
#define	R_EOF		0x00200		/* end of input file reached. */
#define	R_FIXLEN	0x00400		/* fixed length records */
#define	R_MEMMAPPED	0x00800		/* memory mapped file. */
#define	R_INMEM		0x01000		/* in-memory file */
#define	R_MODIFIED	0x02000		/* modified file */
#define	R_RECNO		0x04000		/* record oriented tree */

#define	B_NODUPS	0x08000		/* no duplicate keys permitted */

#define	F_ISSET(p, f)	((p)->flags & (f))
#define	F_SET(p, f)	((p)->flags |= (f))
#define	F_CLR(p, f)	((p)->flags &= ~(f))

#define	MPOOL_DIRTY	0x01		/* page needs to be written */

extern void	*mpool_get(MPOOL *, pgno_t, u_int);
extern int	 mpool_put(MPOOL *, void *, u_int);
extern int	 __bt_cmp(BTREE *, const DBT *, EPG *);
extern int	 __bt_dleaf(BTREE *, const DBT *, PAGE *, u_int);
extern EPG	*__bt_search(BTREE *, const DBT *, int *);
extern int	 __bt_setcur(BTREE *, pgno_t, u_int);
extern int	 __bt_split(BTREE *, PAGE *, const DBT *, const DBT *, int,
		    size_t, u_int32_t);
extern int	 __ovfl_put(BTREE *, const DBT *, pgno_t *);

/* ------------------------------------------------------------------------- */
/* hbsd/src/lib/libc/db/btree/bt_put.c                                       */
/* ------------------------------------------------------------------------- */

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

EPG *ref_bt_fast(BTREE *, const DBT *, const DBT *, int *);

/*
 * __BT_PUT -- Add a btree item to the tree.
 *
 * Parameters:
 *	dbp:	pointer to access method
 *	key:	key
 *	data:	data
 *	flag:	R_NOOVERWRITE, R_SETCURSOR, R_CURSOR
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS and RET_SPECIAL if the key is already in the
 *	tree and R_NOOVERWRITE specified.
 */
int
ref___bt_put(const DB *dbp, DBT *key, const DBT *data, u_int flags)
{
	BTREE *t;
	DBT tkey, tdata;
	EPG *e;
	PAGE *h;
	indx_t idx, nxtindex;
	pgno_t pg;
	u_int32_t nbytes, tmp;
	int dflags, exact, status;
	char *dest, db[NOVFLSIZE], kb[NOVFLSIZE];

	t = dbp->internal;

	/* Toss any page pinned across calls. */
	if (t->bt_pinned != NULL) {
		mpool_put(t->bt_mp, t->bt_pinned, 0);
		t->bt_pinned = NULL;
	}

	/* Check for change to a read-only tree. */
	if (F_ISSET(t, B_RDONLY)) {
		errno = EPERM;
		return (RET_ERROR);
	}

	switch (flags) {
	case 0:
	case R_NOOVERWRITE:
	case R_SETCURSOR:
		break;
	case R_CURSOR:
		/*
		 * If flags is R_CURSOR, put the cursor.  Must already
		 * have started a scan and not have already deleted it.
		 */
		if (F_ISSET(&t->bt_cursor, CURS_INIT) &&
		    !F_ISSET(&t->bt_cursor,
			CURS_ACQUIRE | CURS_AFTER | CURS_BEFORE))
			break;
		/* FALLTHROUGH */
	default:
		errno = EINVAL;
		return (RET_ERROR);
	}

	/*
	 * If the key/data pair won't fit on a page, store it on overflow
	 * pages.  Only put the key on the overflow page if the pair are
	 * still too big after moving the data to an overflow page.
	 *
	 * XXX
	 * If the insert fails later on, the overflow pages aren't recovered.
	 */
	dflags = 0;
	if (key->size + data->size > t->bt_ovflsize) {
		if (key->size > t->bt_ovflsize) {
storekey:		if (__ovfl_put(t, key, &pg) == RET_ERROR)
				return (RET_ERROR);
			tkey.data = kb;
			tkey.size = NOVFLSIZE;
			memmove(kb, &pg, sizeof(pgno_t));
			tmp = key->size;
			memmove(kb + sizeof(pgno_t),
			    &tmp, sizeof(u_int32_t));
			dflags |= P_BIGKEY;
			key = &tkey;
		}
		if (key->size + data->size > t->bt_ovflsize) {
			if (__ovfl_put(t, data, &pg) == RET_ERROR)
				return (RET_ERROR);
			tdata.data = db;
			tdata.size = NOVFLSIZE;
			memmove(db, &pg, sizeof(pgno_t));
			tmp = data->size;
			memmove(db + sizeof(pgno_t),
			    &tmp, sizeof(u_int32_t));
			dflags |= P_BIGDATA;
			data = &tdata;
		}
		if (key->size + data->size > t->bt_ovflsize)
			goto storekey;
	}

	/* Replace the cursor. */
	if (flags == R_CURSOR) {
		if ((h = mpool_get(t->bt_mp, t->bt_cursor.pg.pgno, 0)) == NULL)
			return (RET_ERROR);
		idx = t->bt_cursor.pg.index;
		goto delete;
	}

	/*
	 * Find the key to delete, or, the location at which to insert.
	 * Bt_fast and __bt_search both pin the returned page.
	 */
	if (t->bt_order == NOT || (e = ref_bt_fast(t, key, data, &exact)) == NULL)
		if ((e = __bt_search(t, key, &exact)) == NULL)
			return (RET_ERROR);
	h = e->page;
	idx = e->index;

	/*
	 * Add the key/data pair to the tree.  If an identical key is already
	 * in the tree, and R_NOOVERWRITE is set, an error is returned.  If
	 * R_NOOVERWRITE is not set, the key is either added (if duplicates are
	 * permitted) or an error is returned.
	 */
	switch (flags) {
	case R_NOOVERWRITE:
		if (!exact)
			break;
		mpool_put(t->bt_mp, h, 0);
		return (RET_SPECIAL);
	default:
		if (!exact || !F_ISSET(t, B_NODUPS))
			break;
		/*
		 * !!!
		 * Note, the delete may empty the page, so we need to put a
		 * new entry into the page immediately.
		 */
delete:		if (__bt_dleaf(t, key, h, idx) == RET_ERROR) {
			mpool_put(t->bt_mp, h, 0);
			return (RET_ERROR);
		}
		break;
	}

	/*
	 * If not enough room, or the user has put a ceiling on the number of
	 * keys permitted in the page, split the page.  The split code will
	 * insert the key and data and unpin the current page.  If inserting
	 * into the offset array, shift the pointers up.
	 */
	nbytes = NBLEAFDBT(key->size, data->size);
	if ((u_int32_t)(h->upper - h->lower) < nbytes + sizeof(indx_t)) {
		if ((status = __bt_split(t, h, key,
		    data, dflags, nbytes, idx)) != RET_SUCCESS)
			return (status);
		goto success;
	}

	if (idx < (nxtindex = NEXTINDEX(h)))
		memmove(h->linp + idx + 1, h->linp + idx,
		    (nxtindex - idx) * sizeof(indx_t));
	h->lower += sizeof(indx_t);

	h->linp[idx] = h->upper -= nbytes;
	dest = (char *)h + h->upper;
	WR_BLEAF(dest, key, data, dflags);

	/* If the cursor is on this page, adjust it as necessary. */
	if (F_ISSET(&t->bt_cursor, CURS_INIT) &&
	    !F_ISSET(&t->bt_cursor, CURS_ACQUIRE) &&
	    t->bt_cursor.pg.pgno == h->pgno && t->bt_cursor.pg.index >= idx)
		++t->bt_cursor.pg.index;

	if (t->bt_order == NOT) {
		if (h->nextpg == P_INVALID) {
			if (idx == NEXTINDEX(h) - 1) {
				t->bt_order = FORWARD;
				t->bt_last.index = idx;
				t->bt_last.pgno = h->pgno;
			}
		} else if (h->prevpg == P_INVALID) {
			if (idx == 0) {
				t->bt_order = BACK;
				t->bt_last.index = 0;
				t->bt_last.pgno = h->pgno;
			}
		}
	}

	mpool_put(t->bt_mp, h, MPOOL_DIRTY);

success:
	if (flags == R_SETCURSOR)
		__bt_setcur(t, e->page->pgno, e->index);

	F_SET(t, B_MODIFIED);
	return (RET_SUCCESS);
}

#ifdef STATISTICS
u_long bt_cache_hit, bt_cache_miss;
#endif

/*
 * BT_FAST -- Do a quick check for sorted data.
 *
 * Parameters:
 *	t:	tree
 *	key:	key to insert
 *
 * Returns:
 *	EPG for new record or NULL if not found.
 */
EPG *
ref_bt_fast(BTREE *t, const DBT *key, const DBT *data, int *exactp)
{
	PAGE *h;
	u_int32_t nbytes;
	int cmp;

	if ((h = mpool_get(t->bt_mp, t->bt_last.pgno, 0)) == NULL) {
		t->bt_order = NOT;
		return (NULL);
	}
	t->bt_cur.page = h;
	t->bt_cur.index = t->bt_last.index;

	/*
	 * If won't fit in this page or have too many keys in this page,
	 * have to search to get split stack.
	 */
	nbytes = NBLEAFDBT(key->size, data->size);
	if ((u_int32_t)(h->upper - h->lower) < nbytes + sizeof(indx_t))
		goto miss;

	if (t->bt_order == FORWARD) {
		if (t->bt_cur.page->nextpg != P_INVALID)
			goto miss;
		if (t->bt_cur.index != NEXTINDEX(h) - 1)
			goto miss;
		if ((cmp = __bt_cmp(t, key, &t->bt_cur)) < 0)
			goto miss;
		t->bt_last.index = cmp ? ++t->bt_cur.index : t->bt_cur.index;
	} else {
		if (t->bt_cur.page->prevpg != P_INVALID)
			goto miss;
		if (t->bt_cur.index != 0)
			goto miss;
		if ((cmp = __bt_cmp(t, key, &t->bt_cur)) > 0)
			goto miss;
		t->bt_last.index = 0;
	}
	*exactp = cmp == 0;
#ifdef STATISTICS
	++bt_cache_hit;
#endif
	return (&t->bt_cur);

miss:
#ifdef STATISTICS
	++bt_cache_miss;
#endif
	t->bt_order = NOT;
	mpool_put(t->bt_mp, h, 0);
	return (NULL);
}
