/*
 * PBSD batch b0039 -- C++23 module port of
 *
 *	lib/libc/locale/mbsrtowcs.c
 *	lib/libc/locale/wcsrtombs.c
 *	lib/libc/locale/mbtowc.c
 *	lib/libc/locale/wcwidth.c
 *
 * The four files are pure dispatch wrappers over the libc locale internals.
 * The internals themselves are not part of this batch; mock UTF-8 backends
 * below stand in so the unmodified wrapper bodies compile and link.
 */

module;

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>

#ifndef SIZE_T_MAX
#define SIZE_T_MAX	SIZE_MAX
#endif

#ifndef MB_LEN_MAX
#define MB_LEN_MAX	4
#endif

#define	XLOCALE_CTYPE(l)	(&((l)->ctype))
#define	FIX_LOCALE(l)		((l) = ((l) == NULL ? pbsd_get_active_locale() : (l)))

namespace {

struct pbsd_mbstate {
	unsigned int	want;
	unsigned int	have;
	unsigned int	wch;
	unsigned int	lbound;
};
using pbsd_mbstate_t = pbsd_mbstate;

struct pbsd_xlocale_ctype {
	size_t	(*__mbsnrtowcs)(wchar_t *, const char **, size_t, size_t,
		    pbsd_mbstate_t *);
	size_t	(*__wcsnrtombs)(char *, const wchar_t **, size_t, size_t,
		    pbsd_mbstate_t *);
	size_t	(*__mbrtowc)(wchar_t *, const char *, size_t,
		    pbsd_mbstate_t *);
	pbsd_mbstate_t	mbsrtowcs;
	pbsd_mbstate_t	wcsrtombs;
	pbsd_mbstate_t	mbtowc;
	int		wcwidth_mode;
};

struct pbsd_locale {
	pbsd_xlocale_ctype ctype;
};

using pbsd_locale_t = pbsd_locale *;

pbsd_locale		pbsd_global_locale;
pbsd_locale		pbsd_alt_locale;
pbsd_locale		*pbsd_active_locale = &pbsd_global_locale;

static int	utf8_decode(const unsigned char *, size_t, wchar_t *,
		    size_t *);
static int	utf8_encode(wchar_t, unsigned char *, size_t, size_t *);

static size_t
pbsd_mbsnrtowcs_impl(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, pbsd_mbstate_t * __restrict ps)
{
	const unsigned char *s;
	size_t nconv, nwritten, consumed;
	wchar_t wc;
	int err;

	(void)ps;
	if (src == NULL)
		return ((size_t)-1);
	s = (const unsigned char *)*src;
	if (s == NULL)
		return ((size_t)-1);
	nconv = 0;
	nwritten = 0;
	while (nms > 0) {
		if (*s == '\0') {
			if (dst != NULL && len > 0) {
				if (nwritten >= len)
					break;
				dst[nwritten++] = L'\0';
			}
			*src = (const char *)s;
			return (nconv);
		}
		err = utf8_decode(s, nms, &wc, &consumed);
		if (err == -1)
			return ((size_t)-1);
		if (err == -2)
			break;
		if (dst != NULL) {
			if (nwritten >= len)
				break;
			dst[nwritten++] = wc;
		}
		nconv++;
		s += consumed;
		nms -= consumed;
	}
	*src = (const char *)s;
	return (nconv);
}

static size_t
pbsd_wcsnrtombs_impl(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwcs, size_t len, pbsd_mbstate_t * __restrict ps)
{
	const wchar_t *s;
	size_t nconv, nwritten, produced;
	unsigned char buf[MB_LEN_MAX];
	int err;

	(void)ps;
	if (src == NULL)
		return ((size_t)-1);
	s = *src;
	if (s == NULL)
		return ((size_t)-1);
	nconv = 0;
	nwritten = 0;
	while (nwcs > 0) {
		if (*s == L'\0') {
			if (dst != NULL && len > 0) {
				if (nwritten >= len)
					break;
				dst[nwritten++] = '\0';
			}
			*src = s;
			return (nconv);
		}
		err = utf8_encode(*s, buf, sizeof(buf), &produced);
		if (err != 0)
			return ((size_t)-1);
		if (dst != NULL) {
			if (nwritten + produced > len)
				break;
			memcpy(dst + nwritten, buf, produced);
			nwritten += produced;
		}
		nconv++;
		s++;
		nwcs--;
	}
	*src = s;
	return (nconv);
}

static size_t
pbsd_mbrtowc_impl(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    pbsd_mbstate_t * __restrict ps)
{
	wchar_t wc;
	size_t consumed;
	int err;

	(void)ps;
	if (n == 0)
		return ((size_t)-2);
	err = utf8_decode((const unsigned char *)s, n, &wc, &consumed);
	if (err == -1)
		return ((size_t)-1);
	if (err == -2)
		return ((size_t)-2);
	if (pwc != NULL)
		*pwc = wc;
	if (wc == L'\0')
		return (0);
	return (consumed);
}

static int
pbsd_wcwidth_impl(wchar_t wc)
{

	if (wc == 0)
		return (0);
	if ((wc >= 0 && wc < 0x20) || wc == 0x7f)
		return (-1);
	if (wc < 0x80)
		return (1);
	if (wc < 0x1100)
		return (2);
	if (wc >= 0xAC00 && wc <= 0xD7A3)
		return (2);
	return (1);
}

static int
utf8_decode(const unsigned char *p, size_t n, wchar_t *wc, size_t *consumed)
{
	uint32_t c;

	if (n == 0)
		return (-2);
	if (p[0] < 0x80) {
		*wc = (wchar_t)p[0];
		*consumed = 1;
		return (0);
	}
	if ((p[0] & 0xE0) == 0xC0) {
		if (n < 2)
			return (-2);
		if ((p[1] & 0xC0) != 0x80)
			return (-1);
		c = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
		if (c < 0x80)
			return (-1);
		*wc = (wchar_t)c;
		*consumed = 2;
		return (0);
	}
	if ((p[0] & 0xF0) == 0xE0) {
		if (n < 3)
			return (-2);
		if ((p[1] & 0xC0) != 0x80 || (p[2] & 0xC0) != 0x80)
			return (-1);
		c = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
		if (c < 0x800)
			return (-1);
		*wc = (wchar_t)c;
		*consumed = 3;
		return (0);
	}
	if ((p[0] & 0xF8) == 0xF0) {
		if (n < 4)
			return (-2);
		if ((p[1] & 0xC0) != 0x80 || (p[2] & 0xC0) != 0x80 ||
		    (p[3] & 0xC0) != 0x80)
			return (-1);
		c = ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) |
		    ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
		if (c < 0x10000 || c > 0x10FFFF)
			return (-1);
		*wc = (wchar_t)c;
		*consumed = 4;
		return (0);
	}
	return (-1);
}

