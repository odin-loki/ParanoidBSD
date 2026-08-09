/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Margo Seltzer.
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
module;

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

export module pbsd.lib.libc.db.hash.b0194s3;

export namespace pbsd::lib_libc_db_hash::b0194s3 {

/*-
 * Shared declarations and mocks for batch b0194s3.
 */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
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
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef howmany
#define howmany(x, y) (((x) + ((y) - 1)) / (y))
#endif
#undef BIG_ENDIAN
#undef LITTLE_ENDIAN
#define BIG_ENDIAN 4321
#define LITTLE_ENDIAN 1234
#if defined(__BYTE_ORDER__)
#define BYTE_ORDER __BYTE_ORDER__
#elif defined(__BYTE_ORDER)
#define BYTE_ORDER __BYTE_ORDER
#else
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#define M_16_SWAP(a) { \
	uint16_t _tmp = (a); \
	((char *)&(a))[0] = ((char *)&_tmp)[1]; \
	((char *)&(a))[1] = ((char *)&_tmp)[0]; \
}
#define M_32_SWAP(a) { \
	uint32_t _tmp = (a); \
	((char *)&(a))[0] = ((char *)&_tmp)[3]; \
	((char *)&(a))[1] = ((char *)&_tmp)[2]; \
	((char *)&(a))[2] = ((char *)&_tmp)[1]; \
	((char *)&(a))[3] = ((char *)&_tmp)[0]; \
}
#define P_32_COPY(a, b) { \
	((char *)&(b))[0] = ((char *)&(a))[3]; \
	((char *)&(b))[1] = ((char *)&(a))[2]; \
	((char *)&(b))[2] = ((char *)&(a))[1]; \
	((char *)&(b))[3] = ((char *)&(a))[0]; \
}
#define P_16_COPY(a, b) { \
	((char *)&(b))[0] = ((char *)&(a))[1]; \
	((char *)&(b))[1] = ((char *)&(a))[0]; \
}

typedef unsigned char u_char;
typedef unsigned int u_int;
typedef uint32_t u_int32_t;
typedef uint16_t u_int16_t;
typedef int32_t int32_t;

typedef struct {
	void *data;
	size_t size;
} DBT;

typedef enum {
	HASH_GET, HASH_PUT, HASH_PUTNEW, HASH_DELETE, HASH_FIRST, HASH_NEXT
} ACTION;

typedef struct _bufhead BUFHEAD;

struct _bufhead {
	BUFHEAD *prev;
	BUFHEAD *next;
	BUFHEAD *ovfl;
	u_int32_t addr;
	char *page;
	char flags;
};

#define BUF_MOD 0x0001
#define BUF_DISK 0x0002
#define BUF_BUCKET 0x0004
#define BUF_PIN 0x0008

typedef BUFHEAD **SEGMENT;

#define NCACHED 32

typedef struct hashhdr {
	int32_t magic;
	int32_t version;
	u_int32_t lorder;
	int32_t bsize;
	int32_t bshift;
	int32_t dsize;
	int32_t ssize;
	int32_t sshift;
	int32_t ovfl_point;
	int32_t last_freed;
	u_int32_t max_bucket;
	u_int32_t high_mask;
	u_int32_t low_mask;
	u_int32_t ffactor;
	int32_t nkeys;
	int32_t hdrpages;
	int32_t h_charkey;
	int32_t spares[NCACHED];
	u_int16_t bitmaps[NCACHED];
} HASHHDR;

typedef struct htab {
	HASHHDR hdr;
	int nsegs;
	int exsegs;
	u_int32_t (*hash)(const void *, size_t);
	int flags;
	int fp;
	char *tmp_buf;
	char *tmp_key;
	BUFHEAD *cpage;
	int cbucket;
	int cndx;
	int error;
	int new_file;
	int save_file;
	u_int32_t *mapp[NCACHED];
	int nmaps;
	int nbufs;
	BUFHEAD bufhead;
	SEGMENT *dir;
} HTAB;

#define BSIZE hdr.bsize
#define BSHIFT hdr.bshift
#define DSIZE hdr.dsize
#define SGSIZE hdr.ssize
#define SSHIFT hdr.sshift
#define LORDER hdr.lorder
#define OVFL_POINT hdr.ovfl_point
#define LAST_FREED hdr.last_freed
#define MAX_BUCKET hdr.max_bucket
#define FFACTOR hdr.ffactor
#define HIGH_MASK hdr.high_mask
#define LOW_MASK hdr.low_mask
#define NKEYS hdr.nkeys
#define HDRPAGES hdr.hdrpages
#define SPARES hdr.spares
#define BITMAPS hdr.bitmaps
#define VERSION hdr.version
#define MAGIC hdr.magic
#define H_CHARKEY hdr.h_charkey

