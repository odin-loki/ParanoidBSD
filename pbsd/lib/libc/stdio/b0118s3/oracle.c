/*
 * oracle.c -- reference implementations for batch b0118s3.
 *
 * The source file of this batch is concatenated below with every function
 * renamed with a "ref_" prefix.  The function bodies are otherwise
 * unmodified.
 *
 * The batch operates on the private FreeBSD stdio FILE internals (_p, _r,
 * _flags, __srefill(), the xlocale ctype conversion vector, ...) which do
 * not exist on this host.  A self contained mock of exactly those internals
 * is provided first; it is compiled into this translation unit and shared
 * verbatim by the C++ port, so that the reference and the port execute
 * against bit-identical infrastructure and any observed difference is a
 * difference between the two function bodies.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>
#include <limits.h>

/* ------------------------------------------------------------------ */
/* Mock of <stdio.h> private flags (FreeBSD sys/_stdio.h values).      */
/* ------------------------------------------------------------------ */

#define	__SRD	0x0004		/* OK to read */
#define	__SWR	0x0008		/* OK to write */
#define	__SRW	0x0010		/* open for reading & writing */
#define	__SEOF	0x0020		/* found EOF */
#define	__SERR	0x0040		/* found error */
#define	__SAPP	0x0100		/* fdopen()ed in append mode */

#define	PB_BUFSZ	256

typedef struct pb_mbstate {
	int		__want;
	unsigned int	__ch;
	unsigned int	__lbound;
} pb_mbstate_t;

typedef struct pb_file {
	unsigned char	*_p;		/* current position in (some) buffer */
	int		 _r;		/* read space left for getc() */
	int		 _w;		/* write space left for putc() */
	short		 _flags;	/* flags, below */
	short		 _file;		/* fileno, if Unix descriptor */
	short		 _orientation;	/* orientation for fwide() */
	int		 _flags2;
	pb_mbstate_t	 _mbstate;	/* multibyte conversion state */
	void		*_cookie;
	/* mock plumbing */
	unsigned char	 _buf[PB_BUFSZ];
	const unsigned char *in_data;
	size_t		 in_len;
	size_t		 in_pos;
	size_t		 chunk;
	int		 fail_refill_at;
	int		 refill_calls;
} pb_file_t;

struct xlocale_ctype {
	size_t	(*__mbsnrtowcs)(wchar_t * __restrict, const char ** __restrict,
		    size_t, size_t, pb_mbstate_t * __restrict);
	int	(*__mbsinit)(const pb_mbstate_t *);
};

typedef struct xlocale_ctype *pb_locale_t;

/* ------------------------------------------------------------------ */
/* Mock character conversion (a UTF-8 locale and a single byte locale).*/
/* ------------------------------------------------------------------ */

static size_t
pb_utf8_mbrtowc(wchar_t *pwc, const char *s, size_t n, pb_mbstate_t *ps)
{
	const unsigned char *us;
	unsigned int ch, wch, lbound;
	int want, i, mask;

	if (n == 0)
		return ((size_t)-2);
	us = (const unsigned char *)s;
	if (ps->__want == 0) {
		ch = us[0];
		if ((ch & 0x80) == 0) {
			mask = 0x7f;
			want = 1;
			lbound = 0;
		} else if ((ch & 0xe0) == 0xc0) {
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
		wch = ch & (unsigned int)mask;
		us++;
		i = 1;
	} else {
		want = ps->__want;
		lbound = ps->__lbound;
		wch = ps->__ch;
		i = 0;
	}
	for (; i < want && (size_t)i < n; i++) {
		if ((us[0] & 0xc0) != 0x80) {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wch = (wch << 6) | (unsigned int)(us[0] & 0x3f);
		us++;
	}
	if (i < want) {
		ps->__want = want - i;
		ps->__lbound = lbound;
		ps->__ch = wch;
		return ((size_t)-2);
	}
	if (wch < lbound) {
		errno = EILSEQ;
		return ((size_t)-1);
	}
	ps->__want = 0;
	ps->__ch = 0;
	ps->__lbound = 0;
	if (pwc != NULL)
		*pwc = (wchar_t)wch;
	return (wch == 0 ? 0 : (size_t)want);
}

static size_t
pb_sb_mbrtowc(wchar_t *pwc, const char *s, size_t n, pb_mbstate_t *ps)
{
	unsigned int ch;

	(void)ps;
	if (n == 0)
		return ((size_t)-2);
	ch = (unsigned char)s[0];
	if (pwc != NULL)
		*pwc = (wchar_t)ch;
	return (ch == 0 ? 0 : 1);
}

static size_t
pb_mbsnrtowcs_std(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, pb_mbstate_t * __restrict ps,
    size_t (*pmbrtowc)(wchar_t *, const char *, size_t, pb_mbstate_t *))
{
	const char *s;
	size_t nchr;
	wchar_t wc;
	size_t nb;

	s = *src;
	nchr = 0;

	if (dst == NULL) {
		for (;;) {
			if ((nb = pmbrtowc(&wc, s, nms, ps)) == (size_t)-1)
				return ((size_t)-1);
			else if (nb == 0 || nb == (size_t)-2)
				return (nchr);
			s += nb;
			nms -= nb;
			nchr++;
		}
	}

	while (len-- > 0) {
		if ((nb = pmbrtowc(dst, s, nms, ps)) == (size_t)-1) {
			*src = s;
			return ((size_t)-1);
		} else if (nb == (size_t)-2) {
			*src = s + nms;
			return (nchr);
		} else if (nb == 0) {
			*src = NULL;
			return (nchr);
		}
		s += nb;
		nms -= nb;
		nchr++;
		dst++;
	}
	*src = s;
	return (nchr);
}

static size_t
pb_utf8_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, pb_mbstate_t * __restrict ps)
{
	return (pb_mbsnrtowcs_std(dst, src, nms, len, ps, pb_utf8_mbrtowc));
}

static size_t
pb_sb_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, pb_mbstate_t * __restrict ps)
{
	return (pb_mbsnrtowcs_std(dst, src, nms, len, ps, pb_sb_mbrtowc));
}

