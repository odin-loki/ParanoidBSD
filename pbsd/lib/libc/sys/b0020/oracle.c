/*
 * oracle.c -- reference specification for PBSD batch b0020.
 *
 * The four HardenedBSD sources of this batch are concatenated below, in the
 * order pdwait.c, closefrom.c, sigwait.c, getdents.c.  Every function has been
 * renamed with a "ref_" prefix; the function bodies are otherwise byte-for-byte
 * unmodified.
 *
 * All four functions are thin dispatchers onto a kernel-facing primitive
 * (__sys_close_range, __sys_getdirentries) or onto the libc interposition
 * table (INTERPOS_SYS).  Neither <sys/param.h>, <sys/procdesc.h> nor the
 * private libc_private.h header exist off a FreeBSD source tree, so the
 * declarations, defines and primitives those headers would have supplied are
 * provided in the scaffolding section below.  The primitives record the
 * arguments they are handed and return a value programmed by the harness, so
 * that the observable behaviour of each dispatcher -- which primitive is
 * reached, with which arguments, in which order, what it does to the caller's
 * buffers and what it returns -- can be compared against the C++ port.
 *
 * Nothing in the scaffolding is part of the specification; the four ref_
 * functions are.
 */

/* ------------------------------------------------------------------ */
/* scaffolding: types, defines and primitives normally from libc/kernel */
/* ------------------------------------------------------------------ */

/* -std=c11 is strict ISO C; ask for the POSIX types the sources use */
#ifndef _GNU_SOURCE
#define	_GNU_SOURCE	1
#endif

#include <sys/types.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* <sys/param.h> */
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif

/* <sys/procdesc.h>, <sys/signal.h>: only ever handled through a pointer */
struct __wrusage;
struct __siginfo;

#define	PBSD_MAX_ARGS	8

#define	PBSD_TAG_NONE		0
#define	PBSD_TAG_CLOSE_RANGE	1
#define	PBSD_TAG_GETDIRENTRIES	2
#define	PBSD_TAG_SIGWAIT	3
#define	PBSD_TAG_PDWAIT		4
#define	PBSD_TAG_SYS_SIGWAIT	5
#define	PBSD_TAG_SYS_PDWAIT	6

struct pbsd_mock_state {
	int			ncalls;
	int			nargs;
	int			tag;
	unsigned long long	args[PBSD_MAX_ARGS];

	/* programmed by the harness before each call */
	long long		prog_ret;
	int			prog_out_int;
	unsigned int		prog_fill_seed;
	size_t			prog_fill_len;
};

struct pbsd_mock_state pbsd_mock;

void
pbsd_mock_reset(long long ret, int out_int, unsigned int fill_seed,
    size_t fill_len)
{
	memset(&pbsd_mock, 0, sizeof(pbsd_mock));
	pbsd_mock.prog_ret = ret;
	pbsd_mock.prog_out_int = out_int;
	pbsd_mock.prog_fill_seed = fill_seed;
	pbsd_mock.prog_fill_len = fill_len;
}

static void
pbsd_mock_enter(int tag, int nargs)
{
	pbsd_mock.ncalls++;
	pbsd_mock.tag = tag;
	pbsd_mock.nargs = nargs;
}

/*
 * The bytes the getdirentries primitive drops into the caller's buffer.  The
 * high bit is always set so that a written byte can never be mistaken for the
 * 0x7f guard byte the harness fills its buffers with.
 */
static unsigned char
pbsd_fill_byte(unsigned int seed, size_t i)
{
	return ((unsigned char)((seed + (unsigned int)i * 31u +
	    (unsigned int)(i >> 3)) | 0x80u));
}

int
__sys_close_range(unsigned int lowfd, unsigned int highfd, int flags)
{
	pbsd_mock_enter(PBSD_TAG_CLOSE_RANGE, 3);
	pbsd_mock.args[0] = (unsigned long long)lowfd;
	pbsd_mock.args[1] = (unsigned long long)highfd;
	pbsd_mock.args[2] = (unsigned long long)(long long)flags;
	return ((int)pbsd_mock.prog_ret);
}

ssize_t
__sys_getdirentries(int fd, char *buf, size_t nbytes, off_t *basep)
{
	size_t i, n;

	pbsd_mock_enter(PBSD_TAG_GETDIRENTRIES, 4);
	pbsd_mock.args[0] = (unsigned long long)(long long)fd;
	pbsd_mock.args[1] = (unsigned long long)(uintptr_t)buf;
	pbsd_mock.args[2] = (unsigned long long)nbytes;
	pbsd_mock.args[3] = (unsigned long long)(uintptr_t)basep;
	if (buf != NULL) {
		n = pbsd_mock.prog_fill_len < nbytes ?
		    pbsd_mock.prog_fill_len : nbytes;
		for (i = 0; i < n; i++)
			buf[i] = (char)pbsd_fill_byte(pbsd_mock.prog_fill_seed,
			    i);
	}
	if (basep != NULL)
		*basep = (off_t)pbsd_mock.prog_out_int;
	return ((ssize_t)pbsd_mock.prog_ret);
}

