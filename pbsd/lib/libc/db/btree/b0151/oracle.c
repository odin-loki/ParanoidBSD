/*
 * Oracle for batch b0151: original HardenedBSD btree search/overflow/utils/debug
 * sources with ref_ prefix.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

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
typedef unsigned int	u_int;
typedef unsigned char	u_char;
typedef uint32_t	u_int32_t;
typedef uint16_t	u_int16_t;
typedef uint8_t		u_int8_t;
typedef unsigned long	u_long;
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
#define	MPOOL_IGNOREPIN	0x04

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

#define NBLEAF(p)	NBLEAFDBT((p)->ksize, (p)->dsize)

#define NBLEAFDBT(ksize, dsize)						\
	LALIGN(sizeof(u_int32_t) + sizeof(u_int32_t) + sizeof(u_char) +	\
	    (ksize) + (dsize))

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

#define MAX_POOL 256

u_long bt_cache_hit, bt_cache_miss, bt_pfxsaved, bt_rootsplit;
u_long bt_sortsplit, bt_split;

typedef struct {
	unsigned get_calls;
	unsigned put_calls;
	unsigned new_calls;
	unsigned free_calls;
	int get_force_null;
	int new_force_null;
	int calloc_fail;
	int realloc_fail;
	pgno_t get_last_pgno;
	unsigned get_last_flags;
	void *last_put_page;
	unsigned last_put_flags;
	void *last_free_page;
	pgno_t next_pgno;
	int nreg;
	pgno_t reg_pgno[MAX_POOL];
	void *reg_page[MAX_POOL];
	int new_fail_after;
	int seq_scan;
	pgno_t seq_next;
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

void test_mock_seq_begin(pgno_t start)
{
	test_mock.seq_scan = 1;
	test_mock.seq_next = start;
}

void *mpool_get(MPOOL *mp, pgno_t pgno, unsigned int flags)
{
	int i;
	pgno_t scanpg;

	(void)mp;
	test_mock.get_calls++;
	test_mock.get_last_pgno = pgno;
	test_mock.get_last_flags = flags;
	if (test_mock.get_force_null)
		return (NULL);
	if (test_mock.seq_scan && (flags & MPOOL_IGNOREPIN)) {
		scanpg = test_mock.seq_next++;
		for (i = 0; i < test_mock.nreg; i++)
			if (test_mock.reg_pgno[i] == scanpg)
				return (test_mock.reg_page[i]);
		return (NULL);
	}
	for (i = 0; i < test_mock.nreg; i++)
		if (test_mock.reg_pgno[i] == pgno)
			return (test_mock.reg_page[i]);
	return (NULL);
}

int mpool_put(MPOOL *mp, void *page, unsigned int flags)
{
	(void)mp;
	test_mock.put_calls++;
	test_mock.last_put_page = page;
	test_mock.last_put_flags = flags;
	return (RET_SUCCESS);
}

PAGE *__bt_new(BTREE *t, pgno_t *npg)
{
	PAGE *h;

	test_mock.new_calls++;
	if (test_mock.new_force_null)
		return (NULL);
	if (test_mock.new_fail_after > 0) {
		test_mock.new_fail_after--;
		return (NULL);
	}
	if (test_mock.calloc_fail)
		return (NULL);
	h = (PAGE *)calloc(1, t->bt_psize);
	if (h == NULL)
		return (NULL);
	if (npg)
		*npg = test_mock.next_pgno++;
	return (h);
}

void __bt_free(BTREE *t, PAGE *h)
{
	(void)t;
	test_mock.free_calls++;
	test_mock.last_free_page = h;
	free(h);
}

void *reallocf(void *ptr, size_t size)
{
	void *nptr;

	if (test_mock.realloc_fail) {
		free(ptr);
		return (NULL);
	}
	nptr = realloc(ptr, size);
	if (nptr == NULL)
		free(ptr);
	return (nptr);
}

static int ref___bt_snext(BTREE *, PAGE *, const DBT *, int *);
static int ref___bt_sprev(BTREE *, PAGE *, const DBT *, int *);
int ref___bt_cmp(BTREE *, const DBT *, EPG *);
int ref___ovfl_get(BTREE *, void *, size_t *, void **, size_t *);
int ref___ovfl_put(BTREE *, const DBT *, pgno_t *);
int ref___ovfl_delete(BTREE *, void *);
int ref___bt_ret(BTREE *, EPG *, DBT *, DBT *, DBT *, DBT *, int);
void ref___bt_dpage(PAGE *);

#define __bt_cmp	ref___bt_cmp
#define __ovfl_get	ref___ovfl_get
#define __ovfl_put	ref___ovfl_put
#define __ovfl_delete	ref___ovfl_delete
#define __bt_ret	ref___bt_ret
#define __bt_snext	ref___bt_snext
#define __bt_sprev	ref___bt_sprev
#define __bt_dpage	ref___bt_dpage

/* ================================================================== */
/* bt_search.c							      */
/* ================================================================== */

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
 *
 * Parameters:
 *	t:	tree to search
 *	key:	key to find
 *	exactp:	pointer to exact match flag
 *
 * Returns:
 *	The EPG for matching record, if any, or the EPG for the location
 *	of the key, if it were inserted into the tree, is entered into
 *	the bt_cur field of the tree.  A pointer to the field is returned.
 */
