/*
 * Reference oracle for batch b0317.
 *
 * hbsd/src/lib/libc/stdio/tempnam.c, vsnprintf.c, and xprintf_quote.c
 * concatenated, with every function renamed with a "ref_" prefix.  Function
 * bodies are UNMODIFIED.  Declarations from unavailable FreeBSD-private headers
 * are supplied below.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#ifndef P_tmpdir
#define P_tmpdir "/tmp/"
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

#ifndef EOF
#define EOF (-1)
#endif

#define __va_list va_list

/* ------------------------------------------------------------------ */
/* vsnprintf substrate (from local.h / xlocale_private.h)              */
/* ------------------------------------------------------------------ */

#define __SLBF	0x0001
#define __SNBF	0x0002
#define __SRD	0x0004
#define __SWR	0x0008
#define __SRW	0x0010
#define __SEOF	0x0020
#define __SERR	0x0040
#define __SMBF	0x0080
#define __SAPP	0x0100
#define __SSTR	0x0200

struct pbsd_shim_bufdesc {
	unsigned char *_base;
	int _size;
};

struct pbsd_shim_file {
	unsigned char *_p;
	int _r;
	int _w;
	short _flags;
	short _file;
	struct pbsd_shim_bufdesc _bf;
	int _lbfsize;
	int _orientation;
};

locale_t pbsd_shim_locale_c;
locale_t pbsd_shim_locale_utf8;

locale_t
pbsd_shim_get_locale(void)
{
	return (uselocale((locale_t)0));
}

#define __get_locale() pbsd_shim_get_locale()

#define FIX_LOCALE(loc)							\
	do {								\
		if ((loc) == NULL)					\
			(loc) = __get_locale();				\
	} while (0)

#define PBSD_SHIM_PRINTF_MAX 4096

locale_t pbsd_shim_vfprintf_locale;
int pbsd_shim_vfprintf_serrno;
unsigned long long pbsd_shim_vfprintf_calls;
short pbsd_shim_vfprintf_entry_flags;
short pbsd_shim_vfprintf_entry_file;
int pbsd_shim_vfprintf_entry_r;
int pbsd_shim_vfprintf_entry_w;
int pbsd_shim_vfprintf_entry_size;
int pbsd_shim_vfprintf_entry_lbfsize;
int pbsd_shim_vfprintf_entry_orientation;
unsigned char *pbsd_shim_vfprintf_entry_base;
unsigned char *pbsd_shim_vfprintf_entry_p;

int
pbsd_shim_vfprintf(struct pbsd_shim_file *fp, locale_t locale, int serrno,
    const char *fmt, va_list ap)
{
	char tmp[PBSD_SHIM_PRINTF_MAX];
	locale_t oldloc;
	int saved_errno, n, i;

	pbsd_shim_vfprintf_locale = locale;
	pbsd_shim_vfprintf_serrno = serrno;
	pbsd_shim_vfprintf_calls++;
	pbsd_shim_vfprintf_entry_flags = fp->_flags;
	pbsd_shim_vfprintf_entry_file = fp->_file;
	pbsd_shim_vfprintf_entry_r = fp->_r;
	pbsd_shim_vfprintf_entry_w = fp->_w;
	pbsd_shim_vfprintf_entry_size = fp->_bf._size;
	pbsd_shim_vfprintf_entry_lbfsize = fp->_lbfsize;
	pbsd_shim_vfprintf_entry_orientation = fp->_orientation;
	pbsd_shim_vfprintf_entry_base = fp->_bf._base;
	pbsd_shim_vfprintf_entry_p = fp->_p;
	if ((fp->_flags & __SWR) == 0) {
		fp->_flags |= __SERR;
		return (-1);
	}
	saved_errno = errno;
	errno = serrno;
	oldloc = uselocale(locale);
	n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
	uselocale(oldloc);
	errno = saved_errno;
	if (n < 0) {
		fp->_flags |= __SERR;
		return (-1);
	}
	for (i = 0; i < n && i < (int)sizeof(tmp) - 1; i++) {
		if (fp->_w <= 0) {
			fp->_flags |= __SERR;
			return (-1);
		}
		*fp->_p = (unsigned char)tmp[i];
		fp->_p++;
		fp->_w--;
	}
	return (n);
}

