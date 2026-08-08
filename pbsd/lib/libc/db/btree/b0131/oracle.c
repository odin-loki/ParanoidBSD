/*
 * Oracle for batch b0131: original HardenedBSD btree sources with ref_ prefix.
 * Mocks for external dependencies (mpool, __bt_search, __bt_ret, _close) are
 * provided here and controlled from harness.cpp via test_mock_*.
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#define RET_ERROR	-1
#define RET_SUCCESS	 0
#define RET_SPECIAL	 1

typedef uint32_t	pgno_t;
typedef uint16_t	indx_t;
typedef uint32_t	recno_t;
typedef unsigned int	u_int;
typedef unsigned char	u_char;
typedef uint32_t	u_int32_t;
typedef uint8_t		u_int8_t;
typedef char		* caddr_t;

typedef struct {
	void	*data;
	size_t	 size;
} DBT;

typedef struct __db {
	int (*close)(struct __db *);
	void *internal;
} DB;

#define	BTREEMAGIC	0x053162
#define	BTREEVERSION	3

#define __DBINTERFACE_PRIVATE
#define	M_32_SWAP(a) {							\
	uint32_t _tmp = (a);						\
	((char *)&(a))[0] = ((char *)&_tmp)[3];				\
	((char *)&(a))[1] = ((char *)&_tmp)[2];				\
	((char *)&(a))[2] = ((char *)&_tmp)[1];				\
	((char *)&(a))[3] = ((char *)&_tmp)[0];				\
}
#define	P_32_SWAP(a) {							\
	uint32_t _tmp = *(uint32_t *)(a);				\
	((char *)(a))[0] = ((char *)&_tmp)[3];				\
	((char *)(a))[1] = ((char *)&_tmp)[2];				\
	((char *)(a))[2] = ((char *)&_tmp)[1];				\
	((char *)(a))[3] = ((char *)&_tmp)[0];				\
}
#define	M_16_SWAP(a) {							\
	uint16_t _tmp = (a);						\
	((char *)&(a))[0] = ((char *)&_tmp)[1];				\
	((char *)&(a))[1] = ((char *)&_tmp)[0];				\
}

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

#define	BTDATAOFF							\
	(sizeof(pgno_t) + sizeof(pgno_t) + sizeof(pgno_t) +		\
	    sizeof(u_int32_t) + sizeof(indx_t) + sizeof(indx_t))
#define	NEXTINDEX(p)	(((p)->lower - BTDATAOFF) / sizeof(indx_t))
#define	LALIGN(n)	(((n) + sizeof(pgno_t) - 1) & ~(sizeof(pgno_t) - 1))

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

typedef struct _bleaf {
	u_int32_t	ksize;
	u_int32_t	dsize;
	u_char	flags;
	char	bytes[1];
} BLEAF;

#define	GETBLEAF(pg, indx)						\
	((BLEAF *)((char *)(pg) + (pg)->linp[indx]))

typedef struct _epg {
	PAGE	*page;
	indx_t	 index;
} EPG;

typedef struct _cursor {
	struct {
		pgno_t	pgno;
		indx_t	index;
	} pg;
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
	struct {
		PAGE	*page;
		indx_t	 index;
	} bt_cur;
	PAGE	 *bt_pinned;
	CURSOR	  bt_cursor;
	struct {
		pgno_t	pgno;
		indx_t	index;
	} bt_stack[50];
	struct {
		pgno_t	pgno;
		indx_t	index;
	} *bt_sp;
	DBT	  bt_rkey;
	DBT	  bt_rdata;
	int	  bt_fd;
	pgno_t	  bt_free;
	u_int32_t bt_psize;
	indx_t	  bt_ovflsize;
	int	  bt_lorder;
	enum { NOT, BACK, FORWARD } bt_order;
	struct {
		pgno_t	pgno;
		indx_t	index;
	} bt_last;
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

#define	P_INVALID	 0
#define	P_META		 0

/* ------------------------------------------------------------------ */
/* Harness-controlled mocks					      */
/* ------------------------------------------------------------------ */

