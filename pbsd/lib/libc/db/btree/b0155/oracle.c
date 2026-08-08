/*
 * Oracle for batch b0155: original HardenedBSD btree sources with ref_ prefix.
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

enum { NOT, BACK, FORWARD };

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
	if (test_mock.mkostemp_ret < 0)
		errno = ENOENT;
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

EPG *ref_bt_fast(BTREE *, const DBT *, const DBT *, int *);
DB *ref___bt_open(const char *, int, int, const BTREEINFO *, int);
int ref_nroot(BTREE *);
int ref_tmp(void);
int ref_byteorder(void);
int ref___bt_fd(const DB *);
int ref___bt_put(const DB *, DBT *, const DBT *, u_int);
int ref___bt_seq(const DB *, DBT *, DBT *, u_int);
int ref___bt_seqset(BTREE *, EPG *, DBT *, int);
int ref___bt_seqadv(BTREE *, EPG *, int);
int ref___bt_first(BTREE *, const DBT *, EPG *, int *);
void ref___bt_setcur(BTREE *, pgno_t, u_int);
int ref___bt_delete(const DB *, const DBT *, u_int);
int ref___bt_stkacq(BTREE *, PAGE **, CURSOR *);
int ref___bt_bdelete(BTREE *, const DBT *);
int ref___bt_pdelete(BTREE *, PAGE *);
int ref___bt_dleaf(BTREE *, const DBT *, PAGE *, u_int);
int ref___bt_curdel(BTREE *, const DBT *, PAGE *, u_int);
int ref___bt_relink(BTREE *, PAGE *);


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
delete:		if (ref___bt_dleaf(t, key, h, idx) == RET_ERROR) {
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
		ref___bt_setcur(t, e->page->pgno, e->index);

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
 * Implementation of btree access method for 4.4BSD.
 *
 * The design here was originally based on that of the btree access method
 * used in the Postgres database system at UC Berkeley.  This implementation
 * is wholly independent of the Postgres code.
 */




#ifdef DEBUG
#undef	MINPSIZE
#define	MINPSIZE	128
#endif


/*
 * __BT_OPEN -- Open a btree.
 *
 * Creates and fills a DB struct, and calls the routine that actually
 * opens the btree.
 *
 * Parameters:
 *	fname:	filename (NULL for in-memory trees)
 *	flags:	open flag bits
 *	mode:	open permission bits
 *	b:	BTREEINFO pointer
 *
 * Returns:
 *	NULL on failure, pointer to DB on success.
 *
 */