/*
 * The plain syscall stubs whose addresses INTERPOS_SYS() hands to the
 * interposer.  They are never invoked by the harness; only their identity is
 * observed.
 */
int
__sys_sigwait(const sigset_t *set, int *sig)
{
	pbsd_mock_enter(PBSD_TAG_SYS_SIGWAIT, 2);
	pbsd_mock.args[0] = (unsigned long long)(uintptr_t)set;
	pbsd_mock.args[1] = (unsigned long long)(uintptr_t)sig;
	return ((int)pbsd_mock.prog_ret);
}

int
__sys_pdwait(int fd, int *status, int options, struct __wrusage *ru,
    struct __siginfo *infop)
{
	pbsd_mock_enter(PBSD_TAG_SYS_PDWAIT, 5);
	pbsd_mock.args[0] = (unsigned long long)(long long)fd;
	pbsd_mock.args[1] = (unsigned long long)(uintptr_t)status;
	pbsd_mock.args[2] = (unsigned long long)(long long)options;
	pbsd_mock.args[3] = (unsigned long long)(uintptr_t)ru;
	pbsd_mock.args[4] = (unsigned long long)(uintptr_t)infop;
	return ((int)pbsd_mock.prog_ret);
}

/* libc_private.h: the interposition table and the dispatch macro */
typedef int (*interpos_func_t)(void);

enum {
	INTERPOS_pdwait,
	INTERPOS_sigwait,
	INTERPOS_MAX
};

extern interpos_func_t __libc_interposing[INTERPOS_MAX];

#define	INTERPOS_SYS(syscall, ...)					\
	(((int (*)(__typeof__(__sys_ ## syscall) *, ...))		\
	    __libc_interposing[INTERPOS_ ## syscall])(__sys_ ## syscall,	\
	    __VA_ARGS__))

static int
pbsd_interpos_sigwait(int (*sysfn)(const sigset_t *, int *),
    const sigset_t *set, int *sig)
{
	pbsd_mock_enter(PBSD_TAG_SIGWAIT, 3);
	pbsd_mock.args[0] = (unsigned long long)(uintptr_t)sysfn;
	pbsd_mock.args[1] = (unsigned long long)(uintptr_t)set;
	pbsd_mock.args[2] = (unsigned long long)(uintptr_t)sig;
	if (sig != NULL)
		*sig = pbsd_mock.prog_out_int;
	return ((int)pbsd_mock.prog_ret);
}

static int
pbsd_interpos_pdwait(int (*sysfn)(int, int *, int, struct __wrusage *,
    struct __siginfo *), int fd, int *status, int options,
    struct __wrusage *ru, struct __siginfo *infop)
{
	pbsd_mock_enter(PBSD_TAG_PDWAIT, 6);
	pbsd_mock.args[0] = (unsigned long long)(uintptr_t)sysfn;
	pbsd_mock.args[1] = (unsigned long long)(long long)fd;
	pbsd_mock.args[2] = (unsigned long long)(uintptr_t)status;
	pbsd_mock.args[3] = (unsigned long long)(long long)options;
	pbsd_mock.args[4] = (unsigned long long)(uintptr_t)ru;
	pbsd_mock.args[5] = (unsigned long long)(uintptr_t)infop;
	if (status != NULL)
		*status = pbsd_mock.prog_out_int;
	return ((int)pbsd_mock.prog_ret);
}

interpos_func_t __libc_interposing[INTERPOS_MAX] = {
	[INTERPOS_pdwait] = (interpos_func_t)pbsd_interpos_pdwait,
	[INTERPOS_sigwait] = (interpos_func_t)pbsd_interpos_sigwait,
};

/* ------------------------------------------------------------------ */
/* lib/libc/sys/pdwait.c                                              */
/* ------------------------------------------------------------------ */

/*
 * Copyright 2026 The FreeBSD Foundation.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This software was developed by Konstantin Belousov <kib@FreeBSD.org>
 * under sponsorship from the FreeBSD Foundation.
 */

#pragma weak ref_pdwait
int
ref_pdwait(int fd, int *status, int options, struct __wrusage *ru,
    struct __siginfo *infop)
{
	return (INTERPOS_SYS(pdwait, fd, status, options, ru, infop));
}

/* ------------------------------------------------------------------ */
/* lib/libc/sys/closefrom.c                                           */
/* ------------------------------------------------------------------ */

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
ref_closefrom(int lowfd)
{
	__sys_close_range(MAX(0, lowfd), ~0U, 0);
}

/* ------------------------------------------------------------------ */
/* lib/libc/sys/sigwait.c                                             */
/* ------------------------------------------------------------------ */

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

#pragma weak ref_sigwait
int
ref_sigwait(const sigset_t *set, int *sig)
{
	return (INTERPOS_SYS(sigwait, set, sig));
}

/* ------------------------------------------------------------------ */
/* lib/libc/sys/getdents.c                                            */
/* ------------------------------------------------------------------ */

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
ref_getdents(int fd, char *buf, size_t nbytes)
{

	return (__sys_getdirentries(fd, buf, nbytes, NULL));
}
