module;

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

export module pbsd.lib.libc.locale.b0041;

extern "C" {
#define __mbstate_t_defined 1
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} __mbstate_t;
typedef __mbstate_t mbstate_t;

#include <wchar.h>

#ifndef MB_LEN_MAX
#define MB_LEN_MAX	4
#endif

#ifndef MB_CUR_MAX
#define MB_CUR_MAX	4
#endif

#ifndef EOF
#define EOF	(-1)
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
	std::size_t (*__wcsnrtombs)(char * __restrict,
	    const wchar_t ** __restrict, std::size_t, std::size_t,
	    mbstate_t * __restrict);
	mbstate_t	mblen;
	mbstate_t	wctomb;
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
std::size_t	port_wcsnrtombs(char * __restrict,
		    const wchar_t ** __restrict, std::size_t, std::size_t,
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

std::size_t
port_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    std::size_t nwc, std::size_t len, mbstate_t * __restrict ps)
{
	_UTF8State *us;
	char buf[MB_LEN_MAX];
	const wchar_t *s;
	std::size_t nbytes;
	std::size_t nb;

	us = (_UTF8State *)ps;

	if (us->want != 0) {
		errno = EINVAL;
		return ((std::size_t)-1);
	}

	s = *src;
	nbytes = 0;

	if (dst == nullptr) {
		while (nwc-- > 0) {
			if (0 <= *s && *s < 0x80)
				nb = 1;
			else if ((nb = port_wcrtomb(buf, *s, ps)) ==
			    (std::size_t)-1)
				return ((std::size_t)-1);
			if (*s == L'\0')
				return (nbytes + nb - 1);
			s++;
			nbytes += nb;
		}
		return (nbytes);
	}

	while (len > 0 && nwc-- > 0) {
		if (0 <= *s && *s < 0x80) {
			nb = 1;
			*dst = *s;
		} else if (len > (std::size_t)MB_CUR_MAX) {
			if ((nb = port_wcrtomb(dst, *s, ps)) == (std::size_t)-1) {
				*src = s;
				return ((std::size_t)-1);
			}
		} else {
			if ((nb = port_wcrtomb(buf, *s, ps)) == (std::size_t)-1) {
				*src = s;
				return ((std::size_t)-1);
			}
			if (nb > (int)len)
				break;
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

export namespace pbsd::lib_libc_locale::b0041 {

using mbstate_t = ::mbstate_t;
using port_locale_t = ::port_locale_t;

inline void
init_locale()
{
	port_global_ctype.__mbrtowc = port_mbrtowc;
	port_global_ctype.__wcrtomb = port_wcrtomb;
	port_global_ctype.__wcsnrtombs = port_wcsnrtombs;
	memset(&port_global_ctype.mblen, 0, sizeof(port_global_ctype.mblen));
	memset(&port_global_ctype.wctomb, 0, sizeof(port_global_ctype.wctomb));
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
wcstombs_l(char * __restrict s, const wchar_t * __restrict pwcs, std::size_t n,
    port_locale_t locale)
{
	static const mbstate_t initial{};
	mbstate_t mbs;
	const wchar_t *pwcsp;
	FIX_LOCALE(locale);

	mbs = initial;
	pwcsp = pwcs;
	return (XLOCALE_CTYPE(locale)->__wcsnrtombs(s, &pwcsp, SIZE_MAX, n, &mbs));
}

std::size_t
wcstombs(char * __restrict s, const wchar_t * __restrict pwcs, std::size_t n)
{

	return (wcstombs_l(s, pwcs, n, port_get_locale()));
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
wctomb_l(char *s, wchar_t wchar, port_locale_t locale)
{
	static const mbstate_t initial{};
	std::size_t rval;
	FIX_LOCALE(locale);

	if (s == nullptr) {
		/* No support for state dependent encodings. */
		XLOCALE_CTYPE(locale)->wctomb = initial;
		return (0);
	}
	if ((rval = XLOCALE_CTYPE(locale)->__wcrtomb(s, wchar,
	    &(XLOCALE_CTYPE(locale)->wctomb))) == (std::size_t)-1)
		return (-1);
	return ((int)rval);
}

int
wctomb(char *s, wchar_t wchar)
{

	return (wctomb_l(s, wchar, port_get_locale()));
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002, 2003 Tim J. Robbins.
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

wint_t
btowc_l(int c, port_locale_t l)
{
	static const mbstate_t initial{};
	mbstate_t mbs = initial;
	char cc;
	wchar_t wc;
	FIX_LOCALE(l);

	if (c == EOF)
		return (WEOF);
	/*
	 * We expect mbrtowc() to return 0 or 1, hence the check for n > 1
	 * which detects error return values as well as "impossible" byte
	 * counts.
	 */
	cc = (char)c;
	if (XLOCALE_CTYPE(l)->__mbrtowc(&wc, &cc, 1, &mbs) > 1)
		return (WEOF);
	return (wc);
}

wint_t
btowc(int c)
{

	return (btowc_l(c, port_get_locale()));
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
mblen_l(const char *s, std::size_t n, port_locale_t locale)
{
	static const mbstate_t initial{};
	std::size_t rval;
	FIX_LOCALE(locale);

	if (s == nullptr) {
		/* No support for state dependent encodings. */
		XLOCALE_CTYPE(locale)->mblen = initial;
		return (0);
	}
	rval = XLOCALE_CTYPE(locale)->__mbrtowc(nullptr, s, n,
	    &(XLOCALE_CTYPE(locale)->mblen));
	if (rval == (std::size_t)-1 || rval == (std::size_t)-2)
		return (-1);
	return ((int)rval);
}

int
mblen(const char *s, std::size_t n)
{

	return (mblen_l(s, n, port_get_locale()));
}

}