static int
pb_utf8_mbsinit(const pb_mbstate_t *ps)
{
	return (ps->__want == 0);
}

static int
pb_sb_mbsinit(const pb_mbstate_t *ps)
{
	(void)ps;
	return (1);
}

struct xlocale_ctype pb_utf8_locale = { pb_utf8_mbsnrtowcs, pb_utf8_mbsinit };
struct xlocale_ctype pb_sb_locale = { pb_sb_mbsnrtowcs, pb_sb_mbsinit };
pb_locale_t pb_global_locale = &pb_utf8_locale;

pb_locale_t
pb_get_locale(void)
{
	return (pb_global_locale);
}

/* ------------------------------------------------------------------ */
/* Mock FILE plumbing.                                                 */
/* ------------------------------------------------------------------ */

void
pb_file_init(pb_file_t *fp)
{
	memset(fp, 0, sizeof(*fp));
	memset(fp->_buf, 0xaa, sizeof(fp->_buf));
	fp->_file = -1;
}

int
pb_srefill(pb_file_t *fp)
{
	size_t n;

	fp->refill_calls++;
	if (fp->fail_refill_at != 0 && fp->refill_calls == fp->fail_refill_at) {
		fp->_flags |= __SERR;
		fp->_p = fp->_buf;
		fp->_r = 0;
		return (-1);
	}
	if (fp->in_pos >= fp->in_len) {
		fp->_flags |= __SEOF;
		fp->_p = fp->_buf;
		fp->_r = 0;
		return (-1);
	}
	n = fp->in_len - fp->in_pos;
	if (fp->chunk != 0 && n > fp->chunk)
		n = fp->chunk;
	if (n > PB_BUFSZ)
		n = PB_BUFSZ;
	memcpy(fp->_buf, fp->in_data + fp->in_pos, n);
	fp->in_pos += n;
	fp->_p = fp->_buf;
	fp->_r = (int)n;
	return (0);
}

void
pb_file_input(pb_file_t *fp, const unsigned char *data, size_t len,
    size_t chunk, int fail_at, int prefill)
{
	fp->in_data = data;
	fp->in_len = len;
	fp->in_pos = 0;
	fp->chunk = chunk;
	fp->fail_refill_at = fail_at;
	fp->refill_calls = 0;
	fp->_p = NULL;
	fp->_r = 0;
	if (prefill) {
		(void)pb_srefill(fp);
		fp->refill_calls = 0;
		fp->_flags = (short)(fp->_flags & ~(__SEOF | __SERR));
	}
}

/* ------------------------------------------------------------------ */
/* Glue so that the function bodies below can stay verbatim.           */
/* ------------------------------------------------------------------ */

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
					if ((l) == NULL)		\
						(l) = __get_locale();	\
				} while (0)
#define	XLOCALE_CTYPE(l)	(l)

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
ref_fgetws_l(wchar_t * __restrict ws, int n, FILE * __restrict fp, locale_t locale)
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
		src = fp->_p;
		nl = memchr(fp->_p, '\n', fp->_r);
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
			src = memchr(fp->_p, '\0', fp->_r);
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
ref_fgetws(wchar_t * __restrict ws, int n, FILE * __restrict fp)
{
	return ref_fgetws_l(ws, n, fp, __get_locale());
}
