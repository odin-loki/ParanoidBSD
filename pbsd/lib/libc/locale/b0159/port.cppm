/*-
 * PBSD batch b0159 -- C++23 module port of
 *
 *	hbsd/src/lib/libc/locale/none.c
 *	hbsd/src/lib/libc/locale/nl_langinfo.c
 *	hbsd/src/lib/libc/locale/gb18030.c
 *	hbsd/src/lib/libc/locale/setrunelocale.c
 */

module;

#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>

export module pbsd.lib.libc.locale.b0159;

extern "C" {
struct xlocale;
typedef struct xlocale *locale_t;

#define __mbstate_t_defined 1
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} pbsd_mbstate_t;
typedef pbsd_mbstate_t mbstate_t;

#include <cstdarg>
#include <errno.h>
#define _BITS_TYPES_LOCALE_T_H	1
#include <string.h>
#include <wchar.h>

#ifndef __unused
#define __unused	__attribute__((__unused__))
#endif

typedef int nl_item;



#ifndef CODESET
#define CODESET		0
#define D_T_FMT		1
#define D_FMT		2
#define T_FMT		3
#define T_FMT_AMPM	4
#define AM_STR		5
#define PM_STR		6
#define DAY_1		7
#define DAY_2		8
#define DAY_3		9
#define DAY_4		10
#define DAY_5		11
#define DAY_6		12
#define DAY_7		13
#define ABDAY_1		14
#define ABDAY_2		15
#define ABDAY_3		16
#define ABDAY_4		17
#define ABDAY_5		18
#define ABDAY_6		19
#define ABDAY_7		20
#define MON_1		21
#define MON_2		22
#define MON_3		23
#define MON_4		24
#define MON_5		25
#define MON_6		26
#define MON_7		27
#define MON_8		28
#define MON_9		29
#define MON_10		30
#define MON_11		31
#define MON_12		32
#define ABMON_1		33
#define ABMON_2		34
#define ABMON_3		35
#define ABMON_4		36
#define ABMON_5		37
#define ABMON_6		38
#define ABMON_7		39
#define ABMON_8		40
#define ABMON_9		41
#define ABMON_10	42
#define ABMON_11	43
#define ABMON_12	44
#define ALTMON_1	45
#define ALTMON_2	46
#define ALTMON_3	47
#define ALTMON_4	48
#define ALTMON_5	49
#define ALTMON_6	50
#define ALTMON_7	51
#define ALTMON_8	52
#define ALTMON_9	53
#define ALTMON_10	54
#define ALTMON_11	55
#define ALTMON_12	56
#define ERA		57
#define ERA_D_FMT	58
#define ERA_D_T_FMT	59
#define ERA_T_FMT	60
#define ALT_DIGITS	61
#define RADIXCHAR	62
#define THOUSEP		63
#define YESSTR		64
#define YESEXPR		65
#define NOEXPR		66
#define NOSTR		67
#define CRNCYSTR	68
#define D_MD_ORDER	69
#endif

#ifndef __unused
#define __unused	__attribute__((__unused__))
#endif

#ifndef u_char
typedef unsigned char u_char;
#endif

#ifndef MB_LEN_MAX
#define MB_LEN_MAX	4
#endif

#ifndef MB_CUR_MAX
#define MB_CUR_MAX	4
#endif

#ifndef MIN
#define MIN(a, b)	((a) < (b) ? (a) : (b))
#endif

#ifndef EFTYPE
#define EFTYPE	79
#endif

#ifndef _LDP_LOADED
#define _LDP_LOADED	1
#endif

#ifndef _LDP_ERROR
#define _LDP_ERROR	(-1)
#endif

#ifndef LC_GLOBAL_LOCALE
#define LC_GLOBAL_LOCALE	((locale_t)-1)
#endif

typedef int __ct_rune_t;
typedef __ct_rune_t __rune_t;

typedef struct {
	__rune_t	__min;
	__rune_t	__max;
	__rune_t	__map;
	unsigned long	*__types;
} _RuneEntry;

typedef struct {
	int		__nranges;
	_RuneEntry	*__ranges;
} _RuneRange;

