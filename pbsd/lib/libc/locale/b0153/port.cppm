/*
 * pbsd.lib.libc.locale.b0153 - C++23 port of batch b0153.
 *
 * Sources:
 *	lib/libc/locale/wcsftime.c
 *	lib/libc/locale/ldpart.c
 *	lib/libc/locale/gb2312.c
 *
 * The ports below are faithful: same control flow, same operand types, same
 * signedness, same evaluation order, same pointer arithmetic, same bugs.  The
 * copyright headers of the originals are kept with the code they cover.
 *
 * See skipped.txt for what is not here and why.
 */

module;

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <locale.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>

export module pbsd.lib.libc.locale.b0153;

/* <sys/param.h> normally has this. */
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

/* <sys/limits.h> */
#ifndef SIZE_T_MAX
#define SIZE_T_MAX SIZE_MAX
#endif

/* FreeBSD <sys/errno.h>: "Inappropriate file type or format". */
#ifndef EFTYPE
#define EFTYPE 79
#endif

namespace pbsd::lib_libc_locale::b0153 {

/*
 * xlocale_private.h substitutes.  Not exported: they stand in for private
 * libc plumbing, not for anything in the batch.
 */
locale_t
fallback_locale()
{
	static locale_t cached;

	if (cached == NULL)
		cached = newlocale(LC_ALL_MASK, "C", NULL);
	return (cached);
}

size_t
wcsrtombs_l(char * __restrict dst, const wchar_t ** __restrict src, size_t len,
    mbstate_t * __restrict ps, locale_t loc)
{
	locale_t old;
	size_t r;

	old = uselocale(loc);
	r = wcsrtombs(dst, src, len, ps);
	uselocale(old);
	return (r);
}

size_t
mbsrtowcs_l(wchar_t * __restrict dst, const char ** __restrict src, size_t len,
    mbstate_t * __restrict ps, locale_t loc)
{
	locale_t old;
	size_t r;

	old = uselocale(loc);
	r = mbsrtowcs(dst, src, len, ps);
	uselocale(old);
	return (r);
}

} /* namespace pbsd::lib_libc_locale::b0153 */

#define FIX_LOCALE(l)	do {						\
	if ((l) == NULL)						\
		(l) = fallback_locale();					\
} while (0)

#define __get_locale()	fallback_locale()

export namespace pbsd::lib_libc_locale::b0153 {

/*
 * ===========================================================================
 * lib/libc/locale/wcsftime.c
 * ===========================================================================
 *
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
	/*
	 * Qualified: libc also declares ::wcsftime_l(), which argument
	 * dependent lookup would drag in and make the call ambiguous.
	 */
	return pbsd::lib_libc_locale::b0153::wcsftime_l(wcs, maxsize, format,
	    timeptr, __get_locale());
}

/*
 * ===========================================================================
 * lib/libc/locale/ldpart.c
 * ===========================================================================
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2000, 2001 Alexey Zelkin <phantom@FreeBSD.org>
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

/* ldpart.h */
constexpr int _LDP_ERROR = -1;
constexpr int _LDP_LOADED = 0;
constexpr int _LDP_CACHE = 1;

/*
 * setlocale.h: root of the compiled locale tree.  setlocale() sets this in
 * libc.
 */
const char *_PathLocale = "/usr/share/locale";

int split_lines(char *, const char *);

int
__part_load_locale(const char *name,
		int *using_locale,
		char **locale_buf,
		const char *category_filename,
		int locale_buf_size_max,
		int locale_buf_size_min,
		const char **dst_localebuf)
{
	int		saverr, fd, i, num_lines;
	char		*lbuf, *p;
	const char	*plim;
	char		filename[PATH_MAX];
	struct stat	st;
	size_t		namesize, bufsize;

	/* 'name' must be already checked. */
	if (strcmp(name, "C") == 0 || strcmp(name, "POSIX") == 0 ||
	    strncmp(name, "C.", 2) == 0) {
		*using_locale = 0;
		return (_LDP_CACHE);
	}

	/*
	 * If the locale name is the same as our cache, use the cache.
	 */
	if (*locale_buf != NULL && strcmp(name, *locale_buf) == 0) {
		*using_locale = 1;
		return (_LDP_CACHE);
	}

	/*
	 * Slurp the locale file into the cache.
	 */
	namesize = strlen(name) + 1;

	/* 'PathLocale' must be already set & checked. */

	/* Range checking not needed, 'name' size is limited */
	strcpy(filename, _PathLocale);
	strcat(filename, "/");
	strcat(filename, name);
	strcat(filename, "/");
	strcat(filename, category_filename);
	if ((fd = open(filename, O_RDONLY | O_CLOEXEC)) < 0)
		return (_LDP_ERROR);
	if (fstat(fd, &st) != 0)
		goto bad_locale;
	if (st.st_size <= 0) {
		errno = EFTYPE;
		goto bad_locale;
	}
	bufsize = namesize + st.st_size;
	if ((lbuf = (char *)malloc(bufsize)) == NULL) {
		errno = ENOMEM;
		goto bad_locale;
	}
	(void)strcpy(lbuf, name);
	p = lbuf + namesize;
	plim = p + st.st_size;
	if (read(fd, p, (size_t) st.st_size) != st.st_size)
		goto bad_lbuf;
	/*
	 * Parse the locale file into localebuf.
	 */
	if (plim[-1] != '\n') {
		errno = EFTYPE;
		goto bad_lbuf;
	}
	num_lines = split_lines(p, plim);
	if (num_lines >= locale_buf_size_max)
		num_lines = locale_buf_size_max;
	else if (num_lines >= locale_buf_size_min)
		num_lines = locale_buf_size_min;
	else {
		errno = EFTYPE;
		goto bad_lbuf;
	}
	(void)close(fd);
	/*
	 * Record the successful parse in the cache.
	 */
	if (*locale_buf != NULL)
		free(*locale_buf);
	*locale_buf = lbuf;
	for (p = *locale_buf, i = 0; i < num_lines; i++)
		dst_localebuf[i] = (p += strlen(p) + 1);
	for (i = num_lines; i < locale_buf_size_max; i++)
		dst_localebuf[i] = NULL;
	*using_locale = 1;

	return (_LDP_LOADED);

bad_lbuf:
	saverr = errno;
	free(lbuf);
	errno = saverr;
bad_locale:
	saverr = errno;
	(void)close(fd);
	errno = saverr;

	return (_LDP_ERROR);
}

