/*
 * b0155s3 oracle: original HardenedBSD C source for
 *
 *	lib/libc/db/btree/bt_seq.c
 *
 * concatenated with every function renamed with a ref_ prefix.  Mock
 * implementations of external dependencies are included so the harness can
 * drive both the oracle and the C++23 port differentially.  Function bodies
 * from bt_seq.c are otherwise UNMODIFIED.
 */

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#ifndef EFTYPE
#define EFTYPE 79
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif

#define RET_ERROR	-1
#define RET_SUCCESS	 0
#define RET_SPECIAL	 1

#define	MAX_PAGE_OFFSET	65535

typedef uint32_t	pgno_t;
typedef uint16_t	indx_t;
typedef uint32_t	recno_t;
typedef unsigned int	u_int;
typedef unsigned char	u_char;
typedef uint32_t	u_int32_t;
typedef uint16_t	u_int16_t;
typedef uint8_t		u_int8_t;
typedef char		*caddr_t;

typedef enum { DB_BTREE, DB_HASH, DB_RECNO } DBTYPE;

typedef struct {
	void	*data;
	size_t	 size;
} DBT;

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

#define	R_CURSOR	1
#define	R_FIRST		3
#define	R_LAST		6
#define	R_NEXT		7
#define	R_PREV		9

#define	F_SET(p, f)	(p)->flags |= (f)
#define	F_CLR(p, f)	(p)->flags &= ~(f)
#define	F_ISSET(p, f)	((p)->flags & (f))

#define	MPOOL_DIRTY	0x01

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

typedef struct _bleaf {
	u_int32_t	ksize;
	u_int32_t	dsize;
	u_char	flags;
	char	bytes[1];
} BLEAF;

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

typedef struct _btree {
	MPOOL	 *bt_mp;
	DB	 *bt_dbp;
	EPG	  bt_cur;
	PAGE	 *bt_pinned;
	CURSOR	  bt_cursor;
	EPGNO	  bt_stack[50];
	EPGNO	 *bt_sp;
	DBT	  bt_rkey;
	DBT	  bt_rdata;
	int	  bt_fd;
	pgno_t	  bt_free;
	u_int32_t bt_psize;
	indx_t	  bt_ovflsize;
	int	  bt_lorder;
	int	  bt_order;
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
#define	B_NODUPS	0x00020
#define	B_DB_LOCK	0x04000
	u_int32_t flags;
} BTREE;

#define MAX_POOL 64

typedef struct {
	unsigned get_calls, put_calls;
	unsigned search_calls, cmp_calls, ret_calls;
	int get_force_null, search_force_null;
	int search_exact, cmp_ret, ret_status;
	pgno_t get_last_pgno;
	unsigned get_last_flags, last_put_flags;
	void *last_put_page;
	int nreg;
	pgno_t reg_pgno[MAX_POOL];
	void *reg_page[MAX_POOL];
	EPG search_epg;
} test_mock_state;

test_mock_state test_mock;

