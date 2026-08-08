module;

#include <cstddef>
#include <cstdint>
#include <cstdlib>

export module pbsd.sys.kern.b0146s3;

namespace pbsd::sys_kern::b0146s3::detail {

#define KASSERT(cond, msg) ((void)0)
#define nitems(x) (sizeof((x)) / sizeof((x)[0]))

using u_long = unsigned long;

#define M_NOWAIT 0x0001
#define M_WAITOK 0x0002

#define HASH_NOWAIT 0x00000001
#define HASH_WAITOK 0x00000002

#define LIST_HEAD(name, type)						\
struct name {								\
	struct type *lh_first;						\
}

#define LIST_INIT(head) do {						\
	(head)->lh_first = nullptr;					\
} while (0)

#define LIST_EMPTY(head) ((head)->lh_first == nullptr)

struct generic {
	struct generic *lh_first;
};

struct malloc_type {
	const char *ks_shortdesc;
};

inline int g_malloc_calls;
inline int g_malloc_fail_at;
inline std::size_t g_malloc_last_size;
inline int g_malloc_last_flags;

inline void malloc_reset() noexcept
{
	g_malloc_calls = 0;
	g_malloc_fail_at = 0;
	g_malloc_last_size = 0;
	g_malloc_last_flags = 0;
}

inline void malloc_fail_at(int n) noexcept
{
	g_malloc_fail_at = n;
}

inline void *
kern_malloc(u_long size, malloc_type *type, int flags)
{
	void *p;

	(void)type;

	g_malloc_calls++;
	g_malloc_last_size = size;
	g_malloc_last_flags = flags;
	if (g_malloc_fail_at != 0 && g_malloc_calls >= g_malloc_fail_at)
		return (nullptr);
	p = std::malloc(size);
	return (p);
}

inline void
kern_free(void *addr, malloc_type *type)
{
	(void)type;
	std::free(addr);
}

#define malloc kern_malloc
#define free kern_free

} // namespace pbsd::sys_kern::b0146s3::detail

export namespace pbsd::sys_kern::b0146s3 {

struct malloc_type {
	const char *ks_shortdesc;
};

using u_long = unsigned long;

#define KASSERT(cond, msg) ((void)0)
#define nitems(x) (sizeof((x)) / sizeof((x)[0]))
#define LIST_INIT(head) do { (head)->lh_first = nullptr; } while (0)
#define LIST_EMPTY(head) ((head)->lh_first == nullptr)

#define M_NOWAIT 0x0001
#define M_WAITOK 0x0002
#define HASH_NOWAIT 0x00000001
#define HASH_WAITOK 0x00000002

struct generic_list_head {
	struct detail::generic *lh_first;
};

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

void *
hashinit_flags(int elements, malloc_type *type, u_long *hashmask,
    int flags)
{
	long hashsize, i;
	generic_list_head *hashtbl;

KASSERT(elements > 0, ("%s: bad elements", __func__));
KASSERT((flags & HASH_WAITOK) ^ (flags & HASH_NOWAIT),
	    ("Bad flags (0x%x) passed to hashinit_flags", flags));

	for (hashsize = 1; hashsize <= elements; hashsize <<= 1)
		continue;
	hashsize >>= 1;

	hashtbl = static_cast<generic_list_head *>(
	    detail::kern_malloc(static_cast<u_long>(hashsize) * sizeof(*hashtbl),
	    reinterpret_cast<detail::malloc_type *>(type),
	    hash_mflags(flags)));
	if (hashtbl != nullptr) {
		for (i = 0; i < hashsize; i++)
LIST_INIT(&hashtbl[i]);
		*hashmask = hashsize - 1;
	}
	return (hashtbl);
}

void *
hashinit(int elements, malloc_type *type, u_long *hashmask)
{

	return (hashinit_flags(elements, type, hashmask, HASH_WAITOK));
}

void
hashdestroy(void *vhashtbl, malloc_type *type, u_long hashmask)
{
	generic_list_head *hashtbl, *hp;

	hashtbl = static_cast<generic_list_head *>(vhashtbl);
	for (hp = hashtbl; hp <= &hashtbl[hashmask]; hp++)
KASSERT(LIST_EMPTY(hp), ("%s: hashtbl %p not empty "
		    "(malloc type %s)", __func__, hashtbl, type->ks_shortdesc));
detail::kern_free(hashtbl, reinterpret_cast<detail::malloc_type *>(type));
}

static const int primes[] = { 1, 13, 31, 61, 127, 251, 509, 761, 1021, 1531,
			2039, 2557, 3067, 3583, 4093, 4603, 5119, 5623, 6143,
			6653, 7159, 7673, 8191, 12281, 16381, 24571, 32749 };
#define	NPRIMES nitems(primes)

void *
phashinit_flags(int elements, malloc_type *type, u_long *nentries, int flags)
{
	long hashsize, i;
	generic_list_head *hashtbl;

KASSERT(elements > 0, ("%s: bad elements", __func__));
KASSERT((flags & HASH_WAITOK) ^ (flags & HASH_NOWAIT),
	    ("Bad flags (0x%x) passed to phashinit_flags", flags));

	for (i = 1, hashsize = primes[1]; hashsize <= elements;) {
		i++;
		if (i == NPRIMES)
			break;
		hashsize = primes[i];
	}
	hashsize = primes[i - 1];

	hashtbl = static_cast<generic_list_head *>(
	    detail::kern_malloc(static_cast<u_long>(hashsize) * sizeof(*hashtbl),
	    reinterpret_cast<detail::malloc_type *>(type),
	    hash_mflags(flags)));
	if (hashtbl == nullptr)
		return (nullptr);

	for (i = 0; i < hashsize; i++)
LIST_INIT(&hashtbl[i]);
	*nentries = hashsize;
	return (hashtbl);
}

void *
phashinit(int elements, malloc_type *type, u_long *nentries)
{

	return (phashinit_flags(elements, type, nentries, HASH_WAITOK));
}

inline void malloc_reset() noexcept
{
	detail::malloc_reset();
}

inline void malloc_fail_at(int n) noexcept
{
	detail::malloc_fail_at(n);
}

inline int malloc_calls() noexcept
{
	return (detail::g_malloc_calls);
}

inline std::size_t malloc_last_size() noexcept
{
	return (detail::g_malloc_last_size);
}

inline int malloc_last_flags() noexcept
{
	return (detail::g_malloc_last_flags);
}

} // namespace pbsd::sys_kern::b0146s3
