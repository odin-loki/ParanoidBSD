/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
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
 * Reference oracle for batch b0285:
 *	hbsd/src/lib/libc/db/recno/rec_search.c
 *	hbsd/src/lib/libc/db/recno/rec_seq.c
 *	hbsd/src/lib/libc/db/recno/rec_close.c
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>

typedef unsigned int	u_int;
typedef unsigned char	u_char;
typedef uint8_t		u_int8_t;
typedef uint32_t	u_int32_t;
typedef char *		caddr_t;

#define	RET_ERROR	-1
#define	RET_SUCCESS	 0
#define	RET_SPECIAL	 1

#define	MAX_PAGE_NUMBER	0xffffffff
typedef uint32_t	pgno_t;
#define	MAX_PAGE_OFFSET	65535
typedef uint16_t	indx_t;
#define	MAX_REC_NUMBER	0xffffffff
typedef uint32_t	recno_t;

typedef struct {
	void	*data;
	size_t	 size;
} DBT;

#define	R_CURSOR	1
#define	R_FIRST		3
#define	R_LAST		6
#define	R_NEXT		7
#define	R_PREV		9
#define	R_RECNOSYNC	11

typedef enum { DB_BTREE, DB_HASH, DB_RECNO } DBTYPE;

typedef struct __db {
	DBTYPE type;
	int (*close)(struct __db *);
	int (*del)(const struct __db *, const DBT *, unsigned int);
	int (*get)(const struct __db *, const DBT *, DBT *, unsigned int);
	int (*put)(const struct __db *, DBT *, const DBT *, unsigned int);
	int (*seq)(const struct __db *, DBT *, DBT *, unsigned int);
	int (*sync)(const struct __db *, unsigned int);
	void *internal;
	int (*fd)(const struct __db *);
} DB;

typedef struct MPOOL MPOOL;

#define	MPOOL_DIRTY	0x01

#define	F_SET(p, f)	(p)->flags |= (f)
#define	F_CLR(p, f)	(p)->flags &= ~(f)
#define	F_ISSET(p, f)	((p)->flags & (f))

#define	P_INVALID	 0
#define	P_META		 0
#define	P_ROOT		 1

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

#define	BTDATAOFF							\
	(sizeof(pgno_t) + sizeof(pgno_t) + sizeof(pgno_t) +		\
	    sizeof(u_int32_t) + sizeof(indx_t) + sizeof(indx_t))
#define	NEXTINDEX(p)	(((p)->lower - BTDATAOFF) / sizeof(indx_t))

typedef struct _rinternal {
	recno_t	nrecs;
	pgno_t	pgno;
} RINTERNAL;

#define	GETRINTERNAL(pg, indx)						\
	((RINTERNAL *)((char *)(pg) + (pg)->linp[indx]))

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

enum SRCHOP { SDELETE, SINSERT, SEARCH};

void	*mpool_get(MPOOL *, pgno_t, u_int);
int	 mpool_put(MPOOL *, void *, u_int);
int	 __rec_ret(BTREE *, EPG *, recno_t, DBT *, DBT *);
int	 __bt_close(DB *);
int	 __bt_sync(const DB *, u_int);
int	 munmap(void *, size_t);
int	 _close(int);
off_t	 lseek(int, off_t, int);
ssize_t	 _write(int, const void *, size_t);
ssize_t	 _writev(int, const struct iovec *, int);
int	 ftruncate(int, off_t);

/* --- rec_search.c --- */

EPG *
ref___rec_search(BTREE *t, recno_t recno, enum SRCHOP op)
{
	indx_t idx;
	PAGE *h;
	EPGNO *parent;
	RINTERNAL *r;
	pgno_t pg;
	indx_t top;
	recno_t total;
	int sverrno;

	BT_CLR(t);
	for (pg = P_ROOT, total = 0;;) {
		if ((h = mpool_get(t->bt_mp, pg, 0)) == NULL)
			goto err;
		if (h->flags & P_RLEAF) {
			t->bt_cur.page = h;
			t->bt_cur.index = recno - total;
			return (&t->bt_cur);
		}
		for (idx = 0, top = NEXTINDEX(h);;) {
			r = GETRINTERNAL(h, idx);
			if (++idx == top || total + r->nrecs > recno)
				break;
			total += r->nrecs;
		}

		BT_PUSH(t, pg, idx - 1);

		pg = r->pgno;
		switch (op) {
		case SDELETE:
			--GETRINTERNAL(h, (idx - 1))->nrecs;
			mpool_put(t->bt_mp, h, MPOOL_DIRTY);
			break;
		case SINSERT:
			++GETRINTERNAL(h, (idx - 1))->nrecs;
			mpool_put(t->bt_mp, h, MPOOL_DIRTY);
			break;
		case SEARCH:
			mpool_put(t->bt_mp, h, 0);
			break;
		}

	}
	/* Try and recover the tree. */
err:	sverrno = errno;
	if (op != SEARCH)
		while  ((parent = BT_POP(t)) != NULL) {
			if ((h = mpool_get(t->bt_mp, parent->pgno, 0)) == NULL)
				break;
			if (op == SINSERT)
				--GETRINTERNAL(h, parent->index)->nrecs;
			else
				++GETRINTERNAL(h, parent->index)->nrecs;
			mpool_put(t->bt_mp, h, MPOOL_DIRTY);
		}
	errno = sverrno;
	return (NULL);
}

/* --- rec_seq.c --- */

