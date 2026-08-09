/*
 * PBSD batch b0244 -- reference oracle.
 *
 * The original HardenedBSD sources for
 *     lib/libc/stdio/fcloseall.c
 *     lib/libc/stdio/getline.c
 *     lib/libc/stdio/dprintf.c
 *     lib/libc/stdio/fwide.c
 * concatenated, with every function renamed with a "ref_" prefix.  Function
 * bodies are byte-for-byte unmodified.  Only the declarations/defines that the
 * original private libc headers ("local.h", "namespace.h", "libc_private.h")
 * would have supplied are added below, because those headers are not part of
 * this batch.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <wchar.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

/*
 * FreeBSD's struct __sFILE carries the wide-orientation flag that fwide()
 * manipulates; glibc's FILE has no such member, so the single field the
 * function touches is modelled here.  The guard arrays and the lock counters
 * let the differential harness observe out-of-object writes and the
 * FLOCKFILE/FUNLOCKFILE pairing.  The identical declaration is used by the
 * port.
 */
struct __pbsd_sFILE {
	unsigned char _pbsd_guard_lo[8];
	int _orientation;
	int _pbsd_lockdepth;
	int _pbsd_lockseq;
	unsigned char _pbsd_guard_hi[8];
};

typedef struct __pbsd_sFILE b0244_FILE;
#define FILE b0244_FILE

/*
 * Shared mock substrate: both the oracle and the port link against these
 * definitions so differential tests compare only the batch function bodies.
 */

int __isthreaded = 0;

int mock_fwalk_calls = 0;
void *mock_fwalk_fn = NULL;
int mock_fclose_calls = 0;

ssize_t mock_getdelim_ret = 0;
int mock_getdelim_last_delim = 0;
char **mock_getdelim_last_linep = NULL;
size_t *mock_getdelim_last_linecapp = NULL;
FILE *mock_getdelim_last_fp = NULL;
size_t mock_getdelim_set_cap = 0;
size_t mock_getdelim_write_len = 0;
unsigned char mock_getdelim_write_buf[256];

int mock_vdprintf_ret = 0;
int mock_vdprintf_last_fd = -1;
const char *mock_vdprintf_last_fmt = NULL;
int mock_vdprintf_last_arg = 0;

int mock_flock_calls = 0;
int mock_funlock_calls = 0;

int
fclose(FILE *fp)
{
	(void)fp;
	mock_fclose_calls++;
	return (0);
}

int
_fwalk(int (*fn)(FILE *))
{
	mock_fwalk_calls++;
	mock_fwalk_fn = (void *)fn;
	return (0);
}

ssize_t
getdelim(char **linep, size_t *linecapp, int delim, FILE *fp)
{
	size_t wi, n;

	mock_getdelim_last_delim = delim;
	mock_getdelim_last_linep = linep;
	mock_getdelim_last_linecapp = linecapp;
	mock_getdelim_last_fp = fp;

	if (linecapp != NULL)
		*linecapp = mock_getdelim_set_cap;

	if (linep != NULL && *linep != NULL && mock_getdelim_write_len != 0) {
		n = mock_getdelim_write_len;
		if (n > sizeof(mock_getdelim_write_buf))
			n = sizeof(mock_getdelim_write_buf);
		for (wi = 0; wi < n; wi++)
			(*linep)[wi] = (char)mock_getdelim_write_buf[wi];
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

/* From "libc_private.h": lock only when the process is multi-threaded. */
#define	FLOCKFILE(fp)	do {						\
		if (__isthreaded) {						\
			mock_flock_calls++;					\
			(fp)->_pbsd_lockdepth++;				\
			(fp)->_pbsd_lockseq++;					\
		}								\
	} while (0)
#define	FUNLOCKFILE(fp)	do {						\
		if (__isthreaded) {						\
			mock_funlock_calls++;					\
			(fp)->_pbsd_lockdepth--;				\
			(fp)->_pbsd_lockseq++;					\
		}								\
	} while (0)

/* ====================================================================== */
/* lib/libc/stdio/fcloseall.c                                             */
/* ====================================================================== */

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

/* __weak_reference(__fcloseall, fcloseall); -- linker directive, not code. */

void
ref___fcloseall(void)
{

	(void)_fwalk(fclose);
}

/* ====================================================================== */
/* lib/libc/stdio/getline.c                                               */
/* ====================================================================== */

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

ssize_t
ref_getline(char ** __restrict linep, size_t * __restrict linecapp,
	FILE * __restrict fp)
{

	return (getdelim(linep, linecapp, '\n', fp));
}

/* ====================================================================== */
/* lib/libc/stdio/dprintf.c                                               */
/* ====================================================================== */

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

/* ====================================================================== */
/* lib/libc/stdio/fwide.c                                                 */
/* ====================================================================== */

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
