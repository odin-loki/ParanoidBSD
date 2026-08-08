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

module;

#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

export module pbsd.lib.libc.db.btree.b0155s1;

/*
 * The batch supplies bt_put.c only; <db.h>, btree.h and mpool.h are not part
 * of it.  The declarations bt_put.c consumes are reproduced below with the
 * same layout, so the ported code compiles and can be driven against the
 * C oracle over a shared environment.
 */
export {

typedef struct MPOOL MPOOL;

typedef u_int32_t	pgno_t;
typedef u_int16_t	indx_t;
typedef u_int32_t	recno_t;

typedef enum __dbtype_e { DB_BTREE, DB_HASH, DB_RECNO } DBTYPE;

typedef struct __dbt_s {
	void	*data;			/* data */
	size_t	 size;			/* data length */
} DBT;

typedef struct __db {
	DBTYPE type;			/* underlying db type */
	int (*close)(struct __db *);
	int (*del)(const struct __db *, const DBT *, u_int);
	int (*get)(const struct __db *, const DBT *, DBT *, u_int);
	int (*put)(const struct __db *, DBT *, const DBT *, u_int);
	int (*seq)(const struct __db *, DBT *, DBT *, u_int);
	int (*sync)(const struct __db *, u_int);
	void *internal;			/* access method private */
	int (*fd)(const struct __db *);
} DB;

typedef struct _page {
	pgno_t	pgno;			/* this page's page number */
	pgno_t	prevpg;			/* left sibling */
	pgno_t	nextpg;			/* right sibling */

	u_int32_t flags;
	indx_t	lower;			/* lower bound of free space on page */
	indx_t	upper;			/* upper bound of free space on page */
	indx_t	linp[1];		/* indx_t-aligned VAR. LENGTH DATA */
} PAGE;

typedef struct _bleaf {
	u_int32_t	ksize;		/* size of key */
	u_int32_t	dsize;		/* size of data */
	u_char	flags;			/* P_BIGDATA, P_BIGKEY */
	char	bytes[1];		/* data */
} BLEAF;

typedef struct _epgno {
	pgno_t	pgno;			/* the page number */
	indx_t	index;			/* the index on the page */
} EPGNO;

typedef struct _epg {
	PAGE	*page;			/* the (pinned) page */
	indx_t	 index;			/* the index on the page */
} EPG;

typedef struct _cursor {
	EPGNO	 pg;			/* B: Saved tree reference. */
	DBT	 key;			/* B: Saved key, or key.data == NULL. */
	recno_t	 rcursor;		/* R: recno cursor (1-based) */

	u_int8_t flags;
} CURSOR;

enum bt_order_e { NOT, BACK, FORWARD };

typedef struct _btree {
	MPOOL	 *bt_mp;		/* memory pool cookie */

	DB	 *bt_dbp;		/* pointer to enclosing DB */

	EPG	  bt_cur;		/* current (pinned) page */
	PAGE	 *bt_pinned;		/* page pinned across calls */

	CURSOR	  bt_cursor;		/* cursor */

	EPGNO	  bt_stack[50];		/* stack of parent pages */
	EPGNO	 *bt_sp;		/* current stack pointer */

	DBT	  bt_rkey;		/* returned key */
	DBT	  bt_rdata;		/* returned data */

	int	  bt_fd;		/* tree file descriptor */

	pgno_t	  bt_free;		/* next free page */
	u_int32_t bt_psize;		/* page size */
	indx_t	  bt_ovflsize;		/* cut-off for key/data overflow */
	int	  bt_lorder;		/* byte order */
					/* sorted order */
	enum bt_order_e bt_order;
	EPGNO	  bt_last;		/* last insert */

					/* B: key comparison function */
	int	(*bt_cmp)(const DBT *, const DBT *);
					/* B: prefix comparison function */
	size_t	(*bt_pfx)(const DBT *, const DBT *);
					/* R: recno input function */
	int	(*bt_irec)(struct _btree *, recno_t);

	FILE	 *bt_rfp;		/* R: record FILE pointer */
	int	  bt_rfd;		/* R: record file descriptor */

	caddr_t	  bt_cmap;		/* R: current point in mapped space */
	caddr_t	  bt_smap;		/* R: start of mapped space */
	caddr_t   bt_emap;		/* R: end of mapped space */
	size_t	  bt_msize;		/* R: size of mapped region. */

	recno_t	  bt_nrecs;		/* R: number of records */
	size_t	  bt_reclen;		/* R: fixed record length */
	u_char	  bt_bval;		/* R: delimiting byte/pad character */

	u_int32_t flags;
} BTREE;

}

