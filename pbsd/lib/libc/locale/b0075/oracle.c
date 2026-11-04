/*
 * Reference oracle for batch b0075.
 *
 * Original HardenedBSD sources concatenated, every function renamed with a
 * ref_ prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/locale/collcmp.c
 *   hbsd/src/lib/libc/locale/mbrtowc.c
 *   hbsd/src/lib/libc/locale/c32rtomb.c
 *   hbsd/src/lib/libc/locale/mbstowcs.c
 *
 * Private libc headers (collate.h, mblocal.h, xlocale_private.h) and the UTF-8
 * ctype backend are not part of this batch.  The declarations and mock
 * implementations below stand in for them so the unmodified function bodies
 * compile and link.  FreeBSD uses a 128-byte mbstate_t; glibc's is smaller, so
 * __mbstate_t is defined before <wchar.h> to match FreeBSD.
 */

#ifndef SIZE_T_MAX
#define SIZE_T_MAX	((size_t)-1)
#endif

#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * CHAR_BIT)
#endif

#define __mbstate_t_defined 1
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} __mbstate_t;
typedef __mbstate_t mbstate_t;

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <uchar.h>
#include <wchar.h>

#ifndef MB_LEN_MAX
#define MB_LEN_MAX	4
#endif

#ifndef MB_CUR_MAX
#define MB_CUR_MAX	4
#endif

#define MIN(a, b)	((a) < (b) ? (a) : (b))

typedef struct xlocale *locale_t;

typedef struct {
	wchar_t	ch;
	int	want;
	wchar_t	lbound;
} _UTF8State;

enum {
	XLC_CTYPE = 1,
};

struct xlocale_ctype {
	size_t (*__mbrtowc)(wchar_t * __restrict, const char * __restrict,
	    size_t, mbstate_t * __restrict);
	size_t (*__wcrtomb)(char * __restrict, wchar_t, mbstate_t * __restrict);
	size_t (*__mbsnrtowcs)(wchar_t * __restrict, const char ** __restrict,
	    size_t, size_t, mbstate_t * __restrict);
	mbstate_t	mbrtowc;
	mbstate_t	c32rtomb;
	mbstate_t	wcrtomb;
};

struct xlocale {
	void		*components[6];
};

#define XLOCALE_CTYPE(x)	((struct xlocale_ctype *)(x)->components[XLC_CTYPE])

struct xlocale_ctype	ref_global_ctype;
struct xlocale		ref_global_locale;

static size_t	mock_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
static size_t	mock_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);
static size_t	mock_mbsnrtowcs(wchar_t * __restrict, const char ** __restrict,
		    size_t, size_t, mbstate_t * __restrict);
static int	utf8_decode(const unsigned char *, size_t, wchar_t *,
		    size_t *);

locale_t
ref_get_locale(void)
{

	return (&ref_global_locale);
}

static inline locale_t
ref_fix_locale(locale_t l)
{

	if (l == NULL)
		return (&ref_global_locale);
	return (l);
}

#define FIX_LOCALE(l)	((l) = ref_fix_locale(l))
#define __get_locale()	ref_get_locale()

static void __attribute__((constructor))
ref_locale_init(void)
{

	ref_global_ctype.__mbrtowc = mock_mbrtowc;
	ref_global_ctype.__wcrtomb = mock_wcrtomb;
	ref_global_ctype.__mbsnrtowcs = mock_mbsnrtowcs;
	memset(&ref_global_ctype.mbrtowc, 0, sizeof(ref_global_ctype.mbrtowc));
	memset(&ref_global_ctype.c32rtomb, 0, sizeof(ref_global_ctype.c32rtomb));
	memset(&ref_global_ctype.wcrtomb, 0, sizeof(ref_global_ctype.wcrtomb));
	ref_global_locale.components[XLC_CTYPE] = &ref_global_ctype;
}

