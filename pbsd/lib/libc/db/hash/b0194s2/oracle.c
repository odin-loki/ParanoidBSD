/*
 * PBSD batch b0194s2 -- reference oracle.
 *
 * The original C sources of hbsd/src/lib/libc/db/hash/hash_page.c, with every
 * function renamed with a "ref_" prefix.  Function bodies are UNMODIFIED.
 * Only missing platform defines and the private header material that the
 * original translation unit pulled in (db.h private section, hash.h, page.h)
 * have been added so that this file compiles standalone.
 *
 * __split_page() and ugly_split() are absent; see skipped.txt.
 */

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

/*
 * PACKAGE:  hashing
 *
 * DESCRIPTION:
 *	Page manipulation for hashing package.
 *
 * ROUTINES:
 *
 * External
 *	__get_page
 *	__add_ovflpage
 * Internal
 *	overflow_page
 *	open_temp
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/param.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ------------------------------------------------------------------ */
/* Platform defines missing on this host.                             */
/* ------------------------------------------------------------------ */

#ifndef LONG_BIT
#define	LONG_BIT	(sizeof(long) * CHAR_BIT)
#endif

#ifndef EFTYPE
#define	EFTYPE		EINVAL		/* not a FreeBSD host */
#endif

#ifndef MAXPATHLEN
#define	MAXPATHLEN	PATH_MAX
#endif

#ifndef rounddown2
#define	rounddown2(x, y)	((x)&(~((y)-1)))
#endif

/* ------------------------------------------------------------------ */
/* From <db.h>, __DBINTERFACE_PRIVATE section.                        */
/* ------------------------------------------------------------------ */

typedef struct {
	void	*data;			/* data */
	size_t	 size;			/* data length */
} DBT;

#define	M_32_SWAP(a) {							\
	uint32_t _tmp = a;						\
	((char *)&a)[0] = ((char *)&_tmp)[3];				\
	((char *)&a)[1] = ((char *)&_tmp)[2];				\
	((char *)&a)[2] = ((char *)&_tmp)[1];				\
	((char *)&a)[3] = ((char *)&_tmp)[0];				\
}

#define	M_16_SWAP(a) {							\
	uint16_t _tmp = a;						\
	((char *)&a)[0] = ((char *)&_tmp)[1];				\
	((char *)&a)[1] = ((char *)&_tmp)[0];				\
}

/* ------------------------------------------------------------------ */
/* From "hash.h".                                                     */
/* ------------------------------------------------------------------ */

/* Buffer Management structures */
typedef struct _bufhead BUFHEAD;

struct _bufhead {
	BUFHEAD		*prev;		/* LRU links */
	BUFHEAD		*next;		/* LRU links */
	BUFHEAD		*ovfl;		/* Overflow page buffer header */
	u_int32_t	 addr;		/* Address of this page */
	char		*page;		/* Actual page data */
	char	 	flags;
#define	BUF_MOD		0x0001
#define BUF_DISK	0x0002
#define	BUF_BUCKET	0x0004
#define	BUF_PIN		0x0008
};

#define IS_BUCKET(X)	((X) & BUF_BUCKET)

typedef BUFHEAD **SEGMENT;

/* Hash Table Information */
typedef struct hashhdr {		/* Disk resident portion */
	int32_t		magic;		/* Magic NO for hash tables */
	int32_t		version;	/* Version ID */
	u_int32_t	lorder;		/* Byte Order */
	int32_t		bsize;		/* Bucket/Page Size */
	int32_t		bshift;		/* Bucket shift */
	int32_t		dsize;		/* Directory Size */
	int32_t		ssize;		/* Segment Size */
	int32_t		sshift;		/* Segment shift */
	int32_t		ovfl_point;	/* Where overflow pages are being
					 * allocated */
	int32_t		last_freed;	/* Last overflow page freed */
	u_int32_t	max_bucket;	/* ID of Maximum bucket in use */
	u_int32_t	high_mask;	/* Mask to modulo into entire table */
	u_int32_t	low_mask;	/* Mask to modulo into lower half of
					 * table */
	u_int32_t	ffactor;	/* Fill factor */
	int32_t		nkeys;		/* Number of keys in hash table */
	int32_t		hdrpages;	/* Size of table header */
	int32_t		h_charkey;	/* value of hash(CHARKEY) */
#define NCACHED	32			/* number of bit maps and spare
					 * points */
	int32_t		spares[NCACHED];/* spare pages for overflow */
	u_int16_t	bitmaps[NCACHED];	/* address of overflow page
						 * bitmaps */
} HASHHDR;

typedef struct htab	 {		/* Memory resident data structure */
	HASHHDR 	hdr;		/* Header */
	int		nsegs;		/* Number of allocated segments */
	int		exsegs;		/* Number of extra allocated
					 * segments */
	u_int32_t			/* Hash function */
	    (*hash)(const void *, size_t);
	int		flags;		/* Flag values */
	int		fp;		/* File pointer */
	char		*tmp_buf;	/* Temporary Buffer for BIG data */
	char		*tmp_key;	/* Temporary Buffer for BIG keys */
	BUFHEAD 	*cpage;		/* Current page */
	int		cbucket;	/* Current bucket */
	int		cndx;		/* Index of next item on cpage */
	int		error;		/* Error Number -- for DBM
					 * compatibility */
	int		new_file;	/* Indicates if fd is backing store
					 * or no */
	int		save_file;	/* Indicates whether we need to flush
					 * file at
					 * exit */
	u_int32_t	*mapp[NCACHED];	/* Pointers to page maps */
	int		nmaps;		/* Initial number of bitmaps */
	int		nbufs;		/* Number of buffers left to
					 * allocate */
	BUFHEAD 	bufhead;	/* Header of buffer lru list */
	SEGMENT 	*dir;		/* Hash Bucket directory */
} HTAB;

/*
 * Constants
 */