typedef struct {
	int put_ret;
	unsigned put_calls;
	void *last_put_page;
	unsigned last_put_flags;

	int get_force_null;
	unsigned get_calls;
	pgno_t get_last_pgno;
	unsigned get_last_flags;

	int new_force_null;
	unsigned new_calls;
	pgno_t new_pgno;
	void *new_page;

	int sync_ret;
	unsigned sync_calls;
	int close_ret;
	unsigned close_calls;

	int search_force_null;
	unsigned search_calls;
	int search_exact;

	int ret_status;
	unsigned ret_calls;
	unsigned char ret_data[256];
	size_t ret_data_size;

	int close_fd_ret;
	unsigned close_fd_calls;
	int close_fd_arg;
} test_mock_state;

test_mock_state test_mock;

void
test_mock_reset(void)
{
	memset(&test_mock, 0, sizeof(test_mock));
	test_mock.put_ret = RET_SUCCESS;
	test_mock.sync_ret = RET_SUCCESS;
	test_mock.close_ret = RET_SUCCESS;
	test_mock.ret_status = RET_SUCCESS;
}

void *
mpool_get(MPOOL *mp, pgno_t pgno, unsigned int flags)
{
	(void)mp;
	test_mock.get_calls++;
	test_mock.get_last_pgno = pgno;
	test_mock.get_last_flags = flags;
	if (test_mock.get_force_null)
		return (NULL);
	return (test_mock.new_page);
}

int
mpool_put(MPOOL *mp, void *page, unsigned int flags)
{
	(void)mp;
	test_mock.put_calls++;
	test_mock.last_put_page = page;
	test_mock.last_put_flags = flags;
	return (test_mock.put_ret);
}

void *
mpool_new(MPOOL *mp, pgno_t *npg, unsigned int flags)
{
	(void)mp;
	(void)flags;
	test_mock.new_calls++;
	if (test_mock.new_force_null)
		return (NULL);
	if (npg != NULL)
		*npg = test_mock.new_pgno;
	return (test_mock.new_page);
}

int
mpool_sync(MPOOL *mp)
{
	(void)mp;
	test_mock.sync_calls++;
	return (test_mock.sync_ret);
}

int
mpool_close(MPOOL *mp)
{
	(void)mp;
	test_mock.close_calls++;
	return (test_mock.close_ret);
}

static EPG search_epg;

EPG *
__bt_search(BTREE *t, const DBT *key, int *exact)
{
	(void)t;
	(void)key;
	test_mock.search_calls++;
	if (test_mock.search_force_null)
		return (NULL);
	*exact = test_mock.search_exact;
	search_epg.page = (PAGE *)test_mock.new_page;
	search_epg.index = 0;
	return (&search_epg);
}

int
__bt_ret(BTREE *t, EPG *e, DBT *key, DBT *rkey, DBT *data, DBT *rdata, int flags)
{
	(void)t;
	(void)e;
	(void)key;
	(void)rkey;
	(void)rdata;
	(void)flags;
	test_mock.ret_calls++;
	if (data != NULL) {
		data->size = test_mock.ret_data_size;
		data->data = test_mock.ret_data;
	}
	return (test_mock.ret_status);
}

int
_close(int fd)
{
	test_mock.close_fd_calls++;
	test_mock.close_fd_arg = fd;
	return (test_mock.close_fd_ret);
}

/* ================================================================== */
/* bt_page.c							      */
/* ================================================================== */

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

int
ref___bt_free(BTREE *t, PAGE *h)
{
	/* Insert the page at the head of the free list. */
	h->prevpg = P_INVALID;
	h->nextpg = t->bt_free;
	t->bt_free = h->pgno;
	F_SET(t, B_METADIRTY);

	/* Make sure the page gets written back. */
	return (mpool_put(t->bt_mp, h, MPOOL_DIRTY));
}

PAGE *
ref___bt_new(BTREE *t, pgno_t *npg)
{
	PAGE *h;

	if (t->bt_free != P_INVALID &&
	    (h = mpool_get(t->bt_mp, t->bt_free, 0)) != NULL) {
		*npg = t->bt_free;
		t->bt_free = h->nextpg;
		F_SET(t, B_METADIRTY);
		return (h);
	}
	return (mpool_new(t->bt_mp, npg, MPOOL_PAGE_NEXT));
}

/* ================================================================== */
/* bt_get.c							      */
/* ================================================================== */

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

