/*
 * PBSD batch b0156s3 -- reference oracle.
 *
 * This file is the original HardenedBSD C source
 *
 *	lib/libc/locale/euc.c
 *
 * with every function renamed with a `ref_' prefix (and `static' dropped so
 * the harness can reach them).  Function BODIES are byte-for-byte the
 * originals; nothing else has been touched.  Defines/types that normally come
 * from FreeBSD private headers (mblocal.h, runetype.h, sys/param.h,
 * xlocale_private.h) are supplied in the support prologue below.
 */

/* ------------------------------------------------------------------- */
/* Support prologue -- definitions normally provided by FreeBSD headers */
/* ------------------------------------------------------------------- */

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

/*
 * FreeBSD's mbstate_t is a 128-byte opaque union (sys/_types.h,
 * __mbstate_t).  glibc's is only 8 bytes, which is too small to hold
 * _EucState, so the FreeBSD shape is reproduced here.
 */
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} __pbsd_mbstate_t;
#define	mbstate_t	__pbsd_mbstate_t

/* sys/param.h */
#ifndef MIN
#define	MIN(a,b)	(((a)<(b))?(a):(b))
#endif

/*
 * stdlib.h's MB_CUR_MAX is __mb_cur_max() on FreeBSD, i.e. the current ctype
 * locale's value.  euc.c reads it.  It is pinned to 4 (the value a EUC-CN /
 * EUC-TW locale installs) so the oracle is deterministic; the port pins it
 * identically.
 */
#undef	MB_CUR_MAX
#define	MB_CUR_MAX	((size_t)4)

/* EUC single shift bytes (ISO 2022 SS2/SS3). */
#define	SS2		0x8e
#define	SS3		0x8f

/* runetype.h -- only ever used as an opaque pointer here. */
typedef struct _RuneLocale _RuneLocale;

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
 * at the bottom of this file as support code (port.cppm carries an identical
 * copy).  They are not part of the batch proper.
 */
typedef size_t (*mbrtowc_pfn_t)(wchar_t * __restrict, const char * __restrict,
    size_t, mbstate_t * __restrict);
typedef size_t (*wcrtomb_pfn_t)(char * __restrict, wchar_t,
    mbstate_t * __restrict);

size_t	ref___mbsnrtowcs_std(wchar_t * __restrict, const char ** __restrict,
	    size_t, size_t, mbstate_t * __restrict, mbrtowc_pfn_t);
size_t	ref___wcsnrtombs_std(char * __restrict, const wchar_t ** __restrict,
	    size_t, size_t, mbstate_t * __restrict, wcrtomb_pfn_t);
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

size_t	ref__EUC_mbrtowc_impl(wchar_t * __restrict, const char * __restrict,
    size_t, mbstate_t * __restrict, uint8_t, uint8_t, uint8_t, uint8_t);
size_t	ref__EUC_wcrtomb_impl(char * __restrict, wchar_t,
    mbstate_t * __restrict, uint8_t, uint8_t, uint8_t, uint8_t);

size_t	ref__EUC_CN_mbrtowc(wchar_t * __restrict, const char * __restrict,
	    size_t, mbstate_t * __restrict);
size_t	ref__EUC_JP_mbrtowc(wchar_t * __restrict, const char * __restrict,
	    size_t, mbstate_t * __restrict);
size_t	ref__EUC_KR_mbrtowc(wchar_t * __restrict, const char * __restrict,
	    size_t, mbstate_t * __restrict);
size_t	ref__EUC_TW_mbrtowc(wchar_t * __restrict, const char * __restrict,
	    size_t, mbstate_t * __restrict);

size_t	ref__EUC_CN_wcrtomb(char * __restrict, wchar_t,
	    mbstate_t * __restrict);
size_t	ref__EUC_JP_wcrtomb(char * __restrict, wchar_t,
	    mbstate_t * __restrict);
size_t	ref__EUC_KR_wcrtomb(char * __restrict, wchar_t,
	    mbstate_t * __restrict);
size_t	ref__EUC_TW_wcrtomb(char * __restrict, wchar_t,
	    mbstate_t * __restrict);

size_t	ref__EUC_CN_mbsnrtowcs(wchar_t * __restrict,
	    const char ** __restrict, size_t, size_t,
	    mbstate_t * __restrict);
