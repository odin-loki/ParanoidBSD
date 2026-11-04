/*
 * Reference oracle: the original HardenedBSD sources for
 *	lib/libc/sys/lockf.c
 *	lib/libc/sys/brk.c
 * concatenated, with every function renamed with a ref_ prefix.  The function
 * bodies are unmodified.  libc_private.h is not available outside the libc
 * build, so the declarations it supplies (interpos_func_t, the INTERPOS_*
 * slot numbers, __libc_interposing, __libc_interposing_slot(), INTERPOS_SYS()
 * and the __sys_* raw syscall entry points) are supplied here instead.
 */

#define _POSIX_C_SOURCE 200809L
#define __SSP_FORTIFY_LEVEL 0

#include <sys/types.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef __DECONST
#define	__DECONST(type, var)	((type)(uintptr_t)(const void *)(var))
#endif

#ifndef F_ULOCK
#define	F_ULOCK		0
#define	F_LOCK		1
#define	F_TLOCK		2
#define	F_TEST		3
#endif

#ifndef F_UNLCK
#define	F_UNLCK		2
#define	F_WRLCK		1
#define	F_GETLK		5
#define	F_SETLK		6
#define	F_SETLKW	7
#endif

#ifndef SEEK_CUR
#define	SEEK_CUR	1
#endif

struct flock {
	short	l_type;
	short	l_whence;
	off_t	l_start;
	off_t	l_len;
	pid_t	l_pid;
	long	l_sysid;
};

typedef int (*interpos_func_t)(void);

typedef int (__sys_fcntl_t)(int, int, intptr_t);

enum {
	INTERPOS_accept,
	INTERPOS_accept4,
	INTERPOS_aio_suspend,
	INTERPOS_close,
	INTERPOS_connect,
	INTERPOS_fcntl,
	INTERPOS_fsync,
	INTERPOS_fork,
	INTERPOS_msync,
	INTERPOS_nanosleep,
	INTERPOS_openat,
	INTERPOS_poll,
	INTERPOS_pselect,
	INTERPOS_recvfrom,
	INTERPOS_recvmsg,
	INTERPOS_select,
	INTERPOS_sendmsg,
	INTERPOS_sendto,
	INTERPOS_setcontext,
	INTERPOS_sigaction,
	INTERPOS_sigprocmask,
	INTERPOS_sigsuspend,
	INTERPOS_sigwait,
	INTERPOS_sigtimedwait,
	INTERPOS_sigwaitinfo,
	INTERPOS_swapcontext,
	INTERPOS_system,
	INTERPOS_tcdrain,
	INTERPOS_read,
	INTERPOS_readv,
	INTERPOS_wait4,
	INTERPOS_write,
	INTERPOS_writev,
	INTERPOS__pthread_mutex_init_calloc_cb,
	INTERPOS_spinlock,
	INTERPOS_spinunlock,
	INTERPOS_kevent,
	INTERPOS_wait6,
	INTERPOS_ppoll,
	INTERPOS_map_stacks_exec,
	INTERPOS_fdatasync,
	INTERPOS_clock_nanosleep,
	INTERPOS__reserved0, /* was distribute_static_tls */
	INTERPOS_pdfork,
	INTERPOS_uexterr_gettext,
	INTERPOS_pdwait,
	INTERPOS_MAX
};

interpos_func_t ref___libc_interposing[INTERPOS_MAX];

interpos_func_t *
__libc_interposing_slot(int interposno)
{

	return (&ref___libc_interposing[interposno]);
}

#define	_INTERPOS_SYS(type, idx, ...)				\
    ((type *)*(__libc_interposing_slot(idx)))(__VA_ARGS__)
#define	INTERPOS_SYS(syscall, ...)				\
    _INTERPOS_SYS(__sys_## syscall ##_t, INTERPOS_## syscall,	\
    __VA_ARGS__)

void *__sys_break(char *nsize);

/*	$NetBSD: lockf.c,v 1.3 2008/04/28 20:22:59 martin Exp $	*/
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1997 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Klaus Klein.
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

int
ref_lockf(int filedes, int function, off_t size)
{
	struct flock fl;
	int cmd;

	fl.l_start = 0;
	fl.l_len = size;
	fl.l_whence = SEEK_CUR;

	switch (function) {
	case F_ULOCK:
		cmd = F_SETLK;
		fl.l_type = F_UNLCK;
		break;
	case F_LOCK:
		cmd = F_SETLKW;
		fl.l_type = F_WRLCK;
		break;
	case F_TLOCK:
		cmd = F_SETLK;
		fl.l_type = F_WRLCK;
		break;
	case F_TEST:
		fl.l_type = F_WRLCK;
		if (((int (*)(int, int, ...))
		    *(__libc_interposing_slot(INTERPOS_fcntl)))
		    (filedes, F_GETLK, &fl) == -1)
			return (-1);
		if (fl.l_type == F_UNLCK || (fl.l_sysid == 0 &&
		    fl.l_pid == getpid()))
			return (0);
		errno = EAGAIN;
		return (-1);
		/* NOTREACHED */
	default:
		errno = EINVAL;
		return (-1);
		/* NOTREACHED */
	}

	return (INTERPOS_SYS(fcntl, filedes, cmd, (intptr_t)&fl));
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2018 Mark Johnston <markj@FreeBSD.org>
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

static uintptr_t curbrk, minbrk;
static int curbrk_initted;

static int
ref_initbrk(void)
{
	void *newbrk;

	if (!curbrk_initted) {
		newbrk = __sys_break(NULL);
		if (newbrk == (void *)-1)
			return (-1);
		curbrk = minbrk = (uintptr_t)newbrk;
		curbrk_initted = 1;
	}
	return (0);
}

static void *
ref_mvbrk(void *addr)
{
	uintptr_t oldbrk;

	if ((uintptr_t)addr < minbrk) {
		/* Emulate legacy error handling in the syscall. */
		errno = EINVAL;
		return ((void *)-1);
	}
	if (__sys_break(addr) == (void *)-1)
		return ((void *)-1);
	oldbrk = curbrk;
	curbrk = (uintptr_t)addr;
	return ((void *)oldbrk);
}

int
ref_brk(const void *addr)
{

	if (ref_initbrk() == -1)
		return (-1);
	if ((uintptr_t)addr < minbrk)
		addr = (void *)minbrk;
	return (ref_mvbrk(__DECONST(void *, addr)) == (void *)-1 ? -1 : 0);
}

int
ref__brk(const void *addr)
{

	if (ref_initbrk() == -1)
		return (-1);
	return (ref_mvbrk(__DECONST(void *, addr)) == (void *)-1 ? -1 : 0);
}

void *
ref_sbrk(intptr_t incr)
{

	if (ref_initbrk() == -1)
		return ((void *)-1);
	if ((incr > 0 && curbrk + incr < curbrk) ||
	    (incr < 0 && curbrk + incr > curbrk)) {
		/* Emulate legacy error handling in the syscall. */
		errno = EINVAL;
		return ((void *)-1);
	}
	return (ref_mvbrk((void *)(curbrk + incr)));
}
