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

export module pbsd.lib.libc.db.btree.b0155s4;

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

/*
 * btree.h spells this as an unnamed enum inside struct _btree; hoisted here
 * so that NOT/BACK/FORWARD name the same things in C and in C++, where an
 * enum nested in a class would scope its enumerators to that class.
 */
enum _bt_order { NOT, BACK, FORWARD };

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

extern "C" {
void *mpool_get(MPOOL *, pgno_t, unsigned int);
int mpool_put(MPOOL *, void *, unsigned int);
void *mpool_new(MPOOL *, pgno_t *, unsigned int);
int mpool_delete(MPOOL *, void *);
MPOOL *mpool_open(void *, int, u_int32_t, pgno_t);
void mpool_filter(MPOOL *, void (*)(void *, pgno_t, void *), void (*)(void *, pgno_t, void *), void *);
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

export namespace pbsd::lib_libc_db_btree::b0155s4 {

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

int	__bt_delete(const DB *, const DBT *, u_int);
int	__bt_stkacq(BTREE *, PAGE **, CURSOR *);
int	__bt_bdelete(BTREE *, const DBT *);
int	__bt_pdelete(BTREE *, PAGE *);
int	__bt_dleaf(BTREE *, const DBT *, PAGE *, u_int);
int	__bt_curdel(BTREE *, const DBT *, PAGE *, u_int);
int	__bt_relink(BTREE *, PAGE *);


/*
 * __bt_delete
 *	Delete the item(s) referenced by a key.
 *
 * Return RET_SPECIAL if the key is not found.
 */
int
__bt_delete(const DB *dbp, const DBT *key, u_int flags)
{
	BTREE *t;
	CURSOR *c;
	PAGE *h;
	int status;

	t = (BTREE *)dbp->internal;

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
		status = __bt_bdelete(t, key);
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
			if ((h = (PAGE *)mpool_get(t->bt_mp, c->pg.pgno, 0)) == NULL)
				return (RET_ERROR);

			/*
			 * If the page is about to be emptied, we'll need to
			 * delete it, which means we have to acquire a stack.
			 */
			if (NEXTINDEX(h) == 1)
				if (__bt_stkacq(t, &h, &t->bt_cursor))
					return (RET_ERROR);

			status = __bt_dleaf(t, NULL, h, c->pg.index);

			if (NEXTINDEX(h) == 0 && status == RET_SUCCESS) {
				if (__bt_pdelete(t, h))
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
__bt_stkacq(BTREE *t, PAGE **hp, CURSOR *c)
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
			if ((h = (PAGE *)mpool_get(t->bt_mp, parent->pgno, 0)) == NULL)
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
			if ((h = (PAGE *)mpool_get(t->bt_mp, pgno, 0)) == NULL)
				return (1);
			idx = 0;
		}
		mpool_put(t->bt_mp, h, 0);
		if ((h = (PAGE *)mpool_get(t->bt_mp, nextpg, 0)) == NULL)
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
			if ((h = (PAGE *)mpool_get(t->bt_mp, parent->pgno, 0)) == NULL)
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
			if ((h = (PAGE *)mpool_get(t->bt_mp, pgno, 0)) == NULL)
				return (1);

			idx = NEXTINDEX(h) - 1;
			BT_PUSH(t, pgno, idx);
		}
		mpool_put(t->bt_mp, h, 0);
		if ((h = (PAGE *)mpool_get(t->bt_mp, prevpg, 0)) == NULL)
			return (1);
	}


ret:	mpool_put(t->bt_mp, h, 0);
	return ((*hp = (PAGE *)mpool_get(t->bt_mp, c->pg.pgno, 0)) == NULL);
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
__bt_bdelete(BTREE *t, const DBT *key)
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
		if (__bt_dleaf(t, key, h, e->index)) {
			mpool_put(t->bt_mp, h, 0);
			return (RET_ERROR);
		}
		if (F_ISSET(t, B_NODUPS)) {
			if (NEXTINDEX(h) == 0) {
				if (__bt_pdelete(t, h))
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
		if (__bt_dleaf(t, key, h, e->index) == RET_ERROR) {
			mpool_put(t->bt_mp, h, 0);
			return (RET_ERROR);
		}
		if (e->index == 0)
			redo = 1;
	}

	/* Check for an empty page. */
	if (NEXTINDEX(h) == 0) {
		if (__bt_pdelete(t, h))
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
__bt_pdelete(BTREE *t, PAGE *h)
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
		if ((pg = (PAGE *)mpool_get(t->bt_mp, parent->pgno, 0)) == NULL)
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
				if (__bt_relink(t, pg) || __bt_free(t, pg))
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
	return (__bt_relink(t, h) || __bt_free(t, h));
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
__bt_dleaf(BTREE *t, const DBT *key, PAGE *h, u_int idx)
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
	    __bt_curdel(t, key, h, idx))
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
__bt_curdel(BTREE *t, const DBT *key, PAGE *h, u_int idx)
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
			if ((pg = (PAGE *)mpool_get(t->bt_mp, h->prevpg, 0)) == NULL)
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
			if ((pg = (PAGE *)mpool_get(t->bt_mp, h->nextpg, 0)) == NULL)
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
__bt_relink(BTREE *t, PAGE *h)
{
	PAGE *pg;

	if (h->nextpg != P_INVALID) {
		if ((pg = (PAGE *)mpool_get(t->bt_mp, h->nextpg, 0)) == NULL)
			return (RET_ERROR);
		pg->prevpg = h->prevpg;
		mpool_put(t->bt_mp, pg, MPOOL_DIRTY);
	}
	if (h->prevpg != P_INVALID) {
		if ((pg = (PAGE *)mpool_get(t->bt_mp, h->prevpg, 0)) == NULL)
			return (RET_ERROR);
		pg->nextpg = h->nextpg;
		mpool_put(t->bt_mp, pg, MPOOL_DIRTY);
	}
	return (0);
}



} // namespace pbsd::lib_libc_db_btree::b0155s4
