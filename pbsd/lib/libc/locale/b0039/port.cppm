module;

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>

export module pbsd.lib.libc.locale.b0039;

extern "C" {
#define __mbstate_t_defined 1
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} __mbstate_t;
typedef __mbstate_t mbstate_t;

#include <wchar.h>

typedef struct xlocale *locale_t;

enum {
	PORT_XLC_CTYPE = 1,
};

struct port_xlocale_ctype {
	mbstate_t	mbsrtowcs;
	mbstate_t	wcsrtombs;
	mbstate_t	mbtowc;
	std::size_t	(*__mbsnrtowcs)(wchar_t * __restrict, const char ** __restrict,
		    std::size_t, std::size_t, mbstate_t * __restrict);
	std::size_t	(*__wcsnrtombs)(char * __restrict, const wchar_t ** __restrict,
		    std::size_t, std::size_t, mbstate_t * __restrict);
	std::size_t	(*__mbrtowc)(wchar_t * __restrict, const char * __restrict,
		    std::size_t, mbstate_t * __restrict);
};

struct port_xlocale {
	void		*components[6];
};

using port_locale_t = port_xlocale *;

port_xlocale_ctype	port_global_ctype;
port_xlocale		port_global_locale;

std::size_t mock_mbsnrtowcs(wchar_t * __restrict, const char ** __restrict,
    std::size_t, std::size_t, mbstate_t * __restrict);
std::size_t mock_wcsnrtombs(char * __restrict, const wchar_t ** __restrict,
    std::size_t, std::size_t, mbstate_t * __restrict);
std::size_t mock_mbrtowc(wchar_t * __restrict, const char * __restrict,
    std::size_t, mbstate_t * __restrict);
int __wcwidth(wchar_t);
int __wcwidth_l(wchar_t, locale_t);

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

}

export namespace pbsd::lib_libc_locale::b0039 {

using mbstate_t = ::mbstate_t;
using port_locale_t = ::port_locale_t;

inline void
init_locale()
{
	port_global_locale.components[PORT_XLC_CTYPE] = &port_global_ctype;
	port_global_ctype.__mbsnrtowcs = mock_mbsnrtowcs;
	port_global_ctype.__wcsnrtombs = mock_wcsnrtombs;
	port_global_ctype.__mbrtowc = mock_mbrtowc;
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
mbsrtowcs_l(wchar_t * __restrict dst, const char ** __restrict src, std::size_t len,
    mbstate_t * __restrict ps, port_locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == nullptr)
		ps = &(XLOCALE_CTYPE(locale)->mbsrtowcs);
	return (XLOCALE_CTYPE(locale)->__mbsnrtowcs(dst, src, SIZE_MAX, len, ps));
}
std::size_t
mbsrtowcs(wchar_t * __restrict dst, const char ** __restrict src, std::size_t len,
    mbstate_t * __restrict ps)
{
	return mbsrtowcs_l(dst, src, len, ps, port_get_locale());
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
wcsrtombs_l(char * __restrict dst, const wchar_t ** __restrict src, std::size_t len,
    mbstate_t * __restrict ps, port_locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == nullptr)
		ps = &(XLOCALE_CTYPE(locale)->wcsrtombs);
	return (XLOCALE_CTYPE(locale)->__wcsnrtombs(dst, src, SIZE_MAX, len, ps));
}

std::size_t
wcsrtombs(char * __restrict dst, const wchar_t ** __restrict src, std::size_t len,
    mbstate_t * __restrict ps)
{
	return wcsrtombs_l(dst, src, len, ps, port_get_locale());
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
mbtowc_l(wchar_t * __restrict pwc, const char * __restrict s, std::size_t n,
    port_locale_t locale)
{
	static const mbstate_t initial;
	std::size_t rval;
	FIX_LOCALE(locale);

	if (s == nullptr) {
		/* No support for state dependent encodings. */
		XLOCALE_CTYPE(locale)->mbtowc = initial;
		return (0);
	}
	rval = XLOCALE_CTYPE(locale)->__mbrtowc(pwc, s, n,
	    &(XLOCALE_CTYPE(locale)->mbtowc));
	switch (rval) {
	case (std::size_t)-2:
		errno = EILSEQ;
		/* FALLTHROUGH */
	case (std::size_t)-1:
		return (-1);
	default:
		return ((int)rval);
	}
}
int
mbtowc(wchar_t * __restrict pwc, const char * __restrict s, std::size_t n)
{
	return mbtowc_l(pwc, s, n, port_get_locale());
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

#undef wcwidth

int
wcwidth(wchar_t wc)
{
	return (__wcwidth(wc));
}
int
wcwidth_l(wchar_t wc, port_locale_t locale)
{
	return (__wcwidth_l(wc, locale));
}

} // namespace pbsd::lib_libc_locale::b0039