EPG *
ref___bt_search(BTREE *t, const DBT *key, int *exactp)
{
	PAGE *h;
	indx_t base, idx, lim;
	pgno_t pg;
	int cmp;

	BT_CLR(t);
	for (pg = P_ROOT;;) {
		if ((h = mpool_get(t->bt_mp, pg, 0)) == NULL)
			return (NULL);

		/* Do a binary search on the current page. */
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

		/*
		 * If it's a leaf page, we're almost done.  If no duplicates
		 * are allowed, or we have an exact match, we're done.  Else,
		 * it's possible that there were matching keys on this page,
		 * which later deleted, and we're on a page with no matches
		 * while there are matches on other pages.  If at the start or
		 * end of a page, check the adjacent page.
		 */
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

		/*
		 * No match found.  Base is the smallest index greater than
		 * key and may be zero or a last + 1 index.  If it's non-zero,
		 * decrement by one, and record the internal page which should
		 * be a parent page for the key.  If a split later occurs, the
		 * inserted page will be to the right of the saved page.
		 */
		idx = base ? base - 1 : base;

next:		BT_PUSH(t, h->pgno, idx);
		pg = GETBINTERNAL(h, idx)->pgno;
		mpool_put(t->bt_mp, h, 0);
	}
}

/*
 * __bt_snext --
 *	Check for an exact match after the key.
 *
 * Parameters:
 *	t:	tree
 *	h:	current page
 *	key:	key
 *	exactp:	pointer to exact match flag
 *
 * Returns:
 *	If an exact match found.
 */