DB *
ref___bt_open(const char *fname, int flags, int mode, const BTREEINFO *openinfo, int dflags)
{
	struct stat sb;
	BTMETA m;
	BTREE *t;
	BTREEINFO b;
	DB *dbp;
	pgno_t ncache;
	ssize_t nr;
	int machine_lorder, saved_errno;

	t = NULL;

	/*
	 * Intention is to make sure all of the user's selections are okay
	 * here and then use them without checking.  Can't be complete, since
	 * we don't know the right page size, lorder or flags until the backing
	 * file is opened.  Also, the file's page size can cause the cachesize
	 * to change.
	 */
	machine_lorder = ref_byteorder();
	if (openinfo) {
		b = *openinfo;

		/* Flags: R_DUP. */
		if (b.flags & ~(R_DUP))
			goto einval;

		/*
		 * Page size must be indx_t aligned and >= MINPSIZE.  Default
		 * page size is set farther on, based on the underlying file
		 * transfer size.
		 */
		if (b.psize &&
		    (b.psize < MINPSIZE || b.psize > MAX_PAGE_OFFSET + 1 ||
		    b.psize & (sizeof(indx_t) - 1) ))
			goto einval;

		/* Minimum number of keys per page; absolute minimum is 2. */
		if (b.minkeypage) {
			if (b.minkeypage < 2)
				goto einval;
		} else
			b.minkeypage = DEFMINKEYPAGE;

		/* If no comparison, use default comparison and prefix. */
		if (b.compare == NULL) {
			b.compare = __bt_defcmp;
			if (b.prefix == NULL)
				b.prefix = __bt_defpfx;
		}

		if (b.lorder == 0)
			b.lorder = machine_lorder;
	} else {
		b.compare = __bt_defcmp;
		b.cachesize = 0;
		b.flags = 0;
		b.lorder = machine_lorder;
		b.minkeypage = DEFMINKEYPAGE;
		b.prefix = __bt_defpfx;
		b.psize = 0;
	}

	/* Check for the ubiquitous PDP-11. */
	if (b.lorder != BIG_ENDIAN && b.lorder != LITTLE_ENDIAN)
		goto einval;

	/* Allocate and initialize DB and BTREE structures. */
	if ((t = (BTREE *)calloc(1, sizeof(BTREE))) == NULL)
		goto err;
	t->bt_fd = -1;			/* Don't close unopened fd on error. */
	t->bt_lorder = b.lorder;
	t->bt_order = NOT;
	t->bt_cmp = b.compare;
	t->bt_pfx = b.prefix;
	t->bt_rfd = -1;

	if ((t->bt_dbp = dbp = (DB *)calloc(1, sizeof(DB))) == NULL)
		goto err;
	if (t->bt_lorder != machine_lorder)
		F_SET(t, B_NEEDSWAP);

	dbp->type = DB_BTREE;
	dbp->internal = t;
	dbp->close = __bt_close;
	dbp->del = ref___bt_delete;
	dbp->fd = ref___bt_fd;
	dbp->get = __bt_get;
	dbp->put = ref___bt_put;
	dbp->seq = ref___bt_seq;
	dbp->sync = __bt_sync;

	/*
	 * If no file name was supplied, this is an in-memory btree and we
	 * open a backing temporary file.  Otherwise, it's a disk-based tree.
	 */
	if (fname) {
		switch (flags & O_ACCMODE) {
		case O_RDONLY:
			F_SET(t, B_RDONLY);
			break;
		case O_RDWR:
			break;
		case O_WRONLY:
		default:
			goto einval;
		}

		if ((t->bt_fd = _open(fname, flags | O_CLOEXEC, mode)) < 0)
			goto err;

	} else {
		if ((flags & O_ACCMODE) != O_RDWR)
			goto einval;
		if ((t->bt_fd = ref_tmp()) == -1)
			goto err;
		F_SET(t, B_INMEM);
	}

	if (_fstat(t->bt_fd, &sb))
		goto err;
	if (sb.st_size) {
		if ((nr = _read(t->bt_fd, &m, sizeof(BTMETA))) < 0)
			goto err;
		if (nr != sizeof(BTMETA))
			goto eftype;

		/*
		 * Read in the meta-data.  This can change the notion of what
		 * the lorder, page size and flags are, and, when the page size
		 * changes, the cachesize value can change too.  If the user
		 * specified the wrong byte order for an existing database, we
		 * don't bother to return an error, we just clear the NEEDSWAP
		 * bit.
		 */
		if (m.magic == BTREEMAGIC)
			F_CLR(t, B_NEEDSWAP);
		else {
			F_SET(t, B_NEEDSWAP);
			M_32_SWAP(m.magic);
			M_32_SWAP(m.version);
			M_32_SWAP(m.psize);
			M_32_SWAP(m.free);
			M_32_SWAP(m.nrecs);
			M_32_SWAP(m.flags);
		}
		if (m.magic != BTREEMAGIC || m.version != BTREEVERSION)
			goto eftype;
		if (m.psize < MINPSIZE || m.psize > MAX_PAGE_OFFSET + 1 ||
		    m.psize & (sizeof(indx_t) - 1) )
			goto eftype;
		if (m.flags & ~SAVEMETA)
			goto eftype;
		b.psize = m.psize;
		F_SET(t, m.flags);
		t->bt_free = m.free;
		t->bt_nrecs = m.nrecs;
	} else {
		/*
		 * Set the page size to the best value for I/O to this file.
		 * Don't overflow the page offset type.
		 */
		if (b.psize == 0) {
			b.psize = sb.st_blksize;
			if (b.psize < MINPSIZE)
				b.psize = MINPSIZE;
			if (b.psize > MAX_PAGE_OFFSET + 1)
				b.psize = MAX_PAGE_OFFSET + 1;
		}

		/* Set flag if duplicates permitted. */
		if (!(b.flags & R_DUP))
			F_SET(t, B_NODUPS);

		t->bt_free = P_INVALID;
		t->bt_nrecs = 0;
		F_SET(t, B_METADIRTY);
	}

	t->bt_psize = b.psize;

	/* Set the cache size; must be a multiple of the page size. */
	if (b.cachesize && b.cachesize & (b.psize - 1) )
		b.cachesize += (~b.cachesize & (b.psize - 1) ) + 1;
	if (b.cachesize < b.psize * MINCACHE)
		b.cachesize = b.psize * MINCACHE;

	/* Calculate number of pages to cache. */
	ncache = howmany(b.cachesize, t->bt_psize);

	/*
	 * The btree data structure requires that at least two keys can fit on
	 * a page, but other than that there's no fixed requirement.  The user
	 * specified a minimum number per page, and we translated that into the
	 * number of bytes a key/data pair can use before being placed on an
	 * overflow page.  This calculation includes the page header, the size
	 * of the index referencing the leaf item and the size of the leaf item
	 * structure.  Also, don't let the user specify a minkeypage such that
	 * a key/data pair won't fit even if both key and data are on overflow
	 * pages.
	 */
	t->bt_ovflsize = (t->bt_psize - BTDATAOFF) / b.minkeypage -
	    (sizeof(indx_t) + NBLEAFDBT(0, 0));
	if (t->bt_ovflsize < NBLEAFDBT(NOVFLSIZE, NOVFLSIZE) + sizeof(indx_t))
		t->bt_ovflsize =
		    NBLEAFDBT(NOVFLSIZE, NOVFLSIZE) + sizeof(indx_t);

	/* Initialize the buffer pool. */
	if ((t->bt_mp =
	    mpool_open(NULL, t->bt_fd, t->bt_psize, ncache)) == NULL)
		goto err;
	if (!F_ISSET(t, B_INMEM))
		mpool_filter(t->bt_mp, __bt_pgin, __bt_pgout, t);

	/* Create a root page if new tree. */
	if (ref_nroot(t) == RET_ERROR)
		goto err;

	/* Global flags. */
	if (dflags & DB_LOCK)
		F_SET(t, B_DB_LOCK);
	if (dflags & DB_SHMEM)
		F_SET(t, B_DB_SHMEM);
	if (dflags & DB_TXN)
		F_SET(t, B_DB_TXN);

	return (dbp);

einval:	errno = EINVAL;
	goto err;

eftype:	errno = EFTYPE;
	goto err;

err:	saved_errno = errno;
	if (t) {
		if (t->bt_dbp)
			free(t->bt_dbp);
		if (t->bt_fd != -1)
			(void)_close(t->bt_fd);
		free(t);
	}
	errno = saved_errno;
	return (NULL);
}

