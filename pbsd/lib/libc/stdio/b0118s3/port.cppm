// PBSD batch b0118s3 -- C++23 port of HardenedBSD lib/libc/stdio/fgetws.c
//
// The ports below are behaviour-faithful transliterations of the C sources.
// Only what C++ strictly requires has been changed: the void * results of
// memchr() are explicitly cast.  Signedness, evaluation order, pointer
// arithmetic and every conditional are preserved exactly as written.
//
// The private stdio internals these functions are built on (the FILE layout,
// __srefill(), the xlocale conversion vector) are not part of this batch; they
// are declared here and supplied by the batch runtime with C linkage.

module;

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

export module pbsd.lib.libc.stdio.b0118s3;

// ---------------------------------------------------------------------
// Private stdio flags (FreeBSD sys/_stdio.h values).
// ---------------------------------------------------------------------

export inline constexpr int PB_SRD = 0x0004;
export inline constexpr int PB_SWR = 0x0008;
export inline constexpr int PB_SRW = 0x0010;
export inline constexpr int PB_SEOF = 0x0020;
export inline constexpr int PB_SERR = 0x0040;
export inline constexpr int PB_SAPP = 0x0100;

#define	__SRD	0x0004
#define	__SWR	0x0008
#define	__SRW	0x0010
#define	__SEOF	0x0020
#define	__SERR	0x0040
#define	__SAPP	0x0100

export inline constexpr int PB_BUFSZ = 256;

// ---------------------------------------------------------------------
// FILE layout and friends.  Must stay layout-identical to oracle.c.
// ---------------------------------------------------------------------

export struct pb_mbstate {
	int		__want;
	unsigned int	__ch;
	unsigned int	__lbound;
};

export using pb_mbstate_t = pb_mbstate;

export struct pb_file {
	unsigned char	*_p;
	int		 _r;
	int		 _w;
	short		 _flags;
	short		 _file;
	short		 _orientation;
	int		 _flags2;
	pb_mbstate_t	 _mbstate;
	void		*_cookie;
	/* mock plumbing */
	unsigned char	 _buf[PB_BUFSZ];
	const unsigned char *in_data;
	size_t		 in_len;
	size_t		 in_pos;
	size_t		 chunk;
	int		 fail_refill_at;
	int		 refill_calls;
};

export using pb_file_t = pb_file;

export struct xlocale_ctype {
	size_t	(*__mbsnrtowcs)(wchar_t * __restrict, const char ** __restrict,
		    size_t, size_t, pb_mbstate_t * __restrict);
	int	(*__mbsinit)(const pb_mbstate_t *);
};

export using pb_locale_t = xlocale_ctype *;

extern "C" {
int pb_srefill(pb_file_t *);
pb_locale_t pb_get_locale(void);
}

#define	FILE			pb_file_t
#define	locale_t		pb_locale_t
#define	FLOCKFILE_CANCELSAFE(fp)	{ {
#define	FUNLOCKFILE_CANCELSAFE()	} }
#define	ORIENT(fp, o)		do {					\
					if ((fp)->_orientation == 0)	\
						(fp)->_orientation = (o); \
				} while (0)
#define	__sfeof(fp)		(((fp)->_flags & __SEOF) != 0)
#define	__srefill		pb_srefill
#define	__get_locale()		pb_get_locale()
#define	FIX_LOCALE(l)		do {					\
					if ((l) == nullptr)		\
						(l) = __get_locale();	\
				} while (0)
#define	XLOCALE_CTYPE(l)	(l)

export namespace pbsd::lib_libc_stdio::b0118s3 {

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

/* lib/libc/stdio/fgetws.c */

wchar_t *
fgetws_l(wchar_t * __restrict ws, int n, FILE * __restrict fp, locale_t locale)
{
	int sret;
	wchar_t *wsp, *ret;
	size_t nconv;
	const char *src;
	unsigned char *nl;
	FIX_LOCALE(locale);
	struct xlocale_ctype *l = XLOCALE_CTYPE(locale);

	FLOCKFILE_CANCELSAFE(fp);
	ORIENT(fp, 1);

	if (n <= 0) {
		fp->_flags |= __SERR;
		errno = EINVAL;
		goto error;
	}

	wsp = ws;
	if (n == 1)
		goto ok;

	if (fp->_r <= 0 && __srefill(fp))
		/* EOF or ferror */
		goto error;

	sret = 0;
	do {
		src = (const char *)fp->_p;
		nl = (unsigned char *)memchr(fp->_p, '\n', fp->_r);
		nconv = l->__mbsnrtowcs(wsp, &src,
		    nl != NULL ? (nl - fp->_p + 1) : fp->_r,
		    n - 1, &fp->_mbstate);
		if (nconv == (size_t)-1) {
			/* Conversion error */
			fp->_flags |= __SERR;
			goto error;
		}
		if (src == NULL) {
			/*
			 * We hit a null byte. Increment the character count,
			 * since mbsnrtowcs()'s return value doesn't include
			 * the terminating null, then resume conversion
			 * after the null.
			 */
			nconv++;
			src = (const char *)memchr(fp->_p, '\0', fp->_r);
			src++;
		}
		fp->_r -= (unsigned char *)src - fp->_p;
		fp->_p = (unsigned char *)src;
		n -= nconv;
		wsp += nconv;
	} while ((wsp == ws || wsp[-1] != L'\n') && n > 1 && (fp->_r > 0 ||
	    (sret = __srefill(fp)) == 0));
	if (sret && !__sfeof(fp))
		/* ferror */
		goto error;
	if (!l->__mbsinit(&fp->_mbstate)) {
		/* Incomplete character */
		fp->_flags |= __SERR;
		errno = EILSEQ;
		goto error;
	}
	if (wsp == ws)
		/* EOF */
		goto error;
ok:
	*wsp = L'\0';
	ret = ws;
end:
	FUNLOCKFILE_CANCELSAFE();
	return (ret);

error:
	ret = NULL;
	goto end;
}

wchar_t *
fgetws(wchar_t * __restrict ws, int n, FILE * __restrict fp)
{
	return fgetws_l(ws, n, fp, __get_locale());
}

} // namespace pbsd::lib_libc_stdio::b0118s3
