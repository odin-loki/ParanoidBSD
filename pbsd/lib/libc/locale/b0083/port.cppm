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

module;

#include <cctype>
#include <climits>
#include <cstddef>
#include <cstdio>

export module pbsd.lib.libc.locale.b0083;

namespace pbsd::lib_libc_locale::b0083 {

static const char nogrouping[] = { '\0' };

}

export namespace pbsd::lib_libc_locale::b0083 {

/*
 * Stand-ins for the private libc headers the originals include
 * (runetype.h, xlocale_private.h).  Only the members reached by the ported
 * functions are modelled.
 */
using __ct_rune_t = int;
using __rune_t = __ct_rune_t;

struct _RuneEntry {
	__rune_t	__min;		/* minimum rune of the range */
	__rune_t	__max;		/* maximum rune of the range */
	__rune_t	__map;		/* either map to const or map to array index */
	unsigned long	*__types;	/* array of types in range */
};

struct _RuneRange {
	int		__nranges;	/* number of ranges stored */
	_RuneEntry	*__ranges;	/* array of ranges */
};

struct _RuneLocale {
	_RuneRange	__maplower_ext;
	_RuneRange	__mapupper_ext;
};

struct xlocale_ctype {
	_RuneLocale	*runes;
};

struct xlocale {
	xlocale_ctype	*__ctype;
};

using locale_t = xlocale *;

/* Stands in for the thread/global locale. */
locale_t current_locale = nullptr;

locale_t
__get_locale()
{
	return current_locale;
}

}

namespace pbsd::lib_libc_locale::b0083 {

static xlocale_ctype *
XLOCALE_CTYPE(locale_t l)
{
	return l->__ctype;
}

}

export namespace pbsd::lib_libc_locale::b0083 {

/*
 * Internal helper used to convert grouping sequences from string
 * representation into POSIX specified form, i.e.
 *
 * "3;3;-1" -> "\003\003\177\000"
 */

const char *
__fix_locale_grouping_str(const char *str)
{
	char *src, *dst;
	char n;

	if (str == nullptr || *str == '\0') {
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

		if (!std::isdigit((unsigned char)*src)) {
			/* broken grouping string */
			return nogrouping;
		}

		/* assume all numbers <= 99 */
		n = *src - '0';
		if (std::isdigit((unsigned char)*(src+1))) {
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

__ct_rune_t
___tolower_l(__ct_rune_t c, locale_t l)
{
	std::size_t lim;
	if (l == nullptr)
		l = __get_locale();
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
___tolower(__ct_rune_t c)
{
	return ___tolower_l(c, __get_locale());
}

__ct_rune_t
___toupper_l(__ct_rune_t c, locale_t l)
{
	std::size_t lim;
	if (l == nullptr)
		l = __get_locale();
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
___toupper(__ct_rune_t c)
{
	return ___toupper_l(c, __get_locale());
}

}
