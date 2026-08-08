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

module;

#include <sys/stat.h>
#include <sys/types.h>

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

export module pbsd.lib.libc.db.mpool.b0226;

export namespace pbsd::lib_libc_db_mpool::b0226 {

#define TRACEBUF
#define TRACEBUF_INITIALIZER
#define QMD_TRACE_ELEM(elem)
#define QMD_TRACE_HEAD(head)
#define QMD_SAVELINK(name, link)
#define TRASHIT(x)
#define QMD_ASSERT(test, fmt, ...) do {} while (0)

#define TAILQ_HEAD(name, type)						\
struct name {								\
	struct type *tqh_first;						\
	struct type **tqh_last;						\
	TRACEBUF								\
}

#define TAILQ_ENTRY(type)						\
struct {								\
	struct type *tqe_next;						\
	struct type **tqe_prev;						\
	TRACEBUF								\
}

#define TAILQ_EMPTY(head)	((head)->tqh_first == NULL)
#define TAILQ_FIRST(head)	((head)->tqh_first)
#define TAILQ_NEXT(elm, field) ((elm)->field.tqe_next)

#define TAILQ_FOREACH(var, head, field)					\
	for ((var) = TAILQ_FIRST((head));				\
	    (var);							\
	    (var) = TAILQ_NEXT((var), field))

#define TAILQ_INIT(head) do {						\
	TAILQ_FIRST((head)) = NULL;					\
	(head)->tqh_last = &TAILQ_FIRST((head));			\
} while (0)

#define TAILQ_INSERT_HEAD(head, elm, field) do {			\
	if ((TAILQ_NEXT((elm), field) = TAILQ_FIRST((head))) != NULL)	\
		TAILQ_FIRST((head))->field.tqe_prev =			\
		    &TAILQ_NEXT((elm), field);				\
	else								\
		(head)->tqh_last = &TAILQ_NEXT((elm), field);		\
	TAILQ_FIRST((head)) = (elm);					\
	(elm)->field.tqe_prev = &TAILQ_FIRST((head));			\
} while (0)

#define TAILQ_INSERT_TAIL(head, elm, field) do {			\
	TAILQ_NEXT((elm), field) = NULL;				\
	(elm)->field.tqe_prev = (head)->tqh_last;			\
	*(head)->tqh_last = (elm);					\
	(head)->tqh_last = &TAILQ_NEXT((elm), field);			\
} while (0)

#define TAILQ_REMOVE(head, elm, field) do {				\
	if ((TAILQ_NEXT((elm), field)) != NULL)				\
		TAILQ_NEXT((elm), field)->field.tqe_prev =		\
		    (elm)->field.tqe_prev;				\
	else								\
		(head)->tqh_last = (elm)->field.tqe_prev;		\
	*(elm)->field.tqe_prev = TAILQ_NEXT((elm), field);		\
} while (0)

#define RET_ERROR	-1
#define RET_SUCCESS	 0

#define MAX_PAGE_NUMBER	0xffffffffu
using pgno_t = std::uint32_t;

using u_int = unsigned int;
using u_int8_t = std::uint8_t;

#define HASHSIZE	128
#define HASHKEY(pgno)	((pgno - 1 + HASHSIZE) % HASHSIZE)

#define MPOOL_DIRTY	0x01
#define MPOOL_PINNED	0x02
#define MPOOL_INUSE	0x04

#define MPOOL_IGNOREPIN	0x01
#define MPOOL_PAGE_REQUEST	0x01
#define MPOOL_PAGE_NEXT		0x02

struct _bkt {
	TAILQ_ENTRY(_bkt) hq;
	TAILQ_ENTRY(_bkt) q;
	void    *page;
	pgno_t   pgno;
	u_int8_t flags;
};
using BKT = struct _bkt;

struct MPOOL {
	TAILQ_HEAD(_lqh, _bkt) lqh;
	TAILQ_HEAD(_hqh, _bkt) hqh[HASHSIZE];
	pgno_t	curcache;
	pgno_t	maxcache;
	pgno_t	npages;
	unsigned long	pagesize;
	int	fd;
	void    (*pgin)(void *, pgno_t, void *);
	void    (*pgout)(void *, pgno_t, void *);
	void	*pgcookie;
};

static BKT *mpool_bkt(MPOOL *);
static BKT *mpool_look(MPOOL *, pgno_t);
static int  mpool_write(MPOOL *, BKT *);

MPOOL *
mpool_open(void *key, int fd, pgno_t pagesize, pgno_t maxcache)
{
	struct stat sb;
	MPOOL *mp;
	int entry;

	/*
	 * Get information about the file.
	 *
	 * XXX
	 * We don't currently handle pipes, although we should.
	 */
	if (fstat(fd, &sb))
		return (NULL);
	if (!S_ISREG(sb.st_mode)) {
		errno = ESPIPE;
		return (NULL);
	}

	/* Allocate and initialize the MPOOL cookie. */
	if ((mp = (MPOOL *)calloc(1, sizeof(MPOOL))) == NULL)
		return (NULL);
	TAILQ_INIT(&mp->lqh);
	for (entry = 0; entry < HASHSIZE; ++entry)
		TAILQ_INIT(&mp->hqh[entry]);
	mp->maxcache = maxcache;
	mp->npages = sb.st_size / pagesize;
	mp->pagesize = pagesize;
	mp->fd = fd;
	return (mp);
}

