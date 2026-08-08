/*
 * PBSD batch b0145 -- reference oracle.
 *
 * Original HardenedBSD sources concatenated, with every function renamed with
 * a ref_ prefix.  Function bodies are UNMODIFIED.
 *
 * Sources:
 *   hbsd/src/lib/libc/locale/wcstoul.c
 *   hbsd/src/lib/libc/locale/wcstoumax.c
 *   hbsd/src/lib/libc/locale/lnumeric.c
 */

#ifndef LONG_BIT
#define LONG_BIT	(sizeof(long) * CHAR_BIT)
#endif

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#define _LDP_LOADED	1
#define _LDP_ERROR	(-1)

enum {
	XLC_NUMERIC = 2,
};

struct lc_numeric_T {
	const char	*decimal_point;
	const char	*thousands_sep;
	const char	*grouping;
};

struct xlocale_component_header {
	void		(*destructor)(void *);
};

struct xlocale_component {
	struct xlocale_component_header header;
};

struct xlocale_numeric {
	struct xlocale_component header;
	char		*buffer;
	struct lc_numeric_T locale;
};

struct xlocale {
	int		using_numeric_locale;
	int		numeric_locale_changed;
	void		*components[8];
};

typedef struct xlocale *locale_t;

#define LCNUMERIC_SIZE (sizeof(struct lc_numeric_T) / sizeof(char *))

#define FIX_LOCALE(loc)	if ((loc) == NULL) (loc) = ref___get_locale()
#define __get_locale()	ref___get_locale()

typedef struct {
	int		ret;
	const char	*decimal_point;
	const char	*thousands_sep;
	const char	*grouping;
	int		empty_decimal;
	unsigned int	call_count;
} pbsd_numeric_part_load_hook_t;

typedef struct {
	const char	*(*fn)(const char *);
	unsigned int	call_count;
} pbsd_fix_grouping_hook_t;

pbsd_numeric_part_load_hook_t	pbsd_numeric_part_load_hook;
pbsd_fix_grouping_hook_t	pbsd_fix_grouping_hook;

struct xlocale_numeric	ref___xlocale_global_numeric;
struct xlocale		ref___xlocale_global_locale;

#define __xlocale_global_numeric	ref___xlocale_global_numeric
#define __xlocale_global_locale	ref___xlocale_global_locale

void
pbsd_reset_hooks(void)
{
	memset(&pbsd_numeric_part_load_hook, 0,
	    sizeof(pbsd_numeric_part_load_hook));
	memset(&pbsd_fix_grouping_hook, 0, sizeof(pbsd_fix_grouping_hook));
	pbsd_numeric_part_load_hook.ret = _LDP_LOADED;
	pbsd_numeric_part_load_hook.decimal_point = ".";
	pbsd_numeric_part_load_hook.thousands_sep = "";
	pbsd_numeric_part_load_hook.grouping = "\3";
	pbsd_fix_grouping_hook.fn = NULL;
}

locale_t
ref___get_locale(void)
{
	return (&ref___xlocale_global_locale);
}

int
iswspace_l(wint_t wc, locale_t locale)
{
	(void)locale;
	return (iswspace(wc));
}

static void
atomic_store_rel_int(int *p, int v)
{
	*p = v;
}

const char *
__fix_locale_grouping_str(const char *s)
{
	pbsd_fix_grouping_hook.call_count++;
	if (pbsd_fix_grouping_hook.fn != NULL)
		return (pbsd_fix_grouping_hook.fn(s));
	return (s);
}

int
__part_load_locale(const char *name, int *using_locale, char **buffer,
    const char *category, int size_full, int size_min, const char **dst)
{
	const char **fields;
	int i;

	(void)name;
	(void)category;
	(void)size_full;
	(void)size_min;
	pbsd_numeric_part_load_hook.call_count++;
	fields = (const char **)dst;
	if (pbsd_numeric_part_load_hook.ret != _LDP_LOADED)
		return (pbsd_numeric_part_load_hook.ret);
	for (i = 0; i < LCNUMERIC_SIZE; i++)
		fields[i] = NULL;
	fields[0] = pbsd_numeric_part_load_hook.empty_decimal ? "" :
	    pbsd_numeric_part_load_hook.decimal_point;
	fields[1] = pbsd_numeric_part_load_hook.thousands_sep;
	fields[2] = pbsd_numeric_part_load_hook.grouping;
	if (using_locale != NULL)
		*using_locale = 1;
	if (buffer != NULL)
		*buffer = NULL;
	return (_LDP_LOADED);
}