int
pbsd_shim_init(void)
{
	int degraded = 0;

	pbsd_shim_locale_c = newlocale(LC_ALL_MASK, "C", NULL);
	pbsd_shim_locale_utf8 = newlocale(LC_ALL_MASK, "C.UTF-8", NULL);
	if (pbsd_shim_locale_utf8 == NULL) {
		pbsd_shim_locale_utf8 = newlocale(LC_ALL_MASK, "en_US.UTF-8",
		    NULL);
	}
	if (pbsd_shim_locale_utf8 == NULL) {
		pbsd_shim_locale_utf8 = newlocale(LC_ALL_MASK, "C", NULL);
		degraded = 1;
	}
	if (pbsd_shim_locale_c == NULL || pbsd_shim_locale_utf8 == NULL)
		return (-1);
	return (degraded);
}

#define FILE struct pbsd_shim_file
#define __vfprintf(fp, loc, serrno, fmt, ap)				\
	pbsd_shim_vfprintf((fp), (loc), (serrno), (fmt), (ap))
#define FAKE_FILE { ._file = -1 }

/* ------------------------------------------------------------------ */
/* tempnam substrate                                                   */
/* ------------------------------------------------------------------ */

unsigned char tempnam_guard_before[32];
char tempnam_alloc_buf[MAXPATHLEN];
unsigned char tempnam_guard_after[32];

int mock_malloc_fail;
int mock_malloc_calls;
int mock_free_calls;
char *mock_secure_getenv_val;
int mock_secure_getenv_calls;
unsigned mock_mktemp_calls;
int mock_mktemp_succeed_at;
char *mock_mktemp_last_name;

void *
__real_malloc(size_t nbytes);

void *
__wrap_malloc(size_t nbytes)
{
	mock_malloc_calls++;
	if (mock_malloc_fail)
		return (NULL);
	if (nbytes == (size_t)MAXPATHLEN)
		return (tempnam_alloc_buf);
	return (__real_malloc(nbytes));
}

void
__wrap_free(void *ptr)
{
	mock_free_calls++;
	(void)ptr;
}

char *
secure_getenv(const char *name)
{
	mock_secure_getenv_calls++;
	if (name != NULL && strcmp(name, "TMPDIR") == 0)
		return (mock_secure_getenv_val);
	return (NULL);
}

char *
_mktemp(char *name)
{
	mock_mktemp_calls++;
	mock_mktemp_last_name = name;
	if (mock_mktemp_succeed_at >= 0 &&
	    mock_mktemp_calls == (unsigned)(mock_mktemp_succeed_at + 1))
		return (name);
	return (NULL);
}

/* ------------------------------------------------------------------ */
/* xprintf_quote substrate                                             */
/* ------------------------------------------------------------------ */

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

int mock_printf_out_ret;
int mock_printf_out_calls;
struct __printf_io *mock_printf_out_last_io;
const struct printf_info *mock_printf_out_last_pi;
const char *mock_printf_out_last_buf;
int mock_printf_out_last_len;

int mock_printf_flush_calls;
struct __printf_io *mock_printf_flush_last_io;

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

/* ====================================================================== */
/* lib/libc/stdio/tempnam.c                                               */
/* ====================================================================== */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1988, 1993
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

