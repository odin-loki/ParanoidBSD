/*
 * Reference oracle for batch b0140.
 *
 * hbsd/src/lib/libc/stdio/putchar.c, vdprintf.c, xprintf_errno.c, and printf.c
 * concatenated with every function renamed with a ref_ prefix.  Function
 * bodies are UNMODIFIED.  Declarations from unavailable FreeBSD/HardenedBSD
 * private headers are supplied below.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if __has_include(<xlocale.h>)
#include <xlocale.h>
#else
#include <locale.h>
#endif

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef NL_TEXTMAX
#define NL_TEXTMAX 2048
#endif

/* ------------------------------------------------------------------------ */
/* vdprintf internals (custom FILE layout)                                   */
/* ------------------------------------------------------------------------ */

struct __sbuf {
	unsigned char *_base;
	int _size;
};

struct b0140_FILE {
	unsigned char *_p;
	int _r;
	int _w;
	short _flags;
	short _file;
	struct __sbuf _bf;
	int _lbfsize;
	void *_cookie;
	int (*_close)(void *);
	int (*_read)(void *, char *, int);
	long (*_seek)(void *, long, int);
	int (*_write)(void *, const char *, int);
	struct __sbuf _ub;
	unsigned char *_up;
	int _ur;
	unsigned char _ubuf[3];
	unsigned char _nbuf[1];
	struct __sbuf _lb;
	int _blksize;
	long _offset;
	void *_fl_mutex;
	void *_fl_owner;
	int _fl_count;
	int _orientation;
	mbstate_t _mbstate;
	int _flags2;
};

#define FILE b0140_FILE
#define FAKE_FILE { ._file = -1 }

#define	__SWR	0x0008
#define	__SERR	0x0040

static int
b0140_swrite(void *cookie, const char *buf, int n)
{
	FILE *fp = (FILE *)cookie;

	return ((int)write(fp->_file, buf, (size_t)n));
}

static int
b0140_fflush(FILE *fp)
{
	unsigned char *p;
	int n, t;

	if ((fp->_flags & __SWR) == 0)
		return (0);
	p = fp->_bf._base;
	if (p == NULL)
		return (0);
	n = (int)(fp->_p - p);
	if (n <= 0)
		return (0);
	fp->_p = p;
	fp->_w = fp->_bf._size;
	t = fp->_write(fp->_cookie, (const char *)p, n);
	if (t < 0) {
		fp->_flags |= __SERR;
		return (EOF);
	}
	return (0);
}

static void *
b0140_get_locale(void)
{

	return ((void *)0);
}

static int
b0140_vfprintf(FILE *fp, locale_t locale, int serrno, const char *fmt,
    va_list ap)
{
	char *whole;
	int n, i, ret;

	(void)locale;
	(void)serrno;

	n = vasprintf(&whole, fmt, ap);
	if (n < 0)
		return (n);
	ret = n;
	for (i = 0; i < n; ) {
		int room, chunk;

		room = fp->_w;
		if (room <= 0) {
			if (b0140_fflush(fp) != 0) {
				ret = EOF;
				break;
			}
			room = fp->_w;
			if (room <= 0) {
				ret = EOF;
				break;
			}
		}
		chunk = n - i;
		if (chunk > room)
			chunk = room;
		memcpy(fp->_p, whole + i, (size_t)chunk);
		fp->_p += chunk;
		fp->_w -= chunk;
		i += chunk;
	}
	free(whole);
	return (ret);
}

#define __swrite	b0140_swrite
#define __fflush	b0140_fflush
#define __get_locale	b0140_get_locale
#define __vfprintf	b0140_vfprintf

/* ======================= vdprintf.c ======================= */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2009 David Schultz <das@FreeBSD.org>
 * All rights reserved.
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
ref_vdprintf(int fd, const char * __restrict fmt, va_list ap)
{
	FILE f = FAKE_FILE;
	unsigned char buf[BUFSIZ];
	int serrno = errno;
	int ret;

	if (fd > SHRT_MAX) {
		errno = EMFILE;
		return (EOF);
	}

	f._p = buf;
	f._w = sizeof(buf);
	f._flags = __SWR;
	f._file = fd;
	f._cookie = &f;
	f._write = __swrite;
	f._bf._base = buf;
	f._bf._size = sizeof(buf);

	if ((ret = __vfprintf(&f, __get_locale(), serrno, fmt, ap)) < 0)
		return (ret);

	return (__fflush(&f) ? EOF : ret);
}

#undef FILE
#undef FAKE_FILE
#undef __swrite
#undef __fflush
#undef __get_locale
#undef __vfprintf

