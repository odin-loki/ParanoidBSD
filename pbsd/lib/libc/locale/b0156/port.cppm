/*
 * PBSD batch b0156 -- C++23 module port of
 *
 *	lib/libc/locale/utf8.c
 *	lib/libc/locale/euc.c
 *
 * This is a faithful transliteration: behaviour, integer signedness,
 * evaluation order, pointer arithmetic and the original bugs (e.g. euc.c
 * setting errno = EILSEQ on the incomplete-sequence path that returns
 * (size_t)-2) are preserved exactly.  Nothing is improved.
 *
 * lib/libc/locale/table.c and lib/libc/locale/collate.c are not part of this
 * module; see skipped.txt.
 */

module;

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

export module pbsd.lib.libc.locale.b0156;

export namespace pbsd::lib_libc_locale::b0156 {

/* ------------------------------------------------------------------- */
/* Support prologue -- definitions normally provided by FreeBSD headers */
/* ------------------------------------------------------------------- */

/*
 * FreeBSD's mbstate_t is a 128-byte opaque union (sys/_types.h,
 * __mbstate_t).  glibc's is only 8 bytes, which is too small to hold
 * _UTF8State/_EucState, so the FreeBSD shape is reproduced here.
 */
union pbsd_mbstate_t {
	char		__mbstate8[128];
	long long	_mbstateL;
};

/* runetype.h -- only ever used as an opaque pointer here. */
struct _RuneLocale;

}  /* namespace pbsd::lib_libc_locale::b0156 */

#define	mbstate_t	::pbsd::lib_libc_locale::b0156::pbsd_mbstate_t
#define	_RuneLocale	::pbsd::lib_libc_locale::b0156::_RuneLocale

/* sys/param.h */
#undef	MIN
#define	MIN(a,b)	(((a)<(b))?(a):(b))

/*
 * stdlib.h's MB_CUR_MAX is __mb_cur_max() on FreeBSD, i.e. the current ctype
 * locale's value.  Both euc.c and utf8.c read it.  It is pinned to 4 (the
 * value a UTF-8 / EUC-CN / EUC-TW locale installs) so the port is
 * deterministic; the oracle pins it identically.
 */
#undef	MB_CUR_MAX
#define	MB_CUR_MAX	((size_t)4)

/* EUC single shift bytes (ISO 2022 SS2/SS3). */
#define	SS2		0x8e
#define	SS3		0x8f

