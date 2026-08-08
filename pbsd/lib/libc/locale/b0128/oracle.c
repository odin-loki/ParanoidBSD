/*
 * Reference oracle for batch b0128.
 *
 * Original HardenedBSD sources concatenated, every function renamed with a
 * ref_ prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/locale/wcstof.c
 *   hbsd/src/lib/libc/locale/wcstold.c
 *   hbsd/src/lib/libc/locale/wctrans.c
 *   hbsd/src/lib/libc/locale/mbrtoc16.c
 *
 * Private libc headers (mblocal.h, xlocale_private.h) and locale-runtime
 * helpers are not part of this batch.  The declarations and mock
 * implementations below stand in for them so the unmodified function bodies
 * compile and link.
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
#include <sys/types.h>
#include <wchar.h>
#include <wctype.h>

typedef uint16_t char16_t;
typedef uint32_t char32_t;

#ifndef MB_LEN_MAX
#define MB_LEN_MAX	4
#endif

#define MIN(a, b)	((a) < (b) ? (a) : (b))

typedef struct {
	wchar_t	ch;
	int	want;
	wchar_t	lbound;
} _UTF8State;

typedef int wctrans_t;

enum {
	XLC_CTYPE = 1,
};

struct xlocale_ctype {
	size_t (*__mbrtowc)(wchar_t * __restrict, const char * __restrict,
	    size_t, mbstate_t * __restrict);
	size_t (*__wcsnrtombs)(char * __restrict, const wchar_t ** __restrict,
	    size_t, size_t, mbstate_t * __restrict);
	mbstate_t	mbrtoc16;
	mbstate_t	mbrtoc32;
	mbstate_t	wcsrtombs;
};

struct xlocale {
	void		*components[6];
};

typedef struct xlocale *locale_t;

#define XLOCALE_CTYPE(x)	((struct xlocale_ctype *)(x)->components[XLC_CTYPE])

struct xlocale_ctype	ref_global_ctype;
struct xlocale		ref_global_locale;

static size_t	mock_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
static size_t	mock_wcrtomb(char * __restrict, wchar_t, mbstate_t * __restrict);
static size_t	mock_wcsnrtombs(char * __restrict, const wchar_t ** __restrict,
		    size_t, size_t, mbstate_t * __restrict);

unsigned long	ref_malloc_calls;
int		ref_malloc_fail_at;

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

void
ref_reset_malloc(void)
{

	ref_malloc_calls = 0;
	ref_malloc_fail_at = 0;
}

void
ref_set_malloc_fail_at(int at)
{

	ref_malloc_calls = 0;
	ref_malloc_fail_at = at;
}

void
ref_reset_locale_states(void)
{

	memset(&ref_global_ctype.mbrtoc16, 0, sizeof(ref_global_ctype.mbrtoc16));
	memset(&ref_global_ctype.mbrtoc32, 0, sizeof(ref_global_ctype.mbrtoc32));
	memset(&ref_global_ctype.wcsrtombs, 0, sizeof(ref_global_ctype.wcsrtombs));
}

static void * __attribute__((used))
ref_libc_malloc(size_t n)
{

	ref_malloc_calls++;
	if (ref_malloc_fail_at != 0 &&
	    ref_malloc_calls == (unsigned long)ref_malloc_fail_at)
		return (NULL);
	return (malloc(n));
}

#define malloc ref_libc_malloc

static void __attribute__((constructor))
ref_locale_init(void)
{

	ref_global_ctype.__mbrtowc = mock_mbrtowc;
	ref_global_ctype.__wcsnrtombs = mock_wcsnrtombs;
	memset(&ref_global_ctype.mbrtoc16, 0, sizeof(ref_global_ctype.mbrtoc16));
	memset(&ref_global_ctype.mbrtoc32, 0, sizeof(ref_global_ctype.mbrtoc32));
	memset(&ref_global_ctype.wcsrtombs, 0, sizeof(ref_global_ctype.wcsrtombs));
	ref_global_locale.components[XLC_CTYPE] = &ref_global_ctype;
}

size_t
mbrtowc_l(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps, locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->mbrtoc32);
	return (XLOCALE_CTYPE(locale)->__mbrtowc(pwc, s, n, ps));
}

size_t
mbrtoc32_l(char32_t * __restrict pc32, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps, locale_t locale)
{

	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->mbrtoc32);

	/* Assume wchar_t uses UTF-32. */
	return (mbrtowc_l((wchar_t *)(void *)pc32, s, n, ps, locale));
}

