/*
 * oracle.c -- reference implementation for PBSD batch b0138.
 *
 * The original HardenedBSD kernel sources are concatenated below with every
 * function renamed with a "ref_" prefix.  Function bodies are UNMODIFIED.
 * Supporting types, macros, and allocator shims are added only where the
 * original files obtained them from kernel headers.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

#define __predict_true(x) (__builtin_expect(!!(x), 1))
#define __read_frequently
#define __unused __attribute__((__unused__))

#define M_NOWAIT 0x0001
#define M_WAITOK 0x0002
#define M_ZERO   0x0100

#define powerof2(x) ((((x) - 1) & (x)) == 0)

#define KASSERT(cond, msg) ((void)0)

typedef char *caddr_t;

struct malloc_type {
	const char *ks_shortdesc;
};

struct mtx {
	int dummy;
};

struct vdso_timehands {
	uint64_t pad[4];
};

struct vdso_timehands32 {
	uint32_t pad[4];
};

struct timecounter {
	void *tc_priv;
};

struct buf_ring {
	uint32_t br_prod_head;
	uint32_t br_prod_tail;
	int br_prod_size;
	int br_prod_mask;
	uint64_t br_drops;
	uint32_t br_cons_head __attribute__((aligned(CACHE_LINE_SIZE)));
	uint32_t br_cons_tail;
	int br_cons_size;
	int br_cons_mask;
	void *br_ring[0] __attribute__((aligned(CACHE_LINE_SIZE)));
};

#define SDT_PROVIDER_DEFINE(prov)

typedef void (*sdt_probe_func_t)(uint32_t, uintptr_t arg0, uintptr_t arg1,
    uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);

static int g_malloc_calls;
static int g_malloc_fail_at;

void
oracle_malloc_reset(void)
{
	g_malloc_calls = 0;
	g_malloc_fail_at = 0;
}

void
oracle_malloc_fail_at(int n)
{
	g_malloc_fail_at = n;
}

static void *
malloc_kern(size_t size, struct malloc_type *type, int flags)
{
	(void)type;
	(void)flags;

	g_malloc_calls++;
	if (g_malloc_fail_at != 0 && g_malloc_calls >= g_malloc_fail_at)
		return (NULL);
	return (malloc(size));
}

#define malloc(size, type, flags) ({					\
	void *_malloc_item;						\
	size_t _size = (size);						\
	if (__builtin_constant_p(size) && __builtin_constant_p(flags) &&\
	    ((flags) & M_ZERO) != 0) {					\
		_malloc_item = malloc_kern(_size, type, (flags) & ~M_ZERO); \
		if (((flags) & M_WAITOK) != 0 ||				\
		    __predict_true(_malloc_item != NULL))			\
			memset(_malloc_item, 0, _size);			\
	} else {							\
		_malloc_item = malloc_kern(_size, type, flags);		\
	}								\
	_malloc_item;							\
})

static void
free_kern(void *addr, struct malloc_type *type)
{
	(void)type;
	free(addr);
}

#define free(addr, type) free_kern((addr), (type))

static int g_printf_calls;
static int g_backtrace_calls;

int
oracle_printf_calls(void)
{
	return (g_printf_calls);
}

int
oracle_backtrace_calls(void)
{
	return (g_backtrace_calls);
}

void
oracle_io_reset(void)
{
	g_printf_calls = 0;
	g_backtrace_calls = 0;
}

int
printf(const char *fmt, ...)
{
	(void)fmt;
	g_printf_calls++;
	return (0);
}

void
kdb_backtrace(void)
{
	g_backtrace_calls++;
}

/* ------------------------------------------------------------------ */
/* subr_dummy_vdso_tc.c */
/* ------------------------------------------------------------------ */

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

uint32_t
ref_cpu_fill_vdso_timehands(struct vdso_timehands *vdso_th, struct timecounter *tc)
{

	return (0);
}

#define COMPAT_FREEBSD32

#ifdef COMPAT_FREEBSD32
uint32_t
ref_cpu_fill_vdso_timehands32(struct vdso_timehands32 *vdso_th32,
    struct timecounter *tc)
{

	return (0);
}
#endif

/* ------------------------------------------------------------------ */
/* subr_early.c */
/* ------------------------------------------------------------------ */

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

#ifndef MEMSET_EARLY_FUNC
#define MEMSET_EARLY_FUNC memset
#else
void *MEMSET_EARLY_FUNC(void *, int, size_t);
#endif

void *
ref_memset_early(void *buf, int c, size_t len)
{

	return (MEMSET_EARLY_FUNC(buf, c, len));
}

#ifndef MEMCPY_EARLY_FUNC
#define MEMCPY_EARLY_FUNC memcpy
#else
void *MEMCPY_EARLY_FUNC(void *, const void *, size_t);
#endif

void *
ref_memcpy_early(void *to, const void *from, size_t len)
{

	return (MEMCPY_EARLY_FUNC(to, from, len));
}

#ifndef MEMMOVE_EARLY_FUNC
#define MEMMOVE_EARLY_FUNC memmove
#else
void *MEMMOVE_EARLY_FUNC(void *, const void *, size_t);
#endif

void *
ref_memmove_early(void *to, const void *from, size_t len)
{

	return (MEMMOVE_EARLY_FUNC(to, from, len));
}

/* ------------------------------------------------------------------ */
/* subr_bufring.c */
/* ------------------------------------------------------------------ */

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

struct buf_ring *
ref_buf_ring_alloc(int count, struct malloc_type *type, int flags, struct mtx *lock)
{
	struct buf_ring *br;

	KASSERT(powerof2(count), ("buf ring must be size power of 2"));

	br = malloc(sizeof(struct buf_ring) + count*sizeof(caddr_t),
	    type, flags|M_ZERO);
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

void
ref_buf_ring_free(struct buf_ring *br, struct malloc_type *type)
{
	free(br, type);
}

/* ------------------------------------------------------------------ */
/* kern_sdt.c */
/* ------------------------------------------------------------------ */

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

SDT_PROVIDER_DEFINE(sdt);

/*
 * Hook for the DTrace probe function. The SDT provider will set this to
 * dtrace_probe() when it loads.
 */
sdt_probe_func_t sdt_probe_func = ref_sdt_probe_stub;
volatile bool __read_frequently sdt_probes_enabled;

/*
 * This is a stub for probe calls in case kernel DTrace support isn't
 * enabled. It should never get called because there is no DTrace support
 * to enable it.
 */
void
ref_sdt_probe_stub(uint32_t id __unused, uintptr_t arg0 __unused,
    uintptr_t arg1 __unused, uintptr_t arg2 __unused, uintptr_t arg3 __unused,
    uintptr_t arg4 __unused, uintptr_t arg5 __unused)
{
	printf("sdt_probe_stub: unexpectedly called\n");
	kdb_backtrace();
}

void
ref_sdt_probe(uint32_t id, uintptr_t arg0, uintptr_t arg1,
    uintptr_t arg2, uintptr_t arg3, uintptr_t arg4)
{
	sdt_probe_func(id, arg0, arg1, arg2, arg3, arg4, 0);
}

void
ref_sdt_probe6(uint32_t id, uintptr_t arg0, uintptr_t arg1,
    uintptr_t arg2, uintptr_t arg3, uintptr_t arg4, uintptr_t arg5)
{
	sdt_probe_func(id, arg0, arg1, arg2, arg3, arg4, arg5);
}