#define HASHMAGIC 0x061561
#define HASHVERSION 2
#define CHARKEY "%$sniglet^&"
#define MAX_BSIZE 32768
#define MINHDRSIZE 512
#define DEF_BUFSIZE 65536
#define DEF_BUCKET_SIZE 4096
#define DEF_BUCKET_SHIFT 12
#define DEF_SEGSIZE 256
#define DEF_SEGSIZE_SHIFT 8
#define DEF_DIRSIZE 256
#define DEF_FFACTOR 65536
#define MIN_FFACTOR 4
#define BYTE_SHIFT 3

#define SPLITSHIFT 11
#define SPLITMASK 0x7FF
#define SPLITNUM(N) (((u_int32_t)(N)) >> SPLITSHIFT)
#define OPAGENUM(N) ((N) & SPLITMASK)
#define OADDR_OF(S,O) ((u_int32_t)((u_int32_t)(S) << SPLITSHIFT) + (O))

#define OVFLPAGE 0
#define PARTIAL_KEY 1
#define FULL_KEY 2
#define FULL_KEY_DATA 3
#define REAL_KEY 4

#define PAIRSIZE(K,D) (2*sizeof(u_int16_t) + (K)->size + (D)->size)
#define BIGOVERHEAD (4*sizeof(u_int16_t))
#define OVFLSIZE (2*sizeof(u_int16_t))
#define FREESPACE(P) ((P)[(P)[0]+1])
#define OFFSET(P) ((P)[(P)[0]+2])
#define PAIRFITS(P,K,D) \
	(((P)[2] >= REAL_KEY) && \
	    (PAIRSIZE((K),(D)) + OVFLSIZE) <= FREESPACE((P)))
#define PAGE_META(N) (((N)+3) * sizeof(u_int16_t))

#define R_CURSOR 1
#define R_FIRST 3
#define R_NEXT 7
#define R_NOOVERWRITE 8

typedef enum { DB_BTREE, DB_HASH, DB_RECNO } DBTYPE;

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
	unsigned int bsize;
	unsigned int ffactor;
	unsigned int nelem;
	unsigned int cachesize;
	u_int32_t (*hash)(const void *, size_t);
	int lorder;
} HASHINFO;

#define _open open
#define _close close
#define _read read
#define _fstat fstat
#define _fsync fsync

extern "C" {
extern u_int32_t (*__default_hash)(const void *, size_t);
u_int32_t __log2(u_int32_t);
int __buf_init(HTAB *, int);
int __buf_free(HTAB *, int, int);
int __addel(HTAB *, BUFHEAD *, const DBT *, const DBT *);
int __delpair(HTAB *, BUFHEAD *, int);
int __find_bigpair(HTAB *, BUFHEAD *, int, char *, int);
u_int16_t __find_last_page(HTAB *, BUFHEAD **);
int __big_return(HTAB *, BUFHEAD *, int, DBT *, int);
int __big_keydata(HTAB *, BUFHEAD *, DBT *, DBT *, int);
int __split_page(HTAB *, u_int32_t, u_int32_t);
int __ibitmap(HTAB *, int, int, int);
int __put_page(HTAB *, char *, u_int32_t, int, int);
BUFHEAD *__get_buf(HTAB *, u_int32_t, BUFHEAD *, int);
}

#define static

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Margo Seltzer.
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




u_int32_t __call_hash(HTAB *, char *, int);

static int   alloc_segs(HTAB *, int);
static int   flush_meta(HTAB *);
static int   hash_access(HTAB *, ACTION, DBT *, DBT *);
static int   hash_close(DB *);
static int   hash_delete(const DB *, const DBT *, u_int32_t);
static int   hash_fd(const DB *);
static int   hash_get(const DB *, const DBT *, DBT *, u_int32_t);
static int   hash_put(const DB *, DBT *, const DBT *, u_int32_t);
static void *hash_realloc(SEGMENT **, int, int);
static int   hash_seq(const DB *, DBT *, DBT *, u_int32_t);
static int   hash_sync(const DB *, u_int32_t);
static int   hdestroy(HTAB *);
static HTAB *init_hash(HTAB *, const char *, const HASHINFO *);
static int   init_htab(HTAB *, int);
#if BYTE_ORDER == LITTLE_ENDIAN
static void  swap_header(HTAB *);
static void  swap_header_copy(HASHHDR *, HASHHDR *);
#endif

/* Fast arithmetic, relying on powers of 2, */
#define MOD(x, y)		((x) & ((y) - 1))

#define RETURN_ERROR(ERR, LOC)	{ save_errno = ERR; goto LOC; }

