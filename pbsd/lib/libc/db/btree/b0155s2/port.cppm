module;

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
__bt_open(const char *fname, int flags, int mode, const BTREEINFO *openinfo, int dflags)
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
	machine_lorder = byteorder();
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
	dbp->del = __bt_delete;
	dbp->fd = __bt_fd;
	dbp->get = __bt_get;
	dbp->put = __bt_put;
	dbp->seq = __bt_seq;
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
		if ((t->bt_fd = tmp()) == -1)
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
	if (nroot(t) == RET_ERROR)
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
nroot(BTREE *t)
{
	PAGE *meta, *root;
	pgno_t npg;

	if ((root = (PAGE *)mpool_get(t->bt_mp, 1, 0)) != NULL) {
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

	if ((meta = (PAGE *)mpool_new(t->bt_mp, &npg, MPOOL_PAGE_NEXT)) == NULL)
		return (RET_ERROR);

	if ((root = (PAGE *)mpool_new(t->bt_mp, &npg, MPOOL_PAGE_NEXT)) == NULL)
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
tmp(void)
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
byteorder(void)
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
__bt_fd(const DB *dbp)
{
	BTREE *t;

	t = (BTREE *)dbp->internal;

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

} // namespace pbsd::lib_libc_db_btree::b0155s2
