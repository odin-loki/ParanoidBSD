/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1989, 1993
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

module;

#define __SSP_FORTIFY_LEVEL 0

#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stddef.h>

#ifndef WAIT_ANY
#define	WAIT_ANY	(-1)
#endif

#ifndef __weak_symbol
#define	__weak_symbol	__attribute__((__weak__))
#endif

#define	__ssp_real_(fun)	fun
#define	__ssp_real(fun)		__ssp_real_(fun)

export module pbsd.lib.libc.sys.b0253;

namespace pbsd::lib_libc_sys::b0253 {

using interpos_func_t = int (*)(void);

enum {
	INTERPOS_openat,
	INTERPOS_wait4,
	INTERPOS_recvfrom,
	INTERPOS_MAX
};

using interpos_sig_openat = int (*)(int, const char *, int, int);
using interpos_sig_wait4 = pid_t (*)(pid_t, int *, int, struct rusage *);
using interpos_sig_recvfrom = ssize_t (*)(int, void *, size_t, int,
    struct sockaddr *, socklen_t *);

} /* namespace pbsd::lib_libc_sys::b0253 */

#define	__libc_interposing_slot(interposno)				\
	(*(interpos_func_t *)&pbsd::lib_libc_sys::b0253::__libc_interposing[interposno])

#define	INTERPOS_SYS(syscall_name, ...)					\
	(((pbsd::lib_libc_sys::b0253::interpos_sig_ ## syscall_name)	\
	    __libc_interposing_slot(INTERPOS_ ## syscall_name))		\
	    (__VA_ARGS__))

export namespace pbsd::lib_libc_sys::b0253 {

interpos_func_t __libc_interposing[INTERPOS_MAX];

int
__creat(const char *path, mode_t mode)
{
	return (INTERPOS_SYS(openat, AT_FDCWD, path,
	    O_WRONLY | O_CREAT | O_TRUNC, mode));
}

pid_t
__wait3(int *istat, int options, struct rusage *rup)
{
	return (INTERPOS_SYS(wait4, WAIT_ANY, istat, options, rup));
}

ssize_t __weak_symbol
__ssp_real(recv)(int s, void *buf, size_t len, int flags)
{
	/*
	 * POSIX says recv() shall be a cancellation point, so call the
	 * cancellation-enabled recvfrom() and not _recvfrom().
	 */
	return (INTERPOS_SYS(recvfrom, s, buf, len, flags, NULL, NULL));
}

pid_t
__waitpid(pid_t pid, int *istat, int options)
{
	return (INTERPOS_SYS(wait4, pid, istat, options, NULL));
}

} /* namespace pbsd::lib_libc_sys::b0253 */