size_t
wcsrtombs_l(char * __restrict dst, const wchar_t ** __restrict src, size_t len,
    mbstate_t * __restrict ps, locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->wcsrtombs);
	return (XLOCALE_CTYPE(locale)->__wcsnrtombs(dst, src, SIZE_T_MAX, len, ps));
}

int
iswspace_l(wint_t wc, locale_t locale)
{

	(void)locale;
	return (iswspace(wc));
}

float
strtof_l(const char * __restrict nptr, char ** __restrict endptr,
    locale_t locale)
{

	(void)locale;
	return (strtof(nptr, endptr));
}

long double
strtold_l(const char * __restrict nptr, char ** __restrict endptr,
    locale_t locale)
{

	(void)locale;
	return (strtold(nptr, endptr));
}

wint_t
towlower_l(wint_t wc, locale_t locale)
{

	(void)locale;
	if (wc >= L'A' && wc <= L'Z')
		return (wc - L'A' + L'a');
	return (wc);
}

wint_t
towupper_l(wint_t wc, locale_t locale)
{

	(void)locale;
	if (wc >= L'a' && wc <= L'z')
		return (wc - L'a' + L'A');
	return (wc);
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
	int i, blen;

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
		blen = 2;
	} else if ((wc & ~0xffff) == 0) {
		if (wc >= 0xd800 && wc <= 0xdfff) {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		lead = 0xe0;
		blen = 3;
	} else if (wc >= 0 && wc <= 0x10ffff) {
		lead = 0xf0;
		blen = 4;
	} else {
		errno = EILSEQ;
		return ((size_t)-1);
	}

	for (i = blen - 1; i > 0; i--) {
		s[i] = (wc & 0x3f) | 0x80;
		wc >>= 6;
	}
	*s = (wc & 0xff) | lead;

	return (blen);
}

