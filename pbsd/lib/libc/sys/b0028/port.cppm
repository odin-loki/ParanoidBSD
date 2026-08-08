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
#include <sys/uio.h>
#include <unistd.h>

export module pbsd.lib.libc.sys.b0028;

namespace pbsd::lib_libc_sys::b0028 {

using interpos_func_t = int (*)(void);

enum {
	INTERPOS_accept,
	INTERPOS_connect,
	INTERPOS_sendmsg,
	INTERPOS_writev,
	INTERPOS_MAX
};

using interpos_sig_accept = int (*)(int, struct sockaddr *, socklen_t *);
using interpos_sig_connect = int (*)(int, const struct sockaddr *, socklen_t);
using interpos_sig_sendmsg = ssize_t (*)(int, const struct msghdr *, int);
using interpos_sig_writev = ssize_t (*)(int, const struct iovec *, int);

} /* namespace pbsd::lib_libc_sys::b0028 */

#define	__libc_interposing_slot(interposno)				\
	(*(interpos_func_t *)&pbsd::lib_libc_sys::b0028::__libc_interposing[interposno])

#define	INTERPOS_SYS(syscall_name, ...)					\
	(((pbsd::lib_libc_sys::b0028::interpos_sig_ ## syscall_name)	\
	    __libc_interposing_slot(INTERPOS_ ## syscall_name))		\
	    (__VA_ARGS__))

export namespace pbsd::lib_libc_sys::b0028 {

interpos_func_t __libc_interposing[INTERPOS_MAX];

[[gnu::weak]] ssize_t
sendmsg(int s, const struct msghdr *msg, int flags)
{
	return (INTERPOS_SYS(sendmsg, s, msg, flags));
}

[[gnu::weak]] int
accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
	return (INTERPOS_SYS(accept, s, addr, addrlen));
}

[[gnu::weak]] int
connect(int s, const struct sockaddr *addr, socklen_t addrlen)
{
	return (INTERPOS_SYS(connect, s, addr, addrlen));
}

[[gnu::weak]] ssize_t
writev(int fd, const struct iovec *iov, int iovcnt)
{
	return (INTERPOS_SYS(writev, fd, iov, iovcnt));
}

} /* namespace pbsd::lib_libc_sys::b0028 */