typedef struct {
	char		__magic[8];
	char		__encoding[32];
	__rune_t	(*__sgetrune)(const char *, size_t, char const **);
	int		(*__sputrune)(__rune_t, char *, size_t, char **);
	__rune_t	__invalid_rune;
	unsigned long	__runetype[256];
	__rune_t	__maplower[256];
	__rune_t	__mapupper[256];
	_RuneRange	__runetype_ext;
	_RuneRange	__maplower_ext;
	_RuneRange	__mapupper_ext;
	void		*__variable;
	int		__variable_len;
} _RuneLocale;

typedef size_t (*mbrtowc_pfn_t)(wchar_t * __restrict, const char * __restrict,
    size_t, mbstate_t * __restrict);
typedef size_t (*wcrtomb_pfn_t)(char * __restrict, wchar_t,
    mbstate_t * __restrict);

struct xlocale_component_header {
	void		(*destructor)(void *);
};

struct xlocale_ctype {
	struct {
		struct xlocale_component_header header;
		char		locale_name[32];
	} header;
	_RuneLocale	*runes;
	size_t		(*__mbrtowc)(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
	int		(*__mbsinit)(const mbstate_t *);
	size_t		(*__mbsnrtowcs)(wchar_t * __restrict, const char ** __restrict,
		    size_t, size_t, mbstate_t * __restrict);
	size_t		(*__wcrtomb)(char * __restrict, wchar_t, mbstate_t * __restrict);
	size_t		(*__wcsnrtombs)(char * __restrict, const wchar_t ** __restrict,
		    size_t, size_t, mbstate_t * __restrict);
	int		__mb_cur_max;
	int		__mb_sb_limit;
	mbstate_t	c16rtomb;
	mbstate_t	c32rtomb;
	mbstate_t	mblen;
	mbstate_t	mbrlen;
	mbstate_t	mbrtoc16;
	mbstate_t	mbrtoc32;
	mbstate_t	mbrtowc;
	mbstate_t	mbsnrtowcs;
	mbstate_t	mbsrtowcs;
	mbstate_t	mbtowc;
	mbstate_t	wcrtomb;
	mbstate_t	wcsnrtombs;
	mbstate_t	wcsrtombs;
	mbstate_t	wctomb;
};

enum {
	XLC_CTYPE = 1,
	XLC_NUMERIC = 2,
	XLC_MONETARY = 3,
	XLC_MESSAGES = 5,
	XLC_TIME = 6,
};

struct lc_time_T {
	const char	*c_fmt;
	const char	*x_fmt;
	const char	*X_fmt;
	const char	*ampm_fmt;
	const char	*am;
	const char	*pm;
	const char	*weekday[7];
	const char	*wday[7];
	const char	*month[12];
	const char	*mon[12];
	const char	*alt_month[12];
	const char	*md_order;
};

struct lc_numeric_T {
	const char	*decimal_point;
	const char	*thousands_sep;
};

struct lc_messages_T {
	const char	*yesexpr;
	const char	*noexpr;
	const char	*yesstr;
	const char	*nostr;
};

struct lc_monetary_T {
	const char	*currency_symbol;
	const char	*mon_decimal_point;
};

struct pbsd_lconv {
	char	p_cs_precedes;
	char	n_cs_precedes;
};

struct xlocale {
	void		*components[8];
	char		*csym;
	struct pbsd_lconv conv;
};

typedef struct {
	const char	*encoding;
	int		read_fail;
	int		asprintf_fail;
} pbsd_rune_hook_t;

typedef struct {
	struct lc_time_T	time;
	struct lc_numeric_T	numeric;
	struct lc_messages_T	messages;
	struct lc_monetary_T	monetary;
	struct pbsd_lconv	conv;
	char			encoding[32];
} pbsd_nl_hook_t;

#define FIX_LOCALE(loc)	if ((loc) == NULL) (loc) = __get_locale()
#define XLOCALE_CTYPE(l)	((struct xlocale_ctype *)(l)->components[XLC_CTYPE])

extern _RuneLocale		_DefaultRuneLocale;
extern _RuneLocale const	*_CurrentRuneLocale;
extern const char		*_PathLocale;
extern int			__mb_cur_max;
extern int			__mb_sb_limit;
extern struct xlocale		__xlocale_global_locale;
extern struct xlocale_ctype	__xlocale_global_ctype;

void		*calloc(size_t, size_t);
void		free(void *);
void		*reallocf(void *, size_t);
int		asprintf(char **, const char *, ...);

locale_t	__get_locale(void);
struct lc_time_T *__get_current_time_locale(locale_t);
struct lc_numeric_T *__get_current_numeric_locale(locale_t);
struct lc_messages_T *__get_current_messages_locale(locale_t);
struct lc_monetary_T *__get_current_monetary_locale(locale_t);
struct pbsd_lconv *localeconv_l(locale_t);
_RuneLocale	*_Read_RuneMagi(const char *);

int	_ascii_init(struct xlocale_ctype *, _RuneLocale *);
int	_UTF8_init(struct xlocale_ctype *, _RuneLocale *);
int	_EUC_CN_init(struct xlocale_ctype *, _RuneLocale *);
int	_EUC_JP_init(struct xlocale_ctype *, _RuneLocale *);
int	_EUC_KR_init(struct xlocale_ctype *, _RuneLocale *);
int	_EUC_TW_init(struct xlocale_ctype *, _RuneLocale *);
int	_GB2312_init(struct xlocale_ctype *, _RuneLocale *);
int	_GBK_init(struct xlocale_ctype *, _RuneLocale *);
int	_BIG5_init(struct xlocale_ctype *, _RuneLocale *);
int	_MSKanji_init(struct xlocale_ctype *, _RuneLocale *);

} // extern "C"