int
split_lines(char *p, const char *plim)
{
	int i;

	i = 0;
	while (p < plim) {
		if (*p == '\n') {
			*p = '\0';
			i++;
		}
		p++;
	}
	return (i);
}

/*
 * ===========================================================================
 * lib/libc/locale/gb2312.c
 * ===========================================================================
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2013 Garrett D'Amore <garrett@damore.org>
 * Copyright 2010 Nexenta Systems, Inc.  All rights reserved.
 * Copyright (c) 2004 Tim J. Robbins. All rights reserved.
 * Copyright (c) 2003 David Xu <davidxu@freebsd.org>
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
 * _GB2312_init() sets l->__mb_cur_max = 2, i.e. MB_CUR_MAX is 2 for every
 * caller of the functions below.
 */
#undef MB_CUR_MAX
#define MB_CUR_MAX 2

typedef struct {
	int	count;
	u_char	bytes[2];
} GB2312State;

int	GB2312_mbsinit(const mbstate_t *);
int	GB2312_check(const char *, size_t);
size_t	GB2312_mbrtowc(wchar_t * __restrict, const char * __restrict, size_t,
	    mbstate_t * __restrict);
size_t	GB2312_wcrtomb(char * __restrict, wchar_t, mbstate_t * __restrict);

int
GB2312_mbsinit(const mbstate_t *ps)
{

	return (ps == NULL || ((const GB2312State *)ps)->count == 0);
}

int
GB2312_check(const char *str, size_t n)
{
	const u_char *s = (const u_char *)str;

	if (n == 0)
		/* Incomplete multibyte sequence */
		return (-2);
	if (s[0] >= 0xa1 && s[0] <= 0xfe) {
		if (n < 2)
			/* Incomplete multibyte sequence */
			return (-2);
		if (s[1] < 0xa1 || s[1] > 0xfe)
			/* Invalid multibyte sequence */
			return (-1);
		return (2);
	} else if (s[0] & 0x80) {
		/* Invalid multibyte sequence */
		return (-1);
	}
	return (1);
}

size_t
GB2312_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps)
{
	GB2312State *gs;
	wchar_t wc;
	int i, len, ocount;
	size_t ncopy;

	gs = (GB2312State *)ps;

	if (gs->count < 0 || gs->count > sizeof(gs->bytes)) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL) {
		s = "";
		n = 1;
		pwc = NULL;
	}

	ncopy = MIN(MIN(n, MB_CUR_MAX), sizeof(gs->bytes) - gs->count);
	memcpy(gs->bytes + gs->count, s, ncopy);
	ocount = gs->count;
	gs->count += ncopy;
	s = (char *)gs->bytes;
	n = gs->count;

	if ((len = GB2312_check(s, n)) < 0)
		return ((size_t)len);
	wc = 0;
	i = len;
	while (i-- > 0)
		wc = (wc << 8) | (unsigned char)*s++;
	if (pwc != NULL)
		*pwc = wc;
	gs->count = 0;
	return (wc == L'\0' ? 0 : len - ocount);
}

size_t
GB2312_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	GB2312State *gs;

	gs = (GB2312State *)ps;

	if (gs->count != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);
	if (wc & 0x8000) {
		*s++ = (wc >> 8) & 0xff;
		*s = wc & 0xff;
		return (2);
	}
	*s = wc & 0xff;
	return (1);
}

} /* export namespace pbsd::lib_libc_locale::b0153 */
