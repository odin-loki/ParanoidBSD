/*
 * pbsd.lib.libc.locale.b0153s2 - C++23 port of batch b0153s2.
 *
 * Source: lib/libc/locale/wcsftime.c
 *
 * Faithful port: same control flow, operand types, signedness, evaluation
 * order, pointer arithmetic, and bugs.  Copyright headers from the original
 * are kept with the code they cover.
 */

module;

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

export module pbsd.lib.libc.locale.b0153s2;

#ifndef SIZE_T_MAX
#define SIZE_T_MAX SIZE_MAX
#endif

export namespace pbsd::lib_libc_locale::b0153s2 {

locale_t
test_locale()
{
	static locale_t cached;

	if (cached == NULL)
		cached = newlocale(LC_ALL_MASK, "C", NULL);
	return (cached);
}

} /* export namespace - helpers only */

export namespace pbsd::lib_libc_locale::b0153s2 {

extern "C" {
size_t	wcsrtombs_l(char * __restrict, const wchar_t ** __restrict, size_t,
	    mbstate_t * __restrict, locale_t);
size_t	mbsrtowcs_l(wchar_t * __restrict, const char ** __restrict, size_t,
	    mbstate_t * __restrict, locale_t);
size_t	strftime_l(char * __restrict, size_t, const char * __restrict,
	    const struct tm * __restrict, locale_t);
}

#define FIX_LOCALE(l)	do {						\
	if ((l) == NULL)						\
		(l) = pbsd::lib_libc_locale::b0153s2::test_locale();	\
} while (0)

#define __get_locale()	pbsd::lib_libc_locale::b0153s2::test_locale()

export namespace pbsd::lib_libc_locale::b0153s2 {

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Tim J. Robbins
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
 * Convert date and time to a wide-character string.
 *
 * This is the wide-character counterpart of strftime(). So that we do not
 * have to duplicate the code of strftime(), we convert the format string to
 * multibyte, call strftime(), then convert the result back into wide
 * characters.
 *
 * This technique loses in the presence of stateful multibyte encoding if any
 * of the conversions in the format string change conversion state. When
 * stateful encoding is implemented, we will need to reset the state between
 * format specifications in the format string.
 */
size_t
wcsftime_l(wchar_t * __restrict wcs, size_t maxsize,
	const wchar_t * __restrict format, const struct tm * __restrict timeptr,
	locale_t locale)
{
	static const mbstate_t initial{};
	mbstate_t mbs;
	char *dst, *sformat;
	const char *dstp;
	const wchar_t *formatp;
	size_t n, sflen;
	int sverrno;
	FIX_LOCALE(locale);

	sformat = dst = NULL;

	/*
	 * Convert the supplied format string to a multibyte representation
	 * for strftime(), which only handles single-byte characters.
	 */
	mbs = initial;
	formatp = format;
	sflen = wcsrtombs_l(NULL, &formatp, 0, &mbs, locale);
	if (sflen == (size_t)-1)
		goto error;
	if ((sformat = (char *)malloc(sflen + 1)) == NULL)
		goto error;
	mbs = initial;
	wcsrtombs_l(sformat, &formatp, sflen + 1, &mbs, locale);

	/*
	 * Allocate memory for longest multibyte sequence that will fit
	 * into the caller's buffer and call strftime() to fill it.
	 * Then, copy and convert the result back into wide characters in
	 * the caller's buffer.
	 */
	if (SIZE_T_MAX / MB_CUR_MAX <= maxsize) {
		/* maxsize is prepostorously large - avoid int. overflow. */
		errno = EINVAL;
		goto error;
	}
	if ((dst = (char *)malloc(maxsize * MB_CUR_MAX)) == NULL)
		goto error;
	if (strftime_l(dst, maxsize, sformat, timeptr, locale) == 0)
		goto error;
	dstp = dst;
	mbs = initial;
	n = mbsrtowcs_l(wcs, &dstp, maxsize, &mbs, locale);
	if (n == (size_t)-2 || n == (size_t)-1 || dstp != NULL)
		goto error;

	free(sformat);
	free(dst);
	return (n);

error:
	sverrno = errno;
	free(sformat);
	free(dst);
	errno = sverrno;
	return (0);
}

size_t
wcsftime(wchar_t * __restrict wcs, size_t maxsize,
	const wchar_t * __restrict format, const struct tm * __restrict timeptr)
{
	return pbsd::lib_libc_locale::b0153s2::wcsftime_l(wcs, maxsize, format,
	    timeptr, __get_locale());
}

} /* export namespace pbsd::lib_libc_locale::b0153s2 */
