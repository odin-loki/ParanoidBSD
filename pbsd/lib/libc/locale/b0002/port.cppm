module;

#define __mbstate_t_defined 1
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} __mbstate_t;
typedef __mbstate_t mbstate_t;

#include <cassert>
#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <uchar.h>

export module pbsd.lib.libc.locale.b0002;

namespace pbsd::lib_libc_locale::b0002 {

#define _CITRUS_ICONV_F_HIDE_INVALID	0x0001

struct _citrus_iconv_ops;
struct _citrus_iconv_shared;
struct _citrus_iconv;

}

extern "C" {
struct _citrus_iconv;

int _citrus_iconv_open(struct _citrus_iconv * __restrict * __restrict,
    const char * __restrict, const char * __restrict);
int _citrus_iconv_convert(struct _citrus_iconv * __restrict,
    char * __restrict * __restrict, std::size_t * __restrict,
    char * __restrict * __restrict, std::size_t * __restrict, std::uint32_t,
    std::size_t * __restrict);

extern int port_iconv_open_fail;

enum {
	XLC_CTYPE = 1,
};

struct port_xlocale_ctype {
	mbstate_t	mbrtoc16;
	mbstate_t	mbrtoc32;
	mbstate_t	c16rtomb;
	mbstate_t	c32rtomb;
};

struct port_xlocale {
	void		*components[6];
};

using locale_t = port_xlocale *;

port_xlocale_ctype	port_global_ctype;
port_xlocale		port_global_locale;

locale_t
port_get_locale()
{
	return (&port_global_locale);
}

static inline locale_t
port_fix_locale(locale_t l)
{
	if (l == nullptr)
		return (&port_global_locale);
	return (l);
}

#define FIX_LOCALE(l)	((l) = port_fix_locale(l))

#define MB_CUR_MAX_L(x)	((std::size_t)4)

char *
port_nl_langinfo_l(int item, locale_t locale)
{
	(void)locale;
	if (item == 0x0000E) /* CODESET */
		return ("UTF-8");
	return ("");
}

struct _citrus_iconv {
	struct _citrus_iconv_shared	*cv_shared;
	void				*cv_closure;
};

}

namespace pbsd::lib_libc_locale::b0002 {

static port_xlocale_ctype *
XLOCALE_CTYPE(locale_t l)
{
	return (static_cast<port_xlocale_ctype *>(l->components[XLC_CTYPE]));
}

}

