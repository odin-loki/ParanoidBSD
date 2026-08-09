/*
 * oracle.c -- reference implementation for batch b0247.
 *
 * Original C sources, concatenated, with every function renamed with a
 * "ref_" prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/gen/_thread_init.c
 *   hbsd/src/lib/libc/gen/__pthread_mutex_init_calloc_cb_stub.c
 *   hbsd/src/lib/libc/gen/cap_sandboxed.c
 */

#include <stddef.h>
#include <sys/types.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#ifndef __weak_reference
#define	__weak_reference(sym, alias)
#endif

/* libc_private.h is not available outside the libc build. */
#include <pthread.h>

typedef unsigned int u_int;
int cap_getmode(u_int *modep);

#include <assert.h>
#include <errno.h>
#include <stdbool.h>

/* ------------------------------------------------------------------ */
/* _thread_init.c                                                     */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2001 Daniel Eischen <deischen@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY DANIEL EISCHEN AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
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

void ref__thread_init_stub(void);

__weak_reference(ref__thread_init_stub, _thread_init);
__weak_reference(ref__thread_autoinit_dummy_decl_stub,
    _thread_autoinit_dummy_decl);

int	ref__thread_autoinit_dummy_decl_stub = 0;

void
ref__thread_init_stub(void)
{
	/* This is just a stub; there is nothing to do. */
}

/* ------------------------------------------------------------------ */
/* __pthread_mutex_init_calloc_cb_stub.c                              */
/* ------------------------------------------------------------------ */

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

int
ref__pthread_mutex_init_calloc_cb_stub(pthread_mutex_t *mutex,
    void *(calloc_cb)(size_t, size_t))
{

	return (0);
}

/* ------------------------------------------------------------------ */
/* cap_sandboxed.c                                                    */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2012 The FreeBSD Foundation
 *
 * This software was developed by Pawel Jakub Dawidek under sponsorship from
 * the FreeBSD Foundation.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

bool
ref_cap_sandboxed(void)
{
	u_int mode;

	if (cap_getmode(&mode) != 0) {
		assert(errno == ENOSYS);
		return (false);
	}
	assert(mode == 0 || mode == 1);
	return (mode == 1);
}