#define	MAX_BSIZE		32768		/* 2^15 but should be 65536 */
#define MIN_BUFFERS		6
#define MINHDRSIZE		512
#define DEF_BUFSIZE		65536		/* 64 K */
#define DEF_BUCKET_SIZE		4096
#define DEF_BUCKET_SHIFT	12		/* log2(BUCKET) */
#define DEF_SEGSIZE		256
#define DEF_SEGSIZE_SHIFT	8		/* log2(SEGSIZE)	 */
#define DEF_DIRSIZE		256
#define DEF_FFACTOR		65536
#define MIN_FFACTOR		4
#define SPLTMAX			8
#define CHARKEY			"%$sniglet^&"
#define NUMKEY			1038583
#define BYTE_SHIFT		3
#define INT_TO_BYTE		2
#define INT_BYTE_SHIFT		5
#define ALL_SET			((u_int32_t)0xFFFFFFFF)
#define ALL_CLEAR		0

#define PTROF(X)	((BUFHEAD *)((intptr_t)(X)&~0x3))
#define ISMOD(X)	((u_int32_t)(intptr_t)(X)&0x1)
#define DOMOD(X)	((X) = (char *)((intptr_t)(X)|0x1))
#define ISDISK(X)	((u_int32_t)(intptr_t)(X)&0x2)
#define DODISK(X)	((X) = (char *)((intptr_t)(X)|0x2))

#define BITS_PER_MAP	32

/* Given the address of the beginning of a big map, clear/set the nth bit */
#define CLRBIT(A, N)	((A)[(N)/BITS_PER_MAP] &= ~(1<<((N)%BITS_PER_MAP)))
#define SETBIT(A, N)	((A)[(N)/BITS_PER_MAP] |= (1<<((N)%BITS_PER_MAP)))
#define ISSET(A, N)	((A)[(N)/BITS_PER_MAP] & (1<<((N)%BITS_PER_MAP)))

#define SPLITSHIFT	11
#define SPLITMASK	0x7FF
#define SPLITNUM(N)	(((u_int32_t)(N)) >> SPLITSHIFT)
#define OPAGENUM(N)	((N) & SPLITMASK)
#define	OADDR_OF(S,O)	((u_int32_t)((u_int32_t)(S) << SPLITSHIFT) + (O))

#define BUCKET_TO_PAGE(B) \
	(B) + hashp->HDRPAGES + ((B) ? hashp->SPARES[__log2((B)+1)-1] : 0)
#define OADDR_TO_PAGE(B) 	\
	BUCKET_TO_PAGE ( (1 << SPLITNUM((B))) -1 ) + OPAGENUM((B));

#define OVFLPAGE	0
#define PARTIAL_KEY	1
#define FULL_KEY	2
#define FULL_KEY_DATA	3
#define	REAL_KEY	4

/* Short hands for accessing structure */
#define BSIZE		hdr.bsize
#define BSHIFT		hdr.bshift
#define DSIZE		hdr.dsize
#define SGSIZE		hdr.ssize
#define SSHIFT		hdr.sshift
#define LORDER		hdr.lorder
#define OVFL_POINT	hdr.ovfl_point
#define	LAST_FREED	hdr.last_freed
#define MAX_BUCKET	hdr.max_bucket
#define FFACTOR		hdr.ffactor
#define HIGH_MASK	hdr.high_mask
#define LOW_MASK	hdr.low_mask
#define NKEYS		hdr.nkeys
#define HDRPAGES	hdr.hdrpages
#define SPARES		hdr.spares
#define BITMAPS		hdr.bitmaps
#define VERSION		hdr.version
#define MAGIC		hdr.magic
#define NEXT_FREE	hdr.next_free
#define H_CHARKEY	hdr.h_charkey

/* ------------------------------------------------------------------ */
/* From "page.h".                                                     */
/* ------------------------------------------------------------------ */

#define	PAIRSIZE(K,D)	(2*sizeof(u_int16_t) + (K)->size + (D)->size)
#define BIGOVERHEAD	(4*sizeof(u_int16_t))
#define KEYSIZE(K)	(4*sizeof(u_int16_t) + (K)->size);
#define OVFLSIZE	(2*sizeof(u_int16_t))
#define FREESPACE(P)	((P)[(P)[0]+1])
#define	OFFSET(P)	((P)[(P)[0]+2])
#define PAIRFITS(P,K,D) \
	(((P)[2] >= REAL_KEY) && \
	    (PAIRSIZE((K),(D)) + OVFLSIZE) <= FREESPACE((P)))
#define PAGE_META(N)	(((N)+3) * sizeof(u_int16_t))

typedef struct {
	BUFHEAD *newp;
	BUFHEAD *oldp;
	BUFHEAD *nextp;
	u_int16_t next_addr;
}       SPLIT_RETURN;

/* ------------------------------------------------------------------ */
/* Out-of-batch routines.  Supplied identically to port and oracle by  */
/* the harness.                                                        */
/* ------------------------------------------------------------------ */

extern u_int32_t __log2(u_int32_t);
extern int	 __big_delete(HTAB *, BUFHEAD *);
extern int	 __big_split(HTAB *, BUFHEAD *, BUFHEAD *, BUFHEAD *, int,
	    u_int32_t, SPLIT_RETURN *);
extern int	 __big_insert(HTAB *, BUFHEAD *, const DBT *, const DBT *);
extern int	 __expand_table(HTAB *);
extern BUFHEAD	*__get_buf(HTAB *, u_int32_t, BUFHEAD *, int);
extern u_int32_t __call_hash(HTAB *, char *, int);
extern void	 __reclaim_buf(HTAB *, BUFHEAD *);
extern ssize_t	 _write(int, const void *, size_t);
extern int	 __libc_sigprocmask(int, const sigset_t *, sigset_t *);
extern char	*secure_getenv(const char *);

/* ------------------------------------------------------------------ */
/* The reference implementations.                                     */
/* ------------------------------------------------------------------ */

