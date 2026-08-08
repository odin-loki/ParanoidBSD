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

/*
 * Copyright (c) 2017 Eric van Gyzen
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

module;

#define _POSIX_C_SOURCE 200809L
#define __SSP_FORTIFY_LEVEL 0

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <signal.h>
#include <time.h>

#ifdef ppoll
#undef ppoll
#endif

#ifndef __weak_symbol
#define	__weak_symbol	__attribute__((__weak__))
#endif

export module pbsd.lib.libc.sys.b0097;

#define	__ssp_real_(fun)	fun
#define	__ssp_real(fun)		__ssp_real_(fun)

namespace pbsd::lib_libc_sys::b0097 {

struct __wrusage;

using interpos_func_t = int (*)(void);

enum {
	INTERPOS_wait6,
	INTERPOS_sendto,
	INTERPOS_clock_nanosleep,
	INTERPOS_ppoll,
	INTERPOS_MAX
};

using interpos_sig_wait6 = pid_t (*)(idtype_t, id_t, int *, int,
    struct __wrusage *, siginfo_t *);
using interpos_sig_sendto = ssize_t (*)(int, const void *, size_t, int,
    const struct sockaddr *, socklen_t);
using interpos_sig_clock_nanosleep = int (*)(clockid_t, int,
    const struct timespec *, struct timespec *);
using interpos_sig_ppoll = int (*)(struct pollfd *, nfds_t,
    const struct timespec *, const sigset_t *);

} /* namespace pbsd::lib_libc_sys::b0097 */

#define	__libc_interposing_slot(interposno)				\
	(*(interpos_func_t *)&pbsd::lib_libc_sys::b0097::__libc_interposing[interposno])

#define	INTERPOS_SYS(syscall_name, ...)					\
	(((pbsd::lib_libc_sys::b0097::interpos_sig_ ## syscall_name)	\
	    __libc_interposing_slot(INTERPOS_ ## syscall_name))		\
	    (__VA_ARGS__))

export namespace pbsd::lib_libc_sys::b0097 {

interpos_func_t __libc_interposing[INTERPOS_MAX];

[[gnu::weak]] pid_t
wait6(idtype_t idtype, id_t id, int *status, int options,
    struct __wrusage *ru, siginfo_t *infop)
{
	return (INTERPOS_SYS(wait6, idtype, id, status, options, ru, infop));
}

[[gnu::weak]] ssize_t
sendto(int s, const void *msg, size_t len, int flags,
    const struct sockaddr *to, socklen_t tolen)
{
	return (INTERPOS_SYS(sendto, s, msg, len, flags, to, tolen));
}

[[gnu::weak]] int
clock_nanosleep(clockid_t clock_id, int flags, const struct timespec *rqtp,
    struct timespec *rmtp)
{
	return (INTERPOS_SYS(clock_nanosleep, clock_id, flags, rqtp, rmtp));
}

[[gnu::weak]] int __weak_symbol
__ssp_real(ppoll)(struct pollfd pfd[], nfds_t nfds,
    const struct timespec *__restrict timeout,
    const sigset_t *__restrict newsigmask)
{
	return (INTERPOS_SYS(ppoll, pfd, nfds, timeout, newsigmask));
}

/*
 * Test-only access to this module's interposing table.  Not part of any
 * ported function; libc fills the real table from _libc_init().
 */
void
set_interpos(int slot, interpos_func_t func)
{
	__libc_interposing[slot] = func;
}

interpos_func_t
get_interpos(int slot)
{
	return (__libc_interposing[slot]);
}

} /* namespace pbsd::lib_libc_sys::b0097 */
