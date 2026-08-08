/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1989, 1993, 1994
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

/*
 * Reference oracle for PBSD batch b0195: bin/chmod/chmod.c.
 *
 * The original function bodies are reproduced verbatim; only the names
 * carry the ref_ prefix.  main() and may_have_nfs4acl() are absent for
 * the reasons recorded in skipped.txt.
 */

#include <sys/param.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Defines missing outside FreeBSD. */
#ifndef __dead2
#define	__dead2		__attribute__((__noreturn__))
#endif
#ifndef __unused
#define	__unused	__attribute__((__unused__))
#endif
#ifndef LONG_BIT
#define	LONG_BIT	(sizeof(long) * CHAR_BIT)
#endif

static volatile sig_atomic_t siginfo;

static void ref_usage(void) __dead2;

static void
ref_siginfo_handler(int sig __unused)
{

	siginfo = 1;
}

static void
ref_usage(void)
{
	(void)fprintf(stderr,
	    "usage: chmod [-fhv] [-R [-H | -L | -P]] mode file ...\n");
	exit(1);
}

/*
 * Test-only entry points.  Both originals have internal linkage and
 * `siginfo' is file-scope static, so the differential harness needs a
 * way to reach them.  Nothing above is altered by these additions.
 */
void
ref_siginfo_handler_call(int sig)
{

	ref_siginfo_handler(sig);
}

void
ref_usage_call(void)
{

	ref_usage();
}

sig_atomic_t
ref_siginfo_get(void)
{

	return (siginfo);
}

void
ref_siginfo_set(sig_atomic_t v)
{

	siginfo = v;
}
