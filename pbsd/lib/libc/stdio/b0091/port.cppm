module;

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <clocale>
#include <cstdint>
#include <cstdio>
#include <cwchar>
#include <locale.h>

extern "C" {
wint_t fgetwc_l(FILE *fp, locale_t loc);
wint_t fputwc_l(wchar_t wc, FILE *fp, locale_t loc);
}

export module pbsd.lib.libc.stdio.b0091;

namespace {

::locale_t
b0091_get_C_locale(void)
{
	static ::locale_t c_locale = nullptr;
	static int inited = 0;

	if (!inited) {
		c_locale = ::newlocale(LC_ALL_MASK, "C", (::locale_t)0);
		inited = 1;
	}
	return (c_locale);
}

static inline ::locale_t
get_real_locale(::locale_t locale)
{
	switch ((intptr_t)locale) {
	case 0:
		return (b0091_get_C_locale());
	case -1:
		return (LC_GLOBAL_LOCALE);
	default:
		return (locale);
	}
}

#define FIX_LOCALE(l) (l = get_real_locale(l))

static inline ::locale_t
__get_locale(void)
{
	::locale_t loc;

	loc = ::uselocale((::locale_t)0);
	if (loc == (::locale_t)0)
		return (LC_GLOBAL_LOCALE);
	return (loc);
}

} /* namespace */

export namespace pbsd::lib_libc_stdio::b0091 {

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Tim J. Robbins.
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
 * Synonym for fgetwc(stdin).
 */
::wint_t
getwchar(void)
{
	return (::fgetwc(stdin));
}
::wint_t
getwchar_l(::locale_t locale)
{
	return (::fgetwc_l(stdin, locale));
}

/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2002 Tim J. Robbins.
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
 * Synonym for fputwc(wc, stdout).
 */
::wint_t
putwchar_l(wchar_t wc, ::locale_t locale)
{
	FIX_LOCALE(locale);
	return (::fputwc_l(wc, stdout, locale));
}
::wint_t
putwchar(wchar_t wc)
{
	return putwchar_l(wc, __get_locale());
}

} /* namespace pbsd::lib_libc_stdio::b0091 */