u_int32_t *ref_fetch_bitmap(HTAB *, int);
u_int32_t  ref_first_free(u_int32_t);
int	   ref_open_temp(HTAB *);
u_int16_t  ref_overflow_page(HTAB *);
void	   ref_putpair(char *, const DBT *, const DBT *);
void	   ref_squeeze_key(u_int16_t *, const DBT *, const DBT *);
int	   ref_ugly_split(HTAB *, u_int32_t, BUFHEAD *, BUFHEAD *, int, int);
int	   ref___ibitmap(HTAB *, int, int, int);
int	   ref___get_page(HTAB *, char *, u_int32_t, int, int, int);
int	   ref___put_page(HTAB *, char *, u_int32_t, int, int);
int	   ref___delpair(HTAB *, BUFHEAD *, int);
int	   ref___split_page(HTAB *, u_int32_t, u_int32_t);
int	   ref___addel(HTAB *, BUFHEAD *, const DBT *, const DBT *);
BUFHEAD	  *ref___add_ovflpage(HTAB *, BUFHEAD *);
void	   ref___free_ovflpage(HTAB *, BUFHEAD *);

#define	PAGE_INIT(P) { \
	((u_int16_t *)(P))[0] = 0; \
	((u_int16_t *)(P))[1] = hashp->BSIZE - 3 * sizeof(u_int16_t); \
	((u_int16_t *)(P))[2] = hashp->BSIZE; \
}

/*
 * This is called AFTER we have verified that there is room on the page for
 * the pair (PAIRFITS has returned true) so we go right ahead and start moving
 * stuff on.
 */
void
ref_putpair(char *p, const DBT *key, const DBT *val)
{
	u_int16_t *bp, n, off;

	bp = (u_int16_t *)p;

	/* Enter the key first. */
	n = bp[0];

	off = OFFSET(bp) - key->size;
	memmove(p + off, key->data, key->size);
	bp[++n] = off;

	/* Now the data. */
	off -= val->size;
	memmove(p + off, val->data, val->size);
	bp[++n] = off;

	/* Adjust page info. */
	bp[0] = n;
	bp[n + 1] = off - ((n + 3) * sizeof(u_int16_t));
	bp[n + 2] = off;
}

/*
 * Returns:
 *	 0 OK
 *	-1 error
 */
int
ref___delpair(HTAB *hashp, BUFHEAD *bufp, int ndx)
{
	u_int16_t *bp, newoff, pairlen;
	int n;

	bp = (u_int16_t *)bufp->page;
	n = bp[0];

	if (bp[ndx + 1] < REAL_KEY)
		return (__big_delete(hashp, bufp));
	if (ndx != 1)
		newoff = bp[ndx - 1];
	else
		newoff = hashp->BSIZE;
	pairlen = newoff - bp[ndx + 1];

	if (ndx != (n - 1)) {
		/* Hard Case -- need to shuffle keys */
		int i;
		char *src = bufp->page + (int)OFFSET(bp);
		char *dst = src + (int)pairlen;
		memmove(dst, src, bp[ndx + 1] - OFFSET(bp));

		/* Now adjust the pointers */
		for (i = ndx + 2; i <= n; i += 2) {
			if (bp[i + 1] == OVFLPAGE) {
				bp[i - 2] = bp[i];
				bp[i - 1] = bp[i + 1];
			} else {
				bp[i - 2] = bp[i] + pairlen;
				bp[i - 1] = bp[i + 1] + pairlen;
			}
		}
		if (ndx == hashp->cndx) {
			/*
			 * We just removed pair we were "pointing" to.
			 * By moving back the cndx we ensure subsequent
			 * hash_seq() calls won't skip over any entries.
			 */
			hashp->cndx -= 2;
		}
	}
	/* Finally adjust the page data */
	bp[n] = OFFSET(bp) + pairlen;
	bp[n - 1] = bp[n + 1] + pairlen + 2 * sizeof(u_int16_t);
	bp[0] = n - 2;
	hashp->NKEYS--;

	bufp->flags |= BUF_MOD;
	return (0);
}
/*
 * Returns:
 *	 0 ==> OK
 *	-1 ==> Error
 */
int
ref___split_page(HTAB *hashp, u_int32_t obucket, u_int32_t nbucket)
{
	BUFHEAD *new_bufp, *old_bufp;
	u_int16_t *ino;
	char *np;
	DBT key, val;
	int n, ndx, retval;
	u_int16_t copyto, diff, off, moved;
	char *op;

	copyto = (u_int16_t)hashp->BSIZE;
	off = (u_int16_t)hashp->BSIZE;
	old_bufp = __get_buf(hashp, obucket, NULL, 0);
	if (old_bufp == NULL)
		return (-1);
	new_bufp = __get_buf(hashp, nbucket, NULL, 0);
	if (new_bufp == NULL)
		return (-1);

	old_bufp->flags |= (BUF_MOD | BUF_PIN);
	new_bufp->flags |= (BUF_MOD | BUF_PIN);

	ino = (u_int16_t *)(op = old_bufp->page);
	np = new_bufp->page;

	moved = 0;

	for (n = 1, ndx = 1; n < ino[0]; n += 2) {
		if (ino[n + 1] < REAL_KEY) {
			retval = ref_ugly_split(hashp, obucket, old_bufp, new_bufp,
			    (int)copyto, (int)moved);
			old_bufp->flags &= ~BUF_PIN;
			new_bufp->flags &= ~BUF_PIN;
			return (retval);

		}
		key.data = (u_char *)op + ino[n];
		key.size = off - ino[n];

		if (__call_hash(hashp, key.data, key.size) == obucket) {
			/* Don't switch page */
			diff = copyto - off;
			if (diff) {
				copyto = ino[n + 1] + diff;
				memmove(op + copyto, op + ino[n + 1],
				    off - ino[n + 1]);
				ino[ndx] = copyto + ino[n] - ino[n + 1];
				ino[ndx + 1] = copyto;
			} else
				copyto = ino[n + 1];
			ndx += 2;
		} else {
			/* Switch page */
			val.data = (u_char *)op + ino[n + 1];
			val.size = ino[n] - ino[n + 1];
			ref_putpair(np, &key, &val);
			moved += 2;
		}

		off = ino[n + 1];
	}

	/* Now clean up the page */
	ino[0] -= moved;
	FREESPACE(ino) = copyto - sizeof(u_int16_t) * (ino[0] + 3);
	OFFSET(ino) = copyto;

#ifdef DEBUG3
	(void)fprintf(stderr, "split %d/%d\n",
	    ((u_int16_t *)np)[0] / 2,
	    ((u_int16_t *)op)[0] / 2);
#endif
	/* unpin both pages */
	old_bufp->flags &= ~BUF_PIN;
	new_bufp->flags &= ~BUF_PIN;
	return (0);
}

