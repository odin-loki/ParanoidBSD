/***********************************************************
Copyright 1990, by Alfalfa Software Incorporated, Cambridge, Massachusetts.
Copyright 2010, Gabor Kovesdan <gabor@FreeBSD.org>

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that Alfalfa's name not be used in
advertising or publicity pertaining to distribution of the software
without specific, written prior permission.

ALPHALPHA DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
ALPHALPHA BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

If you make any modifications, bugfixes or other changes to this software
we'd appreciate it if you could send a copy to us so we can keep things
up-to-date.  Many thanks.
				Kee Hinckley
				Alfalfa Software, Inc.
				267 Allston St., #3
				Cambridge, MA 02139  USA
				nazgul@alfalfa.com

******************************************************************/

#define _NLS_PRIVATE

#include "namespace.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/queue.h>

#include <arpa/inet.h>		/* for ntohl() */
#include <machine/atomic.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <nl_types.h>
#include <paths.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "un-namespace.h"

#include "../locale/xlocale_private.h"
#include "libc_private.h"

#define _DEFAULT_NLS_PATH "/usr/share/nls/%L/%N.cat:/usr/share/nls/%N/%L:"	\
				_PATH_LOCALBASE "/share/nls/%L/%N.cat:"		\
				_PATH_LOCALBASE "/share/nls/%N/%L"

#define RLOCK(fail)	{ int ret;						\
			  if (__isthreaded &&					\
			      ((ret = _pthread_rwlock_rdlock(&rwlock)) != 0)) {	\
				  errno = ret;					\
				  return (fail);				\
			  }}
#define WLOCK(fail)	{ int ret;						\
			  if (__isthreaded &&					\
			      ((ret = _pthread_rwlock_wrlock(&rwlock)) != 0)) {	\
				  errno = ret;					\
				  return (fail);				\
			  }}
#define UNLOCK		{ if (__isthreaded)					\
			      _pthread_rwlock_unlock(&rwlock); }

#define	NLERR		((nl_catd) -1)
#define NLRETERR(errc)  { errno = errc; return (NLERR); }
/*
 * A write lock that reports failure instead of returning from the caller.
 *
 * WLOCK()'s early return is a `return (fail);' in the middle of whatever
 * the caller was about to hand to the cache, and the cache is where the
 * ownership of that thing lives - so on the failure path it is simply
 * lost.  The lock really can fail: `rwlock' is statically initialised,
 * libthr initialises it on first use in rwlock_init()
 * (lib/libthr/thread/thr_rwlock.c:97), and that is an aligned_alloc()
 * which returns ENOMEM.
 */
#define	TRY_WLOCK()	(!__isthreaded || _pthread_rwlock_wrlock(&rwlock) == 0)
#define SAVEFAIL(n, l, e)	{ np = calloc(1, sizeof(struct catentry));	\
				  if (np != NULL) {				\
				  	np->name = strdup(n);			\
					np->catd = NLERR;			\
					np->lang = (l == NULL) ? NULL :		\
					    strdup(l);				\
					np->caterrno = e;			\
					if (np->name == NULL ||			\
					    (l != NULL && np->lang == NULL) ||	\
					    !TRY_WLOCK()) {			\
						free(np->name);			\
						free(np->lang);			\
						free(np);			\
					} else {				\
						SLIST_INSERT_HEAD(&cache, np,	\
						    list);			\
						UNLOCK;				\
					}					\
				  }						\
				  errno = e;					\
				}

static nl_catd load_msgcat(const char *, const char *, const char *);

static pthread_rwlock_t		 rwlock = PTHREAD_RWLOCK_INITIALIZER;

struct catentry {
	SLIST_ENTRY(catentry)	 list;
	char			*name;
	char			*path;
	int			 caterrno;
	nl_catd			 catd;
	char			*lang;
	int			 refcount;
};

SLIST_HEAD(listhead, catentry) cache =
    SLIST_HEAD_INITIALIZER(cache);

