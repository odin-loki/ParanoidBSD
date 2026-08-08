/*
 * PBSD batch b0138 -- C++23 port of:
 *
 *	sys/kern/subr_dummy_vdso_tc.c
 *	sys/kern/subr_early.c
 *	sys/kern/subr_bufring.c
 *	sys/kern/kern_sdt.c
 *
 * The original copyright notices are reproduced above each ported block.
 * The ports are literal: signedness, evaluation order, pointer arithmetic
 * and the (non-INVARIANTS) no-op KASSERT are preserved exactly.
 */

module;

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

export module pbsd.sys.kern.b0138;

namespace pbsd::sys_kern::b0138 {

/*
 * Kernel environment: sys/param.h, sys/systm.h, sys/malloc.h,
 * sys/buf_ring.h, sys/sdt.h, sys/kdb.h and sys/vdso.h equivalents.
 */

export using caddr_t = char *;

export struct malloc_type;
export struct mtx;
export struct vdso_timehands;
export struct vdso_timehands32;
export struct timecounter;

export inline constexpr int M_NOWAIT = 0x0001;
export inline constexpr int M_WAITOK = 0x0002;
export inline constexpr int M_ZERO = 0x0100;

#define	CACHE_LINE_SIZE	64
#define	__aligned(x)	__attribute__((__aligned__(x)))

/* sys/buf_ring.h */
export struct buf_ring {
	volatile uint32_t	br_prod_head;
	volatile uint32_t	br_prod_tail;
	int			br_prod_size;
	int			br_prod_mask;
	uint64_t		br_drops;
	volatile uint32_t	br_cons_head __aligned(CACHE_LINE_SIZE);
	volatile uint32_t	br_cons_tail;
	int			br_cons_size;
	int			br_cons_mask;
#ifdef DEBUG_BUFRING
	mtx			*br_lock;
#endif
	void			*br_ring[0] __aligned(CACHE_LINE_SIZE);
};

/* sys/sdt.h */
export struct sdt_provider {
	const char	*name;
	void		*prov;
	uintptr_t	 sdt_id;
};

export using sdt_probe_func_t = void (*)(uint32_t, uintptr_t, uintptr_t,
    uintptr_t, uintptr_t, uintptr_t, uintptr_t);

export void sdt_probe_stub(uint32_t id, uintptr_t arg0, uintptr_t arg1,
    uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);

/* Kernel services, provided by the host. */
extern "C" void *pbsd_kern_malloc(size_t size, malloc_type *type, int flags);
extern "C" void pbsd_kern_free(void *addr, malloc_type *type);
extern "C" int pbsd_kern_printf(const char *fmt, ...);
extern "C" void kdb_backtrace(void);

/*
 * ==================================================================
 * sys/kern/subr_dummy_vdso_tc.c
 * ==================================================================
 */

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

export uint32_t
cpu_fill_vdso_timehands(vdso_timehands *vdso_th, timecounter *tc)
{

	return (0);
}

/* COMPAT_FREEBSD32 */
export uint32_t
cpu_fill_vdso_timehands32(vdso_timehands32 *vdso_th32,
    timecounter *tc)
{

	return (0);
}

/*
 * ==================================================================
 * sys/kern/subr_early.c
 * ==================================================================
 */

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

/* MEMSET_EARLY_FUNC, MEMCPY_EARLY_FUNC, MEMMOVE_EARLY_FUNC default to the
 * plain string routines on this machine. */

export void *
memset_early(void *buf, int c, size_t len)
{

	return (::memset(buf, c, len));
}

export void *
memcpy_early(void *to, const void *from, size_t len)
{

	return (::memcpy(to, from, len));
}

export void *
memmove_early(void *to, const void *from, size_t len)
{

	return (::memmove(to, from, len));
}

/*
 * ==================================================================
 * sys/kern/subr_bufring.c
 * ==================================================================
 */

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

export buf_ring *
buf_ring_alloc(int count, malloc_type *type, int flags, mtx *lock)
{
	buf_ring *br;

	/* KASSERT(powerof2(count), ...) -- no-op without INVARIANTS. */

	br = static_cast<buf_ring *>(pbsd_kern_malloc(sizeof(buf_ring) +
	    count*sizeof(caddr_t), type, flags|M_ZERO));
	if (br == NULL)
		return (NULL);
#ifdef DEBUG_BUFRING
	br->br_lock = lock;
#endif
	br->br_prod_size = br->br_cons_size = count;
	br->br_prod_mask = br->br_cons_mask = count-1;
	br->br_prod_head = br->br_cons_head = 0;
	br->br_prod_tail = br->br_cons_tail = 0;

	return (br);
}

export void
buf_ring_free(buf_ring *br, malloc_type *type)
{
	pbsd_kern_free(br, type);
}

/*
 * ==================================================================
 * sys/kern/kern_sdt.c
 * ==================================================================
 */

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

/* SDT_PROVIDER_DEFINE(sdt); */
export sdt_provider sdt_provider_sdt = { "sdt", NULL, 0 };

/*
 * Hook for the DTrace probe function. The SDT provider will set this to
 * dtrace_probe() when it loads.
 */
export sdt_probe_func_t sdt_probe_func = sdt_probe_stub;
export volatile bool sdt_probes_enabled;

/*
 * This is a stub for probe calls in case kernel DTrace support isn't
 * enabled. It should never get called because there is no DTrace support
 * to enable it.
 */
export void
sdt_probe_stub(uint32_t id, uintptr_t arg0,
    uintptr_t arg1, uintptr_t arg2, uintptr_t arg3,
    uintptr_t arg4, uintptr_t arg5)
{
	pbsd_kern_printf("sdt_probe_stub: unexpectedly called\n");
	kdb_backtrace();
}

export void
sdt_probe(uint32_t id, uintptr_t arg0, uintptr_t arg1,
    uintptr_t arg2, uintptr_t arg3, uintptr_t arg4)
{
	sdt_probe_func(id, arg0, arg1, arg2, arg3, arg4, 0);
}

export void
sdt_probe6(uint32_t id, uintptr_t arg0, uintptr_t arg1,
    uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5)
{
	sdt_probe_func(id, arg0, arg1, arg2, arg3, arg4, arg5);
}

} /* namespace pbsd::sys_kern::b0138 */