export namespace pbsd::lib_libc_locale::b0159 {

using mbstate_t = ::mbstate_t;
using locale_t = ::locale_t;
using nl_item = ::nl_item;
using _RuneLocale = ::_RuneLocale;


size_t
__mbsnrtowcs_std(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps,
    mbrtowc_pfn_t pmbrtowc)
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

size_t
__wcsnrtombs_std(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps,
    wcrtomb_pfn_t pwcrtomb)
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
			if ((nb = pwcrtomb(dst, *s, ps)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
		} else {
			mbsbak = *ps;
			if ((nb = pwcrtomb(buf, *s, ps)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
			if (nb > len) {
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


/* none.c */
/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2013 Garrett D'Amore <garrett@damore.org>
 * Copyright 2010 Nexenta Systems, Inc.  All rights reserved.
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


size_t	_none_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
int	_none_mbsinit(const mbstate_t *);
size_t	_none_mbsnrtowcs(wchar_t * __restrict dst,
		    const char ** __restrict src, size_t nms, size_t len,
		    mbstate_t * __restrict ps __unused);
size_t	_none_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);
size_t	_none_wcsnrtombs(char * __restrict, const wchar_t ** __restrict,
		    size_t, size_t, mbstate_t * __restrict);

/* setup defaults */


int
_none_init(struct xlocale_ctype *l, _RuneLocale *rl)
{

	l->__mbrtowc = _none_mbrtowc;
	l->__mbsinit = _none_mbsinit;
	l->__mbsnrtowcs = _none_mbsnrtowcs;
	l->__wcrtomb = _none_wcrtomb;
	l->__wcsnrtombs = _none_wcsnrtombs;
	l->runes = rl;
	l->__mb_cur_max = 1;
	l->__mb_sb_limit = 256;
	return(0);
}

int
_none_mbsinit(const mbstate_t *ps __unused)
{

	/*
	 * Encoding is not state dependent - we are always in the
	 * initial state.
	 */
	return (1);
}

size_t
_none_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps __unused)
{

	if (s == nullptr)
		/* Reset to initial shift state (no-op) */
		return (0);
	if (n == 0)
		/* Incomplete multibyte sequence */
		return ((size_t)-2);
	if (pwc != nullptr)
		*pwc = (unsigned char)*s;
	return (*s == '\0' ? 0 : 1);
}

size_t
_none_wcrtomb(char * __restrict s, wchar_t wc,
    mbstate_t * __restrict ps __unused)
{

	if (s == nullptr)
		/* Reset to initial shift state (no-op) */
		return (1);
	if (wc < 0 || wc > UCHAR_MAX) {
		errno = EILSEQ;
		return ((size_t)-1);
	}
	*s = (unsigned char)wc;
	return (1);
}

size_t
_none_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps __unused)
{
	const char *s;
	size_t nchr;

	if (dst == nullptr) {
		s = (const char *)memchr(*src, '\0', nms);
		return (s != nullptr ? s - *src : nms);
	}

	s = *src;
	nchr = 0;
	while (len-- > 0 && nms-- > 0) {
		if ((*dst++ = (unsigned char)*s++) == L'\0') {
			*src = nullptr;
			return (nchr);
		}
		nchr++;
	}
	*src = s;
	return (nchr);
}

size_t
_none_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps __unused)
{
	const wchar_t *s;
	size_t nchr;

	if (dst == nullptr) {
		for (s = *src; nwc > 0 && *s != L'\0'; s++, nwc--) {
			if (*s < 0 || *s > UCHAR_MAX) {
				errno = EILSEQ;
				return ((size_t)-1);
			}
		}
		return (s - *src);
	}

	s = *src;
	nchr = 0;
	while (len-- > 0 && nwc-- > 0) {
		if (*s < 0 || *s > UCHAR_MAX) {
			*src = s;
			errno = EILSEQ;
			return ((size_t)-1);
		}
		if ((*dst++ = *s++) == '\0') {
			*src = nullptr;
			return (nchr);
		}
		nchr++;
	}
	*src = s;
	return (nchr);
}

/* setup defaults - globals in oracle.c */

/* nl_langinfo.c */
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2001, 2003 Alexey Zelkin <phantom@FreeBSD.org>
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



#define _REL(BASE) ((int)item-BASE)

char *
nl_langinfo_l(nl_item item, locale_t loc)
{
   char *ret, *cs;
   const char *s;
   FIX_LOCALE(loc);

   switch (item) {
	case CODESET:
		s = XLOCALE_CTYPE(loc)->runes->__encoding;
		if (strcmp(s, "EUC-CN") == 0)
			ret = "eucCN";
		else if (strcmp(s, "EUC-JP") == 0)
			ret = "eucJP";
		else if (strcmp(s, "EUC-KR") == 0)
			ret = "eucKR";
		else if (strcmp(s, "EUC-TW") == 0)
			ret = "eucTW";
		else if (strcmp(s, "BIG5") == 0)
			ret = "Big5";
		else if (strcmp(s, "MSKanji") == 0)
			ret = "SJIS";
		else if (strcmp(s, "NONE") == 0)
			ret = "US-ASCII";
		else if (strncmp(s, "NONE:", 5) == 0)
			ret = (char *)(s + 5);
		else
			ret = (char *)s;
		break;
	case D_T_FMT:
		ret = (char *) __get_current_time_locale(loc)->c_fmt;
		break;
	case D_FMT:
		ret = (char *) __get_current_time_locale(loc)->x_fmt;
		break;
	case T_FMT:
		ret = (char *) __get_current_time_locale(loc)->X_fmt;
		break;
	case T_FMT_AMPM:
		ret = (char *) __get_current_time_locale(loc)->ampm_fmt;
		break;
	case AM_STR:
		ret = (char *) __get_current_time_locale(loc)->am;
		break;
	case PM_STR:
		ret = (char *) __get_current_time_locale(loc)->pm;
		break;
	case DAY_1: case DAY_2: case DAY_3:
	case DAY_4: case DAY_5: case DAY_6: case DAY_7:
		ret = (char*) __get_current_time_locale(loc)->weekday[_REL(DAY_1)];
		break;
	case ABDAY_1: case ABDAY_2: case ABDAY_3:
	case ABDAY_4: case ABDAY_5: case ABDAY_6: case ABDAY_7:
		ret = (char*) __get_current_time_locale(loc)->wday[_REL(ABDAY_1)];
		break;
	case MON_1: case MON_2: case MON_3: case MON_4:
	case MON_5: case MON_6: case MON_7: case MON_8:
	case MON_9: case MON_10: case MON_11: case MON_12:
		ret = (char*) __get_current_time_locale(loc)->month[_REL(MON_1)];
		break;
	case ABMON_1: case ABMON_2: case ABMON_3: case ABMON_4:
	case ABMON_5: case ABMON_6: case ABMON_7: case ABMON_8:
	case ABMON_9: case ABMON_10: case ABMON_11: case ABMON_12:
		ret = (char*) __get_current_time_locale(loc)->mon[_REL(ABMON_1)];
		break;
	case ALTMON_1: case ALTMON_2: case ALTMON_3: case ALTMON_4:
	case ALTMON_5: case ALTMON_6: case ALTMON_7: case ALTMON_8:
	case ALTMON_9: case ALTMON_10: case ALTMON_11: case ALTMON_12:
		ret = (char*)
		    __get_current_time_locale(loc)->alt_month[_REL(ALTMON_1)];
		break;
	case ERA:
		/* XXX: need to be implemented  */
		ret = "";
		break;
	case ERA_D_FMT:
		/* XXX: need to be implemented  */
		ret = "";
		break;
	case ERA_D_T_FMT:
		/* XXX: need to be implemented  */
		ret = "";
		break;
	case ERA_T_FMT:
		/* XXX: need to be implemented  */
		ret = "";
		break;
	case ALT_DIGITS:
		/* XXX: need to be implemented  */
		ret = "";
		break;
	case RADIXCHAR:
		ret = (char*) __get_current_numeric_locale(loc)->decimal_point;
		break;
	case THOUSEP:
		ret = (char*) __get_current_numeric_locale(loc)->thousands_sep;
		break;
	case YESEXPR:
		ret = (char*) __get_current_messages_locale(loc)->yesexpr;
		break;
	case NOEXPR:
		ret = (char*) __get_current_messages_locale(loc)->noexpr;
		break;
	/*
	 * YESSTR and NOSTR items marked with LEGACY are available, but not
	 * recommended by SUSv2 to be used in portable applications since
	 * they're subject to remove in future specification editions.
	 */
	case YESSTR:            /* LEGACY  */
		ret = (char*) __get_current_messages_locale(loc)->yesstr;
		break;
	case NOSTR:             /* LEGACY  */
		ret = (char*) __get_current_messages_locale(loc)->nostr;
		break;
	/*
	 * SUSv2 special formatted currency string 
	 */
	case CRNCYSTR:
		ret = "";
		cs = (char*) __get_current_monetary_locale(loc)->currency_symbol;
		if (*cs != '\0') {
			char pos = localeconv_l(loc)->p_cs_precedes;

			if (pos == localeconv_l(loc)->n_cs_precedes) {
				char psn = '\0';

				if (pos == CHAR_MAX) {
					if (strcmp(cs, __get_current_monetary_locale(loc)->mon_decimal_point) == 0)
						psn = '.';
				} else
					psn = pos ? '-' : '+';
				if (psn != '\0') {
					int clen = strlen(cs);

					if ((loc->csym = (char *)reallocf(loc->csym, clen + 2)) != nullptr) {
						*loc->csym = psn;
						strcpy(loc->csym + 1, cs);
						ret = loc->csym;
					}
				}
			}
		}
		break;
	case D_MD_ORDER:        /* FreeBSD local extension */
		ret = (char *) __get_current_time_locale(loc)->md_order;
		break;
	default:
		ret = "";
   }
   return (ret);
}

char *
nl_langinfo(nl_item item)
{
	return nl_langinfo_l(item, __get_locale());
}

/* gb18030.c */
/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright 2013 Garrett D'Amore <garrett@damore.org>
 * Copyright 2010 Nexenta Systems, Inc.  All rights reserved.
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

/*
 * PRC National Standard GB 18030-2000 encoding of Chinese text.
 *
 * See gb18030(5) for details.
 */


size_t	_GB18030_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
int	_GB18030_mbsinit(const mbstate_t *);
size_t	_GB18030_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);
size_t	_GB18030_mbsnrtowcs(wchar_t * __restrict,
		    const char ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);
size_t	_GB18030_wcsnrtombs(char * __restrict,
		    const wchar_t ** __restrict, size_t, size_t,
		    mbstate_t * __restrict);


typedef unsigned char u_char;

typedef struct {
	int	count;
	u_char	bytes[4];
} _GB18030State;

int
_GB18030_init(struct xlocale_ctype *l, _RuneLocale *rl)
{

	l->__mbrtowc = _GB18030_mbrtowc;
	l->__wcrtomb = _GB18030_wcrtomb;
	l->__mbsinit = _GB18030_mbsinit;
	l->__mbsnrtowcs = _GB18030_mbsnrtowcs;
	l->__wcsnrtombs = _GB18030_wcsnrtombs;
	l->runes = rl;
	l->__mb_cur_max = 4;
	l->__mb_sb_limit = 128;

	return (0);
}

int
_GB18030_mbsinit(const mbstate_t *ps)
{

	return (ps == nullptr || ((const _GB18030State *)ps)->count == 0);
}

size_t
_GB18030_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s,
    size_t n, mbstate_t * __restrict ps)
{
	_GB18030State *gs;
	wchar_t wch;
	int ch, len, ocount;
	size_t ncopy;

	gs = (_GB18030State *)ps;

	if (gs->count < 0 || gs->count > sizeof(gs->bytes)) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == nullptr) {
		s = "";
		n = 1;
		pwc = nullptr;
	}

	ncopy = MIN(MIN(n, MB_CUR_MAX), sizeof(gs->bytes) - gs->count);
	memcpy(gs->bytes + gs->count, s, ncopy);
	ocount = gs->count;
	gs->count += ncopy;
	s = (char *)gs->bytes;
	n = gs->count;

	if (n == 0)
		/* Incomplete multibyte sequence */
		return ((size_t)-2);

	/*
	 * Single byte:		[00-7f]
	 * Two byte:		[81-fe][40-7e,80-fe]
	 * Four byte:		[81-fe][30-39][81-fe][30-39]
	 */
	ch = (unsigned char)*s++;
	if (ch <= 0x7f) {
		len = 1;
		wch = ch;
	} else if (ch >= 0x81 && ch <= 0xfe) {
		wch = ch;
		if (n < 2)
			return ((size_t)-2);
		ch = (unsigned char)*s++;
		if ((ch >= 0x40 && ch <= 0x7e) || (ch >= 0x80 && ch <= 0xfe)) {
			wch = (wch << 8) | ch;
			len = 2;
		} else if (ch >= 0x30 && ch <= 0x39) {
			/*
			 * Strip high bit off the wide character we will
			 * eventually output so that it is positive when
			 * cast to wint_t on 32-bit twos-complement machines.
			 */
			wch = ((wch & 0x7f) << 8) | ch;
			if (n < 3)
				return ((size_t)-2);
			ch = (unsigned char)*s++;
			if (ch < 0x81 || ch > 0xfe)
				goto ilseq;
			wch = (wch << 8) | ch;
			if (n < 4)
				return ((size_t)-2);
			ch = (unsigned char)*s++;
			if (ch < 0x30 || ch > 0x39)
				goto ilseq;
			wch = (wch << 8) | ch;
			len = 4;
		} else
			goto ilseq;
	} else
		goto ilseq;

	if (pwc != nullptr)
		*pwc = wch;
	gs->count = 0;
	return (wch == L'\0' ? 0 : len - ocount);
