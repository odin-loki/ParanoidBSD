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
 * Reference oracle for batch b0155s4:
 *	hbsd/src/lib/libc/db/btree/bt_delete.c
 *
 * Every function of the original translation unit is present, renamed with
 * a ref_ prefix; the bodies are unmodified (calls to the renamed siblings
 * follow the rename, nothing else changed).  The "static" storage class was
 * dropped so that the differential harness can reach each function.
 *
 * The declarations that bt_delete.c obtained from <db.h>, "btree.h",
 * <mpool.h> and "extern.h" are reproduced verbatim below, because those
 * headers are not part of this batch.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* ------------------------------------------------------------------ */
/* <sys/types.h> spellings used by the original translation unit.      */
/* ------------------------------------------------------------------ */

typedef unsigned int	u_int;
typedef unsigned char	u_char;
typedef uint8_t		u_int8_t;
typedef uint32_t	u_int32_t;
typedef char *		caddr_t;

/* ------------------------------------------------------------------ */
/* <db.h>                                                             */
/* ------------------------------------------------------------------ */

#define	RET_ERROR	-1		/* Return values. */
#define	RET_SUCCESS	 0
#define	RET_SPECIAL	 1

#define	MAX_PAGE_NUMBER	0xffffffff	/* >= # of pages in a file */
typedef uint32_t	pgno_t;
#define	MAX_PAGE_OFFSET	65535		/* >= # of bytes in a page */
typedef uint16_t	indx_t;
#define	MAX_REC_NUMBER	0xffffffff	/* >= # of records in a tree */
typedef uint32_t	recno_t;

/* Key/data structure -- a Data-Base Thang. */
typedef struct {
	void	*data;			/* data */
	size_t	 size;			/* data length */
} DBT;

/* Routine flags. */
#define	R_CURSOR	1		/* del, put, seq */

typedef enum { DB_BTREE, DB_HASH, DB_RECNO } DBTYPE;

/* Access method description structure. */
typedef struct __db {
	DBTYPE type;			/* Underlying db type. */
	int (*close)(struct __db *);
	int (*del)(const struct __db *, const DBT *, unsigned int);
	int (*get)(const struct __db *, const DBT *, DBT *, unsigned int);
	int (*put)(const struct __db *, DBT *, const DBT *, unsigned int);
	int (*seq)(const struct __db *, DBT *, DBT *, unsigned int);
	int (*sync)(const struct __db *, unsigned int);
	void *internal;			/* Access method private. */
	int (*fd)(const struct __db *);
} DB;

/* ------------------------------------------------------------------ */
/* <mpool.h> -- only the cookie type is needed here.                   */
/* ------------------------------------------------------------------ */

typedef struct MPOOL MPOOL;

#define	MPOOL_DIRTY	0x01		/* Page needs to be written. */

/* ------------------------------------------------------------------ */
/* "btree.h"                                                          */
/* ------------------------------------------------------------------ */

/* Macros to set/clear/test flags. */
#define	F_SET(p, f)	(p)->flags |= (f)
#define	F_CLR(p, f)	(p)->flags &= ~(f)
#define	F_ISSET(p, f)	((p)->flags & (f))

#define	DEFMINKEYPAGE	(2)		/* Minimum keys per page */
#define	MINCACHE	(5)		/* Minimum cached pages */
#define	MINPSIZE	(512)		/* Minimum page size */

#define	P_INVALID	 0		/* Invalid tree page number. */
#define	P_META		 0		/* Tree metadata page number. */
#define	P_ROOT		 1		/* Tree root page number. */

typedef struct _page {
	pgno_t	pgno;			/* this page's page number */
	pgno_t	prevpg;			/* left sibling */
	pgno_t	nextpg;			/* right sibling */

#define	P_BINTERNAL	0x01		/* btree internal page */
#define	P_BLEAF		0x02		/* leaf page */
#define	P_OVERFLOW	0x04		/* overflow page */
#define	P_RINTERNAL	0x08		/* recno internal page */
#define	P_RLEAF		0x10		/* leaf page */
#define P_TYPE		0x1f		/* type mask */
#define	P_PRESERVE	0x20		/* never delete this chain of pages */
	u_int32_t flags;

	indx_t	lower;			/* lower bound of free space on page */
	indx_t	upper;			/* upper bound of free space on page */
	indx_t	linp[1];		/* indx_t-aligned VAR. LENGTH DATA */
} PAGE;

