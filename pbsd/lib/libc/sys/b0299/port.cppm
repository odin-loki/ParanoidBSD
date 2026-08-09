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

module;

#define __SSP_FORTIFY_LEVEL 0

#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef __DECONST
#define	__DECONST(type, var)	((type)(uintptr_t)(const void *)(var))
#endif

export module pbsd.lib.libc.sys.b0299;

namespace pbsd::lib_libc_sys::b0299 {

using interpos_func_t = int (*)(void);

using __sys_fcntl_t = int (*)(int, int, intptr_t);

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

inline interpos_func_t *
__libc_interposing_slot(int interposno)
{

	return (&__libc_interposing[interposno]);
}

#define	_INTERPOS_SYS(type, idx, ...)				\
    ((type *)*(__libc_interposing_slot(idx)))(__VA_ARGS__)
#define	INTERPOS_SYS(syscall, ...)				\
    _INTERPOS_SYS(__sys_## syscall ##_t, INTERPOS_## syscall, __VA_ARGS__)

} /* namespace pbsd::lib_libc_sys::b0299 */

export namespace pbsd::lib_libc_sys::b0299 {

interpos_func_t __libc_interposing[INTERPOS_MAX];

extern "C" void *__sys_break(char *nsize);

static uintptr_t curbrk, minbrk;
static int curbrk_initted;

static int
initbrk(void)
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
mvbrk(void *addr)
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
lockf(int filedes, int function, off_t size)
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

int
brk(const void *addr)
{

	if (initbrk() == -1)
		return (-1);
	if ((uintptr_t)addr < minbrk)
		addr = (void *)minbrk;
	return (mvbrk(__DECONST(void *, addr)) == (void *)-1 ? -1 : 0);
}

int
_brk(const void *addr)
{

	if (initbrk() == -1)
		return (-1);
	return (mvbrk(__DECONST(void *, addr)) == (void *)-1 ? -1 : 0);
}

void *
sbrk(intptr_t incr)
{

	if (initbrk() == -1)
		return ((void *)-1);
	if ((incr > 0 && curbrk + incr < curbrk) ||
	    (incr < 0 && curbrk + incr > curbrk)) {
		/* Emulate legacy error handling in the syscall. */
		errno = EINVAL;
		return ((void *)-1);
	}
	return (mvbrk((void *)(curbrk + incr)));
}

} /* namespace pbsd::lib_libc_sys::b0299 */