/*
 * Called when we encounter an overflow or big key/data page during split
 * handling.  This is special cased since we have to begin checking whether
 * the key/data pairs fit on their respective pages and because we may need
 * overflow pages for both the old and new pages.
 *
 * The first page might be a page with regular key/data pairs in which case
 * we have a regular overflow condition and just need to go on to the next
 * page or it might be a big key/data pair in which case we need to fix the
 * big key/data pair.
 *
 * Returns:
 *	 0 ==> success
 *	-1 ==> failure
 */
int
ref_ugly_split(HTAB *hashp,
    u_int32_t obucket,	/* Same as __split_page. */
    BUFHEAD *old_bufp,
    BUFHEAD *new_bufp,
    int copyto,		/* First byte on page which contains key/data values. */
    int moved)		/* Number of pairs moved to new page. */
{
	BUFHEAD *bufp;	/* Buffer header for ino */
	u_int16_t *ino;	/* Page keys come off of */
	u_int16_t *np;	/* New page */
	u_int16_t *op;	/* Page keys go on to if they aren't moving */

	BUFHEAD *last_bfp;	/* Last buf header OVFL needing to be freed */
	DBT key, val;
	SPLIT_RETURN ret;
	u_int16_t n, off, ov_addr, scopyto;
	char *cino;		/* Character value of ino */

	bufp = old_bufp;
	ino = (u_int16_t *)old_bufp->page;
	np = (u_int16_t *)new_bufp->page;
	op = (u_int16_t *)old_bufp->page;
	last_bfp = NULL;
	scopyto = (u_int16_t)copyto;	/* ANSI */

	n = ino[0] - 1;
	while (n < ino[0]) {
		if (ino[2] < REAL_KEY && ino[2] != OVFLPAGE) {
			if (__big_split(hashp, old_bufp,
			    new_bufp, bufp, bufp->addr, obucket, &ret))
				return (-1);
			old_bufp = ret.oldp;
			if (!old_bufp)
				return (-1);
			op = (u_int16_t *)old_bufp->page;
			new_bufp = ret.newp;
			if (!new_bufp)
				return (-1);
			np = (u_int16_t *)new_bufp->page;
			bufp = ret.nextp;
			if (!bufp)
				return (0);
			cino = (char *)bufp->page;
			ino = (u_int16_t *)cino;
			last_bfp = ret.nextp;
		} else if (ino[n + 1] == OVFLPAGE) {
			ov_addr = ino[n];
			/*
			 * Fix up the old page -- the extra 2 are the fields
			 * which contained the overflow information.
			 */
			ino[0] -= (moved + 2);
			FREESPACE(ino) =
			    scopyto - sizeof(u_int16_t) * (ino[0] + 3);
			OFFSET(ino) = scopyto;

			bufp = __get_buf(hashp, ov_addr, bufp, 0);
			if (!bufp)
				return (-1);

			ino = (u_int16_t *)bufp->page;
			n = 1;
			scopyto = hashp->BSIZE;
			moved = 0;

			if (last_bfp)
				ref___free_ovflpage(hashp, last_bfp);
			last_bfp = bufp;
		}
		/* Move regular sized pairs of there are any */
		off = hashp->BSIZE;
		for (n = 1; (n < ino[0]) && (ino[n + 1] >= REAL_KEY); n += 2) {
			cino = (char *)ino;
			key.data = (u_char *)cino + ino[n];
			key.size = off - ino[n];
			val.data = (u_char *)cino + ino[n + 1];
			val.size = ino[n] - ino[n + 1];
			off = ino[n + 1];

			if (__call_hash(hashp, key.data, key.size) == obucket) {
				/* Keep on old page */
				if (PAIRFITS(op, (&key), (&val)))
					ref_putpair((char *)op, &key, &val);
				else {
					old_bufp =
					    ref___add_ovflpage(hashp, old_bufp);
					if (!old_bufp)
						return (-1);
					op = (u_int16_t *)old_bufp->page;
					ref_putpair((char *)op, &key, &val);
				}
				old_bufp->flags |= BUF_MOD;
			} else {
				/* Move to new page */
				if (PAIRFITS(np, (&key), (&val)))
					ref_putpair((char *)np, &key, &val);
				else {
					new_bufp =
					    ref___add_ovflpage(hashp, new_bufp);
					if (!new_bufp)
						return (-1);
					np = (u_int16_t *)new_bufp->page;
					ref_putpair((char *)np, &key, &val);
				}
				new_bufp->flags |= BUF_MOD;
			}
		}
	}
	if (last_bfp)
		ref___free_ovflpage(hashp, last_bfp);
	return (0);
}

/*
 * Add the given pair to the page
 *
 * Returns:
 *	0 ==> OK
 *	1 ==> failure
 */