/* First and next index. */
#define	BTDATAOFF							\
	(sizeof(pgno_t) + sizeof(pgno_t) + sizeof(pgno_t) +		\
	    sizeof(u_int32_t) + sizeof(indx_t) + sizeof(indx_t))
#define	NEXTINDEX(p)	(((p)->lower - BTDATAOFF) / sizeof(indx_t))

#define	LALIGN(n)	(((n) + sizeof(pgno_t) - 1) & ~(sizeof(pgno_t) - 1))
#define	NOVFLSIZE	(sizeof(pgno_t) + sizeof(u_int32_t))

typedef struct _binternal {
	u_int32_t ksize;		/* key size */
	pgno_t	pgno;			/* page number stored on */
#define	P_BIGDATA	0x01		/* overflow data */
#define	P_BIGKEY	0x02		/* overflow key */
	u_char	flags;
	char	bytes[1];		/* data */
} BINTERNAL;

/* Get the page's BINTERNAL structure at index indx. */
#define	GETBINTERNAL(pg, indx)						\
	((BINTERNAL *)((char *)(pg) + (pg)->linp[indx]))

/* Get the number of bytes in the entry. */
#define NBINTERNAL(len)							\
	LALIGN(sizeof(u_int32_t) + sizeof(pgno_t) + sizeof(u_char) + (len))

/* For the btree leaf pages, the item is a key and data pair. */
typedef struct _bleaf {
	u_int32_t	ksize;		/* size of key */
	u_int32_t	dsize;		/* size of data */
	u_char	flags;			/* P_BIGDATA, P_BIGKEY */
	char	bytes[1];		/* data */
} BLEAF;

/* Get the page's BLEAF structure at index indx. */
#define	GETBLEAF(pg, indx)						\
	((BLEAF *)((char *)(pg) + (pg)->linp[indx]))

/* Get the number of bytes in the entry. */
#define NBLEAF(p)	NBLEAFDBT((p)->ksize, (p)->dsize)

/* Get the number of bytes in the user's key/data pair. */
#define NBLEAFDBT(ksize, dsize)						\
	LALIGN(sizeof(u_int32_t) + sizeof(u_int32_t) + sizeof(u_char) +	\
	    (ksize) + (dsize))

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

#define	CURS_ACQUIRE	0x01		/*  B: Cursor needs to be reacquired. */
#define	CURS_AFTER	0x02		/*  B: Unreturned cursor after key. */
#define	CURS_BEFORE	0x04		/*  B: Unreturned cursor before key. */
#define	CURS_INIT	0x08		/* RB: Cursor initialized. */
	u_int8_t flags;
} CURSOR;

/* The in-memory btree/recno data structure. */
typedef struct _btree {
	MPOOL	 *bt_mp;		/* memory pool cookie */

	DB	 *bt_dbp;		/* pointer to enclosing DB */

	EPG	  bt_cur;		/* current (pinned) page */
	PAGE	 *bt_pinned;		/* page pinned across calls */

	CURSOR	  bt_cursor;		/* cursor */

#define	BT_PUSH(t, p, i) {						\
	t->bt_sp->pgno = p;						\
	t->bt_sp->index = i;						\
	++t->bt_sp;							\
}
#define	BT_POP(t)	(t->bt_sp == t->bt_stack ? NULL : --t->bt_sp)
#define	BT_CLR(t)	(t->bt_sp = t->bt_stack)
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
	enum { NOT, BACK, FORWARD } bt_order;
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

#define	B_INMEM		0x00001		/* in-memory tree */
#define	B_METADIRTY	0x00002		/* need to write metadata */
#define	B_MODIFIED	0x00004		/* tree modified */
#define	B_NEEDSWAP	0x00008		/* if byte order requires swapping */
#define	B_RDONLY	0x00010		/* read-only tree */

#define	B_NODUPS	0x00020		/* no duplicate keys permitted */
#define	R_RECNO		0x00080		/* record oriented tree */