ilseq:
	errno = EILSEQ;
	return ((size_t)-1);
}

size_t
_GB18030_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	_GB18030State *gs;
	size_t len;
	int c;

	gs = (_GB18030State *)ps;

	if (gs->count != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == nullptr)
		/* Reset to initial shift state (no-op) */
		return (1);
	if ((wc & ~0x7fffffff) != 0)
		goto ilseq;
	if (wc & 0x7f000000) {
		/* Replace high bit that mbrtowc() removed. */
		wc |= 0x80000000;
		c = (wc >> 24) & 0xff;
		if (c < 0x81 || c > 0xfe)
			goto ilseq;
		*s++ = c;
		c = (wc >> 16) & 0xff;
		if (c < 0x30 || c > 0x39)
			goto ilseq;
		*s++ = c;
		c = (wc >> 8) & 0xff;
		if (c < 0x81 || c > 0xfe)
			goto ilseq;
		*s++ = c;
		c = wc & 0xff;
		if (c < 0x30 || c > 0x39)
			goto ilseq;
		*s++ = c;
		len = 4;
	} else if (wc & 0x00ff0000)
		goto ilseq;
	else if (wc & 0x0000ff00) {
		c = (wc >> 8) & 0xff;
		if (c < 0x81 || c > 0xfe)
			goto ilseq;
		*s++ = c;
		c = wc & 0xff;
		if (c < 0x40 || c == 0x7f || c == 0xff)
			goto ilseq;
		*s++ = c;
		len = 2;
	} else if (wc <= 0x7f) {
		*s++ = wc;
		len = 1;
	} else
		goto ilseq;

	return (len);