/* Return values */
#define	SUCCESS	 (0)
#define	ERROR	(-1)
#define	ABNORMAL (1)


/************************** INTERFACE ROUTINES ***************************/
/* OPEN/CLOSE */

/* ARGSUSED */
DB *
__hash_open(const char *file, int flags, int mode,
    const HASHINFO *info,	/* Special directives for create */
    int dflags)
{
	HTAB *hashp;
	struct stat statbuf;
	DB *dbp;
	int bpages, hdrsize, new_table, nsegs, save_errno;

	if (!(hashp = (HTAB *)std::calloc(1, sizeof(HTAB))))
		return (NULL);
	hashp->fp = -1;

	/*
	 * Even if user wants write only, we need to be able to read
	 * the actual file, so we need to open it read/write. But, the
	 * field in the hashp structure needs to be accurate so that
	 * we can check accesses.
	 */
	hashp->flags = flags;
	if ((flags & O_ACCMODE) == O_WRONLY) {
		flags &= ~O_WRONLY;
		flags |= O_RDWR;
	}

	if (file) {
		if ((hashp->fp = _open(file, flags | O_CLOEXEC, mode)) == -1)
			RETURN_ERROR(errno, error0);
		new_table = _fstat(hashp->fp, &statbuf) == 0 &&
		    statbuf.st_size == 0 &&
		    ((flags & O_ACCMODE) != O_RDONLY || (flags & O_CREAT) != 0);
	} else
		new_table = 1;

	if (new_table) {
		if (!(hashp = init_hash(hashp, file, info)))
			RETURN_ERROR(errno, error1);
	} else {
		/* Table already exists */
		if (info && info->hash)
			hashp->hash = info->hash;
		else
			hashp->hash = __default_hash;

		hdrsize = _read(hashp->fp, &hashp->hdr, sizeof(HASHHDR));
#if BYTE_ORDER == LITTLE_ENDIAN
		swap_header(hashp);
#endif
		if (hdrsize == -1)
			RETURN_ERROR(errno, error1);
		if (hdrsize != sizeof(HASHHDR))
			RETURN_ERROR(EFTYPE, error1);
		/* Verify file type, versions and hash function */
		if (hashp->MAGIC != HASHMAGIC)
			RETURN_ERROR(EFTYPE, error1);
#define	OLDHASHVERSION	1
		if (hashp->VERSION != HASHVERSION &&
		    hashp->VERSION != OLDHASHVERSION)
			RETURN_ERROR(EFTYPE, error1);
		if ((int32_t)hashp->hash(CHARKEY, sizeof(CHARKEY)) != hashp->H_CHARKEY)
			RETURN_ERROR(EFTYPE, error1);
		/*
		 * Figure out how many segments we need.  Max_Bucket is the
		 * maximum bucket number, so the number of buckets is
		 * max_bucket + 1.
		 */
		nsegs = howmany(hashp->MAX_BUCKET + 1, hashp->SGSIZE);
		if (alloc_segs(hashp, nsegs))
			/*
			 * If alloc_segs fails, table will have been destroyed
			 * and errno will have been set.
			 */
			return (NULL);
		/* Read in bitmaps */
		bpages = (hashp->SPARES[hashp->OVFL_POINT] +
		    (hashp->BSIZE << BYTE_SHIFT) - 1) >>
		    (hashp->BSHIFT + BYTE_SHIFT);

		hashp->nmaps = bpages;
		(void)std::memset(&hashp->mapp[0], 0, bpages * sizeof(u_int32_t *));
	}

	/* Initialize Buffer Manager */
	if (info && info->cachesize)
		__buf_init(hashp, info->cachesize);
	else
		__buf_init(hashp, DEF_BUFSIZE);

	hashp->new_file = new_table;
	hashp->save_file = file && (flags & O_RDWR);
	hashp->cbucket = -1;
	if (!(dbp = (DB *)std::malloc(sizeof(DB)))) {
		save_errno = errno;
		hdestroy(hashp);
		errno = save_errno;
		return (NULL);
	}
	dbp->internal = hashp;
	dbp->close = hash_close;
	dbp->del = hash_delete;
	dbp->fd = hash_fd;
	dbp->get = hash_get;
	dbp->put = reinterpret_cast<int (*)(__db *, DBT *, const DBT *, unsigned int)>(hash_put);
	dbp->seq = hash_seq;
	dbp->sync = hash_sync;
	dbp->type = DB_HASH;

	return (dbp);

error1:
	if (hashp != NULL)
		(void)_close(hashp->fp);

error0:
	std::free(hashp);
	errno = save_errno;
	return (NULL);
}