#define	R_CLOSEFP	0x00040		/* opened a file pointer */
#define	R_EOF		0x00100		/* end of input file reached. */
#define	R_FIXLEN	0x00200		/* fixed length records */
#define	R_MEMMAPPED	0x00400		/* memory mapped file. */
#define	R_INMEM		0x00800		/* in-memory file */
#define	R_MODIFIED	0x01000		/* modified file */
#define	R_RDONLY	0x02000		/* read-only file */

#define	B_DB_LOCK	0x04000		/* DB_LOCK specified. */
#define	B_DB_SHMEM	0x08000		/* DB_SHMEM specified. */
#define	B_DB_TXN	0x10000		/* DB_TXN specified. */
	u_int32_t flags;
} BTREE;

/* ------------------------------------------------------------------ */
/* <mpool.h> and "extern.h" prototypes actually referenced.            */
/* ------------------------------------------------------------------ */

void	*mpool_get(MPOOL *, pgno_t, u_int);
int	 mpool_put(MPOOL *, void *, u_int);

int	 __bt_cmp(BTREE *, const DBT *, EPG *);
int	 __bt_free(BTREE *, PAGE *);
int	 __bt_ret(BTREE *, EPG *, DBT *, DBT *, DBT *, DBT *, int);
EPG	*__bt_search(BTREE *, const DBT *, int *);
int	 __ovfl_delete(BTREE *, void *);

/* ================================================================== */
/* bt_delete.c                                                        */
/* ================================================================== */

int ref___bt_bdelete(BTREE *, const DBT *);
int ref___bt_curdel(BTREE *, const DBT *, PAGE *, u_int);
int ref___bt_pdelete(BTREE *, PAGE *);
int ref___bt_relink(BTREE *, PAGE *);
int ref___bt_stkacq(BTREE *, PAGE **, CURSOR *);
int ref___bt_dleaf(BTREE *, const DBT *, PAGE *, u_int);
int ref___bt_delete(const DB *, const DBT *, u_int);

/*
 * __bt_delete
 *	Delete the item(s) referenced by a key.
 *
 * Return RET_SPECIAL if the key is not found.
 */
int
ref___bt_delete(const DB *dbp, const DBT *key, u_int flags)
{
	BTREE *t;
	CURSOR *c;
	PAGE *h;
	int status;

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
		status = ref___bt_bdelete(t, key);
		break;
	case R_CURSOR:
		/*
		 * If flags is R_CURSOR, delete the cursor.  Must already
		 * have started a scan and not have already deleted it.
		 */
		c = &t->bt_cursor;
		if (F_ISSET(c, CURS_INIT)) {
			if (F_ISSET(c, CURS_ACQUIRE | CURS_AFTER | CURS_BEFORE))
				return (RET_SPECIAL);
			if ((h = mpool_get(t->bt_mp, c->pg.pgno, 0)) == NULL)
				return (RET_ERROR);

			/*
			 * If the page is about to be emptied, we'll need to
			 * delete it, which means we have to acquire a stack.
			 */
			if (NEXTINDEX(h) == 1)
				if (ref___bt_stkacq(t, &h, &t->bt_cursor))
					return (RET_ERROR);

			status = ref___bt_dleaf(t, NULL, h, c->pg.index);

			if (NEXTINDEX(h) == 0 && status == RET_SUCCESS) {
				if (ref___bt_pdelete(t, h))
					return (RET_ERROR);
			} else
				mpool_put(t->bt_mp,
				    h, status == RET_SUCCESS ? MPOOL_DIRTY : 0);
			break;
		}
		/* FALLTHROUGH */
	default:
		errno = EINVAL;
		return (RET_ERROR);
	}
	if (status == RET_SUCCESS)
		F_SET(t, B_MODIFIED);
	return (status);
}

/*
 * __bt_stkacq --
 *	Acquire a stack so we can delete a cursor entry.
 *
 * Parameters:
 *	  t:	tree
 *	 hp:	pointer to current, pinned PAGE pointer
 *	  c:	pointer to the cursor
 *
 * Returns:
 *	0 on success, 1 on failure
 */