void
mpool_filter(MPOOL *mp, void (*pgin) (void *, pgno_t, void *),
    void (*pgout) (void *, pgno_t, void *), void *pgcookie)
{
	mp->pgin = pgin;
	mp->pgout = pgout;
	mp->pgcookie = pgcookie;
}
	
void *
mpool_new(MPOOL *mp, pgno_t *pgnoaddr, u_int flags)
{
	struct _hqh *head;
	BKT *bp;

	if (mp->npages == MAX_PAGE_NUMBER) {
		(void)fprintf(stderr, "mpool_new: page allocation overflow.\n");
		abort();
	}
#ifdef STATISTICS
	++mp->pagenew;
#endif
	/*
	 * Get a BKT from the cache.  Assign a new page number, attach
	 * it to the head of the hash chain, the tail of the lru chain,
	 * and return.
	 */
	if ((bp = mpool_bkt(mp)) == NULL)
		return (NULL);
	if (flags == MPOOL_PAGE_REQUEST) {
		mp->npages++;
		bp->pgno = *pgnoaddr;
	} else
		bp->pgno = *pgnoaddr = mp->npages++;

	bp->flags = MPOOL_PINNED | MPOOL_INUSE;

	head = &mp->hqh[HASHKEY(bp->pgno)];
	TAILQ_INSERT_HEAD(head, bp, hq);
	TAILQ_INSERT_TAIL(&mp->lqh, bp, q);
	return (bp->page);
}

int
mpool_delete(MPOOL *mp, void *page)
{
	struct _hqh *head;
	BKT *bp;

	bp = (BKT *)((char *)page - sizeof(BKT));

#ifdef DEBUG
	if (!(bp->flags & MPOOL_PINNED)) {
		(void)fprintf(stderr,
		    "mpool_delete: page %d not pinned\n", bp->pgno);
		abort();
	}
#endif

	/* Remove from the hash and lru queues. */
	head = &mp->hqh[HASHKEY(bp->pgno)];
	TAILQ_REMOVE(head, bp, hq);
	TAILQ_REMOVE(&mp->lqh, bp, q);

	free(bp);
	mp->curcache--;
	return (RET_SUCCESS);
}	
	
void *
mpool_get(MPOOL *mp, pgno_t pgno,
    u_int flags)		/* XXX not used? */
{
	struct _hqh *head;
	BKT *bp;
	off_t off;
	int nr;

#ifdef STATISTICS
	++mp->pageget;
#endif

	/* Check for a page that is cached. */
	if ((bp = mpool_look(mp, pgno)) != NULL) {
#ifdef DEBUG
		if (!(flags & MPOOL_IGNOREPIN) && bp->flags & MPOOL_PINNED) {
			(void)fprintf(stderr,
			    "mpool_get: page %d already pinned\n", bp->pgno);
			abort();
		}
#endif
		/*
		 * Move the page to the head of the hash chain and the tail
		 * of the lru chain.
		 */
		head = &mp->hqh[HASHKEY(bp->pgno)];
		TAILQ_REMOVE(head, bp, hq);
		TAILQ_INSERT_HEAD(head, bp, hq);
		TAILQ_REMOVE(&mp->lqh, bp, q);
		TAILQ_INSERT_TAIL(&mp->lqh, bp, q);

		/* Return a pinned page. */
		bp->flags |= MPOOL_PINNED;
		return (bp->page);
	}

	/* Get a page from the cache. */
	if ((bp = mpool_bkt(mp)) == NULL)
		return (NULL);

	/* Read in the contents. */
	off = mp->pagesize * pgno;
	if ((nr = pread(mp->fd, bp->page, mp->pagesize, off)) != (ssize_t)mp->pagesize) {
		switch (nr) {
		case -1:
			/* errno is set for us by pread(). */
			free(bp);
			mp->curcache--;
			return (NULL);
		case 0:
			/*
			 * A zero-length read means you need to create a
			 * new page.
			 */
			memset(bp->page, 0, mp->pagesize);
			break;
		default:
			/* A partial read is definitely bad. */
			free(bp);
			mp->curcache--;
			errno = EINVAL;
			return (NULL);
		}
	}
#ifdef STATISTICS
	++mp->pageread;
#endif

	/* Set the page number, pin the page. */
	bp->pgno = pgno;
	if (!(flags & MPOOL_IGNOREPIN))
		bp->flags = MPOOL_PINNED;
	bp->flags |= MPOOL_INUSE;

	/*
	 * Add the page to the head of the hash chain and the tail
	 * of the lru chain.
	 */
	head = &mp->hqh[HASHKEY(bp->pgno)];
	TAILQ_INSERT_HEAD(head, bp, hq);
	TAILQ_INSERT_TAIL(&mp->lqh, bp, q);

	/* Run through the user's filter. */
	if (mp->pgin != NULL)
		(mp->pgin)(mp->pgcookie, bp->pgno, bp->page);

	return (bp->page);
}

