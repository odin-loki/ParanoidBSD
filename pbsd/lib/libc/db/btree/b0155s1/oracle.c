/*
 * Oracle for batch b0155s1: original HardenedBSD btree sources with ref_ prefix.
 */

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
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

#ifndef O_CLOEXEC
#define O_CLOEXEC 02000000
#endif

#ifndef EFTYPE
#define EFTYPE 79
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif

#ifndef howmany
#define howmany(x, y) (((x) + ((y) - 1)) / (y))
#endif

#define RET_ERROR	-1
#define RET_SUCCESS	 0
#define RET_SPECIAL	 1

#define	MAX_PAGE_OFFSET	65535
#define	BTREEMAGIC	0x053162
#define	BTREEVERSION	3
#define DEFMINKEYPAGE	(2)
#define MINCACHE	(5)
#define MINPSIZE	(512)

#define BIG_ENDIAN	4321
#define LITTLE_ENDIAN	1234

#define __DBINTERFACE_PRIVATE
#define	M_32_SWAP(a) {							\
	uint32_t _tmp = (a);						\
	((char *)&(a))[0] = ((char *)&_tmp)[3];				\
	((char *)&(a))[1] = ((char *)&_tmp)[2];				\
	((char *)&(a))[2] = ((char *)&_tmp)[1];				\
	((char *)&(a))[3] = ((char *)&_tmp)[0];				\
}

typedef uint32_t	pgno_t;
typedef uint16_t	indx_t;
typedef uint32_t	recno_t;
typedef unsigned int	u_int;
typedef unsigned char	u_char;
typedef uint32_t	u_int32_t;
typedef uint16_t	u_int16_t;
typedef uint8_t		u_int8_t;
typedef char		* caddr_t;

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
	int (*put)(const struct __db *, DBT *, const DBT *, unsigned int);
	int (*seq)(const struct __db *, DBT *, DBT *, unsigned int);
	int (*sync)(const struct __db *, unsigned int);
	void *internal;
	int (*fd)(const struct __db *);
} DB;

typedef struct {
#define	R_DUP		0x01
	unsigned long	flags;
	unsigned int	cachesize;
	int		maxkeypage;
	int		minkeypage;
	unsigned int	psize;
	int		(*compare)(const DBT *, const DBT *);
	size_t		(*prefix)(const DBT *, const DBT *);
	int		lorder;
} BTREEINFO;

#define	R_CURSOR	1
#define	R_FIRST		3
#define	R_LAST		6
#define	R_NEXT		7
#define	R_NOOVERWRITE	8
#define	R_PREV		9
#define	R_SETCURSOR	10

#if UINT_MAX > 65535
#define	DB_LOCK		0x20000000
#define	DB_SHMEM	0x40000000
#define	DB_TXN		0x80000000
#else
#define	DB_LOCK		    0x2000
#define	DB_SHMEM	    0x4000
#define	DB_TXN		    0x8000
#endif

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
#define	NOVFLSIZE	(sizeof(pgno_t) + sizeof(u_int32_t))

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

#define MAX_POOL 64

typedef struct {
	unsigned get_calls, put_calls, new_calls, delete_calls;
	unsigned open_calls, filter_calls;
	unsigned search_calls, split_calls, ovfl_put_calls, ovfl_del_calls;
	unsigned ret_calls, free_calls, cmp_calls;
	unsigned open_fd_calls, fstat_calls, read_calls, close_calls;
	unsigned mkostemp_calls, getenv_calls, sigmask_calls, unlink_calls;
	unsigned calloc_calls;

	int get_force_null, new_force_null, search_force_null;
	int split_ret, ovfl_put_ret, ovfl_del_ret, ret_status, free_ret;
	int open_ret, fstat_ret, read_ret, close_ret, mkostemp_ret;
	int calloc_fail_after;
	int search_exact, cmp_ret;
	int split_force_error;

	pgno_t get_last_pgno, new_pgno_seq;
	unsigned get_last_flags, last_put_flags;
	void *last_put_page;

	int nreg;
	pgno_t reg_pgno[MAX_POOL];
	void *reg_page[MAX_POOL];

	EPG search_epg;
	pgno_t ovfl_pgno;
	int new_fail_after;
	int delete_ret;

	char *getenv_val;
	struct stat fstat_sb;
	unsigned char read_buf[512];
	ssize_t read_ret_val;
	int read_errno_val;
	MPOOL *mpool_open_ret;
} test_mock_state;

test_mock_state test_mock;

