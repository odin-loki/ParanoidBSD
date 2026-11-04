module;

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

export module pbsd.lib.libc.locale.b0075;

extern "C" {
#define __mbstate_t_defined 1
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} __mbstate_t;
typedef __mbstate_t mbstate_t;

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

typedef struct {
	wchar_t	ch;
	int	want;
	wchar_t	lbound;
} _UTF8State;

enum {
	PORT_XLC_CTYPE = 1,
};

struct port_xlocale_ctype {
	std::size_t (*__mbrtowc)(wchar_t * __restrict, const char * __restrict,
	    std::size_t, mbstate_t * __restrict);
	std::size_t (*__wcrtomb)(char * __restrict, wchar_t,
	    mbstate_t * __restrict);
	std::size_t (*__mbsnrtowcs)(wchar_t * __restrict, const char ** __restrict,
	    std::size_t, std::size_t, mbstate_t * __restrict);
	mbstate_t	mbrtowc;
	mbstate_t	c32rtomb;
	mbstate_t	wcrtomb;
};

struct port_xlocale {
	void		*components[6];
};

using port_locale_t = port_xlocale *;

port_xlocale_ctype	port_global_ctype;
port_xlocale		port_global_locale;

std::size_t	port_mbrtowc(wchar_t * __restrict,
		    const char * __restrict, std::size_t, mbstate_t * __restrict);
std::size_t	port_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);
std::size_t	port_mbsnrtowcs(wchar_t * __restrict,
		    const char ** __restrict, std::size_t, std::size_t,
		    mbstate_t * __restrict);
static int		port_utf8_decode(const unsigned char *, std::size_t,
		    wchar_t *, std::size_t *);

port_locale_t
port_get_locale()
{

	return (&port_global_locale);
}

static inline port_locale_t
port_fix_locale(port_locale_t l)
{

	if (l == nullptr)
		return (&port_global_locale);
	return (l);
}

#define FIX_LOCALE(l)	((l) = port_fix_locale(l))

static port_xlocale_ctype *
XLOCALE_CTYPE(port_locale_t l)
{

	return (static_cast<port_xlocale_ctype *>(l->components[PORT_XLC_CTYPE]));
}

std::size_t
port_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, std::size_t n,
    mbstate_t * __restrict ps)
{
	_UTF8State *us;
	int ch, i, mask, want;
	wchar_t lbound, wch;

	us = (_UTF8State *)ps;

	if (us->want < 0 || us->want > 6) {
		errno = EINVAL;
		return ((std::size_t)-1);
	}

	if (s == nullptr) {
		s = "";
		n = 1;
		pwc = nullptr;
	}

	if (n == 0)
		return ((std::size_t)-2);

	if (us->want == 0) {
		ch = (unsigned char)*s;
		if ((ch & 0x80) == 0) {
			if (pwc != nullptr)
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
			return ((std::size_t)-1);
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
			return ((std::size_t)-1);
		}
		wch <<= 6;
		wch |= *s++ & 0x3f;
	}
	if (i < want) {
		us->want = want - i;
		us->lbound = lbound;
		us->ch = wch;
		return ((std::size_t)-2);
	}
	if (wch < lbound) {
		errno = EILSEQ;
		return ((std::size_t)-1);
	}
	if ((wch >= 0xd800 && wch <= 0xdfff) || wch > 0x10ffff) {
		errno = EILSEQ;
		return ((std::size_t)-1);
	}
	if (pwc != nullptr)
		*pwc = wch;
	us->want = 0;
	return (wch == L'\0' ? 0 : want);
}

std::size_t
port_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	_UTF8State *us;
	unsigned char lead;
	int i, len;

	us = (_UTF8State *)ps;

	if (us->want != 0) {
		errno = EINVAL;
		return ((std::size_t)-1);
	}

	if (s == nullptr)
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
			return ((std::size_t)-1);
		}
		lead = 0xe0;
		len = 3;
	} else if (wc >= 0 && wc <= 0x10ffff) {
		lead = 0xf0;
		len = 4;
	} else {
		errno = EILSEQ;
		return ((std::size_t)-1);
	}

	for (i = len - 1; i > 0; i--) {
		s[i] = (wc & 0x3f) | 0x80;
		wc >>= 6;
	}
	*s = (wc & 0xff) | lead;

	return (len);
}

static int
port_utf8_decode(const unsigned char *p, std::size_t n, wchar_t *wc,
    std::size_t *consumed)
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