static int
hash_close(DB *dbp)
{
	HTAB *hashp;
	int retval;

	if (!dbp)
		return (ERROR);

	hashp = (HTAB *)dbp->internal;
	retval = hdestroy(hashp);
	std::free(dbp);
	return (retval);
}

static int
hash_fd(const DB *dbp)
{
	HTAB *hashp;

	if (!dbp)
		return (ERROR);

	hashp = (HTAB *)dbp->internal;
	if (hashp->fp == -1) {
		errno = ENOENT;
		return (-1);
	}
	return (hashp->fp);
}

/************************** LOCAL CREATION ROUTINES **********************/
static HTAB *
init_hash(HTAB *hashp, const char *file, const HASHINFO *info)
{
	struct stat statbuf;
	int nelem;

	nelem = 1;
	hashp->NKEYS = 0;
	hashp->LORDER = BYTE_ORDER;
	hashp->BSIZE = DEF_BUCKET_SIZE;
	hashp->BSHIFT = DEF_BUCKET_SHIFT;
	hashp->SGSIZE = DEF_SEGSIZE;
	hashp->SSHIFT = DEF_SEGSIZE_SHIFT;
	hashp->DSIZE = DEF_DIRSIZE;
	hashp->FFACTOR = DEF_FFACTOR;
	hashp->hash = __default_hash;
	std::memset(hashp->SPARES, 0, sizeof(hashp->SPARES));
	std::memset(hashp->BITMAPS, 0, sizeof (hashp->BITMAPS));

	/* Fix bucket size to be optimal for file system */
	if (file != NULL) {
		if (stat(file, &statbuf))
			return (NULL);
		hashp->BSIZE = statbuf.st_blksize;
		if (hashp->BSIZE > MAX_BSIZE)
			hashp->BSIZE = MAX_BSIZE;
		hashp->BSHIFT = __log2(hashp->BSIZE);
	}

	if (info) {
		if (info->bsize) {
			/* Round pagesize up to power of 2 */
			hashp->BSHIFT = __log2(info->bsize);
			hashp->BSIZE = 1 << hashp->BSHIFT;
			if (hashp->BSIZE > MAX_BSIZE) {
				errno = EINVAL;
				return (NULL);
			}
		}
		if (info->ffactor)
			hashp->FFACTOR = info->ffactor;
		if (info->hash)
			hashp->hash = info->hash;
		if (info->nelem)
			nelem = info->nelem;
		if (info->lorder) {
			if (info->lorder != BIG_ENDIAN &&
			    info->lorder != LITTLE_ENDIAN) {
				errno = EINVAL;
				return (NULL);
			}
			hashp->LORDER = info->lorder;
		}
	}
	/* init_htab should destroy the table and set errno if it fails */
	if (init_htab(hashp, nelem))
		return (NULL);
	else
		return (hashp);
}
/*
 * This calls alloc_segs which may run out of memory.  Alloc_segs will destroy
 * the table and set errno, so we just pass the error information along.
 *
 * Returns 0 on No Error
 */
static int
init_htab(HTAB *hashp, int nelem)
{
	int nbuckets, nsegs, l2;

	/*
	 * Divide number of elements by the fill factor and determine a
	 * desired number of buckets.  Allocate space for the next greater
	 * power of two number of buckets.
	 */
	nelem = (nelem - 1) / hashp->FFACTOR + 1;

	l2 = __log2(MAX(nelem, 2));
	nbuckets = 1 << l2;

	hashp->SPARES[l2] = l2 + 1;
	hashp->SPARES[l2 + 1] = l2 + 1;
	hashp->OVFL_POINT = l2;
	hashp->LAST_FREED = 2;

	/* First bitmap page is at: splitpoint l2 page offset 1 */
	if (__ibitmap(hashp, OADDR_OF(l2, 1), l2 + 1, 0))
		return (-1);

	hashp->MAX_BUCKET = hashp->LOW_MASK = nbuckets - 1;
	hashp->HIGH_MASK = (nbuckets << 1) - 1;
	hashp->HDRPAGES = ((MAX(sizeof(HASHHDR), MINHDRSIZE) - 1) >>
	    hashp->BSHIFT) + 1;

	nsegs = (nbuckets - 1) / hashp->SGSIZE + 1;
	nsegs = 1 << __log2(nsegs);

	if (nsegs > hashp->DSIZE)
		hashp->DSIZE = nsegs;
	return (alloc_segs(hashp, nsegs));
}

/********************** DESTROY/CLOSE ROUTINES ************************/

/*
 * Flushes any changes to the file if necessary and destroys the hashp
 * structure, freeing all allocated space.
 */
