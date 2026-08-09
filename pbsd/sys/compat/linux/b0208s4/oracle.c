/*
 * b0208s4 oracle -- reference C implementation.
 *
 * Source: hbsd/src/sys/compat/linux/linux_getcwd.c
 *
 * Every ported function appears below with a `ref_' prefix and an otherwise
 * UNMODIFIED body.  Everything above the ported function is the kernel
 * environment the body needs in order to link in user space: the types it
 * dereferences (struct thread, struct linux_getcwd_args), the constants it
 * tests (LINUX_PATH_MAX, ERANGE, ENOMEM, M_TEMP, M_WAITOK) and the three
 * kernel primitives it calls (malloc/free, vn_getcwd, copyout).
 *
 * The primitives are deterministic models driven by the kmock_* globals, and
 * both the oracle and the C++23 port call THESE SAME primitives, so any
 * observable difference between the two runs comes from the ported logic
 * alone.  Every call is appended to kmock_trace[], which lets the harness
 * compare not only the return value but the exact sequence of kernel
 * interactions (allocation size, the length handed to vn_getcwd, the offset
 * and length handed to copyout, the pointer handed to free).
 */

#include <stddef.h>
#include <string.h>
#include <errno.h>

/* ------------------------------------------------------------------ */
/* Kernel types and constants the ported body depends on.             */
/* ------------------------------------------------------------------ */

#define LINUX_PATH_MAX	4096		/* compat/linux/linux.h */

#define M_NOWAIT	0x0001		/* sys/malloc.h */
#define M_WAITOK	0x0002

#ifndef __predict_false
#define __predict_false(exp)	__builtin_expect((exp), 0)
#endif

typedef unsigned long l_ulong;		/* linux ABI unsigned long */
typedef long register_t;		/* machine/types.h */

struct malloc_type {
	int mt_id;
};

struct malloc_type kmock_M_TEMP = { 1 };
#define M_TEMP	(&kmock_M_TEMP)

struct thread {
	register_t td_retval[2];
};

struct linux_getcwd_args {
	char *buf;
	l_ulong bufsize;
};

/* ------------------------------------------------------------------ */
/* Observation trace.                                                  */
/* ------------------------------------------------------------------ */

#define KMOCK_TRACE_MAX		16

#define KMOCK_EV_MALLOC		1
#define KMOCK_EV_VNGETCWD	2
#define KMOCK_EV_COPYOUT	3
#define KMOCK_EV_FREE		4

struct kmock_event {
	int kind;
	unsigned long a;
	unsigned long b;
	unsigned long c;
	long r;
};

struct kmock_event kmock_trace[KMOCK_TRACE_MAX];
int kmock_trace_n;
int kmock_trace_overflow;
int kmock_live;			/* outstanding allocations */

/* ------------------------------------------------------------------ */
/* Kernel heap model.  A single fixed arena so that every pointer the  */
/* harness sees can be reported as an offset from kmock_arena.         */
/* ------------------------------------------------------------------ */

#define KMOCK_ARENA_SIZE	4352	/* LINUX_PATH_MAX + 256 slack */

char kmock_arena[KMOCK_ARENA_SIZE];
static size_t kmock_cap;		/* usable bytes of the live block */

/* ------------------------------------------------------------------ */
/* Knobs the harness turns.                                            */
/* ------------------------------------------------------------------ */

int kmock_vn_error;		/* what vn_getcwd() returns */
unsigned long kmock_vn_off;	/* selects the retbuf offset */
unsigned long kmock_vn_len;	/* selects the returned length */
unsigned char kmock_vn_fill;	/* path byte pattern seed */
int kmock_copyout_error;	/* what copyout() returns */
size_t kmock_user_size;		/* size of the harness user buffer */

static void
kmock_rec(int kind, unsigned long a, unsigned long b, unsigned long c, long r)
{

	if (kmock_trace_n < KMOCK_TRACE_MAX) {
		kmock_trace[kmock_trace_n].kind = kind;
		kmock_trace[kmock_trace_n].a = a;
		kmock_trace[kmock_trace_n].b = b;
		kmock_trace[kmock_trace_n].c = c;
		kmock_trace[kmock_trace_n].r = r;
		kmock_trace_n++;
	} else
		kmock_trace_overflow++;
}

void
kmock_reset(void)
{

	memset(kmock_trace, 0, sizeof(kmock_trace));
	kmock_trace_n = 0;
	kmock_trace_overflow = 0;
	kmock_live = 0;
	kmock_cap = 0;
	memset(kmock_arena, 0x5a, sizeof(kmock_arena));
}

void *
kmock_malloc(size_t size, struct malloc_type *type, int flags)
{
	size_t cap;

	cap = size;
	if (cap > sizeof(kmock_arena))
		cap = sizeof(kmock_arena);
	/* M_WAITOK never fails and never zeroes; use a fixed junk pattern. */
	memset(kmock_arena, 0xa5, sizeof(kmock_arena));
	kmock_cap = cap;
	kmock_live++;
	kmock_rec(KMOCK_EV_MALLOC, (unsigned long)size, (unsigned long)cap,
	    (unsigned long)flags, (long)(type == &kmock_M_TEMP));
	return (kmock_arena);
}