int
ref___rec_seq(const DB *dbp, DBT *key, DBT *data, u_int flags)
{
	BTREE *t;
	EPG *e;
	recno_t nrec;
	int status;

	t = dbp->internal;

	/* Toss any page pinned across calls. */
	if (t->bt_pinned != NULL) {
		mpool_put(t->bt_mp, t->bt_pinned, 0);
		t->bt_pinned = NULL;
	}

	switch(flags) {
	case R_CURSOR:
		if ((nrec = *(recno_t *)key->data) == 0)
			goto einval;
		break;
	case R_NEXT:
		if (F_ISSET(&t->bt_cursor, CURS_INIT)) {
			nrec = t->bt_cursor.rcursor + 1;
			break;
		}
		/* FALLTHROUGH */
	case R_FIRST:
		nrec = 1;
		break;
	case R_PREV:
		if (F_ISSET(&t->bt_cursor, CURS_INIT)) {
			if ((nrec = t->bt_cursor.rcursor - 1) == 0)
				return (RET_SPECIAL);
			break;
		}
		/* FALLTHROUGH */
	case R_LAST:
		if (!F_ISSET(t, R_EOF | R_INMEM) &&
		    t->bt_irec(t, MAX_REC_NUMBER) == RET_ERROR)
			return (RET_ERROR);
		nrec = t->bt_nrecs;
		break;
	default:
einval:		errno = EINVAL;
		return (RET_ERROR);
	}

	if (t->bt_nrecs == 0 || nrec > t->bt_nrecs) {
		if (!F_ISSET(t, R_EOF | R_INMEM) &&
		    (status = t->bt_irec(t, nrec)) != RET_SUCCESS)
			return (status);
		if (t->bt_nrecs == 0 || nrec > t->bt_nrecs)
			return (RET_SPECIAL);
	}

	if ((e = ref___rec_search(t, nrec - 1, SEARCH)) == NULL)
		return (RET_ERROR);

	F_SET(&t->bt_cursor, CURS_INIT);
	t->bt_cursor.rcursor = nrec;

	status = __rec_ret(t, e, nrec, key, data);
	if (F_ISSET(t, B_DB_LOCK))
		mpool_put(t->bt_mp, e->page, 0);
	else
		t->bt_pinned = e->page;
	return (status);
}

/* --- rec_close.c --- */

int
ref___rec_close(DB *dbp)
{
	BTREE *t;
	int status;

	t = dbp->internal;

	/* Toss any page pinned across calls. */
	if (t->bt_pinned != NULL) {
		mpool_put(t->bt_mp, t->bt_pinned, 0);
		t->bt_pinned = NULL;
	}

	if (ref___rec_sync(dbp, 0) == RET_ERROR)
		return (RET_ERROR);

	/* Committed to closing. */
	status = RET_SUCCESS;
	if (F_ISSET(t, R_MEMMAPPED) && munmap(t->bt_smap, t->bt_msize))
		status = RET_ERROR;

	if (!F_ISSET(t, R_INMEM)) {
		if (F_ISSET(t, R_CLOSEFP)) {
			if (fclose(t->bt_rfp))
				status = RET_ERROR;
		} else {
			if (_close(t->bt_rfd))
				status = RET_ERROR;
		}
	}

	if (__bt_close(dbp) == RET_ERROR)
		status = RET_ERROR;

	return (status);
}

int
ref___rec_sync(const DB *dbp, u_int flags)
{
	struct iovec iov[2];
	BTREE *t;
	DBT data, key;
	off_t off;
	recno_t scursor, trec;
	int status;

	t = dbp->internal;

	/* Toss any page pinned across calls. */
	if (t->bt_pinned != NULL) {
		mpool_put(t->bt_mp, t->bt_pinned, 0);
		t->bt_pinned = NULL;
	}

	if (flags == R_RECNOSYNC)
		return (__bt_sync(dbp, 0));

	if (F_ISSET(t, R_RDONLY | R_INMEM) || !F_ISSET(t, R_MODIFIED))
		return (RET_SUCCESS);

	/* Read any remaining records into the tree. */
	if (!F_ISSET(t, R_EOF) && t->bt_irec(t, MAX_REC_NUMBER) == RET_ERROR)
		return (RET_ERROR);

	/* Rewind the file descriptor. */
	if (lseek(t->bt_rfd, (off_t)0, SEEK_SET) != 0)
		return (RET_ERROR);

	/* Save the cursor. */
	scursor = t->bt_cursor.rcursor;

	key.size = sizeof(recno_t);
	key.data = &trec;

	if (F_ISSET(t, R_FIXLEN)) {
		/*
		 * We assume that fixed length records are all fixed length.
		 * Any that aren't are either EINVAL'd or corrected by the
		 * record put code.
		 */
		status = (dbp->seq)(dbp, &key, &data, R_FIRST);
		while (status == RET_SUCCESS) {
			if (_write(t->bt_rfd, data.data, data.size) !=
			    (ssize_t)data.size)
				return (RET_ERROR);
			status = (dbp->seq)(dbp, &key, &data, R_NEXT);
		}
	} else {
		iov[1].iov_base = &t->bt_bval;
		iov[1].iov_len = 1;

		status = (dbp->seq)(dbp, &key, &data, R_FIRST);
		while (status == RET_SUCCESS) {
			iov[0].iov_base = data.data;
			iov[0].iov_len = data.size;
			if (_writev(t->bt_rfd, iov, 2) != (ssize_t)(data.size + 1))
				return (RET_ERROR);
			status = (dbp->seq)(dbp, &key, &data, R_NEXT);
		}
	}

	/* Restore the cursor. */
	t->bt_cursor.rcursor = scursor;

	if (status == RET_ERROR)
		return (RET_ERROR);
	if ((off = lseek(t->bt_rfd, (off_t)0, SEEK_CUR)) == -1)
		return (RET_ERROR);
	if (ftruncate(t->bt_rfd, off))
		return (RET_ERROR);
	F_CLR(t, R_MODIFIED);
	return (RET_SUCCESS);
}
