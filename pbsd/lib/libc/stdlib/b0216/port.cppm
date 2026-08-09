
module;

#define _DEFAULT_SOURCE
#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif
#ifndef MAXSYMLINKS
#define MAXSYMLINKS 40
#endif
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#ifndef AT_FDCWD
#define AT_FDCWD (-100)
#endif
#ifndef __noinline
#define __noinline __attribute__((__noinline__))
#endif

#define locale_t __glibc_locale_t
#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <search.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#undef locale_t

export module pbsd.lib.libc.stdlib.b0216;

extern "C" int __sys___realpathat(int fd, const char *path, char *buf,
    size_t size, int flags);

export namespace pbsd::lib_libc_stdlib::b0216 {

struct xlocale {
	int __unused;
};

typedef struct xlocale *locale_t;

static struct xlocale default_locale;

locale_t
__get_locale(void)
{
	return (&default_locale);
}

#define FIX_LOCALE(loc)							\
	do {								\
		if ((loc) == NULL)						\
			(loc) = __get_locale();				\
	} while (0)

static int
b0216_isspace_l(int c, locale_t locale)
{
	unsigned char uc = (unsigned char)c;
	(void)locale;
	return (uc == ' ' || uc == '\f' || uc == '\n' || uc == '\r' ||
	    uc == '\t' || uc == '\v');
}

#ifdef isspace_l
#undef isspace_l
#endif
#define isspace_l(c, loc)	b0216_isspace_l((c), (loc))

typedef struct __posix_tnode {
	void			*key;
	struct __posix_tnode	*llink, *rlink;
	signed char		 balance;
} posix_tnode;


/* tsearch_path.h inline */

struct path {
	uintptr_t steps[2];
	unsigned int nsteps;
};

/* Initializes the path structure with a zero-length path. */
static inline void
path_init(struct path *p)
{

	p->nsteps = 0;
}

#define	STEPS_BIT (sizeof(uintptr_t) * CHAR_BIT)

/* Pushes a step to the left to the end of the path. */
static inline void
path_taking_left(struct path *p)
{

	p->steps[p->nsteps / STEPS_BIT] |=
	    (uintptr_t)1 << (p->nsteps % STEPS_BIT);
	++p->nsteps;
}

/* Pushes a step to the right to the end of the path. */
static inline void
path_taking_right(struct path *p)
{

	p->steps[p->nsteps / STEPS_BIT] &=
	    ~((uintptr_t)1 << (p->nsteps % STEPS_BIT));
	++p->nsteps;
}

/*
 * Pops the first step from the path and returns whether it was a step
 * to the left.
 */
static inline bool
path_took_left(struct path *p)
{
	bool result;

	result = p->steps[0] & 0x1;
	p->steps[0] = (p->steps[0] >> 1) | (p->steps[1] << (STEPS_BIT - 1));
	p->steps[1] >>= 1;
	return (result);
}




/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 *
 * Portions of this software were developed by David Chisnall
 * under sponsorship from the FreeBSD Foundation.
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
 * Convert a string to a long long integer.
 *
 * Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
long long
strtoll_l(const char * __restrict nptr, char ** __restrict endptr, int base,
		locale_t locale)
{
	const char *s;
	unsigned long long acc;
	char c;
	unsigned long long cutoff;
	int neg, any, cutlim;
	FIX_LOCALE(locale);

	/*
	 * Skip white space and pick up leading +/- sign if any.
	 * If base is 0, allow 0b for binary, 0x for hex, and 0 for
	 * octal, else assume decimal; if base is already 2, allow
	 * 0b; if base is already 16, allow 0x.
	 */
	s = nptr;
	do {
		c = *s++;
	} while (isspace_l((unsigned char)c, locale));
	if (c == '-') {
		neg = 1;
		c = *s++;
	} else {
		neg = 0;
		if (c == '+')
			c = *s++;
	}
	if ((base == 0 || base == 16) &&
	    c == '0' && (*s == 'x' || *s == 'X') &&
	    ((s[1] >= '0' && s[1] <= '9') ||
	    (s[1] >= 'A' && s[1] <= 'F') ||
	    (s[1] >= 'a' && s[1] <= 'f'))) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if ((base == 0 || base == 2) &&
	    c == '0' && (*s == 'b' || *s == 'B') &&
	    (s[1] >= '0' && s[1] <= '1')) {
		c = s[1];
		s += 2;
		base = 2;
	}
	if (base == 0)
		base = c == '0' ? 8 : 10;
	acc = any = 0;
	if (base < 2 || base > 36)
		goto noconv;

	/*
	 * Compute the cutoff value between legal numbers and illegal
	 * numbers.  That is the largest legal value, divided by the
	 * base.  An input number that is greater than this value, if
	 * followed by a legal input character, is too big.  One that
	 * is equal to this value may be valid or not; the limit
	 * between valid and invalid numbers is then based on the last
	 * digit.  For instance, if the range for quads is
	 * [-9223372036854775808..9223372036854775807] and the input base
	 * is 10, cutoff will be set to 922337203685477580 and cutlim to
	 * either 7 (neg==0) or 8 (neg==1), meaning that if we have
	 * accumulated a value > 922337203685477580, or equal but the
	 * next digit is > 7 (or 8), the number is too big, and we will
	 * return a range error.
	 *
	 * Set 'any' if any `digits' consumed; make it negative to indicate
	 * overflow.
	 */
	cutoff = neg ? (unsigned long long)-(LLONG_MIN + LLONG_MAX) + LLONG_MAX
	    : LLONG_MAX;
	cutlim = cutoff % base;
	cutoff /= base;
	for ( ; ; c = *s++) {
		if (c >= '0' && c <= '9')
			c -= '0';
		else if (c >= 'A' && c <= 'Z')
			c -= 'A' - 10;
		else if (c >= 'a' && c <= 'z')
			c -= 'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = neg ? LLONG_MIN : LLONG_MAX;
		errno = ERANGE;
	} else if (!any) {
noconv:
		errno = EINVAL;
	} else if (neg)
		acc = -acc;
	if (endptr != NULL)
		*endptr = (char *)(any ? s - 1 : nptr);
	return (acc);
}
long long
strtoll(const char * __restrict nptr, char ** __restrict endptr, int base)
{
	return strtoll_l(nptr, endptr, base, __get_locale());
}


/*-
 * Copyright (c) 2015 Nuxi, https://nuxi.nl/
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
 */

posix_tnode *
tsearch(const void *key, posix_tnode **rootp,
    int (*compar)(const void *, const void *))
{
	struct path path;
	posix_tnode **leaf, *result, *n, *x, *y, *z;
	int cmp;

	/* POSIX requires that tsearch() returns NULL if rootp is NULL. */
	if (rootp == NULL)
		return (NULL);

	/*
	 * Find the leaf where the new key needs to be inserted. Return
	 * if we've found an existing entry. Keep track of the path that
	 * is taken to get to the node, as we will need it to adjust the
	 * balances.
	*/
	path_init(&path);
	leaf = rootp;
	while (*leaf != NULL) {
		if ((*leaf)->balance != 0) {
			/*
			 * If we reach a node that has a non-zero
			 * balance on the way, we know that we won't
			 * need to perform any rotations above this
			 * point. In this case rotations are always
			 * capable of keeping the subtree in balance.
			 * Make this the root node and reset the path.
			 */
			rootp = leaf;
			path_init(&path);
		}
		cmp = compar(key, (*leaf)->key);
		if (cmp < 0) {
			path_taking_left(&path);
			leaf = &(*leaf)->llink;
		} else if (cmp > 0) {
			path_taking_right(&path);
			leaf = &(*leaf)->rlink;
		} else {
			return (*leaf);
		}
	}

	/* Did not find a matching key in the tree. Insert a new node. */
	result = *leaf = (posix_tnode *)malloc(sizeof(**leaf));
	if (result == NULL)
		return (NULL);
	result->key = (void *)key;
	result->llink = NULL;
	result->rlink = NULL;
	result->balance = 0;

	/*
	 * Walk along the same path a second time and adjust the
	 * balances. Except for the first node, all of these nodes must
	 * have a balance of zero, meaning that these nodes will not get
	 * out of balance.
	*/
	for (n = *rootp; n != *leaf;) {
		if (path_took_left(&path)) {
			n->balance += 1;
			n = n->llink;
		} else {
			n->balance -= 1;
			n = n->rlink;
		}
	}

	/*
	 * Adjusting the balances may have pushed the balance of the
	 * root node out of range. Perform a rotation to bring the
	 * balance back in range.
	 */
	x = *rootp;
	if (x->balance > 1) {
		y = x->llink;
		if (y->balance < 0) {
			/*
			 * Left-right case.
			 *
			 *         x
			 *        / \            z
			 *       y   D          / \
			 *      / \     -->    y   x
			 *     A   z          /|   |\
			 *        / \        A B   C D
			 *       B   C
			 */
			z = y->rlink;
			y->rlink = z->llink;
			z->llink = y;
			x->llink = z->rlink;
			z->rlink = x;
			*rootp = z;

			x->balance = z->balance > 0 ? -1 : 0;
			y->balance = z->balance < 0 ? 1 : 0;
			z->balance = 0;
		} else {
			/*
			 * Left-left case.
			 *
			 *        x           y
			 *       / \         / \
			 *      y   C  -->  A   x
			 *     / \             / \
			 *    A   B           B   C
			 */
			x->llink = y->rlink;
			y->rlink = x;
			*rootp = y;

			x->balance = 0;
			y->balance = 0;
		}
	} else if (x->balance < -1) {
		y = x->rlink;
		if (y->balance > 0) {
			/*
			 * Right-left case.
			 *
			 *       x
			 *      / \              z
			 *     A   y            / \
			 *        / \   -->    x   y
			 *       z   D        /|   |\
			 *      / \          A B   C D
			 *     B   C
			 */
			posix_tnode *z = y->llink;
			x->rlink = z->llink;
			z->llink = x;
			y->llink = z->rlink;
			z->rlink = y;
			*rootp = z;

			x->balance = z->balance < 0 ? 1 : 0;
			y->balance = z->balance > 0 ? -1 : 0;
			z->balance = 0;
		} else {
			/*
			 * Right-right case.
			 *
			 *       x               y
			 *      / \             / \
			 *     A   y    -->    x   C
			 *        / \         / \
			 *       B   C       A   B
			 */
			x->rlink = y->llink;
			y->llink = x;
			*rootp = y;

			x->balance = 0;
			y->balance = 0;
		}
	}

	/* Return the new entry. */
	return (result);
}


/*-
 * Copyright (c) 2015 Nuxi, https://nuxi.nl/
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
 */

/*
 * Makes a step to the left along the binary search tree. This step is
 * also saved, so it can be replayed while rebalancing.
*/
#define	GO_LEFT() do {							\
	if ((*leaf)->balance == 0 ||					\
	    ((*leaf)->balance < 0 && (*leaf)->rlink->balance == 0)) {	\
		/*							\
		 * If we reach a node that is balanced, or has a child	\
		 * in the opposite direction that is balanced, we know	\
		 * that we won't need to perform any rotations above	\
		 * this point. In this case rotations are always	\
		 * capable of keeping the subtree in balance. Make	\
		 * this the root node and reset the path.		\
		 */							\
		rootp = leaf;						\
		path_init(&path);					\
	}								\
	path_taking_left(&path);					\
	leaf = &(*leaf)->llink;						\
} while (0)

