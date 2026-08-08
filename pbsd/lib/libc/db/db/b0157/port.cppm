module;

#include <sys/types.h>

#include <cerrno>
#include <fcntl.h>
#include <climits>
#include <cstddef>
#include <cstdio>

export module pbsd.lib.libc.db.db.b0157;

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#define	RET_ERROR	-1
#define	RET_SUCCESS	 0
#define	RET_SPECIAL	 1

typedef struct {
	void	*data;
	size_t	 size;
} DBT;

typedef enum { DB_BTREE, DB_HASH, DB_RECNO } DBTYPE;

#if UINT_MAX > 65535
#define	DB_LOCK		0x20000000
#define	DB_SHMEM	0x40000000
#define	DB_TXN		0x80000000
#else
#define	DB_LOCK		    0x2000
#define	DB_SHMEM	    0x4000
#define	DB_TXN		    0x8000
#endif

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
	unsigned long	flags;
	unsigned int	cachesize;
	int		maxkeypage;
	int		minkeypage;
	unsigned int	psize;
	int		(*compare)(const DBT *, const DBT *);
	size_t		(*prefix)(const DBT *, const DBT *);
	int		lorder;
} BTREEINFO;

typedef struct {
	unsigned int	bsize;
	unsigned int	ffactor;
	unsigned int	nelem;
	unsigned int	cachesize;
	uint32_t	(*hash)(const void *, size_t);
	int	lorder;
} HASHINFO;

typedef struct {
	unsigned long	flags;
	unsigned int	cachesize;
	unsigned int	psize;
	int		lorder;
	size_t		reclen;
	unsigned char	bval;
	char	*bfname;
} RECNOINFO;

extern "C" {
DB *__bt_open(const char *, int, int, const BTREEINFO *, int);
DB *__hash_open(const char *, int, int, const HASHINFO *, int);
DB *__rec_open(const char *, int, int, const RECNOINFO *, int);
}

export namespace pbsd::lib_libc_db_db::b0157 {

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1991, 1993
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

static int __dberr(void);

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#ifndef O_EXLOCK
#define O_EXLOCK 0
#endif
#ifndef O_SHLOCK
#define O_SHLOCK 0
#endif

DB *
dbopen(const char *fname, int flags, int mode, DBTYPE type, const void *openinfo)
{

#define	DB_FLAGS	(DB_LOCK | DB_SHMEM | DB_TXN)
#define	USE_OPEN_FLAGS							\
	(O_CREAT | O_EXCL | O_EXLOCK | O_NOFOLLOW | O_NONBLOCK | 	\
	 O_RDONLY | O_RDWR | O_SHLOCK | O_SYNC | O_TRUNC | O_CLOEXEC)

	if ((flags & ~(USE_OPEN_FLAGS | DB_FLAGS)) == 0)
		switch (type) {
		case DB_BTREE:
			return (__bt_open(fname, flags & USE_OPEN_FLAGS,
			    mode, openinfo, flags & DB_FLAGS));
		case DB_HASH:
			return (__hash_open(fname, flags & USE_OPEN_FLAGS,
			    mode, openinfo, flags & DB_FLAGS));
		case DB_RECNO:
			return (__rec_open(fname, flags & USE_OPEN_FLAGS,
			    mode, openinfo, flags & DB_FLAGS));
		}
	errno = EINVAL;
	return (NULL);
}

int
cfi_libc_db_dbclose(DB *dbp)
{

	if (dbp == NULL) {
		errno = EINVAL;
		return (RET_ERROR);
	}

	return (dbp->close(dbp));
}

int
cfi_libc_db_dbdel(const DB *dbp, const DBT *key, unsigned int flags)
{

	if (dbp == NULL) {
		errno = EINVAL;
		return (RET_ERROR);
	}

	return (dbp->del(dbp, key, flags));
}

int
cfi_libc_db_dbget(const DB *dbp, const DBT *key, DBT *data, unsigned int flags)
{

	if (dbp == NULL) {
		errno = EINVAL;
		return (RET_ERROR);
	}

	return (dbp->get(dbp, key, data, flags));
}

int
cfi_libc_db_dbput(const DB *dbp, DBT *key, const DBT *data, unsigned int flags)
{

	if (dbp == NULL) {
		errno = EINVAL;
		return (RET_ERROR);
	}

	return (dbp->put(dbp, key, data, flags));
}

int
cfi_libc_db_dbseq(const DB *dbp, DBT *key, DBT *data, unsigned int flags)
{

	if (dbp == NULL) {
		errno = EINVAL;
		return (RET_ERROR);
	}

	return (dbp->seq(dbp, key, data, flags));
}

int
cfi_libc_db_dbsync(const DB *dbp, unsigned int flags)
{

	if (dbp == NULL) {
		errno = EINVAL;
		return (RET_ERROR);
	}

	return (dbp->sync(dbp, flags));
}

int
cfi_libc_db_dbfd(const DB *dbp)
{

	if (dbp == NULL) {
		errno = EINVAL;
		return (RET_ERROR);
	}

	return (dbp->fd(dbp));
}

static int
__dberr(void)
{
	return (RET_ERROR);
}

void
__dbpanic(DB *dbp)
{
	/* The only thing that can succeed is a close. */
	dbp->del = (int (*)(const struct __db *, const DBT*, u_int))__dberr;
	dbp->fd = (int (*)(const struct __db *))__dberr;
	dbp->get = (int (*)(const struct __db *, const DBT*, DBT *, u_int))__dberr;
	dbp->put = (int (*)(const struct __db *, DBT *, const DBT *, u_int))__dberr;
	dbp->seq = (int (*)(const struct __db *, DBT *, DBT *, u_int))__dberr;
	dbp->sync = (int (*)(const struct __db *, u_int))__dberr;
}

}  // namespace pbsd::lib_libc_db_db::b0157