#define	RET_ERROR	-1
#define	RET_SUCCESS	 0
#define	RET_SPECIAL	 1

#define	R_CURSOR	 1
#define	__R_UNUSED	 2
#define	R_FIRST		 3
#define	R_IAFTER	 4
#define	R_IBEFORE	 5
#define	R_LAST		 6
#define	R_NEXT		 7
#define	R_NOOVERWRITE	 8
#define	R_PREV		 9
#define	R_SETCURSOR	10
#define	R_RECNOSYNC	11

#define	P_INVALID	 0xffffffff

#define	DEFMINKEYPAGE	 (2)
#define	MINCACHE	 (5)
#define	MINPSIZE	 (512)

#define	P_BINTERNAL	0x01		/* btree internal page */
#define	P_BLEAF		0x02		/* leaf page */
#define	P_OVERFLOW	0x04		/* overflow page */
#define	P_RINTERNAL	0x08		/* recno internal page */
#define	P_RLEAF		0x10		/* leaf page */
#define	P_TYPE		0x1f		/* type mask */
#define	P_PRESERVE	0x20		/* never delete this chain of pages */

#define	LALIGN(n)	(((n) + sizeof(pgno_t) - 1) & ~(sizeof(pgno_t) - 1))
#define	NOVFLSIZE	(sizeof(pgno_t) + sizeof(u_int32_t))

#define	BTDATAOFF							\
	(sizeof(pgno_t) + sizeof(pgno_t) + sizeof(pgno_t) +		\
	    sizeof(u_int32_t) + sizeof(indx_t) + sizeof(indx_t))
#define	NEXTINDEX(p)	(((p)->lower - BTDATAOFF) / sizeof(indx_t))

#define	P_BIGDATA	0x01		/* overflow data */
#define	P_BIGKEY	0x02		/* overflow key */

#define	NBLEAFDBT(ksize, dsize)						\
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

#define	CURS_ACQUIRE	0x01		/*  B: Cursor intialized. */
#define	CURS_AFTER	0x02		/*  B: Unreturned cursor after. */
#define	CURS_BEFORE	0x04		/*  B: Unreturned cursor before. */
#define	CURS_INIT	0x08		/*  B: Cursor initialized. */

#define	B_INMEM		0x00001		/* in-memory tree */
#define	B_METADIRTY	0x00002		/* need to write metadata */
#define	B_MODIFIED	0x00004		/* tree modified */
#define	B_NEEDSWAP	0x00008		/* if byte order requires swapping */
#define	B_RDONLY	0x00010		/* read-only tree */

#define	B_DB_LOCK	0x00020		/* DB_LOCK specified. */
#define	B_DB_SHMEM	0x00040		/* DB_SHMEM specified. */
#define	B_DB_TXN	0x00080		/* DB_TXN specified. */

#define	R_CLOSEFP	0x00100		/* opened a file pointer */
#define	R_EOF		0x00200		/* end of input file reached. */
#define	R_FIXLEN	0x00400		/* fixed length records */
#define	R_MEMMAPPED	0x00800		/* memory mapped file. */
#define	R_INMEM		0x01000		/* in-memory file */
#define	R_MODIFIED	0x02000		/* modified file */
#define	R_RECNO		0x04000		/* record oriented tree */

#define	B_NODUPS	0x08000		/* no duplicate keys permitted */

#define	F_ISSET(p, f)	((p)->flags & (f))
#define	F_SET(p, f)	((p)->flags |= (f))
#define	F_CLR(p, f)	((p)->flags &= ~(f))

#define	MPOOL_DIRTY	0x01		/* page needs to be written */

extern "C" {
void	*mpool_get(MPOOL *, pgno_t, u_int);
int	 mpool_put(MPOOL *, void *, u_int);
int	 __bt_cmp(BTREE *, const DBT *, EPG *);
int	 __bt_dleaf(BTREE *, const DBT *, PAGE *, u_int);
EPG	*__bt_search(BTREE *, const DBT *, int *);
int	 __bt_setcur(BTREE *, pgno_t, u_int);
int	 __bt_split(BTREE *, PAGE *, const DBT *, const DBT *, int, size_t,
	    u_int32_t);
int	 __ovfl_put(BTREE *, const DBT *, pgno_t *);
}
/*__BODY__*/