std::size_t
port_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    std::size_t nms, std::size_t len, mbstate_t * __restrict ps)
{
	const unsigned char *s;
	std::size_t nconv, nwritten, consumed;
	wchar_t wc;
	int err;

	(void)ps;
	if (src == nullptr)
		return ((std::size_t)-1);
	s = (const unsigned char *)*src;
	if (s == nullptr)
		return ((std::size_t)-1);
	nconv = 0;
	nwritten = 0;
	while (nms > 0) {
		if (*s == '\0') {
			if (dst != nullptr && len > 0) {
				if (nwritten >= len)
					break;
				dst[nwritten++] = L'\0';
			}
			*src = (const char *)s;
			return (nconv);
		}
		err = port_utf8_decode(s, nms, &wc, &consumed);
		if (err == -1)
			return ((std::size_t)-1);
		if (err == -2)
			break;
		if (dst != nullptr) {
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

std::size_t
wcrtomb_l(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps,
		port_locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == nullptr)
		ps = &(XLOCALE_CTYPE(locale)->wcrtomb);
	return (XLOCALE_CTYPE(locale)->__wcrtomb(s, wc, ps));
}

port_locale_t
__get_locale()
{

	return (port_get_locale());
}

}

export namespace pbsd::lib_libc_locale::b0075 {

using mbstate_t = ::mbstate_t;
using port_locale_t = ::port_locale_t;

inline void
init_locale()
{
	port_global_ctype.__mbrtowc = port_mbrtowc;
	port_global_ctype.__wcrtomb = port_wcrtomb;
	port_global_ctype.__mbsnrtowcs = port_mbsnrtowcs;
	memset(&port_global_ctype.mbrtowc, 0, sizeof(port_global_ctype.mbrtowc));
	memset(&port_global_ctype.c32rtomb, 0, sizeof(port_global_ctype.c32rtomb));
	memset(&port_global_ctype.wcrtomb, 0, sizeof(port_global_ctype.wcrtomb));
	port_global_locale.components[PORT_XLC_CTYPE] = &port_global_ctype;
}

inline port_locale_t
global_locale()
{

	return (&port_global_locale);
}

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

/*
 * Compare two characters using collate
 */

int __collate_range_cmp(char c1, char c2)
{
	char s1[2], s2[2];

	s1[0] = c1;
	s1[1] = '\0';
	s2[0] = c2;
	s2[1] = '\0';
	return (strcoll(s1, s2));
}

int __wcollate_range_cmp(wchar_t c1, wchar_t c2)
{
	wchar_t s1[2], s2[2];

	s1[0] = c1;
	s1[1] = L'\0';
	s2[0] = c2;
	s2[1] = L'\0';
	return (wcscoll(s1, s2));
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

std::size_t
mbrtowc_l(wchar_t * __restrict pwc, const char * __restrict s,
    std::size_t n, mbstate_t * __restrict ps, port_locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == nullptr)
		ps = &(XLOCALE_CTYPE(locale)->mbrtowc);
	return (XLOCALE_CTYPE(locale)->__mbrtowc(pwc, s, n, ps));
}

std::size_t
mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
    std::size_t n, mbstate_t * __restrict ps)
{
	return mbrtowc_l(pwc, s, n, ps, __get_locale());
}

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

std::size_t
c32rtomb_l(char * __restrict s, char32_t c32, mbstate_t * __restrict ps,
    port_locale_t locale)
{

	/* Unicode Standard 5.0, D90: ill-formed characters. */
	if ((c32 >= 0xd800 && c32 <= 0xdfff) || c32 > 0x10ffff) {
		errno = EILSEQ;
		return ((std::size_t)-1);
	}

	FIX_LOCALE(locale);
	if (ps == nullptr)
		ps = &(XLOCALE_CTYPE(locale)->c32rtomb);

	/* Assume wchar_t uses UTF-32. */
	return (wcrtomb_l(s, c32, ps, locale));
}

std::size_t
c32rtomb(char * __restrict s, char32_t c32, mbstate_t * __restrict ps)
{

	return (c32rtomb_l(s, c32, ps, __get_locale()));
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

std::size_t
mbstowcs_l(wchar_t * __restrict pwcs, const char * __restrict s, std::size_t n,
    port_locale_t locale)
{
	static const mbstate_t initial{};
	mbstate_t mbs;
	const char *sp;
	FIX_LOCALE(locale);

	mbs = initial;
	sp = s;
	return (XLOCALE_CTYPE(locale)->__mbsnrtowcs(pwcs, &sp, SIZE_MAX, n, &mbs));
}

std::size_t
mbstowcs(wchar_t * __restrict pwcs, const char * __restrict s, std::size_t n)
{
	return mbstowcs_l(pwcs, s, n, __get_locale());
}

}