int
ref___addel(HTAB *hashp, BUFHEAD *bufp, const DBT *key, const DBT *val)
{
	u_int16_t *bp, *sop;
	int do_expand;

	bp = (u_int16_t *)bufp->page;
	do_expand = 0;
	while (bp[0] && (bp[2] < REAL_KEY || bp[bp[0]] < REAL_KEY))
		/* Exception case */
		if (bp[2] == FULL_KEY_DATA && bp[0] == 2)
			/* This is the last page of a big key/data pair
			   and we need to add another page */
			break;
		else if (bp[2] < REAL_KEY && bp[bp[0]] != OVFLPAGE) {
			bufp = __get_buf(hashp, bp[bp[0] - 1], bufp, 0);
			if (!bufp)
				return (-1);
			bp = (u_int16_t *)bufp->page;
		} else if (bp[bp[0]] != OVFLPAGE) {
			/* Short key/data pairs, no more pages */
			break;
		} else {
			/* Try to squeeze key on this page */
			if (bp[2] >= REAL_KEY &&
			    FREESPACE(bp) >= PAIRSIZE(key, val)) {
				ref_squeeze_key(bp, key, val);
				goto stats;
			} else {
				bufp = __get_buf(hashp, bp[bp[0] - 1], bufp, 0);
				if (!bufp)
					return (-1);
				bp = (u_int16_t *)bufp->page;
			}
		}

	if (PAIRFITS(bp, key, val))
		ref_putpair(bufp->page, key, val);
	else {
		do_expand = 1;
		bufp = ref___add_ovflpage(hashp, bufp);
		if (!bufp)
			return (-1);
		sop = (u_int16_t *)bufp->page;

		if (PAIRFITS(sop, key, val))
			ref_putpair((char *)sop, key, val);
		else
			if (__big_insert(hashp, bufp, key, val))
				return (-1);
	}
stats:
	bufp->flags |= BUF_MOD;
	/*
	 * If the average number of keys per bucket exceeds the fill factor,
	 * expand the table.
	 */
	hashp->NKEYS++;
	if (do_expand ||
	    (hashp->NKEYS / (hashp->MAX_BUCKET + 1) > hashp->FFACTOR))
		return (__expand_table(hashp));
	return (0);
}

/*
 *
 * Returns:
 *	pointer on success
 *	NULL on error
 */
BUFHEAD *
ref___add_ovflpage(HTAB *hashp, BUFHEAD *bufp)
{
	u_int16_t *sp, ndx, ovfl_num;
#ifdef DEBUG1
	int tmp1, tmp2;
#endif
	sp = (u_int16_t *)bufp->page;

	/* Check if we are dynamically determining the fill factor */
	if (hashp->FFACTOR == DEF_FFACTOR) {
		hashp->FFACTOR = sp[0] >> 1;
		if (hashp->FFACTOR < MIN_FFACTOR)
			hashp->FFACTOR = MIN_FFACTOR;
	}
	bufp->flags |= BUF_MOD;
	ovfl_num = ref_overflow_page(hashp);
#ifdef DEBUG1
	tmp1 = bufp->addr;
	tmp2 = bufp->ovfl ? bufp->ovfl->addr : 0;
#endif
	if (!ovfl_num || !(bufp->ovfl = __get_buf(hashp, ovfl_num, bufp, 1)))
		return (NULL);
	bufp->ovfl->flags |= BUF_MOD;
#ifdef DEBUG1
	(void)fprintf(stderr, "ADDOVFLPAGE: %d->ovfl was %d is now %d\n",
	    tmp1, tmp2, bufp->ovfl->addr);
#endif
	ndx = sp[0];
	/*
	 * Since a pair is allocated on a page only if there's room to add
	 * an overflow page, we know that the OVFL information will fit on
	 * the page.
	 */
	sp[ndx + 4] = OFFSET(sp);
	sp[ndx + 3] = FREESPACE(sp) - OVFLSIZE;
	sp[ndx + 1] = ovfl_num;
	sp[ndx + 2] = OVFLPAGE;
	sp[0] = ndx + 2;
#ifdef HASH_STATISTICS
	hash_overflows++;
#endif
	return (bufp->ovfl);
}

/*
 * Returns:
 *	 0 indicates SUCCESS
 *	-1 indicates FAILURE
 */
int
ref___get_page(HTAB *hashp, char *p, u_int32_t bucket, int is_bucket, int is_disk,
    int is_bitmap)
{
	int fd, page, size, rsize;
	u_int16_t *bp;

	fd = hashp->fp;
	size = hashp->BSIZE;

	if ((fd == -1) || !is_disk) {
		PAGE_INIT(p);
		return (0);
	}
	if (is_bucket)
		page = BUCKET_TO_PAGE(bucket);
	else
		page = OADDR_TO_PAGE(bucket);
	if ((rsize = pread(fd, p, size, (off_t)page << hashp->BSHIFT)) == -1)
		return (-1);
	bp = (u_int16_t *)p;
	if (!rsize)
		bp[0] = 0;	/* We hit the EOF, so initialize a new page */
	else
		if (rsize != size) {
			errno = EFTYPE;
			return (-1);
		}
	if (!is_bitmap && !bp[0]) {
		PAGE_INIT(p);
	} else
		if (hashp->LORDER != BYTE_ORDER) {
			int i, max;

			if (is_bitmap) {
				max = hashp->BSIZE >> 2; /* divide by 4 */
				for (i = 0; i < max; i++)
					M_32_SWAP(((int *)p)[i]);
			} else {
				M_16_SWAP(bp[0]);
				max = bp[0] + 2;
				for (i = 1; i <= max; i++)
					M_16_SWAP(bp[i]);
			}
		}
	return (0);
}

/*
 * Write page p to disk
 *
 * Returns:
 *	 0 ==> OK
 *	-1 ==>failure
 */
