// b0091 -- C++23 port of
//
//	hbsd/src/lib/libc/stdio/fsetpos.c
//	hbsd/src/lib/libc/stdio/getwchar.c
//	hbsd/src/lib/libc/stdio/putwchar.c
//
// Faithful: every body is the original expression, unchanged.  The host
// compatibility glue below (hbsd_fpos_t alias, __get_locale, FIX_LOCALE,
// fgetwc_l, fputwc_l) mirrors oracle.c line for line; see the comment at the
// top of oracle.c for why each piece is needed on a glibc host.

module;

#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <sys/types.h>
#include <wchar.h>

#ifndef LONG_BIT
#define LONG_BIT (sizeof(long) * 8)
#endif

#undef getwchar
#undef putwchar

export module pbsd.lib.libc.stdio.b0091;

namespace pbsd::lib_libc_stdio::b0091 {

//
// Host compatibility glue (module linkage, not part of the port proper).
//

locale_t
__get_locale()
{
	return (uselocale((locale_t)0));
}

#define FIX_LOCALE(loc) if (loc == NULL) loc = __get_locale()

wint_t
fgetwc_l(FILE *fp, locale_t locale)
{
	locale_t old;
	wint_t r;
	int e;

	FIX_LOCALE(locale);
	old = uselocale(locale);
	r = fgetwc(fp);
	e = errno;
	uselocale(old);
	errno = e;
	return (r);
}

wint_t
fputwc_l(wchar_t wc, FILE *fp, locale_t locale)
{
	locale_t old;
	wint_t r;
	int e;

	FIX_LOCALE(locale);
	old = uselocale(locale);
	r = fputwc(wc, fp);
	e = errno;
	uselocale(old);
	errno = e;
	return (r);
}

/* ------------------------------------------------------------------ */
/* lib/libc/stdio/fsetpos.c						*/
/* ------------------------------------------------------------------ */

/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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

export using hbsd_fpos_t = ::off_t;

/*
 * fsetpos: like fseek.
 */
export int
fsetpos(FILE *iop, const hbsd_fpos_t *pos)
{
	return (fseeko(iop, (off_t)*pos, SEEK_SET));
}

/* ------------------------------------------------------------------ */
/* lib/libc/stdio/getwchar.c						*/
/* ------------------------------------------------------------------ */

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
export wint_t
getwchar(void)
{
	return (fgetwc(stdin));
}
export wint_t
getwchar_l(locale_t locale)
{
	return (fgetwc_l(stdin, locale));
}

/* ------------------------------------------------------------------ */
/* lib/libc/stdio/putwchar.c						*/
/* ------------------------------------------------------------------ */

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
export wint_t
putwchar_l(wchar_t wc, locale_t locale)
{
	FIX_LOCALE(locale);
	return (fputwc_l(wc, stdout, locale));
}
export wint_t
putwchar(wchar_t wc)
{
	return putwchar_l(wc, __get_locale());
}

} // namespace pbsd::lib_libc_stdio::b0091
