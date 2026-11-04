module;

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <climits>
#include <cstddef>
#include <cstdio>
#include <cwchar>
#include <locale.h>

export module pbsd.lib.libc.stdio.b0106s1;

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

#define	__SWR	0x0008
#define	__SERR	0x0040

extern "C" {
locale_t pbsd_shim_get_locale(void);
struct xlocale_ctype *pbsd_shim_xlocale_ctype(locale_t);
void pbsd_shim_orient(struct pbsd_shim_file *, int);
void pbsd_shim_flockfile(struct pbsd_shim_file *);
void pbsd_shim_funlockfile(struct pbsd_shim_file *);
int pbsd_shim_sputc(int, struct pbsd_shim_file *);
}

#define FILE struct pbsd_shim_file

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

#define ORIENT(fp, dir) pbsd_shim_orient((fp), (dir))
#define XLOCALE_CTYPE(loc) pbsd_shim_xlocale_ctype(loc)
#define __sputc(c, fp) pbsd_shim_sputc((c), (fp))

export namespace pbsd::lib_libc_stdio::b0106s1 {

using shim_file = pbsd_shim_file;

inline constexpr short shim_SWR = 0x0008;
inline constexpr short shim_SERR = 0x0040;

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

} /* namespace pbsd::lib_libc_stdio::b0106s1 */
