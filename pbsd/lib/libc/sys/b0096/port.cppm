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

module;

#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>
#include <time.h>
#if defined(__linux__)
#include <aio.h>
#else
#include <sys/aio.h>
#endif

export module pbsd.lib.libc.sys.b0096;

namespace pbsd::lib_libc_sys::b0096 {

using interpos_func_t = int (*)(void);

enum {
	INTERPOS_accept4,
	INTERPOS_aio_suspend,
	INTERPOS_pselect,
	INTERPOS_recvmsg,
	INTERPOS_MAX
};

using interpos_sig_accept4 = int (*)(int, struct sockaddr *, socklen_t *, int);
using interpos_sig_aio_suspend = int (*)(const struct aiocb * const[], int,
    const struct timespec *);
using interpos_sig_pselect = int (*)(int, fd_set *, fd_set *, fd_set *,
    const struct timespec *, const sigset_t *);
using interpos_sig_recvmsg = ssize_t (*)(int, struct msghdr *, int);

} /* namespace pbsd::lib_libc_sys::b0096 */

#define	__libc_interposing_slot(interposno)				\
	(*(interpos_func_t *)&pbsd::lib_libc_sys::b0096::__libc_interposing[interposno])

#define	INTERPOS_SYS(syscall_name, ...)					\
	(((pbsd::lib_libc_sys::b0096::interpos_sig_ ## syscall_name)	\
	    __libc_interposing_slot(INTERPOS_ ## syscall_name))		\
	    (__VA_ARGS__))

export namespace pbsd::lib_libc_sys::b0096 {

interpos_func_t __libc_interposing[INTERPOS_MAX];

[[gnu::weak]] ssize_t
recvmsg(int s, struct msghdr *msg, int flags)
{
	return (INTERPOS_SYS(recvmsg, s, msg, flags));
}

[[gnu::weak]] int
accept4(int s, struct sockaddr *addr, socklen_t *addrlen, int flags)
{
	return (INTERPOS_SYS(accept4, s, addr, addrlen, flags));
}

[[gnu::weak]] int
pselect(int n, fd_set *rs, fd_set *ws, fd_set *es, const struct timespec *t,
    const sigset_t *s)
{
	return (INTERPOS_SYS(pselect, n, rs, ws, es, t, s));
}

[[gnu::weak]] int
aio_suspend(const struct aiocb * const iocbs[], int niocb,
    const struct timespec *timeout)
{
	return (INTERPOS_SYS(aio_suspend, iocbs, niocb, timeout));
}

} /* namespace pbsd::lib_libc_sys::b0096 */
