/*
 * Reference oracle: the original HardenedBSD sources for
 *	lib/libc/sys/setcontext.c
 *	lib/libc/sys/recvfrom.c
 *	lib/libc/sys/kevent.c
 *	lib/libc/sys/open.c
 * concatenated, with every function renamed with a ref_ prefix.  The function
 * bodies are unmodified.  libc_private.h is not available outside the libc
 * build, so the declarations it supplies (interpos_func_t, the INTERPOS_*
 * slot numbers, __libc_interposing, __libc_interposing_slot(), INTERPOS_SYS()
 * and the __sys_* raw syscall entry points) are supplied here instead.
 */

#define _POSIX_C_SOURCE 200809L
#define __SSP_FORTIFY_LEVEL 0

#include <sys/types.h>
#include <ucontext.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <stdarg.h>

#ifndef __weak_symbol
#define	__weak_symbol	__attribute__((__weak__))
#endif

#ifndef __weak_reference
#define	__weak_reference(sym, alias)
#endif

#ifndef __sym_compat
#define	__sym_compat(sym, impl, ver)
#endif

#ifndef __sym_default
#define	__sym_default(sym, impl, ver)
#endif

#define	__ssp_real_(fun)	fun
#define	__ssp_real(fun)		__ssp_real_(fun)
#define	ref___ssp_real_(fun)	ref_##fun
#define	ref___ssp_real(fun)	ref___ssp_real_(fun)

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_setcontext,
	INTERPOS_recvfrom,
	INTERPOS_kevent,
	INTERPOS_openat,
	INTERPOS_MAX
};

typedef int (*interpos_sig_setcontext)(const ucontext_t *);
typedef ssize_t (*interpos_sig_recvfrom)(int, void *, size_t, int,
    struct sockaddr *, socklen_t *);
typedef int (*interpos_sig_kevent)(int, const struct kevent *, int,
    struct kevent *, int, const struct timespec *);
typedef int (*interpos_sig_openat)(int, const char *, int, int);

interpos_func_t ref___libc_interposing[INTERPOS_MAX];

#define	__libc_interposing_slot(interposno)				\
	(*(interpos_func_t *)&ref___libc_interposing[interposno])

#define	INTERPOS_SYS(syscall_name, ...)					\
	(((interpos_sig_ ## syscall_name)__libc_interposing_slot(	\
	    INTERPOS_ ## syscall_name))(__VA_ARGS__))

/*
 * Copyright (c) 2014 The FreeBSD Foundation.
 *
 * Portions of this software were developed by Konstantin Belousov
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice(s), this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified other than the possible
 *    addition of one or more copyright notices.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice(s), this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
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

__sym_compat(setcontext, __impl_setcontext, FBSD_1.0);
__weak_reference(setcontext, __impl_setcontext);
__sym_default(setcontext, setcontext, FBSD_1.2);

#pragma weak ref_setcontext
int
ref_setcontext(const ucontext_t *uc)
{
	return (INTERPOS_SYS(setcontext, uc));
}

/*
 * Copyright (c) 2014 The FreeBSD Foundation.
 *
 * Portions of this software were developed by Konstantin Belousov
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice(s), this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified other than the possible
 *    addition of one or more copyright notices.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice(s), this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
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

ssize_t __weak_symbol
ref___ssp_real(recvfrom)(int s, void *buf, size_t len, int flags,
    struct sockaddr * __restrict from, socklen_t * __restrict fromlen)
{
	return (INTERPOS_SYS(recvfrom, s, buf, len, flags, from, fromlen));
}

/*
 * Copyright (c) 2015 The FreeBSD Foundation.
 *
 * Portions of this software were developed by Konstantin Belousov
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice(s), this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified other than the possible
 *    addition of one or more copyright notices.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice(s), this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
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

#pragma weak ref_kevent
int
ref_kevent(int kq, const struct kevent *changelist, int nchanges,
    struct kevent *eventlist, int nevents, const struct timespec *timeout)
{
	return (INTERPOS_SYS(kevent, kq, changelist, nchanges, eventlist,
	    nevents, timeout));
}

/*
 * Copyright (c) 2014 The FreeBSD Foundation.
 *
 * Portions of this software were developed by Konstantin Belousov
 * under sponsorship from the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice(s), this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified other than the possible
 *    addition of one or more copyright notices.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice(s), this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
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

#pragma weak ref_open
int
ref_open(const char *path, int flags, ...)
{
	va_list ap;
	int mode;

	if ((flags & O_CREAT) != 0) {
		va_start(ap, flags);
		mode = va_arg(ap, int);
		va_end(ap);
	} else {
		mode = 0;
	}
	return (INTERPOS_SYS(openat, AT_FDCWD, path, flags, mode));
}