int
ref___bt_get(const DB *dbp, const DBT *key, DBT *data, u_int flags)
{
	BTREE *t;
	EPG *e;
	int exact, status;

	t = dbp->internal;

	/* Toss any page pinned across calls. */
	if (t->bt_pinned != NULL) {
		mpool_put(t->bt_mp, t->bt_pinned, 0);
		t->bt_pinned = NULL;
	}

	/* Get currently doesn't take any flags. */
	if (flags) {
		errno = EINVAL;
		return (RET_ERROR);
	}

	if ((e = __bt_search(t, key, &exact)) == NULL)
		return (RET_ERROR);
	if (!exact) {
		mpool_put(t->bt_mp, e->page, 0);
		return (RET_SPECIAL);
	}

	status = __bt_ret(t, e, NULL, NULL, data, &t->bt_rdata, 0);

	/*
	 * If the user is doing concurrent access, we copied the
	 * key/data, toss the page.
	 */
	if (F_ISSET(t, B_DB_LOCK))
		mpool_put(t->bt_mp, e->page, 0);
	else
		t->bt_pinned = e->page;
	return (status);
}

/* ================================================================== */
/* bt_close.c							      */
/* ================================================================== */

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

static int ref_bt_meta(BTREE *);
int ref___bt_sync(const DB *dbp, u_int flags);

int
ref___bt_close(DB *dbp)
{
	BTREE *t;
	int fd;

	t = dbp->internal;

	/* Toss any page pinned across calls. */
	if (t->bt_pinned != NULL) {
		mpool_put(t->bt_mp, t->bt_pinned, 0);
		t->bt_pinned = NULL;
	}

	/* Sync the tree. */
	if (ref___bt_sync(dbp, 0) == RET_ERROR)
		return (RET_ERROR);

	/* Close the memory pool. */
	if (mpool_close(t->bt_mp) == RET_ERROR)
		return (RET_ERROR);

	/* Free random memory. */
	if (t->bt_cursor.key.data != NULL) {
		free(t->bt_cursor.key.data);
		t->bt_cursor.key.size = 0;
		t->bt_cursor.key.data = NULL;
	}
	if (t->bt_rkey.data) {
		free(t->bt_rkey.data);
		t->bt_rkey.size = 0;
		t->bt_rkey.data = NULL;
	}
	if (t->bt_rdata.data) {
		free(t->bt_rdata.data);
		t->bt_rdata.size = 0;
		t->bt_rdata.data = NULL;
	}

	fd = t->bt_fd;
	free(t);
	free(dbp);
	return (_close(fd) ? RET_ERROR : RET_SUCCESS);
}

int
ref___bt_sync(const DB *dbp, u_int flags)
{
	BTREE *t;
	int status;

	t = dbp->internal;

	/* Toss any page pinned across calls. */
	if (t->bt_pinned != NULL) {
		mpool_put(t->bt_mp, t->bt_pinned, 0);
		t->bt_pinned = NULL;
	}

	/* Sync doesn't currently take any flags. */
	if (flags != 0) {
		errno = EINVAL;
		return (RET_ERROR);
	}

	if (F_ISSET(t, B_INMEM | B_RDONLY) ||
	    !F_ISSET(t, B_MODIFIED | B_METADIRTY))
		return (RET_SUCCESS);

	if (F_ISSET(t, B_METADIRTY) && ref_bt_meta(t) == RET_ERROR)
		return (RET_ERROR);

	if ((status = mpool_sync(t->bt_mp)) == RET_SUCCESS)
		F_CLR(t, B_MODIFIED);

	return (status);
}

static int
ref_bt_meta(BTREE *t)
{
	BTMETA m;
	void *p;

	if ((p = mpool_get(t->bt_mp, P_META, 0)) == NULL)
		return (RET_ERROR);

	/* Fill in metadata. */
	m.magic = BTREEMAGIC;
	m.version = BTREEVERSION;
	m.psize = t->bt_psize;
	m.free = t->bt_free;
	m.nrecs = t->bt_nrecs;
	m.flags = F_ISSET(t, SAVEMETA);

	memmove(p, &m, sizeof(BTMETA));
	mpool_put(t->bt_mp, p, MPOOL_DIRTY);
	return (RET_SUCCESS);
}

/* ================================================================== */
/* bt_conv.c							      */
/* ================================================================== */

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

static void ref_mswap(PAGE *);

