/*
 * PBSD batch b0039 -- reference oracle.
 *
 * The four batch sources are concatenated below with every function renamed
 * with a `ref_' prefix.  The function bodies are UNMODIFIED.
 *
 * All four sources are thin wrappers over the libc locale runtime
 * (mblocal.h / xlocale_private.h): they dispatch through
 * XLOCALE_CTYPE(locale)->__mbsnrtowcs, ->__wcsnrtombs, ->__mbrtowc and
 * through __wcwidth()/__wcwidth_l().  That runtime is not part of this batch,
 * so the missing declarations, macros and a deterministic conversion backend
 * are supplied here as scaffolding.  The scaffolding is shared: port.cppm
 * declares the very same backend with C linkage and calls into these
 * definitions, so the differential test compares wrapper behaviour only.
 */

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

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/*
 * Missing defines.  FreeBSD spells the maximum value of size_t SIZE_T_MAX
 * (machine/_limits.h); LONG_BIT is not in the C11 <limits.h>.
 */
#ifndef SIZE_T_MAX
#define	SIZE_T_MAX	SIZE_MAX
#endif
#ifndef LONG_BIT
#define	LONG_BIT	(sizeof(long) * CHAR_BIT)
#endif

/*
 * Missing types.  <wchar.h> is deliberately not included: mbstate_t is
 * modelled here so that its contents are inspectable by the harness and so
 * that port.cppm can declare a layout-identical copy.
 */
typedef struct pbsd_mbstate {
	unsigned int	count;		/* bytes stashed in `lead' */
	unsigned int	lead;		/* stashed lead byte */
	unsigned long	touch;		/* bumped by every backend call */
} mbstate_t;

struct pbsd_xlocale_ctype;
struct pbsd_xlocale;
typedef struct pbsd_xlocale *locale_t;

struct pbsd_xlocale_ctype {
	size_t	(*__mbsnrtowcs)(wchar_t * __restrict,
		    const char ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);
	size_t	(*__wcsnrtombs)(char * __restrict,
		    const wchar_t ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);
	size_t	(*__mbrtowc)(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
	mbstate_t	mbsrtowcs;
	mbstate_t	wcsrtombs;
	mbstate_t	mbtowc;
};

struct pbsd_xlocale {
	struct pbsd_xlocale_ctype	*ctype;
	int				 id;
	int				 wide_width;
};

/* Missing defines from mblocal.h / xlocale_private.h. */
#define	FIX_LOCALE(loc)		if ((loc) == NULL) (loc) = __get_locale()
#define	XLOCALE_CTYPE(l)	((l)->ctype)

/* ------------------------------------------------------------------ */
/* Shared locale-runtime scaffolding.  Not part of the ported batch.   */
/* ------------------------------------------------------------------ */

typedef struct pbsd_trace_s {
	unsigned long	calls;
	unsigned long	null_ps;
	unsigned long	hash;
	unsigned long	last_nms;
	unsigned long	last_len;
	int		last_kind;
	int		last_flags;
	const void	*last_dst;
	const void	*last_src;
	const void	*last_ps;
	const void	*last_locale;
} pbsd_trace_t;

pbsd_trace_t	pbsd_trace;
mbstate_t	pbsd_static_state;
locale_t	pbsd_current_locale;

void
pbsd_reset_trace(void)
{
	memset(&pbsd_trace, 0, sizeof(pbsd_trace));
	memset(&pbsd_static_state, 0, sizeof(pbsd_static_state));
}

static void
pbsd_note(int kind, unsigned long nms, unsigned long len, int flags,
    const void *dst, const void *src, const void *ps, const void *loc)
{
	pbsd_trace.calls++;
	pbsd_trace.last_kind = kind;
	pbsd_trace.last_flags = flags;
	pbsd_trace.last_nms = nms;
	pbsd_trace.last_len = len;
	pbsd_trace.last_dst = dst;
	pbsd_trace.last_src = src;
	pbsd_trace.last_ps = ps;
	pbsd_trace.last_locale = loc;
	pbsd_trace.hash = pbsd_trace.hash * 1000003UL +
	    (unsigned long)kind * 97UL + (unsigned long)flags * 8191UL +
	    nms * 31UL + len * 7UL + 1UL;
}

locale_t
__get_locale(void)
{
	return (pbsd_current_locale);
}

/*
 * The modelled encoding: bytes 0x00-0x7f are single-byte characters, bytes
 * 0xc0-0xff introduce a two-byte character whose wide value is
 * 0x1000 + ((lead & 0x3f) << 8) + trail, and bytes 0x80-0xbf are illegal.
 */
static size_t
pbsd_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
{
	mbstate_t *st;
	const char *s;
	size_t nchr;
	size_t avail;

	pbsd_note(1, (unsigned long)nms, (unsigned long)len,
	    (dst == NULL ? 1 : 0) | (ps == NULL ? 2 : 0), dst,
	    (const void *)*src, ps, NULL);
	if (ps == NULL) {
		pbsd_trace.null_ps++;
		st = &pbsd_static_state;
	} else
		st = ps;
	st->touch++;
	s = *src;
	avail = nms;
	nchr = 0;
	if (dst == NULL)
		len = SIZE_T_MAX;
	while (len > 0) {
		wchar_t wc;
		size_t nb;

		if (avail == 0)
			break;
		if (st->count != 0) {
			wc = (wchar_t)(0x1000 +
			    (int)((st->lead & 0x3fu) << 8) +
			    (unsigned char)s[0]);
			st->count = 0;
			st->lead = 0;
			nb = 1;
		} else {
			unsigned char c = (unsigned char)s[0];

			if (c < 0x80) {
				wc = (wchar_t)c;
				nb = 1;
			} else if (c < 0xc0) {
				errno = EDOM;
				return ((size_t)-1);
			} else if (avail < 2) {
				st->count = 1;
				st->lead = c;
				break;
			} else {
				wc = (wchar_t)(0x1000 +
				    (int)((c & 0x3fu) << 8) +
				    (unsigned char)s[1]);
				nb = 2;
			}
		}
		if (wc == 0) {
			if (dst != NULL) {
				*dst = 0;
				*src = NULL;
			}
			return (nchr);
		}
		if (dst != NULL) {
			*dst = wc;
			dst++;
		}
		s += nb;
		avail -= nb;
		nchr++;
		len--;
	}
	if (dst != NULL)
		*src = s;
	return (nchr);
}

static size_t
pbsd_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	mbstate_t *st;
	const wchar_t *s;
	size_t nb;
	size_t avail;

