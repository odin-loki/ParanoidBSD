/*-
 * PBSD batch b0145 -- C++23 module port of
 *
 *	hbsd/src/lib/libc/locale/wcstoul.c
 *	hbsd/src/lib/libc/locale/wcstoumax.c
 *	hbsd/src/lib/libc/locale/lnumeric.c
 */

module;

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <cwctype>

export module pbsd.lib.libc.locale.b0145;

#define _LDP_LOADED	1
#define _LDP_ERROR	(-1)

enum {
	PORT_XLC_NUMERIC = 2,
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
	xlocale_component_header header;
};

struct xlocale_numeric {
	xlocale_component header;
	char		*buffer;
	lc_numeric_T	locale;
};

struct port_xlocale {
	int		using_numeric_locale;
	int		numeric_locale_changed;
	void		*components[8];
};

typedef struct port_xlocale *port_locale_t;

#define LCNUMERIC_SIZE (sizeof(::lc_numeric_T) / sizeof(char *))

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

extern "C" {
int	__part_load_locale(const char *, int *, char **, const char *, int,
	    int, const char **);
const char	*__fix_locale_grouping_str(const char *);
void	pbsd_reset_hooks(void);
}

extern pbsd_numeric_part_load_hook_t	pbsd_numeric_part_load_hook;
extern pbsd_fix_grouping_hook_t		pbsd_fix_grouping_hook;

extern xlocale_numeric	port___xlocale_global_numeric;
extern port_xlocale	port___xlocale_global_locale;

int	pbsd_iswspace_l(wint_t, port_locale_t);

namespace pbsd::lib_libc_locale::b0145 {

static inline port_locale_t
port_fix_locale(port_locale_t l)
{
	if (l == nullptr)
		return (&port___xlocale_global_locale);
	return (l);
}

#define FIX_LOCALE(l)	((l) = port_fix_locale(l))

port_locale_t
port_get_locale()
{
	return (&port___xlocale_global_locale);
}

} // namespace pbsd::lib_libc_locale::b0145

export namespace pbsd::lib_libc_locale::b0145 {

using locale_t = port_locale_t;
using lc_numeric_T = ::lc_numeric_T;
using xlocale_numeric = ::xlocale_numeric;

struct xlocale {
	int		using_numeric_locale;
	int		numeric_locale_changed;
	void		*components[8];
};

inline void
init_locale()
{
	std::memset(&port___xlocale_global_locale, 0,
	    sizeof(port___xlocale_global_locale));
}

inline int
numeric_locale_changed_flag()
{
	return (port___xlocale_global_locale.numeric_locale_changed);
}

inline void
reset_numeric_locale_changed()
{
	port___xlocale_global_locale.numeric_locale_changed = 0;
}

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

#define iswspace_l	pbsd_iswspace_l

/*
 * Convert a wide character string to an unsigned long integer.
 */
unsigned long
wcstoul_l(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr,
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
#undef iswspace_l
unsigned long
wcstoul(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr, int base)
{
	return wcstoul_l(nptr, endptr, base, port_get_locale());
}

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

#define iswspace_l	pbsd_iswspace_l

/*
 * Convert a wide character string to a uintmax_t integer.
 */
uintmax_t
wcstoumax_l(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr,
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
#undef iswspace_l
uintmax_t
wcstoumax(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr,
    int base)
{
	return wcstoumax_l(nptr, endptr, base, port_get_locale());
}

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

static char	port_numempty[] = { CHAR_MAX, '\0' };

static const ::lc_numeric_T port__C_numeric_locale = {
	".",		/* decimal_point */
	"",		/* thousands_sep */
	port_numempty	/* grouping */
};

static void
destruct_numeric(void *v)
{
	::xlocale_numeric *l = static_cast<::xlocale_numeric *>(v);
	if (l->buffer)
		free(l->buffer);
	free(l);
}

static void
atomic_store_rel_int(int *p, int v)
{
	*p = v;
}

static void
xlocale_release(void *v)
{
	::xlocale_component *c = static_cast<::xlocale_component *>(v);

	if (c != nullptr && c->header.destructor != nullptr)
		c->header.destructor(v);
	else
		free(v);
}

static int
numeric_load_locale(::xlocale_numeric *loc, int *using_locale,
    int *changed, const char *name)
{
	int ret;
	::lc_numeric_T *l = &loc->locale;

	ret = __part_load_locale(name, using_locale,
	    &loc->buffer, "LC_NUMERIC",
	    LCNUMERIC_SIZE, LCNUMERIC_SIZE,
	    (const char **)l);
	if (ret == _LDP_LOADED) {
		/* Can't be empty according to C99 */
		if (*l->decimal_point == '\0')
			l->decimal_point =
			    port__C_numeric_locale.decimal_point;
		l->grouping =
		    __fix_locale_grouping_str(l->grouping);
	}
	if (ret != _LDP_ERROR)
		atomic_store_rel_int(changed, 1);
	return (ret);
}

int
__numeric_load_locale(const char *name)
{
	return (numeric_load_locale(&port___xlocale_global_numeric,
	    &port___xlocale_global_locale.using_numeric_locale,
	    &port___xlocale_global_locale.numeric_locale_changed, name));
}

void *
__numeric_load(const char *name, locale_t l)
{
	::xlocale_numeric *newloc = static_cast<::xlocale_numeric *>(
	    calloc(sizeof(::xlocale_numeric), 1));
	if (newloc == NULL)
		return (NULL);
	newloc->header.header.destructor = destruct_numeric;
	if (numeric_load_locale(newloc, &l->using_numeric_locale,
	    &l->numeric_locale_changed, name) == _LDP_ERROR) {
		xlocale_release(newloc);
		return (NULL);
	}
	return (newloc);
}

::lc_numeric_T *
__get_current_numeric_locale(locale_t loc)
{
	return (loc->using_numeric_locale ? &(static_cast<::xlocale_numeric *>(
	    loc->components[PORT_XLC_NUMERIC])->locale) :
	    (::lc_numeric_T *)&port__C_numeric_locale);
}

} /* namespace pbsd::lib_libc_locale::b0145 */

xlocale_numeric	port___xlocale_global_numeric;
port_xlocale	port___xlocale_global_locale;

int
pbsd_iswspace_l(wint_t wc, port_locale_t locale)
{
	(void)locale;
	return (iswspace(wc));
}