nl_catd
catopen(const char *name, int type)
{
	return (__catopen_l(name, type, __get_locale()));
}

nl_catd
__catopen_l(const char *name, int type, locale_t locale)
{
	struct stat sbuf;
	struct catentry *np;
	char *base, *cptr, *cptr1, *nlspath, *pathP, *pcode;
	char *plang, *pter;
	int saverr, spcleft;
	const char *lang, *tmpptr;
	char path[PATH_MAX];

	/* sanity checking */
	if (name == NULL || *name == '\0')
		NLRETERR(ENOENT);

	if (strchr(name, '/') != NULL)
		/* have a pathname */
		lang = NULL;
	else {
		if (type == NL_CAT_LOCALE)
			lang = querylocale(LC_MESSAGES_MASK, locale);
		else
			lang = getenv("LANG");

		if (lang == NULL || *lang == '\0' || strlen(lang) > ENCODING_LEN ||
		    (lang[0] == '.' &&
		    (lang[1] == '\0' || (lang[1] == '.' && lang[2] == '\0'))) ||
		    strchr(lang, '/') != NULL)
			lang = "C";
	}

	/* Try to get it from the cache first */
	RLOCK(NLERR);
	SLIST_FOREACH(np, &cache, list) {
		if ((strcmp(np->name, name) == 0) &&
		    ((lang != NULL && np->lang != NULL &&
		    strcmp(np->lang, lang) == 0) || (np->lang == lang))) {
			if (np->caterrno != 0) {
				/* Found cached failing entry */
				UNLOCK;
				NLRETERR(np->caterrno);
			} else {
				/* Found cached successful entry */
				atomic_add_int(&np->refcount, 1);
				UNLOCK;
				return (np->catd);
			}
		}
	}
	UNLOCK;

	/* is it absolute path ? if yes, load immediately */
	if (strchr(name, '/') != NULL)
		return (load_msgcat(name, name, lang));

	/* sanity checking */
	if ((plang = cptr1 = strdup(lang)) == NULL)
		return (NLERR);
	if ((cptr = strchr(cptr1, '@')) != NULL)
		*cptr = '\0';
	pter = pcode = "";
	if ((cptr = strchr(cptr1, '_')) != NULL) {
		*cptr++ = '\0';
		pter = cptr1 = cptr;
	}
	if ((cptr = strchr(cptr1, '.')) != NULL) {
		*cptr++ = '\0';
		pcode = cptr;
	}

	if ((nlspath = secure_getenv("NLSPATH")) == NULL)
		nlspath = _DEFAULT_NLS_PATH;

	if ((base = cptr = strdup(nlspath)) == NULL) {
		saverr = errno;
		free(plang);
		errno = saverr;
		return (NLERR);
	}

	while ((nlspath = strsep(&cptr, ":")) != NULL) {
		pathP = path;
		if (*nlspath) {
			for (; *nlspath; ++nlspath) {
				if (*nlspath == '%') {
					switch (*(nlspath + 1)) {
					case 'l':
						tmpptr = plang;
						break;
					case 't':
						tmpptr = pter;
						break;
					case 'c':
						tmpptr = pcode;
						break;
					case 'L':
						tmpptr = lang;
						break;
					case 'N':
						tmpptr = (char *)name;
						break;
					case '%':
						++nlspath;
						/* FALLTHROUGH */
					default:
						if (pathP - path >=
						    sizeof(path) - 1)
							goto too_long;
						*(pathP++) = *nlspath;
						continue;
					}
					++nlspath;
			put_tmpptr:
					spcleft = sizeof(path) -
						  (pathP - path) - 1;
					if (strlcpy(pathP, tmpptr, spcleft) >=
					    spcleft) {
			too_long:
						free(plang);
						free(base);
						SAVEFAIL(name, lang, ENAMETOOLONG);
						NLRETERR(ENAMETOOLONG);
					}
					pathP += strlen(tmpptr);
				} else {
					if (pathP - path >= sizeof(path) - 1)
						goto too_long;
					*(pathP++) = *nlspath;
				}
			}
			*pathP = '\0';
			if (stat(path, &sbuf) == 0) {
				free(plang);
				free(base);
				return (load_msgcat(path, name, lang));
			}
		} else {
			tmpptr = (char *)name;
			--nlspath;
			goto put_tmpptr;
		}
	}
	free(plang);
	free(base);
	SAVEFAIL(name, lang, ENOENT);
	NLRETERR(ENOENT);
}