void
ref___bt_pgin(void *t, pgno_t pg, void *pp)
{
	PAGE *h;
	indx_t i, top;
	u_char flags;
	char *p;

	if (!F_ISSET(((BTREE *)t), B_NEEDSWAP))
		return;
	if (pg == P_META) {
		ref_mswap(pp);
		return;
	}

	h = pp;
	M_32_SWAP(h->pgno);
	M_32_SWAP(h->prevpg);
	M_32_SWAP(h->nextpg);
	M_32_SWAP(h->flags);
	M_16_SWAP(h->lower);
	M_16_SWAP(h->upper);

	top = NEXTINDEX(h);
	if ((h->flags & P_TYPE) == P_BINTERNAL)
		for (i = 0; i < top; i++) {
			M_16_SWAP(h->linp[i]);
			p = (char *)GETBINTERNAL(h, i);
			P_32_SWAP(p);
			p += sizeof(u_int32_t);
			P_32_SWAP(p);
			p += sizeof(pgno_t);
			if (*(u_char *)p & P_BIGKEY) {
				p += sizeof(u_char);
				P_32_SWAP(p);
				p += sizeof(pgno_t);
				P_32_SWAP(p);
			}
		}
	else if ((h->flags & P_TYPE) == P_BLEAF)
		for (i = 0; i < top; i++) {
			M_16_SWAP(h->linp[i]);
			p = (char *)GETBLEAF(h, i);
			P_32_SWAP(p);
			p += sizeof(u_int32_t);
			P_32_SWAP(p);
			p += sizeof(u_int32_t);
			flags = *(u_char *)p;
			if (flags & (P_BIGKEY | P_BIGDATA)) {
				p += sizeof(u_char);
				if (flags & P_BIGKEY) {
					P_32_SWAP(p);
					p += sizeof(pgno_t);
					P_32_SWAP(p);
				}
				if (flags & P_BIGDATA) {
					p += sizeof(u_int32_t);
					P_32_SWAP(p);
					p += sizeof(pgno_t);
					P_32_SWAP(p);
				}
			}
		}
}

void
ref___bt_pgout(void *t, pgno_t pg, void *pp)
{
	PAGE *h;
	indx_t i, top;
	u_char flags;
	char *p;

	if (!F_ISSET(((BTREE *)t), B_NEEDSWAP))
		return;
	if (pg == P_META) {
		ref_mswap(pp);
		return;
	}

	h = pp;
	top = NEXTINDEX(h);
	if ((h->flags & P_TYPE) == P_BINTERNAL)
		for (i = 0; i < top; i++) {
			p = (char *)GETBINTERNAL(h, i);
			P_32_SWAP(p);
			p += sizeof(u_int32_t);
			P_32_SWAP(p);
			p += sizeof(pgno_t);
			if (*(u_char *)p & P_BIGKEY) {
				p += sizeof(u_char);
				P_32_SWAP(p);
				p += sizeof(pgno_t);
				P_32_SWAP(p);
			}
			M_16_SWAP(h->linp[i]);
		}
	else if ((h->flags & P_TYPE) == P_BLEAF)
		for (i = 0; i < top; i++) {
			p = (char *)GETBLEAF(h, i);
			P_32_SWAP(p);
			p += sizeof(u_int32_t);
			P_32_SWAP(p);
			p += sizeof(u_int32_t);
			flags = *(u_char *)p;
			if (flags & (P_BIGKEY | P_BIGDATA)) {
				p += sizeof(u_char);
				if (flags & P_BIGKEY) {
					P_32_SWAP(p);
					p += sizeof(pgno_t);
					P_32_SWAP(p);
				}
				if (flags & P_BIGDATA) {
					p += sizeof(u_int32_t);
					P_32_SWAP(p);
					p += sizeof(pgno_t);
					P_32_SWAP(p);
				}
			}
			M_16_SWAP(h->linp[i]);
		}

	M_32_SWAP(h->pgno);
	M_32_SWAP(h->prevpg);
	M_32_SWAP(h->nextpg);
	M_32_SWAP(h->flags);
	M_16_SWAP(h->lower);
	M_16_SWAP(h->upper);
}

static void
ref_mswap(PAGE *pg)
{
	char *p;

	p = (char *)pg;
	P_32_SWAP(p);		/* magic */
	p += sizeof(u_int32_t);
	P_32_SWAP(p);		/* version */
	p += sizeof(u_int32_t);
	P_32_SWAP(p);		/* psize */
	p += sizeof(u_int32_t);
	P_32_SWAP(p);		/* free */
	p += sizeof(u_int32_t);
	P_32_SWAP(p);		/* nrecs */
	p += sizeof(u_int32_t);
	P_32_SWAP(p);		/* flags */
	p += sizeof(u_int32_t);
}
