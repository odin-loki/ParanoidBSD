/*
 * Reference oracle for batch b0106s3 (lib/libc/stdio/vsprintf.c).
 *
 * The original HardenedBSD source is below with every function renamed with a
 * "ref_" prefix.  The function bodies are UNMODIFIED: the renaming is done
 * with #define, and the FreeBSD libc internals the body reaches for are
 * supplied by the shared test substrate below.  Only #include lines of
 * FreeBSD-private headers were dropped.
 *
 * The substrate is *shared*: port.cppm calls exactly these objects and
 * functions, so a differential test between the port and this oracle compares
 * the logic that actually lives in vsprintf.c.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

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

#define __va_list va_list

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

#undef vsprintf_l
#define vsprintf_l ref_vsprintf_l

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
vsprintf_l(char * __restrict str, locale_t locale,
    const char * __restrict fmt, __va_list ap)
{
	FILE f = FAKE_FILE;
	int serrno = errno;
	int ret;
	FIX_LOCALE(locale);

	f._flags = __SWR | __SSTR;
	f._bf._base = f._p = (unsigned char *)str;
	f._bf._size = f._w = INT_MAX;
	ret = __vfprintf(&f, locale, serrno, fmt, ap);
	*f._p = 0;
	return (ret);
}

#undef vsprintf	/* _FORTIFY_SOURCE */

#define vsprintf ref_vsprintf

int
vsprintf(char * __restrict str, const char * __restrict fmt, __va_list ap)
{
	return vsprintf_l(str, __get_locale(), fmt, ap);
}