char *
catgets(nl_catd catd, int set_id, int msg_id, const char *s)
{
	struct _nls_cat_hdr *cat_hdr;
	struct _nls_msg_hdr *msg_hdr;
	struct _nls_set_hdr *set_hdr;
	int32_t no;
	int i, l, u;

	if (catd == NULL || catd == NLERR) {
		errno = EBADF;
		/* LINTED interface problem */
		return ((char *)s);
	}

	cat_hdr = (struct _nls_cat_hdr *)catd->__data;
	set_hdr = (struct _nls_set_hdr *)(void *)((char *)catd->__data +
	    sizeof(struct _nls_cat_hdr));

	/* binary search, see knuth algorithm b */
	l = 0;
	u = ntohl((u_int32_t)cat_hdr->__nsets) - 1;
	while (l <= u) {
		/*
		 * PBSD: `(l + u) / 2' overflows once u is large, and u is
		 * one less than a count out of the file.  valid_msgcat()
		 * now bounds that count by the mapping, but the midpoint
		 * is written the way that does not need it to.
		 */
		i = l + (u - l) / 2;
		/*
		 * and `set_id - __setno' is a subtraction of two int32_t
		 * -- one the caller's, one the file's -- where only the
		 * sign is ever read.  There is nothing to subtract for.
		 */
		no = (int32_t)ntohl((u_int32_t)set_hdr[i].__setno);

		if (set_id == no) {
			msg_hdr = (struct _nls_msg_hdr *)
			    (void *)((char *)catd->__data +
			    sizeof(struct _nls_cat_hdr) +
			    ntohl((u_int32_t)cat_hdr->__msg_hdr_offset));

			l = ntohl((u_int32_t)set_hdr[i].__index);
			u = l + ntohl((u_int32_t)set_hdr[i].__nmsgs) - 1;
			while (l <= u) {
				/* and the same for the message search */
				i = l + (u - l) / 2;
				no = (int32_t)ntohl(
				    (u_int32_t)msg_hdr[i].__msgno);
				if (msg_id == no) {
					return ((char *) catd->__data +
					    sizeof(struct _nls_cat_hdr) +
					    ntohl((u_int32_t)
					    cat_hdr->__msg_txt_offset) +
					    ntohl((u_int32_t)
					    msg_hdr[i].__offset));
				} else if (msg_id < no) {
					u = i - 1;
				} else {
					l = i + 1;
				}
			}

			/* not found */
			goto notfound;

		} else if (set_id < no) {
			u = i - 1;
		} else {
			l = i + 1;
		}
	}

notfound:
	/* not found */
	errno = ENOMSG;
	/* LINTED interface problem */
	return ((char *)s);
}

static void
catfree(struct catentry *np)
{

	if (np->catd != NULL && np->catd != NLERR) {
		munmap(np->catd->__data, (size_t)np->catd->__size);
		free(np->catd);
	}
	SLIST_REMOVE(&cache, np, catentry, list);
	free(np->name);
	free(np->path);
	free(np->lang);
	free(np);
}

