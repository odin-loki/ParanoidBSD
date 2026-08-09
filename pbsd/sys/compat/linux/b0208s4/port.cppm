/*
 * b0208s4 -- C++23 module port of hbsd/src/sys/compat/linux/linux_getcwd.c
 *
 * The port is deliberately literal: the same types, the same signedness, the
 * same evaluation order, the same calls to the same kernel primitives in the
 * same order.  The primitives (malloc/free, vn_getcwd, copyout) are the ones
 * supplied by the differential-test environment in oracle.c, so the port and
 * the reference exercise identical kernel behaviour and any divergence is the
 * port's own.
 */

module;

#include <stddef.h>
#include <errno.h>

export module pbsd.sys.compat.linux.b0208s4;

/*
 * The kernel primitives the body calls.  They carry C language linkage, so
 * they stay attached to the global module and resolve to the definitions the
 * differential-test environment provides.
 */
extern "C" {

/* sys/malloc.h */
struct malloc_type {
	int mt_id;
};
extern struct malloc_type kmock_M_TEMP;
void *kmock_malloc(size_t size, struct malloc_type *type, int flags);
void kmock_free(void *addr, struct malloc_type *type);

/* sys/vnode.h */
int vn_getcwd(char *buf, char **retbuf, size_t *buflen);

/* sys/systm.h */
int copyout(const void *kaddr, void *udaddr, size_t len);

} /* extern "C" */

#define LINUX_PATH_MAX	4096		/* compat/linux/linux.h */

#define M_NOWAIT	0x0001		/* sys/malloc.h */
#define M_WAITOK	0x0002

#define M_TEMP		(&kmock_M_TEMP)

#ifndef __predict_false
#define __predict_false(exp)	__builtin_expect((exp), 0)
#endif

/*
 * The kernel spellings of malloc()/free() take a malloc type and flags.  Map
 * them onto the environment's models so the ported body reads exactly as the
 * C original does.  The cast is the one concession C++ forces on us; it does
 * not change behaviour.
 */
#define malloc(sz, type, flags)	static_cast<char *>(kmock_malloc((sz), (type), (flags)))
#define free(addr, type)	kmock_free((addr), (type))

export namespace pbsd::sys_compat_linux::b0208s4 {

using l_ulong = unsigned long;		/* linux ABI unsigned long */
using register_t = long;		/* machine/types.h */

struct thread {
	register_t td_retval[2];
};

struct linux_getcwd_args {
	char *buf;
	l_ulong bufsize;
};

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
linux_getcwd(struct thread *td, struct linux_getcwd_args *uap)
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

} /* namespace pbsd::sys_compat_linux::b0208s4 */
