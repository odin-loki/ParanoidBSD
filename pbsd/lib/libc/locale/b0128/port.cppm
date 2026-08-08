module;

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <sys/types.h>

#ifndef SIZE_T_MAX
#define SIZE_T_MAX	SIZE_MAX
#endif

#ifndef MB_LEN_MAX
#define MB_LEN_MAX	4
#endif

#define MIN(a, b)	((a) < (b) ? (a) : (b))

export module pbsd.lib.libc.locale.b0128;

extern "C" {
#define __mbstate_t_defined 1
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} __mbstate_t;
typedef __mbstate_t mbstate_t;

typedef uint16_t char16_t;
typedef uint32_t char32_t;
typedef int wctrans_t;

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
	std::size_t (*__wcsnrtombs)(char * __restrict, const wchar_t ** __restrict,
	    std::size_t, std::size_t, mbstate_t * __restrict);
	mbstate_t	mbrtoc16;
	mbstate_t	mbrtoc32;
	mbstate_t	wcsrtombs;
};

struct port_xlocale {
	void		*components[6];
};

using port_locale_t = port_xlocale *;

port_xlocale_ctype	port_global_ctype;
port_xlocale		port_global_locale;

unsigned long		port_malloc_calls;
int			port_malloc_fail_at;

std::size_t	port_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    std::size_t, mbstate_t * __restrict);
std::size_t	port_wcrtomb(char * __restrict, wchar_t, mbstate_t * __restrict);
std::size_t	port_wcsnrtombs(char * __restrict, const wchar_t ** __restrict,
		    std::size_t, std::size_t, mbstate_t * __restrict);

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

static std::size_t
mbrtowc_l(wchar_t * __restrict pwc, const char * __restrict s, std::size_t n,
    mbstate_t * __restrict ps, port_locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == nullptr)
		ps = &(XLOCALE_CTYPE(locale)->mbrtoc32);
	return (XLOCALE_CTYPE(locale)->__mbrtowc(pwc, s, n, ps));
}

static std::size_t
mbrtoc32_l(char32_t * __restrict pc32, const char * __restrict s, std::size_t n,
    mbstate_t * __restrict ps, port_locale_t locale)
{

	FIX_LOCALE(locale);
	if (ps == nullptr)
		ps = &(XLOCALE_CTYPE(locale)->mbrtoc32);
	return (mbrtowc_l((wchar_t *)(void *)pc32, s, n, ps, locale));
}

static std::size_t
wcsrtombs_l(char * __restrict dst, const wchar_t ** __restrict src,
    std::size_t len, mbstate_t * __restrict ps, port_locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == nullptr)
		ps = &(XLOCALE_CTYPE(locale)->wcsrtombs);
	return (XLOCALE_CTYPE(locale)->__wcsnrtombs(dst, src, SIZE_T_MAX, len, ps));
}

static int
iswspace_l(wint_t wc, port_locale_t locale)
{

	(void)locale;
	return (iswspace(wc));
}

static float
strtof_l(const char * __restrict nptr, char ** __restrict endptr,
    port_locale_t locale)
{

	(void)locale;
	return (strtof(nptr, endptr));
}

static long double
strtold_l(const char * __restrict nptr, char ** __restrict endptr,
    port_locale_t locale)
{

	(void)locale;
	return (strtold(nptr, endptr));
}

static wint_t
towlower_l(wint_t wc, port_locale_t locale)
{

	(void)locale;
	if (wc >= L'A' && wc <= L'Z')
		return (wc - L'A' + L'a');
	return (wc);
}

static wint_t
towupper_l(wint_t wc, port_locale_t locale)
{

	(void)locale;
	if (wc >= L'a' && wc <= L'z')
		return (wc - L'a' + L'A');
	return (wc);
}

static port_locale_t
__get_locale()
{

	return (port_get_locale());
}

static void * __attribute__((used))
port_libc_malloc(std::size_t n)
{

	port_malloc_calls++;
	if (port_malloc_fail_at != 0 &&
	    port_malloc_calls == (unsigned long)port_malloc_fail_at)
		return (nullptr);
	return (malloc(n));
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
	int i, blen;

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
		blen = 2;
	} else if ((wc & ~0xffff) == 0) {
		if (wc >= 0xd800 && wc <= 0xdfff) {
			errno = EILSEQ;
			return ((std::size_t)-1);
		}
		lead = 0xe0;
		blen = 3;
	} else if (wc >= 0 && wc <= 0x10ffff) {
		lead = 0xf0;
		blen = 4;
	} else {
		errno = EILSEQ;
		return ((std::size_t)-1);
	}

	for (i = blen - 1; i > 0; i--) {
		s[i] = (wc & 0x3f) | 0x80;
		wc >>= 6;
	}
	*s = (wc & 0xff) | lead;

	return (blen);
}

