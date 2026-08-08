#!/usr/bin/env python3
"""Generate oracle.c and port.cppm for batch b0155s2 (bt_open.c only)."""
import os
import re

ROOT = "/home/odin/pbsd"
SRC = os.path.join(ROOT, "hbsd/src/lib/libc/db/btree/bt_open.c")
OUT = os.path.join(ROOT, "pbsd/lib/libc/db/btree/b0155s2")

BATCH_FUNCS = ["__bt_open", "nroot", "tmp", "byteorder", "__bt_fd"]

ORACLE_HEADER = r'''/*
 * b0155s2 oracle -- the specification.
 *
 * Original HardenedBSD C source lib/libc/db/btree/bt_open.c concatenated
 * verbatim.  Every function is renamed with a ref_ prefix; no function body
 * has been altered.
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

typedef struct _bleaf {
	u_int32_t	ksize;
	u_int32_t	dsize;
	u_char	flags;
	char	bytes[1];
} BLEAF;

#define NBLEAFDBT(ksize, dsize)						\
	LALIGN(sizeof(u_int32_t) + sizeof(u_int32_t) + sizeof(u_char) +	\
	    (ksize) + (dsize))

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

enum { NOT, BACK, FORWARD };

#define MAX_POOL 64

typedef struct {
	unsigned get_calls, put_calls, new_calls, delete_calls;
	unsigned open_calls, filter_calls;
	unsigned open_fd_calls, fstat_calls, read_calls, close_calls;
	unsigned mkostemp_calls, getenv_calls, sigmask_calls, unlink_calls;
	unsigned calloc_calls;
	int get_force_null, new_force_null;
	int open_ret, fstat_ret, read_ret, close_ret, mkostemp_ret;
	int calloc_fail_after;
	pgno_t get_last_pgno, new_pgno_seq;
	unsigned get_last_flags, last_put_flags;
	void *last_put_page;
	int nreg;
	pgno_t reg_pgno[MAX_POOL];
	void *reg_page[MAX_POOL];
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

int __bt_close(DB *dbp) { (void)dbp; return RET_SUCCESS; }
int __bt_delete(const DB *dbp, const DBT *k, unsigned int f)
	{ (void)dbp; (void)k; (void)f; return RET_SUCCESS; }
int __bt_get(const DB *dbp, const DBT *k, DBT *d, unsigned int f)
	{ (void)dbp; (void)k; (void)d; (void)f; return RET_SUCCESS; }
int __bt_put(DB *dbp, DBT *k, const DBT *d, unsigned int f)
	{ (void)dbp; (void)k; (void)d; (void)f; return RET_SUCCESS; }
int __bt_seq(const DB *dbp, DBT *k, DBT *d, unsigned int f)
	{ (void)dbp; (void)k; (void)d; (void)f; return RET_SUCCESS; }
int __bt_sync(const DB *dbp, unsigned int f) { (void)dbp; (void)f; return RET_SUCCESS; }
int __bt_defcmp(const DBT *a, const DBT *b) { (void)a; (void)b; return 0; }
size_t __bt_defpfx(const DBT *a, const DBT *b) { (void)a; (void)b; return 0; }
void __bt_pgin(void *a, pgno_t b, void *c) { (void)a; (void)b; (void)c; }
void __bt_pgout(void *a, pgno_t b, void *c) { (void)a; (void)b; (void)c; }

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

export module pbsd.lib.libc.db.btree.b0155s2;

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

#undef BIG_ENDIAN
#undef LITTLE_ENDIAN
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

enum { NOT, BACK, FORWARD };

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

#define NBLEAFDBT(ksize, dsize)						\
	LALIGN(sizeof(u_int32_t) + sizeof(u_int32_t) + sizeof(u_char) +	\
	    (ksize) + (dsize))

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

extern "C" {
void *mpool_get(MPOOL *, pgno_t, unsigned int);
int mpool_put(MPOOL *, void *, unsigned int);
void *mpool_new(MPOOL *, pgno_t *, unsigned int);
int mpool_delete(MPOOL *, void *);
MPOOL *mpool_open(void *, int, u_int32_t, pgno_t);
void mpool_filter(MPOOL *, void (*)(void *, pgno_t, void *), void (*)(void *, pgno_t, void *), void *);
int __bt_close(DB *);
int __bt_delete(const DB *, const DBT *, unsigned int);
int __bt_get(const DB *, const DBT *, DBT *, unsigned int);
int __bt_put(DB *, DBT *, const DBT *, unsigned int);
int __bt_seq(const DB *, DBT *, DBT *, unsigned int);
int __bt_sync(const DB *, unsigned int);
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

export namespace pbsd::lib_libc_db_btree::b0155s2 {

using BTREE = ::BTREE;
using PAGE = ::PAGE;
using DB = ::DB;
using DBT = ::DBT;
using BTREEINFO = ::BTREEINFO;
using BTMETA = ::BTMETA;
using pgno_t = ::pgno_t;
using indx_t = ::indx_t;
using MPOOL = ::MPOOL;

DB	*__bt_open(const char *, int, int, const BTREEINFO *, int);
int	nroot(BTREE *);
int	tmp(void);
int	byteorder(void);
int	__bt_fd(const DB *);

'''