int
ref___put_page(HTAB *hashp, char *p, u_int32_t bucket, int is_bucket, int is_bitmap)
{
	int fd, page, size;
	ssize_t wsize;
	char pbuf[MAX_BSIZE];

	size = hashp->BSIZE;
	if ((hashp->fp == -1) && ref_open_temp(hashp))
		return (-1);
	fd = hashp->fp;

	if (hashp->LORDER != BYTE_ORDER) {
		int i, max;

		memcpy(pbuf, p, size);
		if (is_bitmap) {
			max = hashp->BSIZE >> 2;	/* divide by 4 */
			for (i = 0; i < max; i++)
				M_32_SWAP(((int *)pbuf)[i]);
		} else {
			uint16_t *bp = (uint16_t *)(void *)pbuf;
			max = bp[0] + 2;
			for (i = 0; i <= max; i++)
				M_16_SWAP(bp[i]);
		}
		p = pbuf;
	}
	if (is_bucket)
		page = BUCKET_TO_PAGE(bucket);
	else
		page = OADDR_TO_PAGE(bucket);
	if ((wsize = pwrite(fd, p, size, (off_t)page << hashp->BSHIFT)) == -1)
		/* Errno is set */
		return (-1);
	if (wsize != size) {
		errno = EFTYPE;
		return (-1);
	}
	return (0);
}

#define BYTE_MASK	((1 << INT_BYTE_SHIFT) -1)
/*
 * Initialize a new bitmap page.  Bitmap pages are left in memory
 * once they are read in.
 */
int
ref___ibitmap(HTAB *hashp, int pnum, int nbits, int ndx)
{
	u_int32_t *ip;
	int clearbytes, clearints;

	if ((ip = (u_int32_t *)malloc(hashp->BSIZE)) == NULL)
		return (1);
	hashp->nmaps++;
	clearints = ((nbits - 1) >> INT_BYTE_SHIFT) + 1;
	clearbytes = clearints << INT_TO_BYTE;
	(void)memset((char *)ip, 0, clearbytes);
	(void)memset(((char *)ip) + clearbytes, 0xFF,
	    hashp->BSIZE - clearbytes);
	ip[clearints - 1] = ALL_SET << (nbits & BYTE_MASK);
	SETBIT(ip, 0);
	hashp->BITMAPS[ndx] = (u_int16_t)pnum;
	hashp->mapp[ndx] = ip;
	return (0);
}

u_int32_t
ref_first_free(u_int32_t map)
{
	u_int32_t i, mask;

	mask = 0x1;
	for (i = 0; i < BITS_PER_MAP; i++) {
		if (!(mask & map))
			return (i);
		mask = mask << 1;
	}
	return (i);
}

u_int16_t
ref_overflow_page(HTAB *hashp)
{
	u_int32_t *freep;
	int max_free, offset, splitnum;
	u_int16_t addr;
	int bit, first_page, free_bit, free_page, i, in_use_bits, j;
#ifdef DEBUG2
	int tmp1, tmp2;
#endif
	splitnum = hashp->OVFL_POINT;
	max_free = hashp->SPARES[splitnum];

	free_page = (max_free - 1) >> (hashp->BSHIFT + BYTE_SHIFT);
	free_bit = (max_free - 1) & ((hashp->BSIZE << BYTE_SHIFT) - 1);

	/* Look through all the free maps to find the first free block */
	first_page = hashp->LAST_FREED >>(hashp->BSHIFT + BYTE_SHIFT);
	for ( i = first_page; i <= free_page; i++ ) {
		if (!(freep = (u_int32_t *)hashp->mapp[i]) &&
		    !(freep = ref_fetch_bitmap(hashp, i)))
			return (0);
		if (i == free_page)
			in_use_bits = free_bit;
		else
			in_use_bits = (hashp->BSIZE << BYTE_SHIFT) - 1;

		if (i == first_page) {
			bit = hashp->LAST_FREED &
			    ((hashp->BSIZE << BYTE_SHIFT) - 1);
			j = bit / BITS_PER_MAP;
			bit = rounddown2(bit, BITS_PER_MAP);
		} else {
			bit = 0;
			j = 0;
		}
		for (; bit <= in_use_bits; j++, bit += BITS_PER_MAP)
			if (freep[j] != ALL_SET)
				goto found;
	}

	/* No Free Page Found */
	hashp->LAST_FREED = hashp->SPARES[splitnum];
	hashp->SPARES[splitnum]++;
	offset = hashp->SPARES[splitnum] -
	    (splitnum ? hashp->SPARES[splitnum - 1] : 0);

#define	OVMSG	"HASH: Out of overflow pages.  Increase page size\n"
	if (offset > SPLITMASK) {
		if (++splitnum >= NCACHED) {
			(void)_write(STDERR_FILENO, OVMSG, sizeof(OVMSG) - 1);
			errno = EFBIG;
			return (0);
		}
		hashp->OVFL_POINT = splitnum;
		hashp->SPARES[splitnum] = hashp->SPARES[splitnum-1];
		hashp->SPARES[splitnum-1]--;
		offset = 1;
	}

	/* Check if we need to allocate a new bitmap page */
	if (free_bit == (hashp->BSIZE << BYTE_SHIFT) - 1) {
		free_page++;
		if (free_page >= NCACHED) {
			(void)_write(STDERR_FILENO, OVMSG, sizeof(OVMSG) - 1);
			errno = EFBIG;
			return (0);
		}
		/*
		 * This is tricky.  The 1 indicates that you want the new page
		 * allocated with 1 clear bit.  Actually, you are going to
		 * allocate 2 pages from this map.  The first is going to be
		 * the map page, the second is the overflow page we were
		 * looking for.  The init_bitmap routine automatically, sets
		 * the first bit of itself to indicate that the bitmap itself
		 * is in use.  We would explicitly set the second bit, but
		 * don't have to if we tell init_bitmap not to leave it clear
		 * in the first place.
		 */
		if (ref___ibitmap(hashp,
		    (int)OADDR_OF(splitnum, offset), 1, free_page))
			return (0);
		hashp->SPARES[splitnum]++;
#ifdef DEBUG2
		free_bit = 2;
#endif
		offset++;
		if (offset > SPLITMASK) {
			if (++splitnum >= NCACHED) {
				(void)_write(STDERR_FILENO, OVMSG,
				    sizeof(OVMSG) - 1);
				errno = EFBIG;
				return (0);
			}
			hashp->OVFL_POINT = splitnum;
			hashp->SPARES[splitnum] = hashp->SPARES[splitnum-1];
			hashp->SPARES[splitnum-1]--;
			offset = 0;
		}
	} else {
		/*
		 * Free_bit addresses the last used bit.  Bump it to address
		 * the first available bit.
		 */
		free_bit++;
		SETBIT(freep, free_bit);
	}

	/* Calculate address of the new overflow page */
	addr = OADDR_OF(splitnum, offset);
#ifdef DEBUG2
	(void)fprintf(stderr, "OVERFLOW_PAGE: ADDR: %d BIT: %d PAGE %d\n",
	    addr, free_bit, free_page);
#endif
	return (addr);

found:
	bit = bit + ref_first_free(freep[j]);
	SETBIT(freep, bit);
#ifdef DEBUG2
	tmp1 = bit;
	tmp2 = i;
#endif
	/*
	 * Bits are addressed starting with 0, but overflow pages are addressed
	 * beginning at 1. Bit is a bit addressnumber, so we need to increment
	 * it to convert it to a page number.
	 */
	bit = 1 + bit + (i * (hashp->BSIZE << BYTE_SHIFT));
	if (bit >= hashp->LAST_FREED)
		hashp->LAST_FREED = bit - 1;

	/* Calculate the split number for this page */
	for (i = 0; (i < splitnum) && (bit > hashp->SPARES[i]); i++);
	offset = (i ? bit - hashp->SPARES[i - 1] : bit);
	if (offset >= SPLITMASK) {
		(void)_write(STDERR_FILENO, OVMSG, sizeof(OVMSG) - 1);
		errno = EFBIG;
		return (0);	/* Out of overflow pages */
	}
	addr = OADDR_OF(i, offset);
#ifdef DEBUG2
	(void)fprintf(stderr, "OVERFLOW_PAGE: ADDR: %d BIT: %d PAGE %d\n",
	    addr, tmp1, tmp2);
#endif

	/* Allocate and return the overflow page */
	return (addr);
}

