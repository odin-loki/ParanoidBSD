/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
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
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2004 Tim J. Robbins.
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
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2013 Garrett D'Amore <garrett@damore.org>
 * Copyright 2010 Nexenta Systems, Inc.  All rights reserved.
 * Copyright (c) 2002-2004 Tim J. Robbins.
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

module;

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>

#ifndef SIZE_T_MAX
#define SIZE_T_MAX	SIZE_MAX
#endif

#define	_CACHED_RUNES	(1 << 8)

export module pbsd.lib.libc.locale.b0078;

namespace pbsd::lib_libc_locale::b0078 {

using __ct_rune_t = int;
using __rune_t = __ct_rune_t;

struct _RuneEntry {
	__rune_t	__min;
	__rune_t	__max;
	__rune_t	__map;
	unsigned long	*__types;
};

struct _RuneRange {
	int		__nranges;
	_RuneEntry	*__ranges;
};

struct _RuneLocale {
	char		__magic[8];
	char		__encoding[32];
	__rune_t	(*__sgetrune)(const char *, std::size_t, char const **);
	int		(*__sputrune)(__rune_t, char *, std::size_t, char **);
	__rune_t	__invalid_rune;
	unsigned long	__runetype[_CACHED_RUNES];
	__rune_t	__maplower[_CACHED_RUNES];
	__rune_t	__mapupper[_CACHED_RUNES];
	_RuneRange	__runetype_ext;
	_RuneRange	__maplower_ext;
	_RuneRange	__mapupper_ext;
	void		*__variable;
	int		__variable_len;
};

using wctype_t = unsigned long;

struct mbstate_t {
	char		__mbstate8[128];
	long long	_mbstateL;
};

enum {
	XLC_CTYPE = 1,
};

struct xlocale_ctype {
	_RuneLocale	*runes;
	std::size_t	(*__mbsnrtowcs)(wchar_t * __restrict, const char ** __restrict,
		    std::size_t, std::size_t, mbstate_t * __restrict);
	int		__mb_cur_max;
	mbstate_t	mbsnrtowcs;
};

struct xlocale {
	void		*components[6];
};

using locale_t = xlocale *;

using mbrtowc_pfn_t = std::size_t (*)(wchar_t * __restrict,
    const char * __restrict, std::size_t, mbstate_t * __restrict);

locale_t current_locale = nullptr;

locale_t
__get_locale()
{
	return (current_locale);
}

static xlocale_ctype *
XLOCALE_CTYPE(locale_t l)
{
	return (static_cast<xlocale_ctype *>(l->components[XLC_CTYPE]));
}

#define FIX_LOCALE(l)	((l) = ((l) == nullptr ? __get_locale() : (l)))

static int	utf8_decode(const unsigned char *, std::size_t, wchar_t *,
		    std::size_t *);

static int
utf8_decode(const unsigned char *p, std::size_t n, wchar_t *wc,
    std::size_t *consumed)
{
	std::uint32_t c;

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
pbsd_test_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
    std::size_t n, mbstate_t * __restrict ps)
{
	wchar_t wc;
	std::size_t consumed;
	int err;

	(void)ps;
	if (n == 0)
		return ((std::size_t)-2);
	err = utf8_decode((const unsigned char *)s, n, &wc, &consumed);
	if (err == -1)
		return ((std::size_t)-1);
	if (err == -2)
		return ((std::size_t)-2);
	if (pwc != nullptr)
		*pwc = wc;
	if (wc == L'\0')
		return (0);
	return (consumed);
}

} /* namespace pbsd::lib_libc_locale::b0078 */

