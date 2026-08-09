/*
 * PBSD batch b0156s3 -- C++23 module port of
 *
 *	hbsd/src/lib/libc/locale/euc.c
 *
 * Faithful transliteration.  Behaviour, integer signedness, evaluation order,
 * pointer arithmetic and the original bugs are preserved exactly -- including
 * euc.c setting errno = EILSEQ on the incomplete-sequence path that returns
 * (size_t)-2, and the sign-extending `chs = *s' / `s[i] = (wc & 0xff)'
 * conversions.  Nothing is improved.
 *
 * The functions of euc.c that could not be ported without source files outside
 * this batch are listed, with reasons, in skipped.txt.
 */

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

module;

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

export module pbsd.lib.libc.locale.b0156s3;

/*
 * sys/param.h
 */
#undef	MIN
#define	MIN(a,b)	(((a)<(b))?(a):(b))

/*
 * stdlib.h's MB_CUR_MAX is ((size_t)__mb_cur_max()), i.e. the __mb_cur_max of
 * the current ctype locale, which the *_init() functions of this file set to 2
 * (KR), 3 (JP) or 4 (CN, TW).  Outside libc there is no such locale to
 * install, so it is pinned -- as a size_t, exactly as stdlib.h spells it -- to
 * 4, the largest value euc.c ever installs.  oracle.c pins it identically.
 */
#undef	MB_CUR_MAX
#define	MB_CUR_MAX	((size_t)4)

/*
 * The EUC single-shift introducers (ISO 2022 SS2/SS3 in their 8-bit form),
 * used by euc.c through the headers it includes.
 */
#define	SS2		0x8e
#define	SS3		0x8f

export namespace pbsd::lib_libc_locale::b0156s3 {

/*
 * FreeBSD's mbstate_t (sys/_types.h, __mbstate_t) is a 128-byte opaque union
 * that euc.c casts to _EucState.  glibc's mbstate_t is only 8 bytes -- too
 * small to hold _EucState -- so the FreeBSD shape is reproduced here and used
 * in place of mbstate_t in the ported signatures.  oracle.c uses an identical
 * definition.
 */
union pbsd_mbstate_t {
	char		__mbstate8[128];
	long long	_mbstateL;
};

}  /* namespace pbsd::lib_libc_locale::b0156s3 */

#define	mbstate_t	::pbsd::lib_libc_locale::b0156s3::pbsd_mbstate_t

export namespace pbsd::lib_libc_locale::b0156s3 {

/* =================================================================== */
/* lib/libc/locale/euc.c                                               */
/* =================================================================== */

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
size_t
_EUC_CN_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
    size_t n, mbstate_t * __restrict ps)
{
	return (_EUC_mbrtowc_impl(pwc, s, n, ps, SS2, 4, 0, 0));
}

size_t
_EUC_CN_wcrtomb(char * __restrict s, wchar_t wc,
    mbstate_t * __restrict ps)
{
	return (_EUC_wcrtomb_impl(s, wc, ps, SS2, 4, 0, 0));
}

/*
 * EUC-KR uses only CS0 and CS1.
 */
size_t
_EUC_KR_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
    size_t n, mbstate_t * __restrict ps)
{
	return (_EUC_mbrtowc_impl(pwc, s, n, ps, 0, 0, 0, 0));
}

size_t
_EUC_KR_wcrtomb(char * __restrict s, wchar_t wc,
	mbstate_t * __restrict ps)
{
	return (_EUC_wcrtomb_impl(s, wc, ps, 0, 0, 0, 0));
}

/*
 * EUC-JP uses CS0, CS1, CS2, and CS3.
 */
size_t
_EUC_JP_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
    size_t n, mbstate_t * __restrict ps)
{
	return (_EUC_mbrtowc_impl(pwc, s, n, ps, SS2, 2, SS3, 3));
}

size_t
_EUC_JP_wcrtomb(char * __restrict s, wchar_t wc,
    mbstate_t * __restrict ps)
{
	return (_EUC_wcrtomb_impl(s, wc, ps, SS2, 2, SS3, 3));
}

/*
 * EUC-TW uses CS0, CS1, and CS2.
 */
size_t
_EUC_TW_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
	size_t n, mbstate_t * __restrict ps)
{
	return (_EUC_mbrtowc_impl(pwc, s, n, ps, SS2, 4, 0, 0));
}

size_t
_EUC_TW_wcrtomb(char * __restrict s, wchar_t wc,
	mbstate_t * __restrict ps)
{
	return (_EUC_wcrtomb_impl(s, wc, ps, SS2, 4, 0, 0));
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

}  /* namespace pbsd::lib_libc_locale::b0156s3 */
