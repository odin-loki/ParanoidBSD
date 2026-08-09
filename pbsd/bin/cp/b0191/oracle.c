/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1991, 1993, 1994
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
 * PBSD batch b0191 oracle.
 *
 * hbsd/src/bin/cp/utils.c and hbsd/src/bin/cp/cp.c, concatenated.  Every
 * function carried over is renamed with a "ref_" prefix; the "static"
 * storage class is dropped so the differential harness can link against
 * them.  Function bodies are byte-for-byte the originals.
 *
 * Functions of these two files that are not present here could not be
 * compiled on this platform at all (they depend on bin/cp/extern.h, which
 * is not part of this batch, and/or on FreeBSD-only kernel interfaces);
 * see skipped.txt.
 */

#define _GNU_SOURCE

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

/* Supplied by <sys/param.h> on FreeBSD; missing on this platform. */
#ifndef MAXPHYS
#define MAXPHYS (128 * 1024)
#endif
#ifndef __unused
#define __unused __attribute__((__unused__))
#endif

/* ---------------------------------------------------------------- */
/* bin/cp/utils.c                                                     */
/* ---------------------------------------------------------------- */

#define	cp_pct(x, y)	((y == 0) ? 0 : (int)(100.0 * (x) / (y)))

/*
 * Memory strategy threshold, in pages: if physmem is larger then this, use a 
 * large buffer.
 */
#define PHYSPAGES_THRESHOLD (32*1024)

/* Maximum buffer size in bytes - do not allow it to grow larger than this. */
#define BUFSIZE_MAX (2*1024*1024)

/*
 * Small (default) buffer size in bytes. It's inefficient for this to be
 * smaller than MAXPHYS.
 */
#define BUFSIZE_SMALL (MAXPHYS)

/*
 * Prompt used in -i case.
 */
#define YESNO "(y/n [n]) "

ssize_t
ref_copy_fallback(int from_fd, int to_fd)
{
	static char *buf = NULL;
	static size_t bufsize;
	ssize_t rcount, wresid, wcount = 0;
	char *bufp;

	if (buf == NULL) {
		if (sysconf(_SC_PHYS_PAGES) > PHYSPAGES_THRESHOLD)
			bufsize = MIN(BUFSIZE_MAX, MAXPHYS * 8);
		else
			bufsize = BUFSIZE_SMALL;
		buf = malloc(bufsize);
		if (buf == NULL)
			err(1, "Not enough memory");
	}
	rcount = read(from_fd, buf, bufsize);
	if (rcount <= 0)
		return (rcount);
	for (bufp = buf, wresid = rcount; ; bufp += wcount, wresid -= wcount) {
		wcount = write(to_fd, bufp, wresid);
		if (wcount <= 0)
			break;
		if (wcount >= wresid)
			break;
	}
	return (wcount < 0 ? wcount : rcount);
}

void
ref_usage(void)
{

	(void)fprintf(stderr, "%s\n%s\n",
	    "usage: cp [-R [-H | -L | -P]] [-f | -i | -n] [-alpsvx] "
	    "source_file target_file",
	    "       cp [-R [-H | -L | -P]] [-f | -i | -n] [-alpsvx] "
	    "source_file ... "
	    "target_directory");
	exit(EX_USAGE);
}

/* ---------------------------------------------------------------- */
/* bin/cp/cp.c                                                        */
/*                                                                    */
/*-                                                                   */
/* SPDX-License-Identifier: BSD-3-Clause                              */
/*                                                                    */
/* Copyright (c) 1988, 1993, 1994                                     */
/*	The Regents of the University of California.  All rights reserved.*/
/*                                                                    */
/* This code is derived from software contributed to Berkeley by      */
/* David Hitz of Auspex Systems Inc.                                  */
/*                                                                    */
/* (Full BSD-3-Clause text as reproduced at the top of this file.)    */
/* ---------------------------------------------------------------- */

volatile sig_atomic_t info;

int
ref_ftscmp(const FTSENT * const *a, const FTSENT * const *b)
{
	return (strcmp((*a)->fts_name, (*b)->fts_name));
}

void
ref_siginfo(int sig __unused)
{

	info = 1;
}
