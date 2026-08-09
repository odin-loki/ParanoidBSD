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
 * PBSD port of:
 *	hbsd/src/lib/libc/db/recno/rec_delete.c
 *	hbsd/src/lib/libc/db/recno/rec_open.c
 *	hbsd/src/lib/libc/db/recno/rec_get.c
 *	hbsd/src/lib/libc/db/recno/rec_put.c
 */

module;

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

export module pbsd.lib.libc.db.recno.b0331;

export namespace pbsd::lib_libc_db_recno::b0331 {

using u_int = unsigned int;
using u_char = unsigned char;
using u_int8_t = unsigned char;
using u_int32_t = unsigned int;
using caddr_t = char *;

#define	RET_ERROR	-1
#define	RET_SUCCESS	 0
#define	RET_SPECIAL	 1

#define	MAX_PAGE_NUMBER	0xffffffff
using pgno_t = u_int32_t;
#define	MAX_PAGE_OFFSET	65535
using indx_t = u_int16_t;
#define	MAX_REC_NUMBER	0xffffffff
using recno_t = u_int32_t;

typedef struct {
	void	*data;
	size_t	 size;
} DBT;

#define	R_CURSOR	1
#define	R_IAFTER	4
#define	R_IBEFORE	5
#define	R_NOOVERWRITE	8
#define	R_SETCURSOR	10

#define	R_FIXEDLEN	0x01
#define	R_NOKEY		0x02
#define	R_SNAPSHOT	0x04

typedef struct {
	unsigned long	flags;
	unsigned int	cachesize;
	unsigned int	psize;
	int		lorder;
	size_t		reclen;
	unsigned char	bval;
	char	*bfname;
} RECNOINFO;

typedef struct {
	unsigned int	flags;
	unsigned int	cachesize;
	unsigned int	maxkeypage;
	unsigned int	minkeypage;
	unsigned int	psize;
	int		(*compare)(const DBT *, const DBT *);
	size_t		(*prefix)(const DBT *, const DBT *);
	int		lorder;
} BTREEINFO;

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

struct MPOOL;

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

#define	LALIGN(n)	(((n) + sizeof(pgno_t) - 1) & ~(sizeof(pgno_t) - 1))
#define	NOVFLSIZE	(sizeof(pgno_t) + sizeof(u_int32_t))

#define	P_BIGDATA	0x01

typedef struct _rinternal {
	recno_t	nrecs;
	pgno_t	pgno;
} RINTERNAL;

#define	GETRINTERNAL(pg, indx)						\
	((RINTERNAL *)((char *)(pg) + (pg)->linp[indx]))

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

} /* namespace */

namespace pbsd::lib_libc_db_recno::b0331 {

extern "C" {
void	*mpool_get(MPOOL *, pgno_t, u_int);
int	 mpool_put(MPOOL *, void *, u_int);
EPG	*__rec_search(BTREE *, recno_t, enum SRCHOP);
int	 __rec_ret(BTREE *, EPG *, recno_t, DBT *, DBT *);
int	 __ovfl_delete(BTREE *, char *);
int	 __ovfl_put(BTREE *, const DBT *, pgno_t *);
int	 __bt_split(BTREE *, PAGE *, const DBT *, const DBT *, u_int, u_int32_t,
	    indx_t);
DB	*__bt_open(const char *, int, int, const void *, int);
int	 __bt_close(DB *);
int	 __rec_close(DB *);
int	 __rec_seq(const DB *, DBT *, DBT *, u_int);
int	 __rec_sync(const DB *, u_int);
int	 _open(const char *, int, ...);
int	 _close(int);
int	 _fstat(int, struct stat *);
void	*reallocf(void *, size_t);
}

static int rec_rdelete(BTREE *, recno_t);

int __rec_fd(const DB *);
int __rec_get(const DB *, const DBT *, DBT *, u_int);
int __rec_fpipe(BTREE *, recno_t);
int __rec_vpipe(BTREE *, recno_t);
int __rec_fmap(BTREE *, recno_t);
int __rec_vmap(BTREE *, recno_t);
int __rec_put(const DB *, DBT *, const DBT *, u_int);
int __rec_iput(BTREE *, recno_t, const DBT *, u_int);

} /* namespace */

