// PBSD port of HardenedBSD sys/kern batch b0146.

module;

#define _POSIX_C_SOURCE 200809L

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <pthread.h>

export module pbsd.sys.kern.b0146;

namespace pbsd::sys_kern::b0146::detail {

#define __unused __attribute__((__unused__))

#define KASSERT(cond, msg) ((void)0)
#define CTASSERT(x) typedef char __ctassert[(x) ? 1 : -1] __attribute__((__unused__))

#define nitems(x) (sizeof((x)) / sizeof((x)[0]))

using u_long = unsigned long;

#define M_NOWAIT 0x0001
#define M_WAITOK 0x0002

#define HASH_WAITOK 0x00000001
#define HASH_NOWAIT 0x00000002

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

#define GRND_NONBLOCK 0x0001
#define GRND_RANDOM   0x0002
#define GRND_INSECURE 0x0004
#define GRND_VALIDFLAGS (GRND_NONBLOCK | GRND_RANDOM | GRND_INSECURE)

#define IOSIZE_MAX 0x80000000UL

#define UIO_USERSPACE 1
#define UIO_READ      1

#define EINVAL 22
#define EWOULDBLOCK 35
#define EAGAIN 35

struct thread {
	long td_retval[2];
};

struct iovec {
	void *iov_base;
	std::size_t iov_len;
};

struct uio {
	iovec *uio_iov;
	int uio_iovcnt;
	off_t uio_offset;
	std::size_t uio_resid;
	int uio_segflg;
	int uio_rw;
	thread *uio_td;
};

struct getrandom_args {
	void *buf;
	std::size_t buflen;
	unsigned int flags;
};

#define EFI_PAGE_SIZE 4096
#define EXFLAG_NOALLOC 1

#define EFI_MD_TYPE_RESERVED 0
#define EFI_MD_TYPE_LOADER_CODE 1
#define EFI_MD_TYPE_LOADER_DATA 2
#define EFI_MD_TYPE_BS_CODE 3
#define EFI_MD_TYPE_BS_DATA 4
#define EFI_MD_TYPE_RT_CODE 5
#define EFI_MD_TYPE_RT_DATA 6
#define EFI_MD_TYPE_FREE 7
#define EFI_MD_TYPE_UNUSABLE 8
#define EFI_MD_TYPE_RECLAIM 9

#define EFI_MD_TYPE_CODE EFI_MD_TYPE_LOADER_CODE
#define EFI_MD_TYPE_DATA EFI_MD_TYPE_LOADER_DATA

#define EFI_MD_ATTR_UC 0x1
#define EFI_MD_ATTR_WC 0x2
#define EFI_MD_ATTR_WT 0x4
#define EFI_MD_ATTR_WB 0x8
#define EFI_MD_ATTR_UCE 0x10
#define EFI_MD_ATTR_WP 0x20
#define EFI_MD_ATTR_RP 0x40
#define EFI_MD_ATTR_XP 0x80
#define EFI_MD_ATTR_NV 0x100
#define EFI_MD_ATTR_MORE_RELIABLE 0x200
#define EFI_MD_ATTR_RO 0x400
#define EFI_MD_ATTR_RT 0x800

struct efi_map_header {
	std::size_t memory_size;
	std::size_t descriptor_size;
};

struct efi_md {
	std::uint32_t md_type;
	std::uint32_t md_pad;
	std::uint64_t md_phys;
	std::uint64_t md_virt;
	std::uint64_t md_pages;
	std::uint64_t md_attr;
};

using efi_map_entry_cb = void (*)(efi_md *p, void *argp);

inline efi_md *
efi_next_descriptor(efi_md *md, std::size_t size)
{
	return (reinterpret_cast<efi_md *>(reinterpret_cast<std::uint8_t *>(md) +
	    size));
}

constexpr int MTX_DEF = 0;
constexpr int MTX_NOWITNESS = 0;
constexpr int MTX_QUIET = 0;

#define KTR_LOCK 0
#define CTR3(...) ((void)0)
#define CTR4(...) ((void)0)
#define CTR5(...) ((void)0)
#define CTR6(...) ((void)0)

struct mtx {
	pthread_mutex_t lock;
	const char *name;
};

struct cv {
	pthread_cond_t cond;
	const char *name;
};

struct sema {
	mtx sema_mtx;
	cv sema_cv;
	int sema_value;
	int sema_waiters;
};

inline int g_malloc_calls;
inline int g_malloc_fail_at;

inline void malloc_reset() noexcept
{
	g_malloc_calls = 0;
	g_malloc_fail_at = 0;
}

inline void malloc_fail_at(int n) noexcept
{
	g_malloc_fail_at = n;
}

inline void *
kern_malloc(std::size_t size, malloc_type *type, int flags)
{
	void *p;

	(void)type;
	(void)flags;

	g_malloc_calls++;
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

inline void
bzero(void *p, std::size_t n)
{
	std::memset(p, 0, n);
}

inline void
mtx_init(mtx *m, const char *name, const char *desc, int opts)
{
	(void)desc;
	(void)opts;
	m->name = name;
	pthread_mutex_init(&m->lock, nullptr);
}

inline void
mtx_destroy(mtx *m)
{
	pthread_mutex_destroy(&m->lock);
}

inline void
mtx_lock(mtx *m)
{
	pthread_mutex_lock(&m->lock);
}

inline void
mtx_unlock(mtx *m)
{
	pthread_mutex_unlock(&m->lock);
}

inline void
cv_init(cv *c, const char *name)
{
	c->name = name;
	pthread_cond_init(&c->cond, nullptr);
}

inline void
cv_destroy(cv *c)
{
	pthread_cond_destroy(&c->cond);
}

inline void
cv_signal(cv *c)
{
	pthread_cond_signal(&c->cond);
}

inline void
cv_wait(cv *c, mtx *m)
{
	pthread_cond_wait(&c->cond, &m->lock);
}

inline int
cv_timedwait(cv *c, mtx *m, int timo)
{
	struct timespec ts;

	if (timo == 0)
		return (EWOULDBLOCK);
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += timo / 1000;
	ts.tv_nsec += (timo % 1000) * 1000000L;
	if (ts.tv_nsec >= 1000000000L) {
		ts.tv_sec++;
		ts.tv_nsec -= 1000000000L;
	}
	if (pthread_cond_timedwait(&c->cond, &m->lock, &ts) != 0)
		return (EWOULDBLOCK);
	return (0);
}

inline const char *
cv_wmesg(cv *c)
{
	return (c->name);
}

inline int g_read_random_error;
inline int g_read_random_block;
inline ssize_t g_read_random_transfer;

inline void read_random_reset() noexcept
{
	g_read_random_error = 0;
	g_read_random_block = 0;
	g_read_random_transfer = -1;
}

inline void read_random_configure(int error, int block, ssize_t transfer) noexcept
{
	g_read_random_error = error;
	g_read_random_block = block;
	g_read_random_transfer = transfer;
}

inline int
read_random_uio(uio *auio, int nonblock)
{
	std::size_t transfer;
	iovec *iov;
	unsigned char *bp;
	std::size_t i;

	if (g_read_random_error != 0)
		return (g_read_random_error);
	if (nonblock && g_read_random_block)
		return (EWOULDBLOCK);
	transfer = auio->uio_resid;
	if (g_read_random_transfer >= 0 &&
	    transfer > static_cast<std::size_t>(g_read_random_transfer))
		transfer = static_cast<std::size_t>(g_read_random_transfer);
	iov = auio->uio_iov;
	bp = static_cast<unsigned char *>(iov->iov_base);
	for (i = 0; i < transfer; i++)
		bp[i] = static_cast<unsigned char>(0xa5 ^ (i & 0xff));
	auio->uio_resid -= transfer;
	return (0);
}

constexpr int PHYSMEM_LOG_MAX = 4096;

struct physmem_log_rec {
	int exclude;
	std::uint64_t phys;
	std::uint64_t size;
	int exflag;
};

inline physmem_log_rec g_physmem_log[PHYSMEM_LOG_MAX];
inline int g_physmem_log_count;

inline void physmem_reset() noexcept
{
	g_physmem_log_count = 0;
}

inline int physmem_log_count() noexcept
{
	return (g_physmem_log_count);
}

inline const physmem_log_rec *
physmem_log_at(int i) noexcept
{
	return (&g_physmem_log[i]);
}

inline void
physmem_hardware_region(std::uint64_t phys, std::uint64_t size)
{
	if (g_physmem_log_count < PHYSMEM_LOG_MAX) {
		g_physmem_log[g_physmem_log_count].exclude = 0;
		g_physmem_log[g_physmem_log_count].phys = phys;
		g_physmem_log[g_physmem_log_count].size = size;
		g_physmem_log[g_physmem_log_count].exflag = 0;
		g_physmem_log_count++;
	}
}

inline void
physmem_exclude_region(std::uint64_t phys, std::uint64_t size, int exflag)
{
	if (g_physmem_log_count < PHYSMEM_LOG_MAX) {
		g_physmem_log[g_physmem_log_count].exclude = 1;
		g_physmem_log[g_physmem_log_count].phys = phys;
		g_physmem_log[g_physmem_log_count].size = size;
		g_physmem_log[g_physmem_log_count].exflag = exflag;
		g_physmem_log_count++;
	}
}

} // namespace pbsd::sys_kern::b0146::detail

export namespace pbsd::sys_kern::b0146 {

using detail::cv;
using detail::efi_map_entry_cb;
using detail::efi_map_header;
using detail::efi_md;
using detail::getrandom_args;
using detail::iovec;
using detail::malloc_type;
using detail::mtx;
using detail::sema;
using detail::thread;
using detail::u_long;
using detail::uio;

#define __unused __attribute__((__unused__))
#define KASSERT(cond, msg) ((void)0)
#define CTASSERT(x) typedef char __ctassert[(x) ? 1 : -1] __attribute__((__unused__))
#define nitems(x) (sizeof((x)) / sizeof((x)[0]))
#define LIST_INIT(head) do { (head)->lh_first = nullptr; } while (0)
#define LIST_EMPTY(head) ((head)->lh_first == nullptr)

#define M_NOWAIT 0x0001
#define M_WAITOK 0x0002
#define HASH_WAITOK 0x00000001
#define HASH_NOWAIT 0x00000002
#define GRND_NONBLOCK 0x0001
#define GRND_RANDOM 0x0002
#define GRND_INSECURE 0x0004
#define GRND_VALIDFLAGS (GRND_NONBLOCK | GRND_RANDOM | GRND_INSECURE)
#define IOSIZE_MAX 0x80000000UL
#define UIO_USERSPACE 1
#define UIO_READ 1
#define EINVAL 22
#define EWOULDBLOCK 35
#define EAGAIN 35
#define EFI_PAGE_SIZE 4096
#define EXFLAG_NOALLOC 1
#define EFI_MD_TYPE_RECLAIM 9
#define EFI_MD_TYPE_RT_CODE 5
#define EFI_MD_TYPE_RT_DATA 6
#define EFI_MD_TYPE_CODE 1
#define EFI_MD_TYPE_DATA 2
#define EFI_MD_TYPE_BS_CODE 3
#define EFI_MD_TYPE_BS_DATA 4
#define EFI_MD_TYPE_FREE 7
#define EFI_MD_ATTR_UC 0x1
#define EFI_MD_ATTR_WC 0x2
#define EFI_MD_ATTR_WT 0x4
#define EFI_MD_ATTR_WB 0x8
#define EFI_MD_ATTR_UCE 0x10
#define EFI_MD_ATTR_WP 0x20
#define EFI_MD_ATTR_RP 0x40
#define EFI_MD_ATTR_XP 0x80
#define EFI_MD_ATTR_NV 0x100
#define EFI_MD_ATTR_MORE_RELIABLE 0x200
#define EFI_MD_ATTR_RO 0x400
#define EFI_MD_ATTR_RT 0x800
#define KTR_LOCK 0
#define CTR3(...)
#define CTR4(...)
#define CTR5(...)
#define CTR6(...)

struct generic_list_head {
	struct generic *lh_first;
};

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2018 Conrad Meyer <cem@FreeBSD.org>
 * All rights reserved.
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

#define GRND_VALIDFLAGS	(GRND_NONBLOCK | GRND_RANDOM | GRND_INSECURE)

CTASSERT(EWOULDBLOCK == EAGAIN);

static int
kern_getrandom(thread *td, void *user_buf, std::size_t buflen,
    unsigned int flags)
{
uio auio;
iovec aiov;
	int error;

	if ((flags & ~GRND_VALIDFLAGS) != 0)
		return (EINVAL);
	if (buflen > IOSIZE_MAX)
		return (EINVAL);

	if ((flags & GRND_INSECURE) != 0)
		flags |= GRND_NONBLOCK;

	if (buflen == 0) {
		td->td_retval[0] = 0;
		return (0);
	}

	aiov.iov_base = user_buf;
	aiov.iov_len = buflen;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_offset = 0;
	auio.uio_resid = buflen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw = UIO_READ;
	auio.uio_td = td;

	error = detail::read_random_uio(&auio, (flags & GRND_NONBLOCK) != 0);
	if (error == 0)
		td->td_retval[0] = static_cast<long>(buflen - auio.uio_resid);
	return (error);
}

int
sys_getrandom(thread *td, getrandom_args *uap)
{
	return (kern_getrandom(td, uap->buf, uap->buflen, uap->flags));
}

/*
 * Copyright (c) 2014 The FreeBSD Foundation
 * Copyright (c) 2018 Andrew Turner
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

void
efi_map_foreach_entry(efi_map_header *efihdr, efi_map_entry_cb cb, void *argp)
{
	efi_md *map, *p;
	std::size_t efisz;
	int ndesc, i;

	efisz = (sizeof(efi_map_header) + 0xf) & ~0xf;
	map = reinterpret_cast<efi_md *>(reinterpret_cast<std::uint8_t *>(efihdr) +
	    efisz);

	if (efihdr->descriptor_size == 0)
		return;
	ndesc = static_cast<int>(efihdr->memory_size / efihdr->descriptor_size);

	for (i = 0, p = map; i < ndesc; i++,
	    p = detail::efi_next_descriptor(p, efihdr->descriptor_size)) {
		cb(p, argp);
	}
}

static void
handle_efi_map_entry(efi_md *p, void *argp)
{
	bool exclude = *static_cast<bool *>(argp);

	switch (p->md_type) {
	case EFI_MD_TYPE_RECLAIM:
	case EFI_MD_TYPE_RT_CODE:
	case EFI_MD_TYPE_RT_DATA:
		if (exclude) {
			detail::physmem_exclude_region(p->md_phys,
			    p->md_pages * EFI_PAGE_SIZE, EXFLAG_NOALLOC);
			break;
		}
		/* FALLTHROUGH */
	case EFI_MD_TYPE_CODE:
	case EFI_MD_TYPE_DATA:
	case EFI_MD_TYPE_BS_CODE:
	case EFI_MD_TYPE_BS_DATA:
	case EFI_MD_TYPE_FREE:
		if (!exclude)
			detail::physmem_hardware_region(p->md_phys,
			    p->md_pages * EFI_PAGE_SIZE);
		break;
	default:
		break;
	}
}