ilseq:
	errno = EILSEQ;
	return ((size_t)-1);
}

size_t
_GB18030_mbsnrtowcs(wchar_t * __restrict dst,
    const char ** __restrict src, size_t nms, size_t len,
    mbstate_t * __restrict ps)
{
	return (__mbsnrtowcs_std(dst, src, nms, len, ps, _GB18030_mbrtowc));
}

size_t
_GB18030_wcsnrtombs(char * __restrict dst,
    const wchar_t ** __restrict src, size_t nwc, size_t len,
    mbstate_t * __restrict ps)
{
	return (__wcsnrtombs_std(dst, src, nwc, len, ps, _GB18030_wcrtomb));
}

/* setrunelocale.c */
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



#undef _CurrentRuneLocale
extern _RuneLocale const *_CurrentRuneLocale;
/*
 * A cached version of the runes for this thread.  Used by ctype.h
 */
thread_local const _RuneLocale *_ThreadRuneLocale;

extern int __mb_sb_limit;

extern _RuneLocale	*_Read_RuneMagi(const char *);

int		__setrunelocale(struct xlocale_ctype *l, const char *);

void
destruct_ctype(void *v)
{
	struct xlocale_ctype *l = (struct xlocale_ctype *)v;

	if (&_DefaultRuneLocale != l->runes)
		free(l->runes);
	free(l);
}

