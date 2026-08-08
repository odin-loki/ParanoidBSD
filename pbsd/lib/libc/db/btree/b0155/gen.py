#!/usr/bin/env python3
"""Generate oracle.c and port.cppm from HBSD sources for batch b0155."""
import re
import os

ROOT = "/home/odin/pbsd"
SRC = [
    "hbsd/src/lib/libc/db/btree/bt_put.c",
    "hbsd/src/lib/libc/db/btree/bt_open.c",
    "hbsd/src/lib/libc/db/btree/bt_seq.c",
    "hbsd/src/lib/libc/db/btree/bt_delete.c",
]

# Static functions per file that need ref_ prefix
STATIC_RENAMES = {
    "bt_fast": "ref_bt_fast",
    "nroot": "ref_nroot",
    "tmp": "ref_tmp",
    "byteorder": "ref_byteorder",
    "__bt_seqset": "ref___bt_seqset",
    "__bt_seqadv": "ref___bt_seqadv",
    "__bt_first": "ref___bt_first",
    "__bt_stkacq": "ref___bt_stkacq",
    "__bt_bdelete": "ref___bt_bdelete",
    "__bt_pdelete": "ref___bt_pdelete",
    "__bt_curdel": "ref___bt_curdel",
    "__bt_relink": "ref___bt_relink",
}

# All exported functions in batch (public + static)
BATCH_FUNCS = [
    "__bt_put", "bt_fast",
    "__bt_open", "nroot", "tmp", "byteorder", "__bt_fd",
    "__bt_seq", "__bt_seqset", "__bt_seqadv", "__bt_first", "__bt_setcur",
    "__bt_delete", "__bt_stkacq", "__bt_bdelete", "__bt_pdelete",
    "__bt_dleaf", "__bt_curdel", "__bt_relink",
]

ORACLE_HEADER = r'''/*
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

'''

PORT_HEADER = r'''module;

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

export module pbsd.lib.libc.db.btree.b0155;

#include <sys/types.h>

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

struct MPOOL { int opaque; };

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
void *mpool_new(MPOOL *, pgno_t *, unsigned int);
int mpool_delete(MPOOL *, void *);
MPOOL *mpool_open(void *, int, u_int32_t, pgno_t);
void mpool_filter(MPOOL *, void *, void *, void *);
EPG *__bt_search(BTREE *, const DBT *, int *);
int __bt_cmp(BTREE *, const DBT *, EPG *);
int __bt_split(BTREE *, PAGE *, const DBT *, const DBT *, int, u_int32_t, indx_t);
int __ovfl_put(BTREE *, const DBT *, pgno_t *);
int __ovfl_delete(BTREE *, void *);
int __bt_ret(BTREE *, EPG *, DBT *, DBT *, DBT *, DBT *, int);
int __bt_free(BTREE *, PAGE *);
int __bt_close(DB *);
int __bt_get(const DB *, const DBT *, DBT *, u_int);
int __bt_sync(const DB *, u_int);
int __bt_defcmp(const DBT *, const DBT *);
size_t __bt_defpfx(const DBT *, const DBT *);
void __bt_pgin(void *, pgno_t, void *);
void __bt_pgout(void *, pgno_t, void *);
int _open(const char *, int, ...);
int _fstat(int, struct stat *);
ssize_t _read(int, void *, size_t);
int _close(int);
char *secure_getenv(const char *);
int __libc_sigprocmask(int, const sigset_t *, sigset_t *);
int mkostemp(char *, int);
}

export namespace pbsd::lib_libc_db_btree::b0155 {

using BTREE = ::BTREE;
using PAGE = ::PAGE;
using DB = ::DB;
using DBT = ::DBT;
using EPG = ::EPG;
using CURSOR = ::CURSOR;
using BTREEINFO = ::BTREEINFO;
using BTMETA = ::BTMETA;
using pgno_t = ::pgno_t;
using indx_t = ::indx_t;
using MPOOL = ::MPOOL;

'''

PORT_FOOTER = r'''
} // namespace pbsd::lib_libc_db_btree::b0155
'''

