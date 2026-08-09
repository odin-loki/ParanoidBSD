/*
 * Oracle: original HardenedBSD C sources for batch b0020s2, concatenated.
 * Every function is renamed with a ref_ prefix; bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/sys/closefrom.c
 */

/* ---- hbsd/src/lib/libc/sys/closefrom.c ---- */

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

#include <sys/param.h>
#include <unistd.h>

/*
 * Defines/declarations that the in-tree libc build gets from <sys/param.h>
 * and "libc_private.h", supplied here so the oracle builds stand-alone.
 */
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif

extern int __sys_close_range(unsigned int lowfd, unsigned int highfd,
    int flags);

void ref_closefrom(int lowfd);

void
ref_closefrom(int lowfd)
{
	__sys_close_range(MAX(0, lowfd), ~0U, 0);
}
