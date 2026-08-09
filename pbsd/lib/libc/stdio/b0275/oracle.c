/*
 * PBSD batch b0275 -- reference oracle.
 *
 * The original HardenedBSD sources for
 *     lib/libc/stdio/gets.c
 *     lib/libc/stdio/sprintf.c
 *     lib/libc/stdio/xprintf_vis.c
 * concatenated, with every function renamed with a "ref_" prefix.  Function
 * bodies are byte-for-byte unmodified.  Only the declarations/defines that the
 * original private libc headers ("local.h", "namespace.h", "libc_private.h",
 * "xlocale_private.h", "printf.h", "vis.h") would have supplied are added
 * below, because those headers are not part of this batch.
 */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wchar.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

/* ------------------------------------------------------------------ */
/* Minimal libc internals modelled for this batch.                     */
/* ------------------------------------------------------------------ */

struct __pbsd_sFILE {
	unsigned char _pbsd_guard_lo[8];
	int _orientation;
	int _pbsd_lockdepth;
	int _pbsd_lockseq;
	unsigned char _pbsd_guard_hi[8];
};

typedef struct __pbsd_sFILE b0275_FILE;
#define FILE b0275_FILE

typedef void *locale_t;

struct printf_info {
	int prec;
	int width;
	int pad;
	int showsign;
	int alt;
};

struct __printf_io {
	int _dummy;
};

#define PA_POINTER	5

#define VIS_OCTAL	0x01
#define VIS_CSTYLE	0x02
#define VIS_WHITE	0x04
#define VIS_HTTPSTYLE	0x40

int __isthreaded = 0;

b0275_FILE mock_stdin_storage;
FILE *stdin = &mock_stdin_storage;

int mock_flock_calls = 0;
int mock_funlock_calls = 0;

int mock_sgetc_calls = 0;
int mock_sgetc_vals[4096];
int mock_sgetc_len = 0;

ssize_t mock_write_ret = 0;
int mock_write_calls = 0;
int mock_write_last_fd = -1;
size_t mock_write_last_n = 0;
unsigned char mock_write_last_buf[256];

int mock_vsprintf_ret = 0;
char *mock_vsprintf_last_str = NULL;
const char *mock_vsprintf_last_fmt = NULL;
size_t mock_vsprintf_write_len = 0;
unsigned char mock_vsprintf_write_buf[512];

int mock_vsprintf_l_ret = 0;
char *mock_vsprintf_l_last_str = NULL;
locale_t mock_vsprintf_l_last_locale = NULL;
const char *mock_vsprintf_l_last_fmt = NULL;
size_t mock_vsprintf_l_write_len = 0;
unsigned char mock_vsprintf_l_write_buf[512];

locale_t mock_default_locale = (locale_t)(void *)0xB0275UL;

int mock_malloc_fail = 0;

int mock_strvisx_ret = 0;
char *mock_strvisx_last_dst = NULL;
const char *mock_strvisx_last_src = NULL;
unsigned mock_strvisx_last_len = 0;
int mock_strvisx_last_flags = 0;
int mock_strvisx_calls = 0;
int mock_strvisx_do_write = 1;

int mock_printf_out_ret = 0;
int mock_printf_out_calls = 0;
struct __printf_io *mock_printf_out_last_io = NULL;
const struct printf_info *mock_printf_out_last_pi = NULL;
const char *mock_printf_out_last_buf = NULL;
int mock_printf_out_last_len = 0;

int mock_printf_flush_calls = 0;
struct __printf_io *mock_printf_flush_last_io = NULL;

int
__sgetc(FILE *fp)
{
	int c;

	(void)fp;
	if (mock_sgetc_calls >= mock_sgetc_len)
		return (EOF);
	c = mock_sgetc_vals[mock_sgetc_calls++];
	return (c);
}

ssize_t
_write(int fd, const void *buf, size_t n)
{
	size_t i;

	mock_write_calls++;
	mock_write_last_fd = fd;
	mock_write_last_n = n;
	for (i = 0; i < n && i < sizeof(mock_write_last_buf); i++)
		mock_write_last_buf[i] = ((const unsigned char *)buf)[i];
	return (mock_write_ret);
}

int
vsprintf(char *str, const char *fmt, va_list ap)
{
	size_t i;

	(void)ap;
	mock_vsprintf_last_str = str;
	mock_vsprintf_last_fmt = fmt;
	if (str != NULL && mock_vsprintf_write_len != 0) {
		for (i = 0; i < mock_vsprintf_write_len &&
		    i < sizeof(mock_vsprintf_write_buf); i++)
			str[i] = (char)mock_vsprintf_write_buf[i];
	}
	return (mock_vsprintf_ret);
}

int
vsprintf_l(char *str, locale_t locale, const char *fmt, va_list ap)
{
	size_t i;

	(void)ap;
	mock_vsprintf_l_last_str = str;
	mock_vsprintf_l_last_locale = locale;
	mock_vsprintf_l_last_fmt = fmt;
	if (str != NULL && mock_vsprintf_l_write_len != 0) {
		for (i = 0; i < mock_vsprintf_l_write_len &&
		    i < sizeof(mock_vsprintf_l_write_buf); i++)
			str[i] = (char)mock_vsprintf_l_write_buf[i];
	}
	return (mock_vsprintf_l_ret);
}