export namespace pbsd::lib_libc_db_recno::b0331 {

int
__rec_delete(const DB *dbp, const DBT *key, u_int flags)
{
	BTREE *t;
	recno_t nrec;
	int status;

	t = static_cast<BTREE *>(dbp->internal);

	/* Toss any page pinned across calls. */
	if (t->bt_pinned != NULL) {
		mpool_put(t->bt_mp, t->bt_pinned, 0);
		t->bt_pinned = NULL;
	}

	switch(flags) {
	case 0:
		if ((nrec = *(recno_t *)key->data) == 0)
			goto einval;
		if (nrec > t->bt_nrecs)
			return (RET_SPECIAL);
		--nrec;
		status = rec_rdelete(t, nrec);
		break;
	case R_CURSOR:
		if (!F_ISSET(&t->bt_cursor, CURS_INIT))
			goto einval;
		if (t->bt_nrecs == 0)
			return (RET_SPECIAL);
		status = rec_rdelete(t, t->bt_cursor.rcursor - 1);
		if (status == RET_SUCCESS)
			--t->bt_cursor.rcursor;
		break;
	default:
einval:		errno = EINVAL;
		return (RET_ERROR);
	}

	if (status == RET_SUCCESS)
		F_SET(t, B_MODIFIED | R_MODIFIED);
	return (status);
}

int
__rec_dleaf(BTREE *t, PAGE *h, u_int32_t idx)
{
	RLEAF *rl;
	indx_t *ip, cnt, offset;
	u_int32_t nbytes;
	char *from;
	void *to;

	/*
	 * Delete a record from a recno leaf page.  Internal records are never
	 * deleted from internal pages, regardless of the records that caused
	 * them to be added being deleted.  Pages made empty by deletion are
	 * not reclaimed.  They are, however, made available for reuse.
	 *
	 * Pack the remaining entries at the end of the page, shift the indices
	 * down, overwriting the deleted record and its index.  If the record
	 * uses overflow pages, make them available for reuse.
	 */
	to = rl = GETRLEAF(h, idx);
	if (rl->flags & P_BIGDATA && __ovfl_delete(t, rl->bytes) == RET_ERROR)
		return (RET_ERROR);
	nbytes = NRLEAF(rl);

	/*
	 * Compress the key/data pairs.  Compress and adjust the [BR]LEAF
	 * offsets.  Reset the headers.
	 */
	from = (char *)h + h->upper;
	memmove(from + nbytes, from, (char *)to - from);
	h->upper += nbytes;

	offset = h->linp[idx];
	for (cnt = &h->linp[idx] - (ip = &h->linp[0]); cnt--; ++ip)
		if (ip[0] < offset)
			ip[0] += nbytes;
	for (cnt = &h->linp[NEXTINDEX(h)] - ip; --cnt; ++ip)
		ip[0] = ip[1] < offset ? ip[1] + nbytes : ip[1];
	h->lower -= sizeof(indx_t);
	--t->bt_nrecs;
	return (RET_SUCCESS);
}

DB *
__rec_open(const char *fname, int flags, int mode, const RECNOINFO *openinfo,
    int dflags)
{
	BTREE *t;
	BTREEINFO btopeninfo;
	DB *dbp;
	PAGE *h;
	struct stat sb;
	int rfd, sverrno;

	/* Open the user's file -- if this fails, we're done. */
	if (fname != NULL && (rfd = _open(fname, flags | O_CLOEXEC, mode)) < 0)
		return (NULL);

	/* Create a btree in memory (backed by disk). */
	dbp = NULL;
	if (openinfo) {
		if (openinfo->flags & ~(R_FIXEDLEN | R_NOKEY | R_SNAPSHOT))
			goto einval;
		btopeninfo.flags = 0;
		btopeninfo.cachesize = openinfo->cachesize;
		btopeninfo.maxkeypage = 0;
		btopeninfo.minkeypage = 0;
		btopeninfo.psize = openinfo->psize;
		btopeninfo.compare = NULL;
		btopeninfo.prefix = NULL;
		btopeninfo.lorder = openinfo->lorder;
		dbp = static_cast<DB *>(__bt_open(openinfo->bfname,
		    O_RDWR, S_IRUSR | S_IWUSR, &btopeninfo, dflags));
	} else
		dbp = static_cast<DB *>(__bt_open(NULL, O_RDWR, S_IRUSR | S_IWUSR, NULL, dflags));
	if (dbp == NULL)
		goto err;

	/*
	 * Some fields in the tree structure are recno specific.  Fill them
	 * in and make the btree structure look like a recno structure.  We
	 * don't change the bt_ovflsize value, it's close enough and slightly
	 * bigger.
	 */
	t = static_cast<BTREE *>(dbp->internal);
	if (openinfo) {
		if (openinfo->flags & R_FIXEDLEN) {
			F_SET(t, R_FIXLEN);
			t->bt_reclen = openinfo->reclen;
			if (t->bt_reclen == 0)
				goto einval;
		}
		t->bt_bval = openinfo->bval;
	} else
		t->bt_bval = '\n';

	F_SET(t, R_RECNO);
	if (fname == NULL)
		F_SET(t, R_EOF | R_INMEM);
	else
		t->bt_rfd = rfd;

	if (fname != NULL) {
		/*
		 * In 4.4BSD, stat(2) returns true for ISSOCK on pipes.
		 * Unfortunately, that's not portable, so we use lseek
		 * and check the errno values.
		 */
		errno = 0;
		if (lseek(rfd, (off_t)0, SEEK_CUR) == -1 && errno == ESPIPE) {
			switch (flags & O_ACCMODE) {
			case O_RDONLY:
				F_SET(t, R_RDONLY);
				break;
			default:
				goto einval;
			}
slow:			if ((t->bt_rfp = fdopen(rfd, "r")) == NULL)
				goto err;
			F_SET(t, R_CLOSEFP);
			t->bt_irec =
			    F_ISSET(t, R_FIXLEN) ? __rec_fpipe : __rec_vpipe;
		} else {
			switch (flags & O_ACCMODE) {
			case O_RDONLY:
				F_SET(t, R_RDONLY);
				break;
			case O_RDWR:
				break;
			default:
				goto einval;
			}

			if (_fstat(rfd, &sb))
				goto err;
			/*
			 * Kluge -- we'd like to test to see if the file is too
			 * big to mmap.  Since, we don't know what size or type
			 * off_t's or size_t's are, what the largest unsigned
			 * integral type is, or what random insanity the local
			 * C compiler will perpetrate, doing the comparison in
			 * a portable way is flatly impossible.  Hope that mmap
			 * fails if the file is too large.
			 */
			if (sb.st_size == 0)
				F_SET(t, R_EOF);
			else {
#ifdef MMAP_NOT_AVAILABLE
				/*
				 * XXX
				 * Mmap doesn't work correctly on many current
				 * systems.  In particular, it can fail subtly,
				 * with cache coherency problems.  Don't use it
				 * for now.
				 */
				t->bt_msize = sb.st_size;
				if ((t->bt_smap = static_cast<caddr_t>(mmap(NULL, t->bt_msize,
				    PROT_READ, MAP_PRIVATE, rfd,
				    (off_t)0))) == MAP_FAILED)
					goto slow;
				t->bt_cmap = t->bt_smap;
				t->bt_emap = t->bt_smap + sb.st_size;
				t->bt_irec = F_ISSET(t, R_FIXLEN) ?
				    __rec_fmap : __rec_vmap;
				F_SET(t, R_MEMMAPPED);
#else
				goto slow;
#endif
			}
		}
	}

	/* Use the recno routines. */
	dbp->close = __rec_close;
	dbp->del = __rec_delete;
	dbp->fd = __rec_fd;
	dbp->get = __rec_get;
	dbp->put = __rec_put;
	dbp->seq = __rec_seq;
	dbp->sync = __rec_sync;

	/* If the root page was created, reset the flags. */
	if ((h = static_cast<PAGE *>(mpool_get(t->bt_mp, P_ROOT, 0))) == NULL)
		goto err;
	if ((h->flags & P_TYPE) == P_BLEAF) {
		F_CLR(h, P_TYPE);
		F_SET(h, P_RLEAF);
		mpool_put(t->bt_mp, h, MPOOL_DIRTY);
	} else
		mpool_put(t->bt_mp, h, 0);

	if (openinfo && openinfo->flags & R_SNAPSHOT &&
	    !F_ISSET(t, R_EOF | R_INMEM) &&
	    t->bt_irec(t, MAX_REC_NUMBER) == RET_ERROR)
		goto err;
	return (dbp);

einval:	errno = EINVAL;
err:	sverrno = errno;
	if (dbp != NULL)
		(void)__bt_close(dbp);
	if (fname != NULL)
		(void)_close(rfd);
	errno = sverrno;
	return (NULL);
}

int
__rec_fd(const DB *dbp)
{
	BTREE *t;

	t = static_cast<BTREE *>(dbp->internal);

	/* Toss any page pinned across calls. */
	if (t->bt_pinned != NULL) {
		mpool_put(t->bt_mp, t->bt_pinned, 0);
		t->bt_pinned = NULL;
	}

	/* In-memory database can't have a file descriptor. */
	if (F_ISSET(t, R_INMEM)) {
		errno = ENOENT;
		return (-1);
	}
	return (t->bt_rfd);
}

int
__rec_get(const DB *dbp, const DBT *key, DBT *data, u_int flags)
{
	BTREE *t;
	EPG *e;
	recno_t nrec;
	int status;

	t = static_cast<BTREE *>(dbp->internal);

	/* Toss any page pinned across calls. */
	if (t->bt_pinned != NULL) {
		mpool_put(t->bt_mp, t->bt_pinned, 0);
		t->bt_pinned = NULL;
	}

	/* Get currently doesn't take any flags, and keys of 0 are illegal. */
	if (flags || (nrec = *(recno_t *)key->data) == 0) {
		errno = EINVAL;
		return (RET_ERROR);
	}

	/*
	 * If we haven't seen this record yet, try to find it in the
	 * original file.
	 */
	if (nrec > t->bt_nrecs) {
		if (F_ISSET(t, R_EOF | R_INMEM))
			return (RET_SPECIAL);
		if ((status = t->bt_irec(t, nrec)) != RET_SUCCESS)
			return (status);
	}

	--nrec;
	if ((e = __rec_search(t, nrec, SEARCH)) == NULL)
		return (RET_ERROR);

	status = __rec_ret(t, e, 0, NULL, data);
	if (F_ISSET(t, B_DB_LOCK))
		mpool_put(t->bt_mp, e->page, 0);
	else
		t->bt_pinned = e->page;
	return (status);
}

int
__rec_fpipe(BTREE *t, recno_t top)
{
	DBT data;
	recno_t nrec;
	size_t len;
	int ch;
	u_char *p;

	if (t->bt_rdata.size < t->bt_reclen) {
		t->bt_rdata.data = reallocf(t->bt_rdata.data, t->bt_reclen);
		if (t->bt_rdata.data == NULL)
			return (RET_ERROR);
		t->bt_rdata.size = t->bt_reclen;
	}
	data.data = t->bt_rdata.data;
	data.size = t->bt_reclen;

	for (nrec = t->bt_nrecs; nrec < top;) {
		len = t->bt_reclen;
		for (p = static_cast<u_char *>(t->bt_rdata.data);; *p++ = ch)
			if ((ch = getc(t->bt_rfp)) == EOF || !--len) {
				if (ch != EOF)
					*p = ch;
				if (len != 0)
					memset(p, t->bt_bval, len);
				if (__rec_iput(t,
				    nrec, &data, 0) != RET_SUCCESS)
					return (RET_ERROR);
				++nrec;
				break;
			}
		if (ch == EOF)
			break;
	}
	if (nrec < top) {
		F_SET(t, R_EOF);
		return (RET_SPECIAL);
	}
	return (RET_SUCCESS);
}

int
__rec_vpipe(BTREE *t, recno_t top)
{
	DBT data;
	recno_t nrec;
	size_t len;
	size_t sz;
	int bval, ch;
	u_char *p;

	bval = t->bt_bval;
	for (nrec = t->bt_nrecs; nrec < top; ++nrec) {
		for (p = static_cast<u_char *>(t->bt_rdata.data),
		    sz = t->bt_rdata.size;; *p++ = ch, --sz) {
			if ((ch = getc(t->bt_rfp)) == EOF || ch == bval) {
				data.data = t->bt_rdata.data;
				data.size = p - static_cast<u_char *>(t->bt_rdata.data);
				if (ch == EOF && data.size == 0)
					break;
				if (__rec_iput(t, nrec, &data, 0)
				    != RET_SUCCESS)
					return (RET_ERROR);
				break;
			}
			if (sz == 0) {
				len = p - static_cast<u_char *>(t->bt_rdata.data);
				t->bt_rdata.size += (sz = 256);
				t->bt_rdata.data = reallocf(t->bt_rdata.data, t->bt_rdata.size);
				if (t->bt_rdata.data == NULL)
					return (RET_ERROR);
				p = static_cast<u_char *>(t->bt_rdata.data) + len;
			}
		}
		if (ch == EOF)
			break;
	}
	if (nrec < top) {
		F_SET(t, R_EOF);
		return (RET_SPECIAL);
	}
	return (RET_SUCCESS);
}

int
__rec_fmap(BTREE *t, recno_t top)
{
	DBT data;
	recno_t nrec;
	u_char *sp, *ep, *p;
	size_t len;

	if (t->bt_rdata.size < t->bt_reclen) {
		t->bt_rdata.data = reallocf(t->bt_rdata.data, t->bt_reclen);
		if (t->bt_rdata.data == NULL)
			return (RET_ERROR);
		t->bt_rdata.size = t->bt_reclen;
	}
	data.data = t->bt_rdata.data;
	data.size = t->bt_reclen;

	sp = (u_char *)t->bt_cmap;
	ep = (u_char *)t->bt_emap;
	for (nrec = t->bt_nrecs; nrec < top; ++nrec) {
		if (sp >= ep) {
			F_SET(t, R_EOF);
			return (RET_SPECIAL);
		}
		len = t->bt_reclen;
		for (p = static_cast<u_char *>(t->bt_rdata.data);
		    sp < ep && len > 0; *p++ = *sp++, --len);
		if (len != 0)
			memset(p, t->bt_bval, len);
		if (__rec_iput(t, nrec, &data, 0) != RET_SUCCESS)
			return (RET_ERROR);
	}
	t->bt_cmap = (caddr_t)sp;
	return (RET_SUCCESS);
}

int
__rec_vmap(BTREE *t, recno_t top)
{
	DBT data;
	u_char *sp, *ep;
	recno_t nrec;
	int bval;

	sp = (u_char *)t->bt_cmap;
	ep = (u_char *)t->bt_emap;
	bval = t->bt_bval;

	for (nrec = t->bt_nrecs; nrec < top; ++nrec) {
		if (sp >= ep) {
			F_SET(t, R_EOF);
			return (RET_SPECIAL);
		}
		for (data.data = sp; sp < ep && *sp != bval; ++sp);
		data.size = sp - static_cast<u_char *>(data.data);
		if (__rec_iput(t, nrec, &data, 0) != RET_SUCCESS)
			return (RET_ERROR);
		++sp;
	}
	t->bt_cmap = (caddr_t)sp;
	return (RET_SUCCESS);
}

int
__rec_put(const DB *dbp, DBT *key, const DBT *data, u_int flags)
{
	BTREE *t;
	DBT fdata, tdata;
	recno_t nrec;
	int status;

	t = static_cast<BTREE *>(dbp->internal);

	/* Toss any page pinned across calls. */
	if (t->bt_pinned != NULL) {
		mpool_put(t->bt_mp, t->bt_pinned, 0);
		t->bt_pinned = NULL;
	}

	/*
	 * If using fixed-length records, and the record is long, return
	 * EINVAL.  If it's short, pad it out.  Use the record data return
	 * memory, it's only short-term.
	 */
	if (F_ISSET(t, R_FIXLEN) && data->size != t->bt_reclen) {
		if (data->size > t->bt_reclen)
			goto einval;

		if (t->bt_rdata.size < t->bt_reclen) {
			t->bt_rdata.data = 
			    reallocf(t->bt_rdata.data, t->bt_reclen);
			if (t->bt_rdata.data == NULL)
				return (RET_ERROR);
			t->bt_rdata.size = t->bt_reclen;
		}
		memmove(t->bt_rdata.data, data->data, data->size);
		memset((char *)t->bt_rdata.data + data->size,
		    t->bt_bval, t->bt_reclen - data->size);
		fdata.data = t->bt_rdata.data;
		fdata.size = t->bt_reclen;
	} else {
		fdata.data = data->data;
		fdata.size = data->size;
	}

	switch (flags) {
	case R_CURSOR:
		if (!F_ISSET(&t->bt_cursor, CURS_INIT))
			goto einval;
		nrec = t->bt_cursor.rcursor;
		break;
	case R_SETCURSOR:
		if ((nrec = *(recno_t *)key->data) == 0)
			goto einval;
		break;
	case R_IAFTER:
		if ((nrec = *(recno_t *)key->data) == 0) {
			nrec = 1;
			flags = R_IBEFORE;
		}
		break;
	case 0:
	case R_IBEFORE:
		if ((nrec = *(recno_t *)key->data) == 0)
			goto einval;
		break;
	case R_NOOVERWRITE:
		if ((nrec = *(recno_t *)key->data) == 0)
			goto einval;
		if (nrec <= t->bt_nrecs)
			return (RET_SPECIAL);
		break;
	default:
einval:		errno = EINVAL;
		return (RET_ERROR);
	}

	/*
	 * Make sure that records up to and including the put record are
	 * already in the database.  If skipping records, create empty ones.
	 */
	if (nrec > t->bt_nrecs) {
		if (!F_ISSET(t, R_EOF | R_INMEM) &&
		    t->bt_irec(t, nrec) == RET_ERROR)
			return (RET_ERROR);
		if (nrec > t->bt_nrecs + 1) {
			if (F_ISSET(t, R_FIXLEN)) {
				if ((tdata.data = malloc(t->bt_reclen)) == NULL)
					return (RET_ERROR);
				tdata.size = t->bt_reclen;
				memset(tdata.data, t->bt_bval, tdata.size);
			} else {
				tdata.data = NULL;
				tdata.size = 0;
			}
			while (nrec > t->bt_nrecs + 1)
				if (__rec_iput(t,
				    t->bt_nrecs, &tdata, 0) != RET_SUCCESS)
					return (RET_ERROR);
			if (F_ISSET(t, R_FIXLEN))
				free(tdata.data);
		}
	}

	if ((status = __rec_iput(t, nrec - 1, &fdata, flags)) != RET_SUCCESS)
		return (status);

	switch (flags) {
	case R_IAFTER:
		nrec++;
		break;
	case R_SETCURSOR:
		t->bt_cursor.rcursor = nrec;
		break;
	}

	F_SET(t, R_MODIFIED);
	return (__rec_ret(t, NULL, nrec, key, NULL));
}

int
__rec_iput(BTREE *t, recno_t nrec, const DBT *data, u_int flags)
{
	DBT tdata;
	EPG *e;
	PAGE *h;
	indx_t idx, nxtindex;
	pgno_t pg;
	u_int32_t nbytes;
	int dflags, status;
	char *dest, db[NOVFLSIZE];

	/*
	 * If the data won't fit on a page, store it on indirect pages.
	 *
	 * XXX
	 * If the insert fails later on, these pages aren't recovered.
	 */
	if (data->size > t->bt_ovflsize) {
		if (__ovfl_put(t, data, &pg) == RET_ERROR)
			return (RET_ERROR);
		tdata.data = db;
		tdata.size = NOVFLSIZE;
		memcpy(db, &pg, sizeof(pg));
		*(u_int32_t *)(db + sizeof(pgno_t)) = data->size;
		dflags = P_BIGDATA;
		data = &tdata;
	} else
		dflags = 0;

	/* __rec_search pins the returned page. */
	if ((e = __rec_search(t, nrec,
	    nrec > t->bt_nrecs || flags == R_IAFTER || flags == R_IBEFORE ?
	    SINSERT : SEARCH)) == NULL)
		return (RET_ERROR);

	h = e->page;
	idx = e->index;

	/*
	 * Add the specified key/data pair to the tree.  The R_IAFTER and
	 * R_IBEFORE flags insert the key after/before the specified key.
	 *
	 * Pages are split as required.
	 */
	switch (flags) {
	case R_IAFTER:
		++idx;
		break;
	case R_IBEFORE:
		break;
	default:
		if (nrec < t->bt_nrecs &&
		    __rec_dleaf(t, h, idx) == RET_ERROR) {
			mpool_put(t->bt_mp, h, 0);
			return (RET_ERROR);
		}
		break;
	}

	/*
	 * If not enough room, split the page.  The split code will insert
	 * the key and data and unpin the current page.  If inserting into
	 * the offset array, shift the pointers up.
	 */
	nbytes = NRLEAFDBT(data->size);
	if ((u_int32_t)(h->upper - h->lower) < nbytes + sizeof(indx_t)) {
		status = __bt_split(t, h, NULL, data, dflags, nbytes, idx);
		if (status == RET_SUCCESS)
			++t->bt_nrecs;
		return (status);
	}

	if (idx < (nxtindex = NEXTINDEX(h)))
		memmove(h->linp + idx + 1, h->linp + idx,
		    (nxtindex - idx) * sizeof(indx_t));
	h->lower += sizeof(indx_t);

	h->linp[idx] = h->upper -= nbytes;
	dest = (char *)h + h->upper;
	WR_RLEAF(dest, data, dflags);

	++t->bt_nrecs;
	F_SET(t, B_MODIFIED);
	mpool_put(t->bt_mp, h, MPOOL_DIRTY);

	return (RET_SUCCESS);
}

} /* namespace */

namespace pbsd::lib_libc_db_recno::b0331 {

static int
rec_rdelete(BTREE *t, recno_t nrec)
{
	EPG *e;
	PAGE *h;
	int status;

	/* Find the record; __rec_search pins the page. */
	if ((e = __rec_search(t, nrec, SDELETE)) == NULL)
		return (RET_ERROR);

	/* Delete the record. */
	h = e->page;
	status = __rec_dleaf(t, h, e->index);
	if (status != RET_SUCCESS) {
		mpool_put(t->bt_mp, h, 0);
		return (status);
	}
	mpool_put(t->bt_mp, h, MPOOL_DIRTY);
	return (RET_SUCCESS);
}

} /* namespace */
