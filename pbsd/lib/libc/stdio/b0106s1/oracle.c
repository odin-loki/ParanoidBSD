/*
 * Reference oracle for batch b0106s1 (lib/libc/stdio/fputwc.c).
 *
 * The original HardenedBSD source is reproduced below with every function
 * renamed with a "ref_" prefix via the preprocessor.  Function bodies are
 * UNMODIFIED.  FreeBSD libc internals the bodies reach for are supplied by
 * the shared test substrate below; the port calls the same substrate objects
 * with C linkage so a differential test compares only the logic in fputwc.c.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

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
	int _lockdepth;
	int _locktotal;
	mbstate_t _mbstate;
};

struct xlocale_ctype {
	size_t (*__wcrtomb)(char *, wchar_t, mbstate_t *);
	const char *__name;
};

locale_t pbsd_shim_locale_c;
locale_t pbsd_shim_locale_utf8;

int pbsd_shim_sputc_last_c;
unsigned long long pbsd_shim_sputc_calls;
unsigned long long pbsd_shim_sputc_trace;

static size_t
pbsd_shim_wcrtomb_at(locale_t loc, char *s, wchar_t wc, mbstate_t *ps)
{
	locale_t oldloc;
	size_t r;

	oldloc = uselocale(loc);
	r = wcrtomb(s, wc, ps);
	uselocale(oldloc);
	return (r);
}

static size_t
pbsd_shim_wcrtomb_c(char *s, wchar_t wc, mbstate_t *ps)
{
	return (pbsd_shim_wcrtomb_at(pbsd_shim_locale_c, s, wc, ps));
}

static size_t
pbsd_shim_wcrtomb_utf8(char *s, wchar_t wc, mbstate_t *ps)
{
	return (pbsd_shim_wcrtomb_at(pbsd_shim_locale_utf8, s, wc, ps));
}

static struct xlocale_ctype pbsd_shim_ctype_c = { pbsd_shim_wcrtomb_c, "C" };
static struct xlocale_ctype pbsd_shim_ctype_utf8 = {
	pbsd_shim_wcrtomb_utf8, "C.UTF-8"
};

struct xlocale_ctype *
pbsd_shim_xlocale_ctype(locale_t loc)
{
	if (loc == pbsd_shim_locale_utf8)
		return (&pbsd_shim_ctype_utf8);
	return (&pbsd_shim_ctype_c);
}

locale_t
pbsd_shim_get_locale(void)
{
	return (uselocale((locale_t)0));
}

void
pbsd_shim_orient(struct pbsd_shim_file *fp, int dir)
{
	if (fp->_orientation == 0)
		fp->_orientation = dir;
}

void
pbsd_shim_flockfile(struct pbsd_shim_file *fp)
{
	if (fp != NULL) {
		fp->_lockdepth++;
		fp->_locktotal++;
	}
}

void
pbsd_shim_funlockfile(struct pbsd_shim_file *fp)
{
	if (fp != NULL) {
		fp->_lockdepth--;
		fp->_locktotal++;
	}
}

int
pbsd_shim_sputc(int c, struct pbsd_shim_file *fp)
{
	pbsd_shim_sputc_last_c = c;
	pbsd_shim_sputc_calls++;
	pbsd_shim_sputc_trace = pbsd_shim_sputc_trace * 1000003ULL +
	    (unsigned long long)(unsigned int)c;
	if ((fp->_flags & __SWR) == 0) {
		fp->_flags |= __SERR;
		return (EOF);
	}
	if (fp->_w <= 0) {
		fp->_flags |= __SERR;
		return (EOF);
	}
	*fp->_p = (unsigned char)c;
	fp->_p++;
	fp->_w--;
	return ((int)(unsigned char)c);
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

void
pbsd_shim_file_zero(struct pbsd_shim_file *fp)
{
	memset(fp, 0, sizeof(*fp));
	fp->_file = -1;
}

void
pbsd_shim_file_bind(struct pbsd_shim_file *fp, unsigned char *base, int size,
    short flags, int orientation)
{
	pbsd_shim_file_zero(fp);
	fp->_flags = flags;
	fp->_orientation = orientation;
	fp->_bf._base = base;
	fp->_bf._size = size;
	fp->_p = base;
	fp->_w = size;
}

#define __get_locale() pbsd_shim_get_locale()

#define FIX_LOCALE(loc)							\
	do {								\
		if ((loc) == NULL)					\
			(loc) = __get_locale();				\
	} while (0)

#define FLOCKFILE_CANCELSAFE(fp)					\
	{								\
		FILE *_shim_lockfp = (fp);				\
		pbsd_shim_flockfile(_shim_lockfp);
#define FUNLOCKFILE_CANCELSAFE()					\
		pbsd_shim_funlockfile(_shim_lockfp);			\
	}

#define FILE struct pbsd_shim_file

#define ORIENT(fp, dir) pbsd_shim_orient((fp), (dir))
#define XLOCALE_CTYPE(loc) pbsd_shim_xlocale_ctype(loc)
#define __sputc(c, fp) pbsd_shim_sputc((c), (fp))

#undef __fputwc
#undef fputwc_l
#undef fputwc
#define __fputwc ref___fputwc
#define fputwc_l ref_fputwc_l
#define fputwc ref_fputwc

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002-2004 Tim J. Robbins.
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

/*
 * Non-MT-safe version.
 */
wint_t
__fputwc(wchar_t wc, FILE *fp, locale_t locale)
{
	char buf[MB_LEN_MAX];
	size_t i, len;
	struct xlocale_ctype *l = XLOCALE_CTYPE(locale);

	if ((len = l->__wcrtomb(buf, wc, &fp->_mbstate)) == (size_t)-1) {
		fp->_flags |= __SERR;
		return (WEOF);
	}

	for (i = 0; i < len; i++)
		if (__sputc((unsigned char)buf[i], fp) == EOF)
			return (WEOF);

	return ((wint_t)wc);
}

/*
 * MT-safe version.
 */
wint_t
fputwc_l(wchar_t wc, FILE *fp, locale_t locale)
{
	wint_t r;
	FIX_LOCALE(locale);

	FLOCKFILE_CANCELSAFE(fp);
	ORIENT(fp, 1);
	r = __fputwc(wc, fp, locale);
	FUNLOCKFILE_CANCELSAFE();

	return (r);
}
wint_t
fputwc(wchar_t wc, FILE *fp)
{
	return fputwc_l(wc, fp, __get_locale());
}
