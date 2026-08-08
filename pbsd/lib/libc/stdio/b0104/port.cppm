module;

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>

#if __has_include(<xlocale.h>)
#include <xlocale.h>
#else
#include <locale.h>
#endif

#ifndef __va_list
#define __va_list	va_list
#endif

export module pbsd.lib.libc.stdio.b0104;

export namespace pbsd::lib_libc_stdio::b0104 {

namespace detail {

static locale_t
get_real_locale(locale_t locale)
{

	switch ((intptr_t)locale) {
	case 1:
		return (newlocale(LC_ALL_MASK, "C", (locale_t)0));
	case -1:
		return (LC_GLOBAL_LOCALE);
	default:
		return (locale);
	}
}

static inline locale_t
get_locale(void)
{
	locale_t cur;

	cur = uselocale((locale_t)0);
	if (cur == NULL)
		return (LC_GLOBAL_LOCALE);
	return (cur);
}

#if !__has_include(<xlocale.h>)
static int
vfprintf_l(FILE *fp, locale_t locale, const char *fmt, va_list ap)
{
	locale_t loc, old;
	int ret;

	loc = get_real_locale(locale);
	old = uselocale(loc == LC_GLOBAL_LOCALE ? (locale_t)0 : loc);
	ret = vfprintf(fp, fmt, ap);
	uselocale(old);
	return (ret);
}

static int
vsscanf_l(const char *str, locale_t locale, const char *fmt, va_list ap)
{
	locale_t loc, old;
	int ret;

	loc = get_real_locale(locale);
	old = uselocale(loc == LC_GLOBAL_LOCALE ? (locale_t)0 : loc);
	ret = vsscanf(str, fmt, ap);
	uselocale(old);
	return (ret);
}
#endif

static thread_local FILE *flock_fp;

static int
svfscanf(FILE *fp, locale_t locale, const char *fmt, __va_list ap)
{
	locale_t loc, old;
	int retval;

	loc = get_real_locale(locale);
	old = uselocale(loc == LC_GLOBAL_LOCALE ? (locale_t)0 : loc);
	retval = vfscanf(fp, fmt, ap);
	uselocale(old);
	return (retval);
}

} /* namespace detail */

#define FIX_LOCALE(l)	((l) = detail::get_real_locale(l))

#define	FLOCKFILE_CANCELSAFE(fp)					\
	flockfile(pbsd::lib_libc_stdio::b0104::detail::flock_fp = (fp)); \
	for (int _b0104_done = 0; !_b0104_done;				\
	    funlockfile(pbsd::lib_libc_stdio::b0104::detail::flock_fp), \
	    _b0104_done = 1)
#define	FUNLOCKFILE_CANCELSAFE()	((void)0)

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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

int
fprintf(FILE * __restrict fp, const char * __restrict fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = detail::vfprintf_l(fp, detail::get_locale(), fmt, ap);
	va_end(ap);
	return (ret);
}
int
fprintf_l(FILE * __restrict fp, locale_t locale, const char * __restrict fmt, ...)
{
	int ret;
	va_list ap;

	FIX_LOCALE(locale);
	va_start(ap, fmt);
	ret = detail::vfprintf_l(fp, locale, fmt, ap);
	va_end(ap);
	return (ret);
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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

int
sscanf(const char * __restrict str, char const * __restrict fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vsscanf(str, fmt, ap);
	va_end(ap);
	return (ret);
}
int
sscanf_l(const char * __restrict str, locale_t locale,
		char const * __restrict fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = detail::vsscanf_l(str, locale, fmt, ap);
	va_end(ap);
	return (ret);
}

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Donn Seeley at UUNET Technologies, Inc.
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

int
vscanf_l(locale_t locale, const char * __restrict fmt, __va_list ap)
{
	int retval;

	FIX_LOCALE(locale);
	FLOCKFILE_CANCELSAFE(stdin);
	retval = detail::svfscanf(stdin, locale, fmt, ap);
	FUNLOCKFILE_CANCELSAFE();
	return (retval);
}
int
vscanf(const char * __restrict fmt, __va_list ap)
{
	return vscanf_l(detail::get_locale(), fmt, ap);
}

} /* namespace pbsd::lib_libc_stdio::b0104 */
