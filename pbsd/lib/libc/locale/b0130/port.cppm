/*-
 * PBSD batch b0130 -- C++23 module port of
 *
 *	hbsd/src/lib/libc/locale/wcsnrtombs.c
 *	hbsd/src/lib/libc/locale/wcstod.c
 *	hbsd/src/lib/libc/locale/lmessages.c
 */

module;

#include <climits>
#include <cstddef>
#include <cstdlib>
#include <cstring>

export module pbsd.lib.libc.locale.b0130;

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

typedef size_t (*wcrtomb_pfn_t)(char * __restrict, wchar_t, mbstate_t * __restrict);

struct lc_messages_T {
	const char	*yesexpr;
	const char	*noexpr;
	const char	*yesstr;
	const char	*nostr;
};

struct xlocale_component_header {
	void		(*destructor)(void *);
};

struct xlocale_component {
	xlocale_component_header header;
};

struct xlocale_messages {
	xlocale_component header;
	char		*buffer;
	lc_messages_T	locale;
};

struct port_xlocale_ctype {
	size_t		(*__wcsnrtombs)(char * __restrict,
		    const wchar_t ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);
	mbstate_t	wcsnrtombs;
};

struct port_xlocale {
	int		using_messages_locale;
	void		*components[8];
};

typedef struct port_xlocale *port_locale_t;

enum {
	PORT_XLC_CTYPE = 1,
	PORT_XLC_MESSAGES = 5,
};

#define _LDP_LOADED	1
#define _LDP_ERROR	(-1)

#define LCMESSAGES_SIZE_FULL (sizeof(struct lc_messages_T) / sizeof(char *))
#define LCMESSAGES_SIZE_MIN \
		(offsetof(struct lc_messages_T, yesstr) / sizeof(char *))

size_t	pbsd_wcrtomb(char * __restrict, wchar_t, mbstate_t * __restrict);
size_t	pbsd_wcsrtombs_l(char * __restrict, const wchar_t ** __restrict,
	    size_t, mbstate_t * __restrict, port_locale_t);
int	pbsd_iswspace_l(wint_t, port_locale_t);
double	pbsd_strtod_l(const char * __restrict, char ** __restrict,
	    port_locale_t);
int	__part_load_locale(const char *, int *, char **, const char *, int,
	    int, const char **);

extern xlocale_messages	__xlocale_global_messages;
extern port_xlocale		__xlocale_global_locale;

port_xlocale_ctype	port_global_ctype;
port_xlocale		port_global_locale_storage;

static inline port_xlocale_ctype *
port_XLOCALE_CTYPE(port_locale_t l)
{
	return (static_cast<port_xlocale_ctype *>(l->components[PORT_XLC_CTYPE]));
}

static inline port_locale_t
port_fix_locale(port_locale_t l)
{
	if (l == nullptr)
		return (&port_global_locale_storage);
	return (l);
}

#define FIX_LOCALE(l)	((l) = port_fix_locale(l))

port_locale_t
port_get_locale()
{
	return (&port_global_locale_storage);
}
}

export namespace pbsd::lib_libc_locale::b0130 {

using mbstate_t = ::mbstate_t;
using locale_t = ::port_locale_t;
using wcrtomb_pfn_t = ::wcrtomb_pfn_t;

struct lc_messages_T {
	const char	*yesexpr;
	const char	*noexpr;
	const char	*yesstr;
	const char	*nostr;
};

struct xlocale_ctype {
	std::size_t	(*__wcsnrtombs)(char * __restrict,
		    const wchar_t ** __restrict, std::size_t, std::size_t,
		    mbstate_t * __restrict);
	mbstate_t	wcsnrtombs;
};

struct xlocale {
	int		using_messages_locale;
	void		*components[8];
};

#define XLOCALE_CTYPE(l)	port_XLOCALE_CTYPE(l)

inline locale_t
global_locale()
{
	return (&::port_global_locale_storage);
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2013 Garrett D'Amore <garrett@damore.org>
 * Copyright 2010 Nexenta Systems, Inc.  All rights reserved.
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
wcsnrtombs_l(char * __restrict dst, const wchar_t ** __restrict src, std::size_t nwc,
    std::size_t len, mbstate_t * __restrict ps, locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->wcsnrtombs);
	return (XLOCALE_CTYPE(locale)->__wcsnrtombs(dst, src, nwc, len, ps));
}
std::size_t
wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src, std::size_t nwc,
    std::size_t len, mbstate_t * __restrict ps)
{
	return wcsnrtombs_l(dst, src, nwc, len, ps, port_get_locale());
}