static void
xlocale_release(void *v)
{
	struct xlocale_component *c = v;

	if (c != NULL && c->header.destructor != NULL)
		c->header.destructor(v);
	else
		free(v);
}

static void __attribute__((constructor))
ref_locale_init(void)
{
	pbsd_reset_hooks();
}

/* ------------------------------------------------------------------ */
/* wcstoul.c								*/
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 * Convert a wide character string to an unsigned long integer.
 */
unsigned long
ref_wcstoul_l(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr,
		int base, locale_t locale)
{
	const wchar_t *s;
	unsigned long acc;
	wchar_t c;
	unsigned long cutoff;
	int neg, any, cutlim;
	FIX_LOCALE(locale);

	/*
	 * See strtol for comments as to the logic used.
	 */
	s = nptr;
	do {
		c = *s++;
	} while (iswspace_l(c, locale));
	if (c == L'-') {
		neg = 1;
		c = *s++;
	} else {
		neg = 0;
		if (c == L'+')
			c = *s++;
	}
	if ((base == 0 || base == 16) &&
	    c == L'0' && (*s == L'x' || *s == L'X') &&
	    ((s[1] >= L'0' && s[1] <= L'9') ||
	    (s[1] >= L'A' && s[1] <= L'F') ||
	    (s[1] >= L'a' && s[1] <= L'f'))) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if ((base == 0 || base == 2) &&
	    c == L'0' && (*s == L'b' || *s == L'B') &&
	    (s[1] >= L'0' && s[1] <= L'1')) {
		c = s[1];
		s += 2;
		base = 2;
	}
	if (base == 0)
		base = c == L'0' ? 8 : 10;
	acc = any = 0;
	if (base < 2 || base > 36)
		goto noconv;

	cutoff = ULONG_MAX / base;
	cutlim = ULONG_MAX % base;
	for ( ; ; c = *s++) {
#ifdef notyet
		if (iswdigit_l(c, locale))
			c = digittoint_l(c, locale);
		else
#endif
		if (c >= L'0' && c <= L'9')
			c -= L'0';
		else if (c >= L'A' && c <= L'Z')
			c -= L'A' - 10;
		else if (c >= L'a' && c <= L'z')
			c -= L'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = ULONG_MAX;
		errno = ERANGE;
	} else if (!any) {
noconv:
		errno = EINVAL;
	} else if (neg)
		acc = -acc;
	if (endptr != NULL)
		*endptr = (wchar_t *)(any ? s - 1 : nptr);
	return (acc);
}
unsigned long
ref_wcstoul(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr, int base)
{
	return ref_wcstoul_l(nptr, endptr, base, __get_locale());
}

/* ------------------------------------------------------------------ */
/* wcstoumax.c								*/
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 * Convert a wide character string to a uintmax_t integer.
 */