def extract_functions(text):
    """Strip includes and static forward decls, keep copyright blocks and functions."""
    lines = text.split('\n')
    out = []
    i = 0
    while i < len(lines):
        line = lines[i]
        if line.startswith('#include') or line.startswith('#ifdef STATISTICS'):
            # skip until endif for STATISTICS
            if '#ifdef STATISTICS' in line:
                while i < len(lines) and '#endif' not in lines[i]:
                    i += 1
            i += 1
            continue
        if line.startswith('static ') and ';' in line and '(' in line and ')' in line:
            # forward decl
            i += 1
            continue
        out.append(line)
        i += 1
    return '\n'.join(out)

def rename_functions(text, for_oracle=True):
    """Rename function definitions and internal batch calls."""
    result = text
    # Remove STATISTICS blocks
    result = re.sub(r'#ifdef STATISTICS.*?#endif\n', '', result, flags=re.DOTALL)

    all_funcs = list(BATCH_FUNCS)
    # Rename definitions: int\n__bt_put( -> int\nref___bt_put(
    for fn in sorted(all_funcs, key=len, reverse=True):
        ref = f"ref_{fn}" if for_oracle else fn
        orig = fn
        # function definition patterns
        patterns = [
            (rf'\bint\s*\n{re.escape(orig)}\s*\(', f'int\n{ref}('),
            (rf'\bvoid\s*\n{re.escape(orig)}\s*\(', f'void\n{ref}('),
            (rf'\bDB\s*\*\s*\n{re.escape(orig)}\s*\(', f'DB *\n{ref}('),
            (rf'\bstatic\s+int\s*\n{re.escape(orig)}\s*\(', f'static int\n{ref}('),
            (rf'\bstatic\s+void\s*\n{re.escape(orig)}\s*\(', f'static void\n{ref}('),
            (rf'\bstatic\s+EPG\s*\*\s*\n{re.escape(orig)}\s*\(', f'static EPG *\n{ref}('),
            (rf'\bstatic\s+EPG\s*\n{re.escape(orig)}\s*\(', f'static EPG\n{ref}('),
            (rf'\bEPG\s*\*\s*\n{re.escape(orig)}\s*\(', f'EPG *\n{ref}('),
        ]
        for pat, repl in patterns:
            result = re.sub(pat, repl, result)

    # Rename internal calls within batch
    for fn in sorted(all_funcs, key=len, reverse=True):
        ref = f"ref_{fn}" if for_oracle else fn
        if for_oracle:
            # calls to batch functions
            result = re.sub(rf'\b{re.escape(fn)}\s*\(', f'{ref}(', result)
        else:
            pass  # port keeps original names

    if for_oracle:
        # Fix double ref_ from definition renames
        result = result.replace('ref_ref_', 'ref_')
        # static keyword on renamed static funcs - remove static for oracle exports
        result = re.sub(r'static (int|void|EPG \*)\n(ref_\w+)', r'\1\n\2', result)
        # function pointer / bare references
        for fn in sorted(all_funcs, key=len, reverse=True):
            ref = f"ref_{fn}"
            result = re.sub(rf'=\s*{re.escape(fn)}\s*;', f'= {ref};', result)
            result = re.sub(rf'=\s*{re.escape(fn)}\s*,', f'= {ref},', result)
            result = re.sub(rf'\bgoto\s+{re.escape(fn)}\s*;', f'goto {ref};', result)

    return result

def port_transform(text):
    text = extract_functions(text)
    text = rename_functions(text, for_oracle=False)
    text = re.sub(r'\bstatic\s+(?=EPG \*?\n)', '', text)
    text = re.sub(r'\bstatic\s+(?=int\n)', '', text)
    return text

def oracle_transform(text):
    text = extract_functions(text)
    text = rename_functions(text, for_oracle=True)
    return text