const _RuneLocale *
__getCurrentRuneLocale(void)
{

	return (XLOCALE_CTYPE(__get_locale())->runes);
}

void
free_runes(_RuneLocale *rl)
{
	if ((rl != &_DefaultRuneLocale) && (rl)) {
		free(rl);
	}
}

int
__setrunelocale(struct xlocale_ctype *l, const char *encoding)
{
	_RuneLocale *rl;
	int ret;
	char *path;
	struct xlocale_ctype saved = *l;

	/*
	 * The "C" and "POSIX" locale are always here.
	 */
	if (strcmp(encoding, "C") == 0 || strcmp(encoding, "POSIX") == 0) {
		free_runes(saved.runes);
		(void) _none_init(l, (_RuneLocale*)&_DefaultRuneLocale);
		return (0);
	}

	/* Range checking not needed, encoding length already checked before */
	if (asprintf(&path, "%s/%s/LC_CTYPE", _PathLocale, encoding) == -1)
		return (errno);

	if ((rl = _Read_RuneMagi(path)) == nullptr) {
		free(path);
		errno = EINVAL;
		return (errno);
	}
	free(path);

	l->__mbrtowc = nullptr;
	l->__mbsinit = nullptr;
	l->__mbsnrtowcs = nullptr;
	l->__wcrtomb = nullptr;
	l->__wcsnrtombs = nullptr;

	rl->__sputrune = nullptr;
	rl->__sgetrune = nullptr;
	if (strcmp(rl->__encoding, "NONE:US-ASCII") == 0)
		ret = _ascii_init(l, rl);
	else if (strncmp(rl->__encoding, "NONE", 4) == 0)
		ret = _none_init(l, rl);
	else if (strcmp(rl->__encoding, "UTF-8") == 0)
		ret = _UTF8_init(l, rl);
	else if (strcmp(rl->__encoding, "EUC-CN") == 0)
		ret = _EUC_CN_init(l, rl);
	else if (strcmp(rl->__encoding, "EUC-JP") == 0)
		ret = _EUC_JP_init(l, rl);
	else if (strcmp(rl->__encoding, "EUC-KR") == 0)
		ret = _EUC_KR_init(l, rl);
	else if (strcmp(rl->__encoding, "EUC-TW") == 0)
		ret = _EUC_TW_init(l, rl);
	else if (strcmp(rl->__encoding, "GB18030") == 0)
		ret = _GB18030_init(l, rl);
	else if (strcmp(rl->__encoding, "GB2312") == 0)
		ret = _GB2312_init(l, rl);
	else if (strcmp(rl->__encoding, "GBK") == 0)
		ret = _GBK_init(l, rl);
	else if (strcmp(rl->__encoding, "BIG5") == 0)
		ret = _BIG5_init(l, rl);
	else if (strcmp(rl->__encoding, "MSKanji") == 0)
		ret = _MSKanji_init(l, rl);
	else
		ret = EFTYPE;

	if (ret == 0) {
		/* Free the old runes if it exists. */
		free_runes(saved.runes);
		/* Reset the mbstates */
		memset(&l->c16rtomb, 0, sizeof(l->c16rtomb));
		memset(&l->c32rtomb, 0, sizeof(l->c32rtomb));
		memset(&l->mblen, 0, sizeof(l->mblen));
		memset(&l->mbrlen, 0, sizeof(l->mbrlen));
		memset(&l->mbrtoc16, 0, sizeof(l->mbrtoc16));
		memset(&l->mbrtoc32, 0, sizeof(l->mbrtoc32));
		memset(&l->mbrtowc, 0, sizeof(l->mbrtowc));
		memset(&l->mbsnrtowcs, 0, sizeof(l->mbsnrtowcs));
		memset(&l->mbsrtowcs, 0, sizeof(l->mbsrtowcs));
		memset(&l->mbtowc, 0, sizeof(l->mbtowc));
		memset(&l->wcrtomb, 0, sizeof(l->wcrtomb));
		memset(&l->wcsnrtombs, 0, sizeof(l->wcsnrtombs));
		memset(&l->wcsrtombs, 0, sizeof(l->wcsrtombs));
		memset(&l->wctomb, 0, sizeof(l->wctomb));
	} else {
		/* Restore the saved version if this failed. */
		memcpy(l, &saved, sizeof(struct xlocale_ctype));
		free(rl);
	}

	return (ret);
}