static int
hdestroy(HTAB *hashp)
{
	int i, save_errno;

	save_errno = 0;

	/*
	 * Call on buffer manager to free buffers, and if required,
	 * write them to disk.
	 */
	if (__buf_free(hashp, 1, hashp->save_file))
		save_errno = errno;
	if (hashp->dir) {
		std::free(*hashp->dir);	/* Free initial segments */
		/* Free extra segments */
		while (hashp->exsegs--)
			std::free(hashp->dir[--hashp->nsegs]);
		std::free(hashp->dir);
	}
	if (flush_meta(hashp) && !save_errno)
		save_errno = errno;
	/* Free Bigmaps */
	for (i = 0; i < hashp->nmaps; i++)
		if (hashp->mapp[i])
			std::free(hashp->mapp[i]);
	if (hashp->tmp_key)
		std::free(hashp->tmp_key);
	if (hashp->tmp_buf)
		std::free(hashp->tmp_buf);

	if (hashp->fp != -1) {
		if (hashp->save_file)
			(void)_fsync(hashp->fp);
		(void)_close(hashp->fp);
	}

	std::free(hashp);

	if (save_errno) {
		errno = save_errno;
		return (ERROR);
	}
	return (SUCCESS);
}
/*
 * Write modified pages to disk
 *
 * Returns:
 *	 0 == OK
 *	-1 ERROR
 */
static int
hash_sync(const DB *dbp, u_int32_t flags)
{
	HTAB *hashp;

	if (flags != 0) {
		errno = EINVAL;
		return (ERROR);
	}

	if (!dbp)
		return (ERROR);

	hashp = (HTAB *)dbp->internal;
	if (!hashp->save_file)
		return (0);
	if (__buf_free(hashp, 0, 1) || flush_meta(hashp))
		return (ERROR);
	if (hashp->fp != -1 && _fsync(hashp->fp) != 0)
		return (ERROR);
	hashp->new_file = 0;
	return (0);
}

/*
 * Returns:
 *	 0 == OK
 *	-1 indicates that errno should be set
 */
static int
flush_meta(HTAB *hashp)
{
	HASHHDR *whdrp;
#if BYTE_ORDER == LITTLE_ENDIAN
	HASHHDR whdr;
#endif
	int fp, i, wsize;

	if (!hashp->save_file)
		return (0);
	hashp->MAGIC = HASHMAGIC;
	hashp->VERSION = HASHVERSION;
	hashp->H_CHARKEY = hashp->hash(CHARKEY, sizeof(CHARKEY));

	fp = hashp->fp;
	whdrp = &hashp->hdr;
#if BYTE_ORDER == LITTLE_ENDIAN
	whdrp = &whdr;
	swap_header_copy(&hashp->hdr, whdrp);
#endif
	if ((wsize = pwrite(fp, whdrp, sizeof(HASHHDR), (off_t)0)) == -1)
		return (-1);
	else
		if (wsize != sizeof(HASHHDR)) {
			errno = EFTYPE;
			hashp->error = errno;
			return (-1);
		}
	for (i = 0; i < NCACHED; i++)
		if (hashp->mapp[i])
			if (__put_page(hashp, (char *)hashp->mapp[i],
				hashp->BITMAPS[i], 0, 1))
				return (-1);
	return (0);
}

/*******************************SEARCH ROUTINES *****************************/
/*
 * All the access routines return
 *
 * Returns:
 *	 0 on SUCCESS
 *	 1 to indicate an external ERROR (i.e. key not found, etc)
 *	-1 to indicate an internal ERROR (i.e. out of memory, etc)
 */
static int
hash_get(const DB *dbp, const DBT *key, DBT *data, u_int32_t flag)
{
	HTAB *hashp;

	hashp = (HTAB *)dbp->internal;
	if (flag) {
		hashp->error = errno = EINVAL;
		return (ERROR);
	}
	if ((hashp->flags & O_ACCMODE) == O_WRONLY) {
		hashp->error = errno = EPERM;
		return (ERROR);
	}
	return (hash_access(hashp, HASH_GET, (DBT *)key, data));
}

static int
hash_put(const DB *dbp, DBT *key, const DBT *data, u_int32_t flag)
{
	HTAB *hashp;

	hashp = (HTAB *)dbp->internal;
	if (flag && flag != R_NOOVERWRITE) {
		hashp->error = errno = EINVAL;
		return (ERROR);
	}
	if ((hashp->flags & O_ACCMODE) == O_RDONLY) {
		hashp->error = errno = EPERM;
		return (ERROR);
	}
	return (hash_access(hashp, flag == R_NOOVERWRITE ?
	    HASH_PUTNEW : HASH_PUT, (DBT *)key, (DBT *)data));
}