	pbsd_note(2, (unsigned long)nwc, (unsigned long)len,
	    (dst == NULL ? 1 : 0) | (ps == NULL ? 2 : 0), dst,
	    (const void *)*src, ps, NULL);
	if (ps == NULL) {
		pbsd_trace.null_ps++;
		st = &pbsd_static_state;
	} else
		st = ps;
	st->touch++;
	s = *src;
	avail = nwc;
	nb = 0;
	if (dst == NULL)
		len = SIZE_T_MAX;
	while (avail > 0) {
		wchar_t wc;
		unsigned char buf[2];
		size_t n;

		if (st->count != 0) {
			if (len < 1)
				break;
			if (dst != NULL) {
				*dst = (char)(unsigned char)st->lead;
				dst++;
			}
			st->count = 0;
			st->lead = 0;
			len--;
			nb++;
			continue;
		}
		wc = *s;
		if (wc == 0) {
			if (len < 1)
				break;
			if (dst != NULL) {
				*dst = '\0';
				*src = NULL;
			}
			return (nb);
		}
		if (wc > 0 && wc < 0x80) {
			buf[0] = (unsigned char)wc;
			n = 1;
		} else if (wc >= 0x1000 && wc <= 0x4fff) {
			buf[0] = (unsigned char)(0xc0 |
			    (unsigned)((wc - 0x1000) >> 8));
			buf[1] = (unsigned char)((wc - 0x1000) & 0xff);
			n = 2;
		} else {
			errno = EDOM;
			return ((size_t)-1);
		}
		if (len < n)
			break;
		if (dst != NULL) {
			memcpy(dst, buf, n);
			dst += n;
		}
		len -= n;
		nb += n;
		s++;
		avail--;
	}
	if (dst != NULL)
		*src = s;
	return (nb);
}

static size_t
pbsd_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps)
{
	mbstate_t *st;
	unsigned char c;

	pbsd_note(3, (unsigned long)n, 0UL,
	    (pwc == NULL ? 1 : 0) | (ps == NULL ? 2 : 0) |
	    (s == NULL ? 4 : 0), pwc, s, ps, NULL);
	if (ps == NULL) {
		pbsd_trace.null_ps++;
		st = &pbsd_static_state;
	} else
		st = ps;
	st->touch++;
	if (s == NULL) {
		st->count = 0;
		st->lead = 0;
		return (0);
	}
	if (st->count != 0) {
		wchar_t wc;

		if (n == 0) {
			errno = ERANGE;
			return ((size_t)-2);
		}
		c = (unsigned char)s[0];
		wc = (wchar_t)(0x1000 + (int)((st->lead & 0x3fu) << 8) + c);
		st->count = 0;
		st->lead = 0;
		if (pwc != NULL)
			*pwc = wc;
		return ((size_t)1);
	}
	if (n == 0) {
		errno = ERANGE;
		return ((size_t)-2);
	}
	c = (unsigned char)s[0];
	if (c < 0x80) {
		if (pwc != NULL)
			*pwc = (wchar_t)c;
		return (c == 0 ? (size_t)0 : (size_t)1);
	}
	if (c < 0xc0) {
		errno = EDOM;
		return ((size_t)-1);
	}
	if (n < 2) {
		st->count = 1;
		st->lead = c;
		errno = ERANGE;
		return ((size_t)-2);
	}
	if (pwc != NULL)
		*pwc = (wchar_t)(0x1000 + (int)((c & 0x3fu) << 8) +
		    (unsigned char)s[1]);
	return ((size_t)2);
}

