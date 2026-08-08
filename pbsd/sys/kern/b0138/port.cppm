// PBSD port of HardenedBSD sys/kern batch b0138.

module;

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" {
int printf(const char *fmt, ...);
void kdb_backtrace(void);
}

export module pbsd.sys.kern.b0138;

namespace pbsd::sys_kern::b0138::detail {

#define __predict_true(x) (__builtin_expect(!!(x), 1))
#define __read_frequently
#define __unused __attribute__((__unused__))

#define M_NOWAIT 0x0001
#define M_WAITOK 0x0002
#define M_ZERO   0x0100

#define powerof2(x) ((((x) - 1) & (x)) == 0)

#define KASSERT(cond, msg) ((void)0)

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

using caddr_t = char *;

struct malloc_type {
	const char *ks_shortdesc;
};

struct mtx {
	int dummy;
};

struct vdso_timehands {
	std::uint64_t pad[4];
};

struct vdso_timehands32 {
	std::uint32_t pad[4];
};

struct timecounter {
	void *tc_priv;
};

struct buf_ring {
	std::uint32_t br_prod_head;
	std::uint32_t br_prod_tail;
	int br_prod_size;
	int br_prod_mask;
	std::uint64_t br_drops;
	std::uint32_t br_cons_head __attribute__((aligned(CACHE_LINE_SIZE)));
	std::uint32_t br_cons_tail;
	int br_cons_size;
	int br_cons_mask;
	void *br_ring[0] __attribute__((aligned(CACHE_LINE_SIZE)));
};

using sdt_probe_func_t = void (*)(std::uint32_t, std::uintptr_t, std::uintptr_t,
    std::uintptr_t, std::uintptr_t, std::uintptr_t, std::uintptr_t);

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

inline void *malloc_kern(std::size_t size, malloc_type *type, int flags)
{
	void *p;

	(void)type;

	g_malloc_calls++;
	if (g_malloc_fail_at != 0 && g_malloc_calls >= g_malloc_fail_at)
		return (nullptr);
	p = std::malloc(size);
	if (p != nullptr && (flags & M_ZERO) != 0)
		std::memset(p, 0, size);
	return (p);
}

inline void *malloc_var(std::size_t size, malloc_type *type, int flags)
{
	void *item;

	if (__builtin_constant_p(size) && __builtin_constant_p(flags) &&
	    ((flags) & M_ZERO) != 0) {
		item = malloc_kern(size, type, flags & ~M_ZERO);
		if (((flags) & M_WAITOK) != 0 || __predict_true(item != nullptr))
			std::memset(item, 0, size);
	} else {
		item = malloc_kern(size, type, flags);
	}
	return (item);
}

#undef malloc

inline void free_kern(void *addr, malloc_type *type)
{
	(void)type;
	std::free(addr);
}

#ifndef MEMSET_EARLY_FUNC
#define MEMSET_EARLY_FUNC ::memset
#endif

#ifndef MEMCPY_EARLY_FUNC
#define MEMCPY_EARLY_FUNC ::memcpy
#endif

#ifndef MEMMOVE_EARLY_FUNC
#define MEMMOVE_EARLY_FUNC ::memmove
#endif

} // namespace pbsd::sys_kern::b0138::detail

export namespace pbsd::sys_kern::b0138 {

#define __unused __attribute__((__unused__))

using detail::buf_ring;
using detail::malloc_type;
using detail::mtx;
using detail::sdt_probe_func_t;
using detail::timecounter;
using detail::vdso_timehands;
using detail::vdso_timehands32;

inline sdt_probe_func_t sdt_probe_func;
inline volatile bool sdt_probes_enabled;

inline void malloc_reset() noexcept
{
	detail::malloc_reset();
}

inline void malloc_fail_at(int n) noexcept
{
	detail::malloc_fail_at(n);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2012 Konstantin Belousov <kib@FreeBSD.ORG>.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

std::uint32_t
cpu_fill_vdso_timehands(vdso_timehands *vdso_th, timecounter *tc)
{

	return (0);
}

std::uint32_t
cpu_fill_vdso_timehands32(vdso_timehands32 *vdso_th32, timecounter *tc)
{

	return (0);
}

/*-
 * Copyright (c) 2018 The FreeBSD Foundation
 *
 * This software was developed by Mateusz Guzik <mjg@FreeBSD.org>
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

void *
memset_early(void *buf, int c, std::size_t len)
{

	return (MEMSET_EARLY_FUNC(buf, c, len));
}

void *
memcpy_early(void *to, const void *from, std::size_t len)
{

	return (MEMCPY_EARLY_FUNC(to, from, len));
}

void *
memmove_early(void *to, const void *from, std::size_t len)
{

	return (MEMMOVE_EARLY_FUNC(to, from, len));
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2007, 2008 Kip Macy <kmacy@freebsd.org>
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

buf_ring *
buf_ring_alloc(int count, malloc_type *type, int flags, mtx *lock)
{
	buf_ring *br;

	KASSERT(powerof2(count), ("buf ring must be size power of 2"));

	br = static_cast<buf_ring *>(detail::malloc_var(
	    sizeof(buf_ring) + count * sizeof(detail::caddr_t),
	    type, flags | M_ZERO));
	if (br == nullptr)
		return (nullptr);
#ifdef DEBUG_BUFRING
	br->br_lock = lock;
#endif	
	br->br_prod_size = br->br_cons_size = count;
	br->br_prod_mask = br->br_cons_mask = count-1;
	br->br_prod_head = br->br_cons_head = 0;
	br->br_prod_tail = br->br_cons_tail = 0;
		
	return (br);
}

void
buf_ring_free(buf_ring *br, malloc_type *type)
{
	detail::free_kern(br, type);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2006-2008 John Birrell <jb@FreeBSD.org>
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
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

void
sdt_probe_stub(std::uint32_t id __unused, std::uintptr_t arg0 __unused,
    std::uintptr_t arg1 __unused, std::uintptr_t arg2 __unused,
    std::uintptr_t arg3 __unused, std::uintptr_t arg4 __unused,
    std::uintptr_t arg5 __unused)
{
	printf("sdt_probe_stub: unexpectedly called\n");
	kdb_backtrace();
}

void
sdt_probe(std::uint32_t id, std::uintptr_t arg0, std::uintptr_t arg1,
    std::uintptr_t arg2, std::uintptr_t arg3, std::uintptr_t arg4)
{
	sdt_probe_func(id, arg0, arg1, arg2, arg3, arg4, 0);
}

void
sdt_probe6(std::uint32_t id, std::uintptr_t arg0, std::uintptr_t arg1,
    std::uintptr_t arg2, std::uintptr_t arg3, std::uintptr_t arg4,
    std::uintptr_t arg5)
{
	sdt_probe_func(id, arg0, arg1, arg2, arg3, arg4, arg5);
}

} // namespace pbsd::sys_kern::b0138