/*
 * Mark this overflow page as free.
 */
void
ref___free_ovflpage(HTAB *hashp, BUFHEAD *obufp)
{
	u_int16_t addr;
	u_int32_t *freep;
	int bit_address, free_page, free_bit;
	u_int16_t ndx;

	addr = obufp->addr;
#ifdef DEBUG1
	(void)fprintf(stderr, "Freeing %d\n", addr);
#endif
	ndx = (((u_int16_t)addr) >> SPLITSHIFT);
	bit_address =
	    (ndx ? hashp->SPARES[ndx - 1] : 0) + (addr & SPLITMASK) - 1;
	 if (bit_address < hashp->LAST_FREED)
		hashp->LAST_FREED = bit_address;
	free_page = (bit_address >> (hashp->BSHIFT + BYTE_SHIFT));
	free_bit = bit_address & ((hashp->BSIZE << BYTE_SHIFT) - 1);

	if (!(freep = hashp->mapp[free_page]))
		freep = ref_fetch_bitmap(hashp, free_page);
#ifdef DEBUG
	/*
	 * This had better never happen.  It means we tried to read a bitmap
	 * that has already had overflow pages allocated off it, and we
	 * failed to read it from the file.
	 */
	if (!freep)
		assert(0);
#endif
	CLRBIT(freep, free_bit);
#ifdef DEBUG2
	(void)fprintf(stderr, "FREE_OVFLPAGE: ADDR: %d BIT: %d PAGE %d\n",
	    obufp->addr, free_bit, free_page);
#endif
	__reclaim_buf(hashp, obufp);
}

/*
 * Returns:
 *	 0 success
 *	-1 failure
 */
int
ref_open_temp(HTAB *hashp)
{
	sigset_t set, oset;
	int len;
	char *envtmp;
	char path[MAXPATHLEN];

	envtmp = secure_getenv("TMPDIR");
	len = snprintf(path,
	    sizeof(path), "%s/_hash.XXXXXX", envtmp ? envtmp : "/tmp");
	if (len < 0 || len >= (int)sizeof(path)) {
		errno = ENAMETOOLONG;
		return (-1);
	}

	/* Block signals; make sure file goes away at process exit. */
	(void)sigfillset(&set);
	(void)__libc_sigprocmask(SIG_BLOCK, &set, &oset);
	if ((hashp->fp = mkostemp(path, O_CLOEXEC)) != -1)
		(void)unlink(path);
	(void)__libc_sigprocmask(SIG_SETMASK, &oset, (sigset_t *)NULL);
	return (hashp->fp != -1 ? 0 : -1);
}

/*
 * We have to know that the key will fit, but the last entry on the page is
 * an overflow pair, so we need to shift things.
 */
void
ref_squeeze_key(u_int16_t *sp, const DBT *key, const DBT *val)
{
	char *p;
	u_int16_t free_space, n, off, pageno;

	p = (char *)sp;
	n = sp[0];
	free_space = FREESPACE(sp);
	off = OFFSET(sp);

	pageno = sp[n - 1];
	off -= key->size;
	sp[n - 1] = off;
	memmove(p + off, key->data, key->size);
	off -= val->size;
	sp[n] = off;
	memmove(p + off, val->data, val->size);
	sp[0] = n + 2;
	sp[n + 1] = pageno;
	sp[n + 2] = OVFLPAGE;
	FREESPACE(sp) = free_space - PAIRSIZE(key, val);
	OFFSET(sp) = off;
}

u_int32_t *
ref_fetch_bitmap(HTAB *hashp, int ndx)
{
	if (ndx >= hashp->nmaps)
		return (NULL);
	if ((hashp->mapp[ndx] = (u_int32_t *)malloc(hashp->BSIZE)) == NULL)
		return (NULL);
	if (ref___get_page(hashp,
	    (char *)hashp->mapp[ndx], hashp->BITMAPS[ndx], 0, 1, 1)) {
		free(hashp->mapp[ndx]);
		return (NULL);
	}
	return (hashp->mapp[ndx]);
}