export namespace pbsd::lib_libc_locale::b0156 {

/*
 * xlocale_private.h -- the subset of struct xlocale_ctype that the *_init()
 * functions in this batch assign to.
 */
typedef size_t (*__pbsd_mbrtowc_pfn)(wchar_t * __restrict,
    const char * __restrict, size_t, mbstate_t * __restrict);
typedef int (*__pbsd_mbsinit_pfn)(const mbstate_t *);
typedef size_t (*__pbsd_mbsnrtowcs_pfn)(wchar_t * __restrict,
    const char ** __restrict, size_t, size_t, mbstate_t * __restrict);
typedef size_t (*__pbsd_wcrtomb_pfn)(char * __restrict, wchar_t,
    mbstate_t * __restrict);
typedef size_t (*__pbsd_wcsnrtombs_pfn)(char * __restrict,
    const wchar_t ** __restrict, size_t, size_t, mbstate_t * __restrict);

struct xlocale_ctype {
	_RuneLocale		*runes;
	__pbsd_mbrtowc_pfn	__mbrtowc;
	__pbsd_mbsinit_pfn	__mbsinit;
	__pbsd_mbsnrtowcs_pfn	__mbsnrtowcs;
	__pbsd_wcrtomb_pfn	__wcrtomb;
	__pbsd_wcsnrtombs_pfn	__wcsnrtombs;
	int			__mb_cur_max;
	int			__mb_sb_limit;
};

/*
 * mblocal.h declares __mbsnrtowcs_std()/__wcsnrtombs_std(); they are defined
 * in lib/libc/locale/mbsnrtowcs.c and lib/libc/locale/wcsnrtombs.c, which are
 * outside this batch.  euc.c cannot link without them, so they are reproduced
 * at the bottom of this module as support code (oracle.c carries an identical
 * copy).  They are not part of the batch proper.
 */
typedef size_t (*mbrtowc_pfn_t)(wchar_t * __restrict, const char * __restrict,
    size_t, mbstate_t * __restrict);
typedef size_t (*wcrtomb_pfn_t)(char * __restrict, wchar_t,
    mbstate_t * __restrict);

size_t	__mbsnrtowcs_std(wchar_t * __restrict, const char ** __restrict,
	    size_t, size_t, mbstate_t * __restrict, mbrtowc_pfn_t);
size_t	__wcsnrtombs_std(char * __restrict, const wchar_t ** __restrict,
	    size_t, size_t, mbstate_t * __restrict, wcrtomb_pfn_t);

/* =================================================================== */
/* lib/libc/locale/utf8.c                                              */
/* =================================================================== */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2013 Garrett D'Amore <garrett@damore.org>
 * Copyright 2011 Nexenta Systems, Inc.  All rights reserved.
 * Copyright (c) 2002-2004 Tim J. Robbins
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

size_t	_UTF8_mbrtowc(wchar_t * __restrict, const char * __restrict,
	    size_t, mbstate_t * __restrict);
int	_UTF8_mbsinit(const mbstate_t *);
size_t	_UTF8_mbsnrtowcs(wchar_t * __restrict,
	    const char ** __restrict, size_t, size_t,
	    mbstate_t * __restrict);
size_t	_UTF8_wcrtomb(char * __restrict, wchar_t,
	    mbstate_t * __restrict);
size_t	_UTF8_wcsnrtombs(char * __restrict, const wchar_t ** __restrict,
	    size_t, size_t, mbstate_t * __restrict);

typedef struct {
	wchar_t	ch;
	int	want;
	wchar_t	lbound;
} _UTF8State;

int
_UTF8_init(struct xlocale_ctype *l, _RuneLocale *rl)
{

	l->__mbrtowc = _UTF8_mbrtowc;
	l->__wcrtomb = _UTF8_wcrtomb;
	l->__mbsinit = _UTF8_mbsinit;
	l->__mbsnrtowcs = _UTF8_mbsnrtowcs;
	l->__wcsnrtombs = _UTF8_wcsnrtombs;
	l->runes = rl;
	l->__mb_cur_max = 4;
	/*
	 * UCS-4 encoding used as the internal representation, so
	 * slots 0x0080-0x00FF are occuped and must be excluded
	 * from the single byte ctype by setting the limit.
	 */
	l->__mb_sb_limit = 128;

	return (0);
}

int
_UTF8_mbsinit(const mbstate_t *ps)
{

	return (ps == NULL || ((const _UTF8State *)ps)->want == 0);
}

size_t
_UTF8_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps)
{
	_UTF8State *us;
	int ch, i, mask, want;
	wchar_t lbound, wch;

	us = (_UTF8State *)ps;

	if (us->want < 0 || us->want > 6) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL) {
		s = "";
		n = 1;
		pwc = NULL;
	}

	if (n == 0)
		/* Incomplete multibyte sequence */
		return ((size_t)-2);

	if (us->want == 0) {
		/*
		 * Determine the number of octets that make up this character
		 * from the first octet, and a mask that extracts the
		 * interesting bits of the first octet. We already know
		 * the character is at least two bytes long.
		 *
		 * We also specify a lower bound for the character code to
		 * detect redundant, non-"shortest form" encodings. For
		 * example, the sequence C0 80 is _not_ a legal representation
		 * of the null character. This enforces a 1-to-1 mapping
		 * between character codes and their multibyte representations.
		 */
		ch = (unsigned char)*s;
		if ((ch & 0x80) == 0) {
			/* Fast path for plain ASCII characters. */
			if (pwc != NULL)
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
			/*
			 * Malformed input; input is not UTF-8.
			 */
			errno = EILSEQ;
			return ((size_t)-1);
		}
	} else {
		want = us->want;
		lbound = us->lbound;
	}

	/*
	 * Decode the octet sequence representing the character in chunks
	 * of 6 bits, most significant first.
	 */
	if (us->want == 0)
		wch = (unsigned char)*s++ & mask;
	else
		wch = us->ch;

	for (i = (us->want == 0) ? 1 : 0; i < MIN(want, n); i++) {
		if ((*s & 0xc0) != 0x80) {
			/*
			 * Malformed input; bad characters in the middle
			 * of a character.
			 */
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wch <<= 6;
		wch |= *s++ & 0x3f;
	}
	if (i < want) {
		/* Incomplete multibyte sequence. */
		us->want = want - i;
		us->lbound = lbound;
		us->ch = wch;
		return ((size_t)-2);
	}
	if (wch < lbound) {
		/*
		 * Malformed input; redundant encoding.
		 */
		errno = EILSEQ;
		return ((size_t)-1);
	}
	if ((wch >= 0xd800 && wch <= 0xdfff) || wch > 0x10ffff) {
		/*
		 * Malformed input; invalid code points.
		 */
		errno = EILSEQ;
		return ((size_t)-1);
	}
	if (pwc != NULL)
		*pwc = wch;
	us->want = 0;
	return (wch == L'\0' ? 0 : want);
}