void
kmock_free(void *addr, struct malloc_type *type)
{
	unsigned long off;

	off = (unsigned long)((char *)addr - kmock_arena);
	kmock_live--;
	kmock_rec(KMOCK_EV_FREE, off, 0, 0,
	    (long)(type == &kmock_M_TEMP));
}

/*
 * Model of vn_getcwd().  On success it fills the block with a deterministic
 * path pattern, points *retbuf somewhere inside it and reports the resulting
 * length in *buflen.  On failure it reports the error; *buflen is left alone
 * exactly as the real routine leaves it, and *retbuf is pointed at the block
 * so that a mutated port which ignores the error still reads defined memory.
 */
int
vn_getcwd(char *buf, char **retbuf, size_t *buflen)
{
	size_t in, eff, off, out, i;
	unsigned long boff;

	in = *buflen;
	boff = (unsigned long)(buf - kmock_arena);

	eff = in;
	if (eff > kmock_cap)
		eff = kmock_cap;
	for (i = 0; i < eff; i++)
		buf[i] = (char)(kmock_vn_fill + (unsigned char)(i * 7u));
	if (eff > 0)
		buf[eff - 1] = '\0';

	if (kmock_vn_error != 0) {
		*retbuf = buf;
		kmock_rec(KMOCK_EV_VNGETCWD, (unsigned long)in,
		    (unsigned long)*buflen, boff, (long)kmock_vn_error);
		return (kmock_vn_error);
	}

	off = (eff == 0) ? 0 : (size_t)(kmock_vn_off % eff);
	out = (size_t)(kmock_vn_len % (eff - off + 1));
	*retbuf = buf + off;
	*buflen = out;
	kmock_rec(KMOCK_EV_VNGETCWD, (unsigned long)in, (unsigned long)out,
	    boff + (unsigned long)off, 0);
	return (0);
}

/*
 * Model of copyout().  The requested length is what goes into the trace; the
 * actual memcpy is clamped to the arena and to the harness user buffer so a
 * mutated port cannot run off the end before the harness can report it.
 */
int
copyout(const void *kaddr, void *udaddr, size_t len)
{
	unsigned long koff;
	size_t n;

	koff = (unsigned long)((const char *)kaddr - kmock_arena);
	if (kmock_copyout_error != 0) {
		kmock_rec(KMOCK_EV_COPYOUT, koff, (unsigned long)len, 0,
		    (long)kmock_copyout_error);
		return (kmock_copyout_error);
	}
	n = len;
	if (koff > sizeof(kmock_arena))
		n = 0;
	else if (n > sizeof(kmock_arena) - koff)
		n = sizeof(kmock_arena) - koff;
	if (n > kmock_user_size)
		n = kmock_user_size;
	if (n > 0)
		memcpy(udaddr, (const char *)kaddr, n);
	kmock_rec(KMOCK_EV_COPYOUT, koff, (unsigned long)len,
	    (unsigned long)n, 0);
	return (0);
}

/*
 * The kernel malloc()/free() take a type and flags; map the three/two
 * argument spellings onto the models above so the ported body below can be
 * copied across verbatim.
 */
#define malloc(sz, type, flags)		kmock_malloc((sz), (type), (flags))
#define free(addr, type)		kmock_free((addr), (type))

/* ================================================================== */
/* hbsd/src/sys/compat/linux/linux_getcwd.c                            */
/* ================================================================== */

/* $OpenBSD: linux_getcwd.c,v 1.2 2001/05/16 12:50:21 ho Exp $ */
/* $NetBSD: vfs_getcwd.c,v 1.3.2.3 1999/07/11 10:24:09 sommerfeld Exp $ */
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1999 The NetBSD Foundation, Inc.
 * Copyright (c) 2015 The FreeBSD Foundation
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Bill Sommerfeld.
 *
 * Portions of this software were developed by Edward Tomasz Napierala
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
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Find pathname of process's current directory.
 */
int
ref_linux_getcwd(struct thread *td, struct linux_getcwd_args *uap)
{
	char *buf, *retbuf;
	size_t buflen;
	int error;

	buflen = uap->bufsize;
	if (__predict_false(buflen < 2))
		return (ERANGE);
	if (buflen > LINUX_PATH_MAX)
		buflen = LINUX_PATH_MAX;

	buf = malloc(buflen, M_TEMP, M_WAITOK);
	error = vn_getcwd(buf, &retbuf, &buflen);
	if (error == ENOMEM)
		error = ERANGE;
	if (error == 0) {
		error = copyout(retbuf, uap->buf, buflen);
		if (error == 0)
			td->td_retval[0] = buflen;
	}
	free(buf, M_TEMP);
	return (error);
}