static size_t
mock_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps)
{
	_UTF8State *us;
	int ch, i, mask, want;
	wchar_t lbound, wch;

	us = (_UTF8State *)ps;

	if (us->want < 0 || us->want > 6) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL) {
		s = "";
		n = 1;
		pwc = NULL;
	}

	if (n == 0)
		return ((size_t)-2);

	if (us->want == 0) {
		ch = (unsigned char)*s;
		if ((ch & 0x80) == 0) {
			if (pwc != NULL)
				*pwc = ch;
			return (ch != '\0' ? 1 : 0);
		}
		if ((ch & 0xe0) == 0xc0) {
			mask = 0x1f;
			want = 2;
			lbound = 0x80;
		} else if ((ch & 0xf0) == 0xe0) {
			mask = 0x0f;
			want = 3;
			lbound = 0x800;
		} else if ((ch & 0xf8) == 0xf0) {
			mask = 0x07;
			want = 4;
			lbound = 0x10000;
		} else {
			errno = EILSEQ;
			return ((size_t)-1);
		}
	} else {
		want = us->want;
		lbound = us->lbound;
	}

	if (us->want == 0)
		wch = (unsigned char)*s++ & mask;
	else
		wch = us->ch;

	for (i = (us->want == 0) ? 1 : 0; i < MIN(want, n); i++) {
		if ((*s & 0xc0) != 0x80) {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wch <<= 6;
		wch |= *s++ & 0x3f;
	}
	if (i < want) {
		us->want = want - i;
		us->lbound = lbound;
		us->ch = wch;
		return ((size_t)-2);
	}
	if (wch < lbound) {
		errno = EILSEQ;
		return ((size_t)-1);
	}
	if ((wch >= 0xd800 && wch <= 0xdfff) || wch > 0x10ffff) {
		errno = EILSEQ;
		return ((size_t)-1);
	}
	if (pwc != NULL)
		*pwc = wch;
	us->want = 0;
	return (wch == 0 ? 0 : want);
}

static size_t
mock_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	_UTF8State *us;
	unsigned char lead;
	int i, len;

	us = (_UTF8State *)ps;

	if (us->want != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		return (1);

	if ((wc & ~0x7f) == 0) {
		*s = (char)wc;
		return (1);
	} else if ((wc & ~0x7ff) == 0) {
		lead = 0xc0;
		len = 2;
	} else if ((wc & ~0xffff) == 0) {
		if (wc >= 0xd800 && wc <= 0xdfff) {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		lead = 0xe0;
		len = 3;
	} else if (wc >= 0 && wc <= 0x10ffff) {
		lead = 0xf0;
		len = 4;
	} else {
		errno = EILSEQ;
		return ((size_t)-1);
	}

	for (i = len - 1; i > 0; i--) {
		s[i] = (wc & 0x3f) | 0x80;
		wc >>= 6;
	}
	*s = (wc & 0xff) | lead;

	return (len);
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

static size_t
mock_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
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
wcrtomb_l(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps,
		locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->wcrtomb);
	return (XLOCALE_CTYPE(locale)->__wcrtomb(s, wc, ps));
}

/* collcmp.c */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (C) 1996 by Andrey A. Chernov, Moscow, Russia.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND
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
ref___collate_range_cmp(char c1, char c2)
{
	char s1[2], s2[2];

	s1[0] = c1;
	s1[1] = '\0';
	s2[0] = c2;
	s2[1] = '\0';
	return (strcoll(s1, s2));
}

int
ref___wcollate_range_cmp(wchar_t c1, wchar_t c2)
{
	wchar_t s1[2], s2[2];

	s1[0] = c1;
	s1[1] = L'\0';
	s2[0] = c2;
	s2[1] = L'\0';
	return (wcscoll(s1, s2));
}

/* mbrtowc.c */

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

size_t
ref_mbrtowc_l(wchar_t * __restrict pwc, const char * __restrict s,
    size_t n, mbstate_t * __restrict ps, locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->mbrtowc);
	return (XLOCALE_CTYPE(locale)->__mbrtowc(pwc, s, n, ps));
}

size_t
ref_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
    size_t n, mbstate_t * __restrict ps)
{
	return ref_mbrtowc_l(pwc, s, n, ps, __get_locale());
}

/* c32rtomb.c */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2013 Ed Schouten <ed@FreeBSD.org>
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

size_t
ref_c32rtomb_l(char * __restrict s, char32_t c32, mbstate_t * __restrict ps,
    locale_t locale)
{

	/* Unicode Standard 5.0, D90: ill-formed characters. */
	if ((c32 >= 0xd800 && c32 <= 0xdfff) || c32 > 0x10ffff) {
		errno = EILSEQ;
		return ((size_t)-1);
	}

	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->c32rtomb);

	/* Assume wchar_t uses UTF-32. */
	return (wcrtomb_l(s, c32, ps, locale));
}

size_t
ref_c32rtomb(char * __restrict s, char32_t c32, mbstate_t * __restrict ps)
{

	return (ref_c32rtomb_l(s, c32, ps, __get_locale()));
}

/* mbstowcs.c */

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

size_t
ref_mbstowcs_l(wchar_t * __restrict pwcs, const char * __restrict s, size_t n, locale_t locale)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	const char *sp;
	FIX_LOCALE(locale);

	mbs = initial;
	sp = s;
	return (XLOCALE_CTYPE(locale)->__mbsnrtowcs(pwcs, &sp, SIZE_T_MAX, n, &mbs));
}
size_t
ref_mbstowcs(wchar_t * __restrict pwcs, const char * __restrict s, size_t n)
{
	return ref_mbstowcs_l(pwcs, s, n, __get_locale());
}
