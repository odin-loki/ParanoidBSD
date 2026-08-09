/*
 * Reference oracle: the original HardenedBSD sources for
 *	lib/libc/sys/creat.c
 *	lib/libc/sys/wait3.c
 *	lib/libc/sys/recv.c
 *	lib/libc/sys/waitpid.c
 * concatenated, with every function renamed with a ref_ prefix.  The function
 * bodies are unmodified.  libc_private.h is not available outside the libc
 * build, so the declarations it supplies (interpos_func_t, the INTERPOS_*
 * slot numbers, __libc_interposing, __libc_interposing_slot(), INTERPOS_SYS()
 * and the __sys_* raw syscall entry points) are supplied here instead.
 */

#define _POSIX_C_SOURCE 200809L
#define __SSP_FORTIFY_LEVEL 0

#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stddef.h>

#ifndef __weak_symbol
#define	__weak_symbol	__attribute__((__weak__))
#endif

#ifndef __weak_reference
#define	__weak_reference(sym, alias)
#endif

#define	__ssp_real_(fun)	fun
#define	__ssp_real(fun)		__ssp_real_(fun)
#define	ref___ssp_real_(fun)	ref_##fun
#define	ref___ssp_real(fun)	ref___ssp_real_(fun)

typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_openat,
	INTERPOS_wait4,
	INTERPOS_recvfrom,
	INTERPOS_MAX
};

typedef int (*interpos_sig_openat)(int, const char *, int, int);
typedef pid_t (*interpos_sig_wait4)(pid_t, int *, int, struct rusage *);
typedef ssize_t (*interpos_sig_recvfrom)(int, void *, size_t, int,
    struct sockaddr *, socklen_t *);

interpos_func_t ref___libc_interposing[INTERPOS_MAX];

#define	__libc_interposing_slot(interposno)				\
	(*(interpos_func_t *)&ref___libc_interposing[interposno])

#define	INTERPOS_SYS(syscall_name, ...)					\
	(((interpos_sig_ ## syscall_name)__libc_interposing_slot(	\
	    INTERPOS_ ## syscall_name))(__VA_ARGS__))

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

__weak_reference(ref___creat, creat);
__weak_reference(ref___creat, _creat);

int ref___creat(const char *path, mode_t mode);

#pragma weak ref___creat
int
ref___creat(const char *path, mode_t mode)
{
	return (INTERPOS_SYS(openat, AT_FDCWD, path,
	    O_WRONLY | O_CREAT | O_TRUNC, mode));
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

pid_t ref___wait3(int *, int, struct rusage *);

pid_t
ref___wait3(int *istat, int options, struct rusage *rup)
{
	return (INTERPOS_SYS(wait4, WAIT_ANY, istat, options, rup));
}

__weak_reference(ref___wait3, wait3);

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
ref___ssp_real(recv)(int s, void *buf, size_t len, int flags)
{
	/*
	 * POSIX says recv() shall be a cancellation point, so call the
	 * cancellation-enabled recvfrom() and not _recvfrom().
	 */
	return (INTERPOS_SYS(recvfrom, s, buf, len, flags, NULL, NULL));
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

pid_t ref___waitpid(pid_t, int *, int);

pid_t
ref___waitpid(pid_t pid, int *istat, int options)
{
	return (INTERPOS_SYS(wait4, pid, istat, options, NULL));
}

__weak_reference(ref___waitpid, waitpid);
__weak_reference(ref___waitpid, _waitpid);