static int
pbsd_width(wchar_t wc, locale_t locale)
{
	int scale;

	if (locale == NULL)
		locale = pbsd_current_locale;
	scale = (locale == NULL) ? 1 : locale->wide_width;
	if (wc == 0)
		return (0);
	if (wc < 0)
		return (-1);
	if (wc < 0x20)
		return (-1);
	if (wc >= 0x7f && wc < 0xa0)
		return (-1);
	if (wc >= 0x300 && wc <= 0x36f)
		return (0);
	if (wc >= 0x1000 && wc <= 0x4fff)
		return (scale);
	return (1);
}

int
__wcwidth(wchar_t wc)
{
	pbsd_note(4, (unsigned long)(unsigned int)wc, 0UL, 0, NULL, NULL,
	    NULL, pbsd_current_locale);
	return (pbsd_width(wc, pbsd_current_locale));
}

int
__wcwidth_l(wchar_t wc, locale_t locale)
{
	pbsd_note(5, (unsigned long)(unsigned int)wc, 0UL,
	    (locale == NULL ? 1 : 0), NULL, NULL, NULL, locale);
	return (pbsd_width(wc, locale));
}

void
pbsd_init_locale(struct pbsd_xlocale *l, struct pbsd_xlocale_ctype *c, int id,
    int wide_width)
{
	memset(c, 0, sizeof(*c));
	memset(l, 0, sizeof(*l));
	c->__mbsnrtowcs = pbsd_mbsnrtowcs;
	c->__wcsnrtombs = pbsd_wcsnrtombs;
	c->__mbrtowc = pbsd_mbrtowc;
	l->ctype = c;
	l->id = id;
	l->wide_width = wide_width;
}

/* ------------------------------------------------------------------ */
/* lib/libc/locale/mbsrtowcs.c                                        */
/* ------------------------------------------------------------------ */

size_t
ref_mbsrtowcs_l(wchar_t * __restrict dst, const char ** __restrict src, size_t len,
    mbstate_t * __restrict ps, locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->mbsrtowcs);
	return (XLOCALE_CTYPE(locale)->__mbsnrtowcs(dst, src, SIZE_T_MAX, len, ps));
}
size_t
ref_mbsrtowcs(wchar_t * __restrict dst, const char ** __restrict src, size_t len,
    mbstate_t * __restrict ps)
{
	return ref_mbsrtowcs_l(dst, src, len, ps, __get_locale());
}

/* ------------------------------------------------------------------ */
/* lib/libc/locale/wcsrtombs.c                                        */
/* ------------------------------------------------------------------ */

size_t
ref_wcsrtombs_l(char * __restrict dst, const wchar_t ** __restrict src, size_t len,
    mbstate_t * __restrict ps, locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &(XLOCALE_CTYPE(locale)->wcsrtombs);
	return (XLOCALE_CTYPE(locale)->__wcsnrtombs(dst, src, SIZE_T_MAX, len, ps));
}

size_t
ref_wcsrtombs(char * __restrict dst, const wchar_t ** __restrict src, size_t len,
    mbstate_t * __restrict ps)
{
	return ref_wcsrtombs_l(dst, src, len, ps, __get_locale());
}

/* ------------------------------------------------------------------ */
/* lib/libc/locale/mbtowc.c                                           */
/* ------------------------------------------------------------------ */

int
ref_mbtowc_l(wchar_t * __restrict pwc, const char * __restrict s, size_t n, locale_t locale)
{
	static const mbstate_t initial;
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
ref_mbtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n)
{
	return ref_mbtowc_l(pwc, s, n, __get_locale());
}

/* ------------------------------------------------------------------ */
/* lib/libc/locale/wcwidth.c                                          */
/* ------------------------------------------------------------------ */

#undef wcwidth

int
ref_wcwidth(wchar_t wc)
{
	return (__wcwidth(wc));
}
int
ref_wcwidth_l(wchar_t wc, locale_t locale)
{
	return (__wcwidth_l(wc, locale));
}