void test_mock_reset(void)
{
	memset(&test_mock, 0, sizeof(test_mock));
	test_mock.split_ret = RET_SUCCESS;
	test_mock.ovfl_put_ret = RET_SUCCESS;
	test_mock.ovfl_del_ret = RET_SUCCESS;
	test_mock.ret_status = RET_SUCCESS;
	test_mock.free_ret = RET_SUCCESS;
	test_mock.open_ret = 3;
	test_mock.fstat_ret = 0;
	test_mock.read_ret = 0;
	test_mock.close_ret = 0;
	test_mock.mkostemp_ret = 4;
	test_mock.delete_ret = RET_SUCCESS;
	test_mock.new_pgno_seq = 100;
	test_mock.fstat_sb.st_blksize = 4096;
	test_mock.mpool_open_ret = (MPOOL *)0x1;
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

void *mpool_new(MPOOL *mp, pgno_t *npg, unsigned int flags)
{
	(void)mp;
	(void)flags;
	test_mock.new_calls++;
	if (test_mock.new_force_null)
		return NULL;
	if (test_mock.new_fail_after > 0) {
		test_mock.new_fail_after--;
		return NULL;
	}
	if (npg)
		*npg = test_mock.new_pgno_seq++;
	return calloc(1, 512);
}

int mpool_delete(MPOOL *mp, void *page)
{
	(void)mp;
	(void)page;
	test_mock.delete_calls++;
	return test_mock.delete_ret;
}

MPOOL *mpool_open(void *a, int fd, u_int32_t psize, pgno_t ncache)
{
	(void)a; (void)fd; (void)psize; (void)ncache;
	test_mock.open_calls++;
	return test_mock.mpool_open_ret;
}

void mpool_filter(MPOOL *mp, void *a, void *b, void *c)
{
	(void)mp; (void)a; (void)b; (void)c;
	test_mock.filter_calls++;
}

EPG * __bt_search(BTREE *t, const DBT *key, int *exact)
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

int __bt_split(BTREE *t, PAGE *h, const DBT *key, const DBT *data,
    int dflags, u_int32_t nbytes, indx_t idx)
{
	(void)t; (void)h; (void)key; (void)data; (void)dflags; (void)nbytes; (void)idx;
	test_mock.split_calls++;
	if (test_mock.split_force_error)
		return RET_ERROR;
	return test_mock.split_ret;
}

int __ovfl_put(BTREE *t, const DBT *dbt, pgno_t *pg)
{
	(void)t; (void)dbt;
	test_mock.ovfl_put_calls++;
	if (test_mock.ovfl_put_ret == RET_ERROR)
		return RET_ERROR;
	*pg = test_mock.ovfl_pgno++;
	return RET_SUCCESS;
}

int __ovfl_delete(BTREE *t, void *p)
{
	(void)t; (void)p;
	test_mock.ovfl_del_calls++;
	return test_mock.ovfl_del_ret;
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
	if (rkey != NULL && rkey->data == NULL) {
		rkey->data = malloc(64);
		rkey->size = 0;
	}
	return test_mock.ret_status;
}

int __bt_free(BTREE *t, PAGE *h)
{
	(void)t; (void)h;
	test_mock.free_calls++;
	return test_mock.free_ret;
}

int __bt_close(DB *dbp) { (void)dbp; return RET_SUCCESS; }
int __bt_get(const DB *dbp, const DBT *k, DBT *d, u_int f) { (void)dbp;(void)k;(void)d;(void)f; return RET_SUCCESS; }
int __bt_sync(const DB *dbp, u_int f) { (void)dbp;(void)f; return RET_SUCCESS; }
int __bt_defcmp(const DBT *a, const DBT *b) { (void)a;(void)b; return 0; }
size_t __bt_defpfx(const DBT *a, const DBT *b) { (void)a;(void)b; return 0; }
void __bt_pgin(void *a, pgno_t b, void *c) { (void)a;(void)b;(void)c; }
void __bt_pgout(void *a, pgno_t b, void *c) { (void)a;(void)b;(void)c; }

int _open(const char *path, int flags, ...)
{
	(void)path; (void)flags;
	test_mock.open_fd_calls++;
	errno = 0;
	return test_mock.open_ret;
}

int _fstat(int fd, struct stat *sb)
{
	(void)fd;
	test_mock.fstat_calls++;
	if (test_mock.fstat_ret != 0) {
		errno = test_mock.fstat_ret;
		return -1;
	}
	*sb = test_mock.fstat_sb;
	return 0;
}

ssize_t _read(int fd, void *buf, size_t nbytes)
{
	(void)fd;
	test_mock.read_calls++;
	if (test_mock.read_ret != 0) {
		errno = test_mock.read_ret;
		return -1;
	}
	memcpy(buf, test_mock.read_buf, nbytes < sizeof(test_mock.read_buf) ?
	    nbytes : sizeof(test_mock.read_buf));
	return test_mock.read_ret_val;
}

int _close(int fd)
{
	(void)fd;
	test_mock.close_calls++;
	return test_mock.close_ret;
}

char *secure_getenv(const char *name)
{
	(void)name;
	test_mock.getenv_calls++;
	return test_mock.getenv_val;
}

int __libc_sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
	(void)how; (void)set; (void)oset;
	test_mock.sigmask_calls++;
	return 0;
}

int mkostemp(char *template, int flags)
{
	(void)template; (void)flags;
	test_mock.mkostemp_calls++;
	return test_mock.mkostemp_ret;
}

void *test_calloc(size_t n, size_t sz);
#define calloc test_calloc

void *test_calloc(size_t n, size_t sz)
{
	void *p;
	test_mock.calloc_calls++;
	if (test_mock.calloc_fail_after > 0) {
		test_mock.calloc_fail_after--;
		return NULL;
	}
	p = malloc(n * sz);
	if (p != NULL)
		memset(p, 0, n * sz);
	return p;
}
int	 __bt_cmp(BTREE *, const DBT *, EPG *);
EPG	*__bt_search(BTREE *, const DBT *, int *);
int	 __bt_split(BTREE *, PAGE *, const DBT *, const DBT *, int,
    u_int32_t, indx_t);
int	 __ovfl_put(BTREE *, const DBT *, pgno_t *);
int	 __bt_dleaf(BTREE *, const DBT *, PAGE *, u_int);
void	 __bt_setcur(BTREE *, pgno_t, u_int);
int	 ref___bt_put(const DB *, DBT *, const DBT *, u_int);
EPG	*ref_bt_fast(BTREE *, const DBT *, const DBT *, int *);

#ifdef __cplusplus
}
#endif



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
	return (&t->bt_cur);

miss:
	t->bt_order = NOT;
	mpool_put(t->bt_mp, h, 0);
	return (NULL);
}
