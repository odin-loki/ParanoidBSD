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

module;

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdarg.h>
#include <stdio.h>
#include <locale.h>

export module pbsd.lib.libc.stdio.b0106s4;

/*
 * The scanf back end and the current-locale accessor live outside this
 * translation unit, exactly as __svfscanf() and __get_locale() live elsewhere
 * in libc.
 */
extern "C" locale_t __get_locale(void);
extern "C" int __svfscanf(FILE *, locale_t, const char *, va_list);

#define FIX_LOCALE(loc)							\
	do {								\
		if ((loc) == NULL)					\
			(loc) = __get_locale();				\
	} while (0)

#define FLOCKFILE_CANCELSAFE(fp)					\
	{								\
		FILE *_shim_lockfp = (fp);				\
		flockfile(_shim_lockfp);
#define FUNLOCKFILE_CANCELSAFE()					\
		funlockfile(_shim_lockfp);				\
	}

export namespace pbsd::lib_libc_stdio::b0106s4 {

int
scanf(char const * __restrict fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	FLOCKFILE_CANCELSAFE(stdin);
	ret = __svfscanf(stdin, __get_locale(), fmt, ap);
	FUNLOCKFILE_CANCELSAFE();
	va_end(ap);
	return (ret);
}
int
scanf_l(locale_t locale, char const * __restrict fmt, ...)
{
	int ret;
	va_list ap;
	FIX_LOCALE(locale);

	va_start(ap, fmt);
	FLOCKFILE_CANCELSAFE(stdin);
	ret = __svfscanf(stdin, locale, fmt, ap);
	FUNLOCKFILE_CANCELSAFE();
	va_end(ap);
	return (ret);
}

} /* namespace pbsd::lib_libc_stdio::b0106s4 */