size_t	ref__EUC_JP_mbsnrtowcs(wchar_t * __restrict,
	    const char ** __restrict, size_t, size_t,
	    mbstate_t * __restrict);
size_t	ref__EUC_KR_mbsnrtowcs(wchar_t * __restrict,
	    const char ** __restrict, size_t, size_t,
	    mbstate_t * __restrict);
size_t	ref__EUC_TW_mbsnrtowcs(wchar_t * __restrict,
	    const char ** __restrict, size_t, size_t,
	    mbstate_t * __restrict);

size_t	ref__EUC_CN_wcsnrtombs(char * __restrict,
	    const wchar_t ** __restrict, size_t, size_t,
	    mbstate_t * __restrict);
size_t	ref__EUC_JP_wcsnrtombs(char * __restrict,
	    const wchar_t ** __restrict, size_t, size_t,
	    mbstate_t * __restrict);
size_t	ref__EUC_KR_wcsnrtombs(char * __restrict,
	    const wchar_t ** __restrict, size_t, size_t,
	    mbstate_t * __restrict);
size_t	ref__EUC_TW_wcsnrtombs(char * __restrict,
	    const wchar_t ** __restrict, size_t, size_t,
	    mbstate_t * __restrict);

int	ref__EUC_mbsinit(const mbstate_t *);

typedef struct {
	wchar_t	ch;
	int	set;
	int	want;
} _EucState;

int
ref__EUC_mbsinit(const mbstate_t *ps)
{

	return (ps == NULL || ((const _EucState *)ps)->want == 0);
}

/*
 * EUC-CN uses CS0, CS1 and CS2 (4 bytes).
 */
int
ref__EUC_CN_init(struct xlocale_ctype *l, _RuneLocale *rl)
{
	l->__mbrtowc = ref__EUC_CN_mbrtowc;
	l->__wcrtomb = ref__EUC_CN_wcrtomb;
	l->__mbsnrtowcs = ref__EUC_CN_mbsnrtowcs;
	l->__wcsnrtombs = ref__EUC_CN_wcsnrtombs;
	l->__mbsinit = ref__EUC_mbsinit;

	l->runes = rl;
	l->__mb_cur_max = 4;
	l->__mb_sb_limit = 128;
	return (0);
}

size_t
ref__EUC_CN_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
    size_t n, mbstate_t * __restrict ps)
{
	return (ref__EUC_mbrtowc_impl(pwc, s, n, ps, SS2, 4, 0, 0));
}

size_t
ref__EUC_CN_mbsnrtowcs(wchar_t * __restrict dst,
    const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
{
	return (ref___mbsnrtowcs_std(dst, src, nms, len, ps,
	    ref__EUC_CN_mbrtowc));
}

size_t
ref__EUC_CN_wcrtomb(char * __restrict s, wchar_t wc,
    mbstate_t * __restrict ps)
{
	return (ref__EUC_wcrtomb_impl(s, wc, ps, SS2, 4, 0, 0));
}

size_t
ref__EUC_CN_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
	size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	return (ref___wcsnrtombs_std(dst, src, nwc, len, ps,
	    ref__EUC_CN_wcrtomb));
}

/*
 * EUC-KR uses only CS0 and CS1.
 */
int
ref__EUC_KR_init(struct xlocale_ctype *l, _RuneLocale *rl)
{
	l->__mbrtowc = ref__EUC_KR_mbrtowc;
	l->__wcrtomb = ref__EUC_KR_wcrtomb;
	l->__mbsnrtowcs = ref__EUC_KR_mbsnrtowcs;
	l->__wcsnrtombs = ref__EUC_KR_wcsnrtombs;
	l->__mbsinit = ref__EUC_mbsinit;

	l->runes = rl;
	l->__mb_cur_max = 2;
	l->__mb_sb_limit = 128;
	return (0);
}

size_t
ref__EUC_KR_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
    size_t n, mbstate_t * __restrict ps)
{
	return (ref__EUC_mbrtowc_impl(pwc, s, n, ps, 0, 0, 0, 0));
}

