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

/*
 * PBSD batch b0146s3: C++23 port of hbsd/src/sys/kern/subr_hash.c.
 *
 * The KASSERT()s of an INVARIANTS kernel are spelled out here as
 * "if (!(exp)) panic(msg)", which is what the macro expands to.  The
 * LIST_HEAD(generic, generic) declared inside three of these functions is a
 * struct holding a single self-typed pointer, so struct generic below has the
 * same size and layout; LIST_INIT() and LIST_EMPTY() are likewise expanded in
 * place.  malloc(), free() and panic() are the kernel primitives, supplied to
 * this translation unit from the outside.
 */

module;

#include <cstddef>

export module pbsd.sys.kern.b0146s3;

extern "C" [[noreturn]] void pbsd_env_panic(const char *fmt, ...);
extern "C" void *pbsd_env_malloc(unsigned long size, void *type, int flags);
extern "C" void pbsd_env_free(void *addr, void *type);

namespace pbsd::sys_kern::b0146s3 {

static const int primes[] = { 1, 13, 31, 61, 127, 251, 509, 761, 1021, 1531,
			2039, 2557, 3067, 3583, 4093, 4603, 5119, 5623, 6143,
			6653, 7159, 7673, 8191, 12281, 16381, 24571, 32749 };
static constexpr std::size_t NPRIMES = sizeof(primes) / sizeof(primes[0]);

} /* namespace pbsd::sys_kern::b0146s3 */

export namespace pbsd::sys_kern::b0146s3 {

using u_long = unsigned long;

struct malloc_type {
	const char *ks_shortdesc;
};

inline constexpr int M_NOWAIT = 0x0001;
inline constexpr int M_WAITOK = 0x0002;

inline constexpr int HASH_NOWAIT = 0x00000001;
inline constexpr int HASH_WAITOK = 0x00000002;

/* LIST_HEAD(generic, generic) */
struct generic {
	generic *lh_first;	/* first element */
};

int
hash_mflags(int flags)
{

	return ((flags & HASH_NOWAIT) ? M_NOWAIT : M_WAITOK);
}

/*
 * General routine to allocate a hash table with control of memory flags.
 */
void *
hashinit_flags(int elements, malloc_type *type, u_long *hashmask,
    int flags)
{
	long hashsize, i;
	generic *hashtbl;

	if (!(elements > 0))
		pbsd_env_panic("%s: bad elements", __func__);
	/* Exactly one of HASH_WAITOK and HASH_NOWAIT must be set. */
	if (!((flags & HASH_WAITOK) ^ (flags & HASH_NOWAIT)))
		pbsd_env_panic("Bad flags (0x%x) passed to hashinit_flags",
		    flags);

	for (hashsize = 1; hashsize <= elements; hashsize <<= 1)
		continue;
	hashsize >>= 1;

	hashtbl = static_cast<generic *>(pbsd_env_malloc(
	    static_cast<u_long>(hashsize) * sizeof(*hashtbl), type,
	    hash_mflags(flags)));
	if (hashtbl != nullptr) {
		for (i = 0; i < hashsize; i++)
			hashtbl[i].lh_first = nullptr;
		*hashmask = hashsize - 1;
	}
	return (hashtbl);
}

/*
 * Allocate and initialize a hash table with default flag: may sleep.
 */
void *
hashinit(int elements, malloc_type *type, u_long *hashmask)
{

	return (hashinit_flags(elements, type, hashmask, HASH_WAITOK));
}

void
hashdestroy(void *vhashtbl, malloc_type *type, u_long hashmask)
{
	generic *hashtbl, *hp;

	hashtbl = static_cast<generic *>(vhashtbl);
	for (hp = hashtbl; hp <= &hashtbl[hashmask]; hp++)
		if (!(hp->lh_first == nullptr))
			pbsd_env_panic("%s: hashtbl %p not empty "
			    "(malloc type %s)", __func__,
			    static_cast<void *>(hashtbl), type->ks_shortdesc);
	pbsd_env_free(hashtbl, type);
}

/*
 * General routine to allocate a prime number sized hash table with control of
 * memory flags.
 */
void *
phashinit_flags(int elements, malloc_type *type, u_long *nentries, int flags)
{
	long hashsize, i;
	generic *hashtbl;

	if (!(elements > 0))
		pbsd_env_panic("%s: bad elements", __func__);
	/* Exactly one of HASH_WAITOK and HASH_NOWAIT must be set. */
	if (!((flags & HASH_WAITOK) ^ (flags & HASH_NOWAIT)))
		pbsd_env_panic("Bad flags (0x%x) passed to phashinit_flags",
		    flags);

	for (i = 1, hashsize = primes[1]; hashsize <= elements;) {
		i++;
		if (i == NPRIMES)
			break;
		hashsize = primes[i];
	}
	hashsize = primes[i - 1];

	hashtbl = static_cast<generic *>(pbsd_env_malloc(
	    static_cast<u_long>(hashsize) * sizeof(*hashtbl), type,
	    hash_mflags(flags)));
	if (hashtbl == nullptr)
		return (nullptr);

	for (i = 0; i < hashsize; i++)
		hashtbl[i].lh_first = nullptr;
	*nentries = hashsize;
	return (hashtbl);
}

/*
 * Allocate and initialize a prime number sized hash table with default flag:
 * may sleep.
 */
void *
phashinit(int elements, malloc_type *type, u_long *nentries)
{

	return (phashinit_flags(elements, type, nentries, HASH_WAITOK));
}

} /* namespace pbsd::sys_kern::b0146s3 */