def main():
    oracle_body = ""
    port_body = ""
    for path in SRC:
        with open(os.path.join(ROOT, path)) as f:
            src = f.read()
        oracle_body += "\n" + oracle_transform(src) + "\n"
        port_body += "\n" + port_transform(src) + "\n"

    # Export declarations for port
    exports = []
    for fn in BATCH_FUNCS:
        if fn == "bt_fast":
            exports.append("EPG\t*bt_fast(BTREE *, const DBT *, const DBT *, int *);")
        elif fn == "__bt_open":
            exports.append("DB\t*__bt_open(const char *, int, int, const BTREEINFO *, int);")
        elif fn == "__bt_fd":
            exports.append("int\t__bt_fd(const DB *);")
        elif fn == "__bt_setcur":
            exports.append("void\t__bt_setcur(BTREE *, pgno_t, u_int);")
        elif fn == "__bt_dleaf":
            exports.append("int\t__bt_dleaf(BTREE *, const DBT *, PAGE *, u_int);")
        else:
            exports.append(f"int\t{fn}(...);  // placeholder")

    # Fix exports properly
    export_lines = [
        "int\t__bt_put(const DB *, DBT *, const DBT *, u_int);",
        "EPG\t*bt_fast(BTREE *, const DBT *, const DBT *, int *);",
        "DB\t*__bt_open(const char *, int, int, const BTREEINFO *, int);",
        "int\tnroot(BTREE *);",
        "int\ttmp(void);",
        "int\tbyteorder(void);",
        "int\t__bt_fd(const DB *);",
        "int\t__bt_seq(const DB *, DBT *, DBT *, u_int);",
        "int\t__bt_seqset(BTREE *, EPG *, DBT *, int);",
        "int\t__bt_seqadv(BTREE *, EPG *, int);",
        "int\t__bt_first(BTREE *, const DBT *, EPG *, int *);",
        "void\t__bt_setcur(BTREE *, pgno_t, u_int);",
        "int\t__bt_delete(const DB *, const DBT *, u_int);",
        "int\t__bt_stkacq(BTREE *, PAGE **, CURSOR *);",
        "int\t__bt_bdelete(BTREE *, const DBT *);",
        "int\t__bt_pdelete(BTREE *, PAGE *);",
        "int\t__bt_dleaf(BTREE *, const DBT *, PAGE *, u_int);",
        "int\t__bt_curdel(BTREE *, const DBT *, PAGE *, u_int);",
        "int\t__bt_relink(BTREE *, PAGE *);",
    ]

    port_out = PORT_HEADER + "\n".join(export_lines) + "\n\n" + port_body + PORT_FOOTER
    oracle_out = ORACLE_HEADER + oracle_body

    # Forward declarations for oracle
    fwd = "/* forward */\n" + "\n".join(
        l.replace('\t', ' ').replace(';', ';') for l in export_lines
    ).replace('nroot', 'ref_nroot').replace('tmp', 'ref_tmp').replace('byteorder', 'ref_byteorder')
    for fn in BATCH_FUNCS:
        pass

    fwd_decls = []
    for line in export_lines:
        fwd_decls.append("ref_" + line.split()[1].split('(')[0] + line[line.index('('):])
    oracle_out = ORACLE_HEADER + "\n".join(
        f.replace('ref_ref_', 'ref_') for f in [
            "EPG *ref_bt_fast(BTREE *, const DBT *, const DBT *, int *);",
            "DB *ref___bt_open(const char *, int, int, const BTREEINFO *, int);",
            "int ref_nroot(BTREE *);",
            "int ref_tmp(void);",
            "int ref_byteorder(void);",
            "int ref___bt_fd(const DB *);",
            "int ref___bt_put(const DB *, DBT *, const DBT *, u_int);",
            "int ref___bt_seq(const DB *, DBT *, DBT *, u_int);",
            "int ref___bt_seqset(BTREE *, EPG *, DBT *, int);",
            "int ref___bt_seqadv(BTREE *, EPG *, int);",
            "int ref___bt_first(BTREE *, const DBT *, EPG *, int *);",
            "void ref___bt_setcur(BTREE *, pgno_t, u_int);",
            "int ref___bt_delete(const DB *, const DBT *, u_int);",
            "int ref___bt_stkacq(BTREE *, PAGE **, CURSOR *);",
            "int ref___bt_bdelete(BTREE *, const DBT *);",
            "int ref___bt_pdelete(BTREE *, PAGE *);",
            "int ref___bt_dleaf(BTREE *, const DBT *, PAGE *, u_int);",
            "int ref___bt_curdel(BTREE *, const DBT *, PAGE *, u_int);",
            "int ref___bt_relink(BTREE *, PAGE *);",
        ]
    ) + "\n\n" + oracle_body

    outdir = os.path.join(ROOT, "pbsd/lib/libc/db/btree/b0155")
    with open(os.path.join(outdir, "oracle.c"), "w") as f:
        f.write(oracle_out)
    with open(os.path.join(outdir, "port.cppm"), "w") as f:
        f.write(port_out)
    print("Generated oracle.c and port.cppm")

if __name__ == "__main__":
    main()
