/*
 * Reference oracle for batch b0106s4 (lib/libc/stdio/scanf.c).
 *
 * The original HardenedBSD source is reproduced below with every function
 * renamed with a "ref_" prefix.  The function bodies are UNMODIFIED: the
 * renaming is done with #define, and the FreeBSD libc internals the bodies
 * reach for (__svfscanf, __get_locale, FIX_LOCALE, FLOCKFILE_CANCELSAFE,
 * FUNLOCKFILE_CANCELSAFE) are supplied by the shared substrate below.  Only
 * the #include lines naming FreeBSD-private headers were dropped.
 *
 * The substrate is shared with the port: both ref_scanf/ref_scanf_l and the
 * C++23 port call the very same scanf back end, exactly as the originals both
 * call libc's single __svfscanf.  The back end records the FILE, the locale
 * and a call counter so that the harness can observe *what the wrapper handed
 * to the engine*, not merely what the engine returned.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <limits.h>
#include <locale.h>
#include <stdarg.h>
#include <stdio.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * CHAR_BIT)
#endif

/* Observable plumbing state, inspected by harness.cpp. */
locale_t pbsd_shim_svfscanf_locale;
FILE *pbsd_shim_svfscanf_file;
long pbsd_shim_svfscanf_calls;

locale_t
pbsd_shim_get_locale(void)
{
	return (uselocale((locale_t)0));
}

int
pbsd_shim_svfscanf(FILE *fp, locale_t locale, const char *fmt, va_list ap)
{
	locale_t oldloc;
	int r;

	pbsd_shim_svfscanf_locale = locale;
	pbsd_shim_svfscanf_file = fp;
	pbsd_shim_svfscanf_calls++;
	oldloc = uselocale(locale);
	r = vfscanf(fp, fmt, ap);
	uselocale(oldloc);
	return (r);
}

locale_t
__get_locale(void)
{
	return (pbsd_shim_get_locale());
}

int
__svfscanf(FILE *fp, locale_t locale, const char *fmt, va_list ap)
{
	return (pbsd_shim_svfscanf(fp, locale, fmt, ap));
}

#define FIX_LOCALE(loc)							\
	do {								\
		if ((loc) == NULL)					\
			(loc) = __get_locale();				\
	} while (0)

/*
 * FreeBSD's cancellation-safe locking macros open a block that
 * FUNLOCKFILE_CANCELSAFE() closes; keep that shape so the bodies compile
 * verbatim.
 */
#define FLOCKFILE_CANCELSAFE(fp)					\
	{								\
		FILE *_shim_lockfp = (fp);				\
		flockfile(_shim_lockfp);
#define FUNLOCKFILE_CANCELSAFE()					\
		funlockfile(_shim_lockfp);				\
	}

#undef scanf
#undef scanf_l
#define scanf ref_scanf
#define scanf_l ref_scanf_l

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
