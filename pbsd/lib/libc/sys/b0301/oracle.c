/*
 * Reference oracle: the original HardenedBSD sources for
 *	lib/libc/sys/swapcontext.c
 *	lib/libc/sys/fcntl.c
 *	lib/libc/sys/wait.c
 *	lib/libc/sys/send.c
 * concatenated, with every function renamed with a ref_ prefix.  The function
 * bodies are unmodified.  libc_private.h is not available outside the libc
 * build, so the declarations it supplies (interpos_func_t, the INTERPOS_*
 * slot numbers, __libc_interposing, __libc_interposing_slot(), INTERPOS_SYS()
 * and the __sys_* raw syscall entry points) are supplied here instead.
 */

#define _POSIX_C_SOURCE 200809L
#define __SSP_FORTIFY_LEVEL 0

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/ucontext.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stddef.h>

#ifndef WAIT_ANY
#define	WAIT_ANY	(-1)
#endif

#ifndef __sym_compat
#define	__sym_compat(sym, impl, ver)
#endif

#ifndef __sym_default
#define	__sym_default(sym, impl, ver)
#endif

#ifndef __weak_symbol
#define	__weak_symbol	__attribute__((__weak__))
#endif

#ifndef __weak_reference
#define	__weak_reference(sym, alias)
#endif

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_swapcontext,
	INTERPOS_fcntl,
	INTERPOS_wait4,
	INTERPOS_sendto,
	INTERPOS_MAX
};

typedef int (*interpos_sig_swapcontext)(ucontext_t *, const ucontext_t *);
typedef int (*interpos_sig_fcntl)(int, int, long);
typedef pid_t (*interpos_sig_wait4)(pid_t, int *, int, struct rusage *);
typedef ssize_t (*interpos_sig_sendto)(int, const void *, size_t, int,
    const struct sockaddr *, socklen_t);

interpos_func_t ref___libc_interposing[INTERPOS_MAX];

#define	__libc_interposing_slot(interposno)				\
	(*(interpos_func_t *)&ref___libc_interposing[interposno])

#define	INTERPOS_SYS(syscall_name, ...)					\
	(((interpos_sig_ ## syscall_name)__libc_interposing_slot(	\
	    INTERPOS_ ## syscall_name))(__VA_ARGS__))

/*
 * Copyright (c) 2001 Daniel M. Eischen <deischen@freebsd.org>
 * Copyright (c) 2014 The FreeBSD Foundation.
 * All rights reserved.
 *
 * Portions of this software were developed by Konstantin Belousov
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
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

__sym_compat(ref_swapcontext, __impl_swapcontext, FBSD_1.0);
__weak_reference(ref_swapcontext, __impl_swapcontext);
__sym_default(ref_swapcontext, swapcontext, FBSD_1.2);

#pragma weak ref_swapcontext
int
ref_swapcontext(ucontext_t *oucp, const ucontext_t *ucp)
{
	return (INTERPOS_SYS(swapcontext, oucp, ucp));
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2008 Isilon Inc http://www.isilon.com/
 * Authors: Doug Rabson <dfr@rabson.org>
 * Developed with Red Inc: Alfred Perlstein <alfred@freebsd.org>
 *
 * Copyright (c) 2014-2015 The FreeBSD Foundation.
 *
 * Portions of this software were developed by Konstantin Belousov
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

#pragma weak ref_fcntl
int
ref_fcntl(int fd, int cmd, ...)
{
	va_list args;
	long arg;

	va_start(args, cmd);
	arg = va_arg(args, long);
	va_end(args);

	return (INTERPOS_SYS(fcntl, fd, cmd, arg));
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
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

pid_t ref___wait(int *);

pid_t
ref___wait(int *istat)
{
	return (INTERPOS_SYS(wait4, WAIT_ANY, istat, 0, NULL));
}

__weak_reference(ref___wait, wait);
__weak_reference(ref___wait, _wait);

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
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

ssize_t
ref_send(int s, const void *msg, size_t len, int flags)
{
	/*
	 * POSIX says send() shall be a cancellation point, so call the
	 * cancellation-enabled sendto() and not _sendto().
	 */
	return (INTERPOS_SYS(sendto, s, msg, len, flags, NULL, 0));
}