static int
hash_delete(const DB *dbp, const DBT *key,
    u_int32_t flag)		/* Ignored */
{
	HTAB *hashp;

	hashp = (HTAB *)dbp->internal;
	if (flag && flag != R_CURSOR) {
		hashp->error = errno = EINVAL;
		return (ERROR);
	}
	if ((hashp->flags & O_ACCMODE) == O_RDONLY) {
		hashp->error = errno = EPERM;
		return (ERROR);
	}
	return (hash_access(hashp, HASH_DELETE, (DBT *)key, NULL));
}

/*
 * Assume that hashp has been set in wrapper routine.
 */
static int
hash_access(HTAB *hashp, ACTION action, DBT *key, DBT *val)
{
	BUFHEAD *rbufp;
	BUFHEAD *bufp, *save_bufp;
	u_int16_t *bp;
	int n, ndx, off, size;
	char *kp;
	u_int16_t pageno;


	off = hashp->BSIZE;
	size = key->size;
	kp = (char *)key->data;
	rbufp = __get_buf(hashp, __call_hash(hashp, kp, size), NULL, 0);
	if (!rbufp)
		return (ERROR);
	save_bufp = rbufp;

	/* Pin the bucket chain */
	rbufp->flags |= BUF_PIN;
	for (bp = (u_int16_t *)rbufp->page, n = *bp++, ndx = 1; ndx < n;)
		if (bp[1] >= REAL_KEY) {
			/* Real key/data pair */
			if (size == off - *bp &&
			    std::memcmp(kp, rbufp->page + *bp, size) == 0)
				goto found;
			off = bp[1];
			bp += 2;
			ndx += 2;
		} else if (bp[1] == OVFLPAGE) {
			rbufp = __get_buf(hashp, *bp, rbufp, 0);
			if (!rbufp) {
				save_bufp->flags &= ~BUF_PIN;
				return (ERROR);
			}
			/* FOR LOOP INIT */
			bp = (u_int16_t *)rbufp->page;
			n = *bp++;
			ndx = 1;
			off = hashp->BSIZE;
		} else if (bp[1] < REAL_KEY) {
			if ((ndx =
			    __find_bigpair(hashp, rbufp, ndx, kp, size)) > 0)
				goto found;
			if (ndx == -2) {
				bufp = rbufp;
				if (!(pageno =
				    __find_last_page(hashp, &bufp))) {
					ndx = 0;
					rbufp = bufp;
					break;	/* FOR */
				}
				rbufp = __get_buf(hashp, pageno, bufp, 0);
				if (!rbufp) {
					save_bufp->flags &= ~BUF_PIN;
					return (ERROR);
				}
				/* FOR LOOP INIT */
				bp = (u_int16_t *)rbufp->page;
				n = *bp++;
				ndx = 1;
				off = hashp->BSIZE;
			} else {
				save_bufp->flags &= ~BUF_PIN;
				return (ERROR);
			}
		}

	/* Not found */
	switch (action) {
	case HASH_PUT:
	case HASH_PUTNEW:
		if (__addel(hashp, rbufp, key, val)) {
			save_bufp->flags &= ~BUF_PIN;
			return (ERROR);
		} else {
			save_bufp->flags &= ~BUF_PIN;
			return (SUCCESS);
		}
	case HASH_GET:
	case HASH_DELETE:
	default:
		save_bufp->flags &= ~BUF_PIN;
		return (ABNORMAL);
	}

found:
	switch (action) {
	case HASH_PUTNEW:
		save_bufp->flags &= ~BUF_PIN;
		return (ABNORMAL);
	case HASH_GET:
		bp = (u_int16_t *)rbufp->page;
		if (bp[ndx + 1] < REAL_KEY) {
			if (__big_return(hashp, rbufp, ndx, val, 0))
				return (ERROR);
		} else {
			val->data = (void *)(rbufp->page + (int)bp[ndx + 1]);
			val->size = bp[ndx] - bp[ndx + 1];
		}
		break;
	case HASH_PUT:
		if ((__delpair(hashp, rbufp, ndx)) ||
		    (__addel(hashp, rbufp, key, val))) {
			save_bufp->flags &= ~BUF_PIN;
			return (ERROR);
		}
		break;
	case HASH_DELETE:
		if (__delpair(hashp, rbufp, ndx))
			return (ERROR);
		break;
	default:
		abort();
	}
	save_bufp->flags &= ~BUF_PIN;
	return (SUCCESS);
}

