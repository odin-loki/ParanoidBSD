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
 * PBSD batch b0153s2 -- C++23 module port of:
 *   hbsd/src/lib/libc/locale/wcsftime.c
 */

module;

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

export module pbsd.lib.libc.locale.b0153s2;

/*
 * Stand-ins for the libc-internal facilities that wcsftime.c relies on:
 * SIZE_T_MAX from FreeBSD's <sys/limits.h>, and FIX_LOCALE, __get_locale()
 * and the extended-locale wide character conversions from
 * <xlocale_private.h>.  Only these interfaces are supplied here; the ported
 * function bodies below are unchanged from the original.
 */
#ifndef SIZE_T_MAX
#define SIZE_T_MAX	((size_t)-1)
#endif

#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * CHAR_BIT)
#endif

#define FIX_LOCALE(l)	do {						\
	if ((l) == NULL)						\
		(l) = __get_locale();					\
} while (0)

namespace pbsd::lib_libc_locale::b0153s2 {

/* Not exported: module linkage, these replace <xlocale_private.h>. */

locale_t
__get_locale(void)
{

	return (uselocale((locale_t)0));
}

size_t
wcsrtombs_l(char * __restrict dst, const wchar_t ** __restrict src,
    size_t len, mbstate_t * __restrict ps, locale_t loc)
{
	locale_t old;
	size_t r;

	old = uselocale(loc);
	r = ::wcsrtombs(dst, src, len, ps);
	uselocale(old);
	return (r);
}

size_t
mbsrtowcs_l(wchar_t * __restrict dst, const char ** __restrict src,
    size_t len, mbstate_t * __restrict ps, locale_t loc)
{
	locale_t old;
	size_t r;

	old = uselocale(loc);
	r = ::mbsrtowcs(dst, src, len, ps);
	uselocale(old);
	return (r);
}

} /* namespace pbsd::lib_libc_locale::b0153s2 */

export namespace pbsd::lib_libc_locale::b0153s2 {

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

/*
 * The host <wchar.h> also declares a ::wcsftime_l, which argument-dependent
 * lookup would drag into the call below.  Redirect the name so that the body
 * itself can stay exactly as it is upstream.
 */
#define wcsftime_l	::pbsd::lib_libc_locale::b0153s2::wcsftime_l

size_t
wcsftime(wchar_t * __restrict wcs, size_t maxsize,
	const wchar_t * __restrict format, const struct tm * __restrict timeptr)
{
	return wcsftime_l(wcs, maxsize, format, timeptr, __get_locale());
}

} /* namespace pbsd::lib_libc_locale::b0153s2 */