int
ref___bt_stkacq(BTREE *t, PAGE **hp, CURSOR *c)
{
	BINTERNAL *bi;
	EPG *e;
	EPGNO *parent;
	PAGE *h;
	indx_t idx;
	pgno_t pgno;
	recno_t nextpg, prevpg;
	int exact, level;

	/*
	 * Find the first occurrence of the key in the tree.  Toss the
	 * currently locked page so we don't hit an already-locked page.
	 */
	h = *hp;
	mpool_put(t->bt_mp, h, 0);
	if ((e = __bt_search(t, &c->key, &exact)) == NULL)
		return (1);
	h = e->page;

	/* See if we got it in one shot. */
	if (h->pgno == c->pg.pgno)
		goto ret;

	/*
	 * Move right, looking for the page.  At each move we have to move
	 * up the stack until we don't have to move to the next page.  If
	 * we have to change pages at an internal level, we have to fix the
	 * stack back up.
	 */
	while (h->pgno != c->pg.pgno) {
		if ((nextpg = h->nextpg) == P_INVALID)
			break;
		mpool_put(t->bt_mp, h, 0);

		/* Move up the stack. */
		for (level = 0; (parent = BT_POP(t)) != NULL; ++level) {
			/* Get the parent page. */
			if ((h = mpool_get(t->bt_mp, parent->pgno, 0)) == NULL)
				return (1);

			/* Move to the next index. */
			if (parent->index != NEXTINDEX(h) - 1) {
				idx = parent->index + 1;
				BT_PUSH(t, h->pgno, idx);
				break;
			}
			mpool_put(t->bt_mp, h, 0);
		}

		/* Restore the stack. */
		while (level--) {
			/* Push the next level down onto the stack. */
			bi = GETBINTERNAL(h, idx);
			pgno = bi->pgno;
			BT_PUSH(t, pgno, 0);

			/* Lose the currently pinned page. */
			mpool_put(t->bt_mp, h, 0);

			/* Get the next level down. */
			if ((h = mpool_get(t->bt_mp, pgno, 0)) == NULL)
				return (1);
			idx = 0;
		}
		mpool_put(t->bt_mp, h, 0);
		if ((h = mpool_get(t->bt_mp, nextpg, 0)) == NULL)
			return (1);
	}

	if (h->pgno == c->pg.pgno)
		goto ret;

	/* Reacquire the original stack. */
	mpool_put(t->bt_mp, h, 0);
	if ((e = __bt_search(t, &c->key, &exact)) == NULL)
		return (1);
	h = e->page;

	/*
	 * Move left, looking for the page.  At each move we have to move
	 * up the stack until we don't have to change pages to move to the
	 * next page.  If we have to change pages at an internal level, we
	 * have to fix the stack back up.
	 */
	while (h->pgno != c->pg.pgno) {
		if ((prevpg = h->prevpg) == P_INVALID)
			break;
		mpool_put(t->bt_mp, h, 0);

		/* Move up the stack. */
		for (level = 0; (parent = BT_POP(t)) != NULL; ++level) {
			/* Get the parent page. */
			if ((h = mpool_get(t->bt_mp, parent->pgno, 0)) == NULL)
				return (1);

			/* Move to the next index. */
			if (parent->index != 0) {
				idx = parent->index - 1;
				BT_PUSH(t, h->pgno, idx);
				break;
			}
			mpool_put(t->bt_mp, h, 0);
		}

		/* Restore the stack. */
		while (level--) {
			/* Push the next level down onto the stack. */
			bi = GETBINTERNAL(h, idx);
			pgno = bi->pgno;

			/* Lose the currently pinned page. */
			mpool_put(t->bt_mp, h, 0);

			/* Get the next level down. */
			if ((h = mpool_get(t->bt_mp, pgno, 0)) == NULL)
				return (1);

			idx = NEXTINDEX(h) - 1;
			BT_PUSH(t, pgno, idx);
		}
		mpool_put(t->bt_mp, h, 0);
		if ((h = mpool_get(t->bt_mp, prevpg, 0)) == NULL)
			return (1);
	}


ret:	mpool_put(t->bt_mp, h, 0);
	return ((*hp = mpool_get(t->bt_mp, c->pg.pgno, 0)) == NULL);
}