static int
hash_seq(const DB *dbp, DBT *key, DBT *data, u_int32_t flag)
{
	u_int32_t bucket;
	BUFHEAD *bufp;
	HTAB *hashp;
	u_int16_t *bp, ndx;

	hashp = (HTAB *)dbp->internal;
	if (flag != R_FIRST && flag != R_NEXT) {
		hashp->error = errno = EINVAL;
		return (ERROR);
	}
	if (flag == R_FIRST) {
		hashp->cbucket = 0;
		hashp->cndx = 1;
		hashp->cpage = NULL;
	} else if (hashp->cbucket < 0) { /* R_NEXT */
		return (ABNORMAL);
	}
next_bucket:
	for (bp = NULL; !bp || !bp[0]; ) {
		if (!(bufp = hashp->cpage)) {
			for (bucket = hashp->cbucket;
			    bucket <= hashp->MAX_BUCKET;
			    bucket++, hashp->cndx = 1) {
				bufp = __get_buf(hashp, bucket, NULL, 0);
				if (!bufp)
					return (ERROR);
				hashp->cpage = bufp;
				bp = (u_int16_t *)bufp->page;
				if (bp[0])
					break;
			}
			hashp->cbucket = bucket;
			if ((u_int32_t)hashp->cbucket > hashp->MAX_BUCKET) {
				hashp->cbucket = -1;
				return (ABNORMAL);
			}
		} else {
			bp = (u_int16_t *)hashp->cpage->page;
			if (flag == R_NEXT || flag == 0) {
				hashp->cndx += 2;
				if (hashp->cndx > bp[0]) {
					hashp->cpage = NULL;
					hashp->cbucket++;
					hashp->cndx = 1;
					goto next_bucket;
				}
			}
		}

		while (bp[hashp->cndx + 1] == OVFLPAGE) {
			bufp = hashp->cpage =
			    __get_buf(hashp, bp[hashp->cndx], bufp, 0);
			if (!bufp)
				return (ERROR);
			bp = (u_int16_t *)(bufp->page);
			hashp->cndx = 1;
		}
		if (!bp[0]) {
			hashp->cpage = NULL;
			++hashp->cbucket;
		}
	}
	ndx = hashp->cndx;
	if (bp[ndx + 1] < REAL_KEY) {
		if (__big_keydata(hashp, bufp, key, data, 1))
			return (ERROR);
	} else {
		if (hashp->cpage == NULL)
			return (ERROR);
		key->data = (void *)(hashp->cpage->page + bp[ndx]);
		key->size = (ndx > 1 ? bp[ndx - 1] : hashp->BSIZE) - bp[ndx];
		data->data = (void *)(hashp->cpage->page + bp[ndx + 1]);
		data->size = bp[ndx] - bp[ndx + 1];
	}
	return (SUCCESS);
}

/********************************* UTILITIES ************************/

/*
 * Returns:
 *	 0 ==> OK
 *	-1 ==> Error
 */
int
__expand_table(HTAB *hashp)
{
	u_int32_t old_bucket, new_bucket;
	int dirsize, new_segnum, spare_ndx;

	new_bucket = ++hashp->MAX_BUCKET;
	old_bucket = (hashp->MAX_BUCKET & hashp->LOW_MASK);

	new_segnum = new_bucket >> hashp->SSHIFT;

	/* Check if we need a new segment */
	if (new_segnum >= hashp->nsegs) {
		/* Check if we need to expand directory */
		if (new_segnum >= hashp->DSIZE) {
			/* Reallocate directory */
			dirsize = hashp->DSIZE * sizeof(SEGMENT *);
			if (!hash_realloc(&hashp->dir, dirsize, dirsize << 1))
				return (-1);
			hashp->DSIZE = dirsize << 1;
		}
		if ((hashp->dir[new_segnum] =
		    (SEGMENT)std::calloc(hashp->SGSIZE, sizeof(SEGMENT))) == NULL)
			return (-1);
		hashp->exsegs++;
		hashp->nsegs++;
	}
	/*
	 * If the split point is increasing (MAX_BUCKET's log base 2
	 * * increases), we need to copy the current contents of the spare
	 * split bucket to the next bucket.
	 */
	spare_ndx = __log2(hashp->MAX_BUCKET + 1);
	if (spare_ndx > hashp->OVFL_POINT) {
		hashp->SPARES[spare_ndx] = hashp->SPARES[hashp->OVFL_POINT];
		hashp->OVFL_POINT = spare_ndx;
	}

	if (new_bucket > hashp->HIGH_MASK) {
		/* Starting a new doubling */
		hashp->LOW_MASK = hashp->HIGH_MASK;
		hashp->HIGH_MASK = new_bucket | hashp->LOW_MASK;
	}
	/* Relocate records to the new bucket */
	return (__split_page(hashp, old_bucket, new_bucket));
}

/*
 * If realloc guarantees that the pointer is not destroyed if the realloc
 * fails, then this routine can go away.
 */