export namespace pbsd::lib_libc_locale::b0002 {

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

struct _ConversionState_c16rtomb {
	bool			initialized;
	_citrus_iconv		iconv;
	union {
		char16_t	widechar[2];
		char		bytes[sizeof(char16_t) * 2];
	} srcbuf;
	std::size_t		srcbuf_len;
};
static_assert(sizeof(_ConversionState_c16rtomb) <= sizeof(mbstate_t),
    "Size of _ConversionState must not exceed mbstate_t's size.");

std::size_t
c16rtomb_l(char * __restrict s, char16_t c, mbstate_t * __restrict ps,
    locale_t locale)
{
	_ConversionState_c16rtomb *cs;
	_citrus_iconv *handle;
	char *src, *dst;
	std::size_t srcleft, dstleft, invlen;
	int err;

	FIX_LOCALE(locale);
	if (ps == nullptr)
		ps = &(XLOCALE_CTYPE(locale)->c16rtomb);
	cs = (_ConversionState_c16rtomb *)ps;
	handle = &cs->iconv;

	/* Reinitialize mbstate_t. */
	if (s == nullptr || !cs->initialized) {
		if (_citrus_iconv_open(&handle, "UTF-16-INTERNAL",
		    port_nl_langinfo_l(0x0000E, locale)) != 0) {
			cs->initialized = false;
			errno = EINVAL;
			return ((std::size_t)-1);
		}
		cs->srcbuf_len = 0;
		cs->initialized = true;
		if (s == nullptr)
			return (1);
	}

	assert(cs->srcbuf_len < sizeof(cs->srcbuf.widechar) / sizeof(char16_t));
	cs->srcbuf.widechar[cs->srcbuf_len++] = c;

	/* Perform conversion. */
	src = cs->srcbuf.bytes;
	srcleft = cs->srcbuf_len * sizeof(char16_t);
	dst = s;
	dstleft = MB_CUR_MAX_L(locale);
	err = _citrus_iconv_convert(handle, &src, &srcleft, &dst, &dstleft,
	    _CITRUS_ICONV_F_HIDE_INVALID, &invlen);

	/* Character is part of a surrogate pair. We need more input. */
	if (err == EINVAL)
		return (0);
	cs->srcbuf_len = 0;
	
	/* Illegal sequence. */
	if (dst == s) {
		errno = EILSEQ;
		return ((std::size_t)-1);
	}
	return ((std::size_t)(dst - s));
}

std::size_t
c16rtomb(char * __restrict s, char16_t c, mbstate_t * __restrict ps)
{

	return (c16rtomb_l(s, c, ps, port_get_locale()));
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

struct _ConversionState_c32rtomb {
	bool			initialized;
	_citrus_iconv		iconv;
	union {
		char32_t	widechar[1];
		char		bytes[sizeof(char32_t) * 1];
	} srcbuf;
	std::size_t		srcbuf_len;
};
static_assert(sizeof(_ConversionState_c32rtomb) <= sizeof(mbstate_t),
    "Size of _ConversionState must not exceed mbstate_t's size.");

std::size_t
c32rtomb_l(char * __restrict s, char32_t c, mbstate_t * __restrict ps,
    locale_t locale)
{
	_ConversionState_c32rtomb *cs;
	_citrus_iconv *handle;
	char *src, *dst;
	std::size_t srcleft, dstleft, invlen;
	int err;

	FIX_LOCALE(locale);
	if (ps == nullptr)
		ps = &(XLOCALE_CTYPE(locale)->c32rtomb);
	cs = (_ConversionState_c32rtomb *)ps;
	handle = &cs->iconv;

	/* Reinitialize mbstate_t. */
	if (s == nullptr || !cs->initialized) {
		if (_citrus_iconv_open(&handle, "UTF-32-INTERNAL",
		    port_nl_langinfo_l(0x0000E, locale)) != 0) {
			cs->initialized = false;
			errno = EINVAL;
			return ((std::size_t)-1);
		}
		cs->srcbuf_len = 0;
		cs->initialized = true;
		if (s == nullptr)
			return (1);
	}

	assert(cs->srcbuf_len < sizeof(cs->srcbuf.widechar) / sizeof(char32_t));
	cs->srcbuf.widechar[cs->srcbuf_len++] = c;

	/* Perform conversion. */
	src = cs->srcbuf.bytes;
	srcleft = cs->srcbuf_len * sizeof(char32_t);
	dst = s;
	dstleft = MB_CUR_MAX_L(locale);
	err = _citrus_iconv_convert(handle, &src, &srcleft, &dst, &dstleft,
	    _CITRUS_ICONV_F_HIDE_INVALID, &invlen);

	/* Character is part of a surrogate pair. We need more input. */
	if (err == EINVAL)
		return (0);
	cs->srcbuf_len = 0;
	
	/* Illegal sequence. */
	if (dst == s) {
		errno = EILSEQ;
		return ((std::size_t)-1);
	}
	return ((std::size_t)(dst - s));
}

std::size_t
c32rtomb(char * __restrict s, char32_t c, mbstate_t * __restrict ps)
{

	return (c32rtomb_l(s, c, ps, port_get_locale()));
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

struct _ConversionState_mbrtoc16 {
	bool			initialized;
	_citrus_iconv		iconv;
	char			srcbuf[MB_LEN_MAX];
	std::size_t		srcbuf_len;
	union {
		char16_t	widechar[2];
		char		bytes[sizeof(char16_t) * 2];
	} dstbuf;
	std::size_t		dstbuf_len;
};
static_assert(sizeof(_ConversionState_mbrtoc16) <= sizeof(mbstate_t),
    "Size of _ConversionState must not exceed mbstate_t's size.");

std::size_t
mbrtoc16_l(char16_t * __restrict pc, const char * __restrict s, std::size_t n,
    mbstate_t * __restrict ps, locale_t locale)
{
	_ConversionState_mbrtoc16 *cs;
	_citrus_iconv *handle;
	std::size_t i, retval;
	char16_t retchar;

	FIX_LOCALE(locale);
	if (ps == nullptr)
		ps = &(XLOCALE_CTYPE(locale)->mbrtoc16);
	cs = (_ConversionState_mbrtoc16 *)ps;
	handle = &cs->iconv;

	/* Reinitialize mbstate_t. */
	if (s == nullptr || !cs->initialized) {
		if (_citrus_iconv_open(&handle,
		    port_nl_langinfo_l(0x0000E, locale), "UTF-16-INTERNAL") != 0) {
			cs->initialized = false;
			errno = EINVAL;
			return ((std::size_t)-1);
		}
		cs->srcbuf_len = cs->dstbuf_len = 0;
		cs->initialized = true;
		if (s == nullptr)
			return (0);
	}

	/* See if we still have characters left from the previous invocation. */
	if (cs->dstbuf_len > 0) {
		retval = (std::size_t)-3;
		goto return_char;
	}

	/* Fill up the read buffer as far as possible. */
	if (n > sizeof(cs->srcbuf) - cs->srcbuf_len)
		n = sizeof(cs->srcbuf) - cs->srcbuf_len;
	memcpy(cs->srcbuf + cs->srcbuf_len, s, n);

	/* Convert as few characters to the dst buffer as possible. */
	for (i = 0; ; i++) {
		char *src, *dst;
		std::size_t srcleft, dstleft, invlen;
		int err;

		src = cs->srcbuf;
		srcleft = cs->srcbuf_len + n;
		dst = cs->dstbuf.bytes;
		dstleft = i * sizeof(char16_t);
		assert(srcleft <= sizeof(cs->srcbuf) &&
		    dstleft <= sizeof(cs->dstbuf.bytes));
		err = _citrus_iconv_convert(handle, &src, &srcleft,
		    &dst, &dstleft, _CITRUS_ICONV_F_HIDE_INVALID, &invlen);
		cs->dstbuf_len = (std::size_t)(dst - cs->dstbuf.bytes) / sizeof(char16_t);

		/* Got new character(s). Return the first. */
		if (cs->dstbuf_len > 0) {
			assert(src - cs->srcbuf > (std::ptrdiff_t)cs->srcbuf_len);
			retval = (std::size_t)(src - cs->srcbuf - cs->srcbuf_len);
			cs->srcbuf_len = 0;
			goto return_char;
		}

		/* Increase dst buffer size, to obtain the surrogate pair. */
		if (err == E2BIG)
			continue;

		/* Illegal sequence. */
		if (invlen > 0) {
			cs->srcbuf_len = 0;
			errno = EILSEQ;
			return ((std::size_t)-1);
		}

		/* Save unprocessed remainder for the next invocation. */
		memmove(cs->srcbuf, src, srcleft);
		cs->srcbuf_len = srcleft;
		return ((std::size_t)-2);
	}

return_char:
	retchar = cs->dstbuf.widechar[0];
	memmove(&cs->dstbuf.widechar[0], &cs->dstbuf.widechar[1],
	    --cs->dstbuf_len * sizeof(char16_t));
	if (pc != nullptr)
		*pc = retchar;
	if (retchar == 0)
		return (0);
	return (retval);
}

std::size_t
mbrtoc16(char16_t * __restrict pc, const char * __restrict s, std::size_t n,
    mbstate_t * __restrict ps)
{

	return (mbrtoc16_l(pc, s, n, ps, port_get_locale()));
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

struct _ConversionState_mbrtoc32 {
	bool			initialized;
	_citrus_iconv		iconv;
	char			srcbuf[MB_LEN_MAX];
	std::size_t		srcbuf_len;
	union {
		char32_t	widechar[1];
		char		bytes[sizeof(char32_t) * 1];
	} dstbuf;
	std::size_t		dstbuf_len;
};
static_assert(sizeof(_ConversionState_mbrtoc32) <= sizeof(mbstate_t),
    "Size of _ConversionState must not exceed mbstate_t's size.");

std::size_t
mbrtoc32_l(char32_t * __restrict pc, const char * __restrict s, std::size_t n,
    mbstate_t * __restrict ps, locale_t locale)
{
	_ConversionState_mbrtoc32 *cs;
	_citrus_iconv *handle;
	std::size_t i, retval;
	char32_t retchar;

	FIX_LOCALE(locale);
	if (ps == nullptr)
		ps = &(XLOCALE_CTYPE(locale)->mbrtoc32);
	cs = (_ConversionState_mbrtoc32 *)ps;
	handle = &cs->iconv;

	/* Reinitialize mbstate_t. */
	if (s == nullptr || !cs->initialized) {
		if (_citrus_iconv_open(&handle,
		    port_nl_langinfo_l(0x0000E, locale), "UTF-32-INTERNAL") != 0) {
			cs->initialized = false;
			errno = EINVAL;
			return ((std::size_t)-1);
		}
		cs->srcbuf_len = cs->dstbuf_len = 0;
		cs->initialized = true;
		if (s == nullptr)
			return (0);
	}

	/* See if we still have characters left from the previous invocation. */
	if (cs->dstbuf_len > 0) {
		retval = (std::size_t)-3;
		goto return_char;
	}

	/* Fill up the read buffer as far as possible. */
	if (n > sizeof(cs->srcbuf) - cs->srcbuf_len)
		n = sizeof(cs->srcbuf) - cs->srcbuf_len;
	memcpy(cs->srcbuf + cs->srcbuf_len, s, n);

	/* Convert as few characters to the dst buffer as possible. */
	for (i = 0; ; i++) {
		char *src, *dst;
		std::size_t srcleft, dstleft, invlen;
		int err;

		src = cs->srcbuf;
		srcleft = cs->srcbuf_len + n;
		dst = cs->dstbuf.bytes;
		dstleft = i * sizeof(char32_t);
		assert(srcleft <= sizeof(cs->srcbuf) &&
		    dstleft <= sizeof(cs->dstbuf.bytes));
		err = _citrus_iconv_convert(handle, &src, &srcleft,
		    &dst, &dstleft, _CITRUS_ICONV_F_HIDE_INVALID, &invlen);
		cs->dstbuf_len = (std::size_t)(dst - cs->dstbuf.bytes) / sizeof(char32_t);

		/* Got new character(s). Return the first. */
		if (cs->dstbuf_len > 0) {
			assert(src - cs->srcbuf > (std::ptrdiff_t)cs->srcbuf_len);
			retval = (std::size_t)(src - cs->srcbuf - cs->srcbuf_len);
			cs->srcbuf_len = 0;
			goto return_char;
		}

		/* Increase dst buffer size, to obtain the surrogate pair. */
		if (err == E2BIG)
			continue;

		/* Illegal sequence. */
		if (invlen > 0) {
			cs->srcbuf_len = 0;
			errno = EILSEQ;
			return ((std::size_t)-1);
		}

		/* Save unprocessed remainder for the next invocation. */
		memmove(cs->srcbuf, src, srcleft);
		cs->srcbuf_len = srcleft;
		return ((std::size_t)-2);
	}

return_char:
	retchar = cs->dstbuf.widechar[0];
	memmove(&cs->dstbuf.widechar[0], &cs->dstbuf.widechar[1],
	    --cs->dstbuf_len * sizeof(char32_t));
	if (pc != nullptr)
		*pc = retchar;
	if (retchar == 0)
		return (0);
	return (retval);
}

std::size_t
mbrtoc32(char32_t * __restrict pc, const char * __restrict s, std::size_t n,
    mbstate_t * __restrict ps)
{

	return (mbrtoc32_l(pc, s, n, ps, port_get_locale()));
}

} // namespace pbsd::lib_libc_locale::b0002