int
__wrap_setrunelocale(const char *locale)
{
	int ret = __setrunelocale(&__xlocale_global_ctype, locale);

	if (ret != 0) {
		errno = ret;
		return (_LDP_ERROR);
	}
	__mb_cur_max = __xlocale_global_ctype.__mb_cur_max;
	__mb_sb_limit = __xlocale_global_ctype.__mb_sb_limit;
	_CurrentRuneLocale = __xlocale_global_ctype.runes;
	return (_LDP_LOADED);
}

void
__set_thread_rune_locale(locale_t loc)
{

	if (loc == nullptr) {
		_ThreadRuneLocale = &_DefaultRuneLocale;
	} else if (loc == LC_GLOBAL_LOCALE) {
		_ThreadRuneLocale = 0;
	} else {
		_ThreadRuneLocale = XLOCALE_CTYPE(loc)->runes;
	}
}

void *
__ctype_load(const char *locale, locale_t unused __unused)
{
	struct xlocale_ctype *l = (struct xlocale_ctype *)calloc(sizeof(struct xlocale_ctype), 1);
	if (l == nullptr)
		return (nullptr);

	l->header.header.destructor = destruct_ctype;
	if (__setrunelocale(l, locale)) {
		free(l);
		return (nullptr);
	}
	return (l);
}

} // namespace