std::size_t
__wcsnrtombs_std(char * __restrict dst, const wchar_t ** __restrict src,
    std::size_t nwc, std::size_t len, mbstate_t * __restrict ps,
    wcrtomb_pfn_t pwcrtomb)
{
	mbstate_t mbsbak;
	char buf[MB_LEN_MAX];
	const wchar_t *s;
	std::size_t nbytes;
	std::size_t nb;

	s = *src;
	nbytes = 0;

	if (dst == NULL) {
		while (nwc-- > 0) {
			if ((nb = pwcrtomb(buf, *s, ps)) == (std::size_t)-1)
				/* Invalid character - wcrtomb() sets errno. */
				return ((std::size_t)-1);
			else if (*s == L'\0')
				return (nbytes + nb - 1);
			s++;
			nbytes += nb;
		}
		return (nbytes);
	}

	while (len > 0 && nwc-- > 0) {
		if (len > (std::size_t)MB_CUR_MAX) {
			/* Enough space to translate in-place. */
			if ((nb = pwcrtomb(dst, *s, ps)) == (std::size_t)-1) {
				*src = s;
				return ((std::size_t)-1);
			}
		} else {
			/*
			 * May not be enough space; use temp. buffer.
			 *
			 * We need to save a copy of the conversion state
			 * here so we can restore it if the multibyte
			 * character is too long for the buffer.
			 */
			mbsbak = *ps;
			if ((nb = pwcrtomb(buf, *s, ps)) == (std::size_t)-1) {
				*src = s;
				return ((std::size_t)-1);
			}
			if (nb > (int)len) {
				/* MB sequence for character won't fit. */
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

std::size_t
dispatch_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    std::size_t nwc, std::size_t len, mbstate_t * __restrict ps)
{
	return (__wcsnrtombs_std(dst, src, nwc, len, ps, pbsd_wcrtomb));
}

inline void
init_locale()
{
	std::memset(&::port_global_ctype, 0, sizeof(::port_global_ctype));
	::port_global_ctype.__wcsnrtombs = dispatch_wcsnrtombs;
	::port_global_locale_storage.components[PORT_XLC_CTYPE] =
	    &::port_global_ctype;
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Tim J. Robbins
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

double
wcstod_l(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr,
		locale_t locale)
{
	static const mbstate_t initial{};
	mbstate_t mbs;
	double val;
	char *buf, *end;
	const wchar_t *wcp;
	std::size_t len;
	std::size_t spaces;
	FIX_LOCALE(locale);

	wcp = nptr;
	spaces = 0;
	while (pbsd_iswspace_l(*wcp, locale)) {
		wcp++;
		spaces++;
	}

	mbs = initial;
	if ((len = pbsd_wcsrtombs_l(NULL, &wcp, 0, &mbs, locale)) == (std::size_t)-1) {
		if (endptr != NULL)
			*endptr = (wchar_t *)nptr;
		return (0.0);
	}
	if ((buf = (char *)malloc(len + 1)) == NULL) {
		if (endptr != NULL)
			*endptr = (wchar_t *)nptr;
		return (0.0);
	}
	mbs = initial;
	pbsd_wcsrtombs_l(buf, &wcp, len + 1, &mbs, locale);

	val = pbsd_strtod_l(buf, &end, locale);

	if (endptr != NULL) {
		*endptr = (wchar_t *)nptr + (end - buf);
		if (buf != end)
			*endptr += spaces;
	}

	free(buf);

	return (val);
}
double
wcstod(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr)
{
	return wcstod_l(nptr, endptr, port_get_locale());
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2001 Alexey Zelkin <phantom@FreeBSD.org>
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

static char empty[] = "";

static const ::lc_messages_T _C_messages_locale = {
	"^[yY]" ,	/* yesexpr */
	"^[nN]" ,	/* noexpr */
	"yes" , 	/* yesstr */
	"no"		/* nostr */
};

static void
destruct_messages(void *v)
{
	::xlocale_messages *l = static_cast<::xlocale_messages *>(v);
	if (l->buffer)
		free(l->buffer);
	free(l);
}

static int
messages_load_locale(::xlocale_messages *loc, int *using_locale,
    const char *name)
{
	int ret;
	::lc_messages_T *l = &loc->locale;

	ret = __part_load_locale(name, using_locale,
		  &loc->buffer, "LC_MESSAGES",
		  LCMESSAGES_SIZE_FULL, LCMESSAGES_SIZE_MIN,
		  (const char **)l);
	if (ret == _LDP_LOADED) {
		if (l->yesstr == NULL)
			l->yesstr = empty;
		if (l->nostr == NULL)
			l->nostr = empty;
	}
	return (ret);
}

static void
xlocale_release(void *v)
{
	::xlocale_component *c = static_cast<::xlocale_component *>(v);

	if (c != NULL && c->header.destructor != NULL)
		c->header.destructor(v);
	free(v);
}

int
__messages_load_locale(const char *name)
{
	return (messages_load_locale(&::__xlocale_global_messages,
	    &__xlocale_global_locale.using_messages_locale, name));
}

void *
__messages_load(const char *name, locale_t l)
{
	::xlocale_messages *newloc = static_cast<::xlocale_messages *>(
	    calloc(sizeof(::xlocale_messages), 1));
	if (newloc == NULL)
		return (NULL);
	newloc->header.header.destructor = destruct_messages;
	if (messages_load_locale(newloc, &l->using_messages_locale, name) ==
	    _LDP_ERROR) {
		xlocale_release(newloc);
		return (NULL);
	}
	return (newloc);
}

::lc_messages_T *
__get_current_messages_locale(locale_t loc)
{
	return (loc->using_messages_locale ? &(static_cast<::xlocale_messages *>(
	    loc->components[PORT_XLC_MESSAGES])->locale) :
	    (::lc_messages_T *)&_C_messages_locale);
}

} /* namespace pbsd::lib_libc_locale::b0130 */