static void *
hash_realloc(SEGMENT **p_ptr, int oldsize, int newsize)
{
	void *p;

	if ( (p = std::malloc(newsize)) ) {
		std::memmove(p, *p_ptr, oldsize);
		std::memset((char *)p + oldsize, 0, newsize - oldsize);
		std::free(*p_ptr);
		*p_ptr = (SEGMENT *)p;
	}
	return (p);
}

u_int32_t
__call_hash(HTAB *hashp, char *k, int len)
{
	unsigned int n, bucket;

	n = hashp->hash(k, len);
	bucket = n & hashp->HIGH_MASK;
	if (bucket > hashp->MAX_BUCKET)
		bucket = bucket & hashp->LOW_MASK;
	return (bucket);
}

/*
 * Allocate segment table.  On error, destroy the table and set errno.
 *
 * Returns 0 on success
 */
static int
alloc_segs(HTAB *hashp, int nsegs)
{
	int i;
	SEGMENT store;

	int save_errno;

	if ((hashp->dir =
	    (SEGMENT *)std::calloc(hashp->DSIZE, sizeof(SEGMENT *))) == NULL) {
		save_errno = errno;
		(void)hdestroy(hashp);
		errno = save_errno;
		return (-1);
	}
	hashp->nsegs = nsegs;
	if (nsegs == 0)
		return (0);
	/* Allocate segments */
	if ((store = (SEGMENT)std::calloc(nsegs << hashp->SSHIFT, sizeof(SEGMENT))) == NULL) {
		save_errno = errno;
		(void)hdestroy(hashp);
		errno = save_errno;
		return (-1);
	}
	for (i = 0; i < nsegs; i++)
		hashp->dir[i] = &store[i << hashp->SSHIFT];
	return (0);
}

#if BYTE_ORDER == LITTLE_ENDIAN
/*
 * Hashp->hdr needs to be byteswapped.
 */
static void
swap_header_copy(HASHHDR *srcp, HASHHDR *destp)
{
	int i;

	P_32_COPY(srcp->magic, destp->magic);
	P_32_COPY(srcp->version, destp->version);
	P_32_COPY(srcp->lorder, destp->lorder);
	P_32_COPY(srcp->bsize, destp->bsize);
	P_32_COPY(srcp->bshift, destp->bshift);
	P_32_COPY(srcp->dsize, destp->dsize);
	P_32_COPY(srcp->ssize, destp->ssize);
	P_32_COPY(srcp->sshift, destp->sshift);
	P_32_COPY(srcp->ovfl_point, destp->ovfl_point);
	P_32_COPY(srcp->last_freed, destp->last_freed);
	P_32_COPY(srcp->max_bucket, destp->max_bucket);
	P_32_COPY(srcp->high_mask, destp->high_mask);
	P_32_COPY(srcp->low_mask, destp->low_mask);
	P_32_COPY(srcp->ffactor, destp->ffactor);
	P_32_COPY(srcp->nkeys, destp->nkeys);
	P_32_COPY(srcp->hdrpages, destp->hdrpages);
	P_32_COPY(srcp->h_charkey, destp->h_charkey);
	for (i = 0; i < NCACHED; i++) {
		P_32_COPY(srcp->spares[i], destp->spares[i]);
		P_16_COPY(srcp->bitmaps[i], destp->bitmaps[i]);
	}
}

static void
swap_header(HTAB *hashp)
{
	HASHHDR *hdrp;
	int i;

	hdrp = &hashp->hdr;

	M_32_SWAP(hdrp->magic);
	M_32_SWAP(hdrp->version);
	M_32_SWAP(hdrp->lorder);
	M_32_SWAP(hdrp->bsize);
	M_32_SWAP(hdrp->bshift);
	M_32_SWAP(hdrp->dsize);
	M_32_SWAP(hdrp->ssize);
	M_32_SWAP(hdrp->sshift);
	M_32_SWAP(hdrp->ovfl_point);
	M_32_SWAP(hdrp->last_freed);
	M_32_SWAP(hdrp->max_bucket);
	M_32_SWAP(hdrp->high_mask);
	M_32_SWAP(hdrp->low_mask);
	M_32_SWAP(hdrp->ffactor);
	M_32_SWAP(hdrp->nkeys);
	M_32_SWAP(hdrp->hdrpages);
	M_32_SWAP(hdrp->h_charkey);
	for (i = 0; i < NCACHED; i++) {
		M_32_SWAP(hdrp->spares[i]);
		M_16_SWAP(hdrp->bitmaps[i]);
	}
}
#endif

} // namespace pbsd::lib_libc_db_hash::b0194s3