void
efi_map_add_entries(efi_map_header *efihdr)
{
	bool exclude = false;
	efi_map_foreach_entry(efihdr, handle_efi_map_entry, &exclude);
}

void
efi_map_exclude_entries(efi_map_header *efihdr)
{
	bool exclude = true;
	efi_map_foreach_entry(efihdr, handle_efi_map_entry, &exclude);
}

static void
print_efi_map_entry(efi_md *p, void *argp __unused)
{
	const char *type;
	static const char *types[] = {
		"Reserved",
		"LoaderCode",
		"LoaderData",
		"BootServicesCode",
		"BootServicesData",
		"RuntimeServicesCode",
		"RuntimeServicesData",
		"ConventionalMemory",
		"UnusableMemory",
		"ACPIReclaimMemory",
		"ACPIMemoryNVS",
		"MemoryMappedIO",
		"MemoryMappedIOPortSpace",
		"PalCode",
		"PersistentMemory"
	};

	if (p->md_type < nitems(types))
		type = types[p->md_type];
	else
		type = "<INVALID>";
	std::printf("%23s %012jx %012jx %08jx ", type,
	    static_cast<std::uintmax_t>(p->md_phys),
	    static_cast<std::uintmax_t>(p->md_virt),
	    static_cast<std::uintmax_t>(p->md_pages));
	if (p->md_attr & EFI_MD_ATTR_UC)
		std::printf("UC ");
	if (p->md_attr & EFI_MD_ATTR_WC)
		std::printf("WC ");
	if (p->md_attr & EFI_MD_ATTR_WT)
		std::printf("WT ");
	if (p->md_attr & EFI_MD_ATTR_WB)
		std::printf("WB ");
	if (p->md_attr & EFI_MD_ATTR_UCE)
		std::printf("UCE ");
	if (p->md_attr & EFI_MD_ATTR_WP)
		std::printf("WP ");
	if (p->md_attr & EFI_MD_ATTR_RP)
		std::printf("RP ");
	if (p->md_attr & EFI_MD_ATTR_XP)
		std::printf("XP ");
	if (p->md_attr & EFI_MD_ATTR_NV)
		std::printf("NV ");
	if (p->md_attr & EFI_MD_ATTR_MORE_RELIABLE)
		std::printf("MORE_RELIABLE ");
	if (p->md_attr & EFI_MD_ATTR_RO)
		std::printf("RO ");
	if (p->md_attr & EFI_MD_ATTR_RT)
		std::printf("RUNTIME");
	std::printf("\n");
}