char *
ref_tempnam(const char *dir, const char *pfx)
{
	int sverrno;
	char *f, *name;

	if (!(name = malloc(MAXPATHLEN)))
		return(NULL);

	if (!pfx)
		pfx = "tmp.";

	if ((f = secure_getenv("TMPDIR")) != NULL) {
		(void)snprintf(name, MAXPATHLEN, "%s%s%sXXXXXX", f,
		    *(f + strlen(f) - 1) == '/'? "": "/", pfx);
		if ((f = _mktemp(name)))
			return(f);
	}

	if ((f = (char *)dir)) {
		(void)snprintf(name, MAXPATHLEN, "%s%s%sXXXXXX", f,
		    *(f + strlen(f) - 1) == '/'? "": "/", pfx);
		if ((f = _mktemp(name)))
			return(f);
	}

	f = P_tmpdir;
	(void)snprintf(name, MAXPATHLEN, "%s%sXXXXXX", f, pfx);
	if ((f = _mktemp(name)))
		return(f);

	f = _PATH_TMP;
	(void)snprintf(name, MAXPATHLEN, "%s%sXXXXXX", f, pfx);
	if ((f = _mktemp(name)))
		return(f);

	sverrno = errno;
	free(name);
	errno = sverrno;
	return(NULL);
}

/* ====================================================================== */
/* lib/libc/stdio/vsnprintf.c                                             */
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
ref_vsnprintf_l(char * __restrict str, size_t n, locale_t locale, 
    const char * __restrict fmt, __va_list ap)
{
	FILE f = FAKE_FILE;
	size_t on;
	int serrno = errno;
	int ret;
	char dummy[2];
	FIX_LOCALE(locale);

	on = n;
	if (n != 0)
		n--;
	if (n > INT_MAX) {
		errno = EOVERFLOW;
		*str = '\0';
		return (EOF);
	}
	/* Stdio internals do not deal correctly with zero length buffer */
	if (n == 0) {
		if (on > 0)
	  		*str = '\0';
		str = dummy;
		n = 1;
	}
	f._flags = __SWR | __SSTR;
	f._bf._base = f._p = (unsigned char *)str;
	f._bf._size = f._w = n;
	ret = __vfprintf(&f, locale, serrno, fmt, ap);
	if (on > 0)
		*f._p = '\0';
	return (ret);
}
int
ref_vsnprintf(char * __restrict str, size_t n, const char * __restrict fmt,
    __va_list ap)
{
	return ref_vsnprintf_l(str, n, __get_locale(), fmt, ap);
}

/* ====================================================================== */
/* lib/libc/stdio/xprintf_quote.c                                         */
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
ref___printf_arginfo_quote(const struct printf_info *pi __unused, size_t n, int *argt)
{

	assert(n >= 1);
	argt[0] = PA_POINTER;
	return (1);
}

int
ref___printf_render_quote(struct __printf_io *io, const struct printf_info *pi __unused, const void *const *arg)
{
	const char *str, *p, *t, *o;
	char r[5];
	int i, ret;

	str = *((const char *const *)arg[0]);
	if (str == NULL)
		return (__printf_out(io, pi, "\"(null)\"", 8));
	if (*str == '\0')
		return (__printf_out(io, pi, "\"\"", 2));

	for (i = 0, p = str; *p; p++)
		if (isspace(*p) || *p == '\\' || *p == '"')
			i++;
	if (!i) 
		return (__printf_out(io, pi, str, strlen(str)));
	
	ret = __printf_out(io, pi, "\"", 1);
	for (t = p = str; *p; p++) {
		o = NULL;
		if (*p == '\\')
			o = "\\\\";
		else if (*p == '\n')
			o = "\\n";
		else if (*p == '\r')
			o = "\\r";
		else if (*p == '\t')
			o = "\\t";
		else if (*p == ' ')
			o = " ";
		else if (*p == '"')
			o = "\\\"";
		else if (isspace(*p)) {
			sprintf(r, "\\%03o", *p);
			o = r;
		} else
			continue;
		if (p != t)
			ret += __printf_out(io, pi, t, p - t);
		ret += __printf_out(io, pi, o, strlen(o));
		t = p + 1;
	}
	if (p != t)
		ret += __printf_out(io, pi, t, p - t);
	ret += __printf_out(io, pi, "\"", 1);
	__printf_flush(io);
	return(ret);
}