static size_t
mock_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	mbstate_t mbsbak;
	char buf[MB_LEN_MAX];
	const wchar_t *s;
	size_t nbytes;
	size_t nb;

	if (src == NULL)
		return ((size_t)-1);
	s = *src;
	if (s == NULL)
		return ((size_t)-1);
	nbytes = 0;

	if (dst == NULL) {
		while (nwc-- > 0) {
			if ((nb = mock_wcrtomb(buf, *s, ps)) == (size_t)-1)
				return ((size_t)-1);
			else if (*s == L'\0')
				return (nbytes + nb - 1);
			s++;
			nbytes += nb;
		}
		return (nbytes);
	}

	while (len > 0 && nwc-- > 0) {
		if (len > (size_t)MB_LEN_MAX) {
			if ((nb = mock_wcrtomb(dst, *s, ps)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
		} else {
			mbsbak = *ps;
			if ((nb = mock_wcrtomb(buf, *s, ps)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
			if (nb > len) {
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

/* wcstof.c */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002, 2003 Tim J. Robbins
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

float
ref_wcstof_l(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr,
		locale_t locale)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	float val;
	char *buf, *end;
	const wchar_t *wcp;
	size_t len;
	size_t spaces;
	FIX_LOCALE(locale);

	wcp = nptr;
	spaces = 0;
	while (iswspace_l(*wcp, locale)) {
		wcp++;
		spaces++;
	}

	mbs = initial;
	if ((len = wcsrtombs_l(NULL, &wcp, 0, &mbs, locale)) == (size_t)-1) {
		if (endptr != NULL)
			*endptr = (wchar_t *)nptr;
		return (0.0);
	}
	if ((buf = malloc(len + 1)) == NULL) {
		if (endptr != NULL)
			*endptr = (wchar_t *)nptr;
		return (0.0);
	}
	mbs = initial;
	wcsrtombs_l(buf, &wcp, len + 1, &mbs, locale);

	val = strtof_l(buf, &end, locale);

	if (endptr != NULL) {
		*endptr = (wchar_t *)nptr + (end - buf);
		if (buf != end)
			*endptr += spaces;
	}

	free(buf);

	return (val);
}
float
ref_wcstof(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr)
{
	return ref_wcstof_l(nptr, endptr, __get_locale());
}

/* wcstold.c */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002, 2003 Tim J. Robbins
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

long double
ref_wcstold_l(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr,
		locale_t locale)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	long double val;
	char *buf, *end;
	const wchar_t *wcp;
	size_t len;
	size_t spaces;
	FIX_LOCALE(locale);

	wcp = nptr;
	spaces = 0;
	while (iswspace_l(*wcp, locale)) {
		wcp++;
		spaces++;
	}

	mbs = initial;
	if ((len = wcsrtombs_l(NULL, &wcp, 0, &mbs, locale)) == (size_t)-1) {
		if (endptr != NULL)
			*endptr = (wchar_t *)nptr;
		return (0.0);
	}
	if ((buf = malloc(len + 1)) == NULL) {
		if (endptr != NULL)
			*endptr = (wchar_t *)nptr;
		return (0.0);
	}
	mbs = initial;
	wcsrtombs_l(buf, &wcp, len + 1, &mbs, locale);

	val = strtold_l(buf, &end, locale);

	if (endptr != NULL) {
		*endptr = (wchar_t *)nptr + (end - buf);
		if (buf != end)
			*endptr += spaces;
	}

	free(buf);

	return (val);
}
long double
ref_wcstold(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr)
{
	return ref_wcstold_l(nptr, endptr, __get_locale());
}

/* wctrans.c */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Tim J. Robbins.
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

enum {
	_WCT_ERROR	= 0,
	_WCT_TOLOWER	= 1,
	_WCT_TOUPPER	= 2
};

wint_t
ref_towctrans_l(wint_t wc, wctrans_t desc, locale_t locale)
{
	switch (desc) {
	case _WCT_TOLOWER:
		wc = towlower_l(wc, locale);
		break;
	case _WCT_TOUPPER:
		wc = towupper_l(wc, locale);
		break;
	case _WCT_ERROR:
	default:
		errno = EINVAL;
		break;
	}

	return (wc);
}
wint_t
ref_towctrans(wint_t wc, wctrans_t desc)
{
	return ref_towctrans_l(wc, desc, __get_locale());
}

wctrans_t
ref_wctrans_l(const char *charclass, locale_t locale)
{
	struct {
		const char	*name;
		wctrans_t	 trans;
	} ccls[] = {
		{ "tolower",	_WCT_TOLOWER },
		{ "toupper",	_WCT_TOUPPER },
		{ NULL,		_WCT_ERROR },		/* Default */
	};
	int i;

	i = 0;
	while (ccls[i].name != NULL && strcmp(ccls[i].name, charclass) != 0)
		i++;

	if (ccls[i].trans == _WCT_ERROR)
		errno = EINVAL;
	return (ccls[i].trans);
}

wctrans_t
ref_wctrans(const char *charclass)
{
	return ref_wctrans_l(charclass, 0);
}

/* mbrtoc16.c */

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

typedef struct {
	char16_t	trail_surrogate;
	mbstate_t	c32_mbstate;
} _Char16State;

size_t
ref_mbrtoc16_l(char16_t * __restrict pc16, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps, locale_t locale)
{
	_Char16State *cs;
	char32_t c32;
	ssize_t len;

	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->mbrtoc16);
	cs = (_Char16State *)ps;

	/*
	 * Call straight into mbrtoc32_l() if we don't need to return a
	 * character value. According to the spec, if s is a null
	 * pointer, the value of parameter pc16 is also ignored.
	 */
	if (pc16 == NULL || s == NULL) {
		cs->trail_surrogate = 0;
		return (mbrtoc32_l(NULL, s, n, &cs->c32_mbstate, locale));
	}

	/* Return the trail surrogate from the previous invocation. */
	if (cs->trail_surrogate >= 0xdc00 && cs->trail_surrogate <= 0xdfff) {
		*pc16 = cs->trail_surrogate;
		cs->trail_surrogate = 0;
		return ((size_t)-3);
	}

	len = mbrtoc32_l(&c32, s, n, &cs->c32_mbstate, locale);
	if (len >= 0) {
		if (c32 < 0x10000) {
			/* Fits in one UTF-16 character. */
			*pc16 = c32;
		} else {
			/* Split up in a surrogate pair. */
			c32 -= 0x10000;
			*pc16 = 0xd800 | (c32 >> 10);
			cs->trail_surrogate = 0xdc00 | (c32 & 0x3ff);
		}
	}
	return (len);
}

size_t
ref_mbrtoc16(char16_t * __restrict pc16, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps)
{

	return (ref_mbrtoc16_l(pc16, s, n, ps, __get_locale()));
}

#undef malloc