/*
 * __bt_bdelete --
 *	Delete all key/data pairs matching the specified key.
 *
 * Parameters:
 *	  t:	tree
 *	key:	key to delete
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS and RET_SPECIAL if the key not found.
 */
int
ref___bt_bdelete(BTREE *t, const DBT *key)
{
	EPG *e;
	PAGE *h;
	int deleted, exact, redo;

	deleted = 0;

	/* Find any matching record; __bt_search pins the page. */
loop:	if ((e = __bt_search(t, key, &exact)) == NULL)
		return (deleted ? RET_SUCCESS : RET_ERROR);
	if (!exact) {
		mpool_put(t->bt_mp, e->page, 0);
		return (deleted ? RET_SUCCESS : RET_SPECIAL);
	}

	/*
	 * Delete forward, then delete backward, from the found key.  If
	 * there are duplicates and we reach either side of the page, do
	 * the key search again, so that we get them all.
	 */
	redo = 0;
	h = e->page;
	do {
		if (ref___bt_dleaf(t, key, h, e->index)) {
			mpool_put(t->bt_mp, h, 0);
			return (RET_ERROR);
		}
		if (F_ISSET(t, B_NODUPS)) {
			if (NEXTINDEX(h) == 0) {
				if (ref___bt_pdelete(t, h))
					return (RET_ERROR);
			} else
				mpool_put(t->bt_mp, h, MPOOL_DIRTY);
			return (RET_SUCCESS);
		}
		deleted = 1;
	} while (e->index < NEXTINDEX(h) && __bt_cmp(t, key, e) == 0);

	/* Check for right-hand edge of the page. */
	if (e->index == NEXTINDEX(h))
		redo = 1;

	/* Delete from the key to the beginning of the page. */
	while (e->index-- > 0) {
		if (__bt_cmp(t, key, e) != 0)
			break;
		if (ref___bt_dleaf(t, key, h, e->index) == RET_ERROR) {
			mpool_put(t->bt_mp, h, 0);
			return (RET_ERROR);
		}
		if (e->index == 0)
			redo = 1;
	}

	/* Check for an empty page. */
	if (NEXTINDEX(h) == 0) {
		if (ref___bt_pdelete(t, h))
			return (RET_ERROR);
		goto loop;
	}

	/* Put the page. */
	mpool_put(t->bt_mp, h, MPOOL_DIRTY);

	if (redo)
		goto loop;
	return (RET_SUCCESS);
}

/*
 * __bt_pdelete --
 *	Delete a single page from the tree.
 *
 * Parameters:
 *	t:	tree
 *	h:	leaf page
 *
 * Returns:
 *	RET_SUCCESS, RET_ERROR.
 *
 * Side-effects:
 *	mpool_put's the page
 */
