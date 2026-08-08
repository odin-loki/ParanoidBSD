/*
 * b0244 oracle -- the specification.
 *
 * hbsd/src/lib/libc/stdio/fcloseall.c, getline.c, dprintf.c, and fwide.c
 * concatenated, with every function renamed with a `ref_' prefix.  Function
 * bodies are UNMODIFIED.  Only defines/declarations that the unavailable
 * FreeBSD/HardenedBSD headers used to supply have been added.
 */

#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef _STDFILE_DECLARED
#define _STDFILE_DECLARED
struct __sFILE {
	int	_orientation;
};
typedef struct __sFILE FILE;
#endif

int	__isthreaded;

/* Harness-visible mock state */
int	mock_fwalk_calls;
void	*mock_fwalk_fn;
int	mock_fclose_calls;

ssize_t	mock_getdelim_ret;
int	mock_getdelim_last_delim;
char	**mock_getdelim_last_linep;
size_t	*mock_getdelim_last_linecapp;
FILE	*mock_getdelim_last_fp;
size_t	mock_getdelim_set_cap;
size_t	mock_getdelim_write_len;
unsigned char mock_getdelim_write_buf[256];

int	mock_vdprintf_ret;
int	mock_vdprintf_last_fd;
const char *mock_vdprintf_last_fmt;
int	mock_vdprintf_last_arg;

int	mock_flock_calls;
int	mock_funlock_calls;

void
_flockfile(void *fp)
{
	(void)fp;
	mock_flock_calls++;
}

void
_funlockfile(void *fp)
{
	(void)fp;
	mock_funlock_calls++;
}

#define	FLOCKFILE(fp)		if (__isthreaded) _flockfile(fp)
#define	FUNLOCKFILE(fp)		if (__isthreaded) _funlockfile(fp)

int
_fwalk(int (*function)(FILE *))
{
	mock_fwalk_calls++;
	mock_fwalk_fn = (void *)function;
	return (0);
}

int
fclose(FILE *fp)
{
	(void)fp;
	mock_fclose_calls++;
	return (0);
}

ssize_t
getdelim(char **linep, size_t *linecapp, int delim, FILE *fp)
{
	mock_getdelim_last_delim = delim;
	mock_getdelim_last_linep = linep;
	mock_getdelim_last_linecapp = linecapp;
	mock_getdelim_last_fp = fp;
	if (linecapp != NULL && mock_getdelim_set_cap != 0)
		*linecapp = mock_getdelim_set_cap;
	if (linep != NULL && *linep != NULL && mock_getdelim_write_len != 0) {
		size_t n = mock_getdelim_write_len;
		if (n > sizeof(mock_getdelim_write_buf))
			n = sizeof(mock_getdelim_write_buf);
		memcpy(*linep, mock_getdelim_write_buf, n);
	}
	return (mock_getdelim_ret);
}

int
vdprintf(int fd, const char *fmt, va_list ap)
{
	mock_vdprintf_last_fd = fd;
	mock_vdprintf_last_fmt = fmt;
	mock_vdprintf_last_arg = va_arg(ap, int);
	return (mock_vdprintf_ret);
}

/* ======================= fcloseall.c ======================= */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (C) 2006 Daniel M. Eischen.  All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* #include <stdio.h>			-- unavailable, substituted above */
/* #include "local.h"			-- unavailable, substituted above */

#define __weak_reference(sym, alias)

void
ref___fcloseall(void)
{

	(void)_fwalk(fclose);
}

/* ======================= getline.c ======================= */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2009 David Schultz <das@FreeBSD.org>
 * All rights reserved.
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

/* #include <stdio.h>			-- unavailable, substituted above */

ssize_t
ref_getline(char ** __restrict linep, size_t * __restrict linecapp,
	FILE * __restrict fp)
{

	return (getdelim(linep, linecapp, '\n', fp));
}

/* ======================= dprintf.c ======================= */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2009 David Schultz <das@FreeBSD.org>
 * All rights reserved.
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

/* #include "namespace.h"		-- unavailable, substituted above */
/* #include <stdarg.h>			-- unavailable, substituted above */
/* #include <stdio.h>			-- unavailable, substituted above */
/* #include "un-namespace.h"		-- unavailable, substituted above */

int
ref_dprintf(int fd, const char * __restrict fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = vdprintf(fd, fmt, ap);
	va_end(ap);
	return (ret);
}

/* ======================= fwide.c ======================= */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Tim J. Robbins.
 * All rights reserved.
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

/* #include "namespace.h"		-- unavailable, substituted above */
/* #include <errno.h>			-- unavailable, substituted above */
/* #include <stdio.h>			-- unavailable, substituted above */
/* #include <wchar.h>			-- unavailable, substituted above */
/* #include "un-namespace.h"		-- unavailable, substituted above */
/* #include "libc_private.h"		-- unavailable, substituted above */
/* #include "local.h"			-- unavailable, substituted above */

int
ref_fwide(FILE *fp, int mode)
{
	int m;

	FLOCKFILE(fp);
	/* Only change the orientation if the stream is not oriented yet. */
	if (mode != 0 && fp->_orientation == 0)
		fp->_orientation = mode > 0 ? 1 : -1;
	m = fp->_orientation;
	FUNLOCKFILE(fp);

	return (m);
}