/* Makes a step to the right along the binary search tree. */
#define	GO_RIGHT() do {							\
	if ((*leaf)->balance == 0 ||					\
	    ((*leaf)->balance > 0 && (*leaf)->llink->balance == 0)) {	\
		rootp = leaf;						\
		path_init(&path);					\
	}								\
	path_taking_right(&path);					\
	leaf = &(*leaf)->rlink;						\
} while (0)

void *
tdelete(const void *key, posix_tnode **rootp,
    int (*compar)(const void *, const void *))
{
	struct path path;
	posix_tnode **leaf, *old, **n, *x, *y, *z, *result;
	int cmp;

	/* POSIX requires that tdelete() returns NULL if rootp is NULL. */
	if (rootp == NULL)
		return (NULL);

	/*
	 * Find the leaf that needs to be removed. Return if we cannot
	 * find an existing entry. Keep track of the path that is taken
	 * to get to the node, as we will need it to adjust the
	 * balances.
	 */
	result = (posix_tnode *)1;
	path_init(&path);
	leaf = rootp;
	for (;;) {
		if (*leaf == NULL)
			return (NULL);
		cmp = compar(key, (*leaf)->key);
		if (cmp < 0) {
			result = *leaf;
			GO_LEFT();
		} else if (cmp > 0) {
			result = *leaf;
			GO_RIGHT();
		} else {
			break;
		}
	}

	/* Found a matching key in the tree. Remove the node. */
	if ((*leaf)->llink == NULL) {
		/* Node has no left children. Replace by its right subtree. */
		old = *leaf;
		*leaf = old->rlink;
		free(old);
	} else {
		/*
		 * Node has left children. Replace this node's key by
		 * its predecessor's and remove that node instead.
		 */
		void **keyp = &(*leaf)->key;
		GO_LEFT();
		while ((*leaf)->rlink != NULL)
			GO_RIGHT();
		old = *leaf;
		*keyp = old->key;
		*leaf = old->llink;
		free(old);
	}

	/*
	 * Walk along the same path a second time and adjust the
	 * balances. Though this code looks similar to the rebalancing
	 * performed in tsearch(), it is not identical. We now also need
	 * to consider the case of outward imbalance in the right-right
	 * and left-left case that only exists when deleting. Hence the
	 * duplication of code.
	 */
	for (n = rootp; n != leaf;) {
		if (path_took_left(&path)) {
			x = *n;
			if (x->balance < 0) {
				y = x->rlink;
				if (y->balance > 0) {
					/* Right-left case. */
					z = y->llink;
					x->rlink = z->llink;
					z->llink = x;
					y->llink = z->rlink;
					z->rlink = y;
					*n = z;

					x->balance = z->balance < 0 ? 1 : 0;
					y->balance = z->balance > 0 ? -1 : 0;
					z->balance = 0;
				} else {
					/* Right-right case. */
					x->rlink = y->llink;
					y->llink = x;
					*n = y;

					if (y->balance < 0) {
						x->balance = 0;
						y->balance = 0;
					} else {
						x->balance = -1;
						y->balance = 1;
					}
				}
			} else {
				--x->balance;
			}
			n = &x->llink;
		} else {
			x = *n;
			if (x->balance > 0) {
				y = x->llink;
				if (y->balance < 0) {
					/* Left-right case. */
					z = y->rlink;
					y->rlink = z->llink;
					z->llink = y;
					x->llink = z->rlink;
					z->rlink = x;
					*n = z;

					x->balance = z->balance > 0 ? -1 : 0;
					y->balance = z->balance < 0 ? 1 : 0;
					z->balance = 0;
				} else {
					/* Left-left case. */
					x->llink = y->rlink;
					y->rlink = x;
					*n = y;

					if (y->balance > 0) {
						x->balance = 0;
						y->balance = 0;
					} else {
						x->balance = 1;
						y->balance = -1;
					}
				}
			} else {
				++x->balance;
			}
			n = &x->rlink;
		}
	}

	/* Return the parent of the old entry. */
	return (result);
}