int
ref___bt_pdelete(BTREE *t, PAGE *h)
{
	BINTERNAL *bi;
	PAGE *pg;
	EPGNO *parent;
	indx_t cnt, idx, *ip, offset;
	u_int32_t nksize;
	char *from;

	/*
	 * Walk the parent page stack -- a LIFO stack of the pages that were
	 * traversed when we searched for the page where the delete occurred.
	 * Each stack entry is a page number and a page index offset.  The
	 * offset is for the page traversed on the search.  We've just deleted
	 * a page, so we have to delete the key from the parent page.
	 *
	 * If the delete from the parent page makes it empty, this process may
	 * continue all the way up the tree.  We stop if we reach the root page
	 * (which is never deleted, it's just not worth the effort) or if the
	 * delete does not empty the page.
	 */
	while ((parent = BT_POP(t)) != NULL) {
		/* Get the parent page. */
		if ((pg = mpool_get(t->bt_mp, parent->pgno, 0)) == NULL)
			return (RET_ERROR);

		idx = parent->index;
		bi = GETBINTERNAL(pg, idx);

		/* Free any overflow pages. */
		if (bi->flags & P_BIGKEY &&
		    __ovfl_delete(t, bi->bytes) == RET_ERROR) {
			mpool_put(t->bt_mp, pg, 0);
			return (RET_ERROR);
		}

		/*
		 * Free the parent if it has only the one key and it's not the
		 * root page. If it's the rootpage, turn it back into an empty
		 * leaf page.
		 */
		if (NEXTINDEX(pg) == 1) {
			if (pg->pgno == P_ROOT) {
				pg->lower = BTDATAOFF;
				pg->upper = t->bt_psize;
				pg->flags = P_BLEAF;
			} else {
				if (ref___bt_relink(t, pg) || __bt_free(t, pg))
					return (RET_ERROR);
				continue;
			}
		} else {
			/* Pack remaining key items at the end of the page. */
			nksize = NBINTERNAL(bi->ksize);
			from = (char *)pg + pg->upper;
			memmove(from + nksize, from, (char *)bi - from);
			pg->upper += nksize;

			/* Adjust indices' offsets, shift the indices down. */
			offset = pg->linp[idx];
			for (cnt = idx, ip = &pg->linp[0]; cnt--; ++ip)
				if (ip[0] < offset)
					ip[0] += nksize;
			for (cnt = NEXTINDEX(pg) - idx; --cnt; ++ip)
				ip[0] = ip[1] < offset ? ip[1] + nksize : ip[1];
			pg->lower -= sizeof(indx_t);
		}

		mpool_put(t->bt_mp, pg, MPOOL_DIRTY);
		break;
	}

	/* Free the leaf page, as long as it wasn't the root. */
	if (h->pgno == P_ROOT) {
		mpool_put(t->bt_mp, h, MPOOL_DIRTY);
		return (RET_SUCCESS);
	}
	return (ref___bt_relink(t, h) || __bt_free(t, h));
}

/*
 * __bt_dleaf --
 *	Delete a single record from a leaf page.
 *
 * Parameters:
 *	t:	tree
 *    key:	referenced key
 *	h:	page
 *	idx:	index on page to delete
 *
 * Returns:
 *	RET_SUCCESS, RET_ERROR.
 */
int
ref___bt_dleaf(BTREE *t, const DBT *key, PAGE *h, u_int idx)
{
	BLEAF *bl;
	indx_t cnt, *ip, offset;
	u_int32_t nbytes;
	void *to;
	char *from;

	/* If this record is referenced by the cursor, delete the cursor. */
	if (F_ISSET(&t->bt_cursor, CURS_INIT) &&
	    !F_ISSET(&t->bt_cursor, CURS_ACQUIRE) &&
	    t->bt_cursor.pg.pgno == h->pgno && t->bt_cursor.pg.index == idx &&
	    ref___bt_curdel(t, key, h, idx))
		return (RET_ERROR);

	/* If the entry uses overflow pages, make them available for reuse. */
	to = bl = GETBLEAF(h, idx);
	if (bl->flags & P_BIGKEY && __ovfl_delete(t, bl->bytes) == RET_ERROR)
		return (RET_ERROR);
	if (bl->flags & P_BIGDATA &&
	    __ovfl_delete(t, bl->bytes + bl->ksize) == RET_ERROR)
		return (RET_ERROR);

	/* Pack the remaining key/data items at the end of the page. */
	nbytes = NBLEAF(bl);
	from = (char *)h + h->upper;
	memmove(from + nbytes, from, (char *)to - from);
	h->upper += nbytes;

	/* Adjust the indices' offsets, shift the indices down. */
	offset = h->linp[idx];
	for (cnt = idx, ip = &h->linp[0]; cnt--; ++ip)
		if (ip[0] < offset)
			ip[0] += nbytes;
	for (cnt = NEXTINDEX(h) - idx; --cnt; ++ip)
		ip[0] = ip[1] < offset ? ip[1] + nbytes : ip[1];
	h->lower -= sizeof(indx_t);

	/* If the cursor is on this page, adjust it as necessary. */
	if (F_ISSET(&t->bt_cursor, CURS_INIT) &&
	    !F_ISSET(&t->bt_cursor, CURS_ACQUIRE) &&
	    t->bt_cursor.pg.pgno == h->pgno && t->bt_cursor.pg.index > idx)
		--t->bt_cursor.pg.index;

	return (RET_SUCCESS);
}