static int
utf8_encode(wchar_t wc, unsigned char *dst, size_t dstl, size_t *produced)
{
	uint32_t cp;

	*produced = 0;
	cp = (uint32_t)wc;
	if (cp < 0x80) {
		if (dstl < 1)
			return (EINVAL);
		dst[0] = (unsigned char)cp;
		*produced = 1;
		return (0);
	}
	if (cp < 0x800) {
		if (dstl < 2)
			return (EINVAL);
		dst[0] = (unsigned char)(0xC0 | (cp >> 6));
		dst[1] = (unsigned char)(0x80 | (cp & 0x3F));
		*produced = 2;
		return (0);
	}
	if (cp < 0x10000) {
		if (dstl < 3)
			return (EINVAL);
		dst[0] = (unsigned char)(0xE0 | (cp >> 12));
		dst[1] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
		dst[2] = (unsigned char)(0x80 | (cp & 0x3F));
		*produced = 3;
		return (0);
	}
	if (cp <= 0x10FFFF) {
		if (dstl < 4)
			return (EINVAL);
		dst[0] = (unsigned char)(0xF0 | (cp >> 18));
		dst[1] = (unsigned char)(0x80 | ((cp >> 12) & 0x3F));
		dst[2] = (unsigned char)(0x80 | ((cp >> 6) & 0x3F));
		dst[3] = (unsigned char)(0x80 | (cp & 0x3F));
		*produced = 4;
		return (0);
	}
	return (EINVAL);
}

static void
pbsd_locale_init_impl(pbsd_locale_t loc, int mode)
{

	memset(loc, 0, sizeof(*loc));
	loc->ctype.__mbsnrtowcs = pbsd_mbsnrtowcs_impl;
	loc->ctype.__wcsnrtombs = pbsd_wcsnrtombs_impl;
	loc->ctype.__mbrtowc = pbsd_mbrtowc_impl;
	loc->ctype.wcwidth_mode = mode;
}

} /* anonymous namespace */