/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2003 Constantin S. Svintsoff <kostik@iclub.nsu.ru>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the authors may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
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
 */



/*
 * Find the real name of path, by removing all ".", ".." and symlink
 * components.  Returns (resolved) on success, or (NULL) on failure,
 * in which case the path which caused trouble is left in (resolved).
 */
static char * __noinline
realpath1(const char *path, char *resolved)
{
	struct stat sb;
	char *p, *q;
	size_t left_len, prev_len, resolved_len, next_token_len;
	unsigned symlinks;
	ssize_t slen;
	char left[PATH_MAX], next_token[PATH_MAX], symlink[PATH_MAX];

	symlinks = 0;
	if (path[0] == '/') {
		resolved[0] = '/';
		resolved[1] = '\0';
		if (path[1] == '\0')
			return (resolved);
		resolved_len = 1;
		left_len = strlcpy(left, path + 1, sizeof(left));
	} else {
		if (getcwd(resolved, PATH_MAX) == NULL) {
			resolved[0] = '.';
			resolved[1] = '\0';
			return (NULL);
		}
		resolved_len = strlen(resolved);
		left_len = strlcpy(left, path, sizeof(left));
	}
	if (left_len >= sizeof(left) || resolved_len >= PATH_MAX) {
		errno = ENAMETOOLONG;
		return (NULL);
	}

	/*
	 * Iterate over path components in `left'.
	 */
	while (left_len != 0) {
		/*
		 * Extract the next path component and adjust `left'
		 * and its length.
		 */
		p = strchr(left, '/');

		next_token_len = p != NULL ? (size_t)(p - left) : left_len;
		memcpy(next_token, left, next_token_len);
		next_token[next_token_len] = '\0';

		if (p != NULL) {
			left_len -= next_token_len + 1;
			memmove(left, p + 1, left_len + 1);
		} else {
			left[0] = '\0';
			left_len = 0;
		}

		if (resolved[resolved_len - 1] != '/') {
			if (resolved_len + 1 >= PATH_MAX) {
				errno = ENAMETOOLONG;
				return (NULL);
			}
			resolved[resolved_len++] = '/';
			resolved[resolved_len] = '\0';
		}
		if (next_token[0] == '\0') {
			/* Handle consequential slashes. */
			continue;
		} else if (strcmp(next_token, ".") == 0) {
			continue;
		} else if (strcmp(next_token, "..") == 0) {
			/*
			 * Strip the last path component except when we have
			 * single "/"
			 */
			if (resolved_len > 1) {
				resolved[resolved_len - 1] = '\0';
				q = strrchr(resolved, '/') + 1;
				*q = '\0';
				resolved_len = q - resolved;
			}
			continue;
		}

		/*
		 * Append the next path component and lstat() it.
		 */
		prev_len = resolved_len;
		resolved_len += strlcpy(resolved + prev_len, next_token,
		    PATH_MAX - prev_len);
		if (resolved_len >= PATH_MAX) {
			errno = ENAMETOOLONG;
			return (NULL);
		}
		if (lstat(resolved, &sb) != 0) {
			/*
			 * EACCES means the parent directory is not
			 * readable, while ENOTDIR means the parent
			 * directory is not a directory.  Rewind the path
			 * to correctly indicate where the error lies.
			 */
			if (errno == EACCES || errno == ENOTDIR) {
				if (prev_len > 1)
					prev_len--;
				resolved[prev_len] = '\0';
			}
			return (NULL);
		}
		if (S_ISLNK(sb.st_mode)) {
			if (symlinks++ > MAXSYMLINKS) {
				errno = ELOOP;
				return (NULL);
			}
			slen = readlink(resolved, symlink, sizeof(symlink));
			if (slen < 0)
				return (NULL);
			if (slen == 0) {
				errno = ENOENT;
				return (NULL);
			}
			if ((size_t)slen >= sizeof(symlink)) {
				errno = ENAMETOOLONG;
				return (NULL);
			}
			symlink[slen] = '\0';
			if (symlink[0] == '/') {
				resolved[1] = 0;
				resolved_len = 1;
			} else {
				/* Strip the last path component. */
				q = strrchr(resolved, '/') + 1;
				*q = '\0';
				resolved_len = q - resolved;
			}

			/*
			 * If there are any path components left, then
			 * append them to symlink. The result is placed
			 * in `left'.
			 */
			if (p != NULL) {
				if (symlink[slen - 1] != '/') {
					if ((size_t)slen + 1 >= sizeof(symlink)) {
						errno = ENAMETOOLONG;
						return (NULL);
					}
					symlink[slen] = '/';
					symlink[slen + 1] = 0;
				}
				left_len = strlcat(symlink, left,
				    sizeof(symlink));
				if (left_len >= sizeof(symlink)) {
					errno = ENAMETOOLONG;
					return (NULL);
				}
			}
			left_len = strlcpy(left, symlink, sizeof(left));
		} else if (!S_ISDIR(sb.st_mode) && p != NULL) {
			errno = ENOTDIR;
			return (NULL);
		}
	}

	/*
	 * Remove trailing slash except when the resolved pathname
	 * is a single "/".
	 */
	if (resolved_len > 1 && resolved[resolved_len - 1] == '/')
		resolved[resolved_len - 1] = '\0';
	return (resolved);
}

char *
realpath(const char * __restrict path, char * __restrict resolved)
{
	char *m, *res;

	if (path == NULL) {
		errno = EINVAL;
		return (NULL);
	}
	if (path[0] == '\0') {
		errno = ENOENT;
		return (NULL);
	}
	if (resolved != NULL) {
		m = NULL;
	} else {
		m = resolved = (char *)malloc(PATH_MAX);
		if (resolved == NULL)
			return (NULL);
	}
	if (__sys___realpathat(AT_FDCWD, path, resolved, PATH_MAX, 0) == 0) {
		return (resolved);
	}
	res = realpath1(path, resolved);
	if (res == NULL)
		free(m);
	return (res);
}

} /* namespace pbsd::lib_libc_stdlib::b0216 */
