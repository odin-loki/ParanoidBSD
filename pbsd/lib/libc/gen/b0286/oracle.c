/*
 * oracle.c -- reference implementation for batch b0286.
 *
 * Original C sources, concatenated, with every function renamed with a
 * "ref_" prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/gen/confstr.c
 *   hbsd/src/lib/libc/gen/statvfs.c
 *   hbsd/src/lib/libc/gen/timezone.c
 *
 * Omitted (see skipped.txt):
 *   hbsd/src/lib/libc/gen/memfd_create.c
 */

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/param.h>
#include <paths.h>
#include <sys/statvfs.h>
#include <sys/vfs.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef MNT_RDONLY
#define MNT_RDONLY ST_RDONLY
#endif
#ifndef MNT_NOSUID
#define MNT_NOSUID ST_NOSUID
#endif

#ifndef f_iosize
#define f_iosize f_frsize
#endif

#define _fstatfs fstatfs
#define _fpathconf fpathconf

#define TZ_MAX_CHARS 255

static char *ref__tztab(int, int);

/* ------------------------------------------------------------------ */
/* confstr.c                                                          */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1993
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

size_t
ref_confstr(int name, char *buf, size_t len)
{
	const char *p;
	const char UPE[] = "unsupported programming environment";

	switch (name) {
	case _CS_PATH:
		p = _PATH_STDPATH;
		goto docopy;

		/*
		 * POSIX/SUS ``Programming Environments'' stuff
		 *
		 * We don't support more than one programming environment
		 * on any platform (yet), so we just return the empty
		 * string for the environment we are compiled for,
		 * and the string "unsupported programming environment"
		 * for anything else.  (The Standard says that if these
		 * values are used on a system which does not support
		 * this environment -- determined via sysconf() -- then
		 * the value we return is unspecified.  So, we return
		 * something which will cause obvious breakage.)
		 */
	case _CS_POSIX_V6_ILP32_OFF32_CFLAGS:
	case _CS_POSIX_V6_ILP32_OFF32_LDFLAGS:
	case _CS_POSIX_V6_ILP32_OFF32_LIBS:
	case _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS:
	case _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS:
	case _CS_POSIX_V6_LPBIG_OFFBIG_LIBS:
		/*
		 * These two environments are never supported.
		 */
		p = UPE;
		goto docopy;

	case _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS:
	case _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS:
	case _CS_POSIX_V6_ILP32_OFFBIG_LIBS:
		if (sizeof(long) * CHAR_BIT == 32 &&
		    sizeof(off_t) > sizeof(long))
			p = "";
		else
			p = UPE;
		goto docopy;

	case _CS_POSIX_V6_LP64_OFF64_CFLAGS:
	case _CS_POSIX_V6_LP64_OFF64_LDFLAGS:
	case _CS_POSIX_V6_LP64_OFF64_LIBS:
		if (sizeof(long) * CHAR_BIT >= 64 &&
		    sizeof(void *) * CHAR_BIT >= 64 &&
		    sizeof(int) * CHAR_BIT >= 32 &&
		    sizeof(off_t) >= sizeof(long))
			p = "";
		else
			p = UPE;
		goto docopy;

	case _CS_POSIX_V6_WIDTH_RESTRICTED_ENVS:
		/* XXX - should have more complete coverage */
		if (sizeof(long) * CHAR_BIT >= 64)
			p = "_POSIX_V6_LP64_OFF64";
		else
			p = "_POSIX_V6_ILP32_OFFBIG";
		goto docopy;

docopy:
		if (len != 0 && buf != NULL)
			strlcpy(buf, p, len);
		return (strlen(p) + 1);

	default:
		errno = EINVAL;
		return (0);
	}
	/* NOTREACHED */
}

/* ------------------------------------------------------------------ */
/* statvfs.c                                                          */
/* ------------------------------------------------------------------ */

/*
 * Copyright 2002 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that both the above copyright notice and this
 * permission notice appear in all copies, that both the above
 * copyright notice and this permission notice appear in all
 * supporting documentation, and that the name of M.I.T. not be used
 * in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  M.I.T. makes
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THIS SOFTWARE IS PROVIDED BY M.I.T. ``AS IS''.  M.I.T. DISCLAIMS
 * ALL EXPRESS OR IMPLIED WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT
 * SHALL M.I.T. BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

static int	ref_sfs2svfs(const struct statfs *from, struct statvfs *to);

int
ref_fstatvfs(int fd, struct statvfs *result)
{
	struct statfs sfs;
	int rv;
	long pcval;

	rv = _fstatfs(fd, &sfs);
	if (rv != 0)
		return (rv);

	rv = ref_sfs2svfs(&sfs, result);
	if (rv != 0)
		return (rv);

	/*
	 * Whether pathconf's -1 return means error or unlimited does not
	 * make any difference in this best-effort implementation.
	 */
	pcval = _fpathconf(fd, _PC_NAME_MAX);
	if (pcval == -1)
		result->f_namemax = ~0UL;
	else
		result->f_namemax = (unsigned long)pcval;
	return (0);
}

