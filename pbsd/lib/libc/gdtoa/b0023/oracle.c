/*
 * Reference oracle for PBSD batch b0023: the original HardenedBSD C sources,
 * concatenated, with every externally visible name given a "ref_" prefix.
 * Function bodies are unmodified.  Only the #define of _GNU_SOURCE (needed to
 * expose strtod_l/locale_t from glibc, the role gdtoaimp.h plays on
 * HardenedBSD) and the ref___gdtoa_locks_size() accessor used by the harness
 * have been added.
 */

#define _GNU_SOURCE 1

/* ===================================================================
 * hbsd/src/lib/libc/gdtoa/glue.c
 * =================================================================== */

/*
 * Machine-independent glue to integrate David Gay's gdtoa
 * package into libc.
 */

#include <pthread.h>

pthread_mutex_t ref___gdtoa_locks[] = {
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_MUTEX_INITIALIZER
};

/* ===================================================================
 * hbsd/src/lib/libc/gdtoa/machdep_ldisd.c
 * =================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2003 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Copyright (c) 2011 The FreeBSD Foundation
 *
 * Portions of this software were developed by David Chisnall
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

/*
 * Machine-dependent glue to integrate David Gay's gdtoa
 * package into libc for architectures where a long double
 * is the same as a double, such as the Alpha.
 */

#include <locale.h>
#include <stdlib.h>

long double
ref_strtold_l(const char * __restrict s, char ** __restrict sp, locale_t locale)
{

	return strtod_l(s, sp, locale);
}

/* ===================================================================
 * Harness accessor (not part of the original sources; touches no body).
 * =================================================================== */

#include <stddef.h>

size_t
ref___gdtoa_locks_size(void)
{

	return sizeof(ref___gdtoa_locks);
}

size_t
ref___gdtoa_locks_elemsize(void)
{

	return sizeof(ref___gdtoa_locks[0]);
}