locale_t
__get_locale(void)
{

	return (mock_default_locale);
}

#define	FIX_LOCALE(loc)							\
	do {								\
		if ((loc) == NULL)					\
			(loc) = __get_locale();				\
	} while (0)

int
strvisx(char *dst, const char *src, size_t len, int flags)
{
	size_t i;

	mock_strvisx_calls++;
	mock_strvisx_last_dst = dst;
	mock_strvisx_last_src = src;
	mock_strvisx_last_len = (unsigned)len;
	mock_strvisx_last_flags = flags;
	if (dst != NULL && mock_strvisx_do_write) {
		for (i = 0; i < len; i++)
			dst[i] = (char)(src[i] ^ (unsigned char)(flags & 0xff));
		dst[len] = '\0';
	}
	return (mock_strvisx_ret);
}

int
__printf_out(struct __printf_io *io, const struct printf_info *pi,
    const char *buf, int len)
{

	mock_printf_out_calls++;
	mock_printf_out_last_io = io;
	mock_printf_out_last_pi = pi;
	mock_printf_out_last_buf = buf;
	mock_printf_out_last_len = len;
	return (mock_printf_out_ret);
}

void
__printf_flush(struct __printf_io *io)
{

	mock_printf_flush_calls++;
	mock_printf_flush_last_io = io;
}

void *
__real_malloc(size_t nbytes);

void *
malloc(size_t nbytes)
{

	if (mock_malloc_fail)
		return (NULL);
	return (__real_malloc(nbytes));
}

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

#define	ORIENT(fp, direction)	do {					\
		(fp)->_orientation = (direction);				\
	} while (0)

#define	FLOCKFILE_CANCELSAFE(fp)	FLOCKFILE(fp)
#define	FUNLOCKFILE_CANCELSAFE()	FUNLOCKFILE(stdin)

/* ====================================================================== */
/* lib/libc/stdio/gets.c                                                  */
/* ====================================================================== */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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

char *
ref___gets_unsafe(char *buf)
{
	int c;
	char *s, *ret;
	static int warned;
	static const char w[] =
	    "warning: this program uses gets(), which is unsafe.\n";

	FLOCKFILE_CANCELSAFE(stdin);
	ORIENT(stdin, -1);
	if (!warned) {
		(void) _write(STDERR_FILENO, w, sizeof(w) - 1);
		warned = 1;
	}
	for (s = buf; (c = __sgetc(stdin)) != '\n'; ) {
		if (c == EOF) {
			if (s == buf) {
				ret = NULL;
				goto end;
			} else
				break;
		} else
			*s++ = c;
	}
	*s = 0;
	ret = buf;
end:
	FUNLOCKFILE_CANCELSAFE();
	return (ret);
}

/* __sym_compat(gets, __gets_unsafe, FBSD_1.0); -- linker directive, not code. */

/* ====================================================================== */
/* lib/libc/stdio/sprintf.c                                               */
/* ====================================================================== */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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

int
ref_sprintf(char * __restrict str, char const * __restrict fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vsprintf(str, fmt, ap);
	va_end(ap);
	return (ret);
}
int
ref_sprintf_l(char * __restrict str, locale_t locale, char const * __restrict fmt,
		...)
{
	int ret;
	va_list ap;
	FIX_LOCALE(locale);

	va_start(ap, fmt);
	ret = vsprintf_l(str, locale, fmt, ap);
	va_end(ap);
	return (ret);
}

/* ====================================================================== */
/* lib/libc/stdio/xprintf_vis.c                                           */
/* ====================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2005 Poul-Henning Kamp
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
ref___printf_arginfo_vis(const struct printf_info *pi, size_t n, int *argt)
{

	assert(n >= 1);
	argt[0] = PA_POINTER;
	return (1);
}

int
ref___printf_render_vis(struct __printf_io *io, const struct printf_info *pi, const void *const *arg)
{
	char *p, *buf;
	unsigned l;
	int ret;

	ret = 0;
	p = *((char **)arg[0]);
	if (p == NULL)
		return (__printf_out(io, pi, "(null)", 6));
	if (pi->prec >= 0)
		l = pi->prec;
	else
		l = strlen(p);
	buf = malloc(l * 4 + 1);
	if (buf == NULL)
		return (-1);
	if (pi->showsign)
		ret = strvisx(buf, p, l, VIS_WHITE | VIS_HTTPSTYLE);
	else if (pi->pad == '0')
		ret = strvisx(buf, p, l, VIS_WHITE | VIS_OCTAL);
	else if (pi->alt)
		ret = strvisx(buf, p, l, VIS_WHITE);
	else
		ret = strvisx(buf, p, l, VIS_WHITE | VIS_CSTYLE | VIS_OCTAL);
	ret += __printf_out(io, pi, buf, ret);
	__printf_flush(io);
	free(buf);
	return(ret);
}