void
efi_map_print_entries(efi_map_header *efihdr)
{

	std::printf("%23s %12s %12s %8s %4s\n",
	    "Type", "Physical", "Virtual", "#Pages", "Attr");
	efi_map_foreach_entry(efihdr, print_efi_map_entry, nullptr);
}

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
	    detail::kern_malloc(static_cast<u_long>(hashsize) * sizeof(*hashtbl), type,
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
detail::kern_free(hashtbl, type);
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
	    detail::kern_malloc(static_cast<u_long>(hashsize) * sizeof(*hashtbl), type,
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

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (C) 2001 Jason Evans <jasone@freebsd.org>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice(s), this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified other than the possible 
 *    addition of one or more copyright notices.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice(s), this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

void
sema_init(sema *sema, int value, const char *description)
{

KASSERT((value >= 0), ("%s(): negative value\n", __func__));

	detail::bzero(sema, sizeof(*sema));
	detail::mtx_init(&sema->sema_mtx, description, "sema backing lock",
	    detail::MTX_DEF | detail::MTX_NOWITNESS | detail::MTX_QUIET);
	detail::cv_init(&sema->sema_cv, description);
	sema->sema_value = value;

	CTR4(KTR_LOCK, "%s(%p, %d, \"%s\")", __func__, sema, value, description);
}

void
sema_destroy(sema *sema)
{

	CTR3(KTR_LOCK, "%s(%p) \"%s\"", __func__, sema,
	    detail::cv_wmesg(&sema->sema_cv));

	detail::KASSERT((sema->sema_waiters == 0), ("%s(): waiters\n", __func__));

	detail::mtx_destroy(&sema->sema_mtx);
	detail::cv_destroy(&sema->sema_cv);
}

void
_sema_post(sema *sema, const char *file, int line)
{

	detail::mtx_lock(&sema->sema_mtx);
	sema->sema_value++;
	if (sema->sema_waiters && sema->sema_value > 0)
		detail::cv_signal(&sema->sema_cv);

	CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
	    detail::cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);

	detail::mtx_unlock(&sema->sema_mtx);
}

void
_sema_wait(sema *sema, const char *file, int line)
{

	detail::mtx_lock(&sema->sema_mtx);
	while (sema->sema_value == 0) {
		sema->sema_waiters++;
		detail::cv_wait(&sema->sema_cv, &sema->sema_mtx);
		sema->sema_waiters--;
	}
	sema->sema_value--;

	CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
	    detail::cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);

	detail::mtx_unlock(&sema->sema_mtx);
}