/*
 * __bt_curdel --
 *	Delete the cursor.
 *
 * Parameters:
 *	t:	tree
 *    key:	referenced key (or NULL)
 *	h:	page
 *    idx:	index on page to delete
 *
 * Returns:
 *	RET_SUCCESS, RET_ERROR.
 */
int
ref___bt_curdel(BTREE *t, const DBT *key, PAGE *h, u_int idx)
{
	CURSOR *c;
	EPG e;
	PAGE *pg;
	int curcopy, status;

	/*
	 * If there are duplicates, move forward or backward to one.
	 * Otherwise, copy the key into the cursor area.
	 */
	c = &t->bt_cursor;
	F_CLR(c, CURS_AFTER | CURS_BEFORE | CURS_ACQUIRE);

	curcopy = 0;
	if (!F_ISSET(t, B_NODUPS)) {
		/*
		 * We're going to have to do comparisons.  If we weren't
		 * provided a copy of the key, i.e. the user is deleting
		 * the current cursor position, get one.
		 */
		if (key == NULL) {
			e.page = h;
			e.index = idx;
			if ((status = __bt_ret(t, &e,
			    &c->key, &c->key, NULL, NULL, 1)) != RET_SUCCESS)
				return (status);
			curcopy = 1;
			key = &c->key;
		}
		/* Check previous key, if not at the beginning of the page. */
		if (idx > 0) {
			e.page = h;
			e.index = idx - 1;
			if (__bt_cmp(t, key, &e) == 0) {
				F_SET(c, CURS_BEFORE);
				goto dup2;
			}
		}
		/* Check next key, if not at the end of the page. */
		if (idx < NEXTINDEX(h) - 1) {
			e.page = h;
			e.index = idx + 1;
			if (__bt_cmp(t, key, &e) == 0) {
				F_SET(c, CURS_AFTER);
				goto dup2;
			}
		}
		/* Check previous key if at the beginning of the page. */
		if (idx == 0 && h->prevpg != P_INVALID) {
			if ((pg = mpool_get(t->bt_mp, h->prevpg, 0)) == NULL)
				return (RET_ERROR);
			e.page = pg;
			e.index = NEXTINDEX(pg) - 1;
			if (__bt_cmp(t, key, &e) == 0) {
				F_SET(c, CURS_BEFORE);
				goto dup1;
			}
			mpool_put(t->bt_mp, pg, 0);
		}
		/* Check next key if at the end of the page. */
		if (idx == NEXTINDEX(h) - 1 && h->nextpg != P_INVALID) {
			if ((pg = mpool_get(t->bt_mp, h->nextpg, 0)) == NULL)
				return (RET_ERROR);
			e.page = pg;
			e.index = 0;
			if (__bt_cmp(t, key, &e) == 0) {
				F_SET(c, CURS_AFTER);
dup1:				mpool_put(t->bt_mp, pg, 0);
dup2:				c->pg.pgno = e.page->pgno;
				c->pg.index = e.index;
				return (RET_SUCCESS);
			}
			mpool_put(t->bt_mp, pg, 0);
		}
	}
	e.page = h;
	e.index = idx;
	if (curcopy || (status =
	    __bt_ret(t, &e, &c->key, &c->key, NULL, NULL, 1)) == RET_SUCCESS) {
		F_SET(c, CURS_ACQUIRE);
		return (RET_SUCCESS);
	}
	return (status);
}

/*
 * __bt_relink --
 *	Link around a deleted page.
 *
 * Parameters:
 *	t:	tree
 *	h:	page to be deleted
 */
int
ref___bt_relink(BTREE *t, PAGE *h)
{
	PAGE *pg;

	if (h->nextpg != P_INVALID) {
		if ((pg = mpool_get(t->bt_mp, h->nextpg, 0)) == NULL)
			return (RET_ERROR);
		pg->prevpg = h->prevpg;
		mpool_put(t->bt_mp, pg, MPOOL_DIRTY);
	}
	if (h->prevpg != P_INVALID) {
		if ((pg = mpool_get(t->bt_mp, h->prevpg, 0)) == NULL)
			return (RET_ERROR);
		pg->nextpg = h->nextpg;
		mpool_put(t->bt_mp, pg, MPOOL_DIRTY);
	}
	return (0);
}