static int
ref___bt_snext(BTREE *t, PAGE *h, const DBT *key, int *exactp)
{
	EPG e;

	/*
	 * Get the next page.  The key is either an exact
	 * match, or not as good as the one we already have.
	 */
	if ((e.page = mpool_get(t->bt_mp, h->nextpg, 0)) == NULL)
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

/*
 * __bt_sprev --
 *	Check for an exact match before the key.
 *
 * Parameters:
 *	t:	tree
 *	h:	current page
 *	key:	key
 *	exactp:	pointer to exact match flag
 *
 * Returns:
 *	If an exact match found.
 */
static int
ref___bt_sprev(BTREE *t, PAGE *h, const DBT *key, int *exactp)
{
	EPG e;

	/*
	 * Get the previous page.  The key is either an exact
	 * match, or not as good as the one we already have.
	 */
	if ((e.page = mpool_get(t->bt_mp, h->prevpg, 0)) == NULL)
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

/* ================================================================== */
/* bt_overflow.c						      */
/* ================================================================== */

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
 * Big key/data code.
 *
 * Big key and data entries are stored on linked lists of pages.  The initial
 * reference is byte string stored with the key or data and is the page number
 * and size.  The actual record is stored in a chain of pages linked by the
 * nextpg field of the PAGE header.
 *
 * The first page of the chain has a special property.  If the record is used
 * by an internal page, it cannot be deleted and the P_PRESERVE bit will be set
 * in the header.
 *
 * XXX
 * A single DBT is written to each chain, so a lot of space on the last page
 * is wasted.  This is a fairly major bug for some data sets.
 */

/*
 * __OVFL_GET -- Get an overflow key/data item.
 *
 * Parameters:
 *	t:	tree
 *	p:	pointer to { pgno_t, u_int32_t }
 *	buf:	storage address
 *	bufsz:	storage size
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS
 */
int
ref___ovfl_get(BTREE *t, void *p, size_t *ssz, void **buf, size_t *bufsz)
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
	/* Make the buffer bigger as necessary. */
	if (*bufsz < sz) {
		*buf = reallocf(*buf, sz);
		if (*buf == NULL)
			return (RET_ERROR);
		*bufsz = sz;
	}

	/*
	 * Step through the linked list of pages, copying the data on each one
	 * into the buffer.  Never copy more than the data's length.
	 */
	plen = t->bt_psize - BTDATAOFF;
	for (p = *buf;; p = (char *)p + nb, pg = h->nextpg) {
		if ((h = mpool_get(t->bt_mp, pg, 0)) == NULL)
			return (RET_ERROR);

		nb = MIN(sz, plen);
		memmove(p, (char *)h + BTDATAOFF, nb);
		mpool_put(t->bt_mp, h, 0);

		if ((sz -= nb) == 0)
			break;
	}
	return (RET_SUCCESS);
}

/*
 * __OVFL_PUT -- Store an overflow key/data item.
 *
 * Parameters:
 *	t:	tree
 *	data:	DBT to store
 *	pgno:	storage page number
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS
 */
int
ref___ovfl_put(BTREE *t, const DBT *dbt, pgno_t *pg)
{
	PAGE *h, *last;
	void *p;
	pgno_t npg;
	size_t nb, plen;
	u_int32_t sz;

	/*
	 * Allocate pages and copy the key/data record into them.  Store the
	 * number of the first page in the chain.
	 */
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

/*
 * __OVFL_DELETE -- Delete an overflow chain.
 *
 * Parameters:
 *	t:	tree
 *	p:	pointer to { pgno_t, u_int32_t }
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS
 */
int
ref___ovfl_delete(BTREE *t, void *p)
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
	if ((h = mpool_get(t->bt_mp, pg, 0)) == NULL)
		return (RET_ERROR);

	/* Don't delete chains used by internal pages. */
	if (h->flags & P_PRESERVE) {
		mpool_put(t->bt_mp, h, 0);
		return (RET_SUCCESS);
	}

	/* Step through the chain, calling the free routine for each page. */
	for (plen = t->bt_psize - BTDATAOFF;; sz -= plen) {
		pg = h->nextpg;
		__bt_free(t, h);
		if (sz <= plen)
			break;
		if ((h = mpool_get(t->bt_mp, pg, 0)) == NULL)
			return (RET_ERROR);
	}
	return (RET_SUCCESS);
}

/* ================================================================== */
/* bt_utils.c							      */
/* ================================================================== */

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
 * __bt_ret --
 *	Build return key/data pair.
 *
 * Parameters:
 *	t:	tree
 *	e:	key/data pair to be returned
 *	key:	user's key structure (NULL if not to be filled in)
 *	rkey:	memory area to hold key
 *	data:	user's data structure (NULL if not to be filled in)
 *	rdata:	memory area to hold data
 *       copy:	always copy the key/data item
 *
 * Returns:
 *	RET_SUCCESS, RET_ERROR.
 */
int
ref___bt_ret(BTREE *t, EPG *e, DBT *key, DBT *rkey, DBT *data, DBT *rdata, int copy)
{
	BLEAF *bl;
	void *p;

	bl = GETBLEAF(e->page, e->index);

	/*
	 * We must copy big keys/data to make them contiguous.  Otherwise,
	 * leave the page pinned and don't copy unless the user specified
	 * concurrent access.
	 */
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
		/* Use +1 in case the first record retrieved is 0 length. */
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

/*
 * __BT_CMP -- Compare a key to a given record.
 *
 * Parameters:
 *	t:	tree
 *	k1:	DBT pointer of first arg to comparison
 *	e:	pointer to EPG for comparison
 *
 * Returns:
 *	< 0 if k1 is < record
 *	= 0 if k1 is = record
 *	> 0 if k1 is > record
 */
int
ref___bt_cmp(BTREE *t, const DBT *k1, EPG *e)
{
	BINTERNAL *bi;
	BLEAF *bl;
	DBT k2;
	PAGE *h;
	void *bigkey;

	/*
	 * The left-most key on internal pages, at any level of the tree, is
	 * guaranteed by the following code to be less than any user key.
	 * This saves us from having to update the leftmost key on an internal
	 * page when the user inserts a new key in the tree smaller than
	 * anything we've yet seen.
	 */
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

/*
 * __BT_DEFCMP -- Default comparison routine.
 *
 * Parameters:
 *	a:	DBT #1
 *	b:	DBT #2
 *
 * Returns:
 *	< 0 if a is < b
 *	= 0 if a is = b
 *	> 0 if a is > b
 */
int
ref___bt_defcmp(const DBT *a, const DBT *b)
{
	size_t len;
	u_char *p1, *p2;

	/*
	 * XXX
	 * If a size_t doesn't fit in an int, this routine can lose.
	 * What we need is an integral type which is guaranteed to be
	 * larger than a size_t, and there is no such thing.
	 */
	len = MIN(a->size, b->size);
	for (p1 = a->data, p2 = b->data; len--; ++p1, ++p2)
		if (*p1 != *p2)
			return ((int)*p1 - (int)*p2);
	return ((int)a->size - (int)b->size);
}

/*
 * __BT_DEFPFX -- Default prefix routine.
 *
 * Parameters:
 *	a:	DBT #1
 *	b:	DBT #2
 *
 * Returns:
 *	Number of bytes needed to distinguish b from a.
 */
size_t
ref___bt_defpfx(const DBT *a, const DBT *b)
{
	u_char *p1, *p2;
	size_t cnt, len;

	cnt = 1;
	len = MIN(a->size, b->size);
	for (p1 = a->data, p2 = b->data; len--; ++p1, ++p2, ++cnt)
		if (*p1 != *p2)
			return (cnt);

	/* a->size must be <= b->size, or they wouldn't be in this order. */
	return (a->size < b->size ? a->size + 1 : a->size);
}

/* ================================================================== */
/* bt_debug.c							      */
/* ================================================================== */

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

#ifdef DEBUG
/*
 * BT_DUMP -- Dump the tree
 *
 * Parameters:
 *	dbp:	pointer to the DB
 */
void
ref___bt_dump(DB *dbp)
{
	BTREE *t;
	PAGE *h;
	pgno_t i;
	char *sep;

	t = dbp->internal;
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
	    (h = mpool_get(t->bt_mp, i, MPOOL_IGNOREPIN)) != NULL; ++i)
		__bt_dpage(h);
}

/*
 * BT_DMPAGE -- Dump the meta page
 *
 * Parameters:
 *	h:	pointer to the PAGE
 */
void
ref___bt_dmpage(PAGE *h)
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

/*
 * BT_DNPAGE -- Dump the page
 *
 * Parameters:
 *	n:	page number to dump.
 */
void
ref___bt_dnpage(DB *dbp, pgno_t pgno)
{
	BTREE *t;
	PAGE *h;

	t = dbp->internal;
	if ((h = mpool_get(t->bt_mp, pgno, MPOOL_IGNOREPIN)) != NULL)
		__bt_dpage(h);
}

/*
 * BT_DPAGE -- Dump the page
 *
 * Parameters:
 *	h:	pointer to the PAGE
 */
void
ref___bt_dpage(PAGE *h)
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
	X(P_BINTERNAL,	"BINTERNAL")		/* types */
	X(P_BLEAF,	"BLEAF")
	X(P_RINTERNAL,	"RINTERNAL")		/* types */
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
/*
 * BT_STAT -- Gather/print the tree statistics
 *
 * Parameters:
 *	dbp:	pointer to the DB
 */
void
ref___bt_stat(DB *dbp)
{
	extern u_long bt_cache_hit, bt_cache_miss, bt_pfxsaved, bt_rootsplit;
	extern u_long bt_sortsplit, bt_split;
	BTREE *t;
	PAGE *h;
	pgno_t i, pcont, pinternal, pleaf;
	u_long ifree, lfree, nkeys;
	int levels;

	t = dbp->internal;
	pcont = pinternal = pleaf = 0;
	nkeys = ifree = lfree = 0;
	for (i = P_ROOT;
	    (h = mpool_get(t->bt_mp, i, MPOOL_IGNOREPIN)) != NULL; ++i)
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

	/* Count the levels of the tree. */
	for (i = P_ROOT, levels = 0 ;; ++levels) {
		h = mpool_get(t->bt_mp, i, MPOOL_IGNOREPIN);
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
