/*
 * PBSD batch b0039 -- C++23 module port of
 *
 *	lib/libc/locale/mbsrtowcs.c
 *	lib/libc/locale/wcsrtombs.c
 *	lib/libc/locale/mbtowc.c
 *	lib/libc/locale/wcwidth.c
 *
 * The four files are pure dispatch wrappers over the libc locale internals.
 * The internals themselves are not part of this batch; they are declared
 * below with C linkage and supplied by the shared locale substrate.  The two
 * FreeBSD type names that collide with the host libc are renamed
 * (mbstate_t -> pbsd_mbstate_t, locale_t -> pbsd_locale_t); everything else
 * is carried over unchanged, including signedness, evaluation order and the
 * (size_t)-2 / (size_t)-1 switch in mbtowc_l().
 */

module;

#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#define	SIZE_T_MAX		SIZE_MAX
#define	XLOCALE_CTYPE(l)	(&((l)->ctype))
#define	FIX_LOCALE(l)		((l) = ((l) == NULL ? __get_locale() : (l)))

export module pbsd.lib.libc.locale.b0039;

export namespace pbsd::lib_libc_locale::b0039 {

struct pbsd_mbstate {
	unsigned int	want;
	unsigned int	have;
	unsigned int	wch;
	unsigned int	lbound;
};
using pbsd_mbstate_t = pbsd_mbstate;

struct pbsd_locale;
using pbsd_locale_t = pbsd_locale *;

struct pbsd_xlocale_ctype {
	size_t	(*__mbsnrtowcs)(wchar_t *, const char **, size_t, size_t,
		    pbsd_mbstate_t *);
	size_t	(*__wcsnrtombs)(char *, const wchar_t **, size_t, size_t,
		    pbsd_mbstate_t *);
	size_t	(*__mbrtowc)(wchar_t *, const char *, size_t,
		    pbsd_mbstate_t *);
	pbsd_mbstate_t	mbsrtowcs;
	pbsd_mbstate_t	wcsrtombs;
	pbsd_mbstate_t	mbtowc;
	int		wcwidth_mode;
};

struct pbsd_locale {
	pbsd_xlocale_ctype ctype;
};

/* Locale substrate: the libc internals these wrappers dispatch through. */
extern "C" {
size_t	pbsd_mbrtowc(wchar_t *, const char *, size_t, pbsd_mbstate_t *);
size_t	pbsd_wcrtomb(char *, wchar_t, pbsd_mbstate_t *);
size_t	pbsd_mbsnrtowcs(wchar_t *, const char **, size_t, size_t,
	    pbsd_mbstate_t *);
size_t	pbsd_wcsnrtombs(char *, const wchar_t **, size_t, size_t,
	    pbsd_mbstate_t *);
int	pbsd_wcwidth(wchar_t);
int	pbsd_wcwidth_l(wchar_t, pbsd_locale_t);
pbsd_locale_t	pbsd_get_active_locale();
void	pbsd_set_active_locale(pbsd_locale_t);
void	pbsd_locale_init(pbsd_locale_t, int);
}

inline pbsd_locale_t
__get_locale()
{
	return (pbsd_get_active_locale());
}

inline int
__wcwidth(wchar_t wc)
{
	return (pbsd_wcwidth(wc));
}

inline int
__wcwidth_l(wchar_t wc, pbsd_locale_t locale)
{
	return (pbsd_wcwidth_l(wc, locale));
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

/* lib/libc/locale/mbsrtowcs.c */

size_t
mbsrtowcs_l(wchar_t * __restrict dst, const char ** __restrict src, size_t len,
    pbsd_mbstate_t * __restrict ps, pbsd_locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->mbsrtowcs);
	return (XLOCALE_CTYPE(locale)->__mbsnrtowcs(dst, src, SIZE_T_MAX, len, ps));
}
size_t
mbsrtowcs(wchar_t * __restrict dst, const char ** __restrict src, size_t len,
    pbsd_mbstate_t * __restrict ps)
{
	return mbsrtowcs_l(dst, src, len, ps, __get_locale());
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

/* lib/libc/locale/wcsrtombs.c */

size_t
wcsrtombs_l(char * __restrict dst, const wchar_t ** __restrict src, size_t len,
    pbsd_mbstate_t * __restrict ps, pbsd_locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->wcsrtombs);
	return (XLOCALE_CTYPE(locale)->__wcsnrtombs(dst, src, SIZE_T_MAX, len, ps));
}

size_t
wcsrtombs(char * __restrict dst, const wchar_t ** __restrict src, size_t len,
    pbsd_mbstate_t * __restrict ps)
{
	return wcsrtombs_l(dst, src, len, ps, __get_locale());
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

/* lib/libc/locale/mbtowc.c */

int
mbtowc_l(wchar_t * __restrict pwc, const char * __restrict s, size_t n, pbsd_locale_t locale)
{
	static const pbsd_mbstate_t initial{};
	size_t rval;
	FIX_LOCALE(locale);

	if (s == NULL) {
		/* No support for state dependent encodings. */
		XLOCALE_CTYPE(locale)->mbtowc = initial;
		return (0);
	}
	rval = XLOCALE_CTYPE(locale)->__mbrtowc(pwc, s, n,
	    &(XLOCALE_CTYPE(locale)->mbtowc));
	switch (rval) {
	case (size_t)-2:
		errno = EILSEQ;
		/* FALLTHROUGH */
	case (size_t)-1:
		return (-1);
	default:
		return ((int)rval);
	}
}
int
mbtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n)
{
	return mbtowc_l(pwc, s, n, __get_locale());
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

/* lib/libc/locale/wcwidth.c */

int
wcwidth(wchar_t wc)
{
	return (__wcwidth(wc));
}
int
wcwidth_l(wchar_t wc, pbsd_locale_t locale)
{
	return (__wcwidth_l(wc, locale));
}

} /* namespace pbsd::lib_libc_locale::b0039 */