PORT_FOOTER = r'''
} // namespace pbsd::lib_libc_db_btree::b0155s2
'''


def extract_functions(text):
    lines = text.split('\n')
    out = []
    i = 0
    while i < len(lines):
        line = lines[i]
        if line.startswith('#include'):
            i += 1
            continue
        if line.startswith('static ') and ';' in line and '(' in line:
            i += 1
            continue
        out.append(line)
        i += 1
    return '\n'.join(out)


def rename_functions(text, for_oracle=True):
    result = re.sub(r'#ifdef STATISTICS.*?#endif\n', '', text, flags=re.DOTALL)
    for fn in sorted(BATCH_FUNCS, key=len, reverse=True):
        ref = f"ref_{fn}" if for_oracle else fn
        patterns = [
            (rf'\bint\s*\n{re.escape(fn)}\s*\(', f'int\n{ref}('),
            (rf'\bDB\s*\*\s*\n{re.escape(fn)}\s*\(', f'DB *\n{ref}('),
            (rf'\bstatic\s+int\s*\n{re.escape(fn)}\s*\(', f'static int\n{ref}('),
        ]
        for pat, repl in patterns:
            result = re.sub(pat, repl, result)
    if for_oracle:
        for fn in sorted(BATCH_FUNCS, key=len, reverse=True):
            ref = f"ref_{fn}"
            result = re.sub(rf'\b{re.escape(fn)}\s*\(', f'{ref}(', result)
        result = result.replace('ref_ref_', 'ref_')
        result = re.sub(r'static (int)\n(ref_\w+)', r'\1\n\2', result)
        for fn in sorted(BATCH_FUNCS, key=len, reverse=True):
            result = re.sub(rf'=\s*{re.escape(fn)}\s*;', f'= {ref};', result)
            result = re.sub(rf'=\s*{re.escape(fn)}\s*,', f'= {ref},', result)
    return result


def port_cpp_fixup(text):
    text = re.sub(r'\bt = dbp->internal;', 't = (BTREE *)dbp->internal;', text)
    text = re.sub(r'\(root = mpool_get\(', '(root = (PAGE *)mpool_get(', text)
    text = re.sub(r'\(meta = mpool_new\(', '(meta = (PAGE *)mpool_new(', text)
    text = re.sub(r'\(root = mpool_new\(', '(root = (PAGE *)mpool_new(', text)
    return text


def main():
    with open(SRC) as f:
        src = f.read()

    oracle_body = rename_functions(extract_functions(src), for_oracle=True)
    port_body = port_cpp_fixup(rename_functions(extract_functions(src), for_oracle=False))
    port_body = re.sub(r'\bstatic\s+(?=int\n)', '', port_body)

    oracle_out = ORACLE_HEADER + oracle_body
    port_out = PORT_HEADER + port_body + PORT_FOOTER

    with open(os.path.join(OUT, "oracle.c"), "w") as f:
        f.write(oracle_out)
    with open(os.path.join(OUT, "port.cppm"), "w") as f:
        f.write(port_out)
    print("Generated oracle.c and port.cppm")


if __name__ == "__main__":
    main()