void test_mock_reset(void)
{
	memset(&test_mock, 0, sizeof(test_mock));
	test_mock.ret_status = RET_SUCCESS;
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

EPG *__bt_search(BTREE *t, const DBT *key, int *exact)
{
	(void)t; (void)key;
	test_mock.search_calls++;
	if (test_mock.search_force_null)
		return NULL;
	*exact = test_mock.search_exact;
	return &test_mock.search_epg;
}

int __bt_cmp(BTREE *t, const DBT *key, EPG *ep)
{
	(void)t; (void)key; (void)ep;
	test_mock.cmp_calls++;
	return test_mock.cmp_ret;
}

int __bt_ret(BTREE *t, EPG *e, DBT *key, DBT *rkey, DBT *data, DBT *rdata, int copy)
{
	(void)t; (void)e; (void)key; (void)rkey; (void)rdata; (void)copy;
	test_mock.ret_calls++;
	if (data != NULL) {
		static char dbuf[256];
		data->data = dbuf;
		data->size = 4;
	}
	return test_mock.ret_status;
}

int ref___bt_seqset(BTREE *, EPG *, DBT *, int);
int ref___bt_seqadv(BTREE *, EPG *, int);
int ref___bt_first(BTREE *, const DBT *, EPG *, int *);
void ref___bt_setcur(BTREE *, pgno_t, u_int);

/* --- bt_seq.c --- */

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
 * Sequential scan support.
 *
 * The tree can be scanned sequentially, starting from either end of the
 * tree or from any specific key.  A scan request before any scanning is
 * done is initialized as starting from the least node.
 */

/*
 * __bt_seq --
 *	Btree sequential scan interface.
 *
 * Parameters:
 *	dbp:	pointer to access method
 *	key:	key for positioning and return value
 *	data:	data return value
 *	flags:	R_CURSOR, R_FIRST, R_LAST, R_NEXT, R_PREV.
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS or RET_SPECIAL if there's no next key.
 */
int
ref___bt_seq(const DB *dbp, DBT *key, DBT *data, u_int flags)
{
	BTREE *t;
	EPG e;
	int status;

	t = dbp->internal;

	/* Toss any page pinned across calls. */
	if (t->bt_pinned != NULL) {
		mpool_put(t->bt_mp, t->bt_pinned, 0);
		t->bt_pinned = NULL;
	}

	/*
	 * If scan uninitialized as yet, or starting at a specific record, set
	 * the scan to a specific key.  Both __bt_seqset and __bt_seqadv pin
	 * the page the cursor references if they're successful.
	 */
	switch (flags) {
	case R_NEXT:
	case R_PREV:
		if (F_ISSET(&t->bt_cursor, CURS_INIT)) {
			status = ref___bt_seqadv(t, &e, flags);
			break;
		}
		/* FALLTHROUGH */
	case R_FIRST:
	case R_LAST:
	case R_CURSOR:
		status = ref___bt_seqset(t, &e, key, flags);
		break;
	default:
		errno = EINVAL;
		return (RET_ERROR);
	}

	if (status == RET_SUCCESS) {
		ref___bt_setcur(t, e.page->pgno, e.index);

		status =
		    __bt_ret(t, &e, key, &t->bt_rkey, data, &t->bt_rdata, 0);

		/*
		 * If the user is doing concurrent access, we copied the
		 * key/data, toss the page.
		 */
		if (F_ISSET(t, B_DB_LOCK))
			mpool_put(t->bt_mp, e.page, 0);
		else
			t->bt_pinned = e.page;
	}
	return (status);
}

/*
 * __bt_seqset --
 *	Set the sequential scan to a specific key.
 *
 * Parameters:
 *	t:	tree
 *	ep:	storage for returned key
 *	key:	key for initial scan position
 *	flags:	R_CURSOR, R_FIRST, R_LAST, R_NEXT, R_PREV
 *
 * Side effects:
 *	Pins the page the cursor references.
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS or RET_SPECIAL if there's no next key.
 */
int
ref___bt_seqset(BTREE *t, EPG *ep, DBT *key, int flags)
{
	PAGE *h;
	pgno_t pg;
	int exact;

	/*
	 * Find the first, last or specific key in the tree and point the
	 * cursor at it.  The cursor may not be moved until a new key has
	 * been found.
	 */
	switch (flags) {
	case R_CURSOR:				/* Keyed scan. */
		/*
		 * Find the first instance of the key or the smallest key
		 * which is greater than or equal to the specified key.
		 */
		if (key->data == NULL || key->size == 0) {
			errno = EINVAL;
			return (RET_ERROR);
		}
		return (ref___bt_first(t, key, ep, &exact));
	case R_FIRST:				/* First record. */
	case R_NEXT:
		/* Walk down the left-hand side of the tree. */
		for (pg = P_ROOT;;) {
			if ((h = mpool_get(t->bt_mp, pg, 0)) == NULL)
				return (RET_ERROR);

			/* Check for an empty tree. */
			if (NEXTINDEX(h) == 0) {
				mpool_put(t->bt_mp, h, 0);
				return (RET_SPECIAL);
			}

			if (h->flags & (P_BLEAF | P_RLEAF))
				break;
			pg = GETBINTERNAL(h, 0)->pgno;
			mpool_put(t->bt_mp, h, 0);
		}
		ep->page = h;
		ep->index = 0;
		break;
	case R_LAST:				/* Last record. */
	case R_PREV:
		/* Walk down the right-hand side of the tree. */
		for (pg = P_ROOT;;) {
			if ((h = mpool_get(t->bt_mp, pg, 0)) == NULL)
				return (RET_ERROR);

			/* Check for an empty tree. */
			if (NEXTINDEX(h) == 0) {
				mpool_put(t->bt_mp, h, 0);
				return (RET_SPECIAL);
			}

			if (h->flags & (P_BLEAF | P_RLEAF))
				break;
			pg = GETBINTERNAL(h, NEXTINDEX(h) - 1)->pgno;
			mpool_put(t->bt_mp, h, 0);
		}

		ep->page = h;
		ep->index = NEXTINDEX(h) - 1;
		break;
	}
	return (RET_SUCCESS);
}

/*
 * __bt_seqadvance --
 *	Advance the sequential scan.
 *
 * Parameters:
 *	t:	tree
 *	flags:	R_NEXT, R_PREV
 *
 * Side effects:
 *	Pins the page the new key/data record is on.
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS or RET_SPECIAL if there's no next key.
 */
int
ref___bt_seqadv(BTREE *t, EPG *ep, int flags)
{
	CURSOR *c;
	PAGE *h;
	indx_t idx;
	pgno_t pg;
	int exact;

	/*
	 * There are a couple of states that we can be in.  The cursor has
	 * been initialized by the time we get here, but that's all we know.
	 */
	c = &t->bt_cursor;

	/*
	 * The cursor was deleted where there weren't any duplicate records,
	 * so the key was saved.  Find out where that key would go in the
	 * current tree.  It doesn't matter if the returned key is an exact
	 * match or not -- if it's an exact match, the record was added after
	 * the delete so we can just return it.  If not, as long as there's
	 * a record there, return it.
	 */
	if (F_ISSET(c, CURS_ACQUIRE))
		return (ref___bt_first(t, &c->key, ep, &exact));

	/* Get the page referenced by the cursor. */
	if ((h = mpool_get(t->bt_mp, c->pg.pgno, 0)) == NULL)
		return (RET_ERROR);

	/*
	 * Find the next/previous record in the tree and point the cursor at
	 * it.  The cursor may not be moved until a new key has been found.
	 */
	switch (flags) {
	case R_NEXT:			/* Next record. */
		/*
		 * The cursor was deleted in duplicate records, and moved
		 * forward to a record that has yet to be returned.  Clear
		 * that flag, and return the record.
		 */
		if (F_ISSET(c, CURS_AFTER))
			goto usecurrent;
		idx = c->pg.index;
		if (++idx == NEXTINDEX(h)) {
			pg = h->nextpg;
			mpool_put(t->bt_mp, h, 0);
			if (pg == P_INVALID)
				return (RET_SPECIAL);
			if ((h = mpool_get(t->bt_mp, pg, 0)) == NULL)
				return (RET_ERROR);
			idx = 0;
		}
		break;
	case R_PREV:			/* Previous record. */
		/*
		 * The cursor was deleted in duplicate records, and moved
		 * backward to a record that has yet to be returned.  Clear
		 * that flag, and return the record.
		 */
		if (F_ISSET(c, CURS_BEFORE)) {
usecurrent:		F_CLR(c, CURS_AFTER | CURS_BEFORE);
			ep->page = h;
			ep->index = c->pg.index;
			return (RET_SUCCESS);
		}
		idx = c->pg.index;
		if (idx == 0) {
			pg = h->prevpg;
			mpool_put(t->bt_mp, h, 0);
			if (pg == P_INVALID)
				return (RET_SPECIAL);
			if ((h = mpool_get(t->bt_mp, pg, 0)) == NULL)
				return (RET_ERROR);
			idx = NEXTINDEX(h) - 1;
		} else
			--idx;
		break;
	}

	ep->page = h;
	ep->index = idx;
	return (RET_SUCCESS);
}

/*
 * __bt_first --
 *	Find the first entry.
 *
 * Parameters:
 *	t:	the tree
 *    key:	the key
 *  erval:	return EPG
 * exactp:	pointer to exact match flag
 *
 * Returns:
 *	The first entry in the tree greater than or equal to key,
 *	or RET_SPECIAL if no such key exists.
 */
int
ref___bt_first(BTREE *t, const DBT *key, EPG *erval, int *exactp)
{
	PAGE *h;
	EPG *ep, save;
	pgno_t pg;

	/*
	 * Find any matching record; __bt_search pins the page.
	 *
	 * If it's an exact match and duplicates are possible, walk backwards
	 * in the tree until we find the first one.  Otherwise, make sure it's
	 * a valid key (__bt_search may return an index just past the end of a
	 * page) and return it.
	 */
	if ((ep = __bt_search(t, key, exactp)) == NULL)
		return (0);
	if (*exactp) {
		if (F_ISSET(t, B_NODUPS)) {
			*erval = *ep;
			return (RET_SUCCESS);
		}

		/*
		 * Walk backwards, as long as the entry matches and there are
		 * keys left in the tree.  Save a copy of each match in case
		 * we go too far.
		 */
		save = *ep;
		h = ep->page;
		do {
			if (save.page->pgno != ep->page->pgno) {
				mpool_put(t->bt_mp, save.page, 0);
				save = *ep;
			} else
				save.index = ep->index;

			/*
			 * Don't unpin the page the last (or original) match
			 * was on, but make sure it's unpinned if an error
			 * occurs.
			 */
			if (ep->index == 0) {
				if (h->prevpg == P_INVALID)
					break;
				if (h->pgno != save.page->pgno)
					mpool_put(t->bt_mp, h, 0);
				if ((h = mpool_get(t->bt_mp,
				    h->prevpg, 0)) == NULL) {
					if (h->pgno == save.page->pgno)
						mpool_put(t->bt_mp,
						    save.page, 0);
					return (RET_ERROR);
				}
				ep->page = h;
				ep->index = NEXTINDEX(h);
			}
			--ep->index;
		} while (__bt_cmp(t, key, ep) == 0);

		/*
		 * Reach here with the last page that was looked at pinned,
		 * which may or may not be the same as the last (or original)
		 * match page.  If it's not useful, release it.
		 */
		if (h->pgno != save.page->pgno)
			mpool_put(t->bt_mp, h, 0);

		*erval = save;
		return (RET_SUCCESS);
	}

	/* If at the end of a page, find the next entry. */
	if (ep->index == NEXTINDEX(ep->page)) {
		h = ep->page;
		pg = h->nextpg;
		mpool_put(t->bt_mp, h, 0);
		if (pg == P_INVALID)
			return (RET_SPECIAL);
		if ((h = mpool_get(t->bt_mp, pg, 0)) == NULL)
			return (RET_ERROR);
		ep->index = 0;
		ep->page = h;
	}
	*erval = *ep;
	return (RET_SUCCESS);
}

/*
 * __bt_setcur --
 *	Set the cursor to an entry in the tree.
 *
 * Parameters:
 *	t:	the tree
 *   pgno:	page number
 *    idx:	page index
 */
void
ref___bt_setcur(BTREE *t, pgno_t pgno, u_int idx)
{
	/* Lose any already deleted key. */
	if (t->bt_cursor.key.data != NULL) {
		free(t->bt_cursor.key.data);
		t->bt_cursor.key.size = 0;
		t->bt_cursor.key.data = NULL;
	}
	F_CLR(&t->bt_cursor, CURS_ACQUIRE | CURS_AFTER | CURS_BEFORE);

	/* Update the cursor. */
	t->bt_cursor.pg.pgno = pgno;
	t->bt_cursor.pg.index = idx;
	F_SET(&t->bt_cursor, CURS_INIT);
}
