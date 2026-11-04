/*
 * port.cppm -- PBSD batch b0020: pdwait, closefrom, sigwait, getdents.
 *
 * A faithful C++23 port of lib/libc/sys/{pdwait,closefrom,sigwait,getdents}.c.
 * Each original copyright header is retained immediately above the function it
 * covers.  Behaviour, argument order, integer signedness and the conversions
 * the C code performs implicitly are preserved exactly; nothing is improved.
 *
 * The originals bind their entry points weakly (`#pragma weak pdwait`,
 * `#pragma weak sigwait`) so that a threading library can interpose them at
 * link time.  A module-linkage C++ name cannot be interposed that way, so the
 * weak binding has no counterpart here; the dispatch through
 * __libc_interposing[] that the bodies perform is preserved unchanged.
 */

module;

#include <sys/types.h>
#include <signal.h>
#include <stddef.h>

/* <sys/param.h> */
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif

export module pbsd.lib.libc.sys.b0020;

/*
 * Everything the private FreeBSD headers would have declared.  The
 * linkage-specification attaches these declarations to the global module, so
 * the incomplete struct types below are the same entities as the identically
 * named ones the C oracle and the test harness declare for themselves.
 */
extern "C" {

/* <sys/procdesc.h>, <sys/signal.h>: only ever handled through a pointer */
struct __wrusage;
struct __siginfo;

int __sys_close_range(unsigned int lowfd, unsigned int highfd, int flags);
ssize_t __sys_getdirentries(int fd, char *buf, size_t nbytes, off_t *basep);
int __sys_sigwait(const sigset_t *set, int *sig);
int __sys_pdwait(int fd, int *status, int options, struct __wrusage *ru,
    struct __siginfo *infop);

/* libc_private.h */
typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_pdwait,
	INTERPOS_sigwait,
	INTERPOS_MAX
};

extern interpos_func_t __libc_interposing[INTERPOS_MAX];

}

#define	INTERPOS_SYS(syscall, ...)					\
	((reinterpret_cast<int (*)(decltype(&__sys_ ## syscall), ...)>(	\
	    __libc_interposing[INTERPOS_ ## syscall]))(&__sys_ ## syscall,	\
	    __VA_ARGS__))

export namespace pbsd::lib_libc_sys::b0020 {

/*
 * Copyright 2026 The FreeBSD Foundation.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This software was developed by Konstantin Belousov <kib@FreeBSD.org>
 * under sponsorship from the FreeBSD Foundation.
 */

int
pdwait(int fd, int *status, int options, struct __wrusage *ru,
    struct __siginfo *infop)
{
	return (INTERPOS_SYS(pdwait, fd, status, options, ru, infop));
}

/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Kyle Evans <kevans@FreeBSD.org>
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

void
closefrom(int lowfd)
{
	__sys_close_range(MAX(0, lowfd), ~0U, 0);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2010 davidxu@freebsd.org
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

int
sigwait(const sigset_t *set, int *sig)
{
	return (INTERPOS_SYS(sigwait, set, sig));
}

/*-
 * Copyright (c) 2012 Gleb Kurtsou <gleb@FreeBSD.org>
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

ssize_t
getdents(int fd, char *buf, size_t nbytes)
{

	return (__sys_getdirentries(fd, buf, nbytes, NULL));
}

}
