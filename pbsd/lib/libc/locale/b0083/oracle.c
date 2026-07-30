/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2001 Alexey Zelkin <phantom@FreeBSD.org>
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
/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
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

/*
 * Oracle for batch b0083: the original C sources concatenated, every function
 * renamed with a ref_ prefix.  Function bodies are UNMODIFIED.
 *
 * Sources: lib/libc/locale/fix_grouping.c
 *          lib/libc/locale/tolower.c
 *          lib/libc/locale/toupper.c
 *
 * lib/libc/locale/c16rtomb.c is not part of this oracle; see skipped.txt.
 *
 * The declarations below stand in for the private libc headers that the
 * original translation units include (runetype.h, mblocal.h,
 * xlocale_private.h).  Only the members touched by the ported functions are
 * modelled; the harness builds these objects and hands the same memory to
 * both the oracle and the port.
 */

#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>

typedef int __ct_rune_t;
typedef __ct_rune_t __rune_t;

typedef struct {
	__rune_t	__min;		/* minimum rune of the range */
	__rune_t	__max;		/* maximum rune of the range */
	__rune_t	__map;		/* either map to const or map to array index */
	unsigned long	*__types;	/* array of types in range */
} _RuneEntry;

typedef struct {
	int		__nranges;	/* number of ranges stored */
	_RuneEntry	*__ranges;	/* array of ranges */
} _RuneRange;

typedef struct {
	_RuneRange	__maplower_ext;
	_RuneRange	__mapupper_ext;
} _RuneLocale;

struct ref_xlocale_ctype {
	_RuneLocale	*runes;
};

struct ref_xlocale {
	struct ref_xlocale_ctype *__ctype;
};

typedef struct ref_xlocale *locale_t;

/* Stands in for the thread/global locale; the harness sets it. */
locale_t ref_current_locale = NULL;

#define	__get_locale()		(ref_current_locale)
#define	FIX_LOCALE(l)		do { if ((l) == NULL) (l) = __get_locale(); } while (0)
#define	XLOCALE_CTYPE(l)	((l)->__ctype)

/* ------------------------------------------------------------------ */
/* lib/libc/locale/fix_grouping.c					*/
/* ------------------------------------------------------------------ */

static const char nogrouping[] = { '\0' };

/*
 * Internal helper used to convert grouping sequences from string
 * representation into POSIX specified form, i.e.
 *
 * "3;3;-1" -> "\003\003\177\000"
 */

const char *
ref___fix_locale_grouping_str(const char *str)
{
	char *src, *dst;
	char n;

	if (str == NULL || *str == '\0') {
		return nogrouping;
	}

	for (src = (char*)str, dst = (char*)str; *src != '\0'; src++) {

		/* input string examples: "3;3", "3;2;-1" */
		if (*src == ';')
			continue;
	
		if (*src == '-' && *(src+1) == '1') {
			*dst++ = CHAR_MAX;
			src++;
			continue;
		}

		if (!isdigit((unsigned char)*src)) {
			/* broken grouping string */
			return nogrouping;
		}

		/* assume all numbers <= 99 */
		n = *src - '0';
		if (isdigit((unsigned char)*(src+1))) {
			src++;
			n *= 10;
			n += *src - '0';
		}

		*dst = n;
		/* NOTE: assume all input started with "0" as 'no grouping' */
		if (*dst == '\0')
			return (dst == (char*)str) ? nogrouping : str;
		dst++;
	}
	*dst = '\0';
	return str;
}

/* ------------------------------------------------------------------ */
/* lib/libc/locale/tolower.c						*/
/* ------------------------------------------------------------------ */

__ct_rune_t
ref____tolower_l(__ct_rune_t c, locale_t l)
{
	size_t lim;
	FIX_LOCALE(l);
	_RuneRange *rr = &XLOCALE_CTYPE(l)->runes->__maplower_ext;
	_RuneEntry *base, *re;

	if (c < 0 || c == EOF)
		return(c);

	/* Binary search -- see bsearch.c for explanation. */
	base = rr->__ranges;
	for (lim = rr->__nranges; lim != 0; lim >>= 1) {
		re = base + (lim >> 1);
		if (re->__min <= c && c <= re->__max)
			return (re->__map + c - re->__min);
		else if (c > re->__max) {
			base = re + 1;
			lim--;
		}
	}

	return(c);
}
__ct_rune_t
ref____tolower(__ct_rune_t c)
{
	return ref____tolower_l(c, __get_locale());
}

/* ------------------------------------------------------------------ */
/* lib/libc/locale/toupper.c						*/
/* ------------------------------------------------------------------ */

__ct_rune_t
ref____toupper_l(__ct_rune_t c, locale_t l)
{
	size_t lim;
	FIX_LOCALE(l);
	_RuneRange *rr = &XLOCALE_CTYPE(l)->runes->__mapupper_ext;
	_RuneEntry *base, *re;

	if (c < 0 || c == EOF)
		return(c);

	/* Binary search -- see bsearch.c for explanation. */
	base = rr->__ranges;
	for (lim = rr->__nranges; lim != 0; lim >>= 1) {
		re = base + (lim >> 1);
		if (re->__min <= c && c <= re->__max)
		{
			return (re->__map + c - re->__min);
		}
		else if (c > re->__max) {
			base = re + 1;
			lim--;
		}
	}

	return(c);
}
__ct_rune_t
ref____toupper(__ct_rune_t c)
{
	return ref____toupper_l(c, __get_locale());
}