uintmax_t
ref_wcstoumax_l(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr,
    int base, locale_t locale)
{
	const wchar_t *s;
	uintmax_t acc;
	wchar_t c;
	uintmax_t cutoff;
	int neg, any, cutlim;
	FIX_LOCALE(locale);

	/*
	 * See strtoimax for comments as to the logic used.
	 */
	s = nptr;
	do {
		c = *s++;
	} while (iswspace_l(c, locale));
	if (c == L'-') {
		neg = 1;
		c = *s++;
	} else {
		neg = 0;
		if (c == L'+')
			c = *s++;
	}
	if ((base == 0 || base == 16) &&
	    c == L'0' && (*s == L'x' || *s == L'X') &&
	    ((s[1] >= L'0' && s[1] <= L'9') ||
	    (s[1] >= L'A' && s[1] <= L'F') ||
	    (s[1] >= L'a' && s[1] <= L'f'))) {
		c = s[1];
		s += 2;
		base = 16;
	}
	if ((base == 0 || base == 2) &&
	    c == L'0' && (*s == L'b' || *s == L'B') &&
	    (s[1] >= L'0' && s[1] <= L'1')) {
		c = s[1];
		s += 2;
		base = 2;
	}
	if (base == 0)
		base = c == L'0' ? 8 : 10;
	acc = any = 0;
	if (base < 2 || base > 36)
		goto noconv;

	cutoff = UINTMAX_MAX / base;
	cutlim = UINTMAX_MAX % base;
	for ( ; ; c = *s++) {
#ifdef notyet
		if (iswdigit_l(c, locale))
			c = digittoint_l(c, locale);
		else
#endif
		if (c >= L'0' && c <= L'9')
			c -= L'0';
		else if (c >= L'A' && c <= L'Z')
			c -= L'A' - 10;
		else if (c >= L'a' && c <= L'z')
			c -= L'a' - 10;
		else
			break;
		if (c >= base)
			break;
		if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
			any = -1;
		else {
			any = 1;
			acc *= base;
			acc += c;
		}
	}
	if (any < 0) {
		acc = UINTMAX_MAX;
		errno = ERANGE;
	} else if (!any) {
noconv:
		errno = EINVAL;
	} else if (neg)
		acc = -acc;
	if (endptr != NULL)
		*endptr = (wchar_t *)(any ? s - 1 : nptr);
	return (acc);
}
uintmax_t
ref_wcstoumax(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr,
    int base)
{
	return ref_wcstoumax_l(nptr, endptr, base, __get_locale());
}

/* ------------------------------------------------------------------ */
/* lnumeric.c								*/
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2000, 2001 Alexey Zelkin <phantom@FreeBSD.org>
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

static char	ref_numempty[] = { CHAR_MAX, '\0' };

static const struct lc_numeric_T ref__C_numeric_locale = {
	".",		/* decimal_point */
	"",		/* thousands_sep */
	ref_numempty	/* grouping */
};

static void
ref_destruct_numeric(void *v)
{
	struct xlocale_numeric *l = v;
	if (l->buffer)
		free(l->buffer);
	free(l);
}

#define destruct_numeric	ref_destruct_numeric

static int
ref_numeric_load_locale(struct xlocale_numeric *loc, int *using_locale,
    int *changed, const char *name)
{
	int ret;
	struct lc_numeric_T *l = &loc->locale;

	ret = __part_load_locale(name, using_locale,
	    &loc->buffer, "LC_NUMERIC",
	    LCNUMERIC_SIZE, LCNUMERIC_SIZE,
	    (const char**)l);
	if (ret == _LDP_LOADED) {
		/* Can't be empty according to C99 */
		if (*l->decimal_point == '\0')
			l->decimal_point =
			    ref__C_numeric_locale.decimal_point;
		l->grouping =
		    __fix_locale_grouping_str(l->grouping);
	}
	if (ret != _LDP_ERROR)
		atomic_store_rel_int(changed, 1);
	return (ret);
}

#define numeric_load_locale	ref_numeric_load_locale

int
ref___numeric_load_locale(const char *name)
{
	return (numeric_load_locale(&__xlocale_global_numeric,
	    &__xlocale_global_locale.using_numeric_locale,
	    &__xlocale_global_locale.numeric_locale_changed, name));
}

void *
ref___numeric_load(const char *name, locale_t l)
{
	struct xlocale_numeric *new = calloc(sizeof(struct xlocale_numeric),
	    1);
	if (new == NULL)
		return (NULL);
	new->header.header.destructor = destruct_numeric;
	if (numeric_load_locale(new, &l->using_numeric_locale,
	    &l->numeric_locale_changed, name) == _LDP_ERROR) {
		xlocale_release(new);
		return (NULL);
	}
	return (new);
}

struct lc_numeric_T *
ref___get_current_numeric_locale(locale_t loc)
{
	return (loc->using_numeric_locale ?
	    &((struct xlocale_numeric *)loc->components[XLC_NUMERIC])->locale :
	    (struct lc_numeric_T *)&ref__C_numeric_locale);
}

void
ref_release_numeric(void *v)
{
	xlocale_release(v);
}