export namespace pbsd::lib_libc_locale::b0078 {

unsigned long
___runetype_l(__ct_rune_t c, locale_t locale)
{
	std::size_t lim;
	FIX_LOCALE(locale);
	_RuneRange *rr = &(XLOCALE_CTYPE(locale)->runes->__runetype_ext);
	_RuneEntry *base, *re;

	if (c < 0 || c == EOF)
		return(0L);

	/* Binary search -- see bsearch.c for explanation. */
	base = rr->__ranges;
	for (lim = rr->__nranges; lim != 0; lim >>= 1) {
		re = base + (lim >> 1);
		if (re->__min <= c && c <= re->__max) {
			if (re->__types)
			    return(re->__types[c - re->__min]);
			else
			    return(re->__map);
		} else if (c > re->__max) {
			base = re + 1;
			lim--;
		}
	}

	return(0L);
}
unsigned long
___runetype(__ct_rune_t c)
{
	return ___runetype_l(c, __get_locale());
}

int ___mb_cur_max(void)
{
	return XLOCALE_CTYPE(__get_locale())->__mb_cur_max;
}
int ___mb_cur_max_l(locale_t locale)
{
	FIX_LOCALE(locale);
	return XLOCALE_CTYPE(locale)->__mb_cur_max;
}

wint_t
nextwctype_l(wint_t wc, wctype_t wct, locale_t locale)
{
	std::size_t lim;
	FIX_LOCALE(locale);
	_RuneLocale *runes = XLOCALE_CTYPE(locale)->runes;
	_RuneRange *rr = &runes->__runetype_ext;
	_RuneEntry *base, *re;
	int noinc;

	noinc = 0;
	if (wc < _CACHED_RUNES) {
		wc++;
		while (wc < _CACHED_RUNES) {
			if (runes->__runetype[wc] & wct)
				return (wc);
			wc++;
		}
		wc--;
	}
	if (rr->__ranges != NULL && wc < rr->__ranges[0].__min) {
		wc = rr->__ranges[0].__min;
		noinc = 1;
	}

	/* Binary search -- see bsearch.c for explanation. */
	base = rr->__ranges;
	for (lim = rr->__nranges; lim != 0; lim >>= 1) {
		re = base + (lim >> 1);
		if (re->__min <= wc && wc <= re->__max)
			goto found;
		else if (wc > re->__max) {
			base = re + 1;
			lim--;
		}
	}
	return (-1);
found:
	if (!noinc)
		wc++;
	if (re->__min <= wc && wc <= re->__max) {
		if (re->__types != NULL) {
			for (; wc <= re->__max; wc++)
				if (re->__types[wc - re->__min] & wct)
					return (wc);
		} else if (re->__map & wct)
			return (wc);
	}
	while (++re < rr->__ranges + rr->__nranges) {
		wc = re->__min;
		if (re->__types != NULL) {
			for (; wc <= re->__max; wc++)
				if (re->__types[wc - re->__min] & wct)
					return (wc);
		} else if (re->__map & wct)
			return (wc);
	}
	return (-1);
}
wint_t
nextwctype(wint_t wc, wctype_t wct)
{
	return nextwctype_l(wc, wct, __get_locale());
}

std::size_t
mbsnrtowcs_l(wchar_t * __restrict dst, const char ** __restrict src,
    std::size_t nms, std::size_t len, mbstate_t * __restrict ps,
    locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->mbsnrtowcs);
	return (XLOCALE_CTYPE(locale)->__mbsnrtowcs(dst, src, nms, len, ps));
}
std::size_t
mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    std::size_t nms, std::size_t len, mbstate_t * __restrict ps)
{
	return mbsnrtowcs_l(dst, src, nms, len, ps, __get_locale());
}

std::size_t
__mbsnrtowcs_std(wchar_t * __restrict dst, const char ** __restrict src,
    std::size_t nms, std::size_t len, mbstate_t * __restrict ps,
    mbrtowc_pfn_t pmbrtowc)
{
	const char *s;
	std::size_t nchr;
	wchar_t wc;
	std::size_t nb;

	s = *src;
	nchr = 0;

	if (dst == NULL) {
		for (;;) {
			if ((nb = pmbrtowc(&wc, s, nms, ps)) == (std::size_t)-1)
				/* Invalid sequence - mbrtowc() sets errno. */
				return ((std::size_t)-1);
			else if (nb == 0 || nb == (std::size_t)-2)
				return (nchr);
			s += nb;
			nms -= nb;
			nchr++;
		}
		/*NOTREACHED*/
	}

	while (len-- > 0) {
		if ((nb = pmbrtowc(dst, s, nms, ps)) == (std::size_t)-1) {
			*src = s;
			return ((std::size_t)-1);
		} else if (nb == (std::size_t)-2) {
			*src = s + nms;
			return (nchr);
		} else if (nb == 0) {
			*src = NULL;
			return (nchr);
		}
		s += nb;
		nms -= nb;
		nchr++;
		dst++;
	}
	*src = s;
	return (nchr);
}

std::size_t
pbsd_backend_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    std::size_t nms, std::size_t len, mbstate_t * __restrict ps)
{

	return (__mbsnrtowcs_std(dst, src, nms, len, ps, pbsd_test_mbrtowc));
}

void
pbsd_set_active_locale(locale_t loc)
{
	current_locale = loc;
}

locale_t
pbsd_get_active_locale()
{
	return (current_locale);
}

void
pbsd_locale_init(locale_t loc, xlocale_ctype *ctype, _RuneLocale *runes,
    int mb_cur_max)
{

	std::memset(loc, 0, sizeof(*loc));
	std::memset(ctype, 0, sizeof(*ctype));
	loc->components[XLC_CTYPE] = ctype;
	ctype->runes = runes;
	ctype->__mb_cur_max = mb_cur_max;
	ctype->__mbsnrtowcs = pbsd_backend_mbsnrtowcs;
}

} /* export namespace */