/* ------------------------------------------------------------------ */
/* Harness mock layer for out-of-batch dependencies.                  */
/* ------------------------------------------------------------------ */

typedef struct {
	unsigned get_buf_calls, call_hash_calls, big_delete_calls, big_split_calls,
	    big_insert_calls, expand_calls, reclaim_calls;
	unsigned mkostemp_calls, getenv_calls, sigmask_calls, write_calls;
	int get_buf_force_null, get_buf_fail_after, call_hash_ret, big_delete_ret,
	    big_split_ret, big_insert_ret, expand_ret;
	int mkostemp_ret;
	char *getenv_val;
	int write_ret;
	u_int32_t next_buf_addr;
	int nbufs;
} hash_mock_state;

hash_mock_state hash_mock;

#define MOCK_MAX_BUFS 64
#define MOCK_MAX_REG 16

static BUFHEAD mock_bufs[MOCK_MAX_BUFS];
static char mock_pages[MOCK_MAX_BUFS][576];
static int mock_nbufs;

static struct {
	u_int32_t addr;
	void *page;
} mock_registered[MOCK_MAX_REG];
static int mock_nreg;

void
hash_mock_reset(void)
{
	memset(&hash_mock, 0, sizeof(hash_mock));
	mock_nbufs = 0;
	mock_nreg = 0;
	memset(mock_bufs, 0, sizeof(mock_bufs));
}

void
hash_mock_register_page(u_int32_t addr, void *page)
{
	if (mock_nreg < MOCK_MAX_REG) {
		mock_registered[mock_nreg].addr = addr;
		mock_registered[mock_nreg].page = page;
		mock_nreg++;
	}
}

static void *
mock_lookup_page(u_int32_t addr)
{
	int i;

	for (i = 0; i < mock_nreg; i++)
		if (mock_registered[i].addr == addr)
			return (mock_registered[i].page);
	return (NULL);
}

u_int32_t
__log2(u_int32_t num)
{
	u_int32_t i, limit;

	limit = 1;
	for (i = 0; limit < num; limit = limit << 1, i++)
		;
	return (i);
}

static void
mock_page_init(HTAB *hashp, char *p)
{
	u_int16_t *bp = (u_int16_t *)p;

	bp[0] = 0;
	bp[1] = hashp->BSIZE - 3 * sizeof(u_int16_t);
	bp[2] = hashp->BSIZE;
}

BUFHEAD *
__get_buf(HTAB *hashp, u_int32_t addr, BUFHEAD *prev, int newpage)
{
	BUFHEAD *bp;
	void *pg;

	(void)prev;
	hash_mock.get_buf_calls++;
	if (hash_mock.get_buf_force_null)
		return (NULL);
	if (hash_mock.get_buf_fail_after > 0) {
		hash_mock.get_buf_fail_after--;
		if (hash_mock.get_buf_fail_after == 0)
			return (NULL);
	}
	if (mock_nbufs >= MOCK_MAX_BUFS)
		return (NULL);

	bp = &mock_bufs[mock_nbufs++];
	pg = mock_lookup_page(addr);
	if (pg != NULL)
		bp->page = (char *)pg;
	else {
		bp->page = mock_pages[mock_nbufs - 1];
		mock_page_init(hashp, bp->page);
	}
	bp->addr = addr;
	bp->ovfl = NULL;
	bp->flags = 0;
	bp->prev = bp->next = NULL;
	(void)newpage;
	return (bp);
}

u_int32_t
__call_hash(HTAB *hashp, char *key, int size)
{
	(void)hashp;
	(void)key;
	(void)size;
	hash_mock.call_hash_calls++;
	return ((u_int32_t)hash_mock.call_hash_ret);
}

int
__big_delete(HTAB *hashp, BUFHEAD *bufp)
{
	(void)hashp;
	(void)bufp;
	hash_mock.big_delete_calls++;
	return (hash_mock.big_delete_ret);
}

int
__big_split(HTAB *hashp, BUFHEAD *old_bufp, BUFHEAD *new_bufp, BUFHEAD *bufp,
    int addr, u_int32_t obucket, SPLIT_RETURN *ret)
{
	(void)addr;
	(void)obucket;
	hash_mock.big_split_calls++;
	if (hash_mock.big_split_ret != 0)
		return (hash_mock.big_split_ret);
	ret->oldp = old_bufp;
	ret->newp = new_bufp;
	ret->nextp = NULL;
	(void)bufp;
	(void)hashp;
	return (0);
}

int
__big_insert(HTAB *hashp, BUFHEAD *bufp, const DBT *key, const DBT *val)
{
	(void)hashp;
	(void)bufp;
	(void)key;
	(void)val;
	hash_mock.big_insert_calls++;
	return (hash_mock.big_insert_ret);
}

int
__expand_table(HTAB *hashp)
{
	(void)hashp;
	hash_mock.expand_calls++;
	return (hash_mock.expand_ret);
}

void
__reclaim_buf(HTAB *hashp, BUFHEAD *obufp)
{
	(void)hashp;
	(void)obufp;
	hash_mock.reclaim_calls++;
}

int
__libc_sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
	hash_mock.sigmask_calls++;
	return (sigprocmask(how, set, oset));
}

ssize_t
_write(int fd, const void *buf, size_t nbyte)
{
	hash_mock.write_calls++;
	if (hash_mock.write_ret != 0)
		return (hash_mock.write_ret);
	return (write(fd, buf, nbyte));
}

char *
secure_getenv(const char *name)
{
	(void)name;
	hash_mock.getenv_calls++;
	if (hash_mock.getenv_val != NULL)
		return (hash_mock.getenv_val);
	return (getenv("TMPDIR"));
}
