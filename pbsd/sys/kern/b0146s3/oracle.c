/*
 * PBSD batch b0146s3 -- reference oracle (the specification).
 *
 * Sources concatenated below, verbatim:
 *	hbsd/src/sys/kern/subr_hash.c
 *
 * Every function of the batch is given a ref_ prefix by the #defines in the
 * prologue, so that not one character of any function body has to change.
 * The prologue also supplies the pieces of the kernel environment that
 * <sys/param.h>, <sys/systm.h> and <sys/malloc.h> would otherwise provide:
 * u_long, struct malloc_type, the HASH_/M_ flag values, nitems(), the LIST
 * queue macros, KASSERT and malloc()/free().  KASSERT is live here, exactly
 * as in an INVARIANTS kernel; ref_env_panic() is supplied by the harness and
 * does not return.
 */

#include <stddef.h>

typedef unsigned long u_long;

struct malloc_type {
	const char *ks_shortdesc;
};

#define	M_NOWAIT	0x0001
#define	M_WAITOK	0x0002

#define	HASH_NOWAIT	0x00000001
#define	HASH_WAITOK	0x00000002

#define	__inline	inline

#define	nitems(x)	(sizeof((x)) / sizeof((x)[0]))

#define	LIST_HEAD(name, type)						\
struct name {								\
	struct type *lh_first;	/* first element */			\
}

#define	LIST_INIT(head) do {						\
	LIST_FIRST((head)) = NULL;					\
} while (0)

#define	LIST_FIRST(head)	((head)->lh_first)
#define	LIST_EMPTY(head)	(LIST_FIRST((head)) == NULL)

_Noreturn void ref_env_panic(const char *fmt, ...);

#define	KASSERT(exp, msg) do {						\
	if (!(exp))							\
		ref_env_panic msg;					\
} while (0)

void *ref_env_malloc(unsigned long size, struct malloc_type *type, int flags);
void ref_env_free(void *addr, struct malloc_type *type);

#define	malloc(size, type, flags)	ref_env_malloc((size), (type), (flags))
#define	free(addr, type)		ref_env_free((addr), (type))

#define	hash_mflags		ref_hash_mflags
#define	hashinit_flags		ref_hashinit_flags
#define	hashinit		ref_hashinit
#define	hashdestroy		ref_hashdestroy
#define	phashinit_flags		ref_phashinit_flags
#define	phashinit		ref_phashinit

/* ------------------------------------------------------------------------
 * hbsd/src/sys/kern/subr_hash.c
 * ------------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1982, 1986, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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

static __inline int
hash_mflags(int flags)
{

	return ((flags & HASH_NOWAIT) ? M_NOWAIT : M_WAITOK);
}

/*
 * General routine to allocate a hash table with control of memory flags.
 */
void *
hashinit_flags(int elements, struct malloc_type *type, u_long *hashmask,
    int flags)
{
	long hashsize, i;
	LIST_HEAD(generic, generic) *hashtbl;

	KASSERT(elements > 0, ("%s: bad elements", __func__));
	/* Exactly one of HASH_WAITOK and HASH_NOWAIT must be set. */
	KASSERT((flags & HASH_WAITOK) ^ (flags & HASH_NOWAIT),
	    ("Bad flags (0x%x) passed to hashinit_flags", flags));

	for (hashsize = 1; hashsize <= elements; hashsize <<= 1)
		continue;
	hashsize >>= 1;

	hashtbl = malloc((u_long)hashsize * sizeof(*hashtbl), type,
	    hash_mflags(flags));
	if (hashtbl != NULL) {
		for (i = 0; i < hashsize; i++)
			LIST_INIT(&hashtbl[i]);
		*hashmask = hashsize - 1;
	}
	return (hashtbl);
}

/*
 * Allocate and initialize a hash table with default flag: may sleep.
 */
void *
hashinit(int elements, struct malloc_type *type, u_long *hashmask)
{

	return (hashinit_flags(elements, type, hashmask, HASH_WAITOK));
}

void
hashdestroy(void *vhashtbl, struct malloc_type *type, u_long hashmask)
{
	LIST_HEAD(generic, generic) *hashtbl, *hp;

	hashtbl = vhashtbl;
	for (hp = hashtbl; hp <= &hashtbl[hashmask]; hp++)
		KASSERT(LIST_EMPTY(hp), ("%s: hashtbl %p not empty "
		    "(malloc type %s)", __func__, hashtbl, type->ks_shortdesc));
	free(hashtbl, type);
}

static const int primes[] = { 1, 13, 31, 61, 127, 251, 509, 761, 1021, 1531,
			2039, 2557, 3067, 3583, 4093, 4603, 5119, 5623, 6143,
			6653, 7159, 7673, 8191, 12281, 16381, 24571, 32749 };
#define	NPRIMES nitems(primes)

/*
 * General routine to allocate a prime number sized hash table with control of
 * memory flags.
 */
void *
phashinit_flags(int elements, struct malloc_type *type, u_long *nentries, int flags)
{
	long hashsize, i;
	LIST_HEAD(generic, generic) *hashtbl;

	KASSERT(elements > 0, ("%s: bad elements", __func__));
	/* Exactly one of HASH_WAITOK and HASH_NOWAIT must be set. */
	KASSERT((flags & HASH_WAITOK) ^ (flags & HASH_NOWAIT),
	    ("Bad flags (0x%x) passed to phashinit_flags", flags));

	for (i = 1, hashsize = primes[1]; hashsize <= elements;) {
		i++;
		if (i == NPRIMES)
			break;
		hashsize = primes[i];
	}
	hashsize = primes[i - 1];

	hashtbl = malloc((u_long)hashsize * sizeof(*hashtbl), type,
	    hash_mflags(flags));
	if (hashtbl == NULL)
		return (NULL);

	for (i = 0; i < hashsize; i++)
		LIST_INIT(&hashtbl[i]);
	*nentries = hashsize;
	return (hashtbl);
}

/*
 * Allocate and initialize a prime number sized hash table with default flag:
 * may sleep.
 */
void *
phashinit(int elements, struct malloc_type *type, u_long *nentries)
{

	return (phashinit_flags(elements, type, nentries, HASH_WAITOK));
}

/* ------------------------------------------------------------------------
 * Test hook.  hash_mflags() has internal linkage in the original file, so
 * the harness cannot reach it directly.  This wrapper only forwards; the
 * helper's own body above is untouched.
 * ------------------------------------------------------------------------ */

int
ref_hash_mflags_probe(int flags)
{

	return (ref_hash_mflags(flags));
}
