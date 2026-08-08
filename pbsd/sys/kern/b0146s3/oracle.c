/*
 * PBSD batch b0146s3 -- reference oracle.
 *
 * Source: hbsd/src/sys/kern/subr_hash.c
 *
 * Every function is renamed with a "ref_" prefix.  Function bodies are
 * otherwise UNMODIFIED.  The kernel environment (types, constants, and
 * malloc(9)) is modelled below and shared identically with the C++23 port
 * under test.
 */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef LONG_BIT
#define	LONG_BIT	(sizeof(long) * CHAR_BIT)
#endif

#define	__unused		__attribute__((__unused__))
#define	__inline		inline
#define	nitems(x)	(sizeof((x)) / sizeof((x)[0]))

typedef unsigned long	u_long;

#define	KASSERT(exp, msg)	((void)0)

#define	LIST_HEAD(name, type)						\
struct name {								\
	struct type *lh_first;						\
}
#define	LIST_FIRST(head)	((head)->lh_first)
#define	LIST_EMPTY(head)	(LIST_FIRST((head)) == NULL)
#define	LIST_INIT(head) do {						\
	LIST_FIRST((head)) = NULL;					\
} while (0)

struct malloc_type {
	const char	*ks_shortdesc;
};

#define	M_NOWAIT	0x0001
#define	M_WAITOK	0x0002

#define	HASH_NOWAIT	0x00000001
#define	HASH_WAITOK	0x00000002

static int	oracle_malloc_calls;
static int	oracle_malloc_fail_at;
static size_t	oracle_malloc_last_size;
static int	oracle_malloc_last_flags;

void
oracle_malloc_reset(void)
{

	oracle_malloc_calls = 0;
	oracle_malloc_fail_at = 0;
	oracle_malloc_last_size = 0;
	oracle_malloc_last_flags = 0;
}

void
oracle_malloc_fail_at(int n)
{

	oracle_malloc_fail_at = n;
}

int
oracle_malloc_calls_count(void)
{

	return (oracle_malloc_calls);
}

size_t
oracle_malloc_last_size(void)
{

	return (oracle_malloc_last_size);
}

int
oracle_malloc_last_flags(void)
{

	return (oracle_malloc_last_flags);
}

static void *
oracle_kmalloc(u_long size, struct malloc_type *type, int flags)
{
	void *p;

	(void)type;

	oracle_malloc_calls++;
	oracle_malloc_last_size = size;
	oracle_malloc_last_flags = flags;
	if (oracle_malloc_fail_at != 0 &&
	    oracle_malloc_calls >= oracle_malloc_fail_at)
		return (NULL);
	p = malloc(size);
	return (p);
}

static void
oracle_kfree(void *addr, struct malloc_type *type)
{
	(void)type;
	free(addr);
}

#define	malloc	oracle_kmalloc
#define	free	oracle_kfree

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
ref_hash_mflags(int flags)
{

	return ((flags & HASH_NOWAIT) ? M_NOWAIT : M_WAITOK);
}

/*
 * General routine to allocate a hash table with control of memory flags.
 */
void *
ref_hashinit_flags(int elements, struct malloc_type *type, u_long *hashmask,
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
	    ref_hash_mflags(flags));
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
ref_hashinit(int elements, struct malloc_type *type, u_long *hashmask)
{

	return (ref_hashinit_flags(elements, type, hashmask, HASH_WAITOK));
}

void
ref_hashdestroy(void *vhashtbl, struct malloc_type *type, u_long hashmask)
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
ref_phashinit_flags(int elements, struct malloc_type *type, u_long *nentries, int flags)
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
	    ref_hash_mflags(flags));
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
ref_phashinit(int elements, struct malloc_type *type, u_long *nentries)
{

	return (ref_phashinit_flags(elements, type, nentries, HASH_WAITOK));
}
