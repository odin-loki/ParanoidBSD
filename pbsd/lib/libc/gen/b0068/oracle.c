/*
 * oracle.c -- reference implementation for batch b0068.
 *
 * The original HardenedBSD C sources, concatenated, with every function
 * renamed with a `ref_' prefix.  Function bodies are UNMODIFIED.  Only
 * declarations that FreeBSD supplies through private headers which do not
 * exist on this host (libc_private.h, gen-private.h, telldir.h,
 * namespace.h) have been replaced by the equivalent extern declaration,
 * and the two DTF_* constants from FreeBSD's <sys/dirent.h> have been
 * added.  This file is the specification.
 */

/*
 * ===========================================================================
 * lib/libc/gen/libc_dlopen.c
 * ===========================================================================
 */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2011 Xin LI <delphij@FreeBSD.org>
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

#include <dlfcn.h>
#include <stddef.h>
#include <unistd.h>

/* From "libc_private.h": the only declaration this file needs. */
extern void _rtld_error(const char *fmt, ...);

/*
 * Whether we want to restrict dlopen()s.
 */
static int __libc_restricted_mode = 0;

void *
ref_libc_dlopen(const char *path, int mode)
{

	if (__libc_restricted_mode) {
		_rtld_error("Service unavailable -- libc in restricted mode");
		return (NULL);
	}
	return (dlopen(path, mode));
}

void
ref___FreeBSD_libc_enter_restricted_mode(void)
{

	__libc_restricted_mode = 1;
}

/*
 * ===========================================================================
 * lib/libc/gen/uname.c
 * ===========================================================================
 */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1994
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

#define uname wrapped_uname
#include <sys/param.h>
/* <sys/sysctl.h> does not exist on this host; uname.c does not use it. */
#include <sys/utsname.h>
#include <errno.h>
#undef uname

/* From "libc_private.h". */
extern int __xuname(int, void *);

int ref_uname(struct utsname *);

int
ref_uname(struct utsname *name)
{
	return __xuname(32, name);
}

/*
 * ===========================================================================
 * lib/libc/gen/opendir.c
 * ===========================================================================
 */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1983, 1993
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

#include <dirent.h>

/* From FreeBSD <sys/dirent.h>, which <dirent.h> pulls in there. */
#ifndef DTF_HIDEW
#define	DTF_HIDEW	0x0001	/* hide whiteout entries */
#endif
#ifndef DTF_NODUP
#define	DTF_NODUP	0x0002	/* don't return duplicate names */
#endif

/* From "gen-private.h". */
extern DIR *__opendir2(const char *, int);

/*
 * Open a directory.
 */
DIR *
ref_opendir(const char *name)
{
	return (__opendir2(name, DTF_HIDEW | DTF_NODUP));
}