size_t
_UTF8_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
{
	_UTF8State *us;
	const char *s;
	size_t nchr;
	wchar_t wc;
	size_t nb;

	us = (_UTF8State *)ps;

	s = *src;
	nchr = 0;

	if (dst == NULL) {
		/*
		 * The fast path in the loop below is not safe if an ASCII
		 * character appears as anything but the first byte of a
		 * multibyte sequence. Check now to avoid doing it in the loop.
		 */
		if (nms > 0 && us->want > 0 && (signed char)*s > 0) {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		for (;;) {
			if (nms > 0 && (signed char)*s > 0)
				/*
				 * Fast path for plain ASCII characters
				 * excluding NUL.
				 */
				nb = 1;
			else if ((nb = _UTF8_mbrtowc(&wc, s, nms, ps)) ==
			    (size_t)-1)
				/* Invalid sequence - mbrtowc() sets errno. */
				return ((size_t)-1);
			else if (nb == 0 || nb == (size_t)-2)
				return (nchr);
			s += nb;
			nms -= nb;
			nchr++;
		}
		/*NOTREACHED*/
	}

	/*
	 * The fast path in the loop below is not safe if an ASCII
	 * character appears as anything but the first byte of a
	 * multibyte sequence. Check now to avoid doing it in the loop.
	 */
	if (nms > 0 && len > 0 && us->want > 0 && (signed char)*s > 0) {
		errno = EILSEQ;
		return ((size_t)-1);
	}
	while (len-- > 0) {
		if (nms > 0 && (signed char)*s > 0) {
			/*
			 * Fast path for plain ASCII characters
			 * excluding NUL.
			 */
			*dst = (wchar_t)*s;
			nb = 1;
		} else if ((nb = _UTF8_mbrtowc(dst, s, nms, ps)) ==
		    (size_t)-1) {
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

size_t
_UTF8_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	_UTF8State *us;
	unsigned char lead;
	int i, len;

	us = (_UTF8State *)ps;

	if (us->want != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);

	/*
	 * Determine the number of octets needed to represent this character.
	 * We always output the shortest sequence possible. Also specify the
	 * first few bits of the first octet, which contains the information
	 * about the sequence length.
	 */
	if ((wc & ~0x7f) == 0) {
		/* Fast path for plain ASCII characters. */
		*s = (char)wc;
		return (1);
	} else if ((wc & ~0x7ff) == 0) {
		lead = 0xc0;
		len = 2;
	} else if ((wc & ~0xffff) == 0) {
		if (wc >= 0xd800 && wc <= 0xdfff) {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		lead = 0xe0;
		len = 3;
	} else if (wc >= 0 && wc <= 0x10ffff) {
		lead = 0xf0;
		len = 4;
	} else {
		errno = EILSEQ;
		return ((size_t)-1);
	}

	/*
	 * Output the octets representing the character in chunks
	 * of 6 bits, least significant last. The first octet is
	 * a special case because it contains the sequence length
	 * information.
	 */
	for (i = len - 1; i > 0; i--) {
		s[i] = (wc & 0x3f) | 0x80;
		wc >>= 6;
	}
	*s = (wc & 0xff) | lead;

	return (len);
}

size_t
_UTF8_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	_UTF8State *us;
	char buf[MB_LEN_MAX];
	const wchar_t *s;
	size_t nbytes;
	size_t nb;

	us = (_UTF8State *)ps;

	if (us->want != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	s = *src;
	nbytes = 0;

	if (dst == NULL) {
		while (nwc-- > 0) {
			if (0 <= *s && *s < 0x80)
				/* Fast path for plain ASCII characters. */
				nb = 1;
			else if ((nb = _UTF8_wcrtomb(buf, *s, ps)) ==
			    (size_t)-1)
				/* Invalid character - wcrtomb() sets errno. */
				return ((size_t)-1);
			if (*s == L'\0')
				return (nbytes + nb - 1);
			s++;
			nbytes += nb;
		}
		return (nbytes);
	}

	while (len > 0 && nwc-- > 0) {
		if (0 <= *s && *s < 0x80) {
			/* Fast path for plain ASCII characters. */
			nb = 1;
			*dst = *s;
		} else if (len > (size_t)MB_CUR_MAX) {
			/* Enough space to translate in-place. */
			if ((nb = _UTF8_wcrtomb(dst, *s, ps)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
		} else {
			/*
			 * May not be enough space; use temp. buffer.
			 */
			if ((nb = _UTF8_wcrtomb(buf, *s, ps)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
			if (nb > (int)len)
				/* MB sequence for character won't fit. */
				break;
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

/* =================================================================== */
/* lib/libc/locale/euc.c                                               */
/* =================================================================== */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2013 Garrett D'Amore <garrett@damore.org>
 * Copyright 2011 Nexenta Systems, Inc.  All rights reserved.
 * Copyright (c) 2002-2004 Tim J. Robbins. All rights reserved.
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
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

size_t	_EUC_mbrtowc_impl(wchar_t * __restrict, const char * __restrict,
    size_t, mbstate_t * __restrict, uint8_t, uint8_t, uint8_t, uint8_t);
size_t	_EUC_wcrtomb_impl(char * __restrict, wchar_t,
    mbstate_t * __restrict, uint8_t, uint8_t, uint8_t, uint8_t);

size_t	_EUC_CN_mbrtowc(wchar_t * __restrict, const char * __restrict,
	    size_t, mbstate_t * __restrict);
size_t	_EUC_JP_mbrtowc(wchar_t * __restrict, const char * __restrict,
	    size_t, mbstate_t * __restrict);
size_t	_EUC_KR_mbrtowc(wchar_t * __restrict, const char * __restrict,
	    size_t, mbstate_t * __restrict);
size_t	_EUC_TW_mbrtowc(wchar_t * __restrict, const char * __restrict,
	    size_t, mbstate_t * __restrict);

size_t	_EUC_CN_wcrtomb(char * __restrict, wchar_t,
	    mbstate_t * __restrict);
size_t	_EUC_JP_wcrtomb(char * __restrict, wchar_t,
	    mbstate_t * __restrict);
size_t	_EUC_KR_wcrtomb(char * __restrict, wchar_t,
	    mbstate_t * __restrict);
size_t	_EUC_TW_wcrtomb(char * __restrict, wchar_t,
	    mbstate_t * __restrict);

size_t	_EUC_CN_mbsnrtowcs(wchar_t * __restrict,
	    const char ** __restrict, size_t, size_t,
	    mbstate_t * __restrict);
size_t	_EUC_JP_mbsnrtowcs(wchar_t * __restrict,
	    const char ** __restrict, size_t, size_t,
	    mbstate_t * __restrict);
size_t	_EUC_KR_mbsnrtowcs(wchar_t * __restrict,
	    const char ** __restrict, size_t, size_t,
	    mbstate_t * __restrict);
size_t	_EUC_TW_mbsnrtowcs(wchar_t * __restrict,
	    const char ** __restrict, size_t, size_t,
	    mbstate_t * __restrict);

size_t	_EUC_CN_wcsnrtombs(char * __restrict,
	    const wchar_t ** __restrict, size_t, size_t,
	    mbstate_t * __restrict);
size_t	_EUC_JP_wcsnrtombs(char * __restrict,
	    const wchar_t ** __restrict, size_t, size_t,
	    mbstate_t * __restrict);
size_t	_EUC_KR_wcsnrtombs(char * __restrict,
	    const wchar_t ** __restrict, size_t, size_t,
	    mbstate_t * __restrict);
size_t	_EUC_TW_wcsnrtombs(char * __restrict,
	    const wchar_t ** __restrict, size_t, size_t,
	    mbstate_t * __restrict);

int	_EUC_mbsinit(const mbstate_t *);

typedef struct {
	wchar_t	ch;
	int	set;
	int	want;
} _EucState;

int
_EUC_mbsinit(const mbstate_t *ps)
{

	return (ps == NULL || ((const _EucState *)ps)->want == 0);
}

/*
 * EUC-CN uses CS0, CS1 and CS2 (4 bytes).
 */
int
_EUC_CN_init(struct xlocale_ctype *l, _RuneLocale *rl)
{
	l->__mbrtowc = _EUC_CN_mbrtowc;
	l->__wcrtomb = _EUC_CN_wcrtomb;
	l->__mbsnrtowcs = _EUC_CN_mbsnrtowcs;
	l->__wcsnrtombs = _EUC_CN_wcsnrtombs;
	l->__mbsinit = _EUC_mbsinit;

	l->runes = rl;
	l->__mb_cur_max = 4;
	l->__mb_sb_limit = 128;
	return (0);
}

size_t
_EUC_CN_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
    size_t n, mbstate_t * __restrict ps)
{
	return (_EUC_mbrtowc_impl(pwc, s, n, ps, SS2, 4, 0, 0));
}

size_t
_EUC_CN_mbsnrtowcs(wchar_t * __restrict dst,
    const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
{
	return (__mbsnrtowcs_std(dst, src, nms, len, ps, _EUC_CN_mbrtowc));
}

size_t
_EUC_CN_wcrtomb(char * __restrict s, wchar_t wc,
    mbstate_t * __restrict ps)
{
	return (_EUC_wcrtomb_impl(s, wc, ps, SS2, 4, 0, 0));
}

size_t
_EUC_CN_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
	size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	return (__wcsnrtombs_std(dst, src, nwc, len, ps, _EUC_CN_wcrtomb));
}

/*
 * EUC-KR uses only CS0 and CS1.
 */
int
_EUC_KR_init(struct xlocale_ctype *l, _RuneLocale *rl)
{
	l->__mbrtowc = _EUC_KR_mbrtowc;
	l->__wcrtomb = _EUC_KR_wcrtomb;
	l->__mbsnrtowcs = _EUC_KR_mbsnrtowcs;
	l->__wcsnrtombs = _EUC_KR_wcsnrtombs;
	l->__mbsinit = _EUC_mbsinit;

	l->runes = rl;
	l->__mb_cur_max = 2;
	l->__mb_sb_limit = 128;
	return (0);
}

size_t
_EUC_KR_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
    size_t n, mbstate_t * __restrict ps)
{
	return (_EUC_mbrtowc_impl(pwc, s, n, ps, 0, 0, 0, 0));
}

size_t
_EUC_KR_mbsnrtowcs(wchar_t * __restrict dst,
    const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
{
	return (__mbsnrtowcs_std(dst, src, nms, len, ps, _EUC_KR_mbrtowc));
}

size_t
_EUC_KR_wcrtomb(char * __restrict s, wchar_t wc,
	mbstate_t * __restrict ps)
{
	return (_EUC_wcrtomb_impl(s, wc, ps, 0, 0, 0, 0));
}

size_t
_EUC_KR_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
	size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	return (__wcsnrtombs_std(dst, src, nwc, len, ps, _EUC_KR_wcrtomb));
}

/*
 * EUC-JP uses CS0, CS1, CS2, and CS3.
 */
int
_EUC_JP_init(struct xlocale_ctype *l, _RuneLocale *rl)
{
	l->__mbrtowc = _EUC_JP_mbrtowc;
	l->__wcrtomb = _EUC_JP_wcrtomb;
	l->__mbsnrtowcs = _EUC_JP_mbsnrtowcs;
	l->__wcsnrtombs = _EUC_JP_wcsnrtombs;
	l->__mbsinit = _EUC_mbsinit;

	l->runes = rl;
	l->__mb_cur_max = 3;
	l->__mb_sb_limit = 128;
	return (0);
}

size_t
_EUC_JP_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
    size_t n, mbstate_t * __restrict ps)
{
	return (_EUC_mbrtowc_impl(pwc, s, n, ps, SS2, 2, SS3, 3));
}

size_t
_EUC_JP_mbsnrtowcs(wchar_t * __restrict dst,
    const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
{
	return (__mbsnrtowcs_std(dst, src, nms, len, ps, _EUC_JP_mbrtowc));
}

size_t
_EUC_JP_wcrtomb(char * __restrict s, wchar_t wc,
    mbstate_t * __restrict ps)
{
	return (_EUC_wcrtomb_impl(s, wc, ps, SS2, 2, SS3, 3));
}

size_t
_EUC_JP_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
	size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	return (__wcsnrtombs_std(dst, src, nwc, len, ps, _EUC_JP_wcrtomb));
}

/*
 * EUC-TW uses CS0, CS1, and CS2.
 */
int
_EUC_TW_init(struct xlocale_ctype *l, _RuneLocale *rl)
{
	l->__mbrtowc = _EUC_TW_mbrtowc;
	l->__wcrtomb = _EUC_TW_wcrtomb;
	l->__mbsnrtowcs = _EUC_TW_mbsnrtowcs;
	l->__wcsnrtombs = _EUC_TW_wcsnrtombs;
	l->__mbsinit = _EUC_mbsinit;

	l->runes = rl;
	l->__mb_cur_max = 4;
	l->__mb_sb_limit = 128;
	return (0);
}

size_t
_EUC_TW_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
	size_t n, mbstate_t * __restrict ps)
{
	return (_EUC_mbrtowc_impl(pwc, s, n, ps, SS2, 4, 0, 0));
}

size_t
_EUC_TW_mbsnrtowcs(wchar_t * __restrict dst,
	const char ** __restrict src,
	size_t nms, size_t len, mbstate_t * __restrict ps)
{
	return (__mbsnrtowcs_std(dst, src, nms, len, ps, _EUC_TW_mbrtowc));
}

size_t
_EUC_TW_wcrtomb(char * __restrict s, wchar_t wc,
	mbstate_t * __restrict ps)
{
	return (_EUC_wcrtomb_impl(s, wc, ps, SS2, 4, 0, 0));
}

size_t
_EUC_TW_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
	size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	return (__wcsnrtombs_std(dst, src, nwc, len, ps, _EUC_TW_wcrtomb));
}

/*
 * Common EUC code.
 */

size_t
_EUC_mbrtowc_impl(wchar_t * __restrict pwc, const char * __restrict s,
	size_t n, mbstate_t * __restrict ps,
	uint8_t cs2, uint8_t cs2width, uint8_t cs3, uint8_t cs3width)
{
	_EucState *es;
	int i, want;
	wchar_t wc = 0;
	unsigned char ch, chs;

	es = (_EucState *)ps;

	if (es->want < 0 || es->want > MB_CUR_MAX) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL) {
		s = "";
		n = 1;
		pwc = NULL;
	}

	if (n == 0)
		/* Incomplete multibyte sequence */
		return ((size_t)-2);

	if (es->want == 0) {
		/* Fast path for plain ASCII (CS0) */
		if (((ch = (unsigned char)*s) & 0x80) == 0) {
			if (pwc != NULL)
				*pwc = ch;
			return (ch != '\0' ? 1 : 0);
		}

		if (ch >= 0xa1) {
			/* CS1 */
			want = 2;
		} else if (ch == cs2) {
			want = cs2width;
		} else if (ch == cs3) {
			want = cs3width;
		} else {
			errno = EILSEQ;
			return ((size_t)-1);
		}


		es->want = want;
		es->ch = 0;
	} else {
		want = es->want;
		wc = es->ch;
	}

	for (i = 0; i < MIN(want, n); i++) {
		wc <<= 8;
		chs = *s;
		wc |= chs;
		s++;
	}
	if (i < want) {
		/* Incomplete multibyte sequence */
		es->want = want - i;
		es->ch = wc;
		errno = EILSEQ;
		return ((size_t)-2);
	}
	if (pwc != NULL)
		*pwc = wc;
	es->want = 0;
	return (wc == L'\0' ? 0 : want);
}

size_t
_EUC_wcrtomb_impl(char * __restrict s, wchar_t wc,
    mbstate_t * __restrict ps,
    uint8_t cs2, uint8_t cs2width, uint8_t cs3, uint8_t cs3width)
{
	_EucState *es;
	int i, len;
	wchar_t nm;

	es = (_EucState *)ps;

	if (es->want != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);

	if ((wc & ~0x7f) == 0) {
		/* Fast path for plain ASCII (CS0) */
		*s = (char)wc;
		return (1);
	}

	/* Determine the "length" */
	if ((unsigned)wc > 0xffffff) {
		len = 4;
	} else if ((unsigned)wc > 0xffff) {
		len = 3;
	} else if ((unsigned)wc > 0xff) {
		len = 2;
	} else {
		len = 1;
	}

	if (len > MB_CUR_MAX) {
		errno = EILSEQ;
		return ((size_t)-1);
	}

	/* This first check excludes CS1, which is implicitly valid. */
	if ((wc < 0xa100) || (wc > 0xffff)) {
		/* Check for valid CS2 or CS3 */
		nm = (wc >> ((len - 1) * 8)) & 0xff;
		if (nm == cs2) {
			if (len != cs2width) {
				errno = EILSEQ;
				return ((size_t)-1);
			}
		} else if (nm == cs3) {
			if (len != cs3width) {
				errno = EILSEQ;
				return ((size_t)-1);
			}
		} else {
			errno = EILSEQ;
			return ((size_t)-1);
		}
	}

	/* Stash the bytes, least significant last */
	for (i = len - 1; i >= 0; i--) {
		s[i] = (wc & 0xff);
		wc >>= 8;
	}
	return (len);
}

/* =================================================================== */
/* Support code: lib/libc/locale/mbsnrtowcs.c, lib/libc/locale/        */
/* wcsnrtombs.c                                                        */
/*                                                                     */
/* Not part of batch b0156.  euc.c's *_mbsnrtowcs()/*_wcsnrtombs()     */
/* wrappers are one-line calls into these two helpers, so they are     */
/* required for the batch to link.  oracle.c carries an identical      */
/* copy, so the differential test remains meaningful.                  */
/* =================================================================== */

size_t
__mbsnrtowcs_std(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps, mbrtowc_pfn_t pmbrtowc)
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
				/* Invalid sequence - mbrtowc() sets errno. */
				return ((size_t)-1);
			else if (nb == 0 || nb == (size_t)-2)
				return (nchr);
			s += nb;
			nms -= nb;
			nchr++;
		}
		/*NOTREACHED*/
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

size_t
__wcsnrtombs_std(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps, wcrtomb_pfn_t pwcrtomb)
{
	mbstate_t mbsbak;
	char buf[MB_LEN_MAX];
	const wchar_t *s;
	size_t nbytes;
	size_t nb;

	s = *src;
	nbytes = 0;

	if (dst == NULL) {
		while (nwc-- > 0) {
			if ((nb = pwcrtomb(buf, *s, ps)) == (size_t)-1)
				/* Invalid character - wcrtomb() sets errno. */
				return ((size_t)-1);
			else if (*s == L'\0')
				return (nbytes + nb - 1);
			s++;
			nbytes += nb;
		}
		return (nbytes);
	}

	while (len > 0 && nwc-- > 0) {
		if (len > (size_t)MB_CUR_MAX) {
			/* Enough space to translate in-place. */
			if ((nb = pwcrtomb(dst, *s, ps)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
		} else {
			/*
			 * May not be enough space; use temp. buffer.
			 */
			mbsbak = *ps;
			if ((nb = pwcrtomb(buf, *s, ps)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
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

}  /* namespace pbsd::lib_libc_locale::b0156 */