int
ref_statvfs(const char * __restrict path, struct statvfs * __restrict result)
{
	struct statfs sfs;
	int rv;
	long pcval;

	rv = statfs(path, &sfs);
	if (rv != 0)
		return (rv);

	ref_sfs2svfs(&sfs, result);

	/*
	 * Whether pathconf's -1 return means error or unlimited does not
	 * make any difference in this best-effort implementation.
	 */
	pcval = pathconf(path, _PC_NAME_MAX);
	if (pcval == -1)
		result->f_namemax = ~0UL;
	else
		result->f_namemax = (unsigned long)pcval;
	return (0);
}

static int
ref_sfs2svfs(const struct statfs *from, struct statvfs *to)
{
	static const struct statvfs zvfs;

	*to = zvfs;

	if (from->f_flags & MNT_RDONLY)
		to->f_flag |= ST_RDONLY;
	if (from->f_flags & MNT_NOSUID)
		to->f_flag |= ST_NOSUID;

	/* XXX should we clamp negative values? */
#define COPY(field) \
	do { \
		to->field = from->field; \
		if (from->field != to->field) { \
			errno = EOVERFLOW; \
			return (-1); \
		} \
	} while(0)

	COPY(f_bavail);
	COPY(f_bfree);
	COPY(f_blocks);
	COPY(f_ffree);
	COPY(f_files);
	to->f_bsize = from->f_iosize;
	to->f_frsize = from->f_bsize;
	to->f_favail = to->f_ffree;
	return (0);
}

/* ------------------------------------------------------------------ */
/* timezone.c                                                         */
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1987, 1993
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

static char	czone[TZ_MAX_CHARS];		/* space for zone name */

char *
ref___timezone_compat(int zone, int dst)
{
	char	*beg,
			*end;

	if ( (beg = getenv("TZNAME")) ) {	/* set in environment */
		if ((end = strchr(beg, ','))) {	/* "PST,PDT" */
			if (dst)
				return(++end);
			*end = '\0';
			(void)strncpy(czone,beg,sizeof(czone) - 1);
			czone[sizeof(czone) - 1] = '\0';
			*end = ',';
			return(czone);
		}
		return(beg);
	}
	return(ref__tztab(zone,dst));	/* default: table or created zone */
}

static struct zone {
	int	offset;
	char	*stdzone;
	char	*dlzone;
} zonetab[] = {
	{-1*60,	"MET",	"MET DST"},	/* Middle European */
	{-2*60,	"EET",	"EET DST"},	/* Eastern European */
	{4*60,	"AST",	"ADT"},		/* Atlantic */
	{5*60,	"EST",	"EDT"},		/* Eastern */
	{6*60,	"CST",	"CDT"},		/* Central */
	{7*60,	"MST",	"MDT"},		/* Mountain */
	{8*60,	"PST",	"PDT"},		/* Pacific */
#ifdef notdef
	/* there's no way to distinguish this from WET */
	{0,	"GMT",	0},		/* Greenwich */
#endif
	{0*60,	"WET",	"WET DST"},	/* Western European */
	{-10*60,"EST",	"EST"},		/* Aust: Eastern */
     {-10*60+30,"CST",	"CST"},		/* Aust: Central */
	{-8*60,	"WST",	0},		/* Aust: Western */
	{-1}
};

static char *
ref__tztab(int zone, int dst)
{
	struct zone	*zp;
	char	sign;

	for (zp = zonetab; zp->offset != -1;++zp)	/* static tables */
		if (zp->offset == zone) {
			if (dst && zp->dlzone)
				return(zp->dlzone);
			if (!dst && zp->stdzone)
				return(zp->stdzone);
		}

	if (zone < 0) {					/* create one */
		zone = -zone;
		sign = '+';
	}
	else
		sign = '-';
	(void)snprintf(czone, sizeof(czone),
	    "GMT%c%d:%02d",sign,zone / 60,zone % 60);
	return(czone);
}