int
mpool_put(MPOOL *mp, void *page, u_int flags)
{
	BKT *bp;

#ifdef STATISTICS
	++mp->pageput;
#endif
	bp = (BKT *)((char *)page - sizeof(BKT));
#ifdef DEBUG
	if (!(bp->flags & MPOOL_PINNED)) {
		(void)fprintf(stderr,
		    "mpool_put: page %d not pinned\n", bp->pgno);
		abort();
	}
#endif
	bp->flags &= ~MPOOL_PINNED;
	if (flags & MPOOL_DIRTY)
		bp->flags |= flags & MPOOL_DIRTY;
	return (RET_SUCCESS);
}

int
mpool_close(MPOOL *mp)
{
	BKT *bp;

	/* Free up any space allocated to the lru pages. */
	while (!TAILQ_EMPTY(&mp->lqh)) {
		bp = TAILQ_FIRST(&mp->lqh);
		TAILQ_REMOVE(&mp->lqh, bp, q);
		free(bp);
	}

	/* Free the MPOOL cookie. */
	free(mp);
	return (RET_SUCCESS);
}

int
mpool_sync(MPOOL *mp)
{
	BKT *bp;

	/* Walk the lru chain, flushing any dirty pages to disk. */
	TAILQ_FOREACH(bp, &mp->lqh, q)
		if (bp->flags & MPOOL_DIRTY &&
		    mpool_write(mp, bp) == RET_ERROR)
			return (RET_ERROR);

	/* Sync the file descriptor. */
	return (fsync(mp->fd) ? RET_ERROR : RET_SUCCESS);
}

static BKT *
mpool_bkt(MPOOL *mp)
{
	struct _hqh *head;
	BKT *bp;

	/* If under the max cached, always create a new page. */
	if (mp->curcache < mp->maxcache)
		goto new;

	/*
	 * If the cache is max'd out, walk the lru list for a buffer we
	 * can flush.  If we find one, write it (if necessary) and take it
	 * off any lists.  If we don't find anything we grow the cache anyway.
	 * The cache never shrinks.
	 */
	TAILQ_FOREACH(bp, &mp->lqh, q)
		if (!(bp->flags & MPOOL_PINNED)) {
			/* Flush if dirty. */
			if (bp->flags & MPOOL_DIRTY &&
			    mpool_write(mp, bp) == RET_ERROR)
				return (NULL);
#ifdef STATISTICS
			++mp->pageflush;
#endif
			/* Remove from the hash and lru queues. */
			head = &mp->hqh[HASHKEY(bp->pgno)];
			TAILQ_REMOVE(head, bp, hq);
			TAILQ_REMOVE(&mp->lqh, bp, q);
#ifdef DEBUG
			{ void *spage;
				spage = bp->page;
				memset(bp, 0xff, sizeof(BKT) + mp->pagesize);
				bp->page = spage;
			}
#endif
			bp->flags = 0;
			return (bp);
		}

new:	if ((bp = (BKT *)calloc(1, sizeof(BKT) + mp->pagesize)) == NULL)
		return (NULL);
#ifdef STATISTICS
	++mp->pagealloc;
#endif
	bp->page = (char *)bp + sizeof(BKT);
	bp->flags = 0;
	++mp->curcache;
	return (bp);
}

static int
mpool_write(MPOOL *mp, BKT *bp)
{
	off_t off;

#ifdef STATISTICS
	++mp->pagewrite;
#endif

	/* Run through the user's filter. */
	if (mp->pgout)
		(mp->pgout)(mp->pgcookie, bp->pgno, bp->page);

	off = mp->pagesize * bp->pgno;
	if (pwrite(mp->fd, bp->page, mp->pagesize, off) != (ssize_t)mp->pagesize)
		return (RET_ERROR);

	/*
	 * Re-run through the input filter since this page may soon be
	 * accessed via the cache, and whatever the user's output filter
	 * did may screw things up if we don't let the input filter
	 * restore the in-core copy.
	 */
	if (mp->pgin)
		(mp->pgin)(mp->pgcookie, bp->pgno, bp->page);

	bp->flags &= ~MPOOL_DIRTY;
	return (RET_SUCCESS);
}

static BKT *
mpool_look(MPOOL *mp, pgno_t pgno)
{
	struct _hqh *head;
	BKT *bp;

	head = &mp->hqh[HASHKEY(pgno)];
	TAILQ_FOREACH(bp, head, hq)
		if ((bp->pgno == pgno) &&
			((bp->flags & MPOOL_INUSE) == MPOOL_INUSE)) {
#ifdef STATISTICS
			++mp->cachehit;
#endif
			return (bp);
		}
#ifdef STATISTICS
	++mp->cachemiss;
#endif
	return (NULL);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2009 Xin LI <delphij@FreeBSD.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

void *
__mpool_new__44bsd(MPOOL *mp, pgno_t *pgnoaddr)
{

	return (mpool_new(mp, pgnoaddr, MPOOL_PAGE_NEXT));
}

} // namespace pbsd::lib_libc_db_mpool::b0226