/*
 * NROOT -- Create the root of a new tree.
 *
 * Parameters:
 *	t:	tree
 *
 * Returns:
 *	RET_ERROR, RET_SUCCESS
 */
int
ref_nroot(BTREE *t)
{
	PAGE *meta, *root;
	pgno_t npg;

	if ((root = mpool_get(t->bt_mp, 1, 0)) != NULL) {
		if (root->lower == 0 &&
		    root->pgno == 0 &&
		    root->linp[0] == 0) {
			mpool_delete(t->bt_mp, root);
			errno = EINVAL;
		} else {
			mpool_put(t->bt_mp, root, 0);
			return (RET_SUCCESS);
		}
	}
	if (errno != EINVAL)		/* It's OK to not exist. */
		return (RET_ERROR);
	errno = 0;

	if ((meta = mpool_new(t->bt_mp, &npg, MPOOL_PAGE_NEXT)) == NULL)
		return (RET_ERROR);

	if ((root = mpool_new(t->bt_mp, &npg, MPOOL_PAGE_NEXT)) == NULL)
		return (RET_ERROR);

	if (npg != P_ROOT)
		return (RET_ERROR);
	root->pgno = npg;
	root->prevpg = root->nextpg = P_INVALID;
	root->lower = BTDATAOFF;
	root->upper = t->bt_psize;
	root->flags = P_BLEAF;
	memset(meta, 0, t->bt_psize);
	mpool_put(t->bt_mp, meta, MPOOL_DIRTY);
	mpool_put(t->bt_mp, root, MPOOL_DIRTY);
	return (RET_SUCCESS);
}

int
ref_tmp(void)
{
	sigset_t set, oset;
	int fd, len;
	char *envtmp;
	char path[MAXPATHLEN];

	envtmp = secure_getenv("TMPDIR");
	len = snprintf(path,
	    sizeof(path), "%s/bt.XXXXXXXXXX", envtmp ? envtmp : "/tmp");
	if (len < 0 || len >= (int)sizeof(path)) {
		errno = ENAMETOOLONG;
		return(-1);
	}

	(void)sigfillset(&set);
	(void)__libc_sigprocmask(SIG_BLOCK, &set, &oset);
	if ((fd = mkostemp(path, O_CLOEXEC)) != -1)
		(void)unlink(path);
	(void)__libc_sigprocmask(SIG_SETMASK, &oset, NULL);
	return(fd);
}

int
ref_byteorder(void)
{
	u_int32_t x;
	u_char *p;

	x = 0x01020304;
	p = (u_char *)&x;
	switch (*p) {
	case 1:
		return (BIG_ENDIAN);
	case 4:
		return (LITTLE_ENDIAN);
	default:
		return (0);
	}
}

int
ref___bt_fd(const DB *dbp)
{
	BTREE *t;

	t = dbp->internal;

	/* Toss any page pinned across calls. */
	if (t->bt_pinned != NULL) {
		mpool_put(t->bt_mp, t->bt_pinned, 0);
		t->bt_pinned = NULL;
	}

	/* In-memory database can't have a file descriptor. */
	if (F_ISSET(t, B_INMEM)) {
		errno = ENOENT;
		return (-1);
	}
	return (t->bt_fd);
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