extern "C" {

size_t
pbsd_mbrtowc(wchar_t *pwc, const char *s, size_t n, pbsd_mbstate_t *ps)
{
	return (pbsd_mbrtowc_impl(pwc, s, n, ps));
}

size_t
pbsd_wcrtomb(char *s, wchar_t wc, pbsd_mbstate_t *ps)
{
	unsigned char buf[MB_LEN_MAX];
	size_t produced;
	int err;

	(void)ps;
	if (s == NULL)
		return (1);
	err = utf8_encode(wc, buf, sizeof(buf), &produced);
	if (err != 0)
		return ((size_t)-1);
	memcpy(s, buf, produced);
	return (produced);
}

size_t
pbsd_mbsnrtowcs(wchar_t *dst, const char **src, size_t nms, size_t len,
    pbsd_mbstate_t *ps)
{
	return (pbsd_mbsnrtowcs_impl(dst, src, nms, len, ps));
}

size_t
pbsd_wcsnrtombs(char *dst, const wchar_t **src, size_t nwcs, size_t len,
    pbsd_mbstate_t *ps)
{
	return (pbsd_wcsnrtombs_impl(dst, src, nwcs, len, ps));
}

int
pbsd_wcwidth(wchar_t wc)
{
	return (pbsd_wcwidth_impl(wc));
}

int
pbsd_wcwidth_l(wchar_t wc, pbsd_locale_t locale)
{

	(void)locale;
	return (pbsd_wcwidth_impl(wc));
}

pbsd_locale_t
pbsd_get_active_locale()
{
	return (pbsd_active_locale);
}

void
pbsd_set_active_locale(pbsd_locale_t loc)
{
	pbsd_active_locale = loc;
}

void
pbsd_locale_init(pbsd_locale_t loc, int mode)
{
	pbsd_locale_init_impl(loc, mode);
}

} /* extern "C" */

export module pbsd.lib.libc.locale.b0039;

export namespace pbsd::lib_libc_locale::b0039 {

using pbsd_mbstate_t = ::pbsd_mbstate_t;
using pbsd_locale = ::pbsd_locale;
using pbsd_locale_t = ::pbsd_locale_t;

inline pbsd_locale_t
__get_locale()
{
	return (pbsd_get_active_locale());
}

inline int
__wcwidth(wchar_t wc)
{
	return (pbsd_wcwidth(wc));
}

inline int
__wcwidth_l(wchar_t wc, pbsd_locale_t locale)
{
	return (pbsd_wcwidth_l(wc, locale));
}

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

/* lib/libc/locale/mbsrtowcs.c */

size_t
mbsrtowcs_l(wchar_t * __restrict dst, const char ** __restrict src, size_t len,
    pbsd_mbstate_t * __restrict ps, pbsd_locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->mbsrtowcs);
	return (XLOCALE_CTYPE(locale)->__mbsnrtowcs(dst, src, SIZE_T_MAX, len, ps));
}
size_t
mbsrtowcs(wchar_t * __restrict dst, const char ** __restrict src, size_t len,
    pbsd_mbstate_t * __restrict ps)
{
	return mbsrtowcs_l(dst, src, len, ps, __get_locale());
}

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

/* lib/libc/locale/wcsrtombs.c */

size_t
wcsrtombs_l(char * __restrict dst, const wchar_t ** __restrict src, size_t len,
    pbsd_mbstate_t * __restrict ps, pbsd_locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->wcsrtombs);
	return (XLOCALE_CTYPE(locale)->__wcsnrtombs(dst, src, SIZE_T_MAX, len, ps));
}

size_t
wcsrtombs(char * __restrict dst, const wchar_t ** __restrict src, size_t len,
    pbsd_mbstate_t * __restrict ps)
{
	return wcsrtombs_l(dst, src, len, ps, __get_locale());
}

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

/* lib/libc/locale/mbtowc.c */

int
mbtowc_l(wchar_t * __restrict pwc, const char * __restrict s, size_t n, pbsd_locale_t locale)
{
	static const pbsd_mbstate_t initial{};
	size_t rval;
	FIX_LOCALE(locale);

	if (s == NULL) {
		/* No support for state dependent encodings. */
		XLOCALE_CTYPE(locale)->mbtowc = initial;
		return (0);
	}
	rval = XLOCALE_CTYPE(locale)->__mbrtowc(pwc, s, n,
	    &(XLOCALE_CTYPE(locale)->mbtowc));
	switch (rval) {
	case (size_t)-2:
		errno = EILSEQ;
		/* FALLTHROUGH */
	case (size_t)-1:
		return (-1);
	default:
		return ((int)rval);
	}
}
int
mbtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n)
{
	return mbtowc_l(pwc, s, n, __get_locale());
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
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

/* lib/libc/locale/wcwidth.c */

int
wcwidth(wchar_t wc)
{
	return (__wcwidth(wc));
}
int
wcwidth_l(wchar_t wc, pbsd_locale_t locale)
{
	return (__wcwidth_l(wc, locale));
}

using ::pbsd_locale_init;
using ::pbsd_set_active_locale;
using ::pbsd_get_active_locale;
using ::pbsd_wcrtomb;

} /* namespace pbsd::lib_libc_locale::b0039 */