int
catclose(nl_catd catd)
{
	struct catentry *np;

	/* sanity checking */
	if (catd == NULL || catd == NLERR) {
		errno = EBADF;
		return (-1);
	}

	/* Remove from cache if not referenced any more */
	WLOCK(-1);
	SLIST_FOREACH(np, &cache, list) {
		if (catd == np->catd) {
			if (atomic_fetchadd_int(&np->refcount, -1) == 1)
				catfree(np);
			break;
		}
	}
	UNLOCK;
	return (0);
}

/*
 * Internal support functions
 */

/*
 * PBSD: nothing in this file validated anything.  load_msgcat() checked
 * the size against sizeof(u_int32_t) and the magic number, and then
 * catgets() binary-searched with counts and offsets read straight out
 * of the mapping -- `u = ntohl(cat_hdr->__nsets) - 1', `l =
 * ntohl(set_hdr[i].__index)', `u = l + ntohl(set_hdr[i].__nmsgs) - 1'
 * -- and returned
 *
 *	catd->__data + sizeof(struct _nls_cat_hdr) +
 *	    ntohl(cat_hdr->__msg_txt_offset) + ntohl(msg_hdr[i].__offset)
 *
 * which its caller reads as a NUL-terminated string.  Every one of
 * those numbers is a file-chosen int32_t.  catopen(3) finds the file
 * through NLSPATH, so for a program that is not setuid the catalogue
 * belongs to whoever runs it.
 *
 * The tables are small and this runs once per open, so the whole file
 * is checked here and catgets() is allowed to trust it.  The property
 * the caller actually needs from a message is not that __msglen agrees
 * with anything -- it is that the returned pointer is inside the
 * mapping and that a NUL follows it before the end, so that is what is
 * checked, with no dependence on gencat(1)'s convention.
 */
static int
valid_msgcat(const void *data, size_t size)
{
	const struct _nls_cat_hdr *cat;
	const struct _nls_set_hdr *sets;
	const struct _nls_msg_hdr *msgs;
	const char *base, *text;
	size_t after_hdr, maxmsgs, textspace;
	int32_t nsets, mhoff, mtoff;
	int32_t i, j, idx, nmsgs, off;

	if (size < sizeof(*cat))
		return (0);
	base = data;
	cat = data;
	after_hdr = size - sizeof(*cat);

	nsets = (int32_t)ntohl((u_int32_t)cat->__nsets);
	mhoff = (int32_t)ntohl((u_int32_t)cat->__msg_hdr_offset);
	mtoff = (int32_t)ntohl((u_int32_t)cat->__msg_txt_offset);
	if (nsets < 0 || mhoff < 0 || mtoff < 0)
		return (0);
	if ((size_t)mhoff > after_hdr || (size_t)mtoff > after_hdr)
		return (0);

	/* The set table follows the header. */
	if ((size_t)nsets > after_hdr / sizeof(*sets))
		return (0);
	sets = (const void *)(base + sizeof(*cat));

	/* The message-header table starts __msg_hdr_offset past it. */
	msgs = (const void *)(base + sizeof(*cat) + mhoff);
	maxmsgs = (after_hdr - (size_t)mhoff) / sizeof(*msgs);

	text = base + sizeof(*cat) + mtoff;
	textspace = after_hdr - (size_t)mtoff;

	for (i = 0; i < nsets; i++) {
		idx = (int32_t)ntohl((u_int32_t)sets[i].__index);
		nmsgs = (int32_t)ntohl((u_int32_t)sets[i].__nmsgs);
		if (idx < 0 || nmsgs < 0)
			return (0);
		if ((size_t)idx > maxmsgs ||
		    (size_t)nmsgs > maxmsgs - (size_t)idx)
			return (0);
		for (j = idx; j < idx + nmsgs; j++) {
			off = (int32_t)ntohl((u_int32_t)msgs[j].__offset);
			if (off < 0 || (size_t)off >= textspace)
				return (0);
			if (memchr(text + off, '\0',
			    textspace - (size_t)off) == NULL)
				return (0);
		}
	}
	return (1);
}