size_t
ref__EUC_KR_mbsnrtowcs(wchar_t * __restrict dst,
    const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
{
	return (ref___mbsnrtowcs_std(dst, src, nms, len, ps,
	    ref__EUC_KR_mbrtowc));
}

size_t
ref__EUC_KR_wcrtomb(char * __restrict s, wchar_t wc,
	mbstate_t * __restrict ps)
{
	return (ref__EUC_wcrtomb_impl(s, wc, ps, 0, 0, 0, 0));
}

size_t
ref__EUC_KR_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
	size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	return (ref___wcsnrtombs_std(dst, src, nwc, len, ps,
	    ref__EUC_KR_wcrtomb));
}

/*
 * EUC-JP uses CS0, CS1, CS2, and CS3.
 */
int
ref__EUC_JP_init(struct xlocale_ctype *l, _RuneLocale *rl)
{
	l->__mbrtowc = ref__EUC_JP_mbrtowc;
	l->__wcrtomb = ref__EUC_JP_wcrtomb;
	l->__mbsnrtowcs = ref__EUC_JP_mbsnrtowcs;
	l->__wcsnrtombs = ref__EUC_JP_wcsnrtombs;
	l->__mbsinit = ref__EUC_mbsinit;

	l->runes = rl;
	l->__mb_cur_max = 3;
	l->__mb_sb_limit = 128;
	return (0);
}

size_t
ref__EUC_JP_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
    size_t n, mbstate_t * __restrict ps)
{
	return (ref__EUC_mbrtowc_impl(pwc, s, n, ps, SS2, 2, SS3, 3));
}

size_t
ref__EUC_JP_mbsnrtowcs(wchar_t * __restrict dst,
    const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps)
{
	return (ref___mbsnrtowcs_std(dst, src, nms, len, ps,
	    ref__EUC_JP_mbrtowc));
}

size_t
ref__EUC_JP_wcrtomb(char * __restrict s, wchar_t wc,
    mbstate_t * __restrict ps)
{
	return (ref__EUC_wcrtomb_impl(s, wc, ps, SS2, 2, SS3, 3));
}

size_t
ref__EUC_JP_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
	size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	return (ref___wcsnrtombs_std(dst, src, nwc, len, ps,
	    ref__EUC_JP_wcrtomb));
}

/*
 * EUC-TW uses CS0, CS1, and CS2.
 */
int
ref__EUC_TW_init(struct xlocale_ctype *l, _RuneLocale *rl)
{
	l->__mbrtowc = ref__EUC_TW_mbrtowc;
	l->__wcrtomb = ref__EUC_TW_wcrtomb;
	l->__mbsnrtowcs = ref__EUC_TW_mbsnrtowcs;
	l->__wcsnrtombs = ref__EUC_TW_wcsnrtombs;
	l->__mbsinit = ref__EUC_mbsinit;

	l->runes = rl;
	l->__mb_cur_max = 4;
	l->__mb_sb_limit = 128;
	return (0);
}

size_t
ref__EUC_TW_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
	size_t n, mbstate_t * __restrict ps)
{
	return (ref__EUC_mbrtowc_impl(pwc, s, n, ps, SS2, 4, 0, 0));
}

size_t
ref__EUC_TW_mbsnrtowcs(wchar_t * __restrict dst,
	const char ** __restrict src,
	size_t nms, size_t len, mbstate_t * __restrict ps)
{
	return (ref___mbsnrtowcs_std(dst, src, nms, len, ps,
	    ref__EUC_TW_mbrtowc));
}

size_t
ref__EUC_TW_wcrtomb(char * __restrict s, wchar_t wc,
	mbstate_t * __restrict ps)
{
	return (ref__EUC_wcrtomb_impl(s, wc, ps, SS2, 4, 0, 0));
}

size_t
ref__EUC_TW_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
	size_t nwc, size_t len, mbstate_t * __restrict ps)
{
	return (ref___wcsnrtombs_std(dst, src, nwc, len, ps,
	    ref__EUC_TW_wcrtomb));
}

/*
 * Common EUC code.
 */

size_t
ref__EUC_mbrtowc_impl(wchar_t * __restrict pwc, const char * __restrict s,
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
ref__EUC_wcrtomb_impl(char * __restrict s, wchar_t wc,
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
/* required for the batch to link.  port.cppm carries an identical     */
/* copy, so the differential test remains meaningful.                  */
/* =================================================================== */

size_t
ref___mbsnrtowcs_std(wchar_t * __restrict dst, const char ** __restrict src,
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
ref___wcsnrtombs_std(char * __restrict dst, const wchar_t ** __restrict src,
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
