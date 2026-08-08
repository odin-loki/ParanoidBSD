/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Olson.
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
 * b0155s2 oracle -- the specification.
 *
 * Sources: hbsd/src/lib/libc/db/btree/bt_open.c
 *
 * Every function carries a ref_ prefix and the `static' storage class has been
 * dropped so the harness can link against it; no function body has been
 * altered in any other way.  Defines that the libc build supplies from private
 * headers are added below: _GNU_SOURCE for secure_getenv() and mkostemp(), and
 * __libc_sigprocmask, libc's internal non-cancellable alias of sigprocmask().
 *
 * __bt_open(), nroot() and __bt_fd() are not present; see skipped.txt.
 */

#define _GNU_SOURCE

#include <sys/param.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef LONG_BIT
#define	LONG_BIT	(sizeof(long) * 8)
#endif

#ifndef __libc_sigprocmask
#define __libc_sigprocmask sigprocmask
#endif

int ref_tmp(void);
int ref_byteorder(void);

int
ref_tmp(void)
{
	sigset_t set, oset;
	int fd, len;
	char *envtmp;
	char path[MAXPATHLEN];

	envtmp = secure_getenv("TMPDIR");
	len = snprintf(path,
	    sizeof(path), "%s/bt.XXXXXXXXXX", envtmp ? envtmp : "/tmp");
	if (len < 0 || len >= (int)sizeof(path)) {
		errno = ENAMETOOLONG;
		return(-1);
	}

	(void)sigfillset(&set);
	(void)__libc_sigprocmask(SIG_BLOCK, &set, &oset);
	if ((fd = mkostemp(path, O_CLOEXEC)) != -1)
		(void)unlink(path);
	(void)__libc_sigprocmask(SIG_SETMASK, &oset, NULL);
	return(fd);
}

int
ref_byteorder(void)
{
	u_int32_t x;
	u_char *p;

	x = 0x01020304;
	p = (u_char *)&x;
	switch (*p) {
	case 1:
		return (BIG_ENDIAN);
	case 4:
		return (LITTLE_ENDIAN);
	default:
		return (0);
	}
}