int
_sema_timedwait(sema *sema, int timo, const char *file, int line)
{
	int error;

	detail::mtx_lock(&sema->sema_mtx);

	for (error = 0; sema->sema_value == 0 && error == 0;) {
		sema->sema_waiters++;
		error = detail::cv_timedwait(&sema->sema_cv, &sema->sema_mtx, timo);
		sema->sema_waiters--;
	}
	if (sema->sema_value > 0) {
		sema->sema_value--;
		error = 0;

		CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
		    detail::cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);
	} else {
		CTR5(KTR_LOCK, "%s(%p) \"%s\" fail at %s:%d", __func__, sema,
		    detail::cv_wmesg(&sema->sema_cv), file, line);
	}

	detail::mtx_unlock(&sema->sema_mtx);
	return (error);
}

int
_sema_trywait(sema *sema, const char *file, int line)
{
	int ret;

mtx_lock(&sema->sema_mtx);

	if (sema->sema_value > 0) {
		sema->sema_value--;
		ret = 1;

		CTR6(KTR_LOCK, "%s(%p) \"%s\" v = %d at %s:%d", __func__, sema,
cv_wmesg(&sema->sema_cv), sema->sema_value, file, line);
	} else {
		ret = 0;

		CTR5(KTR_LOCK, "%s(%p) \"%s\" fail at %s:%d", __func__, sema,
cv_wmesg(&sema->sema_cv), file, line);
	}

mtx_unlock(&sema->sema_mtx);
	return (ret);
}

