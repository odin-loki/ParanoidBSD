/*
 * pbsd.lib.libc.locale.b0153s4 - C++23 port of batch b0153s4.
 *
 * Source:
 *   hbsd/src/lib/libc/locale/gb2312.c
 */

module;

#include <sys/types.h>

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

export module pbsd.lib.libc.locale.b0153s4;

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

export namespace pbsd::lib_libc_locale::b0153s4 {

typedef int __ct_rune_t;
typedef __ct_rune_t __rune_t;

typedef struct {
	__rune_t	__min;
	__rune_t	__max;
	__rune_t	__map;
	unsigned long	*__types;
} _RuneEntry;

typedef struct {
	int		__nranges;
	_RuneEntry	*__ranges;
} _RuneRange;

typedef struct {
	char		__magic[8];
	char		__encoding[32];
	__rune_t	(*__sgetrune)(const char *, size_t, char const **);
	int		(*__sputrune)(__rune_t, char *, size_t, char **);
	__rune_t	__invalid_rune;
	unsigned long	__runetype[256];
	__rune_t	__maplower[256];
	__rune_t	__mapupper[256];
	_RuneRange	__runetype_ext;
	_RuneRange	__maplower_ext;
	_RuneRange	__mapupper_ext;
	void		*__variable;
	int		__variable_len;
} _RuneLocale;

struct xlocale_ctype {
	_RuneLocale	*runes;
	size_t		(*__mbrtowc)(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
	int		(*__mbsinit)(const mbstate_t *);
	size_t		(*__mbsnrtowcs)(wchar_t * __restrict, const char ** __restrict,
		    size_t, size_t, mbstate_t * __restrict);
	size_t		(*__wcrtomb)(char * __restrict, wchar_t, mbstate_t * __restrict);
	size_t		(*__wcsnrtombs)(char * __restrict, const wchar_t ** __restrict,
		    size_t, size_t, mbstate_t * __restrict);
	int		__mb_cur_max;
	int		__mb_sb_limit;
};

typedef size_t (*mbrtowc_pfn_t)(wchar_t * __restrict,
    const char * __restrict, size_t, mbstate_t * __restrict);
typedef size_t (*wcrtomb_pfn_t)(char * __restrict, wchar_t,
    mbstate_t * __restrict);

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

#ifndef MB_LEN_MAX
#define MB_LEN_MAX 4
#endif

typedef struct {
	int	count;
	u_char	bytes[2];
} GB2312State;

size_t	GB2312_mbrtowc(wchar_t * __restrict, const char * __restrict, size_t,
	    mbstate_t * __restrict);
int	GB2312_mbsinit(const mbstate_t *);
size_t	GB2312_wcrtomb(char * __restrict, wchar_t, mbstate_t * __restrict);
size_t	GB2312_mbsnrtowcs(wchar_t * __restrict, const char ** __restrict,
	    size_t, size_t, mbstate_t * __restrict);
size_t	GB2312_wcsnrtombs(char * __restrict, const wchar_t ** __restrict,
	    size_t, size_t, mbstate_t * __restrict);

static size_t
mbsnrtowcs_std(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps,
    mbrtowc_pfn_t pmbrtowc)
{
	const char *s;
	size_t nchr;
	wchar_t wc;
	size_t nb;

	s = *src;
	nchr = 0;

	if (dst == NULL) {
		for (;;) {
			if ((nb = pmbrtowc(&wc, s, nms, ps)) == (size_t)-1)
				return ((size_t)-1);
			else if (nb == 0 || nb == (size_t)-2)
				return (nchr);
			s += nb;
			nms -= nb;
			nchr++;
		}
	}

	while (len-- > 0) {
		if ((nb = pmbrtowc(dst, s, nms, ps)) == (size_t)-1) {
			*src = s;
			return ((size_t)-1);
		} else if (nb == (size_t)-2) {
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

static size_t
wcsnrtombs_std(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps,
    wcrtomb_pfn_t pwcrtomb)
{
	mbstate_t mbsbak;
	char buf[MB_LEN_MAX];
	const wchar_t *s;
	size_t nbytes;
	size_t nb;

	s = *src;
	nbytes = 0;

	if (dst == NULL) {
		while (nwc-- > 0) {
			if ((nb = pwcrtomb(buf, *s, ps)) == (size_t)-1)
				return ((size_t)-1);
			else if (*s == L'\0')
				return (nbytes + nb - 1);
			s++;
			nbytes += nb;
		}
		return (nbytes);
	}

	while (len > 0 && nwc-- > 0) {
		if (len > (size_t)MB_CUR_MAX) {
			if ((nb = pwcrtomb(dst, *s, ps)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
		} else {
			mbsbak = *ps;
			if ((nb = pwcrtomb(buf, *s, ps)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
			if (nb > (int)len) {
				*ps = mbsbak;
				break;
			}
			memcpy(dst, buf, nb);
		}
		if (*s == L'\0') {
			*src = NULL;
			return (nbytes + nb - 1);
		}
		s++;
		dst += nb;
		len -= nb;
		nbytes += nb;
	}
	*src = s;
	return (nbytes);
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

int
GB2312_init(struct xlocale_ctype *l, _RuneLocale *rl)
{

	l->runes = rl;
	l->__mbrtowc = GB2312_mbrtowc;
	l->__wcrtomb = GB2312_wcrtomb;
	l->__mbsinit = GB2312_mbsinit;
	l->__mbsnrtowcs = GB2312_mbsnrtowcs;
	l->__wcsnrtombs = GB2312_wcsnrtombs;
	l->__mb_cur_max = 2;
	l->__mb_sb_limit = 128;
	return (0);
}

int
GB2312_mbsinit(const mbstate_t *ps)
{

	return (ps == NULL || ((const GB2312State *)ps)->count == 0);
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

size_t
GB2312_mbsnrtowcs(wchar_t * __restrict dst,
    const char ** __restrict src, size_t nms, size_t len,
    mbstate_t * __restrict ps)
{
	return (mbsnrtowcs_std(dst, src, nms, len, ps, GB2312_mbrtowc));
}

size_t
GB2312_wcsnrtombs(char * __restrict dst,
    const wchar_t ** __restrict src, size_t nwc, size_t len,
    mbstate_t * __restrict ps)
{
	return (wcsnrtombs_std(dst, src, nwc, len, ps, GB2312_wcrtomb));
}

} /* export namespace pbsd::lib_libc_locale::b0153s4 */