/* ------------------------------------------------------------------------ */
/* xprintf_errno internals                                                   */
/* ------------------------------------------------------------------------ */

#include <stdio.h>
#include <wchar.h>

struct printf_info {
	int		prec;
	int		width;
	wchar_t		spec;
	unsigned 	is_long_double;
	unsigned 	is_char;
	unsigned	is_short;
	unsigned	is_long;
	unsigned	alt;
	unsigned	space;
	unsigned	left;
	unsigned	showsign;
	unsigned	group;
	unsigned	extra;
	unsigned	wide;
	wchar_t		pad;
	unsigned	is_quad;
	unsigned	is_intmax;
	unsigned	is_ptrdiff;
	unsigned	is_size;
	int		sofar;
	unsigned	get_width;
	unsigned	get_prec;
	const char	*begin;
	const char	*end;
	void 		*arg[2];
};

enum {
	PA_INT		= (1 << 0),
};

struct __printf_io {
	unsigned char	*out;
	size_t		out_cap;
	size_t		out_len;
	int		flush_count;
};

static const int b0140_sys_nerr = 134;
#define __hidden_sys_nerr b0140_sys_nerr

static int
b0140_printf_pad(struct __printf_io *io, int howmany, int zero)
{
	unsigned char fill;
	int n, ret = 0;

	if (howmany <= 0)
		return (0);
	fill = zero ? (unsigned char)'0' : (unsigned char)' ';
	for (n = 0; n < howmany; n++) {
		if (io->out_len < io->out_cap) {
			io->out[io->out_len] = fill;
			io->out_len++;
		}
		ret++;
	}
	return (ret);
}

void
__printf_flush(struct __printf_io *io)
{

	io->flush_count++;
}

int
__printf_out(struct __printf_io *io, const struct printf_info *pi,
    const void *ptr, int len)
{
	int ret = 0;
	size_t n;

	if ((!pi->left) && pi->width > len)
		ret += b0140_printf_pad(io, pi->width - len, pi->pad == '0');
	n = (size_t)len;
	if (n > io->out_cap - io->out_len)
		n = io->out_cap - io->out_len;
	if (n > 0) {
		memcpy(io->out + io->out_len, ptr, n);
		io->out_len += n;
	}
	ret += len;
	if (pi->left && pi->width > len)
		ret += b0140_printf_pad(io, pi->width - len, pi->pad == '0');
	return (ret);
}

/* ======================= xprintf_errno.c ======================= */

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
ref___printf_arginfo_errno(const struct printf_info *pi __unused, size_t n,
    int *argt)
{

	assert(n >= 1);
	argt[0] = PA_INT;
	return (1);
}

int
ref___printf_render_errno(struct __printf_io *io, const struct printf_info *pi
    __unused, const void *const *arg)
{
	int ret, error;
	char buf[64];
	char errnomsg[NL_TEXTMAX];

	ret = 0;
	error = *((const int *)arg[0]);
	if (error >= 0 && error < __hidden_sys_nerr) {
		strerror_r(error, errnomsg, sizeof(errnomsg));
		return (__printf_out(io, pi, errnomsg, strlen(errnomsg)));
	}
	sprintf(buf, "errno=%d/0x%x", error, error);
	ret += __printf_out(io, pi, buf, strlen(buf));
	__printf_flush(io);
	return(ret);
}

/* ------------------------------------------------------------------------ */
/* putchar / printf                                                          */
/* ------------------------------------------------------------------------ */

#define	FLOCKFILE_CANCELSAFE(fp)	do { (void)(fp); } while (0)
#define	FUNLOCKFILE_CANCELSAFE()	do { } while (0)

int
__sputc(int c, FILE *fp)
{

	return (fputc(c, fp));
}

/* ======================= putchar.c ======================= */

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

int
ref_putchar(int c)
{
	int retval;
	FILE *so = stdout;

	FLOCKFILE_CANCELSAFE(so);
	/* Orientation set by __sputc() when buffer is full. */
	/* ORIENT(so, -1); */
	retval = __sputc(c, so);
	FUNLOCKFILE_CANCELSAFE();
	return (retval);
}

int
ref_putchar_unlocked(int ch)
{

	return (__sputc(ch, stdout));
}

/* ======================= printf.c ======================= */

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
ref_printf(char const * __restrict fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vfprintf(stdout, fmt, ap);
	va_end(ap);
	return (ret);
}
int
ref_printf_l(locale_t locale, char const * __restrict fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vfprintf_l(stdout, locale, fmt, ap);
	va_end(ap);
	return (ret);
}
