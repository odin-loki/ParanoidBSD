/*
 * oracle.c -- reference implementation for batch b0266.
 *
 * The original C sources, concatenated, with every function renamed with a
 * "ref_" prefix.  Function bodies are UNMODIFIED.  Only defines/declarations
 * that the unavailable FreeBSD/HardenedBSD headers used to supply have been
 * added.
 */

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#undef MAXPATHLEN
#define MAXPATHLEN 1024

#define	P_OSREL_TIDPID			1400079
#define	_PID_MAX			99999

#define	CPU_LEVEL_WHICH			3
#define	CPU_WHICH_TID			1
#define	CPU_WHICH_PID			2
#define	CPU_WHICH_TIDPID		3

typedef unsigned long cpuset_t;
typedef int cpuwhich_t;

int __getosreldate(void);
int cpuset_getaffinity(int level, int which, id_t id, size_t setsize,
    cpuset_t *cpuset);

/* ------------------------------------------------------------------------- */
/* hbsd/src/lib/libc/gen/sched_getaffinity.c */

/*-
 * Copyright (c) 2021 The FreeBSD Foundation
 *
 * This software were developed by Konstantin Belousov <kib@FreeBSD.org>
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

/* #define	_WANT_P_OSREL */
/* #include <sys/param.h> */
/* #include <errno.h> */
/* #include <sched.h> */
/* #include <string.h> */
/* #include "libc_private.h" */

int
ref_sched_getaffinity(pid_t pid, size_t cpusetsz, cpuset_t *cpuset)
{
	cpuwhich_t which;
	int error;

	if (__getosreldate() < P_OSREL_TIDPID) {
		if (pid == 0 || pid > _PID_MAX)
			which = CPU_WHICH_TID;
		else
			which = CPU_WHICH_PID;
	} else
		which = CPU_WHICH_TIDPID;

	error = cpuset_getaffinity(CPU_LEVEL_WHICH, which,
	    pid == 0 ? -1 : pid, cpusetsz, cpuset);
	if (error == -1 && errno == ERANGE)
		errno = EINVAL;
	return (error);
}

/* ------------------------------------------------------------------------- */
/* hbsd/src/lib/libc/gen/dirname_compat.c */

/*	$OpenBSD: dirname.c,v 1.13 2005/08/08 08:05:33 espie Exp $	*/

/*
 * Copyright (c) 1997, 2004 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* #include <errno.h> */
/* #include <libgen.h> */
/* #include <stdlib.h> */
/* #include <string.h> */
/* #include <sys/param.h> */

char * ref___freebsd11_dirname(char *path);

char *
ref___freebsd11_dirname(char *path)
{
	static char *dname = NULL;
	size_t len;
	const char *endp;

	if (dname == NULL) {
		dname = (char *)malloc(MAXPATHLEN);
		if (dname == NULL)
			return(NULL);
	}

	/* Empty or NULL string gets treated as "." */
	if (path == NULL || *path == '\0') {
		dname[0] = '.';
		dname[1] = '\0';
		return (dname);
	}

	/* Strip any trailing slashes */
	endp = path + strlen(path) - 1;
	while (endp > path && *endp == '/')
		endp--;

	/* Find the start of the dir */
	while (endp > path && *endp != '/')
		endp--;

	/* Either the dir is "/" or there are no slashes */
	if (endp == path) {
		dname[0] = *endp == '/' ? '/' : '.';
		dname[1] = '\0';
		return (dname);
	} else {
		/* Move forward past the separating slashes */
		do {
			endp--;
		} while (endp > path && *endp == '/');
	}

	len = endp - path + 1;
	if (len >= MAXPATHLEN) {
		errno = ENAMETOOLONG;
		return (NULL);
	}
	memcpy(dname, path, len);
	dname[len] = '\0';
	return (dname);
}

/* __sym_compat(dirname, __freebsd11_dirname, FBSD_1.0); -- not a function */