static nl_catd
load_msgcat(const char *path, const char *name, const char *lang)
{
	struct stat st;
	nl_catd	catd;
	struct catentry *np;
	void *data;
	char *copy_path, *copy_name, *copy_lang;
	int fd, saved_errno;

	/* path/name will never be NULL here */

	/*
	 * One more try in cache; if it was not found by name,
	 * it might still be found by absolute path.
	 */
	RLOCK(NLERR);
	SLIST_FOREACH(np, &cache, list) {
		if ((np->path != NULL) && (strcmp(np->path, path) == 0)) {
			atomic_add_int(&np->refcount, 1);
			UNLOCK;
			return (np->catd);
		}
	}
	UNLOCK;

	if ((fd = _open(path, O_RDONLY | O_CLOEXEC)) == -1) {
		SAVEFAIL(name, lang, errno);
		NLRETERR(errno);
	}

	if (_fstat(fd, &st) != 0) {
		saved_errno = errno;
		_close(fd);
		SAVEFAIL(name, lang, saved_errno);
		NLRETERR(saved_errno);
	}

	/* The file is too small to contain a _NLS_MAGIC. */
	if (st.st_size < sizeof(u_int32_t)) {
		_close(fd);
		SAVEFAIL(name, lang, ENOENT);
		NLRETERR(ENOENT);
	}

	/*
	 * If the file size cannot be held in size_t we cannot mmap()
	 * it to the memory.  Probably, this will not be a problem given
	 * that catalog files are usually small.
	 */
	if (st.st_size > SIZE_T_MAX || st.st_size > INT_MAX) {
		_close(fd);
		SAVEFAIL(name, lang, ENOENT);
		NLRETERR(ENOENT);
	}

	data = mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		saved_errno = errno;
		_close(fd);
		SAVEFAIL(name, lang, saved_errno);
		NLRETERR(saved_errno);
	}
	_close(fd);

	if (ntohl((u_int32_t)((struct _nls_cat_hdr *)data)->__magic) !=
	    _NLS_MAGIC) {
		munmap(data, (size_t)st.st_size);
		SAVEFAIL(name, lang, ENOENT);
		NLRETERR(ENOENT);
	}

	/* PBSD: and the rest of the header, which nothing else checks. */
	if (!valid_msgcat(data, (size_t)st.st_size)) {
		munmap(data, (size_t)st.st_size);
		SAVEFAIL(name, lang, EINVAL);
		NLRETERR(EINVAL);
	}

	copy_name = strdup(name);
	copy_path = strdup(path);
	copy_lang = (lang == NULL) ? NULL : strdup(lang);
	catd = malloc(sizeof (*catd));
	np = calloc(1, sizeof(struct catentry));

	if (copy_name == NULL || copy_path == NULL ||
	    (lang != NULL && copy_lang == NULL) ||
	    catd == NULL || np == NULL) {
		free(copy_name);
		free(copy_path);
		free(copy_lang);
		free(catd);
		free(np);
		munmap(data, (size_t)st.st_size);
		SAVEFAIL(name, lang, ENOMEM);
		NLRETERR(ENOMEM);
	}

	catd->__data = data;
	catd->__size = (int)st.st_size;

	/* Caching opened catalog */
	np->name = copy_name;
	np->path = copy_path;
	np->catd = catd;
	np->lang = copy_lang;
	atomic_store_int(&np->refcount, 1);
	if (!TRY_WLOCK()) {
		/*
		 * The cache entry is the ownership record: catclose() frees
		 * the catalogue by finding it there, so returning catd
		 * uncached would lose the mapping instead of the entry.
		 * Undo the load, the same way the ENOMEM arm above does.
		 */
		free(copy_name);
		free(copy_path);
		free(copy_lang);
		free(catd);
		free(np);
		munmap(data, (size_t)st.st_size);
		NLRETERR(ENOMEM);
	}
	SLIST_INSERT_HEAD(&cache, np, list);
	UNLOCK;
	return (catd);
}