int
sema_value(sema *sema)
{
	int ret;

mtx_lock(&sema->sema_mtx);
	ret = sema->sema_value;
mtx_unlock(&sema->sema_mtx);
	return (ret);
}

} // namespace pbsd::sys_kern::b0146::detail

export namespace pbsd::sys_kern::b0146 {

using detail::cv;
using detail::efi_map_entry_cb;
using detail::efi_map_header;
using detail::efi_md;
using detail::getrandom_args;
using detail::malloc_type;
using detail::mtx;
using detail::physmem_log_rec;
using detail::sema;
using detail::thread;
using detail::u_long;

using detail::hashdestroy;
using detail::hashinit;
using detail::hashinit_flags;
using detail::phashinit;
using detail::phashinit_flags;
using detail::sys_getrandom;
using detail::efi_map_add_entries;
using detail::efi_map_exclude_entries;
using detail::efi_map_foreach_entry;
using detail::efi_map_print_entries;
using detail::sema_destroy;
using detail::sema_init;
using detail::sema_value;
using detail::_sema_post;
using detail::_sema_timedwait;
using detail::_sema_trywait;
using detail::_sema_wait;

inline void malloc_reset() noexcept
{
	detail::malloc_reset();
}

inline void malloc_fail_at(int n) noexcept
{
	detail::malloc_fail_at(n);
}

inline void read_random_reset() noexcept
{
	detail::read_random_reset();
}

inline void read_random_configure(int error, int block, ssize_t transfer) noexcept
{
	detail::read_random_configure(error, block, transfer);
}

inline void physmem_reset() noexcept
{
	detail::physmem_reset();
}

inline int physmem_log_count() noexcept
{
	return (detail::physmem_log_count());
}

inline const physmem_log_rec *physmem_log_entry(int i) noexcept
{
	return (detail::physmem_log_at(i));
}

} // namespace pbsd::sys_kern::b0146
