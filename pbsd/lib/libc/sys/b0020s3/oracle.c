/*
 * b0020s3 oracle -- reference C implementations.
 *
 * Sources concatenated from HardenedBSD, every function renamed with a
 * ref_ prefix.  Function bodies are byte-for-byte unmodified.
 *
 *   hbsd/src/lib/libc/sys/sigwait.c
 */

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

/* -std=c11 is strict ISO C, under which <signal.h> hides sigset_t. */
#ifndef _POSIX_C_SOURCE
#define	_POSIX_C_SOURCE	200809L
#endif

#include <signal.h>

/*
 * "libc_private.h" is not part of this batch, so the definitions the body
 * below needs from it are supplied here.  INTERPOS_SYS() in libc expands to
 * an indirect call through libc's interposing table slot for the named
 * syscall; the slot is given the ref_ prefix alongside the function so that
 * the oracle owns its own table entry and can be driven independently of the
 * port's.  The function body itself is unmodified.
 */
typedef int (*ref_interpos_sigwait_t)(const sigset_t *, int *);

ref_interpos_sigwait_t ref_interpos_sigwait_slot;

#define	INTERPOS_SYS(syscall, ...)					\
	((*(ref_interpos_ ## syscall ## _slot))(__VA_ARGS__))

#pragma weak ref_sigwait
int
ref_sigwait(const sigset_t *set, int *sig)
{
	return (INTERPOS_SYS(sigwait, set, sig));
}
