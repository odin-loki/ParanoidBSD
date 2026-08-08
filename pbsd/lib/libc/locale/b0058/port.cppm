module;

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

export module pbsd.lib.libc.locale.b0058;

extern "C" {
#define __mbstate_t_defined 1
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} __mbstate_t;
typedef __mbstate_t mbstate_t;

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
	mbstate_t	mbrtoc32;
	mbstate_t	mbrlen;
	mbstate_t	wcrtomb;
	mbstate_t	mbrtowc;
};

struct port_xlocale {
	void		*components[6];
};

using port_locale_t = port_xlocale *;

port_xlocale_ctype	port_global_ctype;
port_xlocale		port_global_locale;

static std::size_t	port_mbrtowc(wchar_t * __restrict,
		    const char * __restrict, std::size_t, mbstate_t * __restrict);
static std::size_t	port_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);

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

static std::size_t
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

std::size_t
mbrtowc_l(wchar_t * __restrict pwc, const char * __restrict s, std::size_t n,
    mbstate_t * __restrict ps, port_locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == nullptr)
		ps = &(XLOCALE_CTYPE(locale)->mbrtowc);
	return (XLOCALE_CTYPE(locale)->__mbrtowc(pwc, s, n, ps));
}

port_locale_t
__get_locale()
{

	return (port_get_locale());
}

}

export namespace pbsd::lib_libc_locale::b0058 {

using mbstate_t = ::mbstate_t;
using port_locale_t = ::port_locale_t;

inline void
init_locale()
{
	port_global_ctype.__mbrtowc = port_mbrtowc;
	port_global_ctype.__wcrtomb = port_wcrtomb;
	memset(&port_global_ctype.mbrtoc32, 0, sizeof(port_global_ctype.mbrtoc32));
	memset(&port_global_ctype.mbrlen, 0, sizeof(port_global_ctype.mbrlen));
	memset(&port_global_ctype.wcrtomb, 0, sizeof(port_global_ctype.wcrtomb));
	memset(&port_global_ctype.mbrtowc, 0, sizeof(port_global_ctype.mbrtowc));
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
mbrtoc32_l(char32_t * __restrict pc32, const char * __restrict s, std::size_t n,
    mbstate_t * __restrict ps, port_locale_t locale)
{

	FIX_LOCALE(locale);
	if (ps == nullptr)
		ps = &(XLOCALE_CTYPE(locale)->mbrtoc32);

	/* Assume wchar_t uses UTF-32. */
	return (mbrtowc_l(pc32, s, n, ps, locale));
}

std::size_t
mbrtoc32(char32_t * __restrict pc32, const char * __restrict s, std::size_t n,
    mbstate_t * __restrict ps)
{

	return (mbrtoc32_l(pc32, s, n, ps, __get_locale()));
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
wcrtomb_l(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps,
		port_locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == nullptr)
		ps = &(XLOCALE_CTYPE(locale)->wcrtomb);
	return (XLOCALE_CTYPE(locale)->__wcrtomb(s, wc, ps));
}

std::size_t
wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	return wcrtomb_l(s, wc, ps, __get_locale());
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

int
wctob_l(wint_t c, port_locale_t locale)
{
	static const mbstate_t initial;
	mbstate_t mbs = initial;
	char buf[MB_LEN_MAX];
	FIX_LOCALE(locale);

	if (c == WEOF || XLOCALE_CTYPE(locale)->__wcrtomb(buf, c, &mbs) != 1)
		return (EOF);
	return ((unsigned char)*buf);
}
int
wctob(wint_t c)
{
	return wctob_l(c, __get_locale());
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
mbrlen_l(const char * __restrict s, std::size_t n, mbstate_t * __restrict ps, port_locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == nullptr)
		ps = &(XLOCALE_CTYPE(locale)->mbrlen);
	return (XLOCALE_CTYPE(locale)->__mbrtowc(nullptr, s, n, ps));
}

std::size_t
mbrlen(const char * __restrict s, std::size_t n, mbstate_t * __restrict ps)
{
	return mbrlen_l(s, n, ps, __get_locale());
}

}