std::size_t
port_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    std::size_t nwc, std::size_t len, mbstate_t * __restrict ps)
{
	mbstate_t mbsbak;
	char buf[MB_LEN_MAX];
	const wchar_t *s;
	std::size_t nbytes;
	std::size_t nb;

	if (src == nullptr)
		return ((std::size_t)-1);
	s = *src;
	if (s == nullptr)
		return ((std::size_t)-1);
	nbytes = 0;

	if (dst == nullptr) {
		while (nwc-- > 0) {
			if ((nb = port_wcrtomb(buf, *s, ps)) == (std::size_t)-1)
				return ((std::size_t)-1);
			else if (*s == L'\0')
				return (nbytes + nb - 1);
			s++;
			nbytes += nb;
		}
		return (nbytes);
	}

	while (len > 0 && nwc-- > 0) {
		if (len > (std::size_t)MB_LEN_MAX) {
			if ((nb = port_wcrtomb(dst, *s, ps)) == (std::size_t)-1) {
				*src = s;
				return ((std::size_t)-1);
			}
		} else {
			mbsbak = *ps;
			if ((nb = port_wcrtomb(buf, *s, ps)) == (std::size_t)-1) {
				*src = s;
				return ((std::size_t)-1);
			}
			if (nb > len) {
				*ps = mbsbak;
				break;
			}
			memcpy(dst, buf, nb);
		}
		if (*s == L'\0') {
			*src = nullptr;
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

}

export namespace pbsd::lib_libc_locale::b0128 {

using mbstate_t = ::mbstate_t;
using port_locale_t = ::port_locale_t;
using char16_t = ::char16_t;
using char32_t = ::char32_t;
using wctrans_t = ::wctrans_t;

enum {
	WCT_ERROR = 0,
	WCT_TOLOWER = 1,
	WCT_TOUPPER = 2,
};

inline void
init_locale()
{
	port_global_ctype.__mbrtowc = port_mbrtowc;
	port_global_ctype.__wcsnrtombs = port_wcsnrtombs;
	memset(&port_global_ctype.mbrtoc16, 0, sizeof(port_global_ctype.mbrtoc16));
	memset(&port_global_ctype.mbrtoc32, 0, sizeof(port_global_ctype.mbrtoc32));
	memset(&port_global_ctype.wcsrtombs, 0, sizeof(port_global_ctype.wcsrtombs));
	port_global_locale.components[PORT_XLC_CTYPE] = &port_global_ctype;
}

inline port_locale_t
global_locale()
{

	return (&port_global_locale);
}

inline void
reset_malloc()
{

	port_malloc_calls = 0;
	port_malloc_fail_at = 0;
}

inline void
set_malloc_fail_at(int at)
{

	port_malloc_calls = 0;
	port_malloc_fail_at = at;
}

inline void
reset_locale_states()
{

	memset(&port_global_ctype.mbrtoc16, 0, sizeof(port_global_ctype.mbrtoc16));
	memset(&port_global_ctype.mbrtoc32, 0, sizeof(port_global_ctype.mbrtoc32));
	memset(&port_global_ctype.wcsrtombs, 0, sizeof(port_global_ctype.wcsrtombs));
}

#define malloc port_libc_malloc

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
wcstof_l(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr,
		port_locale_t locale)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	float val;
	char *buf, *end;
	const wchar_t *wcp;
	std::size_t len;
	std::size_t spaces;
	FIX_LOCALE(locale);

	wcp = nptr;
	spaces = 0;
	while (iswspace_l(*wcp, locale)) {
		wcp++;
		spaces++;
	}

	mbs = initial;
	if ((len = wcsrtombs_l(NULL, &wcp, 0, &mbs, locale)) == (std::size_t)-1) {
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
wcstof(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr)
{
	return wcstof_l(nptr, endptr, __get_locale());
}

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
wcstold_l(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr,
		port_locale_t locale)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	long double val;
	char *buf, *end;
	const wchar_t *wcp;
	std::size_t len;
	std::size_t spaces;
	FIX_LOCALE(locale);

	wcp = nptr;
	spaces = 0;
	while (iswspace_l(*wcp, locale)) {
		wcp++;
		spaces++;
	}

	mbs = initial;
	if ((len = wcsrtombs_l(NULL, &wcp, 0, &mbs, locale)) == (std::size_t)-1) {
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
wcstold(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr)
{
	return wcstold_l(nptr, endptr, __get_locale());
}

#undef malloc

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
towctrans_l(wint_t wc, wctrans_t desc, port_locale_t locale)
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
towctrans(wint_t wc, wctrans_t desc)
{
	return towctrans_l(wc, desc, __get_locale());
}

wctrans_t
wctrans_l(const char *charclass, port_locale_t locale)
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
wctrans(const char *charclass)
{
	return wctrans_l(charclass, 0);
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

typedef struct {
	char16_t	trail_surrogate;
	mbstate_t	c32_mbstate;
} _Char16State;

std::size_t
mbrtoc16_l(char16_t * __restrict pc16, const char * __restrict s, std::size_t n,
    mbstate_t * __restrict ps, port_locale_t locale)
{
	_Char16State *cs;
	char32_t c32;
	ssize_t len;

	FIX_LOCALE(locale);
	if (ps == nullptr)
		ps = &(XLOCALE_CTYPE(locale)->mbrtoc16);
	cs = (_Char16State *)ps;

	/*
	 * Call straight into mbrtoc32_l() if we don't need to return a
	 * character value. According to the spec, if s is a null
	 * pointer, the value of parameter pc16 is also ignored.
	 */
	if (pc16 == nullptr || s == nullptr) {
		cs->trail_surrogate = 0;
		return (mbrtoc32_l(nullptr, s, n, &cs->c32_mbstate, locale));
	}

	/* Return the trail surrogate from the previous invocation. */
	if (cs->trail_surrogate >= 0xdc00 && cs->trail_surrogate <= 0xdfff) {
		*pc16 = cs->trail_surrogate;
		cs->trail_surrogate = 0;
		return ((std::size_t)-3);
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

std::size_t
mbrtoc16(char16_t * __restrict pc16, const char * __restrict s, std::size_t n,
    mbstate_t * __restrict ps)
{

	return (mbrtoc16_l(pc16, s, n, ps, __get_locale()));
}

}
